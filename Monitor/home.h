#ifndef HOME_H
#define HOME_H

#include <QWidget>
#include <QTimer>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QLabel>
#include <QPushButton>
#include <QTextEdit>
#include <QRandomGenerator>
#include <QDateTime>
#include <QtCharts/QChart>
#include <QtCharts/QPieSeries>
#include <QtCharts/QPieSlice>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QCategoryAxis>
#include <QtCharts/QChartView>

// Qt5必须声明图表命名空间
QT_CHARTS_USE_NAMESPACE

class Home : public QWidget
{
    Q_OBJECT
public:
    explicit Home(QWidget *parent = nullptr);
    ~Home() override;

private slots:
    // 刷新模拟OCR数据
    void refreshMockOcrData();
    // 更新统计卡片数值
    void updateStatCards();
    // 更新饼图和柱状图
    void updateCharts();
    // 添加实时日志
    void addMonitorLog(const QString &logContent);

private:
    // 初始化UI布局
    void initHomeUI();
    // 初始化模拟灾害数据
    void initMockDisasterData();
    // 创建统计卡片（复用控件）
    QFrame* createStatCard(const QString &title, const QString &icon, int value, const QString &bgColor);

    // ========== 成员变量 ==========
    // 监测状态标记
    bool m_isMonitoring = false;
    // 数据刷新定时器（模拟OCR实时监测）
    QTimer *m_refreshTimer = nullptr;

    // 灾害类型统计数组（替代OCR真实数据）
    // 索引对应：0-火灾 1-抢险救援 2-社会救助 3-灾害事故 4-反恐排爆 5-其他出动
    int m_disasterCount[6] = {0, 0, 0, 0, 0, 0};
    // 灾害类型名称
    QStringList m_disasterNames = {"火灾", "抢险救援", "社会救助", "灾害事故", "反恐排爆", "其他出动"};
    // 统计卡片数值标签（用于实时更新）
    QLabel *m_statValueLabels[6] = {nullptr};

    // 图表组件
    QPieSeries *m_pieSeries = nullptr;   // 占比饼图
    QBarSeries *m_barSeries = nullptr;   // 数量柱状图
    QTextEdit *m_logTextEdit = nullptr;  // 日志显示框
};

#endif // HOME_H
