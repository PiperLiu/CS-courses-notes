# MIT6.824 实验准备

目录：

<!-- @import "[TOC]" {cmd="toc" depthFrom=2 depthTo=6 orderedList=false} -->

<!-- code_chunk_output -->

- [Linux 环境](#linux-环境)
- [根据 Lab 1 文档安装 Go 1.13](#根据-lab-1-文档安装-go-113)
- [下载并编译 Lab](#下载并编译-lab)

<!-- /code_chunk_output -->

## Linux 环境

具体来讲是 WSL2 (Windows Subsystem for Linux 2) ：

```
$ cat /proc/version
Linux version 4.19.104-microsoft-standard (oe-user@oe-host) (gcc version 8.2.0 (GCC)) #1 SMP Wed Feb 19 06:37:35 UTC 2020

$ uname -a
Linux DESKTOP-AIHBG7R 4.19.104-microsoft-standard #1 SMP Wed Feb 19 06:37:35 UTC 2020 x86_64 x86_64 x86_64 GNU/Linux
```

是 64-bit kernel ，没问题。

## 根据 Lab 1 文档安装 Go 1.13

```bash
$ echo $PATH | grep -c "/usr/local/bin"
1
# 说明 $PATH 中有 /usr/local/bin

$ wget -qO- https://dl.google.com/go/go1.13.6.linux-amd64.tar.gz | sudo tar xz -C /usr/local
```

把 `go/bin` 添加到 `$PATH` 中。我这里修改的是 `zshrc` 。

```bash
$ go version
go version go1.13.6 linux/amd64
```

## 下载并编译 Lab

```bash
git clone git://g.csail.mit.edu/6.824-golabs-2020 6.824
```

如果成功下载就大功告成了，因为你只需要一个 linux 和一个 go 。

```bash
$ cd ~/6.824
$ cd src/main
$ go build -buildmode=plugin ../mrapps/wc.go
$ rm mr-out*
$ go run mrsequential.go wc.so pg*.txt
$ more mr-out-0
A 509
ABOUT 2
ACT 8
...
```

我没有把仓库发布为 public （ **belows are private** ）：
- https://github.com/PiperLiu/mit6.824-lab-2020
- https://gitee.com/piperliu/mit6.824-lab-2020
