
KERNELDIR := /lib/modules/`uname -r`/build

all: module shoot

module:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) V=1

shoot: shoot.o
	$(CC) -o shoot $^

clean::
	$(MAKE) -C $(KERNELDIR) M=$(PWD) clean
	rm -f shoot shoot.o
	rm -f Module.symvers

