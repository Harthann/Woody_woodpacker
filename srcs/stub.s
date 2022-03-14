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
size_of_section dq 0xffffffffffffffff
offset_of_section dq 0xffffffffffffffff
offset_where_to_write dq 0xffffffffffffffff

_payload:
	pop r8
	sub r8, 5
	push rdx
	mov rax, 1
	mov rdi, 1
	lea rsi, [rel msg]
	mov rdx, 14
	syscall

setup_decrypt:
	mov r11, [rel key]
	mov rdi, [rel offset_of_section]
	mov rsi, [rel size_of_section]
	mov r10, [rel offset_where_to_write]
	mov r12, r8
	sub r12, r10
	add rdi, r12
	add rsi, rdi
	
decrypt:		
	xor [rdi], r11
	add rdi, 8
	cmp rdi, rsi
	jng decrypt	

	pop rdx
	mov rax, [rel where_to_jump]
	add rax, r8
	jmp rax

_end_payload:
