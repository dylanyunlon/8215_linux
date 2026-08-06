###########################################################################
# $RCSfile: Main Makefile,v $
# $Revision: #15 $
# $Date: 2015/12/16 $
# $Author: zhuoming.deng $
#
# Description:
#         Makefile to build a "Linux" demo board build. The following
#         targets are supported:
#
#             all:           Compiles uboot/kernel/oss/qt and app. then copy binary
#                            to out/ directory
#             clean:         Cleans the linux output
#             clean_kernel:  Cleans the dtv build (mt5395_atsc_linux)
#             clean_app:     Cleans the android build (froyo-2.2)
#             uboot:         Builds uboot
#             kernel:        Builds linux kernel
#             drivers:       Build ko module driver
#             arm2:          Build arm2
#             oss:           Build all open source
#             app:           Build app
#             rootfs
#
#         The following commands are supported:
#
#             BUILD_CFG=debug
#                 Builds the specified target with symbolic debug info
#
#############################################################################
include $(DA_TOP)/$(TOP_MAKE)/host.mak

ifeq "$(BUILD_CFG)" "rel"
    KCONFIG_FILE := $(KERNEL_SRC)/arch/arm/configs/ac83xx_defconfig
else
    KCONFIG_FILE := $(KERNEL_SRC)/arch/arm/configs/ac83xx_defconfig_debug
endif

LINUX_BIN_DIR=$(KERNEL_SRC)/arch/arm/boot

export PATH := $(LINUX_BIN_DIR):$(PATH)


.PHONY: uboot kernel clean_kernel clean_uboot

#uboot: 
#	cd $(UBOOT_SRC) && \
#	rm -f u-boot.bin &&  rm -f ../tools/ac83xx_bootloader_ddr3.bin && \
#	chmod +x gen_bootloader.sh && \
#	$(UBOOT_SRC)/gen_bootloader.sh || exit 
		
kernel:
	@ echo -e "\033[44;32m kernel build start \033[0m"
ifeq "$(BOOT_DEVICE)" "nand"

ifeq "$(FILE_SYSTEM)"  "ubi"
	@cd $(KERNEL_SRC) && $(FMAKE) ARCH=arm ac83xx_nand_ubi_defconfig && $(FMAKE) ARCH=arm || exit $?
else
	@cd $(KERNEL_SRC) && $(FMAKE) ARCH=arm ac83xx_nand_defconfig && $(FMAKE) ARCH=arm || exit $?
endif
else
	@cd $(KERNEL_SRC) && $(FMAKE) ARCH=arm ac83xx_defconfig && $(FMAKE) ARCH=arm || exit $?	
endif
	$(BUILD_TOOLDIR)/mkimage -f $(LINUX_BIN_DIR)/kernel_fdt.its $(LINUX_BIN_DIR)/kernel_fdt.itb || exit $?
	@if [ -e "$(LINUX_BIN_DIR)/uImage" ];then \
	  mv $(LINUX_BIN_DIR)/uImage $(LINUX_BIN_DIR)/uImage.old; \
	fi
	@mv $(LINUX_BIN_DIR)/kernel_fdt.itb  $(LINUX_BIN_DIR)/uImage
	@if [ ! -d $(IMG_RLEASE) ]; then \
		$(MKDIR) $(MKDIR_FLAG) $(IMG_REL); \
	fi
	#$(CP) $(CP_FLAG) $(KERNELDIR)/arch/arm/boot/uImage $(IMG_REL)/
	$(CP) $(CP_FLAG) $(KERNELDIR)/vmlinux $(IMG_REL)/
	# copy zImage / Image /ac83xx.dtb to image_release dir
	$(CP) $(CP_FLAG) $(LINUX_BIN_DIR)/zImage $(IMG_REL)/zImage.bin
	$(CP) $(CP_FLAG) $(LINUX_BIN_DIR)/Image $(IMG_REL)/Image.bin

ifeq "$(BOOT_DEVICE)" "nand"
ifeq "$(FILE_SYSTEM)"  "ubi"
	$(CP) $(CP_FLAG) $(LINUX_BIN_DIR)/dts/ac83xx_nand_ubi.dtb  $(IMG_REL)/ac83xx.dtb.bin
else
	$(CP) $(CP_FLAG) $(LINUX_BIN_DIR)/dts/ac83xx_nand_ext4.dtb  $(IMG_REL)/ac83xx.dtb.bin
endif
else
	$(CP) $(CP_FLAG) $(LINUX_BIN_DIR)/dts/ac83xx.dtb $(IMG_REL)/ac83xx.dtb.bin
endif

#	$(CP) $(CP_FLAG) $(KERNELDIR)/vmlinux $(KERNEL_OBJ_ROOT)/
	@ echo -e "\033[44;32m kernel build End \033[0m"

################################################################
# Before clean kernel, make sure last compilation has been done 
################################################################ 
#clean_kernel:
#	@echo "clean kernel"
#	@if [ ! -e "$KCONFIG_FILE" ];then \
#	    echo "no configure file, you can not clean! " \
#	    exit 1 ; \
#	fi
#	@cd $(KERNELDIR) && $(FMAKE) clean && $(FMAKE) mrproper && $(FMAKE) ARCH=arm ac83xx_defconfig && make modules_prepare

#clean_uboot:
#	@echo "clean uboot"
