BASEINCLUDE ?= /lib/modules/`uname -r`/build

oops_mod9-objs := oops.o

obj-m	:=   oops_mod9.o

all :
	$(MAKE) -C $(BASEINCLUDE) M=$(PWD) modules;

clean:
	$(MAKE) -C $(BASEINCLUDE) M=$(PWD) clean;
	rm -f *.ko;


