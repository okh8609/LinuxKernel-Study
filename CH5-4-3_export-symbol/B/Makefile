KBUILD_EXTRA_SYMBOLS = /home/khaos/LinuxKernel-Study/CH5-4-3_export-symbol/A/Module.symvers

BASEINCLUDE ?= /lib/modules/`uname -r`/build

obj-m := module-b.o

all:
	$(MAKE) -C $(BASEINCLUDE) M=$(PWD) modules;

clean:
	$(MAKE) -C $(BASEINCLUDE) M=$(PWD) clean;
