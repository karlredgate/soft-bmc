
KERNELDIR := /lib/modules/`uname -r`/build

all::
	$(MAKE) -C $(KERNELDIR) M=$(PWD) V=1

clean::
	$(MAKE) -C $(KERNELDIR) M=$(PWD) clean


