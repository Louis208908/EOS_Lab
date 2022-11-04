obj-m += led_driver.o
obj-m += lab3_driver.o
# KDIR放的是我們編譯kernel的路徑
# 可以前往/usr/lib/modules/KERNEL版本/ 使用 ls -al 查看 (build後面會有箭頭顯示)
KDIR = /home/ubuntu/Embedded_OS_ws/src/lab2/linux
CC=aarch64-linux-gnu-
ARCH=arm64

all:
	make -C  $(KDIR) M=$(shell pwd) modules 
	aarch64-linux-gnu-gcc writer.c -o writer
clean:
	make -C $(KDIR) M=$(pwd) clean