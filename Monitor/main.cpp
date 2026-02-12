#include "mainwindow.h"
#include "login.h"
#include "ocrhelper.h"
#include "index.h"
#include <QApplication>
#include <QCoreApplication>
#include <QMessageBox>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QStringList>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlError>
#include <QtSql/QSqlQuery>
#include <exception>
#include <iostream>
#include <opencv2/opencv.hpp>

static bool initSqliteSchema(QString* errorMessage)
{
    const QString dbPath = QDir(QCoreApplication::applicationDirPath()).filePath("monitor.db");

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

        "CREATE INDEX IF NOT EXISTS idx_disaster_tasks_disaster_id ON disaster_tasks(disaster_id);",
        "CREATE INDEX IF NOT EXISTS idx_disaster_tasks_handler_user_id ON disaster_tasks(handler_user_id);",
        "CREATE INDEX IF NOT EXISTS idx_chat_messages_sender ON chat_messages(sender_user_id);",
        "CREATE INDEX IF NOT EXISTS idx_chat_messages_receiver ON chat_messages(receiver_user_id);",
        "CREATE INDEX IF NOT EXISTS idx_disasters_severity ON disasters(severity);"
    };

    for (const QString& sql : statements) {
        if (!query.exec(sql)) {
            db.rollback();
            if (errorMessage) {
                *errorMessage = query.lastError().text();
            }
            return false;
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
            if (query.value(1).toString().compare("unit_id", Qt::CaseInsensitive) == 0) {
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
            if (query.value(1).toString().compare("dispatcher_user_id", Qt::CaseInsensitive) == 0) {
                hasDispatcherUserId = true;
                break;
            }
        }
        query.finish();

        if (!hasDispatcherUserId) {
            if (!query.exec("ALTER TABLE users ADD COLUMN dispatcher_user_id INTEGER;")) {
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

        if (!query.exec("UPDATE users "
                        "SET unit_id = (SELECT id FROM units WHERE units.name = TRIM(users.unit)) "
                        "WHERE (unit_id IS NULL OR unit_id = 0) AND unit IS NOT NULL AND TRIM(unit) <> '';")) {
            db.rollback();
            if (errorMessage) {
                *errorMessage = query.lastError().text();
            }
            return false;
        }

        if (!query.exec("CREATE INDEX IF NOT EXISTS idx_users_unit_id ON users(unit_id);")) {
            db.rollback();
            if (errorMessage) {
                *errorMessage = query.lastError().text();
            }
            return false;
        }

        if (!query.exec("CREATE INDEX IF NOT EXISTS idx_users_dispatcher_user_id ON users(dispatcher_user_id);")) {
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

static QString findUpwardsFile(const QString& relativePathFromBase, int maxUp) {
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
void myMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    // 将日志输出到控制台
    std::cerr << qPrintable(QString("[%1] %2").arg(context.category).arg(msg)) << std::endl;
    // 致命错误时弹出提示
    if (type == QtFatalMsg) {
        QMessageBox::critical(nullptr, "致命错误", msg);
        abort();
    }
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // 安装全局消息处理器
    qInstallMessageHandler(myMessageHandler);

    const QByteArray selfTestEnv = qgetenv("OCR_SELFTEST");
    if (!selfTestEnv.isEmpty() && selfTestEnv != "0") {
        const QString imgPath = findUpwardsFile("third_party/RapidOcrOnnx/images/1.jpg", 8);
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
    }

    // 捕获C++异常，避免程序直接崩溃
    try {
        index in;
        in.show();
//        MainWindow w;
//        w.show();
//        Login l;
//        l.show();
        return a.exec();
    } catch (const std::exception &e) {
        std::cerr << "C++ Exception: " << e.what() << std::endl;
        QMessageBox::critical(nullptr, "异常", QString("C++异常：%1").arg(e.what()));
    } catch (...) {
        std::cerr << "Unknown Exception!" << std::endl;
        QMessageBox::critical(nullptr, "异常", "未知异常！");
    }

    return -1;
}
