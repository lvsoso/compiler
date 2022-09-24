.text
.globl sw_ins
#a0内存地址
#a1储存的值
sw_ins:
    sw a1, 0(a0)       #储存a1到a0+0地址处
    jr ra              #返回