#include "ds_elementTypeInit.h"
#include <stdlib.h>
#include <stdio.h>

struct Node
{
    ElementType Data;
    struct Node *Next;
};
struct QNode
{                       // 链队列结构
    struct Node *rear;  // 指向队尾结点
    struct Node *front; // 指向队头结点
};
typedef struct QNode *Queue;
Queue PtrQ;

// 出队操作
ElementType DeleteQ(Queue PtrQ)
{
    struct Node *FrontCell;
    ElementType FrontElem;

    if (PtrQ->front == NULL)
    {
        printf("队列空");
        return NULL;
    }
    FrontCell = PtrQ->front;
    if (PtrQ->front == PtrQ->rear)       // 若队列只有一个元素
        PtrQ->front = PtrQ->rear = NULL; // 删除后队列置为空
    else
        PtrQ->front = PtrQ->front->Next;
    FrontElem = FrontCell->Data;
    free(FrontCell); // 释放被删除结点空间
    return FrontElem;
}

// 入队操作，PiperLiu@qq.com自己写的
void AddQ(ElementType item, Queue PtrQ)
{
    struct Node TmpNode;
    TmpNode.Data = item;
    TmpNode.Next = NULL;

    if (PtrQ->front == NULL)
    { // 若当前队列为空
        PtrQ->front = &TmpNode;
        PtrQ->rear = &TmpNode;
    }
    else
    {
        PtrQ->rear->Next = &TmpNode;
        PtrQ->rear = &TmpNode;
    }
}
