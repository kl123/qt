#ifndef OVERVIEW_H
#define OVERVIEW_H

#include <QDialog>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <QList>
#include <QListWidget>
#include <QLabel>
#include <QMessageBox>
#include <QLineEdit>
#include <QComboBox>
#include <QDateTimeEdit>
#include <QSpinBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QTimer>
#include <QSettings>

#include "disasterdao.h"    // 确保包含 DisasterRecord 定义
#include "userauth.h"       // 确保包含 AuthUser 定义

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
    QTableWidget *disasterTable;
    QVBoxLayout *mainLayout;

    // UI 初始化与数据加载
    void initTableUI();
    void fillFakeData();
    void InitialData();
    void refreshTable(); // 新增：刷新表格

    // 辅助功能
    QWidget* createActionWidget(qint64 id);
    void loadEmee();
    void performAssignTask(qint64 disasterId);

    // 核心业务逻辑
    void addInfo(const DisasterRecord &record); // 修改：接收结构体参数
    void addInfo(); // 保留无参版本以防旧代码调用，但内部不再使用硬编码

private slots:
    // 现有操作
    void onAssignClicked();
    void onEditClicked();
    void onDeleteClicked();

    // 新增操作
    void onAddDisasterClicked();
};

#endif // OVERVIEW_H
