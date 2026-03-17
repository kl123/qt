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
#include <QTimer>
#include <QMap>
#include <QPointF>
#include <QDateEdit>

// QtCharts 相关头文件 (确保包含所有用到的类)
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QScatterSeries>
#include <QtCharts/QValueAxis>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QBarCategoryAxis> // 【新增】条形图分类轴
#include <QtCharts/QPieSeries>
#include <QtCharts/QPieSlice>        // 【新增】饼图切片，用于槽函数参数
#include <QtCharts/QChart>

#include "disasterdao.h"

QT_CHARTS_USE_NAMESPACE

class index : public QMainWindow
{
    Q_OBJECT

public:
    explicit index(QWidget *parent = nullptr);
    ~index();

private slots:
    // 【新增】声明饼图悬停处理的槽函数
    void onPieSliceHovered(QPieSlice *slice, bool state);

private:
    // 界面初始化
    void setupTopNav();
    void setupLeftPanel();
    void setupRightPanel();
    void applyGlobalStyle();
    void histogram();

    // 辅助布局函数
    void setupDateRangeControl(QHBoxLayout *layout, QDateEdit *&startEdit, QDateEdit *&endEdit, const QString &label);

    // 数据获取与图表更新
    void GetData();
    void updateLineChart(const QList<DisasterRecord> &records);
    void updateBarChart();
    void updateTypeChart();

    // 自定义提示框逻辑
    void showTooltip(QPoint pos, const QString &text);
    void hideTooltip();

    // --- 界面组件 ---
    QWidget *centralWidget;
    QVBoxLayout *mainLayout;
    QHBoxLayout *topNavLayout;
    QHBoxLayout *contentLayout;
    QVBoxLayout *leftPanelLayout;

    // 顶部导航按钮
    QPushButton *btnRealTime;
    QPushButton *btnFireAlarm;
    QPushButton *btnNational;
    QPushButton *btnConfig;
    QPushButton *outbtn;
    QPushButton *btnRefresh;

    QDateEdit *dateSelector;
    QHBoxLayout *topControlLayout;

    // 顶部Logo和系统名称
    QFrame *logoFrame;
    QLabel *labLogo;
    QLabel *labSystemName;

    // 左侧四个独立卡片
    QFrame *cardUnit;
    QFrame *cardDevice;
    QFrame *cardMonitor;
    QFrame *cardService;

    // 右侧面板组件
    QFrame *rightPanel;
    QHBoxLayout *cardLayout;
    QFrame *cardFireAlarm;
    QFrame *cardHighRisk;
    QFrame *cardElecFire;
    QFrame *cardRealFire;

    // 折线图组件
    QChartView *chartView;
    QChart *lineChart;
    QLineSeries *lineSeries;
    QScatterSeries *scatterSeries;
    QValueAxis *axisX;
    QValueAxis *axisY;

    // 自定义 Tooltip 标签
    QLabel *tooltipLabel;

    // 底部图表布局占位 (已替换为真实图表，但变量名保留或可清理)
    QHBoxLayout *bottomChartLayout;
    // QFrame *chartBarPlaceholder; // 如果不再使用占位符，可以注释掉或删除
    // QFrame *chartCallPlaceholder;

    // 【核心数据映射】
    QMultiMap<qint64, DisasterRecord> pointDataMap;

    // --- 新增：底部图表相关变量 ---
    QDateEdit *barStartDateEdit;
    QDateEdit *barEndDateEdit;
    QDateEdit *pieStartDateEdit;
    QDateEdit *pieEndDateEdit;

    QChartView *barChartView;
    QChartView *pieChartView;
    QChart *barChart;
    QChart *pieChart;

    QBarSeries *barSeries;
    QPieSeries *pieSeries;
};

#endif // INDEX_H
