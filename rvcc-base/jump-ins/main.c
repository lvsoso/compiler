#include "stdio.h"
int jal_ins(); //声明jal_ins函数
int jalr_ins(void* (*func)(int)); //声明jalr_ins函数

int beq_ins(int a, int b); //声明beq_ins函数
int bne_ins(int a, int b); //声明bne_ins函数

int blt_ins(int a, int b); //声明blt_ins函数
int bltu_ins(int a, int b); //声明bltu_ins函数

int bge_ins(int a, int b); //声明bge_ins函数
int bgeu_ins(int a, int b); //声明bgeu_ins函数

void* testjalr(int x)//声明并定义testjalr函数
{
    printf("This testjalr address is:0x%x\n", x);
    return jalr_ins;
}

int main()
{
    int result = 0;
    result = bge_ins(2, 1); //result = 1 = 2 > 1
    printf("This a >= b:%d\n", result);
    result = bge_ins(4, 4); //result = 1 = 4 >= 4
    printf("This a >= b:%d\n", result);
    result = bgeu_ins(3, -1); //result = 0 = 3 < (-1)0xffffffff
    printf("This ua >= ub:%d\n", result);

    return 0;
}