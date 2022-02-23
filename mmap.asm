

global _start

	
_start:	mov	rdi, 0x0
	mov	rsi, 1
	mov	rdx, 6
	mov	r10, 34
	mov	r8, 0
	mov	r9, 0
	mov	rax, 9
	syscall
	mov	byte [rax], 204
	jmp	rax
