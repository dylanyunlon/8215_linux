/*
 *  drivers/mtd/nandids.c
 *
 *  Copyright (C) 2002 Thomas Gleixner (tglx@linutronix.de)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#include <common.h>
#include <linux/mtd/nand.h>
/*
*	Chip ID list
*
*	Name. ID code, pagesize, chipsize in MegaByte, eraseblock size,
*	options
*
*	Pagesize; 0, 256, 512
*	0	get this information from the extended chip ID
+	256	256 Byte page size
*	512	512 Byte page size
*/
struct nand_flash_dev nand_flash_ids[] = {
	/*
	 * These are the new chips with large page size. The pagesize and the
	 * erasesize is determined from the extended id bytes
	 */
#define LP_OPTIONS (NAND_SAMSUNG_LP_OPTIONS | NAND_NO_READRDY | NAND_NO_AUTOINCR)
#define LP_OPTIONS16 (LP_OPTIONS | NAND_BUSWIDTH_16)
	/* Name, id[], page size, chip size(MB), block size, options, id length, oob size, timing setting */
/* Hynix */
	{"H27U8G8T2B 1GiB 3,3V 8-bit",
		{.id = {0xAD, 0xD3, 0x14, 0xB6}},4096, 1024, 0x80000, LP_OPTIONS, 4, 128,0x00000135},
	{"H27UBG8T2A 4GiB 3,3V 8-bit",
		{.id = {0xAD, 0xD7, 0x94, 0x9A}},8192, 4096, 0x200000, LP_OPTIONS, 4, 448, 0x00000135},
	{"H27UBG8T2B 4GiB 3,3V 8-bit",
		{.id = {0xAD, 0xD7, 0x94, 0xDA}},8192, 4096, 0x200000, LP_OPTIONS, 4, 640, 0x00000135},
/* Samsung */
	{"K9GBG08U0A 4GiB 3,3V 8-bit",
		{.id = {0xEC, 0xD7, 0x94, 0x7A}},8192, 4096, 0x100000, LP_OPTIONS, 4, 640, 0x00000135},
	{"K9F4G08U0E 4GiB 3,3V 8-bit",
		{.id = {0xEC, 0xDC, 0x10, 0x95, 0x55}},2048, 512, 0x20000, LP_OPTIONS, 5, 64, 0x00000123},
/* Micron */
	//{"MT29F32G08CBACA 4GiB 3,3V 8-bit",
	//	{.id = {0x2C, 0x68, 0x04, 0x4A}},4096, 4096, 0x100000, LP_OPTIONS, 4, 224, 0x00000135},
	//{"MT29F32G08ACABA 32GiB 3,3V 8-bit",
	//	{.id = {0x2C, 0x38, 0x00, 0x26}},4096, 256, 0x40000, LP_OPTIONS, 4, 224, 0x00000135},
	//{"MT29F32G08ACABA 32GiB 3,3V 8-bit",
		//{.id = {0x2C, 0x38, 0x00, 0x26}},4096, 1024, 0x40000, LP_OPTIONS, 4, 224, 0x00000135},
	{"MT29F8G08ABACA 8Gb 3,3V 8-bit",
		{.id = {0x2C, 0xD3, 0x90, 0xA6}},4096, 1024, 0x40000, LP_OPTIONS, 4, 224, 0x00000123},
	{"MT29F8G08ABABA 8Gb 3,3V 8-bit",
		{.id = {0x2C, 0x38, 0x00, 0x26}},4096, 1024, 0x80000, LP_OPTIONS, 4, 224, 0x00000123},
	{"MT29F4G08ABADA 4Gb 3,3V 8-bit",
		{.id = {0x2C, 0xDC, 0x90, 0x95}},2048, 512, 0x20000, LP_OPTIONS, 4, 64, 0x00000135},
	{"MT29F4G08ABAEA 4Gb 3,3V 8-bit",
		{.id = {0x2C, 0xDC, 0x90, 0xA6}},4096, 512, 0x40000, LP_OPTIONS, 4, 224, 0x00000123},
	{"MT29F2G08ABAEA 2Gb 3,3V 8-bit",
		{.id = {0x2C, 0xDA, 0x90, 0x95}},2048, 256, 0x20000, LP_OPTIONS, 4, 64, 0x00000123},
	{"MT29F8G08ABACA 8Gb 3,3V 8-bit",
		{.id = {0x2C, 0xDA, 0x90, 0x95}},4096, 1024, 0x40000, LP_OPTIONS, 4, 224, 0x00000123},
	{"MT29F4G08ABBDA 4Gb 3,3V 8-bit",
		{.id = {0x2C, 0xAC, 0x90, 0x15}},2048, 512, 0x20000, LP_OPTIONS, 4, 64, 0x00000123},
	{"MT29F8G08ADBDA 8Gb 3,3V 8-bit",
		{.id = {0x2C, 0xA3, 0xD1, 0x15}},2048, 1024, 0x20000, LP_OPTIONS, 4, 64, 0x00000123},
	{"MT29F4G08ABAFA 4Gb 3,3V 8-bit",
		{.id = {0x2C, 0xDC, 0x80, 0xA6}},4096, 512, 0x40000, LP_OPTIONS, 4, 128, 0x00000135},
/* MXIC */
	{"MX60LF8G28AB 8GiB 3,3V 8-bit",
		{.id = {0xC2, 0xD3, 0xD1, 0x95, 0x5B}},2048, 1024, 0x20000, LP_OPTIONS, 5, 112, 0x00000123},
	{"MX30LF4G18AC 4GiB 3,3V 8-bit",
		{.id = {0xC2, 0xDC, 0x90, 0x95, 0x56}},2048, 512, 0x20000, LP_OPTIONS, 5, 64, 0x00000123},
	{"MX30LF4G28AB 4GiB 3,3V 8-bit",
		{.id = {0xC2, 0xDC, 0x90, 0x95, 0x57}},2048, 512, 0x20000, LP_OPTIONS, 5, 112, 0x00000123},
	{"MX30LF2G18AC 2GiB 3,3V 8-bit",
		{.id = {0xC2, 0xDA, 0x90, 0x95, 0x06}},2048, 256, 0x20000, LP_OPTIONS, 5, 64, 0x00000123},
	{"MX30LF2G28AB 2GiB 3,3V 8-bit",
		{.id = {0xC2, 0xDA, 0x90, 0x95, 0x07}},2048, 256, 0x20000, LP_OPTIONS, 5, 112, 0x00000123},
	{"MX30LF2GE8AB 2GiB 3,3V 8-bit",
		{.id = {0xC2, 0xDA, 0x90, 0x95, 0x86}},2048, 256, 0x20000, LP_OPTIONS, 5, 64, 0x00000123},
	{"MX30LF4G28AD 4Gb 3,3V 8-bit",
		{.id = {0xC2, 0xDC, 0x90, 0xA2, 0x57}},4096, 512, 0x40000, LP_OPTIONS, 5, 128, 0x00000123},
/* Toshiba */
	{"TC58NVG1S3HTA00 2GiB 3,3V 8-bit",
		{.id = {0x98, 0xDA, 0x90, 0x15, 0x76}},2048, 256, 0x20000, LP_OPTIONS, 5, 128, 0x00000123},
	{"TC58NVG2S0HTAI0 4GiB 3,3V 8-bit",
		{.id = {0x98, 0xDC, 0x90, 0x26, 0x76}},4096, 512, 0x40000, LP_OPTIONS, 5, 128, 0x00000135},
/*Winbond*/
	{"W29N04GVSAAA 4Gb 3,3V 8-bit",
			{.id = {0xEF, 0xDC, 0x90, 0x95}},2048, 512, 0x20000, LP_OPTIONS, 4, 64, 0x00000135},
/*CYRESS*/
	{"S34ML04G2  4Gb 3,3V 8-bit",
			{.id = {0x01, 0xDC, 0x90, 0x95}},2048, 512, 0x20000, LP_OPTIONS, 4, 64, 0x00000123},
/*GigaDevice**/
	{"GD9FU1G8F2D  1Gb 3,3V 8-bit",
			{.id = {0xC8, 0xF1, 0x80, 0x95}},2048, 128, 0x20000, LP_OPTIONS, 4, 64, 0x00000123},

	{"GD9FU2G8F2A  2Gb 3,3V 8-bit",
			{.id = {0xC8, 0xDA, 0x90, 0x95}},2048, 256, 0x20000, LP_OPTIONS, 4, 64, 0x00000123},

	{"GD9FU4G8F2A  4Gb 3,3V 8-bit",
			{.id = {0xC8, 0xDC, 0x90, 0x95}},2048, 512, 0x20000, LP_OPTIONS, 4, 64, 0x00000123},

	{"GD9FU4G8F4D  4Gb 3,3V 8-bit",
			{.id = {0xC8, 0xDC, 0x80, 0xA6}},4096, 512, 0x40000, LP_OPTIONS, 4, 128, 0x00000123},

	{"W29N02KVSIAF  2Gb 3,3V 8-bit",
			{.id = {0xEF, 0xDA, 0x10, 0x95}},2048, 256, 0x20000, LP_OPTIONS, 4, 64, 0x00000123},
	{"W29N02GVSAAF  2Gb 3,3V 8-bit",
			{.id = {0xEF, 0xDA, 0x90, 0x95}},2048, 256, 0x20000, LP_OPTIONS, 4, 64, 0x00000123},

	{"XT27G02BTSIGA  2Gb 3,3V 8-bit",
			{.id = {0x98, 0xDA, 0x90, 0x15, 0xF6}},2048, 256, 0x20000, LP_OPTIONS, 5, 64, 0x00000123},

	{"XT27G04ATSIGA  4Gb 3,3V 8-bit",
			{.id = {0x98, 0xDC, 0x90, 0x26, 0x76}},2048, 512, 0x20000, LP_OPTIONS, 5, 64, 0x00000123},
	{NULL,}
};

struct nand_flash_dev_ext nand_flash_ids_ext[] = {
         /* name,                         maf_id,  dev_id,    id3,     id4,   pagesize,   chipsize,   erasesize,    oobsize,   buswidth*/
        {"NAND_TC58NVG4D1DTG",  0x98,  0xD5,  0x94, 0xBA,   4096,   2048,   0x80000,   208,     0 },
    	{"NAND_H27UAG8T2ATR" ,  0xAD,  0xD5,  0x94, 0x25,   4096,   2048,   0x80000,   224,     0 },
	{"NAND_TC58NVG0S3ETA00", 0x98,  0xD1,	0x90, 0x15,  2048,	 128,	 0x20000,	64,   0 },
	{"NAND_MT29F8G08ABA",	 0x2C,	   0x38,  0x00, 0x26,  4096,   1024,   0x80000,    224, 	0 },
	{"NAND_MT29F16G08CBA",  0x2C,	 0x48,	0x04, 0x46,  4096,	 2048,	 0x100000,	 224,	  0 },
	{"NAND_MT29F8G08ABADA 8Gb 3,3V 8-bit", 0x2C,0xDC,0x90,0x15,2048, 64,1024, 0x20000, LP_OPTIONS},
	{NULL,}
};

/*
*	Manufacturer ID list
*/
struct nand_manufacturers nand_manuf_ids[] = {
	{NAND_MFR_TOSHIBA, "Toshiba"},
	{NAND_MFR_SAMSUNG, "Samsung"},
	{NAND_MFR_FUJITSU, "Fujitsu"},
	{NAND_MFR_NATIONAL, "National"},
	{NAND_MFR_RENESAS, "Renesas"},
	{NAND_MFR_STMICRO, "ST Micro"},
	{NAND_MFR_HYNIX, "Hynix"},
	{NAND_MFR_MICRON, "Micron"},
	{NAND_MFR_AMD, "AMD"},
	{NAND_MFR_MXIC, "MXIC"},
	{NAND_MFR_WINBOND, "WINBOND"},
	{NAND_MFR_GIGA, "Gigadevice"},
	{0x0, "Unknown"}
};
