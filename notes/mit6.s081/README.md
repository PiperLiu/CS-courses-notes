# MIT6.S081: Operating System Engineering


<!-- @import "[TOC]" {cmd="toc" depthFrom=3 depthTo=6 orderedList=false} -->

<!-- code_chunk_output -->

- [🥂课程资料](#课程资料)
- [🥞本仓库资源](#本仓库资源)
- [🍱课程索引](#课程索引)
  - [LEC 1 (rtm): Introduction and examples](#lec-1-rtm-introduction-and-examples)
  - [LEC 2 (TAs/dm): C and gdb](#lec-2-tasdm-c-and-gdb)
  - [LEC 3 (fk): OS organization and system calls](#lec-3-fk-os-organization-and-system-calls)
  - [LEC 4 (fk): Page tables](#lec-4-fk-page-tables)
  - [LEC 5 (TAs/nk): Calling conventions and stack frames RISC-V](#lec-5-tasnk-calling-conventions-and-stack-frames-risc-v)
  - [LEC 6 (rtm): Isolation & system call entry/exit](#lec-6-rtm-isolation-system-call-entryexit)
  - [LEC 7 (fk): Q&A labs](#lec-7-fk-qa-labs)
  - [LEC 8 (fk): Page faults](#lec-8-fk-page-faults)
  - [LEC 9 (fk): Interrupts](#lec-9-fk-interrupts)
  - [LEC 10 (fk): Multiprocessors and locking](#lec-10-fk-multiprocessors-and-locking)
  - [LEC 11 (rtm): Thread switching](#lec-11-rtm-thread-switching)

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

#### LEC 2 (TAs/dm): C and gdb

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

#### LEC 4 (fk): Page tables

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

lab 3 地址如下：[./docs/drafts/lec.04hw.md](./docs/drafts/lec.04hw.md)

这个实验对我来讲真的是难，断断续续做了五天。做完后，发现其实真的很简单：只要捋清了页表在哪里会被创建（与进程被创建、被 fork 、被扩容有关），在哪里被取消映射、取消哪里的映射。实际上，由于这个操作系统的优秀设计，把用户虚拟地址和内核虚拟地址放在一个页表上完全不会冲突。

#### LEC 5 (TAs/nk): Calling conventions and stack frames RISC-V

sep 21

- LEC 5 (TAs/nk): [Calling conventions and stack frames RISC-V](./docs/lec/l-riscv.txt) ([slides](./docs/lec/l-riscv-slides.pdf)) [video](https://youtu.be/s-Z5t_yTyTM)
- Preparation: Read [Calling Convention](./docs/lec/riscv-calling.pdf)

课前读书是关于 the C compiler standards for RV32 and RV64 programs ，读书笔记在此：[./docs/drafts/lec.05.md](./docs/drafts/lec.05.md)

- [读书 Chapter 18 Calling Convention](./docs/drafts/lec.05.md#读书-chapter-18-calling-convention)
  - [18.1 C Datatypes and Alignment](./docs/drafts/lec.05.md#181-c-datatypes-and-alignment)
  - [18.2 RVG Calling Convention](./docs/drafts/lec.05.md#182-rvg-calling-convention)
  - [18.3 Soft-Float Calling Convention](./docs/drafts/lec.05.md#183-soft-float-calling-convention)

课上内容：[./docs/drafts/lec.05c.md](./docs/drafts/lec.05c.md)

- [关于 RISC-V 处理器与指令集](./docs/drafts/lec.05c.md#关于-risc-v-处理器与指令集)
  - [ISA（Instruction Sets Architecture）](./docs/drafts/lec.05c.md#isainstruction-sets-architecture)
  - [x86指令集（CISC）](./docs/drafts/lec.05c.md#x86指令集cisc)
  - [简单提一下 ARM 指令架构（RISC）](./docs/drafts/lec.05c.md#简单提一下-arm-指令架构risc)
  - [RISC-V 区分了基本int指令与其他标准扩展](./docs/drafts/lec.05c.md#risc-v-区分了基本int指令与其他标准扩展)
- [汇编代码在内存的位置及其执行](./docs/drafts/lec.05c.md#汇编代码在内存的位置及其执行)
  - [“代码段”的具象化.text](./docs/drafts/lec.05c.md#代码段的具象化text)
  - [关于 gdb](./docs/drafts/lec.05c.md#关于-gdb)
- [关于 RISC-V 寄存器](./docs/drafts/lec.05c.md#关于-risc-v-寄存器)
- [关于栈 stack](./docs/drafts/lec.05c.md#关于栈-stack)
  - [【重要】关于stack的gdb调试（查看当前函数调用）](./docs/drafts/lec.05c.md#重要关于stack的gdb调试查看当前函数调用)
- [struct （内存中是一段连续的地址）](./docs/drafts/lec.05c.md#struct-内存中是一段连续的地址)

#### LEC 6 (rtm): Isolation & system call entry/exit

sep 23

- LEC 6 (rtm): [Isolation & system call entry/exit](./docs/lec/l-internal.txt) ([video](https://youtu.be/T26UuauaxWA))
- Preparation: [Read Chapter 4, except 4.6](./docs/lec/book-riscv-rev1.pdf) and kernel/riscv.h, kernel/trampoline.S, and kernel/trap.c
- Assignment: <a href="./docs/assignment/Lab_ Traps.html">Lab traps: Trap</a>

课前读书笔记在这里：[./docs/drafts/lec.06.md](./docs/drafts/lec.06.md)

- [读书 Chapter 4 Traps and system calls](./docs/drafts/lec.06.md#读书-chapter-4-traps-and-system-calls)
  - [4.1 RISC-V trap machinery](./docs/drafts/lec.06.md#41-risc-v-trap-machinery)
  - [4.2 Traps from user space](./docs/drafts/lec.06.md#42-traps-from-user-space)
  - [4.3 Code: Calling system calls](./docs/drafts/lec.06.md#43-code-calling-system-calls)
  - [4.4 Code: System call arguments](./docs/drafts/lec.06.md#44-code-system-call-arguments)
  - [4.5 Traps from kernel space](./docs/drafts/lec.06.md#45-traps-from-kernel-space)
  - [4.7 Real world](./docs/drafts/lec.06.md#47-real-world)
  - [4.8 Exercises](./docs/drafts/lec.06.md#48-exercises)

课堂笔记在这里：[./docs/drafts/lec.06c.md](./docs/drafts/lec.06c.md)

- [trap概述](./docs/drafts/lec.06c.md#trap概述)
  - [trap时硬件以及kernel要做什么？](./docs/drafts/lec.06c.md#trap时硬件以及kernel要做什么)
  - [supervisor mode能做什么](./docs/drafts/lec.06c.md#supervisor-mode能做什么)
  - [trap 执行流程](./docs/drafts/lec.06c.md#trap-执行流程)
- [使用gdb跟踪trap流程](./docs/drafts/lec.06c.md#使用gdb跟踪trap流程)
  - [查看 shell 调用 write（打断点、看寄存器、看页表）](./docs/drafts/lec.06c.md#查看-shell-调用-write打断点-看寄存器-看页表)
  - [进入 ecall](./docs/drafts/lec.06c.md#进入-ecall)
  - [ecall 实际上只做了三件事（我们该额外做什么）](./docs/drafts/lec.06c.md#ecall-实际上只做了三件事我们该额外做什么)
  - [trampoline page的起始，也是uservec函数的起始](./docs/drafts/lec.06c.md#trampoline-page的起始也是uservec函数的起始)
  - [usertrap函数是位于trap.c文件的一个函数](./docs/drafts/lec.06c.md#usertrap函数是位于trapc文件的一个函数)
  - [usertrapret函数，在返回到用户空间之前内核要做的工作](./docs/drafts/lec.06c.md#usertrapret函数在返回到用户空间之前内核要做的工作)
  - [userret函数（又到了trampoline）](./docs/drafts/lec.06c.md#userret函数又到了trampoline)

lab 先不做，先把下节课答疑听了再做。

#### LEC 7 (fk): Q&A labs

sep 28

- LEC 7 (fk): [Q&A labs](./docs/lec/l-QA1.txt) ([boards](./docs/lec/l-QA1.pdf)) [video](https://youtu.be/_WWjNIJAfVg)

一些课堂笔记：[./docs/drafts/lec.07c.md](./docs/drafts/lec.07c.md)
- [复习页表](./docs/drafts/lec.07c.md#复习页表)
  - [关于 guard page 和 stack page](./docs/drafts/lec.07c.md#关于-guard-page-和-stack-page)
- [之前的 page table lab](./docs/drafts/lec.07c.md#之前的-page-table-lab)
  - [第二问：共享内核页](./docs/drafts/lec.07c.md#第二问共享内核页)

好了，现在把 Lab traps: Trap 给做了吧，笔记：[./docs/drafts/lec.07hw.md](./docs/drafts/lec.07hw.md)

- [RISC-V assembly（汇编、RISC-V是小端机）](./docs/drafts/lec.07hw.md#risc-v-assembly汇编-risc-v是小端机)
- [Backtrace 报错时回溯打印栈 stack](./docs/drafts/lec.07hw.md#backtrace-报错时回溯打印栈-stack)
- [Alarm 根据 tick 做回调](./docs/drafts/lec.07hw.md#alarm-根据-tick-做回调)
  - [test0: invoke handler](./docs/drafts/lec.07hw.md#test0-invoke-handler)
  - [test1/test2(): resume interrupted code](./docs/drafts/lec.07hw.md#test1test2-resume-interrupted-code)

#### LEC 8 (fk): Page faults

sep 30

- LEC 8 (fk): [Page faults](./docs/lec/l-pgfaults.txt) ([boards](./docs/lec/l-pgfaults.pdf)) [video](https://youtu.be/KSYO-gTZo0A)
- Preparation: Read [Section 4.6](./docs/lec/book-riscv-rev1.pdf)
- Assignment: <a href="./docs/assignment/Lab_ xv6 lazy page allocation.html">Lab lazy: Lazy allocation</a>

读了一个小节，与课堂内容有很多重叠，不详细记录了：[./docs/drafts/lec.08.md](./docs/drafts/lec.08.md)
- [读书 4.6 Page-fault exceptions](./docs/drafts/lec.08.md#读书-46-page-fault-exceptions)
  - [什么是 page-fault exception](./docs/drafts/lec.08.md#什么是-page-fault-exception)

上课：[./docs/drafts/lec.08c.md](./docs/drafts/lec.08c.md)
- [page fault 课程内容概述](./docs/drafts/lec.08c.md#page-fault-课程内容概述)
  - [内核需要什么样的信息才能够响应 page fault](./docs/drafts/lec.08c.md#内核需要什么样的信息才能够响应-page-fault)
- [Lazy page allocation (sbrk)](./docs/drafts/lec.08c.md#lazy-page-allocation-sbrk)
  - [简单修改 sys_sbrk](./docs/drafts/lec.08c.md#简单修改-sys_sbrk)
- [Zero Fill On Demand](./docs/drafts/lec.08c.md#zero-fill-on-demand)
- [Copy On Write Fork](./docs/drafts/lec.08c.md#copy-on-write-fork)
- [Demand paging](./docs/drafts/lec.08c.md#demand-paging)
  - [在lazy allocation中，如果内存耗尽了该如何办？（dirty page）](./docs/drafts/lec.08c.md#在lazy-allocation中如果内存耗尽了该如何办dirty-page)
- [Memory Mapped Files](./docs/drafts/lec.08c.md#memory-mapped-files)

lab 就是设计一个 lazy allocation ，挺简单的：[./docs/drafts/lec.08hw.md](./docs/drafts/lec.08hw.md)
- [Eliminate allocation from sbrk()](./docs/drafts/lec.08hw.md#eliminate-allocation-from-sbrk)
- [Lazy allocation](./docs/drafts/lec.08hw.md#lazy-allocation)
- [Lazytests and Usertests](./docs/drafts/lec.08hw.md#lazytests-and-usertests)

#### LEC 9 (fk): Interrupts

oct 5

- LEC 9 (fk): [Interrupts](./docs/lec/l-interrupt.txt) ([boards](./docs/lec/l-interrupt.pdf)) [video](https://youtu.be/zRnGNndcVEA)
- Preparation: Read [Chapter 5](./docs/lec/book-riscv-rev1.pdf) and kernel/kernelvec.S, kernel/plic.c, kernel/console.c, kernel/uart.c, kernel/printf.c

课前预习读读书：[./docs/drafts/lec.09.md](./docs/drafts/lec.09.md)
- [Chapter 5 Interrupts and device drivers](./docs/drafts/lec.09.md#chapter-5-interrupts-and-device-drivers)
  - [5.1 Code: Console input](./docs/drafts/lec.09.md#51-code-console-input)
  - [5.2 Code: Console output (I/O concurrency IO 并行)](./docs/drafts/lec.09.md#52-code-console-output-io-concurrency-io-并行)
  - [5.3 Concurrency in drivers](./docs/drafts/lec.09.md#53-concurrency-in-drivers)
  - [5.4 Timer interrupts](./docs/drafts/lec.09.md#54-timer-interrupts)
  - [5.5 Real world (DMA 这里引出了 Linux 常用的多路复用技术 polling 等)](./docs/drafts/lec.09.md#55-real-world-dma-这里引出了-linux-常用的多路复用技术-polling-等)
  - [5.6 Exercises](./docs/drafts/lec.09.md#56-exercises)
- [中断相关代码](./docs/drafts/lec.09.md#中断相关代码)
  - [kernel/kernelvec.S](./docs/drafts/lec.09.md#kernelkernelvecs)
  - [kernel/plic.c](./docs/drafts/lec.09.md#kernelplicc)
  - [kernel/console.c](./docs/drafts/lec.09.md#kernelconsolec)
  - [kernel/uart.c](./docs/drafts/lec.09.md#kerneluartc)
  - [kernel/printf.c](./docs/drafts/lec.09.md#kernelprintfc)

上课内容：[./docs/drafts/lec.09c.md](./docs/drafts/lec.09c.md)
- [真实操作系统内存使用情况](./docs/drafts/lec.09c.md#真实操作系统内存使用情况)
- [Interrupt硬件部分](./docs/drafts/lec.09c.md#interrupt硬件部分)
  - [中断是从哪里产生的？](./docs/drafts/lec.09c.md#中断是从哪里产生的)
- [Interrupt软件部分](./docs/drafts/lec.09c.md#interrupt软件部分)
  - [设备驱动概述（UART设备为例）](./docs/drafts/lec.09c.md#设备驱动概述uart设备为例)
  - [如何对设备进行编程（memory mapped I/O）](./docs/drafts/lec.09c.md#如何对设备进行编程memory-mapped-io)
  - [在XV6中设置中断](./docs/drafts/lec.09c.md#在xv6中设置中断)
  - [start.c的start函数以及 main 中如何处理 External 中断](./docs/drafts/lec.09c.md#startc的start函数以及-main-中如何处理-external-中断)
  - [PLIC编程，让断能被CPU感知以及main中scheduler让CPU接收中断](./docs/drafts/lec.09c.md#plic编程让断能被cpu感知以及main中scheduler让cpu接收中断)
- [详细看UART驱动实现](./docs/drafts/lec.09c.md#详细看uart驱动实现)
  - [UART驱动的top部分](./docs/drafts/lec.09c.md#uart驱动的top部分)
  - [UART驱动的bottom部分](./docs/drafts/lec.09c.md#uart驱动的bottom部分)
- [Interrupt相关的并发（对buffer的producer/consumer并发）](./docs/drafts/lec.09c.md#interrupt相关的并发对buffer的producerconsumer并发)
- [UART读取键盘输入](./docs/drafts/lec.09c.md#uart读取键盘输入)
- [Interrupt的演进（引出polling）](./docs/drafts/lec.09c.md#interrupt的演进引出polling)

好了，在进入 Multiprocessors and locking 之前，先把 page allocation 的内容解决掉，把 Lab 6 Copy-on-Write Fork for xv6 写了：[./docs/drafts/lec.09hw.md](./docs/drafts/lec.09hw.md)
- [Implement copy-on write](./docs/drafts/lec.09hw.md#implement-copy-on-write)

#### LEC 10 (fk): Multiprocessors and locking

oct 7

- LEC 10 (fk): [Multiprocessors and locking](./docs/lec/l-lockv2.txt) ([boards](./docs/lec/l-lock.pdf)) [video](https://youtu.be/NGXu3vN7yAk)
- Preparation: Read "[Locking](./docs/lec/book-riscv-rev1.pdf)" with kernel/spinlock.h and kernel/spinlock.c
- Assignment: <a href="./docs/assignment/Lab_ Copy-on-Write Fork for xv6.html">Lab cow: Copy-on-write fork</a>

这个 Lab 上节课写完了。

读书预习一下：[./docs/drafts/lec.10.md](./docs/drafts/lec.10.md)
- [Chapter 6 Locking](./docs/drafts/lec.10.md#chapter-6-locking)
  - [6.1 Race conditions 竞态条件](./docs/drafts/lec.10.md#61-race-conditions-竞态条件)
    - [critical sectio 临界区](./docs/drafts/lec.10.md#critical-sectio-临界区)
  - [6.2 Code: Locks 这里以自旋锁 spinlock 为例](./docs/drafts/lec.10.md#62-code-locks-这里以自旋锁-spinlock-为例)
  - [6.3 Code: Using locks](./docs/drafts/lec.10.md#63-code-using-locks)
  - [6.4 Deadlock and lock ordering](./docs/drafts/lec.10.md#64-deadlock-and-lock-ordering)
  - [6.5 Locks and interrupt handlers](./docs/drafts/lec.10.md#65-locks-and-interrupt-handlers)
  - [6.6 Instruction and memory ordering](./docs/drafts/lec.10.md#66-instruction-and-memory-ordering)
  - [6.7 Sleep locks 引出不用自旋锁 spinlock](./docs/drafts/lec.10.md#67-sleep-locks-引出不用自旋锁-spinlock)
  - [6.8 Real world](./docs/drafts/lec.10.md#68-real-world)
  - [6.9 Exercises](./docs/drafts/lec.10.md#69-exercises)

课堂笔记：[./docs/drafts/lec.10c.md](./docs/drafts/lec.10c.md)
- [为什么要使用锁？](./docs/drafts/lec.10c.md#为什么要使用锁)
  - [为什么要使用多个CPU核](./docs/drafts/lec.10c.md#为什么要使用多个cpu核)
  - [race condition 竞态条件](./docs/drafts/lec.10c.md#race-condition-竞态条件)
  - [critical section 临界区](./docs/drafts/lec.10c.md#critical-section-临界区)
- [什么时候使用锁？](./docs/drafts/lec.10c.md#什么时候使用锁)
  - [能通过自动的创建锁来自动避免race condition吗？（不能）](./docs/drafts/lec.10c.md#能通过自动的创建锁来自动避免race-condition吗不能)
- [锁的特性和死锁](./docs/drafts/lec.10c.md#锁的特性和死锁)
  - [锁缺点：死锁](./docs/drafts/lec.10c.md#锁缺点死锁)
  - [锁缺点：破坏模块化](./docs/drafts/lec.10c.md#锁缺点破坏模块化)
  - [锁缺点：影响性能](./docs/drafts/lec.10c.md#锁缺点影响性能)
- [锁是如何在XV6中工作的（以UART为例）](./docs/drafts/lec.10c.md#锁是如何在xv6中工作的以uart为例)
  - [消费者-生产者模式](./docs/drafts/lec.10c.md#消费者-生产者模式)
  - [uart中的函数](./docs/drafts/lec.10c.md#uart中的函数)
- [自旋锁 spin lock 的实现](./docs/drafts/lec.10c.md#自旋锁-spin-lock-的实现)
  - [test-and-set 操作原子性指令 amoswap](./docs/drafts/lec.10c.md#test-and-set-操作原子性指令-amoswap)
  - [spinlock 具体实现（acquire 和 release）](./docs/drafts/lec.10c.md#spinlock-具体实现acquire-和-release)
  - [为什么在acquire函数的最开始，会先关闭中断？](./docs/drafts/lec.10c.md#为什么在acquire函数的最开始会先关闭中断)
  - [memory ordering 防止指令顺序被优化](./docs/drafts/lec.10c.md#memory-ordering-防止指令顺序被优化)

#### LEC 11 (rtm): Thread switching

oct 14

- LEC 11 (rtm): [Thread switching](./docs/lec/l-threads.txt) ([video](https://youtu.be/vsgrTHY5tkg))
- Preparation: Read "[Scheduling](./docs/lec/book-riscv-rev1.pdf)" through Section 7.4, and kernel/proc.c, kernel/swtch.S
- Assignment: <a href="./docs/assignment/Lab_ Multithreading.html">Assignment: Lab thread: Multithreading</a>

课前预习把书 7.4 读了：[./docs/drafts/lec.11.md](./docs/drafts/lec.11.md)
- [Chapter 7 Scheduling](./docs/drafts/lec.11.md#chapter-7-scheduling)
  - [7.4 Code: mycpu and myproc](./docs/drafts/lec.11.md#74-code-mycpu-and-myproc)

听课笔记：[./docs/drafts/lec.11c.md](./docs/drafts/lec.11c.md)
