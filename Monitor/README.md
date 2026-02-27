# Monitor 项目文档

本项目是一个基于 Qt C++ 的监控系统，集成了 SQLite 数据库管理、用户权限认证、灾害信息监测与调度以及 OCR 文字识别功能。

## 目录

- [数据库结构](#数据库结构)
- [核心接口文档 (C++ API)](#核心接口文档-c-api)
  - [1. 用户认证与管理 (UserAuth)](#1-用户认证与管理-userauth)
  - [2. 灾害数据管理 (DisasterDao)](#2-灾害数据管理-disasterdao)
  - [3. OCR 文字识别 (OcrHelper)](#3-ocr-文字识别-ocrhelper)
- [SQLite 工具使用](#sqlite-工具使用)

---

## 数据库结构

程序启动时会在 `QCoreApplication::applicationDirPath()`（即 `Monitor.exe` 所在目录）创建或打开 `monitor.db`。

### 1. units（企业单位）
| 字段 | 类型 | 约束/默认 | 说明 |
|---|---|---|---|
| id | INTEGER | PK, AUTOINCREMENT | 单位ID |
| name | TEXT | NOT NULL, UNIQUE | 单位名称（全局唯一） |
| created_at | TEXT | DEFAULT datetime('now') | 创建时间 |

### 2. users（用户）
| 字段 | 类型 | 约束/默认 | 说明 |
|---|---|---|---|
| id | INTEGER | PK, AUTOINCREMENT | 用户ID |
| username | TEXT | NOT NULL, UNIQUE | 用户名 |
| password | TEXT | NOT NULL | 密文密码 |
| phone | TEXT | | 手机号 |
| role | TEXT | NOT NULL | 角色 |
| unit_id | INTEGER | FK -> units.id | 所属单位ID |
| dispatcher_user_id | INTEGER | FK -> users.id | 绑定的调度员ID |
| created_at | TEXT | DEFAULT datetime('now') | 创建时间 |

### 3. disasters（灾害监测）
| 字段 | 类型 | 约束/默认 | 说明 |
|---|---|---|---|
| id | INTEGER | PK, AUTOINCREMENT | 记录ID |
| disaster_type | TEXT | NOT NULL | 灾害类型 |
| location | TEXT | | 地点 |
| occurred_at | TEXT | | 发生时间 |
| content | TEXT | | 内容描述 |
| severity | INTEGER | DEFAULT 0 | 严重等级 |
| created_at | TEXT | DEFAULT datetime('now') | 创建时间 |

### 4. disaster_tasks（任务指派）
| 字段 | 类型 | 约束/默认 | 说明 |
|---|---|---|---|
| id | INTEGER | PK, AUTOINCREMENT | 任务ID |
| disaster_id | INTEGER | FK -> disasters.id | 灾害ID |
| handler_user_id | INTEGER | FK -> users.id | 处置员ID |
| progress | INTEGER | DEFAULT 0 | 进度 |

### 5. chat_messages（聊天消息）
| 字段 | 类型 | 约束/默认 | 说明 |
|---|---|---|---|
| id | INTEGER | PK, AUTOINCREMENT | 消息ID |
| sender_user_id | INTEGER | FK -> users.id | 发送者 |
| receiver_user_id | INTEGER | FK -> users.id | 接收者 |
| content | TEXT | NOT NULL | 内容 |

---

## 核心接口文档 (C++ API)

### 1. 用户认证与管理 (UserAuth)

头文件：`userauth.h`

#### 1.0 相关数据结构 (Structs)

以下是 `UserAuth` 模块中涉及的主要数据结构，作为参数或返回值使用。

##### **1.0.1 UnitInfo (单位信息)**
| 字段名 | 类型 | 说明 |
|---|---|---|
| `id` | `qint64` | 单位 ID |
| `name` | `QString` | 单位名称 |

##### **1.0.2 DispatcherInfo (调度员信息)**
| 字段名 | 类型 | 说明 |
|---|---|---|
| `id` | `qint64` | 调度员用户 ID |
| `username` | `QString` | 用户名 |
| `phone` | `QString` | 手机号 |
| `unit` | `QString` | 所属单位名称 |

##### **1.0.3 AuthUser (认证用户信息)**
| 字段名 | 类型 | 说明 |
|---|---|---|
| `id` | `qint64` | 用户 ID |
| `username` | `QString` | 用户名 |
| `phone` | `QString` | 手机号 |
| `role` | `QString` | 角色（如"现场调度员"） |
| `unit` | `QString` | 所属单位名称 |
| `dispatcherUserId` | `qint64` | 绑定的调度员ID（若角色为处置员） |

#### 1.1 创建单位
创建新的企业单位，单位名称必须唯一。

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
| unitId | qint64* | 成功时返回新创建的单位 ID |
| errorMessage | QString* | 失败时的错误信息 |
| (return) | bool | 成功返回 `true`，失败返回 `false` |

- **调用示例**
  ```cpp
  qint64 unitId;
  QString err;
  // 注意：传入 &unitId 以获取返回的新 ID
  if (UserAuth::createUnit("某某消防大队", &unitId, &err)) {
      qDebug() << "创建成功，新单位ID:" << unitId;
  } else {
      qDebug() << "创建失败:" << err;
  }
  ```

#### 1.2 搜索单位
根据关键词模糊搜索单位。

- **接口定义**
  ```cpp
  static bool searchUnits(const QString& keyword, int limit, QList<UnitInfo>* units, QString* errorMessage);
  ```

- **输入参数**

| 参数名 | 类型 | 说明 |
|---|---|---|
| keyword | const QString& | 搜索关键词 |
| limit | int | 返回的最大记录数 |

- **输出参数 / 返回值**

| 参数名 | 类型 | 说明 |
|---|---|---|
| units | QList<UnitInfo>* | 返回的单位列表（详见 [UnitInfo](#101-unitinfo-单位信息)） |
| errorMessage | QString* | 错误信息 |
| (return) | bool | 成功返回 `true`，失败返回 `false` |

- **调用示例**
  ```cpp
  QList<UnitInfo> units;
  QString err;
  // 注意：查询结果会填充到 units 列表中
  if (UserAuth::searchUnits("消防", 10, &units, &err)) {
      qDebug() << "查询成功，共找到" << units.size() << "个单位";
      for (const auto& unit : units) {
          // 获取具体字段
          qDebug() << "单位名称:" << unit.name << "ID:" << unit.id;
      }
  } else {
      qDebug() << "查询失败:" << err;
  }
  ```

#### 1.3 注册调度员 (带单位)
注册角色为调度员的用户，必须绑定单位。

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
| userId | qint64* | 返回新用户 ID |
| errorMessage | QString* | 错误信息 |
| (return) | bool | 成功返回 `true`，失败返回 `false` |

- **调用示例**
  ```cpp
  qint64 userId;
  QString err;
  // 注册成功后，userId 将包含新生成的 ID
  if (UserAuth::registerDispatcherWithUnitId("admin", "123456", "13800000000", 
                                             "指挥调度员", 1, &userId, &err)) {
      qDebug() << "调度员注册成功，用户ID:" << userId;
  } else {
      qDebug() << "注册失败:" << err;
  }
  ```

#### 1.4 搜索调度员
根据单位 ID 搜索该单位下的调度员（用于处置员绑定）。

- **接口定义**
  ```cpp
  static bool searchDispatchersByUnit(qint64 unitId, const QString& keyword, int limit, 
                                    QList<DispatcherInfo>* dispatchers, QString* errorMessage);
  ```

- **输入参数**

| 参数名 | 类型 | 说明 |
|---|---|---|
| unitId | qint64 | 单位 ID |
| keyword | const QString& | 搜索关键词 |
| limit | int | 返回的最大记录数 |

- **输出参数 / 返回值**

| 参数名 | 类型 | 说明 |
|---|---|---|
| dispatchers | QList<DispatcherInfo>* | 返回的调度员列表（详见 [DispatcherInfo](#102-dispatcherinfo-调度员信息)） |
| errorMessage | QString* | 错误信息 |
| (return) | bool | 成功返回 `true`，失败返回 `false` |

- **调用示例**
  ```cpp
  QList<DispatcherInfo> dispatchers;
  QString err;
  // 数据返回到 dispatchers
  if (UserAuth::searchDispatchersByUnit(1, "张", 10, &dispatchers, &err)) {
      for (const auto& d : dispatchers) {
          qDebug() << "姓名:" << d.username << "电话:" << d.phone;
      }
  }
  ```

#### 1.5 注册处置员 (绑定调度员)
注册现场处置员，并绑定到指定的调度员。

- **接口定义**
  ```cpp
  static bool registerHandlerWithDispatcherId(
      const QString& username, const QString& password, const QString& phone,
      qint64 dispatcherUserId,
      qint64* userId, QString* errorMessage);
  ```

- **输入参数**

| 参数名 | 类型 | 说明 |
|---|---|---|
| username | const QString& | 用户名 |
| password | const QString& | 密码 |
| phone | const QString& | 手机号 |
| dispatcherUserId | qint64 | 归属的调度员用户 ID |

- **输出参数 / 返回值**

| 参数名 | 类型 | 说明 |
|---|---|---|
| userId | qint64* | 返回新用户 ID |
| errorMessage | QString* | 错误信息 |
| (return) | bool | 成功返回 `true`，失败返回 `false` |

- **调用示例**
  ```cpp
  qint64 userId;
  QString err;
  if (UserAuth::registerHandlerWithDispatcherId("handler01", "123456", "13900000000", 
                                                101, &userId, &err)) {
      qDebug() << "处置员注册成功，ID:" << userId;
  } else {
      qDebug() << "注册失败:" << err;
  }
  ```

#### 1.6 用户登录
验证用户名密码并返回用户信息。

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
| user | AuthUser* | 用户信息（详见 [AuthUser](#103-authuser-认证用户信息)） |
| errorMessage | QString* | 错误信息 |
| (return) | bool | 成功返回 `true`，失败返回 `false` |

- **调用示例**
  ```cpp
  AuthUser user;
  QString err;
  // 注意：传入 &user，登录成功后 user 包含完整信息
  if (UserAuth::login("zhangsan", "123456", &user, &err)) {
      qDebug() << "登录成功：" << user.username;
      qDebug() << "角色：" << user.role;
      qDebug() << "所属单位：" << user.unit;
  } else {
      qDebug() << "登录失败：" << err;
  }
  ```

#### 1.7 获取调度员下属处置员
查询某位调度员（通过其用户ID）所关联的所有现场处置员信息。

- **接口定义**
  ```cpp
  static bool getHandlersByDispatcherId(qint64 dispatcherId, 
                                        QList<DispatcherInfo>* handlers, 
                                        QString* errorMessage);
  ```

- **输入参数**

| 参数名 | 类型 | 说明 |
|---|---|---|
| dispatcherId | qint64 | 指挥调度员的用户ID |

- **输出参数 / 返回值**

| 参数名 | 类型 | 说明 |
|---|---|---|
| handlers | QList<DispatcherInfo>* | 返回的处置员列表，复用 `DispatcherInfo` 结构体 |
| errorMessage | QString* | 错误信息 |
| (return) | bool | 成功返回 `true`，失败返回 `false` |

---

### 2. 灾害数据管理 (DisasterDao)

头文件：`disasterdao.h`

#### 2.0 相关数据结构 (Structs)

以下是 `DisasterDao` 模块中涉及的主要数据结构，作为参数或返回值使用。

##### **2.0.1 DisasterRecord (灾害记录)**
用于创建和查询返回的完整记录。

| 字段名 | 类型 | 说明 |
|---|---|---|
| `id` | `qint64` | 记录 ID (创建时由数据库生成，查询时返回) |
| `disasterType` | `QString` | 灾害类型（如“火灾”） |
| `location` | `QString` | 发生地点 |
| `occurredAt` | `QString` | 发生时间 |
| `content` | `QString` | 灾害描述 |
| `systemAlarmAt` | `QString` | 系统告警时间 |
| `severity` | `int` | 严重等级（0为最低） |
| `createdAt` | `QString` | 创建时间 |

##### **2.0.2 DisasterPatch (灾害更新补丁)**
用于局部更新灾害记录。每个字段都有一个对应的 `has...` 标志位，只有标志位为 `true` 时才会更新对应字段。

| 字段名 | 类型 | 说明 |
|---|---|---|
| `id` | `qint64` | 要更新的记录 ID (必须设置) |
| `hasDisasterType` / `disasterType` | `bool` / `QString` | 是否更新类型 / 新类型 |
| `hasLocation` / `location` | `bool` / `QString` | 是否更新地点 / 新地点 |
| `hasOccurredAt` / `occurredAt` | `bool` / `QString` | 是否更新发生时间 / 新时间 |
| `hasContent` / `content` | `bool` / `QString` | 是否更新描述 / 新描述 |
| `hasSystemAlarmAt` / `systemAlarmAt` | `bool` / `QString` | 是否更新告警时间 / 新时间 |
| `hasSeverity` / `severity` | `bool` / `int` | 是否更新等级 / 新等级 |

##### **2.0.3 DisasterQuery (灾害查询条件)**
用于多条件组合查询。

| 字段名 | 类型 | 说明 |
|---|---|---|
| `keyword` | `QString` | 综合模糊搜索关键词 |
| `disasterTypeLike` | `QString` | 类型模糊匹配 |
| `locationLike` | `QString` | 地点模糊匹配 |
| `contentLike` | `QString` | 内容模糊匹配 |
| `hasSeverityMin` / `severityMin` | `bool` / `int` | 是否启用最小等级 / 最小等级值 |
| `hasSeverityMax` / `severityMax` | `bool` / `int` | 是否启用最大等级 / 最大等级值 |
| `occurredAtFrom` | `QString` | 发生时间起始（YYYY-MM-DD HH:MM:SS） |
| `occurredAtTo` | `QString` | 发生时间结束 |
| `createdAtFrom` | `QString` | 创建时间起始 |
| `createdAtTo` | `QString` | 创建时间结束 |
| `orderBy` | `enum OrderBy` | 排序方式 (CreatedAtDesc, CreatedAtAsc, SeverityDesc 等) |
| `limit` | `int` | 返回记录数量限制 (默认50) |
| `offset` | `int` | 分页偏移量 (默认0) |

##### **2.0.4 DisasterTaskRecord (灾害任务记录)**
用于灾害任务的指派和管理，关联了处置员的直观数据。

| 字段名 | 类型 | 说明 |
|---|---|---|
| `id` | `qint64` | 指派记录ID |
| `disasterId` | `qint64` | 灾害本身ID |
| `handlerUserId` | `qint64` | 现场处置员ID |
| `progress` | `int` | 进度 (0-100) |
| `assignedAt` | `QString` | 指派时间 |
| `handlerName` | `QString` | (关联数据) 处置员姓名 |
| `handlerPhone` | `QString` | (关联数据) 处置员电话 |

#### 2.1 创建灾害记录
- **接口定义**
  ```cpp
  static bool createDisaster(const DisasterRecord& record, qint64* id, QString* errorMessage);
  ```

- **输入参数**

| 参数名 | 类型 | 说明 |
|---|---|---|
| record | const DisasterRecord& | 灾害记录结构体（详见 [DisasterRecord](#201-disasterrecord-灾害记录)） |

- **输出参数 / 返回值**

| 参数名 | 类型 | 说明 |
|---|---|---|
| id | qint64* | 返回新记录 ID |
| errorMessage | QString* | 错误信息 |
| (return) | bool | 成功返回 `true`，失败返回 `false` |

- **调用示例**
  ```cpp
  DisasterRecord rec;
  rec.disasterType = "火灾";
  rec.location = "科技园A栋";
  rec.severity = 3;
  
  qint64 id;
  QString err;
  // 成功后 id 会被赋值
  if (DisasterDao::createDisaster(rec, &id, &err)) {
      qDebug() << "灾害记录创建成功，ID:" << id;
  } else {
      qDebug() << "创建失败:" << err;
  }
  ```

#### 2.2 查询灾害 (条件查询)
支持多条件组合查询。

- **接口定义**
  ```cpp
  static bool queryDisasters(const DisasterQuery& query, QList<DisasterRecord>* records, QString* errorMessage);
  ```

- **输入参数**

| 参数名 | 类型 | 说明 |
|---|---|---|
| query | const DisasterQuery& | 查询条件结构体（详见 [DisasterQuery](#203-disasterquery-灾害查询条件)） |

- **输出参数 / 返回值**

| 参数名 | 类型 | 说明 |
|---|---|---|
| records | QList<DisasterRecord>* | 返回的灾害记录列表 |
| errorMessage | QString* | 错误信息 |
| (return) | bool | 成功返回 `true`，失败返回 `false` |

- **调用示例**
  ```cpp
  DisasterQuery q;
  q.severityMin = 2;
  q.orderBy = DisasterQuery::OrderBy::SeverityDesc;
  
  QList<DisasterRecord> records;
  QString err;
  // 注意：传入 &records 接收查询结果
  if (DisasterDao::queryDisasters(q, &records, &err)) {
      qDebug() << "查询成功，找到" << records.size() << "条记录";
      for (const auto& record : records) {
          qDebug() << "ID:" << record.id 
                   << "类型:" << record.disasterType 
                   << "严重等级:" << record.severity;
      }
  } else {
      qDebug() << "查询失败:" << err;
  }
  ```

#### 2.3 更新灾害信息
使用 Patch 模式局部更新字段。

- **接口定义**
  ```cpp
  static bool updateDisaster(const DisasterPatch& patch, QString* errorMessage);
  ```

- **输入参数**

| 参数名 | 类型 | 说明 |
|---|---|---|
| patch | const DisasterPatch& | 补丁结构体（详见 [DisasterPatch](#202-disasterpatch-灾害更新补丁)） |

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
  patch.severity = 4; // 仅更新等级
  
  if (DisasterDao::updateDisaster(patch, &err)) {
      qDebug() << "更新成功";
  } else {
      qDebug() << "更新失败:" << err;
  }
  ```

#### 2.4 删除灾害
- **接口定义**
  ```cpp
  static bool deleteDisaster(qint64 id, QString* errorMessage);
  ```

- **输入参数**

| 参数名 | 类型 | 说明 |
|---|---|---|
| id | qint64 | 待删除的灾害记录 ID |

- **输出参数 / 返回值**

| 参数名 | 类型 | 说明 |
|---|---|---|
| errorMessage | QString* | 错误信息 |
| (return) | bool | 成功返回 `true`，失败返回 `false` |

#### 2.5 获取未指派灾害
获取特定调度员负责的，且尚未被下发包含在 `disaster_tasks` 中的剩余灾害警情。

- **接口定义**
  ```cpp
  static bool getUnassignedDisastersByDispatcher(
      qint64 dispatcherId, qint64 unitId, int limit, int offset,
      QList<DisasterRecord>* records, QString* errorMessage);
  ```

#### 2.6 分配灾害任务
将某个灾害警情指派给一至多名现场处置员进行处理，自动开启事务绑定。

- **接口定义**
  ```cpp
  static bool assignDisasterTasks(qint64 disasterId, const QList<qint64>& handlerIds, QString* errorMessage);
  ```

| 参数名 | 类型 | 说明 |
|---|---|---|
| disasterId | qint64 | 需要指派的灾害ID |
| handlerIds | QList<qint64> | 参与处理的多名处置员ID列表 |

#### 2.7 获取灾害任务列表
根据特定的灾害ID，返回包含全部接单人信息以及他们各自处理进度的综合任务记录列表。

- **接口定义**
  ```cpp
  static bool getTasksForDisaster(qint64 disasterId, QList<DisasterTaskRecord>* tasks, QString* errorMessage);
  ```

#### 2.8 更新灾害任务进度
更改某个指定任务单（`disaster_tasks`）进度。

- **接口定义**
  ```cpp
  static bool updateDisasterTaskProgress(qint64 taskId, int progress, QString* errorMessage);
  ```

#### 2.9 删除灾害任务
撤回（删除）特定分派给某人的灾害任务记录。

- **接口定义**
  ```cpp
  static bool deleteDisasterTask(qint64 taskId, QString* errorMessage);
  ```

---

### 3. OCR 文字识别 (OcrHelper)

头文件：`ocrhelper.h`

#### 3.1 识别图片文字
识别 OpenCV 图像中的所有文字。

- **接口定义**
  ```cpp
  QString recognizeText(const cv::Mat &image);
  ```

- **输入参数**

| 参数名 | 类型 | 说明 |
|---|---|---|
| image | const cv::Mat& | OpenCV 格式的图像矩阵 |

- **输出参数 / 返回值**

| 参数名 | 类型 | 说明 |
|---|---|---|
| (return) | QString | 识别出的完整文本 |

- **调用示例**
  ```cpp
  cv::Mat img = cv::imread("test.jpg");
  OcrHelper ocr;
  QString text = ocr.recognizeText(img);
  qDebug() << "识别结果：" << text;
  ```

#### 3.2 提取新消息
对比新旧文本，提取新增的部分（用于监控屏幕日志更新）。

- **接口定义**
  ```cpp
  QString extractNewMessage(const QString &oldText, const QString &newText);
  ```

- **输入参数**

| 参数名 | 类型 | 说明 |
|---|---|---|
| oldText | const QString& | 上一次识别的文本 |
| newText | const QString& | 当前识别的文本 |

- **输出参数 / 返回值**

| 参数名 | 类型 | 说明 |
|---|---|---|
| (return) | QString | 差异部分（新消息） |

---

## SQLite 工具使用

如果需要手动查看数据库内容，可使用项目目录下的 `sqlite-tools-win-x64-3510200/sqlite3.exe`。

**常用命令：**
```powershell
# 打开数据库
.\sqlite-tools-win-x64-3510200\sqlite3.exe monitor.db

# 查看所有表
.tables

# 查看表结构
.schema users

# 查询数据
.headers on
.mode column
SELECT * FROM users;

# 退出
.quit
```
