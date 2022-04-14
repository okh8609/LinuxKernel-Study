CONFIG_MODULE_SIG=n

# 交叉編譯(Cross compile)
BASEINCLUDE ?= /lib/modules/`uname -r`/build

# 本地編譯
# BASEINCLUDE ?= /usr/src/linux

hello-world-objs := hello_world.o 

obj-m	:= hello-world.o
all : 
# 當make的目標為all時，
# -C $(KDIR) 指明跳轉到源碼目錄下讀取那裡的Makefile；
# M=$(PWD) 表明然後返回到目前的目錄繼續讀入、執行當前的Makefile。
	$(MAKE) -C $(BASEINCLUDE) M=$(PWD) modules;

clean:
	$(MAKE) -C $(BASEINCLUDE) M=$(PWD) clean;
	rm -f *.ko;


