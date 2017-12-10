#ifndef JOS_INC_X86_H
#define JOS_INC_X86_H

#include <inc/types.h>

static inline uint8_t inb(int port) {
	uint8_t data;
	asm volatile("inb %w1,%0" : "=a" (data) : "d" (port));
	return data;
}

static inline void insl(int port, void *addr, int cnt) { 
	asm volatile("cld\n\trepne\n\tinsl"
		     : "=D" (addr), "=c" (cnt)
		     : "d" (port), "0" (addr), "1" (cnt)
		     : "memory", "cc");
}

static inline void outb(int port, uint8_t data) {
	asm volatile("outb %0,%w1" : : "a" (data), "d" (port));
}

static inline void outw(int port, uint16_t data) {
	asm volatile("outw %0,%w1" : : "a" (data), "d" (port));
}


#endif /* !JOS_INC_X86_H */
