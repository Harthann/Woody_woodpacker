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


_payload:
	push rdx
	mov rax, 1
	mov rdi, 1
	lea rsi, [rel msg]
	mov rdx, 14
	syscall

	; copy call of cypher_section

 ;ad3:	48 89 7d e8          	mov    QWORD PTR [rbp-0x18],rdi
 ;ad7:	48 89 75 e0          	mov    QWORD PTR [rbp-0x20],rsi
 ;adb:	48 89 55 d8          	mov    QWORD PTR [rbp-0x28],rdx
 ;adf:	48 8b 45 e8          	mov    rax,QWORD PTR [rbp-0x18]
 ;ae3:	48 8b 50 08          	mov    rdx,QWORD PTR [rax+0x8]
 ;ae7:	48 8b 45 e0          	mov    rax,QWORD PTR [rbp-0x20]
 ;aeb:	48 8b 40 18          	mov    rax,QWORD PTR [rax+0x18]
 ;aef:	48 01 d0             	add    rax,rdx
 ;af2:	48 89 45 f8          	mov    QWORD PTR [rbp-0x8],rax
 ;af6:	c7 45 f4 00 00 00 00 	mov    DWORD PTR [rbp-0xc],0x0
 ;afd:	eb 2d                	jmp    b2c <cypher_section+0x61>
 ;aff:	8b 45 f4             	mov    eax,DWORD PTR [rbp-0xc]
 ;b02:	48 63 d0             	movsxd rdx,eax
 ;b05:	48 8b 45 f8          	mov    rax,QWORD PTR [rbp-0x8]
 ;b09:	48 01 d0             	add    rax,rdx
 ;b0c:	48 8b 00             	mov    rax,QWORD PTR [rax]
 ;b0f:	8b 55 f4             	mov    edx,DWORD PTR [rbp-0xc]
 ;b12:	48 63 ca             	movsxd rcx,edx
 ;b15:	48 8b 55 f8          	mov    rdx,QWORD PTR [rbp-0x8]
 ;b19:	48 01 ca             	add    rdx,rcx
 ;b1c:	48 33 45 d8          	xor    rax,QWORD PTR [rbp-0x28]
 ;b20:	48 89 02             	mov    QWORD PTR [rdx],rax
 ;b23:	8b 45 f4             	mov    eax,DWORD PTR [rbp-0xc]
 ;b26:	83 c0 08             	add    eax,0x8
 ;b29:	89 45 f4             	mov    DWORD PTR [rbp-0xc],eax
 ;b2c:	8b 45 f4             	mov    eax,DWORD PTR [rbp-0xc]
 ;b2f:	48 63 d0             	movsxd rdx,eax
 ;b32:	48 8b 45 e0          	mov    rax,QWORD PTR [rbp-0x20]
 ;b36:	48 8b 40 20          	mov    rax,QWORD PTR [rax+0x20]
 ;b3a:	48 39 c2             	cmp    rdx,rax
 ;b3d:	72 c0                	jb     aff <cypher_section+0x34>
 ;b3f:	90                   	nop
 ;b40:	90                   	nop
 ;b41:	5d                   	pop    rbp
 ;b42:	c3                   	ret    


	
	; copy call of cypher_section

	pop rdx
	pop r12
	mov rax, [rel where_to_jump]
	add rax, r12
	sub rax, 5


	jmp rax

_end_payload:
