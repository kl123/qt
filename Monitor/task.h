#ifndef TASK_H
#define TASK_H

#include <QDialog>
#include <QTableWidget> // 新增包含
#include <QList>
#include "disasterdao.h" // 确保包含结构体定义

namespace Ui {
class task;
}

class task : public QDialog
{
    Q_OBJECT

public:
    explicit task(QWidget *parent = nullptr);
    ~task();

private slots:
    // 新增：处理按钮点击
    void onUpdateProgressClicked(qint64 taskId, qint64 disasterId, int currentProgress);

private:
    Ui::task *ui;
    QTableWidget *m_table; // 新增：表格指针

    void initUI();         // 新增：初始化 UI
    void SeekInfo();       // 原有的数据加载
};

#endif // TASK_H
