# CMU 15-445/645 Database Systems

<!-- @import "[TOC]" {cmd="toc" depthFrom=3 depthTo=6 orderedList=false} -->

<!-- code_chunk_output -->

- [课程资料](#课程资料)
- [课程索引](#课程索引)
  - [Course Introduction and the Relational Model](#course-introduction-and-the-relational-model)
  - [Advanced SQL](#advanced-sql)
  - [Database Storage I](#database-storage-i)
  - [Database Storage II](#database-storage-ii)
  - [Buffer Pools](#buffer-pools)
  - [Hash Tables](#hash-tables)
  - [Trees Indexes I](#trees-indexes-i)
  - [Trees Indexes II](#trees-indexes-ii)
  - [Index Concurrency Control](#index-concurrency-control)
  - [Sorting + Aggregations](#sorting--aggregations)
  - [Joins Algorithms](#joins-algorithms)
  - [Query Execution I](#query-execution-i)
  - [Query Execution II](#query-execution-ii)
  - [Query Planning & Optimization I](#query-planning--optimization-i)

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

Readings: Chapters 1-2, 6

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

Readings: Chapters 3-5

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

Readings: Chapter 10.1-10.2, 10.5-10.6

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

#### Database Storage II

- [./fall2019/notes/04-storage2.pdf](./fall2019/notes/04-storage2.pdf)
- [./fall2019/slides/04-storage2.pdf](./fall2019/slides/04-storage2.pdf)

AI 播客
- https://notebooklm.google.com/notebook/50c474db-14e3-4eb3-b270-b2223f92eb6f?artifactId=2ccce847-ca43-4a73-b8bc-6afa49e1cfc2
- https://mp.weixin.qq.com/s/Ns4tt9_7-tXTseEsXdO4ZA

我听完播客记下的要点，请 NotebookLM 我重新组织语言，纠正其中错误/不清楚的地方，重新结合课堂内容整理一份正确的笔记出来：
* **浮点数（FLOAT/REAL）与定点数（NUMERIC/DECIMAL）的存储**
  * **浮点数** （如 `FLOAT`、`REAL`/`DOUBLE`）通常采用 **IEEE-754 标准** 进行存储，与 C/C++ 等编程语言中的原生浮点数表示方式一致。
  * 这种存储方式的 **优点** 是执行速度快，因为 CPU 拥有直接对这些数字进行操作的指令，效率很高。
  * **缺点** 是它们是 **非精确的** 可变精度数字，存在 **舍入误差** 。这意味着某些十进制数无法在硬件中精确表示，导致计算结果可能与预期不符。例如，0.1 + 0.2 可能不等于精确的 0.3。
  * 因此，浮点数 **不适合** 对精度要求极高的场景，例如 **金融和会计领域** ，因为哪怕是微小的舍入误差也可能导致严重问题。
  * 为了解决这个问题，数据库系统提供了 **定点数** （如 `NUMERIC`、`DECIMAL`），它们提供 **任意精度和标度** 。
  * 定点数通常以精确的、可变长度的二进制形式存储，并附加 **元数据** （如小数点位置、标度信息、符号等），而不是直接作为字符串存储。
  * 虽然定点数能够避免舍入误差，但数据库管理系统（DBMS）必须自己实现处理这些数字的算术操作和聚合功能，这需要大量的额外代码。因此，定点数的计算 **通常比浮点数慢得多** ，因为它需要执行更多的指令而非单一的 CPU 指令。
* **变长数据（如字符串）的存储**
  * 对于变长数据类型（如 `VARCHAR`、`VARBINARY`、`TEXT`、`BLOB`），数据库系统通常会使用一个 **头部（header）来记录其长度** ，然后才是实际的数据字节序列。
  * 这种方式与 C 语言中以空字符（null terminator）结束字符串的方式不同，通过长度前缀，DBMS 可以轻松地确定数据的大小并跳转到下一个值。
* **大值（Large Values）的存储策略**
  * 大多数 DBMS 不允许单个元组的大小超过单个页面的限制。当需要存储比页面更大的值（例如非常长的字符串或二进制大对象 BLOB）时，数据库系统有两种主要的存储思路：
    * **溢出页（Overflow Pages）**
      * 这是最常见的处理方式。当一个属性的值过大时，DBMS 会将其存储在单独的 **溢出页面** 中，而原始元组只包含一个指向该溢出页的 **指针** （通常是页面号和偏移量）。
      * 不同的数据库系统有不同的触发阈值，例如 PostgreSQL 使用 TOAST 机制，当值超过 2KB 时就会使用溢出页；SQL Server 在值超过页面大小时使用；MySQL 在值超过页面一半大小时使用。
      * 溢出页的 **优点** 是它们 **仍然由 DBMS 管理** ，享受与常规数据相同的 **所有保护** ，包括 **持久性（durability）** 、 **事务保护** 以及各种 **优化** （例如压缩，因为溢出页通常是只读或读多写少的）。对于应用程序而言，这种存储方式是 **透明的** ，应用程序通常感知不到数据存储在溢出页上。
    * **外部文件存储（External File Storage）**
      * 某些系统允许将非常大的值存储在 **外部文件** 中（例如，Oracle 的 BFILE 数据类型和 Microsoft 的 FILESTREAM 数据类型）。在这种情况下，元组中存储的不是数据本身，而是一个指向本地磁盘或网络存储上该文件的**文件路径或指针**。
      * 这种方式的 **主要缺点** 是 **DBMS 无法操纵这些外部文件的内容** 。这意味着这些文件 **不享有 DBMS 提供的任何耐久性或事务保护** 。如果文件在数据库外部被修改，数据库读取时会反映出这些修改，因为它不在数据库系统的控制范围之内。
      * 选择这种方式通常是出于经济考虑或处理超大型文件（例如视频文件、图像文件），避免将它们存储在昂贵的高性能数据库存储中，而是将其放置在更廉价的存储解决方案（如 HDFS 或 S3）中。
* **工作负载类型 (Workloads)** 数据库系统通常会处理两种主要的工作负载类型，它们有不同的特性和优化需求：
  * **OLTP (On-Line Transaction Processing) - 在线事务处理**
    * **特点** ：OLTP 涉及的查询通常是 **快速、短时运行的操作** 。它们通常 **只对单个实体进行操作** ，例如添加商品到购物车或更新用户账户信息。这类工作负载的特点是 **写操作多于读操作** ，并且操作具有 **重复性** 。
    * **应用场景** ：OLTP 是人们在构建新应用时（例如网站或手机应用）最常首先考虑的应用类型。
    * **举例** ：亚马逊购物网站的结账或更新账户信息等操作就属于 OLTP，每次操作只影响小部分数据。
  * **OLAP (On-Line Analytical Processing) - 在线分析处理**
    * **特点** ：OLAP 查询通常是 **复杂且长时间运行的** 。它们需要 **读取数据库的很大一部分数据** ，用于 **探索性查询** 和从 OLTP 系统收集的数据中 **推导出新的信息** 。这类工作负载的特点是 **读操作多于写操作** 。
    * **应用场景** ：当您已经从 OLTP 应用中收集了大量数据，现在想要对其进行分析以获取洞察时，就会使用 OLAP。
    * **举例** ：计算某个月份内来自特定地理区域的商品购买量，或者分析维基百科中特定用户群体的登录行为，这些都需要扫描大量数据并进行聚合。
  * **总结** ：OLTP 关注数据的 **写入和少量读取** ，强调 **并发性和快速响应** ；OLAP 关注数据的 **大量读取和分析** ，强调 **吞吐量和复杂计算** 。
* **存储模型 (Storage Models)** 数据库管理系统（DBMS）可以使用不同的方式在页面中存储元组（数据行），以更好地支持特定的工作负载。
  * **N-Ary Storage Model (NSM) / 行式存储 (Row Store)**
    * **定义** ：NSM 是指 DBMS 将单个元组的 **所有属性（列）连续存储在页面中** 。这也是到目前为止我们课程中默认假设的存储方式。
    * **优点**
      * 对于 **插入、更新和删除操作非常快** 。
      * **适合需要获取整个元组的查询** ，例如获取用户的所有账户信息。因为所有数据都在一起，只需要一次读取就能获取整个元组。
    * **缺点**
      * 不适合 **扫描表中大部分数据或仅需要部分属性的查询** 。在这种情况下，即使查询只关心少数几列，也必须将整个数据行（包含不必要的列）从磁盘加载到内存中，这会 **浪费 I/O 并污染缓冲区** 。
    * **适用场景** ： **最适合 OLTP 工作负载** ，因为 OLTP 查询通常只操作单个实体，且涉及大量插入操作。
  * **Decomposition Storage Model (DSM) / 列式存储 (Column Store)**
    * **定义** ：DSM 是指 DBMS 将 **所有元组的单个属性（列）的值连续存储在一个数据块中** 。
    * **优点**
      * **减少了查询执行过程中不必要的 I/O 浪费** ，因为 DBMS 只读取查询所需的那些列的数据。
      * **实现了更好的数据压缩** ，因为同一列的所有值都具有相同的数据类型，并且通常具有相似的模式或范围，这有助于高效压缩。
      * **有利于查询处理和优化** ，例如可以直接在压缩数据上操作。
    * **缺点**
      * 对于 **点查询（只获取一个元组）、插入、更新和删除操作较慢** ，因为需要进行 **元组的拆分和重新组装** （tuple splitting/stitching）。
      * 为了重新组合元组，通常采用 **定长偏移量** （Fixed-length offsets）的方式，即为每一列中的每个值设定固定长度，通过算术计算快速定位；另一种方式是 **嵌入元组 ID** （Embedded Tuple Ids），为每个值存储其对应的元组 ID，但这种方式会增加存储开销。大多数 DBMS 都使用定长偏移量。
    * **适用场景** ： **最适合 OLAP 工作负载** ，因为 OLAP 查询通常是只读的，并且对表的某个子集属性执行大规模扫描。
  * **核心结论** ： **OLTP 适合行式存储** (Row Store)，而 **OLAP 适合列式存储** (Column Store)。在现实世界中，一些系统会尝试混合使用这两种模型，或者通过流式传输数据将 OLTP 和 OLAP 系统分开部署，例如，前端 OLTP 系统（如 MySQL）负责新数据，后端数据仓库（如 Vertica/Clickhouse）负责分析。

#### Buffer Pools

Readings: Chapter 10.5-10.8
Project Released: Buffer Pool Manager

- [./fall2019/notes/05-bufferpool.pdf](./fall2019/notes/05-bufferpool.pdf)
- [./fall2019/slides/05-bufferpool.pdf](./fall2019/slides/05-bufferpool.pdf)

AI 播客
- https://notebooklm.google.com/notebook/2d504cc7-90d5-4d72-99a4-e1544a77d46c?artifactId=1d6c8deb-de50-4b78-81a7-038883d38aa5
- https://mp.weixin.qq.com/s/8Y7eYNDRXlkAcv01jkEteg

笔记： [./drafts/lec05.md](./drafts/lec05.md)
- [缓冲池（Buffer Pool）与帧（Frame）](./drafts/lec05.md#缓冲池buffer-pool与帧frame)
- [缓冲池元数据（Meta-data）](./drafts/lec05.md#缓冲池元数据meta-data)
- [并发控制：锁（Locks）与栓锁（Latches）](./drafts/lec05.md#并发控制锁locks与栓锁latches)
- [缓冲池优化手段](./drafts/lec05.md#缓冲池优化手段)
- [页面替换策略（Replacement Policies）](./drafts/lec05.md#页面替换策略replacement-policies)
- [脏页（Dirty Pages）处理与后台写入](./drafts/lec05.md#脏页dirty-pages处理与后台写入)
- [PostgresQL 与操作系统页面缓存的利用](./drafts/lec05.md#postgresql-与操作系统页面缓存的利用)

#### Hash Tables

Readings: Chapter 11.6-11.7
Homework Released: Indexes

- [./fall2019/notes/06-hashtables.pdf](./fall2019/notes/06-hashtables.pdf)
- [./fall2019/slides/06-hashtables.pdf](./fall2019/slides/06-hashtables.pdf)

AI 播客
- https://notebooklm.google.com/notebook/df5e3f61-7c79-43cf-8e8e-42116c89b7b9?artifactId=33e11224-1827-4d10-bb1e-2ac718234f35
- https://mp.weixin.qq.com/s/GAVrZT8oE3K__AowZxrNhg

让 NotebookLM 帮我把要点总结了一下：
* **速度与碰撞的权衡 (Trade-off between Speed and Collision Rate)**
  * 不使用加密哈希函数，我们只关注 **速度快** 和 **碰撞率低** 的哈希函数。
  * **常数因子很重要 (Constant Factors Matter)** ：在处理大量数据时，即使O(1)操作的常数因子差异也会导致巨大的性能差距和金钱成本。
* 静态哈希方案 (Static Hashing Schemes)：静态哈希方案的哈希表大小是固定的。如果存储空间不足，DBMS必须从头开始重建一个更大的哈希表（通常是原大小的两倍），这会 **非常昂贵** 。
  * **线性探测哈希 (Linear Probe Hashing)**
    * **实现原理/方法** ：哈希函数计算出槽位后，如果该槽位已被占用，则 **线性扫描** 到下一个空闲槽位进行插入。查找时，也从哈希到的槽位开始线性扫描，直到找到目标键或遇到空槽。为了进行验证，每个槽位必须存储原始键。
    * **优点** ： **最基本也通常是最快** 的哈希方案，因为它具有良好的缓存局部性，且分支预测失败较少。
    * **代价** ：删除操作很复杂。如果直接删除，会中断后续键的查找链； **墓碑标记 (Tombstone)** 复杂，在删除的槽位放置一个“墓碑”标记，表示该槽位逻辑上为空但物理上占用，查找时会跳过此标记继续扫描。浪费空间，需要后续垃圾回收。将后续的键向前移动以填充空位，代价是移动后的键可能不再位于其最佳哈希位置的下游，导致查找失败。
  * **罗宾汉哈希 (Robin Hood Hashing)**
    * **实现原理/方法** ：线性探测哈希的一种变体。每个键都会记录它 **距离其理想哈希位置的“跳跃”次数** （即贫富程度）。在插入时，如果新键比当前占据该槽位的键“更贫穷”（即距离其理想位置更远），新键会 **“偷走”** 这个槽位，而被“偷走”的键则会被重新插入到哈希表中。
    * **优点** ：旨在 **平衡整个哈希表中键的距离** ，最小化任何键距离其理想位置的最大距离。
    * **代价** ： **写入/插入更昂贵** ，在实践中，对于内存中的数据结构，通常 **比线性探测哈希慢** 。
  * **布谷鸟哈希 (Cuckoo Hashing)**
    * **实现原理/方法** ：使用 **多个哈希表** （通常是两个）和 **不同的哈希函数种子** 。插入时，尝试在每个哈希表中找到一个空闲槽位。如果所有可能的槽位都被占用，则 **驱逐** 其中一个哈希表中的现有元素，并将其重新哈希以找到新位置，这可能导致连锁驱逐（像布谷鸟占巢）。
    * **优点** ： **查找和删除总是O(1)** ，因为只需检查每个哈希表中的一个特定位置。
    * **代价** ： **插入可能昂贵** ，可能导致“乒乓”效应或连锁驱逐，甚至陷入 **循环** （无限循环）；如果检测到循环或哈希表变得太满，就需要 **重建所有哈希表** ，使用新的哈希函数种子或更大的表。
* 动态哈希方案 (Dynamic Hashing Schemes)，能够在需要时 **按需调整大小** ，而无需重建整个哈希表。
  * **链式哈希 (Chained Hashing / Bucket Hashing)**
    * **实现原理/方法** ：每个主槽位都维护一个 **桶（bucket）的链表** 。所有哈希到同一个槽位的键都放在该链表的桶中。当桶满时，就 **分配并链接一个新的桶** 。
    * **优点** ：实现简单，且通过不断添加新桶可以“无限”增长。易于实现线程安全，只需对槽位或单个页面进行加锁。
    * **代价** ：如果所有键都映射到同一个桶链，哈希表可能会退化为 **O(n)的线性搜索** ，性能显著下降；可能存在空间浪费，尤其是有许多短链时。
  * **可扩展哈希 (Extendible Hashing)**
    * **实现原理/方法** ：链式哈希的改进变体，它会 **分裂桶** 而不是让链无限增长。维护一个 **全局计数器（global depth）** ，指示哈希值需要检查的位数，以确定在槽数组中的位置。
      * 每个桶也有一个 **局部计数器（local depth）** 。
      * 当桶溢出时，会触发分裂。如果局部深度小于全局深度，则重新分配桶中的元素到新的桶中。如果局部深度等于全局深度，则 **全局深度会增加，槽数组的大小会加倍** （这个操作是廉价的，因为只是指针数组），然后重新分配桶中的元素。
    * **优点** ： **数据移动被局部化** 到溢出的桶链，其他桶不受影响。槽数组的加倍操作相对廉价，因为它只涉及指针数组的更新，不涉及数据的物理移动。
    * **代价** ：槽数组加倍时，需要对整个槽数组 **获取全局锁** ，这可能成为并发访问的瓶颈；删除操作比较复杂，可能需要合并桶并逆向分裂过程
  * **线性哈希 (Linear Hashing)**
    * **实现原理/方法** ：维护一个 **分裂指针 (split pointer)** ，它跟踪下一个要分裂的桶，而不管哪个桶实际溢出。使用 **多个哈希函数** （例如 `key % n` 和 `key % 2n`）。
      * 当任何桶溢出时， **分裂指针指向的桶** 会被分裂（即使它不是溢出的那个），将其内容重新分配到新的槽位，并添加一个新的哈希函数（`key % 2n`）。
      * 查询时，首先使用第一个哈希函数。如果映射到的槽位在分裂指针 **之上** （即已被分裂），则需要使用第二个哈希函数来找到实际位置。
    * **优点** ：将 **调整大小的操作局部化** 到分裂指针所指向的桶，避免了对整个哈希表进行全局加锁，从而减少了并发瓶颈。
    * **代价** ：由于分裂的桶不一定是溢出的桶，这可能导致 **临时出现更多的溢出链** ；删除操作很复杂，可能涉及分裂指针的逆向移动和内存回收。

#### Trees Indexes I

Readings: Chapter 11.1-11.4

- [./fall2019/notes/07-trees1.pdf](./fall2019/notes/07-trees1.pdf)
- [./fall2019/slides/07-trees1.pdf](./fall2019/slides/07-trees1.pdf)

AI 播客
- https://notebooklm.google.com/notebook/15f1863b-aaf2-4226-ae42-dad2e69608df?artifactId=5f788af8-e36f-4b8a-9163-0cbb5e2ba791
- https://mp.weixin.qq.com/s/SnSoEZ8HKYGfDnaGoCr0bA

笔记： [./drafts/lec07.md](./drafts/lec07.md)
- [B+ 树的核心特性](./drafts/lec07.md#b-树的核心特性)
- [插入与删除操作](./drafts/lec07.md#插入与删除操作)
- [B+ 树的设计考量](./drafts/lec07.md#b-树的设计考量)
- [B+ 树的优化技术](./drafts/lec07.md#b-树的优化技术)

#### Trees Indexes II

Readings: Chapter 11.1-11.4

- [./fall2019/notes/08-trees2.pdf](./fall2019/notes/08-trees2.pdf)
- [./fall2019/slides/08-trees2.pdf](./fall2019/slides/08-trees2.pdf)

AI 播客
- https://notebooklm.google.com/notebook/fff02ff9-3a21-48a7-8412-f1b80c744174?artifactId=2eae936d-a3be-482a-be52-74a90ea90388
- https://mp.weixin.qq.com/s/zhRuh4rs63XfWgBNjcRTnA

**B+ 树如何处理重复键？** B+ 树有两种主要方法处理重复键，以确保索引的效率和正确性：
1. **追加记录ID (Append Record Id)**
  * 这种方法通过在每个键后面追加对应的元组的 **唯一记录ID** （通常是页面ID和偏移量），使每个键在索引中变得独一无二。
  * **优势** ：在B+树中，即使键被追加了记录ID，数据库管理系统（DBMS）仍然可以进行 **部分键查找** ，仅使用原始属性值进行搜索，然后沿着叶子节点扫描以找到所有匹配项。
  * **缺点** ：这种方法会 **增加索引的大小** ，因为每个键都包含了额外的记录ID信息。
2. **使用溢出叶子节点 (Overflow Leaf Nodes)**
  * 这种方法允许叶子节点“溢出”到额外的 **溢出页面** 或 **溢出节点** 中，这些溢出页面专门存储重复的键。这些溢出页面会垂直地链接到主叶子节点。
  * **优势** ：这种方法 **不会存储任何冗余信息** 来使键唯一。
  * **缺点** ：它增加了索引的 **复杂性** ，因为在扫描叶子节点时需要额外逻辑来跟随溢出页面，尤其是在反向扫描时。

**聚簇索引 (Table Clustering)**
* 表聚簇是指DBMS使用索引来 **强制表本身元组的物理排序顺序** 。
* 在PostgreSQL等系统中，这是一个 **一次性操作** 。这意味着表在首次聚簇后会根据索引排序，但随着后续的修改（插入、更新、删除），元组的物理顺序可能会再次变得无序。
* 然而，在MySQL、SQL Server和Oracle等其他系统中，你可以声明一个表是聚簇表，这样 **无论插入顺序如何，底层物理存储都将保持排序** 。
* **优势** ：对于某些查询，这允许DBMS直接在表数据上执行 **二分查找** ，而无需通过索引本身，从而提高性能。在MySQL中，主键索引的叶子节点实际上就是元组本身，使得沿着叶子节点扫描就等同于对表的顺序扫描。

**字典树 / Radix (Tries / Radix Trees)**
* **字典树 (Trie)** 是一种树形数据结构，它不存储键的完整副本，而是存储键的 **数字或原子子集** （如单个字节或位）。键的值通过从根到叶子的路径隐式表示，并且不需要像B+树那样进行重新平衡操作。
* **基数树 (Radix Tree)** 是字典树的一种 **特化形式** ，它省略了所有只有一个子节点的节点（即进行了 **垂直压缩** ）。基数树有时也被称为 **Patricia树** 。
* **特性**
  * 它们的形状仅取决于 **键空间的分布和长度** ，而不取决于现有键或插入顺序，因此是确定性的。
  * 所有操作的复杂性是 **O(k)** ，其中k是键的长度。这意味着，如果在查找过程中发现前缀不匹配，可以立即停止，而无需遍历到底层。
  * 基数树在 **点查询** 方面通常比B+树更快，但在 **顺序扫描** 方面可能较慢。
* **键的字节序和编码技巧**
  * 并非所有属性类型都能直接分解为基数树所需的 **二进制可比数字** 。
  * 例如，无符号整数可能需要翻转字节序（对于小端系统），带符号整数需要调整二进制补码，浮点数需要先分类再作为无符号整数处理。复合键需要对每个属性分别进行转换。这些都需要特殊的编码技巧来确保按字节比较的正确性。

**倒排索引解决 B+ 树解决不了的问题（文本查询）**
* 传统的B+树和哈希索引擅长处理“点查询”（精确匹配）和“范围查询”，例如查找邮政编码或日期范围内的记录。
* 然而，它们 **不适用于关键词搜索** ，例如在大量文本中查找包含特定词语的文档，或执行`LIKE '%word%'`这样的模式匹配。这是因为B+树需要对整个键进行精确或范围查找，而不能查找键内部分子元素。
* **倒排索引 (Inverted Index)** 专门用于解决这个问题。它存储了 **单词到包含这些单词的记录的映射** 。
* **需要考虑的因素**
  * **存储内容 (What to Store)** ：最简单的形式是存储单词本身并映射到记录ID。但也可以包含 **词频、位置信息以及其他元数据** ，以便支持更复杂的查询。
  * **更新时机 (When to Update)** ：频繁更新倒排索引成本很高。因此，许多DBMS会维护辅助数据结构来 **分批暂存更新** ，然后定期批量更新索引。
* **支持的查询类型** ：倒排索引能够支持 **短语搜索** （查找包含特定顺序词语的记录）、 **邻近搜索** （查找两个词语在指定距离内出现的记录）以及 **通配符搜索** （匹配复杂模式）。
* 许多主流DBMS都原生支持倒排索引，也有专门的全文搜索数据库系统（如Elasticsearch）。

**高级索引技术：部分索引 (Partial Indexes)**
* **部分索引** 是指仅在表的 **子集** 上创建索引。通过在 `CREATE INDEX` 命令中添加 `WHERE` 子句来实现，指定哪些元组应该包含在索引中。
* **优势** ：这种方法可以 **减小索引的大小** ，降低维护成本，并减少不必要数据对缓冲池（Buffer Pool）的污染。
* **常见用例** ：按日期范围分区索引，例如为每个月或每年创建单独的索引。

**避免回表 (Avoiding Table Lookups / Index-Only Scans)**
* “避免回表”是指查询所需的所有数据都可以在索引中直接找到，而无需再去访问实际的表（堆）中的元组。这可以显著减少磁盘I/O和提高查询性能。
* **覆盖索引 (Covering Indexes)** ：如果处理查询所需的所有字段都可以在索引中找到，那么DBMS就不需要检索原始元组。这是DBMS **自动判断** 和利用的特性。
* **索引包含列 (Index Include Columns)** ：这是一种允许在索引中嵌入 **额外列** 的技术，这些额外列虽然不作为搜索键的一部分，但会存储在索引的叶子节点中。这使得即使查询需要这些非搜索键的列，也能实现索引唯一扫描。PostgreSQL 11和SQL Server支持此功能。

**函数表达式索引 (Functional/Expression Indexes)**
* **函数表达式索引** 允许你将 **函数或表达式的输出** 作为键来构建索引，而不是直接使用原始列的值。
* **示例** ：如果经常需要查询某个日期时间字段是星期几，可以创建一个索引在 `EXTRACT(dow FROM login_timestamp)` 的结果上，这样查询时DBMS就可以直接利用这个索引。
* **关键** ：DBMS的查询优化器必须能够识别哪些查询可以使用这种基于表达式的索引。
*  要注意的是，用于创建表达式索引的函数必须是 **不可变** 的（即给定相同的输入，每次调用都会产生相同的输出，不会因外部状态而改变）。

#### Index Concurrency Control

Readings: Chapter 15.10

- [./fall2019/notes/09-indexconcurrency.pdf](./fall2019/notes/09-indexconcurrency.pdf)
- [./fall2019/slides/09-indexconcurrency.pdf](./fall2019/slides/09-indexconcurrency.pdf)

AI 播客
- https://notebooklm.google.com/notebook/002fd889-062d-4a65-998b-dfad555d6b8a?artifactId=d95acd78-e21d-48ab-8f9a-f6677b229cb0
- https://mp.weixin.qq.com/s/R9H_fcTzvP1U5Y61wYRk3g

笔记： [./drafts/lec09.md](./drafts/lec09.md)
- [锁 (Locks) 与锁存 (Latches) 的区别](./drafts/lec09.md#锁-locks-与锁存-latches-的区别)
- [锁存的实现方式](./drafts/lec09.md#锁存的实现方式)
- [哈希表 (Hash Table) 的锁存](./drafts/lec09.md#哈希表-hash-table-的锁存)
- [B+树 (B+Tree) 的锁存](./drafts/lec09.md#b树-btree-的锁存)
  - [锁存爬行 (Latch Crabbing) 的基本思想](./drafts/lec09.md#锁存爬行-latch-crabbing-的基本思想)
  - [根节点争用 (Root Contention) 问题](./drafts/lec09.md#根节点争用-root-contention-问题)
  - [乐观锁存 (Optimistic Latching)](./drafts/lec09.md#乐观锁存-optimistic-latching)
  - [叶节点遍历 (Leaf Node Scans) 与死锁](./drafts/lec09.md#叶节点遍历-leaf-node-scans-与死锁)
  - [延迟父节点更新 (Delayed Parent Updates) / Blink-Tree 优化](./drafts/lec09.md#延迟父节点更新-delayed-parent-updates--blink-tree-优化)
- [总结](./drafts/lec09.md#总结)

#### Sorting + Aggregations

Readings: Chapter 12.4-12.5
Homework Released: Join Algorithms
Project Released: Hash Index

- [./fall2019/notes/10-sorting.pdf](./fall2019/notes/10-sorting.pdf)
- [./fall2019/slides/10-sorting.pdf](./fall2019/slides/10-sorting.pdf)

AI 播客
- https://notebooklm.google.com/notebook/656b25ca-8622-4c7e-967f-be90cf923167?artifactId=344893d0-af96-4b23-9eec-c8c90d5b780b
- https://mp.weixin.qq.com/s/z139nUf5Aq2NwUdRzfxKtA

笔记： [./drafts/lec10.md](./drafts/lec10.md)
- [内存不足时的大数据排序：外部归并排序 (External Merge Sort)](./drafts/lec10.md#内存不足时的大数据排序外部归并排序-external-merge-sort)
- [聚合操作 (Aggregations)](./drafts/lec10.md#聚合操作-aggregations)
  - [基于排序的聚合 (Sorting Aggregation)](./drafts/lec10.md#基于排序的聚合-sorting-aggregation)
  - [基于哈希的聚合 (Hashing Aggregate)](./drafts/lec10.md#基于哈希的聚合-hashing-aggregate)

#### Joins Algorithms

Readings: Chapter 12.4-12.6

- [./fall2019/notes/11-joins.pdf](./fall2019/notes/11-joins.pdf)
- [./fall2019/slides/11-joins.pdf](./fall2019/slides/11-joins.pdf)

AI 播客
- https://notebooklm.google.com/notebook/066a6598-db27-45a4-9065-615e4c3c0790?artifactId=5a12d9ae-b3b9-4e8b-af99-f239e129fdd0
- https://mp.weixin.qq.com/s/rjmQdkDp7JUhCRKfWdDtaQ

笔记： [./drafts/lec11.md](./drafts/lec11.md)
- [连接操作的核心概念](./drafts/lec11.md#连接操作的核心概念)
  - [连接的评估基准：I/O 成本](./drafts/lec11.md#连接的评估基准io-成本)
  - [外层表与内层表的选择：为何“小表驱动大表”？](./drafts/lec11.md#外层表与内层表的选择为何小表驱动大表)
  - [连接的输出方式：物化 vs. 记录 ID](./drafts/lec11.md#连接的输出方式物化-vs-记录-id)
- [三大主流连接算法](./drafts/lec11.md#三大主流连接算法)
  - [嵌套循环连接 (Nested Loop Join)](./drafts/lec11.md#嵌套循环连接-nested-loop-join)
  - [排序合并连接 (Sort-Merge Join)](./drafts/lec11.md#排序合并连接-sort-merge-join)
  - [哈希连接 (Hash Join)](./drafts/lec11.md#哈希连接-hash-join)
- [算法选择与总结](./drafts/lec11.md#算法选择与总结)

#### Query Execution I

Readings: Chapter 12.1-12.3, 12.7

- [./fall2019/notes/12-queryexecution1.pdf](./fall2019/notes/12-queryexecution1.pdf)
- [./fall2019/slides/12-queryexecution1.pdf](./fall2019/slides/12-queryexecution1.pdf)

AI 播客
- https://notebooklm.google.com/notebook/5bce03f8-2b86-49ff-91a0-266074dd95cf?artifactId=e71c6d0d-ba18-4d0b-a71c-11ceeecf4e2d
- https://mp.weixin.qq.com/s/JoGxftYgh_V3nDz3jfoYGg

笔记： [./drafts/lec12.md](./drafts/lec12.md)
- [查询计划处理模型](./drafts/lec12.md#查询计划处理模型)
  - [迭代模型（Iterator Model）](./drafts/lec12.md#迭代模型iterator-model)
  - [物化模型（Materialization Model）](./drafts/lec12.md#物化模型materialization-model)
  - [向量化模型（Vectorized / Batch Model）](./drafts/lec12.md#向量化模型vectorized--batch-model)
- [数据访问方法（Access Methods）](./drafts/lec12.md#数据访问方法access-methods)
  - [顺序扫描（Sequential Scan）](./drafts/lec12.md#顺序扫描sequential-scan)
  - [索引扫描（Index Scan）](./drafts/lec12.md#索引扫描index-scan)
- [表达式评估（Expression Evaluation）](./drafts/lec12.md#表达式评估expression-evaluation)

#### Query Execution II

Readings: Chapter 18

- [./fall2019/notes/13-queryexecution2.pdf](./fall2019/notes/13-queryexecution2.pdf)
- [./fall2019/slides/13-queryexecution2.pdf](./fall2019/slides/13-queryexecution2.pdf)

AI 播客
- https://notebooklm.google.com/notebook/c206f7d4-9195-46cf-bf32-d0a360835ca8?artifactId=fcea16ea-da34-4ed8-9385-b9b889481397
- https://mp.weixin.qq.com/s/Cd2_f-JLIylNABN1l7gl5w

笔记： [./drafts/lec13.md](./drafts/lec13.md)
- [查询计划处理模型](./drafts/lec13.md#查询计划处理模型)
  - [迭代模型（Iterator Model）](./drafts/lec13.md#迭代模型iterator-model)
  - [物化模型（Materialization Model）](./drafts/lec13.md#物化模型materialization-model)
  - [向量化模型（Vectorized / Batch Model）](./drafts/lec13.md#向量化模型vectorized--batch-model)
- [数据访问方法（Access Methods）](./drafts/lec13.md#数据访问方法access-methods)
  - [顺序扫描（Sequential Scan）](./drafts/lec13.md#顺序扫描sequential-scan)
  - [索引扫描（Index Scan）](./drafts/lec13.md#索引扫描index-scan)
- [表达式评估（Expression Evaluation）](./drafts/lec13.md#表达式评估expression-evaluation)

#### Query Planning & Optimization I

Readings: Chapter 13

- [./fall2019/notes/14-optimization1.pdf](./fall2019/notes/14-optimization1.pdf)
- [./fall2019/slides/14-optimization1.pdf](./fall2019/slides/14-optimization1.pdf)

AI 播客
- https://notebooklm.google.com/notebook/4846443a-876c-42c6-8b28-a0b275419b98?artifactId=059466ed-01b4-4370-9cba-0f241d1d01b0
- https://mp.weixin.qq.com/s/1K9Kr0-b7V5I5wuk0HzkiA

笔记： [./drafts/lec14.md](./drafts/lec14.md)
- [查询优化概述](./drafts/lec14.md#查询优化概述)
- [查询优化的两种主要方法](./drafts/lec14.md#查询优化的两种主要方法)
- [静态规则与查询重写](./drafts/lec14.md#静态规则与查询重写)
  - [谓词下推 (Predicate Pushdown)](./drafts/lec14.md#谓词下推-predicate-pushdown)
  - [投影下推 (Projection Pushdown)](./drafts/lec14.md#投影下推-projection-pushdown)
  - [表达式简化与重写 (Expression Simplification and Rewriting)](./drafts/lec14.md#表达式简化与重写-expression-simplification-and-rewriting)
- [复杂查询与基于成本的搜索](./drafts/lec14.md#复杂查询与基于成本的搜索)
  - [成本模型 (Cost Model)](./drafts/lec14.md#成本模型-cost-model)
  - [统计信息 (Statistics Information)](./drafts/lec14.md#统计信息-statistics-information)
