#
#
#
ccflags-y += -I$(srctree)/drivers/misc/atc/inc
ccflags-y += -I$(srctree)/include/media/atc/ac823x
ccflags-y += -I$(srctree)/drivers/media/platform/atc/vcp/$(CONFIG_ATC_PLATFORM)/include/

obj-$(CONFIG_VCP_ATC) := vcp.o
vcp-objs := hal/cp_if.o cp_drv.o
