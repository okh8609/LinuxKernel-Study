BASEINCLUDE ?= /lib/modules/`uname -r`/build

tkttt-objs := tkt.o 

obj-m	:=   tkttt.o
all : 
	$(MAKE) -C $(BASEINCLUDE) M=$(PWD) modules;

clean:
	$(MAKE) -C $(BASEINCLUDE) M=$(PWD) clean;
	rm -f *.ko;


