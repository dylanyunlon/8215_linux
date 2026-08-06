#
#    compile var
#
STATIC_FLAG    :=
DEBUG_FLAG     :=
CROSS_COMPILE  :=


ifeq "$(TARGET_TOP)" ""
  TARGET_TOP := $(CURDIR)
endif

ifeq "$(BUILD_DIR)" ""
  BUILD_DIR := $(TARGET_TOP)
endif

OBJTREE := $(if $(BUILD_DIR),$(BUILD_DIR),$(CURDIR))
$(warning $(TARGET_TOP))
$(warning $(OBJTREE))
ifeq ($(CC), gcc)
$(error $(CC) is not the expected cross compiler )
endif


SRCTREE := $(CURDIR)

ifeq "$(LOCAL_FORCE_STATIC_EXECUTABLE)" "true"
  STATIC_FLAG := --static
endif

CROSS_COMPILE :=
TARGET32 := arm
TARGET64 := aarch64

ifneq (, $(filter $(TARGET), $(TARGET32) $(TARGET64)))
  ifeq "$(TARGET)" "$(TARGET32)"
   CROSS_COMPILE = arm-pokymllib32-linux-gnueabi-
   $(warning CROSS_COMPILE is $(CROSS_COMPILE))
  else
   CROSS_COMPILE = aarch64-poky-linux-
   $(warning CROSS_COMPILE is $(CROSS_COMPILE))
  endif
endif

CC  := $(CC)
CXX := $(CXX)
LD  := $(LD)
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

ifeq "$(TARGET_BUILD_PROJ)" "ivi"
CFLAGS      += $(addprefix -D,$(ATC_PROJECT_IVI))
CPPFLAGS    += $(addprefix -D,$(ATC_PROJECT_IVI))
else ifeq "$(TARGET_BUILD_PROJ)" "4p1"
CFLAGS      += $(addprefix -D,$(ATC_PROJECT_4P1))
CPPFLAGS    += $(addprefix -D,$(ATC_PROJECT_4P1))
endif

OBJS_FROM_C := $(addprefix $(OBJTREE)/,$(COBJS))
SRCS_FROM_C := $(addprefix $(SRCTREE)/,$(CSRCS))
$(warning $(OBJS_FROM_C))

OBJS_FROM_C_PLUS := $(addprefix $(OBJTREE)/,$(CPPOBJS))
SRCS_FROM_C_PLUS := $(addprefix $(SRCTREE)/,$(CPPSRCS))
$(warning $(OBJS_FROM_C_PLUS))

OBJS_DEPS_C := $(addprefix $(OBJTREE)/,$(CDEPS))
$(warning $(OBJS_DEPS_C))

OBJS_DEPS_CPLUS := $(addprefix $(OBJTREE)/,$(CPPDEPS))
$(warning $(OBJS_DEPS_CPLUS))

define COMPILE_C
	$(CC) $(CFLAGS) $(CPPFLAGS)  -c -o $@ $<
endef

define COMPILE_ASM
	$(AS) $(AFLAGS) -c $@ $<
endef

define COMPILE_C_PLUS
	$(CXX) $(CFLAGS) $(CPPFLAGS)  -c -o $@ $<
endef


$(OBJTREE)/%.o : $(SRCTREE)/%.c
	@mkdir -p $(dir $@)
	$(COMPILE_C)

$(OBJTREE)/%.o : $(SRCTREE)/%.cpp
	@mkdir -p $(dir $@)
	$(COMPILE_C_PLUS)

$(OBJTREE)/%.d: $(SRCTREE)/%.c
	set -e;  mkdir -p $(dir $@); \
	$(CC) $(CFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -MM $< > $@.$$$$; \
	$(SED) 's,.*:,$(OBJTREE)/$*\.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

$(OBJTREE)/%.d: $(SRCTREE)/%.cpp
	set -e;  mkdir -p $(dir $@); \
	$(CXX) $(CFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -MM $< > $@.$$$$; \
	$(SED) 's,.*:,$(OBJTREE)/$*\.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$
