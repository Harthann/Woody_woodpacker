#ifndef WOODY_H
#define WOODY_H

#define PAYLOAD_LEN (long unsigned)(_falseend - _payload)

extern void _payload(void);
extern void _falseend(void);
extern void _mybegin(void);

typedef struct {
	char			*file;
	unsigned int	size;
}					t_file;

typedef struct {
	char			call[5];
	unsigned long	addr;
	unsigned long	key;
	unsigned long	encrypt_offset;
	unsigned long	text_len;
	char			msg[16];
	char			shellcode[44];
}	__attribute__((packed))				t_payload;




#endif
