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
