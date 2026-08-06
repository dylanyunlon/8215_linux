#
# Makefile for Autochips pwm
# 
#
EXTRA_CFLAGS += -I$(src)/../../../arch/arm/mach-ac83xx/include/mach
EXTRA_CFLAGS += -I$(src)/../../misc/atc/inc

obj-$(CONFIG_PWM_ATC) := pwm.o
pwm-y    := pwm_oal.o hal/pwm_hal.o
pwm-objs := $(pwm-y)	
