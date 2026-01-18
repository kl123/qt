#include "mainwindow.h"
#include <QApplication>
#include <QMessageBox>
#include <exception>
#include <iostream>

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

    // 捕获C++异常，避免程序直接崩溃
    try {
        MainWindow w;
        w.show();
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
