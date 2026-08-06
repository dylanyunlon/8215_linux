################################################################################
#
# carplayconfigs
#
################################################################################
CARLINKCONFIGS_VERSION = 1.0
CARLINKCONFIGS_CODE_PATH = $(TOPDIR)/../source/packages/connectivity/carlink/configs
CARLINKCONFIGS_SITE = $(shell echo $(CARLINKCONFIGS_CODE_PATH))
CARLINKCONFIGS_SITE_METHOD = local
CARLINKCONFIGS_ALWAYS_BUILD = YES
CARLINKCONFIGS_INSTALL_STAGING = YES
CARLINKCONFIGS_DEPENDENCIES = libmetazone universal_utils carplaytests bluecommon blueclient wifi-private

CARLINKCONFIGS_MAKE_ARGS := \
    SYSROOT_DIR=$(TOPDIR)/../out/host/arm-buildroot-linux-gnueabi/sysroot \
    RECURSION_MAK=$(TOPDIR)/../build/mak/recursion_build.mak

define CARLINKCONFIGS_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) -C $(@D) $(CARLINKCONFIGS_MAKE_ARGS) all;
endef

define CARLINKCONFIGS_INSTALL_STAGING_CMDS
    $(INSTALL) -m 0644 -D $(@D)/accessoryinfo/libaccessoryinfo.so $(STAGING_DIR)/usr/lib
    $(INSTALL) -m 0644 -D $(@D)/MFICoprocessor/libMFICoprocessor.so $(STAGING_DIR)/usr/lib
endef

define CARLINKCONFIGS_INSTALL_TARGET_CMDS
    $(INSTALL) -m 0644 -D $(STAGING_DIR)/usr/lib/libaccessoryinfo.so $(TARGET_DIR)/usr/lib
    $(INSTALL) -m 0644 -D $(STAGING_DIR)/usr/lib/libMFICoprocessor.so $(TARGET_DIR)/usr/lib
    $(INSTALL) -d ${TARGET_DIR}/etc/carplay
    $(INSTALL) -d ${TARGET_DIR}/etc/androidauto
    $(INSTALL) -m 0644 ${CARLINKCONFIGS_CODE_PATH}/config/carplay/Accessory.xml ${TARGET_DIR}/etc/carplay
    $(INSTALL) -m 0644 ${CARLINKCONFIGS_CODE_PATH}/config/carplay/CarPlaySession.xml ${TARGET_DIR}/etc/carplay
    $(INSTALL) -m 0644 ${CARLINKCONFIGS_CODE_PATH}/config/carplay/IAP2Session.xml ${TARGET_DIR}/etc/carplay
    $(INSTALL) -m 0644 ${CARLINKCONFIGS_CODE_PATH}/config/carplay/icon_104x104.png ${TARGET_DIR}/etc/carplay
    $(INSTALL) -m 0644 ${CARLINKCONFIGS_CODE_PATH}/config/carplay/icon_180x180.png ${TARGET_DIR}/etc/carplay
    $(INSTALL) -m 0644 ${CARLINKCONFIGS_CODE_PATH}/config/carplay/icon_120x120.png ${TARGET_DIR}/etc/carplay
    $(INSTALL) -m 0644 ${CARLINKCONFIGS_CODE_PATH}/config/carplay/icon_256x256.png ${TARGET_DIR}/etc/carplay
    $(INSTALL) -m 0644 ${CARLINKCONFIGS_CODE_PATH}/config/androidauto/headunitinfo.xml ${TARGET_DIR}/etc/androidauto
endef

$(eval $(generic-package))

