#include <stdio.h>
#include <woody.h>

long long random_key(void)
{
	int fd;
	long long key;

	fd = open("/dev/urandom", O_RDONLY);
	if (fd < 0)
		return 0xdeadbeef;
	read(fd, (char*)&key, 8);
	printf("Randomized key: 0x%llx\n", key);
	close(fd);
	return key;
}

void	block_encrypt( uint32_t v[2], const uint32_t k[4])
{
	uint32_t v0 = v[0], v1 = v[1];
	uint32_t sum = 0;
	uint32_t delta = 0x9e3779b9;

	for(int i =0; i < 32; i++)
	{
		sum += delta;
		v0 += ((v1<<4) + k[0]) ^ (v1 + sum) ^ ((v1>>5) + k[1]);
		v1 += ((v0<<4) + k[2]) ^ (v0 + sum) ^ ((v0>>5) + k[3]);
	}
	v[0] = v0;
	v[1] = v1;
}

void encrypt(char *str, unsigned int size, t_payload *pay)
{

	if (size % 8 != 0)
		size += 8 - (size % 8);
	uint32_t k[4] = {
		((uint32_t*)pay->key)[0],
		((uint32_t*)pay->key)[1],
		((uint32_t*)pay->key)[2],
		((uint32_t*)pay->key)[3]
	}; 

	pay->blocks = size / 8;
	for (unsigned int i = 0; i < pay->blocks; i++)
		block_encrypt((uint32_t*)(str + i * 8), k);
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
	
	/* Check if ELF headers offset are valid */
	if (info.ehdr->e_phoff >= file.size || info.ehdr->e_shoff >= file.size)
		return (ERR_INV_HEADER);

	info.phdr = (Elf64_Phdr*)(file.file + info.ehdr->e_phoff);
	info.shdr = (Elf64_Shdr*)(file.file + info.ehdr->e_shoff);
	printf("%d\n", info.ehdr->e_shstrndx);
	info.shst = (char *)(file.file + info.shdr[info.ehdr->e_shstrndx].sh_offset);
	info.old_entry = info.ehdr->e_entry;
	printf("Old entry addr %lx\n", info.old_entry);

	info.starget = find_target_segment(info.phdr, info.ehdr, &info.pagediff, &info.pagelen);
	if (info.starget == NULL)
		return ERR_INV_SEGMENT;

	/* Writing payload inside target section */
	payload = write_payload(file, info.ehdr, info.starget);
	printf("Payload successfully writed\n");

	/* Filling page information on payload for later mprotect manipulation */
	payload->pagediff = info.pagediff;
	payload->pagelen = info.pagelen;

	/*  Check if string section isn't out of bound  */
	if (info.ehdr->e_shoff + info.ehdr->e_shnum * sizeof(Elf64_Shdr) >= file.size)
		return (ERR_INV_SECTION);
	text_section = find_section(info.shdr, info.ehdr->e_shnum, info.shst ,".text");
	payload->key[0] = random_key();
	payload->key[1] = random_key();
	printf("%ld, %ld, %u\n", text_section->sh_offset, text_section->sh_size, file.size);
	
	/*  Check if .text section isn't out of bound */
	if (text_section->sh_offset >= file.size || text_section->sh_offset + text_section->sh_size >= file.size)
		return (ERR_INV_SECTION);
	encrypt(file.file + text_section->sh_offset, text_section->sh_size, payload);
	payload->encrypt_offset = text_section->sh_addr - info.old_entry;

	/* Log payload to debug	*/
	int logfd = open("paylog", O_CREAT | O_WRONLY | O_TRUNC, 0667);
	write(logfd, (char*)payload, sizeof(t_payload));

	/* Create file and writing our packed binary */
	create_file(file);
	return 0;
}

int main(int ac, char **av)
{
	int err;
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
	if ((err = code_inject(file)) != 0)
		fprintf(stderr, "File format error, check if your file is a valid ELF\nError Code {%d}\n", err); 
	munmap(file.file, file.size); 
	return 0;
}

