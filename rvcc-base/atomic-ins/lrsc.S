
#text
.globl lrsc_ins
#a0内存地址
#a1预期值
#a2所需值
#a0返回值, 如果成功, 则为0! 否则为1
lrsc_ins:
cas:
    lr.w t0, (a0)       #加载以前的值
    bne t0, a1, fail    #不相等则跳转到fail
    sc.w a0, a2, (a0)   #尝试更新
    jr ra               #返回
fail:
    li a0, 1            #a0 = 1
    jr ra               #返回