#include "ocrhelper.h"
#include <QDir>
#include <QDebug>
#include <QCoreApplication>
#include <QFileInfo>
#include <QProcess>
#include <QTemporaryFile>
#include <QFile>

// 辅助函数：查找目录
static QString findDirWithChild(const QString& startDir, const QString& childDirName, int maxUp) {
    QDir dir(startDir);
    for (int i = 0; i <= maxUp; ++i) {
        QString candidate = dir.filePath(childDirName);
        if (QFileInfo(candidate).exists()) {
            return dir.absolutePath();
        }
        if (!dir.cdUp()) {
            break;
        }
    }
    return QString();
}

static QString findAssetDir() {
    const QString start = QCoreApplication::applicationDirPath();
    const QString base = findDirWithChild(start, "asset", 6);
    if (base.isEmpty()) {
        return QString();
    }
    return QDir(base).filePath("asset");
}

OcrHelper::OcrHelper()
{
}

OcrHelper::~OcrHelper()
{
}

cv::Mat OcrHelper::preprocessImageForOCR(const cv::Mat &input)
{
    if (input.empty()) return input;
    cv::Mat processed;
    if (input.channels() == 1) {
        cv::cvtColor(input, processed, cv::COLOR_GRAY2BGR);
    } else if (input.channels() == 4) {
        cv::cvtColor(input, processed, cv::COLOR_BGRA2BGR);
    } else {
        processed = input.clone();
    }

    // 调整大小，放大2倍以提高识别率
    cv::resize(processed, processed, cv::Size(), 2.0, 2.0, cv::INTER_CUBIC);

    // 锐化
    cv::Mat kernel = (cv::Mat_<float>(3,3) <<
                      0, -1, 0,
                      -1, 5, -1,
                      0, -1, 0);
    cv::filter2D(processed, processed, processed.depth(), kernel);

    return processed;
}

QString OcrHelper::recognizeText(const cv::Mat &image)
{
    if (image.empty()) {
        return QString();
    }

    cv::Mat processedImg = preprocessImageForOCR(image);

    // 1. 保存图片到临时文件
    QString tempImgPath = QDir::temp().filePath("monitor_ocr_temp.png");
    if (!cv::imwrite(tempImgPath.toStdString(), processedImg)) {
        qDebug() << "Failed to save temp image for OCR";
        return QString();
    }

    // 2. 查找 Tesseract 可执行文件
    QString assetDir = findAssetDir();
    if (assetDir.isEmpty()) {
        qDebug() << "Asset dir not found";
        return QString();
    }
    QString tesseractExe = QDir(assetDir).filePath("Tesseract-OCR/tesseract.exe");
    QString tessDataDir = QDir(assetDir).filePath("Tesseract-OCR/tessdata");

    if (!QFileInfo(tesseractExe).exists()) {
        qDebug() << "Tesseract exe not found at:" << tesseractExe;
        return QString();
    }

    // 3. 调用 Tesseract
    QProcess process;
    // 设置 TESSDATA_PREFIX 环境变量
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert("TESSDATA_PREFIX", tessDataDir);
    process.setProcessEnvironment(env);

    QStringList args;
    args << tempImgPath << "stdout" << "-l" << "chi_sim+eng";

    process.start(tesseractExe, args);
    if (!process.waitForFinished(10000)) { // 等待最多10秒
        qDebug() << "Tesseract process timed out or failed";
        return QString();
    }

    QByteArray output = process.readAllStandardOutput();
    QByteArray error = process.readAllStandardError();

    if (!error.isEmpty()) {
        // Tesseract 输出很多 info 到 stderr，不一定是错误
        // qDebug() << "Tesseract stderr:" << error;
    }

    // 清理临时文件
    QFile::remove(tempImgPath);

    QString result = QString::fromUtf8(output).trimmed();
    return result;
}

QString OcrHelper::extractNewMessage(const QString &oldText, const QString &newText)
{
    QStringList oldLines = oldText.split('\n', Qt::SkipEmptyParts);
    QStringList newLines = newText.split('\n', Qt::SkipEmptyParts);

    if (oldLines.isEmpty()) {
        return newText;
    }

    // 查找重叠部分
    for (int i = 0; i < oldLines.size(); ++i) {
        bool match = true;
        int overlapLen = oldLines.size() - i;
        if (overlapLen > newLines.size()) {
            continue;
        }

        for (int k = 0; k < overlapLen; ++k) {
            // 使用简化比较以容忍微小的 OCR 错误
            QString s1 = oldLines[i + k].simplified().remove(" ");
            QString s2 = newLines[k].simplified().remove(" ");
            if (s1 != s2) {
                match = false;
                break;
            }
        }

        if (match) {
            QStringList newMessages;
            for (int j = overlapLen; j < newLines.size(); ++j) {
                newMessages.append(newLines[j]);
            }
            return newMessages.join('\n');
        }
    }

    if (oldText == newText) {
        return QString();
    }

    return newText;
}
