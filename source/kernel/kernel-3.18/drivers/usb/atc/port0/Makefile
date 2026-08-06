#
# for USB OTG silicon based on Mentor Graphics INVENTRA designs
#

ifeq ($(ATC_ALPS_BOX_SUPPORT),yes)
ccflags-y += -DATC_ALPS_BOX_SUPPORT
endif

#QMU Flags
ccflags-$(CONFIG_ATC_MUSB_QMU_SUPPORT) += -DMUSB_QMU_SUPPORT
##ccflags-$(CONFIG_ATC_MUSB_QMU_SUPPORT) += -DQMU_TASKLET

#Platform
#obj-$(CONFIG_ARCH_MT8163) += $(subst ",,$(CONFIG_ATC_PLATFORM))/
obj-y += $(subst ",,$(CONFIG_ATC_PLATFORM))/

#Driver
obj-$(CONFIG_USB_ATC_HDRC) += musb_hdrc.o
musb_hdrc-$(CONFIG_USB_ATC_HDRC) := musb_core.o
musb_hdrc-$(CONFIG_USB_ATC_HDRC) += musb_gadget_ep0.o musb_gadget.o
musb_hdrc-$(CONFIG_USB_ATC_HDRC) += musb_virthub.o musb_host.o
musb_hdrc-$(CONFIG_USB_ATC_HDRC) += musbhsdma.o
musb_hdrc-$(CONFIG_USB_ATC_HDRC) += musb_debugfs.o

# QMU Realted
obj-$(CONFIG_ATC_MUSB_QMU_SUPPORT) += atc_qmu.o musb_qmu.o
