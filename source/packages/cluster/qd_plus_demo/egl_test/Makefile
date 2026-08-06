CXX=arm-buildroot-linux-gnueabi-g++
TARGET := arm

#CC=arm-buildroot-linux-gnueabi-gcc
#CXX=aarch64-buildroot-linux-gnu-g++
#CXX=~/Git/8317_linux/sources/build/tmp/sysroots/ac83xxhf/usr/bin/crossscripts/arm-poky-linux-gnueabi-libtool

TOPDIR := $(shell pwd)
$(info TOPDIR is $(TOPDIR))
$(info TARGET_DIR is $(TARGET_DIR))
INCLUDE=-I. \
       -I$(TARGET_DIR)/usr/include \
       -I$(TOPDIR)/../../../../source/packages/multimedia/vba/include \
       -I$(TOPDIR)/../../../../source/kernel/kernel-3.18/drivers/misc/atc/bootanimation/ \
       -I$(TOPDIR)/../../../../prebuild/opengles/opengles-r6p1/3rdparty/include/khronos/ \
       -I$(TOPDIR)/../../../../prebuild/opengles/opengles-r6p1/include/EGL/platform_fbdev/ \
       -I$(TOPDIR)/../../../../prebuild/opengles/opengles-r6p1/include/ \


LDFLAGS= -g
CXXFLAGS := `pkg-config --cflags --libs`
CXXFLAGS += $(INCLUDE)
ifeq ($(ATC_SHOW_VBA), false)
$(info ATC_SHOW_VBA is false,adding -DATC_SHOW_LOGO_BOOT)
GLOBAL_CFLAG += -DATC_SHOW_LOGO_BOOT
CXXFLAGS += $(GLOBAL_CFLAG)
endif
#CXXFLAGS+=-lgbm -ldrm -lEGL -lGLESv2 -lusc -lsrv_um -lIMGegl -lglslcompiler
CXXFLAGS+=-lEGL -lGLESv2 -lMali -latcbootanicom -lsettings_atc
CXXFLAGS+=-DEGL_EGLEXT_PROTOTYPES -DGL_GLEXT_PROTOTYPES -DRENDER_TO_SCREEN -fpermissive


TARGET_LIB=QD_demo

SRC_FILES= ./fb-egl-sample.cpp \
           ./QD_HMI/hmi_action_engine.c \
           ./QD_HMI/hmi_engine.c \
           ./QD_HMI/hmi_loop.c \
           ./QD_HMI/hmi_loop_driver.c \
           ./QD_HMI/hmi_reset.c \
           ./QD_HMI/hmi_status.c \
           ./QD_HMI/hmi_task_touchpanel.c \
           ./QD_HMI/hmi_opengles2_driver.c \
           ./QD_HMI/hmi_task_touchpanel_driver.c \
           ./QD_HMI/lv_bidi.c \
           ./QD_HMI/lv_txt.c \
           ./QD_HMI/hmi_utils.c \
           ./QD_HMI/hmi_user_interface.c 



all: $(TARGET_LIB)
$(TARGET_LIB): $(SRC_FILES) 
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $(SRC_FILES) 


clean:
	rm -f $(TARGET_LIB) *.o 


