#ifndef FUZHU_H
#define FUZHU_H

#include <QMainWindow>
#include <QTimer>
#include <QSize>
#include <opencv2/opencv.hpp>

namespace Ui {
class fuzhu;
}

class fuzhu : public QMainWindow
{
    Q_OBJECT

public:
    explicit fuzhu(QWidget *parent = nullptr);
    ~fuzhu();

signals:
    // 简化信号：仅传递检测区域参数（4个）
    void areaParamsConfirmed(
        int detectStartX, int detectWidth, int detectStartY, int detectHeight
    );

private slots:
    // 定时截屏的槽函数
    void captureScreenAndShow();
    // 滑块值变化时，重新画框并更新显示
    void onSliderValueChanged(cv::Mat &drawMat);
    // 点击确定按钮，计算并打印百分比（仅检测区域）
    void onConfirmButtonClicked();

private:
    Ui::fuzhu *ui;

    // 定时截屏的定时器
    QTimer *m_captureTimer;
    // 存储当前屏幕截图的OpenCV图像
    cv::Mat m_screenMat;
    // 屏幕的实际尺寸（像素）
    QSize m_screenSize;

    // 区域调整的参数（0-100的百分比）
    int m_x1 = 20; // 左上角x百分比
    int m_x2 = 80; // 右下角x百分比
    int m_y1 = 20; // 左上角y百分比
    int m_y2 = 80; // 右下角y百分比

    // 仅保留检测区域相关变量（移除卡片区域和阶段变量）
    int m_detectStartX = 0;
    int m_detectWidth = 0;
    int m_detectStartY = 0;
    int m_detectHeight = 0;

    void saveAreaSettings();   // 保存配置（仅保存检测区域）
};

// 辅助函数：将cv::Mat转换为QImage（深拷贝，避免内存冲突）
QImage matToQImage(const cv::Mat &mat);

#endif // FUZHU_H
