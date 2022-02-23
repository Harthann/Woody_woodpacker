
#include <sys/mman.h>
#include <stdint.h>
#include <stddef.h>





void main() {
	void *addr;

	char flag = MAP_ANONYMOUS | MAP_PRIVATE;
	char prot = PROT_WRITE | PROT_EXEC;

	printf("%d %d\n", prot, flag);

	addr = NULL;
	addr = mmap(addr, 1, prot, flag, 0,0);
	*((uint8_t*)addr) = 204;
	((void(*)())addr)();
}
