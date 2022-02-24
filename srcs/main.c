#include <stdio.h>
#include <woody.h>


#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>

int handle_init_error_or_return_fd(int argc, char **argv)
{
	char should_we_exit;
	int fd_file;

	should_we_exit = NO;


	if (argc < 2) {
		printf("not_enough_argument_for_this_to_work\n");
		should_we_exit = YES;
	}
	if (should_we_exit == NO)
		if ((fd_file = open(argv[1], O_RDONLY)) == -1)
		{
			printf("this argument is not a regular file that can be accessed and writed by you\n");
			should_we_exit = YES;
		}
	if (should_we_exit)
		exit(YES);
	return (fd_file);
}

uint64_t	get_the_file_length(int fd)
{
	uint64_t file_length;
	file_length = lseek(fd, 0, SEEK_END);
	return file_length;
}

char is_this_an_elf64_file(t_file_informations *file_given) {
	uint32_t tmp;
	tmp = (*(uint32_t *)file_given->mmaped);
	if (file_given->length >= 64 && tmp == MAGIC_ELF_BYTES_REVERSED)
		if (file_given->mmaped[4] == 0x2)
			return YES;
	return NO;
}

t_header_elf64 *get_header_table_of_elf_file(t_file_informations *file_given)
{
	uint16_t 	number_of_headers;
	uint64_t	offset_where_headers_start;
	uint16_t	tmp;

	tmp = -1;
	file_given->number_of_headers = (uint16_t)file_given->mmaped[56];
	offset_where_headers_start = (uint64_t)file_given->mmaped[32];
	file_given->headers = (t_header_elf64*)malloc(sizeof(t_header_elf64) * file_given->number_of_headers);
	while(++tmp < file_given->number_of_headers)
	{
		file_given->headers[tmp] = *(t_header_elf64*)(&file_given->mmaped[offset_where_headers_start + (sizeof(t_header_elf64) * tmp)]);
		printf("header_type 0x%x | offset 0x%lx | file_size 0x%lx | memoire_size 0x%lx \n", file_given->headers[tmp].type, file_given->headers[tmp].offset, file_given->headers[tmp].file_size, file_given->headers[tmp].memory_size );
	}

}

int main(int argc, char **argv)
{
	t_file_informations file_given;
	int	size_payload;

	size_payload = _start_payload - _end_payload;
	file_given.fd =	handle_init_error_or_return_fd(argc, argv);
	file_given.length = get_the_file_length(file_given.fd);
	file_given.mmaped = (char*)mmap(
			NULL,	 		//where to put mmaped area
			file_given.length,
			PROT_READ | PROT_WRITE,		//I want to read and write in the memory mmaped	
			MAP_PRIVATE,			//I don't want that others process can read what I do
			file_given.fd, 				
			0); 				//offset
	if (file_given.mmaped == NULL)
		return(1);
	if (is_this_an_elf64_file(&file_given) == NO)
		return(1);
	get_header_table_of_elf_file(&file_given);

	//	_payload();
	//	_mmap();
}

