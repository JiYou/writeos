#include <inc/mmu.h>
#include <inc/memlayout.h>

#define RELOC(x) ((x) - KERNBASE)

.text

# The Multiboot header
.align 4

# '_start' specifies the ELF entry point.  Since we haven't set up
# virtual memory when the bootloader enters this code, we need the
# bootloader to jump to the *physical* address of the entry point.
.globl		_start
_start = RELOC(entry)
.globl entry
entry:

	# 这里开始设置页表
	# 注意这里实际上是物理地址
	# 设置的是页目录表所在的物理地址
	# entry_pgdir的定义是在C语言中。
	movl    $(RELOC(entry_pgdir)), %eax
	# cr3是页目录表地址寄存器
	movl    %eax, %cr3

	# 准备开启分页
	movl    %cr0, %eax
	# CR0_PE表示已经开启了保护模式
	# CR0_PG表示要开启分页
	# CR0_WP表示要开启写保护
	orl     $(CR0_PE|CR0_PG|CR0_WP), %eax
	# 正式打开分页
	movl    %eax, %cr0

	# 这里利用jmp来清空流水线
	mov $relocated, %eax

relocated:
	# 设置ebp
	# 这是和C语言的栈的部局有关系，当调用C语言的时候
	# 总是会执行
	# push %ebp
	# mov %esp, %ebp
	# 实际上就是先把旧有的ebp基线入栈，在这个基线以下的
	# 栈的内容是不能动的
	# 当前的%ebp指向的位置，里面存放的内容就是上次函数
	# 调用的%ebp。形成了一个链表。
	movl $0x0, %ebp
	# 这里在初始化栈顶，需要注意的是x86的入栈是
	# 栈顶在上面，然后入栈一个元素，就栈顶指针向下走一格。
	movl $bootstacktop, %esp

	# 当设置好栈之后，开始调用C语言的函数
	call i386_init

	# 实际上，不应该跑到这里来的，如果跑到这里来了，直接hlt
	# 也就是停机。
	hlt

# 这里设置调用栈
###################################################################
# boot stack
###################################################################
        .p2align        PGSHIFT         # force page alignment
        .globl          bootstack
bootstack:
        .space          KSTKSIZE
        .globl          bootstacktop
bootstacktop:
