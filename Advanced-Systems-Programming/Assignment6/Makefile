
obj-m := assignment6.o

KERNEL_DIR = /usr/src/linux-headers-$(shell uname -r)

all:
	$(MAKE) -C $(KERNEL_DIR) SUBDIRS=$(shell pwd)
	
app:
	g++ -std=c++11  case1.cpp -o case1 -lpthread
	g++ -std=c++11  case2.cpp -o case2 -lpthread
	g++ -std=c++11  case3.cpp -o case3 -lpthread
	g++ -std=c++11  case4.cpp -o case4 -lpthread

clean:
	rm -rf *.o *.ko *.mod.* *.symvers *.order *~
