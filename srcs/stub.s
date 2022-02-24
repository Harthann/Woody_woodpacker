bits 64

SECTION .TEXT
	global _payload
	global _falseend


_payload:


	mov rax,1
	mov rdi, 1
	lea rsi,[rel msg]
	mov rdx, 15
	syscall
    mov rax, qword 0xffffffffffffffff


msg db `....WOODY.....\n`, 0x0
addr dq 0xffffffffffffffff

_falseend:
