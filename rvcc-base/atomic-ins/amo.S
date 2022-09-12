.text
.globl amoswap_ins
#a0内存地址
#a1将要交换的值
#a0返回值
amoswap_ins:
    amoswap.w a0, a1, (a0)  #原子交换a0 = [a0] =a1
    jr ra                   #返回

.globl amoadd_ins
#a0内存地址
#a1相加的值
#a0返回值
amoadd_ins:
    amoadd.w a0, a1, (a0)   #原子相加a0 = [a0] [a0] = [a0] + a1
    jr ra                   #返回

.globl amoand_ins
#a0内存地址
#a1相与的值
#a0返回值
amoand_ins:
    amoand.w a0, a1, (a0)   #原子相与a0 = [a0] [a0] = [a0] & a1
    jr ra                   #返回

.globl amoor_ins
#a0内存地址
#a1相或的值
#a0返回值
amoor_ins:
    amoor.w a0, a1, (a0)    #原子相或a0 = [a0] [a0] = [a0] | a1
    jr ra                   #返回

.globl amoxor_ins
#a0内存地址
#a1相异或的值
#a0返回值
amoxor_ins:
    amoxor.w a0, a1, (a0)   #原子相异或a0 = [a0] [a0] = [a0] ^ a1
    jr ra                   #返回


.globl amomax_ins
#a0内存地址
#a1相比的值
#a0返回值
amomax_ins:
    amomax.w a0, a1, (a0)   #原子相与a0 = [a0] [a0] = max([a0] , a1)
    jr ra                   #返回

.globl amomaxu_ins
#a0内存地址
#a1相比的值
#a0返回值
amomaxu_ins:
    amomaxu.w a0, a1, (a0)   #原子相与a0 = [a0] [a0] = maxu([a0] , a1)
    jr ra                   #返回

.globl amomin_ins
#a0内存地址
#a1相比的值
#a0返回值
amomin_ins:
    amomin.w a0, a1, (a0)   #原子相与a0 = [a0] [a0] = min([a0] , a1)
    jr ra                   #返回

.globl amominu_ins
#a0内存地址
#a1相比的值
#a0返回值
amominu_ins:
    amominu.w a0, a1, (a0)   #原子相与a0 = [a0] [a0] = minu([a0] , a1)
    jr ra                    #返回
