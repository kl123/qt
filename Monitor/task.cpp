#include "task.h"
#include "ui_task.h"
#include "disasterdao.h"
#include <QSettings>
#include <QDebug>
#include <QTableWidget>
#include <QHeaderView>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QColor>
#include <QFont>
#include <QBrush>
#include <QProgressBar>
#include <QInputDialog>
#include <QMessageBox>
#include <QDialogButtonBox>
#include <QSlider>
#include <QLabel>
#include <QApplication>
#include <QScreen>

task::task(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::task)
{
    ui->setupUi(this);

    // 设置窗口默认大小（大尺寸）
    this->resize(1400, 800);
    // 设置窗口最小大小
    this->setMinimumSize(1000, 600);
    // 设置窗口标题
    this->setWindowTitle("任务管理");

    // 设置窗口标志（移除问号，添加全屏/最小化/最大化）
    setupWindowFlags();

    // 初始化 UI 组件
    initUI();

    // 加载数据
    SeekInfo();
}

task::~task()
{
    delete ui;
}

/**
 * @brief 设置窗口标志，移除帮助问号，添加标准窗口控制按钮
 */
void task::setupWindowFlags() {
    Qt::WindowFlags flags = this->windowFlags();
    flags &= ~Qt::WindowContextHelpButtonHint;
    flags |= Qt::WindowMinimizeButtonHint;
    flags |= Qt::WindowMaximizeButtonHint;
    flags |= Qt::WindowCloseButtonHint;
    this->setWindowFlags(flags);
}

/**
 * @brief 初始化表格样式和列结构
 */
void task::initUI() {
    m_table = this->findChild<QTableWidget*>("taskTableWidget");

    if (!m_table) {
        m_table = new QTableWidget(this);
        QVBoxLayout *layout = new QVBoxLayout(this);
        layout->setContentsMargins(10, 10, 10, 10);
        layout->addWidget(m_table);
        this->setLayout(layout);
    }

    m_table->setColumnCount(0);
    m_table->setRowCount(0);
    m_table->setAlternatingRowColors(true);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->horizontalHeader()->setStretchLastSection(true);
    m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    m_table->verticalHeader()->setVisible(false);
    m_table->setWordWrap(false);
    m_table->setCornerButtonEnabled(false);

    // 【关键】设置行高，确保按钮有足够空间
    m_table->verticalHeader()->setDefaultSectionSize(45);

    QString styleSheet = R"(
        QTableWidget {
            background-color: #ffffff;
            border: 1px solid #dcdcdc;
            border-radius: 4px;
            font-size: 13px;
            color: #333333;
            gridline-color: #f0f0f0;
        }
        QTableWidget::item {
            padding: 5px;
            border-bottom: 1px solid #f0f0f0;
        }
        QTableWidget::item:selected {
            background-color: #e3f2fd;
            color: #1565c0;
        }
        QHeaderView::section {
            background-color: #f5f7fa;
            color: #4a5568;
            font-weight: bold;
            padding: 10px;
            border: none;
            border-bottom: 2px solid #cbd5e0;
            font-size: 14px;
        }
        QHeaderView::section:hover {
            background-color: #ebf4ff;
        }
        QPushButton {
            background-color: #4299e1;
            color: white;
            border: none;
            border-radius: 4px;
            padding: 6px 12px;
            font-weight: bold;
            min-width: 80px;
        }
        QPushButton:hover {
            background-color: #3182ce;
        }
        QPushButton:pressed {
            background-color: #2b6cb0;
        }
        QProgressBar {
            border: 1px solid #dcdcdc;
            border-radius: 3px;
            text-align: center;
            font-weight: bold;
            color: #333333;
        }
        QProgressBar::chunk {
            background-color: #4299e1;
            border-radius: 2px;
        }
    )";
    m_table->setStyleSheet(styleSheet);
}

void task::SeekInfo() {
    QSettings settings("System", "disaster");

    qint64 userId = settings.value("userid", 0).toLongLong();
    QString role = settings.value("role", "user").toString();

    qDebug() << "========== 加载任务列表 ==========";
    qDebug() << "当前用户 ID:" << userId << "| 角色:" << role;

    if (userId == 0) {
        qWarning() << "警告：未获取到有效的用户 ID，无法查询任务！";
        if(m_table) {
            m_table->setRowCount(1);
            m_table->setColumnCount(1);
            QTableWidgetItem *item = new QTableWidgetItem("未登录或配置错误");
            item->setTextAlignment(Qt::AlignCenter);
            item->setForeground(QBrush(QColor("red")));
            m_table->setItem(0, 0, item);
            m_table->setSpan(0, 0, 1, 1);
        }
        return;
    }

    QList<DisasterTaskRecord> tasks;
    QString err;

    bool ok = DisasterDao::getTasksForUser(userId, role, 100, 0, &tasks, &err);

    if (!ok) {
        qCritical() << "[错误] 获取任务列表失败！错误信息:" << err;
        return;
    }

    if (!m_table) return;

    m_table->clearContents();
    m_table->setRowCount(0);

    QStringList headers;
    headers << "灾害 ID" << "类型" << "地点" << "等级" << "处置员" << "联系电话" << "进度" << "指派时间";

    int operatorColIndex = -1;
    if (role == "现场处置员" || role == "handler" || role == "worker") {
        headers << "操作";
        operatorColIndex = headers.size() - 1;
    }

    m_table->setColumnCount(headers.size());
    m_table->setHorizontalHeaderLabels(headers);

    if (tasks.isEmpty()) {
        m_table->setRowCount(1);
        QTableWidgetItem *item = new QTableWidgetItem("暂无分配的任务");
        item->setTextAlignment(Qt::AlignCenter);
        item->setFlags(Qt::NoItemFlags);
        m_table->setItem(0, 0, item);
        m_table->setSpan(0, 0, 1, headers.size());
        return;
    }

    m_table->setRowCount(tasks.size());

    for (int i = 0; i < tasks.size(); ++i) {
        const auto &t = tasks[i];

        auto setItem = [&](int col, const QString &text, int align) {
            QTableWidgetItem *item = new QTableWidgetItem(text);
            item->setTextAlignment(static_cast<Qt::AlignmentFlag>(align));
            m_table->setItem(i, col, item);
        };

        setItem(0, QString::number(t.disasterId), Qt::AlignCenter);
        setItem(1, t.disasterType, Qt::AlignLeft | Qt::AlignVCenter);
        setItem(2, t.location, Qt::AlignLeft | Qt::AlignVCenter);

        QString severityText = QString::number(t.severity) + "级";
        QTableWidgetItem *sevItem = new QTableWidgetItem(severityText);
        sevItem->setTextAlignment(Qt::AlignCenter);
        if (t.severity >= 5) sevItem->setForeground(QBrush(QColor("#e53e3e")));
        else if (t.severity >= 3) sevItem->setForeground(QBrush(QColor("#dd6b20")));
        else sevItem->setForeground(QBrush(QColor("#38a169")));
        m_table->setItem(i, 3, sevItem);

        setItem(4, t.handlerName.isEmpty() ? "待指派" : t.handlerName, Qt::AlignCenter);
        setItem(5, t.handlerPhone.isEmpty() ? "-" : t.handlerPhone, Qt::AlignCenter);

        // 进度条
        QProgressBar *progressBar = new QProgressBar();
        progressBar->setRange(0, 100);
        progressBar->setValue(t.progress);
        progressBar->setFormat("%p%");
        progressBar->setFixedHeight(22);
        progressBar->setMinimumWidth(120);
        progressBar->setStyleSheet(R"(
            QProgressBar {
                border: 1px solid #dcdcdc;
                border-radius: 3px;
                text-align: center;
                font-weight: bold;
                color: #333333;
                background-color: #f7fafc;
            }
            QProgressBar::chunk {
                background-color: #4299e1;
                border-radius: 2px;
            }
        )");

        if (t.progress == 100) {
            progressBar->setStyleSheet(R"(
                QProgressBar {
                    border: 1px solid #dcdcdc;
                    border-radius: 3px;
                    text-align: center;
                    font-weight: bold;
                    color: #333333;
                    background-color: #f0fff4;
                }
                QProgressBar::chunk {
                    background-color: #38a169;
                    border-radius: 2px;
                }
            )");
        }

        m_table->setCellWidget(i, 6, progressBar);

        setItem(7, t.assignedAt, Qt::AlignCenter);

        // 【关键修复】操作列按钮
        if (operatorColIndex != -1) {
            QWidget *cellWidget = new QWidget();
            cellWidget->setStyleSheet("background-color: transparent; border: none;");

            QHBoxLayout *layout = new QHBoxLayout(cellWidget);
            // 【关键】设置更小的 margins，确保按钮不被裁剪
            layout->setContentsMargins(4, 2, 4, 2);
            layout->setSpacing(3);

            QPushButton *btn = new QPushButton("更新进度");
            btn->setCursor(Qt::PointingHandCursor);
            // 【关键】使用 setFixedSize 确保按钮大小固定
            btn->setFixedSize(80, 28);
            btn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
            btn->setStyleSheet(R"(
                QPushButton {
                    background-color: #4299e1;
                    color: white;
                    border: none;
                    border-radius: 4px;
                    font-weight: bold;
                    font-size: 12px;
                }
                QPushButton:hover {
                    background-color: #3182ce;
                }
                QPushButton:pressed {
                    background-color: #2b6cb0;
                }
            )");

            connect(btn, &QPushButton::clicked, this, [this, t]() {
                onUpdateProgressClicked(t.id, t.disasterId, t.progress);
            });

            layout->addWidget(btn, 0, Qt::AlignCenter);
            cellWidget->setLayout(layout);

            m_table->setCellWidget(i, operatorColIndex, cellWidget);
        }
    }

    // 【关键】设置固定列宽，确保操作列有足够空间
    m_table->setColumnWidth(0, 70);   // 灾害 ID
    m_table->setColumnWidth(1, 90);   // 类型
    m_table->setColumnWidth(2, 140);  // 地点
    m_table->setColumnWidth(3, 55);   // 等级
    m_table->setColumnWidth(4, 90);   // 处置员
    m_table->setColumnWidth(5, 110);  // 联系电话
    m_table->setColumnWidth(6, 130);  // 进度
    m_table->setColumnWidth(7, 155);  // 指派时间

    if (operatorColIndex != -1) {
        // 【关键】操作列设置更宽，确保按钮完整显示
        m_table->setColumnWidth(operatorColIndex, 100);
    }

    // 【关键】禁用自动调整，保持固定列宽
    m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
}

void task::showProgressDialog(qint64 taskId, qint64 disasterId, int currentProgress) {
    QDialog *dialog = new QDialog(this);
    dialog->setWindowTitle("更新任务进度");
    dialog->setMinimumSize(400, 200);
    dialog->setModal(true);

    QVBoxLayout *mainLayout = new QVBoxLayout(dialog);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    QLabel *infoLabel = new QLabel(QString("灾害 ID: %1\n当前进度：%2%").arg(disasterId).arg(currentProgress));
    infoLabel->setStyleSheet("font-size: 13px; color: #4a5568;");
    mainLayout->addWidget(infoLabel);

    QLabel *sliderLabel = new QLabel("设置新进度：");
    sliderLabel->setStyleSheet("font-size: 13px; font-weight: bold;");
    mainLayout->addWidget(sliderLabel);

    QSlider *slider = new QSlider(Qt::Horizontal);
    slider->setRange(0, 100);
    slider->setValue(currentProgress);
    slider->setTickPosition(QSlider::TicksBelow);
    slider->setTickInterval(10);
    slider->setStyleSheet(R"(
        QSlider::groove:horizontal {
            height: 8px;
            background: #e2e8f0;
            border-radius: 4px;
        }
        QSlider::handle:horizontal {
            background: #4299e1;
            width: 18px;
            margin: -5px 0;
            border-radius: 9px;
        }
        QSlider::sub-page:horizontal {
            background: #4299e1;
            border-radius: 4px;
        }
    )");
    mainLayout->addWidget(slider);

    QLabel *progressValueLabel = new QLabel(QString("%1%").arg(currentProgress));
    progressValueLabel->setAlignment(Qt::AlignCenter);
    progressValueLabel->setStyleSheet("font-size: 24px; font-weight: bold; color: #2b6cb0;");
    mainLayout->addWidget(progressValueLabel);

    connect(slider, &QSlider::valueChanged, this, [progressValueLabel](int value) {
        progressValueLabel->setText(QString("%1%").arg(value));
    });

    QDialogButtonBox *buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    buttonBox->button(QDialogButtonBox::Ok)->setText("确认更新");
    buttonBox->button(QDialogButtonBox::Cancel)->setText("取消");

    connect(buttonBox, &QDialogButtonBox::accepted, dialog, [this, dialog, taskId, slider]() {
        dialog->accept();
        onConfirmProgressUpdate(taskId, slider->value());
    });
    connect(buttonBox, &QDialogButtonBox::rejected, dialog, &QDialog::reject);

    mainLayout->addWidget(buttonBox);

    dialog->exec();
    dialog->deleteLater();
}

void task::onUpdateProgressClicked(qint64 taskId, qint64 disasterId, int currentProgress) {
    qDebug() << "[UI 交互] 用户点击更新进度 -> 任务 ID:" << taskId
             << ", 灾害 ID:" << disasterId
             << ", 当前进度:" << currentProgress << "%";

    showProgressDialog(taskId, disasterId, currentProgress);
}

void task::onConfirmProgressUpdate(qint64 taskId, int newProgress) {
    qDebug() << "[UI 交互] 确认更新进度 -> 任务 ID:" << taskId
             << ", 新进度:" << newProgress << "%";

    QString errorMessage;
    bool success = DisasterDao::updateDisasterTaskProgress(taskId, newProgress, &errorMessage);

    if (success) {
        QMessageBox::information(this, "成功",
            QString("任务进度已更新为 %1%").arg(newProgress));
        SeekInfo();
    } else {
        QMessageBox::critical(this, "失败",
            QString("更新进度失败：%1").arg(errorMessage.isEmpty() ? "未知错误" : errorMessage));
    }
}
