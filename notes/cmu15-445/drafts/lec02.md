# ClickHouse 高级 DQL 用法

工作中用 Clickhouse DQL 比较多，正好总结了一些较为高级方便的用法。这里我列举出来，用 Gemini 2.5 Pro 润了色，文章如下。

<!-- @import "[TOC]" {cmd="toc" depthFrom=2 depthTo=6 orderedList=false} -->

<!-- code_chunk_output -->

- [公用表表达式 (Common Table Expressions, CTE)](#公用表表达式-common-table-expressions-cte)
  - [作为常量或参数源](#作为常量或参数源)
  - [简化子查询与逻辑分层](#简化子查询与逻辑分层)
- [`GROUP BY` 的高级聚合](#group-by-的高级聚合)
  - [获取“最值”所在行的其他列：`argMax` / `argMin`](#获取最值所在行的其他列argmax--argmin)
  - [条件聚合](#条件聚合)
  - [多维聚合：`GROUPING SETS`、`ROLLUP`、`CUBE`](#多维聚合grouping-sets-rollup-cube)
- [窗口函数 (Window Functions)](#窗口函数-window-functions)
  - [排序与排名：`ROW_NUMBER`, `RANK`, `DENSE_RANK`](#排序与排名row_number-rank-dense_rank)
  - [累计与移动聚合](#累计与移动聚合)
- [总结](#总结)

<!-- /code_chunk_output -->

### 公用表表达式 (Common Table Expressions, CTE)

公用表表达式，通常由 `WITH` 关键字开启，它允许你定义一个或多个临时的、在单个查询中可以被重复引用的命名结果集。使用 CTE 的核心优势在于能够将复杂的查询逻辑拆分成独立的、可读性强的模块，从而极大地增强了 SQL 的结构化和可维护性。

生产代码或者复杂的 OLAP 数据库查询中使用 CTE ，可以极大地增强代码阅读体验。

#### 作为常量或参数源

一个非常实用的技巧是将 CTE 用作“常量”或“参数”的存放地。当你的查询中有多处需要引用同一个固定的值或一个动态计算出的值（例如，某个特定的日期范围）时，将其定义在一个 CTE 中可以避免硬编码和重复计算。

假设我们需要分析某个活动期间（比如从当前日期回溯 180 天）的用户行为。

```sql
WITH date_range AS (
    SELECT
        today() AS cur_date,
        today() - INTERVAL 180 DAY AS sta_date
)
SELECT
    user_id,
    COUNT(DISTINCT product_id) AS distinct_products_viewed
FROM user_actions
WHERE
    event_date <= (SELECT cur_date FROM date_range)
    AND event_date >= (SELECT sta_date FROM date_range)
GROUP BY user_id
ORDER BY distinct_products_viewed DESC
LIMIT 10;
```

在这个例子中，`date_range` 这个 CTE 就好像一个只包含一行的配置表，存储了查询的起止日期。后续的 `WHERE` 子句通过子查询从 `date_range` 中获取这些值。这样做的好处是，如果需要调整日期范围，只需修改 CTE 内部的逻辑一处即可。

#### 简化子查询与逻辑分层

CTE 也是组织复杂业务逻辑的利器。你可以将一个大的查询任务分解为多个步骤，每个步骤对应一个 CTE，后续的 CTE 或主查询可以引用在它之前已经定义好的 CTE。

例如，我们想找出“高价值用户”（在特定时间内购买总额超过 1000 元的用户），然后查询这些高价值用户最近都浏览了哪些商品。

```sql
WITH high_value_users AS (
    SELECT user_id
    FROM user_actions
    WHERE action_type = 'purchase' AND event_date >= '2025-01-01'
    GROUP BY user_id
    HAVING SUM(price * quantity) > 1000
)
SELECT
    user_name,
    product_id
FROM user_actions
WHERE
    user_id IN (SELECT user_id FROM high_value_users)
    AND action_type = 'view'
    AND event_date >= '2025-07-01'
LIMIT 100;
```

这里，`high_value_users` 首先筛选出符合条件的用户群体。主查询的 `WHERE` 子句通过 `IN (SELECT ...)` 的方式直接复用了这个结果集，使得整个查询的意图一目了然：先定义“谁是高价值用户”，再查询“这些用户的行为”。

另外，片段中提到的 `SELECT a, 100 / (SELECT x FROM table2 ...)` 是一种标量子查询（scalar subquery），即子查询返回的是单个值（单行单列）。虽然它在形式上不属于 CTE，但其理念相通：将一个计算结果作为值嵌入到查询中。在 ClickHouse 中，如果这个标量值需要被多处使用，将其放入 CTE 往往是更优的选择。

### `GROUP BY` 的高级聚合

`GROUP BY` 是数据聚合的核心，但它的能力远不止于 `SUM`、`COUNT`、`AVG`。ClickHouse 提供了一系列强大的聚合函数，能够在一个聚合查询中实现更复杂的分析逻辑。

#### 获取“最值”所在行的其他列：`argMax` / `argMin`

这是一个非常有用的 ClickHouse 扩展函数。通常的 `MAX()` 或 `MIN()` 只能返回分组中的最大/最小值本身，但如果我们想知道“取得最大/最小值的 **那一行** 的 **其他列** 的值”时，`argMax` 和 `argMin` 就派上了用场。

其语法为 `argMax(column_to_return, column_with_max_value)`。

例如，要找出每个用户 **最后一次** 购买的商品是什么？

```sql
SELECT
    user_id,
    argMax(product_id, event_time) AS last_purchased_product
FROM user_actions
WHERE action_type = 'purchase'
GROUP BY user_id;
```

查询结果大概是这样：

```txt
┌─user_id─┬─last_purchased_product─┐
│ 101     │ P0034                  │
│ 102     │ P0088                  │
│ 103     │ P0152                  │
└─────────┴────────────────────────┘
```

这里的 `argMax(product_id, event_time)` 意味着：根据 `event_time` 找到最大值（即最晚的时间），并返回那一行的 `product_id` 值。

#### 条件聚合

有时我们需要在一个聚合查询中，根据不同的条件进行计算。这可以通过将 `CASE WHEN` 或 `if` 函数与聚合函数结合来实现。

例如，我们想统计每个商品的“加购率”，即“加购”次数占“总浏览”次数的比例。

```sql
SELECT
    product_id,
    COUNT(CASE WHEN action_type = 'add_to_cart' THEN 1 ELSE NULL END) AS add_to_cart_count,
    COUNT(CASE WHEN action_type = 'view' THEN 1 ELSE NULL END) AS view_count,
    add_to_cart_count / view_count AS add_to_cart_ratio
FROM user_actions
GROUP BY product_id
HAVING view_count > 100 -- 只看有一定浏览量的商品
ORDER BY add_to_cart_ratio DESC;
```

`COUNT` 函数会忽略 `NULL` 值，因此 `COUNT(CASE WHEN ... ELSE NULL END)` 就巧妙地实现了对符合特定条件的行进行计数。片段中的 `AVG(CASE WHEN ...)` 也是同理，通过给不同条件赋予不同分值来计算平均分。

ClickHouse 的 `if` 函数 `if(condition, then, else)` 在这种场景下也同样适用，尤其对于二元条件判断更为简洁。例如 `argMax(if(date = '2025-07-30', gmv, 0), time)` 表示，只在日期为 '2025-07-30' 时才考虑 `gmv` 的值，否则将其视为 0 来寻找时间最晚的那一行。

#### 多维聚合：`GROUPING SETS`、`ROLLUP`、`CUBE`

在报表制作中，我们经常需要对不同维度组合进行聚合，例如，既要看“每个分类每天”的销售额，又要看“每个分类”的总销售额。传统方法是写两个 SQL 再用 `UNION ALL` 合并，但这既繁琐又低效。

`GROUPING SETS` 提供了一种优雅的解决方案，它允许在一次查询中定义多个分组集。

假设我们要计算每个商品分类（`product_category`）每天的销售额，并同时计算每个分类的总销售额。

```sql
SELECT
    product_category,
    if(grouping(event_date), 'Total', toString(event_date)) AS date,
    SUM(price * quantity) AS total_sales
FROM user_actions
WHERE action_type = 'purchase'
GROUP BY GROUPING SETS (
    (product_category, event_date), -- 分组集1: 按分类和日期
    (product_category)              -- 分组集2: 只按分类
)
ORDER BY product_category, date;
```

`grouping(column)` 是一个辅助函数，当该列在当前分组集中被聚合掉了（即作为更高维度的汇总行），它返回 `1`，否则返回 `0`。我们用它来判断当前行是否是“总计”行，并相应地显示 'Total'。

查询结果可能如下：

```txt
┌─product_category─┬─date─────────┬─total_sales─┐
│ Electronics      │ 2025-07-29   │ 12500.00    │
│ Electronics      │ 2025-07-30   │ 18300.50    │
│ Electronics      │ Total        │ 30800.50    │
│ Books            │ 2025-07-29   │ 850.75      │
│ Books            │ 2025-07-30   │ 1200.00     │
│ Books            │ Total        │ 2050.75     │
└──────────────────┴──────────────┴─────────────┘
```

* **`WITH ROLLUP`** ：是 `GROUPING SETS` 的一种简化语法，用于生成具有层级关系的聚合结果。`GROUP BY a, b WITH ROLLUP` 等价于 `GROUP BY GROUPING SETS ((a, b), (a), ())`。
* **`WITH CUBE`** ：则会生成所有维度组合的聚合结果。`GROUP BY a, b WITH CUBE` 等价于 `GROUP BY GROUPING SETS ((a, b), (a), (b), ())`。

### 窗口函数 (Window Functions)

窗口函数对与当前行相关的多行（即“窗口”）进行计算，但与聚合函数不同，它在输出结果中 **保留了原始行** ，并为每一行都附加一个计算结果。窗口函数通过 `OVER()` 子句来定义计算窗口，是处理排序、排名和移动聚合等问题的强大工具。

#### 排序与排名：`ROW_NUMBER`, `RANK`, `DENSE_RANK`

这三个函数用于在窗口内为每一行分配一个排名。

* `ROW_NUMBER()`: 无论值是否相同，都分配连续的排名（1, 2, 3, 4）。
* `RANK()`: 值相同时排名相同，但后续排名会跳过相应的位置（1, 2, 2, 4）。
* `DENSE_RANK()`: 值相同时排名相同，且后续排名是连续的（1, 2, 2, 3）。

例如，我们想对每个商品分类下的商品，按其销量进行排名。

```sql
SELECT
    product_id,
    product_category,
    total_sales,
    DENSE_RANK() OVER (PARTITION BY product_category ORDER BY total_sales DESC) AS sales_rank_in_category
FROM (
    SELECT
        product_id,
        product_category,
        SUM(price * quantity) AS total_sales
    FROM user_actions
    WHERE action_type = 'purchase'
    GROUP BY product_id, product_category
)
ORDER BY product_category, sales_rank_in_category;
```

* `PARTITION BY product_category`: 定义了窗口的分区，排名将在每个 `product_category` 内独立进行。
* `ORDER BY total_sales DESC`: 定义了窗口内的排序规则，决定了排名的依据。

查询结果看起来像这样：

```txt
┌─product_id─┬─product_category─┬─total_sales─┬─sales_rank_in_category─┐
│ P0101      │ Electronics      │ 25000.00    │ 1                      │
│ P0105      │ Electronics      │ 22000.00    │ 2                      │
│ P0102      │ Electronics      │ 22000.00    │ 2                      │
│ P0201      │ Books            │ 5000.00     │ 1                      │
│ P0203      │ Books            │ 4500.00     │ 2                      │
└────────────┴──────────────────┴─────────────┴────────────────────────┘
```

#### 累计与移动聚合

窗口函数也非常适合计算累计值和移动平均值。

**累计计算从窗口开始到当前行的聚合值。**

例如，计算每日的累计销售额。

```sql
SELECT
    event_date,
    daily_sales,
    SUM(daily_sales) OVER (ORDER BY event_date) AS cumulative_sales
FROM (
    SELECT
        event_date,
        SUM(price * quantity) AS daily_sales
    FROM user_actions
    WHERE action_type = 'purchase'
    GROUP BY event_date
)
ORDER BY event_date;
```

`OVER (ORDER BY event_date)` 定义了一个按时间排序的窗口，`SUM` 会累加从第一天到当前日期的所有 `daily_sales`。

**移动计算（滑动窗口）：通过 `ROWS BETWEEN ...` 子句定义一个固定大小的、随当前行移动的窗口。**

例如，计算 7 日移动平均活跃用户数（7-day moving average DAU）。

```sql
SELECT
    event_date,
    dau,
    AVG(dau) OVER (ORDER BY event_date ROWS BETWEEN 6 PRECEDING AND CURRENT ROW) AS dau_7_day_ma
FROM (
    SELECT
        event_date,
        COUNT(DISTINCT user_id) AS dau
    FROM user_actions
    GROUP BY event_date
)
ORDER BY event_date;
```

`ROWS BETWEEN 6 PRECEDING AND CURRENT ROW` 精确地定义了窗口范围为：包括当前行在内的往前 6 行，总共 7 行（7 天）的数据。

查询结果：

```txt
┌─event_date─┬───dau─┬─────────dau_7_day_ma─┐
│ 2025-07-01 │  1000 │ 1000.0               │
│ 2025-07-02 │  1100 │ 1050.0               │
│ ...        │   ... │ ...                  │
│ 2025-07-08 │  1250 │ 1150.71...           │
│ 2025-07-09 │  1300 │ 1180.14...           │
└────────────┴───────┴──────────────────────┘
```

### 总结

* **CTE** 帮助我们编写模块化、易于理解和维护的复杂查询。
* **高级聚合函数** 如 `argMax` 和条件聚合，让我们能用更少的代码、在一次查询中完成更丰富的聚合分析，这在 ClickHouse 中尤为重要，因为减少查询次数通常意味着性能的提升。
* **窗口函数** 则为处理需要行与行之间关联计算的场景（如排名、趋势分析、周期对比）提供了标准且强大的武器。
