#add kernel source code path as head file search path
#MTK_PLATFORM := y
#ccflags-y += -I$(srctree)/drivers/misc/mediatek/include
#ccflags-y += -I$(srctree)/drivers/misc/mediatek/include/mt-plat/$(MTK_PLATFORM)/include
ccflags-y += -I$(srctree)/drivers/misc/atc/include/msdc
#ccflags-y += -I$(srctree)/drivers/misc/mediatek/base/power/$(MTK_PLATFORM)
#workaround for include mmc/card/queue.h
ccflags-y += -I$(srctree)/drivers/mmc/card
ccflags-y += -I$(srctree)/drivers/mmc/core
ccflags-y += -I$(srctree)/arch/arm64/boot/dts/include/dt-bindings/mmc
ccflags-y += -I$(srctree)/drivers/misc/atc/inc/
obj-$(CONFIG_MMC_ATC) += sd.o
obj-$(CONFIG_MMC_ATC) += dbg.o
obj-$(CONFIG_MMC_ATC) += mt_sd_misc.o
obj-$(CONFIG_MMC_ATC) += msdc_tune.o
obj-$(CONFIG_MMC_ATC) += msdc_partition.o
obj-$(CONFIG_MMC_ATC) += msdc_io.o
obj-$(CONFIG_MMC_ATC) += autok.o
obj-$(CONFIG_MMC_ATC) += top_layer.o
obj-$(CONFIG_MMC_ATC) += mt_dump.o

