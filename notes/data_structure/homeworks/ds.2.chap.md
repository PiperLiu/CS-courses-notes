
<!-- @import "[TOC]" {cmd="toc" depthFrom=1 depthTo=6 orderedList=false} -->

<!-- code_chunk_output -->

- [选择题](#选择题)
  - [堆栈将中缀表达式A-(B-C/D)*E转为后缀表达式，则该堆栈的大小](#堆栈将中缀表达式a-b-cde转为后缀表达式则该堆栈的大小)
  - [链表存堆栈](#链表存堆栈)
- [讨论题](#讨论题)
  - [如何用两个堆栈模拟实现一个队列?](#如何用两个堆栈模拟实现一个队列)
- [编程题](#编程题)
  - [02-线性结构1 两个有序链表序列的合并 (15 分)](#02-线性结构1-两个有序链表序列的合并-15-分)
  - [02-线性结构2 一元多项式的乘法与加法运算 (20 分)](#02-线性结构2-一元多项式的乘法与加法运算-20-分)
  - [02-线性结构3 Reversing Linked List (25 分)](#02-线性结构3-reversing-linked-list-25-分)
  - [02-线性结构4 Pop Sequence (25 分)](#02-线性结构4-pop-sequence-25-分)

<!-- /code_chunk_output -->


## 选择题

### 堆栈将中缀表达式A-(B-C/D)*E转为后缀表达式，则该堆栈的大小

借助堆栈将中缀表达式A-(B-C/D)*E转换为后缀表达式，则该堆栈的大小至少为：$4$

参考：[中缀表达式转后缀](../../../DataStructure_数据结构/course_2/5_中缀表达式转后缀.md)

以下是堆栈快照：
```
stack       post
            A
-           A
-(          A
-(          AB
-(-         AB
-(-         ABC
-(-/        ABCD
-           ABCD/-
-*          ABCD/-
-*          ABCD/-E
            ABCD/-E*-
```

### 链表存堆栈

若用单向链表实现一个堆栈，当前链表状态为：1->2->3。当对该堆栈执行`pop()`、`push(4)`操作后，链表状态变成怎样？
- (1)  4->2->3 对
- (2)  1->2->4 错
- **只能用链表头来表示栈顶：** 用表头做栈顶的话，入栈和出栈的时间复杂度都是$O(1)$，若表尾，每次入栈和出栈都要遍历链表，时间复杂度为$O(n)$

## 讨论题

### 如何用两个堆栈模拟实现一个队列?

如何用两个堆栈模拟实现一个队列？如果这两个堆栈的容量分别是`m`和`n` `(m>n)`，你的方法能保证的队列容量是多少？

参考[mooc73291796535938949](https://www.icourse163.org/learn/ZJU-93001?tid=1465570445#/learn/forumpersonal?uid=1450061257)、[SteveWang](https://www.cnblogs.com/eniac12/p/4865158.html)：

队列的容量为`2n+1`，首先入栈时向`m`进行入栈操作，当入栈操作结束时或`m`中数据个数为`n`时，`m`进行出栈，将`m`出栈的数据入栈到`n`，此时`n`进行出栈操作即可模拟队列的出列，若`n`栈满时，继续向`m`中进行入栈操作，可向`m`中入栈`n+1`个数据，进行出列操作即`n`进行出栈操作，当`n`为空时将`m`中的数据出栈后入栈到`n`，`n`再进行出栈操作。

## 编程题

### 02-线性结构1 两个有序链表序列的合并 (15 分)

```c
#include <stdio.h>
#include <stdlib.h>

typedef int ElementType;
typedef struct Node *PtrToNode;
struct Node {
    ElementType Data;
    PtrToNode   Next;
};
typedef PtrToNode List;

List Read(); /* 细节在此不表 */
void Print( List L ); /* 细节在此不表；空链表将输出NULL */

List Merge( List L1, List L2 );

int main()
{
    List L1, L2, L;
    L1 = Read();
    L2 = Read();
    L = Merge(L1, L2);
    Print(L);
    Print(L1);
    Print(L2);
    return 0;
}

/* 你的代码将被嵌在这里 */
```

做一个 `Merge()` 。

```c
List Merge(List L1, List L2)
{
    List p = (List) malloc(sizeof(List));  // 不可以 List p;  否则空指针，段错误
    List p1 = L1->Next;
    List p2 = L2->Next;
    List head = p;
    while (p1 && p2)
    {
        if (p1->Data <= p2->Data)
        {
            p->Next = p1;
            p1 = p1->Next;
            p = p->Next;
        }
        else
        {
            p->Next = p2;
            p2 = p2->Next;
            p = p->Next;
        }
    }
    
    while (p1)
    {
        p->Next = p1;
        p1 = p1->Next;
        p = p->Next;
    }
    
    while (p2)
    {
        p->Next = p2;
        p2 = p2->Next;
        p = p->Next;
    }
    
    p->Next = NULL;
    // 操作头指针，为了符合题意
    L1->Next = NULL;
    L2->Next = NULL;

    return head;
}
```

### 02-线性结构2 一元多项式的乘法与加法运算 (20 分)

```cpp
/*
输入：
4 3 4 -5 2  6 1  -2 0
3 5 20  -7 4  3 1

输出：
15 24 -25 22 30 21 -10 20 -21 8 35 6 -33 5 14 4 -15 3 18 2 -6 1
5 20 -4 4 -5 2 9 1 -2 0

注意：
- 0 0 不输出，除非没有别的项
- 按照指数降序输出
*/

// 用哈希表做，其实就是用链表做
#include<iostream>
#include<map>
using namespace std;

int main()
{
    map<int, int> hash1, hash2;
    
    int n, a, b;
    cin >> n;
    for (int i = 0; i < n; ++ i)
    {
        cin >> a >> b;
        hash1[b] = a;
    }
    cin >> n;
    for (int i = 0; i < n; ++ i)
    {
        cin >> a >> b;
        hash2[b] = a;
    }
 
    // 负数保存，为了降序输出
    map<int, int> ans;
    ans.clear();
    for (auto&& t1: hash1)
        for (auto&& t2: hash2)
            ans[- t1.first - t2.first] += t1.second * t2.second;

    bool flag = false;
    for (auto&& t: ans)
    {
        if (t.second == 0) continue;  // 这是个陷阱
        if (!flag) cout << t.second << " " << - t.first;
        else cout << " " << t.second << " " << - t.first;
        flag = true;
    }
    if (!flag) cout << "0 0";
    cout << endl;
    
    ans.clear();
    for (auto&& t: hash1) ans[- t.first] += t.second;
    for (auto&& t: hash2) ans[- t.first] += t.second;
    
    flag = false;
    for (auto&& t: ans)
    {
        if (t.second == 0) continue;  // 这是个陷阱
        if (!flag) cout << t.second << " " << - t.first;
        else cout << " " << t.second << " " << - t.first;
        flag = true;
    }
    if (!flag) cout << "0 0";
    cout << endl;
}
```

### 02-线性结构3 Reversing Linked List (25 分)

PAT 里做过：https://github.com/PiperLiu/ACMOI_Journey/blob/master/notes/acwings/PAT%E7%94%B2%E7%BA%A7%E8%BE%85%E5%AF%BC%E8%AF%BE/drafts/pat.13.1.md#%E5%8F%8D%E8%BD%AC%E9%93%BE%E8%A1%A8-1074-reversing-linked-list-25-points

每 k 位翻转一下。用 `vector` 存节点地址就行。

```cpp
#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;

const int N = 1e5 + 10;
int e[N], ne[N];
vector<int> q;

int main()
{
    int h, n, k;
    scanf("%d %d %d", &h, &n, &k);
    
    int address, value, next;
    for (int i = 0; i < n; ++ i)
    {
        scanf("%d %d %d", &address, &value, &next);
        e[address] = value, ne[address] = next;
    }
    
    // 地址代表各个节点，推入 q
    for (int i = h; i != -1; i = ne[i]) q.push_back(i);
    
    for (int i = 0; i < q.size() - k + 1; i += k)  // 别用 i < n - k + 1 因为有些节点可能没在链表上
        reverse(q.begin() + i, q.begin() + i + k);
    
    for (int i = 0; i < q.size(); ++ i)
    {
        printf("%05d %d ", q[i], e[q[i]]);
        if (i  + 1 == q.size()) printf("-1\n");
        else printf("%05d\n", q[i + 1]);
    }
}
```

### 02-线性结构4 Pop Sequence (25 分)

PAT 里做过：https://github.com/PiperLiu/ACMOI_Journey/blob/master/notes/acwings/PAT%E7%94%B2%E7%BA%A7%E8%BE%85%E5%AF%BC%E8%AF%BE/drafts/pat.14.2.md#%E5%BC%B9%E5%87%BA%E5%BA%8F%E5%88%97-1051-pop-sequence-25-points

```cpp
// 模拟弹出过程
#include <stack>
#include <iostream>

using namespace std;

const int N = 1010;
int a[N];
int m, n, k;

bool check()
{
    stack<int> stk;
    
    for (int i = 1, j = 0; i <= n; ++ i)
    {
        stk.push(i);
        if (stk.size() > m) return false;
        
        while (stk.size() && stk.top() == a[j])
        {
            j ++ ;
            stk.pop();
        }
    }
    
    return stk.empty();
}

int main()
{
    scanf("%d %d %d", &m, &n, &k);
    while (k --)
    {
        for (int i = 0; i < n; ++ i) scanf("%d", &a[i]);
        if (check()) printf("YES\n");
        else printf("NO\n");
    }
}
```
