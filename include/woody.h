#ifndef WOODY_H
#define WOODY_H

#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <elf.h>
#include <string.h>
#include <stdio.h>

#define PAYLOAD_LEN (long unsigned)(_falseend - _payload)
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
	long long       key[2];
	unsigned long	encrypt_offset;
//    char            tmp[80];
	unsigned long	blocks;
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

/************************/
/*      PROTOTYPE       */
/************************/

/*      Elfind.c   */
Elf64_Phdr 	*find_target_segment(Elf64_Phdr *phdr, Elf64_Ehdr *ehdr);
Elf64_Shdr 	*find_section(Elf64_Shdr *shdr, int shnum, char *shst, char *section_name );

/*      file_handler.c    */
t_file load_file(char *path);
t_payload *write_payload(t_file file, Elf64_Ehdr *ehdr, Elf64_Phdr *phdr);
void create_file(t_file file);


#endif
