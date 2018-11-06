.global _start
_start:
	call main
	mov %rax, %rdi
	mov $60, %eax
	syscall

non_global_func:
	mov $10, %eax
	ret

.global test001
test001:
	call non_global_func
	ret
