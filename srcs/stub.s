SECTION .TEXT
	global _start_payload
	global _end_payload
	global where_to_jump
	global key

_start_payload:
	call _payload 

msg db `....WOODY....`, 10, 0x0
where_to_jump dq 0xffffffffffffffff
key dq 0xffffffffffffffff

_payload:
	push rdx
	mov rax, 1
	mov rdi, 1
	lea rsi, [rel msg]
	mov rdx, 16
	syscall
	pop rdx
	pop r12
	mov rax, [rel where_to_jump]
	add rax, r12
	sub rax, 5
	jmp rax	 

_end_payload:
