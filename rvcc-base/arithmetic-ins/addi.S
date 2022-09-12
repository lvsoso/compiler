.text		
.global	addi_ins

addi_ins:
    addi a0, a0, 5  # a0 = a0 + 5
    jr ra



.global addi_ins2
addi_ins2:
        addi a0, a0, -2048  # a0 = a0-2048
        jr ra