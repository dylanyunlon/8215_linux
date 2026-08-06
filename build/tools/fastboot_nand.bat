@ECHO OFF

fastboot.exe flash preloader 83XX_Preloader_realchip_nand.bin

fastboot.exe flash preloader_bk 83XX_Preloader_realchip_nand.bin

fastboot.exe flash uboot u-boot.bin

fastboot.exe flash trustzone tz.bin

fastboot.exe flash arm2 arm2.bin

fastboot.exe flash kernel Image.bin

fastboot.exe flash dtb ac83xx.dtb.bin

fastboot.exe flash system system.img.ext4

fastboot.exe flash usrdata data.img.ext4

fastboot.exe flash logo logo.mrf

fastboot.exe flash metazone metazone.bin

pause

pause
