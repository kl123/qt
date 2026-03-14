
## 接口一：根据日期查询灾害列表（`getDisastersByDate`）

### 功能描述

输入一个具体日期，返回该日期当天**发生**（`occurred_at`）的所有灾害记录列表。
适用于「今日灾害速览」「历史某天复盘」等场景。

### 接口定义

```cpp
static bool getDisastersByDate(const QString& date,
                               QList<DisasterRecord>* records,
                               QString* errorMessage);
```

### 输入参数

| 参数名   | 类型               | 必填 | 说明                                                         |
| -------- | ------------------ | ---- | ------------------------------------------------------------ |
| `date` | `const QString&` | 是   | 日期字符串，格式严格为 `"YYYY-MM-DD"`，如 `"2025-03-14"` |

### 输出参数 / 返回值

| 参数名           | 类型                       | 说明                                                    |
| ---------------- | -------------------------- | ------------------------------------------------------- |
| `records`      | `QList<DisasterRecord>*` | 查询到的灾害记录列表（按 `occurred_at` 升序排列）     |
| `errorMessage` | `QString*`               | 失败时填入具体错误描述                                  |
| *(return)*     | `bool`                   | 查询成功返回 `true`；参数非法或 DB 异常返回 `false` |

> **注意**：查询结果为空（当天无灾害）时函数仍返回 `true`，`records` 为空列表。

### 调用示例

```cpp
QList<DisasterRecord> records;
QString err;
if (DisasterDao::getDisastersByDate("2025-03-14", &records, &err)) {
    for (const auto& r : records)
        qDebug() << r.disasterType << r.location << r.occurredAt;
    // 输出示例：火灾  某某路1号  2025-03-14 09:30:00
} else {
    qWarning() << err;
}
```

---

## 接口二：按日期范围统计各灾害类型数量（`countDisasterTypesByDateRange`）

### 功能描述

输入起始日期和截止日期，统计该时间段内（按 `occurred_at`）各灾害类型的发生次数，返回 `QMap<QString, int>` 形式的统计结果。
适用于「月报/季报数据汇总」「灾害类型趋势分析」「大屏统计展示」等场景。

### 接口定义

```cpp
static bool countDisasterTypesByDateRange(const QString& dateFrom,
                                          const QString& dateTo,
                                          QMap<QString, int>* result,
                                          QString* errorMessage);
```

### 输入参数

| 参数名       | 类型               | 必填 | 说明                                                           |
| ------------ | ------------------ | ---- | -------------------------------------------------------------- |
| `dateFrom` | `const QString&` | 是   | 起始日期，格式 `"YYYY-MM-DD"`，查询时自动补全为 `00:00:00` |
| `dateTo`   | `const QString&` | 是   | 截止日期，格式 `"YYYY-MM-DD"`，查询时自动补全为 `23:59:59` |

### 输出参数 / 返回值

| 参数名           | 类型                    | 说明                                                    |
| ---------------- | ----------------------- | ------------------------------------------------------- |
| `result`       | `QMap<QString, int>*` | 统计结果，key=灾害类型，value=发生次数，按数量降序排列  |
| `errorMessage` | `QString*`            | 失败时填入具体错误描述                                  |
| *(return)*     | `bool`                | 统计成功返回 `true`；参数非法或 DB 异常返回 `false` |

### 返回结果示例

```
{
  "洪涝": 4,
  "火灾": 2,
  "地震": 1
}
```

### 调用示例

```cpp
QMap<QString, int> stats;
QString err;
if (DisasterDao::countDisasterTypesByDateRange("2025-01-01", "2025-12-31", &stats, &err)) {
    for (auto it = stats.cbegin(); it != stats.cend(); ++it)
        qDebug() << it.key() << ":" << it.value() << "次";
    // 输出示例：洪涝:4次  火灾:2次  地震:1次
} else {
    qWarning() << err;
}
```
