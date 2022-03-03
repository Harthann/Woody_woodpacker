SECTION .TEXT
	global _start_payload
	global _end_payload
	global where_to_jump

_start_payload:
	call _payload 

msg db `....WOODY......`, 10, 0x0
where_to_jump dq 0xffffffffffffffff

_payload:
	pop r12
	push rdx
	mov rax, 1
	mov rdi, 1
	lea rsi, [rel msg]
	mov rdx, 16
	syscall
	mov r11, where_to_jump
	add r12, r11
	sub r12, 0x1e
	pop rdx
	jmp rbp	 

_end_payload:
