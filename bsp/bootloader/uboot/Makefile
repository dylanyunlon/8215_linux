
unexport LDFLAGS
export CMD_SDAGENT := false
#JOB := 4

export CROSS_COMPILE := arm-buildroot-linux-gnueabi-
#export CROSS_COMPILE ?= armv7a-mediatek451_001_vfp-linux-gnueabi-

define _gen_bootloader_
  cp ./uboot-83xx/u-boot.bin ./tools ;  \
  cd ./tools ;                      \
  cp ./u-boot.bin ../../../image-release ;     \
  cp ./83XX_Preloader_realchip_sd.bin ../../../image-release ;       \
  cp ./83XX_Preloader_realchip_sd_dcon.bin ../../../image-release ;       \
  chmod +x ./multimerge ;       \
  if [ "$(AC83XX_BOOT_DEVICE)" =  "nand" ];then \
    ./multimerge ;                                                   \
    cp ./ac83xx_bootloader_ddr3.bin ../../../image-release ;       \
    cp ./ac83xx_bootloader_ddr3_dcon.bin ../../../image-release ;  \
fi
endef


.PHONY: uboot clean

$(info "PATH:${PATH}")
uboot:
	@$(MAKE) -C ./uboot-83xx mrproper ;\
        $(MAKE) -C ./uboot-83xx ; \
        if [ $$? != 0 ]; then \
        $(MAKE) -C ./uboot-83xx mrproper ; \
        $(MAKE) -C ./uboot-83xx ac83xx_evb_config ; \
        $(MAKE) -C ./uboot-83xx ;   \
        fi
	cp ./uboot-83xx/u-boot.bin ./

#bootloader:
#	$(_gen_bootloader_)

clean:
	$(MAKE) -C ./uboot-83xx clean
