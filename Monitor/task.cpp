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

task::task(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::task)
{
    ui->setupUi(this);

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
 * @brief 初始化表格样式和列结构
 */
void task::initUI() {
    // 1. 获取或创建表格控件
    m_table = this->findChild<QTableWidget*>("taskTableWidget");

    if (!m_table) {
        m_table = new QTableWidget(this);
        // 如果是在代码中创建的，需要设置布局
        QVBoxLayout *layout = new QVBoxLayout(this);
        layout->addWidget(m_table);
        this->setLayout(layout);
    }

    // 2. 设置表格基本属性
    m_table->setColumnCount(0); // 先清空
    m_table->setRowCount(0);
    m_table->setAlternatingRowColors(true); // 开启交替行颜色
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows); // 选中整行
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers); // 不可直接编辑单元格
    m_table->horizontalHeader()->setStretchLastSection(true); // 最后一列自动拉伸
    m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive); // 允许手动调整列宽
    m_table->verticalHeader()->setVisible(false); // 隐藏行号

    // 3. 应用美观的 QSS 样式表
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
            padding: 8px;
            border-bottom: 1px solid #f0f0f0;
        }
        QTableWidget::item:selected {
            background-color: #e3f2fd; /* 选中行淡蓝色 */
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
    headers << "灾害ID" << "类型" << "地点" << "等级" << "处置员" << "联系电话" << "进度" << "指派时间";

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

        // 【关键修改】: 将 align 参数类型改为 int，避免枚举类型推导错误
        auto setItem = [&](int col, const QString &text, int align) {
            QTableWidgetItem *item = new QTableWidgetItem(text);
            item->setTextAlignment(static_cast<Qt::AlignmentFlag>(align));
            m_table->setItem(i, col, item);
        };

        // 填充数据
        setItem(0, QString::number(t.disasterId), Qt::AlignCenter);
        setItem(1, t.disasterType, Qt::AlignLeft | Qt::AlignVCenter);
        setItem(2, t.location, Qt::AlignLeft | Qt::AlignVCenter);

        // 等级带颜色显示
        QString severityText = QString::number(t.severity) + "级";
        QTableWidgetItem *sevItem = new QTableWidgetItem(severityText);
        sevItem->setTextAlignment(Qt::AlignCenter);
        if (t.severity >= 5) sevItem->setForeground(QBrush(QColor("#e53e3e")));
        else if (t.severity >= 3) sevItem->setForeground(QBrush(QColor("#dd6b20")));
        else sevItem->setForeground(QBrush(QColor("#38a169")));
        m_table->setItem(i, 3, sevItem);

        setItem(4, t.handlerName.isEmpty() ? "待指派" : t.handlerName, Qt::AlignCenter);
        setItem(5, t.handlerPhone.isEmpty() ? "-" : t.handlerPhone, Qt::AlignCenter);

        // 进度
        QString progressText = QString::number(t.progress) + "%";
        QTableWidgetItem *progItem = new QTableWidgetItem(progressText);
        progItem->setTextAlignment(Qt::AlignCenter);
        if (t.progress == 100) progItem->setForeground(QBrush(QColor("#38a169")));
        m_table->setItem(i, 6, progItem);

        setItem(7, t.assignedAt, Qt::AlignCenter);

        // 按钮列
        if (operatorColIndex != -1) {
            QWidget *cellWidget = new QWidget();
            QHBoxLayout *layout = new QHBoxLayout(cellWidget);
            layout->setContentsMargins(5, 5, 5, 5);

            QPushButton *btn = new QPushButton("更新进度");
            btn->setCursor(Qt::PointingHandCursor);
            btn->setFixedSize(90, 30);

            connect(btn, &QPushButton::clicked, this, [this, t]() {
                onUpdateProgressClicked(t.id, t.disasterId, t.progress);
            });

            layout->addWidget(btn);
            layout->addStretch();
            cellWidget->setLayout(layout);

            m_table->setCellWidget(i, operatorColIndex, cellWidget);
        }
    }

    m_table->resizeColumnsToContents();
    for(int c=0; c<m_table->columnCount(); ++c) {
        if (m_table->columnWidth(c) < 80) m_table->setColumnWidth(c, 80);
    }
    if (operatorColIndex != -1) {
        m_table->setColumnWidth(operatorColIndex, 120);
    }
}

/**
 * @brief 按钮点击槽函数 (占位)
 */
void task::onUpdateProgressClicked(qint64 taskId, qint64 disasterId, int currentProgress) {
    qDebug() << "[UI 交互] 用户点击更新进度 -> 任务 ID:" << taskId
             << ", 灾害 ID:" << disasterId
             << ", 当前进度:" << currentProgress << "%";

    // TODO: 这里后续可以弹出对话框让用户输入新进度
    // QMessageBox::inputInt(...) 或者自定义 Dialog
    // 然后调用 DisasterDao::updateDisasterTaskProgress(taskId, newProgress, &err);
}
