#
# for USB OTG silicon based on Mentor Graphics INVENTRA designs
#

#QMU Flags
ccflags-$(CONFIG_ATC_MUSBH_QMU_SUPPORT) += -DMUSB_QMU_SUPPORT
##ccflags-$(CONFIG_ATC_MUSBH_QMU_SUPPORT) += -DQMU_TASKLET

#Platform
#obj-$(CONFIG_ARCH_MT8163) += $(subst ",,$(CONFIG_ATC_PLATFORM))/
obj-y += $(subst ",,$(CONFIG_ATC_PLATFORM))/

#Driver
obj-$(CONFIG_USB_ATC_HCD) += musbh_hdrc.o
musbh_hdrc-$(CONFIG_USB_ATC_HCD) := musb_core.o
musbh_hdrc-$(CONFIG_USB_ATC_HCD) += musb_virthub.o musb_host.o
musbh_hdrc-$(CONFIG_USB_ATC_HCD) += musbhsdma.o
musbh_hdrc-$(CONFIG_USB_ATC_HCD) += musb_debugfs.o

# QMU Realted
obj-$(CONFIG_ATC_MUSB_QMU_SUPPORT) += atc_qmu.o musb_qmu.o
