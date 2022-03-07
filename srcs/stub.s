bits 64

SECTION .TEXT
	global _payload
	global _falseend
	global _mybegin

_payload:
    call _mybegin 

addr dq 0xffffffffffffffff
key  dq 0xffffffffffffffff
encrypt_offset dq 0xffffffffffffffff
text_len dq 0xffffffffffffffff

msg db `....WOODY.....\n`, 0x0

_mybegin:
    ; Message print
    push rdx
	mov rax,1
	mov rdi, 1
	lea rsi,[rel msg]
	mov rdx, 15
	syscall
    ; Decrypt
	
	pop rdx
	pop r11
	sub r11, 5
	mov rsi, [rel encrypt_offset]
	add rsi, r11
	mov rax, 1
	mov rdi, 2
	pop rdx
	mov rdx, 8
	syscall
	mov rax, 60
	syscall

    ; Prepare base program flow
    pop rdx
    pop r11
    sub r11, 5
    mov rax, [rel addr]
    add rax, r11
    jmp rax

_falseend:
