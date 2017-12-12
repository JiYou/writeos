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

// Control Register flags
#define CR0_PE          0x00000001      // Protection Enable
#define CR0_MP          0x00000002      // Monitor coProcessor
#define CR0_EM          0x00000004      // Emulation
#define CR0_TS          0x00000008      // Task Switched
#define CR0_ET          0x00000010      // Extension Type
#define CR0_NE          0x00000020      // Numeric Errror
#define CR0_WP          0x00010000      // Write Protect
#define CR0_AM          0x00040000      // Alignment Mask
#define CR0_NW          0x20000000      // Not Writethrough
#define CR0_CD          0x40000000      // Cache Disable
#define CR0_PG          0x80000000      // Paging

// 这里定义一个页表里面offset所占用的位数。
#define PGSHIFT        12        // log2(PGSIZE)
// 一页为4KB
#define PGSIZE        4096        // bytes mapped by a page
// 一个页表里面有多少个表项
#define NPTENTRIES    1024        // page table entries per page table
// 一个页目录表里面有多少表项
#define NPDENTRIES    1024        // page directory entries per page directory

// 一个页表，一页为4KB，一共就是4MB了
#define PTSIZE        (PGSIZE*NPTENTRIES) // bytes mapped by a page directory entry

#define PDXSHIFT    22        // offset of PDX in a linear address

// 页表的属性项
#define PTE_P        0x001    // Present
#define PTE_W        0x002    // Writeable

#endif
