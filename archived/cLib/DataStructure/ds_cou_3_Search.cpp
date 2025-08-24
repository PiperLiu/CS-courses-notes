#include "ds_elementTypeInit.h"
#include "stdlib.h"
#include "stdio.h"

typedef struct LNode *List;
struct LNode
{
    ElementType Element[MAXSIZE];
    int Length;
};

// int SequentialSearch(List Tbl, ElementType K)
// { /* 在 Element[1] ~ Element[n] 中查找 */
//     int i;
//     for (i = Tbl->Length; i>0 && Tbl->Element[i] != K; i--)
//         ;
//     return i; /* 查找成功，则返回所在单元下标，否则返回0 */
// }

int SequentialSearch(List Tbl, ElementType K)
{ /* 在 Element[1] ~ Element[n] 中查找 */
    int i;
    Tbl->Element[0] = K; /* 建立哨兵 */
    for (i = Tbl->Length; Tbl->Element[i] != K; i--)
        ;
    return i; /* 查找成功，则返回所在单元下标，否则返回0 */
}

int BinarySearch(List Tbl, ElementType K)
{
    int left, right, mid, NotFound = -1;
    left = 1;
    right = Tbl->Length;
    while (left <= right)
    {
        mid = (left + right) / 2;
        if (K < Tbl->Element[mid])
            right = mid - 1;
        else if (K > Tbl->Element[mid])
            left = mid + 1;
        else
            return mid;
    }
    return NotFound;
}

int main()
{
    struct LNode Lnode = {{0, 21, 23, 46, 51, 60}, 5};
    int output = SequentialSearch(&Lnode, 23);
    printf("%d\n", output);
    output = BinarySearch(&Lnode, 23);
    printf("%d\n", output);
    return 0;
}