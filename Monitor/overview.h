#ifndef OVERVIEW_H
#define OVERVIEW_H

#include <QDialog>
#include <QTableWidget>
#include <QVBoxLayout>


// 定义灾情数据结构体，与数据库表字段一一对应
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

    // 私有方法：初始化表格UI
    void initTableUI();
    // 私有方法：填充假数据
    void fillFakeData();
    void InitialData();
};

#endif // OVERVIEW_H
