# LEC 12: Distributed Transactions

<!-- @import "[TOC]" {cmd="toc" depthFrom=2 depthTo=6 orderedList=false} -->

<!-- code_chunk_output -->

- [NotebookLM 生成的双人播客](#notebooklm-生成的双人播客)
- [Gemini 2.5 Pro 生成的“分布式系统中的原子性、并发控制与两阶段提交”](#gemini-25-pro-生成的分布式系统中的原子性-并发控制与两阶段提交)
  - [ACID 确保事务的正确性](#acid-确保事务的正确性)
  - [一切或一无所有，这就是原子性](#一切或一无所有这就是原子性)
  - [并发控制与可串行化：承诺「先到后到」](#并发控制与可串行化承诺先到后到)
  - [两阶段锁定](#两阶段锁定)
  - [乐观与悲观之争](#乐观与悲观之争)
  - [乐观锁与悲观锁在 MySQL 中的实现](#乐观锁与悲观锁在-mysql-中的实现)
    - [悲观锁 (Pessimistic Locking)](#悲观锁-pessimistic-locking)
    - [乐观锁 (Optimistic Locking)](#乐观锁-optimistic-locking)
  - [分布式两阶段提交](#分布式两阶段提交)
    - [阶段一：投票阶段 (Voting Phase)](#阶段一投票阶段-voting-phase)
    - [阶段二：决定阶段 (Decision Phase)](#阶段二决定阶段-decision-phase)
    - [2PC 的挑战与替代方案](#2pc-的挑战与替代方案)
  - [实战中的两阶段提交 —— 以 MySQL 为例](#实战中的两阶段提交--以-mysql-为例)
    - [背景：MySQL 的主从复制](#背景mysql-的主从复制)
    - [问题：`redo log` 和 `binlog` 的一致性](#问题redo-log-和-binlog-的一致性)
    - [解决方案：内部的两阶段提交](#解决方案内部的两阶段提交)
    - [XID 的作用与崩溃恢复](#xid-的作用与崩溃恢复)

<!-- /code_chunk_output -->

## NotebookLM 生成的双人播客

注：播客里的“分式”实际上是“分布式”， AI 读错了

https://notebooklm.google.com/notebook/1b5332a0-a8e8-49c7-86fc-27da074e90dd/audio

微信链接： https://mp.weixin.qq.com/s/302zFsLVJe2khu9tE4k5Rg

给我留下的较深的印象：
1. 一致性 Consistency 我认为是最抽象的特性，并不直接规定事务的行为，而是规定系统的表现
2. 两阶段锁确保一致性 Isolation ，解决的是并发控制的问题（并不是分布式系统独有）
3. 两阶段提交解决的是分布式系统中主从一致的问题，并不是 Raft 解决的分布式系统对外保证线性一致性的问题

## Gemini 2.5 Pro 生成的“分布式系统中的原子性、并发控制与两阶段提交”

### ACID 确保事务的正确性

事务（Transaction）是并发控制和原子提交的抽象，它将一系列操作（可能在数据库中的不同记录上）视为一个单一的单元，并且不受故障或来自其他活动观察的影响。事务处理系统要求程序员标记操作序列的开始和结束。

数据库通常采用 **ACID** 原则来确保事务的正确性：

*   **原子性（Atomicity - A）**
  * **要么全部完成，要么全部不完成** 。即使发生故障，也不会出现部分更新完成而部分更新未完成的情况，它是“全有或全无”（all-or-nothing）的。原子性旨在掩盖程序执行过程中发生的故障。
*   **一致性（Consistency - C）**
  * 一致性通常指数据库将强制执行由应用程序声明的特定不变性（invariants）。
  * 如果我们在银行进行转账，那么一个重要的不变性是 **银行的总金额不应改变** 。也就是说，即使钱从一个账户转移到另一个账户，银行所有账户的总和也应该保持不变。
  * “一致性”属性确保在事务开始和结束时，数据库的状态都符合预定义的规则或约束。例如，如果一个转账事务成功完成，那么在扣除和增加金额之后，总资金量应仍然保持一致，否则事务将被视为无效并回滚。
*   **隔离性（Isolation - I）**
  * 隔离性是指并发执行的事务是否能够看到彼此的中间更改。目标是“不能”。从技术上讲，隔离性意味着事务的执行是 **可串行化（serializable）的** 。
  * **可串行化（Serializable）的含义：** 如果一组事务并发执行并产生结果（包括新的数据库记录和任何输出），那么这些结果是可串行化的， **当且仅当存在这些相同事务的某种串行执行顺序（即一次执行一个，不并行）能够产生与实际执行相同的结果**。 这意味着，即使事务是并发执行的，它们最终的效果也如同按某种顺序逐个执行一样。
  * **两阶段锁定（Two-Phase Locking - 2PL）** 是一种常用的并发控制机制，通过要求事务在操作数据前获取锁，并在事务提交或中止前一直持有锁，来强制实现隔离性或可串行化。两阶段锁定的“两个阶段”是指事务在 **获取锁** 和 **释放锁** 行为上的两个独立阶段（后文会详细探讨）。
*   **持久性（Durability - D）**
  * 持久性意味着在事务提交后（即客户端收到数据库已执行事务的回复后）， **事务对数据库的修改将是持久的，不会因任何形式的故障而被擦除** 。
  * 这通常意味着数据必须写入 **非易失性存储** ，例如磁盘。
  * **日志（Logging）** 是实现持久性的一种重要技术。系统会在将更改写入实际数据库之前，先将更改记录到一个 **日志** 中。即使系统在将更改“安装”（install）到主存储之前崩溃，恢复程序也可以使用日志来重做这些更改，确保数据最终的持久性。

---

### 一切或一无所有，这就是原子性

想象一下你在进行一次银行转账：从你的账户 A 转 100 元到朋友的账户 B。这个操作至少包含两个步骤：

1.  从账户 A 扣除 100 元。
2.  给账户 B 增加 100 元。

现在，设想一个最糟糕的情况：系统在完成第一步后突然崩溃了。你的钱被扣了，但你朋友没收到。这显然是不可接受的。

为了解决这类问题，我们需要一个保证： **这一系列操作要么全部成功，要么就像从未发生过一样** 。这就是 **原子性 (atomicity)** 的核心思想，它也是我们今天要讨论的 **事务 (transaction)** 最重要的特性之一。一个事务就是一组操作，它被设计为在面对并发和故障时，表现得像一个单一的、不可分割的单元。

### 并发控制与可串行化：承诺「先到后到」

在现实世界中，系统很少一次只处理一个请求。银行的系统可能同时在处理成千上万笔转账和查询。当多个事务并发执行，并且它们试图读写相同的数据时，新的问题就出现了。

比如，在你的转账事务 (T1) 正在进行的同时，另一个事务 (T2) 正在做全行审计，计算所有账户的总金额。如果 T2 在 T1 从账户 A 扣款后、向账户 B 加款前读取了 A 和 B 的余额，那么它会发现总金额少了 100 元，从而引发错误的警报。

为了防止这种混乱，我们需要另一种保证，称为 **先后原子性 (before-or-after atomicity)** 。它的意思是，并发事务的执行结果，必须和它们按照 *某个* 串行顺序（一个接一个）执行的结果完全一样。这个属性也叫做 **可串行化 (serializability)** 。

至于究竟是哪个串行顺序，我们通常不关心。只要最终结果是 `T1; T2` 或者 `T2; T1` 两种串行顺序之一的结果即可。这种保证让我们可以在享受并发带来的高性能的同时，不必担心事务之间互相干扰，产生意想不到的错误结果。

### 两阶段锁定

如何实现可串行化呢？一种常见的策略是 **悲观并发控制 (pessimistic concurrency control)** 。它的核心思想是“先申请再使用”，它假设冲突很可能会发生，因此通过锁定机制来阻止潜在的冲突。

最著名的悲观锁协议就是 **两阶段锁定 (two-phase locking, 简称 2PL)** 。注意，它的名字和我们稍后要讲的“两阶段提交”很像，但它们是完全不同的两个概念。

2PL 的规则很简单：

1.  **扩展阶段 (Phase 1)** ：事务可以根据需要获取锁，但不能释放任何锁。
2.  **收缩阶段 (Phase 2)** ：一旦事务释放了第一个锁，它就进入收缩阶段，此后只能释放锁，不能再获取任何新的锁。

在实践中，一种更严格也更常见的变体叫“强严格两阶段锁定”，它要求事务必须持有所有锁，直到事务结束（提交或中止）后才能一次性释放。

**为什么锁必须持有到事务结束？**

这是一个核心问题。想象一下，如果事务 T1 修改了数据 `x`，然后立即释放了对 `x` 的锁。此时，事务 T2 读取了 `x` 的新值并提交。但随后，T1 因为某种原因决定 **中止 (abort)** ，它会撤销自己对 `x` 的修改。这时，T2 的计算结果就建立在一个“从未存在过”的数据之上，破坏了系统的一致性。持有锁直到事务最终状态（提交或中止）确定，就是为了防止这种“脏读”问题。

**两阶段锁定会产生死锁吗？**

**会的** 。这是一个经典的场景：事务 T1 锁定了资源 A，然后尝试获取资源 B 的锁；同时，事务 T2 锁定了资源 B，并尝试获取资源 A 的锁。两者将永远地等待对方，形成 **死锁 (deadlock)** 。数据库系统通常有专门的机制来处理这种情况，比如通过超时或者检测等待图中的循环来发现死锁，然后强制中止其中一个事务来打破僵局。

**锁是排他的，还是允许多个读者？**

为了简化讨论，我们常假设锁是 **排他锁 (exclusive locks)** 。但在实际系统中，为了提高性能，通常会区分 **共享锁 (shared locks)** 和排他锁。多个事务可以同时持有同一个数据的共享锁（用于读取），但只要有一个事务想写入，它就必须获取排他锁，并且此时不能有任何其他事务持有该数据的任何锁（无论是共享还是排他）。

### 乐观与悲观之争

与悲观锁“先问后走”的策略相反，还有一种 **乐观并发控制 (optimistic concurrency control)** 。它的哲学是“先走再说，不行再道歉”。

在这种模型下，事务执行时不会加锁，它们自由地读取数据，并将修改写入一个私有工作区。直到事务准备提交时，系统才会进行冲突检查，看看在它执行期间，它读取的数据是否被其他已提交的事务修改过。如果没有冲突，就提交；如果发现冲突，那么这个事务就必须中止并重试。

**如何在悲观和乐观并发控制之间选择？**

这取决于你的应用场景中冲突发生的频率。

* 如果事务之间冲突非常频繁（比如很多用户抢购同一件商品）， **悲观锁** 更合适。虽然它可能会因为等待锁而降低并发度，但它避免了大量事务因冲突而中止重试所带来的无效工作。
* 如果事务之间冲突很少（比如用户大多在修改自己的个人资料）， **乐观锁** 更优。它省去了加锁和解锁的开销，允许更高的并发，只有在极少数发生冲突时才付出中止重日志回滚的代价。

### 乐观锁与悲观锁在 MySQL 中的实现

在 MySQL 中，这两种锁更多的是一种设计思想的体现，而不是两种有明确开关的独立功能。它们通过不同的 SQL 命令和表结构设计来实现。

#### 悲观锁 (Pessimistic Locking)

悲观锁的实现，完全依赖于数据库提供的原生锁机制。在 MySQL (主要指 InnoDB 存储引擎) 中，当你执行特定的 `SELECT` 语句时，就可以显式地为数据行加上悲观锁。

主要有两种方式：

* **共享锁 (Shared Lock)**

```sql
SELECT ... LOCK IN SHARE MODE;
```

这条语句会为你查询的行加上一个共享锁。其他事务可以读取这些行（也可以加共享锁），但不能修改它们，直到你的事务提交或回滚。这允许多个“读者”同时存在，但会阻塞“写者”。

* **排他锁 (Exclusive Lock)**

```sql
SELECT ... FOR UPDATE;
```

这是更强的锁。它会为你查询的行加上一个排他锁。其他任何事务都不能再为这些行加任何锁（无论是共享还是排他），也不能修改它们，直到你的事务结束。它同时阻塞了“读者”和“写者”。

当你执行一个普通的 `UPDATE` 或 `DELETE` 语句时，InnoDB 实际上也会自动地为涉及的行加上排他锁，这本身就是一种悲观锁的体现。

**总而言之，悲观锁在 MySQL 中是通过 `LOCK IN SHARE MODE` 和 `FOR UPDATE` 以及隐式的 `UPDATE/DELETE` 锁来实现的，它利用数据库的锁机制来强制同步，保证在修改数据期间的独占访问。**

#### 乐观锁 (Optimistic Locking)

乐观锁则完全相反，它不依赖于数据库的锁机制，而是在 **应用层面** 实现的一种并发控制策略。实现它的前提是，你需要在你的数据表中增加一个额外的列，通常是 `version` (版本号) 或者 `timestamp` (时间戳)。

实现步骤如下：

**一、增加版本列**

在你的表中增加一个 `version` 列，通常是整型，默认值为 0 或 1。

```sql
ALTER TABLE products ADD COLUMN version INT NOT NULL DEFAULT 1;
```

**二、读取数据时包含版本号**

当你的应用程序需要修改一条数据时，你首先将这条数据连同它的 `version` 值一起读出来。

```sql
SELECT id, name, stock, version FROM products WHERE id = 101;
```

假设读出的 `stock` 是 50，`version` 是 2。

**三、更新数据时校验并更新版本号**

当你准备将修改写回数据库时，你的 `UPDATE` 语句必须同时满足两个条件：`id` 匹配，并且 `version` 也要匹配你当初读出来的值。如果更新成功，则同时将 `version` 加一。

```sql
UPDATE products
SET stock = 49, version = version + 1
WHERE id = 101 AND version = 2;
```

**工作原理**

* 如果这条 `UPDATE` 语句成功执行，并且影响的行数为 1，说明在你读取数据到写入数据的这段时间内，没有其他事务修改过这条数据。更新成功！
* 如果影响的行数为 0，则说明在你操作的这段时间里，有另一个事务已经修改了这条数据，并增加了 `version` 的值。你手里的 `version = 2` 已经过时了。此时，更新失败。你的应用程序需要捕获这个“失败”，然后通常会重新读取最新的数据，再尝试一遍修改流程，或者提示用户操作冲突。

**总结来说，乐观锁在 MySQL 中是通过在表中增加版本字段，并在 `UPDATE` 时利用 `WHERE` 子句进行版本校验来实现的。它将并发控制的责任从数据库转移到了应用程序。**

### 分布式两阶段提交

当一个事务需要修改分布在不同服务器上的数据时，问题变得更加复杂。比如，一个跨行转账事务，既要操作 A 银行的数据库，也要操作 B 银行的数据库。我们如何保证这个分布式事务的原子性？

这就是 **两阶段提交 (two-phase commit, 简称 2PC)** 大显身手的地方。2PC 引入了两个角色：一个 **协调者 (coordinator)** 和多个 **参与者 (participants)** (或称为 workers)。

**会有多个事务同时活跃吗？参与者如何知道消息属于哪个事务？**

是的，系统中可以同时有许多活跃的分布式事务。为了区分它们，协调者发起的每个事务都会带有一个全局唯一的 **事务 ID (transaction ID)** 。所有在参与者之间传递的消息都会包含这个 ID，这样每个参与者就知道自己是在为哪个事务工作。

2PC 的流程如下：

#### 阶段一：投票阶段 (Voting Phase)

1.  **准备 (Prepare)** ：协调者向所有参与者发送一个 `PREPARE` 消息，询问“你们是否准备好提交？”。
2.  **投票 (Vote)** ：
  * 每个参与者收到 `PREPARE` 消息后，会检查自己是否能完成任务。如果可以，它会将所有需要的数据和操作记录到持久化的 **日志 (log)** 中，确保即使现在崩溃，重启后也能完成提交。然后，它向协调者回复 `PREPARED` 消息。一旦发送了 `PREPARED`，参与者就进入了“准备就绪”状态，它放弃了单方面中止的权利，只能等待协调者的最终指令。
  * 如果参与者因为任何原因无法完成任务，它会直接回复 `ABORT` 消息。

**参与者为何会发送 `ABORT` 而不是 `PREPARED`？**

有多种可能的原因：
* **本地约束冲突** ：例如，事务试图插入一个重复的主键，而表定义了主键唯一性约束。
* **死锁** ：参与者可能卷入了一个本地的锁死锁，为了打破死锁，它必须中止当前事务。
* **崩溃恢复** ：参与者可能在收到 `PREPARE` 之前就已经崩溃并重启，导致它丢失了为该事务所做的临时修改和持有的锁，因此无法保证能完成提交。

#### 阶段二：决定阶段 (Decision Phase)

1.  **做决定** ：协调者收集所有参与者的投票。
  * 如果 **所有** 参与者都回复了 `PREPARED`，协调者就决定 **提交 (commit)** 整个事务。
  * 如果 **任何一个** 参与者回复了 `ABORT`，或者在超时时间内没有响应，协调者就决定 **中止 (abort)** 整个事务。
2.  **通知结果** ：协调者将最终决定（`COMMIT` 或 `ABORT`）广播给所有参与者。参与者收到后，执行相应的操作（正式提交或回滚），然后释放资源。

**2PC 系统如何撤销修改？**

关键在于日志。在准备阶段，参与者不仅仅记录了要“做什么” (`redo` 信息)，也记录了如何“撤销” (`undo` 信息)。如果最终决定是中止，参与者就会根据日志中的 `undo` 记录，执行反向操作，将数据恢复到事务开始前的状态。

#### 2PC 的挑战与替代方案

2PC 虽然经典，但有一个致命弱点： **阻塞问题** 。

**如果协调者崩溃了，参与者该怎么办？**

这是 2PC 最大的问题。如果一个参与者已经发送了 `PREPARED` 消息，然后协调者崩溃了，这个参与者就完全不知道该提交还是中止。它不能自己做决定，因为其他参与者可能投了反对票。因此，它只能 **无限期地等待** ，并持有事务期间获得的锁，这会阻塞其他需要这些资源的事务，直到协调者恢复。

**为什么不用三阶段提交 (3PC)？**

3PC 确实是为了解决 2PC 的阻塞问题而设计的，它在准备和提交之间增加了一个“预提交”阶段。理论上，这允许在协调者崩溃后，存活的参与者们可以互相通信并达成一个一致的决定。然而，3PC 协议更复杂，通信开销更大，并且在面对网络分区（一部分参与者无法与另一部分通信）时仍然可能阻塞。在工程实践中，许多系统认为这种复杂性带来的收益有限，因此 2PC 仍然是更主流的选择。

**可以用 Raft 替代 2PC 吗？**

不行，它们解决的是不同的问题。
* **2PC** 是用来协调多个节点执行 **不同但相关** 的操作，并保证这些操作的原子性（要么都做，要么都不做）。它通常要求所有参与者都存活才能做出进展。
* **Raft** 是一种共识算法，用于让一组节点（副本）就 **同一个值或同一个操作序列** 达成一致，从而实现一个高可用的状态机。Raft 只需要大多数节点存活即可工作。

### 实战中的两阶段提交 —— 以 MySQL 为例

你可能觉得分布式事务离我们很遥远，但实际上，像 MySQL 这样的常用数据库内部就在使用 2PC 的思想来解决一致性问题。

#### 背景：MySQL 的主从复制

在生产环境中，MySQL 常常采用主从（Primary-Replica）或主备（Primary-Secondary）架构。所有写操作在主库上进行，然后通过一种叫做 **二进制日志 (binary log, `binlog`)** 的文件记录下来。从库会读取主库的 `binlog`，并在自己身上重放这些操作，从而与主库保持数据同步。

同时，MySQL 的 InnoDB 存储引擎自身也有一套用于崩溃恢复的日志系统，叫做 **重做日志 (redo log)** 。

#### 问题：`redo log` 和 `binlog` 的一致性

现在问题来了：一次事务提交，既要写 `redo log`（为了保证 InnoDB 自身崩溃后能恢复），也要写 `binlog`（为了让从库能同步）。这两次写操作必须是原子的。

想象一下，如果先写了 `redo log`，事务在主库上生效了，但还没来得及写 `binlog`，主库就崩溃了。主库重启后，通过 `redo log` 恢复了数据，但 `binlog` 里没有这次的修改记录，导致所有从库都丢失了这次更新，数据就不一致了。

反之，如果先写了 `binlog`，但还没写 `redo log` 就崩溃了。主库重启后，通过 `redo log` 回滚了未完成的事务，数据被撤销了。但 `binlog` 里却有这次的记录，从库会执行这次更新，数据同样不一致。

#### 解决方案：内部的两阶段提交

为了解决这个问题，MySQL 巧妙地在数据库服务器内部实现了一个两阶段提交。在这里：

* **协调者** ：是 MySQL 服务器本身。
* **参与者** ：主要是 InnoDB 存储引擎。

当客户端执行 `COMMIT` 时，流程如下：

1.  **阶段一：准备 (Prepare)**
  * MySQL 服务器通知 InnoDB：“准备提交事务”。
  * InnoDB 写入 `redo log`，并将这个事务标记为 **`prepared`** 状态。注意，此时事务并未真正提交，只是处于可以被提交的状态。

2.  **阶段二：提交 (Commit)**
  * 如果 InnoDB prepare 成功，MySQL 服务器就会将该事务写入 **`binlog`** 。
  * 写完 `binlog` 后，MySQL 服务器再通知 InnoDB：“正式提交事务”。
  * InnoDB 收到指令后，将 `redo log` 中该事务的状态从 `prepared` 修改为 **`committed`** 。提交完成。

#### XID 的作用与崩溃恢复

在这个过程中，一个关键的东西是 **事务 ID (XID)** 。它会被同时写入 `redo log` 和 `binlog`，作为两者关联的凭证。

如果系统在写完 `binlog` 后、InnoDB 最终提交前崩溃，重启时 MySQL 会这样做：
* 它会扫描最后的 `binlog` 文件，找出其中已经包含的事务 XID。
* 然后去检查 InnoDB `redo log` 中处于 `prepared` 状态的事务。
* 如果一个 `prepared` 状态的事务，其 XID 存在于 `binlog` 中，说明协调者（MySQL Server）在崩溃前已经做出了“提交”的决定（因为 `binlog` 已经写入），那么就命令 InnoDB 提交这个事务。
* 如果一个 `prepared` 状态的事务，其 XID **不** 存在于 `binlog` 中，说明协调者在崩溃前还没来得及做决定，那么就命令 InnoDB 回滚这个事务。

通过这种方式，MySQL 保证了 `redo log` 和 `binlog` 之间的数据一致性，从而确保了整个主从复制架构的可靠性。

**先写 `redo log` 还是 `binlog`？**

答案是： **先写 `redo log` (prepare 阶段)，再写 `binlog`** 。

这个顺序至关重要，是保证数据一致性的核心。让我们再回顾一下不这么做的后果：

* **如果先写 `binlog`，后写 `redo log`**
  1.  `binlog` 写入成功。（此时从库已经可以看到这个修改，并准备同步）
  2.  数据库 **崩溃** 。
  3.  `redo log` 还没来得及写。
  4.  **重启后** MySQL 通过 `redo log` 进行崩溃恢复，发现这个事务没有完成 `prepare` 和 `commit`，于是 **回滚** 了它。
  5.  结果主库数据被回滚，但从库执行了 `binlog` 中的操作。 **主从数据不一致** 。

* **正确的顺序：先写 `redo log` (prepare)，后写 `binlog`**
  1.  `redo log` 写入成功，状态为 `prepared`。
  2.  数据库 **崩溃** 。
  3.  `binlog` 还没来得及写。
  4.  **重启后** ：MySQL 进行恢复。它发现 `redo log` 中有一个 `prepared` 状态的事务，然后它会去 `binlog` 中查找对应的事务 ID (XID)。
  5.  **决策** ：因为它在 `binlog` 中 **找不到** 这个事务的记录，MySQL 就知道这个事务在崩溃前并没有被分发给从库，于是决定 **回滚** 它。
  6.  **结果** ：主库回滚了事务，`binlog` 里也没有这个事务，从库自然也不会执行。 **主从数据保持一致** 。

只有当 `binlog` 也成功写入后，整个事务才被认为是“可以安全提交的”。这时即使在最终 `commit redo log` 之前崩溃，恢复时也会因为在 `binlog` 中能找到记录而决定提交事务，最终依然能保证主从一致性。
