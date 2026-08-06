#
#  For ac8x hsm build
#
HSM_SRC := $(TOP_DIR)hsm
HSM_OUT := $(TARGET_OUT_DIR)/HSM_OBJ/BOOTLOADER_OBJ

ifeq ($(HSM_SRC), $(wildcard $(HSM_SRC)))
$(INSTALLED_HSM_TARGET): hsm

.PHONY: hsm
hsm: 
	cd $(HSM_SRC)
	mkdir -p $(HSM_OUT)
	make -C $(HSM_SRC)/preloader O=$(HSM_OUT) 
	cp $(HSM_OUT)/target/8015_Preloader_realchip_sd.bin $(TARGET_OUT_DIR)
	@echo "hsm build done ->:)"

hsm-clean:
	rm -rf $(HSM_OUT)/info/*
	rm -rf $(HSM_OUT)/obj/*
	rm -rf $(HSM_OUT)/target/*
	rm -rf $(HSM_OUT)

endif

