#include <inc/types.h>
#include <inc/x86.h>
#include <inc/elf.h>

/*
 * 这里很重要的一点是，JOS的bootloader的布局对于真正操作系统的
 * 要求是固定的，也就是必须是要加载到固定的物理位置处。也就是
 * 0x100000，也就是1MB这里。
 * 
 * － 磁盘上的布局
 *    |--boot sector--|---kernel---|---kernel---|---kernel---| ........|
 *    |_______________|________________________________________________|
 *           |                                 |
 *        第一扇区                        第2.....N扇区 
 *
 * - kernel必须是ELF格式的
 *
 * - 代码的加载顺序是：BIOS -> boot.S -> boot/main.c -> kernel code
 */

/**********************************************************************
 * This a dirt simple boot loader, whose sole job is to boot
 * an ELF kernel image from the first IDE hard disk.
 *
 * DISK LAYOUT
 *  * This program(boot.S and main.c) is the bootloader.  It should
 *    be stored in the first sector of the disk.
 *
 *  * The 2nd sector onward holds the kernel image.
 *
 *  * The kernel image must be in ELF format.
 *
 * BOOT UP STEPS
 *  * when the CPU boots it loads the BIOS into memory and executes it
 *
 *  * the BIOS intializes devices, sets of the interrupt routines, and
 *    reads the first sector of the boot device(e.g., hard-drive)
 *    into memory and jumps to it.
 *
 *  * Assuming this boot loader is stored in the first sector of the
 *    hard-drive, this code takes over...
 *
 *  * control starts in boot.S -- which sets up protected mode,
 *    and a stack so C code then run, then calls bootmain()
 *
 *  * bootmain() in this file takes over, reads in the kernel and jumps to it.
 **********************************************************************/

// 磁盘上一个扇区的大小
#define SECTSIZE        512

// 操作系统物理起始扇区
#define OS_PHYBASE_MEM  0x10000

// ELF文件头开始存放的地方
#define ELFHDR          ((struct Elf *)OS_PHYBASE_MEM)

// 读一个扇区的函数
void readsect(void*, uint32_t);

// 读一长段磁盘内容到内存里面
void readseg(uint32_t, uint32_t, uint32_t);

void bootmain(void) {
	struct Proghdr *ph, *eph;

	// 读8个扇区到ELFHDR内存地址处
	// 开始读的位置是从kernel文件自身的开头为标准
	// 的偏移处0开始算。
	// 这里一定要注意，kernel文件自身的开头
	// 与磁盘的内容布局是有一定差异的
	// * － 磁盘上的布局
	// *    |--boot sector--|---kernel---|---kernel---|---kernel---| ........|
	// *    |_______________|________________________________________________|
	// *           |                                 |
	// *        第一扇区                        第2.....N扇区
	// 第三个参数offset = 0, 表示的是相对于kernel文件头而言。
	readseg((uint32_t)ELFHDR, 8*SECTSIZE, 0);

        // is this a valid ELF?
        if (ELFHDR->e_magic != ELF_MAGIC)
		goto bad;

	// 开始加载每个程序中定义的各种段
	// 找到段表的头
        ph = (struct Proghdr *) ((uint8_t *) ELFHDR + ELFHDR->e_phoff);
	// 段表结束的地址eph = end of ph
        eph = ph + ELFHDR->e_phnum;
	// 依次读每个段
	while (ph < eph) {
		// p_pa是希望被加载到的内存地址处
		// 由于目前MMU还没有被打开，所以物理地址与虚拟地址是重合的
		// 所以p_pa就是加载到的物理地址处。
		// memsz表示段需要占用的内存大小
		// p_offset是相对于整个kernel文件来说的偏移量
		readseg(ph->p_pa, ph->p_memsz, ph->p_offset);
		ph++;
	}
	// 开始跳转到入口地址处，也就是entry.S以及kernel.ld中约定的
	// 入口处
	((void (*)(void)) (ELFHDR->e_entry))();
	// 这里已经跳到内存里面去执行了，按理说，不应该再返回了。
	// 所以后面的这段程序是用来作出错处理的

bad:
	// If bootmain returns (it shouldn't), trigger a Bochs 
	// breakpoint if running under Bochs, then loop.  
        outw(0x8A00, 0x8A00);
        outw(0x8A00, 0x8E00);
        while (1)
                /* do nothing */;
}

// 这里是从kernel文件的offset位置处，开始读count个bytes到内存地址pa处.
// pa指的是物理内存地址
// count指的是需要读取的bytes数
// offset指的是相对于kernel文件的逻辑偏移。
// 注意：offset不是相对于物理磁盘的偏移!!!
// 考虑这种极端的情况，也就是从256读257个bytes到内存地址256处。offset为256
void readseg(uint32_t pa, uint32_t count, uint32_t offset) {
	const uint32_t kernel_start_sector = 1;
	// 物理内存的结束地址
	uint32_t end_pa = pa + count;
	// 把物理内存与512byte地址对齐，因为磁盘读上来的内容就是
	// 512bytes, 注意，这里是向小方向取齐，比如256地址对齐后就是0地址。
	pa &= ~(SECTSIZE - 1);
	// 把相对于kernel文件开头的bytes数转换成为sectors数目
	// 直接除，也是向小方向取齐。
	offset /= SECTSIZE;
	// 再加上kernel是从哪里开始部局的
	offset += kernel_start_sector;
	while (pa < end_pa) {
		readsect((uint8_t *)pa, offset);
		offset++;
		pa += SECTSIZE;
	}
}

void waitdisk(void) {
	// wait for disk reaady
	while ((inb(0x1F7) & 0xC0) != 0x40) {
		/*这里什么也不用做，空等*/
	}
}

void readsect(void *dst, uint32_t offset) {
	// 等待磁盘准备好
        waitdisk();

	// 发送读磁盘指令
        outb(0x1F2, 1); 	// 这里只读一个扇区
        outb(0x1F3, offset);	// 从offset扇区位置开始读
        outb(0x1F4, offset >> 8);
        outb(0x1F5, offset >> 16);
        outb(0x1F6, (offset >> 24) | 0xE0);
        outb(0x1F7, 0x20);      // cmd 0x20 - read sectors

	// 发送完命令后，等待磁盘准备好
        waitdisk();

	// 开始读取数据到内存地址处
        insl(0x1F0, dst, SECTSIZE>>2);
}
