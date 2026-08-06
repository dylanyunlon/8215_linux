################################################################################
#
# qt cluster-QD
#
################################################################################
ifeq  ($(ATC_AB_PARTITION_SUPPORT),true)
ifneq ($(ATC_DDR_SIZE),128)
CLUSTER_QD_VERSION = 1.0
CLUSTER_QD_CODE_PATH = $(TOPDIR)/../source/packages/cluster/qd_plus_demo/egl_test

CLUSTER_QD_SITE = $(TOPDIR)/../source/packages/cluster/qd_plus_demo/egl_test

CLUSTER_QD_SITE_METHOD = local
CLUSTER_QD_DEPENDENCIES += vba  mali-t400
CLUSTER_QD_ALWAYS_BUILD = YES

define CLUSTER_QD_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) -C $(@D)
endef

define CLUSTER_QD_INSTALL_TARGET_CMDS
	$(INSTALL) -D -m0755 $(@D)/QD_demo $(TARGET_DIR)/usr/bin
	mkdir -p $(TARGET_DIR)/usr/share/QD_HMI/HMI_Data
	$(INSTALL) -D -m0755 $(@D)/QD_HMI/HMI_Data/StResource.bin $(TARGET_DIR)/usr/share/QD_HMI/HMI_Data
	$(INSTALL) -D -m0755 $(@D)/cluster-qd_start.sh $(TARGET_DIR)/etc/init.d/S01cluster-qd
endef

$(eval $(generic-package))
else
$(warning QD is disabled beacause ATC_DDR_SIZE is == 128)
endif
else
$(warning QD is disabled if AB_PARTITION_SUPPORT is false)
endif
