#ifndef USERAUTH_H
#define USERAUTH_H

#include <QtCore/QList>
#include <QtCore/QString>

/**
 * @brief 单位信息结构体
 */
struct UnitInfo
{
    qint64 id = 0;      // 单位ID
    QString name;       // 单位名称 (唯一)
};

/**
 * @brief 调度员信息结构体
 * 用于搜索调度员列表
 */
struct DispatcherInfo
{
    qint64 id = 0;      // 调度员的用户ID
    QString username;   // 用户名
    QString phone;      // 手机号
    QString unit;       // 所属单位名称
};

/**
 * @brief 认证用户信息结构体
 * 用于登录后返回的完整用户信息
 */
struct AuthUser
{
    qint64 id = 0;              // 用户ID
    QString username;           // 用户名
    QString phone;              // 手机号
    QString role;               // 角色 (如"指挥调度员", "现场处置员")
    QString unit;               // 所属单位名称
    qint64 dispatcherUserId = 0; // 绑定的调度员ID (仅当角色为处置员时有效)
};

class UserAuth
{
public:
    // 创建单位
    static bool createUnit(
        const QString& unitName,
        qint64* unitId,
        QString* errorMessage);

    // 搜索单位
    static bool searchUnits(
        const QString& keyword,
        int limit,
        QList<UnitInfo>* units,
        QString* errorMessage);

    // 搜索指定单位下的调度员
    static bool searchDispatchersByUnit(
        qint64 unitId,
        const QString& keyword,
        int limit,
        QList<DispatcherInfo>* dispatchers,
        QString* errorMessage);

    // 注册普通用户
    static bool registerUser(
        const QString& username,
        const QString& password,
        const QString& phone,
        const QString& role,
        const QString& unit,
        qint64* userId,
        QString* errorMessage);

    // 注册调度员 (绑定单位)
    static bool registerDispatcherWithUnitId(
        const QString& username,
        const QString& password,
        const QString& phone,
        const QString& role,
        qint64 unitId,
        qint64* userId,
        QString* errorMessage);

    // 注册处置员 (绑定调度员)
    static bool registerHandlerWithDispatcherId(
        const QString& username,
        const QString& password,
        const QString& phone,
        qint64 dispatcherUserId,
        qint64* userId,
        QString* errorMessage);

    // 注册用户并绑定单位 (通用)
    static bool registerUserWithUnitId(
        const QString& username,
        const QString& password,
        const QString& phone,
        const QString& role,
        qint64 unitId,
        qint64* userId,
        QString* errorMessage);

    // 用户登录
    static bool login(
        const QString& username,
        const QString& password,
        AuthUser* user,
        QString* errorMessage);
};

#endif // USERAUTH_H
