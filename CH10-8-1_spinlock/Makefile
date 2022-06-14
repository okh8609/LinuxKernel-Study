
BASEINCLUDE ?= /lib/modules/`uname -r`/build

spinlock-ok-objs := spinlock_OK.o

obj-m	:=   spinlock-ok.o
all :
	$(MAKE) -C $(BASEINCLUDE) M=$(PWD) modules;
clean:
	$(MAKE) -C $(BASEINCLUDE) M=$(PWD) clean;
	rm -f *.ko;

