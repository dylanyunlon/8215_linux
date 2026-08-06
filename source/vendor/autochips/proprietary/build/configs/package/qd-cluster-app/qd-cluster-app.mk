################################################################################
#
# qd cluster-app
#
################################################################################

QD_CLUSTER_APP_VERSION = 1.0
QD_CLUSTER_APP_SITE = $(TOPDIR)/../vendor/autochips/proprietary/hardware/samplecode/QD/qd_egl_gbm
QD_CLUSTER_APP_SITE_METHOD = local
QD_CLUSTER_APP_DEPENDENCIES = mali-t82x libdrm
QD_CLUSTER_APP_ALWAYS_BUILD = YES

#QD_CLUSTER_APP_CFLAGS += -funwind-tables
#QD_CLUSTER_APP_CPPFLAGS += -funwind-tables
#QD_CLUSTER_APP_LDFLAGS += -rdynamic


define QD_CLUSTER_APP_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) -C $(@D)
endef


define QD_CLUSTER_APP_INSTALL_TARGET_CMDS
	$(INSTALL) -D -m0755 $(@D)/qd_hmi.bin $(TARGET_DIR)/data/
	$(INSTALL) -D -m0755 $(TOPDIR)/../vendor/autochips/proprietary/hardware/samplecode/QD/qd_egl_gbm/QD_HMI/HMI_Data/StResource.bin $(TARGET_DIR)/data/
endef

$(eval $(generic-package))

