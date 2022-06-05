BASEINCLUDE ?= /lib/modules/`uname -r`/build

slab_m_modu-objs := slab_m.o

obj-m	:=   slab_m_modu.o

all :
	$(MAKE) -C $(BASEINCLUDE) M=$(PWD) modules;

clean:
	$(MAKE) -C $(BASEINCLUDE) M=$(PWD) clean;
	rm -f *.ko;


