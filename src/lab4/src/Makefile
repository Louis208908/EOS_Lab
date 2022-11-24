obj-m += lab4_driver.o



# KDIR放的是我們編譯kernel的路徑

# 可以前往/usr/lib/modules/KERNEL版本/ 使用 ls -al 查看 (build後面會有箭頭顯示)

KDIR = /home/ubuntu/Embedded_OS_ws/src/lab2/linux



all:

	make ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- -C  $(KDIR) M=$(shell pwd) modules 

	aarch64-linux-gnu-gcc writer.c -o writer

	aarch64-linux-gnu-gcc reader.c -o reader
clean:

	make ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- -C $(KDIR) M=$(shell pwd) clean

	rm writer

	rm reader
