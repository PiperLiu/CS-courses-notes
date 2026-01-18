# MIT6.824: Distributed Systems


<!-- @import "[TOC]" {cmd="toc" depthFrom=3 depthTo=6 orderedList=false} -->

<!-- code_chunk_output -->

- [🏓课程资料](#课程资料)
- [🏸本仓库资源](#本仓库资源)
- [🎾课程索引](#课程索引)
  - [LEC 1: Introduction](#lec-1-introduction)
  - [LEC 2: RPC and Threads](#lec-2-rpc-and-threads)
  - [LEC 3: GFS](#lec-3-gfs)
  - [LEC 4: Primary-Backup Replication](#lec-4-primary-backup-replication)
  - [LEC 5: Go, Threads, and Raft, video, code samples](#lec-5-go-threads-and-raft-video-code-samples)
  - [LEC 6: Fault Tolerance: Raft (1)](#lec-6-fault-tolerance-raft-1)
  - [LEC 7: Fault Tolerance: Raft (2)](#lec-7-fault-tolerance-raft-2)
  - [LEC 8: Zookeeper](#lec-8-zookeeper)
  - [LEC 9: More Replication, CRAQ](#lec-9-more-replication-craq)
  - [LEC 10: Cloud Replicated DB, Aurora](#lec-10-cloud-replicated-db-aurora)
  - [LEC 11: Cache Consistency: Frangipani](#lec-11-cache-consistency-frangipani)
  - [LEC 12: Distributed Transactions](#lec-12-distributed-transactions)
  - [LEC 13: Spanner](#lec-13-spanner)
  - [LEC 14: Optimistic Concurrency Control](#lec-14-optimistic-concurrency-control)
  - [LEC 15: Big Data: Spark](#lec-15-big-data-spark)
  - [LEC 16: Cache Consistency: Memcached at Facebook](#lec-16-cache-consistency-memcached-at-facebook)
  - [LEC 17: Causal Consistency, COPS](#lec-17-causal-consistency-cops)
  - [LEC 18: Fork Consistency, Certificate Transparency](#lec-18-fork-consistency-certificate-transparency)
  - [LEC 19: Peer-to-peer: Bitcoin](#lec-19-peer-to-peer-bitcoin)
  - [LEC 20: Blockstack](#lec-20-blockstack)
  - [LEC 21: Project demos](#lec-21-project-demos)
- [🥏Lab 笔记索引](#lab-笔记索引)
  - [Lab 1: MapReduce](#lab-1-mapreduce)
  - [Lab 2: Raft](#lab-2-raft)
  - [Lab 3: Fault-tolerant Key/Value Service](#lab-3-fault-tolerant-keyvalue-service)
  - [Lab 4: Sharded Key/Value Service](#lab-4-sharded-keyvalue-service)

<!-- /code_chunk_output -->

### 🏓课程资料

官方课程安排与资源索引：
- [http://nil.csail.mit.edu/6.824/2020/schedule.html](http://nil.csail.mit.edu/6.824/2020/schedule.html)

大佬的中文文档与翻译链接：
- [https://zhuanlan.zhihu.com/p/166409783](https://zhuanlan.zhihu.com/p/166409783)

B 站视频：
- [MIT 6.824: 分布式系统 2020春（简体中文字幕）](https://www.bilibili.com/video/BV1iD4y1U7gu)

官方视频：
- [MIT 6.824: Distributed Systems](https://www.youtube.com/channel/UC_7WrbZTCODu1o_kfUMq88g)

### 🏸本仓库资源

- 资源比如课堂资料、作业要求在[docs/html/](./docs/assignment/)和[docs/lec/](./docs/lec/)中
- 实验说明：<a href="./html/6.824 Lab 1_ MapReduce.html">6.824 Lab 1_ MapReduce.html</a>

### 🎾课程索引

#### LEC 1: Introduction

- LEC 1: [Introduction](./lec/l01.txt), [video](http://nil.csail.mit.edu/6.824/2020/video/1.html)
- Preparation: [Read MapReduce (2004)](./lec/mapreduce.pdf)
- Assigned: <a href="./html/6.824 Lab 1_ MapReduce.html">Lab 1: MapReduce</a>

本节课的笔记：[./drafts/lec01.md](./drafts/lec01.md)
- [分布式系统的驱动力和挑战 Drivens and Challenges](./drafts/lec01.md#分布式系统的驱动力和挑战-drivens-and-challenges)
  - [大量的相互协作的计算机驱动力](./drafts/lec01.md#大量的相互协作的计算机驱动力)
  - [分布式系统的问题（挑战）](./drafts/lec01.md#分布式系统的问题挑战)
- [课程结构（Course Structure）](./drafts/lec01.md#课程结构course-structure)
- [分布式系统的抽象和实现工具 Abstraction and Implementation](./drafts/lec01.md#分布式系统的抽象和实现工具-abstraction-and-implementation)
- [分布式系统评判维度](./drafts/lec01.md#分布式系统评判维度)
  - [可扩展性 Scalability](./drafts/lec01.md#可扩展性-scalability)
  - [可用性 Availability](./drafts/lec01.md#可用性-availability)
  - [一致性 Consistency](./drafts/lec01.md#一致性-consistency)
- [MapReduce](./drafts/lec01.md#mapreduce)
  - [MapReduce 诞生背景](./drafts/lec01.md#mapreduce-诞生背景)
  - [Map Reduce 基本工作方式](./drafts/lec01.md#map-reduce-基本工作方式)
  - [Map 函数伪代码](./drafts/lec01.md#map-函数伪代码)
  - [Reduce 函数伪代码](./drafts/lec01.md#reduce-函数伪代码)
  - [关于 MapReduce 的问答](./drafts/lec01.md#关于-mapreduce-的问答)

#### LEC 2: RPC and Threads

- LEC 2: [RPC and Threads](./lec/l-rpc.txt), [crawler.go](./lec/crawler.go), [kv.go](./lec/kv.go), [video](http://nil.csail.mit.edu/6.824/2020/video/2.html)
- Preparation: Do [Online Go tutorial](http://tour.golang.org/) ([FAQ](tour-faq.txt))

我在 Go tutorial 把 Go 基本了解了，笔记放在 [back-end-notes](https://github.com/PiperLiu/back-end-notes/blob/master/notes/golang/a-tour-of-Go/README.md) 中了：
- [Basics](https://github.com/PiperLiu/back-end-notes/blob/master/notes/golang/a-tour-of-Go/README.md#basics)
  - [Packages, variables, and functions](https://github.com/PiperLiu/back-end-notes/blob/master/notes/golang/a-tour-of-Go/README.md#packages-variables-and-functions)
  - [Flow control statements: for, if, else, switch and defer](https://github.com/PiperLiu/back-end-notes/blob/master/notes/golang/a-tour-of-Go/README.md#flow-control-statements-for-if-else-switch-and-defer)
  - [More types: structs, slices, and maps](https://github.com/PiperLiu/back-end-notes/blob/master/notes/golang/a-tour-of-Go/README.md#more-types-structs-slices-and-maps)
- [Methods and interfaces](https://github.com/PiperLiu/back-end-notes/blob/master/notes/golang/a-tour-of-Go/README.md#methods-and-interfaces)
- [Generics](https://github.com/PiperLiu/back-end-notes/blob/master/notes/golang/a-tour-of-Go/README.md#generics)
- [Concurrency](https://github.com/PiperLiu/back-end-notes/blob/master/notes/golang/a-tour-of-Go/README.md#concurrency)

课就不听了，把两个程序看懂似乎才是重点（课上也讲这两个代码）。语言这个东西还是知道思想和原理，然后便用边查最好。

在 [./lec/crawler.go](./lec/crawler.go) 中，举了串行爬取、锁实现并行爬取、通道并行爬取三个例子，有这些要点：
- 在 `lock` 后立即 `defer unlock` 让程序不易出错，读起来也更美观
- `sync.WaitGroup` 的经典使用：有任务我就先 `wg.Add(1)` ，每个任务开始时 `defer wg.done(1)`
- `master-worked` 模式配合 `channel` 的经典使用

在 [./lec/kv.go](./lec/kv.go) 中，主要是对 `net/rpc` 做了经典实现。

#### LEC 3: GFS

- LEC 3: [GFS](./lec/l-gfs.txt), [video](http://nil.csail.mit.edu/6.824/2020/video/3.html)
- Preparation: Read [GFS (2003)](./lec/gfs.pdf) ([FAQ](./lec/gfs-faq.txt))
- Assigned: <a href="./html/6.824 Lab 2_ Raft.html">Lab 2: Raft</a>

本节课的笔记：[./drafts/lec03.md](./drafts/lec03.md)

#### LEC 4: Primary-Backup Replication

- LEC 4: [Primary-Backup Replication](./lec/l-vm-ft.txt), [video](http://nil.csail.mit.edu/6.824/2020/video/4.html)
- Preparation: Read [Fault-Tolerant Virtual Machines (2010)](./lec/vm-ft.pdf) ([FAQ](./lec/vm-ft-faq.txt))

本节课的笔记：[./drafts/lec04.md](./drafts/lec04.md)

#### LEC 5: Go, Threads, and Raft, video, code samples

- LEC 5: [Go, Threads, and Raft](./lec/l-go-concurrency.txt), [video](http://nil.csail.mit.edu/6.824/2020/video/5.html), [code samples](./lec/go-concurrency/)
- Preparation: [Read The Go Memory Model](https://go.dev/ref/mem)

本节课的笔记：[./drafts/lec05.md](./drafts/lec05.md)

#### LEC 6: Fault Tolerance: Raft (1)

- LEC 6: [Fault Tolerance: Raft (1)](./lec/l-raft.txt), [video](http://nil.csail.mit.edu/6.824/2020/video/6.html)
- Preparation: [Read Raft (extended) (2014), to end of Section 5](./lec/raft-extended.pdf) ([FAQ](./lec/raft-faq.txt))

本节课的笔记：[./drafts/lec06.md](./drafts/lec06.md)

#### LEC 7: Fault Tolerance: Raft (2)

- LEC 7: [Fault Tolerance: Raft (2)](./lec/l-raft2.txt), [video](http://nil.csail.mit.edu/6.824/2020/video/7.html)
- Preparation: [Read Raft (extended) (2014), Section 7 to end (but not Section 6)](./lec/raft-extended.pdf) ([FAQ](./lec/raft2-faq.txt))

本节课的笔记：[./drafts/lec07.md](./drafts/lec07.md)

#### LEC 8: Zookeeper

- LEC 8: [Zookeeper](./lec/l-zookeeper.txt), [video](http://nil.csail.mit.edu/6.824/2020/video/8.html)
- Preparation: Read [ZooKeeper (2010)](./lec/zookeeper.pdf) ([FAQ](./lec/zookeeper-faq.txt)) (Question)

本节课的笔记：[./drafts/lec08.md](./drafts/lec08.md)

#### LEC 9: More Replication, CRAQ

- LEC 9: [More Replication, CRAQ](./lec/l-craq.txt), [video](http://nil.csail.mit.edu/6.824/2020/video/9.html)
- Preparation: Read [CRAQ (2009)](./lec/craq.pdf) ([FAQ](./lec/craq-faq.txt))
- Assigned: <a href="./html/6.824 Lab 3_ Fault-tolerant Key_Value Service.html">Lab 3: KV Raft</a>

本节课程的笔记：[./drafts/lec09.md](./drafts/lec09.md)

#### LEC 10: Cloud Replicated DB, Aurora

- LEC 10: [Cloud Replicated DB, Aurora](./lec/l-aurora.txt), [video](http://nil.csail.mit.edu/6.824/2020/video/10.html)
- Preparation: Read [Aurora (2017)](./lec/aurora.pdf) ([FAQ](./lec/aurora-faq.txt))

本节课的笔记：[./drafts/lec10.md](./drafts/lec10.md)

#### LEC 11: Cache Consistency: Frangipani

- LEC 11: [Cache Consistency: Frangipani](./lec/l-frangipani.txt), [video](http://nil.csail.mit.edu/6.824/2020/video/11.html)
- Preparation: Read [Frangipani](./lec/thekkath-frangipani.pdf) ([FAQ](./lec/frangipani-faq.txt))

本节课的笔记：[./drafts/lec11.md](./drafts/lec11.md)

#### LEC 12: Distributed Transactions

- LEC 12: [Distributed Transactions](./lec/l-2pc.txt), [video](http://nil.csail.mit.edu/6.824/2020/video/12.html)
- Preparation: Read [6.033 Chapter 9](https://ocw.mit.edu/courses/res-6-004-principles-of-computer-system-design-an-introduction-spring-2009/pages/online-textbook/), [just 9.1.5, 9.1.6, 9.5.2, 9.5.3, 9.6.3](./lec/atomicity_open_5_0.pdf) ([FAQ](./lec/chapter9-faq.txt))
- Assigned: <a href="./html/6.824 Lab 4_ Sharded Key_Value Service.html">Lab 4: Sharded KV</a>

本节课的笔记：[./drafts/lec12.md](./drafts/lec12.md)

#### LEC 13: Spanner

- LEC 13: [Spanner](./lec/l-spanner.txt), [video](http://nil.csail.mit.edu/6.824/2020/video/13.html)
- Preparation: Read [Spanner (2012)](./lec/spanner.pdf) ([FAQ](./lec/spanner-faq.txt))

本节课的笔记：[./drafts/lec13.md](./drafts/lec13.md)

#### LEC 14: Optimistic Concurrency Control

- LEC 14: [Optimistic Concurrency Control](./lec/l-farm.txt), [video](http://nil.csail.mit.edu/6.824/2020/video/14.html)
- Preparation: [Read FaRM (2015)](./lec/farm-2015.pdf) ([FAQ](./lec/farm-faq.txt))

本节课的笔记：[./drafts/lec14.md](./drafts/lec14.md)

#### LEC 15: Big Data: Spark

- LEC 15: [Big Data: Spark](./lec/l-spark.txt), [video](http://nil.csail.mit.edu/6.824/2020/video/15.html)
- Preparation: Read [Spark (2012)](./lec/zaharia-spark.pdf) ([FAQ](./lec/spark-faq.txt))

本节课的笔记：[./drafts/lec15.md](./drafts/lec15.md)

#### LEC 16: Cache Consistency: Memcached at Facebook

- LEC 16: [Cache Consistency: Memcached at Facebook](./lec/l-memcached.txt), [video](http://nil.csail.mit.edu/6.824/2020/video/17.html)
- Preparation: Read [Memcached at Facebook (2013)](./lec/memcache-fb.pdf) ([FAQ](./lec/memcache-faq.txt))

本节课的笔记：[./drafts/lec16.md](./drafts/lec16.md)

#### LEC 17: Causal Consistency, COPS

- LEC 17: [Causal Consistency, COPS](./lec/l-cops.txt), [video](http://nil.csail.mit.edu/6.824/2020/video/17.html)
- Preparation: Read [COPS (2011)](./lec/cops.pdf)

本节课的笔记：[./drafts/lec17.md](./drafts/lec17.md)

#### LEC 18: Fork Consistency, Certificate Transparency

- LEC 18: [Fork Consistency, Certificate Transparency](./lec/l-ct.txt), [video](http://nil.csail.mit.edu/6.824/2020/video/18.html)
- Preparation: Read [Certificate Transparency](https://certificate.transparency.dev/), [Also This](https://www.certificate-transparency.org/how-ct-works), [And This](https://research.swtch.com/tlog), but skip the Tiles sections and the appendices. ([FAQ](./lec/ct-faq.txt))

本节课的笔记：[./drafts/lec18.md](./drafts/lec18.md)

#### LEC 19: Peer-to-peer: Bitcoin

- LEC 19: Peer-to-peer: [Bitcoin](./lec/l-bitcoin.txt), [video](http://nil.csail.mit.edu/6.824/2020/video/17.html)
- Preparation: Read [Bitcoin (2008)](./lec/bitcoin.pdf), and [summary](https://michaelnielsen.org/ddi/how-the-bitcoin-protocol-actually-works/) ([FAQ](./lec/bitcoin-faq.txt))

本节课的笔记：[./drafts/lec19.md](./drafts/lec19.md)

#### LEC 20: Blockstack

- LEC 20: [Blockstack](./lec/l-blockstack.txt), [video](http://nil.csail.mit.edu/6.824/2020/video/20.html)
- Preparation: Read [BlockStack (2017)](./lec/blockstack-2017.pdf) ([FAQ](./lec/blockstack-faq.txt))

本节课的笔记：[./drafts/lec20.md](./drafts/lec20.md)

#### LEC 21: Project demos

- Preparation: Read [AnalogicFS experience paper](./lec/katabi-analogicfs.pdf) ([FAQ](./lec/analogicfs-faq.txt))
- 这里额外记录下当年的考试内容：[Q1](./lec/Q1.txt), [Q2](./lec/Q2.txt)

本节课的笔记：[./drafts/lec21.md](./drafts/lec21.md)

### 🥏Lab 笔记索引

关于实验的环境配置见这里：[./drafts/lab00.md](./drafts/lab00.md)

#### Lab 1: MapReduce

[./drafts/lab01.md](./drafts/lab01.md)
- [所有的 test 都过了](./drafts/lab01.md#所有的-test-都过了)
- [大体思路](./drafts/lab01.md#大体思路)
- [与论文的区别](./drafts/lab01.md#与论文的区别)
- [一些其他收获](./drafts/lab01.md#一些其他收获)
  - [go: compile to so file AND load so file](./drafts/lab01.md#go-compile-to-so-file-and-load-so-file)
  - [防止文件操作冲突，先写到 temp/xxx 文件中，写完再拷贝](./drafts/lab01.md#防止文件操作冲突先写到-tempxxx-文件中写完再拷贝)
  - [Map+Reduce 总体给我的感觉就是分布式 groupby ？](./drafts/lab01.md#mapreduce-总体给我的感觉就是分布式-groupby-)
  - [Scala 中 groupMap 与 groupMapReduce](./drafts/lab01.md#scala-中-groupmap-与-groupmapreduce)

#### Lab 2: Raft

[./drafts/lab02.md](./drafts/lab02.md)

- [2A 实现 Leader 选举](./drafts/lab02.md#2a-实现-leader-选举)
  - [架构设计探索](./drafts/lab02.md#架构设计探索)
  - [最终实现方案](./drafts/lab02.md#最终实现方案)
    - [1. 状态机结构](./drafts/lab02.md#1-状态机结构)
    - [2. 主循环设计](./drafts/lab02.md#2-主循环设计)
    - [3. 角色转换逻辑](./drafts/lab02.md#3-角色转换逻辑)
    - [4. 时间参数设置](./drafts/lab02.md#4-时间参数设置)
  - [遇到的问题与解决](./drafts/lab02.md#遇到的问题与解决)
    - [问题 1: 死锁](./drafts/lab02.md#问题-1-死锁)
    - [问题 2: Split Vote 死锁](./drafts/lab02.md#问题-2-split-vote-死锁)
    - [问题 3: Ticker 切换问题](./drafts/lab02.md#问题-3-ticker-切换问题)
  - [核心 RPC 实现与决策逻辑详解](./drafts/lab02.md#核心-rpc-实现与决策逻辑详解)
    - [RequestVote RPC Handler（被请求方）](./drafts/lab02.md#requestvote-rpc-handler被请求方)
    - [startElection（发起选举，请求方）](./drafts/lab02.md#startelection发起选举请求方)
    - [AppendEntries RPC (心跳)](./drafts/lab02.md#appendentries-rpc-心跳)
  - [测试结果](./drafts/lab02.md#测试结果)
  - [ChatGPT 5.2 介绍 etcd-io/raft 的方案：驱动型 goroutine / event-loop](./drafts/lab02.md#chatgpt-52-介绍-etcd-ioraft-的方案驱动型-goroutine--event-loop)
    - [1）设计哲学：只做“Raft 算法内核”，IO 全交给使用者](./drafts/lab02.md#1设计哲学只做raft-算法内核io-全交给使用者)
    - [2）三层结构：`raft`（纯状态机） / `RawNode`（薄封装） / `Node`（并发友好外壳）](./drafts/lab02.md#2三层结构raft纯状态机--rawnode薄封装--node并发友好外壳)
    - [3）关键交互协议：`Ready()` →（持久化/发送/apply）→ `Advance()`](./drafts/lab02.md#3关键交互协议ready-持久化发送apply-advance)
    - [4）时间驱动：`Tick()` + 抽象 tick（不是直接用 time.Time）](./drafts/lab02.md#4时间驱动tick--抽象-tick不是直接用-timetime)
    - [5）消息驱动：`Step()` / `Propose()` / `Campaign()`](./drafts/lab02.md#5消息驱动step--propose--campaign)
    - [6）典型 event-loop 长相（把 Ready 当成“输出”，Tick/Step/Propose 当成“输入”）](./drafts/lab02.md#6典型-event-loop-长相把-ready-当成输出ticksteppropose-当成输入)
- [2B 实现日志状态同步](./drafts/lab02.md#2b-实现日志状态同步)
  - [2B 测试用例分析](./drafts/lab02.md#2b-测试用例分析)
  - [测试框架详解](./drafts/lab02.md#测试框架详解)
    - [Start() 函数的含义](./drafts/lab02.md#start-函数的含义)
    - [cfg.one() 函数](./drafts/lab02.md#cfgone-函数)
    - [cfg.nCommitted() 函数](./drafts/lab02.md#cfgncommitted-函数)
    - [数据流完整路径](./drafts/lab02.md#数据流完整路径)
    - [TestBasicAgree2B - 基本一致性测试](./drafts/lab02.md#testbasicagree2b---基本一致性测试)
    - [TestFailAgree2B - 网络分区恢复测试](./drafts/lab02.md#testfailagree2b---网络分区恢复测试)
    - [TestBackup2B - 快速日志回退测试](./drafts/lab02.md#testbackup2b---快速日志回退测试)
  - [实现方案](./drafts/lab02.md#实现方案)
    - [1. 数据结构扩展](./drafts/lab02.md#1-数据结构扩展)
    - [2. Start() 实现](./drafts/lab02.md#2-start-实现)
    - [3. AppendEntries RPC 完整实现](./drafts/lab02.md#3-appendentries-rpc-完整实现)
    - [4. commitIndex 更新和日志应用](./drafts/lab02.md#4-commitindex-更新和日志应用)
  - [关键概念 FAQ](./drafts/lab02.md#关键概念-faq)
    - [Q1: "乐观初始化" nextIndex 是什么意思？](./drafts/lab02.md#q1-乐观初始化-nextindex-是什么意思)
    - [Q2: 日志不一致时如何处理？nextIndex 回退机制](./drafts/lab02.md#q2-日志不一致时如何处理nextindex-回退机制)
    - [Q3: "顺带提交" 是什么意思？](./drafts/lab02.md#q3-顺带提交-是什么意思)
    - [ChatGPT 5.2: 为什么不能直接提交旧 term 的日志？（Figure 8 讲的到底是什么）](./drafts/lab02.md#chatgpt-52-为什么不能直接提交旧-term-的日志figure-8-讲的到底是什么)
      - [1）Figure 8 面临的核心问题：“复制到多数”≠“Leader 能确认已经提交”](./drafts/lab02.md#1figure-8-面临的核心问题复制到多数leader-能确认已经提交)
      - [2）Figure 8 的时序（a→e）到底在演示什么](./drafts/lab02.md#2figure-8-的时序ae到底在演示什么)
      - [3）所以 Raft 的正确规则是什么？为什么它正确？](./drafts/lab02.md#3所以-raft-的正确规则是什么为什么它正确)
    - [5. becomeLeader 初始化](./drafts/lab02.md#5-becomeleader-初始化)
  - [测试结果](./drafts/lab02.md#测试结果-1)
- [2C 持久化](./drafts/lab02.md#2c-持久化)
  - [需要持久化的状态](./drafts/lab02.md#需要持久化的状态)
  - [persist() 和 readPersist() 实现](./drafts/lab02.md#persist-和-readpersist-实现)
  - [何时调用 persist()](./drafts/lab02.md#何时调用-persist)
  - [快速 nextIndex 回退优化](./drafts/lab02.md#快速-nextindex-回退优化)
    - [快速回退优化详细示例](./drafts/lab02.md#快速回退优化详细示例)
  - [遇到的问题与解决](./drafts/lab02.md#遇到的问题与解决-1)
    - [问题 1: 节点重启后日志被覆盖](./drafts/lab02.md#问题-1-节点重启后日志被覆盖)
    - [问题 2: applyLogs 中的并发问题](./drafts/lab02.md#问题-2-applylogs-中的并发问题)
      - [为什么原有逻辑不对？](./drafts/lab02.md#为什么原有逻辑不对)
      - [为什么要"不持有锁时发送 ApplyMsg"？](./drafts/lab02.md#为什么要不持有锁时发送-applymsg)
      - [正确的解决方案：专门的 applier goroutine](./drafts/lab02.md#正确的解决方案专门的-applier-goroutine)
  - [测试结果](./drafts/lab02.md#测试结果-2)

#### Lab 3: Fault-tolerant Key/Value Service

[./drafts/lab03.md](./drafts/lab03.md)

- [架构与要求：线性强一致性](./drafts/lab03.md#架构与要求线性强一致性)
- [3A: 基于 raft 的 k/v 服务](./drafts/lab03.md#3a-基于-raft-的-kv-服务)
  - [项目结构](./drafts/lab03.md#项目结构)
  - [第一步：单 client 实现](./drafts/lab03.md#第一步单-client-实现)
    - [核心设计思路](./drafts/lab03.md#核心设计思路)
    - [数据结构设计](./drafts/lab03.md#数据结构设计)
    - [Client (Clerk) 实现](./drafts/lab03.md#client-clerk-实现)
    - [Server RPC Handler 实现](./drafts/lab03.md#server-rpc-handler-实现)
    - [applier goroutine 实现](./drafts/lab03.md#applier-goroutine-实现)
  - [第二步：处理故障与重复请求](./drafts/lab03.md#第二步处理故障与重复请求)
    - [问题分析](./drafts/lab03.md#问题分析)
    - [解决方案：请求去重](./drafts/lab03.md#解决方案请求去重)
    - [数据结构设计](./drafts/lab03.md#数据结构设计-1)
    - [Client 实现](./drafts/lab03.md#client-实现)
    - [Server 去重逻辑](./drafts/lab03.md#server-去重逻辑)
    - [测试结果](./drafts/lab03.md#测试结果)
    - [关键点总结](./drafts/lab03.md#关键点总结)
- [3B: 基于日志压缩的快照](./drafts/lab03.md#3b-基于日志压缩的快照)
  - [第一步：修改 Raft 支持日志截断](./drafts/lab03.md#第一步修改-raft-支持日志截断)
    - [核心设计：逻辑索引 vs 物理索引](./drafts/lab03.md#核心设计逻辑索引-vs-物理索引)
    - [数据结构修改](./drafts/lab03.md#数据结构修改)
    - [索引转换辅助函数](./drafts/lab03.md#索引转换辅助函数)
    - [Snapshot 接口实现](./drafts/lab03.md#snapshot-接口实现)
    - [需要修改的关键位置](./drafts/lab03.md#需要修改的关键位置)
    - [AppendEntries 的特殊处理](./drafts/lab03.md#appendentries-的特殊处理)
    - [关键点总结](./drafts/lab03.md#关键点总结-1)
  - [第二步：修改 KVServer 支持快照](./drafts/lab03.md#第二步修改-kvserver-支持快照)
    - [核心设计思路](./drafts/lab03.md#核心设计思路-1)
    - [数据结构修改](./drafts/lab03.md#数据结构修改-1)
    - [takeSnapshot 实现](./drafts/lab03.md#takesnapshot-实现)
    - [readSnapshot 实现](./drafts/lab03.md#readsnapshot-实现)
    - [修改 applier：触发快照 + 处理 InstallSnapshot](./drafts/lab03.md#修改-applier触发快照--处理-installsnapshot)
    - [applySnapshot 实现](./drafts/lab03.md#applysnapshot-实现)
  - [第三步：实现 InstallSnapshot RPC](./drafts/lab03.md#第三步实现-installsnapshot-rpc)
    - [RPC 参数定义](./drafts/lab03.md#rpc-参数定义)
    - [InstallSnapshot RPC Handler](./drafts/lab03.md#installsnapshot-rpc-handler)
    - [sendSnapshot 实现](./drafts/lab03.md#sendsnapshot-实现)
    - [修改 broadcastAppendEntries ：判断何时发送快照](./drafts/lab03.md#修改-broadcastappendentries-判断何时发送快照)
  - [第四步：调试过程——一个隐蔽的 Bug](./drafts/lab03.md#第四步调试过程一个隐蔽的-bug)
    - [问题现象](./drafts/lab03.md#问题现象)
    - [排查过程](./drafts/lab03.md#排查过程)
    - [解决方案](./drafts/lab03.md#解决方案)
    - [测试结果](./drafts/lab03.md#测试结果-1)
  - [调试经验总结](./drafts/lab03.md#调试经验总结)
  - [Lab 3B 关键点总结](./drafts/lab03.md#lab-3b-关键点总结)

#### Lab 4: Sharded Key/Value Service

[./drafts/lab04.md](./drafts/lab04.md)

- [架构与要求：分片 K/V 服务](./drafts/lab04.md#架构与要求分片-kv-服务)
  - [为什么需要分片？](./drafts/lab04.md#为什么需要分片)
  - [关键概念： shard, replica group, configuration, shard master](./drafts/lab04.md#关键概念-shard-replica-group-configuration-shard-master)
  - [Raft 在哪里起作用？](./drafts/lab04.md#raft-在哪里起作用)
  - [Shard Master 的 RPC 接口](./drafts/lab04.md#shard-master-的-rpc-接口)
  - [客户端请求流程](./drafts/lab04.md#客户端请求流程)
  - [核心挑战：Reconfiguration（配置变更）](./drafts/lab04.md#核心挑战reconfiguration配置变更)
  - [理解测试代码：测试场景示例](./drafts/lab04.md#理解测试代码测试场景示例)
  - [Lab 4 的两个 Part](./drafts/lab04.md#lab-4-的两个-part)
  - [Challenge 挑战（可选）](./drafts/lab04.md#challenge-挑战可选)
  - [关键实现提示](./drafts/lab04.md#关键实现提示)
- [4A: Shard Master 实现](./drafts/lab04.md#4a-shard-master-实现)
  - [整体思路](./drafts/lab04.md#整体思路)
  - [数据结构设计](./drafts/lab04.md#数据结构设计)
    - [RPC 参数扩展](./drafts/lab04.md#rpc-参数扩展)
    - [Op 结构体](./drafts/lab04.md#op-结构体)
    - [ShardMaster 结构体](./drafts/lab04.md#shardmaster-结构体)
  - [Client 实现](./drafts/lab04.md#client-实现)
  - [Server RPC Handler 实现](./drafts/lab04.md#server-rpc-handler-实现)
  - [applier 实现](./drafts/lab04.md#applier-实现)
  - [核心：负载均衡算法](./drafts/lab04.md#核心负载均衡算法)
    - [算法思路](./drafts/lab04.md#算法思路)
    - [关键：确定性](./drafts/lab04.md#关键确定性)
    - [示例](./drafts/lab04.md#示例)
  - [处理 Join/Leave/Move](./drafts/lab04.md#处理-joinleavemove)
    - [Join](./drafts/lab04.md#join)
    - [Leave](./drafts/lab04.md#leave)
    - [Move](./drafts/lab04.md#move)
  - [深拷贝 Config](./drafts/lab04.md#深拷贝-config)
  - [测试结果](./drafts/lab04.md#测试结果)
- [4B: ShardKV 实现](./drafts/lab04.md#4b-shardkv-实现)
  - [整体思路](./drafts/lab04.md#整体思路-1)
  - [数据结构设计](./drafts/lab04.md#数据结构设计-1)
    - [RPC 结构体](./drafts/lab04.md#rpc-结构体)
    - [Shard 状态机](./drafts/lab04.md#shard-状态机)
    - [Op 结构体](./drafts/lab04.md#op-结构体-1)
    - [ShardKV 结构体](./drafts/lab04.md#shardkv-结构体)
  - [Client 实现](./drafts/lab04.md#client-实现-1)
  - [Server 核心实现](./drafts/lab04.md#server-核心实现)
    - [canServe：判断是否可以服务某个 shard](./drafts/lab04.md#canserve判断是否可以服务某个-shard)
    - [Get/PutAppend RPC Handler](./drafts/lab04.md#getputappend-rpc-handler)
    - [Migrate RPC Handler](./drafts/lab04.md#migrate-rpc-handler)
  - [applier 实现](./drafts/lab04.md#applier-实现-1)
    - [applyClientOp ：应用客户端操作](./drafts/lab04.md#applyclientop-应用客户端操作)
    - [applyConfig ：应用配置变更](./drafts/lab04.md#applyconfig-应用配置变更)
    - [applyMigrate ：应用 shard 迁移](./drafts/lab04.md#applymigrate-应用-shard-迁移)
  - [后台 Goroutine](./drafts/lab04.md#后台-goroutine)
    - [configPoller：定期获取配置](./drafts/lab04.md#configpoller定期获取配置)
    - [migrator：执行 shard 迁移](./drafts/lab04.md#migrator执行-shard-迁移)
  - [Snapshot 支持](./drafts/lab04.md#snapshot-支持)
  - [配置变更流程图](./drafts/lab04.md#配置变更流程图)
  - [关键设计决策](./drafts/lab04.md#关键设计决策)
  - [测试结果](./drafts/lab04.md#测试结果-1)
- [Challenge 1: Garbage Collection（垃圾回收）](./drafts/lab04.md#challenge-1-garbage-collection垃圾回收)
  - [问题背景](./drafts/lab04.md#问题背景)
  - [核心挑战](./drafts/lab04.md#核心挑战)
  - [设计方案](./drafts/lab04.md#设计方案)
  - [数据结构设计](./drafts/lab04.md#数据结构设计-2)
    - [新增操作类型](./drafts/lab04.md#新增操作类型)
    - [新增 RPC](./drafts/lab04.md#新增-rpc)
    - [ShardKV 扩展字段](./drafts/lab04.md#shardkv-扩展字段)
  - [核心实现](./drafts/lab04.md#核心实现)
    - [1. 配置变更时记录待 GC 的 shard](./drafts/lab04.md#1-配置变更时记录待-gc-的-shard)
    - [2. Migrate RPC 检查 gcWaitList](./drafts/lab04.md#2-migrate-rpc-检查-gcwaitlist)
    - [3. GC RPC 处理](./drafts/lab04.md#3-gc-rpc-处理)
    - [4. 应用 GC 操作](./drafts/lab04.md#4-应用-gc-操作)
    - [5. 拉取数据后发送 GC](./drafts/lab04.md#5-拉取数据后发送-gc)
    - [6. GC 通知重试机制](./drafts/lab04.md#6-gc-通知重试机制)
  - [快照支持](./drafts/lab04.md#快照支持)
  - [状态转换图](./drafts/lab04.md#状态转换图)
  - [配置推进条件](./drafts/lab04.md#配置推进条件)
  - [测试结果](./drafts/lab04.md#测试结果-2)
  - [遇到的问题与解决](./drafts/lab04.md#遇到的问题与解决)
    - [问题 1：死锁导致测试超时](./drafts/lab04.md#问题-1死锁导致测试超时)
    - [问题 2：GC 通知无限循环](./drafts/lab04.md#问题-2gc-通知无限循环)
    - [问题 3：快照恢复兼容性](./drafts/lab04.md#问题-3快照恢复兼容性)
  - [设计思考](./drafts/lab04.md#设计思考)
- [Challenge 2: 配置变更期间的无中断服务](./drafts/lab04.md#challenge-2-配置变更期间的无中断服务)
  - [问题背景](./drafts/lab04.md#问题背景-1)
  - [为什么 Challenge 2 被自然支持](./drafts/lab04.md#为什么-challenge-2-被自然支持)
    - [1. 独立的 shard 状态](./drafts/lab04.md#1-独立的-shard-状态)
    - [2. 请求只检查特定 shard 的状态](./drafts/lab04.md#2-请求只检查特定-shard-的状态)
    - [3. 迁移完成后立即可用](./drafts/lab04.md#3-迁移完成后立即可用)
  - [状态流转图](./drafts/lab04.md#状态流转图)
  - [设计要点](./drafts/lab04.md#设计要点)
    - [1. 不阻塞整个 group](./drafts/lab04.md#1-不阻塞整个-group)
    - [2. 配置推进不依赖"全部 shard 就绪"](./drafts/lab04.md#2-配置推进不依赖全部-shard-就绪)
    - [3. 迁移器独立工作](./drafts/lab04.md#3-迁移器独立工作)
  - [测试结果](./drafts/lab04.md#测试结果-3)
- [Lab 4 设计要点总结](./drafts/lab04.md#lab-4-设计要点总结)
  - [1. 分片系统的核心抽象](./drafts/lab04.md#1-分片系统的核心抽象)
    - [三层架构](./drafts/lab04.md#三层架构)
    - [状态流转](./drafts/lab04.md#状态流转)
  - [2. 关键设计决策](./drafts/lab04.md#2-关键设计决策)
    - [Pull vs Push 模型](./drafts/lab04.md#pull-vs-push-模型)
    - [配置顺序处理](./drafts/lab04.md#配置顺序处理)
    - [GC 确认机制](./drafts/lab04.md#gc-确认机制)
  - [3. 并发控制策略](./drafts/lab04.md#3-并发控制策略)
    - [单一 Mutex 原则](./drafts/lab04.md#单一-mutex-原则)
    - [后台 Goroutine 设计](./drafts/lab04.md#后台-goroutine-设计)
  - [4. 容错与幂等性](./drafts/lab04.md#4-容错与幂等性)
    - [RPC 幂等性保证](./drafts/lab04.md#rpc-幂等性保证)
    - [崩溃恢复](./drafts/lab04.md#崩溃恢复)
- [综合总结：MIT 6.824 Raft 实现要点](./drafts/lab04.md#综合总结mit-6824-raft-实现要点)
  - [1. 架构分层](./drafts/lab04.md#1-架构分层)
  - [2. 锁的使用原则](./drafts/lab04.md#2-锁的使用原则)
    - [原则 1：单一 Mutex](./drafts/lab04.md#原则-1单一-mutex)
    - [原则 2：持有锁的时间最小化](./drafts/lab04.md#原则-2持有锁的时间最小化)
    - [原则 3：不在持有锁时发送 RPC](./drafts/lab04.md#原则-3不在持有锁时发送-rpc)
  - [3. Goroutine 设计模式](./drafts/lab04.md#3-goroutine-设计模式)
    - [模式 1：定时器驱动](./drafts/lab04.md#模式-1定时器驱动)
    - [模式 2：事件驱动](./drafts/lab04.md#模式-2事件驱动)
    - [模式 3：等待通道](./drafts/lab04.md#模式-3等待通道)
  - [4. 幂等性设计](./drafts/lab04.md#4-幂等性设计)
  - [5. 调试技巧](./drafts/lab04.md#5-调试技巧)
    - [使用 DPrintf](./drafts/lab04.md#使用-dprintf)
    - [检查 race condition](./drafts/lab04.md#检查-race-condition)
    - [重复运行测试](./drafts/lab04.md#重复运行测试)
  - [6. 常见陷阱](./drafts/lab04.md#6-常见陷阱)
    - [陷阱 1：忘记深拷贝](./drafts/lab04.md#陷阱-1忘记深拷贝)
    - [陷阱 2：goroutine 泄漏](./drafts/lab04.md#陷阱-2goroutine-泄漏)
    - [陷阱 3：只提交当前 term 的日志](./drafts/lab04.md#陷阱-3只提交当前-term-的日志)
  - [最终测试结果](./drafts/lab04.md#最终测试结果)
