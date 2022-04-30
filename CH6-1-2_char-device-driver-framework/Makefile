
BASEINCLUDE ?= /lib/modules/`uname -r`/build

obj-m := my_char_driver.o

all : 
	$(MAKE) -C $(BASEINCLUDE) M=$(PWD) modules;

clean:
	$(MAKE) -C $(BASEINCLUDE) M=$(PWD) clean;


