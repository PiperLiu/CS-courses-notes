# LEC 6 (rtm): Isolation & system call entry/exit
目录：

<!-- @import "[TOC]" {cmd="toc" depthFrom=2 depthTo=2 orderedList=false} -->

<!-- code_chunk_output -->

- [读书 Chapter 4 Traps and system calls](#读书-chapter-4-traps-and-system-calls)

<!-- /code_chunk_output -->

细分目录：

<!-- @import "[TOC]" {cmd="toc" depthFrom=2 depthTo=6 orderedList=false} -->

<!-- code_chunk_output -->

- [读书 Chapter 4 Traps and system calls](#读书-chapter-4-traps-and-system-calls)
  - [4.1 RISC-V trap machinery](#41-risc-v-trap-machinery)
  - [4.2 Traps from user space](#42-traps-from-user-space)
  - [4.3 Code: Calling system calls](#43-code-calling-system-calls)
  - [4.4 Code: System call arguments](#44-code-system-call-arguments)
  - [4.5 Traps from kernel space](#45-traps-from-kernel-space)
  - [4.7 Real world](#47-real-world)
  - [4.8 Exercises](#48-exercises)

<!-- /code_chunk_output -->

## 读书 Chapter 4 Traps and system calls

打断 CPU 执行指令的三个方式：
- system call （ecall 指令）
- exception
- device interrupt

上面统称为 trap 。 **We often want traps to be transparen.** 此外，三种 trap 也可以划分成：
- traps from user space
- traps from kernel space
- traps from timer interrupts

### 4.1 RISC-V trap machinery

关于 trap 有几个重要的寄存器：
- `stvec` The kernel writes the address of its trap handler here; the RISC-V jumps here to handle a trap
- `sepc` When a trap occurs, RISC-V saves the program counter here (since the `pc` is then overwritten with `stvec`). The `sret` (return from trap) instruction copies `sepc` to the `pc`. The kernel can write to `sepc` to control where `sret` goes.
- `scause` The RISC-V puts a number here that describes the reason for the trap.
- `sscratch` The kernel places a value here that comes in handy at the very start of a trap handler
- `sstatus` The SIE bit in `sstatus` controls whether device interrupts are enabled. If the kernel clears SIE, the RISC-V will defer device interrupts until the kernel sets SIE. The SPP bit indicates whether a trap came from user mode or supervisor mode, and controls to what
mode `sret` returns.

上述寄存器在 user mode 不能用。

不管是何种 trap （除了 timer interrupts）， RISC-V 的硬件做如下的事：
1. If the trap is a device interrupt, and the sstatus SIE bit is clear, don’t do any of the following.
2. Disable interrupts by clearing SIE.
3. Copy the pc to sepc.
4. Save the current mode (user or supervisor) in the SPP bit in sstatus.
5. Set scause to reflect the trap’s cause.
6. Set the mode to supervisor.
7. Copy stvec to the pc.
8. Start executing at the new pc.

注意， CPU 并不会切换页表、或者切换栈为 kernel stack ，也不会保存除了 pc 以外的寄存器。

RISC-V 的设计者希望这样能提供最大的灵活性，因为并不是每个 trap 都需要切换页表。

是否还可以再简化 trap 做的事呢？比如不自动地修改 pc 。这是不行的，因为如果你在内核态执行用户态的指令，这就打破了隔离。

### 4.2 Traps from user space

一个 trap from user space 的路径是：
```
uservec (kernel/trampoline.S:16),
then usertrap (kernel/trap.c:37);
and when returning, usertrapret (kernel/trap.c:90)
and then userret (kernel/trampoline.S:16)
```

这里最为精巧的设计我认为是 trampoline 段：之所以在所有的用户页表和 kernel 页表中相同的位置放上相同的代码，就是用于 trap 时不能使用 pc 和页表的空档期。具体流程课上老师会讲，见课堂笔记。

### 4.3 Code: Calling system calls

这个我们在第二节就体验过。

### 4.4 Code: System call arguments

The kernel implements functions that safely transfer data to and from user-supplied addresses.  fetchstr is an example (kernel/syscall.c:25). File system calls such as exec use fetchstr to retrieve string file-name arguments from user space. fetchstr calls copyinstr to do the hard work.

### 4.5 Traps from kernel space

`kernelvec` (kernel/kernelvec.S:10) saves the registers on the stack of the interrupted kernel thread, which makes sense because the register values belong to that thread. This is particularly important if the trap causes a switch to a different thread – in that case the trap will actually return on the stack of the new thread, leaving the interrupted thread’s saved registers safely on its stack.

Xv6 sets a CPU’s stvec to kernelvec when that CPU enters the kernel from user space; you can see this in usertrap (kernel/trap.c:29). There’s a window of time when the kernel is executing but stvec is set to uservec, and it’s crucial that device interrupts be disabled during that window. Luckily the RISC-V always disables interrupts when it starts to take a trap, and xv6 doesn’t enable them again until after it sets stvec.

### 4.7 Real world

这里还是对应上节课关于 page table 的 lab ，讲的是用户进程的页表和内核页表可以有一部分重叠，这样免去了 trap from user space into kernel 的页表的切换，效率更高。

Xv6 没有用这个思路，因为可能导致 bug 和复杂度。Xv6 avoids them in order to reduce the chances of security bugs in the kernel due to inadvertent use of user pointers, and to reduce some complexity that would be required to ensure that user and kernel virtual addresses don’t overlap.

### 4.8 Exercises

这个 exercises 应该就是关于 page table 的 lab ，上节课刚刚做完。
