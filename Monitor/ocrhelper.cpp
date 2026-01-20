#include "ocrhelper.h"
#include <QDir>
#include <QDebug>
#include <QCoreApplication>
#include <QFileInfo>

#include <onnxruntime_c_api.h>

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <cstdlib>
#include <new>
#include <vector>

#include "OcrLite.h"

namespace std {
__attribute__((weak)) [[noreturn]] void __throw_bad_array_new_length() {
#if defined(__cpp_exceptions) && __cpp_exceptions
    throw bad_array_new_length();
#else
    std::abort();
#endif
}
}

static std::wstring exeDirPathW() {
    wchar_t buffer[MAX_PATH];
    DWORD len = GetModuleFileNameW(nullptr, buffer, MAX_PATH);
    if (len == 0 || len >= MAX_PATH) {
        return L"";
    }
    std::wstring full(buffer, len);
    size_t pos = full.find_last_of(L"\\/"); 
    if (pos == std::wstring::npos) {
        return L"";
    }
    return full.substr(0, pos);
}

static std::wstring normalizePath(const std::wstring& p) {
    wchar_t out[MAX_PATH];
    DWORD len = GetFullPathNameW(p.c_str(), MAX_PATH, out, nullptr);
    if (len == 0 || len >= MAX_PATH) {
        return p;
    }
    return std::wstring(out, len);
}

static bool fileExistsW(const std::wstring& p) {
    DWORD attrs = GetFileAttributesW(p.c_str());
    return (attrs != INVALID_FILE_ATTRIBUTES) && !(attrs & FILE_ATTRIBUTE_DIRECTORY);
}

using OrtGetApiBaseFn = const OrtApiBase* (ORT_API_CALL*)();

extern "C" const OrtApiBase* ORT_API_CALL OrtGetApiBase(void) NO_EXCEPTION {
    static HMODULE ortModule = nullptr;
    static OrtGetApiBaseFn realOrtGetApiBase = nullptr;

    if (realOrtGetApiBase) {
        return realOrtGetApiBase();
    }

    auto tryLoad = [&](const std::wstring& dllPath) -> bool {
        std::wstring full = normalizePath(dllPath);
        if (!fileExistsW(full)) {
            return false;
        }
        ortModule = LoadLibraryW(full.c_str());
        return ortModule != nullptr;
    };

    const std::wstring exeDir = exeDirPathW();
    if (!exeDir.empty()) {
        std::vector<std::wstring> candidates = {
            exeDir + L"\\onnxruntime.dll",
            exeDir + L"\\..\\asset\\onnxruntime-win-x64-1.15.1\\lib\\onnxruntime.dll",
            exeDir + L"\\..\\..\\asset\\onnxruntime-win-x64-1.15.1\\lib\\onnxruntime.dll",
            exeDir + L"\\..\\..\\..\\asset\\onnxruntime-win-x64-1.15.1\\lib\\onnxruntime.dll",
        };
        for (const auto& c : candidates) {
            if (tryLoad(c)) {
                break;
            }
        }
    }

    if (!ortModule) {
        ortModule = LoadLibraryW(L"onnxruntime.dll");
    }

    if (!ortModule) {
        std::abort();
    }

    realOrtGetApiBase = reinterpret_cast<OrtGetApiBaseFn>(GetProcAddress(ortModule, "OrtGetApiBase"));
    if (!realOrtGetApiBase) {
        std::abort();
    }
    return realOrtGetApiBase();
}

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

static bool initRapidOcrOnce(OcrLite& engine) {
    const QString assetDir = findAssetDir();
    if (assetDir.isEmpty()) {
        return false;
    }

    const QString modelDir = QDir(assetDir).filePath("RapidOCR/models");
    const QString det = QDir(modelDir).filePath("ch_PP-OCRv3_det_infer.onnx");
    const QString cls = QDir(modelDir).filePath("ch_ppocr_mobile_v2.0_cls_infer.onnx");
    const QString rec = QDir(modelDir).filePath("ch_PP-OCRv3_rec_infer.onnx");
    const QString keys = QDir(modelDir).filePath("ppocr_keys_v1.txt");

    if (!QFileInfo(det).exists() || !QFileInfo(cls).exists() || !QFileInfo(rec).exists() || !QFileInfo(keys).exists()) {
        return false;
    }

    engine.setNumThread(4);
    engine.initLogger(false, false, false);
    engine.setGpuIndex(-1);

    return engine.initModels(det.toStdString(), cls.toStdString(), rec.toStdString(), keys.toStdString());
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

    cv::resize(processed, processed, cv::Size(), 2.0, 2.0, cv::INTER_CUBIC);

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

    static OcrLite engine;
    static bool initialized = false;
    static bool initOk = false;
    if (!initialized) {
        initOk = initRapidOcrOnce(engine);
        initialized = true;
    }

    if (!initOk) {
        return QString();
    }

    OcrResult result = engine.detect(processedImg, 50, 1024, 0.5f, 0.3f, 1.6f, true, true);
    return QString::fromStdString(result.strRes).trimmed();
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
