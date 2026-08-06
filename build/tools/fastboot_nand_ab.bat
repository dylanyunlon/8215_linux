@ECHO OFF

fastboot.exe flash preloader 83XX_Preloader_realchip_nand.bin

fastboot.exe flash preloader_bk 83XX_Preloader_realchip_nand.bin

fastboot.exe flash uboot_a u-boot.bin

fastboot.exe flash uboot_b u-boot.bin

fastboot.exe flash trustzone_a tz.bin

fastboot.exe flash trustzone_b tz.bin

fastboot.exe flash arm2_a arm2.bin

fastboot.exe flash arm2_b arm2.bin

fastboot.exe flash dtb_a ac83xx.dtb.bin

fastboot.exe flash dtb_b ac83xx.dtb.bin

fastboot.exe flash boot_misc boot_misc.bin

fastboot.exe flash vba_a vba.bin

fastboot.exe flash vba_b vba.bin

fastboot.exe flash logo_a cluster_res.img

fastboot.exe flash logo_b cluster_res.img

fastboot.exe flash metazone metazone.bin

fastboot.exe flash kernel_a Image.bin

fastboot.exe flash kernel_b Image.bin

fastboot.exe flash system_a system.img.ext4

fastboot.exe flash system_b system.img.ext4

fastboot.exe flash usrdata data.img.ext4

pause

pause
