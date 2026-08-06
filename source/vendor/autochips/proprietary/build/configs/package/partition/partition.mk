################################################################################
#
# partition
#
################################################################################

PARTITION_VERSION =
PARTITION_SITE = $(TOPDIR)/../vendor/autochips/proprietary/tools/PartitionUtility
PARTITION_SITE_METHOD = local
#PARTITION_LICENSE = GPL-2.0
#PARTITION_LICENSE_FILES = COPYING
#PARTITION_DEPENDENCIES =

PARTITION_INSTALL_IMAGES = YES

PARTITION_MAKE_OPTS = \
	ATC_PLATFORM="AC8x_Cluster" \
	ATC_PARTITION_SHEET="emmc_cluster_demo" \
	ATC_PARTITION_SHEET_EVB="emmc_cluster" \
	ATC_SHARED_SDCARD_FUNC=no \
	ATC_AB_UPGRADE=$(ATC_AB_UPGRADE) \
	BOARD_AVB_ENABLE=$(BOARD_AVB_ENABLE)

define PARTITION_BUILD_CMDS
	$(MAKE) -C $(@D) $(PARTITION_MAKE_OPTS)
endef

define PARTITION_INSTALL_IMAGES_CMDS
	cp $(@D)/build_out/scatter.mmcboot.ext4*.xml  $(BINARIES_DIR)
	cp $(TOPDIR)/../vendor/autochips/proprietary/tools/ATCUpgradeTool/  $(BINARIES_DIR) -rf
endef

$(eval $(generic-package))
