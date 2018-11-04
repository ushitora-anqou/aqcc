.global _start
_start:
	call main
	mov %rax, %rdi
	mov $60, %eax
	syscall
