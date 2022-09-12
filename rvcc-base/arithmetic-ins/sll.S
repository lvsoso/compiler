.text
.globl slli_ins
slli_ins:
    slli a0, a0, 4          #a0 = a0<<4
    jr ra

.globl sll_ins
sll_ins:
    sll a0, a0, a1          #a0 = a0<<a1
    jr ra

# 逻辑右移
.globl srli_ins
srli_ins:
    srli a0, a0, 8          #a0 = a0>>8
    jr ra

.globl srl_ins
srl_ins:
    srl a0, a0, a1          #a0 = a0>>a1
    jr ra

# 算术右移
# 数据在逻辑右移之后左边多出空位用 0 填充，
# 而数据在算术右移之后左边多出的空位是用数据的符号位填充。
.globl srai_ins
srai_ins:
    srai a0, a0, 8          #a0 = a0>>8
    jr ra

.globl sra_ins
sra_ins:
    sra a0, a0, a1          #a0 = a0>>a1
    jr ra