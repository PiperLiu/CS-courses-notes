# LEC 13 (rtm): sleep&wakeup and code

目录：

<!-- @import "[TOC]" {cmd="toc" depthFrom=2 depthTo=2 orderedList=false} -->

<!-- code_chunk_output -->

- [Chapter 7 Scheduling](#chapter-7-scheduling)

<!-- /code_chunk_output -->

细分目录：

<!-- @import "[TOC]" {cmd="toc" depthFrom=2 depthTo=6 orderedList=false} -->

<!-- code_chunk_output -->

- [Chapter 7 Scheduling](#chapter-7-scheduling)
  - [7.5 Sleep and wakeup 信号量 semaphore](#75-sleep-and-wakeup-信号量-semaphore)
    - [PV 操作](#pv-操作)
    - [一个生产者一个消费者模型示例](#一个生产者一个消费者模型示例)
    - [企图解决 lost wakeup 的讨论](#企图解决-lost-wakeup-的讨论)
  - [7.9 Real world 注意到 Xv6 不支持信号量](#79-real-world-注意到-xv6-不支持信号量)

<!-- /code_chunk_output -->

## Chapter 7 Scheduling

### 7.5 Sleep and wakeup 信号量 semaphore

Sleep and wakeup are often called sequence coordination or conditional synchronization mechanisms.

接着书里举了信号量的例子。

#### PV 操作

- The “V” operation (for the producer) increments the count.
- The “P” operation (for the consumer) waits until the count is non-zero, and then decrements it and returns.

#### 一个生产者一个消费者模型示例

现在是一个生产者，一个消费者，不同的 CPU 上运行，并且编译器没有过度优化。

```c
struct semaphore {
  struct spinlock lock;
  int count;
};

void
V(struct semaphore *s)
{
  acquire(&s->lock);
  s->count += 1;
  release(&s->lock);
}

void
P(struct semaphore *s)
{
  while (s->count == 0)
    ;
  acquire(&s->lock);
  s->count -= 1;
  release(&s->lock);
}
```

上述实现是 expensive 的，因为消费者会自旋。

因此我们加入 sleep 和 wakeup 改善：

```c
void
V(struct semaphore *s)
{
  acquire(&s->lock);
  s->count += 1;
  wakeup(s);
  release(&s->lock);
}

void
P(struct semaphore *s)
{
  while (s->count == 0)
    sleep(s);
  acquire(&s->lock);
  s->count -= 1;
  release(&s->lock);
}
```

上述代码可能导致 lost wakeup ，比如在 P 运行到 sleep(s) 前（现在 count 是 0），而 V 此刻刚好吧 s 的 count 增加为 1 ，接着再 wakeup ，而此刻 P 还没有执行 sleep ，因此这个 wakeup 就丢失了。

#### 企图解决 lost wakeup 的讨论

我们可以通过移动 P 中获取锁的位置来保证 sleep 与 count 的原子性。

```c
void
V(struct semaphore *s)
{
  acquire(&s->lock);
  s->count += 1;
  wakeup(s);
  release(&s->lock);
}

void
P(struct semaphore *s)
{
  acquire(&s->lock);
  while (s->count == 0)
    sleep(s);
  s->count -= 1;
  release(&s->lock);
}
```

上述或许会解决 lost wakeup ，但是有死锁问题： P holds the lock while it sleeps, so V will block forever waiting for the lock.

因此修改如下就没有问题了（对 sleep 修改，修改为课上讲的标准 sleep）：

```c
void
V(struct semaphore *s)
{
  acquire(&s->lock);
  s->count += 1;
  wakeup(s);
  release(&s->lock);
}

void
P(struct semaphore *s)
{
  acquire(&s->lock);
  while (s->count == 0)
    sleep(s, &s->lock);
  s->count -= 1;
  release(&s->lock);
}
```

### 7.9 Real world 注意到 Xv6 不支持信号量

The xv6 scheduler implements a simple scheduling policy, which runs each process in turn. This policy is called round robin. Real operating systems implement more sophisticated policies that, for example, allow processes to have priorities. 现实世界中，进程是有优先级的。

Sleep and wakeup are a simple and effective synchronization method, but there are many others. The first challenge in all of them is to avoid the “lost wakeups” problem we saw at the beginning of the chapter. The original Unix kernel’s sleep simply disabled interrupts, which sufficed because Unix ran on a single-CPU system. Because xv6 runs on multiprocessors, it adds an explicit lock to sleep. FreeBSD’s msleep takes the same approach. Plan 9’s sleep uses a callback function that runs with the scheduling lock held just before going to sleep; the function serves as a last-minute check of the sleep condition, to avoid lost wakeups. The Linux kernel’s sleep uses an explicit process queue, called a wait queue, instead of a wait channel; the queue has its own internal lock.

Scanning the entire process list in wakeup for processes with a matching chan is inefficient. A better solution is to replace the chan in both sleep and wakeup with a data structure that holds a list of processes sleeping on that structure, such as Linux’s wait queue. Plan 9’s sleep and wakeup call that structure a rendezvous point or Rendez. Many thread libraries refer to the same structure as a condition variable; in that context, the operations sleep and wakeup are called wait and signal. All of these mechanisms share the same flavor: the sleep condition is protected by some kind of lock dropped atomically during sleep.

Semaphores are often used for synchronization. The count typically corresponds to something like the number of bytes available in a pipe buffer or the number of zombie children that a process has. Using an explicit count as part of the abstraction avoids the “lost wakeup” problem: there is an explicit count of the number of wakeups that have occurred. The count also avoids the spurious wakeup and thundering herd problems.

Terminating processes and cleaning them up introduces much complexity in xv6. In most operating systems it is even more complex, because, for example, the victim process may be deep inside the kernel sleeping, and unwinding its stack requires much careful programming. Many operating systems unwind the stack using explicit mechanisms for exception handling, such as longjmp.  Furthermore, there are other events that can cause a sleeping process to be woken up, even though the event it is waiting for has not happened yet. For example, when a Unix process is sleeping, another process may send a signal to it. In this case, the process will return from the interrupted system call with the value -1 and with the error code set to EINTR. The application can check for these values and decide what to do. **Xv6 doesn’t support signals and this complexity doesn’t arise.**

A real operating system would find free proc structures with an explicit free list in constant time instead of the linear-time search in allocproc; xv6 uses the linear scan for simplicity. 果然，直接扫描所有 proc 确实是玩具版的实现。
