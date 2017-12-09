#ifndef JOS_INC_MMU_H
#define JOS_INC_MMU_H

/*
 * 如果发现是汇编器在使用这个头文件
 * 因此，对于C语言来说，需要使用不同的宏。不能
 * 直接使用汇编语言的宏
 */
#ifdef __ASSEMBLER__
/*
 * Macros to build GDT entries in assembly.
 */
#define SEG_NULL                        \
    .word 0, 0;                     \
    .byte 0, 0, 0, 0
#define SEG(type,base,lim)                  \
    .word (((lim) >> 12) & 0xffff), ((base) & 0xffff);  \
    .byte (((base) >> 16) & 0xff), (0x90 | (type)),     \
        (0xC0 | (((lim) >> 28) & 0xf)), (((base) >> 24) & 0xff)
#endif


/*
 * C和汇编都可以公用的宏
 */
// Application segment type bits
#define STA_X       0x8     // Executable segment
#define STA_E       0x4     // Expand down (non-executable segments)
#define STA_C       0x4     // Conforming code segment (executable only)
#define STA_W       0x2     // Writeable (non-executable segments)
#define STA_R       0x2     // Readable (executable segments)
#define STA_A       0x1     // Accessed

#endif
