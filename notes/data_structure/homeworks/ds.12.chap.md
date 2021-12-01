
<!-- @import "[TOC]" {cmd="toc" depthFrom=1 depthTo=6 orderedList=false} -->

<!-- code_chunk_output -->

- [选择题](#选择题)
  - [这段程序打印出来什么东西？](#这段程序打印出来什么东西)
- [讨论题](#讨论题)
  - [](#)
- [编程题](#编程题)
  - [](#-1)

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

## 讨论题

### 

## 编程题

### 
