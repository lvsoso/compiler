.text
.globl andi_ins
andi_ins:
        andi a0, a0, 0xff #  a0 = a0 & 0xff 
        jr ra

.globl and_ins
and_ins:
        and a0, a0, a1
        jr ra

.globl ori_ins
ori_ins:
        ori a0, a0, 0
        jr ra

.globl or_ins
or_ins:
        or a0, a0, a1
        jr ra
    
.globl xori_ins
xori_ins:
        xor a0, a0, 0
        jr ra
    
.globl xor_ins
xor_ins:
        xor a0, a0, a1