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

int	is_there_a_place_beetween_those_two_segment_to_put_my_payload(t_header_elf64 *first, t_header_elf64 *second, int size_payload)
{
	return ((second->offset - (first->offset + first->memory_size)) >= size_payload);
}

char *get_address_of_header_segment_in_mmaped_file_given_with_his_number(t_file_informations *file_given, int number) {
	uint64_t offset_where_headers_start = (uint64_t)file_given->mmaped[0x20];
	return (&file_given->mmaped[offset_where_headers_start + sizeof(t_header_elf64) * number]);
}

t_header_to_inject choose_which_segment_to_inject_by_reading_file_header(t_file_informations *file_given, int size_payload)
{
	int counter = -1;
	t_header_elf64 last_header = {0};
	t_header_elf64 *current_readed_header = NULL;
	t_header_to_inject no_segment_to_inject = {NO};
	file_given->number_of_headers = (uint16_t)file_given->mmaped[0x38];
	while(++counter < file_given->number_of_headers)
	{
		current_readed_header = (t_header_elf64*)get_address_of_header_segment_in_mmaped_file_given_with_his_number(file_given, counter);
		if (current_readed_header->type == last_header.type)
			if (is_there_a_place_beetween_those_two_segment_to_put_my_payload(&last_header, current_readed_header, size_payload))
			{
				current_readed_header->flags |=  PF_W;
				t_header_to_inject header_to_inject;
				header_to_inject.header = last_header;
				header_to_inject.address_of_header_in_mmaped_file_given = get_address_of_header_segment_in_mmaped_file_given_with_his_number(file_given, counter - 1);
				return header_to_inject;
			}
		last_header = *current_readed_header;
	}
	return (no_segment_to_inject);
}

void	inject_code_into_file_given(t_file_informations *file_given, t_header_to_inject *header_of_segment_to_inject, int size_payload, t_things_to_write_into_binary *struct_to_write)
{
	uint64_t offset_where_to_write = header_of_segment_to_inject->header.offset + header_of_segment_to_inject->header.memory_size;
	char *where_to_write = &file_given->mmaped[offset_where_to_write];
	uint64_t program_entry_origin = *(uint64_t*)&file_given->mmaped[0x18];
	offset_where_to_write = header_of_segment_to_inject->header.virtual_addr + header_of_segment_to_inject->header.memory_size;
	header_of_segment_to_inject->header.memory_size += size_payload;
	header_of_segment_to_inject->header.file_size += size_payload;
	header_of_segment_to_inject->header.flags |= PF_X;
	uint64_t how_many_to_jump = (uint64_t)(program_entry_origin - offset_where_to_write);
	struct_to_write->where_to_jump = how_many_to_jump;
	struct_to_write->offset_where_to_write = offset_where_to_write;
	void *position = 0;
	while(size_payload--)
	{
		position = (void*)_start_payload + size_payload;
		if ((position) == (void*)&where_to_jump)
			*(t_things_to_write_into_binary*)(&where_to_write[size_payload]) = *struct_to_write;
		else
			where_to_write[size_payload] = ((char*)_start_payload)[size_payload];
	}		
	*(t_header_elf64*)header_of_segment_to_inject->address_of_header_in_mmaped_file_given = header_of_segment_to_inject->header;
	*(uint64_t*)&file_given->mmaped[0x18] = (uint64_t)(offset_where_to_write);
	printf("entry_program %lx, origin %lx, jump of %lx and should be %lx\n", *(uint64_t*)&file_given->mmaped[0x18], program_entry_origin, how_many_to_jump,program_entry_origin - *(uint64_t*)&file_given->mmaped[0x18]);
}

uint64_t my_random() 
{
	int fd = open("/dev/urandom", O_RDONLY);
	if (fd == -1)
		return 0xC0FEEFEE;
	char key[8];
	read(fd, key, 8);
	close(fd);
	return (*(uint64_t*)key);
}

int	ft_strcmp(char *s1, char *s2)
{
	int i;

	i = 0;
	while (s1[i] == s2[i] && s1[i] != '\0')
		i++;
	return (s1[i] - s2[i]);
}

Elf64_Shdr find_header_of_text_section(t_file_informations *file_given)
{
	uint64_t offset_section_table = *(uint64_t*)&file_given->mmaped[0x28];
	uint16_t number_of_section = *(uint16_t*)&file_given->mmaped[0x3c];
	uint16_t index_of_the_section_header_table_entry_that_contains_the_section_names = *(uint16_t*)&file_given->mmaped[0x3e];
	Elf64_Shdr header_of_section_name = *(Elf64_Shdr*)&file_given->mmaped[offset_section_table + sizeof(Elf64_Shdr) * index_of_the_section_header_table_entry_that_contains_the_section_names];
	char *section_name = &file_given->mmaped[header_of_section_name.sh_offset];
	Elf64_Shdr *header_of_section = NULL;
	char *text = ".text";
	char *tmp;
	while (number_of_section--)
	{
		header_of_section = (Elf64_Shdr*)&file_given->mmaped[offset_section_table + (sizeof(Elf64_Shdr) * number_of_section)];
		tmp = &section_name[header_of_section->sh_name];
		//	header_of_section->sh_flags |= SHF_COMPRESSED;
		//	https://android.googlesource.com/platform/external/elfutils/+/android-7.0.0_r1/libelf/libelf.h
		//	ligne 314
		if (ft_strcmp(text, tmp) == 0)
		{
			header_of_section->sh_flags |= 0x1;
			return *header_of_section;
		}
	}
	return *header_of_section;
}

void	cypher_section(t_file_informations *file_given, Elf64_Shdr *header_to_cypher, uint64_t key)
{

	char *to_cypher = &file_given->mmaped[header_to_cypher->sh_offset];
	printf("cypher at %lx\n",header_to_cypher->sh_offset);
	int i = 0;

	while(i < header_to_cypher->sh_size)
	{
		*(uint64_t*)&to_cypher[i] ^= key;
		i += sizeof(uint64_t);
	}
}

char get_to_know_if_file_is_cut(t_file_informations *file_given)
{
	t_header_elf64 *header_segment;
	int segment_number = -1;

	file_given->number_of_headers = (uint16_t)file_given->mmaped[0x38];
	while (++segment_number < file_given->number_of_headers)
	{
		header_segment = (t_header_elf64*)get_address_of_header_segment_in_mmaped_file_given_with_his_number(file_given, segment_number); 
		if (header_segment->offset + header_segment->file_size > file_given->length)
			return YES;
	}
	return NO;
}

char error_in_start_up(t_file_informations *file_given) {
	if (file_given->mmaped == NULL || file_given->length == 0)
	{
		printf("file empty or no allocation possible\n");
		return(YES);
	}
	if (is_this_an_elf64_file(file_given) == NO)
	{
		printf("wrong file type\n");
		return(YES);
	}
	if (get_to_know_if_file_is_cut(file_given) == YES)
	{
		printf("file been cut\n");
		return (YES);
	}
	return NO;
}

int main(int argc, char **argv)
{
	t_file_informations file_given;
	int	size_payload = (_end_payload - _start_payload);
	t_things_to_write_into_binary things_to_change_in_stub = {0};
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
	if (error_in_start_up(&file_given) == YES && file_given.mmaped != NULL)
	{
		munmap(file_given.mmaped, file_given.length);
		return (1);
	}
	things_to_change_in_stub.key = random();
	printf("key to decipher .text section 0x%lx\n", things_to_change_in_stub.key);
	t_header_to_inject header_of_section_to_inject = choose_which_segment_to_inject_by_reading_file_header(&file_given, size_payload);
	if (header_of_section_to_inject.address_of_header_in_mmaped_file_given == NULL)
		return(1);
	printf("we inject code into %dth section type\n", header_of_section_to_inject.header.type);
	Elf64_Shdr header_of_section_to_cypher = find_header_of_text_section(&file_given);
	things_to_change_in_stub.offset_of_section = header_of_section_to_cypher.sh_addr;
	things_to_change_in_stub.size_of_section = header_of_section_to_cypher.sh_size;

	cypher_section(&file_given, &header_of_section_to_cypher, things_to_change_in_stub.key);
	inject_code_into_file_given(&file_given, &header_of_section_to_inject, size_payload, &things_to_change_in_stub);
	int new_file_fd = open("./sample.new", O_RDWR | O_CREAT, 0777);
	write(new_file_fd, file_given.mmaped, file_given.length);
	munmap(file_given.mmaped, file_given.length);
	close(new_file_fd);

	return (0);
}

