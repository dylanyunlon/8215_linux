#
#  Build static lib.
#  Usage:   include $(BUILD_STATIC_LIBRARY)

include $(ATC_BUILD_DIR)/envconf.mak

include $(ATC_BUILD_DIR)/toolconf.mak

include $(ATC_BUILD_DIR)/paramflag.mak

vpath %.h $(LOCAL_INCLUDES)

CFLAGS += -fPIC -shared -Wall -O2 -g
CPPFLAGS += -fPIC -shared -Wall -O2 -g

TARGET_STATIC := $(addprefix $(locallibdir)/,$(LOCAL_MODULE))
.PHONY: all
all: LOCAL_MODULE
	$(RM) $(CDEPS) $(CPPDEPS)

.PHONY: LOCAL_MODULE $(LOCAL_MODULE) clean 
LOCAL_MODULE: $(TARGET_STATIC)

LOCAL_INSTALL_LIBS := $(locallibdir)/$(LOCAL_MODULE)

$(TARGET_STATIC): $(OBJS_FROM_C) $(OBJS_FROM_C_PLUS)
	@mkdir -p $(dir $@)
	$(AR) crD  $@ $^ 
#	@if [ ! -d $(locallibdir) ]; then mkdir -p $(locallibdir); fi
#	@$(MV) $(LOCAL_MODULE) $(locallibdir);

clean:
	$(RM) $(CPPOBJS) $(COBJS) $(locallibdir)/$(LOCAL_MODULE) $(CDEPS) $(CPPDEPS)

include $(ATC_BUILD_DIR)/installpolicy.mak

ifneq "$(MAKECMDGOALS)" "clean"
  -include $(OBJS_DEPS_C)
  -include $(OBJS_DEPS_CPLUS)
endif

