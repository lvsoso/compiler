.text
.global beq_ins
beq_ins:
    beq a0, a1, imm_l1          #a0==a1, 跳转到imm_l1地址处开始运行
    mv a0, zero                 #a0=0
    jr ra                       #函数返回    
imm_l1:
    addi a0, zero, 1            #a0=1
    jr ra                       #函数返回

.global bne_ins
bne_ins:
    bne a0, a1, imm_l2          #a0!=a1, 跳转到imm_l2地址处开始运行
    mv a0, zero                 #a0=0
    jr ra                       #函数返回    
imm_l2:
    addi a0, zero, 1            #a0=1
    jr ra

.global blt_ins
blt_ins:
    blt a0, a1, imm_l3          #a0<a1,跳转到imm_l3地址处开始运行
    mv a0, zero                 #a0=0
    jr ra                       #函数返回    
imm_l3:
    addi a0, zero, 1            #a0=1
    jr ra                       #函数返回

.global bltu_ins
bltu_ins:
    bltu a0, a1, imm_l4         #a0<a1, 跳转到imm_l4地址处开始运行
    mv a0, zero                 #a0=0
    jr ra                       #函数返回    
imm_l4:
    addi a0, zero, 1            #a0=1
    jr ra                       #函数返回

.global bge_ins
bge_ins:
    bge a0, a1, imm_l5          #a0>=a1, 跳转到imm_l5地址处开始运行
    mv a0, zero                 #a0=0
    jr ra                       #函数返回    
imm_l5:
    addi a0, zero, 1            #a0=1
    jr ra                       #函数返回

.global bgeu_ins
bgeu_ins:
    bgeu a0, a1, imm_l6         #a0>=a1, 跳转到imm_l6地址处开始运行
    mv a0, zero                 #a0=0
    jr ra                       #函数返回    
imm_l6:
    addi a0, zero, 1            #a0=1
    jr ra                       #函数返回        