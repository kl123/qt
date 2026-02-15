#ifndef DISASTERANALYZER_H
#define DISASTERANALYZER_H

#include <QString>
#include <QRegularExpression>
#include <QDateTime>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QEventLoop>
#include "disasterdao.h"

/**
 * @brief 灾害信息分析器
 * 负责从 OCR 识别出的文本中提取灾害类型、地点、等级等关键信息。
 * 支持规则匹配（正则/关键词）和 AI/LLM API 分析。
 */
class DisasterAnalyzer : public QObject
{
    Q_OBJECT
public:
    DisasterAnalyzer(QObject *parent = nullptr);

    /**
     * @brief 分析文本并生成灾害记录
     * @param text OCR 识别出的原始文本
     * @return 填充好关键字段的 DisasterRecord
     */
    DisasterRecord analyze(const QString& text);

    // 设置 AI 配置
    void setAiConfig(const QString& apiUrl, const QString& apiKey, const QString& model);
    void setAiEnabled(bool enabled);

private:
    // 规则分析
    DisasterRecord analyzeWithRules(const QString& text);
    
    // AI 分析
    DisasterRecord analyzeWithAi(const QString& text);

    // 提取灾害类型
    QString extractType(const QString& text);
    
    // 提取地点
    QString extractLocation(const QString& text);
    
    // 提取严重等级
    int extractSeverity(const QString& text);
    
    // 提取发生时间（如有）
    QString extractTime(const QString& text);

    // 辅助：预处理文本
    QString preprocess(const QString& text);

    // AI 配置
    bool m_aiEnabled;
    QString m_apiUrl;
    QString m_apiKey;
    QString m_aiModel;
};

#endif // DISASTERANALYZER_H
