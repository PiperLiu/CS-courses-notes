# LEC 2 (TAs/dm): C and gdb (pointers example)

目录：

<!-- @import "[TOC]" {cmd="toc" depthFrom=2 depthTo=2 orderedList=false} -->

<!-- code_chunk_output -->

- [C 的读书笔记](#c-的读书笔记)
- [Using the GNU Debugger](#using-the-gnu-debugger)
- [读书： Chapter 2 Operating system organization](#读书-chapter-2-operating-system-organization)
- [读书： Chapter 4 Traps and system calls](#读书-chapter-4-traps-and-system-calls)
- [阅读一些代码](#阅读一些代码)

<!-- /code_chunk_output -->

细分目录：

<!-- @import "[TOC]" {cmd="toc" depthFrom=2 depthTo=6 orderedList=false} -->

<!-- code_chunk_output -->

- [C 的读书笔记](#c-的读书笔记)
  - [C 中的内存：静态、堆、栈](#c-中的内存静态-堆-栈)
  - [永远不要 include .c 文件](#永远不要-include-c-文件)
- [Using the GNU Debugger](#using-the-gnu-debugger)
- [读书： Chapter 2 Operating system organization](#读书-chapter-2-operating-system-organization)
  - [2.1 Abstracting physical resources](#21-abstracting-physical-resources)
  - [2.2 User mode, supervisor mode, and system calls （ ecall 是 RISC-V CPU 用于从用户态转为内核态的指令）](#22-user-mode-supervisor-mode-and-system-calls-ecall-是-risc-v-cpu-用于从用户态转为内核态的指令)
  - [2.3 Kernel organization](#23-kernel-organization)
  - [2.4 Code: xv6 organization](#24-code-xv6-organization)
  - [2.5 Process overview 进程与页表](#25-process-overview-进程与页表)
  - [2.6 Code: starting xv6 and the first process](#26-code-starting-xv6-and-the-first-process)
  - [插曲：内联汇编的格式](#插曲内联汇编的格式)
  - [仔细阅读 start 发现是把汇编操作封装](#仔细阅读-start-发现是把汇编操作封装)
  - [2.7 Real world](#27-real-world)
  - [插曲：安装 riscv64-unknown-elf-gdb](#插曲安装-riscv64-unknown-elf-gdb)
  - [2.8 Exercises 尝试 gdb](#28-exercises-尝试-gdb)
- [读书： Chapter 4 Traps and system calls](#读书-chapter-4-traps-and-system-calls)
  - [4.3 Code: Calling system calls](#43-code-calling-system-calls)
  - [4.4 Code: System call arguments 系统调用的参数](#44-code-system-call-arguments-系统调用的参数)
- [阅读一些代码](#阅读一些代码)
  - [用 perl 脚本生成 S 汇编代码](#用-perl-脚本生成-s-汇编代码)

<!-- /code_chunk_output -->

## C 的读书笔记

### C 中的内存：静态、堆、栈

这个道理咱都懂：
- static 内存，全局变量、 static 定义的变量
- stack 内存，在函数里的变量，函数结束了，就自动被销毁了
- heap 内存，由你来控制，比如 malloc() 和 free() ，很危险

来看一段错误的 C 程序：

```c
char* makeABC() {
  char y[3] = {'a', 'b', 'c'};
  return y;
}
```

这段 C 程序的错误，是因为我在函数里开了一个数组，相当于把 y 指向的内容开在了栈上，导致函数结束时被销毁了。

### 永远不要 include .c 文件

.h 中放声明， .c 中放定义，不再多说。

## Using the GNU Debugger

找了一个视频来入门：[Linux 使用gdb调试入门。](https://www.bilibili.com/video/BV1Kq4y1D7n2?p=1)

对于 C 而言，就是你在编译时 `gcc -g -o a.out` ，这个 `-g` 保留了用于 debug 的 Info 。然后你就可以 `gdb a.out` 了。

有很多很多命令：
```
b main 给 main 函数打断点
b /home/a.out:11 给第 11 行打断点
r 运行
n 不进函数体的单步执行
info b 查看断点
d 5 删除编号为 5 的断点
watch i 监控 i 变量的变化
info r 看寄存器的值
p i 查看变量 i 的值
p/x i 查看变量 i 的16进制
layout src 产生一个视窗（用Ctrl+x a退出）
layout asm 产生一个视窗看汇编
si 是汇编的步进
```

## 读书： Chapter 2 Operating system organization

### 2.1 Abstracting physical resources

应用间隔离但又要可以交互。

Thus an operating system must fulfill three requirements: multiplexing, isolation, and
interaction

RISC-V 是 64 位的 CPU ，用的 LP64 C ，即 Long 和 Pointers 都是 64 位的。

为了让应用之间隔离，我们把资源抽象成服务，而非直接让他们接触资源。

To achieve strong isolation it’s helpful to forbid applications from directly accessing sensitive hardware resources, and instead to abstract the resources into services.

比如用 open, read, write 和 close 来访问存储系统，而非让应用直接读进程。

抽象硬件有什么好处？让硬件对应用抽象由什么好处？如下，比如 exec 可以对进程建立自己的内存快照而非直接用物理内存。

As another example, Unix processes use exec to build up their memory image, instead of
directly interacting with physical memory. This allows the operating system to decide where to place a process in memory; if memory is tight, the operating system might even store some of a
process’s data on disk. Exec also provides users with the convenience of a file system to store executable program images.

应用间的交互一般用文件描述符。

Many forms of interaction among Unix processes occur via file descriptors.

### 2.2 User mode, supervisor mode, and system calls （ ecall 是 RISC-V CPU 用于从用户态转为内核态的指令）

隔离有啥用/啥是隔离？应用 A 出问题了（ fail ），别的应用应该还能继续跑。

RISC-V 有三个 CPU 指令级别来支持隔离：
- **machine mode** : a CPU starts in machine mode. Machine
mode is mostly intended for configuring a computer. Xv6 executes a few lines in machine mode
and then changes to supervisor mode.
- **supervisor mode** : CPU 可以执行特权指令，中断，读写存储页表地址的寄存器 In supervisor mode the CPU is allowed to execute privileged instructions: for example, enabling and disabling interrupts, reading and writing the register that holds the address of a page table, etc.
- **user mode** : 不能执行 a privileged instruction ，除非切换为内核态（ in supervisor mode / in supervisor mode ）

**ecall 是 RISC-V CPU 用于从用户态转为内核态的指令。**

An application that wants to invoke a kernel function (e.g., the read system call in xv6) must mtransition to the kernel. CPUs provide a special instruction that switches the CPU from user mode to supervisor mode and enters the kernel at an entry point specified by the kernel. (RISC-V provides the ecall instruction for this purpose.) 

ecall 后在内核态，内核对系统调用的参数等进行检验，决定是否让应用执行操作。**（什么是内核？ The software running in kernel space (or in upervisor mode) is called the kernel. ）**

### 2.3 Kernel organization

操作系统的哪些部分应该属于内核态呢？有两种思路：
- **monolithic kernel** 整个操作系统都有完整的硬件特权，好处是操作系统的各个部分更加易于合作，比如 an operating system might have a buffer cache that can be shared both by the file system and the virtual memory system. **Unix 用的是这种内核组织，这里 xv6 也是** ，坏处是，内核容易出问题，一旦出问题，整个机器就得重启
- **microkernel** 微内核，即将尽量少的 OS 操作放在 supervisor mode ，如下图， the file system runs as a user-level process. OS services running as processes are called servers. **To allow applications to interact with the file server, the kernel provides an inter-process communication mechanism to send messages from one user-mode process to another.** For example, if an application like the shell wants to read or write a file, it sends a message to the file server and waits for a response.

![](./images/2022030901.png)

### 2.4 Code: xv6 organization

看看 kernel 文件夹中有啥：

![](./images/2022030902.png)

模块间的接口 the inter-module interfaces 在 defs.h 。

```c
struct buf;
struct context;
struct file;
struct inode;
struct pipe;
struct proc;
struct spinlock;
struct sleeplock;
struct stat;
struct superblock;

// bio.c
void            binit(void);
struct buf*     bread(uint, uint);
void            brelse(struct buf*);
void            bwrite(struct buf*);
void            bpin(struct buf*);
void            bunpin(struct buf*);

// console.c
void            consoleinit(void);
void            consoleintr(int);
void            consputc(int);

// exec.c
int             exec(char*, char**);

// file.c
struct file*    filealloc(void);
void            fileclose(struct file*);
struct file*    filedup(struct file*);
void            fileinit(void);
int             fileread(struct file*, uint64, int n);
int             filestat(struct file*, uint64 addr);
int             filewrite(struct file*, uint64, int n);

...
```

### 2.5 Process overview 进程与页表

**一个单独的隔离就是进程。 The unit of isolation in xv6 (as in other Unix operating systems) is a process.**

一个进程有私有的内存系统/地址空间，进程也保证了程序“专属”的 CPU 来执行指令（?）

A process provides a program with what appears to be a private memory system, or address space, which other processes cannot read or write. A process also provides the program with what appears to be its own CPU to execute the program’s instructions.

**Xv6 基于页表 page tables （已经被硬件实现）来给各个进程独立的地址空间。**

RISC-V 指令操作的实际上也是虚拟地址 virtual address ，这个虚拟地址具体是什么 physical address 由 RISC-V 映射。 The RISC-V page table translates (or “maps”) a virtual address (the address that an RISC-V instruction manipulates) to a physical address (an address that the CPU chip sends to main memory).

一个进程的虚拟地址空间如下。

![](./images/2022030903.png)

注意这里的 user stack 应该和 Intel x86 不太一样， Intel 的 x86 是从高地址到低地址，并且再往下才是 heap 。

你能在 `kernel/riscv.h:348` 里面看见最高地址的定义：

```c
// one beyond the highest possible virtual address.
// MAXVA is actually one bit less than the max allowed by
// Sv39, to avoid having to sign-extend virtual addresses
// that have the high bit set.
#define MAXVA (1L << (9 + 9 + 9 + 12 - 1))
```

最顶部 trampoline 用于页表映射，这里不详细展开。

如下是用于描述进程的结构。

```c
enum procstate { UNUSED, USED, SLEEPING, RUNNABLE, RUNNING, ZOMBIE };

// Per-process state
struct proc {
  struct spinlock lock;

  // p->lock must be held when using these:
  enum procstate state;        // Process state
  void *chan;                  // If non-zero, sleeping on chan
  int killed;                  // If non-zero, have been killed
  int xstate;                  // Exit status to be returned to parent's wait
  int pid;                     // Process ID

  // wait_lock must be held when using this:
  struct proc *parent;         // Parent process

  // these are private to the process, so p->lock need not be held.
  uint64 kstack;               // Virtual address of kernel stack
  uint64 sz;                   // Size of process memory (bytes)
  pagetable_t pagetable;       // User page table
  struct trapframe *trapframe; // data page for trampoline.S
  struct context context;      // swtch() here to run process
  struct file *ofile[NOFILE];  // Open files
  struct inode *cwd;           // Current directory
  char name[16];               // Process name (debugging)
};
```

每个进程都有一个执行的线程（？并不是编程中的“线程”）： a thread of execution (or thread for short) ，用于执行进程的指令。

线程可以被暂停与继续。切换进程就是通过暂停与继续线程。线程的状态就被存在 thread's stacks 中。 **Much of the state of a thread (local variables, function call return addresses) is stored on the thread’s stacks.**

每个进程都有两个栈： user stack 和 kernel stack (`p->kstack`) 。

执行用户指令时， kernel stack 是空的；通过系统调用或者中断切换到内核态时， user stack 还保存着数据。

进程的线程交替使用这两种 stack 。 A process’s thread alternates between actively using its user stack and its kernel stack.The kernel stack is separate and protected from user code) so that the kernel can execute even if a process has wrecked its user stack.

如何切换呢？使用 ecall 和 sret 两个指令。

A process can make a system call by executing the RISC-V ecall instruction. This instruction mraises the hardware privilege level and changes the program counter to a kernel-defined entry point. The code at the entry point switches to a kernel stack and executes the kernel instructions that implement the system call. When the system call completes, the kernel switches back to the user stack and returns to user space by calling the sret instruction, which lowers the hardware privilege level and resumes executing user instructions just after the system call instruction. A process’s thread can “block” in the kernel to wait for I/O, and resume where it left off when the I/O has finished.

### 2.6 Code: starting xv6 and the first process

xv6 内核如何启动？如何执行第一个进程？

通电后， a boot loader which is stored in read-only memory 跑起来了。这个 boot loader 把内核读入内存。然后在 machine mode 中， CPU 执行 kernel/entry.S 的 _entry 。此时，页表硬件是不可用的，虚地址就是物理地址。

如下是 kernel/entry.S 。

```S
	# qemu -kernel loads the kernel at 0x80000000
        # and causes each CPU to jump there.
        # kernel.ld causes the following code to
        # be placed at 0x80000000.
.section .text
.global _entry
_entry:
	# set up a stack for C.
        # stack0 is declared in start.c,
        # with a 4096-byte stack per CPU.
        # sp = stack0 + (hartid * 4096)
        la sp, stack0
        li a0, 1024*4
	csrr a1, mhartid
        addi a1, a1, 1
        mul a0, a0, a1
        add sp, sp, a0
	# jump to start() in start.c
        call start
spin:
        j spin
```

为什么把 xv6 kernel 存入物理地址 0x80000000 而非 0x0 ？因为 0x0:0x80000000 包含了 I/O 设备。

_entry 中的指令建了一个 stack ，由此 xv6 可以执行 C 代码。

_entry 讲 stack 指针寄存器 sp 放置为 stack0 + 4096 这个值，即栈顶（注意栈是从高地址到低地址增长的）。现在内核有了一个 stack ， _entry 可以调用 C 的代码了。即 kernel/start.c ：

```c
#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "defs.h"

void main();
void timerinit();

// entry.S needs one stack per CPU.
__attribute__ ((aligned (16))) char stack0[4096 * NCPU];

// a scratch area per CPU for machine-mode timer interrupts.
uint64 timer_scratch[NCPU][5];

// assembly code in kernelvec.S for machine-mode timer interrupt.
extern void timervec();

// entry.S jumps here in machine mode on stack0.
void
start()
{
  // set M Previous Privilege mode to Supervisor, for mret.
  unsigned long x = r_mstatus();
  x &= ~MSTATUS_MPP_MASK;
  x |= MSTATUS_MPP_S;
  w_mstatus(x);

  // set M Exception Program Counter to main, for mret.
  // requires gcc -mcmodel=medany
  w_mepc((uint64)main);

  // disable paging for now.
  w_satp(0);

  // delegate all interrupts and exceptions to supervisor mode.
  w_medeleg(0xffff);
  w_mideleg(0xffff);
  w_sie(r_sie() | SIE_SEIE | SIE_STIE | SIE_SSIE);

  // configure Physical Memory Protection to give supervisor mode
  // access to all of physical memory.
  w_pmpaddr0(0x3fffffffffffffull);
  w_pmpcfg0(0xf);

  // ask for clock interrupts.
  timerinit();

  // keep each CPU's hartid in its tp register, for cpuid().
  int id = r_mhartid();
  w_tp(id);

  // switch to supervisor mode and jump to main().
  asm volatile("mret");
}

// set up to receive timer interrupts in machine mode,
// which arrive at timervec in kernelvec.S,
// which turns them into software interrupts for
// devintr() in trap.c.
void
timerinit()
{
  // each CPU has a separate source of timer interrupts.
  int id = r_mhartid();

  // ask the CLINT for a timer interrupt.
  int interval = 1000000; // cycles; about 1/10th second in qemu.
  *(uint64*)CLINT_MTIMECMP(id) = *(uint64*)CLINT_MTIME + interval;

  // prepare information in scratch[] for timervec.
  // scratch[0..2] : space for timervec to save registers.
  // scratch[3] : address of CLINT MTIMECMP register.
  // scratch[4] : desired interval (in cycles) between timer interrupts.
  uint64 *scratch = &timer_scratch[id][0];
  scratch[3] = CLINT_MTIMECMP(id);
  scratch[4] = interval;
  w_mscratch((uint64)scratch);

  // set the machine-mode trap handler.
  w_mtvec((uint64)timervec);

  // enable machine-mode interrupts.
  w_mstatus(r_mstatus() | MSTATUS_MIE);

  // enable machine-mode timer interrupts.
  w_mie(r_mie() | MIE_MTIE);
}
```

### 插曲：内联汇编的格式

参考：[__asm__ volatile 之 C语言嵌入式汇编](https://cloud.tencent.com/developer/article/1434865)

`__asm__　__volatile__("Instruction List" : Output : Input : Clobber/Modify);` 中冒号表示这个汇编分为四个部分，比如你只想写第四部分，你可以这么写 `__asm__ __volatile__(:::memory);` （它向GCC声明：“我对内存作了改动”，GCC在编译的时候，会将此因素考虑进去）

如果 Output ， Input ， Clobber/Modify 都为空， Output ， Input 之前的冒号(`:`)既可以省略，也可以不省略。

如果都省略，则此汇编退化为一个基本内联汇编；否则，仍然是一个带有C/C++表达式的内联汇编，此时"Instruction List"中的寄存器写法要遵守相关规定，比如寄存器前必须使用两个百分号(`%%`)，而不是像基本汇编格式一样在寄存器前只使用一个百分号(%)。比如 `__asm__( " mov %%eax,  %%ebx" : : )` `__asm__( " mov %%eax, %%ebx" : )` 和 `__asm__( " mov %eax, %ebx" )` 都是正确的写法。

Output用来指定当前内联汇编语句的输出。我们看一看这个例子：

```c
__asm__("movl %%cr0, %0": "=a" (cr0));
```

这个内联汇编语句的输出部分为`"=r"(cr0)`，它是一个“操作表达式”，指定了一个输出操作。我们可以很清楚得看到这个输出操作由两部分组成：括号括住的部分(`cr0`)和引号引住的部分`"=a"`。这两部分都是每一个输出操作必不可少的。括号括住的部分是一个C/C++表达式，用来保存内联汇编的一个输出值，其操作就等于C/C++的相等赋值`cr0 = output_value`，因此，括号中的输出表达式只能是C/C++的左值表达式，也就是说它只能是一个可以合法的放在C/C++赋值操作中等号(`=`)  左边的表达式。那么右值`output_value`从何而来呢？

答案是引号中的内容，被称作“操作约束”（Operation Constraint），在这个例子中操作约束为`"=a"`，它包含两个约束：等号(`=`)和字母`a`，其中等号(`=`)说明括号中左值表达式 `cr0` 是一个  Write-Only 的，只能够被作为当前内联汇编的输入，而不能作为输入。而字母`a`是寄存器`EAX / AX / AL`的简写，说明`cr0`的值要从`eax`寄存器中获取，也就是说`cr0 = eax`，最终这一点被转化成汇编指令就是`movl %eax`, `address_of_cr0`。现在你应该清楚了吧，操作约束中会给出：到底从哪个寄存器传递值给`cr0`。

### 仔细阅读 start 发现是把汇编操作封装


start 这个函数会进行一些配置，这些配置只有在 machine mode 中才能被允许，然后再切换到 supervisor mode 。

start 如何切换到 supervisor mode 呢？仔细观察 start 前几行代码块，你会发现就是对汇编的封装，并且准确无误地做了以下事： it sets the previous privilege mode to supervisor in the register mstatus, it sets the return address to main by writing main’s address into the register mepc, disables virtual address translation in supervisor mode by writing 0 into the page-table register satp, and delegates all interrupts and exceptions to supervisor mode.

这里有个技巧，首先给 mstatus 寄存器的 previous privilege 那几位设置成 supervisor mode ，然后最后 start 结束时用的 mret 指令，自动就跳回 previous privilege 即我们想要的 supervisor mode 了。

依靠时钟，最终在 mret 指令后 program counter 被切换到 kernal/main.c （？如何做到的）。

在 main 中， After main (kernel/main.c:11) initializes several devices and subsystems, it creates the first process by calling userinit (kernel/proc.c:212). The first process executes a small program written in RISC-V assembly, initcode.S (user/initcode.S:1), which re-enters the kernel by invoking the exec system call.

### 2.7 Real world

微内核用在嵌入式多一点。

现代操作系统一个进程可以有多个线程，跑在多个 CPU 上。比如 Linux's clone （一种 fork 的变种）就可以帮助做到这点。

### 插曲：安装 riscv64-unknown-elf-gdb

安装 riscv64-unknown-elf-gdb 参考 https://www.cnblogs.com/dream397/p/15427944.html ：

- python 并非必须
- 在 Ubuntu 20.04 等系统中，python 和 python-dev 需要替换成 python2 和 python2-dev

```
sudo apt-get install libncurses5-dev python python-dev texinfo libreadline-dev
```

前往[清华镜像下载最新的 GDB 源代码](https://mirrors.tuna.tsinghua.edu.cn/gnu/gdb/?C=M&O=D)，解压源代码，并定位到目录，执行以下命令

```
--prefix 是安装的路径，按照以下指令会安装到 /usr/local/bin/ 下
--with-python 是 python2 的地址，它和 --enable-tui 都是为了支持后续安装一个可视化插件，并非必须
```

```bash
mkdir build
cd build/
../configure --prefix=/opt/riscv/ --with-python=/usr/bin/python --target=riscv64-unknown-elf --enable-tui=yes

make -j$(nproc)
make install

find /opt/riscv/bin riscv64-unknown-elf-gdb

安装完成：
/opt/riscv/bin
/opt/riscv/bin/riscv64-unknown-elf-gdb
/opt/riscv/bin/riscv64-unknown-elf-gdb-add-index
/opt/riscv/bin/riscv64-unknown-elf-run
riscv64-unknown-elf-gdb
```



### 2.8 Exercises 尝试 gdb

You can use gdb to observe the very first kernel-to-user transition. Run make qemu-gdb. In another window, in the same directory, run gdb. Type the gdb command break *0x3ffffff10e, which sets a breakpoint at the sret instruction in the kernel that jumps into user space. Type the continue gdb command. gdb should stop at the breakpoint, about to execute sret. Type stepi. gdb should now indicate that it is executing at address 0x0, which is in user space at the start of initcode.S.

![](./images/20220310gdb01.png)

如上：
- 左： make qemu-gdb
- 右： 调用 riscv64-unknown-elf-gdb ，之后连接 qemu

![](./images/20220310gdb02.png)

如上：
- 右： `break *0x3ffffff10e` 添加断点，然后 c 一下

![](./images/20220310gdb03.png)

如上:
- 右：果然如文章所说，在 `*0x3ffffff10e` 就是在执行 sret 的前一步，再执行 `stepi` 的话，就是 `in user space at the start of initcode.S`

![](./images/gdb.break.at.0x3ffffff10e.gif)

如上，在输出 `init: starting sh` 的每一个字母前，都会在 `0x3ffffff10e` 这里经过。

## 读书： Chapter 4 Traps and system calls

### 4.3 Code: Calling system calls

在 user/initcode.S ：
```s
# Initial process that execs /init.
# This code runs in user space.

#include "syscall.h"

# exec(init, argv)
# The user code places the arguments for exec in registers a0 and a1, and puts the system call
# number in a7. System call numbers match the entries in the syscalls array, a table of function
# pointers (kernel/syscall.c:108). The ecall instruction traps into the kernel and executes uservec,
# usertrap, and then syscall, as we saw above.
.globl start
start:
        la a0, init
        la a1, argv
        li a7, SYS_exec
        ecall

# for(;;) exit();
exit:
        li a7, SYS_exit
        ecall
        jal exit

# char init[] = "/init\0";
init:
  .string "/init\0"

# char *argv[] = { init, 0 };
.p2align 2
argv:
  .long init
  .long 0
```

上面提到的 a table of function pointers 在 kernel/syscall.c 里：

```c
static uint64 (*syscalls[])(void) = {
[SYS_fork]    sys_fork,
[SYS_exit]    sys_exit,
[SYS_wait]    sys_wait,
[SYS_pipe]    sys_pipe,
[SYS_read]    sys_read,
[SYS_kill]    sys_kill,
[SYS_exec]    sys_exec,
[SYS_fstat]   sys_fstat,
[SYS_chdir]   sys_chdir,
[SYS_dup]     sys_dup,
[SYS_getpid]  sys_getpid,
[SYS_sbrk]    sys_sbrk,
[SYS_sleep]   sys_sleep,
[SYS_uptime]  sys_uptime,
[SYS_open]    sys_open,
[SYS_write]   sys_write,
[SYS_mknod]   sys_mknod,
[SYS_unlink]  sys_unlink,
[SYS_link]    sys_link,
[SYS_mkdir]   sys_mkdir,
[SYS_close]   sys_close,
};

void
syscall(void)
{
  int num;
  struct proc *p = myproc();

  num = p->trapframe->a7;
  if(num > 0 && num < NELEM(syscalls) && syscalls[num]) {
    p->trapframe->a0 = syscalls[num]();
  } else {
    printf("%d %s: unknown sys call %d\n",
            p->pid, p->name, num);
    p->trapframe->a0 = -1;
  }
}
```

如上， syscall (kernel/syscall.c:133) retrieves the system call number from the saved a7 in the trapframe and uses it to index into syscalls. For the first system call, a7 contains SYS_exec (kernel/syscall.h:8), resulting in a call to the system call implementation function sys_exec.

When the system call implementation function returns, syscall records its return value in p->trapframe->a0. This will cause the original user-space call to exec() to return that value, since the C calling convention on RISC-V places return values in a0. System calls conventionally return negative numbers to indicate errors, and zero or positive numbers for success. If the system call number is invalid, syscall prints an error and returns −1.

### 4.4 Code: System call arguments 系统调用的参数

Because user code calls system call wrapper functions, the arguments are initially where the RISC-V C calling convention places them: in registers. The kernel trap code saves user registers to the current process’s trap frame, where kernel code can find them. The functions argint, argaddr, and argfd retrieve the n ’th system call argument from the trap frame as an integer, pointer, or a file descriptor. They all call argraw to retrieve the appropriate saved user register (kernel/syscall.c:35).

一般而言，从寄存器里读这些参数。如下是 kernel/syscall.c ：

```c
static uint64
argraw(int n)
{
  struct proc *p = myproc();
  switch (n) {
  case 0:
    return p->trapframe->a0;
  case 1:
    return p->trapframe->a1;
  case 2:
    return p->trapframe->a2;
  case 3:
    return p->trapframe->a3;
  case 4:
    return p->trapframe->a4;
  case 5:
    return p->trapframe->a5;
  }
  panic("argraw");
  return -1;
}

// Fetch the nth 32-bit system call argument.
int
argint(int n, int *ip)
{
  *ip = argraw(n);
  return 0;
}

// Retrieve an argument as a pointer.
// Doesn't check for legality, since
// copyin/copyout will do that.
int
argaddr(int n, uint64 *ip)
{
  *ip = argraw(n);
  return 0;
}

// Fetch the nth word-sized system call argument as a null-terminated string.
// Copies into buf, at most max.
// Returns string length if OK (including nul), -1 if error.
int
argstr(int n, char *buf, int max)
{
  uint64 addr;
  if(argaddr(n, &addr) < 0)
    return -1;
  return fetchstr(addr, buf, max);
}
```

有些系统调用会传入 pointers 作为参数，这面临两个问题：
- 用户传进来的是有问题的指针
- kernel page table mappings 与 user page table mappings 与不同，用 kernel 直接用普通指令读或存用户提供的地址是不行的

kernel 里有函数用于与 user-supplied address 转换数据。比如 fetchstr 是一个例子 kernel/syscall.c:25 ，作用是从 user space 获取文件名称：

```c
// Fetch the nul-terminated string at addr from the current process.
// Returns length of string, not including nul, or -1 for error.
int
fetchstr(uint64 addr, char *buf, int max)
{
  struct proc *p = myproc();
  int err = copyinstr(p->pagetable, buf, addr, max);
  if(err < 0)
    return err;
  return strlen(buf);
}
```

注意到这里主要的工作由 copyinstr 做的 kernel/vm.c:406:

```c
// Copy a null-terminated string from user to kernel.
// Copy bytes to dst from virtual address srcva in a given page table,
// until a '\0', or max.
// Return 0 on success, -1 on error.
int
copyinstr(pagetable_t pagetable, char *dst, uint64 srcva, uint64 max)
{
  uint64 n, va0, pa0;
  int got_null = 0;

  while(got_null == 0 && max > 0){
    va0 = PGROUNDDOWN(srcva); // #define PGROUNDDOWN(a) (((a)) & ~(PGSIZE-1))
    pa0 = walkaddr(pagetable, va0);  // 在虚地址与实地址之间做了映射
    if(pa0 == 0)
      return -1;
    n = PGSIZE - (srcva - va0);
    if(n > max)
      n = max;

    char *p = (char *) (pa0 + (srcva - va0));
    while(n > 0){
      if(*p == '\0'){
        *dst = '\0';
        got_null = 1;
        break;
      } else {
        *dst = *p;
      }
      --n;
      --max;
      p++;
      dst++;
    }

    srcva = va0 + PGSIZE;  // 到下一页
  }
  if(got_null){
    return 0;
  } else {
    return -1;
  }
}
```

## 阅读一些代码

### 用 perl 脚本生成 S 汇编代码

user/usys.pl ：

```perl
sub entry {
    my $name = shift;
    print ".global $name\n";
    print "${name}:\n";
    print " li a7, SYS_${name}\n";
    print " ecall\n";
    print " ret\n";
}
	
entry("fork");
entry("exit");
...
```

生成如下代码 user/usys.S ：

```S
.global fork
fork:
 li a7, SYS_fork
 ecall
 ret
.global exit
exit:
 li a7, SYS_exit
 ecall
 ret
```
