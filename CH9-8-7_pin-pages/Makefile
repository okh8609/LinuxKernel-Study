BASEINCLUDE ?= /lib/modules/`uname -r`/build

pin_modu-objs := pin.o

obj-m := pin_modu.o

all :
	$(MAKE) -C $(BASEINCLUDE) M=$(PWD) modules;

clean:
	$(MAKE) -C $(BASEINCLUDE) M=$(PWD) clean;
	rm -f *.ko;


