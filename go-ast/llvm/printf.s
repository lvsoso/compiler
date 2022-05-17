	.text
	.file	"printf.ll"
	.section	.rodata.cst8,"aM",@progbits,8
	.p2align	3               # -- Begin function main
.LCPI0_0:
	.quad	4593671619917905920     # double 0.125
	.text
	.globl	main
	.p2align	4, 0x90
	.type	main,@function
main:                                   # @main
	.cfi_startproc
# %bb.0:
	pushq	%rax
	.cfi_def_cfa_offset 16
	movsd	.LCPI0_0(%rip), %xmm0   # xmm0 = mem[0],zero
	movl	$str, %edi
	movq	$-10, %rsi
	movl	$15, %edx
	movb	$1, %al
	callq	printf
	xorl	%eax, %eax
	popq	%rcx
	retq
.Lfunc_end0:
	.size	main, .Lfunc_end0-main
	.cfi_endproc
                                        # -- End function
	.type	str,@object             # @str
	.section	.rodata,"a",@progbits
	.globl	str
	.p2align	4
str:
	.asciz	"%ld - 0x%x - %lf\n"
	.size	str, 18


	.section	".note.GNU-stack","",@progbits
