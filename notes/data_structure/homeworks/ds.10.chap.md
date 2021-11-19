
<!-- @import "[TOC]" {cmd="toc" depthFrom=1 depthTo=6 orderedList=false} -->

<!-- code_chunk_output -->

- [选择题](#选择题)
  - [什么是快速排序算法的最好情况？](#什么是快速排序算法的最好情况)
  - [快速排序是稳定的算法（错）](#快速排序是稳定的算法错)
  - [给定A[]调用非递归的归并排序加表排序执行第1趟后，表元素的结果是](#给定a调用非递归的归并排序加表排序执行第1趟后表元素的结果是)
  - [给定A[]调用表排序后，表元素的结果是](#给定a调用表排序后表元素的结果是)
  - [基数排序是稳定的算法（对）](#基数排序是稳定的算法对)
  - [请选择下面四种排序算法中最快又是稳定的排序算法](#请选择下面四种排序算法中最快又是稳定的排序算法)
  - [下列排序算法中，哪种算法可能出现：在最后一趟开始之前，所有的元素都不在其最终的位置上](#下列排序算法中哪种算法可能出现在最后一趟开始之前所有的元素都不在其最终的位置上)
  - [当待排序列已经基本有序时，下面哪个排序算法效率最差](#当待排序列已经基本有序时下面哪个排序算法效率最差)
  - [数据序列（3,2,4,9,8,11,6,20）只能是下列哪种排序算法的两趟排序结果](#数据序列3249811620只能是下列哪种排序算法的两趟排序结果)
- [讨论题](#讨论题)
  - [快排问题](#快排问题)
  - [从1000个数字中找出最大的10个数字，最快的算法是——](#从1000个数字中找出最大的10个数字最快的算法是)
- [编程题](#编程题)
  - [10-排序4 统计工龄 (20 分)](#10-排序4-统计工龄-20-分)
  - [10-排序5 PAT Judge (25 分)](#10-排序5-pat-judge-25-分)
  - [10-排序6 Sort with Swap(0, i) (25 分)](#10-排序6-sort-with-swap0-i-25-分)

<!-- /code_chunk_output -->

## 选择题

### 什么是快速排序算法的最好情况？

每次正好中分。

因为这样才严格地进行了分而治之，则完美用到了二分的时间复杂度 $Nlog N$ 。

反之，如果每次都选到了边缘，则每次递归处理的序列大小为 $n-1, n-2, ..., 1$ 时间复杂度就成了 $N^2$ 。

### 快速排序是稳定的算法（错）

观察程序显而易见。

### 给定A[]调用非递归的归并排序加表排序执行第1趟后，表元素的结果是

```
A[]={46, 23, 8, 99, 31, 12, 85}

1, 0, 2, 3, 5, 4, 6
```

实际上就是对索引排序，然后所谓`非递归的归并排序执行第1趟`就是`(0,1) (1,2) ...`这些互换位置。

### 给定A[]调用表排序后，表元素的结果是

```
A[]={23, 46, 8, 99, 31, 12, 85}

2, 5, 0, 4, 1, 6, 3
```

### 基数排序是稳定的算法（对）

### 请选择下面四种排序算法中最快又是稳定的排序算法

- A.希尔排序
- B.堆排序
- C.归并排序（对）
- D.快速排序

### 下列排序算法中，哪种算法可能出现：在最后一趟开始之前，所有的元素都不在其最终的位置上

- A.堆排序
- B.插入排序（对）
- C.冒泡排序
- D.快速排序

有可能最后一个元素插到最前面，所有元素后移。

### 当待排序列已经基本有序时，下面哪个排序算法效率最差

- A.快速排序
- B.直接插入
- C.选择排序（对）
- D.堆排序

选择排序咋都得来个 $O(N^2)$ 。

### 数据序列（3,2,4,9,8,11,6,20）只能是下列哪种排序算法的两趟排序结果

- A.冒泡排序（后2个数必须是最大的）
- B.插入排序（前2个数必须是有序的）
- C.选择排序（前2个数必须是最小的）
- D.快速排序（至少`1+2`个元素在正确的位置上）

```
2 3 4 6 8 9 11 20
3 2 4 9 8 11 6 20
    √   √      √
```

## 讨论题

### 快排问题

![](./images/2021111801.png)

这个快排问题在哪？

额外的空间就不是 $O(\log N)$ 了，而是 $O(N\log N)$ 。

### 从1000个数字中找出最大的10个数字，最快的算法是——

A. 归并排序
B. 快速排序
C. 堆排序
D. 选择排序

答案是C。但是这个答案真的对吗？

摘自[曾纪岚](https://www.icourse163.org/learn/ZJU-93001?tid=1465570445#/learn/forumpersonal?uid=1418130817)：感觉优化过的选择排序或许会更好，将选择排序中的maxdate换成一个数组形式，可以用最小堆进行存储。每次把读入数据和堆顶进行比较，然后删除，插入即可。（没太懂）

摘自[mooc41747005296524519](https://www.icourse163.org/learn/ZJU-93001?tid=1465570445#/learn/forumpersonal?uid=1402956424)：如果只有这四个选项，我会选堆排序；但我觉得冒泡排序性能可能会更好

## 编程题

### 10-排序4 统计工龄 (20 分)

给定公司 N 名员工的工龄，要求按工龄增序输出每个工龄段有多少员工。

输入格式:
- 输入首先给出正整 N（≤$10^5$），即员工总人数；随后给出 N 个整数，即每个员工的工龄，范围在 $[0, 50]$ 。

输出格式:
- 按工龄的递增顺序输出每个工龄的员工个数，格式为：“`工龄:人数`”。每项占一行。如果人数为 0 则不输出该项。

输入样例:
```
8
10 2 0 5 7 2 5 2
```

输出样例:
```
0:1
2:3
5:2
7:1
10:1
```

```cpp
#include <iostream>
#include <map>
using namespace std;

int main()
{
    map<int, int> hash;
    int n;
    cin >> n;
    for (int i = 0; i < n; ++ i)
    {
        int a;
        cin >> a;
        hash[a] ++ ;
    }
    for (auto&& item: hash)
        printf("%d:%d\n", item.first, item.second);
}
```

### 10-排序5 PAT Judge (25 分)

参考我的算法笔记：[https://github.com/PiperLiu/ACMOI_Journey](https://github.com/PiperLiu/ACMOI_Journey/blob/master/notes/acwings/PAT%E7%94%B2%E7%BA%A7%E8%BE%85%E5%AF%BC%E8%AF%BE/drafts/pat.4.1.md#pat-%E8%AF%84%E6%B5%8B-1075-pat-judge-25-points)

```cpp
#include <iostream>
#include <algorithm>
#include <unordered_map>
#include <vector>

using namespace std;

const int K = 6;

int n, k, m;
int p_score[K];

struct Student
{
    string id;
    int grade[K];
    int total, cnt;  // 总分 与 完美完成题目数量

    Student(){}
    Student(string _id) : id(_id)
    {
        // 初始化， -2 代表从来没有提交过
        for (int i = 1; i <= k; i ++ ) grade[i] = -2;
        total = cnt = 0;
    }

    void calc()
    {
        for (int i = 1; i <= k; i ++ )
        {
            total += max(0, grade[i]);
            if (grade[i] == p_score[i]) cnt ++ ;
        }
    }

    bool has_submit()
    {
        for (int i = 1; i <= k; i ++ )
            if (grade[i] >= 0)
                return true;
        return false;
    }

    // 重载 < 为数值的 > 用于从大到小排
    bool operator< (const Student& t) const
    {
        if (total != t.total) return total > t.total;
        if (cnt != t.cnt) return cnt > t.cnt;
        return id < t.id;  // 注意 id 是字典序升序
    }
};

int main()
{
    unordered_map<string, Student> students;

    scanf("%d%d%d", &n, &k, &m);
    for (int i = 1; i <= k; i ++ ) scanf("%d", &p_score[i]);

    while (m -- )
    {
        string u_id;
        char u_id_s[6];
        int p_id, grade;
        scanf("%s%d%d", u_id_s, &p_id, &grade);
        u_id = u_id_s;

        if (students.count(u_id) == 0) students[u_id] = Student(u_id);
        students[u_id].grade[p_id] = max(students[u_id].grade[p_id], grade);
    }

    vector<Student> res;
    for (auto& item: students)
    {
        auto& s = item.second;
        if (s.has_submit())
        {
            s.calc();
            res.push_back(s);
        }
    }

    sort(res.begin(), res.end());

    // total 相同则 rank 相同
    for (int i = 0, rank = 1; i < res.size(); i ++ )
    {
        if (i && res[i].total != res[i - 1].total) rank = i + 1;
        printf("%d %s %d", rank, res[i].id.c_str(), res[i].total);
        for (int j = 1; j <= k; j ++ )
            if (res[i].grade[j] == -2) printf(" -");
            else printf(" %d", max(0, res[i].grade[j]));
        puts("");
    }

    return 0;
}
```

### 10-排序6 Sort with Swap(0, i) (25 分)

参考我的算法笔记：[https://github.com/PiperLiu/ACMOI_Journey](https://github.com/PiperLiu/ACMOI_Journey/blob/master/notes/acwings/PAT%E7%94%B2%E7%BA%A7%E8%BE%85%E5%AF%BC%E8%AF%BE/drafts/pat.12.1.md#%E7%94%A8-swap0-i-%E6%93%8D%E4%BD%9C%E8%BF%9B%E8%A1%8C%E6%8E%92%E5%BA%8F-1067-sort-with-swap0-i-25-points)

```cpp
// 转换为图论问题，如图， i 在 p[i] 位置上，则 i 指向 p[i]
#include <iostream>
#include <cstring>

using namespace std;

const int N = 100010;

int n;
int p[N];

int main()
{
    scanf("%d", &n);
    for (int i = 0; i < n; i ++ )
    {
        int id;
        scanf("%d", &id);
        p[id] = i;
    }

    int res = 0;
    for (int i = 1; i < n;)
    {
        while (p[0]) swap(p[0], p[p[0]]), res ++ ;  // p[0] 不指向 0 ，把环上点变为自环
        while (i < n && p[i] == i) i ++ ;  // 找下一个不是自环的环
        if (i < n) swap(p[0], p[i]), res ++ ;  // 让 0 进入该环
    }

    printf("%d\n", res);

    return 0;
}
```
