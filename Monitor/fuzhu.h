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
    // 发射区域参数：起始X百分比、宽度百分比、起始Y百分比、高度百分比
    void areaParamsConfirmed(int startXPercent, int widthPercent, int startYPercent, int heightPercent);

private slots:
    // 定时截屏的槽函数
    void captureScreenAndShow();
    // 滑块值变化时，重新画框并更新显示
    void onSliderValueChanged();
    // 点击确定按钮，计算并打印百分比
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
};

// 辅助函数：将cv::Mat转换为QImage（深拷贝，避免内存冲突）
QImage matToQImage(const cv::Mat &mat);

#endif // FUZHU_H
