#include "ocrhelper.h"
#include <QDir>
#include <QDebug>
#include <QCoreApplication>
#include <QFileInfo>
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
    // 向上查找包含 asset 的目录
    const QString base = findDirWithChild(start, "asset", 6);
    if (base.isEmpty()) {
        return QString();
    }
    return QDir(base).filePath("asset");
}

OcrHelper::OcrHelper() : ocrEngine(nullptr), isOcrInitialized(false)
{
    QString assetDir = findAssetDir();
    if (assetDir.isEmpty()) {
        qDebug() << "Asset dir not found for RapidOCR";
        return;
    }

    QString rapidOcrDir = QDir(assetDir).filePath("RapidOCR");
    QString modelsDir = QDir(rapidOcrDir).filePath("models");

    // 检查模型文件是否存在
    QString detFile = QDir(modelsDir).filePath("ch_PP-OCRv3_det_infer.onnx");
    QString clsFile = QDir(modelsDir).filePath("ch_ppocr_mobile_v2.0_cls_infer.onnx");
    QString recFile = QDir(modelsDir).filePath("ch_PP-OCRv3_rec_infer.onnx");
    QString keysFile = QDir(modelsDir).filePath("ppocr_keys_v1.txt");

    if (!QFileInfo::exists(detFile) || !QFileInfo::exists(clsFile) || 
        !QFileInfo::exists(recFile) || !QFileInfo::exists(keysFile)) {
        qDebug() << "RapidOCR models missing in:" << modelsDir;
        return;
    }

    std::string detPath = detFile.toStdString();
    std::string clsPath = clsFile.toStdString();
    std::string recPath = recFile.toStdString();
    std::string keysPath = keysFile.toStdString();

    try {
        ocrEngine = new OcrLite();
        ocrEngine->setNumThread(4);
        ocrEngine->initLogger(false, false, false);
        
        bool ret = ocrEngine->initModels(detPath, clsPath, recPath, keysPath);
        if (!ret) {
            qDebug() << "Failed to init RapidOCR models";
            delete ocrEngine;
            ocrEngine = nullptr;
        } else {
            isOcrInitialized = true;
            qDebug() << "RapidOCR initialized successfully";
        }
    } catch (const std::exception& e) {
        qDebug() << "Exception during RapidOCR init:" << e.what();
        if (ocrEngine) {
            delete ocrEngine;
            ocrEngine = nullptr;
        }
    }
}

OcrHelper::~OcrHelper()
{
    if (ocrEngine) {
        delete ocrEngine;
        ocrEngine = nullptr;
    }
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
    // RapidOCR 也可以处理小图，但放大通常有助于识别小字
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

    if (!isOcrInitialized || !ocrEngine) {
        qDebug() << "OCR engine not initialized, cannot recognize text";
        return QString();
    }

    cv::Mat processedImg = preprocessImageForOCR(image);

    // RapidOCR 参数
    int padding = 50;
    int maxSideLen = 1024;
    float boxScoreThresh = 0.5f;
    float boxThresh = 0.3f;
    float unClipRatio = 1.6f;
    bool doAngle = true;
    bool mostAngle = true;

    try {
        OcrResult result = ocrEngine->detect(processedImg, padding, maxSideLen, 
                                            boxScoreThresh, boxThresh, unClipRatio, 
                                            doAngle, mostAngle);
        
        // 简单返回所有识别到的文本
        return QString::fromStdString(result.strRes);
    } catch (const std::exception& e) {
        qDebug() << "Exception during OCR detection:" << e.what();
        return QString();
    }
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
