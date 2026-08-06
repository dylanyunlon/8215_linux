MV             := mv -f
RM             := rm -f
SED            := sed
CP             := cp -ruf

STATIC_FLAG    :=
DEBUG_FLAG     :=
CROSS_COMPILE  :=


#ifeq "$(LOCAL_DEBUG)" "true"
  DEBUG_FLAG := -g -rdynamic -funwind-tables
#endif

ifeq "$(LOCAL_FORCE_STATIC_EXECUTABLE)" "true"
  STATIC_FLAG := --static
endif

CROSS_COMPILE := 
ifeq "$(TARGET)" "arm"
  CROSS_COMPILE := $(CROSS_COMPILE_PREFIX)
endif

CC  := $(CROSS_COMPILE)gcc
CXX := $(CROSS_COMPILE)g++
LD  := $(CROSS_COMPILE)ld
strip := $(CROSS_COMPILE)strip

CSRCS       := $(filter %.c,$(LOCAL_SRC_FILES))
COBJS       := $(CSRCS:.c=.o)
CDEPS       := $(CSRCS:.c=.d)

CPPSRCS     := $(filter %.cpp,$(LOCAL_SRC_FILES))
CPPOBJS     := $(CPPSRCS:.cpp=.o)
CPPDEPS     := $(CPPSRCS:.cpp=.d)

CFLAGS      += $(addprefix -D,$(LOCAL_MACRO))
CPPFLAGS    += $(addprefix -D,$(LOCAL_MACRO))

CFLAGS      += $(addprefix -I,$(LOCAL_INCLUDES))
LFLAGS      += $(addprefix -L,$(LOCAL_LIB_PATH))
CPPFLAGS    += $(addprefix -I,$(LOCAL_INCLUDES))

CFLAGS += -fPIC -shared -Wall -O2 --sysroot=$(DA_SYSROOT) $(DEBUG_FLAG)
CPPFLAGS += -fPIC -shared -Wall -O2 --sysroot=$(DA_SYSROOT) $(DEBUG_FLAG)
CPPFLAGS += 
vpath %.h $(LOCAL_INCLUDES)

ifeq "$(LOCAL_DST_DIR)" ""
	LOCAL_DST_DIR := $(DA_LIBDIR)/lib
endif	
ifeq "$(LOCAL_DST_INC)" ""
	LOCAL_DST_INC := $(DA_LIBDIR)/include
endif

.PHONY: all
all: LOCAL_MODULE
	@$(RM) $(CDEPS) $(CPPDEPS)
	@if [ ! -d $(LOCAL_DST_DIR) ]; then \
		mkdir -p $(LOCAL_DST_DIR) ; \
	fi	
	@$(CP) $(LOCAL_MODULE) $(LOCAL_DST_DIR)
	@$(CP) $(LOCAL_MODULE) $(DA_SYSROOT)/usr/lib
	@for i in $(LOCAL_INSTALL_FILES) ; \
	do \
		cp -f $$i $(LOCAL_DST_INC) ; \
		cp -f $$i $(DA_SYSROOT)/usr/include ; \
	done

.PHONY: LOCAL_MODULE $(LOCAL_MODULE)
LOCAL_MODULE: $(LOCAL_MODULE)

$(LOCAL_MODULE): $(COBJS) $(CPPOBJS)
	$(CXX) $(CPPFLAGS) $(LFLAGS)  $^ -o $@ $(STATIC_FLAG) $(LOCAL_SHARE_LIBRARIES) -Wl,--no-undefined -Wl,-soname,$(notdir $@) -Wl,--whole-archive $(LOCAL_STATIC_LIBRARIES) -Wl,--no-whole-archive

.PHONY: clean 

clean:
	$(RM) $(CPPOBJS) $(COBJS)  $(LOCAL_MODULE)

ifneq "$(MAKECMDGOALS)" "clean"
  -include $(CDEPS)
  -include $(CPPDEPS)
endif

%.d: %.c
	@$(CC) $(CFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -M $< | \
	$(SED) 's,.*:,$*\.o $@ : ,g' > $@.tmp
	@$(MV) $@.tmp $@

%.d: %.cpp
	@$(CXX) $(CFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -M $< | \
	$(SED) 's,.*:,$*\.o $@ : ,g' > $@.tmp
	@$(MV) $@.tmp $@
