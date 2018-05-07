obj-m := leds.o
#KERNELDIR := /lib/modules/4.4.0-31-generic/build
KERNELDIR := /home/github/bb-black-linux/linux-sdk
PWD := $(shell pwd)

modules:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

modules_install:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules_install

clean:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) clean
