

CONFIGFILE   := $(KERNEL_DEFCONFIG)
#CONFIGFILE   := ac83xx_m_defconfig
#KARCH        := arm
KARCH        := $(TARGET_ARCH)
#DTSNAME      := ac83xx_m
DTSNAME      := $(KERNEL_DTSNAME)
KBUILD_DEBUG := 0

export KERNELDIR       := $(ANDROID_BUILD_TOP)/kernel/kernel-3.18
export KERNEL_SRCDIR   := $(KERNELDIR)
ifeq "$(KARCH)" "arm64"
# use prebuilt toolchain
export CROSS_COMPILE   := $(ANDROID_BUILD_TOP)/prebuilts/gcc/linux-x86/aarch64/aarch64-linux-android-4.9/bin/aarch64-linux-android-
else
# use atc compile
export CROSS_COMPILE   := armv7a-mediatek451_001_vfp-linux-gnueabi-
endif
export KERNEL_BUILDDIR := $(ANDROID_BUILD_TOP)/$(PRODUCT_OUT)/obj/KERNEL_OBJ
export KERNEL_EXT_LIBS  := $(ANDROID_BUILD_TOP)/kernel/klibs
STRIP  = $(CROSS_COMPILE)strip
ifeq "$(ATC_AOSP_ENHANCEMENT_CTS)" "yes"
CTS_FLAG := ATC_AOSP_ENHANCEMENT_CTS=yes
else
CTS_FLAG := ATC_AOSP_ENHANCEMENT_CTS=no
endif


CUSTOMER_NAME ?= atc
DEVICE_NAME   ?= ac8317
BOARD_NAME    ?= evb

.PHONY: kernel kernel_config kernel_clean

droid: $(PRODUCT_OUT)/kernel

$(PRODUCT_OUT)/kernel: kernel kernel_install 

kernel_install: module_prepare kernel gen-atc-header protect_libs
	$(hide) cp -f $(KERNEL_BUILDDIR)/arch/$(KARCH)/boot/Image $(ANDROID_BUILD_TOP)/$(PRODUCT_OUT)/kernel
	$(hide) cp -f $(KERNEL_BUILDDIR)/arch/$(KARCH)/boot/dts/$(DTSNAME).dtb $(ANDROID_BUILD_TOP)/$(PRODUCT_OUT)/
	$(hide) mkdir -p $(ANDROID_BUILD_TOP)/$(PRODUCT_OUT)/system/drivers
	$(hide) for m in `find $(KERNEL_BUILDDIR)/ -name *.ko`; do  \
		cp -f $$m $(ANDROID_BUILD_TOP)/$(PRODUCT_OUT)/system/drivers ; \
	done
	$(STRIP) --strip-debug $(ANDROID_BUILD_TOP)/$(PRODUCT_OUT)/system/drivers/*.ko

#Note, for compile modules
module_prepare: kernel_config gen-atc-header
	$(hide) make -C $(KERNEL_SRCDIR) ARCH=$(KARCH) modules_prepare O=$(KERNEL_BUILDDIR) $(CTS_FLAG)  

#Note
protect_libs: module_prepare
	$(hide)cd $(KERNEL_EXT_LIBS)/libs/ && make DEVICE=$(MTK_TARGET_PROJECT) OS=android $(CTS_FLAG) 

kernel: kernel_config gen-atc-header protect_libs
	$(hide) make -C $(KERNEL_SRCDIR) ARCH=$(KARCH) O=$(KERNEL_BUILDDIR) V=$(KBUILD_DEBUG) $(CTS_FLAG) -j24

$(KERNEL_MODULES_DEPS): kernel

kernel_config: 
	$(hide) make -C $(KERNEL_SRCDIR) ARCH=$(KARCH) O=$(KERNEL_BUILDDIR) $(CONFIGFILE) V=$(KBUILD_DEBUG) $(CTS_FLAG) 

## param:
##   $1  defconfig file path
##   $2  output path
##   $3  project name
##   $4  platform name
##   $5  os
##   $6  os version
##   $7  customer name
##   $8  device
##   $9  board

gen-atc-header: kernel_config
	$(hide) sh $(KERNEL_SRCDIR)/scripts/gen_atc_project.sh $(KERNEL_SRCDIR)/arch/$(KARCH)/configs/$(CONFIGFILE) $(KERNEL_BUILDDIR) $(MTK_PROJECT)  `echo $(ATC_PLATFORM) | tr '[A-Z]' '[a-z]'`  android m $(CUSTOMER_NAME) $(DEVICE_NAME) $(BOARD_NAME) 
ifeq ($(MTK_PROJECT), ac823x_evb)
	$(hide) perl $(ANDROID_BUILD_TOP)/vendor/atc/proprietary/tools/PartitionUtility/ParsePartition2Header.pl $(ANDROID_BUILD_TOP)/vendor/atc/proprietary/tools/PartitionUtility/AndroidM_Partition_Table_AC823x.xls $(KERNEL_SRCDIR)/include/linux/mmc/atc_storage_partition.h
else
	$(hide) perl $(ANDROID_BUILD_TOP)/vendor/atc/proprietary/tools/PartitionUtility/ParsePartition2Header.pl $(ANDROID_BUILD_TOP)/vendor/atc/proprietary/tools/PartitionUtility/AndroidM_Partition_Table_AC8317.xls $(KERNEL_SRCDIR)/include/linux/mmc/atc_storage_partition.h
endif

kernel_clean:
	$(hide) make -C $(KERNEL_SRCDIR) ARCH=$(KARCH) mrproper $(CTS_FLAG)
	$(hide) rm -rf $(KERNEL_BUILDDIR)

$(BUILT_SYSTEMIMAGE): $(PRODUCT_OUT)/kernel
