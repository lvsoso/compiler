	.text
	.file	"phi.ll"
	.globl	main                    # -- Begin function main
	.p2align	4, 0x90
	.type	main,@function
main:                                   # @main
	.cfi_startproc
# %bb.0:                                # %_ifstart
	pushq	%rax
	.cfi_def_cfa_offset 16
	movl	$10, %eax
	xorl	%ecx, %ecx
	testb	%cl, %cl
	jne	.LBB0_2
# %bb.1:                                # %_iftrue
	incl	%eax
	jmp	.LBB0_3
.LBB0_2:                                # %_iffalse
	decl	%eax
.LBB0_3:                                # %_ifend
	movl	%eax, %esi
	shrl	$31, %esi
	addl	%eax, %esi
	sarl	%esi
	movl	$str, %edi
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
