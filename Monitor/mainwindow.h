#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTimer>
#include <QScreen>
#include <QGuiApplication>
#include <opencv2/opencv.hpp>
#include <image.h>
#include <QDateTime>
#include <QTextEdit>
#include <list>
#include <QSettings>

// 前向声明fuzhu类（避免头文件循环依赖）
class fuzhu;

// 定义聊天记录节点结构
struct ChatMessage {
    QDateTime timestamp;
    QString content;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onStartMonitoring();
    void onStopMonitoring();
    void onCaptureScreen();
    void openConfig();
    cv::Mat detectAndCaptureChanges(const cv::Mat &currentFrame);
    void onTestAlert();
    void onViewHistory();
    void onContinuousDetect();
    void onContinuousCapture();
    void stopAllMonitoring();
    // 接收fuzhu窗口传递的区域参数的槽函数
    void onAreaParamsReceived(int startXPercent, int widthPercent, int startYPercent, int heightPercent);

private:
    // 辅助函数：根据百分比裁剪图像
    cv::Mat cropImageByPercent(const cv::Mat &src, int startXPercent, int widthPercent, int startYPercent, int heightPercent);

    QLabel *imageLabel;
    QLabel *croppedLabel;
    QTextEdit *logTextEdit;
    QPushButton *startButton;
    QPushButton *stopButton;
    QPushButton *configButton;
    QTimer *timer;

    cv::Mat lastFrame;
    image m_imageProcessor;

    QDateTime monitoringStartTime;
    bool isMonitoringActive = false;

    QPushButton* testAlertButton;
    QPushButton* viewHistoryButton;

    QPushButton *continuousDetectButton = nullptr;
    QTimer *continuousTimer = nullptr;
    cv::Mat continuousLastFrame;
    bool isContinuousDetecting = false;

    // 聊天记录链表
    std::list<ChatMessage> m_chatHistory;

    // 存储从fuzhu窗口接收的监测区域参数
    int m_areaStartX = 0;
    int m_areaWidth = 100;
    int m_areaStartY = 0;
    int m_areaHeight = 100;
    cv::Mat m_croppedLastFrame;
};

#endif // MAINWINDOW_H
