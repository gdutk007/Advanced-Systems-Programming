
obj-m := char_driver.o

KERNEL_DIR = /usr/src/linux-headers-$(shell uname -r)

all:
	$(MAKE) -C $(KERNEL_DIR) SUBDIRS=$(shell pwd)
	
app:
	gcc -o testing testing_kmod.c
	gcc -o userapp userapp.c

clean:
	rm -rf *.o *.ko *.mod.* *.symvers *.order *~
