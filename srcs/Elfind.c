#include "woody.h"

/* Searching segment with free space for injection using segment header */
Elf64_Phdr 	*find_target_segment(Elf64_Phdr *phdr, Elf64_Ehdr *ehdr, size_t *pagediff, size_t *pagelen)
{
	Elf64_Phdr *target = NULL;
	/* Printing PT_LOAD section info */
	printf("%8s | %8s | %8s | %7s | %7s | %7s\n",
		"id", "type", "offset", "filesz", "memsz", "space");
	for (int i = 0; i < ehdr->e_phnum; i++)
	{
		if (phdr[i].p_type == PT_LOAD)
		{
			printf("%8d | %8x | %8lx | %7ld | %7ld | %7ld\n", i, phdr[i].p_type, phdr[i].p_offset, phdr[i].p_filesz ,phdr[i].p_memsz, (i + 1 == ehdr->e_phnum) ? 0 : phdr[i + 1].p_offset - phdr[i].p_offset - phdr[i].p_filesz);
			if (!target && phdr[i + 1].p_offset - phdr[i].p_offset - phdr[i].p_filesz > PAYLOAD_LEN)
			{
					target = phdr + i;
			}
			if (phdr[i].p_flags & PF_X)
			{
				//target = phdr + i;
				*pagediff = target->p_memsz - (phdr[i].p_vaddr - target->p_vaddr);
				*pagelen = phdr[i].p_memsz;
				if (target == phdr + i)
						*pagelen += PAYLOAD_LEN;
				phdr[i].p_flags ^= PF_X;
				//phdr[i].p_flags |= PF_W;
			}
		}
	}
	printf("\nSegment selected:\n%8.0d | %8x | %8lx | %7ld | %7ld\n", 0, target->p_type, target->p_offset, target->p_filesz ,target->p_memsz);

	return target;
}

/* Find specific section section */
Elf64_Shdr 	*find_section(Elf64_Shdr *shdr, int shnum, char *shst, char *section_name )
{
	write(1, "Here\n", 5);
	printf("Sections info:\n%20s | %10s | %10s | %10s | %10s\n", "name", "type", "offset", "size" ,"addr");
	for (int i = 0; i < shnum; i++)
	{
		printf("%20s | %10x | %10lx | %10lx | %10lx\n", shst + shdr[i].sh_name, shdr[i].sh_type, shdr[i].sh_offset, shdr[i].sh_size, shdr[i].sh_addr);
		if (strcmp(shst + shdr[i].sh_name, section_name) == 0)
		{
			printf("Found .text at offset: %lx\n", shdr[i].sh_offset);
			write(1, "lol\n", 4);
			return shdr + i;
		}
	}
	write(1, "There\n", 6);

	return NULL;
}


