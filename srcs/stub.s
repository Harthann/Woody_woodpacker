SECTION .TEXT
	global _payload
	global _falseend


_payload:
	mov rax,1
	mov rdi, 1
	lea rsi,[rel msg]
	mov rdx, 15
	syscall
	ret

msg db `....WOODY.....\n`, 0x0

_falseend:
