# 数据结构笔记


<!-- @import "[TOC]" {cmd="toc" depthFrom=2 depthTo=6 orderedList=false} -->

<!-- code_chunk_output -->

- [2021 MOOC浙江大学数据结构](#2021-mooc浙江大学数据结构)
  - [第一讲 基本概念](#第一讲-基本概念)
  - [第二讲 线性结构](#第二讲-线性结构)
  - [第三讲 树（上）](#第三讲-树上)
  - [第四讲 树（中）](#第四讲-树中)
  - [第五讲 树（下）](#第五讲-树下)
  - [第六讲 图（上）](#第六讲-图上)

<!-- /code_chunk_output -->

## 2021 MOOC浙江大学数据结构

说实在的，我都研二了，要是数据结构还不过关，就别混了。主要是准备12月份再考一次 PAT （第一次84分🤬），因此把 MOOC 刷一次，搞个 50 块钱代金券。

课程幻灯片PPT/讲义PDF都放在[./resources文件夹](./resources)里了。

### 第一讲 基本概念

我以前的笔记记得就不错，见：
- [数据组织与算法效率](../../DataStructure_数据结构/course_1/1_数据组织与算法效率.md)
- [什么是算法](../../DataStructure_数据结构/course_1/2_什么是算法.md)
- [算法实例：最大子列和](../../DataStructure_数据结构/course_1/3_算法实例：最大子列和.md)

习题与作业：
- [./homeworks/ds.1.chap.md](./homeworks/ds.1.chap.md)

代码：
- [分治法](./codes/1.1.算法3-分治法.c)

### 第二讲 线性结构

我以前的笔记记得就不错，见：
- [线性表及其表示](../../DataStructure_数据结构/course_2/1_线性表及其表示.md)
- [堆栈](../../DataStructure_数据结构/course_2/2_堆栈.md)
- [队列](../../DataStructure_数据结构/course_2/3_队列.md)
- [不带头结点链表实现多项式相加与相乘](../../DataStructure_数据结构/course_2/4_不带头结点链表实现多项式相加与相乘.md)

补充一下：
- [中缀表达式转后缀](../../DataStructure_数据结构/course_2/5_中缀表达式转后缀.md)

代码：
- [线性表的定义与操作-顺序表](./codes/2.1.线性表的定义与操作-顺序表.c)：就是在一个 `struct` 里存了一个数组，包括其最后一位的索引
- [线性表的定义与操作-链式表](./codes/2.1.线性表的定义与操作-链式表.c)：就是在一个 `struct` 里存了一个数组，包括其最后一位的索引
- 注意：**List 是 `struct*`，因此要 `malloc` 后再调用其中成员**
- [堆栈的定义与操作-顺序存储.c](./codes/2.2.堆栈的定义与操作-顺序存储.c)
- [堆栈的定义与操作-链式存储.c](./codes/2.2.堆栈的定义与操作-链式存储.c)
- [队列的定义与操作-顺序存储.c](./codes/2.3.队列的定义与操作-顺序存储.c)
- [队列的定义与操作-链式存储.c](./codes/2.3.队列的定义与操作-链式存储.c)

习题与作业：
- [./homeworks/ds.2.chap.md](./homeworks/ds.2.chap.md)

### 第三讲 树（上）

以前的笔记：
- [树与树的表示](../../DataStructure_数据结构/course_3/1_树与树的表示.md)

代码：
- [二叉树的链表结构.c](./codes/3.1.二叉树的链表结构.c)
- [二叉树的四种遍历.c](./codes/3.3.二叉树的四种遍历.c)

习题与作业：
- [./homeworks/ds.3.chap.md](./homeworks/ds.3.chap.md)

### 第四讲 树（中）

现在的笔记：
- [二叉搜索树拾遗](../../DataStructure_数据结构/course_4/1_二叉搜索树拾遗.md)
- [平衡二叉树拾遗](../../DataStructure_数据结构/course_4/2_平衡二叉树拾遗.md)

代码：
- [二叉搜索树的插入与删除](./codes/4.1.二叉搜索树的插入与删除)
- [AVL树的旋转与插入.c](./codes/4.2.AVL树的旋转与插入.c)

习题与作业：
- [./homeworks/ds.4.chap.md](./homeworks/ds.4.chap.md)

### 第五讲 树（下）

本节讲堆，可以参考我算法基础课的笔记：https://github.com/PiperLiu/ACMOI_Journey/blob/master/notes/acwings/算法基础课/ybase05.md#堆

补充笔记：
- [哈夫曼树与哈夫曼编码](../../DataStructure_数据结构/course_5/1_哈夫曼树与哈夫曼编码.md)
- [集合（并查集）](../../DataStructure_数据结构/course_5/2_集合（并查集）.md)

代码：
- [堆的定义与操作](./codes/5.1.堆的定义与操作.c)
- [集合的定义与并查操作](./codes/5.3.集合的定义与并查操作.c)

习题与作业：
- [./homeworks/ds.5.chap.md](./homeworks/ds.5.chap.md)

### 第六讲 图（上）

代码：
- [图的建立-邻接矩阵表示](./codes/6.1.图的建立-邻接矩阵表示.c)
- [图的建立-邻接表表示](./codes/6.1.图的建立-邻接表表示.c)
- [DFS-邻接表存储](./codes/6.2.DFS-邻接表存储.c)
- [BFS-邻接矩阵存储](./codes/6.2.BFS-邻接矩阵存储.c)

习题与作业：
- [./homeworks/ds.6.chap.md](./homeworks/ds.6.chap.md)
