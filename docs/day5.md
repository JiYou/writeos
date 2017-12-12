页面管理
========

首先值得一提的是，整个内核的虚拟地址空间是4G。虽然物理空间不可能给到4G。但是可通过页表管理，以及swap技术达到4G的效果。

在这个4G的空间里面，需要处理的是，把4G的内存的虚拟地址的高位给kernel使用。把内存的低位给用户程序使用。

所以，前面那种虚拟地址与物理地址严格对应的做法实际上是行不通的。比如，因为低端虚拟内存地址想给用户程序使用，比如小于3G的内存都给用户程序。大于3G的内存地址给内核，这个时候，一般意义上的物理内存可能没有这么多。严格对应就做不到了。

如果把kernel放在虚拟地址的低端，又会遇到的问题是虚拟地址空间会比较浪费。除非虚拟地址空间是从0~xMB划得死死的给kernel使用。但是万一后面的kernel模块需要的空间比这大呢？又要重新做调整了。


所以，办法是物理上而言，有可能内存比较少，那就把kernel加载到相对低端一点的位置。比如。物理上加载到`0x100000`这个位置。

这个物理加载位置是在`kern/kernel.ld`里面可以指定的。

```
        .text : AT(0x100000) {
                *(.text .stub .text.* .gnu.linkonce.t.*)
        }
```

而虚拟地址空间，也是在`kernel.ld`里面指定。

```
SECTIONS
{
        . = 0xF0100000;
```

也就是说，虚扩地址空间的从`0xF0100000`开始都是在给**kernel**使用。

# 如何建立页表

当打开保护模式，可以从实模式跳到32位模式之后。可以直接使用C语言编写程序了。但是限制也是很大的，如果没有物理地址到虚拟地址的映射，那么编译出来的程序，必须在虚拟地址上和物理地址是同一的空间。

如果可以建立页表，完这段虚拟地址到物理地址的映射就OK了。所以在打开了保护模式，把kernel加载到内存之后，再次在kernel里面完成页表的处理。

接下来是考虑建立两级页表。

内存分布
===========

# bootloader读入内核代码之后的分布

[内存分布](http://lzz5235.github.io/2014/03/04/jos.html)

这里主要是引用一下这个图：

![](http://lzz5235.github.io/assets/pic/235.png)

可以发现，在刚读取完成内核代码到内存之后。形成的结构如上图所示。

# 开启分页

分页的机制在《x86汇编语言－从实模式到保护模式》里面介绍得比较清楚。这里就不多说，只引用[一张图](http://neilsh.me/2015/07/02/os_setup_and_virtual_memory_setup/)：

![](http://ww1.sinaimg.cn/large/7f793092gw1etosykok6uj216o1kwdq0.jpg)

# 开启分页代码

这段代码`entry.S`里面：

```
    # Load the physical address of entry_pgdir into cr3.  entry_pgdir
    # is defined in entrypgdir.c.
    movl    $(RELOC(entry_pgdir)), %eax
    movl    %eax, %cr3
    # Turn on paging.
    movl    %cr0, %eax
    orl $(CR0_PE|CR0_PG|CR0_WP), %eax
    movl    %eax, %cr0
```

注意，直到`move %eax, %cr0`这里，实际上都是在物理线性地址模式下。也就是没有开启分页。所以`RELOC`需要用来获取`entry_pgdir`也就是页目录数组的物理地址。

开启分页就是两步：

- 把页目录表放到`CR3`中。
- 把相应的`CR0_PE`等位打开，标志着`x86`CPU就开始工作在分页模式下了。


# 跳到虚拟地址空间

接下来就是从物理线性地址空间执行的代码，跳到虚拟地址空间去。因为现在还在物理线性地址空间中运行嘛。所以后面需要跳转到开启分页后的虚拟地址空间。

```
    # Now paging is enabled, but we're still running at a low EIP
    # (why is this okay?).  Jump up above KERNBASE before entering
    # C code.
    mov $relocated, %eax
    jmp *%eax
relocated:
```

一定需要注意的是，这里的`$relocated`只是一个虚拟地址空间。不能采用物理地址空间的原因是因为：`分页机制已经开启`。如果还是采用物理线性地址空间来跳转，是会出错！！

# 如何分页

分页的时候，会涉及两部分，一部分是页目录，一部分是页表。

## 页目录

```
#define PTE_P        0x001    // Present

// The entry.S page directory maps the first 4MB of physical memory
// starting at virtual address KERNBASE (that is, it maps virtual
// addresses [KERNBASE, KERNBASE+4MB) to physical addresses [0, 4MB)).
// We choose 4MB because that's how much we can map with one page
// table and it's enough to get us through early boot.  We also map
// virtual addresses [0, 4MB) to physical addresses [0, 4MB); this
// region is critical for a few instructions in entry.S and then we
// never use it again.
//
// Page directories (and page tables), must start on a page boundary,
// hence the "__aligned__" attribute.  Also, because of restrictions
// related to linking and static initializers, we use "x + PTE_P"
// here, rather than the more standard "x | PTE_P".  Everywhere else
// you should use "|" to combine flags.
__attribute__((__aligned__(PGSIZE)))
pde_t entry_pgdir[NPDENTRIES] = {
    // Map VA's [0, 4MB) to PA's [0, 4MB)
    [0]
        = ((uintptr_t)entry_pgtable - KERNBASE) + PTE_P,
    // Map VA's [KERNBASE, KERNBASE+4MB) to PA's [0, 4MB)
    [KERNBASE>>PDXSHIFT]
    // #define PDXSHIFT    22 // offset of PDX in a linear address
        = ((uintptr_t)entry_pgtable - KERNBASE) + PTE_P + PTE_W
};
```

先把注释中的英文翻译清楚。


`entry.S`里面的页目录映射了：

- 物理内存开头的4MB内存：即虚拟地址的[KERNBASE, KERNBASE + 4MB) --> [0, 4MB)
- 我们选择`4MB`的原因在于，这刚好是一个页表所需的（32位系统映射完4GB内存需要4MB的页表）。并且这么多内存也足够在启动初期使用了。
- 此外，也把VD [0, 4MB) --> PD[0,4MB)。这么处理的原因是由于，`entry.S`开头的一部分代码是非常关键的，并且在后面并不想使用。这是由于`entry.S`的开头这部分代码是用来做开启分页用的。当程序进入到正常状态之后，肯定是不想再让程序跳转到这里来执行。
- 在写代码的时候，使用了`__attribute__((__aligned__(PGSIZE)))`的原因是由于页目录及页表都是需要从页边界开始启用的。所以这里用这了个标记符。
- 此外，因为链接和静态初始化的限制，这里用的是`x + PTE_P`的方式。在别的地方最好是`x | PTE_P`。

### 第一张表

第一张表的配置代码如下：

```
[0] = ((uintptr_t)entry_pgtable - KERNBASE) + PTE_P,
```

需要注意的是，由于开启分页的时候，页目录也好，页表也好，里面所有的地址都需要是使用物理地址，而不是虚拟地址。

- entry_pgtable就是虚拟地址
- entry_pgtable - KERNBASE就是物理地址。

所以在填表的时候，需要将虚拟地址转换换为物理地址。

此外看一下这个物理地址所指向的页。注意，这里是页目表项，里面记录的是页表的基地址。无论是页目录表，页表，页目录表项，页表项里面都只有开头20位有用。后面的12位都是用来做属性位。

那么`entry_pgtable`里面记录的又是啥内容呢？

```
memlayout.h:144:typedef uint32_t pde_t; 

// Entry 0 of the page table maps to physical page 0, entry 1 to
// physical page 1, etc.
__attribute__((__aligned__(PGSIZE)))
pte_t entry_pgtable[NPTENTRIES] = {
    0x000000 | PTE_P | PTE_W,
    0x001000 | PTE_P | PTE_W,
    0x002000 | PTE_P | PTE_W,
    0x003000 | PTE_P | PTE_W,
    0x004000 | PTE_P | PTE_W,
    ....
    0x3fa000 | PTE_P | PTE_W,
    0x3fb000 | PTE_P | PTE_W,
    0x3fc000 | PTE_P | PTE_W,
    0x3fd000 | PTE_P | PTE_W,
    0x3fe000 | PTE_P | PTE_W,
    0x3ff000 | PTE_P | PTE_W,
```

因此，从这里可以看出，第一张页表比较简单，就是严格照着物理内存对应，没有偏差。


### 第二张表

由于页目录表起用的是一个虚拟地址的高`10`位。所以在设置页目录的时候，直接是将`KERNBASE`的地址往左移`22`位。因为总共32位，高10位用来做`index`。

```
    // Map VA's [KERNBASE, KERNBASE+4MB) to PA's [0, 4MB)
    [KERNBASE>>PDXSHIFT]
        = ((uintptr_t)entry_pgtable - KERNBASE) + PTE_P + PTE_W
```

这里比较不容易理解，那么举个例子就懂了。假设程序在访问的虚拟地址是`KERNBASE`。那么利用这个页目录表，以及页表，得到的物理地址是多少。

```
0xF0000000 = 0b 1111 0000 00|00 0000 0000 |0000 0000 0000
                |___________|_____________|______________|
                       |           |             |
                       H10        M10           L12
```

- 第一步，查页目录表，这个比较简单，直接从CR3寄存器就拿到了。
- 第二步，做页目录表的索引，这个也简单，就直接取`KERNBASE`的高10位。即`H10 = KERNBASE >> 22`。
- 第三步，取`entry_pgdir[H10]`的内容。得到：`entry_pgtable`页表。
- 第四步，取`KERNBASE`的中间10位做为页表的索引。即`M10 = 0`。
- 第五步，取页表项，即`entry_pgtable[0]`
- 第六步，页基地址即`0x000000 | PTE_P | PTE_W,`
- 第七步，取得`L12`也就是低12位为0,偏移值为0。所以得到的物理地址为0。

这也就可以得出`KERNBASE`会映射到0地址。

### 分页结果

至少目前从这两个页目录项看来，的确是如注释所说：

```
Map VA's [0, 4MB) to PA's [0, 4MB)
Map VA's [KERNBASE, KERNBASE+4MB) to PA's [0, 4MB)
```

### 原因

那么问一句，为什么要这么操作呢？如果结合之前链接程序的时候的链接地址是设置是`0xF0100000`。那么这个地址又会映射到物理地址的什么地方？


```
0xF0100000 = 0b 1111 0000 00|01 0000 0000 |0000 0000 0000
                |___________|_____________|______________|
                       |           |             |
                       H10        M10           L12
```

- 第一步，查页目录表，这个比较简单，直接从CR3寄存器就拿到了。
- 第二步，做页目录表的索引，这个也简单，就直接取`KERNBASE`的高10位。即`H10 = KERNBASE >> 22`。
- 第三步，取`entry_pgdir[H10]`的内容。得到：`entry_pgtable`页表。
- 第四步，取`KERNBASE`的中间10位做为页表的索引。即`M10 = 0x100`。
- 第五步，取页表项，即`entry_pgtable[0x100]`即`entry_pgtable[256]`
- 第六步，页基地址即` 256     0x100000 | PTE_P | PTE_W,`
- 第七步，取得`L12`也就是低12位为0,偏移值为0。所以得到的物理地址为`0x100000`。
- 虽然看起来`0x100000`很大，实际上也就是1^20次方，即就是`1MB`。所以仍然是在页目录的映射范围里面。
- 通过这样建立起来的页表，就可以很方便地占用虚拟地址的高端位置，并且把实际位置映射到低端4MB处。
- 读内核的时候，实际上只是把内存放到了1MB处。

这个时候，虚拟地址刚好完美地与物理地址映射在一起。也就是和`bootloader`读的地址映射在一起。


# 结论

这里作者用代码填了分页机制，既然是手写的，所以它的功能就很有限了，只能够把虚拟地址空间的地址范围：0xf0000000~0xf0400000，映射到物理地址范围：0x00000000~0x00400000上面。也可以把虚拟地址范围：0x00000000~0x00400000，同样映射到物理地址范围：0x00000000~0x00400000上面。任何不再这两个虚拟地址范围内的地址都会引起一个硬件异常。虽然只能映射这两块很小的空间，但是已经足够刚启动程序的时候来使用了。

