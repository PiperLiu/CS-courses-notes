# CMU 15-445/645 Database Systems

<!-- @import "[TOC]" {cmd="toc" depthFrom=3 depthTo=6 orderedList=false} -->

<!-- code_chunk_output -->

- [课程资料](#课程资料)
- [课程索引](#课程索引)
  - [Course Introduction and the Relational Model](#course-introduction-and-the-relational-model)

<!-- /code_chunk_output -->

### 课程资料

- https://15445.courses.cs.cmu.edu/fall2019/
- https://www.youtube.com/playlist?list=PLSE8ODhjZXjbohkNBWQs_otTrBTrjyohi

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
