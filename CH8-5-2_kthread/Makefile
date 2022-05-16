BASEINCLUDE ?= /lib/modules/`uname -r`/build

kthread_mod8-objs := kthread.o

obj-m	:=   kthread_mod8.o

all :
	$(MAKE) -C $(BASEINCLUDE) M=$(PWD) modules;

clean:
	$(MAKE) -C $(BASEINCLUDE) M=$(PWD) clean;
	rm -f *.ko;


