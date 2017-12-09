# 	-pipe
#	    Use pipes rather than temporary
#           files for communication between
#           the various stages of compilation.
#           This fails to work on some systems
#           where the assembler is unable to
#           read from a pipe; but the GNU
#           assembler has no trouble.
#           这里的意思就是说，在编译的时候，
#           直接使用pipe来把当前这一阶段的数据
#           扔给下一阶段，而不是使用临时文件。
#           因为，但是在某些系统上，这么干可能
#           会导致失败。但是如果是使用GNU的汇编
#           器，那么就没有这个问题。
CC      := gcc -pipe
AS      := as
AR      := ar
LD      := ld
OBJCOPY := objcopy
OBJDUMP := objdump
NM      := nm
# gcc 编译 32 位程序需要添加参数 -m32 ,ld需要添加参数是 -m elf_i386
LDFLAGS := -m elf_i386

all: hd.img

boot_objs := boot.o

hd.img: /usr/bin/bximage boot
	# 如果是使用如下命令自动创建bochs磁盘，那么
	# 在.bochsrc里面需要添加如下内容
	# ata0-master: type=disk, path="hd.img", mode=flat, cylinders=20, heads=16, spt=63
	# 就可以使用磁盘启动了
	rm -rf hd.img
	bximage -hd -size=10 -q -mode=flat hd.img
	dd if=boot of=hd.img bs=512 seek=0 conv=notrunc

boot: boot.o 
	#       -N
	#       --omagic
	#           Set the text and data sections to
	#           be readable and writable.  Also,
	#           do not page-align the data
	#           segment, and disable linking
	#           against shared libraries.  If the
	#           output format supports Unix style
	#           magic numbers, mark the output as
	#           "OMAGIC". Note: Although a
	#           writable text section is allowed
	#           for PE-COFF targets, it does not
	#           conform to the format
	#           specification published by
	#           Microsoft.
	$(LD) $(LDFLAGS) -N -e start -Ttext 0x7C00 -o $@.out $^
	$(OBJDUMP) -S $@.out >$@.asm
	$(OBJCOPY) -S -O binary $@.out $@

boot.o: boot.S
	# -nostdinc 是指用纯C语言，不要使用
	#  任何标准库
	#  原来-Os相当于-O2.5。
	#  是使用了所有-O2的优化选项，
	#  但又不缩减代码尺寸的方法
	$(CC) -nostdinc -m32 -Os -c -o $@ $<


.PHONY: clean
clean:
	rm -rf temp.bin bochsout.txt hd.img
	rm *.o *.out *.asm boot *.log -fr *.txt bochs.img


.PHONY: run
run:
	bochs
