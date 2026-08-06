################################################################################
#
# qt cluster-app
#
################################################################################
CLUSTER_APP_VERSION = 1.0

CLUSTER_APP_CODE_PATH = $(TOPDIR)/../source/packages/cluster/cluster_app
CLUSTER_APP_PREBUILD_PATH = $(TOPDIR)/../prebuild/cluster/cluster_app

CLUSTER_APP_SITE = $(shell if [ -d $(CLUSTER_APP_CODE_PATH) ]; then echo $(CLUSTER_APP_CODE_PATH); else echo $(CLUSTER_APP_PREBUILD_PATH); fi)

CLUSTER_APP_SITE_METHOD = local
CLUSTER_APP_DEPENDENCIES = qt5base alsa-lib cluster-service vba
CLUSTER_APP_ALWAYS_BUILD = YES

CLUSTER_APP_CFLAGS += -funwind-tables
CLUSTER_APP_CPPFLAGS += -funwind-tables
CLUSTER_APP_LDFLAGS += -rdynamic


ifeq ($(BR2_PACKAGE_QT5_VERSION_LATEST),y)
CLUSTER_APP_LICENSE = GPL-2.0+ or LGPL-3.0, GPL-3.0 with exception(tools), GFDL-1.3 (docs)
CLUSTER_APP_LICENSE_FILES = LICENSE.GPL2 LICENSE.GPL3 LICENSE.GPL3-EXCEPT LICENSE.LGPL3 LICENSE.FDL
else
CLUSTER_APP_LICENSE = GPL-3.0 or LGPL-2.1 with exception or LGPL-3.0, GFDL-1.3 (docs)
CLUSTER_APP_LICENSE_FILES = LICENSE.GPLv3 LICENSE.LGPLv21 LGPL_EXCEPTION.txt LICENSE.LGPLv3 LICENSE.FDL
endif

define CLUSTER_APP_CONFIGURE_CMDS
	@if [ -d $(CLUSTER_APP_CODE_PATH) ]; then \
		echo "Driectory  $(CLUSTER_APP_CODE_PATH) found. qmake."; \
		cd $(@D); $(TARGET_MAKE_ENV) $(HOST_DIR)/bin/qmake; \
	else \
		echo "Driectory  $(CLUSTER_APP_CODE_PATH) not found. skipping qmake."; \
	fi
endef



define CLUSTER_APP_BUILD_CMDS
	@if [ -d $(CLUSTER_APP_CODE_PATH) ]; then \
		echo "build cluster-app"; \
		$(TARGET_MAKE_ENV) $(MAKE) -C $(@D); \
	else \
		echo "cluster-app Prebuild"; \
	fi
endef

define CLUSTER_APP_INSTALL_TARGET_CMDS
	$(INSTALL) -D -m0755 $(@D)/lib/libcluster-app.so* $(TARGET_DIR)/usr/lib
    $(INSTALL) -m 0755 -D $(@D)/lib/libcluster-app.so* $(STAGING_DIR)/usr/lib
	mkdir -p $(TARGET_DIR)/data/cluster
	$(INSTALL) -D -m0666 $(@D)/cluster-app.cfg $(TARGET_DIR)/data/cluster
	$(INSTALL) -D -m0666 $(@D)/cluster-app_turnsignal.wav $(TARGET_DIR)/data/cluster
	$(INSTALL) -D -m0666 $(@D)/fonts/DroidSansFallback.ttf $(TARGET_DIR)/usr/lib/fonts
endef

$(eval $(generic-package))

