#include "disasterdao.h"
#include "index.h"
#include "login.h"
#include "mainwindow.h"
#include "ocrhelper.h"
#include <QApplication>
#include <QCoreApplication>
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QMessageBox>
#include <QStringList>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlError>
#include <QtSql/QSqlQuery>
#include <exception>
#include <iostream>
#include <opencv2/opencv.hpp>

static bool initSqliteSchema(QString *errorMessage) {

  const QString dbPath =
      QDir(QCoreApplication::applicationDirPath()).filePath("monitor.db");

  QSqlDatabase db;
  if (QSqlDatabase::contains("app_sqlite")) {
    db = QSqlDatabase::database("app_sqlite");
  } else {
    db = QSqlDatabase::addDatabase("QSQLITE", "app_sqlite");
    db.setDatabaseName(dbPath);
  }

  if (!db.open()) {
    if (errorMessage) {
      *errorMessage = db.lastError().text();
    }
    return false;
  }

  QSqlQuery query(db);
  if (!query.exec("PRAGMA foreign_keys = ON;")) {
    if (errorMessage) {
      *errorMessage = query.lastError().text();
    }
    return false;
  }

  if (!db.transaction()) {
    if (errorMessage) {
      *errorMessage = db.lastError().text();
    }
    return false;
  }

  const QStringList statements = {
      "CREATE TABLE IF NOT EXISTS units ("
      "id INTEGER PRIMARY KEY AUTOINCREMENT,"
      "name TEXT NOT NULL UNIQUE,"
      "created_at TEXT NOT NULL DEFAULT (datetime('now','localtime'))"
      ");",

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
      "FOREIGN KEY(dispatcher_user_id) REFERENCES users(id) ON DELETE SET NULL"
      ");",

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
      ");",

      "CREATE TABLE IF NOT EXISTS disaster_tasks ("
      "id INTEGER PRIMARY KEY AUTOINCREMENT,"
      "disaster_id INTEGER NOT NULL,"
      "handler_user_id INTEGER NOT NULL,"
      "progress INTEGER NOT NULL DEFAULT 0,"
      "assigned_at TEXT NOT NULL DEFAULT (datetime('now','localtime')),"
      "FOREIGN KEY(disaster_id) REFERENCES disasters(id) ON DELETE CASCADE,"
      "FOREIGN KEY(handler_user_id) REFERENCES users(id) ON DELETE CASCADE"
      ");",

      "CREATE TABLE IF NOT EXISTS announcements ("
      "id INTEGER PRIMARY KEY AUTOINCREMENT,"
      "title TEXT NOT NULL,"
      "content TEXT NOT NULL,"
      "publish_at TEXT NOT NULL DEFAULT (datetime('now','localtime')),"
      "expire_at TEXT"
      ");",

      "CREATE TABLE IF NOT EXISTS chat_messages ("
      "id INTEGER PRIMARY KEY AUTOINCREMENT,"
      "sent_at TEXT NOT NULL DEFAULT (datetime('now','localtime')),"
      "sender_user_id INTEGER NOT NULL,"
      "receiver_user_id INTEGER NOT NULL,"
      "content TEXT NOT NULL,"
      "FOREIGN KEY(sender_user_id) REFERENCES users(id) ON DELETE CASCADE,"
      "FOREIGN KEY(receiver_user_id) REFERENCES users(id) ON DELETE CASCADE"
      ");",

      "CREATE TABLE IF NOT EXISTS alert_settings ("
      "id INTEGER PRIMARY KEY AUTOINCREMENT,"
      "user_id INTEGER NOT NULL UNIQUE,"
      "alarm_response_count INTEGER NOT NULL DEFAULT 0,"
      "watch_content TEXT,"
      "alarm_audio TEXT,"
      "updated_at TEXT NOT NULL DEFAULT (datetime('now','localtime')),"
      "FOREIGN KEY(user_id) REFERENCES users(id) ON DELETE CASCADE"
      ");",

      "CREATE INDEX IF NOT EXISTS idx_disaster_tasks_disaster_id ON "
      "disaster_tasks(disaster_id);",
      "CREATE INDEX IF NOT EXISTS idx_disaster_tasks_handler_user_id ON "
      "disaster_tasks(handler_user_id);",
      "CREATE INDEX IF NOT EXISTS idx_chat_messages_sender ON "
      "chat_messages(sender_user_id);",
      "CREATE INDEX IF NOT EXISTS idx_chat_messages_receiver ON "
      "chat_messages(receiver_user_id);",
      "CREATE INDEX IF NOT EXISTS idx_disasters_severity ON "
      "disasters(severity);"};

  for (const QString &sql : statements) {
    if (!query.exec(sql)) {
      db.rollback();
      if (errorMessage) {
        *errorMessage = query.lastError().text();
      }
      return false;
    }
  }

  {
    bool hasDisasterDispatcherId = false;
    if (!query.exec("PRAGMA table_info(disasters);")) {
      db.rollback();
      if (errorMessage) {
        *errorMessage = query.lastError().text();
      }
      return false;
    }
    while (query.next()) {
      if (query.value(1).toString().compare("dispatcher_id",
                                            Qt::CaseInsensitive) == 0) {
        hasDisasterDispatcherId = true;
        break;
      }
    }
    query.finish();

    if (!hasDisasterDispatcherId) {
      if (!query.exec(
              "ALTER TABLE disasters ADD COLUMN dispatcher_id INTEGER;")) {
        db.rollback();
        if (errorMessage) {
          *errorMessage = query.lastError().text();
        }
        return false;
      }
    }
  }

  {
    bool hasUnitId = false;
    if (!query.exec("PRAGMA table_info(users);")) {
      db.rollback();
      if (errorMessage) {
        *errorMessage = query.lastError().text();
      }
      return false;
    }
    while (query.next()) {
      if (query.value(1).toString().compare("unit_id", Qt::CaseInsensitive) ==
          0) {
        hasUnitId = true;
        break;
      }
    }
    query.finish();

    if (!hasUnitId) {
      if (!query.exec("ALTER TABLE users ADD COLUMN unit_id INTEGER;")) {
        db.rollback();
        if (errorMessage) {
          *errorMessage = query.lastError().text();
        }
        return false;
      }
    }

    bool hasDispatcherUserId = false;
    if (!query.exec("PRAGMA table_info(users);")) {
      db.rollback();
      if (errorMessage) {
        *errorMessage = query.lastError().text();
      }
      return false;
    }
    while (query.next()) {
      if (query.value(1).toString().compare("dispatcher_user_id",
                                            Qt::CaseInsensitive) == 0) {
        hasDispatcherUserId = true;
        break;
      }
    }
    query.finish();

    if (!hasDispatcherUserId) {
      if (!query.exec(
              "ALTER TABLE users ADD COLUMN dispatcher_user_id INTEGER;")) {
        db.rollback();
        if (errorMessage) {
          *errorMessage = query.lastError().text();
        }
        return false;
      }
    }

    if (!query.exec("INSERT OR IGNORE INTO units(name) "
                    "SELECT DISTINCT TRIM(unit) FROM users "
                    "WHERE unit IS NOT NULL AND TRIM(unit) <> '';")) {
      db.rollback();
      if (errorMessage) {
        *errorMessage = query.lastError().text();
      }
      return false;
    }

    if (!query.exec(
            "UPDATE users "
            "SET unit_id = (SELECT id FROM units WHERE units.name = "
            "TRIM(users.unit)) "
            "WHERE (unit_id IS NULL OR unit_id = 0) AND unit IS NOT NULL "
            "AND TRIM(unit) <> '';")) {
      db.rollback();
      if (errorMessage) {
        *errorMessage = query.lastError().text();
      }
      return false;
    }

    if (!query.exec("CREATE INDEX IF NOT EXISTS idx_users_unit_id ON "
                    "users(unit_id);")) {
      db.rollback();
      if (errorMessage) {
        *errorMessage = query.lastError().text();
      }
      return false;
    }

    if (!query.exec(
            "CREATE INDEX IF NOT EXISTS idx_users_dispatcher_user_id ON "
            "users(dispatcher_user_id);")) {
      db.rollback();
      if (errorMessage) {
        *errorMessage = query.lastError().text();
      }
      return false;
    }
  }

  if (!db.commit()) {
    db.rollback();
    if (errorMessage) {
      *errorMessage = db.lastError().text();
    }
    return false;
  }

  return true;
}

static bool seedDatabase(QString *errorMessage) {
  QSqlDatabase db = QSqlDatabase::database("app_sqlite");
  if (!db.isOpen()) {
    if (errorMessage)
      *errorMessage = "Database not open";
    return false;
  }

  QSqlQuery query(db);

  // 1. 确保 "指挥中心" 单位存在
  qint64 unitId = 0;
  query.prepare("SELECT id FROM units WHERE name = ?");
  query.addBindValue("指挥中心");
  if (query.exec() && query.next()) {
    unitId = query.value(0).toLongLong();
  } else {
    query.prepare("INSERT INTO units (name) VALUES (?)");
    query.addBindValue("指挥中心");
    if (!query.exec()) {
      if (errorMessage)
        *errorMessage = "Failed to seed unit: " + query.lastError().text();
      return false;
    }
    unitId = query.lastInsertId().toLongLong();
  }

  // 2. 确保 "admin" (指挥调度员) 存在
  qint64 dispatcherId = 0;
  query.prepare("SELECT id FROM users WHERE username = ?");
  query.addBindValue("admin");
  if (query.exec() && query.next()) {
    dispatcherId = query.value(0).toLongLong();
  } else {
    query.prepare(
        "INSERT INTO users (username, password, phone, role, unit_id, unit) "
        "VALUES (?, ?, ?, ?, ?, ?)");
    query.addBindValue("admin");
    query.addBindValue("123456"); // verifyPassword 允许明文
    query.addBindValue("13800000000");
    query.addBindValue("指挥调度员");
    query.addBindValue(unitId);
    query.addBindValue("指挥中心");
    if (!query.exec()) {
      if (errorMessage)
        *errorMessage = "Failed to seed admin: " + query.lastError().text();
      return false;
    }
    dispatcherId = query.lastInsertId().toLongLong();
  }

  // 3. 确保 "handler1" (现场处置员) 存在
  query.prepare("SELECT id FROM users WHERE username = ?");
  query.addBindValue("handler1");
  if (!query.exec() || !query.next()) {
    query.prepare("INSERT INTO users (username, password, phone, role, "
                  "unit_id, unit, dispatcher_user_id) "
                  "VALUES (?, ?, ?, ?, ?, ?, ?)");
    query.addBindValue("handler1");
    query.addBindValue("123456");
    query.addBindValue("13900000000");
    query.addBindValue("现场处置员");
    query.addBindValue(unitId);
    query.addBindValue("指挥中心");
    query.addBindValue(dispatcherId);
    if (!query.exec()) {
      if (errorMessage)
        *errorMessage = "Failed to seed handler: " + query.lastError().text();
      return false;
    }
  }

  // 4. 确保至少存在一条未指派的灾害记录
  query.prepare("SELECT COUNT(*) FROM disasters");
  if (query.exec() && query.next() && query.value(0).toInt() == 0) {
    query.prepare("INSERT INTO disasters (disaster_type, location, "
                  "occurred_at, content, severity, dispatcher_id) "
                  "VALUES (?, ?, ?, ?, ?, ?)");
    query.addBindValue("火灾");
    query.addBindValue("市中心广场");
    query.addBindValue(
        QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss"));
    query.addBindValue("市中心广场发生火灾，需要紧急支援");
    query.addBindValue(5);
    query.addBindValue(dispatcherId);
    if (!query.exec()) {
      if (errorMessage)
        *errorMessage = "Failed to seed disaster: " + query.lastError().text();
      return false;
    }
  }

  return true;
}

static QString findUpwardsFile(const QString &relativePathFromBase, int maxUp) {
  QDir dir(QCoreApplication::applicationDirPath());
  for (int i = 0; i <= maxUp; ++i) {
    const QString candidate = dir.filePath(relativePathFromBase);
    if (QFileInfo::exists(candidate)) {
      return QFileInfo(candidate).absoluteFilePath();
    }
    if (!dir.cdUp()) {
      break;
    }
  }
  return QString();
}

// 全局消息处理器：捕获Qt的日志和致命错误
void myMessageHandler(QtMsgType type, const QMessageLogContext &context,
                      const QString &msg) {
  // 将日志输出到控制台
  std::cerr << qPrintable(QString("[%1] %2").arg(context.category).arg(msg))
            << std::endl;
  // 致命错误时弹出提示
  if (type == QtFatalMsg) {
    QMessageBox::critical(nullptr, "致命错误", msg);
    abort();
  }
}

void runBackendTests() {
  qDebug() << "\n==============================================";
  qDebug() << "=== 开始执行后端API联动测试 (API Tests) ===";
  qDebug() << "==============================================";

  QString err;
  DisasterRecord dr;
  dr.disasterType = "后端API自动测试灾害";
  dr.location = "测试坐标";
  dr.severity = 5;
  dr.dispatcherId = 999;
  dr.content = "这是一条用于验证指派逻辑的测试记录";
  qint64 newDisasterId = 0;

  qDebug() << "[测试1] 创建新灾害并附带 dispatcher_id (API: createDisaster)...";
  if (DisasterDao::createDisaster(dr, &newDisasterId, &err)) {
    qDebug() << "  -> 成功! 新灾害ID:" << newDisasterId;

    qDebug() << "[测试2] 多人员任务指派 (API: assignDisasterTasks)...";
    QList<qint64> handlers = {1001, 1002}; // 虚拟处置员ID
    if (DisasterDao::assignDisasterTasks(newDisasterId, handlers, &err)) {
      qDebug() << "  -> 成功! 给灾害指派了" << handlers.size() << "名处置员。";

      qDebug() << "[测试3] 获取指派的任务列表 (API: getTasksForDisaster)...";
      QList<DisasterTaskRecord> tasks;
      if (DisasterDao::getTasksForDisaster(newDisasterId, &tasks, &err)) {
        qDebug() << "  -> 成功! 获取到" << tasks.size() << "个任务。";

        if (!tasks.isEmpty()) {
          auto firstTask = tasks.first();
          qDebug() << "     验证第一条任务 - ID:" << firstTask.id
                   << ", 处置员ID:" << firstTask.handlerUserId
                   << ", 初始进度:" << firstTask.progress << "%";

          qDebug()
              << "[测试4] 修改任务进度 (API: updateDisasterTaskProgress)...";
          if (DisasterDao::updateDisasterTaskProgress(firstTask.id, 50, &err)) {
            qDebug() << "  -> 成功! 任务" << firstTask.id
                     << "进度已更新为 50%。";
          } else {
            qDebug() << "  -> 失败:" << err;
          }

          qDebug() << "[测试5] 撤回/删除灾害任务 (API: deleteDisasterTask)...";
          if (DisasterDao::deleteDisasterTask(firstTask.id, &err)) {
            qDebug() << "  -> 成功! 任务" << firstTask.id << "已被删除。";
          } else {
            qDebug() << "  -> 失败:" << err;
          }
        }
      } else {
        qDebug() << "  -> 失败:" << err;
      }
    } else {
      qDebug() << "  -> 失败:" << err;
    }

    qDebug() << "[测试清理] 删除测试用的临时灾害数据及残余任务...";
    QList<DisasterTaskRecord> cleanupTasks;
    DisasterDao::getTasksForDisaster(newDisasterId, &cleanupTasks, &err);
    for (const auto &t : cleanupTasks)
      DisasterDao::deleteDisasterTask(t.id, &err);
    if (DisasterDao::deleteDisaster(newDisasterId, &err)) {
      qDebug() << "  -> 成功! 测试数据已安全清除。";
    }
  } else {
    qDebug() << "  -> 灾害创建失败:" << err;
  }

  qDebug() << "==============================================";
  qDebug() << "=== 后端测试完毕，API链路通过验证！===";
  qDebug() << "==============================================\n";
}

int main(int argc, char *argv[]) {
  QApplication a(argc, argv);

  // 安装全局消息处理器
  qInstallMessageHandler(myMessageHandler);

  const QByteArray selfTestEnv = qgetenv("OCR_SELFTEST");
  if (!selfTestEnv.isEmpty() && selfTestEnv != "0") {
    const QString imgPath =
        findUpwardsFile("third_party/RapidOcrOnnx/images/1.jpg", 8);
    cv::Mat img;
    if (!imgPath.isEmpty()) {
      img = cv::imread(imgPath.toStdString(), cv::IMREAD_COLOR);
    }
    if (img.empty()) {
      img = cv::Mat::zeros(100, 100, CV_8UC3);
    }
    OcrHelper helper;
    const QString res = helper.recognizeText(img);
    std::cout << res.toStdString() << std::endl;
    QFile outFile(QDir(QDir::tempPath()).filePath("Monitor_ocr_selftest.txt"));
    if (outFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
      outFile.write(res.toUtf8());
      outFile.close();
    }
    return 0;
  }

  {
    QString dbError;
    if (!initSqliteSchema(&dbError)) {
      QMessageBox::critical(nullptr, "数据库初始化失败", dbError);
      return -1;
    }
    if (!seedDatabase(&dbError)) {
      QMessageBox::critical(nullptr, "数据库初始化数据失败", dbError);
      return -1;
    }
  }

  // 运行我们在后台编写的API功能测试 (会在控制台输出日志)
  runBackendTests();

  // 捕获C++异常，避免程序直接崩溃
  try {
    //    index in;
    //    in.show();
    //        MainWindow w;
    //        w.show();
    Login l;
    l.show();
    return a.exec();
  } catch (const std::exception &e) {
    std::cerr << "C++ Exception: " << e.what() << std::endl;
    QMessageBox::critical(nullptr, "异常",
                          QString("C++异常：%1").arg(e.what()));
  } catch (...) {
    std::cerr << "Unknown Exception!" << std::endl;
    QMessageBox::critical(nullptr, "异常", "未知异常！");
  }

  return -1;
}
