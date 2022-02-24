SECTION .TEXT
	global _start_payload
	global _end_payload


_start_payload:
	mov rax,1
	mov rdi, 1
	lea rsi,[rel msg]
	mov rdx, 15
	syscall
	ret

msg db `....WOODY.....\n`, 0x0

_end_payload:
