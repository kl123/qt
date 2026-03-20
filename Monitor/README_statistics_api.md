# 灾害预警系统 - 数据大屏统计接口文档 (C++ API)

以下接口为专门面向前端大屏展示（如灾害态势、实时监测等模块）新增的数据聚合接口，底层统一采用 `app_sqlite` 连接池调用 SQLite。

## 1. 获取灾害易发生时间段
此接口用于分析所有历史灾害记录的发生时间，通过分组聚合，返回最易发生灾害的时段区间。

- **接口定义**
  ```cpp
  static bool getDisasterPronePeriod(QString *period, QString *errorMessage);
  ```
- **返回参数**：
  - `period`: `QString*` 格式化好的时间段字符串（例如 `"20-21"`）。若无数据则返回 `"暂无数据"`。
  - `errorMessage`: `QString*` 发生错误时的排错详情。
  - `return bool`: 成功返回 `true`。

## 2. 获取灾害趋势统计（当日/本月及环比）
综合展示灾害基数及增量变化情况。用于前端展示“当日发生数”、“本月累计数”以及它们的环比发生率（增长率）。

- **接口定义**
  ```cpp
  struct DisasterTrendStats {
    int todayCount = 0;
    double todayYoY = 0.0; // 今日环比(%)，例如 40.5 表示 40.5%
    int monthCount = 0;
    double monthYoY = 0.0; // 本月环比(%)
  };
  
  static bool getDisasterTrends(DisasterTrendStats *stats, QString *errorMessage);
  ```
- **返回参数**
  - `stats`: `DisasterTrendStats*` 包含统计信息的结构体实例。
  - `errorMessage`: `QString*` 错误提示。
  - `return bool`: 成功返回 `true`。

## 3. 获取最高频与最罕见灾害类型
对数据库中的灾害类型进行自动聚合汇总，识别出当前极端的分布情况。

- **接口定义**
  ```cpp
  struct DisasterFrequencyStats {
    QString mostFrequentType;  // 最高频灾害名称
    int mostFrequentCount = 0; // 最高频数量
    QString rarestType;        // 最罕见灾害名称
    int rarestCount = 0;       // 最罕见数量
  };
  
  static bool getDisasterFrequencyStats(DisasterFrequencyStats *stats, QString *errorMessage);
  ```
- **返回参数**
  - `stats`: `DisasterFrequencyStats*` 包含最高频项与罕见项信息的结构体数据。
  - `errorMessage`: `QString*` 错误提示。
  - `return bool`: 成功返回 `true`。

## 4. 获取实时监测任务统计
针对处置任务系统周期的生命态势监测，统计系统当前的处置负荷和当天工作成效。

- **接口定义**
  ```cpp
  struct DisasterRealtimeStats {
    int unassignedCount = 0;    // 未分配的灾害数
    int resolvedTodayCount = 0; // 今日已解决数（进度满且在今天收到的单子）
    int processingCount = 0;    // 正在处理中（已指派但未完结）
  };
  
  static bool getRealtimeMonitoringStats(DisasterRealtimeStats *stats, QString *errorMessage);
  ```
- **返回参数**
  - `stats`: `DisasterRealtimeStats*` 实时监测统计结果。
  - `errorMessage`: `QString*` 错误提示。
  - `return bool`: 成功返回 `true`。
