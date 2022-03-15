# MIT6.S081: Operating System Engineering


<!-- @import "[TOC]" {cmd="toc" depthFrom=3 depthTo=6 orderedList=false} -->

<!-- code_chunk_output -->

- [🥂课程资料](#课程资料)
- [🥞本仓库资源](#本仓库资源)
- [🍱课程索引](#课程索引)
  - [LEC 1 (rtm): Introduction and examples](#lec-1-rtm-introduction-and-examples)
  - [LEC 2 (TAs/dm): C and gdb (pointers example)](#lec-2-tasdm-c-and-gdb-pointers-example)
  - [LEC 3 (fk): OS organization and system calls](#lec-3-fk-os-organization-and-system-calls)
  - [LEC 4 (fk): Page tables (boards)](#lec-4-fk-page-tables-boards)

<!-- /code_chunk_output -->

### 🥂课程资料

实际上，这门课就是大名鼎鼎的 MIT 6.828 ，只不过2020年课号改成了 6.S081 。

官方课程安排与资源索引：
- [https://pdos.csail.mit.edu/6.S081/2020/schedule.html](https://pdos.csail.mit.edu/6.S081/2020/schedule.html)

中文文档与翻译：
- [https://mit-public-courses-cn-translatio.gitbook.io/mit6-s081/](https://mit-public-courses-cn-translatio.gitbook.io/mit6-s081/)

B 站视频：
- [MIT 操作系统 6.S081（中英文字幕）](https://www.bilibili.com/video/BV1QA411F7ij)

实验：
- 本课程用到 `xv6` 系统，我从 `GitHub` 上 `fork` 了一个：https://github.com/PiperLiu/xv6-riscv
- 本课程进行实验的作业系统： `git clone git://g.csail.mit.edu/xv6-labs-2020`
  - 我也放到 `GitHub` 上了（❗❗这个仓库很重要❗❗）：https://github.com/PiperLiu/xv6-labs-2020
  - 网上有很多参考答案。实在没思路再去搜 GitHub xv6-labs 。作业很有趣。

参考：
- [如何学好操作系统原理这门课？ - 菜饼不菜的回答 - 知乎](https://www.zhihu.com/question/22874344/answer/2091285378)
- [MIT6.S081（2020）翻译简介 - 肖宏辉的文章 - 知乎](https://zhuanlan.zhihu.com/p/261362497)

### 🥞本仓库资源

- 教材（主要用作预习）：[xv6: a simple, Unix-like teaching operating system](./docs/lec/book-riscv-rev1.pdf)
- 其他的资源比如课堂资料、作业要求在[docs/assignment/](./docs/assignment/)和[docs/lec/](./docs/lec/)中
- 实验说明：<a href="./docs/6.S081 _ Fall 2020.html">6.S081 _ Fall 2020.html</a>

### 🍱课程索引

#### LEC 1 (rtm): Introduction and examples

sep 2

- LEC 1 (rtm): [Introduction](./docs/lec/lec1.l-overview.txt) and [examples](https://pdos.csail.mit.edu/6.S081/2020/lec/l-overview/) (handouts: xv6 book) [video](https://youtu.be/L6YqHxYHa7A)
- Preparation: Read [chapter 1](./docs/lec/book-riscv-rev1.pdf) (for your amusement: [Unix](https://www.youtube.com/watch?v=tc4ROCJYbm0))
- Homework 1 due: Question
- Assignment: <a href="./docs/assignment/Lab_ Xv6 and Unix utilities.html">Lab util: Unix utilities</a>

我认真读了书（第一张，一些 Unix 6 接口简单原理与使用），记了笔记：[./docs/drafts/lec.01.md](./docs/drafts/lec.01.md)。

然后听课，课堂笔记：[./docs/drafts/lec.01c.md](./docs/drafts/lec.01c.md) 

- [课程简介：操作系统做哪些事](./docs/drafts/lec.01c.md#课程简介操作系统做哪些事)
- [计算机系统](./docs/drafts/lec.01c.md#计算机系统)
- [系统调用：read和write](./docs/drafts/lec.01c.md#系统调用read和write)
- [shell](./docs/drafts/lec.01c.md#shell)
- [进程相关系统调用：fork和exec以及wait](./docs/drafts/lec.01c.md#进程相关系统调用fork和exec以及wait)
- [I/O重定向与课程总结](./docs/drafts/lec.01c.md#io重定向与课程总结)

本节作业笔记：[./docs/drafts/lec.01hw.md](./docs/drafts/lec.01hw.md)

#### LEC 2 (TAs/dm): C and gdb (pointers example)

sep 9

- LEC 2 (TAs/dm): [C](./docs/lec/l-c-slides.pdf) and [gdb](./docs/lec/gdb_slides.pdf) ([pointers example](./docs/lec/pointers.c))
- Preparation: 2.9 (Bitwise operators) and 5.1 (Pointers and addresses) through 5.6 (Pointer arrays) and 6.4 (pointers to structures) by Kernighan and Ritchie (K&R)
- Assignment: <a href="./docs/assignment/Lab_ System calls.html">Lab syscall: System calls</a>

课前预习，读书：[./docs/drafts/lec.02.md](./docs/drafts/lec.02.md)

- [C 的读书笔记](./docs/drafts/lec.02.md#c-的读书笔记)
  - [C 中的内存：静态、堆、栈](./docs/drafts/lec.02.md#c-中的内存静态-堆-栈)
  - [永远不要 include .c 文件](./docs/drafts/lec.02.md#永远不要-include-c-文件)
- [Using the GNU Debugger](./docs/drafts/lec.02.md#using-the-gnu-debugger)
- [读书： Chapter 2 Operating system organization](./docs/drafts/lec.02.md#读书-chapter-2-operating-system-organization)
  - [2.1 Abstracting physical resources](./docs/drafts/lec.02.md#21-abstracting-physical-resources)
  - [2.2 User mode, supervisor mode, and system calls （ ecall 是 RISC-V CPU 用于从用户态转为内核态的指令）](./docs/drafts/lec.02.md#22-user-mode-supervisor-mode-and-system-calls-ecall-是-risc-v-cpu-用于从用户态转为内核态的指令)
  - [2.3 Kernel organization](./docs/drafts/lec.02.md#23-kernel-organization)
  - [2.4 Code: xv6 organization](./docs/drafts/lec.02.md#24-code-xv6-organization)
  - [2.5 Process overview 进程与页表](./docs/drafts/lec.02.md#25-process-overview-进程与页表)
  - [2.6 Code: starting xv6 and the first process](./docs/drafts/lec.02.md#26-code-starting-xv6-and-the-first-process)
  - [插曲：内联汇编的格式](./docs/drafts/lec.02.md#插曲内联汇编的格式)
  - [仔细阅读 start 发现是把汇编操作封装](./docs/drafts/lec.02.md#仔细阅读-start-发现是把汇编操作封装)
  - [2.7 Real world](./docs/drafts/lec.02.md#27-real-world)
  - [插曲：安装 riscv64-unknown-elf-gdb](./docs/drafts/lec.02.md#插曲安装-riscv64-unknown-elf-gdb)
  - [2.8 Exercises 尝试 gdb](./docs/drafts/lec.02.md#28-exercises-尝试-gdb)
- [读书： Chapter 4 Traps and system calls](./docs/drafts/lec.02.md#读书-chapter-4-traps-and-system-calls)
  - [4.3 Code: Calling system calls](./docs/drafts/lec.02.md#43-code-calling-system-calls)
  - [4.4 Code: System call arguments 系统调用的参数](./docs/drafts/lec.02.md#44-code-system-call-arguments-系统调用的参数)
- [阅读一些代码](./docs/drafts/lec.02.md#阅读一些代码)
  - [用 perl 脚本生成 S 汇编代码](./docs/drafts/lec.02.md#用-perl-脚本生成-s-汇编代码)

这节课就是读书课，留了个 lab ，下节课听完再做。

#### LEC 3 (fk): OS organization and system calls

- LEC 3 (fk): OS organization and system calls ([boards](./docs/lec/l-os-boards.pdf)) [video](https://youtu.be/o44d---Dk4o)
- Preparation: Read chapter 2 and xv6 code: kernel/proc.h, kernel/defs.h, kernel/entry.S, kernel/main.c, user/initcode.S, user/init.c, and skim kernel/proc.c and kernel/exec.c

这节课需要预习的东西和上节课一样。

听课笔记在这里：[./docs/drafts/lec.03c.md](./docs/drafts/lec.03c.md)

- [Isolation / Kernel mode / User mode / System calls](./docs/drafts/lec.03c.md#isolation-kernel-mode-user-mode-system-calls)
  - [exec 抽象了内存](./docs/drafts/lec.03c.md#exec-抽象了内存)
  - [操作系统防御性（Defensive）](./docs/drafts/lec.03c.md#操作系统防御性defensive)
- [硬件对于强隔离的支持](./docs/drafts/lec.03c.md#硬件对于强隔离的支持)
  - [user/kernle mode](./docs/drafts/lec.03c.md#userkernle-mode)
  - [page table 来制造应用间隔离](./docs/drafts/lec.03c.md#page-table-来制造应用间隔离)
  - [Kernel Mode 和 User Mode 切换](./docs/drafts/lec.03c.md#kernel-mode-和-user-mode-切换)
- [宏内核 vs 微内核 （Monolithic Kernel vs Micro Kernel）](./docs/drafts/lec.03c.md#宏内核-vs-微内核-monolithic-kernel-vs-micro-kernel)
- [编译运行kernel](./docs/drafts/lec.03c.md#编译运行kernel)
  - [插曲：正确使用 gdb](./docs/drafts/lec.03c.md#插曲正确使用-gdb)
  - [代码结构](./docs/drafts/lec.03c.md#代码结构)
  - [简单介绍内核如何编译的](./docs/drafts/lec.03c.md#简单介绍内核如何编译的)
  - [QEMU](./docs/drafts/lec.03c.md#qemu)
  - [XV6启动过程（配合 gdb 实践）](./docs/drafts/lec.03c.md#xv6启动过程配合-gdb-实践)

然后把 lab 2 做了吧：[./docs/drafts/lec.03hw.md](./docs/drafts/lec.03hw.md)

#### LEC 4 (fk): Page tables (boards)

sep 16

- LEC 4 (fk): [Page tables](./docs/lec/l-vm.txt) ([boards](./docs/lec/l-vm-boards.pdf)) [video](https://youtu.be/f1Hpjty3TT8)
- Preparation: Read [Chapter 3](./docs/lec/book-riscv-rev1.pdf) and kernel/memlayout.h, kernel/vm.c, kernel/kalloc.c, kernel/riscv.h, and kernel/exec.c
- Assignment: <a href="./docs/assignment/Lab_ page tables.html">Lab pgtbl: Page tables</a>

课前预习是读书第三章，我的笔记在这了：[./docs/drafts/lec.04.md](./docs/drafts/lec.04.md)

- [Chapter 3 Page tables](./docs/drafts/lec.04.md#chapter-3-page-tables)
  - [3.1 Paging hardware 页表基本概念与分级、 satp 寄存器](./docs/drafts/lec.04.md#31-paging-hardware-页表基本概念与分级-satp-寄存器)
  - [3.2 Kernel address space](./docs/drafts/lec.04.md#32-kernel-address-space)
  - [3.3 Code: creating an address space 以及 TLB](./docs/drafts/lec.04.md#33-code-creating-an-address-space-以及-tlb)
  - [3.4 Physical memory allocation](./docs/drafts/lec.04.md#34-physical-memory-allocation)
  - [3.5 Code: Physical memory allocator](./docs/drafts/lec.04.md#35-code-physical-memory-allocator)
  - [3.6 Process address space](./docs/drafts/lec.04.md#36-process-address-space)
  - [3.7 Code: sbrk 系统调用](./docs/drafts/lec.04.md#37-code-sbrk-系统调用)
  - [3.8 Code: exec](./docs/drafts/lec.04.md#38-code-exec)
  - [3.9 Real world](./docs/drafts/lec.04.md#39-real-world)
  - [3.10 Exercises](./docs/drafts/lec.04.md#310-exercises)

课堂笔记：[./docs/drafts/lec.04c.md](./docs/drafts/lec.04c.md)

- [虚拟内存的基本概念](./docs/drafts/lec.04c.md#虚拟内存的基本概念)
- [页表](./docs/drafts/lec.04c.md#页表)
  - [虚拟地址到物理地址转换](./docs/drafts/lec.04c.md#虚拟地址到物理地址转换)
  - [多级页表](./docs/drafts/lec.04c.md#多级页表)
  - [页表缓存 TLB](./docs/drafts/lec.04c.md#页表缓存-tlb)
  - [kernel page table](./docs/drafts/lec.04c.md#kernel-page-table)
- [阅读 kernel 代码](./docs/drafts/lec.04c.md#阅读-kernel-代码)
  - [kvminit 函数](./docs/drafts/lec.04c.md#kvminit-函数)
  - [kvminithart 函数](./docs/drafts/lec.04c.md#kvminithart-函数)
  - [walk 函数](./docs/drafts/lec.04c.md#walk-函数)

lab 3 地址如下：[]()
