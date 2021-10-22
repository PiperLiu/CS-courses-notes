# MIT6.S081: Operating System Engineering


<!-- @import "[TOC]" {cmd="toc" depthFrom=3 depthTo=6 orderedList=false} -->

<!-- code_chunk_output -->

- [🥂课程资料](#课程资料)
- [🥞本仓库资源](#本仓库资源)
- [课程索引](#课程索引)
  - [LEC 1 (rtm): Introduction and examples](#lec-1-rtm-introduction-and-examples)

<!-- /code_chunk_output -->

### 🥂课程资料

实际上，这门课就是大名鼎鼎的 MIT 6.828 ，只不过2020年课号改成了 6.S081 。

官方课程安排与资源索引：
- [https://pdos.csail.mit.edu/6.S081/2020/schedule.html](https://pdos.csail.mit.edu/6.S081/2020/schedule.html)

中文文档与翻译：
- [https://mit-public-courses-cn-translatio.gitbook.io/mit6-s081/](https://mit-public-courses-cn-translatio.gitbook.io/mit6-s081/)

B 站视频：
- [MIT 操作系统 6.S081（中英文字幕）](https://www.bilibili.com/video/BV1QA411F7ij)

实验：
- 本课程用到 `xv6` 系统，我从 `GitHub` 上 `fork` 了一个：https://github.com/PiperLiu/xv6-riscv
- 本课程进行实验的作业系统： `git clone git://g.csail.mit.edu/xv6-labs-2020`
  - 我也放到 `GitHub` 上了（❗❗这个仓库很重要❗❗）：https://github.com/PiperLiu/xv6-labs-2020
  - 大佬已经写完了作业，我可以参考答案：https://github.com/silvermagic/xv6-labs-2020

参考：
- [如何学好操作系统原理这门课？ - 菜饼不菜的回答 - 知乎](https://www.zhihu.com/question/22874344/answer/2091285378)
- [MIT6.S081（2020）翻译简介 - 肖宏辉的文章 - 知乎](https://zhuanlan.zhihu.com/p/261362497)

### 🥞本仓库资源

- 教材（主要用作预习）：[xv6: a simple, Unix-like teaching operating system](./docs/lec/book-riscv-rev1.pdf)
- 其他的资源比如课堂资料、作业要求在[docs/assignment/](./docs/assignment/)和[docs/lec/](./docs/lec/)中
- 实验说明：<a href="./docs/6.S081 _ Fall 2020.html">6.S081 _ Fall 2020.html</a>

### 课程索引

#### LEC 1 (rtm): Introduction and examples

sep 2

- LEC 1 (rtm): [Introduction](./docs/lec/lec1.l-overview.txt) and [examples](https://pdos.csail.mit.edu/6.S081/2020/lec/l-overview/) (handouts: xv6 book) [video](https://youtu.be/L6YqHxYHa7A)
- Preparation: Read [chapter 1](./docs/lec/book-riscv-rev1.pdf) (for your amusement: [Unix](https://www.youtube.com/watch?v=tc4ROCJYbm0))
- Homework 1 due: Question
- Assignment: <a href="./docs/assignment/Lab_ Xv6 and Unix utilities.html">Lab util: Unix utilities</a>

我认真读了书（第一张，一些 Unix 6 接口简单原理与使用），记了笔记：[./docs/drafts/lec.01.md](./docs/drafts/lec.01.md)。

然后听课，课堂笔记：[./docs/drafts/lec.01c.md](./docs/drafts/lec.01c.md) 

- [课程简介：操作系统做哪些事](./docs/drafts/lec.01c.md#课程简介操作系统做哪些事)
- [计算机系统](./docs/drafts/lec.01c.md#计算机系统)
- [系统调用：read和write](./docs/drafts/lec.01c.md#系统调用read和write)
- [shell](./docs/drafts/lec.01c.md#shell)
- [进程相关系统调用：fork和exec以及wait](./docs/drafts/lec.01c.md#进程相关系统调用fork和exec以及wait)
- [I/O重定向与课程总结](./docs/drafts/lec.01c.md#io重定向与课程总结)
