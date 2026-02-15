#include "disasteranalyzer.h"
#include <QDebug>
#include <QMap>

DisasterAnalyzer::DisasterAnalyzer(QObject *parent)
    : QObject(parent), m_aiEnabled(false)
{
}

void DisasterAnalyzer::setAiConfig(const QString& apiUrl, const QString& apiKey, const QString& model)
{
    m_apiUrl = apiUrl;
    m_apiKey = apiKey;
    m_aiModel = model;
}

void DisasterAnalyzer::setAiEnabled(bool enabled)
{
    m_aiEnabled = enabled;
}

DisasterRecord DisasterAnalyzer::analyze(const QString& text)
{
    // 如果启用了 AI 且配置了 Key，优先尝试 AI 分析
    if (m_aiEnabled && !m_apiKey.isEmpty()) {
        DisasterRecord aiRecord = analyzeWithAi(text);
        // 简单判断 AI 是否返回了有效结果（例如类型不为空）
        if (!aiRecord.disasterType.isEmpty() && aiRecord.disasterType != "未知灾害") {
            return aiRecord;
        }
        qDebug() << "AI 分析失败或结果无效，回退到规则分析";
    }

    return analyzeWithRules(text);
}

DisasterRecord DisasterAnalyzer::analyzeWithRules(const QString& text)
{
    DisasterRecord record;
    
    // 1. 预处理文本
    QString cleanText = preprocess(text);
    record.content = cleanText;

    // 2. 提取灾害类型
    record.disasterType = extractType(cleanText);
    if (record.disasterType.isEmpty()) {
        record.disasterType = "未知灾害"; // 默认值
    }

    // 3. 提取地点
    record.location = extractLocation(cleanText);
    if (record.location.isEmpty()) {
        record.location = "未知地点"; // 默认值
    }

    // 4. 提取严重等级
    record.severity = extractSeverity(cleanText);

    // 5. 提取发生时间（优先从文本中提取，否则使用当前时间）
    QString extractedTime = extractTime(cleanText);
    if (!extractedTime.isEmpty()) {
        record.occurredAt = extractedTime;
    } else {
        record.occurredAt = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
    }

    // 系统告警时间通常是收到消息的时间
    record.systemAlarmAt = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");

    return record;
}

DisasterRecord DisasterAnalyzer::analyzeWithAi(const QString& text)
{
    DisasterRecord record;
    record.content = text; // 保留原始文本

    // 准备网络请求
    QNetworkAccessManager manager;
    QNetworkRequest request(QUrl(m_apiUrl + "/chat/completions"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", QString("Bearer %1").arg(m_apiKey).toUtf8());

    // 构造提示词
    QJsonObject systemMessage;
    systemMessage["role"] = "system";
    systemMessage["content"] = "你是一个灾害信息分析助手。请分析用户提供的文本，提取以下信息并以严格的JSON格式返回：\n"
                               "{\n"
                               "  \"disasterType\": \"灾害类型（如地震、火灾、洪水等，若无法确定则为'未知灾害'）\",\n"
                               "  \"location\": \"具体地点（若无法确定则为'未知地点'）\",\n"
                               "  \"severity\": 3, // 严重等级（1-10的整数，1为轻微，10为毁灭性，默认3）\n"
                               "  \"occurredAt\": \"YYYY-MM-DD HH:mm:ss\" // 发生时间（若未提及则留空）\n"
                               "}\n"
                               "仅返回JSON对象，不要包含Markdown标记或其他解释。";

    QJsonObject userMessage;
    userMessage["role"] = "user";
    userMessage["content"] = text;

    QJsonArray messages;
    messages.append(systemMessage);
    messages.append(userMessage);

    QJsonObject jsonBody;
    jsonBody["model"] = m_aiModel;
    jsonBody["messages"] = messages;
    jsonBody["temperature"] = 0.1; // 低温度以获得确定性结果

    // 发送请求（同步等待）
    QNetworkReply *reply = manager.post(request, QJsonDocument(jsonBody).toJson());
    
    QEventLoop loop;
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    if (reply->error() == QNetworkReply::NoError) {
        QByteArray responseData = reply->readAll();
        QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData);
        if (!jsonDoc.isNull()) {
            QJsonObject root = jsonDoc.object();
            // SiliconFlow 有时返回 choices 数组
            QJsonArray choices = root["choices"].toArray();
            if (!choices.isEmpty()) {
                QString content = choices[0].toObject()["message"].toObject()["content"].toString();
                
                // 使用正则提取 JSON 部分，以防 AI 输出额外文本
                QRegularExpression jsonRegex(R"(\{[\s\S]*\})");
                QRegularExpressionMatch match = jsonRegex.match(content);
                QString jsonStr = content;
                if (match.hasMatch()) {
                    jsonStr = match.captured(0);
                }
                
                // 解析 AI 返回的 JSON 内容
                QJsonDocument contentDoc = QJsonDocument::fromJson(jsonStr.toUtf8());
                if (!contentDoc.isNull() && contentDoc.isObject()) {
                    QJsonObject resultObj = contentDoc.object();
                    
                    record.disasterType = resultObj["disasterType"].toString("未知灾害");
                    record.location = resultObj["location"].toString("未知地点");
                    record.severity = resultObj["severity"].toInt(3);

                    if (record.disasterType.isEmpty()) record.disasterType = "未知灾害";
                    if (record.location.isEmpty()) record.location = "未知地点";
                    if (record.severity < 1) record.severity = 1;
                    if (record.severity > 10) record.severity = 10;
                    
                    QString timeStr = resultObj["occurredAt"].toString();
                    if (timeStr.isEmpty()) {
                        record.occurredAt = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
                    } else {
                        // 简单验证时间格式，如果不对则使用当前时间
                        QDateTime dt = QDateTime::fromString(timeStr, "yyyy-MM-dd HH:mm:ss");
                        if (dt.isValid()) {
                            record.occurredAt = timeStr;
                        } else {
                            record.occurredAt = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
                        }
                    }
                    
                    record.systemAlarmAt = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
                    
                    qDebug() << "AI 分析成功:" << content;
                    reply->deleteLater();
                    return record;
                } else {
                    qDebug() << "AI 返回的内容不是有效 JSON:" << content;
                }
            }
        }
    } else {
        qDebug() << "AI 请求失败:" << reply->errorString();
    }

    reply->deleteLater();
    return DisasterRecord(); // 返回空记录表示失败
}

QString DisasterAnalyzer::preprocess(const QString& text)
{
    // 去除首尾空白，合并多余空格
    return text.trimmed().simplified();
}

QString DisasterAnalyzer::extractType(const QString& text)
{
    // 定义常见灾害关键词
    static const QMap<QString, QStringList> typeKeywords = {
        {"地震", {"地震", "震感", "晃动"}},
        {"火灾", {"火灾", "起火", "冒烟", "火情", "着火"}},
        {"洪水", {"洪水", "淹水", "积水", "洪涝", "内涝"}},
        {"台风", {"台风", "飓风", "暴风"}},
        {"交通事故", {"车祸", "撞车", "追尾", "翻车"}},
        {"泄漏", {"泄漏", "漏气", "漏水", "漏油"}},
        {"停电", {"停电", "断电"}},
        {"人员受伤", {"受伤", "昏迷", "流血", "急救"}}
    };

    for (auto it = typeKeywords.begin(); it != typeKeywords.end(); ++it) {
        const QString& type = it.key();
        const QStringList& keywords = it.value();
        for (const QString& keyword : keywords) {
            if (text.contains(keyword)) {
                return type;
            }
        }
    }

    return QString();
}

QString DisasterAnalyzer::extractLocation(const QString& text)
{
    // 尝试匹配地址模式：xx省xx市xx区xx路xx号xx栋
    // 正则表达式匹配常见的中文地址后缀
    QRegularExpression regex(R"(([\u4e00-\u9fa50-9]+(省|市|区|县|街道|路|号|楼|栋|层|室|园|寓|苑|村|镇|乡))+)");
    QRegularExpressionMatch match = regex.match(text);
    if (match.hasMatch()) {
        return match.captured(0);
    }
    return QString();
}

int DisasterAnalyzer::extractSeverity(const QString& text)
{
    int severity = 1; // 默认等级 1 (最低)

    // 1. 直接匹配等级数字 "3级", "4.5级"
    QRegularExpression levelRegex(R"((\d+(\.\d+)?)级)");
    QRegularExpressionMatch levelMatch = levelRegex.match(text);
    if (levelMatch.hasMatch()) {
        bool ok;
        double val = levelMatch.captured(1).toDouble(&ok);
        if (ok) {
            severity = static_cast<int>(val);
            // 限制范围 1-10
            if (severity < 1) severity = 1;
            if (severity > 10) severity = 10;
            return severity;
        }
    }

    // 2. 根据描述性词汇判断
    if (text.contains("特大") || text.contains("毁灭性")) {
        return 5;
    }
    if (text.contains("重大") || text.contains("严重") || text.contains("高危")) {
        return 4;
    }
    if (text.contains("中度") || text.contains("一般")) {
        return 3;
    }
    if (text.contains("轻微") || text.contains("小")) {
        return 2;
    }

    return severity;
}

QString DisasterAnalyzer::extractTime(const QString& text)
{
    // 尝试匹配常见的日期时间格式
    // 2023-10-27 10:30
    QRegularExpression dtRegex(R"((\d{4}[-/]\d{1,2}[-/]\d{1,2}\s+\d{1,2}:\d{1,2}(:\d{1,2})?))");
    QRegularExpressionMatch match = dtRegex.match(text);
    if (match.hasMatch()) {
        return match.captured(0);
    }

    // 仅匹配时间 10:30
    QRegularExpression timeRegex(R"((\d{1,2}:\d{1,2}))");
    match = timeRegex.match(text);
    if (match.hasMatch()) {
        // 如果只有时间，假设是今天
        QString timeStr = match.captured(0);
        return QDateTime::currentDateTime().toString("yyyy-MM-dd") + " " + timeStr;
    }

    return QString();
}
