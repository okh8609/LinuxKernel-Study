BASEINCLUDE ?= /lib/modules/`uname -r`/build

mem_info_modu-objs := mem_info.o

obj-m	:=   mem_info_modu.o

all :
	$(MAKE) -C $(BASEINCLUDE) M=$(PWD) modules;

clean:
	$(MAKE) -C $(BASEINCLUDE) M=$(PWD) clean;
	rm -f *.ko;


