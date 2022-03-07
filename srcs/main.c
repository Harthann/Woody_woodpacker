#include <stdio.h>
#include <woody.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <elf.h>
#include <string.h>

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
///write(2, payload, PAYLOAD_LEN);

	ehdr->e_entry = phdr->p_vaddr + phdr->p_memsz;
	phdr->p_memsz += PAYLOAD_LEN;
	phdr->p_filesz += PAYLOAD_LEN;
	phdr->p_flags |= PF_X;
	return payload;
}

void create_file(t_file file)
{
	int fd;

	fd = open("woody", O_CREAT | O_WRONLY | O_TRUNC, 0667);
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

void encrypt(char *str, unsigned int size, unsigned long key)
{
	for (unsigned int i = 0; i < 8; i++)
	{
		((unsigned long*)str)[i] = 'A';
	}

	(void)str;
	(void)size;
	(void)key;

}

unsigned long random_key(void)
{
	int fd;
	unsigned long key;

	fd = open("/dev/urandom", O_RDONLY);
	if (fd < 0)
		return 0xdeadbeef;
	read(fd, (char*)&key, 8);
	printf("Randomized key: %ld\n", key);
	close(fd);
	return key;
}

void elfinfo(t_file file)
{
	Elf64_Ehdr 	*ehdr = NULL;	//	File header
	Elf64_Phdr 	*phdr = NULL;	//	Program header
	Elf64_Phdr 	*target = NULL;	//	Header of segment to inject
	Elf64_Shdr 	*shdr = NULL;	//	Section header
	char 		*shst = NULL;	//	Section header strings
	t_payload   *payload = NULL;

	ehdr = (Elf64_Ehdr*)file.file;
	phdr = (Elf64_Phdr*)(file.file + ehdr->e_phoff);
	shdr = (Elf64_Shdr*)(file.file + ehdr->e_shoff);
	shst = (char *)(file.file + shdr[ehdr->e_shstrndx].sh_offset);

	write(1, ehdr->e_ident, 4);
	write(1, "\n", 1);

	/* Printing PT_LOAD section info */
	printf("%8s | %8s | %8s | %7s | %7s | %7s\n", "id", "type", "offset", "filesz" ,"memsz", "space");
	for (int i = 0; i < ehdr->e_phnum; i++)
	{
		if (phdr[i].p_type == PT_LOAD)
		{
			printf("%8d | %8x | %8lx | %7ld | %7ld | %7ld\n", i, phdr[i].p_type, phdr[i].p_offset, phdr[i].p_filesz ,phdr[i].p_memsz, (i + 1 == ehdr->e_phnum) ? 0 : phdr[i + 1].p_offset - phdr[i].p_offset - phdr[i].p_filesz);
			if (!target && phdr[i + 1].p_offset - phdr[i].p_offset - phdr[i].p_filesz > PAYLOAD_LEN)
				target = phdr + i;
		}
	}
	printf("\nSegment selected:\n%8.0d | %8x | %8lx | %7ld | %7ld\n", 0, target->p_type, target->p_offset, target->p_filesz ,target->p_memsz);
	/* Writing payload inside target section */
	payload = write_payload(file, ehdr, target);

	/* Print sections info */
	printf("Sections info:\n%20s | %15s | %15s | %15s | %15s\n", "name", "type", "offset", "size" ,"addr");
	for (int i = 0; i < ehdr->e_shnum; i++)
	{
		//printf("%8s\n",  shst[shdr[i].sh_name]);
		printf("%20s | %10x | %15lx | %15lx | %15lx\n", shst + shdr[i].sh_name, shdr[i].sh_type, shdr[i].sh_offset, shdr[i].sh_size, shdr[i].sh_addr);
		if (strcmp(shst + shdr[i].sh_name, ".text") == 0)
		{
			//encrypt 
			payload->key = random_key();
			encrypt(file.file + shdr[i].sh_offset, shdr[i].sh_size, payload->key);
			payload->encrypt_offset = shdr[i].sh_addr;
			payload->text_len = shdr[i].sh_size;
		}

	}
	/* Encrypt .text section */

	/* Log payload to debug	*/
	int logfd = open("paylog", O_CREAT | O_WRONLY | O_TRUNC, 0667);
	write(logfd, (char*)payload, sizeof(t_payload));


	/* Create file and writing our packed binary */
	create_file(file);
}

int main(int ac, char **av)
{
	t_file file;

	if (ac != 2)
	{
		write(2, "I need something to pack, thanks\n", 32);
		return (0);
	}
	printf("Payload len: %ld\n", PAYLOAD_LEN);
	file = load_file(av[1]);
	if (!file.file)
	{
		write(2, "Failed to load file\n",20);
		return 0;
	}
	elfinfo(file);
	munmap(file.file, file.size); 
	return 0;
}

