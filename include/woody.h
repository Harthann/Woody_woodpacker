#ifndef WOODY_H
#define WOODY_H

#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <elf.h>
#include <string.h>

#define PAYLOAD_LEN (long unsigned)(_falseend - _payload)
//#define SHELLCODE_LEN (long unsigned)(_falseend - _mybegin)
#define SHELLCODE_LEN 44

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
	char			shellcode[SHELLCODE_LEN];
}	__attribute__((packed))				t_payload;

typedef struct {
	/* Elf file headers */
	Elf64_Ehdr	*ehdr;			// File header
	Elf64_Phdr	*phdr;			// Program header
	Elf64_Shdr	*shdr;			// Section header
	char 		*shst;			// Section header strings

	/* Injection info */
	Elf64_Phdr 	*starget;		// Header of segment to inject
	unsigned long old_entry;	// Old entrypoint
} Elf64_info;

#endif
