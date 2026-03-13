#include "disasterdao.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QDir>
#include <QtCore/QVariant>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlError>
#include <QtSql/QSqlQuery>

static bool ensureSqliteConnection(QString *errorMessage) {
  const QString connectionName = "app_sqlite";

  QSqlDatabase db;
  if (QSqlDatabase::contains(connectionName)) {
    db = QSqlDatabase::database(connectionName);
  } else {
    db = QSqlDatabase::addDatabase("QSQLITE", connectionName);
    const QString dbPath =
        QDir(QCoreApplication::applicationDirPath()).filePath("monitor.db");
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
  if (!pragma.exec(QStringLiteral("PRAGMA foreign_keys = ON;"))) {
    if (errorMessage) {
      *errorMessage = pragma.lastError().text();
    }
    return false;
  }

  return true;
}

static bool ensureDisasterSchema(QSqlDatabase db, QString *errorMessage) {
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
          "dispatcher_id INTEGER,"
          "created_at TEXT NOT NULL DEFAULT (datetime('now','localtime'))"
          ");")) {
    if (errorMessage) {
      *errorMessage = query.lastError().text();
    }
    return false;
  }

  if (!query.exec("CREATE INDEX IF NOT EXISTS idx_disasters_severity ON "
                  "disasters(severity);")) {
    if (errorMessage) {
      *errorMessage = query.lastError().text();
    }
    return false;
  }

  return true;
}

static QVariant nullableText(const QString &s) {
  const QString t = s.trimmed();
  return t.isEmpty() ? QVariant(QVariant::String) : QVariant(t);
}

static bool readDisasterRow(QSqlQuery &query, DisasterRecord *record) {
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
  record->dispatcherId = query.value(7).toLongLong();
  record->createdAt = query.value(8).toString();
  return true;
}

bool DisasterDao::createDisaster(const DisasterRecord &record, qint64 *id,
                                 QString *errorMessage) {
  const QString type = record.disasterType.trimmed();
  if (type.isEmpty()) {
    if (errorMessage) {
      *errorMessage = QStringLiteral("灾害类型不能为空");
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
  query.prepare("INSERT INTO disasters(disaster_type, location, occurred_at, "
                "content, system_alarm_at, severity, dispatcher_id) "
                "VALUES(?, ?, ?, ?, ?, ?, ?);");
  query.addBindValue(type);
  query.addBindValue(nullableText(record.location));
  query.addBindValue(nullableText(record.occurredAt));
  query.addBindValue(nullableText(record.content));
  query.addBindValue(nullableText(record.systemAlarmAt));
  query.addBindValue(record.severity);
  query.addBindValue(record.dispatcherId > 0 ? QVariant(record.dispatcherId)
                                             : QVariant(QVariant::LongLong));

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

bool DisasterDao::getDisasterById(qint64 id, DisasterRecord *record,
                                  QString *errorMessage) {
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
  query.prepare("SELECT id, disaster_type, location, occurred_at, content, "
                "system_alarm_at, severity, dispatcher_id, created_at "
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

bool DisasterDao::updateDisaster(const DisasterPatch &patch,
                                 QString *errorMessage) {
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
  if (patch.hasDispatcherId) {
    setParts << "dispatcher_id = ?";
    binds << (patch.dispatcherId > 0 ? QVariant(patch.dispatcherId)
                                     : QVariant(QVariant::LongLong));
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
  const QString sql =
      "UPDATE disasters SET " + setParts.join(", ") + " WHERE id = ?;";
  query.prepare(sql);
  for (const QVariant &v : binds) {
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

bool DisasterDao::deleteDisaster(qint64 id, QString *errorMessage) {
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

static QString orderByToSql(DisasterQuery::OrderBy orderBy) {
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

bool DisasterDao::queryDisasters(const DisasterQuery &queryModel,
                                 QList<DisasterRecord> *records,
                                 QString *errorMessage) {
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

  QString sql = "SELECT id, disaster_type, location, occurred_at, content, "
                "system_alarm_at, severity, dispatcher_id, created_at "
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
  for (const QVariant &v : binds) {
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

bool DisasterDao::getUnassignedDisastersByDispatcher(
    qint64 dispatcherId, int limit, int offset,
    QList<DisasterRecord> *records, QString *errorMessage) {
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

  if (limit <= 0)
    limit = 50;
  if (limit > 500)
    limit = 500;
  if (offset < 0)
    offset = 0;

  QSqlQuery query(db);
  QString sql = "SELECT id, disaster_type, location, occurred_at, content, "
                "system_alarm_at, severity, dispatcher_id, created_at "
                "FROM disasters "
                "WHERE id NOT IN (SELECT disaster_id FROM disaster_tasks) ";
  if (dispatcherId > 0) {
    // Find disasters explicitly assigned to this dispatcher
    sql += "AND dispatcher_id = ? ";
  } else {
    // If needed, can add unit tracking for unassigned. Currently simply
    // fetching those with specific dispatcher or no dispatcher For broad query:
    // sql += "AND (dispatcher_id IS NULL OR dispatcher_id = ?) "
  }
  sql += "ORDER BY created_at DESC LIMIT ? OFFSET ?;";

  query.prepare(sql);
  if (dispatcherId > 0) {
    query.addBindValue(dispatcherId);
  }
  query.addBindValue(limit);
  query.addBindValue(offset);

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
    const QString &keyword, const QString &disasterTypeLike,
    const QString &locationLike, bool hasSeverityMin, int severityMin,
    bool hasSeverityMax, int severityMax, const QString &occurredAtFrom,
    const QString &occurredAtTo, int limit, int offset,
    QList<DisasterRecord> *records, QString *errorMessage) {
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

// === 灾害任务指派相关实现 ===

bool DisasterDao::assignDisasterTasks(qint64 disasterId,
                                      const QList<qint64> &handlerIds,
                                      QString *errorMessage) {
  if (disasterId <= 0) {
    if (errorMessage)
      *errorMessage = QStringLiteral("非法操作：无效的灾害ID");
    return false;
  }

  // ---
  // 逻辑优化重构：增加指派人员列表的自动清洗与去重，预防前端误操作导致的重复派单漏洞
  // ---
  QList<qint64> validHandlerIds;
  for (qint64 hid : handlerIds) {
    if (hid > 0 && !validHandlerIds.contains(hid)) {
      validHandlerIds.append(hid);
    }
  }

  if (validHandlerIds.isEmpty()) {
    if (errorMessage)
      *errorMessage =
          QStringLiteral("派发失败：未筛选到任何有效的现场处置员工号");
    return false;
  }

  QString connError;
  if (!ensureSqliteConnection(&connError)) {
    if (errorMessage)
      *errorMessage = connError;
    return false;
  }
  QSqlDatabase db = QSqlDatabase::database(QStringLiteral("app_sqlite"));

  // 开启事务进行批量安全插入
  if (!db.transaction()) {
    if (errorMessage)
      *errorMessage =
          QStringLiteral("无法开启数据库事务: ") + db.lastError().text();
    return false;
  }

  QSqlQuery query(db);
  query.prepare(QStringLiteral("INSERT INTO disaster_tasks(disaster_id, "
                               "handler_user_id, progress) VALUES(?, ?, 0);"));

  for (qint64 handlerId : validHandlerIds) { // 遍历经过过滤的人员
    query.bindValue(0, disasterId);
    query.bindValue(1, handlerId);
    if (!query.exec()) {
      db.rollback();
      if (errorMessage)
        *errorMessage = QStringLiteral("并行指派处理人异常回滚: ") +
                        query.lastError().text();
      return false;
    }
  }

  if (!db.commit()) {
    db.rollback();
    if (errorMessage)
      *errorMessage =
          QStringLiteral("提交指派事务失败: ") + db.lastError().text();
    return false;
  }

  return true;
}

bool DisasterDao::getTasksForDisaster(qint64 disasterId,
                                      QList<DisasterTaskRecord> *tasks,
                                      QString *errorMessage) {
  if (disasterId <= 0) {
    if (errorMessage)
      *errorMessage = "无效的灾害ID";
    return false;
  }

  QString connError;
  if (!ensureSqliteConnection(&connError)) {
    if (errorMessage)
      *errorMessage = connError;
    return false;
  }
  QSqlDatabase db = QSqlDatabase::database("app_sqlite");

  if (tasks) {
    tasks->clear();
  }

  QSqlQuery query(db);
  query.prepare("SELECT t.id, t.disaster_id, t.handler_user_id, t.progress, "
                "t.assigned_at, u.username, u.phone, "
                "d.disaster_type, d.location, d.severity "
                "FROM disaster_tasks t "
                "LEFT JOIN users u ON t.handler_user_id = u.id "
                "LEFT JOIN disasters d ON t.disaster_id = d.id "
                "WHERE t.disaster_id = ? "
                "ORDER BY t.assigned_at DESC;");
  query.addBindValue(disasterId);

  if (!query.exec()) {
    if (errorMessage)
      *errorMessage = query.lastError().text();
    return false;
  }

  if (tasks) {
    while (query.next()) {
      DisasterTaskRecord r;
      r.id = query.value(0).toLongLong();
      r.disasterId = query.value(1).toLongLong();
      r.handlerUserId = query.value(2).toLongLong();
      r.progress = query.value(3).toInt();
      r.assignedAt = query.value(4).toString();
      r.handlerName = query.value(5).toString();
      r.handlerPhone = query.value(6).toString();
      r.disasterType = query.value(7).toString();
      r.location = query.value(8).toString();
      r.severity = query.value(9).toInt();
      tasks->push_back(r);
    }
  }

  return true;
}

bool DisasterDao::updateDisasterTaskProgress(qint64 taskId, int progress,
                                             QString *errorMessage) {
  if (taskId <= 0) {
    if (errorMessage)
      *errorMessage = QStringLiteral("拒接操作：无效的任务流水号");
    return false;
  }

  // ---
  // 边界逻辑优化：增加严格的数据安全范围修剪，避免底层数据透视或超出进度溢出
  // ---
  if (progress < 0) {
    progress = 0;
  } else if (progress >= 100) {
    progress = 100; // 进度达满即为强制结转归档状态
  }

  QString connError;
  if (!ensureSqliteConnection(&connError)) {
    if (errorMessage)
      *errorMessage = connError;
    return false;
  }
  QSqlDatabase db = QSqlDatabase::database(QStringLiteral("app_sqlite"));

  QSqlQuery query(db);
  query.prepare(
      QStringLiteral("UPDATE disaster_tasks SET progress = ? WHERE id = ?;"));
  query.addBindValue(progress);
  query.addBindValue(taskId);

  if (!query.exec()) {
    if (errorMessage)
      *errorMessage = query.lastError().text();
    return false;
  }

  if (query.numRowsAffected() == 0) {
    if (errorMessage)
      *errorMessage = QStringLiteral(
          "核对未命中：找不到该活跃任务记录，可能已被撤销或删除");
    return false;
  }

  return true;
}

bool DisasterDao::deleteDisasterTask(qint64 taskId, QString *errorMessage) {
  if (taskId <= 0) {
    if (errorMessage)
      *errorMessage = QStringLiteral("无效的任务ID");
    return false;
  }

  QString connError;
  if (!ensureSqliteConnection(&connError)) {
    if (errorMessage)
      *errorMessage = connError;
    return false;
  }
  QSqlDatabase db = QSqlDatabase::database(QStringLiteral("app_sqlite"));

  // --- 健壮性增强：删除前先核实该任务在系统中是否确实存在 ---
  QSqlQuery checkQuery(db);
  checkQuery.prepare(
      QStringLiteral("SELECT count(*) FROM disaster_tasks WHERE id = ?;"));
  checkQuery.addBindValue(taskId);
  if (checkQuery.exec() && checkQuery.next()) {
    if (checkQuery.value(0).toInt() == 0) {
      if (errorMessage)
        *errorMessage =
            QStringLiteral("操作失败：任务ID %1 在系统中不存在，无法执行撤销")
                .arg(taskId);
      return false;
    }
  }

  QSqlQuery query(db);
  query.prepare(QStringLiteral("DELETE FROM disaster_tasks WHERE id = ?;"));
  query.addBindValue(taskId);

  if (!query.exec()) {
    if (errorMessage)
      *errorMessage = query.lastError().text();
    return false;
  }

  if (query.numRowsAffected() == 0) {
    if (errorMessage)
      *errorMessage = QStringLiteral("任务记录状态已变更，删除失败");
    return false;
  }

  return true;
}

bool DisasterDao::getTasksForUser(qint64 userId, const QString& role, int limit, int offset, QList<DisasterTaskRecord> *tasks, QString *errorMessage) {
  if (userId <= 0) {
    if (errorMessage) *errorMessage = QStringLiteral("无效的用户ID");
    return false;
  }

  QString connError;
  if (!ensureSqliteConnection(&connError)) {
    if (errorMessage) *errorMessage = connError;
    return false;
  }
  QSqlDatabase db = QSqlDatabase::database(QStringLiteral("app_sqlite"));

  if (tasks) {
    tasks->clear();
  }

  if (limit <= 0) limit = 100;
  if (limit > 500) limit = 500;
  if (offset < 0) offset = 0;

  QSqlQuery query(db);
  QString sql = "SELECT t.id, t.disaster_id, t.handler_user_id, t.progress, "
                "t.assigned_at, u.username, u.phone, "
                "d.disaster_type, d.location, d.severity "
                "FROM disaster_tasks t "
                "LEFT JOIN users u ON t.handler_user_id = u.id "
                "LEFT JOIN disasters d ON t.disaster_id = d.id ";

  // 指挥调度员查询所有任务，普通处理人员只查询被指派给自己的任务
  if (role != QStringLiteral("指挥调度员")) {
      sql += "WHERE t.handler_user_id = ? ";
  }

  sql += "ORDER BY t.assigned_at DESC LIMIT ? OFFSET ?;";

  query.prepare(sql);
  
  if (role != QStringLiteral("指挥调度员")) {
      query.addBindValue(userId);
  }
  
  query.addBindValue(limit);
  query.addBindValue(offset);

  if (!query.exec()) {
    if (errorMessage) *errorMessage = query.lastError().text();
    return false;
  }

  if (tasks) {
    while (query.next()) {
      DisasterTaskRecord r;
      r.id = query.value(0).toLongLong();
      r.disasterId = query.value(1).toLongLong();
      r.handlerUserId = query.value(2).toLongLong();
      r.progress = query.value(3).toInt();
      r.assignedAt = query.value(4).toString();
      r.handlerName = query.value(5).toString();
      r.handlerPhone = query.value(6).toString();
      r.disasterType = query.value(7).toString();
      r.location = query.value(8).toString();
      r.severity = query.value(9).toInt();
      tasks->push_back(r);
    }
  }

  return true;
}

// === 数据统计接口实现 ===

bool DisasterDao::getDisastersByDate(const QString &date,
                                     QList<DisasterRecord> *records,
                                     QString *errorMessage) {
  const QString dateStr = date.trimmed();
  if (dateStr.isEmpty()) {
    if (errorMessage)
      *errorMessage = QStringLiteral("日期不能为空，请传入格式为 YYYY-MM-DD 的日期字符串");
    return false;
  }

  QString connError;
  if (!ensureSqliteConnection(&connError)) {
    if (errorMessage)
      *errorMessage = connError;
    return false;
  }
  QSqlDatabase db = QSqlDatabase::database(QStringLiteral("app_sqlite"));

  QString schemaError;
  if (!ensureDisasterSchema(db, &schemaError)) {
    if (errorMessage)
      *errorMessage = schemaError;
    return false;
  }

  if (records)
    records->clear();

  // 固定使用 occurred_at（发生时间）进行日期匹配
  const QString sql =
      QString("SELECT id, disaster_type, location, occurred_at, content, "
              "system_alarm_at, severity, dispatcher_id, created_at "
              "FROM disasters "
              "WHERE substr(occurred_at, 1, 10) = ? "
              "ORDER BY occurred_at ASC;");

  QSqlQuery query(db);
  query.prepare(sql);
  query.addBindValue(dateStr.left(10)); // 只取 YYYY-MM-DD 部分

  if (!query.exec()) {
    if (errorMessage)
      *errorMessage = query.lastError().text();
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

bool DisasterDao::countDisasterTypesByDateRange(const QString &dateFrom,
                                                const QString &dateTo,
                                                QMap<QString, int> *result,
                                                QString *errorMessage) {
  const QString fromStr = dateFrom.trimmed();
  const QString toStr   = dateTo.trimmed();

  if (fromStr.isEmpty() || toStr.isEmpty()) {
    if (errorMessage)
      *errorMessage = QStringLiteral("起始日期和截止日期均不能为空");
    return false;
  }

  QString connError;
  if (!ensureSqliteConnection(&connError)) {
    if (errorMessage)
      *errorMessage = connError;
    return false;
  }
  QSqlDatabase db = QSqlDatabase::database(QStringLiteral("app_sqlite"));

  QString schemaError;
  if (!ensureDisasterSchema(db, &schemaError)) {
    if (errorMessage)
      *errorMessage = schemaError;
    return false;
  }

  if (result)
    result->clear();

  // 固定使用 occurred_at（发生时间），截止日期补全到当天末尾 23:59:59
  const QString fromFull = fromStr.left(10) + QStringLiteral(" 00:00:00");
  const QString toFull   = toStr.left(10)   + QStringLiteral(" 23:59:59");

  const QString sql =
      QStringLiteral("SELECT disaster_type, COUNT(*) AS cnt "
                     "FROM disasters "
                     "WHERE occurred_at >= ? AND occurred_at <= ? "
                     "GROUP BY disaster_type "
                     "ORDER BY cnt DESC;");

  QSqlQuery query(db);
  query.prepare(sql);
  query.addBindValue(fromFull);
  query.addBindValue(toFull);

  if (!query.exec()) {
    if (errorMessage)
      *errorMessage = query.lastError().text();
    return false;
  }

  if (result) {
    while (query.next()) {
      const QString type = query.value(0).toString();
      const int     cnt  = query.value(1).toInt();
      (*result)[type]    = cnt;
    }
  }

  return true;
}

