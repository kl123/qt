#include "home.h"
// 新增：补充Qt5必需的头文件
#include <QStyle>
#include <QPainter>
#include <QChart>
#include <QMenuBar>

Home::Home(QWidget *parent) : QWidget(parent)
{
    // 初始化模拟数据
    initMockDisasterData();
    // 构建首页UI
    initHomeUI();

    // 初始化定时器：2秒刷新一次数据（模拟OCR实时监测）
    m_refreshTimer = new QTimer(this);
    m_refreshTimer->setInterval(2000);
    connect(m_refreshTimer, &QTimer::timeout, this, &Home::refreshMockOcrData);\
    this->resize(1200, 900);
}

Home::~Home()
{
}

void Home::initHomeUI()
{
    // ========== 1. 首页基础样式 ==========
    this->setStyleSheet(R"(
        QWidget {
            background-color: #f0f2f5;
            font-family: "Microsoft YaHei", "SimHei", sans-serif;
        }
        QFrame {
            background-color: white;
            border-radius: 12px;
            box-shadow: 0 2px 10px rgba(0, 0, 0, 0.06);
        }
        QLabel {
            color: #303133;
        }
        QPushButton {
            border: none;
            border-radius: 8px;
            padding: 8px 20px;
            font-size: 14px;
            font-weight: 500;
        }
        QPushButton#StartBtn {
            background-color: #409eff;
            color: white;
        }
        QPushButton#StartBtn:hover {
            background-color: #66b1ff;
        }
        QPushButton#StopBtn {
            background-color: #f56c6c;
            color: white;
        }
        QPushButton#StopBtn:hover {
            background-color: #f78989;
        }
        QTextEdit {
            border: 1px solid #e6e6e6;
            border-radius: 8px;
            padding: 10px;
            font-size: 12px;
            background-color: white;
        }
    )");

    // ========== 2. 主布局 ==========
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(25, 25, 25, 25);
    mainLayout->setSpacing(20);

    // ========== 新增：创建菜单栏 ==========
    QMenuBar *menuBar = new QMenuBar(this);
    mainLayout->setMenuBar(menuBar); // 将菜单栏绑定到主布局

    // 1.1 创建“文件”菜单（带Qt自带图标）
    QMenu *fileMenu = menuBar->addMenu("文件");
    // 数据导出（自带保存图标）
    QAction *exportAction = new QAction(QIcon::fromTheme("document-save", QIcon(":/qt-project.org/styles/commonstyle/images/standardbutton-save-16.png")), "数据导出", this);
    fileMenu->addAction(exportAction);
    // 清空日志（自带清除图标）
    QAction *clearLogAction = new QAction(QIcon::fromTheme("edit-clear", QIcon(":/qt-project.org/styles/commonstyle/images/standardbutton-clear-16.png")), "清空日志", this);
    fileMenu->addAction(clearLogAction);
    fileMenu->addSeparator(); // 分隔线
    // 退出（自带退出图标）
    QAction *exitAction = new QAction(QIcon::fromTheme("application-exit", QIcon(":/qt-project.org/styles/commonstyle/images/standardbutton-close-16.png")), "退出系统", this);
    fileMenu->addAction(exitAction);

    // 1.2 创建“系统”菜单（包含系统配置）
    QMenu *sysMenu = menuBar->addMenu("系统");
    // 系统配置（自带齿轮/设置图标）
    QAction *configAction = new QAction(QIcon::fromTheme("preferences-system", QIcon(":/qt-project.org/styles/commonstyle/images/standardbutton-configure-16.png")), "系统配置", this);
    sysMenu->addAction(configAction);
    // 监测参数（自带参数图标）
    QAction *paramAction = new QAction(QIcon::fromTheme("system-run", QIcon(":/qt-project.org/styles/commonstyle/images/standardbutton-ok-16.png")), "监测参数", this);
    sysMenu->addAction(paramAction);

    // 1.3 创建“帮助”菜单
    QMenu *helpMenu = menuBar->addMenu("帮助");
    // 使用帮助（自带帮助图标）
    QAction *helpAction = new QAction(QIcon::fromTheme("help-browser", QIcon(":/qt-project.org/styles/commonstyle/images/standardbutton-help-16.png")), "使用帮助", this);
    helpMenu->addAction(helpAction);
    // 关于系统（自带关于图标）
    QAction *aboutAction = new QAction(QIcon::fromTheme("help-about", QIcon(":/qt-project.org/styles/commonstyle/images/standardbutton-about-16.png")), "关于系统", this);
    helpMenu->addAction(aboutAction);

    // ========== 3. 顶部标题栏 ==========
    QFrame *topBarFrame = new QFrame(this);
    QHBoxLayout *topBarLayout = new QHBoxLayout(topBarFrame);
    topBarLayout->setContentsMargins(20, 15, 20, 15);

    // 系统标题
    QLabel *titleLabel = new QLabel("灾害讯息监测系统 - 首页", this);
    titleLabel->setStyleSheet("font-size: 24px; font-weight: bold; color: #202124;");

    // 监测状态标签
    QLabel *statusLabel = new QLabel("监测状态：未运行", this);
    statusLabel->setStyleSheet("font-size: 14px; color: #f56c6c;");

    // 启停按钮
    QPushButton *controlBtn = new QPushButton("启动监测", this);
    controlBtn->setObjectName("StartBtn");
    connect(controlBtn, &QPushButton::clicked, this, [=]() {
        m_isMonitoring = !m_isMonitoring;
        if (m_isMonitoring) {
            controlBtn->setText("停止监测");
            controlBtn->setObjectName("StopBtn");
            statusLabel->setText("监测状态：运行中");
            statusLabel->setStyleSheet("font-size: 14px; color: #67c23a;");
            m_refreshTimer->start();
            addMonitorLog("OCR监测已启动，开始统计灾害讯息...");
        } else {
            controlBtn->setText("启动监测");
            controlBtn->setObjectName("StartBtn");
            statusLabel->setText("监测状态：未运行");
            statusLabel->setStyleSheet("font-size: 14px; color: #f56c6c;");
            m_refreshTimer->stop();
            addMonitorLog("OCR监测已停止，数据统计暂停");
        }
        // 刷新按钮样式（Qt5必须手动触发）
        controlBtn->style()->unpolish(controlBtn);
        controlBtn->style()->polish(controlBtn);
    });

    topBarLayout->addWidget(titleLabel);
    topBarLayout->addStretch();
    topBarLayout->addWidget(statusLabel);
    topBarLayout->addSpacing(20);
    topBarLayout->addWidget(controlBtn);
    mainLayout->addWidget(topBarFrame);

    // ========== 4. 统计卡片区域（3列2行） ==========
    QFrame *statCardsFrame = new QFrame(this);
    QGridLayout *statCardsLayout = new QGridLayout(statCardsFrame);
    statCardsLayout->setContentsMargins(20, 20, 20, 20);
    statCardsLayout->setSpacing(18);

    // 卡片样式配置：图标+颜色
    QStringList cardIcons = {"🔥", "🚒", "🤝", "⚠️", "🛡️", "📢"};
    QStringList cardColors = {"#f56c6c", "#e6a23c", "#409eff", "#67c23a", "#909399", "#f09a9d"};
    for (int i = 0; i < 6; i++) {
        QFrame *card = createStatCard(m_disasterNames[i], cardIcons[i], m_disasterCount[i], cardColors[i]);
        m_statValueLabels[i] = card->findChild<QLabel*>("ValueLabel");
        statCardsLayout->addWidget(card, i / 3, i % 3);
    }
    mainLayout->addWidget(statCardsFrame);

    // ========== 5. 图表区域（饼图+柱状图 左右分栏） ==========
    QFrame *chartsFrame = new QFrame(this);
    QHBoxLayout *chartsLayout = new QHBoxLayout(chartsFrame);
    chartsLayout->setContentsMargins(20, 20, 20, 20);
    chartsLayout->setSpacing(20);

    // ========== 5.1 占比饼图 ==========
    m_pieSeries = new QPieSeries(this);
    for (int i = 0; i < 6; i++) {
        QPieSlice *slice = m_pieSeries->append(m_disasterNames[i], m_disasterCount[i]);
        slice->setColor(QColor(cardColors[i]));
        slice->setLabelVisible(true);
        slice->setLabelColor(Qt::white);
        slice->setLabelFont(QFont("Microsoft YaHei", 10, QFont::Medium));
    }
    QChart *pieChart = new QChart();
    pieChart->addSeries(m_pieSeries);
    pieChart->setTitle("灾害类型占比统计");
    pieChart->setTitleFont(QFont("Microsoft YaHei", 14, QFont::Bold));
    pieChart->legend()->setAlignment(Qt::AlignRight);
    pieChart->setBackgroundRoundness(8);

    QChartView *pieChartView = new QChartView(pieChart, this);
    pieChartView->setRenderHint(QPainter::Antialiasing);
    pieChartView->setMinimumHeight(300);

    // ========== 5.2 数量柱状图 ==========
    m_barSeries = new QBarSeries(this);
    QBarSet *barSet = new QBarSet("累计数量", this);
    for (int i = 0; i < 6; i++) {
        *barSet << m_disasterCount[i];
    }
    m_barSeries->append(barSet);

    QChart *barChart = new QChart();
    barChart->addSeries(m_barSeries);
    barChart->setTitle("灾害类型数量统计");
    barChart->setTitleFont(QFont("Microsoft YaHei", 14, QFont::Bold));
    barChart->setBackgroundRoundness(8);

    // X轴配置
    QCategoryAxis *xAxis = new QCategoryAxis(this);
    for (int i = 0; i < 6; i++) {
        xAxis->append(m_disasterNames[i], i);
    }
    barChart->createDefaultAxes();
    barChart->setAxisX(xAxis, m_barSeries);
    barChart->axisY()->setTitleText("累计条数");
    barChart->legend()->hide();

    QChartView *barChartView = new QChartView(barChart, this);
    barChartView->setRenderHint(QPainter::Antialiasing);
    barChartView->setMinimumHeight(300);

    chartsLayout->addWidget(pieChartView, 1);
    chartsLayout->addWidget(barChartView, 1);
    mainLayout->addWidget(chartsFrame);

    // ========== 6. 实时日志区域 ==========
    QFrame *logFrame = new QFrame(this);
    QVBoxLayout *logLayout = new QVBoxLayout(logFrame);
    logLayout->setContentsMargins(20, 15, 20, 15);

    QLabel *logTitleLabel = new QLabel("实时监测日志", this);
    logTitleLabel->setStyleSheet("font-size: 16px; font-weight: bold; margin-bottom: 10px;");

    m_logTextEdit = new QTextEdit(this);
    m_logTextEdit->setReadOnly(true);
    m_logTextEdit->setMaximumHeight(160);
    mainLayout->addWidget(logFrame);

    logLayout->addWidget(logTitleLabel);
    logLayout->addWidget(m_logTextEdit);

    // 初始日志
    addMonitorLog("首页已加载完成，等待启动监测...");
}

void Home::initMockDisasterData()
{
    // 初始化模拟数据：随机生成1-20的初始值（替代OCR首次加载数据）
    for (int i = 0; i < 6; i++) {
        m_disasterCount[i] = QRandomGenerator::global()->bounded(1, 20);
    }
}

QFrame *Home::createStatCard(const QString &title, const QString &icon, int value, const QString &bgColor)
{
    QFrame *card = new QFrame(this);
    QVBoxLayout *cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(15, 15, 15, 15);
    cardLayout->setSpacing(12);

    // 卡片头部：图标+标题
    QHBoxLayout *cardHeaderLayout = new QHBoxLayout();
    QLabel *iconLabel = new QLabel(icon, this);
    iconLabel->setStyleSheet(QString("font-size: 26px; color: %1;").arg(bgColor));

    QLabel *titleLabel = new QLabel(title, this);
    titleLabel->setStyleSheet("font-size: 16px; font-weight: 500;");

    cardHeaderLayout->addWidget(iconLabel);
    cardHeaderLayout->addWidget(titleLabel);
    cardHeaderLayout->addStretch();

    // 卡片数值
    QLabel *valueLabel = new QLabel(QString::number(value), this);
    valueLabel->setObjectName("ValueLabel");
    valueLabel->setStyleSheet(QString("font-size: 36px; font-weight: bold; color: %1;").arg(bgColor));
    valueLabel->setAlignment(Qt::AlignCenter);

    cardLayout->addLayout(cardHeaderLayout);
    cardLayout->addWidget(valueLabel);

    return card;
}

void Home::refreshMockOcrData()
{
    // 模拟OCR监测：随机更新数据（±0~3，确保数值≥0）
    for (int i = 0; i < 6; i++) {
        int change = QRandomGenerator::global()->bounded(-1, 4);
        m_disasterCount[i] = qMax(0, m_disasterCount[i] + change);

        // 随机触发告警日志（30%概率）
        if (change > 1 && QRandomGenerator::global()->bounded(10) > 6) {
            addMonitorLog(QString("监测到新的「%1」讯息，当前累计：%2条")
                          .arg(m_disasterNames[i]).arg(m_disasterCount[i]));
        }
    }

    // 更新UI组件
    updateStatCards();
    updateCharts();
}

void Home::updateStatCards()
{
    // 更新6个统计卡片的数值
    for (int i = 0; i < 6; i++) {
        m_statValueLabels[i]->setText(QString::number(m_disasterCount[i]));
    }
}

void Home::updateCharts()
{
    // 更新饼图
    m_pieSeries->clear();
    QStringList cardColors = {"#f56c6c", "#e6a23c", "#409eff", "#67c23a", "#909399", "#f09a9d"};
    for (int i = 0; i < 6; i++) {
        QPieSlice *slice = m_pieSeries->append(m_disasterNames[i], m_disasterCount[i]);
        slice->setColor(QColor(cardColors[i]));
        slice->setLabelVisible(true);
        slice->setLabelColor(Qt::white);
    }

    // 更新柱状图
    m_barSeries->clear();
    QBarSet *barSet = new QBarSet("累计数量", this);
    for (int i = 0; i < 6; i++) {
        *barSet << m_disasterCount[i];
    }
    m_barSeries->append(barSet);
}

void Home::addMonitorLog(const QString &logContent)
{
    // 添加带时间戳的日志
    QString timeStr = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
    m_logTextEdit->append(QString("[%1] %2").arg(timeStr).arg(logContent));
}
