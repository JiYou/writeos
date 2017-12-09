#include <inc/mmu.h>

# 刚进入到启动分区时的情形
# Start the CPU: switch to 32-bit protected mode, jump into C.
# The BIOS loads this code from the first sector of the hard disk into
# memory at physical address 0x7c00 and starts executing in real mode
# with %cs=0 %ip=7c00.

# 代码段选择子
.set PROT_MODE_CSEG, 0x8         # kernel code segment selector
# 数据段选择子
.set PROT_MODE_DSEG, 0x10        # kernel data segment selector
# 保护模式是否打开?
.set CR0_PE_ON,      0x1         # protected mode enable flag

# 指定后面函数的起始位置
# 函数的入口地址在start标签处
.global start

# 指定后面的代码是在实模式下
# 也就是16位模式
.code16

# 下面是代码段开始了
.text
start:
	# 在切换到保护模式的时候，先把中断关了
	cli
	# 在处理字符串的复制方向的时候，把
	# 复制方向改为自增
	cld
	# 这里是让ds,es,ss三个段
	# 寄存器指向0地址。
	# 那么后面在访问数据的时候
	# 都是真实的地址
	mov $0,  	%ax
	mov %ax, 	%ds
	mov %ax, 	%es
	mov %ax, 	%ss
	# 这里栈是从boot sector
	# 拷贝到0x7c00之下的部分
	# 对于启动功能来说，暂时
	# 这个栈是够用的
	mov $0x7c00, 	%sp

	#南桥芯片内的端口
	in $0x92, %al
	or $0x02, %al
  	# 打开A20
  	out %al, $0x92

	# 加载描述符表
	lgdt    gdtdesc
	# 开启分段
	# 获取原有的%cr0的内容
	movl    %cr0, %eax
	# 把分段置1
	orl 	$CR0_PE_ON, %eax
	# 把修改放回cr0
	movl	%eax, %cr0

	# 开始利用长跳转，来清空CPU流水线，使得切换生效
	ljmp $PROT_MODE_CSEG, $protcseg
	# 16位的实模式到这里结束


# 开始进入到保护模式的代码里面了
.code32
protcseg:
	# 跳转成功之后，开始设置各个段寄存器
	# 由于cs 段已经是指向了保护模式下的代码段
	# 这里需要把其他段指向数据段。
	# Set up the protected-mode data segment registers
	movw    $PROT_MODE_DSEG, %ax    # Our data segment selector
	movw    %ax, %ds                # -> DS: Data Segment
	movw    %ax, %es                # -> ES: Extra Segment
	movw    %ax, %fs                # -> FS
	movw    %ax, %gs                # -> GS
	movw    %ax, %ss                # -> SS: Stack Segment

	# 这里就可以开始调用C语言了

	# 是的，启动功能到这里就结束了
	# 这里不再演示更多的功能
	# 于是直接停机
	hlt

# 强制2^2个字节对齐
.p2align 2
gdt:
	# 第一个描述符
	SEG_NULL
	# 第二个描述符表示代码段
	# 可读，可执行，0~4G
	SEG(STA_X|STA_R, 0x0, 0xffffffff)
	# 第三个描述符表示数据段
	# 可写，范围是0~4G
	SEG(STA_W, 0x0, 0xffffffff)

gdtdesc:
	# 这个是gdt表的长度。0x17 = 23
	# 所以gdt表的长度是3 * 8 - 1 = 23
	# 其中一个表项的长度为8.
	.word 0x17
	# 接下来是表示gdt描述符表的地址在哪里
	.long gdt

END:
	# 启动扇区必须是512字节大小
	.org 510
	# 最后两个字节必须是固定的
	.word 0xaa55
