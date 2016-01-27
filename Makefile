# remove the extension in filename. Canvas expects an extension so it was added.

obj-m := Di_You、.o

KERNEL_DIR = /usr/src/linux-headers-3.13.0-32-generic

all:
	$(MAKE) -C $(KERNEL_DIR) SUBDIRS=$(PWD) modules

clean:
	rm -rf *.o *.ko *.mod.* *.symvers *.order *~
