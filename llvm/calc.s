	.text
	.file	"calc.ll"
	.globl	main                    # -- Begin function main
	.p2align	4, 0x90
	.type	main,@function
main:                                   # @main
	.cfi_startproc
# %bb.0:
	pushq	%rax
	.cfi_def_cfa_offset 16
	movl	$str, %edi
	movl	$13, %esi
	xorl	%eax, %eax
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
str:
	.asciz	"%d\n"
	.size	str, 4


	.section	".note.GNU-stack","",@progbits
