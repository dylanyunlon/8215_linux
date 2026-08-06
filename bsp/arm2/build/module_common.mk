#
#
#  ARM2 Module compile rules
#  Wrote by Ke Xu@atc0036

ALLOBJS      := $(addprefix $(TARGETOBJDIR)/,$(ALLOBJS))
DEPS         := $(ALLOBJS:%o=%d)
GLOBAL_CFLAG += $(LOCAL_CFLAG)
GLOBAL_INC   += $(LOCAL_INCULE)

LDSCRIPT ?= $(ARM2BUILDDIR)/arm2_dram.lds

$(TARGETOBJ): OBJ_DIR $(ALLOBJS)
	$(LINK) -r -o $@ -T $(LDSCRIPT) -Bstatic -Map $(ARM2INFODIR)/$(SUBLINKOBJ).map --stats $(ALLOBJS)

include $(ARM2BUILDDIR)/compile.mk
	
OBJ_DIR:
	mkdir -p $(TARGETOBJDIR)

ifeq ($(filter $(MAKECMDGOALS), clean), )
 -include $(DEPS)
endif

# Clean all obj targets
clean:
	rm -rf $(TARGETOBJ)
	rm -rf $(TARGETOBJDIR)

