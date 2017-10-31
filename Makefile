FNAME := BufferedMem

obj-m += $(FNAME).o

KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

all : 
	make -C $(KDIR) M=$(PWD) modules
clean :
	rm -rf $(FNAME).o $(FNAME).mod.* $(FNAME).ko
