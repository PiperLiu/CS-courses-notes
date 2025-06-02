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
- [🥏Lab 笔记索引](#lab-笔记索引)
  - [Lab 1: MapReduce](#lab-1-mapreduce)

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
- <a href="./html/6.824 Lab 1_ MapReduce.html">Assigned: Lab 1: MapReduce</a>

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
- <a href="./html/6.824 Lab 2_ Raft.html">Assigned: Lab 2: Raft</a>

本节课的笔记：[./drafts/lec03.md](./drafts/lec03.md)

#### LEC 4: Primary-Backup Replication

- LEC 4: [Primary-Backup Replication](./lec/l-vm-ft.txt), [video](http://nil.csail.mit.edu/6.824/2020/video/4.html)
- Preparation: Read [Fault-Tolerant Virtual Machines (2010)](./lec/vm-ft.pdf) ([FAQ](./lec/vm-ft-faq.txt))

本节课的笔记：[./drafts/lec04.md](./drafts/lec04.md)

#### LEC 5: Go, Threads, and Raft, video, code samples

- LEC 5: [Go, Threads, and Raft](./lec/l-go-concurrency.txt), [video](http://nil.csail.mit.edu/6.824/2020/video/5.html), [code samples](./lec/go-concurrency/)
- Preparation: [Read The Go Memory Model](https://go.dev/ref/mem)

本节课的笔记：[./drafts/lec05.md](./drafts/lec05.md)

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
