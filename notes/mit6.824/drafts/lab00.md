# MIT6.824 实验准备

目录：

<!-- @import "[TOC]" {cmd="toc" depthFrom=2 depthTo=6 orderedList=false} -->

<!-- code_chunk_output -->

- [Linux 环境](#linux-环境)
- [根据 Lab 1 文档安装 Go 1.13](#根据-lab-1-文档安装-go-113)
- [下载并编译 Lab](#下载并编译-lab)
- [go 1.13 with gopls@v0.8.0](#go-113-with-goplsv080)

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

（似乎没有适配 Mac Silicon ARM64 的 1.13 go 版本，于是就用 docker 开发了）
```bash
docker pull amd64/ubuntu:22.04
docker run -it --name mit6.824lab -v -d /Users/piperliu/Github/:/root/dev amd64/ubuntu:22.04 bash

在 VS Code 中 Attach
apt-get update
apt-get install -y sudo git wget gcc
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

头一次知道 go 也能编译出 .so 文件。

我没有把仓库发布为 public （ **belows are private** ）：
- https://github.com/PiperLiu/mit6.824-lab-2020
- https://gitee.com/piperliu/mit6.824-lab-2020

## go 1.13 with gopls@v0.8.0

需要安装特定版本的 gopls 来适配 go 1.13 。

```bash
GO111MODULE=on GOPROXY=https://goproxy.io go get golang.org/x/tools/gopls@v0.8.0
GO111MODULE=on GOPROXY=https://goproxy.io go get honnef.co/go/tools/cmd/staticcheck@2019.2.3
GO111MODULE=on GOPROXY=https://goproxy.io go get github.com/go-delve/delve/cmd/dlv@v1.6.0
```

如上是我在 GitHub 的 release 页面中找到的支持 1.13 版本的 工具。
