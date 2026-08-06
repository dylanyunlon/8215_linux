-include $(ARM2TOPDIR)/build/config.mk

CSRC :=  vdec_hw_common.c vdec_hal_if_common.c vdec_hw_h264.c vdec_hal_if_h264.c vdec_drv_slt_h264.c
SSRC     = vbaRes.S
COBJ := $(subst .c,.o,$(CSRC))
#SOBJ	 = $(subst .S,.o,$(SSRC))	

ALLOBJS := $(COBJ) \
#           $(SOBJ)

#Use absolute path for finding the source code
NOWDIR  = $(shell pwd)

TARGETOBJ    := $(ARM2OBJDIR)/vba.o
TARGETOBJDIR := $(ARM2OBJDIR)/vba/obj

LDSCRIPT := $(ARM2BUILDDIR)/arm2_dram.lds

include $(ARM2BUILDDIR)/module_common.mk
	
