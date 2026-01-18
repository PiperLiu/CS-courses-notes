# 6.824 Lab 2: Raft

目录：

<!-- @import "[TOC]" {cmd="toc" depthFrom=2 depthTo=2 orderedList=false} -->

<!-- code_chunk_output -->

- [2A 实现 Leader 选举](#2a-实现-leader-选举)
- [2B 实现日志状态同步](#2b-实现日志状态同步)
- [2C 持久化](#2c-持久化)

<!-- /code_chunk_output -->

细分目录：

<!-- @import "[TOC]" {cmd="toc" depthFrom=2 depthTo=6 orderedList=false} -->

<!-- code_chunk_output -->

- [2A 实现 Leader 选举](#2a-实现-leader-选举)
  - [架构设计探索](#架构设计探索)
  - [最终实现方案](#最终实现方案)
    - [1. 状态机结构](#1-状态机结构)
    - [2. 主循环设计](#2-主循环设计)
    - [3. 角色转换逻辑](#3-角色转换逻辑)
    - [4. 时间参数设置](#4-时间参数设置)
  - [遇到的问题与解决](#遇到的问题与解决)
    - [问题 1: 死锁](#问题-1-死锁)
    - [问题 2: Split Vote 死锁](#问题-2-split-vote-死锁)
    - [问题 3: Ticker 切换问题](#问题-3-ticker-切换问题)
  - [核心 RPC 实现与决策逻辑详解](#核心-rpc-实现与决策逻辑详解)
    - [RequestVote RPC Handler（被请求方）](#requestvote-rpc-handler被请求方)
    - [startElection（发起选举，请求方）](#startelection发起选举请求方)
    - [AppendEntries RPC (心跳)](#appendentries-rpc-心跳)
  - [测试结果](#测试结果)
  - [ChatGPT 5.2 介绍 etcd-io/raft 的方案：驱动型 goroutine / event-loop](#chatgpt-52-介绍-etcd-ioraft-的方案驱动型-goroutine--event-loop)
    - [1）设计哲学：只做“Raft 算法内核”，IO 全交给使用者](#1设计哲学只做raft-算法内核io-全交给使用者)
    - [2）三层结构：`raft`（纯状态机） / `RawNode`（薄封装） / `Node`（并发友好外壳）](#2三层结构raft纯状态机--rawnode薄封装--node并发友好外壳)
    - [3）关键交互协议：`Ready()` →（持久化/发送/apply）→ `Advance()`](#3关键交互协议ready-持久化发送apply-advance)
    - [4）时间驱动：`Tick()` + 抽象 tick（不是直接用 time.Time）](#4时间驱动tick--抽象-tick不是直接用-timetime)
    - [5）消息驱动：`Step()` / `Propose()` / `Campaign()`](#5消息驱动step--propose--campaign)
    - [6）典型 event-loop 长相（把 Ready 当成“输出”，Tick/Step/Propose 当成“输入”）](#6典型-event-loop-长相把-ready-当成输出ticksteppropose-当成输入)
- [2B 实现日志状态同步](#2b-实现日志状态同步)
  - [2B 测试用例分析](#2b-测试用例分析)
  - [测试框架详解](#测试框架详解)
    - [Start() 函数的含义](#start-函数的含义)
    - [cfg.one() 函数](#cfgone-函数)
    - [cfg.nCommitted() 函数](#cfgncommitted-函数)
    - [数据流完整路径](#数据流完整路径)
    - [TestBasicAgree2B - 基本一致性测试](#testbasicagree2b---基本一致性测试)
    - [TestFailAgree2B - 网络分区恢复测试](#testfailagree2b---网络分区恢复测试)
    - [TestBackup2B - 快速日志回退测试](#testbackup2b---快速日志回退测试)
  - [实现方案](#实现方案)
    - [1. 数据结构扩展](#1-数据结构扩展)
    - [2. Start() 实现](#2-start-实现)
    - [3. AppendEntries RPC 完整实现](#3-appendentries-rpc-完整实现)
    - [4. commitIndex 更新和日志应用](#4-commitindex-更新和日志应用)
  - [关键概念 FAQ](#关键概念-faq)
    - [Q1: "乐观初始化" nextIndex 是什么意思？](#q1-乐观初始化-nextindex-是什么意思)
    - [Q2: 日志不一致时如何处理？nextIndex 回退机制](#q2-日志不一致时如何处理nextindex-回退机制)
    - [Q3: "顺带提交" 是什么意思？](#q3-顺带提交-是什么意思)
    - [ChatGPT 5.2: 为什么不能直接提交旧 term 的日志？（Figure 8 讲的到底是什么）](#chatgpt-52-为什么不能直接提交旧-term-的日志figure-8-讲的到底是什么)
      - [1）Figure 8 面临的核心问题：“复制到多数”≠“Leader 能确认已经提交”](#1figure-8-面临的核心问题复制到多数leader-能确认已经提交)
      - [2）Figure 8 的时序（a→e）到底在演示什么](#2figure-8-的时序ae到底在演示什么)
      - [3）所以 Raft 的正确规则是什么？为什么它正确？](#3所以-raft-的正确规则是什么为什么它正确)
    - [5. becomeLeader 初始化](#5-becomeleader-初始化)
  - [测试结果](#测试结果-1)
- [2C 持久化](#2c-持久化)
  - [需要持久化的状态](#需要持久化的状态)
  - [persist() 和 readPersist() 实现](#persist-和-readpersist-实现)
  - [何时调用 persist()](#何时调用-persist)
  - [快速 nextIndex 回退优化](#快速-nextindex-回退优化)
    - [快速回退优化详细示例](#快速回退优化详细示例)
  - [遇到的问题与解决](#遇到的问题与解决-1)
    - [问题 1: 节点重启后日志被覆盖](#问题-1-节点重启后日志被覆盖)
    - [问题 2: applyLogs 中的并发问题](#问题-2-applylogs-中的并发问题)
      - [为什么原有逻辑不对？](#为什么原有逻辑不对)
      - [为什么要"不持有锁时发送 ApplyMsg"？](#为什么要不持有锁时发送-applymsg)
      - [正确的解决方案：专门的 applier goroutine](#正确的解决方案专门的-applier-goroutine)
  - [测试结果](#测试结果-2)

<!-- /code_chunk_output -->

在做这个实验之前，我还是保持 go 版本是 1.13.6 ，但是稍微修改了下整个 repo 的 `import` 代码，生成了 `go.mod` 。这样更加适配 VS Code 的插件系统。我安装的 VS Code go 插件是 0.22.0 版本的，可以使用 gopls 而非 gocode 。

之后进入 `src/raft` 运行 `go test -run 2A` 即可； `go fmt` 帮助你整理代码。

## 2A 实现 Leader 选举

### 架构设计探索

我一开始的设想是把 **状态机转换** 实现出来，用一个 goroutine 。

```go
func (rf *Raft) start() {
	pre := time.Now()
	for !rf.killed() {
		// state machine from Figure 4 (part of, not all transforms triggered here)
		// do its own role's routine
		cur := time.Now()
		switch rf.serverRole {
		case Follower:
			if cur.Sub(pre) >= followerWaitTime {
				rf.transRole(Candidate) // this function is atomic
			} else {
				rf.sleepRandom(followerSleepLower, followerSleepUpper)
			}
		case Candidate:
			if cur.Sub(pre) >= candidateWaitTime {
				rf.requestVote()
			} else {
				rf.sleepRandom(candidateSleepLower, candidateSleepUpper)
			}
		case Leader:
			// TODO hearbeat / appendEntries
		}
		pre = cur
	}
	DPrintf("Raft %v found itself killed, main routine ends", rf.me)
}
```

但是的问题是：
- 一个 `switch` 无法完全覆盖 FSM 有限状态机所有转移路径
  - 比如 vote 来了，按照如上的 time trigger 逻辑，放在 Candidate 分支是不合适的
- 因此在其他 goroutine 还可能涉及状态转换，比如 `onVoted` ，但这又涉及到 `Role` 的竞态（虽然我已经让 `transRole` 是 `atomic` 的了）
- 因此目前的设计模式中，对 FSM 刻画能力不够，逻辑分散不适合维护与 debug

决定先学习 [etcd-io/raft](https://github.com/etcd-io/raft) 的设计模式 **驱动型的 goroutine/event-loop** ，主要是以下文件：
- raft node 内部实现： node.go , rawnode.go, raft.go
- storage 接口定义以及实例 `MemoryStorage` ： storage.go
- 一个简单的应用示例（该如何使用 etcd-io/raft ）： rafttest/node.go

> 最终没有选用 etcd-io 的方案。对比放在了本文最后一个小节。

### 最终实现方案

最终采用简化的 **定时器驱动** 架构，核心思路：

#### 1. 状态机结构

```go
type Raft struct {
    // 标准 Raft 状态
    currentTerm int          // 当前任期
    votedFor    int          // 本 term 投票给了谁
    serverRole  RaftRoleEnum // Follower/Candidate/Leader

    // 超时控制
    electionElapsed  time.Time     // 上次重置选举超时的时间
    electionTimeout  time.Duration // 随机化的选举超时
    heartBeatElapsed time.Time     // 上次发送心跳的时间
}
```

#### 2. 主循环设计

使用单一 goroutine + 定时轮询的方式，简化并发控制：

```go
func (rf *Raft) start() {
    rf.becomeFollower(0, NoneLeader)
    for !rf.killed() {
        time.Sleep(time.Millisecond * 10) // 固定间隔检查
        rf.tick()
    }
}

func (rf *Raft) tick() {
    rf.mu.Lock()
    defer rf.mu.Unlock()

    switch rf.serverRole {
    case Follower:
        // 超时则转为 Candidate 并发起选举
    case Candidate:
        // 超时则重新发起选举
    case Leader:
        // 定期发送心跳
    }
}
```

#### 3. 角色转换逻辑

三个角色转换函数，都在持有锁的情况下调用：

- **becomeFollower(term, leader)** ：转为 Follower，重置 votedFor
- **becomeCandidate()** ：转为 Candidate（term 增加在 startElection 中）
- **becomeLeader()** ：转为 Leader，立即发送心跳

#### 4. 时间参数设置

根据 Lab 要求：
- Leader 心跳间隔 ≥ 100ms（≤10次/秒）
- 必须在 5 秒内选出新 leader
- 选举超时 > 心跳间隔

最终参数：

```go
heartBeatInterval    = 100ms
electionTimeoutLower = 250ms
electionTimeoutUpper = 400ms
```

### 遇到的问题与解决

#### 问题 1: 死锁

最初设计中使用多个 RWMutex，在状态转换时容易形成死锁。例如：`tickFollowerTimeout` 持有 `followerState.mu.RLock()` 的同时调用 `becomeCandidate()`，后者又需要获取同一把锁。

**解决方案** ：简化为单一 `sync.Mutex`，所有状态访问和修改都通过这把锁保护。

#### 问题 2: Split Vote 死锁

当 leader 断开后，两个节点同时发起选举，各自投票给自己，形成死循环：

```
[0] starting election at term 3
[2] starting election at term 3
[2] rejected vote for 0: already voted for 2
[0] rejected vote for 2: already voted for 0
(无限循环...)
```

**原因** ：两个 Candidate 在同一个 term 中不断重试选举，但 `votedFor` 没有重置。

**解决方案** ：每次发起选举时（无论是首次还是超时重试）都增加 term：

```go
func (rf *Raft) startElection() {
    rf.currentTerm++    // 每次选举都增加 term
    rf.votedFor = rf.me // 新 term，投票给自己
    // ... 发送 RequestVote RPC
}
```

这样当一方的 term 先增加，另一方收到更高 term 的 RequestVote 时会更新自己的 term 并可能投票给对方。

#### 问题 3: Ticker 切换问题

最初使用多个自定义 `myTicker`，角色切换时修改 `rf.ticker` 指针，但主循环中的 `select` 仍监听旧 channel。

**解决方案** ：改用简单的 `time.Sleep` 轮询 + 时间戳检查，避免 channel 切换问题。

### 核心 RPC 实现与决策逻辑详解

#### RequestVote RPC Handler（被请求方）

```go
func (rf *Raft) RequestVote(args *RequestVoteArgs, reply *RequestVoteReply) {
    rf.mu.Lock()
    defer rf.mu.Unlock()

    reply.Term = rf.currentTerm
    reply.VoteGranted = false

    // 1. term 过期，拒绝
    if args.Term < rf.currentTerm {
        return
    }

    // 2. 发现更高 term，转为 follower
    if args.Term > rf.currentTerm {
        rf.becomeFollower(args.Term, NoneLeader)
    }

    // 3. 检查是否可以投票
    if rf.votedFor == NoneVoted || rf.votedFor == args.CandidateId {
        if rf.isLogUpToDate(args.LastLogIndex, args.LastLogTerm) {
            rf.votedFor = args.CandidateId
            reply.VoteGranted = true
            rf.resetElectionTimeout()
        }
    }
}
```

**Q1: 变成 Follower 后，还需要投票吗？**

**需要** ，这是两个独立的步骤：

```
步骤 2: becomeFollower(args.Term, NoneLeader)
    └─ 内部会执行: rf.votedFor = NoneVoted  // 重置投票状态

步骤 3: 检查 votedFor == NoneVoted  // 此时条件成立
    └─ 如果日志足够新 → 投票给请求者
```

关键在于 `becomeFollower()` 会重置 `votedFor = NoneVoted`：

```go
func (rf *Raft) becomeFollower(term int, leader int) {
    rf.serverRole = Follower
    rf.currentTerm = term
    rf.leader = leader
    rf.votedFor = NoneVoted  // ← 新 term 时重置投票
    rf.resetElectionTimeout()
}
```

**完整决策流程图：**

```
收到 RequestVote(Term=5, CandidateId=2)
当前状态: Term=3, votedFor=1
    │
    ▼
┌─────────────────────────────────────┐
│  args.Term(5) < currentTerm(3) ?    │
│  NO → 继续                           │
└─────────────────────────────────────┘
    │
    ▼
┌─────────────────────────────────────┐
│  args.Term(5) > currentTerm(3) ?    │
│  YES → becomeFollower(5, -1)        │
│        ├─ currentTerm = 5           │
│        └─ votedFor = -1 (NoneVoted) │  ← 关键：重置投票
└─────────────────────────────────────┘
    │
    ▼
┌─────────────────────────────────────┐
│  votedFor(-1) == NoneVoted ?        │
│  YES → 可以考虑投票                   │
│  检查日志是否足够新...                 │
│  → votedFor = 2, VoteGranted = true │
└─────────────────────────────────────┘
```

**为什么要这样设计？** 因为 Raft 的投票规则是 **每个 term 只能投一票** 。当进入新 term 时，之前的投票记录作废，需要重新投票。

#### startElection（发起选举，请求方）

```go
func (rf *Raft) startElection() {
    rf.currentTerm++           // 增加 term
    rf.votedFor = rf.me        // 投票给自己（占用本 term 的投票权）
    rf.resetElectionTimeout()

    votesReceived := 1         // 自己的票
    votesNeeded := len(rf.peers)/2 + 1
    currentTerm := rf.currentTerm  // 保存当前 term

    // 并发发送 RequestVote RPC
    for i := range rf.peers {
        if i == rf.me { continue }
        go func(server int) {
            reply := &RequestVoteReply{}
            if rf.sendRequestVote(server, args, reply) {
                rf.mu.Lock()
                defer rf.mu.Unlock()

                // ===== 票数结算在这里 =====

                // 检查 1: 角色是否还是 Candidate
                if rf.serverRole != Candidate { return }

                // 检查 2: term 是否改变（可能已经进入下一轮选举）
                if rf.currentTerm != currentTerm { return }

                // 检查 3: 对方 term 更大，退回 Follower
                if reply.Term > rf.currentTerm {
                    rf.becomeFollower(reply.Term, NoneLeader)
                    return
                }

                // 检查 4: 收到投票，累加并判断是否过半
                if reply.VoteGranted {
                    votesReceived++
                    if votesReceived >= votesNeeded {
                        rf.becomeLeader()  // ← 成为 Leader
                    }
                }
            }
        }(i)
    }
}
```

**Q2: 在哪里结算票数？如何转为 Leader？**

票数结算发生在 **每个 RPC 回调的 goroutine 中** ，而不是统一收集后处理。

**关键点：**

| 位置 | 操作 |
|------|------|
| `startElection()` 开始 | `votesReceived = 1`（自己投给自己） |
| 每个 RPC 回调 goroutine | 收到 `VoteGranted=true` 时 `votesReceived++` |
| 每次累加后立即检查 | `if votesReceived >= votesNeeded → becomeLeader()` |

**为什么在回调中结算而不是等所有 RPC 返回？**

1. **快速响应** ：只要获得多数票就可以立即成为 Leader，不必等待所有响应
2. **处理网络分区** ：有些节点可能永远不响应，不能阻塞选举
3. **时效性** ：如果等待太久，可能已经超时需要重新选举

**状态转换的完整流程：**

```
Follower ──(选举超时)──→ Candidate ──(获得多数票)──→ Leader
    ▲                        │
    │                        │
    └──(发现更高term/收到心跳)──┘
```

**并发安全保证：**

```go
go func(server int) {
    // ... RPC 调用（无锁）
    
    rf.mu.Lock()         // ← 加锁
    defer rf.mu.Unlock()
    
    // 三重检查，确保状态一致性
    if rf.serverRole != Candidate { return }  // 可能已经变成 Leader/Follower
    if rf.currentTerm != currentTerm { return }  // 可能已经是新一轮选举
    if reply.Term > rf.currentTerm { ... }  // 发现更高 term
    
    // 安全地累加票数
    votesReceived++
    if votesReceived >= votesNeeded {
        rf.becomeLeader()
    }
}(i)
```

**注意** ：`votesReceived` 是在外层函数的局部变量，多个 goroutine 共享它。因为所有对它的访问都在 `rf.mu.Lock()` 保护下，所以是线程安全的。

#### AppendEntries RPC (心跳)

```go
func (rf *Raft) AppendEntries(args *AppendEntriesArgs, reply *AppendEntriesReply) {
    rf.mu.Lock()
    defer rf.mu.Unlock()

    reply.Term = rf.currentTerm
    reply.Success = false

    if args.Term < rf.currentTerm {
        return
    }

    // 收到有效心跳，转为/保持 Follower
    if args.Term > rf.currentTerm {
        rf.becomeFollower(args.Term, args.LeaderId)
    } else {
        rf.serverRole = Follower
        rf.leader = args.LeaderId
        rf.resetElectionTimeout()
    }

    reply.Success = true
}
```

### 测试结果

```bash
$ go test -run 2A -race
Test (2A): initial election ...
  ... Passed --   3.0  3   54    6660    0
Test (2A): election after network failure ...
  ... Passed --   4.5  3  124   10816    0
PASS
ok      raft    8.653s
```

测试稳定通过，无竞态条件。

### ChatGPT 5.2 介绍 etcd-io/raft 的方案：驱动型 goroutine / event-loop

#### 1）设计哲学：只做“Raft 算法内核”，IO 全交给使用者

etcd-io/raft 的核心取舍是：库只实现 **Raft 状态机（算法）** ，而 **网络收发、消息序列化、磁盘持久化、状态机 apply** 都由使用者自己实现。这种“极简内核”带来：

* **可复用** ：同一套 raft 内核可嵌入不同系统（etcd、K8s 等）
* **确定性** ：把 Raft 建模成状态机：输入（tick/网络消息）→ 输出（要发送的消息、要落盘的日志、状态变更）
* **解耦与可测** ：网络/存储可以用 mock 或内存实现，便于压测与单测

#### 2）三层结构：`raft`（纯状态机） / `RawNode`（薄封装） / `Node`（并发友好外壳）

在代码组织上可以粗略这么理解：

* **raft.go 里的 raft 结构体** ：真正的算法状态机（Follower/Candidate/Leader 逻辑、Step、tick、选举、复制等）。
* **RawNode** ：给 raft 状态机套一层“更方便的 API”（例如把一些内部状态组织成 `Ready`）。
* **Node** ：更“工程化”的并发接口：用 channel + 内部 goroutine 串行化所有对 raft 状态机的访问，让使用者通过 **Ready 通道** 拿到输出，通过 **Step/Tick/Propose** 输入事件（消息/时间/提案）。这就是你提到的“驱动型 goroutine/event-loop”。

#### 3）关键交互协议：`Ready()` →（持久化/发送/apply）→ `Advance()`

这是 etcd-io/raft 最“灵魂”的部分：**应用层必须消费 Ready** ，按顺序完成几类动作，然后调用 Advance 告诉 raft 可以产出下一批 Ready。官方文档把“你拿到 Node 以后要做的事”写得非常明确，核心步骤是：

1. **持久化（Storage）** ：把 `HardState / Entries / Snapshot` 写入稳定存储（不为空时）。
2. **网络发送（Transport）** ：把 `Ready.Messages` 发给对应节点；并且强调一些 **顺序约束** ：不要在最新 HardState 没落盘前发送消息；消息序列化也建议在主循环内串行化处理以避免并发坑。
3. **状态机应用（Apply）** ：把 `CommittedEntries`（以及 Snapshot）应用到你的业务状态机；遇到配置变更条目要 `ApplyConfChange`。
4. **推进（Advance）** ：调用 `Node.Advance()` 表示这一批 Ready 处理完成，可以生成下一批。

你可以把它理解成一个非常明确的“生产者-消费者握手协议”：

* raft **生产** ：Ready（要落盘什么 / 要发什么 / 可 apply 什么）
* 应用 **消费** ：落盘、发送、apply
* 应用 **确认** ：Advance（允许 raft 继续往前走）

这套协议的工程价值非常高： **raft 内核永远不直接碰 IO** ，但它能通过 Ready 把“下一步需要你做的 IO”精确描述出来。

#### 4）时间驱动：`Tick()` + 抽象 tick（不是直接用 time.Time）

在 etcd-io/raft 里，“时间”被抽象成离散 tick：应用层用 `time.Ticker` 定期调用 `Node.Tick()`，raft 内部用 electionTick/heartbeatTick 计数推进选举超时与心跳周期。官方文档直接给了一个“总循环长什么样”的参考伪代码：select 里一边 tick，一边读 Ready。

这点和你现在的实现（sleep + tick 检查）精神上很接近，但 etcd 的做法更“纯状态机”：

* raft 内部只关心“过了几个 tick”，不关心真实时间戳
* 应用层决定 tick 的真实间隔（比如 100ms 一次）

#### 5）消息驱动：`Step()` / `Propose()` / `Campaign()`

在 etcd-io/raft 的世界里，输入事件主要来自三类：

* **网络消息** ：收到别的节点的 raftpb.Message → `Node.Step(ctx, m)`
* **本地时间** ：定期 `Node.Tick()`
* **客户端提案** ：业务命令序列化成 bytes → `Node.Propose(ctx, data)`（提交后会以 CommittedEntries 形式回到你手里）

选举也可以通过类似 `Campaign()` 的方式由“上层触发”，但最常见的是 tick 驱动触发超时后由 raft 内部进入选举流程。

#### 6）典型 event-loop 长相（把 Ready 当成“输出”，Tick/Step/Propose 当成“输入”）

把官方文档的核心形态抽象出来，你会得到一个非常经典的“单线程状态机 + 多路输入输出”的循环（伪代码）：

* `Ticker.C`：喂时间 → Tick()
* `recvChan`：喂网络消息 → Step()
* `proposeChan`：喂业务提案 → Propose()
* `Ready()`：取输出 → persist / send / apply → Advance()

这种结构本质上解决了你在笔记里最早遇到的两个痛点：

1. **FSM 转移不再分散在多个 goroutine** ：所有状态变更都串在 raft 内部的主路径里（tick/Step 驱动）。
2. **不靠大锁也能避免竞态** ：Node 通过内部 goroutine/channel，把“谁能碰 raft 状态”这件事收敛成单点。

## 2B 实现日志状态同步

### 2B 测试用例分析

Lab 2B 包含 8 个测试用例，用于验证日志复制和一致性的各种场景：

| 测试函数 | 测试目标 | 关键检测点 |
|---------|---------|-----------|
| **TestBasicAgree2B** | 基本日志一致性 | 3 节点集群能否正确复制和提交日志，index 是否正确递增 |
| **TestRPCBytes2B** | RPC 字节效率 | 每条命令只应发送给每个 peer 一次，避免不必要的日志重传 |
| **TestFailAgree2B** | follower 断连后恢复 | 1 个 follower 断开后，剩余 2 个能否达成一致；重连后能否同步 |
| **TestFailNoAgree2B** | 无多数派时不提交 | 5 节点中 3 个断开，Leader 不应提交新日志；恢复后应能正常工作 |
| **TestConcurrentStarts2B** | 并发 Start() 调用 | 多个 goroutine 同时调用 Start()，所有命令都应被正确复制 |
| **TestRejoin2B** | 分区 leader 重新加入 | 旧 leader 的未提交日志应被新 leader 覆盖 |
| **TestBackup2B** | 快速日志回退 | leader 日志与 follower 大量不一致时，能否快速找到匹配点 |
| **TestCount2B** | RPC 数量控制 | 日志复制 RPC 数量应合理，空闲时心跳数量应在限制内 |

### 测试框架详解

在分析具体测试之前，先理解测试框架中的核心函数和概念。

#### Start() 函数的含义

`Start(command)` 是 Raft 对外暴露的核心 API， **不是执行命令，而是发起命令的一致性协商** ：

```go
// 调用方（客户端/测试框架）
index, term, isLeader := rf.Start(command)
```

**语义：**
- 如果当前节点不是 Leader，立即返回 `isLeader=false`
- 如果是 Leader，将 `command` 追加到本地日志，立即返回（ **不等待提交** ）
- 返回值 `index` 是该命令 **将会** 存储的日志位置（如果最终被提交的话）

**关键点：** `Start()` 只是"提案"，不保证命令会被提交。命令真正被执行是在 Raft 通过 `applyCh` 发送 `ApplyMsg` 之后。

```
┌─────────────┐     Start(cmd)      ┌─────────────┐
│   客户端     │ ──────────────────→ │   Leader    │
└─────────────┘                     └─────────────┘
      │                                    │
      │                            追加到本地日志
      │                                    │
      │                            广播 AppendEntries
      │                                    │
      │                            等待多数派确认
      │                                    │
      │       ApplyMsg(index, cmd)         │
      │ ←──────────────────────────────────
      │
    执行命令
```

#### cfg.one() 函数

`one(cmd, expectedServers, retry)` 是测试框架的核心函数，用于提交一条命令并等待一致性：

```go
func (cfg *config) one(cmd interface{}, expectedServers int, retry bool) int {
    for time.Since(t0).Seconds() < 10 {
        // 1. 遍历所有节点，找到 Leader 并调用 Start()
        for si := 0; si < cfg.n; si++ {
            index1, _, ok := rf.Start(cmd)  // ← 这里调用了 Start
            if ok {
                index = index1
                break
            }
        }
        
        // 2. 等待 expectedServers 个节点提交该命令
        for time.Since(t1).Seconds() < 2 {
            nd, cmd1 := cfg.nCommitted(index)
            if nd >= expectedServers && cmd1 == cmd {
                return index  // 成功！
            }
        }
    }
    // 超时失败
}
```

**参数含义：**
- `cmd`: 要提交的命令
- `expectedServers`: 期望多少个节点提交（用于测试分区场景）
- `retry`: 如果 Leader 失败，是否重试

**返回值：** 命令被存储的日志 index

#### cfg.nCommitted() 函数

`nCommitted(index)` 检查有多少个节点已经提交了指定 index 的日志：

```go
func (cfg *config) nCommitted(index int) (int, interface{}) {
    count := 0
    var cmd interface{} = nil
    for i := 0; i < len(cfg.rafts); i++ {
        cfg.mu.Lock()
        cmd1, ok := cfg.logs[i][index]  // cfg.logs 记录每个节点 apply 的命令
        cfg.mu.Unlock()

        if ok {
            // 检查所有节点提交的值是否一致
            if count > 0 && cmd != cmd1 {
                cfg.t.Fatalf("committed values do not match")
            }
            count++
            cmd = cmd1
        }
    }
    return count, cmd
}
```

**关键：** `cfg.logs[i][index]` 的数据来源是 `applyCh`。测试框架启动时会为每个 Raft 节点创建一个 goroutine 监听 `applyCh`：

```go
// config.go 中的 start1()
applyCh := make(chan ApplyMsg)
go func() {
    for m := range applyCh {
        if m.CommandValid {
            cfg.logs[i][m.CommandIndex] = m.Command  // ← 记录 apply 的命令
        }
    }
}()
rf := Make(ends, i, cfg.saved[i], applyCh)
```

#### 数据流完整路径

```
TestBasicAgree2B
    │
    └──→ cfg.one(100, 3, false)
              │
              ├──→ rf.Start(100)           // Leader 追加日志
              │         │
              │         └──→ broadcastAppendEntries()
              │                    │
              │                    └──→ Follower.AppendEntries()
              │                              │
              │                              └──→ 更新 log, commitIndex
              │                              └──→ applyLogs()
              │                                      │
              │                                      └──→ applyCh <- ApplyMsg
              │
              └──→ cfg.nCommitted(1)        // 检查 cfg.logs
                         │
                         └──→ 读取 cfg.logs[i][1]  // 来自 applyCh
```

#### TestBasicAgree2B - 基本一致性测试

```go
func TestBasicAgree2B(t *testing.T) {
    servers := 3
    cfg := make_config(t, servers, false)
    
    for index := 1; index < 4; index++ {
        // 检查没人在 Start() 之前就 commit 了
        nd, _ := cfg.nCommitted(index)
        if nd > 0 {
            t.Fatalf("some have committed before Start()")
        }

        // cfg.one() 内部调用 Start()，等待所有节点达成一致
        xindex := cfg.one(index*100, servers, false)
        if xindex != index {
            t.Fatalf("got index %v but expected %v", xindex, index)
        }
    }
}
```

**Start() 在哪里被调用？**

在 `cfg.one()` 内部：
```go
// cfg.one() 核心逻辑
for si := 0; si < cfg.n; si++ {
    index1, _, ok := rf.Start(cmd)  // ← 这里调用 Start
    if ok { ... }
}
```

**测试流程：**
1. `cfg.one(100, 3, false)` 找到 Leader 并调用 `Leader.Start(100)`
2. Leader 将命令追加到日志 index=1
3. Leader 广播 AppendEntries 给 Follower
4. 多数节点复制成功后，Leader 更新 commitIndex
5. 所有节点通过 `applyCh` 发送 ApplyMsg
6. `cfg.nCommitted(1)` 检测到 3 个节点都 apply 了该命令
7. 返回 index=1

#### TestFailAgree2B - 网络分区恢复测试

```go
func TestFailAgree2B(t *testing.T) {
    // 1. 正常提交一条日志
    cfg.one(101, servers, false)

    // 2. 断开一个 follower
    leader := cfg.checkOneLeader()
    cfg.disconnect((leader + 1) % servers)

    // 3. 剩余 2 个节点应能继续提交
    cfg.one(102, servers-1, false)
    cfg.one(103, servers-1, false)

    // 4. 重连后，所有 3 个节点应该同步
    cfg.connect((leader + 1) % servers)
    cfg.one(106, servers, true)  // 等待全部同步
}
```

**测试要点：**
1. 2/3 节点可以达成多数派，继续提交日志
2. 断开的节点重连后，应通过 AppendEntries 同步缺失的日志
3. 这测试了 **nextIndex 回退** 机制

#### TestBackup2B - 快速日志回退测试

这是最复杂的测试，模拟了 Figure 8 描述的场景：

![](./todo)

```go
func TestBackup2B(t *testing.T) {
    // 1. 将 leader 和 1 个 follower 放入小分区
    // 2. 在小分区中提交 50 条不会成功的日志
    // 3. 断开小分区，连接大分区（3 个节点）
    // 4. 大分区选出新 leader，成功提交 50 条日志
    // 5. 再制造分区，再次产生 50 条冲突日志
    // 6. 恢复所有节点，验证最终一致性
}
```

**测试要点：**
1. 旧 leader 的未提交日志必须被覆盖
2. 当日志大量不一致时，回退不能太慢（不能每次只回退 1 个）
3. 测试限时，需要实现高效的 **nextIndex 快速回退**

### 实现方案

#### 1. 数据结构扩展

```go
// 日志条目
type LogEntry struct {
    Term    int         // 该条目被创建时的 term
    Command interface{} // 状态机命令
}

type Raft struct {
    // ... 2A 的字段 ...

    // 日志相关
    log         []LogEntry // 日志条目，索引从 1 开始（log[0] 是占位符）
    commitIndex int        // 已知已提交的最高日志索引
    lastApplied int        // 已应用到状态机的最高日志索引

    // Leader 专用（重新当选时重新初始化）
    nextIndex   []int // 对于每个 server，要发送给它的下一条日志的索引
    matchIndex  []int // 对于每个 server，已知已复制到该 server 的最高日志索引

    applyCh     chan ApplyMsg // 用于向状态机发送已提交的日志
}
```

#### 2. Start() 实现

```go
func (rf *Raft) Start(command interface{}) (int, int, bool) {
    rf.mu.Lock()
    defer rf.mu.Unlock()

    if rf.serverRole != Leader {
        return -1, rf.currentTerm, false
    }

    // 追加日志到本地
    entry := LogEntry{Term: rf.currentTerm, Command: command}
    rf.log = append(rf.log, entry)
    index := len(rf.log) - 1

    // 更新自己的 matchIndex
    rf.matchIndex[rf.me] = index
    rf.nextIndex[rf.me] = index + 1

    // 立即触发日志复制
    rf.broadcastAppendEntries()

    return index, rf.currentTerm, true
}
```

**关键点：**
- `Start()` 立即返回，不等待日志提交
- 返回的 index 是日志将被存储的位置
- 立即触发 `broadcastAppendEntries` 加速复制

#### 3. AppendEntries RPC 完整实现

**发送端（Leader）：**

```go
func (rf *Raft) broadcastAppendEntries() {
    for i := range rf.peers {
        if i == rf.me { continue }

        prevLogIndex := rf.nextIndex[i] - 1
        prevLogTerm := rf.log[prevLogIndex].Term

        // 复制要发送的日志条目
        entries := append([]LogEntry{}, rf.log[rf.nextIndex[i]:]...)

        go func(server int, args *AppendEntriesArgs) {
            reply := &AppendEntriesReply{}
            if rf.sendAppendEntries(server, args, reply) {
                rf.mu.Lock()
                defer rf.mu.Unlock()

                if reply.Success {
                    // 更新 matchIndex 和 nextIndex
                    newMatchIndex := args.PrevLogIndex + len(args.Entries)
                    rf.matchIndex[server] = max(rf.matchIndex[server], newMatchIndex)
                    rf.nextIndex[server] = rf.matchIndex[server] + 1

                    // 尝试更新 commitIndex
                    rf.updateCommitIndex()
                } else {
                    // 回退 nextIndex
                    rf.nextIndex[server]--
                }
            }
        }(i, args)
    }
}
```

**接收端（Follower）：**

```go
func (rf *Raft) AppendEntries(args *AppendEntriesArgs, reply *AppendEntriesReply) {
    rf.mu.Lock()
    defer rf.mu.Unlock()
    
    // 1. term 检查
    if args.Term < rf.currentTerm {
        reply.Success = false
        return
    }
    
    // 2. 转为 follower 并重置选举超时
    rf.becomeFollower(args.Term, args.LeaderId)
    
    // 3. 日志一致性检查
    if args.PrevLogIndex >= len(rf.log) ||
       rf.log[args.PrevLogIndex].Term != args.PrevLogTerm {
        reply.Success = false
        return
    }

    // 4. 追加日志（处理冲突）
    // 找到第一个冲突点，截断后追加
    insertIdx := args.PrevLogIndex + 1
    for i, entry := range args.Entries {
        if insertIdx >= len(rf.log) || rf.log[insertIdx].Term != entry.Term {
            rf.log = append(rf.log[:insertIdx], args.Entries[i:]...)
            break
        }
        insertIdx++
    }
    
    // 5. 更新 commitIndex
    if args.LeaderCommit > rf.commitIndex {
        rf.commitIndex = min(args.LeaderCommit, len(rf.log)-1)
        rf.applyLogs()
    }
    
    reply.Success = true
}
```

#### 4. commitIndex 更新和日志应用

```go
// Leader 更新 commitIndex
func (rf *Raft) updateCommitIndex() {
    // 找到最大的 N，使得多数 matchIndex[i] >= N 且 log[N].term == currentTerm
    for n := rf.getLastLogIndex(); n > rf.commitIndex; n-- {
        if rf.log[n].Term != rf.currentTerm {
            continue // 只能提交当前 term 的日志
        }
        count := 1
        for i := range rf.peers {
            if i != rf.me && rf.matchIndex[i] >= n {
                count++
            }
        }
        if count > len(rf.peers)/2 {
            rf.commitIndex = n
            rf.applyLogs()
            break
        }
    }
}

// 应用日志到状态机
func (rf *Raft) applyLogs() {
    for rf.lastApplied < rf.commitIndex {
        rf.lastApplied++
        msg := ApplyMsg{
            CommandValid: true,
            Command:      rf.log[rf.lastApplied].Command,
            CommandIndex: rf.lastApplied,
        }
        rf.mu.Unlock()  // 释放锁后发送，避免死锁
        rf.applyCh <- msg
        rf.mu.Lock()
    }
}
```

**为什么只能提交当前 term 的日志？**

这是 Raft 论文 Figure 8 描述的关键安全性规则。如果 leader 提交旧 term 的日志，可能会导致已提交的日志被覆盖。通过只提交当前 term 的日志，可以"顺带"提交之前的日志（因为 commitIndex 之前的日志都被隐式提交）。

### 关键概念 FAQ

#### Q1: "乐观初始化" nextIndex 是什么意思？

```go
rf.nextIndex[i] = lastLogIndex + 1  // 乐观初始化
```

**"乐观"** 指的是：Leader 初始时 **假设** 所有 Follower 的日志都和自己一样新。

- `lastLogIndex + 1` 意味着：下一次发送 AppendEntries 时，不发送任何旧日志，只发送新日志
- 这是一种 **乐观假设**，假设 Follower 不需要同步旧数据

**为什么这样设计？**
1. **常见情况优化** ：大多数时候 Follower 的日志确实是最新的
2. **快速启动** ：新 Leader 可以立即开始工作
3. **按需回退** ：如果假设错误（日志不一致），通过 `nextIndex--` 逐步回退找到正确位置

**对比"悲观初始化"：**

```go
rf.nextIndex[i] = 1  // 悲观初始化（从头开始同步）
```

这样虽然肯定能同步成功，但会浪费大量带宽重传已有的日志。

#### Q2: 日志不一致时如何处理？nextIndex 回退机制

当 Leader 发送 AppendEntries 时带上 `prevLogIndex` 和 `prevLogTerm`：

```go
prevLogIndex := rf.nextIndex[i] - 1
prevLogTerm := rf.log[prevLogIndex].Term
```

Follower 检查自己在 `prevLogIndex` 位置的日志 term 是否匹配：

```go
if rf.log[args.PrevLogIndex].Term != args.PrevLogTerm {
    reply.Success = false  // 不匹配，拒绝
    return
}
```

**不匹配时的处理流程（nextIndex 回退）：**

```
Leader                              Follower
nextIndex[1] = 5
    │
    ├──→ AppendEntries(prevLogIndex=4, prevLogTerm=3)
    │                                   │
    │                       log[4].Term=2 ≠ 3
    │                                   │
    │ ←── reply.Success = false ────────┘
    │
nextIndex[1] = 4  // 回退
    │
    ├──→ AppendEntries(prevLogIndex=3, prevLogTerm=2)
    │                                   │
    │                       log[3].Term=2 == 2 ✓
    │                                   │
    │ ←── reply.Success = true ─────────┘
    │
更新 matchIndex[1] = 3 + len(entries)
```

**关键代码：**

```go
// Leader 收到拒绝后回退
if reply.Success {
    // 成功，更新 matchIndex
} else {
    // 失败，回退 nextIndex
    if rf.nextIndex[server] > 1 {
        rf.nextIndex[server]--
    }
}
```

**为什么这样能保证最终同步？**

1. 每次心跳/日志复制都会重试 AppendEntries
2. nextIndex 每次失败回退 1，最终会找到匹配点
3. 找到匹配点后，Follower 会截断冲突日志并追加 Leader 的日志

**时序图示例：**

```
初始状态：
Leader log:   [0] [1,term1] [2,term1] [3,term2] [4,term2]
Follower log: [0] [1,term1] [2,term1] [3,term1]  ← term 不同！

第 1 轮：nextIndex=5, prevLogIndex=4
  Follower: log[4] 不存在 → reject

第 2 轮：nextIndex=4, prevLogIndex=3
  Follower: log[3].term=1 ≠ 2 → reject

第 3 轮：nextIndex=3, prevLogIndex=2
  Follower: log[2].term=1 == 1 → accept
  Follower: 截断 log[3], 追加 Leader 的 [3,term2] [4,term2]

最终状态：
Leader log:   [0] [1,term1] [2,term1] [3,term2] [4,term2]
Follower log: [0] [1,term1] [2,term1] [3,term2] [4,term2]  ← 一致！
```

#### Q3: "顺带提交" 是什么意思？

```go
if rf.log[n].Term != rf.currentTerm {
    continue // 只能提交当前 term 的日志
}
```

**场景说明：**

假设 Leader 在 term 4 当选，日志状态：
```
index:  1    2    3    4    5
term:   1    2    3    3    4
        ↑              ↑
      旧日志        当前term
```

Leader 只能直接提交 term=4 的日志（index=5），但当 `commitIndex=5` 时：

```
commitIndex = 5
    │
    └──→ applyLogs() 会 apply index 1, 2, 3, 4, 5 的所有日志
```

#### ChatGPT 5.2: 为什么不能直接提交旧 term 的日志？（Figure 8 讲的到底是什么）

##### 1）Figure 8 面临的核心问题：“复制到多数”≠“Leader 能确认已经提交”

在 Raft 里，“提交（commit）”的含义很强： **一旦某条日志被认为已提交，它以及它之前的所有日志就必须永远不会被覆盖/回滚** （安全性要求）。

直觉上我们会想：

> 只要某条日志已经复制到多数派（majority），那就提交它。

但 Figure 8 证明： **当这条“多数复制”的日志来自旧 term 时，Leader 可能会误判** ——它看到了“多数复制”，却无法保证“将来不会被新 Leader 覆盖”。

##### 2）Figure 8 的时序（a→e）到底在演示什么

为避免抽象，我们按图里的 (a)-(e) 走一遍：

**(a) 旧 Leader S1（term=2）产生了一条日志（index=2, term=2），只复制给了部分节点**

* S1 是 Leader，在 index=2 写入了 term=2 的日志条目。
* 这条日志只复制到了一部分机器（例如 S1、S2），还没来得及稳定扩散。

**(b) S1 崩溃；S5 在 term=3 当选 Leader，并在 **同一个 index=2** 写入了另一条不同的日志（term=3）**

* 由于网络/崩溃等原因， **S5 得到了多数票** （比如 S3、S4、S5）。
* 注意：给 S5 投票的那些节点（S3、S4） **并没有 S1 的 term=2 那条 index=2 日志** ，它们的日志更短/更旧。
* S5 当选后，在 index=2 写入 term=3 的新条目（与 S1 在 index=2 的 term=2 条目冲突）。

**(c) S5 崩溃；S1 恢复并在 term=4 再次当选 Leader，开始继续复制**

* S1 又当选 Leader（term=4），开始把自己日志往外复制。
* 关键点来了：此时 S1 可以把自己那条 **旧 term 的条目（index=2, term=2）** 复制到“多数节点”上（图里强调： **看起来它已经在多数派出现了** ）。

到这里，最容易犯的错误就是：

> “既然 index=2 的 term=2 条目已经复制到多数派了，那我（S1）就把它提交吧。”

Figure 8 要打碎的就是这个直觉。

**(d) 如果此刻 S1 再次崩溃：S5 仍可能当选 Leader，并把 index=2 覆盖成自己的 term=3 条目**

为什么 S5 **还能当选** ？关键在“投票只看你当时这轮选举的多数派是什么样子”，并且“日志新旧”的判定规则可能允许它赢：

* 假设此时多数派由 **S2、S3、S4** 组成（图里就是这么画的）。
* 这三个投票者里，可能都不具备“足够强”的证据来拒绝 S5（例如它们的最后日志 term/index 并不比 S5 更新，或者它们并没有看到 term=4 的任何新日志）。
* 因此 **S5 可以在更高 term 的选举中重新当选 Leader** 。
* 一旦 S5 当选，它会用自己的日志去“统一世界”：把 follower 的冲突日志回滚掉，覆盖成它在 term=3 的版本。于是：

  * **S1 之前如果把 (index=2, term=2) 当成“已提交”，现在它就被覆盖了。**
  * 这直接违反“已提交日志永不回滚”的安全性。

**这就是错误方案的后果：**

> 把“旧 term 的多数复制”当作“已提交”，会导致“已提交条目被未来 Leader 覆盖”的灾难。

**(e) 正确做法：S1 必须先让“当前 term 的新条目”在多数派上落地，然后才能推进 commitIndex**

Figure 8 的结论是：

* 如果 S1 在 term=4 期间，先复制了一条 **term=4 的新日志** 到多数派，那么这条 term=4 的日志才是 S1 能“确定提交”的东西。
* 一旦多数派中已经存在 term=4 的日志，接下来任何候选人想当选，都必须“日志至少和投票者一样新”。
  换句话说： **缺少 term=4 这条新日志的人，很难（或不可能）从这些投票者手里拿到多数票。**
* 因而，未来当选的 Leader 必然包含这条 term=4 日志；而 Raft 的日志匹配性质又保证： **包含这条 term=4 日志的 Leader，不会再把它之前的日志回滚掉** 。
* 于是，当 term=4 的这条新日志被提交时，`commitIndex` 推进到它的位置，前面的旧日志也就“顺带”全部变成提交态（并且是安全的）。

##### 3）所以 Raft 的正确规则是什么？为什么它正确？

**规则（你实现里那句）：**

> Leader 只能依据“多数派复制”来提交 **自己当前 term 的日志** （`log[N].term == currentTerm`）。

**它为什么正确** ？一句话抓住要害：

* **提交当前 term 的日志，会把“选举能否成功”与“该日志是否存在于多数派”绑定起来。**
  只要多数派里已经有了 term=T 的日志条目，那么未来要赢得多数票的新 Leader，日志必须足够新，通常就必须包含这条（或更“新”的）日志；这样就不可能出现“新 Leader 当选后把这条已提交日志覆盖掉”。

而对于旧 term 的日志：

* 即使它在某个时刻“看似”出现在多数派里，只要多数派还没有看到当前 term 的任何新日志，就仍然可能选出一个不包含它（或包含冲突版本）的 Leader，从而把它覆盖掉。
* 因此 **Leader 不能仅凭“旧 term 多数复制”就宣布它已提交**。

#### 5. becomeLeader 初始化

```go
func (rf *Raft) becomeLeader() {
    rf.serverRole = Leader
    rf.leader = rf.me
    
    // 初始化 nextIndex 和 matchIndex
    lastLogIndex := rf.getLastLogIndex()
    rf.nextIndex = make([]int, len(rf.peers))
    rf.matchIndex = make([]int, len(rf.peers))
    for i := range rf.peers {
        rf.nextIndex[i] = lastLogIndex + 1  // 乐观初始化
        rf.matchIndex[i] = 0
    }
    
    // 立即发送心跳
    rf.broadcastAppendEntries()
}
```

### 测试结果

```bash
$ go test -run 2B -race
Test (2B): basic agreement ...
  ... Passed --   0.7  3   16    4142    3
Test (2B): RPC byte count ...
  ... Passed --   1.5  3   48  112956   11
Test (2B): agreement despite follower disconnection ...
  ... Passed --   5.4  3  114   27773    8
Test (2B): no agreement if too many followers disconnect ...
  ... Passed --   3.5  5  208   42082    3
Test (2B): concurrent Start()s ...
  ... Passed --   0.6  3   20    5516    6
Test (2B): rejoin of partitioned leader ...
  ... Passed --   6.0  3  180   43180    4
Test (2B): leader backs up quickly over incorrect follower logs ...
  ... Passed --  26.2  5 2696 1940057  107
Test (2B): RPC counts aren't too high ...
  ... Passed --   2.2  3   64   19500   12
PASS
ok      raft    47.151s
```

所有测试稳定通过。

## 2C 持久化

### 需要持久化的状态

根据 Raft 论文 Figure 2，需要持久化的状态有三个：

- `currentTerm`：当前任期
- `votedFor`：当前任期投票给了谁
- `log[]`：日志条目

这些状态需要在每次修改后立即写入磁盘，以保证节点崩溃重启后能够恢复到一致的状态。

### persist() 和 readPersist() 实现

```go
// persist 将 Raft 状态持久化到磁盘
// 必须在持有 rf.mu 锁的情况下调用
func (rf *Raft) persist() {
	w := new(bytes.Buffer)
	e := labgob.NewEncoder(w)
	e.Encode(rf.currentTerm)
	e.Encode(rf.votedFor)
	e.Encode(rf.log)
	data := w.Bytes()
	rf.persister.SaveRaftState(data)
	DPrintf("[%d] persisted state: term=%d, votedFor=%d, logLen=%d",
		rf.me, rf.currentTerm, rf.votedFor, len(rf.log))
}

// readPersist 从磁盘恢复 Raft 状态
func (rf *Raft) readPersist(data []byte) {
	if data == nil || len(data) < 1 {
		return
	}
	r := bytes.NewBuffer(data)
	d := labgob.NewDecoder(r)
	var currentTerm int
	var votedFor int
	var log []LogEntry
	if d.Decode(&currentTerm) != nil ||
		d.Decode(&votedFor) != nil ||
		d.Decode(&log) != nil {
		DPrintf("[%d] failed to read persisted state", rf.me)
		return
	}
	rf.currentTerm = currentTerm
	rf.votedFor = votedFor
	rf.log = log
	DPrintf("[%d] restored state: term=%d, votedFor=%d, logLen=%d",
		rf.me, rf.currentTerm, rf.votedFor, len(rf.log))
}
```

### 何时调用 persist()

需要在所有修改持久化状态的地方调用 `persist()`：

1. **becomeFollower()** - 修改了 currentTerm 和 votedFor
2. **startElection()** - 修改了 currentTerm 和 votedFor
3. **RequestVote handler** - 投票时修改了 votedFor
4. **AppendEntries handler** - 追加日志时修改了 log
5. **Start()** - Leader 收到新命令，追加到 log

### 快速 nextIndex 回退优化

当 AppendEntries 因为日志不一致而失败时，简单的策略是每次 `nextIndex--`。但这在日志差异很大时效率很低。

**优化策略** ：Follower 返回冲突信息，帮助 Leader 快速定位：

```go
type AppendEntriesReply struct {
	Term    int
	Success bool
	// 快速回退优化
	ConflictTerm  int // 冲突日志条目的 term（-1 表示日志太短）
	ConflictIndex int // 该 term 的第一个日志索引，或日志长度
}
```

**Follower 端处理** ：

```go
// 日志太短
if args.PrevLogIndex >= len(rf.log) {
	reply.ConflictTerm = -1
	reply.ConflictIndex = len(rf.log)
	return
}
// term 不匹配
if rf.log[args.PrevLogIndex].Term != args.PrevLogTerm {
	reply.ConflictTerm = rf.log[args.PrevLogIndex].Term
	// 找到冲突 term 的第一个日志索引
	reply.ConflictIndex = args.PrevLogIndex
	for reply.ConflictIndex > 1 && rf.log[reply.ConflictIndex-1].Term == reply.ConflictTerm {
		reply.ConflictIndex--
	}
	return
}
```

**Leader 端处理** ：

```go
if reply.ConflictTerm == -1 {
	// Follower 日志太短，直接跳到 ConflictIndex
	rf.nextIndex[server] = reply.ConflictIndex
} else {
	// 查找 Leader 日志中是否有 ConflictTerm
	lastIndexOfConflictTerm := -1
	for i := args.PrevLogIndex; i >= 1; i-- {
		if rf.log[i].Term == reply.ConflictTerm {
			lastIndexOfConflictTerm = i
			break
		}
	}
	if lastIndexOfConflictTerm != -1 {
		rf.nextIndex[server] = lastIndexOfConflictTerm + 1
	} else {
		rf.nextIndex[server] = reply.ConflictIndex
	}
}
```

#### 快速回退优化详细示例

为了理解这个优化的作用，让我们通过具体例子来分析。

**场景设定** ：假设 Leader 和 Follower 的日志如下（方括号内数字表示 term）：

```
Leader 日志:   [0] [1] [1] [1] [4] [4] [5] [5] [6]
索引:           0   1   2   3   4   5   6   7   8

Follower 日志: [0] [1] [1] [1] [2] [2] [2] [3] [3] [3] [3]
索引:           0   1   2   3   4   5   6   7   8   9  10
```

Leader 初始 `nextIndex[follower] = 9`（Leader 的 lastLogIndex + 1）

**第一轮 AppendEntries**
- Leader 发送：`prevLogIndex=8, prevLogTerm=6`
- Follower 检查：`log[8].Term = 3 ≠ 6`，term 不匹配！

**Follower 返回冲突信息**

```
ConflictTerm = 3  (Follower 在 index 8 处的 term)
ConflictIndex = 7 (term=3 的第一个日志索引)
```

**Leader 处理**
- 查找自己日志中是否有 term=3 的条目
- 搜索结果：Leader 日志中 **没有** term=3 的条目
- 因此：`nextIndex = ConflictIndex = 7`

```
nextIndex: 9 → 7  (跳过了 index 8, 9, 10 共 3 个位置)
```

**第二轮 AppendEntries**
- Leader 发送：`prevLogIndex=6, prevLogTerm=5`
- Follower 检查：`log[6].Term = 2 ≠ 5`，term 不匹配！

**Follower 返回冲突信息**

```
ConflictTerm = 2  (Follower 在 index 6 处的 term)
ConflictIndex = 4 (term=2 的第一个日志索引)
```

**Leader 处理**
- 查找自己日志中是否有 term=2 的条目
- 搜索结果：Leader 日志中 **没有** term=2 的条目
- 因此：`nextIndex = ConflictIndex = 4`

```
nextIndex: 7 → 4  (跳过了 index 4, 5, 6 共 3 个位置)
```

**第三轮 AppendEntries**
- Leader 发送：`prevLogIndex=3, prevLogTerm=1`
- Follower 检查：`log[3].Term = 1 == 1`，匹配！
- Follower 截断 index 4+ 的日志，追加 Leader 的日志
- 返回 `Success = true`

**最终结果** ：只用了 3 轮 RPC 就完成了同步。

**对比简单回退策略**

如果使用简单的 `nextIndex--` 策略：
```
nextIndex: 9 → 8 → 7 → 6 → 5 → 4 → 3 (匹配)
```
需要 **7 轮 RPC** 才能找到匹配点！

---

**另一个场景：Follower 日志太短**

```
Leader 日志:   [0] [1] [1] [1] [4] [4] [5] [5] [6]
索引:           0   1   2   3   4   5   6   7   8

Follower 日志: [0] [1] [1]
索引:           0   1   2
```

- Leader 发送：`prevLogIndex=8, prevLogTerm=6`
- Follower 检查：`8 >= len(log)=3`，日志太短！

**Follower 返回**

```
ConflictTerm = -1  (特殊标记：日志太短)
ConflictIndex = 3  (Follower 日志长度)
```

**Leader 处理**
- 看到 `ConflictTerm == -1`
- 直接设置 `nextIndex = ConflictIndex = 3`

```
nextIndex: 9 → 3  (一次跳过 6 个位置！)
```

---

**还有一个场景：Leader 也有 ConflictTerm**

```
Leader 日志:   [0] [1] [1] [2] [2] [2] [4] [4] [5]
索引:           0   1   2   3   4   5   6   7   8

Follower 日志: [0] [1] [1] [2] [2] [3] [3] [3]
索引:           0   1   2   3   4   5   6   7
```

- Leader 发送：`prevLogIndex=7, prevLogTerm=4`
- Follower 检查：`log[7].Term = 3 ≠ 4`，不匹配！

**Follower 返回**
```
ConflictTerm = 3
ConflictIndex = 5 (term=3 的第一个索引)
```

**Leader 处理**
- 查找自己日志中是否有 term=3
- 搜索结果：Leader 日志中 **没有** term=3
- 因此：`nextIndex = ConflictIndex = 5`

下一轮：
- Leader 发送：`prevLogIndex=4, prevLogTerm=2`
- Follower 检查：`log[4].Term = 2 == 2`，匹配！

如果 Leader 也有 ConflictTerm （假设场景）：

```
Leader 日志:   [0] [1] [1] [2] [2] [3] [3] [4] [5]
                                   ↑   ↑
                       term=3 的最后一个在 index 6
```

此时 Leader 发现自己有 term=3 的日志，会设置 `nextIndex = 7`（term=3 最后一个日志的下一个位置），这样可以保留共同的 term=3 日志。

### 遇到的问题与解决

#### 问题 1: 节点重启后日志被覆盖

**现象** ：测试报错 `apply error: commit index=6 server=2 17 != server=4 16`，不同服务器在同一 index 应用了不同的命令。

**原因** ：在 `start()` 函数中调用了 `rf.becomeFollower(0, NoneLeader)`，这会：
1. 将 `currentTerm` 设为 0
2. 调用 `persist()` 将状态写入磁盘

这会覆盖掉刚刚通过 `readPersist()` 恢复的状态！

**问题日志** ：
```
[2] persisted state: term=3, votedFor=-1, logLen=7   # 正常恢复后
[2] persisted state: term=0, votedFor=-1, logLen=1   # 被 becomeFollower(0, -1) 覆盖！
```

**解决方案** ：`start()` 函数不应该重新初始化持久化状态，只需要重置选举超时：

```go
func (rf *Raft) start() {
	// 只初始化选举超时（状态已在 Make 中通过 readPersist 恢复）
	rf.mu.Lock()
	rf.resetElectionTimeout()
	rf.mu.Unlock()

	for !rf.killed() {
		time.Sleep(time.Millisecond * 10)
		rf.tick()
	}
}
```

#### 问题 2: applyLogs 中的并发问题

**原始实现**

```go
func (rf *Raft) applyLogs() {
	for rf.lastApplied < rf.commitIndex {
		rf.lastApplied++
		msg := ApplyMsg{...}
		rf.mu.Unlock()
		rf.applyCh <- msg  // 释放锁后发送
		rf.mu.Lock()
	}
}
```

**问题** ：这种"循环中反复 unlock/lock"的模式存在多个并发隐患。

##### 为什么原有逻辑不对？

**已经 commit 的日志按 Raft 协议不应该被更改，为什么还会有问题？**

从 Raft 协议的角度，已 commit 的日志不会被覆盖。但问题出在 **Go 语言层面的并发安全** 和 **代码逻辑层面** 。

**问题 1：Go slice 的并发不安全**

`rf.log` 是一个 slice，当我们释放锁时：

```go
rf.mu.Unlock()
rf.applyCh <- msg   // <-- 在这期间，其他 goroutine 可能 append 到 rf.log
rf.mu.Lock()
```

如果其他 goroutine 执行了 `rf.log = append(rf.log, newEntry...)`，Go 可能会：
- 分配新的底层数组
- 复制数据到新数组
- 更新 slice header

此时我们持有的 `rf.log` 引用可能指向旧数组，或者在读取时与写入发生 **data race** 。即使数据内容不变，这也是未定义行为。

**问题 2：多 goroutine 同时调用导致乱序**

```
Timeline -->

Goroutine A (from updateCommitIndex)    Goroutine B (from AppendEntries)
|                                       |
+-- applyLogs() called                  +-- applyLogs() called
|   lastApplied=3, commitIndex=5        |   lastApplied=3, commitIndex=5
|                                       |
+-- rf.lastApplied++ (=4)               |
+-- msg = {index=4}                     |
+-- rf.mu.Unlock()                      |
|                                       +-- rf.mu.Lock() (gets lock!)
|   (blocked on channel)                +-- rf.lastApplied++ (=5)
|                                       +-- msg = {index=5}
|                                       +-- rf.mu.Unlock()
|                                       +-- rf.applyCh <- {index=5}  <-- SENT FIRST!
|                                       +-- rf.mu.Lock()
+-- rf.applyCh <- {index=4}  <-- SENT SECOND!
+-- rf.mu.Lock()
|
Result: applyCh receives index=5 before index=4  (OUT OF ORDER!)
```

测试框架期望 apply 顺序是 4, 5，但实际收到的是 5, 4，导致测试失败。

**问题 3：重复 apply 的风险**

```
Timeline -->

Goroutine A                             Goroutine B
|                                       |
+-- lastApplied=3, commitIndex=5        |
+-- rf.lastApplied++ (=4)               |
+-- rf.mu.Unlock()                      |
|                                       +-- rf.mu.Lock()
|   (blocked on channel send)           +-- sees lastApplied=4, commitIndex=5
|                                       +-- rf.lastApplied++ (=5)
|                                       +-- send {index=5}
|                                       +-- rf.mu.Unlock()
+-- rf.mu.Lock()                        |
+-- loop continues: lastApplied(=5) < commitIndex(=5)?
+-- NO, but we only sent index=4, never sent index=5!
```

在某些时序下，可能漏掉某些 index 或者重复发送。

##### 为什么要"不持有锁时发送 ApplyMsg"？

这是一个重要的设计约束：**channel 发送可能阻塞，持有锁时阻塞会导致死锁**。

```go
// DANGEROUS implementation!
func (rf *Raft) applyLogs() {
    rf.mu.Lock()
    defer rf.mu.Unlock()

    for rf.lastApplied < rf.commitIndex {
        rf.lastApplied++
        rf.applyCh <- ApplyMsg{...}  // May block if channel is full!
        // Holding lock while blocked
        // Other goroutines cannot acquire lock
        // But receiver may need to call rf.Start(), which needs lock
        // --> DEADLOCK!
    }
}
```

**死锁场景**

```
Raft goroutine                      Service goroutine (kvserver)
|                                   |
+-- rf.mu.Lock()                    |
+-- rf.applyCh <- msg --blocks----> +-- trying to receive from applyCh
|   (channel may be full)           |   ...
|   waiting for consumer...         +-- received msg, processing
|                                   +-- calls rf.Start() to submit new cmd
|                                   +-- rf.mu.Lock() <-- BLOCKED! waiting for lock
|   ^                               |   ^
|   cannot release lock             |   cannot acquire lock
|   because blocked on channel      |   because Raft holds it
|                                   |
+-------------- DEADLOCK! ----------+
```

##### 正确的解决方案：专门的 applier goroutine

核心思想： **在持有锁时做"快照"，释放锁后使用快照数据** 。

```go
// applyLogs just signals the applier, very lightweight
func (rf *Raft) applyLogs() {
    rf.applyCond.Signal()
}

// applier is a dedicated goroutine for applying logs in order
func (rf *Raft) applier() {
    rf.mu.Lock()
    defer rf.mu.Unlock()

    for !rf.killed() {
        // 1. Wait for new logs to apply
        for rf.lastApplied >= rf.commitIndex {
            rf.applyCond.Wait()  // releases lock while waiting
            if rf.killed() {
                return
            }
        }

        // 2. Snapshot while holding lock (CRITICAL!)
        commitIndex := rf.commitIndex
        lastApplied := rf.lastApplied
        entries := make([]LogEntry, commitIndex-lastApplied)
        copy(entries, rf.log[lastApplied+1:commitIndex+1])  // deep copy!

        // 3. Release lock, then send (avoid deadlock)
        rf.mu.Unlock()
        for i, entry := range entries {
            rf.applyCh <- ApplyMsg{
                CommandValid: true,
                Command:      entry.Command,
                CommandIndex: lastApplied + 1 + i,
            }
        }
        
        // 4. Re-acquire lock, update lastApplied
        rf.mu.Lock()
        // Use the saved commitIndex, not current one
        // because current one may have changed
        if commitIndex > rf.lastApplied {
            rf.lastApplied = commitIndex
        }
    }
}
```

**这个设计解决了所有问题**

1. **数据一致性** ：`copy()` 创建了日志的深拷贝，释放锁后使用副本，不受原 slice 修改影响。

2. **避免死锁** ：发送到 channel 时不持有锁，接收方可以自由调用 Raft 的其他方法。

3. **保证顺序** ：只有一个 applier goroutine，所有 apply 操作串行执行，天然保证顺序。

4. **正确更新 lastApplied** ：使用快照时保存的 `commitIndex` 来更新 `lastApplied`，而不是当前值。

**时序图（正确版本）**

```
Timeline -->

applier goroutine                       Other goroutines
|                                       |
+-- rf.mu.Lock()                        |
+-- commitIndex = 5, lastApplied = 3    |
+-- entries = copy(log[4:6])  <---------+ (consistent snapshot)
+-- rf.mu.Unlock() <--------------------+
|                                       +-- rf.mu.Lock()
|   (not holding lock now)              +-- commitIndex = 10, log modified
|                                       +-- rf.mu.Unlock()
+-- send entries[0] (original log[4])   |
+-- send entries[1] (original log[5])   |
|   ^ using snapshot, unaffected        |
+-- rf.mu.Lock()                        |
+-- rf.lastApplied = 5                  |
|   ^ next iteration handles 6~10       |
```

### 测试结果

```bash
$ go test -run 2C -race
Test (2C): basic persistence ...
  ... Passed --   3.5  3   78   20085    6
Test (2C): more persistence ...
  ... Passed --  19.9  5 1156  236704   16
Test (2C): partitioned leader and one follower crash, leader restarts ...
  ... Passed --   2.1  3   38    9212    4
Test (2C): Figure 8 ...
  ... Passed --  29.4  5  940  187996   38
Test (2C): unreliable agreement ...
  ... Passed --   1.6  5 1044  337410  246
Test (2C): Figure 8 (unreliable) ...
  ... Passed --  43.3  5 10800 32741525   98
Test (2C): churn ...
  ... Passed --  16.3  5 4956 7610147 1053
Test (2C): unreliable churn ...
  ... Passed --  16.4  5 4488 1851187  945
PASS
ok      github.com/PiperLiu/mit6.824-lab-2020/src/raft  176.977s
```

所有 2A、2B、2C 测试稳定通过。
