# Lab: page tables

本节作业：
- https://github.com/PiperLiu/xv6-labs-2020/tree/pgtbl

目录：

<!-- @import "[TOC]" {cmd="toc" depthFrom=2 depthTo=2 orderedList=false} -->

<!-- code_chunk_output -->



<!-- /code_chunk_output -->

细分目录：

<!-- @import "[TOC]" {cmd="toc" depthFrom=2 depthTo=6 orderedList=false} -->

<!-- code_chunk_output -->

- [Print a page table](#print-a-page-table)
- [A kernel page table per process](#a-kernel-page-table-per-process)
- [Simplify copyin/copyinstr](#simplify-copyincopyinstr)

<!-- /code_chunk_output -->

### Print a page table

Define a function called `vmprint()`. It should take a pagetable_t argument, and print that pagetable in the format described below. Insert `if(p->pid==1)` `vmprint(p->pagetable)` in exec.c just before the return argc, to print the first process's page table. You receive full credit for this assignment if you pass the pte printout test of make grade.

Now when you start xv6 it should print output like this, describing the page table of the first process at the point when it has just finished `exec()`ing `init`:

```
page table 0x0000000087f6e000
..0: pte 0x0000000021fda801 pa 0x0000000087f6a000
.. ..0: pte 0x0000000021fda401 pa 0x0000000087f69000
.. .. ..0: pte 0x0000000021fdac1f pa 0x0000000087f6b000
.. .. ..1: pte 0x0000000021fda00f pa 0x0000000087f68000
.. .. ..2: pte 0x0000000021fd9c1f pa 0x0000000087f67000
..255: pte 0x0000000021fdb401 pa 0x0000000087f6d000
.. ..511: pte 0x0000000021fdb001 pa 0x0000000087f6c000
.. .. ..510: pte 0x0000000021fdd807 pa 0x0000000087f76000
.. .. ..511: pte 0x0000000020001c0b pa 0x0000000080007000
```

The first line displays the argument to vmprint. After that there is a line for each PTE, including PTEs that refer to page-table pages deeper in the tree. Each PTE line is indented by a number of ".." that indicates its depth in the tree. Each PTE line shows the PTE index in its page-table page, the pte bits, and the physical address extracted from the PTE. Don't print PTEs that are not valid. In the above example, the top-level page-table page has mappings for entries 0 and 255. The next level down for entry 0 has only index 0 mapped, and the bottom-level for that index 0 has entries 0, 1, and 2 mapped.

Your code might emit different physical addresses than those shown above. The number of entries and the virtual addresses should be the same.

Some hints:
- You can put `vmprint()` in kernel/vm.c.
- Use the macros at the end of the file kernel/riscv.h.
- The function freewalk may be inspirational.
- Define the prototype for vmprint in kernel/defs.h so that you can call it from exec.c.
- Use `%p` in your printf calls to print out full 64-bit hex PTEs and addresses as shown in the example.

核心代码如下：

```c
void
vmprint_level(pagetable_t, int level);

void
vmprint(pagetable_t pagetable)
{
  printf("page table %p\n", pagetable);
  vmprint_level(pagetable, 2);
}

void
vmprint_level(pagetable_t pagetable, int level)
{
  if (level < 0) return ;

  for (int i = 0; i < 512; ++ i)
  {
    pte_t pte = pagetable[i];
    if (pte & PTE_V)  // 这个 PTE 可用
    {
      for (int j = 2; j >= level; -- j)
        printf("..");
      uint64 child = PTE2PA(pte);
      printf("%d: pte %p pa %p\n", i, pte, child);
      vmprint_level((pagetable_t)child, level - 1);
    }
  }
}
```

这里有道分析题：

Explain the output of vmprint in terms of Fig 3-4 from the text. What does page 0 contain? What is in page 2? When running in user mode, could the process read/write the memory mapped by page 1?

```
page table 0x0000000087f6e000
..0: pte 0x0000000021fda801 pa 0x0000000087f6a000
....0: pte 0x0000000021fda401 pa 0x0000000087f69000
......0: pte 0x0000000021fdac1f pa 0x0000000087f6b000
......1: pte 0x0000000021fda00f pa 0x0000000087f68000
......2: pte 0x0000000021fd9c1f pa 0x0000000087f67000
..255: pte 0x0000000021fdb401 pa 0x0000000087f6d000
....511: pte 0x0000000021fdb001 pa 0x0000000087f6c000
......510: pte 0x0000000021fdd807 pa 0x0000000087f76000
......511: pte 0x0000000020001c0b pa 0x0000000080007000
```

参考：https://zhuanlan.zhihu.com/p/347172409

```bash
objdump -p user/_init 

user/_init:     file format elf64-little

Program Header:
    LOAD off    0x00000000000000b0 vaddr 0x0000000000000000 paddr 0x0000000000000000 align 2**3
         filesz 0x00000000000009f8 memsz 0x0000000000000a10 flags rwx
   STACK off    0x0000000000000000 vaddr 0x0000000000000000 paddr 0x0000000000000000 align 2**4
         filesz 0x0000000000000000 memsz 0x0000000000000000 flags rw-
```

如上，对于 _init （第一个进程来讲），在 exec 中将会执行把 LOAD 加载到内存的逻辑：

```c
  // Load program into memory.
  for(i=0, off=elf.phoff; i<elf.phnum; i++, off+=sizeof(ph)){
    if(readi(ip, 0, (uint64)&ph, off, sizeof(ph)) != sizeof(ph))
      goto bad;
    if(ph.type != ELF_PROG_LOAD)
      continue;
    if(ph.memsz < ph.filesz)
      goto bad;
    if(ph.vaddr + ph.memsz < ph.vaddr)
      goto bad;
    uint64 sz1;
    if((sz1 = uvmalloc(pagetable, sz, ph.vaddr + ph.memsz)) == 0)
      goto bad;
    sz = sz1;
    if(ph.vaddr % PGSIZE != 0)
      goto bad;
    if(loadseg(pagetable, ph.vaddr, ip, ph.off, ph.filesz) < 0)
      goto bad;
  }
```

因此， PAGE 0 是这个 LOAD 中的内容。

而 STACK 的 `ph.type != ELF_PROG_LOAD` ，因此不会在这个 for 里被加载。

如下，在 exec 截下来的逻辑是：

```c
  // Allocate two pages at the next page boundary.
  // Use the second as the user stack.
  sz = PGROUNDUP(sz);
  uint64 sz1;
  if((sz1 = uvmalloc(pagetable, sz, sz + 2*PGSIZE)) == 0)
    goto bad;
  sz = sz1;
  uvmclear(pagetable, sz-2*PGSIZE);
  sp = sz;
  stackbase = sp - PGSIZE;
```

可以看出是分配一页作为进程的栈，一页作为 guard page 。

所以 page2 是进程的栈， page1 是guard page，看一下 uvmclear 可以发现 guard page 的 PTE_U 被设为 0 ，用户模式代码访问的话硬件会产生一个 page fault 异常。（ uvmclear 如下 ）

```c
// mark a PTE invalid for user access.
// used by exec for the user stack guard page.
void
uvmclear(pagetable_t pagetable, uint64 va)
{
  pte_t *pte;
  
  pte = walk(pagetable, va, 0);
  if(pte == 0)
    panic("uvmclear");
  *pte &= ~PTE_U;
}
```

### A kernel page table per process

Xv6 has a single kernel page table that's used whenever it executes in the kernel. The kernel page table is a direct mapping to physical addresses, so that kernel virtual address x maps to physical address x. Xv6 also has a separate page table for each process's user address space, containing only mappings for that process's user memory, starting at virtual address zero. **Because the kernel page table doesn't contain these mappings, user addresses are not valid in the kernel. Thus, when the kernel needs to use a user pointer passed in a system call (e.g., the buffer pointer passed to `write()`), the kernel must first translate the pointer to a physical address. The goal of this section and the next is to allow the kernel to directly dereference user pointers.**

Your first job is to modify the kernel so that every process uses its own copy of the kernel page table when executing in the kernel. Modify struct proc to maintain a kernel page table for each process, and modify the scheduler to switch kernel page tables when switching processes. **For this step, each per-process kernel page table should be identical to the existing global kernel page table.** You pass this part of the lab if usertests runs correctly.

Read the book chapter and code mentioned at the start of this assignment; it will be easier to modify the virtual memory code correctly with an understanding of how it works. Bugs in page table setup can cause traps due to missing mappings, can cause loads and stores to affect unexpected pages of physical memory, and can cause execution of instructions from incorrect pages of memory.

Some hints:
- Add a field to `struct proc` for the process's kernel page table.
- A reasonable way to produce a kernel page table for a new process is to implement a modified version of `kvminit` that makes a new page table instead of modifying `kernel_pagetable`. You'll want to call this function from `allocproc`.
- Make sure that each process's kernel page table has a mapping for that process's kernel stack. In unmodified xv6, all the kernel stacks are set up in `procinit`. You will need to move some or all of this functionality to `allocproc`.
  - procinit 在 boot 的时候为每个进程在 kernel page table 里申请一个 kernel stack ，这里我们要把这个逻辑抽出来，不让 kernel stack 在一开始就映射好
- Modify `scheduler()` to load the process's kernel page table into the core's satp register (see `kvminithart` for inspiration). Don't forget to call `sfence_vma()` after calling `w_satp()`.
  - scheduler 就是最基本的 main 的最终的死循环函数，负责不断切换进程、允许中断的存在；这里我们要在切换进程的时候，把 satp 寄存器的页表也换成进程自己的 kernel page table 的根地址
- `scheduler()` should use `kernel_pagetable` when no process is running.
- Free a process's kernel page table in `freeproc`.
- You'll need a way to free a page table without also freeing the leaf physical memory pages.
  - 这里进程 free 了，其独有的 kernel page 也被 free 了，但是注意，别把真正的 kernel page 对应的物理地址给 free 了
- `vmprint` may come in handy to debug page tables.
- It's OK to modify xv6 functions or add new functions; you'll probably need to do this in at least `kernel/vm.c` and `kernel/proc.c`. (But, don't modify `kernel/vmcopyin.c`, `kernel/stats.c`, `user/usertests.c`, and `user/stats.c`.)
- A missing page table mapping will likely cause the kernel to encounter a page fault. It will print an error that includes `sepc=0x00000000XXXXXXXX`. You can find out where the fault occurred by searching for `XXXXXXXX` in `kernel/kernel.asm`.

一开始没懂这题要干嘛。原来这题不能单独看，这道题和下一道题结合起来：给每个进程一个 kernel page table ，把进程自己的 page table 也加到 kernel page table 里面。这样，从用户态做系统调用的时候，就不需要针对 user page table 的东西单独 walk 一下把虚地址转为实地址了，只需要跟着 user 进程的 kernel page table 共用一个 SATP 寄存器对应的 page table directory ，就可以把 walk 都交给硬件了。这里这个 lab 首先是把 kernel page 加到各个进程，下个 lab 再将两种页表结合。

其实本题理解了思路就是挺简单一件事，就是在用户进程 alloc 的时候把 kerenl stack 和 kernel page table 分给 proc ；之后在 free 的时候给 unmap 或者 free 掉，按照 hint 提示一步一步来就好。有些细节比如 CLINT 不 map 以及修改 kvmpa 函数参考了[博客](https://blog.csdn.net/u013577996/article/details/109582932)，这些内容 hint 没有提示。

下面是工程顺序，有些头文件啥的细枝末节就不记录了：

首先，给 kernel/proc.h 添加：

```c
struct proc {
  // 进程的内核页表
  pagetable_t kpagetable;
};
```

kernel/vm.c

```c
// 给每个用户进程的 kernel page table 用的
void
ukvmmap(pagetable_t kpagetable, uint64 va, uint64 pa, uint64 sz, int perm)
{
  if (mappages(kpagetable, va, sz, pa, perm) != 0)
    panic("ukvmmap");
}

/*
 * 用于给用户创建独属的 kernel page
 * 完全参考 kvminit 注意不要有 CLINT
 * 因为 CLINT 只有开机时使用，另外 lab 3.3 要求用户虚拟地址不要超过 PLIC
 * 所以我们这里把 PLIC 之下的 CLINT 0x10000 省出来
 */
pagetable_t
ukvminit()
{
  pagetable_t kpagetable = (pagetable_t)kalloc();
  memset(kpagetable, 0, PGSIZE);

  // uart registers
  ukvmmap(kpagetable, UART0, UART0, PGSIZE, PTE_R | PTE_W);

  // virtio mmio disk interface
  ukvmmap(kpagetable, VIRTIO0, VIRTIO0, PGSIZE, PTE_R | PTE_W);

  // PLIC
  ukvmmap(kpagetable, PLIC, PLIC, 0x400000, PTE_R | PTE_W);

  // map kernel text executable and read-only.
  ukvmmap(kpagetable, KERNBASE, KERNBASE, (uint64)etext - KERNBASE, PTE_R | PTE_X);

  // map kernel data and the physical RAM we'll make use of.
  ukvmmap(kpagetable, (uint64)etext, (uint64)etext, PHYSTOP - (uint64)etext, PTE_R | PTE_W);

  // map the trampoline for trap entry/exit to
  // the highest virtual address in the kernel.
  ukvmmap(kpagetable, TRAMPOLINE, (uint64)trampoline, PGSIZE, PTE_R | PTE_X);

  return kpagetable;
}
```

kernel/proc.c

```c
// initialize the proc table at boot time.
void
procinit(void)
{
  struct proc *p;
  
  initlock(&pid_lock, "nextpid");
  for(p = proc; p < &proc[NPROC]; p++) {
      initlock(&p->lock, "proc");

      // Allocate a page for the process's kernel stack.
      // Map it high in memory, followed by an invalid
      // guard page.
      /* 删除分配 kernel stack 的内容
      char *pa = kalloc();
      if(pa == 0)
        panic("kalloc");
      uint64 va = KSTACK((int) (p - proc));
      kvmmap(va, (uint64)pa, PGSIZE, PTE_R | PTE_W);
      p->kstack = va;
      */
  }
  kvminithart();
}

// Look in the process table for an UNUSED proc.
// If found, initialize state required to run in the kernel,
// and return with p->lock held.
// If there are no free procs, or a memory allocation fails, return 0.
static struct proc*
allocproc(void)
{
  struct proc *p;
  ...
  // proc 独有的 kernel page table
  p->kpagetable = ukvminit();
  // procinit 中的内容拿进来，给 kpagetable 分配一个 STACK page
  // Allocate a page for the process's kernel stack.
  // Map it high in memory, followed by an invalid
  // guard page.
  char *pa = kalloc();
  if(pa == 0)
    panic("kalloc");
  /* kstack 的地址做一些改动
  uint64 va = KSTACK((int) (p - proc));
  */
  uint64 va = KSTACK((int) 0);
  ukvmmap(p->kpagetable, va, (uint64)pa, PGSIZE, PTE_R | PTE_W);
  p->kstack = va;

  // An empty user page table.
  p->pagetable = ...
  return p;
}
```

上面是分配 proc 的相关逻辑。

接下来是在 proc 运行中使用 kernel page table 。

kernel/proc.c

```c
// Per-CPU process scheduler.
// Each CPU calls scheduler() after setting itself up.
// Scheduler never returns.  It loops, doing:
//  - choose a process to run.
//  - swtch to start running that process.
//  - eventually that process transfers control
//    via swtch back to the scheduler.
void
scheduler(void)
{
  struct proc *p;
  struct cpu *c = mycpu();
  
  c->proc = 0;
  for(;;){
    // Avoid deadlock by ensuring that devices can interrupt.
    intr_on();
    
    int found = 0;
    for(p = proc; p < &proc[NPROC]; p++) {
      acquire(&p->lock);
      if(p->state == RUNNABLE) {
        // Switch to chosen process.  It is the process's job
        // to release its lock and then reacquire it
        // before jumping back to us.
        p->state = RUNNING;
        c->proc = p;

        // 在把 p 交给 cpu 前，把 cpu 的 SATP 寄存器换成 p 的 kernel page table 的
        w_satp(MAKE_SATP(p->kpagetable));
        sfence_vma();

        swtch(&c->context, &p->context);

        // 别忘了用 kvminithart 把 SATP 换回来
        kvminithart();

        // Process is done running for now.
        // It should have changed its p->state before coming back.
        c->proc = 0;

        found = 1;
      }
      release(&p->lock);
    }
#if !defined (LAB_FS)
    if(found == 0) {
      intr_on();
      asm volatile("wfi");
    }
#else
    ;
#endif
  }
}
```

kernel/vm.c

```c
// 下面这样也要有，否则 kernel/proc.h:87:19: error: field ‘lock’ has incomplete type （无法引入 proc.h）
#include "spinlock.h"
// 下面这个要有，否则 kernel/vm.c:179:22: error: dereferencing pointer to incomplete type ‘struct proc’
#include "proc.h"

// translate a kernel virtual address to
// a physical address. only needed for
// addresses on the stack.
// assumes va is page aligned.
uint64
kvmpa(uint64 va)
{
  uint64 off = va % PGSIZE;
  pte_t *pte;
  uint64 pa;

  /* kvmpa 会在进程中被调用，这里使用 myproc() 自己的页表
  pte = walk(kernel_pagetable, va, 0); */
  pte = walk(myproc()->kpagetable, va, 0);
  if(pte == 0)
    panic("kvmpa");
  if((*pte & PTE_V) == 0)
    panic("kvmpa");
  pa = PTE2PA(*pte);
  return pa+off;
}
```

接下来是释放 proc 是需要的逻辑。

kernel/vm.c

```c
// 仿 uvmfree ，把 kstack 清空后 freewalk
void
uvmfreekstack(pagetable_t pagetable, uint64 kstack)
{
  uvmunmap(pagetable, kstack, 1, 1);
  freewalk(pagetable);
}
```

kernel/proc.c

```c
// 仿造 vm.c etext 否则报错没有 etext , trampoline
extern char etext[]; // kernel.ld sets this to end of kernel code.
// 仿造 proc_freepagetable 把 kernel page table 也给 free 了
// 注意用 uvmunmap(..., do_free=0) 别把物理内存给释放了
// 用 uvmfree 把 kstack 的物理内存给释放了
void
proc_freekpagetable(pagetable_t kpagetable, uint64 kstack)
{
  uvmunmap(kpagetable, UART0, PGSIZE / PGSIZE, 0);
  uvmunmap(kpagetable, VIRTIO0, PGSIZE / PGSIZE, 0);
  uvmunmap(kpagetable, PLIC, 0x400000 / PGSIZE, 0);
  uvmunmap(kpagetable, KERNBASE, ((uint64)etext - KERNBASE) / PGSIZE, 0);
  uvmunmap(kpagetable, (uint64)etext, (PHYSTOP - (uint64)etext) / PGSIZE, 0);
  uvmunmap(kpagetable, TRAMPOLINE, PGSIZE / PGSIZE, 0);
  /*
  // Free user memory pages,
  // then free page-table pages.
  uvmfree(pagetable, sz);
  */
  // 仿 uvmfree
  uvmfreekstack(kpagetable, kstack);
}

// free a proc structure and the data hanging from it,
// including user pages.
// p->lock must be held.
static void
freeproc(struct proc *p)
{
  if(p->trapframe)
    kfree((void*)p->trapframe);
  p->trapframe = 0;
  // 新增
  if(p->kpagetable)
    proc_freekpagetable(p->kpagetable, p->kstack);
  p->kpagetable = 0;
  if(p->pagetable)
    proc_freepagetable(p->pagetable, p->sz);
  p->pagetable = 0;
  p->sz = 0;
  p->pid = 0;
  p->parent = 0;
  p->name[0] = 0;
  p->chan = 0;
  p->killed = 0;
  p->xstate = 0;
  p->state = UNUSED;
}
```

在 make qemu 后，使用用户进程 usertests 测试， ALL TESTS PASSED 。

### Simplify copyin/copyinstr

The kernel's copyin function reads memory pointed to by user pointers. It does this by translating them to physical addresses, which the kernel can directly dereference. It performs this translation by walking the process page-table in software. **Your job in this part of the lab is to add user mappings to each process's kernel page table (created in the previous section) that allow copyin (and the related string function copyinstr) to directly dereference user pointers.**

Replace the body of copyin in kernel/vm.c with a call to copyin_new (defined in kernel/vmcopyin.c); do the same for copyinstr and copyinstr_new. Add mappings for user addresses to each process's kernel page table so that copyin_new and copyinstr_new work. You pass this assignment if usertests runs correctly and all the make grade tests pass.

This scheme relies on the user virtual address range not overlapping the range of virtual addresses that the kernel uses for its own instructions and data. Xv6 uses virtual addresses that start at zero for user address spaces, and luckily the kernel's memory starts at higher addresses. **However, this scheme does limit the maximum size of a user process to be less than the kernel's lowest virtual address. After the kernel has booted, that address is 0xC000000 in xv6, the address of the PLIC registers; see kvminit() in kernel/vm.c, kernel/memlayout.h, and Figure 3-4 in the text. You'll need to modify xv6 to prevent user processes from growing larger than the PLIC address.**

Some hints:
- Replace copyin() with a call to copyin_new first, and make it work, before moving on to copyinstr.
- At each point where the kernel changes a process's user mappings, change the process's kernel page table in the same way. Such points include fork(), exec(), and sbrk().
- Don't forget that to include the first process's user page table in its kernel page table in userinit.
- What permissions do the PTEs for user addresses need in a process's kernel page table? (A page with PTE_U set cannot be accessed in kernel mode.)
- Don't forget about the above-mentioned PLIC limit.

Linux uses a technique similar to what you have implemented. Until a few years ago many kernels used the same per-process page table in both user and kernel space, with mappings for both user and kernel addresses, to avoid having to switch page tables when switching between user and kernel space. However, that setup allowed side-channel attacks such as Meltdown and Spectre.

Explain why the third test srcva + len < srcva is necessary in copyin_new(): give values for srcva and len for which the first two test fail (i.e., they will not cause to return -1) but for which the third one is true (resulting in returning -1).

说白了，就是接着上一个实验，把两个页表合并。这涉及到你要在创建进程的地方进行魔改。

