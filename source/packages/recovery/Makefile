#
#
#
#

# Note: point to server qmake dir

##

export recovery_top := $(PWD)
export recovery_build := $(recovery_top)/build
export recovery_update := $(recovery_top)/recovery-update
export recovery_os := $(recovery_top)/os
export recovery_out := $(recovery_top)/out
export recovery_lib := $(recovery_top)/lib
export recovery_sysroot := $(recovery_out)/sysroot

.PHONY:  lib os rootfs clean_os clean update

all: update lib  
	@echo "Recovery construct done"

os:
	@sh $(recovery_build)/mkos.sh $(KO_DIR)
	
update: lib
	@echo "yzp enter update"
	@cd $(recovery_update)
	@make -C $(recovery_update)
#@echo "app install dir: $(recovery_out)/sysroot"
#	#@make -C $(recovery_update) install INSTALL_ROOT=$(recovery_sysroot)
	

lib:
	@make -C $(recovery_lib)

rootfs:
	@sh $(recovery_build)/mkrootfs.sh
	
clean_os:
	@rm -rf $(recovery_out)/sysroot
	
clean: clean_os
	@rm -rf recovery.gz
	@rm -rf recovery.img.ext4
	@make -C $(recovery_update) distclean
	@make -C $(recovery_lib) clean
