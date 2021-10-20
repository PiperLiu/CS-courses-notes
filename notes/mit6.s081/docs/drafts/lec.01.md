# LEC 1 (rtm): Introduction and examples

目录：

<!-- @import "[TOC]" {cmd="toc" depthFrom=2 depthTo=2 orderedList=false} -->

<!-- code_chunk_output -->

- [课前预习：阅读教材第一章](#课前预习阅读教材第一章)
- [作业](#作业)

<!-- /code_chunk_output -->

细分目录：

<!-- @import "[TOC]" {cmd="toc" depthFrom=2 depthTo=6 orderedList=false} -->

<!-- code_chunk_output -->

- [课前预习：阅读教材第一章](#课前预习阅读教材第一章)
  - [Foreword and acknowledgments](#foreword-and-acknowledgments)
  - [Chapter 1 Operating system interfaces](#chapter-1-operating-system-interfaces)
    - [1.1 Processes and memory](#11-processes-and-memory)
      - [system call: fork](#system-call-fork)
      - [system call: exec](#system-call-exec)
      - [main structure of shell](#main-structure-of-shell)
    - [1.2 I/O and File descriptors](#12-io-and-file-descriptors)
      - [文件描述符 file descriptor](#文件描述符-file-descriptor)
      - [文件描述符补充知识](#文件描述符补充知识)
      - [open和write](#open和write)
      - [close和文件重定向](#close和文件重定向)
      - [dup和文件重定向](#dup和文件重定向)
    - [1.3 Pipes](#13-pipes)
    - [1.4 File system](#14-file-system)
    - [1.5 Real world](#15-real-world)
    - [1.6 Exercises](#16-exercises)
- [作业](#作业)

<!-- /code_chunk_output -->

## 课前预习：阅读教材第一章

教材：[../lec/book-riscv-rev1.pdf](../lec/book-riscv-rev1.pdf)

### Foreword and acknowledgments

xv6 基于 Unix 之父的 Unix Version 6 （最经典的操作系统），并且，是基于 `ANSI C for a multi-core RISC-V` 实现的。

### Chapter 1 Operating system interfaces

什么是操作系统？

The job of an operating system is to share a computer among multiple programs and to provide a more useful set of services than the hardware alone supports. An operating system manages and abstracts the low-level hardware, so that, for example, a word processor need not concern itself with which type of disk hardware is being used. An operating system shares the hardware among multiple programs so that they run (or appear to run) at the same time. Finally, operating systems provide controlled ways for programs to interact, so that they can share data or work together.

我概括一下：
- 封装硬件，给应用程序提供运行环境
- 调配应用程序间资源

接着这里阐明了为什么用 xv6 系统。系统精简巧妙，其基于的 Unix v6 是最经典的操作系统，是现代操作系统的鼻祖：
- 接口单一，但是泛化性很强（功能强）
- 设计思维影响了几乎所有现代操作系统

![](./images/2021101101.png)

如上，按我的理解， `cat` 应该也和 `shell` 一样，也是一个 `process` 。上图中， `shell` 发起了一个系统调用 `system call` 。

![](./images/2021101102.png)

Xv6 实现了 Unix v6 的系统调用的子集。

#### 1.1 Processes and memory

进程 process 由以下部分组成：
- 用户态内存 user-space memory ： instructions, data, stack
- 在内核状态 per-process state private to the kernel

Xv6 管理进程：在后台给进程切换可用 CPU ，进程挂起（保存 its CPU registers, restoring them when it next runs the process），分配 PID 等。

##### system call: fork

一个进程创建新的 process ，可以用系统调用 `fork` 。

`fork` 父子进程的内存相同，父进程返回子进程 PID ，子进程返回 0 。

```c
int pid = fork();
if (pid > 0) {
    printf("parent: child=%d\n", pid);
    pid = wait((int *) 0);
    printf("child %d is done\n", pid);
} else if (pid == 0) {
    printf("child: exiting\n");
    exit(0);
} else {
    printf("fork error\n");
}
```

如上程序，可能输出：
```
parent: child=1234
child: exiting
parent: child 1234 is done
```

解释一下重点：
- `wait` 返回当前进程子进程的 `PID` ，并且传入一个指针，这个指针将被修改为指向 `child exit status`
- 父进程 `wait` 后，开始等待子进程结束，而子进程复制了父进程的内容，因此会把程序再执行一遍，只不过因为其子进程身份，`fork()` 返回的是 `0`
- 所以，如果我们不在乎 `child exit status` 可以传入一个无所谓的 0 指针（`(int *) 0`）
- 初始化时，父子进程的 `memory` 与 `registers` 相同，但之后改变其中一个的变量并不影响另一个的变量

##### system call: exec

`exec` 系统调用会把 calling process 的内存用新内存快照替换掉，这个内存快照会遵守特定的格式：
- ELF 格式（第三章将会详细讨论）
- 包括数据区、指令从哪里开始

`exec` 成功执行后，并不返回调用它的程序（ calling process ），而是到 ELF header 那里。

`exec` 有两个参数：
- 可执行文件名
- 参数（`an array of string`）

```c
char *argv[3];

argv[0] = "echo";
argv[1] = "hello";
argv[2] = 0;
exec("/bin/echo", argv);
printf("exec error\n");
```

我们的程序在、是 `/bin/echo` 文件；大部分程序会无视参数数组的第一个，因为那个往往是程序名称。

##### main structure of shell

`shell` 的 `main` 是一个 `loop` ，用 `getcmd` 读输入，然后调用 `fork` （ `creates a copy of the shell process` ）。

参考：[https://github.com/mit-pdos/xv6-riscv/blob/riscv//user/sh.c#L145](https://github.com/mit-pdos/xv6-riscv/blob/riscv//user/sh.c#L145)

```c
main(void)
{
  static char buf[100];
  int fd;

  // Ensure that three file descriptors are open.
  while((fd = open("console", O_RDWR)) >= 0){
    if(fd >= 3){
      close(fd);
      break;
    }
  }

  // Read and run input commands.
  while(getcmd(buf, sizeof(buf)) >= 0){
    if(buf[0] == 'c' && buf[1] == 'd' && buf[2] == ' '){
      // Chdir must be called by the parent, not the child.
      buf[strlen(buf)-1] = 0;  // chop \n
      if(chdir(buf+3) < 0)
        fprintf(2, "cannot cd %s\n", buf+3);
      continue;
    }
    if(fork1() == 0)
      runcmd(parsecmd(buf));
    wait(0);  // runcmd 执行完毕则 return from wait
  }
  exit(0);
}
```

原来只有 `cd` 指令不是个进程，牛逼。

Xv6 隐式地分配大部分用户态内存：
- `fork` 为子进程分配父进程的内存拷贝
- `exec` 为可执行文件分配足够内存
- 诸如 `malloc` 这种在运行时需要更多内存的进程，可以调用 `sbrk(n)` 来增加数据内存， `n` 表示 `n` 字节， `sbrk` 返回新内存的位置

#### 1.2 I/O and File descriptors

##### 文件描述符 file descriptor

文件描述符是一个小的整数（`small integer`），代表一个内核对象（`kernel-managed object`）。进程会去读或写这个内核对象。

为了方便有简称：
- `file` 就代表 `file descriptor`
- `input and output` 用 `I/O` 表示

文件描述符提供了抽象的接口，使得 `files` 、 `pipes` 、 `devices` 都看起来一样，成了字节流（`streams of bytes`）。

##### 文件描述符补充知识

这里参考了：[百度百科](https://baike.baidu.com/item/%E6%96%87%E4%BB%B6%E6%8F%8F%E8%BF%B0%E7%AC%A6)、[文件描述符（0、1、2）的用法](https://blog.csdn.net/baozhourui/article/details/88265557)、[文件描述符与socket连接](https://www.cnblogs.com/DengGao/p/file_symbol.html)。

每一个文件描述符会与一个打开文件相对应，同时，不同的文件描述符也会指向同一个文件。相同的文件可以被不同的进程打开也可以在同一个进程中被多次打开。系统为每一个进程维护了一个文件描述符表，该表的值都是从0开始的，所以在不同的进程中你会看到相同的文件描述符，这种情况下相同文件描述符有可能指向同一个文件，也有可能指向不同的文件。

![](./images/2021102001.png)

在进程A中，文件描述符`1`和`30`都指向了同一个打开的文件句柄（标号`23`）。这可能是通过调用`dup()`、`dup2()`、`fcntl()`或者对同一个文件多次调用了`open()`函数而形成的。

进程A的文件描述符`2`和进程B的文件描述符`2`都指向了同一个打开的文件句柄（标号73）。这种情形可能是在调用`fork()`后出现的（即，进程`A`、`B`是父子进程关系），或者当某进程通过`UNIX`域套接字将一个打开的文件描述符传递给另一个进程时，也会发生。再者是不同的进程独自去调用`open`函数打开了同一个文件，此时进程内部的描述符正好分配到与其他进程打开该文件的描述符一样。

此外，进程`A`的描述符`0`和进程`B`的描述符`3`分别指向不同的打开文件句柄，但这些句柄均指向`i-node`表的相同条目（`1976`），换言之，指向同一个文件。发生这种情况是因为每个进程各自对同一个文件发起了`open()`调用。同一个进程两次打开同一个文件，也会发生类似情况。

上图对于 `fd 0` 的例子可能并不标准，因为 **还有3个特殊的文件描述符。**
- `0` 代表标准输入 `standard input`
- `1` 代表标准输出 `standard output`
- `2` 代表标准错误 `standard error`

比如， `0<`  表示标准输入， `1>` 就会输出标准输出，而 `2>` 就会输出表征错误，我们随便操作一下 Linux 系统就会看出：

![](./images/2021102002.png)

在 `shell` 的 `main` 里，也会有保证这三个文件描述符被打开了（[user/sh.c:151](https://github.com/mit-pdos/xv6-riscv/blob/riscv//user/sh.c#L151)）：

```c
int
main(void)
{
  static char buf[100];
  int fd;

  // Ensure that three file descriptors are open.
  while((fd = open("console", O_RDWR)) >= 0){
    if(fd >= 3){
      close(fd);
      break;
    }
  }
  ...
}
```

##### open和write

涉及到一些系统调用：
- `open(fd, buf, n)` ，把 `n` 个字节的内容复制到 `buf` ，并且返回其读的字节数
  - 如果没有可读的，就返回 `0`
  - 每个 `fd` 都有一个偏移量， `read` 会在偏移量基础上读
- `write(fd, buf, n)` ， 从 `buf` 复制 `n` 个字节到 `fd` ，也根据并控制偏移量

看一个例子，即简易版 `cat` 命令：

```c
char buf[512];
int n;

for (;;)
{
  n = read(0, buf, sizeof buf);  // 读标准输入
  if (n == 0)
    break;
  if (n < 0)
  {
    fprintf(2, "read error\n");
    exit(1);
  }
  if (write(1, buf, n) != n)  // 写给标准输出
  {
    fprintf(2, "write error\n");
    exit(1);
  }
}
```

**使用标准输入和标准输出，`cat` 就用不着区分它是从文件、控制台还是管道来读的程序了。**

##### close和文件重定向

`close` 则释放一个 `fd` ，这个 `fd` 则可以被以后的 `open` 重新使用。注意 `open` **总是会打开最小的没被占用的** `fd` ，应用这点我们来看 `cat < input.txt` ：

```c
char *argv[2];

argv[0] = "cat";
argv[1] = 0;
if (fork() == 0)
{
  close(0);
  open("input.txt", O_RDONLY);
  exec("cat", argv);
}
```

这里子进程 `close` 把标准输入关了，又因为此时空闲的最小的文件描述符一定是 `0` 了，则 `open` 必会把 `"input.txt` 作为标准输入。此外，父进程的文件描述符不会受影响，甚妙！

##### dup和文件重定向

但是，如果：

```c
if (fork() == 0)
{
  write(1, "hello ", 6);
  exit(0);
} else {
  wait(0);
  write(1, "world\n", 6);
}
```

则输出 `hello world` ，对于 `write` 其改动的 `fd` 的偏移量对于父进程是全局的，无论是否在子进程中调用。

此外，也可以使用 `dup` 一个打开的文件号（`fd`），使两个文件号都指向同一个文件。**且其文件偏移量是共享的。**

因此下面这段程序输出的也是 `hello world` ：
```c
fd = dup(1);
write(1, "hello ", 6);
write(fd, "world\n", 6);
```

`dup` 有什么用？请见这条 `sh` ：
```sh
ls existing-file non-existing-file > tmp1 2>&1
```

`&1` 就是 `tmp1` ，如此，我们在内核中实现时就做了一次 `dup(tmp1)` ，而 `existing-file` 的名字和 `non-existing-file` 的错误信息都将出现在 `tmp1` 文件中。

最后，书中再次强调了：文件描述符这种抽象的牛逼之处，万物皆文件得到很好的体现。

> File descriptors are a powerful abstraction, because they hide the details of what they are connected to: a process writing to file descriptor 1 may be writing to a file, to a device like the console, or to a pipe.

#### 1.3 Pipes

#### 1.4 File system

#### 1.5 Real world

#### 1.6 Exercises

## 作业
