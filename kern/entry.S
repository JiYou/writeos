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

.globl entry
entry:
	call PrintString
spin:	jmp	spin

