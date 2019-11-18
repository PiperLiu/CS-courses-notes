#include <stdio.h>
#include <time.h>
#include <math.h>

#define MAXK 1e5

clock_t start, stop;

double duration;

double f1(int n, double a[], double x);
// 多项式算法

double f2(int n, double a[], double x);
// 秦九韶算法

int main()
{
    double a[11] = {1, 4, 1, 3, 5, 1, 6, 3, 3, 75, 21};

    start = clock();
    for (int i = 0; i < MAXK; i++)
    {
        f1(10, a, 1.1);
    }
    stop = clock();
    duration = ((double)(stop - start) / CLK_TCK / MAXK);
    printf("ticks1 = %f\n", (double)(stop - start));
    printf("duration1 = %6.2e\n", duration);

    start = clock();
    for (int i = 0; i < MAXK; i++)
    {
        f2(10, a, 1.1);
    }
    stop = clock();
    duration = ((double)(stop - start) / CLK_TCK / MAXK);
    printf("ticks2 = %f\n", (double)(stop - start));
    printf("duration2 = %6.2e\n", duration);

    return 0;
}

double f1(int n, double a[], double x)
{
    int i;
    double p = a[0];
    for (i = 1; i <= n; i++)
    {
        p += (a[i] * pow(x, i));
    }
    return p;
}

double f2(int n, double a[], double x)
{
    int i;
    double p = a[n];
    for (i = n; i > 0; i--)
    {
        p = a[i - 1] + x * p;
    }
    return p;
}
