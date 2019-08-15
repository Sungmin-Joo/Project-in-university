# Makefile for a basic kernel module
obj-m += hc-sr04_driver.o
KDIR:= /work/achroimx6q/kernel
PWD :=$(shell pwd)\
	
all: driver app

driver:
	make -C $(KDIR) SUBDIRS=$(PWD) modules
app:
	arm-none-linux-gnueabi-gcc hc-sr04_app.c -o us_test -static
install:
	cp -a *.ko /nfsroot
	cp -a *test /nfsroot
clean:
	rm -rf *.ko
	rm -rf *.mod*
	rm -rf .*.cmd
	rm -rf *.o
	rm -rf Module.symvers
	rm -rf modules.order
	rm -rf us_test
