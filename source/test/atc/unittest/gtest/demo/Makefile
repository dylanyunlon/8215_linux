#!/bin/sh

PWD:=$(shell pwd)

CROSS_COMPILE := arm-buildroot-linux-gnueabi-

CC := $(CROSS_COMPILE)gcc
CXX := $(CROSS_COMPILE)g++
LD :=  $(CROSS_COMPILE)ld
STRIP := $(CROSS_COMPILE)strip

TARGET = demo_unittest

all : $(TARGET)

clean :
	rm -f $(TARGET) *.o *.a

.PHONY : clean

FILES_DIR = .

CXXFLAGS += -I$(GTEST_DIR)/include
CXXFLAGS += -L$(GTEST_DIR)/make

FILES = $(foreach d, $(FILES_DIR), $(wildcard $(d)/*.cpp))
OBJS = $(patsubst %.cpp, %.o, $(FILES))

%.o : %.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $<

# mmisc.a : $(OBJS)
# 	$(AR) $(ARFLAGS) -o $@ $^

$(TARGET) : $(OBJS)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -lgtest -lpthread $^ -o $@
	mv -f $(UNITTEST_DIR)/atc/unittest/gtest/demo/$(TARGET) $(UNITTEST_DIR)/atc/unittest/bin/$(TARGET)
