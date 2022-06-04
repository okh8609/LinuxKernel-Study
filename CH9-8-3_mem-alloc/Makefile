BASEINCLUDE ?= /lib/modules/`uname -r`/build

mem_alloc_modu-objs := mem_alloc.o

obj-m	:=   mem_alloc_modu.o

all :
	$(MAKE) -C $(BASEINCLUDE) M=$(PWD) modules;

clean:
	$(MAKE) -C $(BASEINCLUDE) M=$(PWD) clean;
	rm -f *.ko;


