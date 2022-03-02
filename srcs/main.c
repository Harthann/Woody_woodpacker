#include <stdio.h>
#include <woody.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <elf.h>

#define PAYLOAD_LEN (long unsigned)(_falseend - _payload)

typedef struct {
    char *file;
    unsigned int size;
}           t_file;

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
    
void write_payload(t_file file, Elf64_Ehdr *ehdr, Elf64_Phdr *phdr)
{
    unsigned int offset;

    offset = phdr->p_offset + phdr->p_memsz;
    for (unsigned int i = 0; i < PAYLOAD_LEN; i++)
        file.file[offset + i] = ((char*)_payload)[i];

    *(unsigned long *)(file.file + offset + 5) = (unsigned long)(ehdr->e_entry - (phdr->p_paddr + phdr->p_memsz));

    ehdr->e_entry = phdr->p_paddr + phdr->p_memsz;
    phdr->p_memsz += PAYLOAD_LEN;
    phdr->p_filesz += PAYLOAD_LEN;
    phdr->p_flags |= PF_X;
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


void elfinfo(t_file file)
{
    Elf64_Ehdr *ehdr;
    Elf64_Phdr *phdr;
    Elf64_Phdr *target = NULL;


    ehdr = (Elf64_Ehdr*)file.file;
    phdr = (Elf64_Phdr*)(file.file + ehdr->e_phoff);
    
    write(1, ehdr->e_ident, 4);
    write(1, "\n", 1);

    /* Printing PT_LOAD section info */
    printf("%8s | %8s | %8s | %7s | %7s\n", "id", "type", "offset", "len", "space");
    for (int i = 0; i < ehdr->e_phnum; i++)
    {
        if (phdr[i].p_type == PT_LOAD)
        {
            printf("%8d | %8x | %8lx | %7ld | %7ld\n", i, phdr[i].p_type, phdr[i].p_offset, phdr[i].p_memsz, (i + 1 == ehdr->e_phnum) ? 0 : phdr[i + 1].p_offset - phdr[i].p_offset - phdr[i].p_memsz);
            if (!target && phdr[i + 1].p_offset - phdr[i].p_offset - phdr[i].p_memsz > PAYLOAD_LEN)
                target = phdr + i;
        }
    }
    printf("\nSection selected: %8x | %8lx | %7ld\n", target->p_type, target->p_offset, target->p_memsz);
    
    /* Writing payload inside target section */
    write_payload(file, ehdr, target);

    /* Create file and writing our packed binary */
    create_file(file);
}

int main(int ac, char **av)
{
    t_file file;

	if (ac != 2)
	{
		printf("I need something to pack, thanks\n");
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
}

