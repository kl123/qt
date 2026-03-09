#include "overview.h"
#include "ui_overview.h"
#include <QHeaderView>
#include <QTableWidgetItem>
#include <QFont>
#include <QColor>
#include "disasterdao.h"
#include "userauth.h"
#include <QDebug>
#include <QSettings>
#include <QPushButton>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QListWidget>
#include <QLabel>
#include <QDialog>
#include <QVBoxLayout>
#include <QTimer>

overview::overview(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::overview)
{
    ui->setupUi(this);
    this->setWindowTitle("灾情总览");
    this->resize(1200, 600);

    // 初始化主布局
    mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(10);

    // 初始化表格UI
    initTableUI();

    // 加载真实数据
    InitialData();

    // 预加载员工信息（可选，主要用于调试确认数据连通性）
    loadEmee();
}

overview::~overview()
{
    delete ui;
    // Qt 父子对象机制会自动管理 mainLayout 和 disasterTable 的内存
}

void overview::initTableUI()
{
    // 1. 创建表格，列数改为 9 (原 8 列 + 1 列操作)
    disasterTable = new QTableWidget(0, 9, this);

    QStringList headers = {
        "ID", "灾害类型", "发生地点", "发生时间",
        "灾害详情", "严重程度", "调度员ID", "记录时间", "操作"
    };
    disasterTable->setHorizontalHeaderLabels(headers);

    // 2. 表格样式美化
    disasterTable->setStyleSheet(R"(
        QTableWidget {
            border: 1px solid #E0E0E0;
            border-radius: 8px;
            gridline-color: #E0E0E0;
            background-color: #FFFFFF;
            font-size: 13px;
            color: #333333;
        }
        QTableWidget::horizontalHeader {
            background-color: #2196F3;
            color: #FFFFFF;
            font-weight: bold;
            font-size: 14px;
            height: 35px;
        }
        QTableWidget::horizontalHeader::section {
            border: none;
            padding: 8px;
            border-right: 1px solid #1976D2;
        }
        QTableWidget::horizontalHeader::section:last {
            border-right: none;
        }
        QTableWidget::item:even { background-color: #F5F7FA; }
        QTableWidget::item:odd { background-color: #FFFFFF; }
        QTableWidget::item:selected {
            background-color: #BBDEFB;
            color: #1976D2;
        }
        /* 按钮在表格中的基础样式 */
        QPushButton {
            border: none;
            color: white;
            padding: 4px 8px;
            border-radius: 4px;
            font-size: 12px;
            margin: 2px;
        }
        QPushButton#btnAssign { background-color: #4CAF50; }
        QPushButton#btnAssign:hover { background-color: #45a049; }
        QPushButton#btnEdit { background-color: #2196F3; }
        QPushButton#btnEdit:hover { background-color: #0b7dda; }
        QPushButton#btnDelete { background-color: #f44336; }
        QPushButton#btnDelete:hover { background-color: #da190b; }
    )");

    // 3. 表格功能配置
    disasterTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    disasterTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    disasterTable->setSelectionMode(QAbstractItemView::SingleSelection);
    disasterTable->verticalHeader()->setVisible(false);
    disasterTable->horizontalHeader()->setStretchLastSection(false);

    // 列宽策略
    for (int i = 0; i < 8; ++i) {
        if (i == 4) {
            disasterTable->horizontalHeader()->setSectionResizeMode(i, QHeaderView::Stretch);
        } else {
            disasterTable->horizontalHeader()->setSectionResizeMode(i, QHeaderView::ResizeToContents);
        }
    }
    disasterTable->horizontalHeader()->setSectionResizeMode(8, QHeaderView::Fixed);
    disasterTable->setColumnWidth(8, 180);

    // 4. 将表格加入主布局
    mainLayout->addWidget(disasterTable);
}

void overview::InitialData()
{
    disasterTable->setRowCount(0);

    QSettings settings("System", "disaster");
    qint64 userId = settings.value("userid", 0).toLongLong();

    QList<DisasterRecord> unassigned;
    QString err;

    bool querySuccess = DisasterDao::getUnassignedDisastersByDispatcher(userId, 50, 0, &unassigned, &err);

    if (!querySuccess) {
        qDebug() << "查询失败:" << err;
        QMessageBox::warning(this, "错误", "加载灾情数据失败:\n" + err);
        return;
    }

    if (unassigned.isEmpty()) {
        qDebug() << "暂无未分配的灾情记录。";
        return;
    }

    for (int i = 0; i < unassigned.size(); ++i) {
        const DisasterRecord& record = unassigned.at(i);
        int row = disasterTable->rowCount();
        disasterTable->insertRow(row);

        auto createItem = [this](const QString &text) {
            QTableWidgetItem *item = new QTableWidgetItem(text);
            item->setTextAlignment(Qt::AlignCenter);
            item->setFlags(item->flags() & ~Qt::ItemIsEditable);
            return item;
        };

        disasterTable->setItem(row, 0, createItem(QString::number(record.id)));
        disasterTable->setItem(row, 1, createItem(record.disasterType));
        disasterTable->setItem(row, 2, createItem(record.location));
        disasterTable->setItem(row, 3, createItem(record.occurredAt));

        QTableWidgetItem *contentItem = createItem(record.content);
        if (record.content.length() > 20) {
            contentItem->setToolTip(record.content);
        }
        disasterTable->setItem(row, 4, contentItem);

        QString severityText;
        switch (record.severity) {
            case 0: severityText = "一般"; break;
            case 1: severityText = "较轻"; break;
            case 2: severityText = "中等"; break;
            case 3: severityText = "较重"; break;
            case 4: severityText = "严重"; break;
            case 5: severityText = "特重"; break;
            default: severityText = "未知";
        }
        QTableWidgetItem *sevItem = createItem(severityText);
        if (record.severity >= 4) sevItem->setForeground(QBrush(QColor("#d32f2f")));
        else if (record.severity >= 2) sevItem->setForeground(QBrush(QColor("#f57c00")));
        disasterTable->setItem(row, 5, sevItem);

        disasterTable->setItem(row, 6, createItem(QString::number(record.dispatcherId)));
        disasterTable->setItem(row, 7, createItem(record.createdAt));

        QWidget *actionWidget = createActionWidget(record.id);
        disasterTable->setCellWidget(row, 8, actionWidget);
        disasterTable->setRowHeight(row, 45);
    }
    qDebug() << "数据加载完成，共" << unassigned.size() << "条记录。";
}

void overview::fillFakeData()
{
    // 保留备用
}

QWidget* overview::createActionWidget(qint64 id)
{
    QWidget *container = new QWidget();
    QHBoxLayout *layout = new QHBoxLayout(container);
    layout->setContentsMargins(4, 2, 4, 2);
    layout->setSpacing(4);

    QPushButton *btnAssign = new QPushButton("指派");
    btnAssign->setObjectName("btnAssign");
    btnAssign->setProperty("recordId", id);
    connect(btnAssign, &QPushButton::clicked, this, &overview::onAssignClicked);

    QPushButton *btnEdit = new QPushButton("修改");
    btnEdit->setObjectName("btnEdit");
    btnEdit->setProperty("recordId", id);
    connect(btnEdit, &QPushButton::clicked, this, &overview::onEditClicked);

    QPushButton *btnDelete = new QPushButton("删除");
    btnDelete->setObjectName("btnDelete");
    btnDelete->setProperty("recordId", id);
    connect(btnDelete, &QPushButton::clicked, this, &overview::onDeleteClicked);

    layout->addWidget(btnAssign);
    layout->addWidget(btnEdit);
    layout->addWidget(btnDelete);
    container->setLayout(layout);
    return container;
}

// ================= 核心逻辑：多选指派实现 =================

void overview::performAssignTask(qint64 disasterId)
{
    // 1. 获取当前调度员ID
    QSettings settings("System", "disaster");
    qint64 currentUserId = settings.value("userid", 0).toLongLong();
    if (currentUserId == 0) {
        QMessageBox::critical(this, "错误", "未获取到当前用户 ID，无法指派！");
        return;
    }

    // 2. 动态创建临时对话框
    QDialog *dialog = new QDialog(this);
    dialog->setWindowTitle("指派处置员");
    dialog->setModal(true);
    dialog->resize(400, 500);

    QVBoxLayout *dlgLayout = new QVBoxLayout(dialog);
    dlgLayout->setContentsMargins(20, 20, 20, 20);
    dlgLayout->setSpacing(15);

    // 提示信息
    QLabel *infoLabel = new QLabel(QString("正在为 <b>灾情 ID: %1</b> 指派处置员：<br><span style='color:#666'>按住 Ctrl 键可多选</span>").arg(disasterId));
    infoLabel->setWordWrap(true);
    infoLabel->setStyleSheet("font-size: 14px;");
    dlgLayout->addWidget(infoLabel);

    // 多选列表
    QListWidget *listWidget = new QListWidget();
    listWidget->setSelectionMode(QAbstractItemView::ExtendedSelection); // 关键：允许多选
    listWidget->setStyleSheet(R"(
        QListWidget { border: 1px solid #ddd; border-radius: 4px; font-size: 13px; }
        QListWidget::item:selected { background-color: #2196F3; color: white; }
        QListWidget::item:hover { background-color: #E3F2FD; }
    )");
    dlgLayout->addWidget(listWidget);

    // 加载处置员数据
    QList<AuthUser> handlers;
    QString err;
    if (UserAuth::getHandlersByDispatcherId(currentUserId, &handlers, &err)) {
        if (handlers.isEmpty()) {
            listWidget->addItem("暂无可用处置员");
            listWidget->item(0)->setFlags(Qt::NoItemFlags);
        } else {
            for (const auto& h : handlers) {
                QListWidgetItem *item = new QListWidgetItem(QString("%1  (%2)").arg(h.username).arg(h.phone));
                item->setData(Qt::UserRole, h.id); // 将 ID 绑定到 item
                listWidget->addItem(item);
            }
        }
    } else {
        QMessageBox::critical(dialog, "错误", "加载处置员列表失败:\n" + err);
        delete dialog;
        return;
    }

    // 按钮区域
    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->addStretch();

    QPushButton *btnCancel = new QPushButton("取消");
    QPushButton *btnConfirm = new QPushButton("确认指派");
    btnConfirm->setStyleSheet("background-color: #4CAF50; color: white; font-weight: bold; min-width: 80px;");

    btnLayout->addWidget(btnCancel);
    btnLayout->addWidget(btnConfirm);
    dlgLayout->addLayout(btnLayout);

    // 连接信号
    connect(btnCancel, &QPushButton::clicked, dialog, &QDialog::reject);
    connect(btnConfirm, &QPushButton::clicked, dialog, [dialog, listWidget, disasterId, this]() {
        QList<QListWidgetItem*> selectedItems = listWidget->selectedItems();
        if (selectedItems.isEmpty()) {
            QMessageBox::warning(dialog, "提示", "请至少选择一名处置员！");
            return;
        }

        // 提取 ID 列表
        QList<qint64> handlerIds;
        for (QListWidgetItem *item : selectedItems) {
            handlerIds.append(item->data(Qt::UserRole).toLongLong());
        }

        // 调用数据库接口
        QString err;
        // 假设接口定义：bool assignDisasterTasks(qint64 disasterId, QList<qint64> handlerIds, QString* err)
        bool success = DisasterDao::assignDisasterTasks(disasterId, handlerIds, &err);

        if (success) {
            QMessageBox::information(dialog, "成功",
                QString("已成功将灾情 ID %1 指派给 %2 位处置员！").arg(disasterId).arg(handlerIds.size()));
            dialog->accept();

            // 刷新主界面表格
            // 使用 QTimer 避免在对话框销毁过程中立即刷新可能带来的焦点问题
            QTimer::singleShot(100, this, &overview::InitialData);
        } else {
            QMessageBox::critical(dialog, "失败", "指派操作失败:\n" + err);
        }
    });

    // 显示对话框
    if (dialog->exec() == QDialog::Accepted) {
        qDebug() << "指派流程完成";
    }
    dialog->deleteLater(); // 清理内存
}

// ================= 槽函数实现 =================

void overview::onAssignClicked()
{
    QPushButton *btn = qobject_cast<QPushButton*>(sender());
    if (!btn) return;

    qint64 id = btn->property("recordId").toLongLong();
    qDebug() << "点击了指派按钮，记录 ID:" << id;

    // 调用封装好的指派逻辑
    performAssignTask(id);
}

void overview::onEditClicked()
{
    QPushButton *btn = qobject_cast<QPushButton*>(sender());
    if (!btn) return;
    qint64 id = btn->property("recordId").toLongLong();
    QMessageBox::information(this, "提示", QString("正在编辑灾情 ID: %1").arg(id));
    // TODO: 打开编辑窗口
}

void overview::onDeleteClicked()
{
    QPushButton *btn = qobject_cast<QPushButton*>(sender());
    if (!btn) return;

    qint64 id = btn->property("recordId").toLongLong();

    int ret = QMessageBox::question(this, "确认删除",
                                    QString("确定要删除灾情记录 ID: %1 吗？此操作不可恢复。").arg(id),
                                    QMessageBox::Yes | QMessageBox::No);

    if (ret == QMessageBox::Yes) {
        QString err;
        if (DisasterDao::deleteDisaster(id, &err)) {
            // 查找并移除行
            int targetRow = -1;
            for (int i = 0; i < disasterTable->rowCount(); ++i) {
                QTableWidgetItem *idItem = disasterTable->item(i, 0);
                if (idItem && idItem->text().toLongLong() == id) {
                    targetRow = i;
                    break;
                }
            }

            if (targetRow != -1) {
                disasterTable->removeRow(targetRow);
                if (disasterTable->rowCount() == 0) {
                     QMessageBox::information(this, "提示", "列表已清空。");
                }
            } else {
                // 如果没找到行（极端情况），刷新整个表
                InitialData();
            }
        } else {
            QMessageBox::critical(this, "错误", "删除失败:\n" + err);
        }
    }
}

void overview::loadEmee()
{
    QSettings settings("System", "disaster");
    qint64 userId = settings.value("userid", 0).toLongLong();
    QList<AuthUser> handlers;
    QString err;
    if (UserAuth::getHandlersByDispatcherId(userId, &handlers, &err)) {
        qDebug() << "加载处置员成功，数量:" << handlers.size();
        // 这里仅做调试输出，实际列表是在点击指派时动态加载的
    } else {
        qDebug() << "加载处置员失败:" << err;
    }
}
