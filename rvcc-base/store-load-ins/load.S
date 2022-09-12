.text
.globl lb_ins
#a0内存地址
#a0返回值
lb_ins:
    lb a0, 0(a0)       #加载a0+0地址处的字节到a0中
    jr ra              #返回

.globl lbu_ins
#a0内存地址
#a0返回值
lbu_ins:
    lbu a0, 0(a0)      #加载a0+0地址处的字节到a0中
    jr ra              #返回

.globl lh_ins
#a0内存地址
#a0返回值
lh_ins:
    lh a0, 0(a0)       #加载a0+0地址处的半字到a0中
    jr ra              #返回

.globl lhu_ins
#a0内存地址
#a0返回值
lhu_ins:
    lhu a0, 0(a0)       #加载a0+0地址处的半字到a0中
    jr ra               #返回

.globl lw_ins
#a0内存地址
#a0返回值
lw_ins:
    .word 0x00052503    #lw a0, 0(a0)        #加载a0+0地址处的字到a0中
    jr ra               #返回