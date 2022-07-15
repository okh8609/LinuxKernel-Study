
BASEINCLUDE ?= /lib/modules/`uname -r`/build

kernel-module-objs := kernel_module.o 

obj-m	:=   kernel-module.o
all : 
	$(MAKE) -C $(BASEINCLUDE) M=$(PWD) modules;

clean:
	$(MAKE) -C $(BASEINCLUDE) M=$(PWD) clean;
	rm -f *.ko;


