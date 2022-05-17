	.text
	.file	"ifelse.ll"
	.globl	main                    # -- Begin function main
	.p2align	4, 0x90
	.type	main,@function
main:                                   # @main
	.cfi_startproc
# %bb.0:                                # %_ifstart
	pushq	%rax
	.cfi_def_cfa_offset 16
	xorl	%eax, %eax
	testb	%al, %al
	jne	.LBB0_2
# %bb.1:                                # %_iftrue
	movl	$str0, %edi
	jmp	.LBB0_3
.LBB0_2:                                # %_iffalse
	movl	$str1, %edi
.LBB0_3:                                # %_ifend
	xorl	%eax, %eax
	callq	printf
	xorl	%eax, %eax
	popq	%rcx
	retq
.Lfunc_end0:
	.size	main, .Lfunc_end0-main
	.cfi_endproc
                                        # -- End function
	.type	str0,@object            # @str0
	.section	.rodata,"a",@progbits
	.globl	str0
str0:
	.asciz	"positive!\n"
	.size	str0, 11

	.type	str1,@object            # @str1
	.globl	str1
str1:
	.asciz	"negative!\n"
	.size	str1, 11


	.section	".note.GNU-stack","",@progbits
