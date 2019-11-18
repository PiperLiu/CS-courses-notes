#include <stdio.h>

// 方法1：循环实现
void PrintN_1(int N)
{
    if(N)
    {
        PrintN_1(N-1);
        printf("%d\n", N);
    }
    return;
}
// 方法2：递归实现
void PrintN_2(int N)
{
    if(N)
    {
        PrintN_2(N - 1);
        printf("%d\n", N);
    }
    return;
}

int main()
{
    int N, flag;
    printf("输入N：");
    scanf("%d", &N);
    printf("键入0使用循环，1使用递归：");
    scanf("%d", &flag);
    switch (flag)
    {
    case 0:
        PrintN_1(N);
        break;
    
    default:
        PrintN_2(N);
        break;
    }
}

// develop with vscode
// use "Ctrl + `" to switch terminal
// use "g++ filename.cpp"
// use "./name.exe" to run it

