#include "mainwindow.h"
#include "ocrhelper.h"
#include <QApplication>
#include <QMessageBox>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <exception>
#include <iostream>
#include <opencv2/opencv.hpp>

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
