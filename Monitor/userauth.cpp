#include "userauth.h"

AuthUser UserAuth::currentUser;

#include <QtCore/QByteArray>
#include <QtCore/QCoreApplication>
#include <QtCore/QCryptographicHash>
#include <QtCore/QDir>
#include <QtCore/QRandomGenerator>
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
  if (!pragma.exec("PRAGMA foreign_keys = ON;")) {
    if (errorMessage) {
      *errorMessage = pragma.lastError().text();
    }
    return false;
  }

  return true;
}

static bool usersHasColumn(QSqlDatabase db, const QString &columnName,
                           bool *outHasColumn, QString *errorMessage) {
  if (outHasColumn) {
    *outHasColumn = false;
  }
  QSqlQuery query(db);
  if (!query.exec("PRAGMA table_info(users);")) {
    if (errorMessage) {
      *errorMessage = query.lastError().text();
    }
    return false;
  }
  while (query.next()) {
    if (query.value(1).toString().compare(columnName, Qt::CaseInsensitive) ==
        0) {
      if (outHasColumn) {
        *outHasColumn = true;
      }
      break;
    }
  }
  return true;
}

static bool ensureUnitSchema(QSqlDatabase db, QString *errorMessage) {
  QSqlQuery query(db);
  if (!query.exec(
          "CREATE TABLE IF NOT EXISTS units ("
          "id INTEGER PRIMARY KEY AUTOINCREMENT,"
          "name TEXT NOT NULL UNIQUE,"
          "created_at TEXT NOT NULL DEFAULT (datetime('now','localtime'))"
          ");")) {
    if (errorMessage) {
      *errorMessage = query.lastError().text();
    }
    return false;
  }

  if (!query.exec(
          "CREATE TABLE IF NOT EXISTS users ("
          "id INTEGER PRIMARY KEY AUTOINCREMENT,"
          "username TEXT NOT NULL UNIQUE,"
          "password TEXT NOT NULL,"
          "phone TEXT,"
          "role TEXT NOT NULL,"
          "unit_id INTEGER,"
          "dispatcher_user_id INTEGER,"
          "unit TEXT,"
          "created_at TEXT NOT NULL DEFAULT (datetime('now','localtime')),"
          "FOREIGN KEY(unit_id) REFERENCES units(id) ON DELETE SET NULL,"
          "FOREIGN KEY(dispatcher_user_id) REFERENCES users(id) ON DELETE SET "
          "NULL"
          ");")) {
    if (errorMessage) {
      *errorMessage = query.lastError().text();
    }
    return false;
  }

  bool hasUnitId = false;
  if (!usersHasColumn(db, "unit_id", &hasUnitId, errorMessage)) {
    return false;
  }
  if (!hasUnitId) {
    if (!query.exec("ALTER TABLE users ADD COLUMN unit_id INTEGER;")) {
      if (errorMessage) {
        *errorMessage = query.lastError().text();
      }
      return false;
    }
  }

  bool hasDispatcherUserId = false;
  if (!usersHasColumn(db, "dispatcher_user_id", &hasDispatcherUserId,
                      errorMessage)) {
    return false;
  }
  if (!hasDispatcherUserId) {
    if (!query.exec(
            "ALTER TABLE users ADD COLUMN dispatcher_user_id INTEGER;")) {
      if (errorMessage) {
        *errorMessage = query.lastError().text();
      }
      return false;
    }
  }

  if (!query.exec(
          "CREATE INDEX IF NOT EXISTS idx_units_name ON units(name);")) {
    if (errorMessage) {
      *errorMessage = query.lastError().text();
    }
    return false;
  }
  if (!query.exec(
          "CREATE INDEX IF NOT EXISTS idx_users_unit_id ON users(unit_id);")) {
    if (errorMessage) {
      *errorMessage = query.lastError().text();
    }
    return false;
  }
  if (!query.exec("CREATE INDEX IF NOT EXISTS idx_users_dispatcher_user_id ON "
                  "users(dispatcher_user_id);")) {
    if (errorMessage) {
      *errorMessage = query.lastError().text();
    }
    return false;
  }
  return true;
}

static QString hashPasswordForStorage(const QString &password) {
  QByteArray salt;
  salt.resize(16);
  for (int i = 0; i < salt.size(); ++i) {
    salt[i] = static_cast<char>(QRandomGenerator::global()->bounded(0, 256));
  }
  const QByteArray pwd = password.toUtf8();
  const QByteArray digest =
      QCryptographicHash::hash(salt + pwd, QCryptographicHash::Sha256);
  return QString::fromLatin1(salt.toHex()) + "$" +
         QString::fromLatin1(digest.toHex());
}

static bool verifyPassword(const QString &stored, const QString &password) {
  const int sep = stored.indexOf('$');
  if (sep < 0) {
    return stored == password;
  }
  const QByteArray saltHex = stored.left(sep).toLatin1();
  const QByteArray digestHex = stored.mid(sep + 1).toLatin1();
  const QByteArray salt = QByteArray::fromHex(saltHex);
  const QByteArray expected = QByteArray::fromHex(digestHex);
  const QByteArray actual = QCryptographicHash::hash(
      salt + password.toUtf8(), QCryptographicHash::Sha256);
  return expected == actual;
}

static bool isDispatcherRole(const QString &role) {
  const QString r = role.trimmed();
  return r.compare("现场调度员", Qt::CaseInsensitive) == 0 ||
         r.compare("指挥调度员", Qt::CaseInsensitive) == 0;
}

static bool isHandlerRole(const QString &role) {
  const QString r = role.trimmed();
  return r.compare("现场处置员", Qt::CaseInsensitive) == 0;
}

static bool insertUser(QSqlDatabase db, const QString &username,
                       const QString &password, const QString &phone,
                       const QString &role, qint64 unitId,
                       const QString &unitName, qint64 dispatcherUserId,
                       qint64 *userId, QString *errorMessage) {
  bool hasUnitIdColumn = false;
  if (!usersHasColumn(db, "unit_id", &hasUnitIdColumn, errorMessage)) {
    return false;
  }
  bool hasDispatcherColumn = false;
  if (!usersHasColumn(db, "dispatcher_user_id", &hasDispatcherColumn,
                      errorMessage)) {
    return false;
  }

  QSqlQuery query(db);
  if (hasUnitIdColumn && hasDispatcherColumn) {
    query.prepare("INSERT INTO users(username, password, phone, role, unit_id, "
                  "dispatcher_user_id, unit) "
                  "VALUES(?, ?, ?, ?, ?, ?, ?);");
  } else if (hasUnitIdColumn) {
    query.prepare(
        "INSERT INTO users(username, password, phone, role, unit_id, unit) "
        "VALUES(?, ?, ?, ?, ?, ?);");
  } else {
    query.prepare("INSERT INTO users(username, password, phone, role, unit) "
                  "VALUES(?, ?, ?, ?, ?);");
  }

  const QString storedPassword = hashPasswordForStorage(password);
  query.addBindValue(username.trimmed());
  query.addBindValue(storedPassword);
  query.addBindValue(phone.trimmed().isEmpty() ? QVariant(QVariant::String)
                                               : phone.trimmed());
  query.addBindValue(role.trimmed());

  if (hasUnitIdColumn && hasDispatcherColumn) {
    query.addBindValue(unitId > 0 ? QVariant(unitId)
                                  : QVariant(QVariant::LongLong));
    query.addBindValue(dispatcherUserId > 0 ? QVariant(dispatcherUserId)
                                            : QVariant(QVariant::LongLong));
    query.addBindValue(unitName.trimmed().isEmpty() ? QVariant(QVariant::String)
                                                    : unitName.trimmed());
  } else if (hasUnitIdColumn) {
    query.addBindValue(unitId > 0 ? QVariant(unitId)
                                  : QVariant(QVariant::LongLong));
    query.addBindValue(unitName.trimmed().isEmpty() ? QVariant(QVariant::String)
                                                    : unitName.trimmed());
  } else {
    query.addBindValue(unitName.trimmed().isEmpty() ? QVariant(QVariant::String)
                                                    : unitName.trimmed());
  }

  if (!query.exec()) {
    const QString err = query.lastError().text();
    if (errorMessage) {
      if (err.contains("UNIQUE constraint failed", Qt::CaseInsensitive)) {
        *errorMessage = "用户名已存在";
      } else {
        *errorMessage = err;
      }
    }
    return false;
  }

  if (userId) {
    *userId = query.lastInsertId().toLongLong();
  }
  return true;
}

bool UserAuth::createUnit(const QString &unitName, qint64 *unitId,
                          QString *errorMessage) {
  if (unitName.trimmed().isEmpty()) {
    if (errorMessage) {
      *errorMessage = "单位名称不能为空";
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
  if (!ensureUnitSchema(db, &schemaError)) {
    if (errorMessage) {
      *errorMessage = schemaError;
    }
    return false;
  }

  QSqlQuery query(db);
  query.prepare("INSERT INTO units(name) VALUES(?);");
  query.addBindValue(unitName.trimmed());
  if (!query.exec()) {
    const QString err = query.lastError().text();
    if (errorMessage) {
      if (err.contains("UNIQUE constraint failed", Qt::CaseInsensitive)) {
        *errorMessage = "单位已存在";
      } else {
        *errorMessage = err;
      }
    }
    return false;
  }

  if (unitId) {
    *unitId = query.lastInsertId().toLongLong();
  }
  return true;
}

bool UserAuth::searchUnits(const QString &keyword, int limit,
                           QList<UnitInfo> *units, QString *errorMessage) {
  if (limit <= 0) {
    limit = 20;
  }
  if (limit > 100) {
    limit = 100;
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
  if (!ensureUnitSchema(db, &schemaError)) {
    if (errorMessage) {
      *errorMessage = schemaError;
    }
    return false;
  }

  if (units) {
    units->clear();
  }

  QSqlQuery query(db);
  query.prepare(
      "SELECT id, name FROM units WHERE name LIKE ? ORDER BY name LIMIT ?;");
  query.addBindValue("%" + keyword.trimmed() + "%");
  query.addBindValue(limit);

  if (!query.exec()) {
    if (errorMessage) {
      *errorMessage = query.lastError().text();
    }
    return false;
  }

  if (units) {
    while (query.next()) {
      UnitInfo info;
      info.id = query.value(0).toLongLong();
      info.name = query.value(1).toString();
      units->push_back(info);
    }
  }

  return true;
}

bool UserAuth::searchDispatchersByUnit(qint64 unitId, const QString &keyword,
                                       int limit,
                                       QList<DispatcherInfo> *dispatchers,
                                       QString *errorMessage) {
  if (unitId <= 0) {
    if (errorMessage) {
      *errorMessage = "单位ID无效";
    }
    return false;
  }
  if (limit <= 0) {
    limit = 20;
  }
  if (limit > 100) {
    limit = 100;
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
  if (!ensureUnitSchema(db, &schemaError)) {
    if (errorMessage) {
      *errorMessage = schemaError;
    }
    return false;
  }

  if (dispatchers) {
    dispatchers->clear();
  }

  QSqlQuery query(db);
  query.prepare(
      "SELECT u.id, u.username, u.phone, COALESCE(units.name, u.unit) "
      "FROM users u "
      "LEFT JOIN units ON units.id = u.unit_id "
      "WHERE u.unit_id = ? "
      "AND (u.role = ? OR u.role = ?) "
      "AND u.username LIKE ? "
      "ORDER BY u.username "
      "LIMIT ?;");
  query.addBindValue(unitId);
  query.addBindValue(QStringLiteral("现场调度员"));
  query.addBindValue(QStringLiteral("指挥调度员"));
  query.addBindValue("%" + keyword.trimmed() + "%");
  query.addBindValue(limit);

  if (!query.exec()) {
    if (errorMessage) {
      *errorMessage = query.lastError().text();
    }
    return false;
  }

  if (dispatchers) {
    while (query.next()) {
      DispatcherInfo info;
      info.id = query.value(0).toLongLong();
      info.username = query.value(1).toString();
      info.phone = query.value(2).toString();
      info.unit = query.value(3).toString();
      dispatchers->push_back(info);
    }
  }
  return true;
}

bool UserAuth::getHandlersByDispatcherId(qint64 dispatcherId,
                                         QList<AuthUser> *handlers,
                                         QString *errorMessage) {
  if (dispatcherId <= 0) {
    if (errorMessage) {
      *errorMessage = "调度员ID无效";
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
  if (!ensureUnitSchema(db, &schemaError)) {
    if (errorMessage) {
      *errorMessage = schemaError;
    }
    return false;
  }

  if (handlers) {
    handlers->clear();
  }

  QSqlQuery query(db);
  query.prepare("SELECT u.id, u.username, u.phone, u.role, "
                "COALESCE(units.name, u.unit), u.dispatcher_user_id "
                "FROM users u "
                "LEFT JOIN units ON units.id = u.unit_id "
                "WHERE u.dispatcher_user_id = ? AND (u.role = '现场处置员' OR "
                "u.role = '现场处置员') "
                "ORDER BY u.id ASC;");
  query.addBindValue(dispatcherId);

  if (!query.exec()) {
    if (errorMessage) {
      *errorMessage = query.lastError().text();
    }
    return false;
  }

  if (handlers) {
    while (query.next()) {
      AuthUser user;
      user.id = query.value(0).toLongLong();
      user.username = query.value(1).toString();
      user.phone = query.value(2).toString();
      user.role = query.value(3).toString();
      user.unit = query.value(4).toString();
      user.dispatcherUserId = query.value(5).toLongLong();
      handlers->push_back(user);
    }
  }
  return true;
}

bool UserAuth::registerUser(const QString &username, const QString &password,
                            const QString &phone, const QString &role,
                            const QString &unit, qint64 *userId,
                            QString *errorMessage) {
  if (isDispatcherRole(role)) {
    const QString unitName = unit.trimmed();
    if (unitName.isEmpty()) {
      if (errorMessage) {
        *errorMessage = "调度员注册需要选择单位";
      }
      return false;
    }

    QList<UnitInfo> matches;
    QString searchError;
    if (!searchUnits(unitName, 5, &matches, &searchError)) {
      if (errorMessage) {
        *errorMessage = searchError;
      }
      return false;
    }
    qint64 unitId = 0;
    for (const UnitInfo &u : matches) {
      if (u.name == unitName) {
        unitId = u.id;
        break;
      }
    }
    if (unitId <= 0) {
      if (errorMessage) {
        *errorMessage = "单位不存在，请先创建单位";
      }
      return false;
    }
    return registerDispatcherWithUnitId(username, password, phone, role, unitId,
                                        userId, errorMessage);
  }

  if (isHandlerRole(role)) {
    if (errorMessage) {
      *errorMessage = "现场处置员注册需要绑定指挥调度员，请使用 "
                      "registerHandlerWithDispatcherId";
    }
    return false;
  }

  return registerUserWithUnitId(username, password, phone, role, 0, userId,
                                errorMessage);
}

bool UserAuth::registerDispatcherWithUnitId(
    const QString &username, const QString &password, const QString &phone,
    const QString &role, qint64 unitId, qint64 *userId, QString *errorMessage) {
  if (!isDispatcherRole(role)) {
    if (errorMessage) {
      *errorMessage = "角色不是调度员";
    }
    return false;
  }
  return registerUserWithUnitId(username, password, phone, role, unitId, userId,
                                errorMessage);
}

bool UserAuth::registerHandlerWithDispatcherId(
    const QString &username, const QString &password, const QString &phone,
    qint64 dispatcherUserId, qint64 *userId, QString *errorMessage) {
  if (dispatcherUserId <= 0) {
    if (errorMessage) {
      *errorMessage = "指挥调度员ID无效";
    }
    return false;
  }
  if (username.trimmed().isEmpty()) {
    if (errorMessage) {
      *errorMessage = "用户名不能为空";
    }
    return false;
  }
  if (password.isEmpty()) {
    if (errorMessage) {
      *errorMessage = "密码不能为空";
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
  if (!ensureUnitSchema(db, &schemaError)) {
    if (errorMessage) {
      *errorMessage = schemaError;
    }
    return false;
  }

  QSqlQuery lookup(db);
  lookup.prepare("SELECT u.unit_id, COALESCE(units.name, u.unit), u.role "
                 "FROM users u "
                 "LEFT JOIN units ON units.id = u.unit_id "
                 "WHERE u.id = ? "
                 "LIMIT 1;");
  lookup.addBindValue(dispatcherUserId);
  if (!lookup.exec()) {
    if (errorMessage) {
      *errorMessage = lookup.lastError().text();
    }
    return false;
  }
  if (!lookup.next()) {
    if (errorMessage) {
      *errorMessage = "指挥调度员不存在";
    }
    return false;
  }

  const qint64 unitId = lookup.value(0).toLongLong();
  const QString unitName = lookup.value(1).toString();
  const QString dispatcherRole = lookup.value(2).toString();
  if (!isDispatcherRole(dispatcherRole)) {
    if (errorMessage) {
      *errorMessage = "绑定用户不是指挥/现场调度员";
    }
    return false;
  }

  return insertUser(db, username, password, phone, QStringLiteral("现场处置员"),
                    unitId, unitName, dispatcherUserId, userId, errorMessage);
}

bool UserAuth::registerUserWithUnitId(const QString &username,
                                      const QString &password,
                                      const QString &phone, const QString &role,
                                      qint64 unitId, qint64 *userId,
                                      QString *errorMessage) {
  if (username.trimmed().isEmpty()) {
    if (errorMessage) {
      *errorMessage = "用户名不能为空";
    }
    return false;
  }
  if (password.isEmpty()) {
    if (errorMessage) {
      *errorMessage = "密码不能为空";
    }
    return false;
  }
  if (role.trimmed().isEmpty()) {
    if (errorMessage) {
      *errorMessage = "用户角色不能为空";
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
  if (!ensureUnitSchema(db, &schemaError)) {
    if (errorMessage) {
      *errorMessage = schemaError;
    }
    return false;
  }

  QString unitName;
  if (unitId > 0) {
    QSqlQuery lookup(db);
    lookup.prepare("SELECT name FROM units WHERE id = ? LIMIT 1;");
    lookup.addBindValue(unitId);
    if (!lookup.exec()) {
      if (errorMessage) {
        *errorMessage = lookup.lastError().text();
      }
      return false;
    }
    if (!lookup.next()) {
      if (errorMessage) {
        *errorMessage = "单位不存在";
      }
      return false;
    }
    unitName = lookup.value(0).toString();
  }

  return insertUser(db, username, password, phone, role, unitId, unitName, 0,
                    userId, errorMessage);
}

bool UserAuth::login(const QString &username, const QString &password,
                     AuthUser *user, QString *errorMessage) {
  if (username.trimmed().isEmpty()) {
    if (errorMessage) {
      *errorMessage = "用户名不能为空";
    }
    return false;
  }
  if (password.isEmpty()) {
    if (errorMessage) {
      *errorMessage = "密码不能为空";
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
  if (!ensureUnitSchema(db, &schemaError)) {
    if (errorMessage) {
      *errorMessage = schemaError;
    }
    return false;
  }

  QSqlQuery query(db);
  query.prepare("SELECT u.id, u.username, u.password, u.phone, u.role, "
                "COALESCE(units.name, u.unit), u.dispatcher_user_id "
                "FROM users u "
                "LEFT JOIN units ON units.id = u.unit_id "
                "WHERE u.username = ? "
                "LIMIT 1;");
  query.addBindValue(username.trimmed());

  if (!query.exec()) {
    if (errorMessage) {
      *errorMessage = query.lastError().text();
    }
    return false;
  }

  if (!query.next()) {
    if (errorMessage) {
      *errorMessage = "用户不存在";
    }
    return false;
  }

  const qint64 id = query.value(0).toLongLong();
  const QString dbUsername = query.value(1).toString();
  const QString storedPassword = query.value(2).toString();
  const QString dbPhone = query.value(3).toString();
  const QString dbRole = query.value(4).toString();
  const QString dbUnit = query.value(5).toString();
  const qint64 dispatcherUserId = query.value(6).toLongLong();

  if (!verifyPassword(storedPassword, password)) {
    if (errorMessage) {
      *errorMessage = "密码错误";
    }
    return false;
  }

  if (user) {
    user->id = id;
    user->username = dbUsername;
    user->phone = dbPhone;
    user->role = dbRole;
    user->unit = dbUnit;
    user->dispatcherUserId = dispatcherUserId;
  }

  UserAuth::currentUser.id = id;
  UserAuth::currentUser.username = dbUsername;
  UserAuth::currentUser.phone = dbPhone;
  UserAuth::currentUser.role = dbRole;
  UserAuth::currentUser.unit = dbUnit;
  UserAuth::currentUser.dispatcherUserId = dispatcherUserId;

  return true;
}
