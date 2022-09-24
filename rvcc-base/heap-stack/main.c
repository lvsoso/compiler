#include "stdio.h"
#include "stdlib.h"
#include "unistd.h"
#include "string.h"

// variables from linker
extern int __executable_start,etext, edata, __bss_start, end;

void tmp()
{
    char c;
    printf("Text段,程序运行时指令数据开始:%p,结束:%p\n", &__executable_start, &etext);
    printf("Data段,程序运行时初始化全局变量和静态变量的数据开始:%p,结束:%p\n", &etext, &edata);
    printf("Bss段,程序运行时未初始化全局变量和静态变量的数据开始:%p,结束:%p\n", &__bss_start, &end);

    while (1)
    {
        printf("(pid:%d)应用程序正在运行,请输入:c,退出\n", getpid());
        printf("请输入:");
        c = getchar();
        if(c == 'c')
        {
            printf("应用程序退出\n");
            return;
        }
        printf("%c\n", c);
    }

    return;
}


void heaptest()
{
    // return current heap pointer
    void * currheap = sbrk(0);

    // alloc a byte space
    void* alloc = sbrk(1);
    void* alloc2 = sbrk(0x1000);

    printf("程序运行时当前堆指针:%p 新分配的堆内存:%p :%p\n", currheap, alloc, alloc2);
    memset(alloc, 0, 1024);
    return;
}

void stacktest2()
{
    long val1 = 1;
    long val2 = 2;
    long val3 = 3;
    printf("stacktest2运行时val1地址:%p val2地址:%p val3地址:%p\n", &val1, &val2, &val3);
    return;
}

void stacktest1()
{
    long val1 = 1;
    long val2 = 2;
    long val3 = 3;
    printf("stacktest1运行时val1地址:%p val2地址:%p val3地址:%p\n", &val1, &val2, &val3);
    stacktest2();
    return;
}

int main()
{
    stacktest1();
    //heaptest();
    tmp();
    return 0;
}