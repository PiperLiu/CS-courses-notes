# Standford CS144: Computer Network

<!-- @import "[TOC]" {cmd="toc" depthFrom=3 depthTo=6 orderedList=false} -->

<!-- code_chunk_output -->

- [课程资料](#课程资料)
- [课程索引](#课程索引)
  - [Introduction to Computer Networking CS](#introduction-to-computer-networking-cs)
  - [Transport Layer Protocol, Reliable Communication and End-to-End Principles](#transport-layer-protocol-reliable-communication-and-end-to-end-principles)
  - [Packet Switching](#packet-switching)
  - [Congestion Control](#congestion-control)

<!-- /code_chunk_output -->

### 课程资料

官方课程资料每年一清空，跟着 2025 winter (1-3月) 的版本走吧。

- https://cs144.github.io/
- 如下是 2025 年版本快照： https://web.archive.org/web/20250703002557/https://cs144.github.io/
- https://www.youtube.com/watch?v=r2WZNaFyrbQ&list=PL6RdenZrxrw9inR-IJv-erlOKRHjymxMN

Lab 的话可以 fork 2025 年版本的这个：
- https://github.com/HT4w5/minnow-winter-2025

参考讲义：
- [Kaashoek & Saltzer, “Principles of Computer System Design,” Chapters 7–8 (Networks & Fault Tolerance)](https://ocw.mit.edu/courses/res-6-004-principles-of-computer-system-design-an-introduction-spring-2009/pages/online-textbook/)
- Kurose & Ross, “Computer Networking: A Top-Down Approach”
- [Peterson & Davie, “Computer Networks: A Systems Approach”](https://book.systemsapproach.org/)

### 课程索引

#### Introduction to Computer Networking CS

AI 播客
- https://notebooklm.google.com/notebook/e969d35c-3819-4a5e-b873-d8e5f57fa089?artifactId=031ee575-719a-49bc-a742-58609346ec8b
- https://mp.weixin.qq.com/s/hIgsFW_3cIc36Y2slJ5fLw

笔记： [./drafts/lec01.md](./drafts/lec01.md)
- [宏观设计：互联网的四大支柱](./drafts/lec01.md#宏观设计互联网的四大支柱)
  - [四层互联网模型 (The Four-Layer Internet Model)](./drafts/lec01.md#四层互联网模型-the-four-layer-internet-model)
  - [封装 (Encapsulation)：层层打包的艺术](./drafts/lec01.md#封装-encapsulation层层打包的艺术)
- [网络核心：IP 协议的“薄腰”哲学](./drafts/lec01.md#网络核心ip-协议的薄腰哲学)
  - [IP 服务模型：简单即是美](./drafts/lec01.md#ip-服务模型简单即是美)
  - [端到端原则 (End-to-End Principle)](./drafts/lec01.md#端到端原则-end-to-end-principle)
  - [IPv4 头部字段一览](./drafts/lec01.md#ipv4-头部字段一览)
- [地址与顺序：网络通信的规范](./drafts/lec01.md#地址与顺序网络通信的规范)
  - [字节序 (Byte Order)：大端与小端的约定](./drafts/lec01.md#字节序-byte-order大端与小端的约定)
  - [IPv4 地址与 CIDR](./drafts/lec01.md#ipv4-地址与-cidr)
  - [最长前缀匹配 (Longest Prefix Match)](./drafts/lec01.md#最长前缀匹配-longest-prefix-match)
  - [地址解析协议 (Address Resolution Protocol - ARP)](./drafts/lec01.md#地址解析协议-address-resolution-protocol---arp)
- [应用的千姿百态：不同的通信模型](./drafts/lec01.md#应用的千姿百态不同的通信模型)
  - [万维网 (World Wide Web)：经典的客户端-服务器模型](./drafts/lec01.md#万维网-world-wide-web经典的客户端-服务器模型)
  - [BitTorrent：去中心化的点对点模型](./drafts/lec01.md#bittorrent去中心化的点对点模型)
  - [Skype：应对复杂现实的混合模型](./drafts/lec01.md#skype应对复杂现实的混合模型)
- [互联网的脉动：演进、治理与挑战](./drafts/lec01.md#互联网的脉动演进-治理与挑战)
  - [谁在掌管互联网？](./drafts/lec01.md#谁在掌管互联网)
  - [演进案例：SIP 与 VoIP 的革命](./drafts/lec01.md#演进案例sip-与-voip-的革命)
  - [新兴趋势与未来挑战](./drafts/lec01.md#新兴趋势与未来挑战)
- [结语](./drafts/lec01.md#结语)

#### Transport Layer Protocol, Reliable Communication and End-to-End Principles

AI 播客
- https://notebooklm.google.com/notebook/443a3d00-cc85-45f4-b11a-0c4f5a473add?artifactId=e6926449-9310-46d8-975b-56b77c5c5832
- https://mp.weixin.qq.com/s/3119vtmKAnRzOWERvSozRg

笔记： [./drafts/lec02.md](./drafts/lec02.md)
- [传输层的两大支柱：UDP 与 TCP](./drafts/lec02.md#传输层的两大支柱udp-与-tcp)
  - [UDP：轻装上阵的“信使”](./drafts/lec02.md#udp轻装上阵的信使)
  - [TCP：稳重可靠的“工程师”](./drafts/lec02.md#tcp稳重可靠的工程师)
  - [ICMP：网络层的“侦察兵”](./drafts/lec02.md#icmp网络层的侦察兵)
- [可靠性的基石：TCP 连接管理](./drafts/lec02.md#可靠性的基石tcp-连接管理)
  - [三次握手：建立连接](./drafts/lec02.md#三次握手建立连接)
  - [ACK 的真正含义：确认、时钟、流控三合一](./drafts/lec02.md#ack-的真正含义确认-时钟-流控三合一)
  - [四次挥手：断开连接](./drafts/lec02.md#四次挥手断开连接)
- [TCP 的智慧：流量、窗口与拥塞控制](./drafts/lec02.md#tcp-的智慧流量-窗口与拥塞控制)
  - [TCP 序列号耗尽、PAWS（保护已包装序列）与时间戳选项](./drafts/lec02.md#tcp-序列号耗尽-paws保护已包装序列与时间戳选项)
  - [流量控制：照顾接收方的感受](./drafts/lec02.md#流量控制照顾接收方的感受)
  - [拥塞控制：照顾整个网络的感受](./drafts/lec02.md#拥塞控制照顾整个网络的感受)
  - [总结：流量控制 vs. 拥塞控制](./drafts/lec02.md#总结流量控制-vs-拥塞控制)
- [互联网的设计哲学：端到端原则](./drafts/lec02.md#互联网的设计哲学端到端原则)
  - [弱端到端原则 (Weak End-to-End Principle)](./drafts/lec02.md#弱端到端原则-weak-end-to-end-principle)
  - [强端到端原则 (Strong End-to-End Principle)](./drafts/lec02.md#强端到端原则-strong-end-to-end-principle)
- [数据的“指纹”：错误检测机制](./drafts/lec02.md#数据的指纹错误检测机制)
  - [IPv6 下的伪首部和校验](./drafts/lec02.md#ipv6-下的伪首部和校验)
- [TCP 的生命周期：深入理解 TCP 状态机](./drafts/lec02.md#tcp-的生命周期深入理解-tcp-状态机)
  - [宏观蓝图：TCP 状态转换图](./drafts/lec02.md#宏观蓝图tcp-状态转换图)
  - [第一幕：连接建立（三次握手）](./drafts/lec02.md#第一幕连接建立三次握手)
    - [被动方（服务器）的视角](./drafts/lec02.md#被动方服务器的视角)
    - [主动方（客户端）的视角](./drafts/lec02.md#主动方客户端的视角)
  - [第二幕：数据传输](./drafts/lec02.md#第二幕数据传输)
    - [示例 — 被动方收到 `FIN` -> `CLOSE_WAIT`（服务器端常见情况）](./drafts/lec02.md#示例--被动方收到-fin---close_wait服务器端常见情况)
    - [示例 — 主动方半关闭（client 发 `FIN` 后仍可读）：`FIN_WAIT_1` -> `FIN_WAIT_2` -> `TIME_WAIT`](./drafts/lec02.md#示例--主动方半关闭client-发-fin-后仍可读fin_wait_1---fin_wait_2---time_wait)
  - [第三幕：连接关闭（四次挥手）](./drafts/lec02.md#第三幕连接关闭四次挥手)
    - [主动关闭方（例如，客户端）的视角](./drafts/lec02.md#主动关闭方例如客户端的视角)
    - [`TIME_WAIT`：优雅退场的艺术](./drafts/lec02.md#time_wait优雅退场的艺术)
    - [四元组/五元组与 `TIME\_WAIT` 的端口复用](./drafts/lec02.md#四元组五元组与-time_wait-的端口复用)
    - [被动关闭方（例如，服务器）的视角](./drafts/lec02.md#被动关闭方例如服务器的视角)
    - [示例 — 同时关闭 -> `CLOSING`](./drafts/lec02.md#示例--同时关闭---closing)
    - [示例 — RST（abortive close）：如何让连接立刻变为 `CLOSED`（“暴力关闭”）并产生 RST](./drafts/lec02.md#示例--rstabortive-close如何让连接立刻变为-closed暴力关闭并产生-rst)
    - [示例 — `TIME_WAIT` 与端口重用：`SO_REUSEADDR` / `SO_REUSEPORT` 行为提示](./drafts/lec02.md#示例--time_wait-与端口重用so_reuseaddr--so_reuseport-行为提示)
  - [特殊情况](./drafts/lec02.md#特殊情况)
    - [`CLOSE_WAIT` 泄漏示例](./drafts/lec02.md#close_wait-泄漏示例)
    - [`TIME_WAIT` 普遍存在](./drafts/lec02.md#time_wait-普遍存在)
    - [如果 send 后收到 `SIGPIPE`](./drafts/lec02.md#如果-send-后收到-sigpipe)
  - [小结：把 C 代码的行为映射回状态机](./drafts/lec02.md#小结把-c-代码的行为映射回状态机)

#### Packet Switching

- [./handouts/week-3-packet-switching-a.pdf](./handouts/week-3-packet-switching-a.pdf)

AI 播客
- https://notebooklm.google.com/notebook/76c5b9c7-47a8-4cfd-bd8e-3750fff9c238?artifactId=8bc28f67-1f33-4471-8e54-d7f1ebbfd3ca
- https://mp.weixin.qq.com/s/cVcBA2ELQnYlRB9h2W7cSg

笔记： [./drafts/lec03.md](./drafts/lec03.md)
- [奠基石：电路交换 vs. 包交换](./drafts/lec03.md#奠基石电路交换-vs-包交换)
- [一个数据包的旅程：解构端到端延迟](./drafts/lec03.md#一个数据包的旅程解构端到端延迟)
  - [播延迟 (Propagation Delay, $t_l$)](./drafts/lec03.md#播延迟-propagation-delay-t_l)
  - [序列化延迟 (Serialization Delay, $t_p$)](./drafts/lec03.md#序列化延迟-serialization-delay-t_p)
  - [排队延迟 (Queueing Delay, $Q(t)$)](./drafts/lec03.md#排队延迟-queueing-delay-qt)
- [深入排队：从确定性模型到随机过程](./drafts/lec03.md#深入排队从确定性模型到随机过程)
  - [确定性排队模型](./drafts/lec03.md#确定性排队模型)
  - [统计复用增益与随机过程](./drafts/lec03.md#统计复用增益与随机过程)
- [服务质量 (QoS)：不只是“尽力而为”](./drafts/lec03.md#服务质量-qos不只是尽力而为)
  - [FIFO 队列的局限性](./drafts/lec03.md#fifo-队列的局限性)
  - [严格优先级 (Strict Priorities)](./drafts/lec03.md#严格优先级-strict-priorities)
  - [加权公平排队 (Weighted Fair Queueing, WFQ)](./drafts/lec03.md#加权公平排队-weighted-fair-queueing-wfq)
- [保证延迟：梦想与现实](./drafts/lec03.md#保证延迟梦想与现实)
  - [我们到底想保证什么？](./drafts/lec03.md#我们到底想保证什么)
  - [实现保证的两个关键“原料”](./drafts/lec03.md#实现保证的两个关键原料)
  - [驯服入口流量 —— 漏桶调节器 `(σ, ρ)`](./drafts/lec03.md#驯服入口流量--漏桶调节器-σ-ρ)
  - [提供专属通道 —— 加权公平排队 (WFQ)](./drafts/lec03.md#提供专属通道--加权公平排队-wfq)
  - [理论上的完美结合](./drafts/lec03.md#理论上的完美结合)
  - [残酷的现实：为什么在公共互联网上行不通？](./drafts/lec03.md#残酷的现实为什么在公共互联网上行不通)
  - [务实的替代方案：超额配置](./drafts/lec03.md#务实的替代方案超额配置)
- [网络的心脏：交换机与路由器如何工作](./drafts/lec03.md#网络的心脏交换机与路由器如何工作)
  - [以太网交换机 vs. IP 路由器](./drafts/lec03.md#以太网交换机-vs-ip-路由器)
  - [交换机内部架构](./drafts/lec03.md#交换机内部架构)
- [软件定义网络 SDN 和网络虚拟化 NV](./drafts/lec03.md#软件定义网络-sdn-和网络虚拟化-nv)
  - [软件定义网络 (Software-Defined Networking, SDN)](./drafts/lec03.md#软件定义网络-software-defined-networking-sdn)
  - [网络虚拟化 (Network Virtualization)](./drafts/lec03.md#网络虚拟化-network-virtualization)

#### Congestion Control

- [./handouts/week-4-why_congestion_control.pdf](./handouts/week-4-why_congestion_control.pdf)
- [./handouts/What congestion control.pdf](./handouts/What%20congestion%20control.pdf)
- [./handouts/How congestion control.pdf](./handouts/How%20congestion%20control.pdf)

AI 播客
- https://notebooklm.google.com/notebook/b5f3026d-d15d-41a5-97da-f03c661e6201?artifactId=8a54df56-cdf9-4a8c-9363-8eb47602c37a
- https://mp.weixin.qq.com/s/RIPSK9r7yxjsrMHCNiKwfg

笔记： [./drafts/lec04.md](./drafts/lec04.md)
- [什么是拥塞控制？](./drafts/lec04.md#什么是拥塞控制)
- [拥塞的危害：从性能下降到网络崩溃](./drafts/lec04.md#拥塞的危害从性能下降到网络崩溃)
- [理想模型：带宽时延积 (BDP)](./drafts/lec04.md#理想模型带宽时延积-bdp)
- [AIMD：TCP 拥塞控制的基石](./drafts/lec04.md#aimdtcp-拥塞控制的基石)
  - [加性增窗 (Additive Increase, AI)](./drafts/lec04.md#加性增窗-additive-increase-ai)
  - [乘性减窗 (Multiplicative Decrease, MD)](./drafts/lec04.md#乘性减窗-multiplicative-decrease-md)
  - [“锯齿”形的 `cwnd` 变化](./drafts/lec04.md#锯齿形的-cwnd-变化)
- [慢启动：快速启动，指数增长](./drafts/lec04.md#慢启动快速启动指数增长)
  - [MSS (Maximum Segment Size) 的确定](./drafts/lec04.md#mss-maximum-segment-size-的确定)
  - [ssthresh (Slow Start Threshold) 的确定](./drafts/lec04.md#ssthresh-slow-start-threshold-的确定)
- [TCP 拥塞控制实战：一个完整例子的剖析](./drafts/lec04.md#tcp-拥塞控制实战一个完整例子的剖析)
  - [第 1 阶段：慢启动 (Slow-Start)](./drafts/lec04.md#第-1-阶段慢启动-slow-start)
  - [第 2 阶段：拥塞避免 (Congestion Avoidance)](./drafts/lec04.md#第-2-阶段拥塞避免-congestion-avoidance)
  - [第 3 阶段：拥塞事件 - 三次重复确认 (Reno 模式)](./drafts/lec04.md#第-3-阶段拥塞事件---三次重复确认-reno-模式)
  - [第 4 阶段：新一轮的拥塞避免](./drafts/lec04.md#第-4-阶段新一轮的拥塞避免)
  - [第 5 阶段：严重拥塞事件 - 超时 (Timeout)](./drafts/lec04.md#第-5-阶段严重拥塞事件---超时-timeout)
  - [解惑：ACK 时钟：TCP 如何感知“一轮” RTT？](./drafts/lec04.md#解惑ack-时钟tcp-如何感知一轮-rtt)
    - [核心思想：事件驱动，而非时间驱动](./drafts/lec04.md#核心思想事件驱动而非时间驱动)
    - [如何实现“每轮 RTT”的规则？](./drafts/lec04.md#如何实现每轮-rtt的规则)
  - [解惑：详解快速重传与快速恢复](./drafts/lec04.md#解惑详解快速重传与快速恢复)
    - [快速重传 (Fast Retransmit)：拥塞的“哨声”](./drafts/lec04.md#快速重传-fast-retransmit拥塞的哨声)
    - [快速恢复 (Fast Recovery)：拥塞后的“手术刀”](./drafts/lec04.md#快速恢复-fast-recovery拥塞后的手术刀)
- [TCP 拥塞控制的演进史](./drafts/lec04.md#tcp-拥塞控制的演进史)
  - [TCP Tahoe (1988) - 开创者](./drafts/lec04.md#tcp-tahoe-1988---开创者)
  - [TCP Reno (1990) - 优化者](./drafts/lec04.md#tcp-reno-1990---优化者)
  - [TCP NewReno (1996) - 完善者](./drafts/lec04.md#tcp-newreno-1996---完善者)
- [总结与展望](./drafts/lec04.md#总结与展望)
