#ifndef TASK_H
#define TASK_H

#include <QDialog>
#include <QTableWidget>
#include <QList>
#include "disasterdao.h"

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
    // 处理按钮点击
    void onUpdateProgressClicked(qint64 taskId, qint64 disasterId, int currentProgress);
    // 确认更新进度
    void onConfirmProgressUpdate(qint64 taskId, int newProgress);

private:
    Ui::task *ui;
    QTableWidget *m_table;

    void initUI();
    void SeekInfo();
    void setupWindowFlags();  // 设置窗口标志（移除问号，添加全屏/最小化）
    void showProgressDialog(qint64 taskId, qint64 disasterId, int currentProgress);  // 显示进度更新对话框
};

#endif // TASK_H
