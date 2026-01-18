// monitorconfig.h
#ifndef MONITORCONFIG_H
#define MONITORCONFIG_H

#include <QDialog>
#include <QComboBox>
#include <QLineEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QSpinBox>
#include <QListWidget>
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

    QString audioFilePath() const;
    int loopCount() const;
    QStringList checkedKeywords() const;
    bool isDarkMode() const;
    int detectionIntervalSeconds() const;

    // 对外提供开启警告和关闭警告的方法
    void playAlertSound();
    void stopAlertSound();

private slots:
    void on_audioSourceCombo_changed(int index);
    void on_selectAudioButton_clicked();
    void on_addKeywordButton_clicked();
    void on_radioCustom_toggled(bool checked);
    void on_playAudioButton_clicked();
    void on_shutAudioButton_clicked();
    void on_keywordList_customContextMenuRequested(const QPoint &pos);
    void onLoopTimerTimeout(); // 新增：循环播放定时器槽


private:
    void loadSettings();
    void saveSettings();
    QString extractBuiltInAudioToTemp();

    // 音频相关控件
    QComboBox *m_audioSourceCombo;
    QLineEdit *m_audioPathEdit;
    QPushButton *m_selectAudioButton;

    // 播放设置
    QRadioButton *m_radioOnce;
    QRadioButton *m_radioInfinite;
    QRadioButton *m_radioCustom;
    QSpinBox *m_spinCustom;

    // 关键词
    QLineEdit *m_keywordInput;
    QPushButton *m_addKeywordButton;
    QListWidget *m_keywordList;

    // 主题
    QRadioButton *m_radioLight;
    QRadioButton *m_radioDark;

    // 按钮
    QPushButton *m_playAudioButton;
    QPushButton *m_shutAudioButton;

    // 检测间隔
    QSpinBox *m_intervalSpin;

    // 音频状态
    QString m_tempAudioPath;
    int m_currentLoop;      // 当前已播放次数
    int m_targetLoops;      // 目标循环次数（-1 表示无限）
    QTimer *m_loopTimer;    // 用于循环播放
};

#endif // MONITORCONFIG_H
