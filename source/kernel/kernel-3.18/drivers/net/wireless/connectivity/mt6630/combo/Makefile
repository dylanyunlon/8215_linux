PWD := $(shell pwd)

ifneq ($(KERNELRELEASE),)

#subdir-ccflags-y += -Werror -I$(srctree)/drivers/misc/mediatek/include/mt-plat/$(MTK_PLATFORM)/include
#subdir-ccflags-y += -Werror -I$(srctree)/drivers/misc/mediatek/include/mt-plat
#subdir-ccflags-y += -D MT6630

obj-$(CONFIG_MTK_COMBO) += common/

ifeq ($(CONFIG_ATC_OS_VERSION),"jb2")
    export CONFIG_MTK_COMBO_WIFI := m
    obj-$(CONFIG_MTK_COMBO_WIFI) += drv_wlan/
else
    obj-$(CONFIG_MTK_COMBO) += wlan/
endif

else
     KERNELDIR ?= /lib/modules/$(shell uname -r)/build

default:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

endif
