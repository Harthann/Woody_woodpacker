#ifndef WOODY_H
#define WOODY_H

#include <stdint.h>


extern void _start_payload(void);
extern void _end_payload(void);

#define YES 1
#define NO 0
#define MAGIC_ELF_BYTES_REVERSED 0x464c457f


typedef struct 	s_header_elf64	{
	uint32_t	        type;
	uint32_t	        flags;
	uint64_t		offset;
	uint64_t		virtual_addr;
	uint64_t		physical_addr;
	uint64_t	        file_size;
	uint64_t	        memory_size;
	uint64_t	        align;
} 		t_header_elf64;

typedef struct s_file_informations 
{
	int		fd;
	char 		*mmaped;
	uint64_t 	length;
	t_header_elf64 	*headers;
	uint16_t number_of_headers;	
} 		t_file_informations;


#endif
