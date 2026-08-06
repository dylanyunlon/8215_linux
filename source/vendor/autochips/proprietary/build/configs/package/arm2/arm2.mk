################################################################################
#
# preloader
#
################################################################################

ARM2_SRC = $(TOPDIR)/../bsp/arm2
ARM2_TARGET = $(@D)/obj/arm2.bin
ARM2_IMG = $(BINARIES_DIR)/arm2.bin
LIB_SRC = $(TOPDIR)/../bsp/lib/

ARM2_VERSION = 
ifeq ($(ARM2_SRC), $(wildcard $(ARM2_SRC)))
ARM2_SITE = $(ARM2_SRC)
ARM2_SITE_METHOD = local
endif

#ARM2_LICENSE = GPL-2.0
#ARM2_LICENSE_FILES = COPYING
#ARM2_DEPENDENCIES =

ARM2_INSTALL_IMAGES = YES

ARM2_MAKE_OPTS = \
	target_project=linux-ac83xx \
	O=$(@D)/obj

define ARM2_BUILD_CMDS
	test ! -d $(@D)/../lib || rm -rf $(@D)/../lib
	test ! -d $(LIB_SRC) || cp -arf $(LIB_SRC) $(@D)/../
	test ! -d $(ARM2_SRC) || $(MAKE) -C $(@D) $(ARM2_MAKE_OPTS)
endef

define ARM2_INSTALL_IMAGES_CMDS
	test ! -e $(ARM2_TARGET) || cp -f $(ARM2_TARGET) $(ARM2_IMG)
	test ! -e $(ARM2_TARGET) || cp -f $(ARM2_TARGET) $(ARM2_IMG)
	cp -f $(@D)/obj/arm2 $(BINARIES_DIR)/arm2.elf
	cp -f $(@D)/obj/System.map $(BINARIES_DIR)/arm2.map
	cp -f $(@D)/obj/arm2.debug.lst $(BINARIES_DIR)/
endef

$(eval $(generic-package))
