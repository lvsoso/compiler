.text

.globl slti_ins
slti_ins:
        slti a0, a0, -2048 # if(a0<-2048) a0=1 else a0=0
        jr ra

.globl sltiu_ins
sltiu_ins:
    sltiu a0, a0, 2047      # if(a0<2047) a0=1 else a0=0
    jr ra

.globl slt_ins
slt_ins:
    slt a0, a0, a1          # if(a0<a1) a0=1 else a0=0
    jr ra

.globl sltu_ins
sltu_ins:
    sltu a0, a0, a1         # if(a0<a1) a0=1 else a0=0
    jr ra