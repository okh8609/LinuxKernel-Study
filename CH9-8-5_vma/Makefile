BASEINCLUDE ?= /lib/modules/`uname -r`/build

vma_m_modu-objs := vma_m.o

obj-m	:=   vma_m_modu.o

all :
	$(MAKE) -C $(BASEINCLUDE) M=$(PWD) modules;

clean:
	$(MAKE) -C $(BASEINCLUDE) M=$(PWD) clean;
	rm -f *.ko;


