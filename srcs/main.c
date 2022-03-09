#include <stdio.h>
#include <woody.h>


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
		str[i] = 'A';  
		//((unsigned long*)str)[i] = 'A';
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

/* Searching segment with free space for injection using segment header */
Elf64_Phdr 	*find_target_segment(Elf64_Phdr *phdr, Elf64_Ehdr *ehdr)
{
	Elf64_Phdr *target;
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

	return target;
}

/* Find specific section section */
Elf64_Shdr 	*find_section(Elf64_Shdr *shdr, int shnum, char *shst, char *section_name )
{
	printf("Sections info:\n%20s | %15s | %15s | %15s | %15s\n", "name", "type", "offset", "size" ,"addr");
	for (int i = 0; i < shnum; i++)
	{
		printf("%20s | %10x | %15lx | %15lx | %15lx\n", shst + shdr[i].sh_name, shdr[i].sh_type, shdr[i].sh_offset, shdr[i].sh_size, shdr[i].sh_addr);
		if (strcmp(shst + shdr[i].sh_name, section_name) == 0)
		{
			printf("Found .text at offset: %lx\n", shdr[i].sh_offset);
			return shdr + i;
		}
	}

	return NULL;
}

/* Analyze elf then inject code
 * Return -1 on failure
 * Return 0 on success
*/
int code_inject(t_file file)
{
	Elf64_info 	info;
	Elf64_Shdr 	*text_section;
	t_payload   *payload = NULL;

	/* Gathering elf info */
	info.ehdr = (Elf64_Ehdr*)file.file;
	info.phdr = (Elf64_Phdr*)(file.file + info.ehdr->e_phoff);
	info.shdr = (Elf64_Shdr*)(file.file + info.ehdr->e_shoff);
	info.shst = (char *)(file.file + info.shdr[info.ehdr->e_shstrndx].sh_offset);
	info.old_entry = info.ehdr->e_entry;
	printf("Old entry addr %lx\n", info.old_entry);

	info.starget = find_target_segment(info.phdr, info.ehdr);
	if (info.starget == NULL)
		return -1;

	/* Writing payload inside target section */
	payload = write_payload(file, info.ehdr, info.starget);

	text_section = find_section(info.shdr, info.ehdr->e_shnum, info.shst ,".text");
	payload->key = random_key();
	encrypt(file.file + text_section->sh_offset, text_section->sh_size, payload->key);
	payload->encrypt_offset = text_section->sh_addr - info.old_entry;
	payload->text_len = text_section->sh_size;

	/* Log payload to debug	*/
	int logfd = open("paylog", O_CREAT | O_WRONLY | O_TRUNC, 0667);
	write(logfd, (char*)payload, sizeof(t_payload));

	/* Create file and writing our packed binary */
	create_file(file);
	return 0;
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
	code_inject(file);
	munmap(file.file, file.size); 
	return 0;
}

