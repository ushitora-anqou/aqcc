non_global_func:
	mov $20, %eax
	ret

.global test002
test002:
	call non_global_func
	ret
