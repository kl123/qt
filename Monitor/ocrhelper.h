#ifndef OCRHELPER_H
#define OCRHELPER_H

#include <QString>
#include <opencv2/opencv.hpp>
#include "OcrLite.h"

/**
 * @brief OCR 文字识别助手类
 * 封装了 OpenCV 图像处理和文字识别功能
 */
class OcrHelper
{
public:
    OcrHelper();
    ~OcrHelper();

    /**
     * @brief 识别图片中的文字
     * @param image OpenCV格式的图像矩阵
     * @return 识别出的完整文本
     */
    QString recognizeText(const cv::Mat &image);

    /**
     * @brief 提取新消息
     * 对比新旧文本，提取新增的部分 (通常用于日志监控)
     * @param oldText 上一次识别的文本
     * @param newText 当前识别的文本
     * @return 新增的文本内容
     */
    QString extractNewMessage(const QString &oldText, const QString &newText);

private:
    OcrLite *ocrEngine;
    bool isOcrInitialized;

    /**
     * @brief OCR 预处理
     * 对图像进行二值化、去噪等处理以提高识别率
     * @param input 输入图像
     * @return 处理后的图像
     */
    cv::Mat preprocessImageForOCR(const cv::Mat &input);
};

#endif // OCRHELPER_H
