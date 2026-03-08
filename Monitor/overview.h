#ifndef OVERVIEW_H
#define OVERVIEW_H

#include <QDialog>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <QList>
#include <QListWidget>      // 新增：用于多选列表
#include <QLabel>           // 新增：用于提示文字
#include <QMessageBox>      // 新增：用于弹窗
#include "disasterdao.h"    // 确保这里包含了 DisasterRecord 的定义
#include "userauth.h"       // 确保这里包含了 AuthUser 的定义

// 定义灾情数据结构体 (保留用于兼容旧代码或假数据)
struct DisasterInfo {
    int id;                 // 主键ID
    QString disaster_type;  // 灾害类型
    QString location;       // 发生地点
    QString occurred_at;    // 发生时间
    QString content;        // 灾害详情
    int severity;           // 严重程度（0-5，0为默认）
    int dispatcher_id;      // 调度员ID（外键）
    QString created_at;     // 记录创建时间
};

namespace Ui {
class overview;
}

class overview : public QDialog
{
    Q_OBJECT

public:
    explicit overview(QWidget *parent = nullptr);
    ~overview();

private:
    Ui::overview *ui;
    QTableWidget *disasterTable;  // 灾情展示表格
    QVBoxLayout *mainLayout;      // 主布局

    // 私有方法
    void initTableUI();
    void fillFakeData();
    void InitialData();
    QWidget* createActionWidget(qint64 id);
    void loadEmee();              // 加载员工信息

    // 新增：辅助方法，用于执行指派逻辑（包含弹出多选框）
    void performAssignTask(qint64 disasterId);

private slots:
    // 按钮点击槽函数
    void onAssignClicked();
    void onEditClicked();
    void onDeleteClicked();
};

#endif // OVERVIEW_H
