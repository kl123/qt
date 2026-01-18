// monitorconfig.cpp
#include "monitorconfig.h"
#include <QFileDialog>
#include <QDir>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QListWidgetItem>
#include <QAbstractItemView>
#include <QDialogButtonBox>
#include <QSettings>
#include <QMessageBox>
#include <QTemporaryFile>
#include <QResource>
#include <QFile>
#include <QDebug>
#include <QMenu>
#include <windows.h>
#include <mmsystem.h> // PlaySound 所需

#pragma comment(lib, "winmm.lib") // 链接多媒体库

MonitorConfig::MonitorConfig(QWidget *parent)
    : QDialog(parent)
    , m_tempAudioPath("")
    , m_currentLoop(0)
    , m_targetLoops(1)
    , m_loopTimer(new QTimer(this))
{
    setWindowTitle("设置");
    resize(400, 500);

    // === 音频设置 ===
    m_audioPathEdit = new QLineEdit(this);
    m_audioPathEdit->setReadOnly(true);

    m_audioSourceCombo = new QComboBox(this);
    m_audioSourceCombo->addItem("内置警报音 (alert.wav)");
    m_audioSourceCombo->addItem("选择本地音频...");

    connect(m_audioSourceCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MonitorConfig::on_audioSourceCombo_changed);

    m_selectAudioButton = new QPushButton("浏览...", this);
    m_selectAudioButton->setEnabled(false);
    connect(m_selectAudioButton, &QPushButton::clicked, this, &MonitorConfig::on_selectAudioButton_clicked);

    QGroupBox *audioGroup = new QGroupBox("音频设置", this);
    QHBoxLayout *audioLayout = new QHBoxLayout;
    audioLayout->addWidget(m_audioSourceCombo);
    audioLayout->addWidget(m_selectAudioButton);
    audioLayout->addWidget(m_audioPathEdit);
    audioGroup->setLayout(audioLayout);

    // === 播放设置 ===
    m_radioOnce = new QRadioButton("播放 1 次", this);
    m_radioInfinite = new QRadioButton("无限循环", this);
    m_radioCustom = new QRadioButton("自定义次数：", this);
    m_spinCustom = new QSpinBox(this);
    m_spinCustom->setRange(1, 999);
    m_spinCustom->setValue(3);
    m_spinCustom->setEnabled(false);

    connect(m_radioCustom, &QRadioButton::toggled, this, &MonitorConfig::on_radioCustom_toggled);

    QGroupBox *playGroup = new QGroupBox("播放设置", this);
    QVBoxLayout *playLayout = new QVBoxLayout;
    playLayout->addWidget(m_radioOnce);
    playLayout->addWidget(m_radioInfinite);
    QHBoxLayout *customLayout = new QHBoxLayout;
    customLayout->addWidget(m_radioCustom);
    customLayout->addWidget(m_spinCustom);
    customLayout->addStretch();
    playLayout->addLayout(customLayout);
    playGroup->setLayout(playLayout);

    m_radioOnce->setChecked(true);

    // === 关键词监控 ===
    m_keywordInput = new QLineEdit(this);
    m_addKeywordButton = new QPushButton("添加", this);
    connect(m_addKeywordButton, &QPushButton::clicked, this, &MonitorConfig::on_addKeywordButton_clicked);
    connect(m_keywordInput, &QLineEdit::returnPressed, this, &MonitorConfig::on_addKeywordButton_clicked);

    m_keywordList = new QListWidget(this);
    m_keywordList->setSelectionMode(QAbstractItemView::NoSelection);
    m_keywordList->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_keywordList, &QListWidget::customContextMenuRequested,
            this, &MonitorConfig::on_keywordList_customContextMenuRequested);

    QGroupBox *keywordGroup = new QGroupBox("关键词监控", this);
    QVBoxLayout *keywordLayout = new QVBoxLayout;
    QHBoxLayout *inputLayout = new QHBoxLayout;
    inputLayout->addWidget(m_keywordInput);
    inputLayout->addWidget(m_addKeywordButton);
    keywordLayout->addLayout(inputLayout);
    keywordLayout->addWidget(m_keywordList);
    keywordGroup->setLayout(keywordLayout);

    // === 屏幕检测间隔设置 ===
    m_intervalSpin = new QSpinBox(this);
    m_intervalSpin->setRange(1, 3600);
    m_intervalSpin->setValue(5);
    m_intervalSpin->setSuffix(" 秒");

    QGroupBox *intervalGroup = new QGroupBox("屏幕检测间隔", this);
    QHBoxLayout *intervalLayout = new QHBoxLayout;
    intervalLayout->addWidget(new QLabel("每", this));
    intervalLayout->addWidget(m_intervalSpin);
    intervalLayout->addWidget(new QLabel("检测一次屏幕内容", this));
    intervalLayout->addStretch();
    intervalGroup->setLayout(intervalLayout);

    // === 主题设置 ===
    m_radioLight = new QRadioButton("明亮模式", this);
    m_radioDark = new QRadioButton("黑暗模式", this);
    m_radioLight->setChecked(true);

    QGroupBox *themeGroup = new QGroupBox("微信主题颜色", this);
    QVBoxLayout *themeLayout = new QVBoxLayout;
    themeLayout->addWidget(m_radioLight);
    themeLayout->addWidget(m_radioDark);
    themeGroup->setLayout(themeLayout);

    // === 底部按钮 ===
    m_playAudioButton = new QPushButton("播放音频", this);
    m_shutAudioButton = new QPushButton("停止播放", this);
    QPushButton *okButton = new QPushButton("确定", this);
    QPushButton *cancelButton = new QPushButton("取消", this);

    connect(m_playAudioButton, &QPushButton::clicked, this, &MonitorConfig::on_playAudioButton_clicked);
    connect(m_shutAudioButton, &QPushButton::clicked, this, &MonitorConfig::on_shutAudioButton_clicked);
    connect(okButton, &QPushButton::clicked, this, &QDialog::accept);
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);
    connect(this, &QDialog::accepted, this, &MonitorConfig::saveSettings);

    // 循环播放定时器
    connect(m_loopTimer, &QTimer::timeout, this, &MonitorConfig::onLoopTimerTimeout);

    // === 主布局 ===
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(audioGroup);
    mainLayout->addWidget(playGroup);
    mainLayout->addWidget(keywordGroup);
    mainLayout->addWidget(intervalGroup);
    mainLayout->addWidget(themeGroup);
    mainLayout->addStretch();

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addWidget(m_playAudioButton);
    buttonLayout->addWidget(m_shutAudioButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(okButton);
    buttonLayout->addWidget(cancelButton);
    mainLayout->addLayout(buttonLayout);

    setLayout(mainLayout);

    // 加载上次保存的设置
    loadSettings();
}

MonitorConfig::~MonitorConfig()
{
    stopAlertSound();
    if (!m_tempAudioPath.isEmpty()) {
        QFile::remove(m_tempAudioPath);
    }
}

// ===== 槽函数 =====

void MonitorConfig::on_audioSourceCombo_changed(int index)
{
    if (index == 0) {
        m_selectAudioButton->setEnabled(false);
        m_audioPathEdit->setText(":/sound/alert2.wav");
    } else {
        m_selectAudioButton->setEnabled(true);
        m_audioPathEdit->clear();
    }
}

void MonitorConfig::on_selectAudioButton_clicked()
{
    QString filePath = QFileDialog::getOpenFileName(
        this,
        "选择音频文件",
        QDir::homePath(),
        "WAV 音频文件 (*.wav)"
    );
    if (!filePath.isEmpty()) {
        m_audioPathEdit->setText(filePath);
    }
}

void MonitorConfig::on_addKeywordButton_clicked()
{
    QString keyword = m_keywordInput->text().trimmed();
    if (keyword.isEmpty()) return;

    for (int i = 0; i < m_keywordList->count(); ++i) {
        if (m_keywordList->item(i)->text() == keyword)
            return;
    }

    QListWidgetItem *item = new QListWidgetItem(keyword, m_keywordList);
    item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
    item->setCheckState(Qt::Checked);
    m_keywordInput->clear();
}

void MonitorConfig::on_radioCustom_toggled(bool checked)
{
    m_spinCustom->setEnabled(checked);
}

// ===== 获取配置 =====

QString MonitorConfig::audioFilePath() const
{
    int index = m_audioSourceCombo->currentIndex();
    if (index == 0) {
        return ":/sound/alert2.wav";
    } else {
        return m_audioPathEdit->text();
    }
}

int MonitorConfig::loopCount() const
{
    if (m_radioInfinite->isChecked()) {
        return -1;
    } else if (m_radioCustom->isChecked()) {
        return m_spinCustom->value();
    } else {
        return 1;
    }
}

QStringList MonitorConfig::checkedKeywords() const
{
    QStringList list;
    for (int i = 0; i < m_keywordList->count(); ++i) {
        auto item = m_keywordList->item(i);
        if (item->checkState() == Qt::Checked) {
            list << item->text();
        }
    }
    return list;
}

bool MonitorConfig::isDarkMode() const
{
    return m_radioDark->isChecked();
}

int MonitorConfig::detectionIntervalSeconds() const
{
    return m_intervalSpin->value();
}

// ===== 音频播放逻辑（使用 PlaySound）=====

QString MonitorConfig::extractBuiltInAudioToTemp()
{
    if (!m_tempAudioPath.isEmpty() && QFile::exists(m_tempAudioPath)) {
        return m_tempAudioPath;
    }

    QResource resource(":/sound/alert2.wav");
    if (!resource.isValid()) {
        qWarning() << "内置音频资源 ':/sound/alert2.wav' 不存在！";
        return "";
    }

    QByteArray data = QByteArray::fromRawData(
        reinterpret_cast<const char*>(resource.data()),
        resource.size()
    );

    QTemporaryFile tempFile(QDir::tempPath() + "/alert2_XXXXXX.wav");
    tempFile.setAutoRemove(false);
    if (tempFile.open()) {
        tempFile.write(data);
        tempFile.close();
        m_tempAudioPath = tempFile.fileName();
        qDebug() << "内置音频已解压到临时文件:" << m_tempAudioPath;
        return m_tempAudioPath;
    }

    qWarning() << "无法创建临时音频文件";
    return "";
}

void MonitorConfig::playAlertSound()
{
    QString filePath = audioFilePath();
    if (filePath.isEmpty()) return;

    QString realPath;
    if (filePath.startsWith(":/")) {
        realPath = extractBuiltInAudioToTemp();
        if (realPath.isEmpty()) return;
    } else {
        if (!QFile::exists(filePath)) return;
        realPath = filePath;
    }

    // 停止当前播放
    stopAlertSound();

    // 设置循环参数
    m_targetLoops = loopCount();
    m_currentLoop = 0;

    // 立即播放第一次
    PlaySound((LPCWSTR)realPath.toStdWString().c_str(), NULL, SND_FILENAME | SND_ASYNC);
    m_currentLoop++;

    // 如果需要循环
    if (m_targetLoops != 1) {
        // 使用短延时确保声音结束（简单估算，实际不精确）
        int durationMs = 2500; // 假设 1 秒（可根据实际调整）
        m_loopTimer->start(durationMs);
    }
}

void MonitorConfig::stopAlertSound()
{
    m_loopTimer->stop();
    PlaySound(NULL, NULL, SND_FILENAME); // 停止所有播放
    m_currentLoop = 0;
}

void MonitorConfig::onLoopTimerTimeout()
{
    QString filePath = audioFilePath();
    if (filePath.isEmpty()) {
        m_loopTimer->stop();
        return;
    }

    QString realPath;
    if (filePath.startsWith(":/")) {
        realPath = extractBuiltInAudioToTemp();
        if (realPath.isEmpty()) {
            m_loopTimer->stop();
            return;
        }
    } else {
        if (!QFile::exists(filePath)) {
            m_loopTimer->stop();
            return;
        }
        realPath = filePath;
    }

    if (m_targetLoops > 0 && m_currentLoop >= m_targetLoops) {
        m_loopTimer->stop();
        return;
    }

    PlaySound((LPCWSTR)realPath.toStdWString().c_str(), NULL, SND_FILENAME | SND_ASYNC);
    m_currentLoop++;

    // 若无限循环，继续；否则检查是否达到上限
    if (m_targetLoops == -1) {
        // 继续循环（保持定时器运行）
    } else if (m_currentLoop >= m_targetLoops) {
        m_loopTimer->stop();
    }
}

void MonitorConfig::on_playAudioButton_clicked()
{
    playAlertSound();
}

void MonitorConfig::on_shutAudioButton_clicked()
{
    stopAlertSound();
}

// ===== 右键菜单 =====

void MonitorConfig::on_keywordList_customContextMenuRequested(const QPoint &pos)
{
    QListWidgetItem *item = m_keywordList->itemAt(pos);
    if (!item) return;

    QMenu menu(this);
    QAction *deleteAction = menu.addAction("删除关键词");

    QAction *selectedAction = menu.exec(m_keywordList->mapToGlobal(pos));
    if (selectedAction == deleteAction) {
        int row = m_keywordList->row(item);
        delete m_keywordList->takeItem(row);
    }
}

// ===== 设置保存/加载 =====

void MonitorConfig::loadSettings()
{
    QSettings settings("MyCompany", "MonitorApp");

    int audioIndex = settings.value("audioSourceIndex", 0).toInt();
    m_audioSourceCombo->setCurrentIndex(audioIndex);
    on_audioSourceCombo_changed(audioIndex);

    QString localPath = settings.value("localAudioPath", "").toString();
    if (!localPath.isEmpty()) {
        m_audioPathEdit->setText(localPath);
    }

    bool isOnce = settings.value("playOnce", true).toBool();
    bool isInfinite = settings.value("playInfinite", false).toBool();
    int customCount = settings.value("customCount", 3).toInt();

    if (isOnce) {
        m_radioOnce->setChecked(true);
    } else if (isInfinite) {
        m_radioInfinite->setChecked(true);
    } else {
        m_radioCustom->setChecked(true);
        m_spinCustom->setValue(customCount);
    }

    QStringList keywords = settings.value("keywords").toStringList();
    for (const QString &kw : keywords) {
        QListWidgetItem *item = new QListWidgetItem(kw, m_keywordList);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(Qt::Checked);
    }

    bool isDark = settings.value("darkMode", false).toBool();
    m_radioDark->setChecked(isDark);

    int interval = settings.value("detectionInterval", 5).toInt();
    m_intervalSpin->setValue(interval);
}

void MonitorConfig::saveSettings()
{
    QSettings settings("MyCompany", "MonitorApp");

    settings.setValue("audioSourceIndex", m_audioSourceCombo->currentIndex());
    settings.setValue("localAudioPath", m_audioPathEdit->text());
    settings.setValue("playOnce", m_radioOnce->isChecked());
    settings.setValue("playInfinite", m_radioInfinite->isChecked());
    settings.setValue("customCount", m_spinCustom->value());

    QStringList keywords;
    for (int i = 0; i < m_keywordList->count(); ++i) {
        if (m_keywordList->item(i)->checkState() == Qt::Checked) {
            keywords << m_keywordList->item(i)->text();
        }
    }
    settings.setValue("keywords", keywords);
    settings.setValue("detectionInterval", m_intervalSpin->value());
    settings.setValue("darkMode", m_radioDark->isChecked());
}
