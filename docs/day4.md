地址空间的问题
===========

当切换到保护模式之后，虽然建立的段映射。但是实际上`Linux`并不使用这么复杂的东西。
直接建立了两个段。代码段和数据段都映射了整个`4G`寻址空间。也就是一对一映射。

```cpp
void PrintString() {
	char *p = (char *)0xb8000;
	const char *str = "Hello world!";
	while (*str) {
		*p++ = *str++;
		*p++ = 0x07;
	}
}
```

这里需要特别注意的是，`Hello world!`这个了字符串的放置的物理地址与虚拟地址是完全一样的。
所以在编译的时候，就需要通过`kernel.ld`来指定编译链接的地址。

```cpp
/* Simple linker script for the JOS kernel.
   See the GNU ld 'info' manual ("info ld") to learn the syntax. */

OUTPUT_FORMAT("elf32-i386", "elf32-i386", "elf32-i386")
OUTPUT_ARCH(i386)
ENTRY(_start)

SECTIONS
{
	/* Link the kernel at this address: "." means the current address */
	. = 0x00100000; /*这里指定编译的时候，程序内存的编码起始地址*/

	/* AT(...) gives the load address of this section, which tells
	   the boot loader where to load the kernel in physical memory */
    /*这里指定程序最后被装载的物理地址*/   
	.text : AT(0x100000) {
		*(.text .stub .text.* .gnu.linkonce.t.*)
	}
}
```

如果再按照以前`C`程序的编译思路，不关心虚拟地址以及物理地址，那么程序被加载之后，运行的时候就
会出问题了。因为虚拟地址与物理地址不一致。

**错误的情况**
如果`. = 0xF0100000`，那么后面在取
`const char * = "Hello world!"`的时候，就会去`0xF0100xx`这里去读这个字符串，实际上这段
地址是被加载到了`0x001000xx`这里。

除了这里的改动，另外一定要注意的是`entry.S`里面入口地址。

```C
#define    KERNBASE    0x00000000
#define RELOC(x) ((x) - KERNBASE)

.text

# The Multiboot header
.align 4

# '_start' specifies the ELF entry point.  Since we haven't set up
# virtual memory when the bootloader enters this code, we need the
# bootloader to jump to the *physical* address of the entry point.
.globl		_start
_start = RELOC(entry)
```

这个时候，由于`JOS/xv6`都是利用虚拟地址与`KERNBASE`作差，得到真实的物理地址的入口。
那么这里由于编译的真实地址虚拟地址是一样的。那么也不需要考虑了。直接把

```
_start = entry
```

也是可以成立的。
