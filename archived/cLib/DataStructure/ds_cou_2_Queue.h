#include "ds_elementTypeInit.h"
#include "stdio.h"
#include "stdlib.h"

#define MaxSize 1000

// 循环队列
struct QNode
{
    ElementType Data[MaxSize];
    int rear;
    int front;
};
typedef struct QNode *Queue;

// 仅使用n-1的空间，防止出现rear与front时无法区分状态的情况

// 入队列
void AddQ(Queue PtrQ, ElementType item)
{
    if ((PtrQ->rear + 1) % MaxSize == PtrQ->front)
    {
        printf("队列满");
        return;
    }
    PtrQ->rear = (PtrQ->rear + 1) % MaxSize;
    PtrQ->Data[PtrQ->rear] = item;
}

// 出队列
ElementType DeleteQ(Queue PtrQ)
{
    if (PtrQ->front == PtrQ->rear)
    {
        printf("队列空");
        return NULL;
    }
    else
    {
        PtrQ->front = (PtrQ->front + 1) % MaxSize;
        return PtrQ->Data[PtrQ->front];
    }
}
