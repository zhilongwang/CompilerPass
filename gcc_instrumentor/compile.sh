#obj-m +=etm_module.o
obj-m += testmod.o
#obj-m +=src/vma.o

#etm_module-objs += src/etm_main.o src/sync.o src/new_syscall.o  src/etm_drive.o src/share_mem.o
testmod-objs += src/testmod.o #src/runtime.o

ccflags-y := -fplugin=/home/ubuntu/etmtaint/kernel_instrumentor/gcc_plugin/kernel_instrumentor.so
ldflags-y := /home/ubuntu/etmtaint/kernel_instrumentor/runtime-rt/runtime.o

all:
	make V=1 -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

install:
	sudo insmod testmod.ko 
remove:
	sudo rmmod testmod.ko 
