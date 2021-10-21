#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

int main()
{
    int p1[2], p2[2];
    pipe(p1);
    pipe(p2);

    if (fork() != 0)
    {
        // 父进程
        printf("parent p2[0] %d\n", p2[0]);
        printf("parent p2[1] %d\n", p2[1]);
        printf("parent p1[0] %d\n", p1[0]);
        printf("parent p1[1] %d\n", p1[1]);
        close(p1[0]);  // 关闭父进程自己的 p1 读端
        close(p2[1]);  // 关闭父进程自己的 p2 写端
        char ping[1] = "B";

        clock_t start, end;
        double rtt;

        start = clock();
        for (int i = 0; i < (int)1e5; ++ i)
        {
            write(p1[1], ping, 1);
            read(p2[0], ping, 1);
        }
        end = clock();

        wait(0);
        close(p1[1]);  // 别忘了关闭管道
        close(p2[0]);

        rtt = (((double)(end - start)) / CLOCKS_PER_SEC) / 1e5;
        printf("average RTT: %f ms\n", (rtt * 1000));
        printf("exchanges per second: %ld times\n", (unsigned long)(1 / rtt));
    }
    else
    {
        // 子进程
        printf("child p2[0] %d\n", p2[0]);
        printf("child p2[1] %d\n", p2[1]);
        printf("child p1[0] %d\n", p1[0]);
        printf("child p1[1] %d\n", p1[1]);
        close(p1[1]);
        close(p2[0]);
        char pong[1];

        for (int j = 0; j < (int)1e5; ++ j)
        {
            read(p1[0], pong, 1);
            write(p2[1], pong, 1);
        }

        close(p1[0]);
        close(p2[1]);
        exit(0);
    }

    return 0;
}