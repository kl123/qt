#ifndef MONITORCONFIG_H
#define MONITORCONFIG_H

#include <QDialog>
#include <QComboBox>
#include <QLineEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QSpinBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QTimer>

class MonitorConfig : public QDialog
{
    Q_OBJECT

public:
    explicit MonitorConfig(QWidget *parent = nullptr);
    ~MonitorConfig();

    // 原有接口保留
    QString audioFilePath() const;
    int loopCount() const;
    bool isDarkMode() const;
    int detectionIntervalSeconds() const;

    // 新增：事件类型和等级接口
    QString eventType() const;       // 获取选中的事件类型（火灾/社会救助）
    int eventLevel() const;          // 获取选中的等级（1-4级）

    // 对外提供开启警告和关闭警告的方法
    void playAlertSound();
    void stopAlertSound();

private slots:
    // 移除关键词相关槽函数，保留其他槽函数
    void on_audioSourceCombo_changed(int index);
    void on_selectAudioButton_clicked();
    void on_radioCustom_toggled(bool checked);
    void on_playAudioButton_clicked();
    void on_shutAudioButton_clicked();
    void onLoopTimerTimeout(); // 循环播放定时器槽

private:
    void loadSettings();
    void saveSettings();
    QString extractBuiltInAudioToTemp();

    // 音频相关控件（保留）
    QComboBox *m_audioSourceCombo;
    QLineEdit *m_audioPathEdit;
    QPushButton *m_selectAudioButton;

    // 播放设置（保留）
    QRadioButton *m_radioOnce;
    QRadioButton *m_radioInfinite;
    QRadioButton *m_radioCustom;
    QSpinBox *m_spinCustom;

    // 主题（保留）
    QRadioButton *m_radioLight;
    QRadioButton *m_radioDark;

    // 按钮（保留）
    QPushButton *m_playAudioButton;
    QPushButton *m_shutAudioButton;

    // 检测间隔（保留）
    QSpinBox *m_intervalSpin;

    // 新增：事件类型和等级控件（替换原关键词控件）
    QComboBox *m_eventTypeCombo;    // 事件类型下拉框（火灾/社会救助）
    QSpinBox *m_eventLevelSpin;     // 等级选择（1-4级）

    // 音频状态（保留）
    QString m_tempAudioPath;
    int m_currentLoop;      // 当前已播放次数
    int m_targetLoops;      // 目标循环次数（-1 表示无限）
    QTimer *m_loopTimer;    // 用于循环播放
};

#endif // MONITORCONFIG_H
