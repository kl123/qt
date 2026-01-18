#include "ocrhelper.h"
#include <QProcess>
#include <QFile>
#include <QDir>
#include <QUuid>
#include <QDebug>
#include <QImage>
#include <QCoreApplication>

OcrHelper::OcrHelper()
{
}

OcrHelper::~OcrHelper()
{
}

cv::Mat OcrHelper::preprocessImageForOCR(const cv::Mat &input)
{
    if (input.empty()) return input;
    cv::Mat gray, processed;
    // 1. 转换为灰度图
    if (input.channels() == 3) {
        cv::cvtColor(input, gray, cv::COLOR_BGR2GRAY);
    } else {
        gray = input.clone();
    }
    // 2. 调整大小（放大 2 倍以获得更好的 OCR 效果）
    cv::resize(gray, processed, cv::Size(), 2.0, 2.0, cv::INTER_CUBIC);

    // 3. 锐化
    cv::Mat kernel = (cv::Mat_<float>(3,3) <<
                      0, -1, 0,
                      -1, 5, -1,
                      0, -1, 0);
    cv::filter2D(processed, processed, processed.depth(), kernel);

    // 可选：阈值化（二值化）
    // cv::threshold(processed, processed, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);

    return processed;
}

QString OcrHelper::recognizeText(const cv::Mat &image)
{
    if (image.empty()) {
        return QString();
    }

    // 0. 预处理图像
    cv::Mat processedImg = preprocessImageForOCR(image);

    // 1. 将图像保存到临时文件
    // 使用 QImage 保存以避免 cv::imwrite 导致的 MinGW/OpenCV ABI 问题
    QString tempImgName = QDir::temp().filePath("ocr_" + QUuid::createUuid().toString(QUuid::WithoutBraces) + ".png");
    tempImgName = QDir::toNativeSeparators(tempImgName);

    QImage qImg(processedImg.data, processedImg.cols, processedImg.rows, processedImg.step, QImage::Format_Grayscale8);
    if (qImg.isNull()) {
         qDebug() << "Failed to convert cv::Mat to QImage for OCR";
         return QString();
    }

    if (!qImg.save(tempImgName)) {
        qDebug() << "Failed to save temporary image for OCR:" << tempImgName;
        return QString();
    }

    // 2. 准备 Tesseract 的输出文件（以避免 stdout 管道阻塞/编码问题）
    QString tempOutBase = QDir::temp().filePath("ocr_out_" + QUuid::createUuid().toString(QUuid::WithoutBraces));
    QString tempOutFile = tempOutBase + ".txt"; // Tesseract 根据配置自动添加扩展名，但对于标准的 'txt' 配置，它会添加 .txt
    tempOutBase = QDir::toNativeSeparators(tempOutBase);

    // 3. 运行 tesseract.exe
    // 获取程序所在目录
    QString appDir = QCoreApplication::applicationDirPath();
    QProcess process;
    QString program = appDir+"/asset/Tesseract-OCR/tesseract.exe";
    QStringList arguments;

    // 语法：tesseract imagename outputbase [options...] [configfile...]
    arguments << tempImgName << tempOutBase << "-l" << "chi_sim+eng";

    // 如果需要，设置 TESSDATA_PREFIX（可选，但有利于稳定性）
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert("TESSDATA_PREFIX", appDir+"/asset/Tesseract-OCR/tessdata");
    process.setProcessEnvironment(env);

    process.start(program, arguments);
    if (!process.waitForStarted()) {
        qDebug() << "Failed to start tesseract process. Is it installed at" << program << "?";
        QFile::remove(tempImgName);
        return QString();
    }

    if (!process.waitForFinished(20000)) { // 20 秒超时
        qDebug() << "Tesseract process timed out.";
        process.kill();
        QFile::remove(tempImgName);
        QFile::remove(tempOutFile);
        return QString();
    }

    // 4. 从文件中读取输出
    QString result;
    QFile outFile(tempOutFile);
    if (outFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QByteArray output = outFile.readAll();
        result = QString::fromUtf8(output).trimmed();
        outFile.close();
    } else {
        qDebug() << "Failed to read Tesseract output file:" << tempOutFile;
    }

    // 调试信息
    qDebug() << "Recognized Text Length:" << result.length();
    // qDebug() << "Recognized Text:" << result;

    // 5. 清理
    QFile::remove(tempImgName);
    QFile::remove(tempOutFile);

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
