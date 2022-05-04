	.text
	.file	"limit-loop.ll"
	.globl	main                    # -- Begin function main
	.p2align	4, 0x90
	.type	main,@function
main:                                   # @main
	.cfi_startproc
# %bb.0:                                # %_l0
	pushq	%rbx
	.cfi_def_cfa_offset 16
	.cfi_offset %rbx, -16
	xorl	%ebx, %ebx
	cmpl	$9, %ebx
	jg	.LBB0_3
	.p2align	4, 0x90
.LBB0_2:                                # %_l2
                                        # =>This Inner Loop Header: Depth=1
	movl	$str, %edi
	xorl	%eax, %eax
	movl	%ebx, %esi
	callq	printf
	incl	%ebx
	cmpl	$9, %ebx
	jle	.LBB0_2
.LBB0_3:                                # %_l3
	xorl	%eax, %eax
	popq	%rbx
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
