obj-m += kvict.o

kvict-objs += source/main.o

all:
	make -s -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -s -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
