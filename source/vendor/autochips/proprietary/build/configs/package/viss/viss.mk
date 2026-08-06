################################################################################
#
# viss
#
################################################################################

VISS_VERSION =
VISS_SITE = $(TOPDIR)/../vendor/autochips/proprietary/tinysys/viss/os
VISS_SITE_METHOD = local
#VISS_LICENSE = GPL-2.0
#VISS_LICENSE_FILES = COPYING
#VISS_DEPENDENCIES =

VISS_PROJECT = ac8x_car
VISS_INSTALL_IMAGES = YES

TINYSYS_CROSS_COMPILE = $(TOPDIR)/../vendor/autochips/proprietary/tinysys/rtos/toolchain/7.2.1/bin/arm-none-eabi-

# for viss framework parameters
VISS_MAKE_OPTS = \
	target=realchip \
	bootdevice=mmc \
	target_project=ac8015_android \
	TINYSYS_CROSS_COMPILE=$(TINYSYS_CROSS_COMPILE) \
	VISS_TOP_PATH=$(VISS_SITE) \
	KENREL_VIDEOIN_PATH=$(TOPDIR)/../kernel/kernel-4.9/drivers/soc/autochips/videoin \
	KENREL_HEADER_PATH=$(TOPDIR)/../kernel/kernel-4.9/drivers/soc/autochips \
	ROOTDIR=$(SRC_PATH) \
	TARGET_BUILD_VARIANT=$(TARGET_BUILD_VARIANT) \
	ATC_MEMORY_OPTIMIZATION=$(ATC_MEMORY_OPTIMIZATION) \
	ATC_METAZONE_SUPPORT=$(ATC_METAZONE_SUPPORT) \
	ATC_LOGO_SUPPORT=$(ATC_LOGO_SUPPORT) \
	SECURE_BOOT_ENABLE=$(SECURE_BOOT_ENABLE) \
	ATC_WATCHDOG_ENABLE=$(ATC_WATCHDOG_ENABLE) \
	ATC_3P1_SUPPORT=$(ATC_3P1_SUPPORT) \
	ATC_XEN_SUPPORT=$(ATC_XEN_SUPPORT) \
	ATC_CLUSTER_SUPPORT=$(ATC_CLUSTER_SUPPORT) \
	SRC_TOOLS_DIR_PATH=$(TOPDIR)/../vendor/autochips/proprietary/tools

define VISS_BUILD_CMDS
	$(MAKE) -C $(@D) $(VISS_MAKE_OPTS) out=$(@D)/VISS_OBJ_LPDDR4
	$(MAKE) -C $(@D) $(VISS_MAKE_OPTS) target_device=ac8x_demo out=$(@D)/VISS_OBJ_LPDDR4_DEMO
	#$(MAKE) -C $(@D) $(VISS_MAKE_OPTS) ddr_type=DDR_DDR4 out=$(@D)/VISS_OBJ_DDR4
	$(MAKE) -C $(@D) $(VISS_MAKE_OPTS) ddr_type=DDR_DDR3 target_device=ac8x_demo out=$(@D)/VISS_OBJ_DDR3_DEMO
endef

define VISS_INSTALL_IMAGES_CMDS
	cp $(@D)/VISS_OBJ_LPDDR4/viss_header.bin $(BINARIES_DIR)/viss.bin
	cp $(@D)/VISS_OBJ_LPDDR4_DEMO/viss_header.bin $(BINARIES_DIR)/viss_demo.bin
	#cp $(@D)/VISS_OBJ_DDR4/viss_header.bin $(BINARIES_DIR)/viss_ddr4.bin
	cp $(@D)/VISS_OBJ_DDR3_DEMO/viss_header.bin $(BINARIES_DIR)/viss_ddr3_demo.bin
endef

$(eval $(generic-package))

