obj-m += led_driver.o
KDIR = /home/ubuntu/Embedded_OS_ws/src/lab2/linux

all:
	make ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- -C  $(KDIR) M=$(shell pwd) modules 
clean:
	make ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- -C $(KDIR) M=$(pwd) clean