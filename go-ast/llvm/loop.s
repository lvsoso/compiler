	.text
	.file	"loop.ll"
	.globl	main                    # -- Begin function main
	.p2align	4, 0x90
	.type	main,@function
main:                                   # @main
	.cfi_startproc
# %bb.0:
	pushq	%rax
	.cfi_def_cfa_offset 16
	.p2align	4, 0x90
.LBB0_1:                                # %Dest
                                        # =>This Inner Loop Header: Depth=1
	movl	$str, %edi
	xorl	%eax, %eax
	callq	printf
	jmp	.LBB0_1
.Lfunc_end0:
	.size	main, .Lfunc_end0-main
	.cfi_endproc
                                        # -- End function
	.type	str,@object             # @str
	.section	.rodata,"a",@progbits
	.globl	str
str:
	.asciz	"hello, world!\n"
	.size	str, 15


	.section	".note.GNU-stack","",@progbits
