# 6.824 Lab 4: Sharded Key/Value Service

目录：

<!-- @import "[TOC]" {cmd="toc" depthFrom=2 depthTo=2 orderedList=false} -->

<!-- code_chunk_output -->

- [架构与要求：分片 K/V 服务](#架构与要求分片-kv-服务)
- [4A: Shard Master 实现](#4a-shard-master-实现)
- [4B: ShardKV 实现](#4b-shardkv-实现)
- [Challenge 1: Garbage Collection（垃圾回收）](#challenge-1-garbage-collection垃圾回收)
- [Challenge 2: 配置变更期间的无中断服务](#challenge-2-配置变更期间的无中断服务)
- [Lab 4 设计要点总结](#lab-4-设计要点总结)
- [综合总结：MIT 6.824 Raft 实现要点](#综合总结mit-6824-raft-实现要点)

<!-- /code_chunk_output -->

细分目录：

<!-- @import "[TOC]" {cmd="toc" depthFrom=2 depthTo=6 orderedList=false} -->

<!-- code_chunk_output -->

- [架构与要求：分片 K/V 服务](#架构与要求分片-kv-服务)
  - [为什么需要分片？](#为什么需要分片)
  - [关键概念： shard, replica group, configuration, shard master](#关键概念-shard-replica-group-configuration-shard-master)
  - [Raft 在哪里起作用？](#raft-在哪里起作用)
  - [Shard Master 的 RPC 接口](#shard-master-的-rpc-接口)
  - [客户端请求流程](#客户端请求流程)
  - [核心挑战：Reconfiguration（配置变更）](#核心挑战reconfiguration配置变更)
  - [理解测试代码：测试场景示例](#理解测试代码测试场景示例)
  - [Lab 4 的两个 Part](#lab-4-的两个-part)
  - [Challenge 挑战（可选）](#challenge-挑战可选)
  - [关键实现提示](#关键实现提示)
- [4A: Shard Master 实现](#4a-shard-master-实现)
  - [整体思路](#整体思路)
  - [数据结构设计](#数据结构设计)
    - [RPC 参数扩展](#rpc-参数扩展)
    - [Op 结构体](#op-结构体)
    - [ShardMaster 结构体](#shardmaster-结构体)
  - [Client 实现](#client-实现)
  - [Server RPC Handler 实现](#server-rpc-handler-实现)
  - [applier 实现](#applier-实现)
  - [核心：负载均衡算法](#核心负载均衡算法)
    - [算法思路](#算法思路)
    - [关键：确定性](#关键确定性)
    - [示例](#示例)
  - [处理 Join/Leave/Move](#处理-joinleavemove)
    - [Join](#join)
    - [Leave](#leave)
    - [Move](#move)
  - [深拷贝 Config](#深拷贝-config)
  - [测试结果](#测试结果)
- [4B: ShardKV 实现](#4b-shardkv-实现)
  - [整体思路](#整体思路-1)
  - [数据结构设计](#数据结构设计-1)
    - [RPC 结构体](#rpc-结构体)
    - [Shard 状态机](#shard-状态机)
    - [Op 结构体](#op-结构体-1)
    - [ShardKV 结构体](#shardkv-结构体)
  - [Client 实现](#client-实现-1)
  - [Server 核心实现](#server-核心实现)
    - [canServe：判断是否可以服务某个 shard](#canserve判断是否可以服务某个-shard)
    - [Get/PutAppend RPC Handler](#getputappend-rpc-handler)
    - [Migrate RPC Handler](#migrate-rpc-handler)
  - [applier 实现](#applier-实现-1)
    - [applyClientOp ：应用客户端操作](#applyclientop-应用客户端操作)
    - [applyConfig ：应用配置变更](#applyconfig-应用配置变更)
    - [applyMigrate ：应用 shard 迁移](#applymigrate-应用-shard-迁移)
  - [后台 Goroutine](#后台-goroutine)
    - [configPoller：定期获取配置](#configpoller定期获取配置)
    - [migrator：执行 shard 迁移](#migrator执行-shard-迁移)
  - [Snapshot 支持](#snapshot-支持)
  - [配置变更流程图](#配置变更流程图)
  - [关键设计决策](#关键设计决策)
  - [测试结果](#测试结果-1)
- [Challenge 1: Garbage Collection（垃圾回收）](#challenge-1-garbage-collection垃圾回收)
  - [问题背景](#问题背景)
  - [核心挑战](#核心挑战)
  - [设计方案](#设计方案)
  - [数据结构设计](#数据结构设计-2)
    - [新增操作类型](#新增操作类型)
    - [新增 RPC](#新增-rpc)
    - [ShardKV 扩展字段](#shardkv-扩展字段)
  - [核心实现](#核心实现)
    - [1. 配置变更时记录待 GC 的 shard](#1-配置变更时记录待-gc-的-shard)
    - [2. Migrate RPC 检查 gcWaitList](#2-migrate-rpc-检查-gcwaitlist)
    - [3. GC RPC 处理](#3-gc-rpc-处理)
    - [4. 应用 GC 操作](#4-应用-gc-操作)
    - [5. 拉取数据后发送 GC](#5-拉取数据后发送-gc)
    - [6. GC 通知重试机制](#6-gc-通知重试机制)
  - [快照支持](#快照支持)
  - [状态转换图](#状态转换图)
  - [配置推进条件](#配置推进条件)
  - [测试结果](#测试结果-2)
  - [遇到的问题与解决](#遇到的问题与解决)
    - [问题 1：死锁导致测试超时](#问题-1死锁导致测试超时)
    - [问题 2：GC 通知无限循环](#问题-2gc-通知无限循环)
    - [问题 3：快照恢复兼容性](#问题-3快照恢复兼容性)
  - [设计思考](#设计思考)
- [Challenge 2: 配置变更期间的无中断服务](#challenge-2-配置变更期间的无中断服务)
  - [问题背景](#问题背景-1)
  - [为什么 Challenge 2 被自然支持](#为什么-challenge-2-被自然支持)
    - [1. 独立的 shard 状态](#1-独立的-shard-状态)
    - [2. 请求只检查特定 shard 的状态](#2-请求只检查特定-shard-的状态)
    - [3. 迁移完成后立即可用](#3-迁移完成后立即可用)
  - [状态流转图](#状态流转图)
  - [设计要点](#设计要点)
    - [1. 不阻塞整个 group](#1-不阻塞整个-group)
    - [2. 配置推进不依赖"全部 shard 就绪"](#2-配置推进不依赖全部-shard-就绪)
    - [3. 迁移器独立工作](#3-迁移器独立工作)
  - [测试结果](#测试结果-3)
- [Lab 4 设计要点总结](#lab-4-设计要点总结)
  - [1. 分片系统的核心抽象](#1-分片系统的核心抽象)
    - [三层架构](#三层架构)
    - [状态流转](#状态流转)
  - [2. 关键设计决策](#2-关键设计决策)
    - [Pull vs Push 模型](#pull-vs-push-模型)
    - [配置顺序处理](#配置顺序处理)
    - [GC 确认机制](#gc-确认机制)
  - [3. 并发控制策略](#3-并发控制策略)
    - [单一 Mutex 原则](#单一-mutex-原则)
    - [后台 Goroutine 设计](#后台-goroutine-设计)
  - [4. 容错与幂等性](#4-容错与幂等性)
    - [RPC 幂等性保证](#rpc-幂等性保证)
    - [崩溃恢复](#崩溃恢复)
- [综合总结：MIT 6.824 Raft 实现要点](#综合总结mit-6824-raft-实现要点)
  - [1. 架构分层](#1-架构分层)
  - [2. 锁的使用原则](#2-锁的使用原则)
    - [原则 1：单一 Mutex](#原则-1单一-mutex)
    - [原则 2：持有锁的时间最小化](#原则-2持有锁的时间最小化)
    - [原则 3：不在持有锁时发送 RPC](#原则-3不在持有锁时发送-rpc)
  - [3. Goroutine 设计模式](#3-goroutine-设计模式)
    - [模式 1：定时器驱动](#模式-1定时器驱动)
    - [模式 2：事件驱动](#模式-2事件驱动)
    - [模式 3：等待通道](#模式-3等待通道)
  - [4. 幂等性设计](#4-幂等性设计)
  - [5. 调试技巧](#5-调试技巧)
    - [使用 DPrintf](#使用-dprintf)
    - [检查 race condition](#检查-race-condition)
    - [重复运行测试](#重复运行测试)
  - [6. 常见陷阱](#6-常见陷阱)
    - [陷阱 1：忘记深拷贝](#陷阱-1忘记深拷贝)
    - [陷阱 2：goroutine 泄漏](#陷阱-2goroutine-泄漏)
    - [陷阱 3：只提交当前 term 的日志](#陷阱-3只提交当前-term-的日志)
  - [最终测试结果](#最终测试结果)

<!-- /code_chunk_output -->

## 架构与要求：分片 K/V 服务

基于 Raft 实现一个 Sharded Key/Value Service ，目标是通过分片（sharding）实现水平扩展，让系统吞吐量随 replica group 数量线性增长。

### 为什么需要分片？

Lab 3 实现的 K/V 服务虽然具备容错能力，但 **所有请求都由同一个 replica group 处理** ，无法水平扩展。分片的核心思想是：
- 将 key 空间划分为多个 **shard** （分片）
- 不同的 **replica group** 负责不同的 shard
- 各 group 并行处理请求，总吞吐量 = 单 group 吞吐量 × group 数量

### 关键概念： shard, replica group, configuration, shard master

| 概念 | 说明 |
|------|------|
| **Shard** | key 空间的一个子集，通过 `key2shard(key)` 函数将 key 映射到 shard（本 lab 共 10 个 shard） |
| **Replica Group** | 一组使用 Raft 复制的服务器，负责一部分 shard 的读写。每个 group 有唯一的 GID (Group ID) |
| **Configuration** | 描述"哪个 shard 由哪个 group 负责"的映射关系，配置会随 group 加入/离开而变化 |
| **Shard Master** | 管理 configuration 的服务，本身也是一个基于 Raft 的容错服务 |

**数据结构定义** （`shardmaster/common.go`）：

```go
const NShards = 10  // shard 总数

type Config struct {
    Num    int              // configuration 编号（从 0 开始递增）
    Shards [NShards]int     // shard -> GID 的映射
    Groups map[int][]string // GID -> server names 的映射
}
```

**架构总览图：**

```
                            ┌─────────────────────────┐
                            │     Shard Master        │
                            │  (Raft-based service)   │
                            │                         │
                            │  configs: []Config      │
                            │  ┌─────────────────┐    │
                            │  │ Config #3       │    │
                            │  │ Shards: [G1,G1, │    │
                            │  │  G2,G2,G2,G3,...│    │
                            │  │ Groups: {       │    │
                            │  │   G1: [s0,s1,s2]│    │
                            │  │   G2: [s3,s4,s5]│    │
                            │  │   G3: [s6,s7,s8]│    │
                            │  │ }               │    │
                            │  └─────────────────┘    │
                            └───────────┬─────────────┘
                                        │ Query/Join/Leave/Move
          ┌─────────────────────────────┼─────────────────────────────┐
          │                             │                             │
          ▼                             ▼                             ▼
┌─────────────────┐           ┌─────────────────┐           ┌─────────────────┐
│ Replica Group 1 │           │ Replica Group 2 │           │ Replica Group 3 │
│    (GID=100)    │           │    (GID=101)    │           │    (GID=102)    │
│                 │           │                 │           │                 │
│  ┌───────────┐  │           │  ┌───────────┐  │           │  ┌───────────┐  │
│  │ ShardKV   │  │           │  │ ShardKV   │  │           │  │ ShardKV   │  │
│  │ Server 0  │  │  Shard    │  │ Server 3  │  │  Shard    │  │ Server 6  │  │
│  ├───────────┤  │ Transfer  │  ├───────────┤  │ Transfer  │  ├───────────┤  │
│  │ ShardKV   │◄─┼───────────┼─►│ ShardKV   │◄─┼───────────┼─►│ ShardKV   │  │
│  │ Server 1  │  │   RPC     │  │ Server 4  │  │   RPC     │  │ Server 7  │  │
│  ├───────────┤  │           │  ├───────────┤  │           │  ├───────────┤  │
│  │ ShardKV   │  │           │  │ ShardKV   │  │           │  │ ShardKV   │  │
│  │ Server 2  │  │           │  │ Server 5  │  │           │  │ Server 8  │  │
│  └───────────┘  │           │  └───────────┘  │           │  └───────────┘  │
│                 │           │                 │           │                 │
│ Shards: 0,1,2   │           │ Shards: 3,4,5   │           │ Shards: 6,7,8,9 │
│ (Raft inside)   │           │ (Raft inside)   │           │ (Raft inside)   │
└────────▲────────┘           └────────▲────────┘           └────────▲────────┘
         │                             │                             │
         │         Get/Put/Append      │                             │
         └─────────────────────────────┼─────────────────────────────┘
                                       │
                                ┌──────┴──────┐
                                │   Client    │
                                │  (Clerk)    │
                                └─────────────┘
```

### Raft 在哪里起作用？

本 Lab 中有 **两层 Raft** ：

| 层级 | 服务 | Raft 的作用 |
|------|------|------------|
| 第一层 | Shard Master | 保证 configuration 变更的一致性，所有节点看到相同的配置序列 |
| 第二层 | 每个 Replica Group | 保证该 group 内 K/V 操作的一致性，以及 reconfiguration 的顺序 |

```
┌─────────────────────────────────────────────────────────────────────┐
│                          Shard Master                               │
│  ┌─────────┐    ┌─────────┐    ┌─────────┐                          │
│  │ Server0 │◄──►│ Server1 │◄──►│ Server2 │  (Raft Consensus)        │
│  └─────────┘    └─────────┘    └─────────┘                          │
│       │              │              │                               │
│       └──────────────┼──────────────┘                               │
│                      │                                              │
│              configs []Config                                       │
│              (replicated via Raft)                                  │
└─────────────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────────────┐
│                        Replica Group (GID=100)                      │
│  ┌─────────┐    ┌─────────┐    ┌─────────┐                          │
│  │ShardKV 0│◄──►│ShardKV 1│◄──►│ShardKV 2│  (Raft Consensus)        │
│  └─────────┘    └─────────┘    └─────────┘                          │
│       │              │              │                               │
│       └──────────────┼──────────────┘                               │
│                      │                                              │
│        data map[string]string  (replicated via Raft)                │
│        + reconfiguration logs                                       │
└─────────────────────────────────────────────────────────────────────┘
```

### Shard Master 的 RPC 接口

Shard Master 提供 4 个 RPC 接口来管理 configuration：

| RPC | 参数 | 作用 |
|-----|------|------|
| `Join` | `map[int][]string` (GID → servers) | 添加新的 replica group，重新均衡 shard 分配 |
| `Leave` | `[]int` (GIDs) | 移除 replica group，将其 shard 重新分配给剩余 group |
| `Move` | `shard int, GID int` | 将指定 shard 移动到指定 group（用于测试） |
| `Query` | `num int` | 查询第 num 个 configuration（-1 表示最新） |

**负载均衡要求：**
- `Join` 和 `Leave` 后，shard 应尽可能均匀分布在各 group
- 移动尽可能少的 shard 来达到均衡（Minimal transfers）

**示例：**

```
初始状态 (Config #0):
  Shards: [0,0,0,0,0,0,0,0,0,0]  // 全部未分配 (GID=0 invalid)
  Groups: {}

Join(G1) -> Config #1:
  Shards: [G1,G1,G1,G1,G1,G1,G1,G1,G1,G1]  // G1 拥有全部 10 个 shard
  Groups: {G1: [s0,s1,s2]}

Join(G2) -> Config #2:
  Shards: [G1,G1,G1,G1,G1,G2,G2,G2,G2,G2]  // 均分：各 5 个
  Groups: {G1: [s0,s1,s2], G2: [s3,s4,s5]}

Leave(G1) -> Config #3:
  Shards: [G2,G2,G2,G2,G2,G2,G2,G2,G2,G2]  // G2 接管全部
  Groups: {G2: [s3,s4,s5]}
```

### 客户端请求流程

客户端（Clerk）需要：
1. 通过 `key2shard(key)` 确定 key 属于哪个 shard
2. 查询 configuration 确定该 shard 由哪个 group 负责
3. 向该 group 的 server 发送请求
4. 如果收到 `ErrWrongGroup`，重新查询最新 configuration 并重试

```go
func key2shard(key string) int {
    shard := 0
    if len(key) > 0 {
        shard = int(key[0])
    }
    shard %= NShards  // NShards = 10
    return shard
}
```

**请求流程图：**

```
Client                   Shard Master              Replica Group
  │                           │                          │
  │  1. Query(-1)             │                          │
  │──────────────────────────►│                          │
  │◄──────────────────────────│                          │
  │     Config{Shards, Groups}│                          │
  │                           │                          │
  │  2. key2shard("abc") = 7  │                          │
  │     Config.Shards[7] = G2 │                          │
  │     Config.Groups[G2] = [s3,s4,s5]                   │
  │                           │                          │
  │  3. Get("abc") ───────────┼─────────────────────────►│ Group G2
  │                           │                          │
  │                           │   (if ErrWrongGroup)     │
  │◄──────────────────────────┼──────────────────────────│
  │                           │                          │
  │  4. Re-Query & Retry      │                          │
  │──────────────────────────►│                          │
  ...
```

### 核心挑战：Reconfiguration（配置变更）

这是 Lab 4 **最核心的难点** 。当配置变更时：

**问题 1：请求与配置变更的顺序**

同一个 replica group 内，所有成员必须就"配置变更发生在哪个时间点"达成一致。

```
Timeline:
  ─────────────────────────────────────────────────►
                    │
     Put("x", "1")  │  Reconfiguration    Put("x", "2")
         │          │  (G1 loses shard)       │
         ▼          ▼                         ▼
      ┌──────────────────────────────────────────┐
      │  Group G1: 这个 Put 发生在配置变更之前还是之后？│
      │  所有 replica 必须达成一致！                │
      └──────────────────────────────────────────┘
```

**解决方案** ：将 reconfiguration 也作为 Raft log entry，和 Put/Get 一起排序。

**问题 2：Shard 数据迁移**

当 shard 从 G1 转移到 G2 时：
- G1 必须停止服务该 shard，并将数据发送给 G2
- G2 必须等待收到数据后才能开始服务该 shard
- 迁移期间要保证 **线性一致性**

```
Config #10: Shard S1 belongs to G1
Config #11: Shard S1 belongs to G2

G1                                    G2
│                                     │
│  Stop serving S1                    │  Cannot serve S1 yet
│  ───────────────────────────────────│  (waiting for data)
│                                     │
│  Send S1 data (key/value pairs)     │
│  ──────────────────────────────────►│
│                                     │
│                                     │  Start serving S1
│                                     │  ◄─────────────────
```

**问题 3：去重表迁移**

Lab 3 中的 duplicate detection（去重）机制也需要随 shard 一起迁移：

```go
// 每个 shard 的数据不仅包括 key-value，还要包括该 shard 相关的去重信息
type ShardData struct {
    KV          map[string]string    // key -> value
    LastReplies map[int64]LastReply  // clientId -> last reply (for dedup)
}
```

### 理解测试代码：测试场景示例

结合 `shardkv/config.go` 和 `shardkv/test_test.go` 来理解测试流程：

**测试基础设施：**

```go
// config.go 中的结构
type config struct {
    nmasters      int                       // shard master 节点数
    masterservers []*shardmaster.ShardMaster
    mck           *shardmaster.Clerk        // 用于调用 Join/Leave

    ngroups int                             // replica group 数量
    n       int                             // 每个 group 的 server 数
    groups  []*group                        // 所有 replica groups
}

type group struct {
    gid      int                            // Group ID
    servers  []*ShardKV                     // 该 group 的 servers
    // ...
}
```

**测试场景 1：静态分片 (`TestStaticShards`)**

```go
func TestStaticShards(t *testing.T) {
    cfg := make_config(t, 3, false, -1)  // 3 servers per group
    
    cfg.join(0)   // Join group 0 (GID=100)
    cfg.join(1)   // Join group 1 (GID=101)
    
    // 写入 10 个 key，分布在不同 shard
    for i := 0; i < 10; i++ {
        ck.Put(key, value)
    }
    
    // 关闭 group 1，验证只有部分 Get 能成功
    cfg.ShutdownGroup(1)
    // 约 5 个 key 在 group 0，5 个在 group 1
    // 只有 group 0 的 key 能读取成功
}
```

**测试场景 2：动态配置变更 (`TestJoinLeave`)**

```go
func TestJoinLeave(t *testing.T) {
    cfg.join(0)          // 只有 group 0
    // Put some keys...
    
    cfg.join(1)          // 加入 group 1，shard 会重新分配
    // 验证所有 key 仍可访问（部分已迁移到 group 1）
    
    cfg.leave(0)         // group 0 离开
    // 验证所有 key 仍可访问（全部迁移到 group 1）
    
    cfg.ShutdownGroup(0) // 关闭 group 0
    // 验证所有 key 仍可访问（group 0 已不负责任何 shard）
}
```

**数据流示意：**

```
Phase 1: Only G0
┌──────────────────────────────────────┐
│  Group 0 (GID=100)                   │
│  Shards: [0,1,2,3,4,5,6,7,8,9]       │
│  Keys: "0a","1b","2c",...,"9j"       │
└──────────────────────────────────────┘

Phase 2: Join G1
┌────────────────────┐  ┌────────────────────┐
│  Group 0 (GID=100) │  │  Group 1 (GID=101) │
│  Shards: [0,1,2,3,4]│  │  Shards: [5,6,7,8,9]│
│  Keys: "0a"~"4e"   │  │  Keys: "5f"~"9j"   │
│                    │◄─┤  (migrated from G0)│
└────────────────────┘  └────────────────────┘

Phase 3: Leave G0
┌──────────────────────────────────────┐
│  Group 1 (GID=101)                   │
│  Shards: [0,1,2,3,4,5,6,7,8,9]       │
│  Keys: all (migrated from G0)        │
└──────────────────────────────────────┘
```

### Lab 4 的两个 Part

| Part | 内容 | 难度 |
|------|------|------|
| **Part A** | 实现 Shard Master（配置管理服务） | 较简单，类似 Lab 3 |
| **Part B** | 实现 ShardKV（分片 K/V 服务 + 配置变更） | 较复杂，核心挑战 |

**Part A 要点：**
- 基于 Lab 3 的 kvraft 改造
- 实现 Join/Leave/Move/Query 四个 RPC
- 需要实现负载均衡算法（minimal shard movement）
- 需要实现 duplicate detection

**Part B 要点：**
- 周期性轮询 Shard Master 获取最新配置（约 100ms 一次）
- 检测到配置变更时，启动 shard 迁移
- 迁移期间正确拒绝不属于自己的 shard 请求（返回 `ErrWrongGroup`）
- 保证 group 内所有成员在相同的 log 位置执行配置变更
- 处理 shard 数据 + 去重表的迁移

### Challenge 挑战（可选）

| Challenge | 目标 |
|-----------|------|
| **Challenge 1** | Garbage Collection：及时删除不再负责的 shard 数据 |
| **Challenge 2** | Unaffected Shard Access：配置变更期间，不受影响的 shard 继续服务 |

Challenge 2 的优化目标：

```
Without Challenge 2:
  Config change in progress -> ALL shards blocked

With Challenge 2:
  Config change in progress -> Only affected shards blocked
  
  Example: G3 needs S1 from G1, S2 from G2
           G1 is down, but G2 is up
           -> G3 can start serving S2 immediately
           -> G3 blocks only S1 until G1 recovers
```

### 关键实现提示

1. **配置变更必须按顺序处理** ：一次只处理一个配置变更，从 config N 到 config N+1

2. **Shard 迁移的时机** ：当 group 发现新配置时：
   - 如果失去某个 shard ：立即停止服务，准备发送数据
   - 如果获得某个 shard ：等待数据到达后才开始服务

3. **避免死锁** ：两个 group 可能需要互相传输 shard（G1→G2 同时 G2→G1）

4. **RPC 中的 map/slice** ：发送前要深拷贝，避免并发读写 race

5. **Raft log 中的 map/slice** ：apply 时要深拷贝，避免 Raft 持久化时的 race

## 4A: Shard Master 实现

### 整体思路

Shard Master 本质上是一个 **基于 Raft 的配置管理服务** ，与 Lab 3 的 KVRaft 架构非常类似：
- 使用 Raft 保证配置变更的一致性
- 需要实现 duplicate detection （去重）
- RPC handler 提交操作到 Raft，等待 apply 后返回

**主要区别** ：
- 状态不是 key-value map，而是 `[]Config` 配置序列
- 需要实现 shard 负载均衡算法

### 数据结构设计

#### RPC 参数扩展

为了实现去重，需要在 RPC 参数中添加 `ClientId` 和 `SeqNum`：

```go
type JoinArgs struct {
    Servers  map[int][]string // new GID -> servers mappings
    ClientId int64
    SeqNum   int64
}

type LeaveArgs struct {
    GIDs     []int
    ClientId int64
    SeqNum   int64
}

type MoveArgs struct {
    Shard    int
    GID      int
    ClientId int64
    SeqNum   int64
}

type QueryArgs struct {
    Num      int // desired config number
    ClientId int64
    SeqNum   int64
}
```

#### Op 结构体

统一所有操作类型：

```go
type Op struct {
    Type     string  // "Join", "Leave", "Move", "Query"
    ClientId int64
    SeqNum   int64

    // Join
    Servers map[int][]string
    // Leave
    GIDs []int
    // Move
    Shard int
    GID   int
    // Query
    Num int
}
```

#### ShardMaster 结构体

```go
type ShardMaster struct {
    mu      sync.Mutex
    me      int
    rf      *raft.Raft
    applyCh chan raft.ApplyMsg
    dead    int32

    configs []Config // indexed by config num

    // 等待 Raft commit 的通道
    waitChs map[int]chan Op

    // 去重：记录每个 client 最后处理的请求
    lastReplies map[int64]LastReply
}

type LastReply struct {
    SeqNum int64
    Config Config // 仅 Query 需要缓存结果
}
```

### Client 实现

Client 的实现与 Lab 3 几乎一致，核心是：
1. 维护 `clientId`（随机生成）和 `seqNum`（单调递增）
2. 每次请求前 `seqNum++`
3. 循环尝试所有 server 直到成功

```go
type Clerk struct {
    servers  []*labrpc.ClientEnd
    clientId int64
    seqNum   int64
}

func MakeClerk(servers []*labrpc.ClientEnd) *Clerk {
    ck := new(Clerk)
    ck.servers = servers
    ck.clientId = nrand()
    ck.seqNum = 0
    return ck
}

func (ck *Clerk) Join(servers map[int][]string) {
    ck.seqNum++
    args := &JoinArgs{
        Servers:  servers,
        ClientId: ck.clientId,
        SeqNum:   ck.seqNum,
    }

    for {
        for _, srv := range ck.servers {
            var reply JoinReply
            ok := srv.Call("ShardMaster.Join", args, &reply)
            if ok && reply.WrongLeader == false {
                return
            }
        }
        time.Sleep(100 * time.Millisecond)
    }
}
```

### Server RPC Handler 实现

以 `Join` 为例，流程与 Lab 3 的 KVServer 一致：

```go
func (sm *ShardMaster) Join(args *JoinArgs, reply *JoinReply) {
    sm.mu.Lock()
    // 1. 检查去重
    if sm.isDuplicate(args.ClientId, args.SeqNum) {
        sm.mu.Unlock()
        reply.WrongLeader = false
        reply.Err = OK
        return
    }
    sm.mu.Unlock()

    // 2. 构造 Op 并提交到 Raft
    op := Op{
        Type:     OpJoin,
        ClientId: args.ClientId,
        SeqNum:   args.SeqNum,
        Servers:  args.Servers,
    }

    index, _, isLeader := sm.rf.Start(op)
    if !isLeader {
        reply.WrongLeader = true
        return
    }

    // 3. 等待 Raft commit
    ch := sm.getWaitCh(index)
    select {
    case appliedOp := <-ch:
        if appliedOp.ClientId == op.ClientId && appliedOp.SeqNum == op.SeqNum {
            reply.WrongLeader = false
            reply.Err = OK
        } else {
            reply.WrongLeader = true
        }
    case <-time.After(500 * time.Millisecond):
        reply.WrongLeader = true
    }

    // 4. 清理 channel
    sm.mu.Lock()
    delete(sm.waitChs, index)
    sm.mu.Unlock()
}
```

### applier 实现

`applier` goroutine 持续从 `applyCh` 读取已 commit 的命令并执行：

```go
func (sm *ShardMaster) applier() {
    for !sm.killed() {
        msg := <-sm.applyCh
        if msg.CommandValid {
            sm.applyCommand(msg)
        }
    }
}

func (sm *ShardMaster) applyCommand(msg raft.ApplyMsg) {
    op := msg.Command.(Op)
    index := msg.CommandIndex

    sm.mu.Lock()

    // 检查去重
    if !sm.isDuplicate(op.ClientId, op.SeqNum) {
        switch op.Type {
        case OpJoin:
            sm.applyJoin(op.Servers)
        case OpLeave:
            sm.applyLeave(op.GIDs)
        case OpMove:
            sm.applyMove(op.Shard, op.GID)
        case OpQuery:
            // Query 不修改状态
        }

        // 记录去重信息（Query 需要缓存结果）
        config := sm.getQueryResult(op.Num)
        sm.lastReplies[op.ClientId] = LastReply{
            SeqNum: op.SeqNum,
            Config: config,
        }
    }

    // 通知等待的 RPC handler
    ch, ok := sm.waitChs[index]
    sm.mu.Unlock()

    if ok {
        select {
        case ch <- op:
        default:
        }
    }
}
```

### 核心：负载均衡算法

这是 Part A 最关键的部分。测试要求：
1. **均匀分配** ：每个 group 拥有的 shard 数量相差不超过 1
2. **最小移动** ：Join/Leave 后移动尽可能少的 shard

#### 算法思路

```
1. 计算目标分配：
   - avgShards = NShards / numGroups
   - extraShards = NShards % numGroups
   - 前 extraShards 个 group 拥有 avgShards+1 个 shard
   - 其余 group 拥有 avgShards 个 shard

2. 收集多余 shard：
   - 遍历所有 group
   - 如果某 group 拥有的 shard 超过目标，将多余的放入 unassigned

3. 分配不足 shard：
   - 遍历所有 group
   - 如果某 group 拥有的 shard 不足目标，从 unassigned 中取
```

#### 关键：确定性

**问题** ：Go 的 map 遍历顺序是随机的。如果不同节点按不同顺序处理，会导致状态不一致。

**解决方案** ：将 GID 排序后按固定顺序处理。

```go
func (sm *ShardMaster) rebalance(config *Config) {
    numGroups := len(config.Groups)
    if numGroups == 0 {
        for i := 0; i < NShards; i++ {
            config.Shards[i] = 0
        }
        return
    }

    // 获取排序后的 gid 列表（保证确定性）
    gids := make([]int, 0, numGroups)
    for gid := range config.Groups {
        gids = append(gids, gid)
    }
    sort.Ints(gids)

    // 计算每个 group 应该拥有的 shard 数量
    avgShards := NShards / numGroups
    extraShards := NShards % numGroups

    // 计算每个 group 当前拥有的 shard
    gidToShards := make(map[int][]int)
    for gid := range config.Groups {
        gidToShards[gid] = []int{}
    }
    var unassigned []int
    for shard, gid := range config.Shards {
        if gid == 0 || config.Groups[gid] == nil {
            unassigned = append(unassigned, shard)
        } else {
            gidToShards[gid] = append(gidToShards[gid], shard)
        }
    }

    // 计算每个 group 的目标 shard 数量
    targetShards := make(map[int]int)
    for i, gid := range gids {
        if i < extraShards {
            targetShards[gid] = avgShards + 1
        } else {
            targetShards[gid] = avgShards
        }
    }

    // 收集多余的 shard
    for _, gid := range gids {
        target := targetShards[gid]
        shards := gidToShards[gid]
        if len(shards) > target {
            sort.Ints(shards)  // 排序保证确定性
            unassigned = append(unassigned, shards[target:]...)
            gidToShards[gid] = shards[:target]
        }
    }

    sort.Ints(unassigned)  // 排序保证确定性

    // 分配不足的 shard
    for _, gid := range gids {
        target := targetShards[gid]
        for len(gidToShards[gid]) < target && len(unassigned) > 0 {
            gidToShards[gid] = append(gidToShards[gid], unassigned[0])
            unassigned = unassigned[1:]
        }
    }

    // 更新 config.Shards
    for gid, shards := range gidToShards {
        for _, shard := range shards {
            config.Shards[shard] = gid
        }
    }
}
```

#### 示例

```
初始状态：NShards=10, Groups={G1, G2}
  avgShards = 10/2 = 5
  extraShards = 10%2 = 0
  每个 group 目标：5 个 shard

Join(G3) -> Groups={G1, G2, G3}
  avgShards = 10/3 = 3
  extraShards = 10%3 = 1
  
  排序后 gids = [G1, G2, G3]
  目标分配：G1=4, G2=3, G3=3  (G1 是前 extraShards=1 个)
  
  假设当前：
    G1: [0,1,2,3,4]  (5个，需要减1)
    G2: [5,6,7,8,9]  (5个，需要减2)
    G3: []           (0个，需要加3)
  
  收集多余：
    G1 多余 shard 4 -> unassigned=[4]
    G2 多余 shard 8,9 -> unassigned=[4,8,9]
  
  分配不足：
    G3 需要 3 个 -> 从 unassigned 取 [4,8,9]
  
  最终：
    G1: [0,1,2,3]
    G2: [5,6,7]
    G3: [4,8,9]
```

### 处理 Join/Leave/Move

#### Join

```go
func (sm *ShardMaster) applyJoin(servers map[int][]string) {
    newConfig := sm.copyConfig()
    
    // 添加新的 groups
    for gid, serverList := range servers {
        newConfig.Groups[gid] = append([]string{}, serverList...)
    }
    
    sm.rebalance(&newConfig)
    sm.configs = append(sm.configs, newConfig)
}
```

#### Leave

```go
func (sm *ShardMaster) applyLeave(gids []int) {
    newConfig := sm.copyConfig()
    
    // 删除离开的 groups
    for _, gid := range gids {
        delete(newConfig.Groups, gid)
        // 将这些 group 的 shards 标记为未分配
        for i := 0; i < NShards; i++ {
            if newConfig.Shards[i] == gid {
                newConfig.Shards[i] = 0
            }
        }
    }
    
    sm.rebalance(&newConfig)
    sm.configs = append(sm.configs, newConfig)
}
```

#### Move

Move 不需要 rebalance，直接设置即可：

```go
func (sm *ShardMaster) applyMove(shard int, gid int) {
    newConfig := sm.copyConfig()
    newConfig.Shards[shard] = gid
    sm.configs = append(sm.configs, newConfig)
}
```

### 深拷贝 Config

创建新 Config 时必须深拷贝，避免修改旧 Config：

```go
func (sm *ShardMaster) copyConfig() Config {
    lastConfig := sm.configs[len(sm.configs)-1]
    newConfig := Config{
        Num:    lastConfig.Num + 1,
        Shards: lastConfig.Shards,  // 数组是值拷贝
        Groups: make(map[int][]string),
    }
    for gid, servers := range lastConfig.Groups {
        newConfig.Groups[gid] = append([]string{}, servers...)
    }
    return newConfig
}
```

**注意** ：
- `Shards [NShards]int` 是数组，赋值时会自动拷贝
- `Groups map[int][]string` 是 map，必须手动深拷贝

### 测试结果

```bash
$ go test -race
Test: Basic leave/join ...
  ... Passed
Test: Historical queries ...
  ... Passed
Test: Move ...
  ... Passed
Test: Concurrent leave/join ...
  ... Passed
Test: Minimal transfers after joins ...
  ... Passed
Test: Minimal transfers after leaves ...
  ... Passed
Test: Multi-group join/leave ...
  ... Passed
Test: Concurrent multi leave/join ...
  ... Passed
Test: Minimal transfers after multijoins ...
  ... Passed
Test: Minimal transfers after multileaves ...
  ... Passed
PASS
ok      shardmaster     3.226s
```

所有测试稳定通过。

## 4B: ShardKV 实现

### 整体思路

ShardKV 是一个 **分片式 K/V 服务** ，核心挑战在于 **配置变更时的 shard 迁移** 。每个 replica group：
- 只负责部分 shard 的读写
- 需要定期从 Shard Master 获取最新配置
- 配置变更时需要：
  - 停止服务不再负责的 shard
  - 从其他 group 拉取新获得的 shard 数据
  - 保证 group 内所有节点在同一 Raft log 位置执行配置变更

**与 Lab 3 KVRaft 的区别** ：
- 状态机包含多个 shard 的数据
- 需要处理 `ErrWrongGroup` 错误
- 需要支持 shard 迁移 RPC
- 需要追踪每个 shard 的状态

### 数据结构设计

#### RPC 结构体

在 `common.go` 中扩展 RPC 结构体，添加去重字段和 shard 迁移 RPC：

```go
const (
    OK             = "OK"
    ErrNoKey       = "ErrNoKey"
    ErrWrongGroup  = "ErrWrongGroup"
    ErrWrongLeader = "ErrWrongLeader"
    ErrNotReady    = "ErrNotReady" // shard 还没准备好
)

type GetArgs struct {
    Key      string
    ClientId int64
    SeqNum   int64
}

type PutAppendArgs struct {
    Key      string
    Value    string
    Op       string // "Put" or "Append"
    ClientId int64
    SeqNum   int64
}

// Shard 迁移 RPC
type MigrateArgs struct {
    Shard     int
    ConfigNum int // 请求的配置编号
}

type MigrateReply struct {
    Err         Err
    Data        map[string]string   // shard 的 key-value 数据
    LastReplies map[int64]LastReply // 去重信息
}
```

#### Shard 状态机

每个 shard 可能处于以下状态：

```go
const (
    ShardServing = "Serving" // 正常服务
    ShardPulling = "Pulling" // 需要从其他 group 拉取数据
    ShardInvalid = "Invalid" // 不负责该 shard
)
```

#### Op 结构体

统一所有操作类型：

```go
type Op struct {
    Type     string
    Key      string
    Value    string
    ClientId int64
    SeqNum   int64

    // 配置变更
    Config shardmaster.Config

    // Shard 迁移
    Shard       int
    Data        map[string]string
    LastReplies map[int64]LastReply
    ConfigNum   int
}
```

#### ShardKV 结构体

```go
type ShardKV struct {
    mu           sync.Mutex
    me           int
    rf           *raft.Raft
    applyCh      chan raft.ApplyMsg
    make_end     func(string) *labrpc.ClientEnd
    gid          int
    masters      []*labrpc.ClientEnd
    maxraftstate int
    dead         int32
    persister    *raft.Persister

    // 当前配置
    config shardmaster.Config

    // 每个 shard 的状态
    shardState [shardmaster.NShards]string

    // 每个 shard 的数据（分离存储便于迁移）
    data [shardmaster.NShards]map[string]string

    // 去重信息
    lastReplies map[int64]LastReply

    // 等待 Raft commit 的通道
    waitChs map[int]chan Op

    // 最后应用的日志索引
    lastApplied int
}
```

### Client 实现

Client 实现与 Lab 3 类似，主要区别是需要处理 `ErrWrongGroup` 并重新查询配置：

```go
type Clerk struct {
    sm       *shardmaster.Clerk
    config   shardmaster.Config
    make_end func(string) *labrpc.ClientEnd
    clientId int64
    seqNum   int64
}

func (ck *Clerk) Get(key string) string {
    ck.seqNum++
    args := GetArgs{
        Key:      key,
        ClientId: ck.clientId,
        SeqNum:   ck.seqNum,
    }

    for {
        shard := key2shard(key)
        gid := ck.config.Shards[shard]
        if servers, ok := ck.config.Groups[gid]; ok {
            for si := 0; si < len(servers); si++ {
                srv := ck.make_end(servers[si])
                var reply GetReply
                ok := srv.Call("ShardKV.Get", &args, &reply)
                if ok && (reply.Err == OK || reply.Err == ErrNoKey) {
                    return reply.Value
                }
                if ok && reply.Err == ErrWrongGroup {
                    break // 尝试下一个 group
                }
            }
        }
        time.Sleep(100 * time.Millisecond)
        ck.config = ck.sm.Query(-1) // 重新获取配置
    }
}
```

### Server 核心实现

#### canServe：判断是否可以服务某个 shard

```go
func (kv *ShardKV) canServe(shard int) bool {
    return kv.config.Shards[shard] == kv.gid && 
           kv.shardState[shard] == ShardServing
}
```

关键点：
- 必须是当前配置指定负责的 shard
- 必须处于 Serving 状态（数据已就绪）

#### Get/PutAppend RPC Handler

```go
func (kv *ShardKV) Get(args *GetArgs, reply *GetReply) {
    kv.mu.Lock()
    shard := key2shard(args.Key)

    // 检查是否负责该 shard
    if !kv.canServe(shard) {
        kv.mu.Unlock()
        reply.Err = ErrWrongGroup
        return
    }

    // 检查去重
    if kv.isDuplicate(args.ClientId, args.SeqNum) {
        lastReply := kv.lastReplies[args.ClientId]
        reply.Value = lastReply.Value
        reply.Err = lastReply.Err
        kv.mu.Unlock()
        return
    }
    kv.mu.Unlock()

    // 提交到 Raft
    op := Op{
        Type:     OpGet,
        Key:      args.Key,
        ClientId: args.ClientId,
        SeqNum:   args.SeqNum,
    }

    index, _, isLeader := kv.rf.Start(op)
    if !isLeader {
        reply.Err = ErrWrongLeader
        return
    }

    // 等待 Raft commit
    ch := kv.getWaitCh(index)
    select {
    case appliedOp := <-ch:
        if appliedOp.ClientId == op.ClientId && appliedOp.SeqNum == op.SeqNum {
            kv.mu.Lock()
            // 再次检查（配置可能已变更）
            if !kv.canServe(shard) {
                reply.Err = ErrWrongGroup
            } else {
                lastReply := kv.lastReplies[args.ClientId]
                reply.Value = lastReply.Value
                reply.Err = lastReply.Err
            }
            kv.mu.Unlock()
        } else {
            reply.Err = ErrWrongLeader
        }
    case <-time.After(500 * time.Millisecond):
        reply.Err = ErrWrongLeader
    }

    kv.mu.Lock()
    delete(kv.waitChs, index)
    kv.mu.Unlock()
}
```

#### Migrate RPC Handler

供其他 group 拉取 shard 数据：

```go
func (kv *ShardKV) Migrate(args *MigrateArgs, reply *MigrateReply) {
    kv.mu.Lock()
    defer kv.mu.Unlock()

    // 只有 leader 响应
    if _, isLeader := kv.rf.GetState(); !isLeader {
        reply.Err = ErrWrongLeader
        return
    }

    // 配置版本检查
    if kv.config.Num < args.ConfigNum {
        reply.Err = ErrNotReady
        return
    }

    // 深拷贝 shard 数据
    reply.Data = make(map[string]string)
    for k, v := range kv.data[args.Shard] {
        reply.Data[k] = v
    }

    // 深拷贝去重信息
    reply.LastReplies = make(map[int64]LastReply)
    for clientId, lr := range kv.lastReplies {
        reply.LastReplies[clientId] = lr
    }

    reply.Err = OK
}
```

### applier 实现

#### applyClientOp ：应用客户端操作

```go
func (kv *ShardKV) applyClientOp(op Op) {
    shard := key2shard(op.Key) // 其实也可以使用 op.Shard
    // 由于 op.Key -> shard 是一个固定规则， key2shard 放在哪里计算无所谓的

    // 检查是否可以服务该 shard
    if !kv.canServe(shard) {
        return
    }

    // 检查去重
    if kv.isDuplicate(op.ClientId, op.SeqNum) {
        return
    }

    // 执行操作
    switch op.Type {
    case OpPut:
        kv.data[shard][op.Key] = op.Value
    case OpAppend:
        kv.data[shard][op.Key] += op.Value
    }

    // 记录去重信息
    kv.lastReplies[op.ClientId] = LastReply{
        SeqNum: op.SeqNum,
        Value:  kv.data[shard][op.Key],
        Err:    OK,
    }
}
```

#### applyConfig ：应用配置变更

```go
func (kv *ShardKV) applyConfig(newConfig shardmaster.Config) {
    // 只能按顺序更新配置
    if newConfig.Num != kv.config.Num+1 {
        return
    }

    // 检查是否所有需要拉取的 shard 都已完成
    for shard := 0; shard < shardmaster.NShards; shard++ {
        if kv.shardState[shard] == ShardPulling {
            return
        }
    }

    // 更新 shard 状态
    for shard := 0; shard < shardmaster.NShards; shard++ {
        oldGid := kv.config.Shards[shard]
        newGid := newConfig.Shards[shard]

        if newGid == kv.gid {
            if oldGid == kv.gid || oldGid == 0 {
                // 继续负责或从无效 group 获得
                kv.shardState[shard] = ShardServing
            } else {
                // 需要从其他 group 拉取
                kv.shardState[shard] = ShardPulling
            }
        } else {
            // 不再负责该 shard
            kv.shardState[shard] = ShardInvalid
        }
    }

    kv.config = newConfig
}
```

关键点：
- **顺序处理配置** ：只能处理 `Num+1` 的配置，确保不跳过任何配置
- **等待迁移完成** ：如果有 shard 正在 Pulling，不能切换到下一个配置

#### applyMigrate ：应用 shard 迁移

```go
func (kv *ShardKV) applyMigrate(op Op) {
    // 检查配置版本
    if op.ConfigNum != kv.config.Num {
        return
    }

    // 检查 shard 状态
    if kv.shardState[op.Shard] != ShardPulling {
        return
    }

    // 拷贝数据
    kv.data[op.Shard] = make(map[string]string)
    for k, v := range op.Data {
        kv.data[op.Shard][k] = v
    }

    // 合并去重信息（取较大的 SeqNum）
    for clientId, lr := range op.LastReplies {
        if existing, ok := kv.lastReplies[clientId]; !ok || lr.SeqNum > existing.SeqNum {
            kv.lastReplies[clientId] = lr
        }
    }

    // 更新状态为 Serving
    kv.shardState[op.Shard] = ShardServing
}
```

### 后台 Goroutine

#### configPoller：定期获取配置

```go
func (kv *ShardKV) configPoller() {
    mck := shardmaster.MakeClerk(kv.masters)

    for !kv.killed() {
        if _, isLeader := kv.rf.GetState(); isLeader {
            kv.mu.Lock()
            currentNum := kv.config.Num

            // 检查是否所有 shard 都准备好
            allReady := true
            for shard := 0; shard < shardmaster.NShards; shard++ {
                if kv.shardState[shard] == ShardPulling {
                    allReady = false
                    break
                }
            }
            kv.mu.Unlock()

            if allReady {
                // 获取下一个配置
                newConfig := mck.Query(currentNum + 1)
                if newConfig.Num == currentNum+1 {
                    op := Op{
                        Type:   OpConfig,
                        Config: newConfig,
                    }
                    kv.rf.Start(op)
                }
            }
        }
        time.Sleep(100 * time.Millisecond)
    }
}
```

**注意** ：每个 goroutine 需要创建独立的 shardmaster Clerk，避免 race condition。

#### migrator：执行 shard 迁移

```go
func (kv *ShardKV) migrator() {
    mck := shardmaster.MakeClerk(kv.masters)

    for !kv.killed() {
        if _, isLeader := kv.rf.GetState(); isLeader {
            kv.mu.Lock()
            configNum := kv.config.Num
            shardsToFetch := make(map[int]int)
            for shard := 0; shard < shardmaster.NShards; shard++ {
                if kv.shardState[shard] == ShardPulling {
                    shardsToFetch[shard] = 1
                }
            }
            kv.mu.Unlock()

            if len(shardsToFetch) > 0 && configNum > 0 {
                oldConfig := mck.Query(configNum - 1)
                for shard := range shardsToFetch {
                    oldGid := oldConfig.Shards[shard]
                    if oldGid != 0 {
                        go kv.fetchShard(shard, oldGid, oldConfig, configNum)
                    }
                }
            }
        }
        time.Sleep(50 * time.Millisecond)
    }
}

func (kv *ShardKV) fetchShard(shard int, gid int, oldConfig shardmaster.Config, configNum int) {
    servers := oldConfig.Groups[gid]
    args := MigrateArgs{
        Shard:     shard,
        ConfigNum: configNum,
    }

    for _, serverName := range servers {
        srv := kv.make_end(serverName)
        var reply MigrateReply
        ok := srv.Call("ShardKV.Migrate", &args, &reply)
        if ok && reply.Err == OK {
            // 提交到 Raft
            op := Op{
                Type:        OpMigrate,
                Shard:       shard,
                Data:        reply.Data,
                LastReplies: reply.LastReplies,
                ConfigNum:   configNum,
            }
            kv.rf.Start(op)
            return
        }
    }
}
```

### Snapshot 支持

快照需要保存所有状态：

```go
func (kv *ShardKV) takeSnapshot(index int) {
    w := new(bytes.Buffer)
    e := labgob.NewEncoder(w)
    e.Encode(kv.config)
    e.Encode(kv.shardState)
    e.Encode(kv.data)
    e.Encode(kv.lastReplies)
    kv.rf.Snapshot(index, w.Bytes())
}

func (kv *ShardKV) readSnapshotL(snapshot []byte) {
    if snapshot == nil || len(snapshot) < 1 {
        return
    }
    r := bytes.NewBuffer(snapshot)
    d := labgob.NewDecoder(r)

    var config shardmaster.Config
    var shardState [shardmaster.NShards]string
    var data [shardmaster.NShards]map[string]string
    var lastReplies map[int64]LastReply

    if d.Decode(&config) != nil ||
        d.Decode(&shardState) != nil ||
        d.Decode(&data) != nil ||
        d.Decode(&lastReplies) != nil {
        return
    }

    kv.config = config
    kv.shardState = shardState
    kv.data = data
    kv.lastReplies = lastReplies
}
```

### 配置变更流程图

```
Group G1 (gid=100)                    Group G2 (gid=101)
     │                                      │
     │  Config #1: G1 owns all shards       │
     │                                      │
     ▼                                      ▼
┌─────────────────┐                   ┌─────────────────┐
│ Shards: all     │                   │ Shards: none    │
│ State: Serving  │                   │ State: Invalid  │
└─────────────────┘                   └─────────────────┘
     │                                      │
     │  Config #2: G1 loses S5-S9 to G2     │
     │                                      │
     ▼                                      ▼
┌─────────────────┐                   ┌─────────────────┐
│ S0-S4: Serving  │                   │ S5-S9: Pulling  │
│ S5-S9: Invalid  │                   │                 │
└─────────────────┘                   └─────────────────┘
     │                                      │
     │  G2 fetches S5-S9 from G1            │
     │  ◄─────────────────────────────────  │
     │                                      │
     ▼                                      ▼
┌─────────────────┐                   ┌─────────────────┐
│ S0-S4: Serving  │                   │ S5-S9: Serving  │
│ S5-S9: Invalid  │                   │                 │
└─────────────────┘                   └─────────────────┘
```

### 关键设计决策

1. **Pull 模式 vs Push 模式** ：选择 Pull 模式，由新负责的 group 主动拉取数据。优点是：
   - 不需要等待旧 group 推送
   - 逻辑更简单，只需要在新 group 处理

2. **去重信息迁移** ：去重表需要随 shard 一起迁移，否则可能导致重复执行。实现时取各 client 的较大 SeqNum。

3. **独立 Clerk** ：configPoller 和 migrator 各自创建独立的 shardmaster Clerk，避免并发访问导致的 race condition。

4. **顺序配置处理** ：一次只处理一个配置变更（从 N 到 N+1），确保不会跳过配置。

### 测试结果

```bash
$ go test -race -run 'Test(StaticShards|JoinLeave|Snapshot|MissChange|Concurrent1|Concurrent2|Unreliable1|Unreliable2|Unreliable3)'
Test: static shards ...
  ... Passed
Test: join then leave ...
  ... Passed
Test: snapshots, join, and leave ...
  ... Passed
Test: servers miss configuration changes...
  ... Passed
Test: concurrent puts and configuration changes...
  ... Passed
Test: more concurrent puts and configuration changes...
  ... Passed
Test: unreliable 1...
  ... Passed
Test: unreliable 2...
  ... Passed
Test: unreliable 3...
  ... Passed
PASS
ok      shardkv     57.239s
```

多次运行测试稳定通过。

## Challenge 1: Garbage Collection（垃圾回收）

### 问题背景

在 Part B 的基础实现中，当一个 replica group 失去某个 shard 的所有权后，数据仍然保留在内存中。这存在以下问题：

1. **内存浪费** ：不再负责的 shard 数据占用内存
2. **快照膨胀** ：旧数据被包含在 Raft 快照中，导致快照体积增长
3. **生产环境不可接受** ：在真实系统中，这会导致严重的资源浪费

Challenge 1 要求实现 **垃圾回收（GC）机制** ：当 shard 迁移完成后，旧 group 应该删除不再负责的 shard 数据。

### 核心挑战

GC 的难点在于： **什么时候可以安全地删除旧数据？**

```
Config N:   Shard S belongs to G1
Config N+1: Shard S belongs to G2

           G1                              G2
           │                               │
Config N+1 │ Stop serving S                │ Start pulling S
applied    │ Can I delete S data now?      │
           │ ─ ─ ─ ─ ─ ─ NO! ─ ─ ─ ─ ─ ─ ─ │ G2 hasn't received data yet!
           │                               │
           │ ◄─────────────────────────────│ Migrate RPC (fetch S data)
           │ ─────────────────────────────►│ 
           │                               │
           │ Can I delete S data now?      │ OpMigrate applied
           │ ─ ─ ─ ─ ─ STILL NO! ─ ─ ─ ─ ─ │ What if G1 crashes before GC?
           │                               │ G2 needs to re-fetch on recovery
           │                               │
           │ ◄─────────────────────────────│ GC RPC (confirm receipt)
           │ Apply OpGC, delete S data     │
           │ NOW it's safe to delete!      │
```

**关键点**
- G1 不能在 G2 确认收到数据之前删除
- 需要处理 G1 崩溃恢复的情况
- 需要通过 Raft 保证 GC 操作的一致性

### 设计方案

采用 **Pull + Confirm** 模式：

1. **新 group 拉取数据**：G2 通过 Migrate RPC 从 G1 拉取 shard 数据
2. **新 group 确认收到**：G2 收到数据并 apply 后，发送 GC RPC 给 G1
3. **旧 group 删除数据**：G1 收到 GC 确认后，通过 Raft 提交 OpGC 删除数据

### 数据结构设计

#### 新增操作类型

```go
const (
    OpGC = "GC"  // 垃圾回收：删除旧 shard 数据
)
```

#### 新增 RPC

```go
// GC RPC: 新 group 通知旧 group 可以删除 shard 数据
type GCArgs struct {
    Shard     int
    ConfigNum int  // 配置版本号，用于幂等性
}

type GCReply struct {
    Err Err
}
```

#### ShardKV 扩展字段

```go
type ShardKV struct {
    // ... existing fields ...

    // GC 相关：记录等待被 GC 的 shard（configNum -> set of shards）
    // 当旧 group 失去 shard 时，记录在这里；收到 GC 确认后删除
    gcWaitList map[int]map[int]bool
}
```

### 核心实现

#### 1. 配置变更时记录待 GC 的 shard

当 group 失去某个 shard 时，将其加入 `gcWaitList` ：

```go
func (kv *ShardKV) applyConfig(newConfig shardmaster.Config) {
    // ... 验证配置顺序 ...

    // 检查是否所有需要拉取的 shard 都已完成
    for shard := 0; shard < shardmaster.NShards; shard++ {
        if kv.shardState[shard] == ShardPulling {
            return
        }
    }

    // 检查是否所有需要 GC 的 shard 都已完成
    // 只有当前配置的 GC 全部完成后，才能进入下一个配置
    if len(kv.gcWaitList[kv.config.Num]) > 0 {
        return
    }

    // 保存旧配置
    kv.prevConfig = kv.config

    // 记录需要 GC 的 shard
    shardsToGC := make(map[int]bool)
    for shard := 0; shard < shardmaster.NShards; shard++ {
        oldGid := kv.config.Shards[shard]
        newGid := newConfig.Shards[shard]

        if newGid == kv.gid {
            // 需要从其他 group 拉取
            if oldGid != kv.gid && oldGid != 0 {
                kv.shardState[shard] = ShardPulling
            } else {
                kv.shardState[shard] = ShardServing
            }
        } else {
            if oldGid == kv.gid {
                // 原来是我负责，现在给别人，加入 GC 等待列表
                shardsToGC[shard] = true
            }
            kv.shardState[shard] = ShardInvalid
        }
    }

    if len(shardsToGC) > 0 {
        kv.gcWaitList[newConfig.Num] = shardsToGC
    }

    kv.config = newConfig
}
```

#### 2. Migrate RPC 检查 gcWaitList

只有在 `gcWaitList` 中的 shard 才能被拉取（数据还在）：

```go
func (kv *ShardKV) Migrate(args *MigrateArgs, reply *MigrateReply) {
    kv.mu.Lock()
    defer kv.mu.Unlock()

    if _, isLeader := kv.rf.GetState(); !isLeader {
        reply.Err = ErrWrongLeader
        return
    }

    if kv.config.Num < args.ConfigNum {
        reply.Err = ErrNotReady
        return
    }

    // 检查 shard 是否在 gcWaitList 中
    shards, ok := kv.gcWaitList[args.ConfigNum]
    if !ok || !shards[args.Shard] {
        // 已经 GC 过了，返回空数据（幂等性）
        reply.Data = make(map[string]string)
        reply.LastReplies = make(map[int64]LastReply)
        reply.Err = OK
        return
    }

    // 深拷贝 shard 数据
    reply.Data = make(map[string]string)
    for k, v := range kv.data[args.Shard] {
        reply.Data[k] = v
    }

    reply.LastReplies = make(map[int64]LastReply)
    for clientId, lr := range kv.lastReplies {
        reply.LastReplies[clientId] = lr
    }

    reply.Err = OK
}
```

#### 3. GC RPC 处理

新 group 发送 GC 确认，旧 group 提交 OpGC 到 Raft：

```go
func (kv *ShardKV) GC(args *GCArgs, reply *GCReply) {
    kv.mu.Lock()

    if _, isLeader := kv.rf.GetState(); !isLeader {
        kv.mu.Unlock()
        reply.Err = ErrWrongLeader
        return
    }

    if kv.config.Num < args.ConfigNum {
        kv.mu.Unlock()
        reply.Err = ErrNotReady
        return
    }

    // 检查是否在 gcWaitList 中，不在说明已 GC 过
    shards, ok := kv.gcWaitList[args.ConfigNum]
    if !ok || !shards[args.Shard] {
        kv.mu.Unlock()
        reply.Err = OK  // 幂等性
        return
    }
    kv.mu.Unlock()

    // 提交 GC 操作到 Raft
    op := Op{
        Type:      OpGC,
        Shard:     args.Shard,
        ConfigNum: args.ConfigNum,
    }

    index, _, isLeader := kv.rf.Start(op)
    if !isLeader {
        reply.Err = ErrWrongLeader
        return
    }

    ch := kv.getWaitCh(index)
    select {
    case <-ch:
        reply.Err = OK
    case <-time.After(500 * time.Millisecond):
        reply.Err = ErrWrongLeader
    }

    kv.mu.Lock()
    delete(kv.waitChs, index)
    kv.mu.Unlock()
}
```

#### 4. 应用 GC 操作

```go
func (kv *ShardKV) applyGC(op Op) {
    // 检查是否在 gcWaitList 中
    if shards, ok := kv.gcWaitList[op.ConfigNum]; ok {
        if shards[op.Shard] {
            // 删除 shard 数据
            kv.data[op.Shard] = make(map[string]string)
            // 从 gcWaitList 中移除
            delete(shards, op.Shard)
            if len(shards) == 0 {
                delete(kv.gcWaitList, op.ConfigNum)
            }
        }
    }
}
```

#### 5. 拉取数据后发送 GC

在 `fetchShard` 成功后，异步发送 GC 通知：

```go
func (kv *ShardKV) fetchShard(shard int, gid int, oldConfig shardmaster.Config, configNum int) {
    servers, ok := oldConfig.Groups[gid]
    if !ok {
        return
    }

    args := MigrateArgs{
        Shard:     shard,
        ConfigNum: configNum,
    }

    for _, serverName := range servers {
        srv := kv.make_end(serverName)
        var reply MigrateReply
        ok := srv.Call("ShardKV.Migrate", &args, &reply)
        if ok && reply.Err == OK {
            // 拉取成功，提交到 Raft
            op := Op{
                Type:        OpMigrate,
                Shard:       shard,
                Data:        reply.Data,
                LastReplies: reply.LastReplies,
                ConfigNum:   configNum,
            }
            kv.rf.Start(op)

            // 异步发送 GC 通知
            go kv.sendGC(shard, gid, oldConfig, configNum)
            return
        }
    }
}
```

#### 6. GC 通知重试机制

使用 `gcNotifier` goroutine 定期重试，处理 GC 请求可能失败的情况：

```go
func (kv *ShardKV) gcNotifier() {
    for !kv.killed() {
        if _, isLeader := kv.rf.GetState(); isLeader {
            kv.mu.Lock()
            prevConfig := kv.prevConfig
            currentConfig := kv.config

            type gcTask struct {
                shard     int
                gid       int
                configNum int
            }
            var tasks []gcTask

            // 只有 prevConfig 和 currentConfig 相邻时才发送 GC
            if prevConfig.Num > 0 && currentConfig.Num == prevConfig.Num+1 {
                for shard := 0; shard < shardmaster.NShards; shard++ {
                    oldGid := prevConfig.Shards[shard]
                    newGid := currentConfig.Shards[shard]

                    // 当前是我负责，上个配置是别的 group 负责
                    // 且已经是 Serving 状态（迁移完成）
                    if newGid == kv.gid && oldGid != kv.gid && oldGid != 0 &&
                        kv.shardState[shard] == ShardServing {
                        tasks = append(tasks, gcTask{shard, oldGid, currentConfig.Num})
                    }
                }
            }
            kv.mu.Unlock()

            for _, task := range tasks {
                go kv.sendGC(task.shard, task.gid, prevConfig, task.configNum)
            }
        }
        time.Sleep(100 * time.Millisecond)
    }
}
```

### 快照支持

`gcWaitList` 需要持久化到快照中：

```go
func (kv *ShardKV) takeSnapshot(index int) {
    w := new(bytes.Buffer)
    e := labgob.NewEncoder(w)
    e.Encode(kv.config)
    e.Encode(kv.prevConfig)
    e.Encode(kv.shardState)
    e.Encode(kv.data)
    e.Encode(kv.lastReplies)
    e.Encode(kv.gcWaitList)  // 新增
    kv.rf.Snapshot(index, w.Bytes())
}

func (kv *ShardKV) readSnapshotL(snapshot []byte) {
    // ... decode other fields ...

    // gcWaitList 是后来添加的字段，可能不存在于旧快照中
    var gcWaitList map[int]map[int]bool
    if d.Decode(&gcWaitList) == nil && gcWaitList != nil {
        kv.gcWaitList = gcWaitList
    }
}
```

### 状态转换图

```
                    applyConfig (lose shard)
    ┌─────────────────────────────────────────────┐
    │                                             │
    ▼                                             │
┌────────┐                                   ┌────────┐
│Serving │                                   │Invalid │
└────────┘                                   └────────┘
    │                                             ▲
    │ applyConfig (gain shard)                    │
    ▼                                             │
┌────────┐     applyMigrate      ┌────────┐       │
│Pulling │ ─────────────────────►│Serving │       │
└────────┘                       └────────┘       │
                                      │           │
                                      │ (GC sent  │
                                      │  to old   │
                                      │  group)   │
                                      │           │
                                      └───────────┘

Old Group (loses shard):
  Serving -> Invalid (state change)
  Data added to gcWaitList[configNum]
  Data deleted when OpGC applied

New Group (gains shard):
  Invalid -> Pulling -> Serving
  Sends GC after Migrate completes
```

### 配置推进条件

要推进到下一个配置，必须满足：
1. 所有 `ShardPulling` 状态的 shard 都已完成迁移
2. 当前配置的 `gcWaitList` 为空（所有给出的 shard 都已被 GC）

```go
func (kv *ShardKV) applyConfig(newConfig shardmaster.Config) {
    // 条件 1: 所有 Pulling 完成
    for shard := 0; shard < shardmaster.NShards; shard++ {
        if kv.shardState[shard] == ShardPulling {
            return
        }
    }

    // 条件 2: 所有 GC 完成
    if len(kv.gcWaitList[kv.config.Num]) > 0 {
        return
    }

    // 可以推进配置
    // ...
}
```

### 测试结果

```bash
$ go test -race -run 'TestChallenge1'
Test: shard deletion (challenge 1) ...
  ... Passed
Test: concurrent configuration change and restart (challenge 1)...
  ... Passed
PASS
ok      shardkv    34.013s
```

全部 Lab 4 测试（包括 Challenge）：

```bash
$ go test -race
Test: static shards ...
  ... Passed
Test: join then leave ...
  ... Passed
Test: snapshots, join, and leave ...
  ... Passed
Test: servers miss configuration changes...
  ... Passed
Test: concurrent puts and configuration changes...
  ... Passed
Test: more concurrent puts and configuration changes...
  ... Passed
Test: unreliable 1...
  ... Passed
Test: unreliable 2...
  ... Passed
Test: unreliable 3...
  ... Passed
Test: shard deletion (challenge 1) ...
  ... Passed
Test: concurrent configuration change and restart (challenge 1)...
  ... Passed
Test: unaffected shard access (challenge 2) ...
  ... Passed
Test: partial migration shard access (challenge 2) ...
  ... Passed
PASS
ok      shardkv    102.850s
```

### 遇到的问题与解决

#### 问题 1：死锁导致测试超时

**现象** ：TestConcurrent1 超时，系统卡住。

**原因** ：最初使用 `ShardBePulling` 状态来标记等待 GC 的 shard，但这导致了复杂的状态管理。当 GC 通知失败或延迟时，系统无法正确推进配置。

**解决方案** ：改用 `gcWaitList` map 来追踪待 GC 的 shard。这样：
- shard 状态直接变为 `Invalid`（停止服务）
- 数据保留直到 GC 确认
- 配置推进依赖 `gcWaitList` 而非 shard 状态

#### 问题 2：GC 通知无限循环

**现象** ：`gcNotifier` 不断发送相同的 GC 请求。

**原因** ：新 group 发送 GC 后，没有更新任何状态来标记"已发送"。

**解决方案** ：
- 旧 group 收到 GC 后从 `gcWaitList` 删除，后续 GC 请求直接返回 OK（幂等性）
- `gcNotifier` 只是作为重试机制，确保网络问题不会导致 GC 永远失败
- 当配置推进后，`prevConfig` 更新，不再满足 GC 条件

#### 问题 3：快照恢复兼容性

**现象** ：添加 `gcWaitList` 后，从旧快照恢复时解码失败。

**解决方案** ：`gcWaitList` 解码失败时不 return，而是使用初始化的空 map：

```go
if d.Decode(&gcWaitList) == nil && gcWaitList != nil {
    kv.gcWaitList = gcWaitList
}
// 不 return，保持已初始化的空 gcWaitList
```

### 设计思考

1. **为什么使用 configNum 作为 gcWaitList 的 key？**
   - 同一个 shard 可能在不同配置中被多次转移
   - 使用 configNum 可以准确标识"哪次转移"需要 GC
   - 保证幂等性：相同 (shard, configNum) 的 GC 只执行一次

2. **为什么 GC 需要通过 Raft？**
   - 保证 group 内所有节点一致地删除数据
   - leader 崩溃后，新 leader 仍能正确处理
   - 防止部分节点删除、部分节点保留导致不一致

3. **为什么不在 applyMigrate 时直接发送 GC？**
   - applyMigrate 在 applier goroutine 中执行，不应阻塞
   - GC 是 RPC 调用，可能失败需要重试
   - 分离关注点：迁移和 GC 是独立的操作

## Challenge 2: 配置变更期间的无中断服务

### 问题背景

在基础实现中，配置变更期间可能会阻塞所有客户端请求。Challenge 2 要求：

1. **Unaffected Shard Access** ：不受配置变更影响的 shard 应继续正常服务
2. **Partial Migration** ：当一个 shard 迁移完成后，应立即开始服务，即使其他 shard 还在迁移中

### 为什么 Challenge 2 被自然支持

核心原因是 **per-shard 的状态管理** 。

#### 1. 独立的 shard 状态

每个 shard 有自己独立的状态：

```go
type ShardKV struct {
    shardState [shardmaster.NShards]string  // 每个 shard 独立的状态
    // ...
}

const (
    ShardServing = "Serving" // 正常服务
    ShardPulling = "Pulling" // 需要从其他 group 拉取数据
    ShardInvalid = "Invalid" // 不负责该 shard
)
```

#### 2. 请求只检查特定 shard 的状态

`canServe()` 只检查目标 shard 的状态，不关心其他 shard：

```go
func (kv *ShardKV) canServe(shard int) bool {
    return kv.config.Shards[shard] == kv.gid && 
           kv.shardState[shard] == ShardServing
}
```

这意味着：
- 如果 shard 5 在 `Pulling`，但 shard 3 是 `Serving`，对 shard 3 的请求正常处理
- 不会因为某个 shard 正在迁移而阻塞其他 shard 的服务

#### 3. 迁移完成后立即可用

当 `applyMigrate` 完成时，该 shard 立即变为 `Serving`：

```go
func (kv *ShardKV) applyMigrate(op Op) {
    // ... 数据拷贝 ...
    
    // 更新状态为 Serving - 立即可以服务！
    kv.shardState[op.Shard] = ShardServing
}
```

不需要等待其他 shard 迁移完成。

### 状态流转图

```
配置变更期间的 shard 状态（以 Group B 视角）：

Config N → Config N+1: B 获得 S1, S2, S3 三个 shard

时间线：
─────────────────────────────────────────────────────────►

S1: Invalid ──────► Pulling ──► [迁移完成] ──► Serving ──►
                                     │
                                     └─ 此时 S1 可服务！

S2: Invalid ──────► Pulling ─────────────────► [迁移完成] ──► Serving
                                                     │
                                                     └─ 此时 S2 可服务！

S3: Invalid ──────► Pulling ──────────────────────────────► [迁移完成] ──► Serving

原有 shard：
S4: Serving ──────────────────────────────────────────────────────────► Serving
              │
              └─ 全程不受影响，持续提供服务
```

### 设计要点

#### 1. 不阻塞整个 group

传统设计可能会：
```go
// 错误做法：配置变更期间阻塞所有请求
if kv.inConfigTransition {
    return ErrConfigChanging
}
```

正确做法（我们的实现）：
```go
// 只检查特定 shard 的状态
if !kv.canServe(shard) {
    return ErrWrongGroup
}
```

#### 2. 配置推进不依赖"全部 shard 就绪"

```go
func (kv *ShardKV) applyConfig(newConfig shardmaster.Config) {
    // 检查是否所有需要拉取的 shard 都已完成
    for shard := 0; shard < shardmaster.NShards; shard++ {
        if kv.shardState[shard] == ShardPulling {
            return  // 还有 shard 在 Pulling，不能推进配置
        }
    }
    // ...
}
```

注意：这里的检查是为了确保配置推进的正确性，但 **不影响已经 Serving 的 shard 继续服务** 。

#### 3. 迁移器独立工作

`migrator` goroutine 独立为每个 Pulling 状态的 shard 发起迁移：

```go
func (kv *ShardKV) migrator() {
    for !kv.killed() {
        // 找出所有需要拉取的 shard
        for shard := 0; shard < shardmaster.NShards; shard++ {
            if kv.shardState[shard] == ShardPulling {
                // 为每个 shard 独立发起拉取（并行）
                go kv.fetchShard(shard, gid, oldConfig, configNum)
            }
        }
        // ...
    }
}
```

### 测试结果

```bash
$ go test -race -run 'TestChallenge2'
Test: unaffected shard access (challenge 2) ...
  ... Passed
Test: partial migration shard access (challenge 2) ...
  ... Passed
PASS
ok      shardkv    11.757s
```

## Lab 4 设计要点总结

### 1. 分片系统的核心抽象

#### 三层架构

```
┌────────────────────────────────────────────────────────────┐
│                    Shard Master                            │
│            (配置管理：shard → group 映射)                    │
├────────────────────────────────────────────────────────────┤
│     ShardKV Group 1    │    ShardKV Group 2    │   ...     │
│     (Raft replicated)  │    (Raft replicated)  │           │
├────────────────────────────────────────────────────────────┤
│                         Raft                               │
│              (共识层：日志复制 + leader 选举)                 │
└────────────────────────────────────────────────────────────┘
```

#### 状态流转

```
Shard 生命周期：

Invalid ──(配置变更：获得)──► Pulling ──(迁移完成)──► Serving
    ▲                                                  |
    └──────────(配置变更：失去 + GC 确认)─────────────────┘
```

### 2. 关键设计决策

#### Pull vs Push 模型

| 模型 | 优点 | 缺点 |
|------|------|------|
| **Pull** （本实现采用） | 新 group 主动控制进度；不依赖旧 group 可用性 | 需要旧 group 保留数据直到被拉取 |
| Push | 旧 group 推送后立即可删除 | 依赖新 group 可用；难以重试 |

#### 配置顺序处理

```go
if newConfig.Num != kv.config.Num+1 {
    return  // 必须严格按顺序处理配置
}
```

**为什么？** 跳过配置可能导致：
- 数据丢失（shard 经过中间 group）
- 一致性问题（不同节点处理不同配置序列）

#### GC 确认机制

```
新 Group                              旧 Group
   │                                     │
   │  Migrate RPC                        │
   │────────────────────────────────────►│
   │◄────────────────────────────────────│ 数据
   │                                     │
   │  Apply OpMigrate                    │
   │  (shard ─► Serving)                 │
   │                                     │
   │  GC RPC                             │
   │────────────────────────────────────►│
   │                                     │ Apply OpGC
   │                                     │ (删除数据，解除阻塞)
```

### 3. 并发控制策略

#### 单一 Mutex 原则

```go
type ShardKV struct {
    mu sync.Mutex  // 唯一的锁
    // ... 所有状态都由这把锁保护
}
```

**优点：**
- 避免死锁（多锁交叉）
- 简化推理（持有锁 = 独占访问）
- 便于 debug

**RPC Handler 模式：**

```go
func (kv *ShardKV) Get(args, reply) {
    kv.mu.Lock()
    // 1. 快速检查（去重、canServe）
    kv.mu.Unlock()
    
    // 2. 提交到 Raft（不持有锁！）
    index, _, isLeader := kv.rf.Start(op)
    
    // 3. 等待结果
    ch := kv.getWaitCh(index)
    select { ... }
    
    kv.mu.Lock()
    // 4. 清理
    kv.mu.Unlock()
}
```

#### 后台 Goroutine 设计

| Goroutine | 职责 | 运行频率 |
|-----------|------|---------|
| `applier` | 应用 Raft 日志 | 持续监听 applyCh |
| `configPoller` | 轮询新配置 | 100ms |
| `migrator` | 拉取 shard 数据 | 50ms |
| `gcNotifier` | 发送 GC 确认 | 100ms |

### 4. 容错与幂等性

#### RPC 幂等性保证

| RPC | 幂等性设计 |
|-----|-----------|
| `Get/Put/Append` | clientId + seqNum 去重 |
| `Migrate` | configNum + shard 检查 gcWaitList |
| `GC` | 已删除的 shard 直接返回 OK |

#### 崩溃恢复

```go
// 所有需要恢复的状态都在快照中
func (kv *ShardKV) takeSnapshot(index int) {
    e.Encode(kv.config)
    e.Encode(kv.prevConfig)
    e.Encode(kv.shardState)
    e.Encode(kv.data)
    e.Encode(kv.lastReplies)
    e.Encode(kv.gcWaitList)
}
```

## 综合总结：MIT 6.824 Raft 实现要点

结合 Lab 2、3、4 的经验，总结分布式系统实现的关键原则：

### 1. 架构分层

```
┌─────────────────────────────────────────────────────────────┐
│  Application Layer (Lab 4: ShardKV / Lab 3: KVRaft)         │
│  - 业务逻辑                                                  │
│  - 去重机制 (clientId + seqNum)                              │
│  - 分片管理 (Lab 4)                                          │
├─────────────────────────────────────────────────────────────┤
│  Consensus Layer (Lab 2: Raft)                              │
│  - Leader 选举                                               │
│  - 日志复制                                                  │
│  - 安全性保证                                                │
├─────────────────────────────────────────────────────────────┤
│  Persistence Layer                                          │
│  - 日志持久化                                                │
│  - 快照                                                     │
└─────────────────────────────────────────────────────────────┘
```

**核心原则：层间通过 channel 通信，层内状态自己管理**

### 2. 锁的使用原则

#### 原则 1：单一 Mutex

```go
// 推荐
type Server struct {
    mu sync.Mutex  // 唯一的锁
}

// 避免
type Server struct {
    stateMu  sync.Mutex
    logMu    sync.RWMutex
    configMu sync.Mutex
    // 多锁容易死锁
}
```

#### 原则 2：持有锁的时间最小化

```go
// 好的模式
func (s *Server) handleRPC() {
    s.mu.Lock()
    // 快速读取/修改状态
    s.mu.Unlock()
    
    // 耗时操作（网络调用、等待）不持有锁
    result := s.rf.Start(op)
    
    s.mu.Lock()
    // 处理结果
    s.mu.Unlock()
}
```

#### 原则 3：不在持有锁时发送 RPC

```go
// 错误
func (s *Server) broadcast() {
    s.mu.Lock()
    defer s.mu.Unlock()
    for _, peer := range s.peers {
        peer.Call("RPC", args, reply)  // 死锁风险！
    }
}

// 正确
func (s *Server) broadcast() {
    s.mu.Lock()
    args := s.prepareArgs()  // 准备参数
    s.mu.Unlock()
    
    for _, peer := range s.peers {
        go func(p *Peer) {
            p.Call("RPC", args, reply)
            s.mu.Lock()
            s.handleReply(reply)
            s.mu.Unlock()
        }(peer)
    }
}
```

### 3. Goroutine 设计模式

#### 模式 1：定时器驱动

```go
func (s *Server) ticker() {
    for !s.killed() {
        s.mu.Lock()
        s.tick()  // 检查超时、触发动作
        s.mu.Unlock()
        time.Sleep(interval)
    }
}
```

#### 模式 2：事件驱动

```go
func (s *Server) applier() {
    for msg := range s.applyCh {
        s.mu.Lock()
        s.apply(msg)
        s.mu.Unlock()
    }
}
```

#### 模式 3：等待通道

```go
// 创建等待通道
ch := s.getWaitCh(index)

// 等待结果或超时
select {
case result := <-ch:
    // 处理成功
case <-time.After(timeout):
    // 处理超时
}
```

### 4. 幂等性设计

```
                    ┌──────────────────────────────────┐
                    │       幂等性三层保障               │
                    ├──────────────────────────────────┤
Client 层:          │ clientId + seqNum 去重            │
                    │ - 每个 client 唯一 ID             │
                    │ - 每个请求单调递增序号              │
                    ├──────────────────────────────────┤
Raft 层:            │ 只 apply 已 commit 的日志          │
                    │ - 保证所有节点执行相同序列           │
                    ├──────────────────────────────────┤
Application 层:     │ 记录 lastReplies                  │
                    │ - 重复请求直接返回缓存结果           │
                    └──────────────────────────────────┘
```

### 5. 调试技巧

#### 使用 DPrintf

```go
const Debug = 1

func DPrintf(format string, a ...interface{}) {
    if Debug > 0 {
        log.Printf(format, a...)
    }
}

// 关键位置添加日志
DPrintf("[%d] term=%d state=%v", rf.me, rf.currentTerm, rf.state)
```

#### 检查 race condition

```bash
go test -race -run TestName
```

#### 重复运行测试

```bash
for i in {1..10}; do go test -race -run TestName || break; done
```

### 6. 常见陷阱

#### 陷阱 1：忘记深拷贝

```go
// 错误：共享引用
reply.Data = kv.data[shard]

// 正确：深拷贝
reply.Data = make(map[string]string)
for k, v := range kv.data[shard] {
    reply.Data[k] = v
}
```

#### 陷阱 2：goroutine 泄漏

```go
// 确保 goroutine 能退出
for !s.killed() {
    // ...
}

// 使用 context 或 done channel
select {
case <-s.done:
    return
case <-time.After(interval):
    // ...
}
```

#### 陷阱 3：只提交当前 term 的日志

```go
// Raft 安全性要求
if rf.log[n].Term != rf.currentTerm {
    continue  // 不能直接提交旧 term 的日志
}
```

### 最终测试结果

```bash
# Lab 2: Raft
$ go test -race raft/...
PASS

# Lab 3: KVRaft  
$ go test -race kvraft/...
PASS

# Lab 4: Sharded KV
$ go test -race shardmaster/...
PASS

$ go test -race shardkv/...
Test: static shards ...
  ... Passed
Test: join then leave ...
  ... Passed
Test: snapshots, join, and leave ...
  ... Passed
Test: servers miss configuration changes...
  ... Passed
Test: concurrent puts and configuration changes...
  ... Passed
Test: more concurrent puts and configuration changes...
  ... Passed
Test: unreliable 1...
  ... Passed
Test: unreliable 2...
  ... Passed
Test: unreliable 3...
  ... Passed
Test: shard deletion (challenge 1) ...
  ... Passed
Test: concurrent configuration change and restart (challenge 1)...
  ... Passed
Test: unaffected shard access (challenge 2) ...
  ... Passed
Test: partial migration shard access (challenge 2) ...
  ... Passed
PASS
ok      shardkv    ~100s
```

至此，MIT 6.824 Lab 2-4 全部完成：
- **Lab 2** ：Raft 共识算法（选举、日志复制、持久化）
- **Lab 3** ：基于 Raft 的容错 KV 服务（线性一致性、去重、快照）
- **Lab 4** ：分片 KV 服务（配置管理、shard 迁移、垃圾回收、无中断服务）
