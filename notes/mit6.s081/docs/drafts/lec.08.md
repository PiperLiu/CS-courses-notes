# LEC 8 (fk): Page faults

目录：

<!-- @import "[TOC]" {cmd="toc" depthFrom=2 depthTo=2 orderedList=false} -->

<!-- code_chunk_output -->

- [读书 4.6 Page-fault exceptions](#读书-46-page-fault-exceptions)

<!-- /code_chunk_output -->

细分目录：

<!-- @import "[TOC]" {cmd="toc" depthFrom=2 depthTo=6 orderedList=false} -->

<!-- code_chunk_output -->

- [读书 4.6 Page-fault exceptions](#读书-46-page-fault-exceptions)
  - [什么是 page-fault exception](#什么是-page-fault-exception)

<!-- /code_chunk_output -->

## 读书 4.6 Page-fault exceptions

注意， fault 不是一定是程序崩溃。在 Xv6 中， fault 被最简形式地设计了：
- 在 user space 发生，则 kernel kills the faulting process
- 在 kernel 发生，则 kernel panics

实际上，面对 fault ，可以有很多其他操作。比如 kernel page faults 可以实现 copy-on-write (COW) fork.

具体如何实现 COW fork 放在正式课堂以及 lab 中。

### 什么是 page-fault exception

When a CPU cannot translate a virtual address to a physical address, the CPU generates a page-fault exception. RISC-V has three different kinds of page fault: 
- load page faults (when a load instruction cannot translate its virtual address),
- store page faults (when a store instruction cannot translate its virtual address),
- and instruction page faults (when the address for an instruction doesn’t translate).

The value in the `scause register` indicates the type of the page fault and the `stval register` contains the address that couldn’t be translated. 注意，专门有硬件寄存器用来记录 page fault 的类型和地址。
