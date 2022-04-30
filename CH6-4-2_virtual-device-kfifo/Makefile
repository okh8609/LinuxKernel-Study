
BASEINCLUDE ?= /lib/modules/`uname -r`/build

obj-m := my_virtual_device.o

all : 
	$(MAKE) -C $(BASEINCLUDE) M=$(PWD) modules;

clean:
	$(MAKE) -C $(BASEINCLUDE) M=$(PWD) clean;


