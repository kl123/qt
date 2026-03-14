# Monitor 项目文档

## 目录
- [数据库结构](#数据库结构)
- [核心接口文档 (C++ API)](#核心接口文档-c-api)
  - [1. 用户认证与管理 (UserAuth)](#1-用户认证与管理-userauth)
  - [2. 灾害数据管理 (DisasterDao)](#2-灾害数据管理-disasterdao)
  - [3. OCR 文字识别 (OcrHelper)](#3-ocr-文字识别-ocrhelper)
- [SQLite 工具使用](#sqlite-工具使用)

---

## 数据库结构

### 1. units
| 字段 | 类型 | 约束/默认 |
|---|---|---|
| id | INTEGER | PK, AUTOINCREMENT |
| name | TEXT | NOT NULL, UNIQUE |
| created_at | TEXT | DEFAULT datetime('now') |

### 2. users
| 字段 | 类型 | 约束/默认 |
|---|---|---|
| id | INTEGER | PK, AUTOINCREMENT |
| username | TEXT | NOT NULL, UNIQUE |
| password | TEXT | NOT NULL |
| phone | TEXT | |
| role | TEXT | NOT NULL |
| unit_id | INTEGER | FK -> units.id |
| dispatcher_user_id | INTEGER | FK -> users.id |
| created_at | TEXT | DEFAULT datetime('now') |

### 3. disasters
| 字段 | 类型 | 约束/默认 |
|---|---|---|
| id | INTEGER | PK, AUTOINCREMENT |
| disaster_type | TEXT | NOT NULL |
| location | TEXT | |
| occurred_at | TEXT | |
| content | TEXT | |
| severity | INTEGER | DEFAULT 0 |
| dispatcher_id | INTEGER | FK -> users.id |
| created_at | TEXT | DEFAULT datetime('now') |

### 4. disaster_tasks
| 字段 | 类型 | 约束/默认 |
|---|---|---|
| id | INTEGER | PK, AUTOINCREMENT |
| disaster_id | INTEGER | FK -> disasters.id |
| handler_user_id | INTEGER | FK -> users.id |
| progress | INTEGER | DEFAULT 0 |
| assigned_at | TEXT | DEFAULT datetime('now') |

---

## 核心接口文档 (C++ API)

### 1. 用户认证与管理 (UserAuth)

#### 1.0 相关数据结构 (Structs)

##### **1.0.1 UnitInfo**
| 字段名 | 类型 | 说明 |
|---|---|---|
| `id` | `qint64` | 单位ID |
| `name` | `QString` | 单位名称 |

##### **1.0.2 DispatcherInfo**
| 字段名 | 类型 | 说明 |
|---|---|---|
| `id` | `qint64` | 具有调度员角色的用户ID |
| `username` | `QString` | 用户名 |
| `phone` | `QString` | 手机号 |
| `unit` | `QString` | 单位名称 |

##### **1.0.3 AuthUser**
| 字段名 | 类型 | 说明 |
|---|---|---|
| `id` | `qint64` | 用户ID |
| `username` | `QString` | 用户名 |
| `phone` | `QString` | 手机号 |
| `role` | `QString` | 角色（如：现场调度员、现场处置员） |
| `unit` | `QString` | 单位名称 |
| `dispatcherUserId` | `qint64` | 所关联的调度员ID |

#### 1.1 创建单位

- **接口定义**
  ```cpp
  static bool createUnit(const QString& unitName, qint64* unitId, QString* errorMessage);
  ```

- **输入参数**
| 参数名 | 类型 | 说明 |
|---|---|---|
| unitName | const QString& | 单位名称 |

- **输出参数 / 返回值**
| 参数名 | 类型 | 说明 |
|---|---|---|
| unitId | qint64* | 单位ID |
| errorMessage | QString* | 错误信息 |
| (return) | bool | 成功返回 `true`，失败返回 `false` |

- **调用示例**
  ```cpp
  qint64 unitId;
  QString err;
  if (UserAuth::createUnit("某某消防大队", &unitId, &err)) {
      qDebug() << "新单位ID:" << unitId;
  }
  ```

#### 1.2 搜索单位

- **接口定义**
  ```cpp
  static bool searchUnits(const QString& keyword, int limit, QList<UnitInfo>* units, QString* errorMessage);
  ```

- **输入参数**
| 参数名 | 类型 | 说明 |
|---|---|---|
| keyword | const QString& | 搜索关键词 |
| limit | int | 返回结果限制数 |

- **输出参数 / 返回值**
| 参数名 | 类型 | 说明 |
|---|---|---|
| units | QList<UnitInfo>* | 单位列表 |
| errorMessage | QString* | 错误信息 |
| (return) | bool | 成功返回 `true`，失败返回 `false` |

- **调用示例**
  ```cpp
  QList<UnitInfo> units;
  QString err;
  if (UserAuth::searchUnits("消防", 10, &units, &err)) {
      for (const auto& unit : units) {
          qDebug() << unit.name << unit.id;
      }
  }
  ```

#### 1.3 注册调度员 (带单位)

- **接口定义**
  ```cpp
  static bool registerDispatcherWithUnitId(
      const QString& username, const QString& password, const QString& phone,
      const QString& role, qint64 unitId,
      qint64* userId, QString* errorMessage);
  ```

- **输入参数**
| 参数名 | 类型 | 说明 |
|---|---|---|
| username | const QString& | 用户名 |
| password | const QString& | 密码 |
| phone | const QString& | 手机号 |
| role | const QString& | 角色名称（如 "指挥调度员"） |
| unitId | qint64 | 所属单位 ID |

- **输出参数 / 返回值**
| 参数名 | 类型 | 说明 |
|---|---|---|
| userId | qint64* | 新用户ID |
| errorMessage | QString* | 错误信息 |
| (return) | bool | 成功返回 `true`，失败返回 `false` |

- **调用示例**
  ```cpp
  qint64 userId;
  QString err;
  if (UserAuth::registerDispatcherWithUnitId("admin", "123456", "13800000000", "指挥调度员", 1, &userId, &err)) {
      qDebug() << "ID:" << userId;
  }
  ```

#### 1.4 搜索调度员

- **接口定义**
  ```cpp
  static bool searchDispatchersByUnit(qint64 unitId, const QString& keyword, int limit, 
                                    QList<DispatcherInfo>* dispatchers, QString* errorMessage);
  ```

- **输入参数**
| 参数名 | 类型 | 说明 |
|---|---|---|
| unitId | qint64 | 单位ID |
| keyword | const QString& | 搜索关键词 |
| limit | int | 结果数量上限 |

- **输出参数 / 返回值**
| 参数名 | 类型 | 说明 |
|---|---|---|
| dispatchers | QList<DispatcherInfo>* | 调度员列表 |
| errorMessage | QString* | 错误信息 |
| (return) | bool | 成功返回 `true`，失败返回 `false` |

- **调用示例**
  ```cpp
  QList<DispatcherInfo> dispatchers;
  QString err;
  if (UserAuth::searchDispatchersByUnit(1, "张", 10, &dispatchers, &err)) {
      for (const auto& d : dispatchers) {
          qDebug() << d.username << d.phone;
      }
  }
  ```

#### 1.5 注册处置员 (绑定调度员)

- **接口定义**
  ```cpp
  static bool registerHandlerWithDispatcherId(
      const QString& username, const QString& password, const QString& phone,
      qint64 dispatcherUserId, qint64* userId, QString* errorMessage);
  ```

- **输入参数**
| 参数名 | 类型 | 说明 |
|---|---|---|
| username | const QString& | 用户名 |
| password | const QString& | 密码 |
| phone | const QString& | 手机号 |
| dispatcherUserId | qint64 | 关联的调度员ID |

- **输出参数 / 返回值**
| 参数名 | 类型 | 说明 |
|---|---|---|
| userId | qint64* | 新处置员ID |
| errorMessage | QString* | 错误信息 |
| (return) | bool | 成功返回 `true`，失败返回 `false` |

- **调用示例**
  ```cpp
  qint64 userId;
  QString err;
  if (UserAuth::registerHandlerWithDispatcherId("handler01", "123456", "13900000000", 101, &userId, &err)) {
      qDebug() << "处置员ID:" << userId;
  }
  ```

#### 1.6 用户登录

- **接口定义**
  ```cpp
  static bool login(const QString& username, const QString& password, 
                  AuthUser* user, QString* errorMessage);
  ```

- **输入参数**
| 参数名 | 类型 | 说明 |
|---|---|---|
| username | const QString& | 用户名 |
| password | const QString& | 密码 |

- **输出参数 / 返回值**
| 参数名 | 类型 | 说明 |
|---|---|---|
| user | AuthUser* | 已登录用户信息（包含角色与单位） |
| errorMessage | QString* | 错误信息 |
| (return) | bool | 成功返回 `true`，失败返回 `false` |

- **调用示例**
  ```cpp
  AuthUser user;
  QString err;
  if (UserAuth::login("zhangsan", "123456", &user, &err)) {
      qDebug() << user.username << user.role << user.unit;
  }
  ```

#### 1.7 获取调度员下属处置员

- **接口定义**
  ```cpp
  static bool getHandlersByDispatcherId(qint64 dispatcherId, 
                                        QList<AuthUser>* handlers, 
                                        QString* errorMessage);
  ```

- **输入参数**
| 参数名 | 类型 | 说明 |
|---|---|---|
| dispatcherId | qint64 | 指挥调度员的ID |

- **输出参数 / 返回值**
| 参数名 | 类型 | 说明 |
|---|---|---|
| handlers | QList<AuthUser>* | 该调度员下属的现场处置员列表 |
| errorMessage | QString* | 错误信息 |
| (return) | bool | 成功返回 `true`，失败返回 `false` |

- **调用示例**
  ```cpp
  QList<AuthUser> handlers;
  QString err;
  if (UserAuth::getHandlersByDispatcherId(101, &handlers, &err)) {
      for (const auto& h : handlers) {
          qDebug() << h.username << h.phone;
      }
  }
  ```

---

### 2. 灾害数据管理 (DisasterDao)

#### 2.0 相关数据结构 (Structs)

##### **2.0.1 DisasterRecord**
| 字段名 | 类型 | 说明 |
|---|---|---|
| `id` | `qint64` | 记录ID |
| `disasterType` | `QString` | 灾害类型 |
| `location` | `QString` | 地点 |
| `occurredAt` | `QString` | 发生时间 |
| `content` | `QString` | 具体内容 |
| `systemAlarmAt` | `QString` | 系统告警时间 |
| `severity` | `int` | 严重警告等级 |
| `dispatcherId` | `qint64` | 指挥调度员ID |
| `createdAt` | `QString` | 创建记录的时间 |

##### **2.0.2 DisasterPatch**
| 字段名 | 类型 | 说明 |
|---|---|---|
| `id` | `qint64` | 记录ID |
| `hasDisasterType` / `disasterType` | `bool` / `QString` |  |
| `hasLocation` / `location` | `bool` / `QString` |  |
| `hasOccurredAt` / `occurredAt` | `bool` / `QString` |  |
| `hasContent` / `content` | `bool` / `QString` |  |
| `hasSystemAlarmAt` / `systemAlarmAt` | `bool` / `QString` |  |
| `hasSeverity` / `severity` | `bool` / `int` |  |
| `hasDispatcherId` / `dispatcherId` | `bool` / `qint64` |  |

##### **2.0.3 DisasterQuery**
| 字段名 | 类型 | 说明 |
|---|---|---|
| `keyword` | `QString` | 综合关键词 |
| `disasterTypeLike` | `QString` | 类型匹配 |
| `locationLike` | `QString` | 地点匹配 |
| `contentLike` | `QString` | 内容匹配 |
| `hasSeverityMin` / `severityMin` | `bool` / `int` |  |
| `hasSeverityMax` / `severityMax` | `bool` / `int` |  |
| `occurredAtFrom` | `QString` | 发生时间 |
| `occurredAtTo` | `QString` |  |
| `createdAtFrom` | `QString` | 创建时间 |
| `createdAtTo` | `QString` |  |
| `orderBy` | `enum OrderBy` | 排序方式 |
| `limit` | `int` | 返回限制 |
| `offset` | `int` | 偏移量 |

##### **2.0.4 DisasterTaskRecord**
| 字段名 | 类型 | 说明 |
|---|---|---|
| `id` | `qint64` | 任务分配表ID |
| `disasterId` | `qint64` | 具体灾害的ID |
| `handlerUserId` | `qint64` | 派单的现场处置员ID |
| `progress` | `int` | 进度 0-100 |
| `assignedAt` | `QString` | 任务分配时间 |
| `handlerName` | `QString` | 处置员用户名 |
| `handlerPhone` | `QString` | 处置员手机号 |

#### 2.1 创建灾害记录

- **接口定义**
  ```cpp
  static bool createDisaster(const DisasterRecord& record, qint64* id, QString* errorMessage);
  ```

- **输入参数**
| 参数名 | 类型 | 说明 |
|---|---|---|
| record | const DisasterRecord& | 灾害结构体 |

- **输出参数 / 返回值**
| 参数名 | 类型 | 说明 |
|---|---|---|
| id | qint64* | 新记录的插入ID |
| errorMessage | QString* | 错误信息 |
| (return) | bool | 成功返回 `true`，失败返回 `false` |

- **调用示例**
  ```cpp
  DisasterRecord rec;
  rec.disasterType = "火灾";
  rec.severity = 3;
  rec.dispatcherId = 101; 
  qint64 id;
  QString err;
  if (DisasterDao::createDisaster(rec, &id, &err)) {
      qDebug() << "灾害ID:" << id;
  }
  ```

#### 2.2 查询灾害 (条件查询)

- **接口定义**
  ```cpp
  static bool queryDisasters(const DisasterQuery& query, QList<DisasterRecord>* records, QString* errorMessage);
  ```

- **输入参数**
| 参数名 | 类型 | 说明 |
|---|---|---|
| query | const DisasterQuery& | 条件组合 |

- **输出参数 / 返回值**
| 参数名 | 类型 | 说明 |
|---|---|---|
| records | QList<DisasterRecord>* | 指向保存结果的列表 |
| errorMessage | QString* | 错误信息 |
| (return) | bool | 成功返回 `true`，失败返回 `false` |

- **调用示例**
  ```cpp
  DisasterQuery q;
  q.severityMin = 2;
  q.orderBy = DisasterQuery::OrderBy::SeverityDesc;
  QList<DisasterRecord> records;
  QString err;
  if (DisasterDao::queryDisasters(q, &records, &err)) {
      for (const auto& r : records) qDebug() << r.id << r.disasterType;
  }
  ```

#### 2.3 更新灾害信息

- **接口定义**
  ```cpp
  static bool updateDisaster(const DisasterPatch& patch, QString* errorMessage);
  ```

- **输入参数**
| 参数名 | 类型 | 说明 |
|---|---|---|
| patch | const DisasterPatch& | 具有标志位更新要求的请求 |

- **输出参数 / 返回值**
| 参数名 | 类型 | 说明 |
|---|---|---|
| errorMessage | QString* | 错误信息 |
| (return) | bool | 成功返回 `true`，失败返回 `false` |

- **调用示例**
  ```cpp
  DisasterPatch patch;
  patch.id = 101;
  patch.hasSeverity = true;
  patch.severity = 4;
  QString err;
  DisasterDao::updateDisaster(patch, &err);
  ```

#### 2.4 删除灾害

- **接口定义**
  ```cpp
  static bool deleteDisaster(qint64 id, QString* errorMessage);
  ```

- **输入参数**
| 参数名 | 类型 | 说明 |
|---|---|---|
| id | qint64 | 待删除的ID |

- **输出参数 / 返回值**
| 参数名 | 类型 | 说明 |
|---|---|---|
| errorMessage | QString* | 错误信息 |
| (return) | bool | 成功返回 `true`，失败返回 `false` |

- **调用示例**
  ```cpp
  QString err;
  if (DisasterDao::deleteDisaster(101, &err)) {
      qDebug() << "已删除";
  }
  ```

#### 2.5 获取未指派灾害

- **接口定义**
  ```cpp
  static bool getUnassignedDisastersByDispatcher(
      qint64 dispatcherId, int limit, int offset,
      QList<DisasterRecord>* records, QString* errorMessage);
  ```

- **输入参数**
| 参数名 | 类型 | 说明 |
|---|---|---|
| dispatcherId | qint64 | 指挥调度员ID |
| limit | int | 返回限制 |
| offset | int | 偏移量 |

- **输出参数 / 返回值**
| 参数名 | 类型 | 说明 |
|---|---|---|
| records | QList<DisasterRecord>* | 未指派的灾害数组 |
| errorMessage | QString* | 错误信息 |
| (return) | bool | 成功返回 `true`，由于没有分配人，返回的是剩余待接单记录 |

- **调用示例**
  ```cpp
  QList<DisasterRecord> unassigned;
  QString err;
  if (DisasterDao::getUnassignedDisastersByDispatcher(101, 50, 0, &unassigned, &err)) {
      for(const auto& r : unassigned) qDebug() << r.id;
  }
  ```

#### 2.6 分配灾害任务

- **接口定义**
  ```cpp
  static bool assignDisasterTasks(qint64 disasterId, const QList<qint64>& handlerIds, QString* errorMessage);
  ```

- **输入参数**
| 参数名 | 类型 | 说明 |
|---|---|---|
| disasterId | qint64 | 灾害ID |
| handlerIds | const QList<qint64>& | 多位现场处置员的ID列表 |

- **输出参数 / 返回值**
| 参数名 | 类型 | 说明 |
|---|---|---|
| errorMessage | QString* | 错误信息 |
| (return) | bool | 成功执行底层事务则返回 `true` |

- **调用示例**
  ```cpp
  QList<qint64> ops = {201, 203};
  QString err;
  if (DisasterDao::assignDisasterTasks(55, ops, &err)) {
      qDebug() << "已同时分配2位处置员";
  }
  ```

#### 2.7 获取灾害任务列表

- **接口定义**
  ```cpp
  static bool getTasksForDisaster(qint64 disasterId, QList<DisasterTaskRecord>* tasks, QString* errorMessage);
  ```

- **输入参数**
| 参数名 | 类型 | 说明 |
|---|---|---|
| disasterId | qint64 | 被分配下去的灾害的ID |

- **输出参数 / 返回值**
| 参数名 | 类型 | 说明 |
|---|---|---|
| tasks | QList<DisasterTaskRecord>* | 所有的任务单、处置员信息和进度数据 |
| errorMessage | QString* | 错误信息 |
| (return) | bool | 成功返回 `true` |

- **调用示例**
  ```cpp
  QList<DisasterTaskRecord> tasks;
  QString err;
  if (DisasterDao::getTasksForDisaster(55, &tasks, &err)) {
      for(const auto& t : tasks) qDebug() << t.handlerName << " 进度:" << t.progress;
  }
  ```

#### 2.8 更新灾害任务进度

- **接口定义**
  ```cpp
  static bool updateDisasterTaskProgress(qint64 taskId, int progress, QString* errorMessage);
  ```

- **输入参数**
| 参数名 | 类型 | 说明 |
|---|---|---|
| taskId | qint64 | 具体一条的任务单ID （不是灾害ID本身） |
| progress | int | 进度，范围0~100 |

- **输出参数 / 返回值**
| 参数名 | 类型 | 说明 |
|---|---|---|
| errorMessage | QString* | 错误信息 |
| (return) | bool | 成功返回 `true` |

- **调用示例**
  ```cpp
  QString err;
  if (DisasterDao::updateDisasterTaskProgress(302, 55, &err)) {
      qDebug() << "进度更新至55%";
  }
  ```

#### 2.9 删除灾害任务

- **接口定义**
  ```cpp
  static bool deleteDisasterTask(qint64 taskId, QString* errorMessage);
  ```

- **输入参数**
| 参数名 | 类型 | 说明 |
|---|---|---|
| taskId | qint64 | 需要撤销修改的具体任务单 |

- **输出参数 / 返回值**
| 参数名 | 类型 | 说明 |
|---|---|---|
| errorMessage | QString* | 错误信息 |
| (return) | bool | 成功返回 `true` |

- **调用示例**
  ```cpp
  QString err;
  if (DisasterDao::deleteDisasterTask(302, &err)) {
      qDebug() << "任务分配单被删除撤下";
  }
  ```

#### 2.10 根据日期查询灾害

- **接口定义**
  ```cpp
  static bool getDisastersByDate(const QString& date,
                                 QList<DisasterRecord>* records, QString* errorMessage);
  ```

- **输入参数**
| 参数名 | 类型 | 说明 |
|---|---|---|
| date | const QString& | 日期字符串，格式 `"YYYY-MM-DD"`，按 `occurred_at`（发生时间）过滤 |

- **输出参数 / 返回值**
| 参数名 | 类型 | 说明 |
|---|---|---|
| records | QList\<DisasterRecord\>* | 指定日期的灾害记录列表 |
| errorMessage | QString* | 错误信息 |
| (return) | bool | 成功返回 `true`，失败返回 `false` |

- **调用示例**
  ```cpp
  QList<DisasterRecord> dayRecords;
  QString err;
  if (DisasterDao::getDisastersByDate("2025-03-14", &dayRecords, &err)) {
      qDebug() << "当天记录数:" << dayRecords.size();
      for (const auto& r : dayRecords) {
          qDebug() << r.disasterType << r.location << r.occurredAt;
      }
  }
  ```

#### 2.11 按日期范围统计各灾害类型数量

- **接口定义**
  ```cpp
  static bool countDisasterTypesByDateRange(const QString& dateFrom, const QString& dateTo,
                                            QMap<QString, int>* result, QString* errorMessage);
  ```

- **输入参数**
| 参数名 | 类型 | 说明 |
|---|---|---|
| dateFrom | const QString& | 起始日期，格式 `"YYYY-MM-DD"`（含当天 `00:00:00`），按 `occurred_at` 统计 |
| dateTo | const QString& | 截止日期，格式 `"YYYY-MM-DD"`（含当天 `23:59:59`） |

- **输出参数 / 返回值**
| 参数名 | 类型 | 说明 |
|---|---|---|
| result | QMap\<QString, int\>* | 统计结果，key=灾害类型名称，value=发生次数 |
| errorMessage | QString* | 错误信息 |
| (return) | bool | 成功返回 `true`，失败返回 `false` |

- **返回结果示例**
  ```
  { "火灾": 2, "洪涝": 4, "地震": 1 }
  ```

- **调用示例**
  ```cpp
  QMap<QString, int> stats;
  QString err;
  if (DisasterDao::countDisasterTypesByDateRange("2025-01-01", "2025-12-31", &stats, &err)) {
      for (auto it = stats.begin(); it != stats.end(); ++it) {
          qDebug() << it.key() << ":" << it.value() << "次";
      }
  }
  ```

---

### 3. OCR 文字识别 (OcrHelper)

#### 3.1 识别图片文字

- **接口定义**
  ```cpp
  QString recognizeText(const cv::Mat &image);
  ```

- **输入参数**
| 参数名 | 类型 | 说明 |
|---|---|---|
| image | const cv::Mat& | OpenCV 的 cv::Mat 格式图片矩阵 |

- **输出参数 / 返回值**
| 参数名 | 类型 | 说明 |
|---|---|---|
| (return) | QString | 图片中的识别文本 |

- **调用示例**
  ```cpp
  cv::Mat img = cv::imread("test.jpg");
  OcrHelper ocr;
  QString text = ocr.recognizeText(img);
  ```

#### 3.2 提取新消息

- **接口定义**
  ```cpp
  QString extractNewMessage(const QString &oldText, const QString &newText);
  ```

- **输入参数**
| 参数名 | 类型 | 说明 |
|---|---|---|
| oldText | const QString& | 旧识别文本 |
| newText | const QString& | 当前帧获取识别文本 |

- **输出参数 / 返回值**
| 参数名 | 类型 | 说明 |
|---|---|---|
| (return) | QString | 从末尾提取出来的新内容段落 |

- **调用示例**
  ```cpp
  OcrHelper ocr;
  QString added = ocr.extractNewMessage("发生火警", "发生火警，注意撤离"); // 返回 "，注意撤离"
  ```

---

## SQLite 工具使用

**常用命令：**
```powershell
.\sqlite-tools-win-x64-3510200\sqlite3.exe monitor.db
.tables
.schema users
.headers on
.mode column
SELECT * FROM users;
.quit
```
