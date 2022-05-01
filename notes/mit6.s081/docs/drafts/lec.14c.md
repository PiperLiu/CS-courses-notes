# LEC 14 (fk): File systems

笔记大量参考了[肖宏辉](https://www.zhihu.com/people/xiao-hong-hui-15)大佬的翻译。

目录：

<!-- @import "[TOC]" {cmd="toc" depthFrom=2 depthTo=2 orderedList=false} -->

<!-- code_chunk_output -->

- [文件系统特性](#文件系统特性)
- [File System 文件系统](#file-system-文件系统)
- [从代码角度认识 file system](#从代码角度认识-file-system)
- [Sleep Lock](#sleep-lock)
- [总结](#总结)

<!-- /code_chunk_output -->

细分目录：

<!-- @import "[TOC]" {cmd="toc" depthFrom=2 depthTo=6 orderedList=false} -->

<!-- code_chunk_output -->

- [文件系统特性](#文件系统特性)
- [File System 文件系统](#file-system-文件系统)
  - [从系统调用推测文件系统大概实现](#从系统调用推测文件系统大概实现)
  - [现代数据库往往建立在操作系统文件系统上](#现代数据库往往建立在操作系统文件系统上)
  - [文件系统结构概述](#文件系统结构概述)
  - [文件系统与 disk 存储设备](#文件系统与-disk-存储设备)
    - [术语：sectors和blocks](#术语sectors和blocks)
    - [存储设备概述](#存储设备概述)
  - [inode](#inode)
    - [从哪个 block 开始读取？](#从哪个-block-开始读取)
    - [directory 目录](#directory-目录)
- [从代码角度认识 file system](#从代码角度认识-file-system)
  - [运行 echo 时文件系统的变化](#运行-echo-时文件系统的变化)
  - [XV6创建inode代码（涉及 sys_open 、 create 以及 bio.c）](#xv6创建inode代码涉及-sys_open-create-以及-bioc)
- [Sleep Lock](#sleep-lock)
- [总结](#总结)

<!-- /code_chunk_output -->

## 文件系统特性

文献系统背后的机制：
- 文件系统对硬件的抽象较为有用，所以理解文件系统对于硬件的抽象是如何实现的还是有点意思的。
- 除此之外，还有个关键且有趣的地方就是crash safety。有可能在文件系统的操作过程中，计算机崩溃了，在重启之后你的文件系统仍然能保持完好，文件系统的数据仍然存在，并且你可以继续使用你的大部分文件。如果文件系统操作过程中计算机崩溃了，然后你重启之后文件系统不存在了或者磁盘上的数据变了，那么崩溃的将会是你。所以crash safety是一个非常重要且经常出现的话题，我们下节课会专门介绍它。
- 之后是一个通用的问题，如何在磁盘上排布文件系统。例如目录和文件，它们都需要以某种形式在磁盘上存在，这样当你重启计算机时，所有的数据都能恢复。所以在磁盘上有一些数据结构表示了文件系统的结构和内容。在XV6中，使用的数据结构非常简单，因为XV6是专门为教学目的创建的。真实的文件系统通常会更加复杂。但是它们都是磁盘上保存的数据结构，我们在今天的课程会重点看这部分。
- 最后一个有趣的话题是性能。文件系统所在的硬件设备通常都较慢，比如说向一个SSD磁盘写数据将会是毫秒级别的操作，而在一个毫秒内，计算机可以做大量的工作，所以尽量避免写磁盘很重要，我们将在几个地方看到提升性能的代码。比如说，所有的文件系统都有buffer cache或者叫block cache。同时这里会有更多的并发，比如说你正在查找文件路径名，这是一个多次交互的操作，首先要找到文件结构，然后查找一个目录的文件名，之后再去查找下一个目录等等。**你会期望当一个进程在做路径名查找时，另一个进程可以并行的运行。这样的并行运行在文件系统中将会是一个大的话题。**

## File System 文件系统

### 从系统调用推测文件系统大概实现

首先让我们来看一个简单的场景，假设我们创建了文件“x/y”，或者说在目录x中创建了文件y，同时我们需要提供一些标志位，现在我们还太关心标志位所以我会忽略它。

```c
fd = open("x/y", ...);
```

上面的系统调用会创建文件，并返回文件描述符给调用者。调用者也就是用户应用程序可以对文件描述符调用write，有关write我们在之前已经看过很多次了，这里我们向文件写入“abc”三个字符。

```c
write(fd, "abc", 3);
```

从这两个调用已经可以看出一些信息了：
- 首先出现在接口中的路径名是可读的名字，而不是一串数字，它是由用户选择的字符串。
- write系统调用并没有使用offset作为参数， **所以写入到文件的哪个位置是隐式包含在文件系统中，文件系统在某个位置必然保存了文件的offset。** 因为如果你再调用write系统调用，新写入的数据会从第4个字节开始。

除此之外，还有一些我们之前没有看过的有趣的系统调用。例如XV6和所有的Unix文件系统都支持通过系统调用创建链接，给同一个文件指定多个名字。你可以通过调用link系统调用，为之前创建的文件“x/y”创建另一个名字“x/z”。

```c
link("x/y", "x/z");
```

所以文件系统内部需要以某种方式跟踪指向同一个文件的多个文件名。

我们还可能会在文件打开时，删除或者更新文件的命名空间。例如，用户可以通过unlink系统调用来删除特定的文件名。如果此时相应的文件描述符还是打开的状态，那我们还可以向文件写数据，并且这也能正常工作。

```c
unlink("x/y");
write(fd, "def", 3);
```

所以，在文件系统内部，文件描述符必然与某个对象关联，而这个对象不依赖文件名。这样，即使文件名变化了，文件描述符仍然能够指向或者引用相同的文件对象。所以， **实际上操作系统内部需要对于文件有内部的表现形式，并且这种表现形式与文件名无关。**

### 现代数据库往往建立在操作系统文件系统上

除此之外，我还想提一点。文件系统的目的是实现上面描述的API，也即是典型的文件系统API。但是，这并不是唯一构建一个存储系统的方式。如果只是在磁盘上存储数据，你可以想出一个完全不同的API。举个例子，数据库也能持久化的存储数据，但是数据库就提供了一个与文件系统完全不一样的API。所以记住这一点很重要： **还存在其他的方式能组织存储系统。** 我们这节课关注在文件系统，文件系统通常由操作系统提供，而数据库如果没有直接访问磁盘的权限的话，通常是在文件系统之上实现的（注，早期数据库通常直接基于磁盘构建自己的文件系统，因为早期操作系统自带的文件系统在性能上较差，且写入不是同步的，进而导致数据库的ACID不能保证。 **不过现代操作系统自带的文件系统已经足够好，所以现代的数据库大部分构建在操作系统自带的文件系统之上** ）。

- 学生提问：link增加了了对于文件的一个引用，unlink减少了一个引用？
- Frans教授：是的。我们稍后会介绍更多相关的内容。
- 学生提问：能介绍一下soft link和hard link吗？
- Frans教授：我今天不会讨论这些内容。但是你们将会在下一个File system lab中实现soft link。所以XV6本身实现了hard link，需要你们来实现soft link。
- 学生提问：link是对inode做操作，而不是对文件描述符做操作，对吧？
- Frans教授：是的，link是对inode做操作，我们接下来介绍这部分内容。

### 文件系统结构概述

文件系统究竟维护了什么样的结构来实现前面介绍的API呢？

首先，最重要的可能就是inode，这是代表一个文件的对象，并且它不依赖于文件名。实际上，inode是通过自身的编号来进行区分的，这里的编号就是个整数。所以文件系统内部通过一个数字，而不是通过文件路径名引用inode。同时，基于之前的讨论，inode必须有一个link count来跟踪指向这个inode的文件名的数量。一个文件（inode）只能在link count为0的时候被删除。实际的过程可能会更加复杂，实际中还有一个openfd count，也就是当前打开了文件的文件描述符计数。**一个文件只能在这两个计数器（link count 和 openfd count）都为0的时候才能被删除。**

同时基于之前的讨论，我们也知道write和read都没有针对文件的offset参数，所以文件描述符必然自己悄悄维护了对于文件的offset。

文件系统中核心的数据结构就是inode和file descriptor。后者主要与用户进程进行交互。

![](./images/2022043001.png)

尽管文件系统的API很相近并且内部实现可能非常不一样。但是很多文件系统都有类似的结构。因为文件系统还挺复杂的，所以最好按照分层的方式进行理解。可以这样看：
- 在最底层是磁盘，也就是一些实际保存数据的存储设备，正是这些设备提供了持久化存储。
- 在这之上是buffer cache或者说block cache，这些cache可以避免频繁的读写磁盘。这里我们将磁盘中的数据保存在了内存中。
- 为了保证持久性，再往上通常会有一个logging层。许多文件系统都有某种形式的logging，我们下节课会讨论这部分内容，所以今天我就跳过它的介绍。
- 在logging层之上，XV6有inode cache，这主要是为了同步（synchronization），我们稍后会介绍。inode通常小于一个disk block，所以多个inode通常会打包存储在一个disk block中。为了向单个inode提供同步操作，XV6维护了inode cache。
- 再往上就是inode本身了。它实现了read/write。
- 再往上，就是文件名，和文件描述符操作。

不同的文件系统组织方式和每一层可能都略有不同，有的时候分层也没有那么严格，即使在XV6中分层也不是很严格，但是从概念上来说这里的结构对于理解文件系统还是有帮助的。实际上所有的文件系统都有组件对应这里不同的分层，例如buffer cache，logging，inode和路径名。

### 文件系统与 disk 存储设备

接下来，我将简单的介绍最底层，也即是存储设备。实际中有非常非常多不同类型的存储设备，这些设备的区别在于性能，容量，数据保存的期限等。其中两种最常见，并且你们应该也挺熟悉的是SSD和HDD。这两类存储虽然有着不同的性能，但是都在合理的成本上提供了大量的存储空间。**SSD通常是0.1到1毫秒的访问时间，而HDD通常是在10毫秒量级完成读写一个disk block。**

#### 术语：sectors和blocks

- sector通常是磁盘驱动可以读写的最小单元，它过去通常是512字节。
- block通常是操作系统或者文件系统视角的数据。它由文件系统定义，在XV6中它是1024字节。所以XV6中一个block对应两个sector。通常来说一个block对应了一个或者多个sector。

有的时候，人们也将磁盘上的sector称为block。所以这里的术语也不是很精确。

#### 存储设备概述

![](./images/2022043002.png)

这些存储设备连接到了电脑总线之上，总线也连接了CPU和内存。一个文件系统运行在CPU上，将内部的数据存储在内存，同时也会以读写block的形式存储在SSD或者HDD。这里的接口还是挺简单的，包括了read/write，然后以block编号作为参数。虽然我们这里描述的过于简单了，但是实际的接口大概就是这样。

在内部，SSD和HDD工作方式完全不一样，但是对于硬件的抽象屏蔽了这些差异。磁盘驱动通常会使用一些标准的协议，例如PCIE，与磁盘交互。从上向下看磁盘驱动的接口，大部分的磁盘看起来都一样，你可以提供block编号，在驱动中通过写设备的控制寄存器，然后设备就会完成相应的工作。这是从一个文件系统的角度的描述。尽管不同的存储设备有着非常不一样的属性，从驱动的角度来看，你可以以大致相同的方式对它们进行编程。

有关存储设备我们就说这么多。

- 学生提问：对于read/write的接口，是不是提供了同步/异步的选项？
- Frans教授：你可以认为一个磁盘的驱动与console的驱动是基本一样的。驱动向设备发送一个命令表明开始读或者写，过了一会当设备完成了操作，会产生一个中断表明完成了相应的命令。但是因为磁盘本身比console复杂的多，所以磁盘的驱动也会比我们之前看过的console的驱动复杂的多。不过驱动中的代码结构还是类似的，也有bottom部分和top部分，中断和读写控制寄存器。

从文件系统的角度来看磁盘还是很直观的。因为对于磁盘就是读写block或者sector， **我们可以将磁盘看作是一个巨大的block的数组，数组从0开始，一直增长到磁盘的最后。**

![](./images/2022043003.png)

而文件系统的工作就是将所有的数据结构以一种能够在重启之后重新构建文件系统的方式，存放在磁盘上。虽然有不同的方式，但是XV6使用了一种非常简单，但是还挺常见的布局结构。

通常来说：
- block0要么没有用，要么被用作boot sector来启动操作系统。
- block1通常被称为super block，它描述了文件系统。它可能包含磁盘上有多少个block共同构成了文件系统这样的信息。我们之后会看到XV6在里面会存更多的信息，你可以通过block1构造出大部分的文件系统信息。
- 在XV6中，log从block2开始，到block32结束。实际上log的大小可能不同，这里在super block中会定义log就是30个block。
- 接下来在block32到block45之间，XV6存储了inode。我之前说过多个inode会打包存在一个block中，一个inode是64字节。
- 之后是bitmap block，这是我们构建文件系统的默认方法，它只占据一个block。它记录了数据block是否空闲。
- 之后就全是数据block了，数据block存储了文件的内容和目录的内容。

通常来说，bitmap block，inode blocks和log blocks被统称为metadata block。它们虽然不存储实际的数据，但是它们存储了能帮助文件系统完成工作的元数据。

- 学生提问： **boot block是不是包含了操作系统启动的代码？**
- Frans教授：完全正确，它里面通常包含了足够启动操作系统的代码。之后再从文件系统中加载操作系统的更多内容。
- 学生提问：所以XV6是存储在虚拟磁盘上？
- Frans教授：在QEMU中，我们实际上走了捷径。QEMU中有个标志位`kernel`，它指向了内核的镜像文件，QEMU会将这个镜像的内容加载到了物理内存的`0x80000000`。所以当我们使用QEMU时，我们不需要考虑boot sector。
- 学生提问： **所以当你运行QEMU时，你就是将程序通过命令行传入，然后直接就运行传入的程序，然后就不需要从虚拟磁盘上读取数据了？**
- Frans教授：完全正确。

假设inode是64字节，如果你想要读取`inode10`，那么你应该按照下面的公式去对应的block读取inode。

$$
32 + \frac{inode\# \times 64}{1024}
$$

所以inode0在block32，inode17会在block33。只要有inode的编号，我们总是可以找到inode在磁盘上存储的位置。

### inode

接下来我们看一下磁盘上存储的inode究竟是什么？首先我们前面已经看过了，这是一个64字节的数据结构。

- 通常来说它有一个type字段，表明inode是文件还是目录。
- nlink字段，也就是link计数器，用来跟踪究竟有多少文件名指向了当前的inode。
- size字段，表明了文件数据有多少个字节。
- 不同文件系统中的表达方式可能不一样，不过在XV6中接下来是一些block的编号，例如编号0，编号1，等等。XV6的inode中总共有12个block编号。这些被称为direct block number。这12个block编号指向了构成文件的前12个block。举个例子，如果文件只有2个字节，那么只会有一个block编号0，它包含的数字是磁盘上文件前2个字节的block的位置。
- 之后还有一个indirect block number，它对应了磁盘上一个block，这个block包含了256个block number，这256个block number包含了文件的数据。所以inode中block number 0到block number 11都是direct block number，而block number 12保存的indirect block number指向了另一个block。

以上基本就是XV6中inode的组成部分。基于上面的内容，XV6中文件最大的长度是多少呢？
- 学生回答：会是268*1024字节

是的，最大文件尺寸对应的是下面的公式。

$$
\text{max file size:}
(256 + 12) \times 1024
$$

可以算出这里就是268KB，这么点大小能存个什么呢？所以这是个很小的文件长度，实际的文件系统，文件最大的长度会大的多得多。那可以做一些什么来让文件系统支持大得多的文件呢？

- 学生回答：可以扩展inode中indirect部分吗？

是的， **可以用类似page table的方式，构建一个双重indirect block number指向一个block，这个block中再包含了256个indirect block number，每一个又指向了包含256个block number的block。** 这样的话，最大的文件长度会大得多（注，是`256*256*1K`）。这里修改了inode的数据结构，你可以使用类似page table的树状结构，也可以按照B树或者其他更复杂的树结构实现。XV6这里极其简单，基本是按照最早的Uinx实现方式来的，不过你可以实现更复杂的结构。实际上，在接下来的File system lab中，你将会实现双重indirect block number来支持更大的文件。

- 学生提问：为什么每个block存储256个block编号？
- Frans教授：因为每个编号是4个字节。`1024/4 = 256`。这又带出了一个问题，如果block编号只是4个字节，磁盘最大能有多大？是的，2的32次方（注，4TB）。有些磁盘比这个数字要大，所以通常人们会使用比32bit更长的数字来表示block编号。

在下一个File system lab，你们需要将inode中的一个block number变成双重indirect block number，这个双重indirect block number将会指向一个包含了256个indirect block number的block，其中的每一个indirect block number再指向一个包含了256个block number的block，这样文件就可以大得多。

#### 从哪个 block 开始读取？

接下来，我们想要实现read系统调用。假设我们需要读取文件的第8000个字节，那么你该读取哪个block呢？从inode的数据结构中该如何计算呢？

- 学生回答：先应该减去12个direct block的大小，然后再看在indirect block中的偏移量是多少。

对于8000，我们首先除以1024，也就是block的大小，得到大概是7。这意味着第7个block就包含了第8000个字节。所以直接在inode的direct block number中，就包含了第8000个字节的block。为了找到这个字节在第7个block的哪个位置，我们需要用8000对1024求余数，我猜结果是是832。

所以为了读取文件的第8000个字节，文件系统查看inode，先用8000除以1024得到block number，然后再用8000对1024求余读取block中对应的字节。

总结一下，inode中的信息完全足够用来实现read/write系统调用，至少可以找到哪个disk block需要用来执行read/write系统调用。

#### directory 目录

接下来我们讨论一下目录（directory）。文件系统的酷炫特性就是层次化的命名空间（hierarchical namespace），你可以在文件系统中保存对用户友好的文件名。大部分Unix文件系统有趣的点在于，一个目录本质上是一个文件加上一些文件系统能够理解的结构。在XV6中，这里的结构极其简单。每一个目录包含了directory entries，每一条entry都有固定的格式：
- 前2个字节包含了目录中文件或者子目录的inode编号
- 接下来的14个字节包含了文件或者子目录名

所以每个entry总共是16个字节。

对于实现路径名查找，这里的信息就足够了。假设我们要查找路径名“`/y/x`”，我们该怎么做呢？

从路径名我们知道，应该从root inode开始查找。通常root inode会有固定的inode编号，在XV6中，这个编号是1。我们该如何根据编号找到root inode呢？从前一节我们可以知道，inode从block 32开始，如果是inode1，那么必然在block 32中的64到128字节的位置。所以文件系统可以直接读到root inode的内容。

![](./images/2022050101.png)

对于路径名查找程序，接下来就是扫描root inode包含的所有block，以找到“`y`”。该怎么找到root inode所有对应的block呢？根据前一节的内容就是读取所有的direct block number和indirect block number。

结果可能是找到了，也可能是没有找到。如果找到了，那么目录y也会有一个inode编号，假设是251。

我们可以继续从inode 251查找，先读取inode 251的内容，之后再扫描inode所有对应的block，找到“`x`”并得到文件x对应的inode编号，最后将其作为路径名查找的结果返回。

- 学生提问：有没有一些元数据表明当前的inode是目录而不是一个文件？
- Frans教授：有的，实际上是在inode中。inode中的type字段表明这是一个目录还是一个文件。如果你对一个类型是文件的inode进行查找，文件系统会返回错误。

**很明现，这里的结构不是很有效。为了找到一个目录名，你需要线性扫描。实际的文件系统会使用更复杂的数据结构来使得查找更快，当然这又是设计数据结构的问题，而不是设计操作系统的问题。你可以使用你喜欢的数据结构并提升性能。** 出于简单和更容易解释的目的，XV6使用了这里这种非常简单的数据结构。

## 从代码角度认识 file system

接下来我们看一下实际中，XV6的文件系统是如何工作的，这部分内容对于下一个lab是有帮助的。

首先我会启动XV6，这里有件事情我想指出。启动XV6的过程中，调用了makefs指令，来创建一个文件系统。

![](./images/2022050102.png)

所以makefs创建了一个全新的磁盘镜像，在这个磁盘镜像中包含了我们在指令中传入的一些文件。makefs为你创建了一个包含这些文件的新的文件系统。

XV6总是会打印文件系统的一些信息，所以从指令的下方可以看出有46个meta block，其中包括了：
- boot block
- super block
- 30个log block
- 13个inode block
- 1个bitmap block
- 之后是954个data block

所以这是一个袖珍级的文件系统，总共就包含了1000个block。在File system lab中，你们会去支持更大的文件系统。

我还稍微修改了一下XV6，使得任何时候写入block都会打印出block的编号。我们从console的输出可以看出，在XV6启动过程中，会有一些对于文件系统的调用，并写入了block 33，45，32。

### 运行 echo 时文件系统的变化

接下来我们运行一些命令，来看一下特定的命令对哪些block做了写操作，并理解为什么要对这些block写入数据。我们通过`echo "hi" > x`，来创建一个文件x，并写入字符 `hi`。我会将输出拷贝出来，并做分隔以方便我们更好的理解。

![](./images/2022050103.png)

这里会有几个阶段
- 第一阶段是创建文件
- 第二阶段将“hi”写入文件
- 第三阶段将“\n”换行符写入到文件

如果你去看echo的代码实现，基本就是这3个阶段。

```c
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  int i;

  for(i = 1; i < argc; i++){
    write(1, argv[i], strlen(argv[i]));
    if(i + 1 < argc){
      write(1, " ", 1);
    } else {
      write(1, "\n", 1);
    }
  }
  exit(0);
}
```

上面就是echo的代码，它先检查参数，并将参数写入到文件描述符1，在最后写入一个换行符。

让我们一个阶段一个阶段的看echo的执行过程，并理解对于文件系统发生了什么。相比看代码，这里直接看磁盘的分布图更方便。

![](./images/2022043003.png)

你们觉得的write 33代表了什么？我们正在创建文件，所以我们期望文件系统干什么呢？

- 学生回答：这是在写inode。

是的，看起来给我们分配的inode位于block 33。之所以有两个write 33，第一个是为了标记inode将要被使用。在XV6中，我记得是使用inode中的type字段来标识inode是否空闲，这个字段同时也会用来表示inode是一个文件还是一个目录。所以这里将inode的type从空闲改成了文件，并写入磁盘表示这个inode已经被使用了。第二个write 33就是实际的写入inode的内容。inode的内容会包含linkcount为1以及其他内容。

write 46是向第一个data block写数据，那么这个data block属于谁呢？

- 学生回答：属于根目录。

是的，block 46是根目录的第一个block。为什么它需要被写入数据呢？

- 学生回答：因为我们正在向根目录创建一个新文件。

是的，这里我们向根目录增加了一个新的entry，其中包含了文件名x，以及我们刚刚分配的inode编号。

接下来的write 32又是什么意思呢？block 32保存的仍然是inode，那么inode中的什么发生了变化使得需要将更新后的inode写入磁盘？ **是的，根目录的大小变了，因为我们刚刚添加了16个字节的entry来代表文件x的信息。**

最后又有一次write 33，我在稍后会介绍这次写入的内容，这里我们再次更新了文件x的inode， 尽管我们又还没有写入任何数据。

以上就是第一阶段创建文件的过程。第二阶段是向文件写入“`hi`”。

首先是write 45，这是更新bitmap。文件系统首先会扫描bitmap来找到一个还没有使用的data block，未被使用的data block对应bit 0。找到之后，文件系统需要将该bit设置为1，表示对应的data block已经被使用了。所以更新block 45是为了更新bitmap。

接下来的两次write 595表明，文件系统挑选了data block 595。所以在文件x的inode中，第一个direct block number是595。因为写入了两个字符，所以write 595被调用了两次。

第二阶段最后的write 33是更新文件x对应的inode中的size字段，因为现在文件x中有了两个字符。

- 学生提问：block 595看起来在磁盘中很靠后了，是因为前面的block已经被系统内核占用了吗？
- Frans教授：我们可以看前面makefs指令，makefs存了很多文件在磁盘镜像中，这些都发生在创建文件x之前，所以磁盘中很大一部分已经被这些文件填满了（如下图 README 开始的部分）。
- 学生提问：第二阶段最后的write 33是否会将block 595与文件x的inode关联起来？
- Frans教授：会的。这里的write 33会发生几件事情：首先inode的size字段会更新；第一个direct block number会更新。这两个信息都会通过write 33一次更新到磁盘上的inode中。

![](./images/2022050102.png)

### XV6创建inode代码（涉及 sys_open 、 create 以及 bio.c）

接下来我们通过查看XV6中的代码，更进一步的了解文件系统。因为我们前面已经分配了inode，我们先来看一下这是如何发生的。sysfile.c中包含了所有与文件系统相关的函数，分配inode发生在sys_open函数中，这个函数会负责创建文件。

```c
uint64
sys_open(void)
{
  char path[MAXPATH];
  int fd, omode;
  struct file *f;
  struct inode *ip;
  int n;

  if((n = argstr(0, path, MAXPATH)) < 0 || argint(1, &omode) < 0)
    return -1;

  begin_op();

  if(omode & O_CREATE){
    ip = create(path, T_FILE, 0, 0);
    if(ip == 0){
      end_op();
      return -1;
    }
  } else {
    if((ip = namei(path)) == 0){
      end_op();
      return -1;
    }
    ilock(ip);
    if(ip->type == T_DIR && omode != O_RDONLY){
      iunlockput(ip);
      end_op();
      return -1;
    }
  }

  if(ip->type == T_DEVICE && (ip->major < 0 || ip->major >= NDEV)){
    iunlockput(ip);
    end_op();
    return -1;
  }

  if((f = filealloc()) == 0 || (fd = fdalloc(f)) < 0){
    if(f)
      fileclose(f);
    iunlockput(ip);
    end_op();
    return -1;
  }

  if(ip->type == T_DEVICE){
    f->type = FD_DEVICE;
    f->major = ip->major;
  } else {
    f->type = FD_INODE;
    f->off = 0;
  }
  f->ip = ip;
  f->readable = !(omode & O_WRONLY);
  f->writable = (omode & O_WRONLY) || (omode & O_RDWR);

  if((omode & O_TRUNC) && ip->type == T_FILE){
    itrunc(ip);
  }

  iunlock(ip);
  end_op();

  return fd;
}
```

在sys_open函数中，会调用create函数。

```c
static struct inode*
create(char *path, short type, short major, short minor)
{
  struct inode *ip, *dp;
  char name[DIRSIZ];

  if((dp = nameiparent(path, name)) == 0)
    return 0;

  ilock(dp);

  if((ip = dirlookup(dp, name, 0)) != 0){
    iunlockput(dp);
    ilock(ip);
    if(type == T_FILE && (ip->type == T_FILE || ip->type == T_DEVICE))
      return ip;
    iunlockput(ip);
    return 0;
  }

  if((ip = ialloc(dp->dev, type)) == 0)
    panic("create: ialloc");

  ilock(ip);
  ip->major = major;
  ip->minor = minor;
  ip->nlink = 1;
  iupdate(ip);

  if(type == T_DIR){  // Create . and .. entries.
    dp->nlink++;  // for ".."
    iupdate(dp);
    // No ip->nlink++ for ".": avoid cyclic ref count.
    if(dirlink(ip, ".", ip->inum) < 0 || dirlink(ip, "..", dp->inum) < 0)
      panic("create dots");
  }

  if(dirlink(dp, name, ip->inum) < 0)
    panic("create: dirlink");

  iunlockput(dp);

  return ip;
}
```

create函数中首先会解析路径名并找到最后一个目录，之后会查看文件是否存在，如果存在的话会返回错误。之后就会调用ialloc（inode allocate），这个函数会为文件x分配inode。ialloc函数位于fs.c文件中。

```c
// Allocate an inode on device dev.
// Mark it as allocated by  giving it type type.
// Returns an unlocked but allocated and referenced inode.
struct inode*
ialloc(uint dev, short type)
{
  int inum;
  struct buf *bp;
  struct dinode *dip;

  for(inum = 1; inum < sb.ninodes; inum++){
    bp = bread(dev, IBLOCK(inum, sb));
    dip = (struct dinode*)bp->data + inum%IPB;
    if(dip->type == 0){  // a free inode
      memset(dip, 0, sizeof(*dip));
      dip->type = type;
      log_write(bp);   // mark it allocated on the disk
      brelse(bp);
      return iget(dev, inum);
    }
    brelse(bp);
  }
  panic("ialloc: no inodes");
}
```

以上就是ialloc函数，与XV6中的大部分函数一样，它很简单，但是又不是很高效。它会遍历所有可能的inode编号，找到inode所在的block，再看位于block中的inode数据的type字段。如果这是一个空闲的inode，那么将其type字段设置为文件，这会将inode标记为已被分配。函数中的log_write就是我们之前看到在console中有关写block的输出。这里的log_write是我们看到的整个输出的第一个。

以上就是第一次写磁盘涉及到的函数调用。这里有个有趣的问题，如果有多个进程同时调用create函数会发生什么？对于一个多核的计算机，进程可能并行运行，两个进程可能同时会调用到ialloc函数，然后进而调用bread（block read）函数。所以必须要有一些机制确保这两个进程不会互相影响。

让我们看一下位于bio.c的buffer cache代码。首先看一下bread函数。（实际上在上一个 lock 的 lab 中我们已经看到过） **这里不记录了，要看的话看 Lab: lock 去。**

bread函数首先会调用bget函数，bget会为我们从buffer cache中找到block的缓存。让我们看一下bget函数。

这里的代码还有点复杂。我猜你们之前已经看过这里的代码，那么这里的代码在干嘛？

- 学生回答：这里遍历了linked-list，来看看现有的cache是否符合要找的block。

是的，我们这里看一下block 33的cache是否存在，如果存在的话，将block对象的引用计数（refcnt）加1，之后再释放bcache锁，因为现在我们已经完成了对于cache的检查并找到了block cache。之后，代码会尝试获取block cache的锁。

所以，如果有多个进程同时调用bget的话，其中一个可以获取bcache的锁并扫描buffer cache。此时，其他进程是没有办法修改buffer cache的（注，因为bacche的锁被占住了）。之后，进程会查找block number是否在cache中，如果在的话将block cache的引用计数加1，表明当前进程对block cache有引用，之后再释放bcache的锁。如果有第二个进程也想扫描buffer cache，那么这时它就可以获取bcache的锁。假设第二个进程也要获取block 33的cache，那么它也会对相应的block cache的引用计数加1。最后这两个进程都会尝试对block 33的block cache调用acquiresleep函数。

acquiresleep是另一种锁，我们称之为sleep lock，本质上来说它获取block 33 cache的锁。其中一个进程获取锁之后函数返回。在ialloc函数中会扫描block 33中是否有一个空闲的inode。而另一个进程会在acquiresleep中等待第一个进程释放锁。

- 学生提问：当一个block cache的refcnt不为0时，可以更新block cache吗？因为释放锁之后，可能会修改block cache。
- Frans教授：这里我想说几点；首先XV6中对bcache做任何修改的话，都必须持有bcache的锁；其次对block 33的cache做任何修改你需要持有block 33的sleep lock。所以在任何时候，`release(&bcache.lock)`之后，`b->refcnt`都大于0。block的cache只会在refcnt为0的时候才会被驱逐，任何时候refcnt大于0都不会驱逐block cache。所以当`b->refcnt`大于0的时候，block cache本身不会被buffer cache修改。这里的第二个锁，也就是block cache的sleep lock，是用来保护block cache的内容的。它确保了任何时候只有一个进程可以读写block cache。

如果buffer cache中有两份block 33的cache将会出现问题。假设一个进程要更新inode19，另一个进程要更新inode20。如果它们都在处理block 33的cache，并且cache有两份，那么第一个进程可能持有一份cache并先将inode19写回到磁盘中，而另一个进程持有另一份cache会将inode20写回到磁盘中，并将inode19的更新覆盖掉。所以一个block只能在buffer cache中出现一次。你们在完成File system lab时，必须要维持buffer cache的这个属性。

- 学生提问：如果多个进程都在使用同一个block的cache，然后有一个进程在修改block，并通过强制向磁盘写数据修改了block的cache，那么其他进程会看到什么结果？
- Frans教授：如果第一个进程结束了对block 33的读写操作，它会对block的cache调用brelse（block cache release）函数。
- 这个函数会对refcnt减1，并释放sleep lock。这意味着，如果有任何一个其他进程正在等待使用这个block cache，现在它就能获得这个block cache的sleep lock，并发现刚刚做的改动。
- 假设两个进程都需要分配一个新的inode，且新的inode都位于block 33。如果第一个进程分配到了inode18并完成了更新，那么它对于inode18的更新是可见的。另一个进程就只能分配到inode19，因为inode18已经被标记为已使用，任何之后的进程都可以看到第一个进程对它的更新。
这正是我们想看到的结果，如果一个进程创建了一个inode或者创建了一个文件，之后的进程执行读就应该看到那个文件。

接下来让我们看一下brelse函数。

```c
// Release a locked buffer.
// Move to the head of the most-recently-used list.
void
brelse(struct buf *b)
{
  if(!holdingsleep(&b->lock))
    panic("brelse");

  releasesleep(&b->lock);

  acquire(&bcache.lock);
  b->refcnt--;
  if (b->refcnt == 0) {
    // no one is waiting for it.
    b->next->prev = b->prev;
    b->prev->next = b->next;
    b->next = bcache.head.next;
    b->prev = &bcache.head;
    bcache.head.next->prev = b;
    bcache.head.next = b;
  }
  
  release(&bcache.lock);
}
```

brelse函数中首先释放了sleep lock；之后获取了bcache的锁；之后减少了block cache的引用计数，表明一个进程不再对block cache感兴趣；最后如果引用计数为0，那么它会修改buffer cache的linked-list，将block cache移到linked-list的头部，这样表示这个block cache是最近使用过的block cache。这一点很重要，当我们在bget函数中不能找到block cache时，我们需要在buffer cache中腾出空间来存放新的block cache，这时会使用LRU（Least Recent Used）算法找出最不常使用的block cache，并撤回它（注，而将刚刚使用过的block cache放在linked-list的头部就可以直接更新linked-list的tail来完成LRU操作）。为什么这是一个好的策略呢？因为通常系统都遵循temporal locality策略，也就是说如果一个block cache最近被使用过，那么很有可能它很快会再被使用，所以最好不要撤回这样的block cache。

以上就是对于block cache代码的介绍。这里有几件事情需要注意：
- 首先在内存中，对于一个block只能有一份缓存。这是block cache必须维护的特性。
- 其次，这里使用了与之前的spinlock略微不同的sleep lock。与spinlock不同的是，可以在I/O操作的过程中持有sleep lock。
- 第三，它采用了LRU作为cache替换策略。
- 第四，它有两层锁。第一层锁用来保护buffer cache的内部数据；第二层锁也就是sleep lock用来保护单个block的cache。

- 学生提问：我有个关于brelse函数的问题，看起来它先释放了block cache的锁，然后再对引用计数refcnt减一，为什么可以这样呢？
- Frans教授：这是个好问题。如果我们释放了sleep lock，这时另一个进程正在等待锁，那么refcnt必然大于1，而`b->refcnt --`只是表明当前执行brelse的进程不再关心block cache。如果还有其他进程正在等待锁，那么refcnt必然不等于0，我们也必然不会执行`if(b->refcnt == 0)`中的代码。

> 实际上我认为这位学生想得到的回答是： 虽然我们释放了 `b->lock` ，但是我们依然持有 `bcache.lock` 。要知道其他进程持有 `b->lock` 并不会改边 `b->refcnt` ，改边 `refcnt` 的操作都被 `bcache.lock` 保护着呢。

## Sleep Lock

block cache使用的是sleep lock。sleep lock区别于一个常规的spinlock。我们先看来一下sleep lock。

```c
// kernel/sleeplock.h
// Long-term locks for processes
struct sleeplock {
  uint locked;       // Is the lock held?
  struct spinlock lk; // spinlock protecting this sleep lock
  
  // For debugging:
  char *name;        // Name of lock.
  int pid;           // Process holding lock
};

// kernel/sleeplock.c
// Sleeping locks

#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"
#include "sleeplock.h"

void
initsleeplock(struct sleeplock *lk, char *name)
{
  initlock(&lk->lk, "sleep lock");
  lk->name = name;
  lk->locked = 0;
  lk->pid = 0;
}

void
acquiresleep(struct sleeplock *lk)
{
  acquire(&lk->lk);
  while (lk->locked) {
    sleep(lk, &lk->lk);
  }
  lk->locked = 1;
  lk->pid = myproc()->pid;
  release(&lk->lk);
}

void
releasesleep(struct sleeplock *lk)
{
  acquire(&lk->lk);
  lk->locked = 0;
  lk->pid = 0;
  wakeup(lk);
  release(&lk->lk);
}

int
holdingsleep(struct sleeplock *lk)
{
  int r;
  
  acquire(&lk->lk);
  r = lk->locked && (lk->pid == myproc()->pid);
  release(&lk->lk);
  return r;
}
```

首先是acquiresleep函数，它用来获取sleep lock。函数里首先获取了一个普通的spinlock，这是与sleep lock关联在一起的一个锁。之后，如果sleep lock被持有，那么就进入sleep状态，并将自己从当前CPU调度开。

既然sleep lock是基于spinlock实现的，为什么对于block cache，我们使用的是sleep lock而不是spinlock？

- 学生回答：因为磁盘的操作需要很长的时间。

是的，这里其实有多种原因。对于spinlock有很多限制，其中之一是加锁时中断必须要关闭。所以如果使用spinlock的话，当我们对block cache做操作的时候需要持有锁，那么我们就永远也不能从磁盘收到数据。或许另一个CPU核可以收到中断并读到磁盘数据，但是如果我们只有一个CPU核的话，我们就永远也读不到数据了。出于同样的原因，也不能在持有spinlock的时候进入sleep状态。所以这里我们使用sleep lock。 **sleep lock的优势就是，我们可以在持有锁的时候不关闭中断。** 我们可以在磁盘操作的过程中持有锁，我们也可以长时间持有锁。当我们在等待sleep lock的时候，我们并没有让CPU一直空转，我们通过sleep将CPU出让出去了。

## 总结

最后让我们来总结一下，并把剩下的内容留到下节课。
- 首先，文件系统是一个位于磁盘的数据结构。我们今天的主要时间都用来介绍这个位于磁盘的数据结构的内容。XV6的这个数据结构实现的很简单，但是你可以实现一个更加复杂的数据结构。
- 其次，我们花了一些时间来看block cache的实现，这对于性能来说是至关重要的，因为读写磁盘是代价较高的操作，可能要消耗数百毫秒，而block cache确保了如果我们最近从磁盘读取了一个block，那么我们将不会再从磁盘读取相同的block。

下节课我将会介绍crash safety，这是文件系统设计中非常棒的一部分。我们将会在crash safety讲两节课。下节课我们会看到基于log实现的crash safety机制，下下节课我们会看到Linux的ext3是如何实现的logging，这种方式要快得多。
