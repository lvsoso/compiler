.text
.global jal_ins

    # jal 无条件的跳转并链接的指令
    # rd = pc + 4;
    # pc = pc + 符号扩展(imm << 1)
jal_ins:
    jal a0, imm_l1              #a0=add x0,x0,x0的地址, 跳转到imm_l1地址处开始运行
    add x0, x0, x0              #相当于nop, 什么也不做
    add x0, x0, x0
imm_l1:
    jal a0, imm_l2              #a0=imm_l2的地址, 跳转到imm_l2地址处开始运行
imm_l2:
    jr ra                       #函数返回

.global jalr_ins
    
    # “r”表示寄存器,相当于 jal 指令的寄存器版本，能够通过寄存器传递跳转地址。
    # rd = pc + 4;
    # pc = (rs1 + 符号扩展(imm << 1)) & 0xfffffffe
    # 为了指令要以 2 字节或者 4 字节地址对齐，所以最低位必须为 0, 形成一个地址值
jalr_ins:
    addi sp, sp, -16            #在栈中分配16字节的空间
    sw  ra, 0(sp)               #保存ra寄存器到栈中
    jalr ra, a0, 0              #ra = lw ra, 0(sp)指令的地址, 跳转到a0+0的地址处开始运行
    lw ra, 0(sp)                #从栈中恢复ra寄存器
    addi sp, sp, 16             #回收栈中分配的16字节的空间
    jr ra                       #函数返回
