#include "disasterdao.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QDir>
#include <QtCore/QVariant>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlError>
#include <QtSql/QSqlQuery>

static bool ensureSqliteConnection(QString* errorMessage)
{
    const QString connectionName = "app_sqlite";

    QSqlDatabase db;
    if (QSqlDatabase::contains(connectionName)) {
        db = QSqlDatabase::database(connectionName);
    } else {
        db = QSqlDatabase::addDatabase("QSQLITE", connectionName);
        const QString dbPath = QDir(QCoreApplication::applicationDirPath()).filePath("monitor.db");
        db.setDatabaseName(dbPath);
    }

    if (db.isOpen()) {
        return true;
    }

    if (!db.open()) {
        if (errorMessage) {
            *errorMessage = db.lastError().text();
        }
        return false;
    }

    QSqlQuery pragma(db);
    if (!pragma.exec("PRAGMA foreign_keys = ON;")) {
        if (errorMessage) {
            *errorMessage = pragma.lastError().text();
        }
        return false;
    }

    return true;
}

static bool ensureDisasterSchema(QSqlDatabase db, QString* errorMessage)
{
    QSqlQuery query(db);
    if (!query.exec(
            "CREATE TABLE IF NOT EXISTS disasters ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "disaster_type TEXT NOT NULL,"
            "location TEXT,"
            "occurred_at TEXT,"
            "content TEXT,"
            "system_alarm_at TEXT,"
            "severity INTEGER NOT NULL DEFAULT 0,"
            "created_at TEXT NOT NULL DEFAULT (datetime('now','localtime'))"
            ");")) {
        if (errorMessage) {
            *errorMessage = query.lastError().text();
        }
        return false;
    }

    if (!query.exec("CREATE INDEX IF NOT EXISTS idx_disasters_severity ON disasters(severity);")) {
        if (errorMessage) {
            *errorMessage = query.lastError().text();
        }
        return false;
    }

    return true;
}

static QVariant nullableText(const QString& s)
{
    const QString t = s.trimmed();
    return t.isEmpty() ? QVariant(QVariant::String) : QVariant(t);
}

static bool readDisasterRow(QSqlQuery& query, DisasterRecord* record)
{
    if (!record) {
        return true;
    }
    record->id = query.value(0).toLongLong();
    record->disasterType = query.value(1).toString();
    record->location = query.value(2).toString();
    record->occurredAt = query.value(3).toString();
    record->content = query.value(4).toString();
    record->systemAlarmAt = query.value(5).toString();
    record->severity = query.value(6).toInt();
    record->createdAt = query.value(7).toString();
    return true;
}

bool DisasterDao::createDisaster(const DisasterRecord& record, qint64* id, QString* errorMessage)
{
    const QString type = record.disasterType.trimmed();
    if (type.isEmpty()) {
        if (errorMessage) {
            *errorMessage = "灾害类型不能为空";
        }
        return false;
    }

    QString connError;
    if (!ensureSqliteConnection(&connError)) {
        if (errorMessage) {
            *errorMessage = connError;
        }
        return false;
    }
    QSqlDatabase db = QSqlDatabase::database("app_sqlite");

    QString schemaError;
    if (!ensureDisasterSchema(db, &schemaError)) {
        if (errorMessage) {
            *errorMessage = schemaError;
        }
        return false;
    }

    QSqlQuery query(db);
    query.prepare(
        "INSERT INTO disasters(disaster_type, location, occurred_at, content, system_alarm_at, severity) "
        "VALUES(?, ?, ?, ?, ?, ?);");
    query.addBindValue(type);
    query.addBindValue(nullableText(record.location));
    query.addBindValue(nullableText(record.occurredAt));
    query.addBindValue(nullableText(record.content));
    query.addBindValue(nullableText(record.systemAlarmAt));
    query.addBindValue(record.severity);

    if (!query.exec()) {
        if (errorMessage) {
            *errorMessage = query.lastError().text();
        }
        return false;
    }

    if (id) {
        *id = query.lastInsertId().toLongLong();
    }
    return true;
}

bool DisasterDao::getDisasterById(qint64 id, DisasterRecord* record, QString* errorMessage)
{
    if (id <= 0) {
        if (errorMessage) {
            *errorMessage = "灾害ID无效";
        }
        return false;
    }

    QString connError;
    if (!ensureSqliteConnection(&connError)) {
        if (errorMessage) {
            *errorMessage = connError;
        }
        return false;
    }
    QSqlDatabase db = QSqlDatabase::database("app_sqlite");

    QString schemaError;
    if (!ensureDisasterSchema(db, &schemaError)) {
        if (errorMessage) {
            *errorMessage = schemaError;
        }
        return false;
    }

    QSqlQuery query(db);
    query.prepare(
        "SELECT id, disaster_type, location, occurred_at, content, system_alarm_at, severity, created_at "
        "FROM disasters "
        "WHERE id = ? "
        "LIMIT 1;");
    query.addBindValue(id);

    if (!query.exec()) {
        if (errorMessage) {
            *errorMessage = query.lastError().text();
        }
        return false;
    }
    if (!query.next()) {
        if (errorMessage) {
            *errorMessage = "灾害记录不存在";
        }
        return false;
    }

    readDisasterRow(query, record);
    return true;
}

bool DisasterDao::updateDisaster(const DisasterPatch& patch, QString* errorMessage)
{
    if (patch.id <= 0) {
        if (errorMessage) {
            *errorMessage = "灾害ID无效";
        }
        return false;
    }

    QStringList setParts;
    QList<QVariant> binds;

    if (patch.hasDisasterType) {
        const QString type = patch.disasterType.trimmed();
        if (type.isEmpty()) {
            if (errorMessage) {
                *errorMessage = "灾害类型不能为空";
            }
            return false;
        }
        setParts << "disaster_type = ?";
        binds << type;
    }
    if (patch.hasLocation) {
        setParts << "location = ?";
        binds << nullableText(patch.location);
    }
    if (patch.hasOccurredAt) {
        setParts << "occurred_at = ?";
        binds << nullableText(patch.occurredAt);
    }
    if (patch.hasContent) {
        setParts << "content = ?";
        binds << nullableText(patch.content);
    }
    if (patch.hasSystemAlarmAt) {
        setParts << "system_alarm_at = ?";
        binds << nullableText(patch.systemAlarmAt);
    }
    if (patch.hasSeverity) {
        setParts << "severity = ?";
        binds << patch.severity;
    }

    if (setParts.isEmpty()) {
        if (errorMessage) {
            *errorMessage = "没有需要更新的字段";
        }
        return false;
    }

    QString connError;
    if (!ensureSqliteConnection(&connError)) {
        if (errorMessage) {
            *errorMessage = connError;
        }
        return false;
    }
    QSqlDatabase db = QSqlDatabase::database("app_sqlite");

    QString schemaError;
    if (!ensureDisasterSchema(db, &schemaError)) {
        if (errorMessage) {
            *errorMessage = schemaError;
        }
        return false;
    }

    QSqlQuery query(db);
    const QString sql = "UPDATE disasters SET " + setParts.join(", ") + " WHERE id = ?;";
    query.prepare(sql);
    for (const QVariant& v : binds) {
        query.addBindValue(v);
    }
    query.addBindValue(patch.id);

    if (!query.exec()) {
        if (errorMessage) {
            *errorMessage = query.lastError().text();
        }
        return false;
    }
    if (query.numRowsAffected() == 0) {
        if (errorMessage) {
            *errorMessage = "未更新任何记录（可能ID不存在或数据无变化）";
        }
        return false;
    }
    return true;
}

bool DisasterDao::deleteDisaster(qint64 id, QString* errorMessage)
{
    if (id <= 0) {
        if (errorMessage) {
            *errorMessage = "灾害ID无效";
        }
        return false;
    }

    QString connError;
    if (!ensureSqliteConnection(&connError)) {
        if (errorMessage) {
            *errorMessage = connError;
        }
        return false;
    }
    QSqlDatabase db = QSqlDatabase::database("app_sqlite");

    QString schemaError;
    if (!ensureDisasterSchema(db, &schemaError)) {
        if (errorMessage) {
            *errorMessage = schemaError;
        }
        return false;
    }

    QSqlQuery query(db);
    query.prepare("DELETE FROM disasters WHERE id = ?;");
    query.addBindValue(id);

    if (!query.exec()) {
        if (errorMessage) {
            *errorMessage = query.lastError().text();
        }
        return false;
    }
    if (query.numRowsAffected() == 0) {
        if (errorMessage) {
            *errorMessage = "未删除任何记录（可能ID不存在）";
        }
        return false;
    }
    return true;
}

static QString orderByToSql(DisasterQuery::OrderBy orderBy)
{
    switch (orderBy) {
    case DisasterQuery::OrderBy::CreatedAtAsc:
        return "created_at ASC";
    case DisasterQuery::OrderBy::CreatedAtDesc:
        return "created_at DESC";
    case DisasterQuery::OrderBy::OccurredAtAsc:
        return "occurred_at ASC";
    case DisasterQuery::OrderBy::OccurredAtDesc:
        return "occurred_at DESC";
    case DisasterQuery::OrderBy::SeverityAsc:
        return "severity ASC";
    case DisasterQuery::OrderBy::SeverityDesc:
        return "severity DESC";
    case DisasterQuery::OrderBy::IdAsc:
        return "id ASC";
    case DisasterQuery::OrderBy::IdDesc:
        return "id DESC";
    }
    return "created_at DESC";
}

bool DisasterDao::queryDisasters(const DisasterQuery& queryModel, QList<DisasterRecord>* records, QString* errorMessage)
{
    QString connError;
    if (!ensureSqliteConnection(&connError)) {
        if (errorMessage) {
            *errorMessage = connError;
        }
        return false;
    }
    QSqlDatabase db = QSqlDatabase::database("app_sqlite");

    QString schemaError;
    if (!ensureDisasterSchema(db, &schemaError)) {
        if (errorMessage) {
            *errorMessage = schemaError;
        }
        return false;
    }

    if (records) {
        records->clear();
    }

    QStringList whereParts;
    QList<QVariant> binds;

    const QString keyword = queryModel.keyword.trimmed();
    if (!keyword.isEmpty()) {
        whereParts << "(disaster_type LIKE ? OR location LIKE ? OR content LIKE ?)";
        const QString like = "%" + keyword + "%";
        binds << like << like << like;
    }

    const QString typeLike = queryModel.disasterTypeLike.trimmed();
    if (!typeLike.isEmpty()) {
        whereParts << "disaster_type LIKE ?";
        binds << ("%" + typeLike + "%");
    }

    const QString locationLike = queryModel.locationLike.trimmed();
    if (!locationLike.isEmpty()) {
        whereParts << "location LIKE ?";
        binds << ("%" + locationLike + "%");
    }

    const QString contentLike = queryModel.contentLike.trimmed();
    if (!contentLike.isEmpty()) {
        whereParts << "content LIKE ?";
        binds << ("%" + contentLike + "%");
    }

    if (queryModel.hasSeverityMin) {
        whereParts << "severity >= ?";
        binds << queryModel.severityMin;
    }
    if (queryModel.hasSeverityMax) {
        whereParts << "severity <= ?";
        binds << queryModel.severityMax;
    }

    const QString occurredFrom = queryModel.occurredAtFrom.trimmed();
    if (!occurredFrom.isEmpty()) {
        whereParts << "occurred_at >= ?";
        binds << occurredFrom;
    }
    const QString occurredTo = queryModel.occurredAtTo.trimmed();
    if (!occurredTo.isEmpty()) {
        whereParts << "occurred_at <= ?";
        binds << occurredTo;
    }

    const QString createdFrom = queryModel.createdAtFrom.trimmed();
    if (!createdFrom.isEmpty()) {
        whereParts << "created_at >= ?";
        binds << createdFrom;
    }
    const QString createdTo = queryModel.createdAtTo.trimmed();
    if (!createdTo.isEmpty()) {
        whereParts << "created_at <= ?";
        binds << createdTo;
    }

    QString sql =
        "SELECT id, disaster_type, location, occurred_at, content, system_alarm_at, severity, created_at "
        "FROM disasters";
    if (!whereParts.isEmpty()) {
        sql += " WHERE " + whereParts.join(" AND ");
    }
    sql += " ORDER BY " + orderByToSql(queryModel.orderBy);

    int limit = queryModel.limit;
    if (limit <= 0) {
        limit = 50;
    }
    if (limit > 500) {
        limit = 500;
    }
    int offset = queryModel.offset;
    if (offset < 0) {
        offset = 0;
    }
    sql += " LIMIT ? OFFSET ?;";
    binds << limit << offset;

    QSqlQuery query(db);
    query.prepare(sql);
    for (const QVariant& v : binds) {
        query.addBindValue(v);
    }

    if (!query.exec()) {
        if (errorMessage) {
            *errorMessage = query.lastError().text();
        }
        return false;
    }

    if (records) {
        while (query.next()) {
            DisasterRecord r;
            readDisasterRow(query, &r);
            records->push_back(r);
        }
    }

    return true;
}

bool DisasterDao::queryDisastersByConditions(
    const QString& keyword,
    const QString& disasterTypeLike,
    const QString& locationLike,
    bool hasSeverityMin,
    int severityMin,
    bool hasSeverityMax,
    int severityMax,
    const QString& occurredAtFrom,
    const QString& occurredAtTo,
    int limit,
    int offset,
    QList<DisasterRecord>* records,
    QString* errorMessage)
{
    DisasterQuery q;
    q.keyword = keyword;
    q.disasterTypeLike = disasterTypeLike;
    q.locationLike = locationLike;
    q.hasSeverityMin = hasSeverityMin;
    q.severityMin = severityMin;
    q.hasSeverityMax = hasSeverityMax;
    q.severityMax = severityMax;
    q.occurredAtFrom = occurredAtFrom;
    q.occurredAtTo = occurredAtTo;
    q.limit = limit;
    q.offset = offset;
    return queryDisasters(q, records, errorMessage);
}

