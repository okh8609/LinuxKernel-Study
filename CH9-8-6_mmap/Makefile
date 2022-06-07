BASEINCLUDE ?= /lib/modules/`uname -r`/build

mmap_dev_modu-objs := mmap_dev.o

obj-m := mmap_dev_modu.o

all :
	$(MAKE) -C $(BASEINCLUDE) M=$(PWD) modules;

clean:
	$(MAKE) -C $(BASEINCLUDE) M=$(PWD) clean;
	rm -f *.ko;


