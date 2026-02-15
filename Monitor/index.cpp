#include "index.h"
#include "mainwindow.h"
#include <QDesktopServices>
#include <QUrl>

#include "monitorconfig.h"

index::index(QWidget *parent) : QMainWindow(parent)
{
    setWindowTitle("灾害监测系统");
    setMinimumSize(1440, 810);

    centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(20);

    setupTopNav();
    setupLeftPanel();
    setupRightPanel();

    contentLayout = new QHBoxLayout();
    contentLayout->addLayout(leftPanelLayout, 1);
    contentLayout->addWidget(rightPanel, 4);
    mainLayout->addLayout(contentLayout);

    applyGlobalStyle();
}

index::~index()
{
}

void index::setupTopNav()
{
    topNavLayout = new QHBoxLayout();
    topNavLayout->setSpacing(8); // 大幅减小导航区域整体间距，让Logo和按钮更靠近
    topNavLayout->setAlignment(Qt::AlignCenter);

    // 创建Logo区域（红框位置）
    logoFrame = new QFrame();
    logoFrame->setFrameShape(QFrame::NoFrame); // 移除边框
    QHBoxLayout *logoLayout = new QHBoxLayout(logoFrame);
    logoLayout->setContentsMargins(15, 15, 15, 15);
    logoLayout->setSpacing(15); // 减小Logo和文字间距

    // Logo图片
    labLogo = new QLabel();
    labLogo->setPixmap(QPixmap(":/sound/newLogo.jpg").scaled(120, 80, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    labLogo->setAlignment(Qt::AlignCenter);

    // 系统名称（增大字体）
    labSystemName = new QLabel("灾警系统");
    labSystemName->setFont(QFont("Microsoft YaHei", 28, QFont::Bold));
    labSystemName->setAlignment(Qt::AlignCenter);

    logoLayout->addWidget(labLogo);
    logoLayout->addWidget(labSystemName);

    topNavLayout->addWidget(logoFrame, 0, Qt::AlignLeft);

    // 顶部导航按钮（增大字体）
    btnRealTime = new QPushButton("▶ 实时概览");
    btnFireAlarm = new QPushButton("▶ 火警分析");
    btnElecFire = new QPushButton("▶ 电气火灾");
    btnNational = new QPushButton("▶ 全国总览");
    btnConfig = new QPushButton("⚙ 系统设置");

    // 连接信号槽
    connect(btnRealTime, &QPushButton::clicked, [=](){
        MainWindow *w = new MainWindow();
        w->setAttribute(Qt::WA_DeleteOnClose);
        w->show();
    });

    connect(btnConfig, &QPushButton::clicked, [=](){
        MonitorConfig dialog(this);
        dialog.exec();
    });

    QList<QPushButton*> navBtns = {btnRealTime, btnFireAlarm, btnElecFire, btnNational, btnConfig};
    for (auto btn : navBtns) {
        btn->setMinimumSize(160, 50); // 减小按钮宽度，更紧凑
        btn->setFont(QFont("Microsoft YaHei", 14, QFont::Bold)); // 增大按钮字体
        topNavLayout->addWidget(btn, 1, Qt::AlignCenter); // 调整为1，让按钮更饱满
    }

    mainLayout->addLayout(topNavLayout);
}

void index::setupLeftPanel()
{
    // 左侧总布局
    leftPanelLayout = new QVBoxLayout();
    leftPanelLayout->setContentsMargins(0, 0, 0, 0);
    leftPanelLayout->setSpacing(20);

    // ========== 第一个卡片：接入单位 ==========
    cardUnit = new QFrame();
    cardUnit->setFrameShape(QFrame::NoFrame); // 移除边框
    QVBoxLayout *unitLayout = new QVBoxLayout(cardUnit);
    unitLayout->setContentsMargins(20, 20, 20, 20);
    unitLayout->setSpacing(20);

    QLabel *lblUnitTitle = new QLabel("📊 接入单位");
    lblUnitTitle->setFont(QFont("Microsoft YaHei", 18, QFont::Bold)); // 增大标题字体
    QLabel *lblEnterpriseUnit = new QLabel("企事业单位 (家)    4623");
    QLabel *lblFamilyUnit = new QLabel("九小/家庭 (家)    4623");
    // 增大内容字体
    lblEnterpriseUnit->setFont(QFont("Microsoft YaHei", 14));
    lblFamilyUnit->setFont(QFont("Microsoft YaHei", 14));

    unitLayout->addWidget(lblUnitTitle);
    unitLayout->addWidget(lblEnterpriseUnit);
    unitLayout->addWidget(lblFamilyUnit);
    unitLayout->addStretch();

    // ========== 第二个卡片：接入设备 ==========
    cardDevice = new QFrame();
    cardDevice->setFrameShape(QFrame::NoFrame); // 移除边框
    QVBoxLayout *deviceLayout = new QVBoxLayout(cardDevice);
    deviceLayout->setContentsMargins(20, 20, 20, 20);
    deviceLayout->setSpacing(20);

    QLabel *lblDeviceTitle = new QLabel("🔧 接入设备");
    lblDeviceTitle->setFont(QFont("Microsoft YaHei", 18, QFont::Bold));
    QLabel *lblDeviceCount = new QLabel("接入设备 (万套)    4623");
    lblDeviceCount->setFont(QFont("Microsoft YaHei", 14));

    deviceLayout->addWidget(lblDeviceTitle);
    deviceLayout->addWidget(lblDeviceCount);
    deviceLayout->addStretch();

    // ========== 第三个卡片：实时监测 ==========
    cardMonitor = new QFrame();
    cardMonitor->setFrameShape(QFrame::NoFrame); // 移除边框
    QVBoxLayout *monitorLayout = new QVBoxLayout(cardMonitor);
    monitorLayout->setContentsMargins(20, 20, 20, 20);
    monitorLayout->setSpacing(20);

    QLabel *lblMonitorTitle = new QLabel("🚨 实时监测");
    lblMonitorTitle->setFont(QFont("Microsoft YaHei", 18, QFont::Bold));
    QLabel *lblFireAlarmCount = new QLabel("实时火警报警数    96234");
    QLabel *lblHighRiskCount = new QLabel("实时高度疑似真警数    4623");
    QLabel *lblElecFireCount = new QLabel("实时电气火灾预警数    4623");
    // 增大内容字体
    lblFireAlarmCount->setFont(QFont("Microsoft YaHei", 14));
    lblHighRiskCount->setFont(QFont("Microsoft YaHei", 14));
    lblElecFireCount->setFont(QFont("Microsoft YaHei", 14));

    monitorLayout->addWidget(lblMonitorTitle);
    monitorLayout->addWidget(lblFireAlarmCount);
    monitorLayout->addWidget(lblHighRiskCount);
    monitorLayout->addWidget(lblElecFireCount);
    monitorLayout->addStretch();

    // ========== 第四个卡片：服务机构 ==========
    cardService = new QFrame();
    cardService->setFrameShape(QFrame::NoFrame); // 移除边框
    QVBoxLayout *serviceLayout = new QVBoxLayout(cardService);
    serviceLayout->setContentsMargins(20, 20, 20, 20);
    serviceLayout->setSpacing(20);

    QLabel *lblServiceTitle = new QLabel("👨💻 服务机构");
    lblServiceTitle->setFont(QFont("Microsoft YaHei", 18, QFont::Bold));
    QLabel *lblServiceOrg = new QLabel("服务机构    4623");
    QLabel *lblServiceStaff = new QLabel("服务人员    4623");
    // 增大内容字体
    lblServiceOrg->setFont(QFont("Microsoft YaHei", 14));
    lblServiceStaff->setFont(QFont("Microsoft YaHei", 14));

    serviceLayout->addWidget(lblServiceTitle);
    serviceLayout->addWidget(lblServiceOrg);
    serviceLayout->addWidget(lblServiceStaff);
    serviceLayout->addStretch();

    // 添加四个卡片到左侧布局
    leftPanelLayout->addWidget(cardUnit);
    leftPanelLayout->addWidget(cardDevice);
    leftPanelLayout->addWidget(cardMonitor);
    leftPanelLayout->addWidget(cardService);
}

void index::setupRightPanel()
{
    rightPanel = new QFrame();
    rightPanel->setFrameShape(QFrame::NoFrame); // 移除边框
    QVBoxLayout *rightPanelLayout = new QVBoxLayout(rightPanel);
    rightPanelLayout->setContentsMargins(20, 20, 20, 20);
    rightPanelLayout->setSpacing(25);

    // 顶部数据卡片
    cardLayout = new QHBoxLayout();
    cardLayout->setSpacing(20);

    // 火警报警卡片
    cardFireAlarm = new QFrame();
    cardFireAlarm->setFrameShape(QFrame::NoFrame); // 移除边框
    QVBoxLayout *fireLayout = new QVBoxLayout(cardFireAlarm);
    fireLayout->setContentsMargins(20, 20, 20, 20);
    fireLayout->setSpacing(18);
    QLabel *fireTitle = new QLabel("火警报警");
    fireTitle->setFont(QFont("Microsoft YaHei", 16, QFont::Bold));
    QLabel *fireReal = new QLabel("实时数  56");
    QLabel *fireTotal = new QLabel("累计数  5620488");
    fireReal->setFont(QFont("Microsoft YaHei", 18, QFont::Bold));
    fireTotal->setFont(QFont("Microsoft YaHei", 14));
    fireLayout->addWidget(fireTitle);
    fireLayout->addWidget(fireReal);
    fireLayout->addWidget(fireTotal);

    // 高度疑似真警数卡片
    cardHighRisk = new QFrame();
    cardHighRisk->setFrameShape(QFrame::NoFrame); // 移除边框
    QVBoxLayout *highRiskLayout = new QVBoxLayout(cardHighRisk);
    highRiskLayout->setContentsMargins(20, 20, 20, 20);
    highRiskLayout->setSpacing(18);
    QLabel *highRiskTitle = new QLabel("高度疑似真警数");
    highRiskTitle->setFont(QFont("Microsoft YaHei", 16, QFont::Bold));
    QLabel *highRiskReal = new QLabel("实时数  536");
    QLabel *highRiskTotal = new QLabel("累计数  15620488");
    highRiskReal->setFont(QFont("Microsoft YaHei", 18, QFont::Bold));
    highRiskTotal->setFont(QFont("Microsoft YaHei", 14));
    highRiskLayout->addWidget(highRiskTitle);
    highRiskLayout->addWidget(highRiskReal);
    highRiskLayout->addWidget(highRiskTotal);

    // 电气火灾预警卡片
    cardElecFire = new QFrame();
    cardElecFire->setFrameShape(QFrame::NoFrame); // 移除边框
    QVBoxLayout *elecLayout = new QVBoxLayout(cardElecFire);
    elecLayout->setContentsMargins(20, 20, 20, 20);
    elecLayout->setSpacing(18);
    QLabel *elecTitle = new QLabel("电气火灾预警");
    elecTitle->setFont(QFont("Microsoft YaHei", 16, QFont::Bold));
    QLabel *elecReal = new QLabel("实时数  56");
    QLabel *elecTotal = new QLabel("累计数  5620488");
    elecReal->setFont(QFont("Microsoft YaHei", 18, QFont::Bold));
    elecTotal->setFont(QFont("Microsoft YaHei", 14));
    elecLayout->addWidget(elecTitle);
    elecLayout->addWidget(elecReal);
    elecLayout->addWidget(elecTotal);

    // 监测真实火情卡片
    cardRealFire = new QFrame();
    cardRealFire->setFrameShape(QFrame::NoFrame); // 移除边框
    QVBoxLayout *realFireLayout = new QVBoxLayout(cardRealFire);
    realFireLayout->setContentsMargins(20, 20, 20, 20);
    realFireLayout->setSpacing(18);
    QLabel *realFireTitle = new QLabel("监测真实火情");
    realFireTitle->setFont(QFont("Microsoft YaHei", 16, QFont::Bold));
    QLabel *realFireReal = new QLabel("实时数  56");
    realFireReal->setFont(QFont("Microsoft YaHei", 18, QFont::Bold));
    realFireLayout->addWidget(realFireTitle);
    realFireLayout->addWidget(realFireReal);

    cardLayout->addWidget(cardFireAlarm, 1);
    cardLayout->addWidget(cardHighRisk, 1);
    cardLayout->addWidget(cardElecFire, 1);
    cardLayout->addWidget(cardRealFire, 1);
    rightPanelLayout->addLayout(cardLayout);

    // 折线图占位区
    chartLinePlaceholder = new QFrame();
    chartLinePlaceholder->setFrameShape(QFrame::NoFrame); // 移除边框
    chartLinePlaceholder->setMinimumHeight(240);
    QLabel *lineLabel = new QLabel("【折线图占位区】历史监测趋势\n后续可在此添加QChart或自定义折线图组件", chartLinePlaceholder);
    lineLabel->setAlignment(Qt::AlignCenter);
    lineLabel->setFont(QFont("Microsoft YaHei", 12));
    rightPanelLayout->addWidget(chartLinePlaceholder);

    // 底部图表区
    bottomChartLayout = new QHBoxLayout();
    bottomChartLayout->setSpacing(20);

    // 条形图占位区
    chartBarPlaceholder = new QFrame();
    chartBarPlaceholder->setFrameShape(QFrame::NoFrame); // 移除边框
    chartBarPlaceholder->setMinimumHeight(220);
    QLabel *barLabel = new QLabel("【条形图占位区】报警时间分布\n后续可在此添加QBarSeries或自定义条形图组件", chartBarPlaceholder);
    barLabel->setAlignment(Qt::AlignCenter);
    barLabel->setFont(QFont("Microsoft YaHei", 12));

    // 呼叫时长图占位区
    chartCallPlaceholder = new QFrame();
    chartCallPlaceholder->setFrameShape(QFrame::NoFrame); // 移除边框
    chartCallPlaceholder->setMinimumHeight(220);
    QLabel *callLabel = new QLabel("【柱状图占位区】监测值守电话呼叫时长\n后续可在此添加QBarSeries或自定义柱状图组件", chartCallPlaceholder);
    callLabel->setAlignment(Qt::AlignCenter);
    callLabel->setFont(QFont("Microsoft YaHei", 12));

    bottomChartLayout->addWidget(chartBarPlaceholder, 2);
    bottomChartLayout->addWidget(chartCallPlaceholder, 1);
    rightPanelLayout->addLayout(bottomChartLayout);
}

void index::applyGlobalStyle()
{
    // 全局深色科技风样式
    this->setStyleSheet(R"(
        QMainWindow {
            background-color: #050b22;
        }
        QFrame {
            background-color: #0a1a3a;
            border-radius: 12px;
        }
        QPushButton {
            background-color: #0a2a5a;
            color: #00d8ff;
            border: 2px solid #0066cc;
            border-radius: 8px;
        }
        QPushButton:hover {
            background-color: #004a7c;
            border-color: #00d8ff;
        }
        QPushButton:pressed {
            background-color: #0066cc;
        }
        QLabel {
            color: #e0e6f6;
            font-family: "Microsoft YaHei";
        }
        QLabel[text^="实时数"] {
            color: #ff4d4d;
            font-size: 18px;
            font-weight: bold;
        }
        QLabel[text^="累计数"] {
            color: #ffaa00;
            font-size: 14px;
        }
        /* 取消标题的下划线边框，让界面更干净 */
        QLabel[text^="📊"], QLabel[text^="🔧"], QLabel[text^="🚨"], QLabel[text^="👨💻"] {
            color: #00d8ff;
            font-size: 18px;
            /* 移除下划线和底部内边距 */
            border-bottom: none;
            padding-bottom: 0px;
        }
    )");

    // Logo区域样式
    logoFrame->setStyleSheet("background-color: #0a1a3a; border-radius: 12px;");

    // 左侧四个卡片样式
    cardUnit->setStyleSheet(R"(
        QFrame {
            background-color: #0f2a4a;
            border-radius: 12px;
        }
    )");
    cardDevice->setStyleSheet(R"(
        QFrame {
            background-color: #2a0f4a;
            border-radius: 12px;
        }
    )");
    cardMonitor->setStyleSheet(R"(
        QFrame {
            background-color: #4a2a0f;
            border-radius: 12px;
        }
    )");
    cardService->setStyleSheet(R"(
        QFrame {
            background-color: #0f4a2a;
            border-radius: 12px;
        }
    )");

    // 右侧数据卡片样式
    cardFireAlarm->setStyleSheet("background-color: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #3a0a1a, stop:1 #5a1a2a);");
    cardHighRisk->setStyleSheet("background-color: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #3a2a0a, stop:1 #5a3a1a);");
    cardElecFire->setStyleSheet("background-color: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #0a2a5a, stop:1 #1a3a6a);");
    cardRealFire->setStyleSheet("background-color: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #3a0a0a, stop:1 #5a1a1a);");

    // 激活当前导航按钮
    btnNational->setStyleSheet("background-color: #004a7c; border-color: #00d8ff; color: #ffffff;");
}
