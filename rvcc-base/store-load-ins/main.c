#include "stdio.h"

int lb_ins(unsigned int addr);//声明lb_ins函数
int lbu_ins(unsigned int addr);//声明lbu_ins函数
int lh_ins(unsigned int addr);//声明lh_ins函数
int lhu_ins(unsigned int addr);//声明lhu_ins函数
int lw_ins(unsigned int addr);//声明lw_ins函数

void sb_ins(unsigned int addr, int val);//声明sb_ins函数
void sh_ins(unsigned int addr, int val);//声明sh_ins函数
void sw_ins(unsigned int addr, int val);//声明sw_ins函数

char byte = -5;
short half = -1;
unsigned int word = 0xffffffff;
int main()
{
    int result = 0;
    result = lb_ins((unsigned int)&byte);                                   //result = 0xfffffffb (-5)
    printf("This is result:%d byte:%d\n", result, (unsigned int)byte);
    result = lbu_ins((unsigned int)&byte);                                  //result = 0xfb (251)
    printf("This is result:%d byte:%d\n", result, (unsigned int)byte);
    result = lh_ins((unsigned int)&half);                                   //result = 0xffffffff (-1)
    printf("This is result:%d half:%d\n", result, (unsigned short)half);
    result = lhu_ins((unsigned int)&half);                                  //result = 0xffff (65535)
    printf("This is result:%d half:%d\n", result, (unsigned short)half);
    result = lw_ins((unsigned int)&word);                                   //result = 0xffffffff (-1)
    printf("This is result:%d word:%u\n", result, word);

    sb_ins((unsigned int)&byte, 128);                                       //byte = 128
    printf("This is byte:%d\n", (unsigned int)byte);
    
    sh_ins((unsigned int)&half, 0xa5a5);                                    //half = 0xa5a5
    printf("This is half:%X\n", (unsigned short)half);

    sw_ins((unsigned int)&word, 0);                                         //word = 0
    printf("This is word:%u\n", (unsigned int)word);
    return 0;
}