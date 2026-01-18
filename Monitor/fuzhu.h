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
    // 修改信号：传递检测区域+事件卡片区域参数（共8个）
    void areaParamsConfirmed(
        int detectStartX, int detectWidth, int detectStartY, int detectHeight,
        int cardStartX, int cardWidth, int cardStartY, int cardHeight
    );

private slots:
    // 定时截屏的槽函数
    void captureScreenAndShow();
    // 滑块值变化时，重新画框并更新显示（新增drawMat参数）
    void onSliderValueChanged(cv::Mat &drawMat);
    // 点击确定按钮，计算并打印百分比（两步选择逻辑）
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

    // 新增：两步选择相关变量
    int m_selectStage = 0;            // 0=选检测区域，1=选事件卡片区域
    // 检测区域参数（第一步确认后保存）
    int m_detectStartX = 0;
    int m_detectWidth = 0;
    int m_detectStartY = 0;
    int m_detectHeight = 0;
    // 事件卡片区域参数（第二步确认后保存）
    int m_cardStartX = 0;
    int m_cardWidth = 0;
    int m_cardStartY = 0;
    int m_cardHeight = 0;

    void saveAreaSettings();   // 保存配置
};

// 辅助函数：将cv::Mat转换为QImage（深拷贝，避免内存冲突）
QImage matToQImage(const cv::Mat &mat);

#endif // FUZHU_H
