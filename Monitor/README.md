# Monitor

## SQLite 数据库

程序启动时会在 `QCoreApplication::applicationDirPath()`（也就是 `Monitor.exe` 所在目录）创建/打开 `monitor.db`，并自动建表/做必要的字段迁移。

### 表结构（当前版本）

#### units（企业单位）

| 字段 | 类型 | 约束/默认 | 注释 |
|---|---|---|---|
| id | INTEGER | PK, AUTOINCREMENT | 单位ID |
| name | TEXT | NOT NULL, UNIQUE | 企业单位名称（全局唯一，不允许重名） |
| created_at | TEXT | NOT NULL, DEFAULT `datetime('now','localtime')` | 创建时间（本地时间字符串） |

索引：

- `idx_units_name` on `units(name)`（由登录/注册模块的 schema 保障创建）

#### users（用户）

| 字段 | 类型 | 约束/默认 | 注释 |
|---|---|---|---|
| id | INTEGER | PK, AUTOINCREMENT | 用户ID |
| username | TEXT | NOT NULL, UNIQUE | 登录用户名（全局唯一） |
| password | TEXT | NOT NULL | 密码存储串（加盐 + SHA256：`saltHex$digestHex`） |
| phone | TEXT |  | 手机号（可空） |
| role | TEXT | NOT NULL | 角色（如：现场调度员/指挥调度员/现场处置员） |
| unit_id | INTEGER | FK -> `units.id`, ON DELETE SET NULL | 所属单位ID（可空） |
| dispatcher_user_id | INTEGER | FK -> `users.id`, ON DELETE SET NULL | 绑定的调度员用户ID（处置员使用；可空） |
| unit | TEXT |  | 旧字段保留（兼容/迁移；新逻辑优先使用 `unit_id` 对应单位名） |
| created_at | TEXT | NOT NULL, DEFAULT `datetime('now','localtime')` | 创建时间（本地时间字符串） |

关系约束：

- 一个调度员/指挥调度员（`users.id`）可关联多个现场处置员（`users.dispatcher_user_id = 调度员id`）
- 一个单位（`units.id`）可关联多个用户（`users.unit_id = 单位id`）

索引：

- `idx_users_unit_id` on `users(unit_id)`
- `idx_users_dispatcher_user_id` on `users(dispatcher_user_id)`

#### disasters（灾害监测）

| 字段 | 类型 | 约束/默认 | 注释 |
|---|---|---|---|
| id | INTEGER | PK, AUTOINCREMENT | 灾害记录ID |
| disaster_type | TEXT | NOT NULL | 灾害类型（如：火灾/洪水/地质灾害等） |
| location | TEXT |  | 发生地点（可空） |
| occurred_at | TEXT |  | 发生时间（可空；时间字符串） |
| content | TEXT |  | 灾害内容/描述（可空） |
| system_alarm_at | TEXT |  | 系统告警时间（可空；时间字符串） |
| severity | INTEGER | NOT NULL, DEFAULT 0 | 严重级别（数值越大越严重，默认 0） |
| created_at | TEXT | NOT NULL, DEFAULT `datetime('now','localtime')` | 创建时间（本地时间字符串） |

索引：

- `idx_disasters_severity` on `disasters(severity)`

#### disaster_tasks（灾害任务指派）

| 字段 | 类型 | 约束/默认 | 注释 |
|---|---|---|---|
| id | INTEGER | PK, AUTOINCREMENT | 任务ID |
| disaster_id | INTEGER | NOT NULL, FK -> `disasters.id`, ON DELETE CASCADE | 关联的灾害ID |
| handler_user_id | INTEGER | NOT NULL, FK -> `users.id`, ON DELETE CASCADE | 处置员用户ID |
| progress | INTEGER | NOT NULL, DEFAULT 0 | 进度（默认 0；具体含义由业务定义） |
| assigned_at | TEXT | NOT NULL, DEFAULT `datetime('now','localtime')` | 指派时间（本地时间字符串） |

索引：

- `idx_disaster_tasks_disaster_id` on `disaster_tasks(disaster_id)`
- `idx_disaster_tasks_handler_user_id` on `disaster_tasks(handler_user_id)`

#### announcements（系统公告）

| 字段 | 类型 | 约束/默认 | 注释 |
|---|---|---|---|
| id | INTEGER | PK, AUTOINCREMENT | 公告ID |
| title | TEXT | NOT NULL | 公告标题 |
| content | TEXT | NOT NULL | 公告正文 |
| publish_at | TEXT | NOT NULL, DEFAULT `datetime('now','localtime')` | 发布时间（本地时间字符串） |
| expire_at | TEXT |  | 过期时间（可空；时间字符串） |

#### chat_messages（聊天消息）

| 字段 | 类型 | 约束/默认 | 注释 |
|---|---|---|---|
| id | INTEGER | PK, AUTOINCREMENT | 消息ID |
| sent_at | TEXT | NOT NULL, DEFAULT `datetime('now','localtime')` | 发送时间（本地时间字符串） |
| sender_user_id | INTEGER | NOT NULL, FK -> `users.id`, ON DELETE CASCADE | 发送者用户ID |
| receiver_user_id | INTEGER | NOT NULL, FK -> `users.id`, ON DELETE CASCADE | 接收者用户ID |
| content | TEXT | NOT NULL | 消息内容 |

索引：

- `idx_chat_messages_sender` on `chat_messages(sender_user_id)`
- `idx_chat_messages_receiver` on `chat_messages(receiver_user_id)`

#### alert_settings（警报设置）

| 字段 | 类型 | 约束/默认 | 注释 |
|---|---|---|---|
| id | INTEGER | PK, AUTOINCREMENT | 设置ID |
| user_id | INTEGER | NOT NULL, UNIQUE, FK -> `users.id`, ON DELETE CASCADE | 用户ID（每个用户一条设置） |
| alarm_response_count | INTEGER | NOT NULL, DEFAULT 0 | 警报响应次数/计数（默认 0） |
| watch_content | TEXT |  | 关注内容（可空） |
| alarm_audio | TEXT |  | 警报音频（可空；通常为文件路径/标识） |
| updated_at | TEXT | NOT NULL, DEFAULT `datetime('now','localtime')` | 更新时间（本地时间字符串） |

## sqlite3 查看数据库

如果你已把 sqlite3 工具解压到本项目目录下的 `sqlite-tools-win-x64-3510200`，可以这样查看（示例路径仅供参考，替换为你的 `monitor.db` 实际路径）：

```powershell
.\sqlite-tools-win-x64-3510200\sqlite3.exe "path\to\monitor.db" ".tables"
.\sqlite-tools-win-x64-3510200\sqlite3.exe "path\to\monitor.db" ".schema users"
```

交互模式：

```powershell
.\sqlite-tools-win-x64-3510200\sqlite3.exe "path\to\monitor.db"
.tables
.schema
.headers on
.mode column
SELECT * FROM users;
.quit
```

## 用户注册/登录与绑定接口

接口文件：

- [userauth.h](file:///d:/project/qt/qt/Monitor/userauth.h)
- [userauth.cpp](file:///d:/project/qt/qt/Monitor/userauth.cpp)

### 1）单位接口（创建/搜索）

创建单位（单位名全局唯一）：

```cpp
#include "userauth.h"

qint64 unitId = 0;
QString err;
bool ok = UserAuth::createUnit("某某单位", &unitId, &err);
```

搜索单位（注册调度员前用于选择单位）：

```cpp
QList<UnitInfo> units;
QString err;
bool ok = UserAuth::searchUnits("某某", 20, &units, &err);
```

### 2）调度员注册（现场调度员/指挥调度员）

调度员必须先选单位（用 `unitId` 注册）：

```cpp
qint64 dispatcherId = 0;
QString err;
bool ok = UserAuth::registerDispatcherWithUnitId(
    "dispatcher1",
    "123456",
    "13800000000",
    "现场调度员",
    unitId,
    &dispatcherId,
    &err
);
```

### 3）处置员注册（绑定调度员，一对多）

先在某单位下搜索调度员：

```cpp
QList<DispatcherInfo> dispatchers;
QString err;
bool ok = UserAuth::searchDispatchersByUnit(unitId, "disp", 20, &dispatchers, &err);
```

选择一个 `dispatcherUserId`，注册现场处置员并绑定：a

```cpp
qint64 handlerId = 0;
QString err;
bool ok = UserAuth::registerHandlerWithDispatcherId(
    "handler1",
    "123456",
    "13900000000",
    dispatchers[0].id,
    &handlerId,
    &err
);
```

### 4）登录

```cpp
AuthUser user;
QString err;
bool ok = UserAuth::login("dispatcher1", "123456", &user, &err);
```

`AuthUser.dispatcherUserId`：

- 对“现场处置员”：为其绑定的调度员 `id`
- 对“调度员/指挥调度员”：通常为 0（未绑定）
