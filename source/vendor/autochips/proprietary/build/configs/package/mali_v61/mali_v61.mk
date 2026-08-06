
MALI_V61_CODE_PATH = $(TOPDIR)/../vendor/autochips/proprietary/hardware/media/multimedia/omx/mali_v61
IS_SRC_EXIST = $(shell if [ -d $(MALI_V61_CODE_PATH) ]; then echo "true"; else echo "false"; fi)
MALI_V61_PREBUILT_PATH = $(TOPDIR)/../prebuilt/mali_v61
MALI_V61_SITE = $(shell if [ -d $(MALI_V61_CODE_PATH) ]; then echo $(MALI_V61_CODE_PATH); else echo $(MALI_V61_PREBUILT_PATH); fi)
MALI_V61_VERSION = 1.0
MALI_V61_SITE_METHOD = local
MALI_V61_ALWAYS_BUILD = YES
MALI_V61_INSTALL_STAGING = YES
MALI_V61_DEPENDENCIES = libdinfox libatccrypto
MALI_V61_PROVIDES = libopenmax
MALI_V61_INSTALL_STAGING = YES
MALI_V61_LDFLAGS = $(TARGET_LDFLAGS)
MALI_V61_CFLAGS = $(TARGET_CFLAGS)

MALI_V61_MAKE_OPTS += TOPDIR=$(TOPDIR)/.. KERNEL_TOPDIR=$(TOPDIR)/../kernel \
                      CC=$(TOPDIR)/../out/host/bin/aarch64-buildroot-linux-gnu-gcc

ENC_TOOLS = "$(TOPDIR)/../vendor/autochips/proprietary/hardware/media/multimedia/omx/mali_v61/omx-components/video/src/tools/enc64.py"
PUB_KEY = "$(TOPDIR)/../vendor/autochips/proprietary/hardware/media/multimedia/omx/mali_v61/omx-components/video/src/tools/pubkey.pem"

define MALI_V61_BUILD_CMDS
	$(MAKE) TOP_DIR="$(TOPDIR)/.."  TARGET_TOP=$(@D) ENABLE_AUDIO=no -C $(@D) $(MALI_V61_MAKE_OPTS) all
endef

define MALI_V61_INSTALL_STAGING_CMDS
	if [  -e $(TARGET_DIR)/etc/firmware ]; then echo "file already exists"; else mkdir $(TARGET_DIR)/etc/firmware; fi;
	cp $(TOPDIR)/../vendor/autochips/proprietary/hardware/media/multimedia/firmware/h264dec.fwb $(TARGET_DIR)/etc/firmware
	cp -rf $(TOPDIR)/../prebuilt/mali_v61/openmax/*.h $(STAGING_DIR)/usr/include/
	(cd $(@D); \
	export includedir="/usr/include"; \
	make install DESTDIR=$(STAGING_DIR) TARGET_TOP=$(@D))
	@if [ "$(IS_SRC_EXIST)" = "true" ]; then \
		python $(ENC_TOOLS) $(STAGING_DIR)/usr/lib64/libmveomx.so $(PUB_KEY); \
	fi
endef

define MALI_V61_INSTALL_TARGET_CMDS
	(cd $(@D); \
	export includedir="/usr/include"; \
	make install_lib DESTDIR=$(TARGET_DIR) TARGET_TOP=$(@D))
	@if [ "$(IS_SRC_EXIST)" = "true" ]; then \
		python $(ENC_TOOLS) $(TARGET_DIR)/usr/lib64/libmveomx.so $(PUB_KEY); \
	fi
endef

$(eval $(generic-package))
