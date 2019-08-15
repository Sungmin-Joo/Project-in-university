# Makefile for a basic kernel module
obj-m += up_down_pwm.o
KDIR :=/work/achroimx6q/kernel
PWD :=$(shell pwd)
all: driver

driver:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules
app:
	arm-none-linux-gnueabi-gcc ext_sensor_app.c -o ext_sens_test -static
install:
	cp -a *.ko /nfsroot/
	cp -a *test /nfsroot/
clean:
	rm -rf *.ko
	rm -rf *.mod*
	rm -rf .*.cmd
	rm -rf Module.symvers
	rm -rf modules.order
	rm -rf *.o
	rm -rf ext_sens_test
