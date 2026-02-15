#include "monitorconfig.h"
#include <QFileDialog>
#include <QDir>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QDialogButtonBox>
#include <QSettings>
#include <QCheckBox>
#include <QMessageBox>
#include <QTemporaryFile>
#include <QResource>
#include <QFile>
#include <QDebug>
#include <QDesktopServices>
#include <QUrl>
#include <windows.h>
#include <mmsystem.h> // PlaySound 所需

#ifdef _MSC_VER
#pragma comment(lib, "winmm.lib") // 链接多媒体库
#endif

MonitorConfig::MonitorConfig(QWidget *parent)
    : QDialog(parent)
    , m_tempAudioPath("")
    , m_currentLoop(0)
    , m_targetLoops(1)
    , m_loopTimer(new QTimer(this))
{
    setWindowTitle("设置");
    resize(400, 550); // 稍微调高一点以容纳 AI 设置

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

    // === 事件监测设置 ===
    m_eventTypeCombo = new QComboBox(this);
    m_eventTypeCombo->addItem("火灾");
    m_eventTypeCombo->addItem("社会救助");
    m_eventTypeCombo->setCurrentIndex(0);

    m_eventLevelSpin = new QSpinBox(this);
    m_eventLevelSpin->setRange(1, 10);
    m_eventLevelSpin->setValue(1);
    m_eventLevelSpin->setSuffix(" 级");

    QGroupBox *eventGroup = new QGroupBox("事件监测设置", this);
    QFormLayout *eventLayout = new QFormLayout;
    eventLayout->addRow("事件类型：", m_eventTypeCombo);
    eventLayout->addRow("事件等级：", m_eventLevelSpin);
    eventGroup->setLayout(eventLayout);

    // === AI 配置 (新) ===
    m_aiEnableCheck = new QCheckBox("启用 AI 分析 (推荐火山引擎 DeepSeek)", this);
    
    m_aiUrlEdit = new QLineEdit(this);
    m_aiUrlEdit->setPlaceholderText("https://ark.cn-beijing.volces.com/api/v3");
    m_aiUrlEdit->setText("https://ark.cn-beijing.volces.com/api/v3"); // 默认火山引擎
    
    m_aiKeyEdit = new QLineEdit(this);
    m_aiKeyEdit->setEchoMode(QLineEdit::Password);
    m_aiKeyEdit->setPlaceholderText("d42f588f-..."); // 示例 Key
    
    // 添加获取 Key 的链接
    QLabel *getKeyLabel = new QLabel("<a href='https://console.volcengine.com/ark/region:ark+cn-beijing/endpoint'>点击此处获取火山引擎 API Key</a>", this);
    getKeyLabel->setOpenExternalLinks(true);
    getKeyLabel->setStyleSheet("QLabel { color: blue; text-decoration: underline; }");
    getKeyLabel->setCursor(Qt::PointingHandCursor);

    m_aiModelEdit = new QLineEdit(this);
    m_aiModelEdit->setPlaceholderText("deepseek-v3-2-251201");
    m_aiModelEdit->setText("deepseek-v3-2-251201"); // 默认 DeepSeek V3

    QGroupBox *aiGroup = new QGroupBox("AI 设置 (已预设免费服务)", this);
    QFormLayout *aiLayout = new QFormLayout;
    aiLayout->addRow(m_aiEnableCheck);
    aiLayout->addRow("API URL:", m_aiUrlEdit);
    aiLayout->addRow("API Key:", m_aiKeyEdit);
    aiLayout->addRow("", getKeyLabel);
    aiLayout->addRow("模型名称:", m_aiModelEdit);
    aiGroup->setLayout(aiLayout);

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
    mainLayout->addWidget(eventGroup);
    mainLayout->addWidget(aiGroup);
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

void MonitorConfig::on_radioCustom_toggled(bool checked)
{
    m_spinCustom->setEnabled(checked);
}

// ===== Getters =====
QString MonitorConfig::eventType() const
{
    return m_eventTypeCombo->currentText();
}

int MonitorConfig::eventLevel() const
{
    return m_eventLevelSpin->value();
}

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

bool MonitorConfig::isDarkMode() const
{
    return m_radioDark->isChecked();
}

int MonitorConfig::detectionIntervalSeconds() const
{
    return m_intervalSpin->value();
}

QString MonitorConfig::aiApiUrl() const
{
    return m_aiUrlEdit->text().trimmed();
}

QString MonitorConfig::aiApiKey() const
{
    return m_aiKeyEdit->text().trimmed();
}

QString MonitorConfig::aiModel() const
{
    return m_aiModelEdit->text().trimmed();
}

bool MonitorConfig::isAiEnabled() const
{
    return m_aiEnableCheck->isChecked();
}

// ===== Audio Logic =====
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

    stopAlertSound();

    m_targetLoops = loopCount();
    m_currentLoop = 0;

    PlaySound((LPCWSTR)realPath.toStdWString().c_str(), NULL, SND_FILENAME | SND_ASYNC);
    m_currentLoop++;

    if (m_targetLoops != 1) {
        int durationMs = 2500; 
        m_loopTimer->start(durationMs);
    }
}

void MonitorConfig::stopAlertSound()
{
    m_loopTimer->stop();
    PlaySound(NULL, NULL, SND_FILENAME);
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

    if (m_targetLoops == -1) {
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

// ===== Settings =====
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

    QString eventType = settings.value("eventType", "火灾").toString();
    int eventLevel = settings.value("eventLevel", 1).toInt();
    int eventIndex = m_eventTypeCombo->findText(eventType);
    if (eventIndex != -1) {
        m_eventTypeCombo->setCurrentIndex(eventIndex);
    }
    m_eventLevelSpin->setValue(eventLevel);

    bool isDark = settings.value("darkMode", false).toBool();
    m_radioDark->setChecked(isDark);

    int interval = settings.value("detectionInterval", 5).toInt();
    m_intervalSpin->setValue(interval);

    // AI 设置
    m_aiEnableCheck->setChecked(settings.value("aiEnabled", false).toBool());
    
    // 如果没有保存过 URL，使用默认值
    QString savedUrl = settings.value("aiApiUrl", "").toString();
    if (savedUrl.isEmpty()) {
        savedUrl = "https://ark.cn-beijing.volces.com/api/v3";
    }
    m_aiUrlEdit->setText(savedUrl);

    m_aiKeyEdit->setText(settings.value("aiApiKey", "d42f588f-420d-4a52-9c1f-d25feae6cba8").toString());
    
    // 如果没有保存过模型，使用默认值
    QString savedModel = settings.value("aiModel", "").toString();
    if (savedModel.isEmpty()) {
        savedModel = "deepseek-v3-2-251201";
    }
    m_aiModelEdit->setText(savedModel);
}

void MonitorConfig::saveSettings()
{
    QSettings settings("MyCompany", "MonitorApp");

    settings.setValue("aiEnabled", m_aiEnableCheck->isChecked());
    settings.setValue("aiApiUrl", m_aiUrlEdit->text().trimmed());
    settings.setValue("aiApiKey", m_aiKeyEdit->text().trimmed());
    settings.setValue("aiModel", m_aiModelEdit->text().trimmed());

    settings.setValue("audioSourceIndex", m_audioSourceCombo->currentIndex());
    settings.setValue("localAudioPath", m_audioPathEdit->text());
    settings.setValue("playOnce", m_radioOnce->isChecked());
    settings.setValue("playInfinite", m_radioInfinite->isChecked());
    settings.setValue("customCount", m_spinCustom->value());

    settings.setValue("eventType", m_eventTypeCombo->currentText());
    settings.setValue("eventLevel", m_eventLevelSpin->value());

    settings.setValue("detectionInterval", m_intervalSpin->value());
    settings.setValue("darkMode", m_radioDark->isChecked());
}
