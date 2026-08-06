################################################################################
#
# preloader
#
################################################################################

TRUSTZONE_SRC = $(TOPDIR)/../bsp/trustzone
TRUSTZONE_TARGET = $(@D)/target/tz.bin
TRUSTZONE_IMG = $(BINARIES_DIR)/tz.bin
LIB_SRC = $(TOPDIR)/../bsp/lib/

TRUSTZONE_VERSION = 
ifeq ($(TRUSTZONE_SRC), $(wildcard $(TRUSTZONE_SRC)))
TRUSTZONE_SITE = $(TRUSTZONE_SRC)
TRUSTZONE_SITE_METHOD = local
endif

#TRUSTZONE_LICENSE = GPL-2.0
#TRUSTZONE_LICENSE_FILES = COPYING
#TRUSTZONE_DEPENDENCIES =

TRUSTZONE_INSTALL_IMAGES = YES

TRUSTZONE_MAKE_OPTS = 

define TRUSTZONE_BUILD_CMDS
	test ! -d $(@D)/../lib || rm -rf $(@D)/../lib
	test ! -d $(LIB_SRC) || cp -arf $(LIB_SRC) $(@D)/../
	test ! -d $(TRUSTZONE_SRC) || $(MAKE) -C $(@D) $(TRUSTZONE_MAKE_OPTS)
endef

define TRUSTZONE_INSTALL_IMAGES_CMDS
	test ! -e $(TRUSTZONE_TARGET) || cp -f $(TRUSTZONE_TARGET) $(TRUSTZONE_IMG)
endef

$(eval $(generic-package))
