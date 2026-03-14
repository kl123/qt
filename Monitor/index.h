#ifndef INDEX_H
#define INDEX_H

#include <QMainWindow>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QFrame>
#include <QLabel>
#include <QPushButton>
#include <QFont>
#include <QPixmap>

class index : public QMainWindow
{
    Q_OBJECT

public:
    explicit index(QWidget *parent = nullptr);
    ~index();

private:
    void setupTopNav();
    void setupLeftPanel();
    void setupRightPanel();
    void applyGlobalStyle();

    QWidget *centralWidget;
    QVBoxLayout *mainLayout;
    QHBoxLayout *topNavLayout;
    QHBoxLayout *contentLayout;
    QVBoxLayout *leftPanelLayout; // 左侧总布局（存放四个卡片）

    // 顶部导航按钮
    QPushButton *btnRealTime;
    QPushButton *btnFireAlarm;
    QPushButton *btnNational;
    QPushButton *btnConfig; // 新增配置按钮
    QPushButton *outbtn;

    // 顶部Logo和系统名称
    QFrame *logoFrame;
    QLabel *labLogo;
    QLabel *labSystemName;

    // 左侧四个独立卡片
    QFrame *cardUnit;        // 接入单位卡片
    QFrame *cardDevice;      // 接入设备卡片
    QFrame *cardMonitor;     // 实时监测卡片
    QFrame *cardService;     // 服务机构卡片

    // 右侧面板组件
    QFrame *rightPanel;
    QHBoxLayout *cardLayout;
    QFrame *cardFireAlarm;
    QFrame *cardHighRisk;
    QFrame *cardElecFire;
    QFrame *cardRealFire;
    QFrame *chartLinePlaceholder; // 折线图占位
    QHBoxLayout *bottomChartLayout;
    QFrame *chartBarPlaceholder;  // 条形图占位
    QFrame *chartCallPlaceholder; // 呼叫时长图占位
};

#endif // INDEX_H
