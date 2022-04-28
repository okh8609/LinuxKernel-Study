BASEINCLUDE ?= /lib/modules/`uname -r`/build

obj-m := module-a.o

all:
	$(MAKE) -C $(BASEINCLUDE) M=$(PWD) modules;

clean:
	$(MAKE) -C $(BASEINCLUDE) M=$(PWD) clean;
