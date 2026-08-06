#
#
# SPDX-License-Identifier:	GPL-2.0+
#

#obj-$(CONFIG_FS_FAT)	:= fat.o
#obj-$(CONFIG_FAT_WRITE):= fat_write.o

#ifndef CONFIG_SPL_BUILD
#obj-$(CONFIG_FS_FAT)	+= file.o
#endif


include $(TOPDIR)/config.mk

LIB	= $(obj)libfat.a

AOBJS	=
COBJS-$(CONFIG_CMD_FAT)	:= fat.o file.o

SRCS	:= $(AOBJS:.o=.S) $(COBJS-y:.o=.c)
OBJS	:= $(addprefix $(obj),$(AOBJS) $(COBJS-y))

all:	$(LIB) $(AOBJS)

$(LIB):	$(obj).depend $(OBJS)
	$(AR) $(ARFLAGS) $@ $(OBJS)


#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################

