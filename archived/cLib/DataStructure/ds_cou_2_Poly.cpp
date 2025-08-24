#include <stdlib.h>
#include <stdio.h>

struct PolyNode
{
    int coef;              // 系数
    int expon;             // 指数
    struct PolyNode *link; // 指向下一节点的指针
};
typedef struct PolyNode *Polynomial;
Polynomial P1, P2;

int Compare(int A, int B);
void Attach(int c, int e, Polynomial *pRear);

Polynomial PolyAdd(Polynomial P1, Polynomial P2)
{
    Polynomial front, rear, temp;
    int sum;
    rear = (Polynomial)malloc(sizeof(struct PolyNode));
    front = rear;
    while (P1 && P2)
    {
        switch (Compare(P1->expon, P2->expon))
        {
        case 1:
            Attach(P1->coef, P1->expon, &rear);
            // Attach()将系数、指数形成新的项，拷贝到rear的后面
            P1 = P1->link;
            break;
        case -1:
            Attach(P2->coef, P2->expon, &rear);
            P2 = P2->link;
            break;
        case 0:
            sum = P1->coef + P2->coef;
            if (sum)
                Attach(sum, P1->expon, &rear);
            P1 = P1->link;
            P2 = P2->link;
            break;
        }
    }
    for (; P1; P1 = P1->link)
        Attach(P1->coef, P2->expon, &rear);
    for (; P2; P2 = P2->link)
        Attach(P2->coef, P2->expon, &rear);
    rear->link = NULL;
    temp = front; // 把空节点赋给temp，front后移，空节点temp释放掉。
    front = front->link;
    free(temp);
    return front;
}

// Compare
int Compare(int A, int B)
{
    int C = A - B;
    if (C > 0)
        return 1;
    else if (C == 0)
        return 0;
    else
        return -1;
}

// Attach
void Attach(int c, int e, Polynomial *pRear)
{
    // 因为c函数常数为值传递，因此传入指针的指针pRear
    // 而*pRear代表pRear（指针的指针）指向的值
    Polynomial P;

    P = (Polynomial)malloc(sizeof(struct PolyNode));
    P->coef = c;
    P->expon = e;
    P->link = NULL;
    (*pRear)->link = P;
    *pRear = P;
}

Polynomial ReadPoly();
Polynomial Mult(Polynomial P1, Polynomial P2);
void PrintPoly(Polynomial P);

int main()
{
    Polynomial P1, P2, PP, PS;

    P1 = ReadPoly();
    P2 = ReadPoly();
    PP = Mult(P1, P2);
    PrintPoly(PP);
    PS = PolyAdd(P1, P2);
    PrintPoly(PS);

    return 0;
}

Polynomial ReadPoly()
{
    Polynomial P, Rear, t;
    int c, e, N;

    printf("N Polynome, N: ");
    scanf("%d", &N);
    P = (Polynomial)malloc(sizeof(struct PolyNode));
    P->link = NULL;
    Rear = P;
    while (N--)
    {
        printf("coef and expon:");
        scanf("%d %d", &c, &e);
        Attach(c, e, &Rear);
    }
    t = P;
    P = P->link;
    free(t);
    return P;
}

Polynomial Mult(Polynomial P1, Polynomial P2)
{
    Polynomial P, Rear, t1, t2, t;
    int c, e;

    if (!P1 || !P2)
        return NULL;

    t1 = P1;
    t2 = P2;
    P = (Polynomial)malloc(sizeof(struct PolyNode));
    P->link = NULL;
    Rear = P;
    while (t2) /* 先用P1的第1项乘以P2，得到P */
    {
        Attach(t1->coef * t2->coef, t1->expon + t2->expon, &Rear);
        t2 = t2->link;
    }
    t1 = t1->link;
    while (t1)
    {
        t2 = P2;
        Rear = P;
        while (t2)
        {
            e = t1->expon + t2->expon;
            c = t1->coef * t2->coef;
            while (Rear->link && Rear->link->expon > e)
            {
                Rear = Rear->link;
            }
            if (Rear->link && Rear->link->expon == e)
            {
                if (Rear->link->coef + c)
                    Rear->link->coef += c;
                else
                { // 如果系数相加等于0，删去这个结点
                    t = Rear->link;
                    Rear->link = t->link;
                    free(t);
                }
            }
            else
            { // 如果指数不相等，即小于的情况，申请新结点
                t = (Polynomial)malloc(sizeof(struct PolyNode));
                t->coef = c;
                t->expon = e;
                t->link = Rear->link;
                Rear->link = t;
                Rear = Rear->link;
            }
            t2 = t2->link;
        }
        t1 = t1->link;
    }
    t2 = P;
    P = P->link;
    free(t2);
    return P;
}

void PrintPoly(Polynomial P)
{
    int flag = 0; // 辅助调整输出格式用
    if (!P)
    {
        printf("0 0\n");
        return;
    }
    while (P)
    {
        if (!flag)
        {
            printf("\n");
            flag = 1;
        }
        else
            printf(" ");
        printf("%d %d", P->coef, P->expon);
        P = P->link;
    }
}
