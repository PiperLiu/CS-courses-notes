#include "ds_elementTypeInit.h"
#include <stdlib.h>
#include <stdio.h>

typedef struct LNode *List;
struct LNode
{
    ElementType Data;
    List Next;
};
struct LNode L;
List PtrL;

// 求表长
int Length(List PtrL)
{
    List p = PtrL;
    int j = 0;
    while (p)
    {
        p = p->Next;
        j++;
    }
    return j;
}

// 查找：按序号查找
List FindKth(int K, List PtrL)
{
    List p = PtrL;
    int i = 1;
    while (p != nullptr && i < K)
    {
        p = p->Next;
        i++;
    }
    if (i == K)
        return p;
    else
        return nullptr;
}

// 查找：按值查找
List Find(ElementType X, List PtrL)
{
    List p = PtrL;
    while (p != nullptr && p->Data != X)
        p = p->Next;
    return p; // 没找到就返回NULL
}

// 插入
List Insert(ElementType X, int i, List PtrL)
{
    List p, s;
    if (i == 1)
    { /* 新节点插入在表头 */
        s = (List)malloc(sizeof(struct LNode));
        s->Data = X;
        s->Next = PtrL;
        return s;
    }
    p = FindKth(i - 1, PtrL);
    if (p == nullptr)
    {
        printf("参数i错");
        return NULL;
    }
    else
    {
        s = (List)malloc(sizeof(struct LNode));
        s->Data = X;
        s->Next = p->Next;
        p->Next = s;
        return PtrL;
    }
}

// 删除
List Delete(int i, List PtrL)
{
    List p, s;
    if (i == 1)
    {
        s = PtrL;
        if (PtrL != NULL)
            PtrL = PtrL->Next;
        else
            return NULL;
        free(s);
        return PtrL;
    }
    p = FindKth(i - 1, PtrL);
    if (p == nullptr)
    {
        printf("第%d个结点不存在", i - 1);
        return NULL;
    }
    else if (p->Next == NULL)
    {
        printf("第%d个结点不存在", i);
        return NULL;
    }
    else
    {
        s = p->Next;
        p->Next = s->Next;
        free(s);
        return PtrL;
    }
}
