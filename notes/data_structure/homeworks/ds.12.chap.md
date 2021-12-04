
<!-- @import "[TOC]" {cmd="toc" depthFrom=1 depthTo=6 orderedList=false} -->

<!-- code_chunk_output -->

- [选择题](#选择题)
  - [这段程序打印出来什么东西？](#这段程序打印出来什么东西)
- [编程题](#编程题)
  - [KMP 串的模式匹配 (25 分)](#kmp-串的模式匹配-25-分)

<!-- /code_chunk_output -->

## 选择题

### 这段程序打印出来什么东西？

```cpp
#include <stdio.h>
#include <string.h>
typedef char* Position;
int main()
{   char string[] = "This is a simple example.";
    char pattern[] = "simple";
    Position p = strstr(string, pattern);
    printf("%s\n", p);
    return 0;
}
```

是 `simple example.` ，注意这里 `Position` 是指针。

## 编程题

### KMP 串的模式匹配 (25 分)

给定两个由英文字母组成的字符串 String 和 Pattern，要求找到 Pattern 在 String 中第一次出现的位置，并将此位置后的 String 的子串输出。如果找不到，则输出“`Not Found`”。

本题旨在测试各种不同的匹配算法在各种数据情况下的表现。各组测试数据特点如下：
- 数据0：小规模字符串，测试基本正确性；
- 数据1：随机数据，String 长度为 $10^5$，Pattern 长度为 $10$；
- 数据2：随机数据，String 长度为 $10^5$，Pattern 长度为 $10^2$；
- 数据3：随机数据，String 长度为 $10^5$，Pattern 长度为 $10^3$；
- 数据4：随机数据，String 长度为 $10^5$，Pattern 长度为 $10^4$；
- 数据5：随机数据，String 长度为 $10^6$，Pattern 长度为 $10^5$；测试尾字符不匹配的情形；
- 数据6：随机数据，String 长度为 $10^6$，Pattern 长度为 $10^5$；测试首字符不匹配的情形。

输入格式:
- 输入第一行给出 String，为由英文字母组成的、长度不超过 $10^6$ 的字符串。第二行给出一个正整数 $N（≤10）$，为待匹配的模式串的个数。随后 $N$ 行，每行给出一个 Pattern，为由英文字母组成的、长度不超过 $10^5$ 的字符串。每个字符串都非空，以回车结束。

输出格式:
- 对每个 Pattern，按照题面要求输出匹配结果。

输入样例:
```
abcabcabcabcacabxy
3
abcabcacab
cabcabcd
abcabcabcabcacabxyz
```

输出样例:
```
abcabcacabxy
Not Found
Not Found
```

```cpp
#include <iostream>
#include <cstring>
using namespace std;

const int N = 1e6 + 10;
const int M = 1e5 + 10;

int T;
char s[N], p[M];
int ne[M];

int main()
{
    cin >> s + 1;
    cin >> T;
    while (T --)
    {
        cin >> p + 1;
        int n = strlen(p + 1);
        
        for (int i = 2, j = 0; i <= n; ++ i)
        {
        	while (j && p[j + 1] != p[i]) j = ne[j];
        	if (p[i] == p[j + 1]) ++ j;
        	ne[i] = j;
        }
        
        bool flag = false;
        for (int i = 1, j = 0; i <= strlen(s + 1); ++ i)
        {
        	while (j && p[j + 1] != s[i]) j = ne[j];
        	if (s[i] == p[j + 1]) ++ j;
        	if (j == n)
        	{
        		flag = true;
        		cout << s + i - n + 1 << endl;
        		break;
        	}
        }

        if (!flag) cout << "Not Found" << endl;
    }
}
```
