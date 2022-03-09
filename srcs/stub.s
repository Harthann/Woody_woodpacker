bits 64

section .text
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
	; Getting our entrypoint on mapped memory
	pop r11
	sub r11, 5

	mov r12, [rel addr]				; Old entrypoint offset base on our entry
	add r12, r11					; Align old entry to mapped memory using our entry
	; Get encrypted text section location 
	mov rsi, [rel encrypt_offset]	; .text section offset
	add rsi, r12					; Add to old entry addr the offset of .text section

	; Print theorically crypted section
	mov rax, 1
	mov rdi, 2
	pop rdx
	mov rdx, [rel text_len]
	syscall

	; exit for test only
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
