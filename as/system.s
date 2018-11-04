.global syscall
syscall:
	mov %rdi, %rax
	mov %rsi, %rdi
	mov %rdx, %rsi
	mov %rcx, %rdx
	mov %r8, %rcx
	mov %r9, %r8
	mov 8(%rsp), %r9
	syscall
	ret

.global _start
_start:
	mov (%rsp), %rdi
	lea 8(%rsp), %rsi
	call main
	mov %rax, %rdi
	mov $60, %eax
	syscall
