#include "overview.h"
#include "ui_overview.h"
#include <QHeaderView>
#include <QTableWidgetItem>
#include <QFont>
#include <QColor>
#include "disasterdao.h"
#include <QDebug>
#include <QSettings>

overview::overview(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::overview)
{
    ui->setupUi(this);
    this->setWindowTitle("灾情总览");  // 设置窗口标题
    this->resize(1200, 600);          // 设置窗口初始大小

    // 初始化主布局
    mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);  // 边距
    mainLayout->setSpacing(10);

    // 初始化表格UI
    initTableUI();

    // 填充假数据
    InitialData();
}

overview::~overview()
{
    delete ui;
    // 手动释放组件（可选，Qt 父子机制会自动回收，此处为规范）
    delete disasterTable;
    delete mainLayout;
}

void overview::initTableUI()
{
    // 1. 创建表格，设置列数（与DisasterInfo字段一致，8列）
    disasterTable = new QTableWidget(0, 8, this);
    // 设置列名
    QStringList headers = {
        "ID", "灾害类型", "发生地点", "发生时间",
        "灾害详情", "严重程度", "调度员ID", "记录时间"
    };
    disasterTable->setHorizontalHeaderLabels(headers);

    // 2. 表格核心样式美化（关键：让表格好看）
    disasterTable->setStyleSheet(R"(
        /* 表格整体样式 */
        QTableWidget {
            border: 1px solid #E0E0E0;
            border-radius: 8px;
            gridline-color: #E0E0E0;
            background-color: #FFFFFF;
            font-size: 13px;
            color: #333333;
        }
        /* 表头样式 */
        QTableWidget::horizontalHeader {
            background-color: #2196F3;
            color: #FFFFFF;
            font-weight: bold;
            font-size: 14px;
        }
        QTableWidget::horizontalHeader::section {
            border: none;
            padding: 8px;
            border-right: 1px solid #1976D2;
        }
        QTableWidget::horizontalHeader::section:last {
            border-right: none;
        }
        /* 行交替颜色（提升可读性） */
        QTableWidget::item:even {
            background-color: #F5F7FA;
        }
        QTableWidget::item:odd {
            background-color: #FFFFFF;
        }
        /* 选中行样式 */
        QTableWidget::item:selected {
            background-color: #BBDEFB;
            color: #1976D2;
        }
        /* 禁止编辑的单元格样式 */
        QTableWidget::item:disabled {
            background-color: #FFFFFF;
            color: #333333;
        }
    )");

    // 3. 表格功能配置（提升体验）
    disasterTable->setEditTriggers(QAbstractItemView::NoEditTriggers);  // 禁止编辑
    disasterTable->setSelectionBehavior(QAbstractItemView::SelectRows); // 整行选中
    disasterTable->setSelectionMode(QAbstractItemView::SingleSelection); // 单选
    disasterTable->verticalHeader()->setVisible(false);                 // 隐藏垂直序号
    disasterTable->horizontalHeader()->setStretchLastSection(true);     // 最后一列拉伸
    // 列宽自适应（按内容+比例）
    disasterTable->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    disasterTable->horizontalHeader()->setSectionResizeMode(4, QHeaderView::Stretch); // 详情列优先拉伸

    // 4. 设置行高
    disasterTable->setRowHeight(0, 40);
    disasterTable->setRowHeight(1, 40);

    // 5. 将表格加入主布局
    mainLayout->addWidget(disasterTable);
}

void overview::fillFakeData()
{
    // 定义两条假数据（数组形式）
    DisasterInfo fakeDisasters[2] = {
        {
            1,
            "洪涝灾害",
            "杭州市西湖区紫金港路",
            "2026-03-01 14:25:00",
            "持续强降雨导致道路积水约80cm，3辆私家车被困，无人员伤亡。",
            3,
            1001,
            "2026-03-01 14:30:15"
        },
        {
            2,
            "山体滑坡",
            "温州市永嘉县楠溪江镇",
            "2026-03-02 08:10:00",
            "山区边坡发生小型滑坡，阻断乡村道路，已组织人员清理。",
            2,
            1002,
            "2026-03-02 08:15:30"
        }
    };

    // 循环填充表格
    for (int i = 0; i < 2; ++i) {
        disasterTable->insertRow(i); // 插入一行

        // 填充各列数据（注意：QTableWidgetItem 需手动创建，禁止编辑）
        disasterTable->setItem(i, 0, new QTableWidgetItem(QString::number(fakeDisasters[i].id)));
        disasterTable->setItem(i, 1, new QTableWidgetItem(fakeDisasters[i].disaster_type));
        disasterTable->setItem(i, 2, new QTableWidgetItem(fakeDisasters[i].location));
        disasterTable->setItem(i, 3, new QTableWidgetItem(fakeDisasters[i].occurred_at));
        disasterTable->setItem(i, 4, new QTableWidgetItem(fakeDisasters[i].content));

        // 严重程度优化：数字转文字（更友好）
        QString severityText;
        switch (fakeDisasters[i].severity) {
            case 0: severityText = "一般"; break;
            case 1: severityText = "较轻"; break;
            case 2: severityText = "中等"; break;
            case 3: severityText = "较重"; break;
            case 4: severityText = "严重"; break;
            case 5: severityText = "特重"; break;
            default: severityText = "未知";
        }
        disasterTable->setItem(i, 5, new QTableWidgetItem(severityText));

        disasterTable->setItem(i, 6, new QTableWidgetItem(QString::number(fakeDisasters[i].dispatcher_id)));
        disasterTable->setItem(i, 7, new QTableWidgetItem(fakeDisasters[i].created_at));

        // 设置所有单元格居中对齐
        for (int j = 0; j < 8; ++j) {
            disasterTable->item(i, j)->setTextAlignment(Qt::AlignCenter);
        }
    }
}
void overview::InitialData(){
    QSettings settings("System", "disaster");
    qint64 userId = settings.value("userid", 0).toLongLong();
    QList<DisasterRecord> unassigned;
    QString err;

    // 执行查询
    bool querySuccess = DisasterDao::getUnassignedDisastersByDispatcher(userId, 50, 0, &unassigned, &err);

    // ========== 核心：打印 QList<DisasterRecord> 完整内容 ==========
    qDebug() << "===== QList<DisasterRecord> 内容 ======";
    // 1. 先打印列表基本信息
    qDebug() << "列表是否为空：" << unassigned.isEmpty();
    qDebug() << "列表元素数量：" << unassigned.size();

    // 2. 打印查询结果状态（辅助排查）
    qDebug() << "查询是否成功：" << querySuccess;
    if (!querySuccess) {
        qDebug() << "查询失败原因：" << err;
    }

    // 3. 遍历打印每条记录的核心字段（按需增减字段）
    if (!unassigned.isEmpty()) {
        for (int i = 0; i < unassigned.size(); ++i) {
            const DisasterRecord& record = unassigned.at(i);
            qDebug() << QString("第 %1 条记录：").arg(i+1);
            qDebug() << "  ID：" << record.id;
            qDebug() << "  发生地点：" << record.location;
            qDebug() << "  严重程度：" << record.severity;
            // 如需打印更多字段，直接追加即可，比如：
            // qDebug() << "  发生时间：" << record.occurred_at;
            // qDebug() << "  灾情内容：" << record.content;
        }
    }
    qDebug() << "======================================";
}
