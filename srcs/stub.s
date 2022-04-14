bits 64

section .text
	global _payload
	global _falseend
	global _mybegin

_payload:
	call _mybegin 

addr dq 0xffffffffffffffff
key  db `................`
encrypt_offset dq 0xffffffffffffffff
blocks dq 0xffffffffffffffff
pagelen dq 0xffffffffffffffff
pagediff dq 0xffffffffffffffff

msg db `....WOODY....\n`

;/*	Stack MAP
; *	
; *	rip
; *	rsp + 16	=>	saved rdx
; *	rsp + 8		=>	mapped addr of payload
; *	rsp			=> .text location
;*/
_mybegin:
	; Calling conventions
	push rdx
	sub rsp, 16

	; Message print
	mov rax,1
	mov rdi, 1
	lea rsi, [rel msg]
	mov rdx, 14
	syscall

	;/************/
	;/*  DECRYPT */
	;/************/

	; Save our entrypoint on the stack
	mov rax, [rsp+8+16]
	sub rax, 5
	mov [rsp+8], rax
	
	; Changing rip to match old entrypoint
	add rax, [rel addr]				; Align old entry to mapped memory using our entry
	mov [rsp+8+16], rax

	; Get encrypted text section location 
	mov rsi, [rel encrypt_offset]	; .text section offset
	add rsi, rax					; Add to old entry addr the offset of .text section
	mov [rsp], rsi

	; Find page starts
	mov rdi, [rsp+8]
	mov rsi, [rel pagediff]
	sub rdi, rsi
	mov rsi, [rel pagelen]
	mov rdx, 7
	mov rax, 10
	syscall
	; mprotect page to allow writeable

	mov r13, 0			;   r13 will be our loop counter
.block_encryption:
	;/*  Loads key in 32 bits registers */
	lea rdx, [rel key]
	mov r8d, [rdx]		;   k0
	mov r9d, [rdx+4]	;   k1
	mov r10d, [rdx+8]	;   k2
	mov r11d, [rdx+12]	;   k3

	;   /*  Loads message   */
	mov rdi, [rsp]						;   base text
	mov esi, DWORD [rdi + r13 * 8]		;   First 4 bytes
	mov edi, DWORD [rdi + r13 * 8 + 4]	;   4 last bytes
	; V1 = edi  V0 = esi

	mov ecx, 0xc6ef3720	 ;   sum
	mov r12, 0
.tea_loop:
	; v1 -= ((v0<<4) + k2) ^ (v0 + sum) ^ ((v0>>5) + k3);
	mov edx, esi
	shl edx, 4
	add edx, r10d   ;   add k2
	lea eax, [esi+ecx]
	xor edx, eax
	mov eax, esi
	shr eax, 5
	add eax, r11d   ;   add k3
	xor edx, eax
	sub edi, edx

	; v0 -= ((v1<<4) + k0) ^ (v1 + sum) ^ ((v1>>5) + k1);
	mov edx, edi
	shl edx, 4
	add edx, r8d		;   add k0
	lea eax, [edi+ecx]
	xor edx, eax
	mov eax, edi
	shr eax, 5
	add eax, r9d		;   add k1
	xor edx, eax
	sub esi, edx

	;   Advance the magic constant and check
	;   If 32 iteration has been done
	sub ecx, 0x9e3779b9	 ; sum -= delta
	inc r12
	cmp r12, 32			 ;   Check if all iterations have been done
	jne .tea_loop

	mov r14, [rsp]	  ;   base text
	mov DWORD [r14 + r13 * 8], esi	;   First 4 bytes
	mov DWORD [r14 + r13 * 8 + 4], edi  ;   4 last bytes
	
	inc r13
	mov r14, [rel blocks]
	cmp r13, r14
	jne .block_encryption


.end:
	mov rdi, [rsp+8]
	mov rsi, [rel pagediff]
	sub rdi, rsi
	mov rsi, [rel pagelen]
	mov rdx, 5
	mov rax, 10
	syscall

	add rsp, 16
	pop rdx
	ret

_falseend:
