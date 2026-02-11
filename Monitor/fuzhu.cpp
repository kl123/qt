#include "fuzhu.h"
#include "ui_fuzhu.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QSlider>
#include <QPushButton>
#include <QDebug>
#include <QImage>
#include <QScreen>
#include <QGuiApplication>
#include <cstring>
#include <iostream>
#include <QList>
#include<QSettings>

// 辅助函数：将cv::Mat转换为QImage（深拷贝，避免内存冲突）
QImage matToQImage(const cv::Mat &mat)
{
    QImage img;
    if (mat.empty()) {
        return img;
    }

    // 处理RGB图像（OpenCV是BGR格式，需要转换）
    if (mat.type() == CV_8UC3) {
        cv::Mat rgbMat;
        cv::cvtColor(mat, rgbMat, cv::COLOR_BGR2RGB);
        // 深拷贝：创建新的QImage，拷贝数据
        img = QImage((const uchar*)rgbMat.data, rgbMat.cols, rgbMat.rows, rgbMat.step, QImage::Format_RGB888)
              .copy(); // 关键：添加copy()深拷贝
    }
    // 处理灰度图像
    else if (mat.type() == CV_8UC1) {
        img = QImage((const uchar*)mat.data, mat.cols, mat.rows, mat.step, QImage::Format_Grayscale8)
              .copy(); // 关键：添加copy()深拷贝
    }

    return img;
}

fuzhu::fuzhu(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::fuzhu)
{
    ui->setupUi(this);

    // ========== 初始化成员变量（避免随机值导致的崩溃） ==========
    m_captureTimer = new QTimer(this);
    m_captureTimer->setInterval(100); // 增大间隔到100ms，降低性能压力
    m_screenSize = QSize(800, 600);   // 默认尺寸，避免后续计算错误

    // ========== 1. 初始化UI控件（手动添加，适配空ui文件） ==========
    // 创建中心窗口和主布局
    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    this->setCentralWidget(centralWidget);

    // 1.1 用于显示截屏的Label（设置固定大小或自适应）
    QLabel *imgLabel = new QLabel(this);
    imgLabel->setMinimumSize(800, 600);
    imgLabel->setStyleSheet("border: 1px solid black;");
    imgLabel->setScaledContents(true); // 自适应Label大小显示图像
    mainLayout->addWidget(imgLabel);

    // 1.2 滑块布局（四个滑块：x1, x2, y1, y2，范围0-100）
    QWidget *sliderWidget = new QWidget(this);
    QVBoxLayout *sliderLayout = new QVBoxLayout(sliderWidget);

    // x1滑块（左上角x百分比）- 标注检测区域
    QHBoxLayout *x1Layout = new QHBoxLayout;
    QLabel *x1Label = new QLabel("检测区域-X1 (左上角%):", this);
    QSlider *x1Slider = new QSlider(Qt::Horizontal, this);
    x1Slider->setRange(0, 100);
    x1Slider->setValue(m_x1);
    x1Layout->addWidget(x1Label);
    x1Layout->addWidget(x1Slider);
    sliderLayout->addLayout(x1Layout);

    // x2滑块（右下角x百分比）- 标注检测区域
    QHBoxLayout *x2Layout = new QHBoxLayout;
    QLabel *x2Label = new QLabel("检测区域-X2 (右下角%):", this);
    QSlider *x2Slider = new QSlider(Qt::Horizontal, this);
    x2Slider->setRange(0, 100);
    x2Slider->setValue(m_x2);
    x2Layout->addWidget(x2Label);
    x2Layout->addWidget(x2Slider);
    sliderLayout->addLayout(x2Layout);

    // y1滑块（左上角y百分比）- 标注检测区域
    QHBoxLayout *y1Layout = new QHBoxLayout;
    QLabel *y1Label = new QLabel("检测区域-Y1 (左上角%):", this);
    QSlider *y1Slider = new QSlider(Qt::Horizontal, this);
    y1Slider->setRange(0, 100);
    y1Slider->setValue(m_y1);
    y1Layout->addWidget(y1Label);
    y1Layout->addWidget(y1Slider);
    sliderLayout->addLayout(y1Layout);

    // y2滑块（右下角y百分比）- 标注检测区域
    QHBoxLayout *y2Layout = new QHBoxLayout;
    QLabel *y2Label = new QLabel("检测区域-Y2 (右下角%):", this);
    QSlider *y2Slider = new QSlider(Qt::Horizontal, this);
    y2Slider->setRange(0, 100);
    y2Slider->setValue(m_y2);
    y2Layout->addWidget(y2Label);
    y2Layout->addWidget(y2Slider);
    sliderLayout->addLayout(y2Layout);

    mainLayout->addWidget(sliderWidget);

    // 1.3 确定按钮 - 固定文本为“确认检测区域”
    QPushButton *confirmBtn = new QPushButton("确认检测区域", this);
    mainLayout->addWidget(confirmBtn, 0, Qt::AlignCenter);

    // ========== 2. 连接信号槽 ==========
    // 定时器触发截屏
    connect(m_captureTimer, &QTimer::timeout, this, [=]() {
        // 执行截屏
        captureScreenAndShow();
        // 创建临时图像副本（避免画框叠加）
        cv::Mat tempMat = m_screenMat.clone();
        // 仅画红框（检测区域）
        onSliderValueChanged(tempMat);
        // 转换Mat为QImage并显示
        QImage img = matToQImage(tempMat);
        if (!img.isNull()) {
            imgLabel->setPixmap(QPixmap::fromImage(img).scaled(imgLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
        } else {
            // 图像为空时显示提示
            imgLabel->setText("无法获取屏幕截图");
        }
    });

    // 滑块值变化时更新参数并刷新红框
    connect(x1Slider, &QSlider::valueChanged, this, [=](int value) {
        m_x1 = value;
        cv::Mat tempMat = m_screenMat.clone();
        onSliderValueChanged(tempMat);
        QImage img = matToQImage(tempMat);
        if (!img.isNull()) {
            imgLabel->setPixmap(QPixmap::fromImage(img).scaled(imgLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
        }
    });
    connect(x2Slider, &QSlider::valueChanged, this, [=](int value) {
        m_x2 = value;
        cv::Mat tempMat = m_screenMat.clone();
        onSliderValueChanged(tempMat);
        QImage img = matToQImage(tempMat);
        if (!img.isNull()) {
            imgLabel->setPixmap(QPixmap::fromImage(img).scaled(imgLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
        }
    });
    connect(y1Slider, &QSlider::valueChanged, this, [=](int value) {
        m_y1 = value;
        cv::Mat tempMat = m_screenMat.clone();
        onSliderValueChanged(tempMat);
        QImage img = matToQImage(tempMat);
        if (!img.isNull()) {
            imgLabel->setPixmap(QPixmap::fromImage(img).scaled(imgLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
        }
    });
    connect(y2Slider, &QSlider::valueChanged, this, [=](int value) {
        m_y2 = value;
        cv::Mat tempMat = m_screenMat.clone();
        onSliderValueChanged(tempMat);
        QImage img = matToQImage(tempMat);
        if (!img.isNull()) {
            imgLabel->setPixmap(QPixmap::fromImage(img).scaled(imgLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
        }
    });

    // 确定按钮点击事件
    connect(confirmBtn, &QPushButton::clicked, this, &fuzhu::onConfirmButtonClicked);

    // ========== 3. 延迟启动定时器（等待UI初始化完成，避免崩溃） ==========
    QTimer::singleShot(500, this, [=]() {
        m_captureTimer->start();
    });

    // 设置窗口属性
    this->setWindowTitle("辅助窗口 - 截屏与检测区域调整");
    this->resize(900, 700);
}

fuzhu::~fuzhu()
{
    // 停止定时器，避免内存泄漏
    if (m_captureTimer && m_captureTimer->isActive()) {
        m_captureTimer->stop();
    }
    delete ui;
}

// ========== 核心功能：定时截屏（添加全量空值保护） ==========
void fuzhu::captureScreenAndShow()
{
    // 1. 检查屏幕获取是否成功
    QScreen *screen = QGuiApplication::primaryScreen();
    if (!screen) {
        // 截图失败，创建灰色空白图像
        m_screenMat = cv::Mat(600, 800, CV_8UC3, cv::Scalar(200, 200, 200));
        m_screenSize = QSize(800, 600);
        return;
    }

    // 2. 截取整个屏幕
    QPixmap pixmap = screen->grabWindow(0);
    if (pixmap.isNull()) {
        // 截图失败，创建灰色空白图像
        m_screenMat = cv::Mat(600, 800, CV_8UC3, cv::Scalar(200, 200, 200));
        m_screenSize = QSize(800, 600);
        return;
    }

    // 3. 转换为QImage并检查有效性
    QImage image = pixmap.toImage().convertToFormat(QImage::Format_RGB888);
    if (image.isNull() || image.width() <= 0 || image.height() <= 0) {
        // 转换失败，创建灰色空白图像
        m_screenMat = cv::Mat(600, 800, CV_8UC3, cv::Scalar(200, 200, 200));
        m_screenSize = QSize(800, 600);
        return;
    }

    // 4. 安全转换QImage到cv::Mat（深拷贝，避免内存越界）
    cv::Mat tempMat(image.height(), image.width(), CV_8UC3, cv::Scalar(0));
    memcpy(tempMat.data, image.bits(), image.sizeInBytes());
    // 转换为BGR格式（OpenCV默认）
    cv::cvtColor(tempMat, m_screenMat, cv::COLOR_RGB2BGR);

    // 5. 更新屏幕尺寸
    m_screenSize = QSize(image.width(), image.height());
}

// ========== 滑块值变化：仅画红框（检测区） ==========
void fuzhu::onSliderValueChanged(cv::Mat &drawMat)
{
    // 空值保护：Mat为空或尺寸无效时直接返回
    if (drawMat.empty() || m_screenSize.width() <= 0 || m_screenSize.height() <= 0) {
        return;
    }

    // 确保x1 < x2，y1 < y2（避免滑块值颠倒）
    int x1 = qMin(m_x1, m_x2);
    int x2 = qMax(m_x1, m_x2);
    int y1 = qMin(m_y1, m_y2);
    int y2 = qMax(m_y1, m_y2);

    // 计算当前滑块对应的像素坐标
    int px1 = (m_screenSize.width() * x1) / 100;
    int px2 = (m_screenSize.width() * x2) / 100;
    int py1 = (m_screenSize.height() * y1) / 100;
    int py2 = (m_screenSize.height() * y2) / 100;

    // 仅画红色框（检测区）
    cv::rectangle(drawMat, cv::Point(px1, py1), cv::Point(px2, py2), cv::Scalar(0, 0, 255), 2);
}

// 简化：仅保存检测区域配置
void fuzhu::saveAreaSettings()
{
    QSettings settings("MyCompany", "MonitorApp");

    // 仅保存检测区域参数
    settings.setValue("detectStartX", m_detectStartX);
    settings.setValue("detectWidth", m_detectWidth);
    settings.setValue("detectStartY", m_detectStartY);
    settings.setValue("detectHeight", m_detectHeight);

    // 保存滑块当前值（可选）
    settings.setValue("sliderX1", m_x1);
    settings.setValue("sliderX2", m_x2);
    settings.setValue("sliderY1", m_y1);
    settings.setValue("sliderY2", m_y2);

    settings.sync(); // 强制写入配置文件（避免缓存）
    qDebug() << "配置保存完成：检测区域参数已写入";
}

// ========== 确定按钮：仅处理检测区域确认 ==========
void fuzhu::onConfirmButtonClicked()
{
    // 确保x1 < x2，y1 < y2
    int x1 = qMin(m_x1, m_x2);
    int x2 = qMax(m_x1, m_x2);
    int y1 = qMin(m_y1, m_y2);
    int y2 = qMax(m_y1, m_y2);

    // 计算区域占比
    int startXPercent = x1;
    int widthPercent = x2 - x1;
    int startYPercent = y1;
    int heightPercent = y2 - y1;
    double totalPercent = (widthPercent * heightPercent) / 100.0;

    // 保存检测区域参数
    m_detectStartX = startXPercent;
    m_detectWidth = widthPercent;
    m_detectStartY = startYPercent;
    m_detectHeight = heightPercent;

    // 打印检测区域信息
    qDebug() << "---------- 检测区域占比信息 ----------";
    qDebug() << "检测区域左上角X起始占屏幕X轴：" << startXPercent << "%";
    qDebug() << "检测区域宽度占屏幕X轴：" << widthPercent << "%";
    qDebug() << "检测区域左上角Y起始占屏幕Y轴：" << startYPercent << "%";
    qDebug() << "检测区域高度占屏幕Y轴：" << heightPercent << "%";
    qDebug() << "检测区域整体占屏幕：" << totalPercent << "%";
    qDebug() << "----------------------------------";

    std::cout << "---------- 检测区域占比信息 ----------" << std::endl;
    std::cout << "检测区域左上角X起始占屏幕X轴：" << startXPercent << "%" << std::endl;
    std::cout << "检测区域宽度占屏幕X轴：" << widthPercent << "%" << std::endl;
    std::cout << "检测区域左上角Y起始占屏幕Y轴：" << startYPercent << "%" << std::endl;
    std::cout << "检测区域高度占屏幕Y轴：" << heightPercent << "%" << std::endl;
    std::cout << "检测区域整体占屏幕：" << totalPercent << "%" << std::endl;
    std::cout << "----------------------------------" << std::endl;

    // 保存配置
    saveAreaSettings();

    // 发射信号：仅传递检测区域参数
    emit areaParamsConfirmed(
        m_detectStartX, m_detectWidth, m_detectStartY, m_detectHeight
    );

    // 关闭窗口
    this->close();
}
