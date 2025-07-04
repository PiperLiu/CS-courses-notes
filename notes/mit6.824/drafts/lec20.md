# LEC 20: Blockstack

<!-- @import "[TOC]" {cmd="toc" depthFrom=2 depthTo=6 orderedList=false} -->

<!-- code_chunk_output -->

- [NotebookLM 生成的双人播客](#notebooklm-生成的双人播客)
- [Gemini 2.5 Pro 生成的“去中心化的互联网应用 Blockstack”](#gemini-25-pro-生成的去中心化的互联网应用-blockstack)
  - [引言：为什么我们需要一个不一样的互联网？](#引言为什么我们需要一个不一样的互联网)
  - [Blockstack 的核心理念：Zooko 三角与去中心化命名](#blockstack-的核心理念zooko-三角与去中心化命名)
    - [Zooko 三角难题](#zooko-三角难题)
    - [Blockstack 的解法：利用区块链](#blockstack-的解法利用区块链)
  - [Blockstack 架构深度解析](#blockstack-架构深度解析)
    - [第一层：控制平面 - 虚拟链 (Virtualchain)](#第一层控制平面---虚拟链-virtualchain)
    - [第二层：数据平面 - Atlas 网络 (Atlas Network)](#第二层数据平面---atlas-网络-atlas-network)
    - [第三层：数据平面 - Gaia 存储系统 (Gaia Storage System)](#第三层数据平面---gaia-存储系统-gaia-storage-system)
  - [探讨与反思：Blockstack 真的完美吗？](#探讨与反思blockstack-真的完美吗)
    - [对开发者的挑战](#对开发者的挑战)
    - [对用户的挑战：隐私、控制与成本](#对用户的挑战隐私-控制与成本)
    - [关于命名和 PKI 的再思考](#关于命名和-pki-的再思考)
  - [总结](#总结)

<!-- /code_chunk_output -->

## NotebookLM 生成的双人播客

https://notebooklm.google.com/notebook/2d807cbf-8501-4928-b536-6270746a7a53/audio

微信链接： https://mp.weixin.qq.com/s/bX3S-Nwn4IGPIo0KwUbK8Q

给我留下的较深的印象：
1. 在 Blockstack 中，对用户要求极高，若用户无法管理自己的私钥，则所有假设实效；适合专业人士小范围使用

## Gemini 2.5 Pro 生成的“去中心化的互联网应用 Blockstack”

### 引言：为什么我们需要一个不一样的互联网？

我们每天都在使用的互联网，从博客、电子邮件到社交网络，其架构通常是 **中心化的** 。你的数据，比如照片、帖子和个人信息，都存储在服务商（如 Google、Facebook）的服务器上，由他们的应用程序代码所控制。这种模式虽然方便了开发者，也取得了巨大的商业成功，但对用户来说却存在诸多弊端：

* **数据控制权丧失** ：你必须通过服务商提供的界面来访问你自己的数据，并且必须遵守他们随时可能改变的规则。
* **隐私风险** ：服务商可能会窥探你的数据，将其出售给广告商，甚至其内部员工也可能出于个人原因进行窥探。这令人失望，因为这本该是属于你自己的数据。
* **访问接口不友好** ：传统的 Web 接口主要面向 UI 展示，而不是为了方便用户控制和访问自己的底层数据。

为了解决这些问题，一个名为 **去中心化应用 (decentralized applications)** 的愿景应运而生。它的核心思想是将数据的 **所有权和控制权** 从中心化的网站手中交还给用户。这种架构将应用代码与用户数据分离开来。你可以把数据存储在自己付费和控制的云存储中（比如 Amazon S3），就像你控制自己笔记本电脑上的文件一样。这样一来，用户可以更轻松地切换应用，因为数据不再被锁定在某个特定的网站里，同时也通过端到端加密增强了隐私保护。

**Blockstack** 正是这一宏大愿景的探索者之一。它不是一个加密货币项目，而是尝试利用区块链技术来构建一个全新的、去中心化的 Web 架构。

### Blockstack 的核心理念：Zooko 三角与去中心化命名

Blockstack 认为，要实现真正的去中心化应用，首先必须解决一个基础且关键的问题： **命名 (naming)** 。我们需要一个可靠的系统，能将一个人类可读的名字（比如 `"robertmorris"`）映射到两个关键信息上：

1.  **数据位置** ：你的数据存储在哪里（例如在 Gaia 系统中的位置）。
2.  **公钥** ：用于验证数据真实性和加密通信的公钥。

一个健壮的、全球性的 **公钥基础设施 (Public Key Infrastructure, PKI)** 是许多安全系统设想的基石，但长期以来一直未能很好地实现。因此，Blockstack 从命名系统入手。

#### Zooko 三角难题

构建一个理想的命名系统异常困难，这被一个叫做 **Zooko 三角 (Zooko's Triangle)** 的理论所概括。该理论指出，一个命名系统很难同时满足以下三个理想属性：

1.  **独一无二 (Unique)** ：一个名字在全球范围内有且仅有一个含义，不会有歧义。
2.  **人类可读 (Human-readable)** ：名字是容易记忆和辨识的，比如 `“alice”`，而不是一长串随机字符。
3.  **去中心化 (Decentralized)** ：名字由用户自己创建和控制，而非由一个中心化机构分配。

在区块链出现之前，人们普遍认为任何命名系统最多只能实现其中两项。例如：
* **电子邮件地址** （如 `alice@gmail.com`）：独一无二且人类可读，但依赖于 `gmail.com` 这个中心化机构。
* **随机生成的公钥** ：独一无二且去中心化，但完全不是人类可读的。
* **通讯录里的昵称** ：人类可读且去中心化（你可以给朋友起任何昵称），但不是独一无二的（不同的人可以用同一个昵称指代不同的人）。

#### Blockstack 的解法：利用区块链

Blockstack 的巧妙之处在于，它利用 **区块链技术** 来同时实现这三个属性，从而“解开”了 Zooko 三角。

它的核心思路是：将 **名字的注册和声明** 记录在像比特币这样的公共区块链上。比特币的区块链提供了一个全球公认的、按时间排序的操作日志。如果我是第一个在比特币区块链上记录“我声明拥有 `'alice.id'` 这个名字”的人，那么根据规则，我就拥有了这个名字。

* **独一无二** ：因为区块链保证了全局一致的顺序，第一个成功的注册者是唯一公认的所有者。
* **人类可读** ：用户可以注册自己想要的、可读的字符串。
* **去中心化** ：整个过程不依赖任何中心化服务器，任何人都可以通过运行比特币节点来参与和验证。

### Blockstack 架构深度解析

为了实现上述目标，Blockstack 设计了一个分层架构，巧妙地将系统分为 **控制平面 (control plane)** 和 **数据平面 (data plane)** 。

```txt
+----------------------------------------------------------------------------+
|                                                                            |
|   +-----------+   +-----------+   +-----------------+    +---------------+ | Layer 3: Storage
|   | Amazon S3 |   |  Dropbox  |   | Microsoft Azure |    | Google Driver | | (Gaia)
|   +-----------+   +-----------+   +-----------------+    +---------------+ |
|        ^                                                                   |
|        | URI points to data                                                |
+--------|-------------------------------------------------------------------+
|        |                                                                   |
|   +----v----+       +-----------------+    +-----------+                   | Layer 2: Peer Network
|   | Local DB|------>| Zone file hash  |--->| Zone file |                   | (Atlas)
|   +---------+       +-----------------+    +-----------+                   |
|        ^                                                                   |
+--------|-------------------------------------------------------------------+
|        |                                                                   |
|   +----v----+       +------------------------------------------------+     | Layer 1: Blockchain
|   | Local DB|<------| Domain name | Public key | Zone file hash      |     | (Virtualchain on Bitcoin)
|   +---------+       +------------------------------------------------+     |
|        ^                                                                   |
|        | Parsed from transactions                                          |
|   +----v--------------------------------------------------------------+    |
|   | ... | Block n | Block n+1 | Block n+2 | Block n+3 | ...           |    |
|   +-------------------------------------------------------------------+    |
|                                                                            |
+----------------------------------------------------------------------------+
```

*图：Blockstack 三层架构示意图*

#### 第一层：控制平面 - 虚拟链 (Virtualchain)

控制平面是 Blockstack 的信任根基，它处理的是最关键但数据量最小的信息： **名字和公钥的绑定** 。它建立在像比特币这样的底层区块链之上。

但 Blockstack 并不想直接修改比特币，也不想创建一个新的、安全性未知的小区块链。为此，它引入了一个非常聪明的抽象层，叫做 **虚拟链 (Virtualchain)** 。

**虚拟链 (Virtualchain)** 就像一个在现有区块链之上运行的“虚拟机”。它定义了一套自己的、新的操作规则（比如“注册名字”、“转移名字”），但将这些操作作为元数据嵌入到底层区块链（如比特币）的普通交易中，通常是放在一个叫做 `OP_RETURN` 的字段里。底层的比特币矿工并不知道这些数据的含义，他们只是正常地打包交易。但 Blockstack 的节点会扫描区块链，解析这些 `OP_RETURN` 数据，并根据虚拟链定义的规则来构建和更新自己的状态数据库。

这种设计的最大好处是 **灵活性和安全性** 。它将复杂的应用逻辑与底层区块链解耦，使得 Blockstack 可以在不改变比特币的情况下添加新功能，甚至可以在未来从一个区块链迁移到另一个。

为了防止 **抢先注册 (front-running)** （即攻击者看到你要注册某个名字的交易后，抢在你之前注册掉），Blockstack 采用 **两阶段提交** 的方式来注册名字：
1.  **预购 (Preorder)** ：用户先提交一个交易，其中包含目标名字的哈希值，但不包含名字本身。
2.  **注册 (Register)** ：在预购交易被确认后，用户再提交第二个交易，这次包含明文的名字和公钥等信息。

攻击者只看到哈希值，无法知道你具体想注册哪个名字，从而无法抢跑。此外，注册名字需要支付一笔费用，这笔费用会被“销毁”，其目的是提高抢注大量名字的成本，防止域名抢占和垃圾信息。

#### 第二层：数据平面 - Atlas 网络 (Atlas Network)

控制平面（区块链）只存储小量的、最关键的绑定信息（比如名字 -> zonefile 哈希），而不存储具体的数据路由信息。这些路由信息，即 **区域文件 (zone files)** ，格式与传统 DNS 的 zone file 类似，被存储在数据平面的第一部分：一个叫做 **Atlas 的对等网络 (Atlas Network)** 中。

Blockstack 的早期版本曾使用基于 **分布式哈希表 (DHT)** 的网络，但实践中发现 DHT 网络存在很多问题，比如容易遭受 **女巫攻击 (Sybil attacks)** 、网络分区（尤其是在全球部署时），以及节点流失（churn）导致的可靠性问题。

因此，Blockstack 设计了 Atlas 网络来取代它。Atlas 的设计基于一个关键观察：所有 zone file 的总大小其实很小（例如，7 万个域名的数据加起来也只有 300MB）。所以，Atlas 网络采用了一种更简单、更稳健的策略： **每个节点都保存所有 zone file 的完整副本** 。

这个设计带来了几个好处：
* **高可靠性** ：没有了复杂的路由和数据分片，网络分区的概念不复存在。节点可以轻松地从其他对等节点那里同步丢失的数据。
* **抗审查性** ：由于数据被完全复制，攻击者要想审查某个信息，必须攻击网络中的大量节点，这比在 DHT 中攻击少量负责存储特定信息的节点要困难得多。
* **安全性** ：Atlas 网络是“白名单”模式的。一个节点只接受并存储那些 **哈希值已经在区块链上注册过** 的 zone file 。这意味着任何人都无法向网络中注入垃圾或伪造的数据。

#### 第三层：数据平面 - Gaia 存储系统 (Gaia Storage System)

Atlas 网络解决了“如何找到数据”的问题，而数据平面最终的 **存储层 (storage layer)** 则负责实际存储用户数据，比如你的个人资料、博客文章、照片等。这一层被称为 **Gaia 存储系统 (Gaia Storage System)** 。

Gaia 的设计哲学非常务实： **它不重新发明轮子，而是将现有的、成熟的商业云存储服务（如 Amazon S3、Dropbox、Google Drive）当作“哑驱动器 (dumb drives)”来使用** 。

具体工作方式如下：
1.  用户的数据在上传前，会在本地设备上进行 **签名和/或加密** 。
2.  加密后的数据（对于云服务商来说只是一堆无意义的二进制数据）被上传到用户自己选择并控制的云存储账户中。
3.  指向这个存储位置的 URI 被记录在用户的 zone file 中，该 zone file 存储在 Atlas 网络里。

当其他用户或应用想要访问你的数据时，整个查找流程是：

```txt
查询名字 -> 访问区块链，获取 zone file 哈希 ->
访问 Atlas 网络，用哈希获取 zone file ->
从 zone file 中找到 Gaia 的存储 URI ->
访问 Gaia，获取加密数据 -> 用所有者的公钥验证签名并解密数据
```

在这个模型中，云存储提供商无法窥探你的数据内容，也无法篡改你的数据（因为任何篡改都会导致签名验证失败）。用户真正拥有了对自己数据的控制权，同时还能享受到商业云存储服务的高性能和高可靠性。

### 探讨与反思：Blockstack 真的完美吗？

Blockstack 的愿景虽然吸引人，但在实践中也面临诸多挑战和质疑。这些问题帮助我们更清醒地看待去中心化系统。

#### 对开发者的挑战

从开发者的角度看，为 Blockstack 构建应用比为传统网站开发要困难得多。

* **受限的存储接口** ：开发者只能使用简单的 **键值存储 (key/value store)** ，而无法利用功能强大的 SQL 数据库进行复杂查询。
* **共享数据和隐私的矛盾** ：在去中心化模型中，代码运行在用户的设备上。这使得一些需要中心化仲裁的应用逻辑变得极难实现。例如，一个在线拍卖应用（如 eBay），服务器需要知道所有人的出价，但又不能让任何一个竞拍者看到其他人的具体出价。在 Blockstack 架构下，如果应用代码能看到所有出价，那么运行该代码的用户也能通过修改代码看到所有出价，这破坏了拍卖的公平性。
* **应用状态管理困难** ：很多 Web 应用需要维护一些不属于任何单个用户的公共状态，比如文章的点赞数、论坛的帖子排名等。在 Blockstack 中，没有中心服务器来聚合和管理这些信息，实现起来非常棘手。

#### 对用户的挑战：隐私、控制与成本

尽管 Blockstack 旨在提升用户隐私和控制权，但现实并非那么简单。

* **隐私真的提升了吗？**：虽然 Gaia 上的数据是加密的，云服务商看不到，但你仍然需要信任 **应用软件本身** 。如果一个由 Facebook 开发的 Blockstack 应用运行在你的电脑上，它依然有可能在本地收集你的信息并发送回 Facebook 的服务器。信任的环节从“信任远程服务器”转移到了“信任本地运行的应用”。
* **密钥管理的难题** ：用户的 **主私钥 (master private key)** 是所有安全的基石，一旦丢失或被盗，后果不堪设想。而普通用户在安全地管理密钥方面往往经验不足，很容易丢失、遗忘密码，或被恶意软件窃取。
* **用户的真实需求** ：用户真的关心数据所有权，愿意为了“切换应用”的自由而牺牲便利性吗？用户是否愿意为自己的 Gaia 存储空间付费，而不是享受免费（但有广告）的中心化服务？这些都是未知数。

#### 关于命名和 PKI 的再思考

Blockstack 的基石是其去中心化的 PKI，但这里也存在一些深刻的挑战。

* **命名与现实身份** ：Blockstack 的名字（如 `satoshi.id`）和你的真实世界身份之间没有必然联系。我如何能确定 `satoshi.id` 真的就是我想找的那个中本聪呢？。这种身份验证问题是所有 PKI 系统都面临的根本难题。
* **去中心化是必须的吗** ：有人提出，是否可以用像 **证书透明度 (Certificate Transparency, CT)** 这样的系统来代替昂贵且缓慢的比特币区块链？答案是否定的。CT 的作用是 **揭露** 冲突（比如两个人都声称注册了同一个域名），但它无法 **解决** 冲突，即无法决定谁是合法的拥有者。而这正是比特币通过挖矿共识和交易费用所实现的核心价值：它能强制全网就“谁先注册”达成唯一共识，并增加作恶成本。

### 总结

通过对 Blockstack 的学习，我们可以得到以下几点启示：

* **去中心化的愿景是美好的** ：将数据控制权交还用户的理念具有强大的吸引力。
* **PKI 是基石** ：任何试图构建可信互联网的努力都离不开一个稳健的公钥基础设施。Blockstack 在这方面的探索非常有价值。
* **理想与现实的差距** ：将应用与数据分离，听起来很棒，但给开发者带来了巨大的挑战，他们可能会因此讨厌这个模型。同时，用户可能并不关心，也不愿为此付费。
* **安全与便利的权衡** ：端到端加密虽然增强了隐私，但密钥管理的复杂性和脆弱性，以及在共享数据上的不便，都是需要付出的代价。
* **信任从未消失，只是转移** ：即便应用运行在你的电脑上，你仍然需要信任软件的开发者。这是否比信任服务器运营商是一个巨大的胜利，目前尚不明确。

总而言之，Blockstack 是对下一代互联网架构的一次勇敢而富有启发性的探索。尽管它面临诸多挑战，但它提出的问题和设计的方案，为我们思考如何构建一个更安全、更公平、更开放的数字世界提供了宝贵的经验。如果 Blockstack 或类似的系统最终能够成功，那将是一件非常了不起的事情。
