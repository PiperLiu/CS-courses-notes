
<!-- @import "[TOC]" {cmd="toc" depthFrom=1 depthTo=6 orderedList=false} -->

<!-- code_chunk_output -->

- [讨论题](#讨论题)
  - [无权图的单源最短路输出](#无权图的单源最短路输出)
  - [Floyd算法与负值圈](#floyd算法与负值圈)
- [编程题](#编程题)
  - [](#)

<!-- /code_chunk_output -->


## 讨论题

### 无权图的单源最短路输出

利用堆栈，先按照倒叙查找push进去，再按照正序pop出来。

### Floyd算法与负值圈

如果图中有负值圈，Floyd算法还能用吗？如何知道图中是否存在负值圈？

负权可以，负权圈不行。

判断负权圈可以用 SPFA ，参考我的算法笔记：
- https://github.com/piperliu/acmoi_journey/blob/master/notes/acwings/算法基础课/ybase08.md#spfa判断负环模板

- 每次更新，都要做两个操作：`dist[x] = dist[t] + w[i]`，以及 `cnt[x] = cnt[t] + 1`
- 如果某次 `cnt[x] >= n` ，说明从 `1` 到 `x` 至少经过了 `n` 条边，则最短路需要 `n+1` 个以上的点，则肯定存在负环

## 编程题

### 