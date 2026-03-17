#include "overview.h"
#include "ui_overview.h"
#include <QHeaderView>
#include <QTableWidgetItem>
#include <QFont>
#include <QColor>
#include <QDebug>
#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QListWidget>
#include <QLabel>
#include <QTimer>
#include <QMessageBox>
#include <QSettings>
#include <QDateTime>

// ================= 内部类：添加灾害对话框 =================
// 直接在 cpp 中定义，无需额外头文件
class AddDisasterDialog : public QDialog {
public:
    explicit AddDisasterDialog(QWidget *parent = nullptr) : QDialog(parent) {
        setWindowTitle("新增灾情记录");
        setModal(true);
        resize(500, 450);

        QVBoxLayout *mainLayout = new QVBoxLayout(this);
        mainLayout->setSpacing(15);
        mainLayout->setContentsMargins(20, 20, 20, 20);

        // 标题提示
        QLabel *titleLabel = new QLabel("请填写详细的灾情信息：");
        titleLabel->setStyleSheet("font-weight: bold; font-size: 14px; color: #333;");
        mainLayout->addWidget(titleLabel);

        QFormLayout *formLayout = new QFormLayout();
        formLayout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
        formLayout->setLabelAlignment(Qt::AlignRight);
        formLayout->setVerticalSpacing(12);

        // 1. 灾害类型
        comboType = new QComboBox();
        comboType->addItems({"火灾", "水灾", "地震", "台风", "泥石流", "交通事故", "危化品泄漏", "其他"});
        comboType->setCurrentText("火灾");
        comboType->setMinimumHeight(30);
        formLayout->addRow("灾害类型:", comboType);

        // 2. 发生地点
        editLocation = new QLineEdit();
        editLocation->setPlaceholderText("例如：XX省XX市XX区XX街道...");
        editLocation->setMinimumHeight(30);
        formLayout->addRow("发生地点:", editLocation);

        // 3. 发生时间
        editTime = new QDateTimeEdit();
        editTime->setDateTime(QDateTime::currentDateTime());
        editTime->setDisplayFormat("yyyy-MM-dd HH:mm:ss");
        editTime->setCalendarPopup(true);
        editTime->setMinimumHeight(30);
        formLayout->addRow("发生时间:", editTime);

        // 4. 严重程度 (0-5)
        spinSeverity = new QSpinBox();
        spinSeverity->setRange(0, 5);
        spinSeverity->setSuffix(" 级");
        spinSeverity->setValue(1);
        spinSeverity->setMinimumHeight(30);
        formLayout->addRow("严重程度:", spinSeverity);

        // 5. 灾害详情
        editContent = new QLineEdit();
        editContent->setPlaceholderText("简要描述灾害现场情况、伤亡预估等...");
        editContent->setMinimumHeight(30);
        formLayout->addRow("灾害详情:", editContent);

        mainLayout->addLayout(formLayout);
        mainLayout->addStretch();

        // 按钮区域
        QHBoxLayout *btnLayout = new QHBoxLayout();
        btnLayout->addStretch();

        QPushButton *btnCancel = new QPushButton("取消");
        btnCancel->setMinimumWidth(80);
        btnCancel->setMinimumHeight(35);

        QPushButton *btnConfirm = new QPushButton("确认添加");
        btnConfirm->setMinimumWidth(80);
        btnConfirm->setMinimumHeight(35);
        btnConfirm->setStyleSheet(R"(
            QPushButton {
                background-color: #4CAF50;
                color: white;
                font-weight: bold;
                border-radius: 4px;
            }
            QPushButton:hover { background-color: #45a049; }
            QPushButton:pressed { background-color: #388E3C; }
        )");

        connect(btnCancel, &QPushButton::clicked, this, &QDialog::reject);
        connect(btnConfirm, &QPushButton::clicked, this, [this]() {
            if (editLocation->text().trimmed().isEmpty()) {
                QMessageBox::warning(this, "验证失败", "发生地点不能为空！");
                return;
            }
            if (editContent->text().trimmed().isEmpty()) {
                QMessageBox::warning(this, "验证失败", "灾害详情不能为空！");
                return;
            }
            accept();
        });

        btnLayout->addWidget(btnCancel);
        btnLayout->addWidget(btnConfirm);
        mainLayout->addLayout(btnLayout);
    }

    // 获取填充好的结构体
    DisasterRecord getRecord() const {
        DisasterRecord rec;
        rec.id = 0; // 新建时ID为0，由数据库生成
        rec.disasterType = comboType->currentText();
        rec.location = editLocation->text().trimmed();
        rec.occurredAt = editTime->dateTime().toString("yyyy-MM-dd HH:mm:ss");
        rec.content = editContent->text().trimmed();
        rec.severity = spinSeverity->value();
        rec.isDisaster = true;
        rec.dispatcherId = 0; // 默认未分配，或由调用者设置
        rec.createdAt = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
        rec.systemAlarmAt = rec.createdAt;
        return rec;
    }

private:
    QComboBox *comboType;
    QLineEdit *editLocation;
    QDateTimeEdit *editTime;
    QSpinBox *spinSeverity;
    QLineEdit *editContent;
};

// ================= overview 类实现 =================

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

    // --- 新增：顶部工具栏 ---
    QHBoxLayout *topBarLayout = new QHBoxLayout();
    topBarLayout->setContentsMargins(0, 0, 0, 10);

    QLabel *titleLabel = new QLabel("<h3 style='margin:0;'>未分配灾情列表</h3>");
    titleLabel->setStyleSheet("color: #333;");

    QPushButton *btnAdd = new QPushButton("+ 新增灾情");
    btnAdd->setObjectName("btnAdd");
    btnAdd->setCursor(Qt::PointingHandCursor);
    btnAdd->setMinimumHeight(35);
    btnAdd->setMinimumWidth(120);
    btnAdd->setStyleSheet(R"(
        QPushButton#btnAdd {
            background-color: #2196F3;
            color: white;
            font-weight: bold;
            border-radius: 4px;
            border: none;
        }
        QPushButton#btnAdd:hover { background-color: #1976D2; }
        QPushButton#btnAdd:pressed { background-color: #0D47A1; }
    )");

    connect(btnAdd, &QPushButton::clicked, this, &overview::onAddDisasterClicked);

    topBarLayout->addWidget(titleLabel);
    topBarLayout->addStretch();
    topBarLayout->addWidget(btnAdd);

    mainLayout->addLayout(topBarLayout);
    // -----------------------

    // 初始化表格UI
    initTableUI();

    // 加载真实数据
    InitialData();

    // 预加载员工信息
    loadEmee();
}

overview::~overview()
{
    delete ui;
}

void overview::initTableUI()
{
    // 1. 创建表格，列数 9
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
            alternate-background-color: #F5F7FA;
        }
        QTableWidget::horizontalHeader {
            background-color: #2196F3;
            color: #FFFFFF;
            font-weight: bold;
            font-size: 14px;
            height: 35px;
            border: none;
        }
        QTableWidget::horizontalHeader::section {
            border: none;
            padding: 8px;
            border-right: 1px solid #1976D2;
        }
        QTableWidget::horizontalHeader::section:last {
            border-right: none;
        }
        QTableWidget::item:selected {
            background-color: #BBDEFB;
            color: #1976D2;
        }
        QPushButton {
            border: none;
            color: white;
            padding: 4px 8px;
            border-radius: 4px;
            font-size: 12px;
            margin: 2px;
            cursor: pointer;
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
        if (i == 4) { // 灾害详情列拉伸
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

    // 获取未分配的灾情
    bool querySuccess = DisasterDao::getUnassignedDisastersByDispatcher(userId, 50, 0, &unassigned, &err);

    if (!querySuccess) {
        qDebug() << "查询失败:" << err;
        // 仅在非空错误时提示，避免频繁打扰
        if (!err.isEmpty()) {
             QMessageBox::warning(this, "错误", "加载灾情数据失败:\n" + err);
        }
        return;
    }

    if (unassigned.isEmpty()) {
        qDebug() << "暂无未分配的灾情记录。";
        // 可选：添加一行提示文字
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
        QColor severityColor;
        switch (record.severity) {
            case 0: severityText = "一般"; break;
            case 1: severityText = "较轻"; break;
            case 2: severityText = "中等"; severityColor = QColor("#f57c00"); break;
            case 3: severityText = "较重"; severityColor = QColor("#f57c00"); break;
            case 4: severityText = "严重"; severityColor = QColor("#d32f2f"); break;
            case 5: severityText = "特重"; severityColor = QColor("#b71c1c"); break;
            default: severityText = "未知";
        }

        QTableWidgetItem *sevItem = createItem(severityText);
        if (record.severity >= 2) {
            sevItem->setForeground(QBrush(severityColor));
            QFont font = sevItem->font();
            font.setBold(true);
            sevItem->setFont(font);
        }
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
    // 保留备用，目前不使用
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

void overview::performAssignTask(qint64 disasterId)
{
    QSettings settings("System", "disaster");
    qint64 currentUserId = settings.value("userid", 0).toLongLong();
    if (currentUserId == 0) {
        QMessageBox::critical(this, "错误", "未获取到当前用户 ID，无法指派！");
        return;
    }

    QDialog *dialog = new QDialog(this);
    dialog->setWindowTitle("指派处置员");
    dialog->setModal(true);
    dialog->resize(400, 500);

    QVBoxLayout *dlgLayout = new QVBoxLayout(dialog);
    dlgLayout->setContentsMargins(20, 20, 20, 20);
    dlgLayout->setSpacing(15);

    QLabel *infoLabel = new QLabel(QString("正在为 <b>灾情 ID: %1</b> 指派处置员：<br><span style='color:#666'>按住 Ctrl 键可多选</span>").arg(disasterId));
    infoLabel->setWordWrap(true);
    infoLabel->setStyleSheet("font-size: 14px;");
    dlgLayout->addWidget(infoLabel);

    QListWidget *listWidget = new QListWidget();
    listWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
    listWidget->setStyleSheet(R"(
        QListWidget { border: 1px solid #ddd; border-radius: 4px; font-size: 13px; }
        QListWidget::item:selected { background-color: #2196F3; color: white; }
        QListWidget::item:hover { background-color: #E3F2FD; }
    )");
    dlgLayout->addWidget(listWidget);

    QList<AuthUser> handlers;
    QString err;
    if (UserAuth::getHandlersByDispatcherId(currentUserId, &handlers, &err)) {
        if (handlers.isEmpty()) {
            QListWidgetItem *item = new QListWidgetItem("暂无可用处置员");
            item->setFlags(Qt::NoItemFlags);
            listWidget->addItem(item);
        } else {
            for (const auto& h : handlers) {
                QListWidgetItem *item = new QListWidgetItem(QString("%1  (%2)").arg(h.username).arg(h.phone));
                item->setData(Qt::UserRole, h.id);
                listWidget->addItem(item);
            }
        }
    } else {
        QMessageBox::critical(dialog, "错误", "加载处置员列表失败:\n" + err);
        delete dialog;
        return;
    }

    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->addStretch();

    QPushButton *btnCancel = new QPushButton("取消");
    QPushButton *btnConfirm = new QPushButton("确认指派");
    btnConfirm->setStyleSheet("background-color: #4CAF50; color: white; font-weight: bold; min-width: 80px; padding: 6px 12px; border-radius: 4px;");

    btnLayout->addWidget(btnCancel);
    btnLayout->addWidget(btnConfirm);
    dlgLayout->addLayout(btnLayout);

    connect(btnCancel, &QPushButton::clicked, dialog, &QDialog::reject);
    connect(btnConfirm, &QPushButton::clicked, dialog, [dialog, listWidget, disasterId, this]() {
        QList<QListWidgetItem*> selectedItems = listWidget->selectedItems();
        if (selectedItems.isEmpty()) {
            QMessageBox::warning(dialog, "提示", "请至少选择一名处置员！");
            return;
        }

        QList<qint64> handlerIds;
        for (QListWidgetItem *item : selectedItems) {
            handlerIds.append(item->data(Qt::UserRole).toLongLong());
        }

        QString err;
        bool success = DisasterDao::assignDisasterTasks(disasterId, handlerIds, &err);

        if (success) {
            QMessageBox::information(dialog, "成功",
                QString("已成功将灾情 ID %1 指派给 %2 位处置员！").arg(disasterId).arg(handlerIds.size()));
            dialog->accept();
            QTimer::singleShot(100, this, &overview::InitialData);
        } else {
            QMessageBox::critical(dialog, "失败", "指派操作失败:\n" + err);
        }
    });

    if (dialog->exec() == QDialog::Accepted) {
        qDebug() << "指派流程完成";
    }
    dialog->deleteLater();
}

// ================= 槽函数实现 =================

void overview::onAddDisasterClicked()
{
    AddDisasterDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        DisasterRecord newRecord = dialog.getRecord();

        // 获取当前用户ID作为调度员ID（如果需要自动关联当前人为调度员）
        QSettings settings("System", "disaster");
        qint64 currentUserId = settings.value("userid", 0).toLongLong();
        if (currentUserId != 0) {
            newRecord.dispatcherId = currentUserId;
        }

        // 调用核心逻辑保存
        addInfo(newRecord);
    }
}

void overview::addInfo(const DisasterRecord &record)
{
    qint64 newId = 0;
    QString err;

    if (DisasterDao::createDisaster(record, &newId, &err)) {
        qDebug() << "灾害添加成功，新 ID:" << newId;

        QMessageBox::information(this, "成功",
            QString("灾情记录已添加！\n类型：%1\n地点：%2").arg(record.disasterType).arg(record.location));

        refreshTable();
    } else {
        qDebug() << "灾害添加失败:" << err;
        QMessageBox::critical(this, "错误", "添加灾情失败:\n" + err);
    }
}

// 兼容旧调用的空实现，建议不再使用
void overview::addInfo() {
    qWarning() << "调用了无参 addInfo()，请使用带参数的版本。";
}

void overview::onAssignClicked()
{
    QPushButton *btn = qobject_cast<QPushButton*>(sender());
    if (!btn) return;

    qint64 id = btn->property("recordId").toLongLong();
    qDebug() << "点击了指派按钮，记录 ID:" << id;

    performAssignTask(id);
}

void overview::onEditClicked()
{
    QPushButton *btn = qobject_cast<QPushButton*>(sender());
    if (!btn) return;
    qint64 id = btn->property("recordId").toLongLong();
    QMessageBox::information(this, "提示", QString("正在编辑灾情 ID: %1 (功能开发中)").arg(id));
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
        qDebug() << "预加载处置员成功，数量:" << handlers.size();
    } else {
        qDebug() << "预加载处置员失败:" << err;
    }
}

void overview::refreshTable()
{
    InitialData();
}
