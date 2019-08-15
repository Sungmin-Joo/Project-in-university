#Makefile for a basic kernel module
obj-m := fpga_text_lcd_driver.o
KDIR :=/work/achroimx6q/kernel
PWD :=$(shell pwd)
APP := fpga_test_text_lcd
all: driver app
#all: driver
driver:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules
app:
	arm-none-linux-gnueabi-gcc -o $(APP) $(APP).c
install:
	cp -a fpga_text_lcd_driver.ko /nfsroot
	cp -a $(APP) /nfsroot
clean:
	rm -rf *.ko
	rm -rf *.mod.*
	rm -rf *.o
	rm -rf $(APP)
	rm -rf Module.symvers
	rm -rf modules.order
	rm -rf .tmp*
	rm -rf .fpga*
