#include <time.h>
#include <stdio.h>

#define MAXK 1e5

double timeCal(int (*MaxSubseqSum)(int *, int), int A[], int N);

int MaxSubseqSum1(int A[], int N);    // 暴力算法，遍历所有
int MaxSubseqSum2(int A[], int N);    // 不做重复的加法
int MaxSubseqSum3(int List[], int N); // 分而治之
int MaxSubseqSum4(int A[], int N);    // 在线处理

int main()
{
    int a[15] = {-1, 4, -1, -3, 5, 1, -6, 3, 3, 5, -21, 1, -3, 6, -2};
    int N = 15;
    double duration[4];

    duration[0] = timeCal(MaxSubseqSum1, a, N);
    duration[1] = timeCal(MaxSubseqSum2, a, N);
    duration[2] = timeCal(MaxSubseqSum3, a, N);
    duration[3] = timeCal(MaxSubseqSum4, a, N);
    for (int i = 0; i < 4; i++)
        printf("duration_%d = %6.2e \n", i + 1, duration[i]);

    return 0;
    /*
    duration_1 = 1.46e-006
    duration_2 = 2.80e-007
    duration_3 = 2.70e-007
    duration_4 = 3.00e-008
    数据规模小，分而治之的方法与不做重复加法的方法相比，优势不明显。
    */
}

double timeCal(int (*MaxSubseqSum)(int *, int), int A[], int N)
{
    clock_t start, stop;
    double duration;

    start = clock();
    for (int i = 0; i < MAXK; i++)
    {
        MaxSubseqSum(A, N);
    }
    stop = clock();
    duration = ((double)(stop - start) / CLK_TCK / MAXK);
    return duration;
}

int MaxSubseqSum1(int A[], int N)
{
    int ThisSum, MaxSum = 0;
    int i, j, k;
    for (i = 0; i < N; i++)
    {
        for (j = 1; j < N; j++)
        {
            ThisSum = 0;
            for (k = i; k <= j; k++)
                ThisSum += A[k];
            if (ThisSum > MaxSum)
                MaxSum = ThisSum;
        }
    }
    return MaxSum;
}

int MaxSubseqSum2(int A[], int N)
{
    int ThisSum, MaxSum = 0;
    int i, j, k;
    for (i = 0; i < N; i++)
    {
        ThisSum = 0;
        for (j = i; j < N; j++)
        {
            ThisSum += A[j];
            if (ThisSum > MaxSum)
                MaxSum = ThisSum;
        }
    }
    return MaxSum;
}

// 分而治之
int Max3(int A, int B, int C)
{ /* 返回3个整数中的最大值 */
    return A > B ? A > C ? A : C : B > C ? B : C;
}

int DivideAndConquer(int List[], int left, int right)
{                                            /* 分治法求List[left]到List[right]的最大子列和 */
    int MaxLeftSum, MaxRightSum;             /* 存放左右子问题的解 */
    int MaxLeftBorderSum, MaxRightBorderSum; /*存放跨分界线的结果*/

    int LeftBorderSum, RightBorderSum;
    int center, i;

    if (left == right)
    { /* 递归的终止条件，子列只有1个数字 */
        if (List[left] > 0)
            return List[left];
        else
            return 0;
    }

    /* 下面是"分"的过程 */
    center = (left + right) / 2; /* 找到中分点 */
    /* 递归求得两边子列的最大和 */
    MaxLeftSum = DivideAndConquer(List, left, center);
    MaxRightSum = DivideAndConquer(List, center + 1, right);

    /* 下面求跨分界线的最大子列和 */
    MaxLeftBorderSum = 0;
    LeftBorderSum = 0;
    for (i = center; i >= left; i--)
    { /* 从中线向左扫描 */
        LeftBorderSum += List[i];
        if (LeftBorderSum > MaxLeftBorderSum)
            MaxLeftBorderSum = LeftBorderSum;
    } /* 左边扫描结束 */

    MaxRightBorderSum = 0;
    RightBorderSum = 0;
    for (i = center + 1; i <= right; i++)
    { /* 从中线向右扫描 */
        RightBorderSum += List[i];
        if (RightBorderSum > MaxRightBorderSum)
            MaxRightBorderSum = RightBorderSum;
    } /* 右边扫描结束 */

    /* 下面返回"治"的结果 */
    return Max3(MaxLeftSum, MaxRightSum, MaxLeftBorderSum + MaxRightBorderSum);
}

int MaxSubseqSum3(int List[], int N)
{ /* 保持与前2种算法相同的函数接口 */
    return DivideAndConquer(List, 0, N - 1);
}

// 在线处理
int MaxSubseqSum4(int A[], int N)
{
    int ThisSum, MaxSum;
    int i;
    ThisSum = MaxSum = 0;
    for (i = 0; i < N; i++)
    {
        ThisSum += A[i]; // 向右累加
        if (ThisSum > MaxSum)
            MaxSum = ThisSum; // 发现更大和则更新当前结果
        else if (ThisSum < 0) // 如果当前子列和为负
            ThisSum = 0;      // 则不可能使后面的部分和增大，抛弃之
    }
    return MaxSum;
}