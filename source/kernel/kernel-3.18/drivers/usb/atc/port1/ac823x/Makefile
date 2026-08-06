#
# for USB OTG silicon based on Mentor Graphics INVENTRA designs
#
ccflags-$(CONFIG_USB_ATC_HCD) += -I$(srctree)/drivers/usb/atc/port1

#QMU Flags
ccflags-$(CONFIG_ATC_MUSBH_QMU_SUPPORT) += -DMUSB_QMU_SUPPORT
##ccflags-$(CONFIG_ATC_MUSBH_QMU_SUPPORT) += -DQMU_TASKLET

obj-$(CONFIG_USB_ATC_HCD)	:= usb20.o usb20_phy.o
obj-$(CONFIG_USB_ATC_HCD)  += usb20_host.o
obj-$(CONFIG_USBIF_COMPLIANCE)   += usb20_otg_if.o
obj-$(CONFIG_USB_ATC_HCD)	+= usb20_phy_debugfs.o
