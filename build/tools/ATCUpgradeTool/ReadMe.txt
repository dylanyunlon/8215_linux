1. （2013-12-03）：
	A. 增加烧写直接从SD Bootup的功能；
	B. Fix SD烧写以后Release SD设备的Issue；
	C. 增加从Scatter File中读取desc和boardtype的功能；

2. （2013-12-28） Version 1.2.0.0：
	A. 为烧写SD升级卡和SD Bootup卡增加烧写完成以后弹出SD卡功能；
	B. 增加SD升级卡的分段烧写功能。（避免在板子上分配大内存而导致烧写失败）
	C. 增加通过USB分段传输Image文件，分段烧写Nand。（避免在板子上分配大内存而导致烧写失败）
	D. 增加通过USB升级板子上SD/eMMC功能；
	E. 增加配置文件config.ini，使常用设置可以配置，例如：将‘Format Flash’设置为default选中（Zeng需求），烧写以后弹出SD卡和USB烧写以后重启板子都可以配置是否为默认选中或不选中。

3. （2014-01-17）Version 1.2.1.0：
	A. 解决 弹出SD卡和重启板子的CheckBox混乱的问题;
	B. 为烧写MMC Bootup的烧写增加Format userdata分区功能，当前的Userdata分区大小和开始位置由config.ini文件设置；（分区大小为0表示剩余所有容量）

4. （2014-02-14） Version 1.2.2.0：
	A. 修复Release SD不彻底的问题；（如果不选择Eject SD Card，在Release之前Close Volume句柄）
	B. 解决可能误选“Format Flash”而非烧写所有分区导致系统无法启动的问题；
	C. 解决使用RW的方式打开Server端的文件，因为权限或者ADO的问题导致Download失败；（改为Read Only方式打开）

5. （2014-02-28） Version 1.2.3.0：
	A. 增加USB Download下，格式化userdata分区的功能；
	B. 适应增加的data4write分区，修改Nand升级使用的u-boot.bin文件；
	C. 为了适应增加分区的需要修改config.ini文件中的Userdata分区的起始地址；
	D. 添加防护Code，防止可能出现的错误读写PC硬盘导致系统崩溃；
	
.......

   2015-11-17  v1.6.0.5 
   rename AndroidUpdateTool to ATCUpdateTool
   
   2015-11-20  v1.6.0.6
   fix an issue that when updating all image by fastboot method may cause system booting fail
   rename tool name from "ATC Update Tool" to "ATC Upgrade Tool"
   	
   2015-12-03  v1.6.1.0
   add win7 support(flash new SD card in win7 or above system)	
   
   2015-12-04  v1.6.1.1
   add some compatible code to give some tips when selecting wrong tab to flash sd card  
   
   2015-12-17  v1.6.1.2
   add load default partition table xml file support  
   (in config.ini   "LoadDefaultXML=1" and "DefaultXMLFileName=scatter.mmcboot.ext4.xml")
   add some tips on the flashing settings
   add some code to check the dependencies betweent the flashing settings
   set the default wifi/bt/gps chip to mt6630
   
   2016-01-07
   change the upgrade flow fro linux reserve memory adjust   ,add dtb and get memory load address from it when do upgrade 
   this tool is not compatiable with old image and old tool is not compatiable with this new tool too.
   
   2016.01.18  version 1.6.1.3  
   modify wifi/bt/gps chip match method
   set "Format "userdata" Partition as default"
   use VERSION MACRO to instead hardcode
   
   2016.04.24  version 1.6.1.4
	merge dtb version with linux main 1.6.1.3 version    
  
   2016.06.04  versuib 1.6.1.5
    add support for config for load default tab(emmc upgrade/nand upgrade)
    
   2017.02.20  versuib 1.7.0.1
    add support for write protect size(emmc upgrade/nand upgrade)
    
   2017.02.23  versuib 1.7.0.2
    add support for parse fastboot flag(emmc upgrade/nand upgrade)
    
     





