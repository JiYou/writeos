#include <inc/types.h>
#include <inc/memlayout.h>

void i386_init() {
	int i = 0;
	char *cp = (char*)(0xB8000 + KERNBASE);
	const char *str = "Hello world!";
	while (*str) {
		*cp++ = *str++;
		*cp++ = 0x07;
	}
	while ( 1 == 1) {
	}
}
