# 6.824 Lab 3: Fault-tolerant Key/Value Service

目录：

<!-- @import "[TOC]" {cmd="toc" depthFrom=2 depthTo=2 orderedList=false} -->

<!-- code_chunk_output -->

- [架构与要求：线性强一致性](#架构与要求线性强一致性)
- [3A: 基于 raft 的 k/v 服务](#3a-基于-raft-的-kv-服务)
- [3B: 基于日志压缩的快照](#3b-基于日志压缩的快照)

<!-- /code_chunk_output -->

细分目录：

<!-- @import "[TOC]" {cmd="toc" depthFrom=2 depthTo=6 orderedList=false} -->

<!-- code_chunk_output -->

- [架构与要求：线性强一致性](#架构与要求线性强一致性)
- [3A: 基于 raft 的 k/v 服务](#3a-基于-raft-的-kv-服务)
  - [项目结构](#项目结构)
  - [第一步：单 client 实现](#第一步单-client-实现)
    - [核心设计思路](#核心设计思路)
    - [数据结构设计](#数据结构设计)
    - [Client (Clerk) 实现](#client-clerk-实现)
    - [Server RPC Handler 实现](#server-rpc-handler-实现)
    - [applier goroutine 实现](#applier-goroutine-实现)
  - [第二步：处理故障与重复请求](#第二步处理故障与重复请求)
    - [问题分析](#问题分析)
    - [解决方案：请求去重](#解决方案请求去重)
    - [数据结构设计](#数据结构设计-1)
    - [Client 实现](#client-实现)
    - [Server 去重逻辑](#server-去重逻辑)
    - [测试结果](#测试结果)
    - [关键点总结](#关键点总结)
- [3B: 基于日志压缩的快照](#3b-基于日志压缩的快照)
  - [第一步：修改 Raft 支持日志截断](#第一步修改-raft-支持日志截断)
    - [核心设计：逻辑索引 vs 物理索引](#核心设计逻辑索引-vs-物理索引)
    - [数据结构修改](#数据结构修改)
    - [索引转换辅助函数](#索引转换辅助函数)
    - [Snapshot 接口实现](#snapshot-接口实现)
    - [需要修改的关键位置](#需要修改的关键位置)
    - [AppendEntries 的特殊处理](#appendentries-的特殊处理)
    - [关键点总结](#关键点总结-1)
  - [第二步：修改 KVServer 支持快照](#第二步修改-kvserver-支持快照)
    - [核心设计思路](#核心设计思路-1)
    - [数据结构修改](#数据结构修改-1)
    - [takeSnapshot 实现](#takesnapshot-实现)
    - [readSnapshot 实现](#readsnapshot-实现)
    - [修改 applier：触发快照 + 处理 InstallSnapshot](#修改-applier触发快照--处理-installsnapshot)
    - [applySnapshot 实现](#applysnapshot-实现)
  - [第三步：实现 InstallSnapshot RPC](#第三步实现-installsnapshot-rpc)
    - [RPC 参数定义](#rpc-参数定义)
    - [InstallSnapshot RPC Handler](#installsnapshot-rpc-handler)
    - [sendSnapshot 实现](#sendsnapshot-实现)
    - [修改 broadcastAppendEntries ：判断何时发送快照](#修改-broadcastappendentries-判断何时发送快照)
  - [第四步：调试过程——一个隐蔽的 Bug](#第四步调试过程一个隐蔽的-bug)
    - [问题现象](#问题现象)
    - [排查过程](#排查过程)
    - [解决方案](#解决方案)
    - [测试结果](#测试结果-1)
  - [调试经验总结](#调试经验总结)
  - [Lab 3B 关键点总结](#lab-3b-关键点总结)

<!-- /code_chunk_output -->

## 架构与要求：线性强一致性

- If called one at a time, the Get/Put/Append methods should act as if the system had only one copy of its state, and each call should observe the modifications to the state implied by the preceding sequence of calls.
- For concurrent calls, the return values and final state must be the same as if the operations had executed one at a time in some order.
  - Calls are concurrent if they overlap in time, for example if client X calls Clerk.Put(), then client Y calls Clerk.Append(), and then client X's call returns.
  - 如果两个调用在时间上有重叠，那么它们就是并发的，例如：客户端 X 调用 Clerk.Put()，随后客户端 Y 调用 Clerk.Append()，然后客户端 X 的调用才返回——这种情况下 X 和 Y 的调用就是并发的（因为 X 的调用还没结束，Y 就开始了）。
- Furthermore, a call must observe the effects of all calls that have completed before the call starts (so we are technically asking for linearizability).
- Providing strong consistency is relatively easy for a single server. It is harder if the service is replicated, since **all servers must choose the same execution order for concurrent requests, and must avoid replying to clients using state that isn't up to date.**

## 3A: 基于 raft 的 k/v 服务

### 项目结构

在 raft 内部，term 是有意义的，但是在 raft 外部，调用者不应关心 term，或者说见不到 term。

### 第一步：单 client 实现

第一步的目标是：实现一个在没有消息丢失、没有服务器故障时能正常工作的 k/v 服务。

数据流程与难点：
1. Clerk （相当于数据库客户端）提交 `Command` ，相当于其远程调用了 Server 的 Get/Put
2. 在 Server 的 Get/Put 中，首先将 `Command` 作为一条 log 提交给 raft ，接着等待 raft 将其 commit ，在返回 `ok` 给 Clerk
3. 难点在于：如何从 raft 中获取该 `Command` 已 commit 这条信息？更难的是，对于并发提交到的 `Command`s ，如何分别得到响应，互不影响？

#### 核心设计思路

**问题：RPC handler 如何知道自己提交的 Op 已经被 Raft commit？**

Raft 的 `Start()` 方法是非阻塞的，它立即返回，不等待命令被复制到多数节点。因此我们需要一种机制让 RPC handler 等待它的命令被 commit。

**解决方案：index + channel 机制**

1. `Start()` 返回命令 **将被存储** 的日志 index
2. 为每个 index 维护一个等待 channel（`waitChs map[int]chan Op`）
3. RPC handler 调用 `Start()` 后，在对应 index 的 channel 上等待
4. `applier` goroutine 从 `applyCh` 读取已 commit 的命令，执行后通知对应的 channel

```
RPC Handler                          Raft                         applier
    │                                  │                             │
    │──────── Start(Op) ──────────────▶│                             │
    │◀─────── (index, term, true) ─────│                             │
    │                                  │                             │
    │─── getWaitCh(index) ───┐         │  ┌── ApplyMsg ─────────────▶│
    │                        │         │  │                          │
    │◀─── wait on channel ───┘         │  │                          │
    │                                  │  │  ┌─ execute Op ─┐        │
    │                                  │  │  │ data[k]=v    │        │
    │                                  │  │  └──────────────┘        │
    │◀───────────────────── Op ────────│──┘                          │
    │                                  │             notify channel◀─│
    │─── reply to client ───▶          │                             │
```

#### 数据结构设计

**Op 结构体** （描述一个 k/v 操作）：

```go
type Op struct {
    Type  string // "Get", "Put", "Append"
    Key   string
    Value string
}
```

**KVServer 新增字段** ：

```go
type KVServer struct {
    // ... 原有字段 ...
    data    map[string]string // k/v 存储
    waitChs map[int]chan Op   // index -> 等待通道
}
```

#### Client (Clerk) 实现

Clerk 的核心逻辑很简单：不断尝试向不同 server 发送 RPC，直到成功：

```go
func (ck *Clerk) Get(key string) string {
    args := GetArgs{Key: key}
    for {
        reply := GetReply{}
        ok := ck.servers[ck.leaderId].Call("KVServer.Get", &args, &reply)
        if ok && (reply.Err == OK || reply.Err == ErrNoKey) {
            return reply.Value
        }
        // 换一个 server 尝试
        ck.leaderId = (ck.leaderId + 1) % len(ck.servers)
    }
}
```

**优化点** ：缓存上一次找到的 leader（`leaderId`），避免每次都从 0 开始搜索。

#### Server RPC Handler 实现

以 `Get` 为例：

```go
func (kv *KVServer) Get(args *GetArgs, reply *GetReply) {
    op := Op{Type: "Get", Key: args.Key}
    
    // 1. 提交到 Raft
    index, _, isLeader := kv.rf.Start(op)
    if !isLeader {
        reply.Err = ErrWrongLeader
        return
    }
    
    // 2. 等待 Raft commit（带超时）
    ch := kv.getWaitCh(index)
    select {
    case appliedOp := <-ch:
        // 3. 检查是否是我们提交的操作
        if appliedOp.Type == op.Type && appliedOp.Key == op.Key {
            value, ok := kv.data[args.Key]
            // ... 设置 reply
        } else {
            reply.Err = ErrWrongLeader
        }
    case <-time.After(500 * time.Millisecond):
        reply.Err = ErrWrongLeader
    }
    
    // 4. 清理
    delete(kv.waitChs, index)
}
```

**为什么要检查 `appliedOp` 是否和我们提交的 `op` 一致？**

因为存在 leader 切换的情况：
1. 旧 leader 调用 `Start()`，返回 index=5
2. 旧 leader 失去领导权（但它不知道）
3. 新 leader 在 index=5 位置写入了不同的命令
4. 旧 leader 的 `applier` 收到 index=5 的 ApplyMsg，但内容不是它提交的

此时应该返回 `ErrWrongLeader`，让 client 重试。

#### applier goroutine 实现

持续从 `applyCh` 读取已 commit 的命令并执行：

```go
func (kv *KVServer) applier() {
    for !kv.killed() {
        msg := <-kv.applyCh
        if msg.CommandValid {
            op := msg.Command.(Op)
            index := msg.CommandIndex

            // 执行命令
            switch op.Type {
            case "Put":
                kv.data[op.Key] = op.Value
            case "Append":
                kv.data[op.Key] += op.Value
            case "Get":
                // Get 不修改状态
            }

            // 通知等待的 RPC handler（非阻塞发送）
            if ch, ok := kv.waitChs[index]; ok {
                select {
                case ch <- op:
                default:
                }
            }
        }
    }
}
```

**关键点**：
- **所有命令都要执行** ，包括 Get（虽然 Get 不修改状态，但 Raft 要求按序执行）
- **只在 applier 中修改 `data`** ，保证线性一致性
- **非阻塞发送** ：使用 `select + default` 避免死锁

**Q: `select { case ch <- op: default: }` 和 `ch <- op` 有什么区别？为什么要用非阻塞发送？**

| 写法 | 行为 |
|------|------|
| `ch <- op` | **阻塞发送**：如果 channel 满了（buffer=1 且已有数据），发送方会阻塞等待直到有接收者 |
| `select { case ch <- op: default: }` | **非阻塞发送**：如果 channel 满了，立即走 default 分支，不阻塞 |

**Q: 这是在防什么死锁？**

考虑以下时序：

```
时间线    RPC Handler                    applier
  │
  │      ch := getWaitCh(index)
  │      select {
  │        case <-ch:            ← 等待中
  │        case <-time.After(500ms):
  │      }
  │           │
  │           │ (500ms 超时!)
  │           ▼
  │      reply.Err = ErrWrongLeader
  │      return                  ← handler 已返回，不再有人接收 ch
  │
  │                                      msg := <-applyCh
  │                                      ch <- op  ← 如果是阻塞发送，永远卡在这里！
  │                                      // 后续所有 ApplyMsg 都无法处理
  │                                      // 整个 KVServer 死锁
```

**场景说明**
1. RPC handler 调用 `Start()` 后，创建 channel 并等待（带 500ms 超时）
2. 由于网络延迟/Raft 复制慢等原因，500ms 内没有收到 ApplyMsg
3. handler 超时返回， **但 channel 还在 `waitChs` 中**
4. 之后 applier 收到这个 index 的 ApplyMsg，尝试发送到 channel
5. 如果使用阻塞发送 `ch <- op`：
   - channel buffer=1，没有接收者
   - applier 永远阻塞在这一行
   - 后续所有 `applyCh` 的消息都无法处理
   - **整个 KVServer 死锁！**

使用 `select + default` 非阻塞发送：即使没有接收者，也不会阻塞，applier 可以继续处理下一个 ApplyMsg。

### 第二步：处理故障与重复请求

#### 问题分析

在有故障的场景下，基础实现会出现问题：

**场景：重复执行**

```
Client                   Leader (KVServer A)              Raft
  │                            │                           │
  │── Put(k, v1) ─────────────▶│                           │
  │                            │── Start(Op) ─────────────▶│
  │                            │                           │ (复制到多数节点)
  │                            │◀── ApplyMsg ──────────────│
  │                            │ (exec Put, data[k]=v1)    │
  │                            │                           │
  │     (网络超时或 Leader 崩溃，client 没收到回复)             │
  │                            ╳                           │
  │                                                        │
  │── Put(k, v1) ─────────────────────────▶ (new Leader B) │
  │                                            │           │
  │                                            │ (再次执行) │
```

如果操作是 `Append`，重复执行会导致数据错误！

#### 解决方案：请求去重

**核心思路**
1. 每个 client 有唯一的 `clientId`（使用随机数生成）
2. 每个请求有单调递增的 `seqNum`（序列号）
3. server 记录每个 client 最后执行的 `seqNum`
4. 如果收到 `seqNum <= lastSeqNum` 的请求，说明是重复请求，直接返回缓存的结果

**为什么这个方案可行？**

Lab 3 的假设（hints 里提到）：
> It's OK to assume that a client will make only one call into a Clerk at a time.

即每个 client 同一时间只有一个请求在进行。这意味着：
- 如果 `seqNum > lastSeqNum + 1`：不可能发生（client 是串行的）
- 如果 `seqNum == lastSeqNum`：重复请求，返回缓存结果
- 如果 `seqNum < lastSeqNum`：更旧的重复请求，也返回缓存结果

#### 数据结构设计

**RPC 参数扩展**

```go
type PutAppendArgs struct {
    Key      string
    Value    string
    Op       string
    ClientId int64  // 新增：client 唯一标识
    SeqNum   int64  // 新增：请求序列号
}

type GetArgs struct {
    Key      string
    ClientId int64
    SeqNum   int64
}
```

**Op 结构体扩展**

```go
type Op struct {
    Type     string
    Key      string
    Value    string
    ClientId int64  // 新增
    SeqNum   int64  // 新增
}
```

**Server 新增字段**

```go
// LastReply 记录每个 client 最后一次请求的结果
type LastReply struct {
    SeqNum int64
    Value  string // 仅 Get 需要
    Err    Err
}

type KVServer struct {
    // ...
    lastReplies map[int64]LastReply // clientId -> 最后执行的请求结果
}
```

#### Client 实现

```go
type Clerk struct {
    servers  []*labrpc.ClientEnd
    leaderId int
    clientId int64  // 唯一标识
    seqNum   int64  // 单调递增
}

func MakeClerk(servers []*labrpc.ClientEnd) *Clerk {
    ck := new(Clerk)
    ck.servers = servers
    ck.leaderId = 0
    ck.clientId = nrand()  // 随机生成唯一 ID
    ck.seqNum = 0
    return ck
}

func (ck *Clerk) PutAppend(key string, value string, op string) {
    ck.seqNum++  // 每个请求分配新的序列号
    args := PutAppendArgs{
        Key:      key,
        Value:    value,
        Op:       op,
        ClientId: ck.clientId,
        SeqNum:   ck.seqNum,
    }
    for {
        reply := PutAppendReply{}
        ok := ck.servers[ck.leaderId].Call("KVServer.PutAppend", &args, &reply)
        if ok && reply.Err == OK {
            return
        }
        ck.leaderId = (ck.leaderId + 1) % len(ck.servers)
    }
}
```

**关键点** ：`seqNum++` 在循环外面，重试时使用相同的 `seqNum`。

#### Server 去重逻辑

**RPC Handler 入口检查**

```go
func (kv *KVServer) PutAppend(args *PutAppendArgs, reply *PutAppendReply) {
    kv.mu.Lock()
    // 检查是否是重复请求
    if kv.isDuplicate(args.ClientId, args.SeqNum) {
        reply.Err = kv.lastReplies[args.ClientId].Err
        kv.mu.Unlock()
        return  // 直接返回缓存结果，不提交到 Raft
    }
    kv.mu.Unlock()
    
    // ... 正常流程：提交到 Raft，等待 commit
}

func (kv *KVServer) isDuplicate(clientId int64, seqNum int64) bool {
    lastReply, ok := kv.lastReplies[clientId]
    if !ok {
        return false
    }
    return seqNum <= lastReply.SeqNum
}
```

**applier 中的去重**

```go
func (kv *KVServer) applier() {
    for !kv.killed() {
        msg := <-kv.applyCh
        if msg.CommandValid {
            op := msg.Command.(Op)
            index := msg.CommandIndex

            kv.mu.Lock()
            // 去重：只执行未执行过的命令
            if !kv.isDuplicate(op.ClientId, op.SeqNum) {
                switch op.Type {
                case "Put":
                    kv.data[op.Key] = op.Value
                case "Append":
                    kv.data[op.Key] += op.Value
                }
                // 记录执行结果（用于去重和返回缓存结果）
                kv.lastReplies[op.ClientId] = LastReply{
                    SeqNum: op.SeqNum,
                    Value:  kv.data[op.Key],
                    Err:    OK,
                }
            }
            // ... 通知 RPC handler
        }
    }
}
```

**为什么要在 applier 里也做去重？**

考虑以下场景：
1. Client 发送请求到 Leader A，A 调用 `Start()` 将 Op 写入 Raft log
2. Op 被复制到多数节点并 commit
3. Leader A 崩溃，Client 没收到回复
4. Client 重试，发送到新 Leader B
5. B 的 RPC handler 检查去重：此时 B 还没 apply 这个 Op，所以认为不是重复
6. B 再次调用 `Start()` 将同一个 Op 写入 Raft log
7. 两个相同的 Op 都会通过 `applyCh` 到达 applier

如果 applier 不做去重，就会执行两次！

#### 测试结果

```bash
$ go test -run 3A -race
Test: one client (3A) ...
  ... Passed --  15.1  5  6031  991
Test: many clients (3A) ...
  ... Passed --  15.6  5  7937 1179
Test: unreliable net, many clients (3A) ...
  ... Passed --  16.6  5  7358 1074
Test: concurrent append to same key, unreliable (3A) ...
  ... Passed --   0.9  3   233   52
Test: progress in majority (3A) ...
  ... Passed --   0.7  5    78    2
Test: no progress in minority (3A) ...
  ... Passed --   1.0  5   126    3
Test: completion after heal (3A) ...
  ... Passed --   1.0  5    59    3
Test: partitions, one client (3A) ...
  ... Passed --  22.9  5  8325  968
Test: partitions, many clients (3A) ...
  ... Passed --  23.3  5 25114 1109
Test: restarts, one client (3A) ...
  ... Passed --  19.2  5  7917 1003
Test: restarts, many clients (3A) ...
  ... Passed --  20.1  5 13331 1203
Test: unreliable net, restarts, many clients (3A) ...
  ... Passed --  20.5  5  7944 1077
Test: restarts, partitions, many clients (3A) ...
  ... Passed --  27.4  5 14785 1216
Test: unreliable net, restarts, partitions, many clients (3A) ...
  ... Passed --  28.1  5  8360  980
Test: unreliable net, restarts, partitions, many clients, linearizability checks (3A) ...
  ... Passed --  25.7  7 17047  872
PASS
ok      kvraft  239.615s
```

#### 关键点总结

| 问题 | 解决方案 |
|------|---------|
| 如何唯一标识请求？ | `(clientId, seqNum)` 组合 |
| 在哪里做去重检查？ | RPC handler 入口 + applier |
| 为什么两处都要检查？ | RPC handler 检查可能漏掉（leader 切换），applier 是最后防线 |
| 如何处理 Get 的去重？ | 同样去重，缓存返回值 |
| 内存如何释放？ | 每个 client 只保留最后一个请求的结果（因为题目特殊假设 a client will make only one call into a Clerk at a time ） |

## 3B: 基于日志压缩的快照

### 第一步：修改 Raft 支持日志截断

#### 核心设计：逻辑索引 vs 物理索引

截断日志后，日志数组的下标（物理索引）和 Raft 论文中的 index（逻辑索引）就不再相等了。

**示例**

```
截断前：
  物理索引:  0    1    2    3    4    5
  逻辑索引:  0    1    2    3    4    5
  log:    [哨兵][e1] [e2] [e3] [e4] [e5]

在 index=3 处创建快照并截断后：
  物理索引:  0    1    2
  逻辑索引:  3    4    5
  log:    [哨兵][e4] [e5]
  
  lastIncludedIndex = 3
  lastIncludedTerm = e3.Term (存储在 log[0].Term)
```

**索引转换公式**

```
物理索引 = 逻辑索引 - lastIncludedIndex
逻辑索引 = 物理索引 + lastIncludedIndex
```

#### 数据结构修改

**Raft 结构体新增字段**

```go
type Raft struct {
    // ... 原有字段 ...
    
    // 3B: 快照相关的持久化状态
    // log[0].Term 存储 lastIncludedTerm
    lastIncludedIndex int // 快照包含的最后一条日志的逻辑索引
}
```

#### 索引转换辅助函数

```go
// toPhysicalIndex 将逻辑索引转换为物理索引
func (rf *Raft) toPhysicalIndex(logicalIndex int) int {
    return logicalIndex - rf.lastIncludedIndex
}

// toLogicalIndex 将物理索引转换为逻辑索引
func (rf *Raft) toLogicalIndex(physicalIndex int) int {
    return physicalIndex + rf.lastIncludedIndex
}

// getLogTerm 根据逻辑索引获取日志 term
func (rf *Raft) getLogTerm(logicalIndex int) int {
    physicalIndex := rf.toPhysicalIndex(logicalIndex)
    if physicalIndex < 0 {
        return -1 // 日志已被截断
    }
    if physicalIndex >= len(rf.log) {
        return -1 // 越界
    }
    return rf.log[physicalIndex].Term
}

// getLogEntry 根据逻辑索引获取日志条目
func (rf *Raft) getLogEntry(logicalIndex int) LogEntry {
    return rf.log[rf.toPhysicalIndex(logicalIndex)]
}

// getLogSlice 获取从 startLogicalIndex 开始的日志切片
func (rf *Raft) getLogSlice(startLogicalIndex int) []LogEntry {
    startPhysical := rf.toPhysicalIndex(startLogicalIndex)
    if startPhysical < 1 {
        startPhysical = 1 // log[0] 是哨兵
    }
    if startPhysical >= len(rf.log) {
        return []LogEntry{}
    }
    entries := make([]LogEntry, len(rf.log)-startPhysical)
    copy(entries, rf.log[startPhysical:])
    return entries
}
```

#### Snapshot 接口实现

```go
// Snapshot 由 kvserver 调用，用于创建快照并截断日志
func (rf *Raft) Snapshot(index int, snapshot []byte) {
    rf.mu.Lock()
    defer rf.mu.Unlock()

    // 检查 index 是否有效
    if index <= rf.lastIncludedIndex {
        return // 已经有更新的快照
    }
    if index > rf.getLastLogIndex() {
        return // index 超出日志范围
    }

    // 截断日志
    physicalIndex := rf.toPhysicalIndex(index)
    lastIncludedTerm := rf.log[physicalIndex].Term

    // 创建新日志数组，log[0] 存储 lastIncludedTerm
    newLog := make([]LogEntry, len(rf.log)-physicalIndex)
    copy(newLog, rf.log[physicalIndex:])
    newLog[0].Command = nil // 哨兵不需要 command

    rf.log = newLog
    rf.lastIncludedIndex = index

    // 持久化 Raft 状态和快照
    rf.persistWithSnapshot(snapshot)
}
```

#### 需要修改的关键位置

所有访问日志的地方都需要使用逻辑索引：

| 函数 | 修改点 |
|------|--------|
| `Start()` | 返回的 index 是逻辑索引 |
| `getLastLogIndex()` | 返回 `lastIncludedIndex + len(log) - 1` |
| `broadcastAppendEntries()` | prevLogIndex, entries 切片都用逻辑索引 |
| `updateCommitIndex()` | 遍历时使用逻辑索引 |
| `AppendEntries()` | 检查 prevLogIndex，处理日志截断边界 |
| `applier()` | 处理 lastApplied < lastIncludedIndex 的情况 |
| `persist()/readPersist()` | 持久化/恢复 lastIncludedIndex |

#### AppendEntries 的特殊处理

当 `prevLogIndex < lastIncludedIndex` 时，说明 Leader 发送的日志有一部分已经在 Follower 的快照中了：

```go
if args.PrevLogIndex < rf.lastIncludedIndex {
    // 计算需要跳过的条目数
    skipCount := rf.lastIncludedIndex - args.PrevLogIndex
    if skipCount >= len(args.Entries) {
        // 所有 entries 都在快照中
        reply.Success = true
        return
    }
    // 跳过已在快照中的 entries
    args.Entries = args.Entries[skipCount:]
    args.PrevLogIndex = rf.lastIncludedIndex
    args.PrevLogTerm = rf.log[0].Term
}
```

#### 关键点总结

| 问题 | 解决方案 |
|------|---------|
| 物理索引 vs 逻辑索引 | 使用 `lastIncludedIndex` 做转换 |
| lastIncludedTerm 存储 | 复用 `log[0].Term` |
| 边界条件处理 | `prevLogIndex < lastIncludedIndex` 时跳过已快照的日志 |
| applier 处理 | 确保 `lastApplied >= lastIncludedIndex` |
| 持久化 | 使用 `SaveStateAndSnapshot` 原子保存 |

### 第二步：修改 KVServer 支持快照

#### 核心设计思路

KVServer 需要：
1. **检测 Raft 状态 size 过大** ：当 `persister.RaftStateSize() >= maxraftstate` 时触发快照
2. **创建** ：将 `data` 和 `lastReplies` 编码并调用 `rf.Snapshot()`
3. **恢复快照** ：启动时从 persister 读取并恢复状态
4. **处理 InstallSnapshot** ：当收到 Leader 发来的快照时，应用到状态机

#### 数据结构修改

**KVServer 新增字段**

```go
type KVServer struct {
    // ... 原有字段 ...
    
    maxraftstate int           // 触发快照的阈值，-1 表示不使用快照
    persister    *raft.Persister // 用于读取快照
}
```

#### takeSnapshot 实现

```go
func (kv *KVServer) takeSnapshot(index int) {
    w := new(bytes.Buffer)
    e := labgob.NewEncoder(w)
    e.Encode(kv.data)
    e.Encode(kv.lastReplies)
    e.Encode(index)
    snapshot := w.Bytes()
    kv.rf.Snapshot(index, snapshot)
}
```

**快照内容**：
- `data`：键值存储
- `lastReplies`：去重信息（每个 client 最后执行的请求）
- `index`：快照对应的日志索引（用于验证）

#### readSnapshot 实现

```go
func (kv *KVServer) readSnapshot() {
    snapshot := kv.persister.ReadSnapshot()
    if snapshot == nil || len(snapshot) < 1 {
        return
    }

    r := bytes.NewBuffer(snapshot)
    d := labgob.NewDecoder(r)

    var data map[string]string
    var lastReplies map[int64]LastReply
    var snapshotIndex int

    if d.Decode(&data) != nil ||
        d.Decode(&lastReplies) != nil ||
        d.Decode(&snapshotIndex) != nil {
        log.Fatalf("Failed to decode snapshot")
    }

    kv.data = data
    kv.lastReplies = lastReplies
}
```

#### 修改 applier：触发快照 + 处理 InstallSnapshot

```go
func (kv *KVServer) applier() {
    for !kv.killed() {
        msg := <-kv.applyCh
        
        if msg.SnapshotValid {
            // 处理 InstallSnapshot RPC 的快照
            kv.applySnapshot(msg.Snapshot, msg.SnapshotIndex)
        } else if msg.CommandValid {
            op := msg.Command.(Op)
            index := msg.CommandIndex

            kv.mu.Lock()
            // 去重：只执行未执行过
            if !kv.isDuplicate(op.ClientId, op.SeqNum) {
                switch op.Type {
                case "Put":
                    kv.data[op.Key] = op.Value
                case "Append":
                    kv.data[op.Key] += op.Value
                }
                kv.lastReplies[op.ClientId] = LastReply{
                    SeqNum: op.SeqNum,
                    Value:  kv.data[op.Key],
                    Err:    OK,
                }
            }

            // 检查是否需要创建快照
            if kv.maxraftstate != -1 && kv.rf.GetRaftStateSize() >= kv.maxraftstate {
                kv.takeSnapshot(index)
            }

            // 通知等待的 RPC handler
            if ch, ok := kv.waitChs[index]; ok {
                select {
                case ch <- op:
                default:
                }
            }
            kv.mu.Unlock()
        }
    }
}
```

#### applySnapshot 实现

```go
func (kv *KVServer) applySnapshot(snapshot []byte, snapshotIndex int) {
    kv.mu.Lock()
    defer kv.mu.Unlock()

    if snapshot == nil || len(snapshot) < 1 {
        return
    }

    r := bytes.NewBuffer(snapshot)
    d := labgob.NewDecoder(r)

    var data map[string]string
    var lastReplies map[int64]LastReply
    var index int

    if d.Decode(&data) != nil ||
        d.Decode(&lastReplies) != nil ||
        d.Decode(&index) != nil {
        log.Printf("Failed to decode snapshot in applySnapshot")
        return
    }

    kv.data = data
    kv.lastReplies = lastReplies
}
```

### 第三步：实现 InstallSnapshot RPC

#### RPC 参数定义

```go
type InstallSnapshotArgs struct {
    Term              int
    LeaderId          int
    LastIncludedIndex int
    LastIncludedTerm  int
    Data              []byte // 快照数据
}

type InstallSnapshotReply struct {
    Term int
}
```

**简化设计** ：Lab 3B 不要求分块传输，一次发送整个快照。

#### InstallSnapshot RPC Handler

```go
func (rf *Raft) InstallSnapshot(args *InstallSnapshotArgs, reply *InstallSnapshotReply) {
    rf.mu.Lock()
    reply.Term = rf.currentTerm

    // 旧 term 的请求，拒绝
    if args.Term < rf.currentTerm {
        rf.mu.Unlock()
        return
    }

    // 更新 term
    if args.Term > rf.currentTerm {
        rf.currentTerm = args.Term
        rf.votedFor = -1
        rf.state = Follower
        rf.persist()
    }

    rf.resetElectionTimer()

    // 快照太旧，忽略
    if args.LastIncludedIndex <= rf.lastIncludedIndex {
        rf.mu.Unlock()
        return
    }

    // 截断日志
    if args.LastIncludedIndex < rf.getLastLogIndex() {
        // 保留
        physicalIndex := rf.toPhysicalIndex(args.LastIncludedIndex)
        newLog := make([]LogEntry, len(rf.log)-physicalIndex)
        copy(newLog, rf.log[physicalIndex:])
        newLog[0].Term = args.LastIncludedTerm
        newLog[0].Command = nil
        rf.log = newLog
    } else {
        // 快照包含所有日志，创建新的哨兵
        rf.log = []LogEntry{{Term: args.LastIncludedTerm}}
    }

    rf.lastIncludedIndex = args.LastIncludedIndex

    // commitIndex 和 lastApplied
    if rf.commitIndex < args.LastIncludedIndex {
        rf.commitIndex = args.LastIncludedIndex
    }
    if rf.lastApplied < args.LastIncludedIndex {
        rf.lastApplied = args.LastIncludedIndex
    }

    rf.persistWithSnapshot(args.Data)
    rf.mu.Unlock()

    // 通知状态机应用快照
    rf.applyCh <- ApplyMsg{
        SnapshotValid: true,
        Snapshot:      args.Data,
        SnapshotTerm:  args.LastIncludedTerm,
        SnapshotIndex: args.LastIncludedIndex,
    }
}
```

**关键点** ：
1. 先解锁再发送 ApplyMsg，避免死锁
2. 更新 `commitIndex` 和 `lastApplied`，避免重放旧日志

#### sendSnapshot 实现

```go
func (rf *Raft) sendSnapshot(server int) {
    rf.mu.Lock()
    if rf.state != Leader {
        rf.mu.Unlock()
        return
    }

    args := InstallSnapshotArgs{
        Term:              rf.currentTerm,
        LeaderId:          rf.me,
        LastIncludedIndex: rf.lastIncludedIndex,
        LastIncludedTerm:  rf.log[0].Term,
        Data:              rf.persister.ReadSnapshot(),
    }
    rf.mu.Unlock()

    reply := InstallSnapshotReply{}
    ok := rf.sendInstallSnapshot(server, &args, &reply)

    if !ok {
        return
    }

    rf.mu.Lock()
    defer rf.mu.Unlock()

    if rf.state != Leader || rf.currentTerm != args.Term {
        return
    }

    if reply.Term > rf.currentTerm {
        rf.currentTerm = reply.Term
        rf.votedFor = -1
        rf.state = Follower
        rf.persist()
        return
    }

    // 更新 nextIndex 和 matchIndex
    rf.nextIndex[server] = args.LastIncludedIndex + 1
    rf.matchIndex[server] = args.LastIncludedIndex
}
```

#### 修改 broadcastAppendEntries ：判断何时发送快照

```go
func (rf *Raft) broadcastAppendEntries() {
    rf.mu.Lock()
    if rf.state != Leader {
        rf.mu.Unlock()
        return
    }
    rf.mu.Unlock()

    for i := range rf.peers {
        if i == rf.me {
            continue
        }

        rf.mu.Lock()
        // 判断是否需要发送快照
        if rf.lastIncludedIndex > 0 && rf.nextIndex[i] <= rf.lastIncludedIndex {
            rf.mu.Unlock()
            go rf.sendSnapshot(i)
            continue
        }
        rf.mu.Unlock()

        // 正常发送 AppendEntries
        go rf.sendAppendEntries(i)
    }
}
```

**触发条件** ：`nextIndex[i] <= lastIncludedIndex`，意味着 follower 需要的日志已经被截断。

### 第四步：调试过程——一个隐蔽的 Bug

#### 问题现象

实现 InstallSnapshot RPC 后，运行测试超时：

```bash
$ go test -run 3B -race
Test: InstallSnapshot RPC (3B) ...
test timed out after 10m0s
```

#### 排查过程

**1. 开启调试日志**

在 `util.go` 中设置 `Debug = 1`，重新运行测试。

**2. 分析日志**

发现以下模式反复出现：

```
[S0] [Term 6] I'm Leader, broadcasting AppendEntries
[S0] AppendEntries to 2: prevLogIndex=50, entries=0
[S0] AppendEntries reply from 2: success=false, term=6
[S0] fast backup for 2: nextIndex=51
[S0] AppendEntries to 2: prevLogIndex=50, entries=0
[S0] AppendEntries reply from 2: success=false, term=6
[S0] fast backup for 2: nextIndex=51
... (无限循环)
```

关键信息：
- Leader 的 `lastIncludedIndex = 50`
- Follower 的 `nextIndex` 一直是 51
- 但 `nextIndex[i] <= lastIncludedIndex` 的条件是 `51 <= 50`，为 false！
- 所以永远不会触发 InstallSnapshot

**3. 定位 Bug**

检查 fast backup 逻辑：

```go
// AppendEntries 失败后的处理
if !reply.Success {
    // ... fast backup 计算新的 nextIndex ...

    // BUG: 这行代码阻止了 nextIndex < lastIncludedIndex
    if rf.nextIndex[server] <= rf.lastIncludedIndex {
        rf.nextIndex[server] = rf.lastIncludedIndex + 1
    }
}
```

**问题分析**

原来的想法是： nextIndex 不能小于 lastIncludedIndex（因为那些日志已被截断）。

但这导致：当 fast backup 计算出 `nextIndex < lastIncludedIndex` 时，强制设为 `lastIncludedIndex + 1`，然后下一次心跳检查 `nextIndex <= lastIncludedIndex` 为 false，不发送快照，而是发送 AppendEntries，又失败，又回到 `lastIncludedIndex + 1`，无限循环！

#### 解决方案

**修复代码**

```go
// 不要强制 nextIndex >= lastIncludedIndex + 1
// 只需要确保 nextIndex >= 1
if rf.nextIndex[server] < 1 {
    rf.nextIndex[server] = 1
}
```

**修复后的流程**

```
1. AppendEntries 失败，fast backup 计算 nextIndex = 30
2. nextIndex (30) <= lastIncludedIndex (50)，条件满足
3. 发送 InstallSnapshot
4. Follower 安装快照成功
5. 更新 nextIndex = lastIncludedIndex + 1 = 51
6. 后续正常发送 AppendEntries
```

#### 测试结果

修复后所有测试通过：

```bash
$ go test -run 3B -race -count=1
Test: InstallSnapshot RPC (3B) ... Passed --   2.9  3   788   63
Test: snapshot size is reasonable (3B) ... Passed --   2.2  3  2836  800
Test: restarts, snapshots, one client (3B) ... Passed --  19.8  5 30352 5544
Test: restarts, snapshots, many clients (3B) ... Passed --  19.5  5 36954 5865
Test: unreliable net, snapshots, many clients (3B) ... Passed --  15.9  5 10686 1631
Test: unreliable net, restarts, snapshots, many clients (3B) ... Passed --  20.9  5 12166 1741
Test: unreliable net, restarts, partitions, snapshots, many clients (3B) ... Passed --  28.1  5  9462  978
Test: unreliable net, restarts, partitions, snapshots, many clients, linearizability checks (3B) ... Passed --  25.7  7 23647 1718
PASS
ok      kvraft  134.405s
```

全部 Lab 3 测试：

```bash
$ go test -race -count=1
... (15 个 3A 测试 PASS) ...
... (8 个 3B 测试 PASS) ...
PASS
ok      kvraft  376.250s
```

### 调试经验总结

| 经验 | 说明 |
|------|------|
| **善用调试日志** | 关键路径打印状态，快速定位问题 |
| **关注边界条件** | `<=` vs `<` 这类差异往往是 bug 来源 |
| **理解条件触发** | 确保触发条件能在需要时满足 |
| **分析死循环** | 打印日志观察状态是否在循环中稳定 |

### Lab 3B 关键点总结

| 组件 | 关键实现 |
|------|---------|
| **Raft 日志截断** | 使用 `lastIncludedIndex` 转换逻辑/物理索引 |
| **快照内容** | 包含 `data`、`lastReplies`（去重信息） |
| **触发时机** | `RaftStateSize() >= maxraftstate` |
| **InstallSnapshot** | 截断日志、更新 commitIndex/lastApplied、通知状态机 |
| **ApplyMsg 扩展** | 新增 `SnapshotValid`、`Snapshot`、`SnapshotTerm`、`SnapshotIndex` |
| **隐蔽 Bug** | fast backup 不能强制 `nextIndex >= lastIncludedIndex + 1` |
