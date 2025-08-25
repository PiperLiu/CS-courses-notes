# Standford CS144: Computer Network

<!-- @import "[TOC]" {cmd="toc" depthFrom=3 depthTo=6 orderedList=false} -->

<!-- code_chunk_output -->

- [课程资料](#课程资料)
- [课程索引](#课程索引)
  - [Introduction to Computer Networking CS](#introduction-to-computer-networking-cs)

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
