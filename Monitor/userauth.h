#ifndef USERAUTH_H
#define USERAUTH_H

#include <QtCore/QList>
#include <QtCore/QString>

/**
 * @brief 单位信息结构体
 */
struct UnitInfo {
  qint64 id = 0; // 单位ID
  QString name;  // 单位名称 (唯一)
};

/**
 * @brief 调度员信息结构体
 * 用于搜索调度员列表
 */
struct DispatcherInfo {
  qint64 id = 0;    // 调度员的用户ID
  QString username; // 用户名
  QString phone;    // 手机号
  QString unit;     // 所属单位名称
};

/**
 * @brief 认证用户信息结构体
 * 用于登录后返回的完整用户信息
 */
struct AuthUser {
  /** @brief 系统内部用户唯一标识ID */
  qint64 id = 0;
  /** @brief 登录账号名称 */
  QString username;
  /** @brief 绑定的有效联系电话 */
  QString phone;
  /** @brief 核心业务角色 (e.g. "指挥调度员", "现场处置员") */
  QString role;
  /** @brief 所属行政/事业机构名称 */
  QString unit;
  /** @brief 关联的领队调度员ID (仅在角色为 '现场处置员' 时作为父级索引有效) */
  qint64 dispatcherUserId = 0;
};

class UserAuth {
public:
  static AuthUser currentUser; // 全局保存的当前登录用户

  // 创建单位
  static bool createUnit(const QString &unitName, qint64 *unitId,
                         QString *errorMessage);

  // 搜索单位
  static bool searchUnits(const QString &keyword, int limit,
                          QList<UnitInfo> *units, QString *errorMessage);

  // 搜索指定单位下的调度员
  static bool searchDispatchersByUnit(qint64 unitId, const QString &keyword,
                                      int limit,
                                      QList<DispatcherInfo> *dispatchers,
                                      QString *errorMessage);

  // 查询指定调度员关联的现场处置员
  static bool getHandlersByDispatcherId(qint64 dispatcherId,
                                        QList<AuthUser> *handlers,
                                        QString *errorMessage);

  // 注册普通用户
  static bool registerUser(const QString &username, const QString &password,
                           const QString &phone, const QString &role,
                           const QString &unit, qint64 *userId,
                           QString *errorMessage);

  // 注册调度员 (绑定单位)
  static bool registerDispatcherWithUnitId(const QString &username,
                                           const QString &password,
                                           const QString &phone,
                                           const QString &role, qint64 unitId,
                                           qint64 *userId,
                                           QString *errorMessage);

  // 注册处置员 (绑定调度员)
  static bool registerHandlerWithDispatcherId(
      const QString &username, const QString &password, const QString &phone,
      qint64 dispatcherUserId, qint64 *userId, QString *errorMessage);

  // 注册用户并绑定单位 (通用)
  static bool registerUserWithUnitId(const QString &username,
                                     const QString &password,
                                     const QString &phone, const QString &role,
                                     qint64 unitId, qint64 *userId,
                                     QString *errorMessage);

  // 重置密码（直接修改指定用户的密码，无权限校验）
  static bool resetPassword(const QString &username, const QString &newPassword,
                            QString *errorMessage);

  // 用户登录
  static bool login(const QString &username, const QString &password,
                    AuthUser *user, QString *errorMessage);
};

#endif // USERAUTH_H
