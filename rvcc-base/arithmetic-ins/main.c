#include "stdio.h"
int addi_ins(int x); //声明一下汇编语言中的函数：addi_ins
int addi_ins2(int x); //声明一下汇编语言中的函数：addi_ins2
int add_ins(int x, int y); //声明一下汇编语言中的函数：add_ins
int sub_ins(int x, int y); //声明一下汇编语言中的函数：sub_ins

int slti_ins(int x); //声明一下汇编语言中的函数：slti_ins
int sltiu_ins(int x); //声明一下汇编语言中的函数：sltiu_ins
int slt_ins(int x, int y); //声明一下汇编语言中的函数：slt_ins
int sltu_ins(int x, int y); //声明一下汇编语言中的函数：sltu_ins

int andi_ins(int x); //声明一下汇编语言中的函数：andi_ins
int and_ins(int x, int y); //声明一下汇编语言中的函数：and_ins

int ori_ins(int x); //声明一下汇编语言中的函数：ori_ins
int or_ins(int x, int y); //声明一下汇编语言中的函数：or_ins

int xori_ins(int x); //声明一下汇编语言中的函数：xori_ins
int xor_ins(int x, int y); //声明一下汇编语言中的函数：xor_ins

int slli_ins(int x); //声明一下汇编语言中的函数：slli_ins
int sll_ins(int x, int y); //声明一下汇编语言中的函数：sll_ins

int srli_ins(int x); //声明一下汇编语言中的函数：srli_ins
int srl_ins(int x, int y); //声明一下汇编语言中的函数：srl_ins

int srai_ins(int x); //声明一下汇编语言中的函数：srai_ins
int sra_ins(int x, int y); //声明一下汇编语言中的函数：sra_ins

int main()
{
    int result = 0;
    printf("add/addi\n");
    result = addi_ins(4);    //result = 4 + 5 = 9
    printf("This result is: %d\n", result);
    result = addi_ins2(2048);    //result = 2048 - 2048 = 0
    printf("This result is: %d\n", result);
    result = add_ins(1, 1);    //result = 2 = 1 + 1
    printf("This result is:%d\n", result);

    result = sub_ins(2, 1);    //result = 1 = 2 - 1
    printf("This result is:%d\n", result);

    printf("slti/sltiu\n");
    result = slti_ins(-2049);    //result = 1 = if(-2049 < -2048)
    printf("This result is:%d\n", result);
    result = sltiu_ins(-2048);    //result = 0 = if(-2048(0xfffff800) < 2047)
    printf("This result is:%d\n", result);
    result = slt_ins(1, 2);    //result = 1 = if(1 < 2)
    printf("This result is:%d\n", result);
    result = sltu_ins(-2, 1);    //result = 0 = if(-2(0xfffffffe) < 1)
    printf("This result is:%d\n", result);


    printf("andi/and\n");
    result = andi_ins(2);    //result = 2 = 2 & 0xff
    printf("This result is:%d\n", result);
    result = andi_ins(0xf0f0);    //result = 0xf0 = 0xf0f0 & 0xff
    printf("This result is:%x\n", result);
    result = and_ins(1, 1);    //result = 1 = 1 & 1
    printf("This result is:%d\n", result);
    result = and_ins(0, 1);    //result = 0 = 0 & 1
    printf("This result is:%d\n", result);


    printf("ori/or\n");
    result = ori_ins(0xf0f0);    //result = 0xf0f0 = 0xf0f0 | 0
    printf("This result is:%x\n", result);
    result = or_ins(0x1000, 0x1111);    //result = 0x1111 = 0x1000 | 0x1111
    printf("This result is:%x\n", result);
    
     printf("xori/xor\n");
    result = xori_ins(0xff);    //result = 0xff = 0xff ^ 0
    printf("This result is:%x\n", result);
    result = xor_ins(0xffff, 0xffff);    //result = 0 = 0xffff ^ 0xffff
    printf("This result is:%x\n", result);

    
    printf("slli_ins/sll_ins\n");
    result = slli_ins(0xffff);    //result = 0xffff0 = 0xffff << 4
    printf("This result is:%x\n", result);
    result = sll_ins(0xeeeeeeee, 31);    //result = 0 = 0xeeeeeeee << 31
    printf("This result is:%x\n", result);
    
    printf("srli_ins/srl_ins\n");
    result = srli_ins(0xffff);    //result = 0xff = 0xffff >> 8
    printf("This result is:%x\n", result);
    result = srl_ins(0xaaaaaaaa, 16);    //result = 0xaaaa = 0xaaaaaaaa >> 16
    printf("This result is:%x\n", result);
    
    // 算术右移
    // 数据在逻辑右移之后左边多出空位用 0 填充，
    // 而数据在算术右移之后左边多出的空位是用数据的符号位填充。      
    printf("srai_ins/sra_ins\n");
    result = srai_ins(0x1111);    //result = 0x11 = 0x1111 >> 8
    printf("This result is:%x\n", result);
    result = sra_ins(0xaaaaaaaa, 16);    //result = 0xffffaaaa = 0xaaaaaaaa >> 16
    printf("This result is:%x\n", result);

    return 0;
}
