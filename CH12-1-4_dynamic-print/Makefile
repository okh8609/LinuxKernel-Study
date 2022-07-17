BASEINCLUDE ?= /lib/modules/`uname -r`/build

my-dyn-pri-objs := my_dyn_pri.o 

obj-m	:=   my-dyn-pri.o
all : 
	$(MAKE) -C $(BASEINCLUDE) M=$(PWD) modules;

clean:
	$(MAKE) -C $(BASEINCLUDE) M=$(PWD) clean;
	rm -f *.ko;


