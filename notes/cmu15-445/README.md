# CMU 15-445/645 Database Systems

<!-- @import "[TOC]" {cmd="toc" depthFrom=3 depthTo=6 orderedList=false} -->

<!-- code_chunk_output -->

- [课程资料](#课程资料)
- [课程索引](#课程索引)
  - [Course Introduction and the Relational Model](#course-introduction-and-the-relational-model)
  - [Advanced SQL](#advanced-sql)
  - [Database Storage I](#database-storage-i)

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

#### Database Storage I

- [./fall2019/notes/03-storage1.pdf](./fall2019/notes/03-storage1.pdf)
- [./fall2019/slides/03-storage1.pdf](./fall2019/slides/03-storage1.pdf)

AI 播客
- https://notebooklm.google.com/notebook/efa457e1-04d9-48da-b726-eeb347f59449?artifactId=fc551a9c-c75f-4e0d-804a-a734f4964a28
- https://mp.weixin.qq.com/s/Y-pgw58c2i55wW60xs6vzQ

我听完播客记下的要点，让 NotebookLM 帮我重新组织语言，纠正其中错误/不清楚的地方，重新结合课堂内容整理一份更加完整正确的笔记出来：
你好！很高兴为你重新组织和澄清播客笔记。根据你提供的要点和课堂内容，以下是一份更完整、准确的笔记：
* 数据库为何不使用 `mmap` 实现自己的“页”管理？数据库系统（DBMS）选择自行管理内存与磁盘之间的数据移动，而非依赖操作系统提供的 `mmap`（内存映射文件）机制，原因如下：
  * **控制数据写入时机和一致性**
    * 操作系统对数据库的上层语义一无所知，它只看到对页的读写操作，不了解事务、日志或并发控制等高级概念。
    * DBMS 需要精确控制数据写入磁盘的顺序，例如，**必须先将日志写入磁盘以确保数据一致性和崩溃恢复的正确性** 。如果使用 `mmap`，操作系统可能会在不正确的时机将脏页（dirty pages）刷新到磁盘，导致数据损坏或不一致问题。
  * **缺乏对上下文的了解与优化能力**
    * 操作系统是通用目的的，它不知道数据库的具体工作负载模式或查询类型。
    * DBMS 能够根据查询需求和数据访问模式实施 **更优化的缓存管理策略** （如更好的替换策略）、 **预取（prefetching）** 和 **线程/进程调度** 。
    * `mmap` 导致的 **缺页中断（page fault）会阻塞整个进程** 。DBMS 不希望在这种情况下被阻塞，而是希望能够继续处理其他查询，以隐藏磁盘访问的延迟。
  * **实际案例证明 `mmap` 的局限性**
    * 即使是像 MongoDB 这样著名的 JSON 数据库，其早期的默认存储引擎使用了 `mmap`，但发现其成为了 **严重的性能瓶颈** 。他们后来投入大量资源放弃了 `mmap`，转而使用非 `mmap` 的存储引擎（WiredTiger）。
    * 像 MySQL、Oracle、DB2、SQL Server 等主流数据库系统 **都没有使用 `mmap`** 作为其核心存储机制，这进一步印证了 `mmap` 在 DBMS 中的不适用性。
* 硬件原子写入与数据库页大小
  * **硬件页（Hardware Page）** ：存储设备（如 SSD 或 HDD）能够保证 **原子写入（atomic write）的最小单位** 通常是 4KB。这意味着，在写入 4KB 数据时，要么全部写入成功，要么全部失败，不会出现部分写入的情况。
  * **数据库页（Database Page）** ：DBMS 使用的数据库页大小通常在 512B 到 16KB 之间，不同系统会有所差异。
  * **“撕裂写入”（Torn Write）问题** ：如果数据库页的大小（例如 8KB 或 16KB） **大于硬件原子写入单位（4KB）** ，那么在写入过程中如果系统崩溃，就可能发生“撕裂写入”——只有部分数据被写入磁盘，导致页面损坏。
  * **保证原子性的额外机制** ：为了应对这个问题，DBMS 需要 **额外的机制来保证数据落盘的原子性** 。你提到的 **预写日志（WAL）** 是一个非常好的例子，它通过先将修改操作记录到持久化的日志中，来保证即使发生崩溃也能恢复数据到一致状态。此外，页面头中的 **校验和（Checksum）** 也可以用于检测数据是否被正确写入。
* 槽页（Slotted Pages）作为页面内部结构
  * **最常见的页面布局方案** ： **槽页（Slotted Pages）** 是目前大多数面向磁盘的数据库系统最常用的页面内部组织方式。
  * **结构与工作原理**
    * 每个页面包含一个 **页头（Header）** ，存储基本元数据（如校验和、页面大小、事务可见性等）。
    * 页面通常有两个数据区域：一个从开头向后增长的 **槽数组（Slot Array）** ，和一个从末尾向前增长的 **元组实际数据（Tuple Data）** 区域。
    * **槽数组** 存储了每个元组在其页面内部的起始偏移量（offset），形成一个间接层。
  * **删除和管理元组的便利性**
    * 当你删除一个元组时，该元组在页面中的物理空间可能会被标记为可用，或者在后台进行 **页面整理/碎片整理（compaction/vacuum）** 。
    * **关键优势在于：物理偏移量可以改变，但是元组的槽位号（slot number）保持不变** 。这意味着，其他部分（例如索引）通过 **记录 ID（Record ID）** 或 **元组 ID（Tuple ID）** 来引用元组时，这个 ID 通常由 **页面 ID (Page ID)** 和 **槽位号（Slot Number）** 组成。由于槽位号不变， **避免了因为页面内部数据移动而需要更新索引或其他元数据结构的开销** 。
* 元组的结构（Tuple Layout）
  * **字节序列表示** ：在数据库系统中，一个元组本质上就是 **一个字节序列** 。
  * **Schema 保证列解析** ：DBMS 的任务是根据表的 **模式（Schema）** 来解释这些字节，将其解析为具体的属性类型和值。Schema 定义了每个列的数据类型和顺序，让 DBMS 知道如何读取对应的字节数据。
  * **元组头（Tuple Header）** ：每个元组通常都有一个 **元组头** ，包含关于该元组的元数据，例如事务可见性信息（用于并发控制）和 NULL 值位图。通常， **元组本身不需要存储完整的模式信息** ，这些信息在更高层次的目录或元数据页中。
  * **属性存储顺序** ：属性通常按照在 `CREATE TABLE` 语句中定义的顺序进行存储。
  * **大字段的特殊处理**
    * 通常， **单个元组的大小不能超过一个页面** 。
    * 对于 **特别大的字段（LOBs - Large Objects）** ，例如存储视频文件，DBMS 通常不会将其直接存储在元组所在的页面中。而是 **单独存储在其他页面中** ，并在原始元组中保留指向这些数据的 **指针** 或元数据。这样做可以避免元组过大，同时保持常用数据在单个页面内可访问。
* 不同表数据在同一页面中的存储（物理去范式化）
  * **一般原则** ：通常情况下，DBMS **不会将来自不同表的数据混合存储在同一个页面中** 。例如，如果创建一个表，该表的所有元组将存储在专门为该表分配的页面中。
  * **特殊优化：物理去范式化（Physical Denormalization）/预连接（Pre-joining）** 
    * 这是一个重要的例外情况。当 **逻辑上相关的、但属于不同表的数据频繁一起被访问（尤其是通过连接操作）时** ，DBMS 可能会进行 **物理去范式化** 的优化。
    * 这意味着，相关联的元组（例如，来自父表和子表的数据）可以 **物理上存储在同一个页面内，甚至嵌入在同一个元组中** 。
    * **优点** ：这种方式可以 **减少 I/O 操作** ，因为读取一个元组时，其相关数据也一并被加载，从而降低了查询（特别是连接查询）的成本。
    * **缺点** ：可能会 **增加更新的开销** ，因为一个更新可能需要修改更大的数据块。
    * **历史与现状** ：这个概念并非新鲜事物，早在 1970 年代 IBM 的 System R 数据库就尝试过。现代的许多 NoSQL 数据库（如 JSON 文档数据库）也通过允许在文档中嵌套相关数据来实现了类似的概念，尽管它们不一定称之为“物理去范式化”。
