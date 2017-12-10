#
# This makefile system follows the structuring conventions
# recommended by Peter Miller in his excellent paper:
#
#	Recursive Make Considered Harmful
#	http://aegis.sourceforge.net/auug97.pdf
#
# 临时编译文件放置的目录
OBJDIR := obj

# Run 'make V=1' to turn on verbose commands, or 'make V=0' to turn them off.
# 通过`make V=1`可以把编译的详细过程写出来。
ifeq ($(V),1)
override V =
endif
ifeq ($(V),0)
override V = @
endif

# 这个变量主要是用来包含头文件所在目录
# 实际上也可以通过-I.来使用。
TOP = .

# 这里主要是检测x86相应的编译链工具是否存在
# 对于x86的检测可以做得更加简单一些。比如直
# 接使用objdump -i来检查是否可以支持i386。
CHECK=$(shell if [[ `objdump -i | grep "elf32-i386" | wc -l` -eq 0 ]]; then \
	echo "*** Error: Couldn't find an i386-*-elf version of GCC/binutils" \
	exit 1; fi\
	)

CC	:= gcc -pipe
AS	:= as
AR	:= ar
LD	:= ld
OBJCOPY	:= objcopy
OBJDUMP	:= objdump
NM	:= nm

# Native commands
NCC	:= gcc $(CC_VER) -pipe
NATIVE_CFLAGS := $(CFLAGS) $(DEFS) $(LABDEFS) -I$(TOP) -MD -Wall
TAR	:= gtar
PERL	:= perl

# Compiler flags
# -fno-builtin is required to avoid refs to undefined functions in the kernel.
# Only optimize to -O1 to discourage inlining, which complicates backtraces.
CFLAGS := $(CFLAGS) $(DEFS) $(LABDEFS) -O1 -fno-builtin -I$(TOP) -MD
CFLAGS += -fno-omit-frame-pointer
CFLAGS += -std=gnu99
CFLAGS += -static
CFLAGS += -Wall -Wno-format -Wno-unused -Werror -gstabs -m32
# -fno-tree-ch prevented gcc from sometimes reordering read_ebp() before
# mon_backtrace()'s function prologue on gcc version: (Debian 4.7.2-5) 4.7.2
CFLAGS += -fno-tree-ch
# Add -fno-stack-protector if the option exists.
CFLAGS += $(shell $(CC) -fno-stack-protector -E -x c /dev/null >/dev/null 2>&1 && echo -fno-stack-protector)
# Common linker flags
LDFLAGS := -m elf_i386
KERN_CFLAGS := $(CFLAGS) -DJOS_KERNEL -gstabs

all: $(OBJDIR)/boot/hd.img $(OBJDIR)/kern/kernel
# Include Makefrags for subdirectories
# 注意，这里采用的是include的方式，所以在根目录上运行make的时候，实际上
# 和cd boot目录下运行make有着根本的不同。
# 在boot目录运行，路径是相对boot而言的，但是
# 如果是在根目录上make，那么Makefrag也需要把路径写成相对于根目录的方式
include boot/Makefrag
include kern/Makefrag

$(OBJDIR)/boot/hd.img: $(OBJDIR)/boot/boot $(OBJDIR)/kern/kernel
	rm -rf $(OBJDIR)/boot/hd.img
	bximage -hd -size=10 -q -mode=flat $(OBJDIR)/boot/hd.img >/dev/null
	dd if=$(OBJDIR)/boot/boot of=$(OBJDIR)/boot/hd.img bs=512 seek=0 conv=notrunc
	dd if=$(OBJDIR)/kern/kernel of=$(OBJDIR)/boot/hd.img seek=1 conv=notrunc
	cp -rf .bochsrc $(OBJDIR)/boot/


.PHONY: clean
clean:
	rm -rf $(OBJDIR) bochsout.txt

.PHONY: kill
kill:
	killall bochs

.PHONY: run
run:
	cd $(OBJDIR)/boot/ && bochs
