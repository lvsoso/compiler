#include "stdio.h"

void LockInit(int* lock);
int Locked(int* lock);
int UnLock(int* lock);

int lrsc_ins(int* old, int cmp, int update);//声明lrsc_ins函数

int amoswap_ins(int* old, int update);//声明amoswap_ins函数
int amoadd_ins(int* old, int update);//声明amoadd_ins函数
int amoand_ins(int* old, int update);//声明amoand_ins函数
int amoor_ins(int* old, int update);//声明amoor_ins函数
int amoxor_ins(int* old, int update);//声明amoxor_ins函数
int amomax_ins(int* old, int update);//声明amomax_ins函数
int amomaxu_ins(int* old, int update);//声明amomaxu_ins函数
int amomin_ins(int* old, int update);//声明amomin_ins函数
int amominu_ins(int* old, int update);//声明amominu_ins函数

int main()
{
    int result = 0;
    int val = 1;

    int lock;
    LockInit(&lock);
    result = lrsc_ins(&val, 0, 1);                 //result = 0  val = 1
    printf("This is result:%d val:%d\n", result, val);
    printf("This is locked:%d :%d :%d\n", Locked(&lock), Locked(&lock), UnLock(&lock));


    result = amoswap_ins(&val, 2);                    //result = 1  val = 2
    printf("amoswap_ins This is result:%d val:%d\n", result, val);

    result = amoadd_ins(&val, 1);                    //result = 2  val = 3
    printf("amoadd_ins This is result:%d val:%d\n", result, val);

    val = 255;
    result = amoand_ins(&val, 15);                    //result = 255  val = 15
    printf("amoand_ins This is result:%d val:%d\n", result, val);
    result = amoor_ins(&val, 0);                     //result = 15  val = 15
    printf("amoor_ins This is result:%d val:%d\n", result, val);
    result = amoxor_ins(&val, val);                     //result = 15  val = 0
    printf("amoxor_ins This is result:%d val:%d\n", result, val);

    val = 0;
    result = amomax_ins(&val, 1);                    //result = 0 val = 1
    printf("amomax_ins This is result:%d val:%d\n", result, val);
    result = amomaxu_ins(&val, -1);                    //result = 1 val = -1
    printf("amomaxu_ins This is result:%d val:%d\n", result, val);
    result = amomin_ins(&val, -2);                    //result = -1 val = -2
    printf("amomin_ins This is result:%d val:%d\n", result, val);
    result = amominu_ins(&val, -3);                    //result = -2 val = -2
    printf("amominu_ins This is result:%d val:%d\n", result, val);
    
    testspinlock();  
    return 0;
}