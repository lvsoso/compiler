//ls.S
.text
.globl sw_ins
# a0 mem address
# a1 store value
sw_ins:
    sw a1, 0(a0) # store a1 to a0+0
    jr ra       # return
