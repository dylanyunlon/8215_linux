ifndef KERNEL_STANDARD_CONFIG
export KERNEL_STANDARD_CONFIG = false
endif

obj-y				+= memrchr.o
obj-y				+= os_linux_mem.o
#obj-y				+= os_linux_timer.o
obj-y				+= os_linux_sema.o
obj-y				+= os_linux_util.o
obj-y				+= os_linux_thread.o
obj-y				+= strtoll.o
obj-y				+= os_linux_event.o

obj-y				+= os_linux_ev_grp.o
obj-y				+= os_linux_msg_q.o

ccflags-y += -I$(srctree)/drivers/misc/atc/inc
ccflags-y += -I$(srctree)/drivers/misc/atc/private_inc
ccflags-y += -I$(srctree)/drivers/misc/atc/libosal/ac823x/inc
