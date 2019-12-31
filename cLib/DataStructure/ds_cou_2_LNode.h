#include "ds_elementTypeInit.h"
#include <stdlib.h>
#include <stdio.h>

typedef struct LNode *List;
struct LNode
{
    ElementType Data[MAXSIZE];
    int Last;
};
struct LNode L;
List PtrL;

/*
** 主要操作实现
*/

// 初始化，建立空的顺序表
List MakeEmpty()
{
    List PtrL;
    PtrL = (List)malloc(sizeof(struct LNode));
    PtrL->Last = -1;
    return PtrL;
}

// 查找
int Find(ElementType X, List PtrL)
{
    int i = 0;
    while (i <= PtrL->Last && PtrL->Data[i] != X)
        i++;
    if (i > PtrL->Last)
        return -1; // 如果没有找到，返回-1
    else
        return i; // 找到后返回的是存储的位置(起始为1)
}

// 插入，第i个位置(起始为1)上插入一个值为X的新元素
void Insert(ElementType X, int i, List PtrL)
{
    int j;
    if (PtrL->Last == MAXSIZE - 1)
    {
        printf("表满");
        return;
    }
    if (i < 1 || i > PtrL->Last + 2)
    {
        printf("位置不合法");
        return;
    }
    for (j = PtrL->Last; j >= i - 1; j--)
        PtrL->Data[j + 1] = PtrL->Data[j]; // 将a_i~a_n倒序向后移动
    PtrL->Data[i - 1] = X;                 // 新元素插入
    PtrL->Last++;                          // Last仍指向最后的元素
    return;
}

// 删除
void Delete(int i, List PtrL)
{
    int j;
    if (i < 1 || i > PtrL->Last + 1)
    {
        printf("不存在第%d个元素", i);
        return;
    }
    for (j = i; j <= PtrL->Last; j++)
        PtrL->Data[j - 1] = PtrL->Data[j];
    PtrL->Last--;
    return;
}
