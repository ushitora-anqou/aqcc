.global main
main:
	mov a(%rip), %eax
	ret

.global _start
_start:
	call main
	mov %rax, %rdi
	mov $60, %eax
	syscall

.data
a:
	.long 4
