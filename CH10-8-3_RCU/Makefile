
BASEINCLUDE ?= /lib/modules/`uname -r`/build
ccflags-y :=-g

rcu-ddd-objs := rcu_dd.o 

obj-m	:=   rcu-ddd.o
all : 
	$(MAKE) -C $(BASEINCLUDE) M=$(PWD) modules;

clean:
	$(MAKE) -C $(BASEINCLUDE) M=$(PWD) clean;
	rm -f *.ko;


