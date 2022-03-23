#include "woody.h"

t_file load_file(char *path)
{
	int fd;
	t_file file = {0};

	fd = open(path, O_RDONLY);
	if (fd < 0)
		return file;
	file.size = lseek(fd, 0, SEEK_END);
	printf("File loaded length: %d\n", file.size);
	lseek(fd, 0, SEEK_SET);
	file.file = mmap(NULL, file.size + 1, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
	close(fd);
	return file;
}

t_payload *write_payload(t_file file, Elf64_Ehdr *ehdr, Elf64_Phdr *phdr)
{
	unsigned int offset;
	t_payload *payload;

	offset = phdr->p_offset + phdr->p_memsz;
	payload = (t_payload*)(file.file + offset);
	for (unsigned int i = 0; i < PAYLOAD_LEN; i++)
		file.file[offset + i] = ((char*)_payload)[i];
	payload->addr = (unsigned long)(ehdr->e_entry - (phdr->p_vaddr + phdr->p_memsz));

	ehdr->e_entry = phdr->p_vaddr + phdr->p_memsz;
	phdr->p_memsz += PAYLOAD_LEN;
	phdr->p_filesz += PAYLOAD_LEN;
	phdr->p_flags |= PF_X;
	return payload;
}


void create_file(t_file file)
{
	int fd;

	fd = open("woody", O_CREAT | O_WRONLY | O_TRUNC, 0755);
	if (fd > 0)
	{
		printf("Opened file with fd: %d\n", fd);
		printf("Writing new binary\n");
		write(fd, file.file, file.size);
		close(fd);
	}
	else
		printf("Failed to open file: errno\n");
}


