BASEINCLUDE ?= /lib/modules/`uname -r`/build

procfs-test-objs := procfs_test.o 

obj-m	:=   procfs-test.o
all : 
	$(MAKE) -C $(BASEINCLUDE) M=$(PWD) modules;

clean:
	$(MAKE) -C $(BASEINCLUDE) M=$(PWD) clean;
	rm -f *.ko;


