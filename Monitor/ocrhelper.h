#ifndef OCRHELPER_H
#define OCRHELPER_H

#include <QString>
#include <opencv2/opencv.hpp>

class OcrHelper
{
public:
    OcrHelper();
    ~OcrHelper();

    QString recognizeText(const cv::Mat &image);
    QString extractNewMessage(const QString &oldText, const QString &newText);

private:
    cv::Mat preprocessImageForOCR(const cv::Mat &input);
};

#endif // OCRHELPER_H
