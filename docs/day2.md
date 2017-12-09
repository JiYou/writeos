制作bochs Image
=================

**提示**这里的汇编纯属测试功能使用，不需要读懂。

# 制作Image

在`Linux`系统上利用`bximage`制作`image`:

```bash
root@oslaba:~/test# bximage
========================================================================
                                bximage
                  Disk Image Creation Tool for Bochs
        $Id: bximage.c,v 1.34 2009/04/14 09:45:22 sshwarts Exp $
========================================================================

Do you want to create a floppy disk image or a hard disk image?
Please type hd or fd. [hd]

What kind of image should I create?
Please type flat, sparse or growing. [flat]

Enter the hard disk size in megabytes, between 1 and 129023
[10] 1

I will create a 'flat' hard disk image with
  cyl=2
  heads=16
  sectors per track=63
  total sectors=2016
  total size=0.98 megabytes

What should I name the image?
[c.img]

Writing: [] Done.

I wrote 1032192 bytes to c.img.

The following line should appear in your bochsrc:
  ata0-master: type=disk, path="c.img", mode=flat, cylinders=2, heads=16, spt=63
root@oslaba:~/test#
```

# bochsrc

制作好`Image`之后，准备开始测试。首先修改`.bochsrc`文件：

```bash
megs: 32
floppya: 1_44=temp.bin, status=inserted
ata0-master: type=disk, path="c.img", mode=flat, cylinders=2, heads=16, spt=63
boot: floppy
log: bochsout.txt
mouse: enabled=0
```

# Makefile的写法

上面的这个过程，为了方便自动化，是写在了`Makefile`里面。后面一旦运行`make`的时候。
就可以自动构建整个过程。

但是`.bochsrc`文件，还是要自己写一下的。内容如下:

```
megs: 32
ata0-master: type=disk, path="hd.img", mode=flat, cylinders=20, heads=16, spt=63
boot: disk
log: bochsout.txt
mouse: enabled=0
```
