#ifndef USERAUTH_H
#define USERAUTH_H

#include <QtCore/QList>
#include <QtCore/QString>

struct UnitInfo
{
    qint64 id = 0;
    QString name;
};

struct DispatcherInfo
{
    qint64 id = 0;
    QString username;
    QString phone;
    QString unit;
};

struct AuthUser
{
    qint64 id = 0;
    QString username;
    QString phone;
    QString role;
    QString unit;
    qint64 dispatcherUserId = 0;
};

class UserAuth
{
public:
    static bool createUnit(
        const QString& unitName,
        qint64* outUnitId,
        QString* errorMessage);

    static bool searchUnits(
        const QString& keyword,
        int limit,
        QList<UnitInfo>* outUnits,
        QString* errorMessage);

    static bool searchDispatchersByUnit(
        qint64 unitId,
        const QString& keyword,
        int limit,
        QList<DispatcherInfo>* outDispatchers,
        QString* errorMessage);

    static bool registerUser(
        const QString& username,
        const QString& password,
        const QString& phone,
        const QString& role,
        const QString& unit,
        qint64* outUserId,
        QString* errorMessage);

    static bool registerDispatcherWithUnitId(
        const QString& username,
        const QString& password,
        const QString& phone,
        const QString& role,
        qint64 unitId,
        qint64* outUserId,
        QString* errorMessage);

    static bool registerHandlerWithDispatcherId(
        const QString& username,
        const QString& password,
        const QString& phone,
        qint64 dispatcherUserId,
        qint64* outUserId,
        QString* errorMessage);

    static bool registerUserWithUnitId(
        const QString& username,
        const QString& password,
        const QString& phone,
        const QString& role,
        qint64 unitId,
        qint64* outUserId,
        QString* errorMessage);

    static bool login(
        const QString& username,
        const QString& password,
        AuthUser* outUser,
        QString* errorMessage);
};

#endif // USERAUTH_H
