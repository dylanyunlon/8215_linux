#! /bin/sh

SYS_USB_CONFIG=sys.usb.config
CURRENT_USB_MODE=persist.current.usb.mode

function main() {
    echo "$USB_MODE"
    if [ ! -z "$(lsmod | grep "usb_hcd_qmu")" ];then
        echo "Switch USB adb Mode"
	setprop $CURRENT_USB_MODE device
	setprop $SYS_USB_CONFIG none
	rmmod usb_hcd_qmu
	insmod ./system/drivers/musb_hdrc.ko
	insmod ./system/drivers/g_android.ko
	setprop $SYS_USB_CONFIG adb,mass_storage
    elif [ ! -z "$(lsmod | grep "musb_hdrc")" ];then
    	echo "Switch USB Host Mode"
	setprop $SYS_USB_CONFIG none
	sleep 1s
	rmmod musb_hdrc
	rmmod g_android
	insmod ./system/drivers/usb_hcd_qmu.ko usb_protocol=usb2.0
        setprop $CURRENT_USB_MODE host
    else
	echo "Not Mass_storage, Not Adb, Please Check ... "
    fi
}

main $@
    
