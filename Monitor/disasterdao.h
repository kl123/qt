#ifndef DISASTERDAO_H
#define DISASTERDAO_H

#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QString>

/**
 * @brief 灾害记录结构体
 * 用于创建和查询返回的完整记录
 */
struct DisasterRecord {
  qint64 id = 0;           // 记录ID (创建时由数据库生成)
  QString disasterType;    // 灾害类型 (如"火灾")
  QString location;        // 发生地点
  QString occurredAt;      // 发生时间
  QString content;         // 灾害描述
  QString systemAlarmAt;   // 系统告警时间
  int severity = 0;        // 严重等级 (0为最低)
  qint64 dispatcherId = 0; // 关联的调度员ID
  QString createdAt;       // 创建时间
  bool isDisaster = false;  // 是否确认为灾害（AI判定）
};

/**
 * @brief 灾害更新补丁
 * 用于局部更新灾害记录，has前缀字段为true时更新对应字段
 */
struct DisasterPatch {
  qint64 id = 0; // 要更新的记录ID

  bool hasDisasterType = false;
  QString disasterType; // 新的灾害类型

  bool hasLocation = false;
  QString location; // 新的地点

  bool hasOccurredAt = false;
  QString occurredAt; // 新的发生时间

  bool hasContent = false;
  QString content; // 新的描述内容

  bool hasSystemAlarmAt = false;
  QString systemAlarmAt; // 新的告警时间

  bool hasSeverity = false;
  int severity = 0; // 新的严重等级

  bool hasDispatcherId = false;
  qint64 dispatcherId = 0; // 新的调度员ID
};

/**
 * @brief 灾害任务记录结构体
 * 用于灾害任务的指派和管理
 */
struct DisasterTaskRecord {
  qint64 id = 0;            // 指派记录ID
  qint64 disasterId = 0;    // 灾害ID
  qint64 handlerUserId = 0; // 现场处置员ID
  int progress = 0;         // 进度 (0-100)
  QString assignedAt;       // 指派时间

  // 关联数据
  QString handlerName;  // 处置员姓名
  QString handlerPhone; // 处置员电话

  // 关联的灾害详情数据（用于任务列表展示）
  QString disasterType; // 灾害类型
  QString location;     // 地点
  int severity = 0;     // 严重程度
};

/**
 * @brief 灾害查询条件
 * 用于多条件组合查询
 */
struct DisasterQuery {
  QString keyword;          // 综合模糊搜索关键词
  QString disasterTypeLike; // 类型模糊匹配
  QString locationLike;     // 地点模糊匹配
  QString contentLike;      // 内容模糊匹配

  bool hasSeverityMin = false;
  int severityMin = 0; // 最小严重等级
  bool hasSeverityMax = false;
  int severityMax = 0; // 最大严重等级

  QString occurredAtFrom; // 发生时间起始
  QString occurredAtTo;   // 发生时间结束
  QString createdAtFrom;  // 创建时间起始
  QString createdAtTo;    // 创建时间结束

  // 排序方式枚举
  enum class OrderBy {
    CreatedAtDesc,  // 创建时间倒序
    CreatedAtAsc,   // 创建时间正序
    OccurredAtDesc, // 发生时间倒序
    OccurredAtAsc,  // 发生时间正序
    SeverityDesc,   // 严重等级倒序
    SeverityAsc,    // 严重等级正序
    IdDesc,         // ID倒序
    IdAsc,          // ID正序
  };

  OrderBy orderBy = OrderBy::CreatedAtDesc; // 默认按创建时间倒序
  int limit = 50;                           // 返回记录数量限制
  int offset = 0;                           // 分页偏移量
};

class DisasterDao {
public:
  // 创建灾害记录
  static bool createDisaster(const DisasterRecord &record, qint64 *id,
                             QString *errorMessage);

  /**
   * @brief 根据唯一ID获取灾害详细记录
   * @param id 灾害记录ID
   * @param record 用于接收查询结果的结构体指针
   * @param errorMessage 失败时的详细错误描述输出
   * @return 查询成功且记录存在返回 true，否则返回 false
   */
  static bool getDisasterById(qint64 id, DisasterRecord *record,
                              QString *errorMessage);

  /**
   * @brief 局部更新灾害记录信息 (Patch模式)
   * @param patch 包含待更新字段及其标志位的补丁对象
   * @param errorMessage 失败时的详细错误描述输出
   * @return 更新成功且受影响行数大于0返回 true
   */
  static bool updateDisaster(const DisasterPatch &patch, QString *errorMessage);

  /**
   * @brief 物理删除指定的灾害记录
   * @param id 待删除的灾害记录ID
   * @param errorMessage 失败时的详细错误描述输出
   * @return 删除成功返回 true
   */
  static bool deleteDisaster(qint64 id, QString *errorMessage);

  // 获取调度员相关的未被指派的灾害
  static bool getUnassignedDisastersByDispatcher(qint64 dispatcherId, int limit,
                                                 int offset,
                                                 QList<DisasterRecord> *records,
                                                 QString *errorMessage);

  // --- 任务指派相关 ---

  // 指派多个处理人到特定灾害
  static bool assignDisasterTasks(qint64 disasterId,
                                  const QList<qint64> &handlerIds,
                                  QString *errorMessage);

  // 查询特定灾害下的所有指派任务
  static bool getTasksForDisaster(qint64 disasterId,
                                  QList<DisasterTaskRecord> *tasks,
                                  QString *errorMessage);

  /**
   * @brief 修改指定的灾害指派任务进度
   * @param taskId 任务分派表中的唯一任务ID
   * @param progress 新进度值 (自动限制在 0-100 范围内)
   * @param errorMessage 失败时的详细错误描述输出
   * @return 修改成功返回 true
   */
  static bool updateDisasterTaskProgress(qint64 taskId, int progress,
                                         QString *errorMessage);

  /**
   * @brief 撤销/删除特定的灾害指派任务
   * @param taskId 待撤销的任务ID
   * @param errorMessage 失败时的详细错误描述输出
   * @return 删除成功返回 true，若任务不存在则返回 false 并填充错误说明
   */
  static bool deleteDisasterTask(qint64 taskId, QString *errorMessage);

  /**
   * @brief 根据用户角色获取对应的灾害任务列表
   * @param userId 操作用户ID
   * @param role 操作用户角色 ("指挥调度员" 可看全部，其他角色只能看自己的)
   * @param limit 返回条数上限
   * @param offset 分页偏移
   * @param tasks 结果返回列表
   * @param errorMessage 失败时的错误信息
   * @return true 成功, false 失败
   */
  static bool getTasksForUser(qint64 userId, const QString& role, int limit, int offset, QList<DisasterTaskRecord> *tasks, QString *errorMessage);


  // 综合查询灾害记录
  static bool queryDisasters(const DisasterQuery &query,
                             QList<DisasterRecord> *records,
                             QString *errorMessage);

  // (旧接口)按条件查询
  static bool queryDisastersByConditions(
      const QString &keyword, const QString &disasterTypeLike,
      const QString &locationLike, bool hasSeverityMin, int severityMin,
      bool hasSeverityMax, int severityMax, const QString &occurredAtFrom,
      const QString &occurredAtTo, int limit, int offset,
      QList<DisasterRecord> *records, QString *errorMessage);

  // === 数据统计接口 ===

  /**
   * @brief 根据指定日期查询当天发生或录入的灾害记录
   * @param date          日期字符串，格式 "YYYY-MM-DD"
   * @param useOccurredAt true=按发生时间(occurred_at)过滤，false=按创建时间(created_at)过滤
   * @param records       结果列表
   * @param errorMessage  失败时的详细错误描述输出
   * @return 查询成功返回 true
   */
  static bool getDisastersByDate(const QString &date,
                                 QList<DisasterRecord> *records,
                                 QString *errorMessage);

  /**
   * @brief 按日期范围统计各灾害类型发生数量
   * @param dateFrom      起始日期 "YYYY-MM-DD"（含）
   * @param dateTo        截止日期 "YYYY-MM-DD"（含，自动补全至当天 23:59:59）
   * @param useOccurredAt true=按发生时间统计，false=按创建时间统计
   * @param result        统计结果，key=灾害类型名称，value=发生数量
   * @param errorMessage  失败时的详细错误描述输出
   * @return 统计成功返回 true
   */
  static bool countDisasterTypesByDateRange(const QString &dateFrom,
                                            const QString &dateTo,
                                            QMap<QString, int> *result,
                                            QString *errorMessage);

  /**
   * @brief 用于大屏监控面板统一的基础数据统计信息聚合
   * 包含实时监测、今日/当月灾害总数及同环比相关指标、频繁统计与时间点特征
   */
  struct DashboardStats {
      int unassignedDisasters = 0;      // 尚未分配灾害
      int todayResolvedTasks = 0;       // 今日已解决灾害（任务进度100%）
      int processingTasks = 0;          // 正在处理灾害（有派单但未完成）

      int todayDisastersCount = 0;      // 今日发生的灾害数
      int monthDisastersCount = 0;      // 本月发生的灾害数
      int yesterdayDisastersCount = 0;  // 昨日发生的灾害数（计算发生率用）

      QString mostFrequentDisaster;     // 当前最频繁灾害
      int mostFrequentDisasterCount = 0;

      QString rareDisaster;             // 罕见灾害
      int rareDisasterCount = 0;

      QString proneTimePeriod;          // 灾害易发生时间段(h)，例如 "20-21"
  };

  /**
   * @brief 获取大屏监控统计数据
   * @param stats 统计数据存放结构体指针
   * @param errorMessage 失败时的详细错误描述输出
   * @return 统计成功返回 true
   */
  static bool getDashboardStats(DashboardStats *stats, QString *errorMessage);
};

#endif // DISASTERDAO_H
