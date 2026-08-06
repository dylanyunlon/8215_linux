-include $(ARM2TOPDIR)/build/config.mk


CUR_CSRC     = $(wildcard *.c)
SSRC     = portASM.S

COBJ     = $(patsubst %.c,%.o,$(CUR_CSRC))
SOBJ     = $(subst .S,.o,$(SSRC))

#COBJ_NODIR = $(notdir $(COBJ))

ALLOBJS := $(COBJ) \
           $(SOBJ)

LOCAL_INCULE += -I. -Iinclude -g \


TARGETOBJ    := $(ARM2OBJDIR)/freertos.o
TARGETOBJDIR := $(ARM2OBJDIR)/freertos/obj

include $(ARM2BUILDDIR)/module_common.mk
