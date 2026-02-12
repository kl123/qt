#include "mainwindow.h"
#include "ocrhelper.h"
#include <QPixmap>
#include <QScreen>
#include <QGuiApplication>
#include <QTimer>
#include <opencv2/opencv.hpp>
#include <iostream>
#include "monitorconfig.h"
#include "image.h"
#include <QSettings>
#include <QDebug>
#include <QString>
#include <QMessageBox>
#include "fuzhu.h"
#include "disasterdao.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      imageLabel(new QLabel(this)),
      croppedLabel(new QLabel(this)),
      logTextEdit(new QTextEdit(this)),
      startButton(new QPushButton("模式1:关键词监测", this)),
      AreaButton(new QPushButton("区域选定", this)),
      stopButton(new QPushButton("停止监测", this)),
      configButton(new QPushButton("参数配置", this)),
      timer(new QTimer(this)),
      testAlertButton(new QPushButton("测试警告", this)),
      viewHistoryButton(new QPushButton("查看历史", this)),
      continuousDetectButton(new QPushButton("模式2:屏幕变化检测", this)),
      continuousTimer(new QTimer(this)),
      isContinuousDetecting(false)
{
    std::cout << "OpenCV version: " << CV_VERSION << std::endl;
    cv::Mat testImg = cv::Mat::zeros(100, 100, CV_8UC3);
    cv::circle(testImg, cv::Point(50, 50), 30, cv::Scalar(0, 255, 0), -1);
    std::cout << "OpenCV image processing works! Size: " << testImg.size() << std::endl;

    // 设置 QLabel
    imageLabel->setAlignment(Qt::AlignCenter);
    imageLabel->setMinimumSize(120, 120);
    imageLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    imageLabel->setStyleSheet("QLabel { background-color: #222; }");

    croppedLabel->setAlignment(Qt::AlignCenter);
    croppedLabel->setMinimumSize(120, 120);
    croppedLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    croppedLabel->setStyleSheet("QLabel { background-color: #333; border: 1px solid #666; }");

    logTextEdit->setReadOnly(true);
    logTextEdit->setPlaceholderText("OCR 识别结果将显示在这里...");
    logTextEdit->setMaximumHeight(150);
    logTextEdit->setStyleSheet("QTextEdit { background-color: #eee; color: #333; font-family: Consolas, Monospace; }");

    // 连接信号槽
    connect(startButton, &QPushButton::clicked, this, &MainWindow::onStartMonitoring);
    connect(stopButton, &QPushButton::clicked, this, &MainWindow::onStopMonitoring);
    connect(configButton, &QPushButton::clicked, this, &MainWindow::openConfig);
    connect(timer, &QTimer::timeout, this, &MainWindow::newOCR);
    connect(testAlertButton, &QPushButton::clicked, this, &MainWindow::onTestAlert);
    connect(viewHistoryButton, &QPushButton::clicked, this, &MainWindow::onViewHistory);
    connect(continuousDetectButton, &QPushButton::clicked, this, &MainWindow::mode2);
    connect(continuousTimer, &QTimer::timeout, this, &MainWindow::onContinuousCapture);
    connect(AreaButton,&QPushButton::clicked,this,&MainWindow::onContinuousDetect);

    // 按钮布局：确保两个主模式按钮相邻
    QHBoxLayout *modeButtonLayout = new QHBoxLayout;
    modeButtonLayout->addWidget(startButton);
    modeButtonLayout->addWidget(continuousDetectButton);
    modeButtonLayout->addWidget(AreaButton);

    QHBoxLayout *auxButtonLayout = new QHBoxLayout;
    auxButtonLayout->addWidget(stopButton);
    auxButtonLayout->addWidget(configButton);
    auxButtonLayout->addWidget(testAlertButton);
    auxButtonLayout->addWidget(viewHistoryButton);
    auxButtonLayout->addStretch();

    QVBoxLayout *buttonLayout = new QVBoxLayout;
    buttonLayout->addLayout(modeButtonLayout);
    buttonLayout->addLayout(auxButtonLayout);

    // 主布局
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(imageLabel);
    mainLayout->addWidget(croppedLabel);
    mainLayout->addWidget(logTextEdit);
    mainLayout->addLayout(buttonLayout);

    QWidget *centralWidget = new QWidget(this);
    centralWidget->setLayout(mainLayout);
    setCentralWidget(centralWidget);

    setWindowTitle("预警系统");
    resize(600, 450);

    // 初始状态
    startButton->setEnabled(true);
    continuousDetectButton->setEnabled(true);
    stopButton->setEnabled(false);
}

MainWindow::~MainWindow()
{
    if (timer->isActive()) timer->stop();
    if (continuousTimer && continuousTimer->isActive()) continuousTimer->stop();
}

// 停止所有监测（统一入口）
void MainWindow::stopAllMonitoring()
{
    // 停止主监测
    if (timer->isActive()) {
        timer->stop();
        isMonitoringActive = false;
    }

    // 停止连续检测
    if (isContinuousDetecting) {
        continuousTimer->stop();
        isContinuousDetecting = false;
        continuousLastFrame.release();
        m_croppedLastFrame.release();
        continuousDetectButton->setText("模式2:屏幕变化检测");
    }

    // 恢复按钮状态
    continuousDetectButton->setEnabled(true);
    startButton->setEnabled(true);
    stopButton->setEnabled(false);

    logTextEdit->append("[系统] 所有监测已停止");
}

// 开始主监测
void MainWindow::onStartMonitoring()
{
    if (isContinuousDetecting) {
        stopAllMonitoring();
    }

    if (!timer->isActive()) {
        QSettings settings("MyCompany", "MonitorApp");
        int intervalSeconds = settings.value("detectionInterval", 5).toInt();
        timer->start(intervalSeconds * 1000);

        startButton->setEnabled(false);
        continuousDetectButton->setEnabled(false);
        stopButton->setEnabled(true);

        monitoringStartTime = QDateTime::currentDateTime();
        isMonitoringActive = false;
        logTextEdit->append("[主监测] 已启动");
    }
}

// 停止监测（通用）
void MainWindow::onStopMonitoring()
{
    stopAllMonitoring();
}

// 打开配置
void MainWindow::openConfig()
{
    MonitorConfig dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        // 配置已保存
    }
}

// 辅助函数：根据百分比裁剪图像（添加边界检查）
cv::Mat MainWindow::cropImageByPercent(const cv::Mat &src, int startXPercent, int widthPercent, int startYPercent, int heightPercent)
{
    if (src.empty()) {
        return cv::Mat();
    }

    // 计算像素坐标（边界检查，避免越界）
    int x = std::max(0, std::min((src.cols * startXPercent) / 100, src.cols - 1));
    int w = std::max(1, std::min((src.cols * widthPercent) / 100, src.cols - x));
    int y = std::max(0, std::min((src.rows * startYPercent) / 100, src.rows - 1));
    int h = std::max(1, std::min((src.rows * heightPercent) / 100, src.rows - y));

    // 裁剪区域（ROI）并返回深拷贝
    cv::Rect roi(x, y, w, h);
    return src(roi).clone();
}

// 接收fuzhu窗口传递的区域参数 + 启动监测
void MainWindow::onAreaParamsReceived(int startXPercent, int widthPercent, int startYPercent, int heightPercent)
{
    // 存储参数
    m_areaStartX = startXPercent;
    m_areaWidth = widthPercent;
    m_areaStartY = startYPercent;
    m_areaHeight = heightPercent;

    // 打印日志，确认参数接收
    logTextEdit->append(QString("[系统] 已接收监测区域参数："));
    logTextEdit->append(QString("→ X起始：%1%，宽度：%2%").arg(startXPercent).arg(widthPercent));
    logTextEdit->append(QString("→ Y起始：%1%，高度：%2%").arg(startYPercent).arg(heightPercent));
    logTextEdit->append("----------------------------------");

    // 重置裁剪后的上一帧
    m_croppedLastFrame.release();

    // ===== 核心：接收参数后启动连续监测 =====
    if (!isContinuousDetecting) {
        QSettings settings("MyCompany", "MonitorApp");
        int interval = settings.value("detectionInterval", 2).toInt() * 1000; // 默认2秒
        continuousTimer->start(interval);

        isContinuousDetecting = true;
        continuousDetectButton->setEnabled(false);
        stopButton->setEnabled(true);
        startButton->setEnabled(false);

        logTextEdit->append("[连续检测] 已启动，开始监测指定区域...");
    }
}

// 模式2：仅打开fuzhu窗口，不直接启动监测
void MainWindow::onContinuousDetect()
{
    // 停止检测
    stopAllMonitoring();
    // ===== 每次点击都创建新的fuzhu窗口（关闭后自动销毁，无内存泄漏） =====
    fuzhu *w1 = new fuzhu(this);
    // 窗口关闭后自动释放内存（避免内存泄漏）
    connect(w1, &fuzhu::destroyed, w1, &QObject::deleteLater);
    w1->show();
}

// 主监测：屏幕捕捉与 OCR(旧版)
void MainWindow::onCaptureScreen()
{
    QScreen *screen = QGuiApplication::primaryScreen();
    if (!screen) return;

    QPixmap pixmap = screen->grabWindow(0);
    if (pixmap.isNull()) return;

    QImage image = pixmap.toImage();
    if (image.format() != QImage::Format_RGB888 &&
        image.format() != QImage::Format_RGBA8888) {
        image = image.convertToFormat(QImage::Format_RGB888);
    }

    cv::Mat mat;
    if (image.format() == QImage::Format_RGB888) {
        mat = cv::Mat(image.height(), image.width(), CV_8UC3, (uchar*)image.bits(), image.bytesPerLine());
        cv::cvtColor(mat, mat, cv::COLOR_RGB2BGR);
    }

    if (!isMonitoringActive) {
        int elapsedSeconds = monitoringStartTime.secsTo(QDateTime::currentDateTime());
        QSettings settings("MyCompany", "MonitorApp");
        int intervalSeconds = settings.value("detectionInterval", 5).toInt();
        if (elapsedSeconds >= intervalSeconds) {
            isMonitoringActive = true;
            std::cout << "starting！" << std::endl;
        } else {
            lastFrame = mat.clone();
            return;
        }
    }

    try {
        cv::Mat prevFrame = lastFrame.clone();
        cv::Mat changedFrame = detectAndCaptureChanges(mat);
        if (!changedFrame.empty())
        {
            QSettings settings("MyCompany", "MonitorApp");
            bool isDarkMode = settings.value("darkMode", false).toBool();
            cv::Mat beforeChange = prevFrame.empty() ? changedFrame.clone() : prevFrame;
            cv::Mat afterChange = changedFrame.clone();

            cv::Mat croppedBefore = m_imageProcessor.CutPhoto(beforeChange, isDarkMode);
            cv::Mat croppedAfter = m_imageProcessor.CutPhoto(afterChange, isDarkMode);

            OcrHelper ocrHelper;
            QString textBefore = ocrHelper.recognizeText(croppedBefore);
            QString textAfter = ocrHelper.recognizeText(croppedAfter);

            logTextEdit->append(QString("[%1] 识别完成").arg(QDateTime::currentDateTime().toString("HH:mm:ss")));
            logTextEdit->append("<b>[Raw Text After]:</b>");
            logTextEdit->append(textAfter);
            logTextEdit->append("-----------------------------");

            QString newMessage = ocrHelper.extractNewMessage(textBefore, textAfter);

            if (!newMessage.isEmpty()) {
                QStringList lines = newMessage.split('\n', Qt::SkipEmptyParts);
                if (!lines.isEmpty()) {
                    QString latestOne = lines.last().trimmed();
                    ChatMessage chatMsg;
                    chatMsg.timestamp = QDateTime::currentDateTime();
                    chatMsg.content = latestOne;
                    m_chatHistory.push_back(chatMsg);
                    if (m_chatHistory.size() > 100) {
                        m_chatHistory.pop_front();
                    }
                    logTextEdit->append("<b>========== 最新消息 (Latest) ==========</b>");
                    logTextEdit->append(latestOne);
                    logTextEdit->append("<b>==================================</b>");
                }
            } else {
                logTextEdit->append("<i>(未提取到有效新内容)</i>");
            }

            QStringList alertKeywords = settings.value("keywords").toStringList();
            bool keywordFound = false;
            QString triggerReason;

            for (const QString &keyword : alertKeywords) {
                if (!newMessage.isEmpty() && newMessage.contains(keyword, Qt::CaseInsensitive)) {
                    keywordFound = true;
                    triggerReason = QString("新消息包含: %1").arg(keyword);
                    break;
                }
                int countBefore = textBefore.count(keyword, Qt::CaseInsensitive);
                int countAfter = textAfter.count(keyword, Qt::CaseInsensitive);
                if (countAfter > countBefore) {
                    keywordFound = true;
                    triggerReason = QString("关键词增量: %1 (%2->%3)").arg(keyword).arg(countBefore).arg(countAfter);
                    break;
                }
            }

            if (keywordFound) {
                logTextEdit->append(QString("<font color='red'><b>[报警触发] %1</b></font>").arg(triggerReason));
                m_imageProcessor.StartAlert();
            }

            logTextEdit->moveCursor(QTextCursor::End);

            // 显示原始变化截图
            cv::Mat displayOriginal = changedFrame.clone();
            cv::cvtColor(displayOriginal, displayOriginal, cv::COLOR_BGR2RGB);
            QImage img1((const uchar*)displayOriginal.data, displayOriginal.cols, displayOriginal.rows, displayOriginal.step, QImage::Format_RGB888);
            imageLabel->setPixmap(QPixmap::fromImage(img1).scaled(imageLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));

            // 显示裁剪区域
            if (!croppedAfter.empty()) {
                cv::Mat displayCropped = croppedAfter.clone();
                cv::cvtColor(displayCropped, displayCropped, cv::COLOR_BGR2RGB);
                QImage img2((const uchar*)displayCropped.data, displayCropped.cols, displayCropped.rows, displayCropped.step, QImage::Format_RGB888);
                croppedLabel->setPixmap(QPixmap::fromImage(img2).scaled(croppedLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
            } else {
                croppedLabel->setText("⚠️ 裁剪失败：未找到聊天区域");
            }
        }
    } catch (const cv::Exception& e) {
        std::cerr << "❌ OpenCV Exception: " << e.what() << std::endl;
    } catch (...) {
        std::cerr << "❌ Unknown Exception!" << std::endl;
    }

    lastFrame = mat.clone();
}

cv::Mat MainWindow::detectAndCaptureChanges(const cv::Mat &currentFrame)
{
    if (lastFrame.empty()) {
        lastFrame = currentFrame.clone();
        return cv::Mat();
    }

    cv::Mat diff;
    cv::absdiff(lastFrame, currentFrame, diff);

    cv::Mat grayDiff;
    cv::cvtColor(diff, grayDiff, cv::COLOR_BGR2GRAY);

    cv::Mat binary;
    cv::threshold(grayDiff, binary, 25, 255, cv::THRESH_BINARY);

    cv::Mat mask = cv::Mat::ones(binary.size(), CV_8UC1) * 255;
    int taskbarHeight = 50;
    cv::rectangle(mask, cv::Point(0, binary.rows - taskbarHeight),
                  cv::Point(binary.cols, binary.rows), cv::Scalar(0), -1);
    cv::bitwise_and(binary, mask, binary);

    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3));
    cv::morphologyEx(binary, binary, cv::MORPH_OPEN, kernel);

    int changedPixels = cv::countNonZero(binary);
    int totalPixels = binary.rows * binary.cols - (binary.rows * taskbarHeight);
    double changeRatio = static_cast<double>(changedPixels) / totalPixels;
    const double MIN_CHANGE_RATIO = 0.003;

    if (changeRatio > MIN_CHANGE_RATIO) {
        std::cout << "监测到消息！变化比例: " << (changeRatio * 100) << "%" << std::endl;
        lastFrame = currentFrame.clone();
        return currentFrame.clone();
    }

    static int frameCount = 0;
    if (++frameCount % 20 == 0) {
        lastFrame = currentFrame.clone();
        frameCount = 0;
    }

    return cv::Mat();
}

void MainWindow::onTestAlert()
{
    m_imageProcessor.StartAlert();
}

void MainWindow::onViewHistory()
{
    logTextEdit->append("<b>===== 聊天历史 =====</b>");
    for (const auto &msg : m_chatHistory) {
        logTextEdit->append(QString("[%1] %2").arg(msg.timestamp.toString("HH:mm:ss")).arg(msg.content));
    }
    logTextEdit->append("<b>====================</b>");
    logTextEdit->moveCursor(QTextCursor::End);
}

// 连续检测的捕获逻辑：限定区域检测变化
void MainWindow::onContinuousCapture()
{
    QScreen *screen = QGuiApplication::primaryScreen();
    if (!screen) {
        logTextEdit->append("<font color='orange'><b>[连续检测] 无法获取屏幕！</b></font>");
        return;
    }

    // 1. 截取整个屏幕
    QPixmap pixmap = screen->grabWindow(0);
    if (pixmap.isNull()) {
        logTextEdit->append("<font color='orange'><b>[连续检测] 截图失败！</b></font>");
        return;
    }

    // 2. 转换为OpenCV Mat（BGR格式）
    QImage image = pixmap.toImage().convertToFormat(QImage::Format_RGB888);
    cv::Mat currentFrame(image.height(), image.width(), CV_8UC3, (uchar*)image.bits(), image.bytesPerLine());
    cv::cvtColor(currentFrame, currentFrame, cv::COLOR_RGB2BGR);

    // 3. 根据接收的参数裁剪监测区域
    cv::Mat croppedFrame = cropImageByPercent(currentFrame, m_areaStartX, m_areaWidth, m_areaStartY, m_areaHeight);
    if (croppedFrame.empty()) {
        logTextEdit->append("<font color='orange'><b>[连续检测] 裁剪监测区域失败！</b></font>");
        return;
    }

    // 4. 对比裁剪后的上一帧，计算变化比例
    double changeRatio = 0.0;
    if (!m_croppedLastFrame.empty()) {
        // 计算两帧的差异
        cv::Mat diff;
        cv::absdiff(m_croppedLastFrame, croppedFrame, diff);

        // 转灰度图→二值化→形态学去噪
        cv::Mat grayDiff, binary;
        cv::cvtColor(diff, grayDiff, cv::COLOR_BGR2GRAY);
        cv::threshold(grayDiff, binary, 25, 255, cv::THRESH_BINARY);
        cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3));
        cv::morphologyEx(binary, binary, cv::MORPH_OPEN, kernel);

        // 计算变化比例
        int changedPixels = cv::countNonZero(binary);
        int totalPixels = binary.total();
        changeRatio = totalPixels > 0 ? static_cast<double>(changedPixels) / totalPixels : 0.0;
    }

    // 5. 判断是否触发预警（阈值：0.3%）
    const double MIN_CHANGE_RATIO = 0.003;
    if (changeRatio > MIN_CHANGE_RATIO) {
        logTextEdit->append(QString("<font color='red'><b>[连续检测] 监测区域发现变化！变化比例: %1%</b></font>")
                            .arg(changeRatio * 100, 0, 'f', 2));
        m_imageProcessor.StartAlert();
    }

    // 6. 更新裁剪后的上一帧
    m_croppedLastFrame = croppedFrame.clone();

    // 7. 界面显示：原始截图（标记监测区域）
    cv::Mat displayOriginal = currentFrame.clone();
    int x = (currentFrame.cols * m_areaStartX) / 100;
    int w = (currentFrame.cols * m_areaWidth) / 100;
    int y = (currentFrame.rows * m_areaStartY) / 100;
    int h = (currentFrame.rows * m_areaHeight) / 100;
    cv::rectangle(displayOriginal, cv::Point(x, y), cv::Point(x + w, y + h), cv::Scalar(0, 0, 255), 2);
    cv::cvtColor(displayOriginal, displayOriginal, cv::COLOR_BGR2RGB);
    QImage imgOriginal(displayOriginal.data, displayOriginal.cols, displayOriginal.rows, displayOriginal.step, QImage::Format_RGB888);
    imageLabel->setPixmap(QPixmap::fromImage(imgOriginal).scaled(imageLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));

    // 8. 界面显示：裁剪后的监测区域
    cv::Mat displayCropped = croppedFrame.clone();
    cv::cvtColor(displayCropped, displayCropped, cv::COLOR_BGR2RGB);
    QImage imgCropped(displayCropped.data, displayCropped.cols, displayCropped.rows, displayCropped.step, QImage::Format_RGB888);
    croppedLabel->setPixmap(QPixmap::fromImage(imgCropped).scaled(croppedLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
}
void MainWindow::mode2()
{
    DisasterQuery q;
    q.severityMin = 0;
    q.orderBy = DisasterQuery::OrderBy::SeverityDesc;

    QList<DisasterRecord> records;
    QString err;
    // 注意：传入 &records 接收查询结果
    if (DisasterDao::queryDisasters(q, &records, &err)) {
        qDebug() << "查询成功，找到" << records.size() << "条记录";
        for (const auto& record : records) {
            qDebug() << "ID:" << record.id
                     << "类型:" << record.disasterType
                     << "严重等级:" << record.severity
                     << "灾害内容:"<<record.content
                     << "系统报警时间:"<<record.systemAlarmAt
                     << "发生时间:"<<record.occurredAt;
        }
    } else {
        qDebug() << "查询失败:" << err;
    }
}
void MainWindow::mode1(){
    // 1. 先停止当前可能正在运行的连续检测（避免重复启动）
       if (isContinuousDetecting) {
           stopAllMonitoring();
           return;
       }

       // 2. 获取配置实例，检查检测区域+卡片区域的完整配置
       QSettings settings("MyCompany", "MonitorApp");

       // 检测区域配置项（4个参数必须都存在）
       bool hasDetectConfig = settings.contains("detectStartX") && settings.contains("detectWidth") &&
                              settings.contains("detectStartY") && settings.contains("detectHeight");
       // 卡片区域配置项（4个参数必须都存在）
       bool hasCardConfig = settings.contains("cardStartX") && settings.contains("cardWidth") &&
                            settings.contains("cardStartY") && settings.contains("cardHeight");

       // 3. 配置不完整则弹窗提示
       if (!hasDetectConfig || !hasCardConfig) {
           QMessageBox::warning(this, "配置缺失", "需要先进行区域选定！\n请点击「区域选定」按钮完成检测区域和卡片区域的框选。", QMessageBox::Ok);
           logTextEdit->append("<font color='red'><b>[模式1] 启动失败：未检测到完整的区域配置，请先完成区域选定</b></font>");
           return;
       }

       // 4. 配置完整，读取参数并启动模式1监测
       // 读取检测间隔（默认5秒）
       int intervalSeconds = settings.value("detectionInterval", 5).toInt();

       // 启动模式1定时器
       if (!timer->isActive()) {
           timer->start(intervalSeconds * 1000);

           // 更新状态和按钮
           isMonitoringActive = false; // 初始标记为未激活，等待首次计时
           monitoringStartTime = QDateTime::currentDateTime();
           startButton->setEnabled(false);
           continuousDetectButton->setEnabled(false);
           stopButton->setEnabled(true);

           // 日志记录
           logTextEdit->append(QString("[模式1] 启动成功！监测间隔：%1秒").arg(intervalSeconds));
           logTextEdit->append("[模式1] 开始监测检测区域变化，变化时将展示卡片区域截图");
       }
}
// 新版OCR：仅检测红框（检测区域）变化，对比变化前后OCR结果提取新增文字
void MainWindow::newOCR()
{
    QScreen *screen = QGuiApplication::primaryScreen();
    if (!screen) {
        logTextEdit->append("<font color='orange'><b>[模式1] 无法获取屏幕！</b></font>");
        return;
    }

    // 1. 截取整个屏幕
    QPixmap pixmap = screen->grabWindow(0);
    if (pixmap.isNull()) {
        logTextEdit->append("<font color='orange'><b>[模式1] 截图失败！</b></font>");
        return;
    }

    // 2. 转换为OpenCV Mat（BGR格式）
    QImage image = pixmap.toImage().convertToFormat(QImage::Format_RGB888);
    cv::Mat currentFrame(image.height(), image.width(), CV_8UC3, (uchar*)image.bits(), image.bytesPerLine());
    cv::cvtColor(currentFrame, currentFrame, cv::COLOR_RGB2BGR);

    // 3. 初始启动等待：确保首次计时完成后再检测变化
    if (!isMonitoringActive) {
        int elapsedSeconds = monitoringStartTime.secsTo(QDateTime::currentDateTime());
        QSettings settings("MyCompany", "MonitorApp");
        int intervalSeconds = settings.value("detectionInterval", 5).toInt();
        if (elapsedSeconds >= intervalSeconds) {
            isMonitoringActive = true;
            // 首次启动时初始化红框区域的基准帧 + 基准OCR文本
            int detectStartX = settings.value("detectStartX").toInt();
            int detectWidth = settings.value("detectWidth").toInt();
            int detectStartY = settings.value("detectStartY").toInt();
            int detectHeight = settings.value("detectHeight").toInt();
            m_croppedLastFrame = cropImageByPercent(currentFrame, detectStartX, detectWidth, detectStartY, detectHeight);

            // 对初始基准帧做OCR，保存基准文本
            if (!m_croppedLastFrame.empty()) {
                OcrHelper ocrHelper;
                m_lastOcrText = ocrHelper.recognizeText(m_croppedLastFrame);
                logTextEdit->append(QString("[模式1] 首次基准OCR完成，初始文本长度：%1字符").arg(m_lastOcrText.length()));
            }

            logTextEdit->append("[模式1] 首次计时完成，开始检测红框区域变化...");
        } else {
            return;
        }
    }

    try {
        // 4. 读取配置：仅红框（检测区域）参数
        QSettings settings("MyCompany", "MonitorApp");
        int detectStartX = settings.value("detectStartX").toInt();
        int detectWidth = settings.value("detectWidth").toInt();
        int detectStartY = settings.value("detectStartY").toInt();
        int detectHeight = settings.value("detectHeight").toInt();

        // 5. 核心：检测红框区域的变化
        // 5.1 裁剪当前帧的红框区域
        cv::Mat croppedFrame = cropImageByPercent(currentFrame, detectStartX, detectWidth, detectStartY, detectHeight);
        if (croppedFrame.empty()) {
            logTextEdit->append("<font color='orange'><b>[模式1] 红框检测区域裁剪失败！</b></font>");
            return;
        }

        // 5.2 对比红框区域的上一帧，计算变化比例
        double changeRatio = 0.0;
        if (!m_croppedLastFrame.empty()) {
            // 计算两帧差异
            cv::Mat diff;
            cv::absdiff(m_croppedLastFrame, croppedFrame, diff);

            // 转灰度→二值化→形态学去噪
            cv::Mat grayDiff, binary;
            cv::cvtColor(diff, grayDiff, cv::COLOR_BGR2GRAY);
            cv::threshold(grayDiff, binary, 25, 255, cv::THRESH_BINARY);
            cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3));
            cv::morphologyEx(binary, binary, cv::MORPH_OPEN, kernel);

            // 计算红框区域变化比例
            int changedPixels = cv::countNonZero(binary);
            int totalPixels = binary.total();
            changeRatio = totalPixels > 0 ? static_cast<double>(changedPixels) / totalPixels : 0.0;
        }

        // 5.3 判断红框区域是否变化（阈值0.3%）
        const double MIN_CHANGE_RATIO = 0.003;
        if (changeRatio > MIN_CHANGE_RATIO) {
            // 红框区域变化，触发OCR对比逻辑
            logTextEdit->append(QString("<font color='red'><b>[模式1] 红框区域发现变化！变化比例: %1%</b></font>")
                                .arg(changeRatio * 100, 0, 'f', 2));

            // 6. OCR对比：变化前（上一帧）vs 变化后（当前帧）
            OcrHelper ocrHelper;
            const QString currentOcrText = ocrHelper.recognizeText(croppedFrame);
            const QString lastOcrText = m_lastOcrText;

            // 仅打印：旧消息（变化前）、新消息（变化后）
            qDebug() << "旧消息：" << lastOcrText;
            qDebug() << "新消息：" << currentOcrText;

            logTextEdit->append(QString("[%1] [模式1] 变化前OCR文本：").arg(QDateTime::currentDateTime().toString("HH:mm:ss")));
            logTextEdit->append(lastOcrText.isEmpty() ? QStringLiteral("(空)") : lastOcrText);
            logTextEdit->append(QString("[%1] [模式1] 变化后OCR文本：").arg(QDateTime::currentDateTime().toString("HH:mm:ss")));
            logTextEdit->append(currentOcrText.isEmpty() ? QStringLiteral("(空)") : currentOcrText);

            // 7. 提取新增文字,下面这个进行一个AI接入，让其自动判断是否有灾害消息，有的话就进行一个格式整理然后插入
            QString newText = extractNewText(lastOcrText, currentOcrText);

            DisasterRecord rec;

            rec.content = newText;//
            rec.disasterType = "地震";
            rec.location = "生活园B栋";
            rec.severity = 3;

            qint64 id;
            QString err;
              // 成功后 id 会被赋值
              if (DisasterDao::createDisaster(rec, &id, &err)) {
                  qDebug() << "灾害记录创建成功，ID:" << id;
              } else {
                  qDebug() << "创建失败:" << err;
              }

            // 仅打印：筛选出的新增消息
            qDebug() << "筛选出的新增消息：" << newText;

            if (!newText.isEmpty()) {
                logTextEdit->append(QString("<font color='red'><b>[模式1] 检测到新增文字（新微信消息）：</b></font>"));
                logTextEdit->append(newText);

                // 触发报警
                QString type = settings.value("eventType").toString();
                const QString trimmedType = type.trimmed();
                if (!trimmedType.isEmpty()) {
                    QString compactType = trimmedType;
                    compactType.remove(' ').remove('\n').remove('\r').remove('\t');
                    QString compactNewText = newText;
                    compactNewText.remove(' ').remove('\n').remove('\r').remove('\t');

                    if (compactNewText.contains(compactType, Qt::CaseInsensitive)) {
                        logTextEdit->append(QString("<font color='red'><b>[报警触发] 新增文字命中监测类型：%1</b></font>").arg(type));
                        m_imageProcessor.StartAlert();
                    }
                }

                // 保存新增文字到聊天记录（保留原有逻辑）
                const QStringList lines = newText.split('\n', Qt::SkipEmptyParts);
                if (!lines.isEmpty()) {
                    const QString latestLine = lines.last().trimmed();
                    ChatMessage chatMsg;
                    chatMsg.timestamp = QDateTime::currentDateTime();
                    chatMsg.content = latestLine;
                    m_chatHistory.push_back(chatMsg);
                    if (m_chatHistory.size() > 100) {
                        m_chatHistory.pop_front();
                    }
                }
            } else {
                logTextEdit->append("[模式1] 未检测到新增文字（仅格式/排版变化）");
            }

            // 8. 界面展示：仅展示红框区域的截图（标记红框）
            cv::Mat displayOriginal = currentFrame.clone();
            int redX = (displayOriginal.cols * detectStartX) / 100;
            int redW = (displayOriginal.cols * detectWidth) / 100;
            int redY = (displayOriginal.rows * detectStartY) / 100;
            int redH = (displayOriginal.rows * detectHeight) / 100;
            cv::rectangle(displayOriginal, cv::Point(redX, redY), cv::Point(redX+redW, redY+redH), cv::Scalar(0,0,255), 2);

            cv::cvtColor(displayOriginal, displayOriginal, cv::COLOR_BGR2RGB);
            QImage img1((const uchar*)displayOriginal.data, displayOriginal.cols, displayOriginal.rows, displayOriginal.step, QImage::Format_RGB888);
            imageLabel->setPixmap(QPixmap::fromImage(img1).scaled(imageLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));

            // 8.2 单独展示红框区域截图
            cv::Mat displayRedFrame = croppedFrame.clone();
            cv::cvtColor(displayRedFrame, displayRedFrame, cv::COLOR_BGR2RGB);
            QImage img2((const uchar*)displayRedFrame.data, displayRedFrame.cols, displayRedFrame.rows, displayRedFrame.step, QImage::Format_RGB888);
            croppedLabel->setPixmap(QPixmap::fromImage(img2).scaled(croppedLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
            croppedLabel->setText(""); // 清空提示

            logTextEdit->append(QString("[%1] [模式1] 红框区域变化 → 展示红框区域截图").arg(QDateTime::currentDateTime().toString("HH:mm:ss")));

            // 9. 更新基准：保存当前帧和当前OCR文本为下次对比的基准
            m_croppedLastFrame = croppedFrame.clone();
            m_lastOcrText = currentOcrText;
        } else {
            // 无变化时仅更新基准帧（不更新OCR，避免无意义的重复识别）
            m_croppedLastFrame = croppedFrame.clone();
        }

    } catch (const cv::Exception& e) {
        std::cerr << "❌ OpenCV Exception: " << e.what() << std::endl;
        logTextEdit->append(QString("<font color='red'><b>[模式1] 异常：%1</b></font>").arg(e.what()));
    } catch (...) {
        std::cerr << "❌ Unknown Exception!" << std::endl;
        logTextEdit->append("<font color='red'><b>[模式1] 未知异常！</b></font>");
    }
}

// 辅助函数：提取变化后独有的文字（新消息）
QString MainWindow::extractNewText(const QString& oldText, const QString& newText)
{
    // 预处理：去除空白字符，统一格式
    auto preprocess = [](const QString& text) {
        QString processed = text;
        processed.remove(' ').remove('\n').remove('\r').remove('\t');
        return processed;
    };

    QString oldProcessed = preprocess(oldText);
    QString newProcessed = preprocess(newText);

    // 如果无变化，直接返回空
    if (oldProcessed == newProcessed) {
        return "";
    }

    // 方法1：按行对比（适合聊天消息按行展示的场景）
    QStringList oldLines = oldText.split('\n', Qt::SkipEmptyParts);
    QStringList newLines = newText.split('\n', Qt::SkipEmptyParts);
    QStringList newLinesOnly;

    for (const QString& line : newLines) {
        QString lineTrimmed = line.trimmed();
        bool isNew = true;
        for (const QString& oldLine : oldLines) {
            if (oldLine.trimmed() == lineTrimmed) {
                isNew = false;
                break;
            }
        }
        if (isNew && !lineTrimmed.isEmpty()) {
            newLinesOnly.append(lineTrimmed);
        }
    }

    // 方法2：字符级对比（备用，防止行分割问题）
    if (newLinesOnly.isEmpty()) {
        QString result;
        for (int i = 0; i < newText.length(); ++i) {
            QChar c = newText.at(i);
            if (!oldText.contains(c) || oldText.count(c) < newText.count(c)) {
                result.append(c);
            }
        }
        return result.trimmed();
    }

    return newLinesOnly.join("\n");
}
