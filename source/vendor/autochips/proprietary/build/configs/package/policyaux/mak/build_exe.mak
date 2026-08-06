#
#  Build executable.
#  Usage:   include $(BUILD_EXECUTABLE)

include $(ATC_BUILD_DIR)/envconf.mak

include $(ATC_BUILD_DIR)/toolconf.mak

include $(ATC_BUILD_DIR)/paramflag.mak

vpath %.h $(LOCAL_INCLUDES)

CFLAGS += -fPIC  -Wall -O2 -g
CPPFLAGS += -fPIC  -Wall -O2 -g

TAGET_BIN := $(addprefix $(localbindir)/,$(LOCAL_MODULE))
.PHONY: all
all: LOCAL_MODULE
	@$(RM) $(CDEPS) $(CPPDEPS)

.PHONY: LOCAL_MODULE $(LOCAL_MODULE) clean 
LOCAL_MODULE: $(TAGET_BIN)

LOCAL_INSTALL_EXES := $(localbindir)/$(LOCAL_MODULE)
$(warning $(LOCAL_INSTALL_EXES)) 
$(TAGET_BIN): $(OBJS_FROM_C) $(OBJS_FROM_C_PLUS)
	@mkdir -p $(dir $@)
	$(CXX) $(CPPFLAGS) $(LFLAGS) $^ -o $@ $(STATIC_FLAG) $(LOCAL_SHARE_LIBRARIES) -Wl,--no-undefined  -Wl,--whole-archive $(LOCAL_STATIC_LIBRARIES) -Wl,--no-whole-archive


clean:
	$(RM) $(CPPOBJS) $(COBJS) $(locallibdir)/$(LOCAL_MODULE) $(CDEPS) $(CPPDEPS)

include $(ATC_BUILD_DIR)/installpolicy.mak

ifneq "$(MAKECMDGOALS)" "clean"
  -include $(OBJS_DEPS_C)
  -include $(OBJS_DEPS_CPLUS)
endif


