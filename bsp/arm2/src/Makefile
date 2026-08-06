#
#
#   Compile ARCH Code
#   Wrote by Ke Xu@atc0036

include $(ARM2TOPDIR)/build/config.mk

CUR_CSRC := eabi_compat.c  \
            aeabi_uidiv.c  \
            bim_if.c       \
            dual_callback.c \
            dual_hal.c     \
            dual_task.c    \
            init.c         \
            interrupts.c   \
            pagetbl.c      \
            printf.c       \
            BootAnimation.c \
            arm2system_service.c       \
            rtc.c       \
            metazone_inter.c \
            ac83xx_uart.c \
            fault.c \
            addr2func.c \
            test_exception.c \
            plat.c \
            libarm2c.c \
            armv6_mmu.c \
            arm_unwind.c \
            uart_test_task.c

# CUR_CSRC += $(ARM2TOPDIR)/metazone/metazone_inter.c
CUR_CSRC += serial.c

ifeq "$(CONFIG_PLATFORM)" "AC823X"
	CUR_CSRC += ac823x_gpio.c
        CUR_CSRC += ac823x_uart.c
        CUR_CSRC += func_stub.c
endif

SSRC     = vectors.S int_83xx.S memset.S memcpy.S exception.S

COBJ     = $(subst .c,.o,$(CUR_CSRC))
SOBJ	 = $(subst .S,.o,$(SSRC))

ALLOBJS := $(COBJ) \
           $(SOBJ)

LOCAL_INCULE := -I$(ARM2TOPDIR)/inc/ \
                -I$(ATC_KERNEL_DIR)/arch/arm/mach-ac83xx/include/ \
                -I$(ATC_KERNEL_DIR)/arch/arm/mach-ac83xx/include/mach/ \
                -I$(ATC_KERNEL_DIR)/drivers/misc/atc/inc/ \
                -I$(ATC_KERNEL_DIR)/drivers/misc/atc/ \
                -I$(ATC_KERNEL_DIR)/include/ \
                -I$(UBOOT_SRC) \
                -DCHIP_VER_AC83XX \
                -I$(KERNEL_OBJTREE)/ \
                -I$(KERNEL_OBJTREE)/include/

# Note: For search metazone.h
# LOCAL_INCULE := -I$(ATC_KERNEL_DIR)/include/misc/atc/metazone

LDSCRIPT := $(ARM2BUILDDIR)/arm2_dram.lds

TARGETOBJ    := $(ARM2OBJDIR)/arm2_src.o
TARGETOBJDIR := $(ARM2OBJDIR)/src/obj

include $(ARM2BUILDDIR)/module_common.mk
