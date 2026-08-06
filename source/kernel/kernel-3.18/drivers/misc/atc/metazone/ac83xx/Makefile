
PWD:=$(shell pwd)

SUBDIR= metazone metazone_drv

export KERNELDIR
include $(BOARD_CFG_FILE)
.phony: meta_zone meta_zone_drv
EXTRA_CFLAGS += -I$(KERNELDIR)/include/generated/

obj-y := metazone_drv/

#all:
#	make meta_zone
#	make meta_zone_drv
	
#meta_zone:
#	make -C metazone

#meta_zone_drv:
#	make -C metazone_drv
	

#clean:
#	for i in ${SUBDIR}; do make clean -C "$${i}"; done



