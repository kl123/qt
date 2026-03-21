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

// 确保包含数据结构定义
#include "disasterdao.h"
#include "userauth.h"

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
    void fillFakeData(); // 保留以防万一，目前主要用 InitialData
    void InitialData();
    void refreshTable();

    // 辅助功能
    QWidget* createActionWidget(qint64 id);
    void loadEmee();
    void performAssignTask(qint64 disasterId);

    // 核心业务逻辑
    void addInfo(const DisasterRecord &record);
    void addInfo(); // 兼容旧代码

private slots:
    // 现有操作
    void onAssignClicked();
    void onEditClicked();
    void onDeleteClicked();

    // 新增操作
    void onAddDisasterClicked();
};

#endif // OVERVIEW_H
