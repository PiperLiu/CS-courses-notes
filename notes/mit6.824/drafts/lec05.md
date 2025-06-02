# LEC 5: Go, Threads, and Raft, video, code samples

<!-- @import "[TOC]" {cmd="toc" depthFrom=2 depthTo=6 orderedList=false} -->

<!-- code_chunk_output -->

- [基于 Grok-3-Thinking/Gemini-2.5-Pro 的提炼](#基于-grok-3-thinkinggemini-25-pro-的提炼)
  - [Happens Before (先行发生)](#happens-before-先行发生)
  - [通道通信的同步细节 (Channel Communication Synchronization Details)](#通道通信的同步细节-channel-communication-synchronization-details)
  - [数据竞争的处理方式](#数据竞争的处理方式)
  - [不正确同步的示例与陷阱](#不正确同步的示例与陷阱)
  - [编译器优化的限制](#编译器优化的限制)
  - [通道通信的同步细节](#通道通信的同步细节)
  - [原子操作的同步能力](#原子操作的同步能力)
  - [Finalizers的同步行为](#finalizers的同步行为)

<!-- /code_chunk_output -->

视频课程助教讲的，关于 Go 的一些答疑，略过。

这节阅读资料（ go memory model ）中提到的 Happens Before 和 **从通道进行的第 k 次接收操作的完成 happens-before 第 k+C 次发送操作的开始** 行为在 go 1.3 release 中有提及，可以参考我之前的后端文章：
- https://github.com/PiperLiu/back-end-notes/blob/master/notes/golang/versions/04-go1.3.md

## 基于 Grok-3-Thinking/Gemini-2.5-Pro 的提炼

https://go.dev/ref/mem

课程是 2020 年的，现在读到的文章已经是 Version of June 6, 2022 了。

我对 LLM OUTPUT 做了些修正。

### Happens Before (先行发生)

“Happens Before”是 Go 内存模型中用以描述不同 goroutine 中操作之间可见性顺序的核心概念。简单来说，如果事件 A “happens before” 事件 B，那么事件 A 的结果保证对事件 B 可见。这种关系是传递的：如果 A happens before B，且 B happens before C，那么 A happens before C。

“Happens Before”关系主要由以下两种情况构成：

1.  **Sequenced Before (顺序先于)**

* **定义** ：在单个 goroutine 内部，操作的执行顺序遵循代码中语句的顺序。例如，在 `x = 1; y = x + 1` 中，对 `x` 的赋值操作顺序先于对 `y` 的赋值操作。
* **作用** ：这是最直观的排序，保证了单个 goroutine 内代码的逻辑按预期执行。

2.  **Synchronized Before (同步先于)**

* **定义** ：当不同的 goroutine 通过同步原语（如通道操作、互斥锁等）进行交互时，这些原语会建立起 goroutine 间的同步关系。例如，对一个通道的发送操作，如果成功匹配到接收操作，那么发送完成“同步先于”接收完成。
* **作用** ：这是跨 goroutine 保证内存可见性的关键。同步操作不仅协调执行，还确保一个 goroutine 中的写操作对另一个 goroutine 中的后续读操作可见。

**Happens Before = Sequenced Before ∪ Synchronized Before (取传递闭包)**

最终的“Happens Before”关系是“顺序先于”和“同步先于”关系的并集的传递闭包。这意味着，一系列操作，即使跨越多个 goroutine，只要它们能通过“顺序先于”和“同步先于”链接起来，就能形成一个“Happens Before”链条。

**为什么重要？**

对于一个普通的（非同步的）内存读取操作 `r`，如果要保证它能观察到某个写入操作 `w` 的值，必须满足：
1.  `w` happens before `r`.
2.  没有其他的写入操作 `w'` (针对同一内存位置) 满足 `w` happens before `w'` 且 `w'` happens before `r`。

如果一个程序没有数据竞争（Data Race-Free, DRF），即所有对共享变量的并发访问都通过同步原语进行了保护，那么该程序的执行就如同所有 goroutine 的操作被以某种顺序交错在一个单一处理器上执行一样，这被称为“顺序一致性”（Sequentially Consistent, DRF-SC）。

### 通道通信的同步细节 (Channel Communication Synchronization Details)

通道是 Go 中主要的 goroutine 间同步机制。每一次在特定通道上的发送操作都会与该通道上相应的接收操作相匹配。

以下是文档中提到的关于通道通信建立的“Synchronized Before”关系的具体规则：

1.  **“A send on a channel is synchronized before the completion of the corresponding receive from that channel.”**

* **中文** ：对一个通道的发送操作，同步先于 (happens before) 从该通道完成的相应接收操作。
* **解释** ：这条规则是最基础的。无论通道是带缓冲的还是不带缓冲的，一旦一个值被成功发送，并且这个值被接收方成功接收，那么发送这个动作（及其之前的所有在同一 goroutine 中的操作）对于接收完成这个动作（及其之后的所有在同一 goroutine 中的操作）是可见的。

* **例子**

```go
var c = make(chan int, 10) // 可以是带缓冲或不带缓冲
var a string

func f() {
    a = "hello, world" // (1) 对 a 的写操作
    c <- 0             // (2) 对 c 的发送操作
}

func main() {
    go f()
    <-c                // (3) 从 c 的接收操作
    print(a)           // (4) 对 a 的读操作
}
```

在这个例子中：
* `(1)` 顺序先于 `(2)` (在 goroutine `f` 内部)。
* 根据本条规则，`(2)` (发送) 同步先于 `(3)` (接收完成)。
* `(3)` 顺序先于 `(4)` (在 goroutine `main` 内部)。
* 因此，通过传递性，对 `a` 的写操作 `(1)` happens before 对 `a` 的读操作 `(4)`。所以程序保证打印出 "hello, world"。

2.  **“The closing of a channel is synchronized before a receive that returns a zero value because the channel is closed.”**

* **中文** ：关闭一个通道的操作，同步先于 (happens before) 因通道已关闭而返回零值的接收操作。
* **解释** ：当一个通道被关闭后，任何后续的接收操作会立即返回一个该通道类型的零值，并且第二个返回值 (通常是 `ok`) 会是 `false`。这个规则保证，关闭通道的动作（及其之前的所有操作）对于那个感知到通道已关闭的接收操作是可见的。
* **例子** ：如果将上一个例子中的 `c <- 0` 替换为 `close(c)`，行为保证是相同的。`main` 中的 `<-c` 会因为通道关闭而返回 (对于 `int` 类型是 `0`)，`print(a)` 仍然会打印 "hello, world"。

3.  **“A receive from an unbuffered channel is synchronized before the completion of the corresponding send on that channel.”**

* **中文** ：从一个无缓冲通道的接收操作，同步先于 (happens before) 在该通道上完成的相应发送操作。
* **解释** ：这条规则特定于 **无缓冲通道** 。无缓冲通道的发送和接收是同步的，发送方会阻塞直到接收方准备好接收，接收方也会阻塞直到发送方准备好发送。这条规则意味着，接收方goroutine中，接收操作完成前的所有操作，对于发送方goroutine中，发送操作完成后的所有操作，是可见的。这与第一条规则方向相反，共同确立了无缓冲通道的“会合点”(rendezvous)特性。
* **例子**

```go
var c = make(chan int) // 无缓冲通道
var a string

func f() {
    a = "hello, world" // (1) 对 a 的写操作
    <-c                // (2) 从 c 的接收操作
}

func main() {
    go f()
    c <- 0             // (3) 对 c 的发送操作
    print(a)           // (4) 对 a 的读操作
}
```

在这个例子中：
* `(1)` 顺序先于 `(2)` (在 goroutine `f` 内部)。
* 根据本条规则，`(2)` (接收) 同步先于 `(3)` (发送完成)。
* `(3)` 顺序先于 `(4)` (在 goroutine `main` 内部)。
* 因此，对 `a` 的写操作 `(1)` happens before 对 `a` 的读操作 `(4)`。程序同样保证打印 "hello, world"。
* **注意** ：如果 `c` 是一个带缓冲通道 (例如 `make(chan int, 1)`)，这个程序就不能保证打印 "hello, world"。因为发送操作 `(3)` 可能在 `f` 中的接收操作 `(2)` 执行之前就完成了（如果缓冲区未满），这样就不会建立 `(2)` 同步先于 `(3)` 的关系。

4.  **“The *k*-th receive on a channel with capacity *C* is synchronized before the completion of the *k+C*-th send from that channel completes.”**

* **中文** ：对于一个容量为 *C* 的通道，第 *k* 个接收操作，同步先于 (happens before) 该通道上的第 *k+C* 个发送操作的完成。
* **解释** ：这条规则是针对 **带缓冲通道** 的，它推广了无缓冲通道的规则，并允许使用带缓冲通道来建模计数信号量。可以这样理解：当通道满了（即已经有 *C* 个元素在缓冲中，等待被接收），下一个（即第 *C+1* 个）发送操作必须等待通道中至少有一个元素被接收出去后才能完成。更一般地，第 *k+C* 个发送者必须等待第 *k* 个接收者完成后才能继续。
* **作用** ：这允许我们限制并发数量。通道中的元素数量代表活跃的“许可”，通道的容量代表最大并发数。发送操作获取许可，接收操作释放许可。
* **例子**

```go
var limit = make(chan int, 3) // 容量 C = 3

func main() {
    // 假设 work 是一个包含多个待执行任务的切片
    for _, w := range work {
        go func(job func()) {
            limit <- 1   // (发送) 获取一个许可。如果 limit 已满 (3个goroutine已获取)，这里会阻塞
            job()        // 执行任务
            <-limit      // (接收) 释放一个许可
        }(w)
    }
    select{} // 保持 main goroutine 存活
}
```

在这个例子中，`limit` 通道确保最多只有3个 `job` 同时运行。
* 当第1, 2, 3个 goroutine 执行 `limit <- 1` (发送) 时，它们都能成功并将元素放入通道。
* 当第4个 goroutine 尝试执行 `limit <- 1` (第4次发送) 时，由于通道已满 (3个元素)，它会阻塞。
* 这符合规则：第1个接收 (`k=1`) 必须发生在第 `1+3=4` 个发送完成之前。也就是说，只有当最早获取许可的3个 goroutine 中的某一个完成了它的 `job()` 并执行了 `<-limit` (第1次接收)，释放了一个位置后，第4个 goroutine 的 `limit <- 1` (第4次发送) 才能成功。

总结一下，通道的这些同步规则是 Go 并发编程中正确性的基石。通过理解并正确使用这些规则，开发者可以构建出没有数据竞争且行为可预测的并发程序。这些规则定义了明确的“Happens Before”边界，从而保证了内存操作的可见性。

### 数据竞争的处理方式

- **精华** ：Go明确定义了数据竞争（data race），即多个goroutine同时访问同一内存位置且至少有一个是写操作，且没有同步保护。不同于C/C++的未定义行为，Go在检测到数据竞争时可能会报告并终止程序，或者允许读取并发写入的值，但结果是有限的。
- **为什么不容易注意到** ：您可能习惯于通过`go build -race`检测数据竞争，但对Go在数据竞争下的具体行为（例如不会像C/C++那样彻底不可预测）缺乏深入思考。
- **值得学习** ：理解Go对数据竞争的处理方式，可以帮助您更好地调试程序，避免依赖未定义行为的假设。此外，这也强化了Go鼓励使用同步机制的哲学，对比C/C++的差异能让您更欣赏Go的设计。

### 不正确同步的示例与陷阱

- **精华** ：文档列举了几个常见的错误同步模式，例如

```go
var a string
var done bool

func setup() {
    a = "hello, world"
    done = true
}

func doprint() {
    if !done {
        once.Do(setup)
    }
    print(a)
}

func twoprint() {
    go doprint()
    go doprint()
}
```

**双重检查锁定（Double-Checked Locking）** ：but there is no guarantee that, in `doprint`, **observing the write to `done` implies observing the write to `a`.** This version can (incorrectly) print an empty string instead of "hello, world".

```go
var a string
var done bool

func setup() {
    a = "hello, world"
    done = true
}

func main() {
    go setup()
    for !done {
    }
    print(a)
}
```

**忙等待（Busy Waiting）** ：As before, **there is no guarantee that, in `main`, observing the write to `done` implies observing the write to `a`, so this program could print an empty string too.** Worse, there is no guarantee that the write to done will ever be observed by main, **since there are no synchronization events between the two threads.** The loop in main is not guaranteed to finish.

- **为什么不容易注意到** ：这些错误模式在单线程逻辑中看似合理，但在并发环境下会失效。有经验的开发者可能偶尔依赖这些“巧妙”的方法，而未意识到它们的风险。
- **值得学习** ：通过研究这些反例，您可以避免在代码中犯类似错误。例如，解决这些问题的正确方法是显式同步（如使用锁或通道），这能显著提高代码的健壮性。

### 编译器优化的限制

- **精华**：Go内存模型对编译器优化施加了严格限制，以确保并发程序的正确性。例如
  - 编译器不能将写操作移出条件语句
  - 不能假设循环会终止或函数会返回，以避免引入意外的内存访问
  - 单次读操作不能观察到多个值，单次写操作不能写入多个值
- **为什么不容易注意到** ：这些限制主要影响底层编译器行为，作为应用开发者，您可能很少直接接触。但它们间接决定了您的代码在并发环境下的表现。
- **值得学习** ：理解这些限制能帮助您在编写高性能代码时，预判哪些优化是安全的。例如，知道编译器不会重新加载共享变量，可以避免在复杂逻辑中意外丢失同步保护。

### 通道通信的同步细节

- **精华**：文档详细描述了通道的同步行为：
  - 无缓冲通道的接收在发送完成之前“happens before”发送完成。
  - 缓冲通道的第 k 次接收在第 k+C 次发送完成之前“happens before”发送完成（C为通道容量）。
  - 关闭通道在接收到零值之前“happens before”。
- **为什么不容易注意到** ：您可能已经熟练使用通道，但对缓冲与无缓冲通道的同步差异、以及关闭通道的精确行为关注不够。
- **值得学习** ：掌握这些细节能帮助您在设计并发模式时（如信号量、限流）更精准地控制goroutine间的同步。例如，理解缓冲通道的同步规则，可以避免因容量误判导致的死锁或数据丢失。

### 原子操作的同步能力

- **精华** ：`sync/atomic`包中的原子操作（如`AddInt32`、`CompareAndSwap`）不仅提供无锁操作，还能建立goroutine间的同步关系。所有原子操作表现得像在一个顺序一致的顺序中执行。
- **为什么不容易注意到** ：原子操作常被视为性能优化工具，而其同步能力可能被忽视。
- **值得学习** ：在需要轻量级同步的场景下，正确使用原子操作可以替代锁，减少开销。例如，理解原子操作的“happens before”关系，可以在高并发计数器或状态标志中确保一致性。

### Finalizers的同步行为

- **精华** ：`runtime.SetFinalizer(x, f)`在finalizer `f(x)`调用之前“happens before”，这是垃圾回收前清理资源的高级机制。
- **为什么不容易注意到** ：finalizers是Go中的高级特性，日常开发中用得不多，且其同步细节容易被忽略。
- **值得学习** ：如果您需要在并发环境中管理资源（如文件句柄、网络连接），理解finalizers的同步行为能帮助您正确实现清理逻辑，避免资源泄露或竞争问题。
