.global _start
_start:
	mov $60, %eax
	xor %rdi, %rdi
	syscall
