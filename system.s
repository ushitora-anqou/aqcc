.global syscall_wrap
syscall_wrap:
	mov %rdi, %rax
	mov %rsi, %rdi
	mov %rdx, %rsi
	mov %rcx, %rdx
	mov %r8, %rcx
	mov %r9, %r8
	mov 8(%rsp), %r9
	syscall
	ret
