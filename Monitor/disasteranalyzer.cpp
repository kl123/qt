#include "disasteranalyzer.h"
#include <QDebug>
#include <QMap>
#include <QSslSocket>
#include <QSslError>
#include <QProcess>
#include <QTemporaryFile>
#include <QCoreApplication>
#include <QFileInfo>

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
    if (m_aiEnabled && !m_apiKey.isEmpty()) {
        DisasterRecord aiRecord = analyzeWithAi(text);

        if (!aiRecord.isDisaster) {
            return aiRecord;
        }

        if (!aiRecord.disasterType.isEmpty() && aiRecord.disasterType != "未知灾害") {
            return aiRecord;
        }

        qDebug() << "AI 未识别出具体灾害类型(结果为: " << aiRecord.disasterType << ")，转由规则引擎分析...";

        DisasterRecord ruleRecord = analyzeWithRules(text);

        if (ruleRecord.disasterType == "未知灾害" && aiRecord.location != "未知地点") {
            if (ruleRecord.location == "未知地点") {
                ruleRecord.location = aiRecord.location;
                qDebug() << "合并 AI 提取的地点信息:" << aiRecord.location;
            }
        }
        
        return ruleRecord;
    }

    return analyzeWithRules(text);
}

DisasterRecord DisasterAnalyzer::analyzeWithRules(const QString& text)
{
    DisasterRecord record;

    QString cleanText = preprocess(text);
    record.content = cleanText;

    record.disasterType = extractType(cleanText);
    if (record.disasterType.isEmpty()) {
        record.disasterType = "未知灾害";
        record.isDisaster = false;
    } else {
        record.isDisaster = true;
    }

    record.location = extractLocation(cleanText);
    if (record.location.isEmpty()) {
        record.location = "未知地点";
    }

    record.severity = extractSeverity(cleanText);

    QString extractedTime = extractTime(cleanText);
    if (!extractedTime.isEmpty()) {
        record.occurredAt = extractedTime;
    } else {
        record.occurredAt = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
    }

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
    systemMessage["content"] = "你是一个灾害信息分析助手。请分析用户提供的文本，判断是否属于灾害或紧急事件。\n"
                               "提取以下信息并以严格的JSON格式返回：\n"
                               "{\n"
                               "  \"isDisaster\": true/false, // 核心判断：这是否是一条关于灾害、事故或紧急情况的信息？\n"
                               "  \"disasterType\": \"灾害类型（如地震、火灾、洪水、交通事故等，若不是灾害则留空）\",\n"
                               "  \"location\": \"提取文本中发生灾害的具体物理地点地址（必须尽可能详细，如包含省市区或标志性建筑物），若文本中未提及任何位置则为'未知地点'\",\n"
                               "  \"severity\": 3, // 严重等级（1-10的整数，1为轻微，10为毁灭性，默认3）\n"
                               "  \"occurredAt\": \"YYYY-MM-DD HH:mm:ss\" // 发生时间（若未提及则留空）\n"
                               "}\n"
                               "注意：日常聊天、无意义文本、或者明显非灾害的内容（如'我已经在路上了'、'去吃饭'），请务必将 isDisaster 设为 false。\n"
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

    // 检查 SSL 支持
    bool sslSupported = QSslSocket::supportsSsl();
    if (!sslSupported) {
        emit log("⚠️ 检测到环境不支持 SSL/HTTPS，AI 请求将尝试使用 curl...");
        qDebug() << "警告: 此环境不支持 SSL/HTTPS。版本:" << QSslSocket::sslLibraryBuildVersionString();
        
        // 尝试使用 curl.exe 作为回退方案
        QTemporaryFile tempFile;
        if (tempFile.open()) {
            tempFile.write(QJsonDocument(jsonBody).toJson());
            tempFile.close();
            
            QString tempFilePath = tempFile.fileName();
            
            QString program = "curl.exe";
            QStringList arguments;
            arguments << "-X" << "POST" << m_apiUrl + "/chat/completions";
            arguments << "-H" << "Content-Type: application/json";
            arguments << "-H" << QString("Authorization: Bearer %1").arg(m_apiKey);
            arguments << "-d" << "@" + tempFilePath;
            arguments << "-s"; // 静默模式

            emit log("🚀 正在通过 curl 发送请求...");
            emit log(QString("URL: %1").arg(m_apiUrl + "/chat/completions"));
            
            QProcess process;
            process.start(program, arguments);
            
            // 使用 QEventLoop 等待 curl 完成，保持 UI 响应
            QEventLoop loop;
            connect(&process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), &loop, &QEventLoop::quit);
            loop.exec();
            
            if (process.exitStatus() == QProcess::NormalExit && process.exitCode() == 0) {
                QByteArray output = process.readAllStandardOutput();
                qDebug() << "curl 请求成功，收到数据:" << output.size() << "bytes";
                emit log("✅ 请求成功，收到响应数据");
                emit log("📄 原始响应内容：\n" + QString::fromUtf8(output));
                
                // 解析 curl 返回的数据
                QJsonDocument jsonDoc = QJsonDocument::fromJson(output);
                if (!jsonDoc.isNull()) {
                     QJsonObject root = jsonDoc.object();
                     QJsonArray choices = root["choices"].toArray();
                     if (!choices.isEmpty()) {
                         QString content = choices[0].toObject()["message"].toObject()["content"].toString();
                         emit log("🧠 AI 思考与回答内容：\n" + content);
                         
                         // 提取 JSON
                         QRegularExpression jsonRegex(R"(\{[\s\S]*\})");
                         QRegularExpressionMatch match = jsonRegex.match(content);
                         QString jsonStr = content;
                         if (match.hasMatch()) {
                             jsonStr = match.captured(0);
                         }
                         
                         QJsonDocument contentDoc = QJsonDocument::fromJson(jsonStr.toUtf8());
                         if (!contentDoc.isNull() && contentDoc.isObject()) {
                              QJsonObject resultObj = contentDoc.object();
                              
                              // 核心判断：是否为灾害
                              bool isDisaster = resultObj["isDisaster"].toBool(false);
                              
                              if (!isDisaster) {
                                  qDebug() << "AI 判定为非灾害信息:" << content;
                                  emit log("⚖️ AI 判定结果：【非灾害信息】");
                                  emit log("ℹ️ 忽略此条消息，不进行入库处理。");
                                  DisasterRecord emptyRec;
                                  emptyRec.isDisaster = false;
                                  return emptyRec; // 返回对象并标记为非灾害
                              }
                              
                              emit log("⚖️ AI 判定结果：【⚠️ 确认灾害/紧急事件】");
                              record.isDisaster = true;
                              record.disasterType = resultObj["disasterType"].toString("未知灾害");
                              record.location = resultObj["location"].toString("未知地点");
                              record.severity = resultObj["severity"].toInt(3);
                              
                              // 简单验证和默认值
                              if (record.disasterType.isEmpty()) record.disasterType = "未知灾害";
                             if (record.location.isEmpty()) record.location = "未知地点";
                             if (record.severity < 1) record.severity = 1;
                             if (record.severity > 10) record.severity = 10;
                             
                             QString timeStr = resultObj["occurredAt"].toString();
                             if (!timeStr.isEmpty() && QDateTime::fromString(timeStr, "yyyy-MM-dd HH:mm:ss").isValid()) {
                                 record.occurredAt = timeStr;
                             } else {
                                 record.occurredAt = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
                             }
                             record.systemAlarmAt = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
                             
                             qDebug() << "AI 分析成功 (via curl):" << content;
                             emit log("🎉 解析成功，提取结果如下：");
                             emit log(QString("类型: %1").arg(record.disasterType));
                             emit log(QString("地点: %1").arg(record.location));
                             emit log(QString("等级: %1").arg(record.severity));
                             return record;
                         }
                     }
                }
                qDebug() << "curl 返回的数据无法解析:" << output;
                emit log("❌ 返回数据解析失败或格式不正确");
            } else {
                qDebug() << "curl 执行失败，退出码:" << process.exitCode();
                qDebug() << "curl 错误输出:" << process.readAllStandardError();
                emit log("❌ curl 执行失败: " + process.errorString());
            }
            // 临时文件会在析构时自动删除
        } else {
            qDebug() << "无法创建临时文件用于 curl 请求";
            emit log("❌ 无法创建临时文件");
        }
        
        // curl 也失败了，或者无法启动，回退到空
        DisasterRecord errorRec;
        errorRec.isDisaster = false;
        return errorRec; 
    }

    // 发送请求（同步等待）
    emit log("🚀 正在通过 QtNetwork (SSL) 发送请求...");
    QNetworkReply *reply = manager.post(request, QJsonDocument(jsonBody).toJson());
    
    QEventLoop loop;
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    connect(reply, QOverload<const QList<QSslError>&>::of(&QNetworkReply::sslErrors), [reply](const QList<QSslError> &errors) {
        for (const QSslError &e : errors) {
            qDebug() << "AI 请求 SSL 错误:" << e.errorString();
        }
        // 尝试忽略 SSL 错误以便继续
        reply->ignoreSslErrors();
    });
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
                    
                    // 核心判断：是否为灾害
                    bool isDisaster = resultObj["isDisaster"].toBool(false);
                    
                    if (!isDisaster) {
                        qDebug() << "AI 判定为非灾害信息:" << content;
                        emit log("⚖️ AI 判定结果：【非灾害信息】");
                        emit log("ℹ️ 忽略此条消息，不进行入库处理。");
                        DisasterRecord emptyRec;
                        emptyRec.isDisaster = false;
                        return emptyRec; // 返回对象并显式标记为非灾害
                    }
                    
                    emit log("⚖️ AI 判定结果：【⚠️ 确认灾害/紧急事件】");
                    record.isDisaster = true;
                    record.disasterType = resultObj["disasterType"].toString("未知灾害");
                    record.location = resultObj["location"].toString("未知地点");
                    record.severity = resultObj["severity"].toInt(3);

                    // 简单验证和默认值
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
    DisasterRecord errorRec;
    errorRec.isDisaster = false;
    return errorRec; // 返回空记录表示失败
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
        {"人员受伤", {"受伤", "摔伤", "昏迷", "流血", "急救"}}
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
    // 注意：使用 QStringLiteral 避免 C++ 原始字符串字面量中 \u 转义的问题
    // 使用 \uXXXX 直接嵌入 Unicode 字符，而不是传递 \u 转义序列给正则引擎
    QRegularExpression regex(QStringLiteral("([\u4e00-\u9fa50-9]+(省|市|区|县|街道|路|号|楼|栋|层|室|园|寓|苑|村|镇|乡|旁|附近|交界处|口))+"));
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
