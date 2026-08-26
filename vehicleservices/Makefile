# =====================================================================
#  libvehicle_services.a -- HCN 车机服务层静态库
#
#  位置    : source/vendor/hcn/vehicle_services/Makefile
#  依赖    : 中间件公共头 <mw/vehicle_param.h>（由 lib_mw 包装到 staging）
#  工具链  : SDK 自带 buildroot 工具链（arm-buildroot-linux-gnueabi-）
#  纯 C，仅 CC/AR；.a 只归档不链接，未解析符号(mw)在最终链入 application 时解析
#  独立编译: make                     （需先 build 出 out/host 与 lib_mw staging）
#  包内编译: 由 hcn-application.mk 调用，传入 CC/AR/MW_INC
# =====================================================================

HOST_DIR ?= $(CURDIR)/../../../../out/host
CROSS    ?= arm-buildroot-linux-gnueabi-
CC        = $(HOST_DIR)/usr/bin/$(CROSS)gcc
AR        = $(HOST_DIR)/usr/bin/$(CROSS)ar

# 中间件公共头路径（vehicle_param.h 装在 staging/usr/include/mw/）
MW_INC   ?= $(HOST_DIR)/arm-buildroot-linux-gnueabi/sysroot/usr/include

CFLAGS  += -fPIC -std=gnu11 -Wall -O2 -I$(MW_INC)

SRCS    := vehicle_services.c
OBJDIR  := build
OBJS    := $(patsubst %.c,$(OBJDIR)/%.o,$(SRCS))
DEPS    := $(OBJS:.o=.d)

.PHONY: all clean

all: libvehicle_services.a

libvehicle_services.a: $(OBJS)
	$(AR) rcs $@ $^

$(OBJDIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

-include $(DEPS)

clean:
	rm -rf $(OBJDIR) libvehicle_services.a
