#include "ds_elementTypeInit.h"
#include "stdlib.h"
#include "stdio.h"

typedef struct SNode *Stack;
struct SNode
{
    ElementType Data;
    struct SNode *Next;
};

Stack CreateStack()
{   /* 构建一个堆栈的头结点，返回指针 */
    Stack S;
    S = (Stack)malloc(sizeof(struct SNode));
    S->Next = NULL;
    return S;
}

int IsEmpty(Stack S)
{
    return (S->Next == NULL);
}

void Push(ElementType item, Stack S)
{
    /*
    ** 应注意，压入的元素在链表的第二节
    ** 第一结S用于表示链表，不包含元素
    */
    struct SNode *TmpCell;
    TmpCell = (struct SNode *)malloc(sizeof(struct SNode));
    TmpCell->Data = item;
    TmpCell->Next = S->Next;
    S->Next = TmpCell;
}

ElementType Pop(Stack S)
{
    struct SNode *FirstCell;
    ElementType TopElem;
    if (IsEmpty(S))
    {
        printf("堆栈空");
        return NULL;
    }
    else
    {
        FirstCell = S->Next;
        S->Next = FirstCell->Next;
        TopElem = FirstCell->Data;
        free(FirstCell);
        return TopElem;
    }
}
