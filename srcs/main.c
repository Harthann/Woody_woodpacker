#include <stdio.h>
#include <woody.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>

char *load_file(char *path)
{
	int fd, len;
	char *file;

	fd = open(path, O_RDONLY);
	if (fd < 0)
		return NULL;
	len = lseek(fd, 0, SEEK_END);
	printf("File loaded length: %d\n", len);
	lseek(fd, 0, SEEK_SET);
	file = mmap(NULL, len + 1, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);	
	close(fd);
	return file;
}

int main(int ac, char **av)
{
	char *file = NULL;
	if (ac != 2)
	{
		printf("Be carefull error\n");
	}
	printf("Hello World\n");
	_payload();
	printf("%ld\n", _falseend - _payload);
	file = load_file(av[1]);
	if (!file)
	{
		write(2, "Failed to load file\n",20);
		return 0;
	}
	printf("%s\n", file);
}

