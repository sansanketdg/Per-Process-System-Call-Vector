obj-m += ioctl_module.o vector1.o vector2.o sys_clone2.o dynamic_vector.o
vector1-y := vector1_module.o overridden_syscalls.o
vector2-y := vector2_module.o

all:  module

module:
	make -C /lib/modules/$(shell uname -r)/build -I/usr/src/hw3-cse506p17/include M=$(PWD) modules
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
