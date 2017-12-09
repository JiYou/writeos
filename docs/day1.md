
这里主要是采用Centos 6.5 + bochs来搭建环境。首先下载镜像

```
wget http://archive.kernel.org/centos-vault/6.5/isos/x86_64/CentOS-6.5-x86_64-bin-DVD1.iso
```

下裁完成之后，利用`virtualbox`来安装操作系统。

然后利用bochs来进行安装。

```
wget https://downloads.sourceforge.net/project/bochs/bochs/2.6/bochs-2.6.tar.gz?r=https%3A%2F%2Fsourceforge.net%2Fprojects%2Fbochs%2F%3Fsource%3Dtyp_redirect&ts=1507970286&use_mirror=superb-dca2
```

```
./configure \
--prefix=/usr \
--enable-debugger \
--enable-disasm \
--enable-iodebug \
--enable-x86-debugger \
--with-x \
--with-x11
```

然后在`Makefile`中找到如下位置

```
 92 LIBS =  -lm -pthread -lgtk-x11-2.0 -lgdk-x11-2.0 -latk-1.0 -lgio-2.0 -lpangoft2-1.0     -lgdk_pixbuf-2.0 -lpangocairo-1.0 -lcairo -lpango-1.0 -lfreetype -lfontconfig -lgobj    ect-2.0 -lgmodule-2.0 -lgthread-2.0 -lrt -lglib-2.0 -lpthread
```

这里需要加上`-lpthread`。然后就可以成功编译并且安装bochs了。

