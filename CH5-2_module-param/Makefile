BASEINCLUDE ?= /lib/modules/`uname -r`/build

obj-m := param.o

all:
	$(MAKE) -C $(BASEINCLUDE) M=$(PWD) modules;

clean:
	$(MAKE) -C $(BASEINCLUDE) M=$(PWD) clean;
