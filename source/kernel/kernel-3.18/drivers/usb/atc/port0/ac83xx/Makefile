#
# for USB OTG silicon based on Mentor Graphics INVENTRA designs
#
ccflags-$(CONFIG_USB_ATC_HDRC) += -I$(srctree)/drivers/usb/atc/port0

#QMU Flags
ccflags-$(CONFIG_ATC_MUSB_QMU_SUPPORT) += -DMUSB_QMU_SUPPORT
##ccflags-$(CONFIG_ATC_MUSB_QMU_SUPPORT) += -DQMU_TASKLET

obj-$(CONFIG_USB_ATC_HDRC)	:= usb20.o usb20_phy.o
obj-$(CONFIG_USB_ATC_HDRC)  += usb20_host.o
obj-$(CONFIG_USBIF_COMPLIANCE)   += usb20_otg_if.o
obj-$(CONFIG_USB_ATC_HDRC)	+= usb20_phy_debugfs.o
