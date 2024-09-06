	.file "test.s"
	.text
	.text
	.globl main
	.type main, @function
main:
	pushq %rbp
	movq %rsp, %rbp
	subq $8, %rsp
	movq $0, -8(%rbp)
	movq $5, %rax
	# Convert integer to string and print
	movq $10, %r9
	movq $0, %r10
	movq %rax, %rcx
	testq %rcx, %rcx
	jns .Lpositive_0
	negq %rcx
	movq $45, (%rsp)
	decq %rsp
	incq %r10
.Lpositive_0:
.Lconvert_loop_0:
	xorq %rdx, %rdx
	divq %r9
	addq $48, %rdx
	decq %rsp
	movb %dl, (%rsp)
	incq %r10
	testq %rax, %rax
	jnz .Lconvert_loop_0
	movq %r10, %rdx
	movq %rsp, %rsi
	movq $1, %rdi
	movq $1, %rax
	syscall
	movq $10, (%rsp)
	movq $1, %rdx
	movq %rsp, %rsi
	movq $1, %rdi
	movq $1, %rax
	syscall
	addq %r10, %rsp
	incq %rsp
	movq %rbp, %rsp
	popq %rbp
	ret
	.section .note.GNU-stack,"",@progbits
