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

int	is_there_a_place_beetween_those_two_section_to_put_my_payload(t_header_elf64 *first, t_header_elf64 *second, int size_payload)
{
	return ((second->offset - (first->offset + first->memory_size)) >= size_payload);
}

char *get_address_of_header_segment_in_mmaped_file_given_with_his_number(t_file_informations *file_given, int number) {
	uint64_t offset_where_headers_start = (uint64_t)file_given->mmaped[32];
	return (&file_given->mmaped[offset_where_headers_start + sizeof(t_header_elf64) * number]);
}

t_header_to_inject choose_which_section_to_inject_by_reading_file_header(t_file_informations *file_given, int size_payload)
{
	int counter = -1;
	t_header_elf64 last_header = {0};
	t_header_elf64 current_readed_header = {0};
	t_header_to_inject no_segment_to_inject = {NO};
	file_given->number_of_headers = (uint16_t)file_given->mmaped[56];
	while(++counter < file_given->number_of_headers)
	{
		current_readed_header = *(t_header_elf64*)get_address_of_header_segment_in_mmaped_file_given_with_his_number(file_given, counter);
		if (current_readed_header.type == last_header.type)
			if (is_there_a_place_beetween_those_two_section_to_put_my_payload(&last_header, &current_readed_header, size_payload))
			{
				t_header_to_inject header_to_inject;
				header_to_inject.header = last_header;
				header_to_inject.address_of_header_in_mmaped_file_given = get_address_of_header_segment_in_mmaped_file_given_with_his_number(file_given, counter - 1);
				return header_to_inject;
			}
		last_header = current_readed_header;
	}
	return (no_segment_to_inject);
}

void	inject_code_into_file_given(t_file_informations *file_given, t_header_to_inject *header_of_segment_to_inject, int size_payload)
{
	char *where_to_write = &file_given->mmaped[header_of_segment_to_inject->header.offset + header_of_segment_to_inject->header.memory_size];
	header_of_segment_to_inject->header.memory_size += size_payload;
	while(size_payload--)
		where_to_write[size_payload] = ((char*)_start_payload)[size_payload];
	*(t_header_elf64*)header_of_segment_to_inject->address_of_header_in_mmaped_file_given = header_of_segment_to_inject->header;
	uint64_t tmp = (uint64_t)file_given->mmaped[0x18];
	file_given->mmaped[0x18] = (uint64_t)(where_to_write - file_given->mmaped);
	//manque d'ajouter la jump instruction pour continuer le deroulement normal du programme
}

int main(int argc, char **argv)
{
	t_file_informations file_given;
	int	size_payload;

	size_payload = (_end_payload - _start_payload);
	file_given.fd =	handle_init_error_or_return_fd(argc, argv);
	file_given.length = get_the_file_length(file_given.fd);
	file_given.mmaped = (char*)mmap(
			NULL,	 		//where to put mmaped area
			file_given.length,
			PROT_READ | PROT_WRITE,		//I want to read and write in the memory mmaped	
			MAP_PRIVATE,			//I don't want that others process can read what I do
			file_given.fd, 				
			0); 				//offset
	close(file_given.fd);
	if (file_given.mmaped == NULL || file_given.length == 0)
		return(1);
	if (is_this_an_elf64_file(&file_given) == NO)
		return(1);
	t_header_to_inject header_of_section_to_inject = choose_which_section_to_inject_by_reading_file_header(&file_given, size_payload);
	if (header_of_section_to_inject.address_of_header_in_mmaped_file_given == NULL)
		return(1);
	printf("we inject code into %dth section type\n", header_of_section_to_inject.header.type);
	inject_code_into_file_given(&file_given, &header_of_section_to_inject, size_payload);

	int new_file_fd = open("./sample.new", O_WRONLY | O_CREAT);
	write(new_file_fd, file_given.mmaped, file_given.length);
	close(new_file_fd);


	//	_payload();
}

