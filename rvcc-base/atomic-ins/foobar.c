#include "stdio.h"
//全局变量A
int A = 0;
//线程A执行的函数
void thread_a()
{
    A++;
    printf("ThreadA A is:%d\n", A);
    return;
}
//线程B执行的函数
void thread_b()
{
    A++;
    printf("ThreadB A is:%d\n", A);
    return;
}