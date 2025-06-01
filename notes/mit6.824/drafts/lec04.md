# LEC 4: Primary-Backup Replication

课堂实录可参考 [肖宏辉大佬的MIT6.824分布式系统课程中文翻译](https://www.zhihu.com/column/c_1273718607160393728) 的笔记。

<!-- @import "[TOC]" {cmd="toc" depthFrom=2 depthTo=6 orderedList=false} -->

<!-- code_chunk_output -->

- [NotebookLM 生成的双人播客](#notebooklm-生成的双人播客)
- [Gemini 2.5 Pro 生成 VMware FT 容错技术详解：原理、挑战与实践](#gemini-25-pro-生成-vmware-ft-容错技术详解原理-挑战与实践)
  - [容错的基本概念与方法](#容错的基本概念与方法)
  - [VMware FT 的核心：基于 Hypervisor 的复制状态机](#vmware-ft-的核心基于-hypervisor-的复制状态机)
  - [关键挑战与解决方案](#关键挑战与解决方案)
    - [1. 输出规则 (Output Rule) 与数据一致性](#1-输出规则-output-rule-与数据一致性)
    - [2. DMA 竞争 (Race) 与 Bounce Buffers](#2-dma-竞争-race-与-bounce-buffers)
    - [3. 故障检测与切换 (Failover)](#3-故障检测与切换-failover)
    - [4. 脑裂 (Split-Brain) 问题及其解决](#4-脑裂-split-brain-问题及其解决)
    - [5. 启动与重启 FT 虚拟机](#5-启动与重启-ft-虚拟机)
    - [6. 日志通道管理](#6-日志通道管理)
    - [7. 磁盘 I/O 的特殊处理](#7-磁盘-io-的特殊处理)
    - [8. 网络 I/O 的挑战与优化](#8-网络-io-的挑战与优化)
  - [性能与局限性](#性能与局限性)
  - [总结](#总结)

<!-- /code_chunk_output -->

## NotebookLM 生成的双人播客

https://notebooklm.google.com/notebook/94e7862f-b0b3-4dcf-97c7-abda275eb82b/audio

微信链接： https://mp.weixin.qq.com/s/jMUhQIrLH64TKdmdzq2DYg

给我留下的较深的印象：
1. VMware FT 的主备复制实现方式很底层，但为了解决不一致性问题（比如为了模拟主服务器的中断、网络情况）引入的 log channel 太细致， 反而造成了极其高昂的成本。
2. 传输状态变化，而非完整的数据到备服务器。

## Gemini 2.5 Pro 生成 VMware FT 容错技术详解：原理、挑战与实践

在现代计算环境中，业务连续性至关重要。服务器故障可能导致服务中断、数据丢失，进而造成巨大的经济损失和声誉损害。容错 (Fault Tolerance, FT) 技术旨在确保单个硬件故障不会影响应用程序的可用性。VMware FT 就是这样一种为虚拟机 (VM) 提供高级别可用性的解决方案，它通过在不同物理服务器上维护一个与主虚拟机 (Primary VM) 同步的备份虚拟机 (Backup VM) 来实现。

### 容错的基本概念与方法

首先，我们需要明确 VMware FT 主要针对的是 **失效-停止 (fail-stop)** 型故障。这类故障指的是组件发生故障后会完全停止工作，例如电源故障、网络中断或整个服务器宕机。这类故障可以通过复制 (replication) 来处理。相比之下，一些更复杂的故障，如软件缺陷 (software bug)、设计缺陷 (design flaw) 或恶意的拜占庭 (Byzantine) 故障（即组件可能产生错误结果但表面上仍在运行），则不在此类 fail-stop 容错系统的主要处理范围内。

实现容错的核心在于复制。通常有两种主要的复制策略：

1.  **状态传输 (State Transfer)** ：定期或持续地将主副本的完整状态（包括 CPU、内存、I/O 设备状态）传输到备份副本。这种方法的缺点是所需带宽可能非常大，尤其是内存变化频繁时。
2.  **复制状态机 (Replicated State Machine, RSM)** ：将服务器视为确定性状态机。如果两个状态机从相同的初始状态开始，并以相同的顺序接收相同的输入，那么它们将经历相同的状态序列并产生相同的输出。 VMware FT 正是采用了这种方法，通过在底层复制虚拟机内存和寄存器的执行来实现。这种方法的带宽需求通常远低于状态传输。

### VMware FT 的核心：基于 Hypervisor 的复制状态机

**Q: 为什么在虚拟机上实现确定性执行比在物理服务器上更容易？**

A: 因为 Hypervisor 模拟并控制了硬件的许多方面，例如中断传递的精确时机。这使得 Hypervisor 能够确保主虚拟机和备份虚拟机在面对相同的输入时，其内部状态以完全相同的方式演进。

**Q: 什么是 Hypervisor？**

A: Hypervisor，也称为虚拟机监视器 (Virtual Machine Monitor, VMM)，是虚拟机系统的一部分。它模拟一台完整的计算机，客户操作系统 (Guest OS) 及其应用程序运行在这个模拟的计算机（即虚拟机）内。在 VMware FT 架构中，Primary VM 和 Backup VM 都是运行在各自 Hypervisor 内的客户机。

VMware FT 的基石是 **确定性重放 (Deterministic Replay)** 技术。 Primary VM 的所有输入（如网络数据包、磁盘读取、键盘鼠标操作以及所有可能导致非确定性行为的事件（如中断的时间、读取 CPU 时钟周期计数器）都会被 Hypervisor 捕获。这些输入和非确定性事件被记录成日志条目，通过一个专用的 **日志通道 (Logging Channel)** 实时发送给 Backup VM。 Backup VM 的 Hypervisor 则严格按照这些日志条目来“重放”Primary VM 的执行过程，从而保持两者状态的精确同步，尽管 Backup VM 的执行会略有延迟。

**Q: VMware FT 如何处理那些看似随机的非确定性事件，以确保主备一致？**

A: Hypervisor 控制了所有随机性的来源。
* **中断传递** ：Hypervisor 会记录 Primary VM 中断发生的精确指令位置，并在 Backup VM 执行到相同指令时传递该中断。这得益于 CPU 的性能计数器 (performance counters) 等硬件辅助特性。 VMware 的实现足够高效，无需像早期某些系统那样将中断延迟到某个“纪元 (epoch)”结束时才处理。
* **时间或硬件周期计数器** ：当应用程序试图读取当前时间或硬件周期计数器时，Hypervisor 会拦截这些指令，并确保 Primary VM 和 Backup VM 获取到相同的值。
* **随机数生成器** ：即使应用程序调用随机数生成器，Hypervisor 也会确保主备获得相同的随机数序列。

### 关键挑战与解决方案

#### 1. 输出规则 (Output Rule) 与数据一致性

**Q: VMware FT 如何保证在 Primary VM 故障后，Backup VM 接管时不会丢失数据或产生外部可见的不一致状态？**

A: 这是通过严格遵守 **输出规则 (Output Rule)** 来实现的。

**输出规则** ：Primary VM 在向外部世界发送任何输出（例如网络数据包、磁盘写入到共享磁盘）之前，必须等待 Backup VM 接收并确认了与该输出操作相关的日志条目。

这条规则至关重要。如果 Backup VM 已经确认收到了包含某个输出操作的日志，那么即使 Primary VM 在该输出实际发送后立即崩溃，Backup VM 也能准确地重放到该输出点之前的状态，并最终产生与 Primary VM 相同的输出（或达到一个与该输出一致的状态）。如果没有这个规则，Backup VM 可能在没有收到所有必要日志的情况下就上线，导致其状态与 Primary VM 已经发出的输出不一致。

值得注意的是，输出规则仅仅延迟了输出的实际发送，Primary VM 本身可以继续执行后续指令，而不会被完全暂停。这与一些早期系统要求 Primary VM 在输出前必须完全停止直到 Backup VM 确认的做法不同。

**Q: 如果 Primary VM 在发送输出后立即失败，输出会重复吗？这会导致问题吗？**

A: Backup VM 在接管后很可能会重复该输出。对于大多数情况，这不是问题：
* **网络输出** ：如果输出是网络数据包，接收端的 TCP 协议栈通常能自动处理和丢弃重复的数据包。
* **磁盘 I/O** ：磁盘写入操作通常是幂等的 (idempotent)，即多次执行相同的写操作（写入相同数据到相同位置且中间无其他 I/O）与执行一次的效果相同。

#### 2. DMA 竞争 (Race) 与 Bounce Buffers

**Q: 什么是 DMA (Direct Memory Access) 竞争？VMware FT 如何避免由此导致的非确定性？**

A: 当硬件设备（如网卡或磁盘控制器）通过 DMA 直接向虚拟机内存写入数据时，如果客户机操作系统或应用程序恰好在几乎同一时刻读取该内存区域，就可能发生竞争。由于主备虚拟机执行时序的微小差异，一个可能读到新数据，另一个可能读到旧数据，从而导致状态发散。

VMware FT 通过使用 **bounce buffers** 来避免这个问题：
1.  当网络数据包或磁盘数据块到达 Primary VM 时，数据首先被 DMA 到 Hypervisor 控制的一个私有“bounce buffer”中，这个 buffer 是 Primary VM 无法直接访问的。
2.  DMA 完成后，FT Hypervisor 中断 Primary VM 的执行，并记录下中断发生的精确指令点。
3.  然后，FT Hypervisor 将 bounce buffer 中的数据复制到 Primary VM 的实际目标内存中。
4.  之后，Primary VM 才被允许恢复执行。
5.  同时，这份数据会通过日志通道发送给 Backup VM。 Backup VM 的 FT Hypervisor 会在相同的指令点中断 Backup VM，将数据从其自己的 bounce buffer（或直接接收到的数据）复制到 Backup VM 的内存中，然后恢复其执行。

通过这种方式，确保了主备虚拟机在完全相同的执行时刻“看到”输入数据，从而消除了 DMA 竞争。尽管 bounce buffer 会增加一点开销，但通常不会造成明显的性能损失。

#### 3. 故障检测与切换 (Failover)

VMware FT 系统使用多种机制来检测 Primary VM 或 Backup VM 的故障：
* 在运行 FT 虚拟机的服务器之间进行 UDP 心跳检测 (heartbeating)。
* 监控日志通道上的流量。由于客户机操作系统通常有规律的定时器中断，日志流量应该是持续的。如果日志条目或确认信息流中断超过预设的超时时间（通常几秒钟），则可能表明发生了故障。

当检测到故障时：
* 如果 Backup VM 失败，Primary VM 会“上线 (go live)”，即停止记录日志并转为普通模式运行。
* 如果 Primary VM 失败，Backup VM 的上线过程稍微复杂：它必须首先消耗掉其日志缓冲区中所有已接收但尚未处理的日志条目，以使其状态达到 Primary VM 失败前的最新点。完成后，Backup VM 停止重放模式，转为新的 Primary VM，并开始向外部世界产生输出。此时，FT 系统会自动更新网络，例如通告新 Primary VM 的 MAC 地址，以便网络交换机能将流量导向正确的服务器。

#### 4. 脑裂 (Split-Brain) 问题及其解决

**Q: 什么是脑裂 (split-brain) 问题？VMware FT 如何防止这种情况？**

A: 当 Primary VM 和 Backup VM 之间的网络连接中断（网络分区），但它们各自的物理服务器仍在运行时，可能会发生脑裂。双方都可能错误地认为对方已经崩溃，并尝试独立“上线”成为唯一的活动虚拟机。如果两者都成功上线，将导致数据损坏和客户端混淆。

VMware FT 使用 **共享存储 (Shared Storage)** 上的一个原子性 **test-and-set** 操作作为决胜机制 (tie-breaker) 来解决脑裂问题。
1.  共享存储上维护一个初始为 false 的标志。
2.  当 Primary VM 或 Backup VM 认为对方失效并决定自己上线时，它会首先向共享存储发送一个 test-and-set 请求。
3.  存储服务会原子地执行类似以下逻辑的操作：

```
test-and-set() {
    acquire_lock()
    if flag == true:
    release_lock()
    return false // 已经被设置过了，你不能上线
    else:
    flag = true
    release_lock()
    return true  // 设置成功，你可以上线
}
```

4.  只有当 test-and-set 操作返回 true 时，该虚拟机才被允许上线。如果返回 false，意味着另一方已经成功上线（或正在上线），当前虚拟机则会停止自己（“自杀”）以避免冲突。如果虚拟机无法访问共享存储，它会等待直到可以访问为止。

这种机制确保了即使在网络分区的情况下，也只有一个虚拟机能够成功上线。

#### 5. 启动与重启 FT 虚拟机

为了能够启动一个新的 Backup VM 与一个正在运行的 Primary VM 同步，或者在故障后重新建立冗余，VMware FT 使用了一种修改版的 **VMotion** 技术，称为 FT VMotion。
* FT VMotion 可以在不显著中断 Primary VM 执行的情况下（通常暂停时间小于1秒），在另一台服务器上克隆出一个与 Primary VM 状态完全相同的 Backup VM，并自动建立起它们之间的日志通道。
* 当发生故障后，比如 Primary VM 失效，原来的 Backup VM 上线成为新的 Primary VM。此时，为了恢复冗余，这个新的 Primary VM 会通知集群管理服务 (clustering service)。集群服务会根据资源使用情况在集群中选择一台合适的服务器，并调用 FT VMotion 来创建并启动一个新的 Backup VM。这个过程通常能在几分钟内自动完成。

#### 6. 日志通道管理

* 主备 Hypervisor 都会为日志条目维护大型缓冲区。 Primary VM 产生日志到其缓冲区，Backup VM 从其缓冲区消耗日志。
* Primary 的日志缓冲区内容会尽快发送到日志通道，Backup 会尽快从通道读取到自己的缓冲区。 Backup 在读取日志后会向 Primary 发送确认 (acknowledgments)。这些确认用于 Output Rule 的判断。
* 如果 Backup 的日志缓冲区为空，它会暂停执行等待新日志。
* 如果 Primary 的日志缓冲区已满（通常因为 Backup 处理过慢或网络拥塞），Primary VM 必须暂停执行，这是一种自然的流控机制。为避免这种情况影响 Primary VM 性能，系统会监控主备之间的执行延迟。如果 Backup VM 落后太多（例如超过1秒），FT 系统会逐渐降低 Primary VM 的 CPU 分配，使其放慢速度，以便 Backup VM 能跟上。

#### 7. 磁盘 I/O 的特殊处理

* **共享磁盘 (Shared Disk)** ：在默认配置中，Primary VM 和 Backup VM 共享虚拟磁盘（如通过 Fibre Channel 或 iSCSI SAN 访问）。此时，只有 Primary VM 实际执行对共享磁盘的写操作，这些写操作也受 Output Rule 的约束。 Backup VM 不执行写操作，其输出会被 Hypervisor丢弃。
* **非共享磁盘 (Non-shared Disk)** ：也支持主备各自拥有独立的（非共享的）虚拟磁盘。在这种情况下，Backup VM 也会执行所有磁盘写操作到它自己的虚拟磁盘上，从而保持与 Primary VM 磁盘内容的同步。这种配置下，Primary VM 的磁盘写操作不再被视为外部输出，因此不需要遵循 Output Rule 进行延迟。这对于共享存储不可用或主备距离较远 (“long-distance FT”) 的场景很有用。
* **Primary 故障时未完成的磁盘 I/O**

**Q: 当 Primary VM 发生故障时，那些已经发起但可能尚未在物理磁盘上完成的 I/O 请求如何处理？**

A: 当 Backup VM 上线成为新的 Primary VM 时，在其“go-live”过程中，它会重新发出这些挂起的 (pending) I/O 请求。具体来说，日志中那些有 I/O 启动记录但没有对应完成中断记录的 I/O 操作，都需要在 Backup VM 上线时重新启动。由于 VMware FT 已经通过 bounce buffer 等机制消除了 I/O 竞争，并且 I/O 操作明确指定了内存和磁盘块，因此这些重新发出的磁盘操作即使之前已经成功完成过，再次执行也是安全的（幂等性）。

#### 8. 网络 I/O 的挑战与优化

为了确保确定性，VMware FT 对网络 I/O 的处理做了一些调整：
* 禁用了某些基于 Hypervisor 异步更新虚拟机网络设备状态的性能优化。例如，接收缓冲区不能再由 Hypervisor 在 VM 执行时直接异步更新，而是需要陷入 (trap) 到 Hypervisor，记录更新操作，然后再应用到 VM。类似的，异步发送数据包的优化也被禁用，发送操作也需要通过 trap 完成。
* 这些改变以及 Output Rule 带来的发送延迟对网络性能构成挑战。

* **优化措施**
    * **聚集优化 (Clustering Optimizations)** ：当 VM 以足够高的比特率传输数据时，Hypervisor 可以将多个数据包的发送合并为一次 trap，甚至在某些情况下（如作为接收新包的一部分发送）可以做到零 trap。类似地，对接收到的数据包也可以合并中断通知。
    * **减少发送延迟** ：关键在于减少日志消息发送到 Backup VM 并获得确认所需的时间。通过确保日志条目和确认信息的发送与接收都可以在没有线程上下文切换的情况下完成（例如使用类似 Linux tasklet 的延迟执行上下文处理 TCP 数据接收），以及在 Primary VM 发送数据包时强制立即刷新相关日志条目，可以显著降低延迟。

### 性能与局限性

VMware FT 在典型企业应用上的性能开销通常小于10%。日志通道所需的带宽对于多数应用也相当合理，一般低于 20 Mbit/s，远低于 1 Gbit/s 网络的承载能力。空闲的客户机操作系统（如 Linux 和 Windows）产生的日志带宽大约在 0.5-1.5 Mbits/sec，主要来自记录定时器中断的传递。

对于网络密集型应用，特别是高接收速率的场景，日志通道带宽可能成为瓶颈，因为所有接收到的网络包都需要通过日志通道发送给 Backup VM。

目前，VMware FT 的生产版本主要支持单处理器 (uni-processor) 虚拟机。这是因为多处理器 (multi-processor) VM 的确定性重放仍然是一个具有挑战性的研究领域，主要难点在于几乎所有对共享内存的访问都可能构成非确定性操作。不过，单处理器虚拟机对于许多工作负载已经足够，并且可以通过横向扩展（使用多个单处理器VM）来满足需求。

### 总结

VMware FT 通过在 Hypervisor 层面实现复制状态机和确定性重放，为虚拟机提供了一种高效、透明的故障切换解决方案。它巧妙地解决了非确定性事件处理、输出一致性、脑裂等关键问题，并通过一系列实用性设计（如 FT VMotion、自动恢复冗余）使其成为一个完整且易用的企业级容错系统。尽管当前主要针对单处理器虚拟机和 fail-stop 故障，但其设计理念和实现为关键业务应用提供了坚实的保障。
