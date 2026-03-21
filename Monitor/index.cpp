#include "index.h"
#include "mainwindow.h"
#include "monitorconfig.h"
#include "overview.h"
#include "task.h"
#include "login.h"
#include "disasterdao.h"

#include <QDesktopServices>
#include <QUrl>
#include <QDebug>
#include <QMessageBox>
#include <QSettings>
#include <QMap>
#include <QMultiMap>
#include <QDateTime>
#include <QMouseEvent>
#include <QPainter>
#include <QGuiApplication>
#include <QScreen>
#include <algorithm>
#include <QTimer>
#include <QDateEdit>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QBarCategoryAxis>
#include <QtCharts/QPieSeries>
#include <QtCharts/QPieSlice>
#include <QtCharts/QValueAxis>

index::index(QWidget *parent) : QMainWindow(parent)
{
    setWindowTitle("灾害监测系统");
    setMinimumSize(1440, 810);

    centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(20);

    // 初始化 Tooltip 标签
    tooltipLabel = new QLabel(this);
    tooltipLabel->setStyleSheet(R"(
        QLabel {
            background-color: rgba(10, 20, 40, 0.95);
            color: #e0e6f6;
            border: 1px solid #00d8ff;
            border-left: 4px solid #00d8ff;
            border-radius: 8px;
            padding: 12px 16px;
            font-size: 15px;
            font-family: "Microsoft YaHei";
            line-height: 1.6;
        }
        b { color: #00d8ff; font-weight: bold; font-size: 17px; }
        .type-fire { color: #ff4d4d; font-size: 16px; font-weight: bold; }
        .type-warn { color: #ffaa00; font-size: 16px; font-weight: bold; }
        hr { border: 0; border-top: 1px dashed #445566; margin: 6px 0; }
    )");
    tooltipLabel->setWindowFlags(Qt::Tool | Qt::FramelessWindowHint);
    tooltipLabel->setAttribute(Qt::WA_TranslucentBackground);
    tooltipLabel->hide();

    setupTopNav();
    setupLeftPanel();
    setupRightPanel();

    contentLayout = new QHBoxLayout();
    contentLayout->addLayout(leftPanelLayout, 1);
    contentLayout->addWidget(rightPanel, 4);
    mainLayout->addLayout(contentLayout);

    applyGlobalStyle();

    GetData();
    updateBarChart();
    updateTypeChart();
}

index::~index()
{
    delete tooltipLabel;
}

void index::setupTopNav()
{
    topNavLayout = new QHBoxLayout();
    topNavLayout->setSpacing(8);
    topNavLayout->setAlignment(Qt::AlignCenter);

    logoFrame = new QFrame();
    logoFrame->setFrameShape(QFrame::NoFrame);
    QHBoxLayout *logoLayout = new QHBoxLayout(logoFrame);
    logoLayout->setContentsMargins(15, 15, 15, 15);
    logoLayout->setSpacing(15);

    labLogo = new QLabel();
    labLogo->setPixmap(QPixmap(":/sound/newLogo.jpg").scaled(120, 80, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    labLogo->setAlignment(Qt::AlignCenter);

    labSystemName = new QLabel("灾警系统");
    labSystemName->setFont(QFont("Microsoft YaHei", 28, QFont::Bold));
    labSystemName->setAlignment(Qt::AlignCenter);
    labSystemName->setStyleSheet("color: white;");

    logoLayout->addWidget(labLogo);
    logoLayout->addWidget(labSystemName);
    topNavLayout->addWidget(logoFrame, 0, Qt::AlignLeft);

    btnRealTime = new QPushButton("▶ 实时概览");
    btnFireAlarm = new QPushButton("▶ 灾情总览");
    btnNational = new QPushButton("▶ 任务中心");
    btnConfig = new QPushButton("🛠️ 系统设置");
    outbtn = new QPushButton("⚙️ 退出登录");

    connect(btnRealTime, &QPushButton::clicked, [=](){
        MainWindow *w = new MainWindow();
        w->setAttribute(Qt::WA_DeleteOnClose);
        w->show();
    });
    connect(btnConfig, &QPushButton::clicked, [=](){
        MonitorConfig dialog(this);
        dialog.exec();
    });
    connect(btnFireAlarm,&QPushButton::clicked, [=](){
        overview *w = new overview();
        w->setAttribute(Qt::WA_DeleteOnClose);
        w->show();
    });
    connect(btnNational,&QPushButton::clicked, [=](){
        task *t = new task();
        t->setAttribute(Qt::WA_DeleteOnClose);
        t->show();
    });
    connect(outbtn,&QPushButton::clicked, [=](){
        QMessageBox msgBox(this);
                msgBox.setWindowTitle("确认退出");

                // 1. 设置大字体
                QFont font("Microsoft YaHei", 14);
                msgBox.setFont(font);
                msgBox.setInformativeText("确定要退出登录吗？");
                msgBox.setIconPixmap(QPixmap()); // 去掉图标
                msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);

                // 修改按钮文字
                QPushButton *btnYes = qobject_cast<QPushButton*>(msgBox.button(QMessageBox::Yes));
                QPushButton *btnNo = qobject_cast<QPushButton*>(msgBox.button(QMessageBox::No));
                if (btnYes) btnYes->setText("确认");
                if (btnNo) btnNo->setText("取消");

                // 2. 【关键修改】强制重置内部样式，对抗全局深色背景
                // 这段样式会强制让 MessageBox 内部的 Label 背景变白/透明，按钮变回灰色
                msgBox.setStyleSheet(R"(
                    QMessageBox {
                        background-color: #ffffff; /* 对话框背景纯白 */
                    }
                    QMessageBox QLabel {
                        color: #000000;          /* 文字黑色 */
                        background-color: transparent; /* 强制去掉文字背后的深色块 */
                        border: none;            /* 去掉边框 */
                        padding: 5px;
                    }
                    QMessageBox QPushButton {
                        background-color: #f0f0f0; /* 按钮默认浅灰 */
                        color: #000000;
                        border: 1px solid #cccccc;
                        border-radius: 4px;
                        padding: 6px 15px;
                        min-width: 80px;
                    }
                    QMessageBox QPushButton:hover {
                        background-color: #e0e0e0; /* 鼠标悬停稍深 */
                        border-color: #999999;
                    }
                    QMessageBox QPushButton:pressed {
                        background-color: #d0d0d0; /* 按下更深 */
                    }
                )");

                int ret = msgBox.exec();

                if (ret == QMessageBox::Yes) {
                    QSettings settings("System", "disaster");
                    settings.remove("userid");
                    settings.remove("role");
                    Login *loginWnd = new Login();
                    loginWnd->show();
                    this->close();
                }
    });

    QList<QPushButton*> navBtns = {btnRealTime, btnFireAlarm,  btnNational, btnConfig, outbtn};
    for (auto btn : navBtns) {
        btn->setMinimumSize(160, 50);
        btn->setFont(QFont("Microsoft YaHei", 14, QFont::Bold));
        topNavLayout->addWidget(btn, 1, Qt::AlignCenter);
    }

    mainLayout->addLayout(topNavLayout);

    btnRefresh = new QPushButton("🔄 刷新数据");
    btnRefresh->setMinimumSize(140, 50);
    btnRefresh->setFont(QFont("Microsoft YaHei", 14, QFont::Bold));
    btnRefresh->setStyleSheet(R"(
        QPushButton {
            background-color: #0066cc;
            color: white;
            border: 2px solid #00d8ff;
            border-radius: 8px;
        }
        QPushButton:hover {
            background-color: #0088ff;
            border-color: #ffffff;
        }
        QPushButton:pressed {
            background-color: #004a99;
        }
    )");

    connect(btnRefresh, &QPushButton::clicked, this, [this]() {
        btnRefresh->setText("⏳ 加载中...");
        btnRefresh->setEnabled(false);
        this->GetData();
        this->updateBarChart();
        this->updateTypeChart();
        QTimer::singleShot(1000, this, [this]() {
            if(btnRefresh) {
                btnRefresh->setText("🔄 刷新数据");
                btnRefresh->setEnabled(true);
            }
        });
    });

    topNavLayout->addWidget(btnRefresh, 0, Qt::AlignRight);
}

void index::setupLeftPanel()
{
    leftPanelLayout = new QVBoxLayout();
    leftPanelLayout->setContentsMargins(0, 0, 0, 0);
    leftPanelLayout->setSpacing(20);

    auto createCard = [](const QString &title, const QStringList &items, const QString &/*colorCode*/, const QString &objPrefix) {
        QFrame *card = new QFrame();
        card->setFrameShape(QFrame::NoFrame);
        QVBoxLayout *layout = new QVBoxLayout(card);
        layout->setContentsMargins(20, 20, 20, 20);
        layout->setSpacing(20);
        QLabel *lblTitle = new QLabel(title);
        lblTitle->setFont(QFont("Microsoft YaHei", 18, QFont::Bold));
        lblTitle->setStyleSheet("color: #00d8ff;");
        layout->addWidget(lblTitle);
        int idx = 0;
        for (const QString &text : items) {
            QLabel *lbl = new QLabel(text);
            lbl->setObjectName(objPrefix + "_" + QString::number(idx++));
            lbl->setFont(QFont("Microsoft YaHei", 14));
            lbl->setStyleSheet("color: #e0e6f6;");
            layout->addWidget(lbl);
        }
        layout->addStretch();
        return card;
    };

    cardUnit = createCard("📊 时间点统计", {"灾害易发生时间段 (h)    --"}, "#0f2a4a", "card_time");
    cardDevice = createCard("🔧 今日使用", {"时长 (h)    1h"}, "#2a0f4a", "card_device");
    cardMonitor = createCard("🚨 实时监测", {"尚未分配灾害    --", "今日已解决灾害    --", "正在处理灾害    --"}, "#4a2a0f", "card_monitor");
    cardService = createCard("👨💻 服务机构", {"单位名  杭州灾害预警中心", "在线用户  10"}, "#0f4a2a", "card_service");

    cardUnit->setStyleSheet("background-color: #0f2a4a; border-radius: 12px;");
    cardDevice->setStyleSheet("background-color: #2a0f4a; border-radius: 12px;");
    cardMonitor->setStyleSheet("background-color: #4a2a0f; border-radius: 12px;");
    cardService->setStyleSheet("background-color: #0f4a2a; border-radius: 12px;");

    leftPanelLayout->addWidget(cardUnit);
    leftPanelLayout->addWidget(cardDevice);
    leftPanelLayout->addWidget(cardMonitor);
    leftPanelLayout->addWidget(cardService);
}

void index::setupDateRangeControl(QHBoxLayout *layout, QDateEdit *&startEdit, QDateEdit *&endEdit, const QString &label) {
    QLabel *lblIcon = new QLabel(label);
    lblIcon->setStyleSheet("color: #00d8ff; font-size: 14px; font-weight: bold;");

    startEdit = new QDateEdit();
    startEdit->setDate(QDate::currentDate().addMonths(-1));
    startEdit->setCalendarPopup(true);
    startEdit->setDisplayFormat("yyyy-MM-dd");
    startEdit->setMinimumWidth(110);

    QLabel *lblTo = new QLabel("至");
    lblTo->setStyleSheet("color: #a0aab5; font-size: 14px; margin: 0 5px;");

    endEdit = new QDateEdit();
    endEdit->setDate(QDate::currentDate());
    endEdit->setCalendarPopup(true);
    endEdit->setDisplayFormat("yyyy-MM-dd");
    endEdit->setMinimumWidth(110);

    QString dateStyle = R"(
        QDateEdit {
            background-color: #0a2a5a;
            color: #ffffff;
            border: 1px solid #0066cc;
            border-radius: 4px;
            padding: 4px 8px;
            font-size: 12px;
            font-family: "Microsoft YaHei";
        }
        QDateEdit::drop-down { border: none; width: 20px; }
        QDateEdit::down-arrow {
            image: none;
            border-left: 4px solid transparent;
            border-right: 4px solid transparent;
            border-top: 6px solid #00d8ff;
            margin-right: 5px;
        }
        QCalendarWidget { background-color: #0a2a5a; color: #1a2b45; font-weight: bold; }
        QCalendarWidget QTableView { alternate-background-color: #0a1a3a; background-color: #0f2a4a; color: #e0e6f6; }
        QCalendarWidget QWidget#qt_calendar_navigationbar { background-color: #0a1a3a; color: white; }
        QCalendarWidget QToolButton { color: white; background-color: transparent; font-weight: bold; }
        QCalendarWidget QMenu { background-color: #0f2a4a; color: white; }
        QCalendarWidget QSpinBox { background-color: #0f2a4a; color: white; }
        QCalendarWidget QAbstractItemView:enabled { background-color: #0f2a4a; color: #e0e6f6; selection-background-color: #00d8ff; selection-color: black; }
        QCalendarWidget QAbstractItemView:disabled { color: #445566; }
    )";
    startEdit->setStyleSheet(dateStyle);
    endEdit->setStyleSheet(dateStyle);

    layout->addWidget(lblIcon);
    layout->addWidget(startEdit);
    layout->addWidget(lblTo);
    layout->addWidget(endEdit);
    layout->addStretch();
}

void index::setupRightPanel()
{
    rightPanel = new QFrame();
    rightPanel->setFrameShape(QFrame::NoFrame);
    QVBoxLayout *rightPanelLayout = new QVBoxLayout(rightPanel);
    rightPanelLayout->setContentsMargins(20, 20, 20, 20);
    rightPanelLayout->setSpacing(25);

    // --- 顶部统计卡片 ---
    cardLayout = new QHBoxLayout();
    cardLayout->setSpacing(20);
    auto createStatCard = [](const QString &title, const QString &realText, const QString &totalText, const QString &gradientStart, const QString &gradientEnd, const QString &objPrefix) {
        QFrame *card = new QFrame();
        card->setFrameShape(QFrame::NoFrame);
        QVBoxLayout *layout = new QVBoxLayout(card);
        layout->setContentsMargins(20, 20, 20, 20);
        layout->setSpacing(18);
        QLabel *lblTitle = new QLabel(title);
        lblTitle->setFont(QFont("Microsoft YaHei", 16, QFont::Bold));
        lblTitle->setStyleSheet("color: white;");
        QLabel *lblReal = new QLabel(realText);
        lblReal->setObjectName(objPrefix + "_real");
        lblReal->setFont(QFont("Microsoft YaHei", 18, QFont::Bold));
        lblReal->setStyleSheet("color: #ff4d4d;");
        QLabel *lblTotal = new QLabel(totalText);
        if (!totalText.isEmpty()) {
            lblTotal->setObjectName(objPrefix + "_total");
            lblTotal->setFont(QFont("Microsoft YaHei", 14));
            lblTotal->setStyleSheet("color: #ffaa00;");
        }
        layout->addWidget(lblTitle);
        layout->addWidget(lblReal);
        if (!totalText.isEmpty()) layout->addWidget(lblTotal);
        card->setStyleSheet(QString("background-color: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 %1, stop:1 %2); border-radius: 8px;")
                            .arg(gradientStart).arg(gradientEnd));
        return card;
    };

    cardFireAlarm = createStatCard("灾害数", "当日实时数  --", "本月累计数  --", "#3a0a1a", "#5a1a2a", "stat_disaster");
    cardHighRisk = createStatCard("发生率", "今日  --%", "本月  --", "#3a2a0a", "#5a3a1a", "stat_rate");
    cardElecFire = createStatCard("当前最频繁灾害", "暂无", "累计数  --", "#0a2a5a", "#1a3a6a", "stat_freq");
    cardRealFire = createStatCard("罕见灾害", "暂无", "累计数 --", "#3a0a0a", "#5a1a1a", "stat_rare");

    cardLayout->addWidget(cardFireAlarm, 1);
    cardLayout->addWidget(cardHighRisk, 1);
    cardLayout->addWidget(cardElecFire, 1);
    cardLayout->addWidget(cardRealFire, 1);
    rightPanelLayout->addLayout(cardLayout);

    // --- 折线图区域 ---
    QFrame *chartContainer = new QFrame();
    chartContainer->setFrameShape(QFrame::NoFrame);
    QVBoxLayout *chartContainerLayout = new QVBoxLayout(chartContainer);
    chartContainerLayout->setContentsMargins(0, 0, 0, 0);
    chartContainerLayout->setSpacing(10);

    QHBoxLayout *chartHeaderLayout = new QHBoxLayout();
    chartHeaderLayout->setContentsMargins(5, 5, 5, 5);
    chartHeaderLayout->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    QLabel *chartTitleLabel = new QLabel("24 小时灾害等级趋势分布");
    chartTitleLabel->setFont(QFont("Microsoft YaHei", 16, QFont::Bold));
    chartTitleLabel->setStyleSheet("color: #00d8ff;");

    QLabel *lblDateHint = new QLabel("📅 查询日期:");
    lblDateHint->setStyleSheet("color: #00d8ff; font-size: 14px; font-weight: bold; margin-left: 10px;");

    dateSelector = new QDateEdit();
    dateSelector->setDate(QDate::currentDate());
    dateSelector->setCalendarPopup(true);
    dateSelector->setDisplayFormat("yyyy-MM-dd");
    dateSelector->setMinimumWidth(140);
    dateSelector->setMaximumWidth(160);
    dateSelector->setStyleSheet(R"(
        QDateEdit { background-color: #0a2a5a; color: #ffffff; border: 1px solid #0066cc; border-radius: 4px; padding: 4px 8px; font-size: 13px; font-family: "Microsoft YaHei"; }
        QDateEdit::drop-down { border: none; width: 20px; }
        QDateEdit::down-arrow { image: none; border-left: 4px solid transparent; border-right: 4px solid transparent; border-top: 6px solid #00d8ff; margin-right: 5px; }
        QCalendarWidget { background-color: #0a2a5a; color: #1a2b45; font-weight: bold; }
        QCalendarWidget QTableView { alternate-background-color: #0a1a3a; background-color: #0f2a4a; color: #e0e6f6; }
        QCalendarWidget QWidget#qt_calendar_navigationbar { background-color: #0a1a3a; color: white; }
        QCalendarWidget QToolButton { color: white; background-color: transparent; font-weight: bold; }
        QCalendarWidget QMenu { background-color: #0f2a4a; color: white; }
        QCalendarWidget QSpinBox { background-color: #0f2a4a; color: white; }
        QCalendarWidget QAbstractItemView:enabled { background-color: #0f2a4a; color: #e0e6f6; selection-background-color: #00d8ff; selection-color: black; }
        QCalendarWidget QAbstractItemView:disabled { color: #445566; }
    )");
    connect(dateSelector, &QDateEdit::dateChanged, this, [this]() { GetData(); });

    chartHeaderLayout->addWidget(chartTitleLabel);
    chartHeaderLayout->addStretch();
    chartHeaderLayout->addWidget(lblDateHint);
    chartHeaderLayout->addWidget(dateSelector);

    lineChart = new QChart();
    lineChart->setTitle("");
    lineChart->setBackgroundVisible(false);
    lineChart->setPlotAreaBackgroundVisible(false);
    lineChart->setAnimationOptions(QChart::SeriesAnimations);
    lineChart->setMargins(QMargins(0, 0, 0, 0));

    axisX = new QDateTimeAxis();
    axisX->setTitleText("发生时刻");
    axisX->setFormat("HH:mm");
    axisX->setGridLineColor(QColor("#2a3b55"));
    axisX->setLabelsColor(QColor("#a0aab5"));
    axisX->setTitleBrush(QBrush(QColor("#a0aab5")));
    axisX->setMinorGridLineColor(QColor("#1a2b45"));
    lineChart->addAxis(axisX, Qt::AlignBottom);

    axisY = new QValueAxis();
    axisY->setTitleText("灾害等级");
    axisY->setRange(0, 10);
    axisY->setGridLineColor(QColor("#2a3b55"));
    axisY->setLabelsColor(QColor("#a0aab5"));
    axisY->setTitleBrush(QBrush(QColor("#a0aab5")));
    axisY->setTickCount(6);
    axisY->setLabelFormat("%.0f");
    lineChart->addAxis(axisY, Qt::AlignLeft);

    lineSeries = new QLineSeries();
    lineSeries->setName("灾害等级");
    lineSeries->setColor(QColor("#00d8ff"));
    lineSeries->setPen(QPen(QColor("#00d8ff"), 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));

    scatterSeries = new QScatterSeries();
    scatterSeries->setName("数据点");
    scatterSeries->setMarkerSize(10);
    scatterSeries->setMarkerShape(QScatterSeries::MarkerShapeCircle);
    scatterSeries->setColor(QColor("#00d8ff"));
    scatterSeries->setBorderColor(Qt::transparent);

    hoverSeries = new QScatterSeries();
    hoverSeries->setName("选中点");
    hoverSeries->setMarkerSize(18);
    hoverSeries->setMarkerShape(QScatterSeries::MarkerShapeCircle);
    hoverSeries->setColor(QColor("#ffffff"));
    hoverSeries->setBorderColor(QColor("#ff4d4d"));

    connect(scatterSeries, &QScatterSeries::hovered, this, [this](const QPointF &point, bool state) {
        if (state) {
            hoverSeries->clear();
            hoverSeries->append(point);
            qint64 timestamp = static_cast<qint64>(point.x());
            QList<DisasterRecord> records = pointDataMap.values(timestamp);
            if (records.isEmpty()) { hideTooltip(); return; }
            QString infoHtml = "<div style='font-size:14px;'>";
            QDateTime firstDt = QDateTime::fromMSecsSinceEpoch(timestamp);
            infoHtml += QString("<b style='color:#00d8ff;'>⏰ %1  (共 %2 起)</b><br><hr>")
                        .arg(firstDt.toString("HH:mm")).arg(records.size());
            for (const auto& rec : records) {
                QString typeName = rec.disasterType.isEmpty() ? "监测报警" : rec.disasterType;
                QString typeClass = typeName.contains("火") ? "type-fire" : "type-warn";
                QString severityInfo = QString("等级：<b style='color:#ff4d4d; font-size:15px;'>%1</b>").arg(rec.severity);
                QString desc = rec.content.isEmpty() ? rec.location : rec.content;
                infoHtml += QString("<div style='margin-bottom:8px; line-height: 1.5;'>"
                                    "<span class='%1'>🔥 %2</span> (%3)<br>📍 %4<br>📝 <span style='color:#ccc;'>%5</span></div>")
                    .arg(typeClass).arg(typeName).arg(severityInfo).arg(rec.location)
                    .arg(desc.length() > 20 ? desc.left(20) + "..." : desc);
                if (&rec != &records.last()) infoHtml += "<hr style='border:0; border-top:1px dashed #334455; margin:6px 0;'>";
            }
            infoHtml += "</div>";
            QPoint chartPos = lineChart->mapToPosition(point).toPoint();
            QPoint globalPos = chartView->mapToGlobal(chartPos);
            showTooltip(globalPos, infoHtml);
        } else {
            hideTooltip();
            hoverSeries->clear();
        }
    });

    lineChart->addSeries(lineSeries);
    lineChart->addSeries(scatterSeries);
    lineChart->addSeries(hoverSeries);
    lineSeries->attachAxis(axisX);
    lineSeries->attachAxis(axisY);
    scatterSeries->attachAxis(axisX);
    scatterSeries->attachAxis(axisY);
    hoverSeries->attachAxis(axisX);
    hoverSeries->attachAxis(axisY);

    chartView = new QChartView(lineChart);
    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->setMinimumHeight(260);
    chartView->setBackgroundBrush(QBrush(QColor("#0a1a3a")));
    chartView->setStyleSheet("border: none; border-radius: 12px;");

    chartContainerLayout->addLayout(chartHeaderLayout);
    chartContainerLayout->addWidget(chartView);
    rightPanelLayout->addWidget(chartContainer);

    // ================== 底部图表区域 ==================
    bottomChartLayout = new QHBoxLayout();
    bottomChartLayout->setSpacing(20);

    // --- 左侧：条形图 ---
    QFrame *barFrame = new QFrame();
    barFrame->setFrameShape(QFrame::NoFrame);
    barFrame->setStyleSheet("background-color: #0f2a4a; border-radius: 12px;");
    QVBoxLayout *barFrameLayout = new QVBoxLayout(barFrame);
    barFrameLayout->setContentsMargins(15, 15, 15, 15);
    barFrameLayout->setSpacing(10);

    QHBoxLayout *barControlLayout = new QHBoxLayout();
    setupDateRangeControl(barControlLayout, barStartDateEdit, barEndDateEdit, "📊 报警时段统计:");

    connect(barStartDateEdit, &QDateEdit::dateChanged, this, &index::updateBarChart);
    connect(barEndDateEdit, &QDateEdit::dateChanged, this, &index::updateBarChart);

    barFrameLayout->addLayout(barControlLayout);

    barChart = new QChart();
    barChart->setTitle("");
    barChart->setBackgroundVisible(false);
    barChart->setPlotAreaBackgroundVisible(false);
    barChart->setAnimationOptions(QChart::SeriesAnimations);
    barChart->legend()->hide();

    barSeries = new QBarSeries();
    barChart->addSeries(barSeries);

    QBarCategoryAxis *axisBarX = new QBarCategoryAxis();
    axisBarX->setGridLineColor(QColor("#2a3b55"));
    axisBarX->setLabelsColor(QColor("#a0aab5"));
    barChart->addAxis(axisBarX, Qt::AlignBottom);
    barSeries->attachAxis(axisBarX);

    QValueAxis *axisBarY = new QValueAxis();
    axisBarY->setGridLineColor(QColor("#2a3b55"));
    axisBarY->setLabelsColor(QColor("#a0aab5"));
    axisBarY->setLabelFormat("%.0f");
    barChart->addAxis(axisBarY, Qt::AlignLeft);
    barSeries->attachAxis(axisBarY);

    barChartView = new QChartView(barChart);
    barChartView->setRenderHint(QPainter::Antialiasing);
    barChartView->setMinimumHeight(220);
    barChartView->setBackgroundBrush(QBrush(QColor("#0f2a4a")));
    barChartView->setStyleSheet("border: none;");

    barFrameLayout->addWidget(barChartView);
    bottomChartLayout->addWidget(barFrame, 2);

    // --- 右侧：饼图 (灾害类型分布) ---
    QFrame *pieFrame = new QFrame();
    pieFrame->setFrameShape(QFrame::NoFrame);
    pieFrame->setStyleSheet("background-color: #0f2a4a; border-radius: 12px;");
    QVBoxLayout *pieFrameLayout = new QVBoxLayout(pieFrame);
    pieFrameLayout->setContentsMargins(15, 15, 15, 15);
    pieFrameLayout->setSpacing(10);

    QHBoxLayout *pieControlLayout = new QHBoxLayout();
    setupDateRangeControl(pieControlLayout, pieStartDateEdit, pieEndDateEdit, "🏷️ 灾害类型分布:");

    connect(pieStartDateEdit, &QDateEdit::dateChanged, this, &index::updateTypeChart);
    connect(pieEndDateEdit, &QDateEdit::dateChanged, this, &index::updateTypeChart);

    pieFrameLayout->addLayout(pieControlLayout);

    pieChart = new QChart();
    pieChart->setTitle("");
    pieChart->setBackgroundVisible(false);
    pieChart->setPlotAreaBackgroundVisible(false);
    pieChart->setAnimationOptions(QChart::SeriesAnimations);
    pieChart->legend()->hide();
    pieChart->setMargins(QMargins(0,0,0,0));

    pieSeries = new QPieSeries();
    pieChart->addSeries(pieSeries);

    // 【关键修复】使用普通函数指针连接，避免 Lambda 类型推导错误
    connect(pieSeries, &QPieSeries::hovered, this, &index::onPieSliceHovered);

    pieChartView = new QChartView(pieChart);
    pieChartView->setRenderHint(QPainter::Antialiasing);
    pieChartView->setMinimumHeight(220);
    pieChartView->setBackgroundBrush(QBrush(QColor("#0f2a4a")));
    pieChartView->setStyleSheet("border: none;");

    pieFrameLayout->addWidget(pieChartView);
    bottomChartLayout->addWidget(pieFrame, 1);

    rightPanelLayout->addLayout(bottomChartLayout);
}

void index::applyGlobalStyle()
{
    this->setStyleSheet(R"(
        QMainWindow { background-color: #050b22; }
        QFrame { background-color: #0a1a3a; border-radius: 12px; }
        QPushButton {
            background-color: #0a2a5a; color: #00d8ff;
            border: 2px solid #0066cc; border-radius: 8px;
        }
        QPushButton:hover { background-color: #004a7c; border-color: #00d8ff; }
        QPushButton:pressed { background-color: #0066cc; }
        QLabel { color: #e0e6f6; font-family: "Microsoft YaHei"; }
    )");
    logoFrame->setStyleSheet("background-color: #0a1a3a; border-radius: 12px;");
    btnNational->setStyleSheet("background-color: #004a7c; border-color: #00d8ff; color: #ffffff;");
}

void index::GetData()
{
    QList<DisasterRecord> records; // 本地变量
    QString err;
    QString targetDateStr = "";

    if (dateSelector) {
        targetDateStr = dateSelector->date().toString("yyyy-MM-dd");
    } else {
        targetDateStr = QDate::currentDate().toString("yyyy-MM-dd");
    }

    // 【修改点】先判断数据库调用结果，再决定做什么
    bool success = DisasterDao::getDisastersByDate(targetDateStr, &records, &err);

    if (!success) {
        qWarning() << "获取数据失败 (" << targetDateStr << "):" << err;
        // 强制清空 records，确保它是真的空且状态健康
        records.clear();
    }

    // 统一调用，传入本地变量 records
    // 无论成功与否，records 现在都是一个合法的 QList（要么有数据，要么被 clear() 过）
    updateLineChart(records);

    // ================= 拉取总览统计数据 =================
    DisasterDao::DashboardStats stats;
    QString statsErr;
    if (DisasterDao::getDashboardStats(&stats, &statsErr)) {
        auto setLabelText = [this](const QString &objName, const QString &text) {
            if (QLabel *lbl = this->findChild<QLabel *>(objName)) {
                lbl->setText(text);
            }
        };

        // 实时监测
        setLabelText("card_monitor_0", QString("尚未分配灾害    %1").arg(stats.unassignedDisasters));
        setLabelText("card_monitor_1", QString("今日已解决灾害    %1").arg(stats.todayResolvedTasks));
        setLabelText("card_monitor_2", QString("正在处理灾害    %1").arg(stats.processingTasks));

        // 时间点统计
        setLabelText("card_time_0", QString("灾害易发生时间段 (h)    %1").arg(stats.proneTimePeriod));

        // 灾害数
        setLabelText("stat_disaster_real", QString("当日实时数  %1").arg(stats.todayDisastersCount));
        setLabelText("stat_disaster_total", QString("本月累计数  %1").arg(stats.monthDisastersCount));

        // 发生率计算
        int todayRt = 0;
        if (stats.yesterdayDisastersCount > 0) {
            todayRt = (stats.todayDisastersCount - stats.yesterdayDisastersCount) * 100 / stats.yesterdayDisastersCount;
        } else if (stats.todayDisastersCount > 0) {
            todayRt = 100;
        }
        QString sign = todayRt > 0 ? "+" : "";
        setLabelText("stat_rate_real", QString("今日  %1%2%").arg(sign).arg(todayRt));
        setLabelText("stat_rate_total", QString("本月  %1").arg(stats.monthDisastersCount));

        // 频繁/罕见灾害
        setLabelText("stat_freq_real", stats.mostFrequentDisaster);
        setLabelText("stat_freq_total", QString("累计数  %1").arg(stats.mostFrequentDisasterCount));

        setLabelText("stat_rare_real", stats.rareDisaster);
        setLabelText("stat_rare_total", QString("累计数  %1").arg(stats.rareDisasterCount));
    } else {
        qWarning() << "获取 Dashboard Stats 失败:" << statsErr;
    }
}

void index::updateLineChart(const QList<DisasterRecord> &records)
{
    if (!lineSeries || !scatterSeries || !axisX || !axisY) return;

        lineSeries->clear();
        scatterSeries->clear();

        // 【修改点】安全清空 map
        if (!pointDataMap.isEmpty()) {
            pointDataMap.clear();
        }

        // 【修改点】再次确认 records 是否真的可用
        if (records.isEmpty()) {
            axisX->setRange(QDateTime::currentDateTime().addDays(-1), QDateTime::currentDateTime());
            axisY->setRange(0, 10);
            return;
        }

        QMap<qint64, int> timeToMaxSeverityMap;
        QMap<qint64, QList<DisasterRecord>> timeToRecordsMap;

        // 【修改点】遍历时加保护
        for (const auto &rec : records) {
            // 防止 rec 内部数据非法导致崩溃（虽然少见，但防一手）
            if (rec.occurredAt.isEmpty()) continue;

            QDateTime dt = QDateTime::fromString(rec.occurredAt, "yyyy-MM-dd HH:mm:ss");
            if (dt.isValid()) {
                qint64 timestamp = dt.toMSecsSinceEpoch();
                if (!timeToMaxSeverityMap.contains(timestamp) || rec.severity > timeToMaxSeverityMap[timestamp]) {
                    timeToMaxSeverityMap[timestamp] = rec.severity;
                }
                timeToRecordsMap[timestamp].append(rec);
            }
        }

        if (timeToMaxSeverityMap.isEmpty()) {
            axisX->setRange(QDateTime::currentDateTime().addDays(-1), QDateTime::currentDateTime());
            axisY->setRange(0, 10);
            return;
        }

        QList<qint64> timestamps = timeToMaxSeverityMap.keys();
        std::sort(timestamps.begin(), timestamps.end());

        qint64 minTime = timestamps.first();
        qint64 maxTime = timestamps.last();

    if (minTime == maxTime) {
        minTime -= 3600000; maxTime += 3600000;
    } else {
        qint64 padding = (maxTime - minTime) * 0.05;
        minTime -= padding; maxTime += padding;
    }
    axisX->setRange(QDateTime::fromMSecsSinceEpoch(minTime), QDateTime::fromMSecsSinceEpoch(maxTime));

    int maxSeverity = 0;
    for (auto it = timeToMaxSeverityMap.begin(); it != timeToMaxSeverityMap.end(); ++it) {
        if (it.value() > maxSeverity) maxSeverity = it.value();
    }
    int yMax = (maxSeverity > 5) ? (maxSeverity + 2) : 10;
    axisY->setRange(0, yMax);

    for (const auto &ts : timestamps) {
        int severity = timeToMaxSeverityMap[ts];
        QPointF point(ts, severity);
        lineSeries->append(point);
        scatterSeries->append(point);
        for (const auto& rec : timeToRecordsMap[ts]) {
            pointDataMap.insert(ts, rec);
        }
    }
}

void index::updateBarChart() {
    if (!barSeries || !barChartView) return;

    QString startStr = barStartDateEdit->date().toString("yyyy-MM-dd");
    QString endStr = barEndDateEdit->date().toString("yyyy-MM-dd");

    QMap<QString, int> stats;
    QString err;

    if (DisasterDao::countDisasterTypesByDateRange(startStr, endStr, &stats, &err)) {
        barSeries->clear();

        QBarSet *set0 = new QBarSet("灾害数量");
        set0->setColor(QColor("#00d8ff"));

        QStringList categories;
        for (auto it = stats.cbegin(); it != stats.cend(); ++it) {
            set0->append(it.value());
            categories << it.key();
        }

        if (categories.isEmpty()) {
            categories << "无数据";
            set0->append(0);
        }

        barSeries->append(set0);

        QBarCategoryAxis *axisX = qobject_cast<QBarCategoryAxis*>(barChart->axes(Qt::Horizontal).first());
        if (axisX) {
            axisX->clear();
            axisX->append(categories);
            axisX->setLabelsAngle(-45);
        }

        QValueAxis *axisY = qobject_cast<QValueAxis*>(barChart->axes(Qt::Vertical).first());
        if (axisY) {
            int maxVal = 0;
            for (int i = 0; i < set0->count(); ++i) {
                double v = set0->at(i);
                if (v > maxVal) maxVal = (int)v;
            }
            axisY->setMax(maxVal > 0 ? maxVal * 1.2 : 10);
        }
    } else {
        qWarning() << "条形图数据加载失败:" << err;
    }
}

void index::updateTypeChart() {
    if (!pieSeries || !pieChartView) return;

    QString startStr = pieStartDateEdit->date().toString("yyyy-MM-dd");
    QString endStr = pieEndDateEdit->date().toString("yyyy-MM-dd");

    QMap<QString, int> stats;
    QString err;

    if (DisasterDao::countDisasterTypesByDateRange(startStr, endStr, &stats, &err)) {
        pieSeries->clear();

        QList<QColor> colors = {
            QColor("#ff4d4d"), QColor("#ffaa00"), QColor("#00d8ff"),
            QColor("#00ff99"), QColor("#aa00ff"), QColor("#ffff00")
        };
        int colorIdx = 0;

        bool hasData = false;
        for (auto it = stats.cbegin(); it != stats.cend(); ++it) {
            if (it.value() > 0) {
                hasData = true;
                QPieSlice *slice = pieSeries->append(it.key(), it.value());
                slice->setColor(colors[colorIdx % colors.count()]);
                slice->setLabelVisible();
                slice->setLabelColor(QColor("#e0e6f6"));
                slice->setLabelFont(QFont("Microsoft YaHei", 10));
                // 初始化标签文本
                slice->setLabel(QString("%1: %2起").arg(it.key()).arg(it.value()));
                colorIdx++;
            }
        }

        if (!hasData) {
            pieSeries->append("无数据", 1)->setColor(QColor("#555555"));
        }

        pieSeries->setPieSize(0.65);

    } else {
        qWarning() << "饼图数据加载失败:" << err;
        pieSeries->clear();
        pieSeries->append("加载失败", 1);
    }
}

// 【新增】独立的槽函数处理饼图悬停事件，解决 Lambda 编译错误
void index::onPieSliceHovered(QPieSlice *slice, bool state)
{
    if (!slice) return;

    if (state) {
        slice->setExploded(true);
        slice->setLabelVisible(true);
        slice->setPen(QPen(QColor("#00d8ff"), 2));
    } else {
        slice->setExploded(false);
        slice->setPen(QPen(QColor("#ffffff"), 1));
    }
}

void index::showTooltip(QPoint pos, const QString &text)
{
    if (!tooltipLabel) return;
    tooltipLabel->setText(text);
    tooltipLabel->setWordWrap(true);
    tooltipLabel->adjustSize();

    QRect screenGeom = QGuiApplication::primaryScreen()->geometry();
    int labelW = tooltipLabel->width();
    int labelH = tooltipLabel->height();

    if (pos.x() + labelW > screenGeom.right()) pos.setX(screenGeom.right() - labelW - 10);
    if (pos.x() < screenGeom.left()) pos.setX(screenGeom.left() + 10);
    if (pos.y() < screenGeom.top()) pos.setY(pos.y() + 100);
    if (pos.y() + labelH > screenGeom.bottom()) pos.setY(screenGeom.bottom() - labelH - 10);

    tooltipLabel->move(pos);
    tooltipLabel->show();
    tooltipLabel->raise();
}

void index::hideTooltip()
{
    if (tooltipLabel) tooltipLabel->hide();
}

void index::histogram(){
    QMap<QString, int> stats;
    QString err;
    if (DisasterDao::countDisasterTypesByDateRange("2025-01-01", "2025-12-31", &stats, &err)) {
        for (auto it = stats.cbegin(); it != stats.cend(); ++it)
            qDebug() << it.key() << ":" << it.value() << "次";
    } else {
        qWarning() << err;
    }
}
