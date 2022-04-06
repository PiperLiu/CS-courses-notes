# Lab traps: Trap

本节作业：
- https://github.com/PiperLiu/xv6-labs-2020/tree/traps

目录：

<!-- @import "[TOC]" {cmd="toc" depthFrom=2 depthTo=2 orderedList=false} -->

<!-- code_chunk_output -->



<!-- /code_chunk_output -->

细分目录：

<!-- @import "[TOC]" {cmd="toc" depthFrom=2 depthTo=6 orderedList=false} -->

<!-- code_chunk_output -->

- [RISC-V assembly（汇编、RISC-V是小端机）](#risc-v-assembly汇编-risc-v是小端机)
- [Backtrace 报错时回溯打印栈 stack](#backtrace-报错时回溯打印栈-stack)
- [Alarm](#alarm)

<!-- /code_chunk_output -->

### RISC-V assembly（汇编、RISC-V是小端机）

It will be important to understand a bit of RISC-V assembly, which you were exposed to in 6.004. There is a file user/call.c in your xv6 repo. `make fs.img` compiles it and also produces a readable assembly version of the program in user/call.asm.

Read the code in call.asm for the functions g, f, and main. The [instruction manual for RISC-V](https://github.com/riscv/riscv-isa-manual) is on the [reference page](https://pdos.csail.mit.edu/6.S081/2020/reference.html). Here are some questions that you should answer (store the answers in a file answers-traps.txt):

Which registers contain arguments to functions? For example, which register holds 13 in main's call to printf?

> `24: 4635  li a2,13` ， `26: 45b1  li a1,12`

Where is the call to function f in the assembly code for main? Where is the call to g? (Hint: the compiler may inline functions.)

> 并没有显示调用， f 和 g 都被编译器优化直接内联 inline 了

At what address is the function printf located?

> `640 <printf>`

What value is in the register ra just after the jalr to printf in main?

> 对于 `34:	600080e7          	jalr	1536(ra) # 630 <printf>` 。 间接跳转指令JALR用I-type编码类型。指令格式为`JALR rd，offset(rs1)`。这里 jalr 没有第一个操作数，因此，返回地址默认存放到ra寄存器中。`ra=pc+4` 即 `0x38` 。

Run the following code.

```
	unsigned int i = 0x00646c72;
	printf("H%x Wo%s", 57616, &i);
```

What is the output? 

[Here's an ASCII table](https://staffwiki.cs.mun.ca/~michael/c/ascii-table.html) that maps bytes to characters.

> `HE110 World` ，对于十六进制 `0xE110 = 0d57616` ，对于字符串， RISC-V 是小端机， i 从低地址到高地址是 `0x72 0x6c 0x64 0x00` ，对应 ascii 表分别是 `r l d` 。

The output depends on that fact that the RISC-V is little-endian. If the RISC-V were instead big-endian what would you set i to in order to yield the same output? Would you need to change 57616 to a different value?

[Here's a description of little- and big-endian](https://www.webopedia.com/definitions/big-endian/) and [a more whimsical description](http://www.networksorcery.com/enp/ien/ien137.txt).

> 如果 RISC-V 是大端机，那么不需要改变 57616 ，只需要改变 i 的值即可： `0x726c6400` 。

In the following code, what is going to be printed after `'y='`? (note: the answer is not a specific value.) Why does this happen?

```
	printf("x=%d y=%d", 3);
```

> 参考[Moksha](https://zhuanlan.zhihu.com/p/444137341)的分析，阅读call.asm, 将3存放到a1寄存器后，在调用printf之前并未对a2寄存器进行修改(本应该有第二个参数的), 第二个参数传入的值是a2寄存器中原有的随机值。

### Backtrace 报错时回溯打印栈 stack

For debugging it is often useful to have a backtrace: a list of the function calls on the stack above the point at which the error occurred.

Implement a backtrace() function in kernel/printf.c. Insert a call to this function in sys_sleep, and then run bttest, which calls sys_sleep. Your output should be as follows:

```
backtrace:
0x0000000080002cda
0x0000000080002bb6
0x0000000080002898
```

After bttest exit qemu. In your terminal: the addresses may be slightly different but if you run `addr2line -e kernel/kernel` (`or riscv64-unknown-elf-addr2line -e kernel/kernel`) and cut-and-paste the above addresses as follows:

```
    $ addr2line -e kernel/kernel
    0x0000000080002de2
    0x0000000080002f4a
    0x0000000080002bfc
    Ctrl-D
```

You should see something like this:
```
    kernel/sysproc.c:74
    kernel/syscall.c:224
    kernel/trap.c:85
```

The compiler puts in each stack frame a frame pointer that holds the address of the caller's frame pointer. Your backtrace should use these frame pointers to walk up the stack and print the saved return address in each stack frame.

Some hints:

- Add the prototype for backtrace to kernel/defs.h so that you can invoke backtrace in sys_sleep.
- The GCC compiler stores the frame pointer of the currently executing function in the register s0. Add the following function to kernel/riscv.h, and call this function in backtrace to read the current frame pointer. This function uses [in-line assembly](https://gcc.gnu.org/onlinedocs/gcc/Using-Assembly-Language-with-C.html) to read s0.

```c
static inline uint64
r_fp()  // 注意 fp 存在 s0 里，这个函数是 read fp
{
  uint64 x;
  asm volatile("mv %0, s0" : "=r" (x) );
  return x;
}
```

- These [lecture notes](../lec/l-riscv-slides.pdf) have a picture of the layout of stack frames. Note that the return address lives at a fixed offset (-8) from the frame pointer of a stackframe, and that the saved frame pointer lives at fixed offset (-16) from the frame pointer. 如下图， fp 为当前函数的栈顶指针， sp 为栈指针， fp-8 存放返回地址， fp-16 存放原栈帧（调用函数的 fp）。

![](./images/2022032202.png)

- Xv6 allocates one page for each stack in the xv6 kernel at PAGE-aligned address. You can compute the top and bottom address of the stack page by using PGROUNDDOWN(fp) and PGROUNDUP(fp) (see kernel/riscv.h. These number are helpful for backtrace to terminate its loop. 注意，在 xv6 中，每个栈都被分配了一个页，因此这个栈的栈底就是 PGROUNDUP(fp) 。
- Once your backtrace is working, call it from panic in kernel/printf.c so that you see the kernel's backtrace when it panics.

按照 hints 就好：

```c
kernel/defs.h
// printf.c
void            backtrace(void);

kernel/riscv.h 加入 r_fp

实现一个 backtrace
// printf.c
void
backtrace(void)
{
  uint64 fp = r_fp();
  uint64 stk_bottom = PGROUNDUP(fp);
  printf("backtrace:\n");
  // 下面一定是 fp < stk_bottom 如果是 fp <= stk_bottom 会有 panic: kerneltrap
  for (; fp < stk_bottom; fp = *((uint64*)(fp - 16)))  // 取地址的值跳转
    printf("%p\n", *((uint64*)(fp - 8)));  // 取返回地址上的值
}

在 sys_sleep 和 panic 添加 backtrace 调用
```

### Alarm

