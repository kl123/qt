#ifndef IMAGE_H
#define IMAGE_H

#include <opencv2/opencv.hpp>
#include <QSettings>

class image
{
public:
    image();
    cv::Mat CutPhoto(const cv::Mat &currentFrame, bool isDarkMode);
    void StartAlert();//调用这个方法会按照设置开始预警

private:
    cv::Mat detectChatArea(const cv::Mat &frame, bool isDark);
};

#endif // IMAGE_H
