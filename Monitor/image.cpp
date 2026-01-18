#include "image.h"
#include <opencv2/opencv.hpp>
#include <QSettings>
#include <QMessageBox>
#include "monitorconfig.h"
// 构造函数
image::image() {
}

// 主要方法：根据是否深色模式，裁剪聊天区域
cv::Mat image::CutPhoto(const cv::Mat &currentFrame, bool isDarkMode)
{
    if (currentFrame.empty()) return cv::Mat();

    cv::Mat chatArea = detectChatArea(currentFrame, isDarkMode);
    return chatArea;
}

// 辅助函数：检测聊天区域（基于颜色阈值）
cv::Mat image::detectChatArea(const cv::Mat &frame, bool isDark)
{
    cv::Mat hsv;
    cv::cvtColor(frame, hsv, cv::COLOR_BGR2HSV);

    cv::Mat mask;
    if (isDark) {
        // 深色模式：背景接近黑色（H: 0~180, S: 0~100, V: 0~50）
        cv::inRange(hsv, cv::Scalar(0, 0, 0), cv::Scalar(180, 100, 50), mask);
    } else {
        // 浅色模式：背景接近灰白（V > 200）
        cv::inRange(hsv, cv::Scalar(0, 0, 200), cv::Scalar(180, 255, 255), mask);
    }

    // 形态学操作去噪
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5, 5));
    cv::morphologyEx(mask, mask, cv::MORPH_CLOSE, kernel);

    // 查找最大轮廓（即聊天主区域）
    std::vector<std::vector<cv::Point>> contours;
    std::vector<cv::Vec4i> hierarchy;
    cv::findContours(mask, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    if (contours.empty()) return cv::Mat();

    // 找最大的轮廓
    auto largestContour = *std::max_element(contours.begin(), contours.end(),
                                            [](const std::vector<cv::Point>& a, const std::vector<cv::Point>& b) {
                                                return cv::contourArea(a) < cv::contourArea(b);
                                            });

    // 绘制外接矩形
    cv::Rect boundingRect = cv::boundingRect(largestContour);

    // -------------------------------------------------------------------------
    // 针对聊天软件的优化：强制去除左侧列表栏（约占宽度的 28%）
    // Optimization for chat apps: Force remove the left sidebar (approx 28% width)
    // -------------------------------------------------------------------------
    if (boundingRect.width > 200) { // 只对足够宽的区域执行此操作，防止误伤
        int sidebarWidth = boundingRect.width * 0.28;
        boundingRect.x += sidebarWidth;
        boundingRect.width -= sidebarWidth;

        // 确保没有减过头
        if (boundingRect.width <= 0) boundingRect.width = 10;
    }

    // 增加一些边界（防止裁剪太紧）
    int pad = 20;
    boundingRect.x = std::max(0, boundingRect.x - pad);
    boundingRect.y = std::max(0, boundingRect.y - pad);
    boundingRect.width = std::min(frame.cols - boundingRect.x, boundingRect.width + 2*pad);
    boundingRect.height = std::min(frame.rows - boundingRect.y, boundingRect.height + 2*pad);

    // 裁剪出聊天区域
    cv::Mat result = frame(boundingRect).clone();

    return result;
}

// 警示窗口
void image::StartAlert(){
    // 1.播放警告音
    MonitorConfig config;
    config.playAlertSound();

    // 2.弹出强制解除的模态警报（按钮显示“确定”）
    QMessageBox::critical(
        nullptr,
        "⚠警报",
        "有重要报警！\n请及时关注。",
        QStringLiteral("确定")  // 👈 关键修改：用 QString 替代 QMessageBox::Ok
        );

    // 3.停止音频
    config.stopAlertSound();
}
