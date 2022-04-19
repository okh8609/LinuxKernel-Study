BASEINCLUDE ?= /lib/modules/`uname -r`/build

rb_test-objs := rbtree_test.o 

obj-m	:=   rb_test.o
all : 
	$(MAKE) -C $(BASEINCLUDE) M=$(PWD) modules;

clean:
	$(MAKE) -C $(BASEINCLUDE) M=$(PWD) clean;
	rm -f *.ko;


# 當make的目標為all時
# -C $(KERNEL_DIR) 指明跳轉到源碼目錄下讀取那裡的Makefile
# M=$(PWD) 表明然後返回到目前的目錄繼續讀入、執行當前的Makefile。