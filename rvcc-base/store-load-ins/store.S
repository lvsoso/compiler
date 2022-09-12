.text
.globl sb_ins
#a0内存地址
#a1储存的值
sb_ins:
    sb a1, 0(a0)       #储存a1低8位到a0+0地址处
    jr ra              #返回

.globl sh_ins
#a0内存地址
#a1储存的值
sh_ins:
    sh a1, 0(a0)       #储存a1低16位到a0+0地址处
    jr ra              #返回

.globl sw_ins
#a0内存地址
#a1储存的值
sw_ins:
    sw a1, 0(a0)       #储存a1到a0+0地址处
    jr ra              #返回