# LEC 11 (rtm): Thread switching

目录：

<!-- @import "[TOC]" {cmd="toc" depthFrom=2 depthTo=2 orderedList=false} -->

<!-- code_chunk_output -->

- [Chapter 7 Scheduling](#chapter-7-scheduling)

<!-- /code_chunk_output -->

细分目录：

<!-- @import "[TOC]" {cmd="toc" depthFrom=2 depthTo=6 orderedList=false} -->

<!-- code_chunk_output -->

- [Chapter 7 Scheduling](#chapter-7-scheduling)
  - [7.4 Code: mycpu and myproc](#74-code-mycpu-and-myproc)

<!-- /code_chunk_output -->

## Chapter 7 Scheduling

### 7.4 Code: mycpu and myproc

每个 CPU core 都有自己的一组寄存器。

```c
// kernel/proc.h
// Per-CPU state.
struct cpu {
  struct proc *proc;          // The process running on this cpu, or null.
  struct context context;     // swtch() here to enter scheduler().
  int noff;                   // Depth of push_off() nesting.
  int intena;                 // Were interrupts enabled before push_off()?
};

// kernel/proc.c
// Return this CPU's cpu struct.
// Interrupts must be disabled.
struct cpu*
mycpu(void) {
  int id = cpuid();
  struct cpu *c = &cpus[id];
  return c;
}
```

CPU 的 tp 寄存器是 CPU 的 id ，在开机时就被设置了。

```c
// kernel/start.c
// void start()
  // keep each CPU's hartid in its tp register, for cpuid().
  int id = r_mhartid();
  w_tp(id);
```

The compiler guarantees never to use the tp register. It would be more convenient if RISC-V allowed xv6 to read the current hartid directly, but that is allowed only in machine mode, not in supervisor mode.

关于中断是否关闭：

The return values of `cpuid` and `mycpu` are fragile: if the timer were to interrupt and cause the thread to yield and then move to a different CPU, a previously returned value would no longer be correct. To avoid this problem, xv6 requires that callers disable interrupts, and only enable them after they finish using the returned `struct cpu`.

The function `myproc` (kernel/proc.c:68) returns the struct proc pointer for the process that is running on the current CPU. `myproc` disables interrupts, invokes mycpu, fetches the current process pointer (`c->proc`) out of the struct cpu, and then enables interrupts. The return value of `myproc` is safe to use even if interrupts are enabled: if a timer interrupt moves the calling process to a different CPU, its `struct proc` pointer will stay the same.
