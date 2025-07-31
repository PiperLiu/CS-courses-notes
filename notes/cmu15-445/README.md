# CMU 15-445/645 Database Systems

<!-- @import "[TOC]" {cmd="toc" depthFrom=3 depthTo=6 orderedList=false} -->

<!-- code_chunk_output -->

- [课程资料](#课程资料)
- [课程索引](#课程索引)
  - [Course Introduction and the Relational Model](#course-introduction-and-the-relational-model)
  - [Advanced SQL](#advanced-sql)

<!-- /code_chunk_output -->

### 课程资料

- https://15445.courses.cs.cmu.edu/fall2019/
- https://www.youtube.com/playlist?list=PLSE8ODhjZXjbohkNBWQs_otTrBTrjyohi

教材：

```latex
@book{silberschatz2019database,
  title     = {Database System Concepts},
  author    = {Silberschatz, Avi and Korth, Henry F. and Sudarshan, S.},
  edition   = {7th},
  publisher = {McGraw‑Hill},
  year      = {2019},
  month     = feb,
  isbn      = {9780078022159},
  url       = {https://db-book.com/},
  note      = {“Face The Real World of Database Systems Fully Equipped”}
}
```

### 课程索引

#### Course Introduction and the Relational Model

- [./fall2019/notes/01-introduction.pdf](./fall2019/notes/01-introduction.pdf)
- [./fall2019/slides/01-introduction.pdf](./fall2019/slides/01-introduction.pdf)

AI 播客
- https://notebooklm.google.com/notebook/6729a450-a4e7-4993-ba12-4b6f77f17115/audio
- https://mp.weixin.qq.com/s/GD3FYGljFWP7-TOQCsvkLA

我提取了三个要点，让 AI 帮我丰富了：
* **关系模型 SQL 是只需要提出结果 非过程化查询**
  * 在关系模型中，SQL（结构化查询语言）是一种 **非过程化（或声明式）语言** 。这意味着只需 **指定想要什么数据** ，而 **无需说明如何获取这些数据** 。
  * 其核心优势在于，它将 **数据检索的执行策略完全交给数据库管理系统（DBMS）** 来决定和优化，让开发者可以专注于业务逻辑。
* **逻辑层和物理层分离 DBMS 在其中做映射和转换，加索引是物理层**
  * 早期的数据库系统面临 **逻辑层（数据概念）和物理层（数据存储）紧密耦合** 的挑战。这意味着应用程序必须了解数据在磁盘上或内存中的具体存储方式，任何存储结构的变化都可能导致应用代码需要重写。Ted Codd 提出的关系模型解决了这一难题，它在应用程序和实际数据存储之间引入了**清晰的逻辑与物理层分离** 。
  * 应用程序仅通过高层逻辑结构（如表和关系）与数据交互，而 **DBMS则负责底层数据如何在物理介质上进行存储、布局、映射和转换的全部细节** 。因此，像 **添加或修改索引（属于物理层实现）** 这样的优化操作， **无需更改应用程序代码** ，大大增强了系统的灵活性和可维护性。
* **关系代数 基本基本动作 选择投影连接；注意关系代数顺序比较重要 比如先 where 再 join 可能更高效 SQL 只注重结果，寻找顺序这需要优化器来做**
  * **关系代数** 是用于检索和操作关系中元组的一组 **基本操作** ，它构成了数据库查询执行的基石。这些基本操作包括 **选择（过滤元组）、投影（选择属性）、并集、交集、差集、乘积和连接** 。关系代数属于 **过程性语言** ，它定义了查询的 **高级步骤和执行顺序** 。
  * 您指出的“先where再join可能更高效”是一个非常重要的观察点： **关系代数操作的顺序对查询性能有着显著影响** 。例如，在执行大表连接操作之前先通过“选择”过滤掉大量不相关的数据，可以极大减少后续连接操作的数据量，从而提升效率。
  * 与此不同， **SQL作为非过程性语言，只关注您期望的最终结果** 。DBMS内部的 **查询优化器** 则负责解析SQL查询，并 **智能地选择最有效率的关系代数操作顺序和执行计划** ，这一过程对用户是 **透明** 的，无需手动干预。

#### Advanced SQL

- [./fall2019/notes/02-advancedsql.pdf](./fall2019/notes/02-advancedsql.pdf)
- [./fall2019/slides/02-advancedsql.pdf](./fall2019/slides/02-advancedsql.pdf)

AI 播客
- https://notebooklm.google.com/notebook/7b5a6556-0b21-420e-9ce1-a3e2a7d60e4b?artifactId=d1677e00-7d4f-4ea3-9757-efdc06b365cf
- https://notebooklm.google.com/notebook/7b5a6556-0b21-420e-9ce1-a3e2a7d60e4b?artifactId=e677abcf-8411-4360-b8a1-caf688dc3380
- https://mp.weixin.qq.com/s/ZjsctKC-lue_VL0qIL1u_g

笔记：工作中用 Clickhouse DQL 比较多，正好总结了一些较为高级方便的用法。这里我列举出来，用 Gemini 2.5 Pro 润了色 [./drafts/lec02.md](./drafts/lec02.md) 。
- [公用表表达式 (Common Table Expressions, CTE)](./drafts/lec02.md#公用表表达式-common-table-expressions-cte)
  - [作为常量或参数源](./drafts/lec02.md#作为常量或参数源)
  - [简化子查询与逻辑分层](./drafts/lec02.md#简化子查询与逻辑分层)
- [`GROUP BY` 的高级聚合](./drafts/lec02.md#group-by-的高级聚合)
  - [获取“最值”所在行的其他列：`argMax` / `argMin`](./drafts/lec02.md#获取最值所在行的其他列argmax--argmin)
  - [条件聚合](./drafts/lec02.md#条件聚合)
  - [多维聚合：`GROUPING SETS`、`ROLLUP`、`CUBE`](./drafts/lec02.md#多维聚合grouping-sets-rollup-cube)
- [窗口函数 (Window Functions)](./drafts/lec02.md#窗口函数-window-functions)
  - [排序与排名：`ROW_NUMBER`, `RANK`, `DENSE_RANK`](./drafts/lec02.md#排序与排名row_number-rank-dense_rank)
  - [累计与移动聚合](./drafts/lec02.md#累计与移动聚合)
- [总结](./drafts/lec02.md#总结)
