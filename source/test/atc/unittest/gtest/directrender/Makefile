#!/bin/sh

PWD:=$(shell pwd)
CUR_DIR := $(shell pwd)
export KERNEL_TOPDIR

CROSS_COMPILE := arm-buildroot-linux-gnueabi-

CC := $(CROSS_COMPILE)gcc
CXX := $(CROSS_COMPILE)g++
LD :=  $(CROSS_COMPILE)ld
STRIP := $(CROSS_COMPILE)strip

#LOCAL_SRC_FILES := \
               #$(CUR_DIR)/test/async_queue.c 
LOCAL_INCLUDES :=  \
  $(CUR_DIR)/../../../../../directrender-1.0/include \
  $(CUR_DIR)/../../../../../directrender-1.0/src/utils \
  $(CUR_DIR)/../../../../../atcomx-1.0/openmax/ \
  $(KERNEL_TOPDIR)/include/media/atc/ \
  $(KERNEL_TOPDIR)/drivers/ \
  $(KERNEL_TOPDIR)/drivers/misc/atc/inc/
  
#LOCAL_SRC_FILES += $(CUR_DIR)/src/directrender_unittest.cpp
#LOCAL_C_INCLUDES :=$(CUR_DIR)/test/

LOCAL_SHARE_LIBRARIES := -lc -ldl
LOCAL_SHARE_LIBRARIES += -lsurface_atc -lpthread -ldirectrender

#LOCAL_SHARED_LIBRARIES := \
    #libsurface_atc \
    #libdirectrender \
    #libpthread

TARGET = directrender_unittest

all : $(TARGET)

clean :
	rm -f $(TARGET) *.o *.a

.PHONY : clean

LOCAL_OBJS := $(LOCAL_SRC_FILES:.c=.o)
INCLUDE_FLAGS      += $(addprefix -I,$(LOCAL_INCLUDES))
vpath %.h $(LOCAL_INCLUDES)

FILES_DIR = .

CXXFLAGS += -I$(GTEST_DIR)/include
CXXFLAGS += -L$(GTEST_DIR)/make

FILES = $(foreach d, $(FILES_DIR), $(wildcard $(d)/*.cpp))
OBJS = $(patsubst %.cpp, %.o, $(FILES))

%.o : %.cpp
	$(CXX) $(INCLUDE_FLAGS) $(CPPFLAGS) $(CXXFLAGS) -c $<

# mmisc.a : $(OBJS)
# 	$(AR) $(ARFLAGS) -o $@ $^

$(TARGET) : $(OBJS)
	$(CXX) $(INCLUDE_FLAGS) $(CPPFLAGS) $(CXXFLAGS) $(LOCAL_SHARE_LIBRARIES) -lgtest -lpthread $^ -o $@
	mv -f $(UNITTEST_DIR)/atc/unittest/gtest/directrender/$(TARGET) $(UNITTEST_DIR)/atc/unittest/bin/$(TARGET)
