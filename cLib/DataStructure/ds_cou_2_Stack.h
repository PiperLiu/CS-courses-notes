#include "ds_elementTypeInit.h"
#include <stdio.h>
#include <stdlib.h>

#define MaxSize 1000

// 使用数组存储
typedef struct SNode *Stack;
struct SNode
{
    ElementType Data[MaxSize];
    int Top;
};

// 创建堆栈
Stack CreateStack()
{
    Stack PtrS;
    PtrS = (Stack)malloc(sizeof(struct SNode));
    PtrS->Top = -1;
    return PtrS;
}

// 入栈
void Push(Stack PtrS, ElementType item)
{
    if (PtrS->Top == MaxSize - 1)
    {
        printf("堆栈满");
        return;
    }
    else
    {
        PtrS->Data[++(PtrS->Top)] = item;
        return;
    }
}

// 出栈
ElementType Pop(Stack PtrS)
{
    if (PtrS->Top == -1)
    {
        printf("堆栈空");
        return NULL;
    }
    else
    {
        return (PtrS->Data[(PtrS->Top)--]);
    }
}
