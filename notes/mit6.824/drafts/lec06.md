# LEC 6: Fault Tolerance: Raft (1)

<!-- @import "[TOC]" {cmd="toc" depthFrom=2 depthTo=6 orderedList=false} -->

<!-- code_chunk_output -->

- [NotebookLM 生成的双人播客](#notebooklm-生成的双人播客)
- [Gemini 2.5 Pro 生成的 Raft 论文解析](#gemini-25-pro-生成的-raft-论文解析)
  - [引言：寻求易理解的共识算法](#引言寻求易理解的共识算法)
  - [复制状态机](#复制状态机)
  - [Paxos 的问题所在](#paxos-的问题所在)
  - [为可理解性而设计](#为可理解性而设计)
  - [Raft 共识算法](#raft-共识算法)
    - [5.1 Raft 基础](#51-raft-基础)
    - [5.2 领导者选举](#52-领导者选举)
    - [5.3 日志复制](#53-日志复制)
    - [5.4 安全性](#54-安全性)
      - [5.4.1 选举限制 (Election restriction)](#541-选举限制-election-restriction)
      - [5.4.2 提交先前任期的条目 (Committing entries from previous terms)](#542-提交先前任期的条目-committing-entries-from-previous-terms)
      - [5.4.3 安全性论证 (Safety argument)](#543-安全性论证-safety-argument)

<!-- /code_chunk_output -->

本文将详细解读 Raft 论文的前五个部分，探讨其设计哲学、核心机制以及相较于 Paxos 的改进之处。Raft 是一种旨在提供更易理解性的共识算法，同时不牺牲实际系统构建所需的功能和效率。

## NotebookLM 生成的双人播客

https://notebooklm.google.com/notebook/b9e439ba-7ab3-466b-8d00-8f1dc49fabe2/audio

微信链接： https://mp.weixin.qq.com/s/kf_WfLoAWL0f8CrvsaTvng

给我留下的较深的印象：
- 思路很明确，如果 follower 认为 leader 不存在，则等待一个随机事件，然后申请成为 leader （并为自己投票），通过等待随机时间来保证“所有 follower 同时给自己投票”成为小概率事件
- 大概思路听懂了，具体实现细节留给做 Lab 去感受吧

## Gemini 2.5 Pro 生成的 Raft 论文解析

### 引言：寻求易理解的共识算法

共识算法是构建可靠的大规模分布式软件系统的关键组件。它们允许多台机器作为一个高内聚的整体协同工作，即使部分成员发生故障也能继续提供服务。在过去的十年中，Paxos 算法一直是共识领域讨论的核心，大多数共识实现都基于或受其影响。然而，Paxos 因其难以理解而臭名昭著，并且其架构需要复杂的修改才能应用于实际系统。

正是由于 Paxos 的这些挑战，斯坦福大学的 Diego Ongaro 和 John Ousterhout 开始着手设计一种新的共识算法——Raft。Raft 的首要设计目标是 可理解性 (understandability)：不仅算法要能工作，更重要的是能让人清楚地理解它为什么能工作。为了实现这一目标，Raft 采用了 问题分解 (problem decomposition) 和 状态空间简化 (state space reduction) 等方法。用户研究也表明，学生学习 Raft 比学习 Paxos 更容易。

Raft 具备几个新颖特性：

* **强领导者 (Strong leader)** ：Raft 采用比其他共识算法更强的领导者模型。例如，日志条目只从领导者单向流向其他服务器，这简化了复制日志的管理。
* **领导者选举 (Leader election)** ：Raft 使用随机化定时器来选举领导者。这种方式在任何共识算法都必需的心跳机制上仅增加了少量开销，却能简单快速地解决冲突。
* **成员变更 (Membership changes)** ：Raft 采用一种新的联合共识 (joint consensus) 方法进行集群成员变更，通过重叠多数 (overlapping majorities) 来保证安全性。

Raft 被认为在教学和实现两方面均优于 Paxos，它更简单、描述更完整、已有多种开源实现并被多家公司采用，同时其安全性和效率也得到了保证。

### 复制状态机

共识算法通常应用于 复制状态机 (replicated state machines, RSMs) 的场景。在 RSM 模型中，一组服务器上的状态机计算相同状态的相同副本，并且即使某些服务器宕机也能继续运行。例如，GFS、HDFS 和 RAMCloud 等系统使用独立的复制状态机来管理领导者选举和存储配置信息。Chubby 和 ZooKeeper 也是复制状态机的著名例子。

复制状态机通常通过 复制日志 (replicated log) 实现，如图 1 (论文中的 Figure 1) 所示。每台服务器存储一个包含一系列命令的日志，其状态机按顺序执行这些命令。由于所有日志包含相同顺序的相同命令，且状态机是确定性的，因此它们会计算出相同的状态和输出序列。

```txt
+--------+      (1)      +------------------------------------+
| Client |-------------> | Server                             |
+--------+               | +-----------------+  State Machine |
                         | | Consensus       |   +-------+    |
                         | | Module          |-->| x: 3  |    |
                         | +-----------------+   | y: 9  |    |
                         |      (2) |            | z: 0  |    |
                         |          v            +-------+    |
                         |      +-----------------+   ^       |
                         |      | Log             |   |       |
                         |      | x←3|y←1|y←9|... |--(3)      |
                         |      +-----------------+           |
                         +------------------------------------+
                           (Multiple such servers)
```

*图示改编自论文 Figure 1：复制状态机架构。(1) 客户端发送命令给服务器。 (2) 共识模块将命令加入日志。 (3) 状态机按日志顺序执行命令。*

共识算法的核心任务是保持复制日志的一致性。服务器上的共识模块接收来自客户端的命令并将其添加到本地日志，然后与其他服务器的共识模块通信，以确保即使发生故障，每个日志最终都包含相同顺序的相同请求。一旦命令被正确复制，每个服务器的状态机就按日志顺序处理它们，并将输出返回给客户端。

一个实用的共识算法通常具备以下特性：

* 在所有非拜占庭条件下确保 安全性 (Safety)，即永不返回错误结果。
* 只要集群中多数服务器正常运行且能够相互通信及与客户端通信，就能保证 可用性 (Availability)。一个典型的五服务器集群可以容忍任意两台服务器的故障。
* 不依赖时序来保证日志的一致性；错误的时钟或极端的消息延迟最多导致可用性问题。
* 通常情况下，一条命令只需一轮 RPC 从领导者到多数服务器即可完成；少数慢速服务器不应影响整体系统性能。

### Paxos 的问题所在

尽管 Paxos 在共识领域占据主导地位，但它存在两个显著缺陷。

第一个缺陷是 Paxos 异常难以理解。其完整解释晦涩难懂，即使是简化版本也颇具挑战性。这种晦涩性可能源于其以 单决策 Paxos (single-decree Paxos) 为基础的构建方式。单决策 Paxos 本身就密集且微妙，其分为两个阶段，缺乏简单直观的解释且难以独立理解。

第二个缺陷是 Paxos 没有为构建实际系统提供良好的基础。没有一个被广泛认可的 多决策 Paxos (multi-Paxos) 算法；Lamport 的描述主要集中在单决策上，对多决策仅给出了大致思路，缺乏许多细节。此外，Paxos 的对等 (peer-to-peer) 核心架构不适合需要做出一系列决策的实际系统；先选举一个领导者，再由领导者协调决策会更简单高效。因此，实际系统往往与 Paxos 的原始描述相去甚远，开发者需要花费大量时间和精力进行修改和扩展，这既耗时又容易出错。正如 Chubby 的实现者所言：“Paxos 算法的描述与真实世界系统的需求之间存在巨大鸿沟……最终的系统将基于一个未经证明的协议。”

### 为可理解性而设计

Raft 的设计将可理解性置于首位。设计者不仅希望算法能够正确、高效地工作，还希望广大的开发者和学生能够轻松理解它，并建立起对算法行为的直观认识。

在设计过程中，Raft 采用了两种主要技术来增强可理解性：

1.  **问题分解 (Problem decomposition)** ：将复杂问题分解为若干个相对独立的子问题，这些子问题可以被独立解决、解释和理解。Raft 将共识问题分解为领导者选举、日志复制、安全性和成员变更几个部分。
2.  **状态空间简化 (State space reduction)** ：通过减少需要考虑的状态数量，使系统行为更具一致性，并尽可能消除不确定性。例如，Raft 不允许日志中存在空洞，并限制了日志之间可能出现不一致的方式。值得一提的是，Raft 在领导者选举中引入的随机化，虽然是一种不确定性，但它通过以相似方式处理所有可能选择，反而简化了状态空间，提升了理解性。

### Raft 共识算法

Raft 算法用于管理一个复制日志，其整体机制可以参考论文中的图 2 (Figure 2) 进行概览，关键属性总结在图 3 (Figure 3)。Raft 通过选举一位唯一的 领导者 (leader)，并赋予其管理复制日志的全部职责来实现共识。领导者从客户端接收日志条目，将其复制到其他服务器上，并告知服务器何时可以安全地将日志条目应用于它们的状态机。这种领导者模型简化了复制日志的管理。如果当前领导者失效，则会选举出新的领导者。

Raft 将共识问题分解为三个相对独立的子问题：

* **领导者选举 (Leader election)** ：当现有领导者失效时，必须选举出新的领导者 (5.2节)。
* **日志复制 (Log replication)** ：领导者必须接受来自客户端的日志条目，并在集群中进行复制，强制其他服务器的日志与自己保持一致 (5.3节)。
* **安全性 (Safety)** ：Raft 的关键安全属性是状态机安全属性 (State Machine Safety Property)：如果任一服务器已将特定日志条目应用于其状态机，则其他服务器不能对同一日志索引应用不同的命令 (5.4节)。

#### 5.1 Raft 基础

一个 Raft 集群包含若干服务器，通常是5台，这样可以容忍两台服务器的故障。在任何时刻，每台服务器都处于以下三种状态之一：领导者 (leader)、跟随者 (follower) 或 候选人 (candidate)。正常情况下，集群中只有一个领导者，其余所有服务器都是跟随者。跟随者是被动的，它们不主动发起请求，仅响应来自领导者和候选人的请求。所有客户端请求都由领导者处理；如果客户端联系到跟随者，跟随者会将其重定向到领导者。候选人状态则用于选举新的领导者。

Raft 将时间划分为连续的 任期 (terms)，每个任期用一个连续整数编号。每个任期从一次选举开始，一个或多个候选人尝试成为领导者。如果一个候选人赢得选举，它将在该任期的剩余时间内担任领导者。有时选举可能以选票分裂告终，导致该任期没有领导者，此时会很快开始一个新的任期（和新一轮选举）。Raft 保证在一个给定的任期中最多只有一个领导者。

任期在 Raft 中充当 逻辑时钟 (logical clock)，允许服务器检测过时的信息（如陈旧的领导者）。每台服务器都存储一个单调递增的当前任期号 (`currentTerm`)。服务器通信时会交换 `currentTerm`；如果一台服务器的 `currentTerm` 小于另一台，它会将自己的 `currentTerm` 更新为较大的值。如果候选人或领导者发现自己的任期已过时，它会立即转变为跟随者状态。如果服务器收到一个带有陈旧任期号的请求，它会拒绝该请求。

Raft 服务器之间通过 远程过程调用 (RPCs) 进行通信。基本的共识算法只需要两种类型的 RPC：

* `RequestVote` RPC：由候选人在选举期间发起 (5.2节)。
* `AppendEntries` RPC：由领导者发起，用于复制日志条目和作为心跳 (5.3节)。

#### 5.2 领导者选举

Raft 使用心跳机制来触发领导者选举。服务器启动时是跟随者状态。只要能从领导者或候选人那里收到有效的 RPC，服务器就保持跟随者状态。领导者会向所有跟随者发送周期性的心跳（不携带日志条目的 `AppendEntries` RPC）以维持其权威。如果一个跟随者在一段时间（称为 选举超时 (election timeout)）内没有收到任何通信，它就假定当前没有可行的领导者，并开始一次选举以选出新的领导者。

开始选举时，跟随者会：

1.  增加其本地的 `currentTerm`。
2.  转变为候选人状态。
3.  为自己投票。
4.  并行地向集群中的其他所有服务器发送 `RequestVote` RPC。

候选人会保持此状态，直到发生以下三种情况之一：(a) 它赢得选举；(b) 另一台服务器确立自己为领导者；(c) 一段时间过去后没有赢家。

* **赢得选举** ：候选人从整个集群中超过半数的服务器那里获得了针对同一任期的选票。每台服务器在给定的任期内最多只能为一名候选人投票（基于先到先得的原则，并受5.4节中额外限制的约束）。多数决规则确保了在一个特定任期内最多只能有一个候选人赢得选举（即选举安全属性）。一旦候选人获胜，它就成为领导者，并向所有其他服务器发送心跳消息以建立其权威并阻止新的选举。

* **发现其他领导者** ：在等待选票期间，候选人可能会收到来自另一台声称是领导者的服务器的 `AppendEntries` RPC。如果该领导者的任期（包含在其 RPC 中）大于或等于候选人的当前任期，则候选人承认该领导者的合法性并转换回跟随者状态。如果 RPC 中的任期小于候选人的当前任期，则候选人拒绝该 RPC 并继续保持候选人状态。

* **没有赢家（选票分裂）** ：如果许多跟随者同时成为候选人，选票可能会被瓜分，导致没有候选人获得多数票。发生这种情况时，每个候选人都会超时，并通过增加其任期号和发起另一轮 `RequestVote` RPC 来开始新的选举。

为了确保选票分裂很少发生并且能够迅速解决，Raft 使用了 随机化选举超时 (randomized election timeouts)。选举超时时间从一个固定的区间内随机选择（例如 150-300 毫秒）。这使得服务器的超时时间分散开，在大多数情况下，只有一个服务器会首先超时并发起选举；它赢得选举并在其他服务器超时之前发送心跳。处理选票分裂也使用相同的机制：每个候选人在开始选举时重新启动其随机化选举超时，并等待该超时结束后才开始下一次选举，这减少了在新选举中再次发生选票分裂的可能性。

#### 5.3 日志复制

一旦选出领导者，它就开始处理客户端请求。每个客户端请求都包含一个由复制状态机执行的命令。领导者将该命令作为新条目追加到其日志中，然后并行地向其他所有服务器发送 `AppendEntries` RPC 以复制该条目。当条目被安全复制后（如下所述），领导者将该条目应用于其状态机，并将执行结果返回给客户端。如果跟随者崩溃、运行缓慢或网络丢包，领导者会无限期地重试 `AppendEntries` RPC（即使在响应客户端之后），直到所有跟随者最终都存储了所有日志条目。

日志条目 (log entries) 包含状态机命令以及领导者收到该条目时的任期号。每个日志条目还有一个整数索引，标识其在日志中的位置。

一个日志条目被认为是 已提交的 (committed) 时，意味着它可以安全地应用于状态机了。Raft 保证已提交的条目是持久的，并且最终会被所有可用的状态机执行。当创建条目的领导者已在多数服务器上复制了该条目时，该条目即被提交（例如论文图 6 中的条目 7）。这也同时提交了领导者日志中所有在它之前的条目，包括由先前领导者创建的条目。领导者会跟踪它所知道的最高已提交条目的索引 (`commitIndex`)，并在未来的 `AppendEntries` RPC（包括心跳）中包含该索引，以便其他服务器最终得知。一旦跟随者得知某个日志条目已提交，它就会按日志顺序将该条目应用于其本地状态机。

Raft 的日志机制旨在维护不同服务器日志之间的高度一致性，这构成了 日志匹配特性 (Log Matching Property)：

1.  如果在不同日志中的两个条目具有相同的索引和任期，那么它们存储相同的命令。这是因为领导者在给定的任期内，对于一个给定的日志索引最多只创建一个条目，并且日志条目在日志中的位置永远不会改变。
2.  如果在不同日志中的两个条目具有相同的索引和任期，那么这些日志在直到该索引（包括该索引）之前的所有条目上都是相同的。这通过 `AppendEntries` RPC 执行的一个简单的一致性检查来保证：领导者在发送 `AppendEntries` RPC 时，会包含其日志中紧接在新条目之前的那个条目的索引和任期。如果跟随者在其日志中找不到具有相同索引和任期的条目，它就会拒绝新的条目。

正常操作期间，领导者和跟随者的日志保持一致，因此 `AppendEntries` 的一致性检查永远不会失败。然而，领导者崩溃可能会导致日志不一致（旧领导者可能没有完全复制其日志中的所有条目）。这些不一致性可能在一系列领导者和跟随者崩溃中累积（如论文图 7 所示）。

Raft 中，领导者通过强制跟随者的日志复制自己的日志来处理不一致性。这意味着跟随者日志中冲突的条目将被领导者日志中的条目覆盖。为了使跟随者的日志与自己的日志一致，领导者必须找到两个日志一致的最新日志条目，删除该点之后跟随者日志中的所有条目，并向跟随者发送该点之后领导者日志中的所有条目。这些操作都是响应 `AppendEntries` RPC 的一致性检查而发生的。

领导者为每个跟随者维护一个 `nextIndex`，即领导者将要发送给该跟随者的下一个日志条目的索引。当领导者刚上任时，它会将所有 `nextIndex` 值初始化为其日志中最后一个条目之后的索引。如果跟随者的日志与领导者的不一致，下一次 `AppendEntries` RPC 中的一致性检查将会失败。在被拒绝后，领导者会递减 `nextIndex` 并重试 `AppendEntries` RPC。最终 `nextIndex` 会达到一个领导者和跟随者日志匹配的点。此时 `AppendEntries` 将成功，它会移除跟随者日志中任何冲突的条目，并追加来自领导者日志的条目（如果有）。一旦 `AppendEntries` 成功，跟随者的日志就与领导者的日志一致了，并且在该任期的剩余时间里都将保持这种状态。

通过这种机制，领导者上任时无需采取任何特殊操作来恢复日志一致性。它只需开始正常操作，日志就会在 `AppendEntries` 一致性检查失败时自动趋于一致。领导者从不覆盖或删除其自身日志中的条目（领导者只追加属性）。

#### 5.4 安全性

前面描述的领导者选举和日志复制机制尚不足以确保每个状态机以完全相同的顺序执行完全相同的命令。例如，一个跟随者可能在领导者提交若干日志条目时不可用，然后它可能被选为领导者并用新的条目覆盖这些已提交的条目。本节通过增加一个关于哪些服务器可以被选举为领导者的限制来完善 Raft 算法。这个限制确保了任何给定任期的领导者都包含先前任期中所有已提交的条目（领导者完整性属性）。

##### 5.4.1 选举限制 (Election restriction)

在任何基于领导者的共识算法中，领导者最终必须存储所有已提交的日志条目。Raft 采用一种更简单的方法，它保证从选举的那一刻起，每个新领导者都拥有先前任期中所有已提交的条目，而无需将这些条目传输给领导者。这意味着日志条目只从领导者单向流向跟随者，并且领导者从不覆盖其日志中的现有条目。

Raft 利用投票过程来阻止一个候选人赢得选举，除非其日志包含了所有已提交的条目。候选人为了当选必须联系集群中的多数服务器，这意味着每个已提交的条目必须存在于该多数服务器中的至少一个服务器上。如果候选人的日志至少与该多数服务器中任何其他日志一样“最新”（“最新”的定义如下），那么它将持有所有已提交的条目。`RequestVote` RPC 实现了这一限制：RPC 中包含了关于候选人日志的信息，如果投票者自己的日志比候选人的日志更新，则它会拒绝投票。

Raft 通过比较日志中最后条目的索引和任期来判断两个日志哪个更新：
* 如果日志的最后条目具有不同的任期，则具有较晚任期的日志更新。
* 如果日志以相同的任期结束，则较长的日志更新。

##### 5.4.2 提交先前任期的条目 (Committing entries from previous terms)

如 5.3 节所述，领导者知道其当前任期的一个条目一旦存储在多数服务器上就被提交了。如果领导者在提交一个条目之前崩溃，未来的领导者将尝试完成该条目的复制。然而，领导者不能仅仅因为一个先前任期的条目存储在多数服务器上，就立即断定该条目已提交。论文中的图 8 展示了一种情况：一个旧的日志条目存储在多数服务器上，但仍可能被未来的领导者覆盖。

为了消除如图 8 所示的问题，Raft 规定：**从不通过计算副本数来提交先前任期的日志条目** 。只有领导者当前任期的日志条目才通过计算副本数来提交。一旦当前任期的某个条目以这种方式提交，那么所有先前的条目都会因为日志匹配特性而间接提交。Raft 采取这种更保守的方法是为了简化设计。

这种提交规则的复杂性源于当领导者复制先前任期的条目时，日志条目会保留其原始任期号。Raft 的这种方法使得推理日志条目更容易，因为它们在不同时间和不同日志中保持相同的任期号。此外，与其他算法相比，Raft 中的新领导者发送的先前任期的日志条目更少。

##### 5.4.3 安全性论证 (Safety argument)

有了完整的 Raft 算法，现在可以更精确地论证 领导者完整性属性 (Leader Completeness Property) 成立。该论证采用反证法。

假设 T 任期的领导者 `leaderT` 提交了其任期内的一个日志条目，但该条目未被某个未来任期 U (U \> T) 的领导者 `leaderU` 存储。

1.  该已提交条目在 `leaderU` 当选时必然不在其日志中（领导者从不删除或覆盖条目）。
2.  `leaderT` 在集群的多数服务器上复制了该条目，而 `leaderU` 从集群的多数服务器获得了选票。因此，至少有一个服务器（称为“投票者”）既接受了来自 `leaderT` 的条目，也投票给了 `leaderU` （如图 9 所示）。
3.  投票者必须在投票给 `leaderU` 之前就接受了来自 `leaderT` 的已提交条目；否则，它会拒绝来自 `leaderT` 的 `AppendEntries` 请求（因为它的当前任期会高于 T）。
4.  投票者在投票给 `leaderU` 时仍然存储着该条目，因为所有中间的领导者都包含该条目（根据假设），领导者从不移除条目，而跟随者只有在与领导者冲突时才移除条目。
5.  投票者将选票投给了 `leaderU`，因此 `leaderU` 的日志必须至少与投票者的日志一样新。这将导致以下两种矛盾之一：
    * 如果投票者和 `leaderU` 的最后日志任期相同，那么 `leaderU` 的日志必须至少与投票者的日志一样长，因此其日志包含了投票者日志中的每个条目。这与假设（`leaderU` 不包含该已提交条目，而投票者包含）相矛盾。
    * 否则，`leaderU` 的最后日志任期必须大于投票者的最后日志任期。而且，它也大于 T，因为投票者的最后日志任期至少是 T（它包含来自 T 任期的已提交条目）。创建 `leaderU` 最后日志条目的先前领导者在其日志中必须包含该已提交条目（根据假设）。那么，根据日志匹配特性，`leaderU` 的日志也必须包含该已提交条目，这又是一个矛盾。
6.  至此，矛盾论证完毕。因此，所有大于 T 的任期的领导者都必须包含 T 任期中在该任期提交的所有条目。

有了领导者完整性属性，就可以证明论文图 3 中的 状态机安全属性 (State Machine Safety Property)：如果一台服务器已将某个给定索引的日志条目应用于其状态机，则其他服务器永远不会对同一索引应用不同的日志条目。当服务器将日志条目应用于其状态机时，其日志必须与领导者的日志在该条目之前（包括该条目）完全相同，并且该条目必须是已提交的。现在考虑任何服务器应用给定日志索引的最低任期；领导者完整性属性保证所有更高任期的领导者都将存储相同的日志条目，因此在后续任期中应用该索引的服务器将应用相同的值。

最后，Raft 要求服务器按日志索引顺序应用条目。结合状态机安全属性，这意味着所有服务器都将以相同的顺序将完全相同的日志条目集应用于其状态机。
