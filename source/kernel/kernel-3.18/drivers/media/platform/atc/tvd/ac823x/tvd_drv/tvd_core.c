/*
* Copyright (c) 2016 AutoChips Inc.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License version 2 as
* published by the Free Software Foundation.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
*/

//#include "x_hal_ic.h"
#include "tvd_core.h"
#include "tvd_hw_reg.h"
#include "tvd_log.h"
#include "tdc.h"
#ifdef __ARM2__
#include "x_ckgen_8317.h"
#else
#include <linux/clk.h>
#include <linux/printk.h>
#endif

static bool gfgAnaIOPwrOn;
static bool gfgCHAPwrOn;
static bool gfgCHBPwrOn;
bool gfgNeedTurnClamp;

#if defined(__ARM2__)
unsigned long  tvd_base[4] = {0x5b000,0x5c000,0x5d000,0x5f000};
#else
extern unsigned long  tvd_base[4];
#endif
extern bool tvd3DComb;

/**************************************************************************

* Global Variable

**************************************************************************/


/**************************************************************************

* Local/Static Functions

**************************************************************************/

/**************************************************************************

* @brief  TVD Comb Setting.

*     The setting is experience value.

* @param

* @return None

**************************************************************************/
enum TVD_SIGNAL_MODE_E {
	TVD_PAL_N,
	TVD_PAL,
	TVD_PAL_M,
	TVD_NTSC,
	TVD_SECAM,
	TVD_PAL_60,
	TVD_UNSTABLE,  /* Represent video signal is not stable yet! In Hardware, it's reserved */
	TVD_NTSC443,
	TVD_NONE
};

void tvd_special_setting(TVD_CHANNEL_ID channel_id)
{
	/*********color process init***********/
	TVD_WRITE32(tvd_base[channel_id] + 0x7C4, 0xFF001718);
	TVD_WRITE32(tvd_base[channel_id] + 0x7C8, 0x2506FF0A);
	TVD_WRITE32(tvd_base[channel_id] + 0x7CC, 0x18F5FFEB);
	TVD_WRITE32(tvd_base[channel_id] + 0x7D0, 0x800FFF00);
	
	TVD_CLR_BIT(tvd_base[channel_id] + 0x7D0, 0x1<<31);

	
	TVD_WRITE32(tvd_base[channel_id] + 0x4e0, (TVD_READ32(tvd_base[channel_id] + 0x4e0)& (~(1<<31)))); //Dclamp _y_en must is close
	TVD_WRITE32(tvd_base[channel_id] + 0x4e4, (TVD_READ32(tvd_base[channel_id] + 0x4e4)& (~(0x1 << 15)))); //make the feedback current status is right
	TVD_WRITE32(tvd_base[channel_id] + 0x4e8, (TVD_READ32(tvd_base[channel_id] + 0x4e8)& (~(0x3FF << 10))) |(0x100 << 10)); //cha target blank level for clamping
	TVD_WRITE32(tvd_base[channel_id] + 0x5fc, (TVD_READ32(tvd_base[channel_id] + 0x5fc)& (~(0x1 << 10)))); //remove the fixed blank level set by BLKLVL
}

void NTSC_3D_setting(TVD_CHANNEL_ID channel_id)
{
	//form lumer plus setting       

	TVD_WRITE32(tvd_base[channel_id] + 0x410, 0x00818000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x414, 0xFF080020);  

	TVD_WRITE32(tvd_base[channel_id] + 0x434, 0xC4D00101);  
	TVD_WRITE32(tvd_base[channel_id] + 0x438, 0xB801404A);  
	TVD_WRITE32(tvd_base[channel_id] + 0x43C, 0x00800801);

	TVD_WRITE32(tvd_base[channel_id] + 0x484, 0x100F0480);  
	TVD_WRITE32(tvd_base[channel_id] + 0x488, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x48C, 0x0200022D);


	TVD_WRITE32(tvd_base[channel_id] + 0x4BC, 0x00000000);  
	//DFE00~~DFE1f
	TVD_WRITE32(tvd_base[channel_id] + 0x4C0, 0x51515C55);  
	TVD_WRITE32(tvd_base[channel_id] + 0x4C4, 0x04557845);  
	TVD_WRITE32(tvd_base[channel_id] + 0x4C8, 0x27F7595E);  
	TVD_WRITE32(tvd_base[channel_id] + 0x4CC, 0x8F340380);  
	TVD_WRITE32(tvd_base[channel_id] + 0x4D0, 0x158C0F80);  
	TVD_WRITE32(tvd_base[channel_id] + 0x4D4, 0x26144040);  
	TVD_WRITE32(tvd_base[channel_id] + 0x4D8, 0x0A00F600);  
	TVD_WRITE32(tvd_base[channel_id] + 0x4DC, 0xF41F0800);  
	TVD_WRITE32(tvd_base[channel_id] + 0x4E0, 0x13178080);  
	TVD_WRITE32(tvd_base[channel_id] + 0x4E4, 0x8AA7075E);  
	TVD_WRITE32(tvd_base[channel_id] + 0x4E8, 0xF08400A0);  
	TVD_WRITE32(tvd_base[channel_id] + 0x4EC, 0x08180C40);  
	TVD_WRITE32(tvd_base[channel_id] + 0x4F0, 0x2008C2C9);  
	TVD_WRITE32(tvd_base[channel_id] + 0x4F4, 0x7180001F);  
	TVD_WRITE32(tvd_base[channel_id] + 0x4F8, 0x8000306C);  
	TVD_WRITE32(tvd_base[channel_id] + 0x4FC, 0x420C33A0);  
	TVD_WRITE32(tvd_base[channel_id] + 0x500, 0x6838FA27);  
	TVD_WRITE32(tvd_base[channel_id] + 0x504, 0x0DFA2090);  
	TVD_WRITE32(tvd_base[channel_id] + 0x508, 0x08E06B40);  
	TVD_WRITE32(tvd_base[channel_id] + 0x50C, 0x83608016);  
	TVD_WRITE32(tvd_base[channel_id] + 0x510, 0x40204080);  
	TVD_WRITE32(tvd_base[channel_id] + 0x514, 0x80326485);  
	TVD_WRITE32(tvd_base[channel_id] + 0x518, 0x11A0A04C);  
	TVD_WRITE32(tvd_base[channel_id] + 0x51C, 0x8040A0C0);  
	TVD_WRITE32(tvd_base[channel_id] + 0x520, 0x7E35AF80);  
	TVD_WRITE32(tvd_base[channel_id] + 0x524, 0x0408F840);  
	TVD_WRITE32(tvd_base[channel_id] + 0x528, 0x5404E030);  
	TVD_WRITE32(tvd_base[channel_id] + 0x52C, 0x30900AA0);  
	TVD_WRITE32(tvd_base[channel_id] + 0x530, 0x0B410B09);  
	TVD_WRITE32(tvd_base[channel_id] + 0x534, 0xF43E40F0);  
	TVD_WRITE32(tvd_base[channel_id] + 0x538, 0x452414F0);  
	TVD_WRITE32(tvd_base[channel_id] + 0x53C, 0x55C29325);  

	TVD_WRITE32(tvd_base[channel_id] + 0x540, 0x30173E63);  
	TVD_WRITE32(tvd_base[channel_id] + 0x544, 0x21A41725);  
	TVD_WRITE32(tvd_base[channel_id] + 0x548, 0x30B080FE);  
	TVD_WRITE32(tvd_base[channel_id] + 0x54C, 0x1245482B);  
	TVD_WRITE32(tvd_base[channel_id] + 0x550, 0xB4000023);  
	TVD_WRITE32(tvd_base[channel_id] + 0x554, 0x04AE0318);  
	TVD_WRITE32(tvd_base[channel_id] + 0x558, 0x106441D0);  
	TVD_WRITE32(tvd_base[channel_id] + 0x55C, 0x57856973);  
	TVD_WRITE32(tvd_base[channel_id] + 0x560, 0x29E518EF);  
	TVD_WRITE32(tvd_base[channel_id] + 0x564, 0x2010573D);  
	TVD_WRITE32(tvd_base[channel_id] + 0x568, 0x204CDEC9);  
	TVD_WRITE32(tvd_base[channel_id] + 0x56C, 0x204A9881);  
	TVD_WRITE32(tvd_base[channel_id] + 0x570, 0xFF29DEC5);  
	TVD_WRITE32(tvd_base[channel_id] + 0x574, 0xCA8B0F00);  
	TVD_WRITE32(tvd_base[channel_id] + 0x578, 0x797A8E61);  
	TVD_WRITE32(tvd_base[channel_id] + 0x57C, 0x420FA8A1);  
	TVD_WRITE32(tvd_base[channel_id] + 0x580, 0x208181D4);  
	TVD_WRITE32(tvd_base[channel_id] + 0x584, 0x28AF88CA);  
	TVD_WRITE32(tvd_base[channel_id] + 0x588, 0xFC844040);  
	TVD_WRITE32(tvd_base[channel_id] + 0x58C, 0x2854C531);  
	TVD_WRITE32(tvd_base[channel_id] + 0x590, 0x0206E61A);  
	TVD_WRITE32(tvd_base[channel_id] + 0x594, 0x4840310E);  
	TVD_WRITE32(tvd_base[channel_id] + 0x598, 0x21594088);  
	TVD_WRITE32(tvd_base[channel_id] + 0x59C, 0x29E290B1);  
	TVD_WRITE32(tvd_base[channel_id] + 0x5A0, 0x44800C80);  
	TVD_WRITE32(tvd_base[channel_id] + 0x5A4, 0x20010000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x5A8, 0x0A4BC6C1);  
	TVD_WRITE32(tvd_base[channel_id] + 0x5AC, 0x400010C0);  
	TVD_WRITE32(tvd_base[channel_id] + 0x5B0, 0x20080400);  
	TVD_WRITE32(tvd_base[channel_id] + 0x5B4, 0x00F9BC00);  
	TVD_WRITE32(tvd_base[channel_id] + 0x5B8, 0x20002004);  
	TVD_WRITE32(tvd_base[channel_id] + 0x5BC, 0x17E0114F);  
	TVD_WRITE32(tvd_base[channel_id] + 0x5C0, 0x1F908420);  
	TVD_WRITE32(tvd_base[channel_id] + 0x5C4, 0x1A191515);  
	TVD_WRITE32(tvd_base[channel_id] + 0x5C8, 0x06658819);  
	TVD_WRITE32(tvd_base[channel_id] + 0x5CC, 0x72B91AB9);  
	TVD_WRITE32(tvd_base[channel_id] + 0x5D0, 0x5A1640DB);  
	TVD_WRITE32(tvd_base[channel_id] + 0x5D4, 0x55F401F4);  
	TVD_WRITE32(tvd_base[channel_id] + 0x5D8, 0x100181FA);  
	TVD_WRITE32(tvd_base[channel_id] + 0x5DC, 0x60506E60);  
	TVD_WRITE32(tvd_base[channel_id] + 0x5E0, 0xA0A797A6);  
	TVD_WRITE32(tvd_base[channel_id] + 0x5E4, 0xFAC9D1A0);  
	TVD_WRITE32(tvd_base[channel_id] + 0x5E8, 0x7CB83BD4);  
	TVD_WRITE32(tvd_base[channel_id] + 0x5EC, 0x80DD43A9);  
	TVD_WRITE32(tvd_base[channel_id] + 0x5F0, 0x80554329);  
	TVD_WRITE32(tvd_base[channel_id] + 0x5F4, 0x1748A14B);  
	TVD_WRITE32(tvd_base[channel_id] + 0x5F8, 0x50E3C67F);  
	TVD_WRITE32(tvd_base[channel_id] + 0x5FC, 0x70D70480);  
	TVD_WRITE32(tvd_base[channel_id] + 0x600, 0x3D34035F);  
	TVD_WRITE32(tvd_base[channel_id] + 0x604, 0xF9228320);  
	TVD_WRITE32(tvd_base[channel_id] + 0x608, 0x32201501);  
	TVD_WRITE32(tvd_base[channel_id] + 0x60C, 0x5985A833);  
	TVD_WRITE32(tvd_base[channel_id] + 0x610, 0x000028AD);  
	TVD_WRITE32(tvd_base[channel_id] + 0x614, 0x55E86B73);  
	TVD_WRITE32(tvd_base[channel_id] + 0x618, 0xE6CB2224);  
	TVD_WRITE32(tvd_base[channel_id] + 0x61C, 0x84460080);  
	TVD_WRITE32(tvd_base[channel_id] + 0x620, 0x00400000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x624, 0x7F084330);  

	//SCART_02
	TVD_WRITE32(tvd_base[channel_id] + 0x628, 0x08880001); 
	TVD_WRITE32(tvd_base[channel_id] + 0x62C, 0xBF488488);  
	TVD_WRITE32(tvd_base[channel_id] + 0x630, 0x0119812C);  
	TVD_WRITE32(tvd_base[channel_id] + 0x634, 0x01000844);  
	TVD_WRITE32(tvd_base[channel_id] + 0x638, 0x88020818);  
	TVD_WRITE32(tvd_base[channel_id] + 0x63C, 0x0FF0AA1F);  

	//comb start
	TVD_WRITE32(tvd_base[channel_id] + 0x640, 0x180B101F);  
	TVD_WRITE32(tvd_base[channel_id] + 0x640, 0x000B101F); 
	//TDC_WRITE32(0x644,0x09A2F600);
	TVD_WRITE32(tvd_base[channel_id] + 0x644, TVD_READ32(tvd_base[channel_id] + 0x644)|(0x1<<27));
	TVD_WRITE32(tvd_base[channel_id] + 0x648, 0x8A645AFB); 

	//TDC_WRITE32(0x64C,0x25030200); 
	TVD_WRITE32(tvd_base[channel_id] + 0x64C,(TVD_READ32(tvd_base[channel_id] + 0x64C)&(0x1<<4)) | 0x25030200); 

	TVD_WRITE32(tvd_base[channel_id] + 0x650, 0x02F70000); 
	TVD_WRITE32(tvd_base[channel_id] + 0x654, 0x27030005); 
	TVD_WRITE32(tvd_base[channel_id] + 0x658, 0x01A60A0D); 
	TVD_WRITE32(tvd_base[channel_id] + 0x65C, 0x01FC1500); 
	TVD_WRITE32(tvd_base[channel_id] + 0x660, 0x0111B106); 
	TVD_WRITE32(tvd_base[channel_id] + 0x664, 0x38E3200B); 
	TVD_WRITE32(tvd_base[channel_id] + 0x668, 0x0A074A36); 
	TVD_WRITE32(tvd_base[channel_id] + 0x66C, 0x0A074A36); 
	TVD_WRITE32(tvd_base[channel_id] + 0x670, 0x0000CCBD); 
	TVD_WRITE32(tvd_base[channel_id] + 0x674, 0x00000004); 
	//TDC_WRITE32(0x678,0xCB656500);  
	TVD_WRITE32(tvd_base[channel_id] + 0x67C, 0x38081808); 
	TVD_WRITE32(tvd_base[channel_id] + 0x680, 0x090A1E05); 
	TVD_WRITE32(tvd_base[channel_id] + 0x684, 0x64180810); 
	TVD_WRITE32(tvd_base[channel_id] + 0x688, 0x64AA321E); 
	TVD_WRITE32(tvd_base[channel_id] + 0x68C, 0x32107023); 
	TVD_WRITE32(tvd_base[channel_id] + 0x690, 0x0E013F85); 
	TVD_WRITE32(tvd_base[channel_id] + 0x694, 0x0000B910); 
	TVD_WRITE32(tvd_base[channel_id] + 0x698, 0x0064C800); 
	TVD_WRITE32(tvd_base[channel_id] + 0x69C, 0x70680610); 
	TVD_WRITE32(tvd_base[channel_id] + 0x6A0, 0x0000170F); 
	TVD_WRITE32(tvd_base[channel_id] + 0x6A4, 0x12782880); 
	TVD_WRITE32(tvd_base[channel_id] + 0x6A8, 0x60084084); 
	TVD_WRITE32(tvd_base[channel_id] + 0x6AC, 0x1E0F1904); 
	TVD_WRITE32(tvd_base[channel_id] + 0x6B0, 0x100F0344); 
	TVD_WRITE32(tvd_base[channel_id] + 0x6B4, 0x3E505F00); 
	TVD_WRITE32(tvd_base[channel_id] + 0x6B8, 0x04414444); 
	TVD_WRITE32(tvd_base[channel_id] + 0x6BC, 0x00110333); 
	TVD_WRITE32(tvd_base[channel_id] + 0x6C0, 0x00A404F3); 
	TVD_WRITE32(tvd_base[channel_id] + 0x6C4, 0x45195030); 
	TVD_WRITE32(tvd_base[channel_id] + 0x6C8, 0x0640285C); 
	TVD_WRITE32(tvd_base[channel_id] + 0x6CC, 0x11600811); 
	TVD_WRITE32(tvd_base[channel_id] + 0x6D0, 0x11111111); 
	TVD_WRITE32(tvd_base[channel_id] + 0x6D4, 0x0008FC00); 
	TVD_WRITE32(tvd_base[channel_id] + 0x6D8, 0x00000000); 
	TVD_WRITE32(tvd_base[channel_id] + 0x6DC, 0xC0031008); 
	TVD_WRITE32(tvd_base[channel_id] + 0x6E0, 0x141420FF); 
	TVD_WRITE32(tvd_base[channel_id] + 0x6E4, 0x32200060); 
	TVD_WRITE32(tvd_base[channel_id] + 0x6E8, 0x20906000); 
	TVD_WRITE32(tvd_base[channel_id] + 0x6EC, 0x0000000D); 
	TVD_WRITE32(tvd_base[channel_id] + 0x6F0, 0x43174537); 
	TVD_WRITE32(tvd_base[channel_id] + 0x6F4, 0x0045633C); 
	TVD_WRITE32(tvd_base[channel_id] + 0x6F8, 0xC0001A50); 
	TVD_WRITE32(tvd_base[channel_id] + 0x6FC, 0x0414050D); 
	TVD_WRITE32(tvd_base[channel_id] + 0x700, 0x11A00020);  
	TVD_WRITE32(tvd_base[channel_id] + 0x704, 0x64000450); 
	TVD_WRITE32(tvd_base[channel_id] + 0x708, 0x410A0028); 
	TVD_WRITE32(tvd_base[channel_id] + 0x70C, 0x00014455); 
	TVD_WRITE32(tvd_base[channel_id] + 0x710, 0x00000000); 
	TVD_WRITE32(tvd_base[channel_id] + 0x714, 0x314859C8); 
	TVD_WRITE32(tvd_base[channel_id] + 0x718, 0x809E3001); 
	TVD_WRITE32(tvd_base[channel_id] + 0x71C, 0x08001020); 
	TVD_WRITE32(tvd_base[channel_id] + 0x720, 0x08081E03); 
	TVD_WRITE32(tvd_base[channel_id] + 0x724, 0x00100008); 
	TVD_WRITE32(tvd_base[channel_id] + 0x728, 0x14061210); 
	TVD_WRITE32(tvd_base[channel_id] + 0x72C, 0x10101022);  
	TVD_WRITE32(tvd_base[channel_id] + 0x730, 0x20502010); 
	TVD_WRITE32(tvd_base[channel_id] + 0x734, 0x3311001A); 
	TVD_WRITE32(tvd_base[channel_id] + 0x738, 0x1111046A); 
	TVD_WRITE32(tvd_base[channel_id] + 0x73C, 0x8000080A); 
	TVD_WRITE32(tvd_base[channel_id] + 0x740, 0x01245678); 
	TVD_WRITE32(tvd_base[channel_id] + 0x744, 0x02345678);
	TVD_WRITE32_MASK(tvd_base[channel_id] + 0x748, 0x0008300A, 0xCFFFFFFF);
	TVD_WRITE32(tvd_base[channel_id] + 0x74C, 0x00022A34); 
	TVD_WRITE32(tvd_base[channel_id] + 0x750, 0x047C0558); 
	TVD_WRITE32(tvd_base[channel_id] + 0x754, 0x00880114); 
	TVD_WRITE32(tvd_base[channel_id] + 0x758, 0x00006A68); 
	TVD_WRITE32(tvd_base[channel_id] + 0x75C, 0x000783FF); 
	TVD_WRITE32(tvd_base[channel_id] + 0x760, 0x90000000); 
	TVD_WRITE32(tvd_base[channel_id] + 0x764, 0x000073FE);  
	TVD_WRITE32(tvd_base[channel_id] + 0x768, 0x0F051818); 
	TVD_WRITE32(tvd_base[channel_id] + 0x76C, 0x00191014); 
	TVD_WRITE32(tvd_base[channel_id] + 0x770, 0x23645050); 
	TVD_WRITE32(tvd_base[channel_id] + 0x774, 0x64641432); 
	TVD_WRITE32(tvd_base[channel_id] + 0x778, 0x1E519696); 
	TVD_WRITE32(tvd_base[channel_id] + 0x77C, 0x10169696); 
	TVD_WRITE32(tvd_base[channel_id] + 0x780, 0x00504600); 
	TVD_WRITE32(tvd_base[channel_id] + 0x784, 0x821FFFFF); 
	TVD_WRITE32(tvd_base[channel_id] + 0x788, 0xFFFF3359);  

	TVD_WRITE32(tvd_base[channel_id] + 0x86C, 0xC0400C0A);  
	TVD_WRITE32(tvd_base[channel_id] + 0x870, 0x00020820);  
	TVD_WRITE32(tvd_base[channel_id] + 0x874, 0xC0016C01);  


	TVD_WRITE32(tvd_base[channel_id] + 0x8B8, 0xC0016C01);  
	TVD_WRITE32(tvd_base[channel_id] + 0x8BC, 0x6EC86EC8); 

	//DFE20~dfe23
	TVD_WRITE32(tvd_base[channel_id] + 0xAB0, 0x22F08C03);  
	TVD_WRITE32(tvd_base[channel_id] + 0xAB4, 0x8F0070D4);  
	TVD_WRITE32(tvd_base[channel_id] + 0xAB8, 0x42400800);  
	TVD_WRITE32(tvd_base[channel_id] + 0xABC, 0x82000820); 
	
    TVD_WRITE32(tvd_base[channel_id] + 0x5aC, 0x000010C0);  //bit 31
	TVD_WRITE32(tvd_base[channel_id] + 0x59C, 0x21E2c0B2); //H start
    TVD_WRITE32(tvd_base[channel_id] + 0x5c4, 0x18181617);  //v start

	return;
}


void PAL_3D_setting(TVD_CHANNEL_ID channel_id)
{
	//form lumer plus

	TVD_WRITE32(tvd_base[channel_id] + 0x410, 0x00818000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x414, 0xFF080020);  

	TVD_WRITE32(tvd_base[channel_id] + 0x434, 0xC4D00101);  


	TVD_WRITE32(tvd_base[channel_id] + 0x438, 0xB801404A);  
	TVD_WRITE32(tvd_base[channel_id] + 0x43C, 0x00800801); 

	TVD_WRITE32(tvd_base[channel_id] + 0x484, 0x100F0480);  
	TVD_WRITE32(tvd_base[channel_id] + 0x488, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x48C, 0x0200022D);  


	TVD_WRITE32(tvd_base[channel_id] + 0x4BC, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x4C0, 0x51515C55);  
	TVD_WRITE32(tvd_base[channel_id] + 0x4C4, 0x04557845);  
	TVD_WRITE32(tvd_base[channel_id] + 0x4C8, 0x27F7595E);  
	TVD_WRITE32(tvd_base[channel_id] + 0x4CC, 0x8F3403A0);  
	TVD_WRITE32(tvd_base[channel_id] + 0x4D0, 0x15890F80);  
	TVD_WRITE32(tvd_base[channel_id] + 0x4D4, 0x26144040);  
	TVD_WRITE32(tvd_base[channel_id] + 0x4D8, 0x0A00F600);  
	TVD_WRITE32(tvd_base[channel_id] + 0x4DC, 0xF41F0800);  
	TVD_WRITE32(tvd_base[channel_id] + 0x4E0, 0x13178080);  
	TVD_WRITE32(tvd_base[channel_id] + 0x4E4, 0x8AA7075E);  
	TVD_WRITE32(tvd_base[channel_id] + 0x4E8, 0xF08400A0);  
	TVD_WRITE32(tvd_base[channel_id] + 0x4EC, 0x08180C40);  
	TVD_WRITE32(tvd_base[channel_id] + 0x4F0, 0x2008C2C9);  
	TVD_WRITE32(tvd_base[channel_id] + 0x4F4, 0x7180001F);  
	TVD_WRITE32(tvd_base[channel_id] + 0x4F8, 0x8000306C);  
	TVD_WRITE32(tvd_base[channel_id] + 0x4FC, 0x420C33A0);  
	TVD_WRITE32(tvd_base[channel_id] + 0x500, 0x6838FA27);  
	TVD_WRITE32(tvd_base[channel_id] + 0x504, 0x0DFA2090);  
	TVD_WRITE32(tvd_base[channel_id] + 0x508, 0x08E06B40);  
	TVD_WRITE32(tvd_base[channel_id] + 0x50C, 0x83608016);  
	TVD_WRITE32(tvd_base[channel_id] + 0x510, 0x40204080);  
	TVD_WRITE32(tvd_base[channel_id] + 0x514, 0x80326485);  
	TVD_WRITE32(tvd_base[channel_id] + 0x518, 0x11A0A04C);  
	TVD_WRITE32(tvd_base[channel_id] + 0x51C, 0x8040A0C0);  
	TVD_WRITE32(tvd_base[channel_id] + 0x520, 0x7E35AF80);  
	TVD_WRITE32(tvd_base[channel_id] + 0x524, 0x0408F840);  
	TVD_WRITE32(tvd_base[channel_id] + 0x528, 0x5404E030);  
	TVD_WRITE32(tvd_base[channel_id] + 0x52C, 0x30900AA0);  
	TVD_WRITE32(tvd_base[channel_id] + 0x530, 0x0B410B09);  
	TVD_WRITE32(tvd_base[channel_id] + 0x534, 0xF43E40F0);  
	TVD_WRITE32(tvd_base[channel_id] + 0x538, 0x452414F0);  
	TVD_WRITE32(tvd_base[channel_id] + 0x53C, 0x55C29325);  
	TVD_WRITE32(tvd_base[channel_id] + 0x540, 0x30173F93);  
	TVD_WRITE32(tvd_base[channel_id] + 0x544, 0x21A41725);  
	TVD_WRITE32(tvd_base[channel_id] + 0x548, 0x408080FE);  
	TVD_WRITE32(tvd_base[channel_id] + 0x54C, 0x1245482B);  
	TVD_WRITE32(tvd_base[channel_id] + 0x550, 0xB4000023);  
	TVD_WRITE32(tvd_base[channel_id] + 0x554, 0x04AE0318);  
	TVD_WRITE32(tvd_base[channel_id] + 0x558, 0x106443D0);  
	TVD_WRITE32(tvd_base[channel_id] + 0x55C, 0x52836471);  
	TVD_WRITE32(tvd_base[channel_id] + 0x560, 0x29E518EF);  
	TVD_WRITE32(tvd_base[channel_id] + 0x564, 0x2010553C);  
	TVD_WRITE32(tvd_base[channel_id] + 0x568, 0x204CDEC9);  
	TVD_WRITE32(tvd_base[channel_id] + 0x56C, 0x204A9881);  
	TVD_WRITE32(tvd_base[channel_id] + 0x570, 0xFF29DEC5);  
	TVD_WRITE32(tvd_base[channel_id] + 0x574, 0xCA8B0F00);  
	TVD_WRITE32(tvd_base[channel_id] + 0x578, 0x797A8E61);  
	TVD_WRITE32(tvd_base[channel_id] + 0x57C, 0x420FA8A1);  
	TVD_WRITE32(tvd_base[channel_id] + 0x580, 0x208181D4);  
	TVD_WRITE32(tvd_base[channel_id] + 0x584, 0x28AF88C8);  
	TVD_WRITE32(tvd_base[channel_id] + 0x588, 0xEC844040);  
	TVD_WRITE32(tvd_base[channel_id] + 0x58C, 0x2854C531);  
	TVD_WRITE32(tvd_base[channel_id] + 0x590, 0x0206E61A);  
	TVD_WRITE32(tvd_base[channel_id] + 0x594, 0x4840310E);  
	TVD_WRITE32(tvd_base[channel_id] + 0x598, 0x21594088);  
	TVD_WRITE32(tvd_base[channel_id] + 0x59C, 0x29E2C0A5);  
	TVD_WRITE32(tvd_base[channel_id] + 0x5A0, 0x44800C80);  
	TVD_WRITE32(tvd_base[channel_id] + 0x5A4, 0x20010000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x5A8, 0x0A4BC680);  
	TVD_WRITE32(tvd_base[channel_id] + 0x5AC, 0x400010C0);  
	TVD_WRITE32(tvd_base[channel_id] + 0x5B0, 0x20008400);  // when sync level from small to larger in benchmark, the h lock is fail in 6 level value
	TVD_WRITE32(tvd_base[channel_id] + 0x5B4, 0x00F9BC00);  
	TVD_WRITE32(tvd_base[channel_id] + 0x5B8, 0x20002004);  
	TVD_WRITE32(tvd_base[channel_id] + 0x5BC, 0x17E0114F);  
	TVD_WRITE32(tvd_base[channel_id] + 0x5C0, 0x1F908420);  
	TVD_WRITE32(tvd_base[channel_id] + 0x5C4, 0x1A191515);  
	TVD_WRITE32(tvd_base[channel_id] + 0x5C8, 0x06658819);  
	TVD_WRITE32(tvd_base[channel_id] + 0x5CC, 0x72B91AB9);  
	TVD_WRITE32(tvd_base[channel_id] + 0x5D0, 0x5A1640DB);  
	TVD_WRITE32(tvd_base[channel_id] + 0x5D4, 0x55F401F4);  
	TVD_WRITE32(tvd_base[channel_id] + 0x5D8, 0x1001825E);  
	TVD_WRITE32(tvd_base[channel_id] + 0x5DC, 0x60506E60);  
	TVD_WRITE32(tvd_base[channel_id] + 0x5E0, 0xA0A793A6);  
	TVD_WRITE32(tvd_base[channel_id] + 0x5E4, 0xFACAD1A0);  
	TVD_WRITE32(tvd_base[channel_id] + 0x5E8, 0x7CB83BD4);  
	TVD_WRITE32(tvd_base[channel_id] + 0x5EC, 0x80DD43A9);  
	TVD_WRITE32(tvd_base[channel_id] + 0x5F0, 0x80554329);  
	TVD_WRITE32(tvd_base[channel_id] + 0x5F4, 0x1748A14B);  
	TVD_WRITE32(tvd_base[channel_id] + 0x5F8, 0x5040C67F);  
	TVD_WRITE32(tvd_base[channel_id] + 0x5FC, 0x70D80480);  
	TVD_WRITE32(tvd_base[channel_id] + 0x600, 0x3D34035F);  
	TVD_WRITE32(tvd_base[channel_id] + 0x604, 0xF9228320);  
	TVD_WRITE32(tvd_base[channel_id] + 0x608, 0x32201501);  
	TVD_WRITE32(tvd_base[channel_id] + 0x60C, 0x5480A833);  
	TVD_WRITE32(tvd_base[channel_id] + 0x610, 0x000028AD);  
	TVD_WRITE32(tvd_base[channel_id] + 0x614, 0x55E8666E);  
	TVD_WRITE32(tvd_base[channel_id] + 0x618, 0xE6CB2224);  
	TVD_WRITE32(tvd_base[channel_id] + 0x61C, 0x84460080);  
	TVD_WRITE32(tvd_base[channel_id] + 0x620, 0x00400000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x624, 0x7F084330);  
	TVD_WRITE32(tvd_base[channel_id] + 0x628, 0x08880001);  
	TVD_WRITE32(tvd_base[channel_id] + 0x62C, 0xBF488488);  
	TVD_WRITE32(tvd_base[channel_id] + 0x630, 0x0119812C);  
	TVD_WRITE32(tvd_base[channel_id] + 0x634, 0x01000844);  
	TVD_WRITE32(tvd_base[channel_id] + 0x638, 0x88020818);  
	TVD_WRITE32(tvd_base[channel_id] + 0x63C, 0x0FF0AA1F);  

	//comb start
	//TDC_WRITE32(0x640,0x000B101F);  
	TVD_WRITE32(tvd_base[channel_id] + 0x640, 0x180B101F);  
	TVD_WRITE32(tvd_base[channel_id] + 0x640, 0x000B101F);  
	//TDC_WRITE32(0x644,0x09A40600);  
	TVD_WRITE32(tvd_base[channel_id] + 0x644, TVD_READ32(tvd_base[channel_id] + 0x644)|(0x1 << 27));  
	TVD_WRITE32(tvd_base[channel_id] + 0x648, 0x8A048BB1);  

	//TDC_WRITE32(0x64C,0x2F360203);
	TVD_WRITE32(tvd_base[channel_id] + 0x64C,(TVD_READ32(tvd_base[channel_id] + 0x64C)&(0x1<<4))| 0x2F360203);
	TVD_WRITE32(tvd_base[channel_id] + 0x650,0x03A80100);	
	TVD_WRITE32(tvd_base[channel_id] + 0x654,0x3136400A);	
	TVD_WRITE32(tvd_base[channel_id] + 0x658,0x02361E74);	
	TVD_WRITE32(tvd_base[channel_id] + 0x65C,0x02441800);	
	TVD_WRITE32(tvd_base[channel_id] + 0x660,0x01151139);	
	TVD_WRITE32(tvd_base[channel_id] + 0x664,0x46F3C00B);	
	TVD_WRITE32(tvd_base[channel_id] + 0x668,0x0A0AD303);	
	TVD_WRITE32(tvd_base[channel_id] + 0x66C,0x0A0AD303);	
	TVD_WRITE32(tvd_base[channel_id] + 0x670,0x000098C4);	
	TVD_WRITE32(tvd_base[channel_id] + 0x674,0x00000007);	
	//TDC_WRITE32(0x678,0x77777777);  
	TVD_WRITE32(tvd_base[channel_id] + 0x67C,0x4A101030);	
	TVD_WRITE32(tvd_base[channel_id] + 0x680,0x510A1E05);	
	TVD_WRITE32(tvd_base[channel_id] + 0x684,0x6218040C);	
	TVD_WRITE32(tvd_base[channel_id] + 0x688,0x8296321E);	
	TVD_WRITE32(tvd_base[channel_id] + 0x68C,0x00107010);	
	TVD_WRITE32(tvd_base[channel_id] + 0x690,0x0F01EF85);	
	TVD_WRITE32(tvd_base[channel_id] + 0x694,0x1070500C);	
	TVD_WRITE32(tvd_base[channel_id] + 0x698,0x00641400);	
	TVD_WRITE32(tvd_base[channel_id] + 0x69C,0x00300500);	
	TVD_WRITE32(tvd_base[channel_id] + 0x6A0,0x0000230A);	
	TVD_WRITE32(tvd_base[channel_id] + 0x6A4,0x1E712808);	
	TVD_WRITE32(tvd_base[channel_id] + 0x6A8,0x8006408A);	
	TVD_WRITE32(tvd_base[channel_id] + 0x6AC,0x54280000);	
	TVD_WRITE32(tvd_base[channel_id] + 0x6B0,0x200F0000);	
	TVD_WRITE32(tvd_base[channel_id] + 0x6B4,0x00969600);	
	TVD_WRITE32(tvd_base[channel_id] + 0x6B8,0x04044444);	
	TVD_WRITE32(tvd_base[channel_id] + 0x6BC,0x0C010444);	
	TVD_WRITE32(tvd_base[channel_id] + 0x6C0,0x00A444F3);	
	TVD_WRITE32(tvd_base[channel_id] + 0x6C4,0x3E1A53D3);	
	TVD_WRITE32(tvd_base[channel_id] + 0x6C8,0x064B1478);	
	TVD_WRITE32(tvd_base[channel_id] + 0x6CC,0x1020D831);	
	TVD_WRITE32(tvd_base[channel_id] + 0x6D0,0x11111111);	
	TVD_WRITE32(tvd_base[channel_id] + 0x6D4,0xC008FC10);	
	TVD_WRITE32(tvd_base[channel_id] + 0x6D8,0x00000000);	
	TVD_WRITE32(tvd_base[channel_id] + 0x6DC,0xC0001020);	
	TVD_WRITE32(tvd_base[channel_id] + 0x6E0,0x0F271858);	
	TVD_WRITE32(tvd_base[channel_id] + 0x6E4,0x3220008F);	
	TVD_WRITE32(tvd_base[channel_id] + 0x6E8,0xA02D0E28);	
	TVD_WRITE32(tvd_base[channel_id] + 0x6EC,0x00000054);	
	TVD_WRITE32(tvd_base[channel_id] + 0x6F0,0x43174537);	
	TVD_WRITE32(tvd_base[channel_id] + 0x6F4,0x0045713C);	
	TVD_WRITE32(tvd_base[channel_id] + 0x6F8,0xC0001A50);
	TVD_WRITE32(tvd_base[channel_id] + 0x6FC,0x4414050D);
	TVD_WRITE32(tvd_base[channel_id] + 0x700, 0x11A00020);  
	TVD_WRITE32(tvd_base[channel_id] + 0x704,0x64800450);
	TVD_WRITE32(tvd_base[channel_id] + 0x708,0x410A0028);
	TVD_WRITE32(tvd_base[channel_id] + 0x70C,0x00014455);
	TVD_WRITE32(tvd_base[channel_id] + 0x710,0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0x714,0x31581E48);
	TVD_WRITE32(tvd_base[channel_id] + 0x718,0x8C9B3000);
	TVD_WRITE32(tvd_base[channel_id] + 0x71C,0x08101020);
	TVD_WRITE32(tvd_base[channel_id] + 0x720,0x08081E14);
	TVD_WRITE32(tvd_base[channel_id] + 0x724,0x0A100008);
	TVD_WRITE32(tvd_base[channel_id] + 0x728,0x14060208);
	TVD_WRITE32(tvd_base[channel_id] + 0x72C,0x0C0A2808);
	TVD_WRITE32(tvd_base[channel_id] + 0x730,0x0A0D9226);
	TVD_WRITE32(tvd_base[channel_id] + 0x734, 0x33110029);  
	TVD_WRITE32(tvd_base[channel_id] + 0x738,0x1311042A);
	TVD_WRITE32(tvd_base[channel_id] + 0x73C,0x8C03100A);
	TVD_WRITE32(tvd_base[channel_id] + 0x740,0x12345678);
	TVD_WRITE32(tvd_base[channel_id] + 0x744,0x02345678);
	TVD_WRITE32_MASK(tvd_base[channel_id] + 0x748,0x00081414, 0xCFFFFFFF);
	TVD_WRITE32(tvd_base[channel_id] + 0x74C,0x00030A10);
	TVD_WRITE32(tvd_base[channel_id] + 0x750,0x047C0558);
	TVD_WRITE32(tvd_base[channel_id] + 0x754,0x0088011A);
	TVD_WRITE32(tvd_base[channel_id] + 0x758,0x00009666);
	TVD_WRITE32(tvd_base[channel_id] + 0x75C,0x000781FF);
	TVD_WRITE32(tvd_base[channel_id] + 0x760,0x90000000);
	TVD_WRITE32(tvd_base[channel_id] + 0x764,0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0x768,0x0F051818);
	TVD_WRITE32(tvd_base[channel_id] + 0x76C,0x00191014);
	TVD_WRITE32(tvd_base[channel_id] + 0x770,0x23645050);
	TVD_WRITE32(tvd_base[channel_id] + 0x774,0x64641432);
	TVD_WRITE32(tvd_base[channel_id] + 0x778,0x1E519696);
	TVD_WRITE32(tvd_base[channel_id] + 0x77C,0x10169696);
	TVD_WRITE32(tvd_base[channel_id] + 0x780,0x053C501E);
	TVD_WRITE32(tvd_base[channel_id] + 0x784,0x820A325A);
	TVD_WRITE32(tvd_base[channel_id] + 0x788, 0xFFFF3359);  
	TVD_WRITE32(tvd_base[channel_id] + 0x78C,0xFFFFFFFF);
	//comb end

	TVD_WRITE32(tvd_base[channel_id] + 0x86C, 0xC0400C0A);  
	TVD_WRITE32(tvd_base[channel_id] + 0x870, 0x00020820);  
	TVD_WRITE32(tvd_base[channel_id] + 0x874, 0xC0016C01);  

	TVD_WRITE32(tvd_base[channel_id] + 0x8B8, 0xC0016C01);  
	TVD_WRITE32(tvd_base[channel_id] + 0x8BC, 0x6EC86EC8);  



	TVD_WRITE32(tvd_base[channel_id] + 0xAB0, 0x22F08C03);  
	TVD_WRITE32(tvd_base[channel_id] + 0xAB4, 0x8F0070D4);  
	TVD_WRITE32(tvd_base[channel_id] + 0xAB8, 0x42400800);  
	TVD_WRITE32(tvd_base[channel_id] + 0xABC, 0x82000820);  
    TVD_WRITE32(tvd_base[channel_id] + 0x5aC, 0x000010C0);  //bit 31
	TVD_WRITE32(tvd_base[channel_id] + 0x59C, 0x21E2c0B2); //H start
    TVD_WRITE32(tvd_base[channel_id] + 0x5c4, 0x19191617);  //v start

	return;
}

void vNTSC_2D_Setting(TVD_CHANNEL_ID channel_id)
{
	TVD_WRITE32(tvd_base[channel_id] + 0x410, 0x00818000);
	TVD_WRITE32(tvd_base[channel_id] + 0x414, 0xFF080020);
	TVD_WRITE32(tvd_base[channel_id] + 0x418, 0x10000000);
	TVD_WRITE32(tvd_base[channel_id] + 0x41C, 0x12400010);
	TVD_WRITE32(tvd_base[channel_id] + 0x420, 0x22788207);
	TVD_WRITE32(tvd_base[channel_id] + 0x424, 0x000000FF);
	TVD_WRITE32(tvd_base[channel_id] + 0x428, 0xFFFFFFFF);
	TVD_WRITE32(tvd_base[channel_id] + 0x42C, 0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0x430, 0x00000000);

	TVD_WRITE32(tvd_base[channel_id] + 0x434, 0xC4D00101);

	TVD_WRITE32(tvd_base[channel_id] + 0x438, 0xB801404A);
	TVD_WRITE32(tvd_base[channel_id] + 0x43C, 0x00800801);
	TVD_WRITE32(tvd_base[channel_id] + 0x440, 0x00D01001);
	TVD_WRITE32(tvd_base[channel_id] + 0x444, 0x1E000000);
	TVD_WRITE32(tvd_base[channel_id] + 0x448, 0x80000114);
	TVD_WRITE32(tvd_base[channel_id] + 0x44C, 0x2AAA8103);
	TVD_WRITE32(tvd_base[channel_id] + 0x450, 0xF0000100);
	TVD_WRITE32(tvd_base[channel_id] + 0x454, 0x1E000000);
	TVD_WRITE32(tvd_base[channel_id] + 0x458, 0x80000114);
	TVD_WRITE32(tvd_base[channel_id] + 0x45C, 0x2AAA8103);
	TVD_WRITE32(tvd_base[channel_id] + 0x460, 0x47B00000);
	TVD_WRITE32(tvd_base[channel_id] + 0x464, 0x1E000000);
	TVD_WRITE32(tvd_base[channel_id] + 0x468, 0x80000114);
	TVD_WRITE32(tvd_base[channel_id] + 0x46C, 0x2AAA8103);
	TVD_WRITE32(tvd_base[channel_id] + 0x470, 0x1100001B);
	TVD_WRITE32(tvd_base[channel_id] + 0x474, 0xAAA04EAB);
	TVD_WRITE32(tvd_base[channel_id] + 0x478, 0x0888AAAA);
	TVD_WRITE32(tvd_base[channel_id] + 0x47C, 0x00050000);
	TVD_WRITE32(tvd_base[channel_id] + 0x480, 0x003E313D);
	TVD_WRITE32(tvd_base[channel_id] + 0x484, 0x100F0480);
	//TVD_WRITE32(0x488,0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0x48C, 0x0200022D);
	TVD_WRITE32(tvd_base[channel_id] + 0x490, 0x0AFFC588);
	TVD_WRITE32(tvd_base[channel_id] + 0x494, 0x08000000);
	TVD_WRITE32(tvd_base[channel_id] + 0x498, 0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0x49C, 0x0009C000);
	TVD_WRITE32(tvd_base[channel_id] + 0x4A0, 0x00090000);
	TVD_WRITE32(tvd_base[channel_id] + 0x4A4, 0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0x4A8, 0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0x4AC, 0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0x4B0, 0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0x4B4, 0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0x4B8, 0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0x4BC, 0x00000000);

	TVD_WRITE32(tvd_base[channel_id] + 0x4C0, 0x51515555);
	TVD_WRITE32(tvd_base[channel_id] + 0x4C4, 0x04557845);
	TVD_WRITE32(tvd_base[channel_id] + 0x4C8, 0x27F7595E);
	TVD_WRITE32(tvd_base[channel_id] + 0x4CC, 0x8F340380);
	TVD_WRITE32(tvd_base[channel_id] + 0x4D0, 0x158C0F80);
	TVD_WRITE32(tvd_base[channel_id] + 0x4D4, 0x26144040);
	TVD_WRITE32(tvd_base[channel_id] + 0x4D8, 0x0A00F600);
	TVD_WRITE32(tvd_base[channel_id] + 0x4DC, 0xF41F0800);
	TVD_WRITE32(tvd_base[channel_id] + 0x4E0, 0x13178080);
	TVD_WRITE32(tvd_base[channel_id] + 0x4E4, 0x8AA7075E);
	TVD_WRITE32(tvd_base[channel_id] + 0x4E8, 0xF08400A0);
	TVD_WRITE32(tvd_base[channel_id] + 0x4EC, 0x08180C40);
	TVD_WRITE32(tvd_base[channel_id] + 0x4F0, 0x2008C2C9);
	TVD_WRITE32(tvd_base[channel_id] + 0x4F4, 0x7180001F);
	TVD_WRITE32(tvd_base[channel_id] + 0x4F8, 0x8000306C);
	TVD_WRITE32(tvd_base[channel_id] + 0x4FC, 0x420C33A0);
	TVD_WRITE32(tvd_base[channel_id] + 0x500, 0x6838FA27);
	TVD_WRITE32(tvd_base[channel_id] + 0x504, 0x0DFA2090);
	TVD_WRITE32(tvd_base[channel_id] + 0x508, 0x08E06B40);
	TVD_WRITE32(tvd_base[channel_id] + 0x50C, 0x83608016);
	TVD_WRITE32(tvd_base[channel_id] + 0x510, 0x40204080);
	TVD_WRITE32(tvd_base[channel_id] + 0x514, 0x80326485);
	TVD_WRITE32(tvd_base[channel_id] + 0x518, 0x11A0A04C);
	TVD_WRITE32(tvd_base[channel_id] + 0x51C, 0x8040A0C0);
	TVD_WRITE32(tvd_base[channel_id] + 0x520, 0x7E35AF80);
	TVD_WRITE32(tvd_base[channel_id] + 0x524, 0x0408F840);

	TVD_WRITE32(tvd_base[channel_id] + 0x528, 0x5404E030);	
	TVD_WRITE32(tvd_base[channel_id] + 0x52C, 0x30900AA0);	
	TVD_WRITE32(tvd_base[channel_id] + 0x530, 0x0B410B09);	
	TVD_WRITE32(tvd_base[channel_id] + 0x534, 0xF43E40F0);	
	TVD_WRITE32(tvd_base[channel_id] + 0x538, 0x452414F0);	
	TVD_WRITE32(tvd_base[channel_id] + 0x53C, 0x2D229325);

	TVD_WRITE32(tvd_base[channel_id] + 0x540, 0x30173E63);
	TVD_WRITE32(tvd_base[channel_id] + 0x544, 0x21A41725);
	TVD_WRITE32(tvd_base[channel_id] + 0x548, 0x30B080FE);
	TVD_WRITE32(tvd_base[channel_id] + 0x54C, 0x1245482B);
	TVD_WRITE32(tvd_base[channel_id] + 0x550, 0xB4000023);
	TVD_WRITE32(tvd_base[channel_id] + 0x554, 0x04AE0318);
	TVD_WRITE32(tvd_base[channel_id] + 0x558, 0x106441D0);
	TVD_WRITE32(tvd_base[channel_id] + 0x55C, 0x57856973);
	TVD_WRITE32(tvd_base[channel_id] + 0x560, 0x29E518EF);
	TVD_WRITE32(tvd_base[channel_id] + 0x564, 0x2010573D);
	TVD_WRITE32(tvd_base[channel_id] + 0x568, 0x204CDEC9);
	TVD_WRITE32(tvd_base[channel_id] + 0x56C, 0x204A9881);
	TVD_WRITE32(tvd_base[channel_id] + 0x570, 0xFF29DEC5);
	TVD_WRITE32(tvd_base[channel_id] + 0x574, 0xCA8B0F00);
	TVD_WRITE32(tvd_base[channel_id] + 0x578, 0x797A8E61);
	TVD_WRITE32(tvd_base[channel_id] + 0x57C, 0x420FA8A1);
	TVD_WRITE32(tvd_base[channel_id] + 0x580, 0x208181D4);
	TVD_WRITE32(tvd_base[channel_id] + 0x584, 0x28AF880A);
	TVD_WRITE32(tvd_base[channel_id] + 0x588, 0xFC844040);
	TVD_WRITE32(tvd_base[channel_id] + 0x58C, 0x2854C531);
	TVD_WRITE32(tvd_base[channel_id] + 0x590, 0x0206E61A);
	TVD_WRITE32(tvd_base[channel_id] + 0x594, 0x4840310E);
	TVD_WRITE32(tvd_base[channel_id] + 0x598, 0x21594088);
	TVD_WRITE32(tvd_base[channel_id] + 0x59C, 0x29E290B1);
	TVD_WRITE32(tvd_base[channel_id] + 0x5A0, 0x44800C80);
	TVD_WRITE32(tvd_base[channel_id] + 0x5A4, 0x20010000);
	TVD_WRITE32(tvd_base[channel_id] + 0x5A8, 0x0A4BC6C1);
	TVD_WRITE32(tvd_base[channel_id] + 0x5AC, 0x400010C0);

	TVD_WRITE32(tvd_base[channel_id] + 0x5B0, 0x20080400);

	TVD_WRITE32(tvd_base[channel_id] + 0x5B4, 0x00F9BC00);
	TVD_WRITE32(tvd_base[channel_id] + 0x5B8, 0x20002004);
	TVD_WRITE32(tvd_base[channel_id] + 0x5BC, 0x17E0114F);
	TVD_WRITE32(tvd_base[channel_id] + 0x5C0, 0x1F908420);
	TVD_WRITE32(tvd_base[channel_id] + 0x5C4, 0x1A191515);
	TVD_WRITE32(tvd_base[channel_id] + 0x5C8, 0x06658819);
	TVD_WRITE32(tvd_base[channel_id] + 0x5CC, 0x72B91AB9);
	TVD_WRITE32(tvd_base[channel_id] + 0x5D0, 0x5A1640DB);
	TVD_WRITE32(tvd_base[channel_id] + 0x5D4, 0x55F401F4);
	TVD_WRITE32(tvd_base[channel_id] + 0x5D8, 0x100181FA);
	TVD_WRITE32(tvd_base[channel_id] + 0x5DC, 0x60506E60);
	TVD_WRITE32(tvd_base[channel_id] + 0x5E0, 0xA0A797A6);
	TVD_WRITE32(tvd_base[channel_id] + 0x5E4, 0xFAC9D1A0);
	TVD_WRITE32(tvd_base[channel_id] + 0x5E8, 0x7CB83BD4);
	TVD_WRITE32(tvd_base[channel_id] + 0x5EC, 0x80DD43A9);
	TVD_WRITE32(tvd_base[channel_id] + 0x5F0, 0x80554329);
	TVD_WRITE32(tvd_base[channel_id] + 0x5F4, 0x1748A14B);
	TVD_WRITE32(tvd_base[channel_id] + 0x5F8, 0x50E3C67F);

	//TVD_WRITE32(0x5FC,0x70D70480);

	TVD_WRITE32(tvd_base[channel_id] + 0x5FC, 0x71000480);

	TVD_WRITE32(tvd_base[channel_id] + 0x600, 0x3D34035F);
	TVD_WRITE32(tvd_base[channel_id] + 0x604, 0xF9228320);
	TVD_WRITE32(tvd_base[channel_id] + 0x608, 0x32201501);
	TVD_WRITE32(tvd_base[channel_id] + 0x60C, 0x5985A833);
	TVD_WRITE32(tvd_base[channel_id] + 0x610, 0x000028AD);
	TVD_WRITE32(tvd_base[channel_id] + 0x614, 0x55E86B73);
	TVD_WRITE32(tvd_base[channel_id] + 0x618, 0xE6CB2224);
	TVD_WRITE32(tvd_base[channel_id] + 0x61C, 0x84460080);
	TVD_WRITE32(tvd_base[channel_id] + 0x620, 0x00400000);
	TVD_WRITE32(tvd_base[channel_id] + 0x624, 0x7F084330);
	TVD_WRITE32(tvd_base[channel_id] + 0x628, 0x08880001);
	TVD_WRITE32(tvd_base[channel_id] + 0x62C, 0xBF488488);
	TVD_WRITE32(tvd_base[channel_id] + 0x630, 0x0119812C);
	TVD_WRITE32(tvd_base[channel_id] + 0x634, 0x01000844);
	TVD_WRITE32(tvd_base[channel_id] + 0x638, 0x88020818);
	TVD_WRITE32(tvd_base[channel_id] + 0x63C, 0x0FF0AA1F);

	//TVD_WRITE32(0x640,0x000B101F);
	TVD_WRITE32(tvd_base[channel_id] + 0x640, 0x180B101F);
	TVD_WRITE32(tvd_base[channel_id] + 0x640, 0x000B101F);

	TVD_WRITE32(tvd_base[channel_id] + 0x644, TVD_READ32(tvd_base[channel_id] + 0x644) | (0x1<<27));
	TVD_WRITE32(tvd_base[channel_id] + 0x648, 0x0A645AFB);
	//TVD_WRITE32(0x64C,0x25030200);
	TVD_WRITE32(tvd_base[channel_id] + 0x64C,(TVD_READ32(tvd_base[channel_id] + 0x64C)&(0x1<<4)) | 0x25030200); 
	TVD_WRITE32(tvd_base[channel_id] + 0x650, 0x02F70000); 
	TVD_WRITE32(tvd_base[channel_id] + 0x654, 0x27030005); 
	TVD_WRITE32(tvd_base[channel_id] + 0x658, 0x01A60A0D); 
	TVD_WRITE32(tvd_base[channel_id] + 0x65C, 0x01FC1500); 
	TVD_WRITE32(tvd_base[channel_id] + 0x660, 0x0111B106); 
	TVD_WRITE32(tvd_base[channel_id] + 0x664, 0x38E3200B); 
	TVD_WRITE32(tvd_base[channel_id] + 0x668, 0x0A074A36); 
	TVD_WRITE32(tvd_base[channel_id] + 0x66C, 0x0A074A36); 
	TVD_WRITE32(tvd_base[channel_id] + 0x670, 0x0000CCBD); 
	TVD_WRITE32(tvd_base[channel_id] + 0x674, 0x00000004); 
	TVD_WRITE32(tvd_base[channel_id] + 0x678, 0xCB656500);
	TVD_WRITE32(tvd_base[channel_id] + 0x67C, 0x38081808); 
	TVD_WRITE32(tvd_base[channel_id] + 0x680, 0x090A1E05);
	TVD_WRITE32(tvd_base[channel_id] + 0x684, 0x64180810);
	TVD_WRITE32(tvd_base[channel_id] + 0x688, 0x64AA321E);
	TVD_WRITE32(tvd_base[channel_id] + 0x68C, 0x32107023);
	TVD_WRITE32(tvd_base[channel_id] + 0x690, 0x0E013F85);
	TVD_WRITE32(tvd_base[channel_id] + 0x694, 0x0000B910);
	TVD_WRITE32(tvd_base[channel_id] + 0x698, 0x0064C800);
	TVD_WRITE32(tvd_base[channel_id] + 0x69C, 0x70680610);
	TVD_WRITE32(tvd_base[channel_id] + 0x6A0, 0x0000170F);
	TVD_WRITE32(tvd_base[channel_id] + 0x6A4, 0x12782880);
	TVD_WRITE32(tvd_base[channel_id] + 0x6A8, 0x60084084);
	TVD_WRITE32(tvd_base[channel_id] + 0x6AC, 0x1E0F1904);
	TVD_WRITE32(tvd_base[channel_id] + 0x6B0, 0x100F0344);
	TVD_WRITE32(tvd_base[channel_id] + 0x6B4, 0x3E505F00);
	TVD_WRITE32(tvd_base[channel_id] + 0x6B8, 0x04414444);
	TVD_WRITE32(tvd_base[channel_id] + 0x6BC, 0x00110333);
	TVD_WRITE32(tvd_base[channel_id] + 0x6C0, 0x00A404F3);
	TVD_WRITE32(tvd_base[channel_id] + 0x6C4, 0x45195030);
	TVD_WRITE32(tvd_base[channel_id] + 0x6C8, 0x0640285C);
	TVD_WRITE32(tvd_base[channel_id] + 0x6CC, 0x11600811);
	TVD_WRITE32(tvd_base[channel_id] + 0x6D0, 0x11111111);
	TVD_WRITE32(tvd_base[channel_id] + 0x6D4, 0x0008FC00);
	TVD_WRITE32(tvd_base[channel_id] + 0x6D8, 0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0x6DC, 0xC0031008);
	TVD_WRITE32(tvd_base[channel_id] + 0x6E0, 0x141420FF);
	TVD_WRITE32(tvd_base[channel_id] + 0x6E4, 0x32200060);
	TVD_WRITE32(tvd_base[channel_id] + 0x6E8, 0x20906000);
	TVD_WRITE32(tvd_base[channel_id] + 0x6EC, 0x0000000D);
	TVD_WRITE32(tvd_base[channel_id] + 0x6F0, 0x43174537);
	TVD_WRITE32(tvd_base[channel_id] + 0x6F4, 0x0045633C);
	TVD_WRITE32(tvd_base[channel_id] + 0x6F8, 0xC0001A50);
	TVD_WRITE32(tvd_base[channel_id] + 0x6FC, 0x0414050D);
	TVD_WRITE32(tvd_base[channel_id] + 0x700, 0x11A00020);
	TVD_WRITE32(tvd_base[channel_id] + 0x704, 0x64000450);
	TVD_WRITE32(tvd_base[channel_id] + 0x708, 0x410A0028);
	TVD_WRITE32(tvd_base[channel_id] + 0x70C, 0x00014455);
	TVD_WRITE32(tvd_base[channel_id] + 0x710, 0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0x714, 0x314859C8); 
	TVD_WRITE32(tvd_base[channel_id] + 0x718, 0x809E3001); 
	TVD_WRITE32(tvd_base[channel_id] + 0x71C, 0x08001020); 
	TVD_WRITE32(tvd_base[channel_id] + 0x720, 0x08081E03); 
	TVD_WRITE32(tvd_base[channel_id] + 0x724, 0x00100008); 
	TVD_WRITE32(tvd_base[channel_id] + 0x728, 0x14061210); 
	TVD_WRITE32(tvd_base[channel_id] + 0x72C, 0x10101022);
	TVD_WRITE32(tvd_base[channel_id] + 0x730, 0x20502010); 
	TVD_WRITE32(tvd_base[channel_id] + 0x734, 0x3311001A); 
	TVD_WRITE32(tvd_base[channel_id] + 0x738, 0x1111046A); 
	TVD_WRITE32(tvd_base[channel_id] + 0x73C, 0x8000080A); 
	TVD_WRITE32(tvd_base[channel_id] + 0x740, 0x01245678); 
	TVD_WRITE32(tvd_base[channel_id] + 0x744, 0x02345678); 
	TVD_WRITE32_MASK(tvd_base[channel_id] + 0x748,0x0008300A, 0xCFFFFFFF);
	TVD_WRITE32(tvd_base[channel_id] + 0x74C, 0x00022A34); 
	TVD_WRITE32(tvd_base[channel_id] + 0x750, 0x047C0558); 
	TVD_WRITE32(tvd_base[channel_id] + 0x754, 0x00880114); 
	TVD_WRITE32(tvd_base[channel_id] + 0x758, 0x00006A68); 
	TVD_WRITE32(tvd_base[channel_id] + 0x75C, 0x000783FF); 
	TVD_WRITE32(tvd_base[channel_id] + 0x760, 0x90000000); 
	TVD_WRITE32(tvd_base[channel_id] + 0x764, 0x000073FE);
	TVD_WRITE32(tvd_base[channel_id] + 0x768, 0x0F051818); 
	TVD_WRITE32(tvd_base[channel_id] + 0x76C, 0x00191014); 
	TVD_WRITE32(tvd_base[channel_id] + 0x770, 0x23645050); 
	TVD_WRITE32(tvd_base[channel_id] + 0x774, 0x64641432); 
	TVD_WRITE32(tvd_base[channel_id] + 0x778, 0x1E519696); 
	TVD_WRITE32(tvd_base[channel_id] + 0x77C, 0x10169696); 
	TVD_WRITE32(tvd_base[channel_id] + 0x780, 0x00504600); 
	TVD_WRITE32(tvd_base[channel_id] + 0x784, 0x821FFFFF); 
	TVD_WRITE32(tvd_base[channel_id] + 0x788, 0xFFFF3359);
	TVD_WRITE32(tvd_base[channel_id] + 0x78C, 0xFFFFFFFF); 
	TVD_WRITE32(tvd_base[channel_id] + 0x790, 0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0x794, 0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0x798, 0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0x79C, 0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0x7A0, 0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0x7A4, 0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0x7A8, 0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0x7AC, 0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0x7B0, 0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0x7B4, 0x100181FA);
	TVD_WRITE32(tvd_base[channel_id] + 0x7B8, 0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0x7BC, 0x00000000);

	TVD_WRITE32(tvd_base[channel_id] + 0x7D8, 0x020F42D0);
	TVD_WRITE32(tvd_base[channel_id] + 0x7DC, 0x00000008);
	TVD_WRITE32(tvd_base[channel_id] + 0x7E0, 0x0F0000A0);
	TVD_WRITE32(tvd_base[channel_id] + 0x7E4, 0x11000020);
	TVD_WRITE32(tvd_base[channel_id] + 0x7E8, 0xC000C000);
	TVD_WRITE32(tvd_base[channel_id] + 0x7EC, 0x00CC0C50);
	TVD_WRITE32(tvd_base[channel_id] + 0x7F0, 0x80008000);
	TVD_WRITE32(tvd_base[channel_id] + 0x7F4, 0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0x7F8, 0x80000000);
	TVD_WRITE32(tvd_base[channel_id] + 0x7FC, 0xD0090901);
	TVD_WRITE32(tvd_base[channel_id] + 0x800, 0x00060604);
	TVD_WRITE32(tvd_base[channel_id] + 0x804, 0x0B9000FF);
	TVD_WRITE32(tvd_base[channel_id] + 0x808, 0x001400FC);
	TVD_WRITE32(tvd_base[channel_id] + 0x80C, 0x005071AD);
	TVD_WRITE32(tvd_base[channel_id] + 0x810, 0xC3080050);
	TVD_WRITE32(tvd_base[channel_id] + 0x814, 0x0F120700);
	TVD_WRITE32(tvd_base[channel_id] + 0x818, 0x02100578);
	TVD_WRITE32(tvd_base[channel_id] + 0x81C, 0x65004020);
	TVD_WRITE32(tvd_base[channel_id] + 0x820, 0x0000F00F);
	TVD_WRITE32(tvd_base[channel_id] + 0x824, 0x14140F0F);
	TVD_WRITE32(tvd_base[channel_id] + 0x828, 0x0A0A0A0A);
	TVD_WRITE32(tvd_base[channel_id] + 0x82C, 0x77700000);
	TVD_WRITE32(tvd_base[channel_id] + 0x830, 0x00A0A000);
	TVD_WRITE32(tvd_base[channel_id] + 0x834, 0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0x838, 0x4400A00A);
	TVD_WRITE32(tvd_base[channel_id] + 0x83C, 0x14000000);
	TVD_WRITE32(tvd_base[channel_id] + 0x840, 0x00003010);
	TVD_WRITE32(tvd_base[channel_id] + 0x844, 0x00201007);
	TVD_WRITE32(tvd_base[channel_id] + 0x848, 0x00201007);
	TVD_WRITE32(tvd_base[channel_id] + 0x84C, 0x0700100A);
	TVD_WRITE32(tvd_base[channel_id] + 0x850, 0x00208858);
	TVD_WRITE32(tvd_base[channel_id] + 0x854, 0x14001010);
	TVD_WRITE32(tvd_base[channel_id] + 0x858, 0x00141414);
	TVD_WRITE32(tvd_base[channel_id] + 0x85C, 0x10141414);
	TVD_WRITE32(tvd_base[channel_id] + 0x860, 0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0x864, 0x00010000);
	TVD_WRITE32(tvd_base[channel_id] + 0x868, 0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0x86C, 0xC0400C0A);
	TVD_WRITE32(tvd_base[channel_id] + 0x870, 0x00020820);
	TVD_WRITE32(tvd_base[channel_id] + 0x874, 0xC0016C01);
	TVD_WRITE32(tvd_base[channel_id] + 0x878, 0x0089A022);
	TVD_WRITE32(tvd_base[channel_id] + 0x87C, 0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0x880, 0x00060604);
	TVD_WRITE32(tvd_base[channel_id] + 0x884, 0x000001FC);
	TVD_WRITE32(tvd_base[channel_id] + 0x888, 0x005071AD);
	TVD_WRITE32(tvd_base[channel_id] + 0x88C, 0x00002010);
	TVD_WRITE32(tvd_base[channel_id] + 0x890, 0x0008F060);
	TVD_WRITE32(tvd_base[channel_id] + 0x894, 0x0020A000);
	TVD_WRITE32(tvd_base[channel_id] + 0x898, 0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0x89C, 0x00606200);
	TVD_WRITE32(tvd_base[channel_id] + 0x8A0, 0xC000C000);
	TVD_WRITE32(tvd_base[channel_id] + 0x8A4, 0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0x8A8, 0x0000000F);
	TVD_WRITE32(tvd_base[channel_id] + 0x8AC, 0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0x8B0, 0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0x8B4, 0x00000078);
	TVD_WRITE32(tvd_base[channel_id] + 0x8B8, 0xC0016C01);
	TVD_WRITE32(tvd_base[channel_id] + 0x8BC, 0x6EC86EC8);
	TVD_WRITE32(tvd_base[channel_id] + 0x8C0, 0x67049804);
	TVD_WRITE32(tvd_base[channel_id] + 0x8C4, 0x120845A0);
	TVD_WRITE32(tvd_base[channel_id] + 0x8C8, 0x0185C501);
	TVD_WRITE32(tvd_base[channel_id] + 0x8CC, 0xC1CBA860);
	TVD_WRITE32(tvd_base[channel_id] + 0x8D0, 0x70000BB8);
	TVD_WRITE32(tvd_base[channel_id] + 0x8D4, 0x7F294A52);
	TVD_WRITE32(tvd_base[channel_id] + 0x8D8, 0x83C28F00);
	TVD_WRITE32(tvd_base[channel_id] + 0x8DC, 0x74D08610);
	TVD_WRITE32(tvd_base[channel_id] + 0x8E0, 0x195D1CCD);
	TVD_WRITE32(tvd_base[channel_id] + 0x8E4, 0x413B0231);
	TVD_WRITE32(tvd_base[channel_id] + 0x8E8, 0x0440283B);
	TVD_WRITE32(tvd_base[channel_id] + 0x8EC, 0xF403DCF7);
	TVD_WRITE32(tvd_base[channel_id] + 0x8F0, 0x532714C2);
	TVD_WRITE32(tvd_base[channel_id] + 0x8F4, 0x67184854);
	TVD_WRITE32(tvd_base[channel_id] + 0x8F8, 0x80000000);
	TVD_WRITE32(tvd_base[channel_id] + 0x8FC, 0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0x900, 0x62F32005);
	TVD_WRITE32(tvd_base[channel_id] + 0x904, 0x120855A0);
	TVD_WRITE32(tvd_base[channel_id] + 0x908, 0x0185D0B9);
	TVD_WRITE32(tvd_base[channel_id] + 0x90C, 0xC1CAA800);
	TVD_WRITE32(tvd_base[channel_id] + 0x910, 0x70000BB8);
	TVD_WRITE32(tvd_base[channel_id] + 0x914, 0x7F29CE73);
	TVD_WRITE32(tvd_base[channel_id] + 0x918, 0x83C28F00);
	TVD_WRITE32(tvd_base[channel_id] + 0x91C, 0x74D08610);
	TVD_WRITE32(tvd_base[channel_id] + 0x920, 0x195D1CCD);
	TVD_WRITE32(tvd_base[channel_id] + 0x924, 0x413B0252);
	TVD_WRITE32(tvd_base[channel_id] + 0x928, 0x0440283B);
	TVD_WRITE32(tvd_base[channel_id] + 0x92C, 0xF403DCF7);
	TVD_WRITE32(tvd_base[channel_id] + 0x930, 0x532714C2);
	TVD_WRITE32(tvd_base[channel_id] + 0x934, 0x87184814);
	TVD_WRITE32(tvd_base[channel_id] + 0x938, 0x08000800);
	TVD_WRITE32(tvd_base[channel_id] + 0x93C, 0x00000800);
	TVD_WRITE32(tvd_base[channel_id] + 0x940, 0x10281400);
	TVD_WRITE32(tvd_base[channel_id] + 0x944, 0x04D0BB40);
	TVD_WRITE32(tvd_base[channel_id] + 0x948, 0x10281400);
	TVD_WRITE32(tvd_base[channel_id] + 0x94C, 0x04D0BB40);
	TVD_WRITE32(tvd_base[channel_id] + 0x950, 0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0x954, 0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0x958, 0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0x95C, 0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0x960, 0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0x964, 0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0x968, 0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0x96C, 0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0x970, 0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0x974, 0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0x978, 0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0x97C, 0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0x980, 0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0x984, 0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0x988, 0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0x98C, 0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0x990, 0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0x994, 0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0x998, 0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0x99C, 0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0x9A0, 0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0x9A4, 0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0x9A8, 0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0x9AC, 0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0x9B0, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x9B4, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x9B8, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x9BC, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x9C0, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x9C4, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x9C8, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x9CC, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x9D0, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x9D4, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x9D8, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x9DC, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x9E0, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x9E4, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x9E8, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x9EC, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x9F0, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x9F4, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x9F8, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x9FC, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0xA00, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0xA04, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0xA08, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0xA0C, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0xA10, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0xA14, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0xA18, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0xA1C, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0xA20, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0xA24, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0xA28, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0xA2C, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0xA30, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0xA34, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0xA38, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0xA3C, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0xA40, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0xA44, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0xA48, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0xA4C, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0xA50, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0xA54, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0xA58, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0xA5C, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0xA60, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0xA64, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0xA68, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0xA6C, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0xA70, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0xA74, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0xA78, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0xA7C, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0xA80, 0x0FF3FCFF);  
	TVD_WRITE32(tvd_base[channel_id] + 0xA84, 0x80008000);  
	TVD_WRITE32(tvd_base[channel_id] + 0xA88, 0x80000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0xA8C, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0xA90, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0xA94, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0xA98, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0xA9C, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0xAA0, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0xAA4, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0xAA8, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0xAAC, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0xAB0, 0x22F08C03);  
	TVD_WRITE32(tvd_base[channel_id] + 0xAB4, 0x8F0070D4);  
	TVD_WRITE32(tvd_base[channel_id] + 0xAB8, 0x42400800);  
	TVD_WRITE32(tvd_base[channel_id] + 0xABC, 0x82000820);  
	TVD_WRITE32(tvd_base[channel_id] + 0x5aC, 0x000010C0);  //bit 31
	TVD_WRITE32(tvd_base[channel_id] + 0x59C, 0x21E2c0B2); //H start
	TVD_WRITE32(tvd_base[channel_id] + 0x5c4, 0x18181617);  //v start
	
	return;
}

void vNTSC443_2D_Setting(TVD_CHANNEL_ID channel_id)
{
	TVD_WRITE32(tvd_base[channel_id] + 0x410, 0x00818000);               
	TVD_WRITE32(tvd_base[channel_id] + 0x414, 0xFF080020);               
	TVD_WRITE32(tvd_base[channel_id] + 0x418, 0x10000000);               
	TVD_WRITE32(tvd_base[channel_id] + 0x41C, 0x12400010);               
	TVD_WRITE32(tvd_base[channel_id] + 0x420, 0x22788207);               
	TVD_WRITE32(tvd_base[channel_id] + 0x424, 0x000000FF);               
	TVD_WRITE32(tvd_base[channel_id] + 0x428, 0xFFFFFFFF);               
	TVD_WRITE32(tvd_base[channel_id] + 0x42C, 0x00000000);               
	TVD_WRITE32(tvd_base[channel_id] + 0x430, 0x00000000);               
	                                         
	                                         
	TVD_WRITE32(tvd_base[channel_id] + 0x434, 0xC4D00101);               
	TVD_WRITE32(tvd_base[channel_id] + 0x438, 0xB801404A);               
	TVD_WRITE32(tvd_base[channel_id] + 0x43C, 0x00800801);               
	TVD_WRITE32(tvd_base[channel_id] + 0x440, 0x00D01001);               
	TVD_WRITE32(tvd_base[channel_id] + 0x444, 0x1E000000);               
	TVD_WRITE32(tvd_base[channel_id] + 0x448, 0x80000114);               
	TVD_WRITE32(tvd_base[channel_id] + 0x44C, 0x2AAA8103);               
	TVD_WRITE32(tvd_base[channel_id] + 0x450, 0xF0000100);               
	TVD_WRITE32(tvd_base[channel_id] + 0x454, 0x1E000000);               
	TVD_WRITE32(tvd_base[channel_id] + 0x458, 0x80000114);               
	TVD_WRITE32(tvd_base[channel_id] + 0x45C, 0x2AAA8103);               
	TVD_WRITE32(tvd_base[channel_id] + 0x460, 0x47B00000);               
	TVD_WRITE32(tvd_base[channel_id] + 0x464, 0x1E000000);               
	TVD_WRITE32(tvd_base[channel_id] + 0x468, 0x80000114);               
	TVD_WRITE32(tvd_base[channel_id] + 0x46C, 0x2AAA8103);               
	TVD_WRITE32(tvd_base[channel_id] + 0x470, 0x1100001B);               
	TVD_WRITE32(tvd_base[channel_id] + 0x474, 0xAAA04EAB);               
	TVD_WRITE32(tvd_base[channel_id] + 0x478, 0x0888AAAA);               
	TVD_WRITE32(tvd_base[channel_id] + 0x47C, 0x00050000);               
	TVD_WRITE32(tvd_base[channel_id] + 0x480, 0x003E313D);               
	TVD_WRITE32(tvd_base[channel_id] + 0x484, 0x100F0480);               
	TVD_WRITE32(tvd_base[channel_id] + 0x488, 0x00000000);               
	TVD_WRITE32(tvd_base[channel_id] + 0x48C, 0x0200022D);               
	TVD_WRITE32(tvd_base[channel_id] + 0x490, 0x0AFFC588);               
	TVD_WRITE32(tvd_base[channel_id] + 0x494, 0x08000000);               
	TVD_WRITE32(tvd_base[channel_id] + 0x498, 0x00000000);               
	TVD_WRITE32(tvd_base[channel_id] + 0x49C, 0x0009C000);               
	TVD_WRITE32(tvd_base[channel_id] + 0x4A0, 0x00090000);               
	TVD_WRITE32(tvd_base[channel_id] + 0x4A4, 0x00000000);               
	TVD_WRITE32(tvd_base[channel_id] + 0x4A8, 0x00000000);               
	TVD_WRITE32(tvd_base[channel_id] + 0x4AC, 0x00000000);               
	TVD_WRITE32(tvd_base[channel_id] + 0x4B0, 0x00000000);               
	TVD_WRITE32(tvd_base[channel_id] + 0x4B4, 0x00000000);               
	TVD_WRITE32(tvd_base[channel_id] + 0x4B8, 0x00000000);               
	TVD_WRITE32(tvd_base[channel_id] + 0x4BC, 0x00000000);               
	TVD_WRITE32(tvd_base[channel_id] + 0x4C0, 0x51515C55);               
	TVD_WRITE32(tvd_base[channel_id] + 0x4C4, 0x04557845);               
	TVD_WRITE32(tvd_base[channel_id] + 0x4C8, 0x27F7595E);               
	TVD_WRITE32(tvd_base[channel_id] + 0x4CC, 0x8F340380);               
	TVD_WRITE32(tvd_base[channel_id] + 0x4D0, 0x158C0F80);               
	TVD_WRITE32(tvd_base[channel_id] + 0x4D4, 0x26144040);               
	TVD_WRITE32(tvd_base[channel_id] + 0x4D8, 0x0A00F600);               
	TVD_WRITE32(tvd_base[channel_id] + 0x4DC, 0xF41F0800);               
	                                     
	TVD_WRITE32(tvd_base[channel_id] + 0x4E0, 0x13178080);               
	                                     
	TVD_WRITE32(tvd_base[channel_id] + 0x4E4, 0x8AA7075E);               
	TVD_WRITE32(tvd_base[channel_id] + 0x4E8, 0xF08400A0);               
	TVD_WRITE32(tvd_base[channel_id] + 0x4EC, 0x08180C40);               
	TVD_WRITE32(tvd_base[channel_id] + 0x4F0, 0x2008C2C9);               
	TVD_WRITE32(tvd_base[channel_id] + 0x4F4, 0x7180001F);               
	TVD_WRITE32(tvd_base[channel_id] + 0x4F8, 0x8000306C);               
	TVD_WRITE32(tvd_base[channel_id] + 0x4FC, 0x420C33A0);               
	TVD_WRITE32(tvd_base[channel_id] + 0x500, 0x6838FA27);               
	TVD_WRITE32(tvd_base[channel_id] + 0x504, 0x0DFA2090);               
	TVD_WRITE32(tvd_base[channel_id] + 0x508, 0x08E06B40);               
	TVD_WRITE32(tvd_base[channel_id] + 0x50C, 0x83608016);               
	TVD_WRITE32(tvd_base[channel_id] + 0x510, 0x40204080);               
	TVD_WRITE32(tvd_base[channel_id] + 0x514, 0x80326485);               
	TVD_WRITE32(tvd_base[channel_id] + 0x518, 0x11A0A04C);               
	TVD_WRITE32(tvd_base[channel_id] + 0x51C, 0x8040A0C0);               
	TVD_WRITE32(tvd_base[channel_id] + 0x520, 0x7E35AF80);               
	TVD_WRITE32(tvd_base[channel_id] + 0x524, 0x0408F840);               
	TVD_WRITE32(tvd_base[channel_id] + 0x528, 0x5404E030);               
	TVD_WRITE32(tvd_base[channel_id] + 0x52C, 0x30900AA0);               
	TVD_WRITE32(tvd_base[channel_id] + 0x530, 0x0B410B09);               
	TVD_WRITE32(tvd_base[channel_id] + 0x534, 0xF43E40F0);               
	TVD_WRITE32(tvd_base[channel_id] + 0x538, 0x452414F0);               
	TVD_WRITE32(tvd_base[channel_id] + 0x53C, 0x55C29325);               
	TVD_WRITE32(tvd_base[channel_id] + 0x540, 0x30173E63);               
	TVD_WRITE32(tvd_base[channel_id] + 0x544, 0x21A41725);               
	TVD_WRITE32(tvd_base[channel_id] + 0x548, 0x30B080FE);               
	TVD_WRITE32(tvd_base[channel_id] + 0x54C, 0x1245482B);               
	TVD_WRITE32(tvd_base[channel_id] + 0x550, 0xB4000023);               
	TVD_WRITE32(tvd_base[channel_id] + 0x554, 0x04AE0318);               
	TVD_WRITE32(tvd_base[channel_id] + 0x558, 0x106441D0);               
	TVD_WRITE32(tvd_base[channel_id] + 0x55C, 0x57856973);               
	TVD_WRITE32(tvd_base[channel_id] + 0x560, 0x29E518EF);               
	TVD_WRITE32(tvd_base[channel_id] + 0x564, 0x2010573D);               
	TVD_WRITE32(tvd_base[channel_id] + 0x568, 0x204CDEC9);               
	TVD_WRITE32(tvd_base[channel_id] + 0x56C, 0x204A9881);               
	TVD_WRITE32(tvd_base[channel_id] + 0x570, 0xFF29DEC5);               
	TVD_WRITE32(tvd_base[channel_id] + 0x574, 0xCA8B0F00);               
	TVD_WRITE32(tvd_base[channel_id] + 0x578, 0x797A8E61);               
	TVD_WRITE32(tvd_base[channel_id] + 0x57C, 0x420FA8A1);               
	TVD_WRITE32(tvd_base[channel_id] + 0x580, 0x208181D4);               
	TVD_WRITE32(tvd_base[channel_id] + 0x584, 0x28AF88CA);               
	TVD_WRITE32(tvd_base[channel_id] + 0x588, 0xFC844040);               
	TVD_WRITE32(tvd_base[channel_id] + 0x58C, 0x2854C531);               
	TVD_WRITE32(tvd_base[channel_id] + 0x590, 0x0206E61A);               
	TVD_WRITE32(tvd_base[channel_id] + 0x594, 0x4840310E);               
	TVD_WRITE32(tvd_base[channel_id] + 0x598, 0x21594088);               
	TVD_WRITE32(tvd_base[channel_id] + 0x59C, 0x29E2C0A5);               
	TVD_WRITE32(tvd_base[channel_id] + 0x5A0, 0x44800C80);               
	TVD_WRITE32(tvd_base[channel_id] + 0x5A4, 0x20010000);               
	TVD_WRITE32(tvd_base[channel_id] + 0x5A8, 0x0A4BC6C1);               
	TVD_WRITE32(tvd_base[channel_id] + 0x5AC, 0x400010C0);               
	TVD_WRITE32(tvd_base[channel_id] + 0x5B0, 0x20180400);               
	TVD_WRITE32(tvd_base[channel_id] + 0x5B4, 0x00F9BC00);               
	TVD_WRITE32(tvd_base[channel_id] + 0x5B8, 0x20002004);               
	TVD_WRITE32(tvd_base[channel_id] + 0x5BC, 0x17E0114F);               
	TVD_WRITE32(tvd_base[channel_id] + 0x5C0, 0x1F908420);               
	TVD_WRITE32(tvd_base[channel_id] + 0x5C4, 0x1A191515);               
	TVD_WRITE32(tvd_base[channel_id] + 0x5C8, 0x06658819);               
	TVD_WRITE32(tvd_base[channel_id] + 0x5CC, 0x72B91AB9);               
	TVD_WRITE32(tvd_base[channel_id] + 0x5D0, 0x5A1640DB);               
	TVD_WRITE32(tvd_base[channel_id] + 0x5D4, 0x55F401F4);               
	TVD_WRITE32(tvd_base[channel_id] + 0x5D8, 0x100181FA);               
	TVD_WRITE32(tvd_base[channel_id] + 0x5DC, 0x60506E60);               
	TVD_WRITE32(tvd_base[channel_id] + 0x5E0, 0xA0A797A6);               
	TVD_WRITE32(tvd_base[channel_id] + 0x5E4, 0xFAC9D1A0);               
	TVD_WRITE32(tvd_base[channel_id] + 0x5E8, 0x7CB83BD4);               
	TVD_WRITE32(tvd_base[channel_id] + 0x5EC, 0x80DD43A9);               
	TVD_WRITE32(tvd_base[channel_id] + 0x5F0, 0x80554329);               
	TVD_WRITE32(tvd_base[channel_id] + 0x5F4, 0x1748A14B);               
	TVD_WRITE32(tvd_base[channel_id] + 0x5F8, 0x50E3C67F);               
	TVD_WRITE32(tvd_base[channel_id] + 0x5FC, 0x71000480);               
	TVD_WRITE32(tvd_base[channel_id] + 0x600, 0x3D34035F);               
	TVD_WRITE32(tvd_base[channel_id] + 0x604, 0xF9228320);               
	TVD_WRITE32(tvd_base[channel_id] + 0x608, 0x32201501);               
	TVD_WRITE32(tvd_base[channel_id] + 0x60C, 0x5985A833);               
	TVD_WRITE32(tvd_base[channel_id] + 0x610, 0x000028AD);               
	TVD_WRITE32(tvd_base[channel_id] + 0x614, 0x55E86B73);               
	TVD_WRITE32(tvd_base[channel_id] + 0x618, 0xE6CB2224);               
	TVD_WRITE32(tvd_base[channel_id] + 0x61C, 0x84460080);               
	TVD_WRITE32(tvd_base[channel_id] + 0x620, 0x00400000);               
	TVD_WRITE32(tvd_base[channel_id] + 0x624, 0x7F084330);               
	TVD_WRITE32(tvd_base[channel_id] + 0x628, 0x08880001);               
	TVD_WRITE32(tvd_base[channel_id] + 0x62C, 0xBF488488);               
	TVD_WRITE32(tvd_base[channel_id] + 0x630, 0x0119812C);               
	TVD_WRITE32(tvd_base[channel_id] + 0x634, 0x01000844);               
	TVD_WRITE32(tvd_base[channel_id] + 0x638, 0x88020818);               
	TVD_WRITE32(tvd_base[channel_id] + 0x63C, 0x0FF0AA1F);               
	TVD_WRITE32(tvd_base[channel_id] + 0x640, 0x000B101F); 
	TVD_WRITE32(tvd_base[channel_id] + 0x644, 0x81A2F600);               
	TVD_WRITE32(tvd_base[channel_id] + 0x648, 0x0A045AFB); 
	TVD_WRITE32(tvd_base[channel_id] + 0x64C, 0x25030213); 
	TVD_WRITE32(tvd_base[channel_id] + 0x650, 0x00000000); 
	TVD_WRITE32(tvd_base[channel_id] + 0x654, 0x2F030438); 
	TVD_WRITE32(tvd_base[channel_id] + 0x658, 0x021E1E74); 
	TVD_WRITE32(tvd_base[channel_id] + 0x65C, 0x02641900); 
	TVD_WRITE32(tvd_base[channel_id] + 0x660, 0x01151139); 
	TVD_WRITE32(tvd_base[channel_id] + 0x664, 0x46F3C00B); 
	TVD_WRITE32(tvd_base[channel_id] + 0x668, 0x0A09052E); 
	TVD_WRITE32(tvd_base[channel_id] + 0x66C, 0x0A09052E); 
	TVD_WRITE32(tvd_base[channel_id] + 0x670, 0x00000000); 
	TVD_WRITE32(tvd_base[channel_id] + 0x674, 0x00000004); 
	TVD_WRITE32(tvd_base[channel_id] + 0x678, 0x393A393A);               
	TVD_WRITE32(tvd_base[channel_id] + 0x67C, 0x38081808); 
	TVD_WRITE32(tvd_base[channel_id] + 0x680, 0x090A1E00);               
	TVD_WRITE32(tvd_base[channel_id] + 0x684, 0x64180810);               
	TVD_WRITE32(tvd_base[channel_id] + 0x688, 0x64AA321E);               
	TVD_WRITE32(tvd_base[channel_id] + 0x68C, 0x32107023);               
	TVD_WRITE32(tvd_base[channel_id] + 0x690, 0x0F013F85);               
	TVD_WRITE32(tvd_base[channel_id] + 0x694, 0x0000B910);               
	TVD_WRITE32(tvd_base[channel_id] + 0x698, 0x0064C800);               
	TVD_WRITE32(tvd_base[channel_id] + 0x69C, 0x00480610);               
	TVD_WRITE32(tvd_base[channel_id] + 0x6A0, 0x0000900F);               
	TVD_WRITE32(tvd_base[channel_id] + 0x6A4, 0x10782880);               
	TVD_WRITE32(tvd_base[channel_id] + 0x6A8, 0x60084084);               
	TVD_WRITE32(tvd_base[channel_id] + 0x6AC, 0x1E0F1904);               
	TVD_WRITE32(tvd_base[channel_id] + 0x6B0, 0x10080344);               
	TVD_WRITE32(tvd_base[channel_id] + 0x6B4, 0x3E505F07);               
	TVD_WRITE32(tvd_base[channel_id] + 0x6B8, 0x04414444);               
	TVD_WRITE32(tvd_base[channel_id] + 0x6BC, 0x00110333);               
	TVD_WRITE32(tvd_base[channel_id] + 0x6C0, 0x00A404F3);               
	TVD_WRITE32(tvd_base[channel_id] + 0x6C4, 0x45195030);               
	TVD_WRITE32(tvd_base[channel_id] + 0x6C8, 0x0640285C);               
	TVD_WRITE32(tvd_base[channel_id] + 0x6CC, 0x11600811);               
	TVD_WRITE32(tvd_base[channel_id] + 0x6D0, 0x11111111);               
	TVD_WRITE32(tvd_base[channel_id] + 0x6D4, 0x00089C00);               
	TVD_WRITE32(tvd_base[channel_id] + 0x6D8, 0x00000000);               
	TVD_WRITE32(tvd_base[channel_id] + 0x6DC, 0x00071004);               
	TVD_WRITE32(tvd_base[channel_id] + 0x6E0, 0x141420FF);               
	TVD_WRITE32(tvd_base[channel_id] + 0x6E4, 0x32200060);               
	TVD_WRITE32(tvd_base[channel_id] + 0x6E8, 0x20A06000);               
	TVD_WRITE32(tvd_base[channel_id] + 0x6EC, 0x00000014);               
	TVD_WRITE32(tvd_base[channel_id] + 0x6F0, 0x43174537);               
	TVD_WRITE32(tvd_base[channel_id] + 0x6F4, 0x0045713C);               
	TVD_WRITE32(tvd_base[channel_id] + 0x6F8, 0xC0001A50);               
	TVD_WRITE32(tvd_base[channel_id] + 0x6FC, 0x4414050D);               
	TVD_WRITE32(tvd_base[channel_id] + 0x700, 0x11A00020);               
	TVD_WRITE32(tvd_base[channel_id] + 0x704, 0x64000450);               
	TVD_WRITE32(tvd_base[channel_id] + 0x708, 0x410A0028);               
	TVD_WRITE32(tvd_base[channel_id] + 0x70C, 0x00014455);               
	TVD_WRITE32(tvd_base[channel_id] + 0x710, 0x00000000);               
	TVD_WRITE32(tvd_base[channel_id] + 0x714, 0x314059C0); 
	TVD_WRITE32(tvd_base[channel_id] + 0x718, 0x801E0000); 
	TVD_WRITE32(tvd_base[channel_id] + 0x71C, 0x08001120); 
	TVD_WRITE32(tvd_base[channel_id] + 0x720, 0x08081E03); 
	TVD_WRITE32(tvd_base[channel_id] + 0x724, 0x00100008); 
	TVD_WRITE32(tvd_base[channel_id] + 0x728, 0x07060A10); 
	TVD_WRITE32(tvd_base[channel_id] + 0x72C, 0x10101022); 
	TVD_WRITE32(tvd_base[channel_id] + 0x730, 0x20B09000); 
	TVD_WRITE32(tvd_base[channel_id] + 0x734, 0x22110A0E); 
	TVD_WRITE32(tvd_base[channel_id] + 0x738, 0x1111043A); 
	TVD_WRITE32(tvd_base[channel_id] + 0x73C, 0x8000080A); 
	TVD_WRITE32(tvd_base[channel_id] + 0x740, 0x12345678); 
	TVD_WRITE32(tvd_base[channel_id] + 0x744, 0x02345678); 
	TVD_WRITE32_MASK(tvd_base[channel_id] + 0x748,0x00080014, 0xCFFFFFFF);
	TVD_WRITE32(tvd_base[channel_id] + 0x74C, 0x00020A34); 
	TVD_WRITE32(tvd_base[channel_id] + 0x750, 0x047C0558); 
	TVD_WRITE32(tvd_base[channel_id] + 0x754, 0x0088011A); 
	TVD_WRITE32(tvd_base[channel_id] + 0x758, 0x00009666); 
	TVD_WRITE32(tvd_base[channel_id] + 0x75C, 0x000781FF); 
	TVD_WRITE32(tvd_base[channel_id] + 0x760, 0x90000000); 
	TVD_WRITE32(tvd_base[channel_id] + 0x764, 0x00000000);               
	TVD_WRITE32(tvd_base[channel_id] + 0x768, 0x0F051818); 
	TVD_WRITE32(tvd_base[channel_id] + 0x76C, 0x00191014); 
	TVD_WRITE32(tvd_base[channel_id] + 0x770, 0x23645050); 
	TVD_WRITE32(tvd_base[channel_id] + 0x774, 0x64641432); 
	TVD_WRITE32(tvd_base[channel_id] + 0x778, 0x1E519696); 
	TVD_WRITE32(tvd_base[channel_id] + 0x77C, 0x10169696); 
	TVD_WRITE32(tvd_base[channel_id] + 0x780, 0x053C501E); 
	TVD_WRITE32(tvd_base[channel_id] + 0x784, 0x820A325A); 
	TVD_WRITE32(tvd_base[channel_id] + 0x788, 0x6BFF3359);               
	TVD_WRITE32(tvd_base[channel_id] + 0x78C, 0xFFFFFFFF); 
	TVD_WRITE32(tvd_base[channel_id] + 0x790, 0x00000000);               
	TVD_WRITE32(tvd_base[channel_id] + 0x794, 0x00000000);               
	TVD_WRITE32(tvd_base[channel_id] + 0x798, 0x00000000);               
	TVD_WRITE32(tvd_base[channel_id] + 0x79C, 0x00000000);               
	TVD_WRITE32(tvd_base[channel_id] + 0x7A0, 0x00000000);               
	TVD_WRITE32(tvd_base[channel_id] + 0x7A4, 0x00000000);               
	TVD_WRITE32(tvd_base[channel_id] + 0x7A8, 0x00000000);               
	TVD_WRITE32(tvd_base[channel_id] + 0x7AC, 0x00000000);               
	TVD_WRITE32(tvd_base[channel_id] + 0x7B0, 0x00000000);               
	TVD_WRITE32(tvd_base[channel_id] + 0x7B4, 0x100181FA);               
	TVD_WRITE32(tvd_base[channel_id] + 0x7B8, 0x00000000);               
	TVD_WRITE32(tvd_base[channel_id] + 0x7BC, 0x00000000);

	TVD_WRITE32(tvd_base[channel_id] + 0x7D8, 0x020F42D0);               
	TVD_WRITE32(tvd_base[channel_id] + 0x7DC, 0x00000008);               
	TVD_WRITE32(tvd_base[channel_id] + 0x7E0, 0x0F0000A0);               
	TVD_WRITE32(tvd_base[channel_id] + 0x7E4, 0x11000020);               
	TVD_WRITE32(tvd_base[channel_id] + 0x7E8, 0xC000C000);               
	TVD_WRITE32(tvd_base[channel_id] + 0x7EC, 0x00CC0C50);               
	TVD_WRITE32(tvd_base[channel_id] + 0x7F0, 0x80008000);               
	TVD_WRITE32(tvd_base[channel_id] + 0x7F4, 0x00000000);               
	TVD_WRITE32(tvd_base[channel_id] + 0x7F8, 0x80000000);               
	TVD_WRITE32(tvd_base[channel_id] + 0x7FC, 0xD0090901);               
	TVD_WRITE32(tvd_base[channel_id] + 0x800, 0x00060604);               
	TVD_WRITE32(tvd_base[channel_id] + 0x804, 0x0B9000FF);               
	TVD_WRITE32(tvd_base[channel_id] + 0x808, 0x001400FC);               
	TVD_WRITE32(tvd_base[channel_id] + 0x80C, 0x005071AD);               
	TVD_WRITE32(tvd_base[channel_id] + 0x810, 0xC3080050);               
	TVD_WRITE32(tvd_base[channel_id] + 0x814, 0x0F120700);               
	TVD_WRITE32(tvd_base[channel_id] + 0x818, 0x02100578);               
	TVD_WRITE32(tvd_base[channel_id] + 0x81C, 0x65004020);               
	TVD_WRITE32(tvd_base[channel_id] + 0x820, 0x0000F00F);               
	TVD_WRITE32(tvd_base[channel_id] + 0x824, 0x14140F0F);               
	TVD_WRITE32(tvd_base[channel_id] + 0x828, 0x0A0A0A0A);               
	TVD_WRITE32(tvd_base[channel_id] + 0x82C, 0x77700000);               
	TVD_WRITE32(tvd_base[channel_id] + 0x830, 0x00A0A000);               
	TVD_WRITE32(tvd_base[channel_id] + 0x834, 0x00000000);               
	TVD_WRITE32(tvd_base[channel_id] + 0x838, 0x4400A00A);               
	TVD_WRITE32(tvd_base[channel_id] + 0x83C, 0x14000000);               
	TVD_WRITE32(tvd_base[channel_id] + 0x840, 0x00003010);               
	TVD_WRITE32(tvd_base[channel_id] + 0x844, 0x00201007);               
	TVD_WRITE32(tvd_base[channel_id] + 0x848, 0x00201007);               
	TVD_WRITE32(tvd_base[channel_id] + 0x84C, 0x0700100A);               
	TVD_WRITE32(tvd_base[channel_id] + 0x850, 0x00208858);               
	TVD_WRITE32(tvd_base[channel_id] + 0x854, 0x14001010);               
	TVD_WRITE32(tvd_base[channel_id] + 0x858, 0x00141414);               
	TVD_WRITE32(tvd_base[channel_id] + 0x85C, 0x10141414);               
	TVD_WRITE32(tvd_base[channel_id] + 0x860, 0x00000000);               
	TVD_WRITE32(tvd_base[channel_id] + 0x864, 0x00010000);               
	TVD_WRITE32(tvd_base[channel_id] + 0x868, 0x00000000);               
	TVD_WRITE32(tvd_base[channel_id] + 0x86C, 0xC0400C0A);               
	TVD_WRITE32(tvd_base[channel_id] + 0x870, 0x00020820);               
	TVD_WRITE32(tvd_base[channel_id] + 0x874, 0xC0016C01);               
	TVD_WRITE32(tvd_base[channel_id] + 0x878, 0x0089A022);               
	TVD_WRITE32(tvd_base[channel_id] + 0x87C, 0x00000000);               
	TVD_WRITE32(tvd_base[channel_id] + 0x880, 0x00060604);               
	TVD_WRITE32(tvd_base[channel_id] + 0x884, 0x000001FC);               
	TVD_WRITE32(tvd_base[channel_id] + 0x888, 0x005071AD);               
	TVD_WRITE32(tvd_base[channel_id] + 0x88C, 0x00002010);               
	TVD_WRITE32(tvd_base[channel_id] + 0x890, 0x0008F060);               
	TVD_WRITE32(tvd_base[channel_id] + 0x894, 0x0020A000);               
	TVD_WRITE32(tvd_base[channel_id] + 0x898, 0x00000000);               
	TVD_WRITE32(tvd_base[channel_id] + 0x89C, 0x00606200);               
	TVD_WRITE32(tvd_base[channel_id] + 0x8A0, 0xC000C000);               
	TVD_WRITE32(tvd_base[channel_id] + 0x8A4, 0x00000000);               
	TVD_WRITE32(tvd_base[channel_id] + 0x8A8, 0x0000000F);               
	TVD_WRITE32(tvd_base[channel_id] + 0x8AC, 0x00000000);               
	TVD_WRITE32(tvd_base[channel_id] + 0x8B0, 0x00000000);               
	TVD_WRITE32(tvd_base[channel_id] + 0x8B4, 0x00000078);               
	TVD_WRITE32(tvd_base[channel_id] + 0x8B8, 0xC0016C01);               
	TVD_WRITE32(tvd_base[channel_id] + 0x8BC, 0x6EC86EC8);               
	TVD_WRITE32(tvd_base[channel_id] + 0x8C0, 0x67049E04);               
	TVD_WRITE32(tvd_base[channel_id] + 0x8C4, 0x120845A0);               
	TVD_WRITE32(tvd_base[channel_id] + 0x8C8, 0x0185C501);               
	TVD_WRITE32(tvd_base[channel_id] + 0x8CC, 0xC1CBA860);               
	TVD_WRITE32(tvd_base[channel_id] + 0x8D0, 0x70000BB8);               
	TVD_WRITE32(tvd_base[channel_id] + 0x8D4, 0x7F294A52);               
	TVD_WRITE32(tvd_base[channel_id] + 0x8D8, 0x83C28F00);               
	TVD_WRITE32(tvd_base[channel_id] + 0x8DC, 0x74D08610);               
	TVD_WRITE32(tvd_base[channel_id] + 0x8E0, 0x195D1CCD);               
	TVD_WRITE32(tvd_base[channel_id] + 0x8E4, 0x413B0231);               
	TVD_WRITE32(tvd_base[channel_id] + 0x8E8, 0x0440283B);               
	TVD_WRITE32(tvd_base[channel_id] + 0x8EC, 0xF3935CD6);               
	TVD_WRITE32(tvd_base[channel_id] + 0x8F0, 0x532714C2);               
	TVD_WRITE32(tvd_base[channel_id] + 0x8F4, 0x67184854);               
	TVD_WRITE32(tvd_base[channel_id] + 0x8F8, 0x80000000);               
	TVD_WRITE32(tvd_base[channel_id] + 0x8FC, 0x00000000);               
	TVD_WRITE32(tvd_base[channel_id] + 0x900, 0x62F32005);               
	TVD_WRITE32(tvd_base[channel_id] + 0x904, 0x120855A0);               
	TVD_WRITE32(tvd_base[channel_id] + 0x908, 0x0185D0B9);               
	TVD_WRITE32(tvd_base[channel_id] + 0x90C, 0xC1CAA800);               
	TVD_WRITE32(tvd_base[channel_id] + 0x910, 0x70000BB8);               
	TVD_WRITE32(tvd_base[channel_id] + 0x914, 0x7F29CE73);               
	TVD_WRITE32(tvd_base[channel_id] + 0x918, 0x83C28F00);               
	TVD_WRITE32(tvd_base[channel_id] + 0x91C, 0x74D08610);               
	TVD_WRITE32(tvd_base[channel_id] + 0x920, 0x195D1CCD);               
	TVD_WRITE32(tvd_base[channel_id] + 0x924, 0x413B0252);               
	TVD_WRITE32(tvd_base[channel_id] + 0x928, 0x0440283B);               
	TVD_WRITE32(tvd_base[channel_id] + 0x92C, 0xF403DCF7);               
	TVD_WRITE32(tvd_base[channel_id] + 0x930, 0x532714C2);               
	TVD_WRITE32(tvd_base[channel_id] + 0x934, 0x87184814);               
	TVD_WRITE32(tvd_base[channel_id] + 0x938, 0x08000800);               
	TVD_WRITE32(tvd_base[channel_id] + 0x93C, 0x00000800);               
	TVD_WRITE32(tvd_base[channel_id] + 0x940, 0x10281400);               
	TVD_WRITE32(tvd_base[channel_id] + 0x944, 0x04D0BB40);               
	TVD_WRITE32(tvd_base[channel_id] + 0x948, 0x10281400);               
	TVD_WRITE32(tvd_base[channel_id] + 0x94C, 0x04D0BB40);               
	TVD_WRITE32(tvd_base[channel_id] + 0x950, 0x00000000);               
	TVD_WRITE32(tvd_base[channel_id] + 0x954, 0x00000000);               
	TVD_WRITE32(tvd_base[channel_id] + 0x958, 0x00000000);               
	TVD_WRITE32(tvd_base[channel_id] + 0x95C, 0x00000000);               
	TVD_WRITE32(tvd_base[channel_id] + 0x960, 0x00000000);               
	TVD_WRITE32(tvd_base[channel_id] + 0x964, 0x00000000);               
	TVD_WRITE32(tvd_base[channel_id] + 0x968, 0x00000000);               
	TVD_WRITE32(tvd_base[channel_id] + 0x96C, 0x00000000);               
	TVD_WRITE32(tvd_base[channel_id] + 0x970, 0x00000000);               
	TVD_WRITE32(tvd_base[channel_id] + 0x974, 0x00000000);               
	TVD_WRITE32(tvd_base[channel_id] + 0x978, 0x00000000);               
	TVD_WRITE32(tvd_base[channel_id] + 0x97C, 0x00000000);               
	TVD_WRITE32(tvd_base[channel_id] + 0x980, 0x00000000);               
	TVD_WRITE32(tvd_base[channel_id] + 0x984, 0x00000000);               
	TVD_WRITE32(tvd_base[channel_id] + 0x988, 0x00000000);               
	TVD_WRITE32(tvd_base[channel_id] + 0x98C, 0x00000000);               
	TVD_WRITE32(tvd_base[channel_id] + 0x990, 0x00000000);               
	TVD_WRITE32(tvd_base[channel_id] + 0x994, 0x00000000);               
	TVD_WRITE32(tvd_base[channel_id] + 0x998, 0x00000000);               
	TVD_WRITE32(tvd_base[channel_id] + 0x99C, 0x00000000);               
	TVD_WRITE32(tvd_base[channel_id] + 0x9A0, 0x00000000);               
	TVD_WRITE32(tvd_base[channel_id] + 0x9A4, 0x00000000);               
	TVD_WRITE32(tvd_base[channel_id] + 0x9A8, 0x00000000);               
	TVD_WRITE32(tvd_base[channel_id] + 0x9AC, 0x00000000);               
	TVD_WRITE32(tvd_base[channel_id] + 0x9B0, 0x00000000);               
	TVD_WRITE32(tvd_base[channel_id] + 0x9B4, 0x00000000);               
	TVD_WRITE32(tvd_base[channel_id] + 0x9B8, 0x00000000);               
	TVD_WRITE32(tvd_base[channel_id] + 0x9BC, 0x00000000);               
	TVD_WRITE32(tvd_base[channel_id] + 0x9C0, 0x00000000);               
	TVD_WRITE32(tvd_base[channel_id] + 0x9C4, 0x00000000);               
	TVD_WRITE32(tvd_base[channel_id] + 0x9C8, 0x00000000);               
	TVD_WRITE32(tvd_base[channel_id] + 0x9CC, 0x00000000);               
	TVD_WRITE32(tvd_base[channel_id] + 0x9D0, 0x00000000);               
	TVD_WRITE32(tvd_base[channel_id] + 0x9D4, 0x00000000);               
	TVD_WRITE32(tvd_base[channel_id] + 0x9D8, 0x00000000);               
	TVD_WRITE32(tvd_base[channel_id] + 0x9DC, 0x00000000);               
	TVD_WRITE32(tvd_base[channel_id] + 0x9E0, 0x00000000);               
	TVD_WRITE32(tvd_base[channel_id] + 0x9E4, 0x00000000);               
	TVD_WRITE32(tvd_base[channel_id] + 0x9E8, 0x00000000);               
	TVD_WRITE32(tvd_base[channel_id] + 0x9EC, 0x00000000);               
	TVD_WRITE32(tvd_base[channel_id] + 0x9F0, 0x00000000);               
	TVD_WRITE32(tvd_base[channel_id] + 0x9F4, 0x00000000);               
	TVD_WRITE32(tvd_base[channel_id] + 0x9F8, 0x00000000);               
	TVD_WRITE32(tvd_base[channel_id] + 0x9FC, 0x00000000);               
	TVD_WRITE32(tvd_base[channel_id] + 0xA00, 0x00000000);               
	TVD_WRITE32(tvd_base[channel_id] + 0xA04, 0x00000000);               
	TVD_WRITE32(tvd_base[channel_id] + 0xA08, 0x00000000);               
	TVD_WRITE32(tvd_base[channel_id] + 0xA0C, 0x00000000);               
	TVD_WRITE32(tvd_base[channel_id] + 0xA10, 0x00000000);               
	TVD_WRITE32(tvd_base[channel_id] + 0xA14, 0x00000000);               
	TVD_WRITE32(tvd_base[channel_id] + 0xA18, 0x00000000);               
	TVD_WRITE32(tvd_base[channel_id] + 0xA1C, 0x00000000);               
	TVD_WRITE32(tvd_base[channel_id] + 0xA20, 0x00000000);               
	TVD_WRITE32(tvd_base[channel_id] + 0xA24, 0x00000000);               
	TVD_WRITE32(tvd_base[channel_id] + 0xA28, 0x00000000);               
	TVD_WRITE32(tvd_base[channel_id] + 0xA2C, 0x00000000);               
	TVD_WRITE32(tvd_base[channel_id] + 0xA30, 0x00000000);               
	TVD_WRITE32(tvd_base[channel_id] + 0xA34, 0x00000000);               
	TVD_WRITE32(tvd_base[channel_id] + 0xA38, 0x00000000);               
	TVD_WRITE32(tvd_base[channel_id] + 0xA3C, 0x00000000);               
	TVD_WRITE32(tvd_base[channel_id] + 0xA40, 0x00000000);               
	TVD_WRITE32(tvd_base[channel_id] + 0xA44, 0x00000000);               
	TVD_WRITE32(tvd_base[channel_id] + 0xA48, 0x00000000);               
	TVD_WRITE32(tvd_base[channel_id] + 0xA4C, 0x00000000);               
	TVD_WRITE32(tvd_base[channel_id] + 0xA50, 0x00000000);               
	TVD_WRITE32(tvd_base[channel_id] + 0xA54, 0x00000000);               
	TVD_WRITE32(tvd_base[channel_id] + 0xA58, 0x00000000);               
	TVD_WRITE32(tvd_base[channel_id] + 0xA5C, 0x00000000);               
	TVD_WRITE32(tvd_base[channel_id] + 0xA60, 0x00000000);               
	TVD_WRITE32(tvd_base[channel_id] + 0xA64, 0x00000000);               
	TVD_WRITE32(tvd_base[channel_id] + 0xA68, 0x00000000);               
	TVD_WRITE32(tvd_base[channel_id] + 0xA6C, 0x00000000);               
	TVD_WRITE32(tvd_base[channel_id] + 0xA70, 0x00000000);               
	TVD_WRITE32(tvd_base[channel_id] + 0xA74, 0x00000000);               
	TVD_WRITE32(tvd_base[channel_id] + 0xA78, 0x00000000);               
	TVD_WRITE32(tvd_base[channel_id] + 0xA7C, 0x00000000);               
	TVD_WRITE32(tvd_base[channel_id] + 0xA80, 0x0FF3FCFF);               
	TVD_WRITE32(tvd_base[channel_id] + 0xA84, 0x80008000);               
	TVD_WRITE32(tvd_base[channel_id] + 0xA88, 0x80000000);               
	TVD_WRITE32(tvd_base[channel_id] + 0xA8C, 0x00000000);               
	TVD_WRITE32(tvd_base[channel_id] + 0xA90, 0x00000000);               
	TVD_WRITE32(tvd_base[channel_id] + 0xA94, 0x00000000);               
	TVD_WRITE32(tvd_base[channel_id] + 0xA98, 0x00000000);               
	TVD_WRITE32(tvd_base[channel_id] + 0xA9C, 0x00000000);               
	TVD_WRITE32(tvd_base[channel_id] + 0xAA0, 0x00000000);               
	TVD_WRITE32(tvd_base[channel_id] + 0xAA4, 0x00000000);               
	TVD_WRITE32(tvd_base[channel_id] + 0xAA8, 0x00000000);               
	TVD_WRITE32(tvd_base[channel_id] + 0xAAC, 0x00000000);               
	TVD_WRITE32(tvd_base[channel_id] + 0xAB0, 0x22F08C03);               
	TVD_WRITE32(tvd_base[channel_id] + 0xAB4, 0x8F0070D4);               
	TVD_WRITE32(tvd_base[channel_id] + 0xAB8, 0x42400800);               
	TVD_WRITE32(tvd_base[channel_id] + 0xABC, 0x82000820);  

	TVD_WRITE32(tvd_base[channel_id] + 0x5aC, 0x000010C0);  //bit 31
	TVD_WRITE32(tvd_base[channel_id] + 0x59C, 0x21E2c0a6); //H start 
	TVD_WRITE32(tvd_base[channel_id] + 0x5c4, 0x18181617);  //v start



	return;
}


void vPAL_2D_Setting(TVD_CHANNEL_ID channel_id)
{
	TVD_WRITE32(tvd_base[channel_id] + 0x410, 0x00818000);	
	TVD_WRITE32(tvd_base[channel_id] + 0x414, 0xFF080020);	
	TVD_WRITE32(tvd_base[channel_id] + 0x418, 0x10000000);	
	TVD_WRITE32(tvd_base[channel_id] + 0x41C, 0x12400010);	
	TVD_WRITE32(tvd_base[channel_id] + 0x420, 0x22788207);	
	TVD_WRITE32(tvd_base[channel_id] + 0x424, 0x000000FF);	
	TVD_WRITE32(tvd_base[channel_id] + 0x428, 0xFFFFFFFF);	
	TVD_WRITE32(tvd_base[channel_id] + 0x42C, 0x00000000);	
	TVD_WRITE32(tvd_base[channel_id] + 0x430, 0x00000000);	

	TVD_WRITE32(tvd_base[channel_id] + 0x434, 0xC4D00101);	


	TVD_WRITE32(tvd_base[channel_id] + 0x438, 0xB801404A);	
	TVD_WRITE32(tvd_base[channel_id] + 0x43C, 0x00800801);	
	TVD_WRITE32(tvd_base[channel_id] + 0x440, 0x00D01001);	
	TVD_WRITE32(tvd_base[channel_id] + 0x444, 0x1E000000);	
	TVD_WRITE32(tvd_base[channel_id] + 0x448, 0x80000114);	
	TVD_WRITE32(tvd_base[channel_id] + 0x44C, 0x2AAA8103);	
	TVD_WRITE32(tvd_base[channel_id] + 0x450, 0xF0000100);	
	TVD_WRITE32(tvd_base[channel_id] + 0x454, 0x1E000000);	
	TVD_WRITE32(tvd_base[channel_id] + 0x458, 0x80000114);	
	TVD_WRITE32(tvd_base[channel_id] + 0x45C, 0x2AAA8103);	
	TVD_WRITE32(tvd_base[channel_id] + 0x460, 0x47B00000);	
	TVD_WRITE32(tvd_base[channel_id] + 0x464, 0x1E000000);	
	TVD_WRITE32(tvd_base[channel_id] + 0x468, 0x80000114);	
	TVD_WRITE32(tvd_base[channel_id] + 0x46C, 0x2AAA8103);	
	TVD_WRITE32(tvd_base[channel_id] + 0x470, 0x1100001B);	
	TVD_WRITE32(tvd_base[channel_id] + 0x474, 0xAAA04EAB);	
	TVD_WRITE32(tvd_base[channel_id] + 0x478, 0x0888AAAA);	
	TVD_WRITE32(tvd_base[channel_id] + 0x47C, 0x00050000);	
	TVD_WRITE32(tvd_base[channel_id] + 0x480, 0x003E313D);	
	TVD_WRITE32(tvd_base[channel_id] + 0x484, 0x100F0480);	
	//TVD_WRITE32(0x488,0x00000000);	
	TVD_WRITE32(tvd_base[channel_id] + 0x48C, 0x0200022D);	
	TVD_WRITE32(tvd_base[channel_id] + 0x490, 0x0AFFC588);	
	TVD_WRITE32(tvd_base[channel_id] + 0x494, 0x08000000);	
	TVD_WRITE32(tvd_base[channel_id] + 0x498, 0x00000000);	
	TVD_WRITE32(tvd_base[channel_id] + 0x49C, 0x0009C000);	
	TVD_WRITE32(tvd_base[channel_id] + 0x4A0, 0x00090000);	
	TVD_WRITE32(tvd_base[channel_id] + 0x4A4, 0x00000000);	
	TVD_WRITE32(tvd_base[channel_id] + 0x4A8, 0x00000000);	
	TVD_WRITE32(tvd_base[channel_id] + 0x4AC, 0x00000000);	
	TVD_WRITE32(tvd_base[channel_id] + 0x4B0, 0x00000000);	
	TVD_WRITE32(tvd_base[channel_id] + 0x4B4, 0x00000000);	
	TVD_WRITE32(tvd_base[channel_id] + 0x4B8, 0x00000000);	
	TVD_WRITE32(tvd_base[channel_id] + 0x4BC, 0x00000000);

	TVD_WRITE32(tvd_base[channel_id] + 0x4C0, 0x51515C55 );	
	TVD_WRITE32(tvd_base[channel_id] + 0x4C4, 0x04555D45 );	
	TVD_WRITE32(tvd_base[channel_id] + 0x4C8, 0x27F7595E );	
	TVD_WRITE32(tvd_base[channel_id] + 0x4CC, 0x8F34039C );	
	TVD_WRITE32(tvd_base[channel_id] + 0x4D0, 0x15890F80 );	
	TVD_WRITE32(tvd_base[channel_id] + 0x4D4, 0x26144040 );	
	TVD_WRITE32(tvd_base[channel_id] + 0x4D8, 0x0A00F600 );	
	TVD_WRITE32(tvd_base[channel_id] + 0x4DC, 0xF41F0800 );	
	TVD_WRITE32(tvd_base[channel_id] + 0x4E0, 0x13178080 );	
	TVD_WRITE32(tvd_base[channel_id] + 0x4E4, 0x8AA7075E );	
	TVD_WRITE32(tvd_base[channel_id] + 0x4E8, 0xF08400A0 );	
	TVD_WRITE32(tvd_base[channel_id] + 0x4EC, 0x08180C40 );	
	TVD_WRITE32(tvd_base[channel_id] + 0x4F0, 0x2008C2C9 );	
	TVD_WRITE32(tvd_base[channel_id] + 0x4F4, 0x7180001F );	
	TVD_WRITE32(tvd_base[channel_id] + 0x4F8, 0x8000306C );	
	TVD_WRITE32(tvd_base[channel_id] + 0x4FC, 0x420C35A0 );	
	TVD_WRITE32(tvd_base[channel_id] + 0x500, 0x6838FA27 );	
	TVD_WRITE32(tvd_base[channel_id] + 0x504, 0x0DFA2090 );	
	TVD_WRITE32(tvd_base[channel_id] + 0x508, 0x08E06B40 );	
	TVD_WRITE32(tvd_base[channel_id] + 0x50C, 0x83608016 );	
	TVD_WRITE32(tvd_base[channel_id] + 0x510, 0x40204080 );	
	TVD_WRITE32(tvd_base[channel_id] + 0x514, 0x80326485 );	
	TVD_WRITE32(tvd_base[channel_id] + 0x518, 0x11A0A04C );	
	TVD_WRITE32(tvd_base[channel_id] + 0x51C, 0x8040A0C0 );	
	TVD_WRITE32(tvd_base[channel_id] + 0x520, 0x7E35AF80 );	
	TVD_WRITE32(tvd_base[channel_id] + 0x524, 0x0408F840 );	
	TVD_WRITE32(tvd_base[channel_id] + 0x528, 0x5404E030 );	
	TVD_WRITE32(tvd_base[channel_id] + 0x52C, 0x30900AA0 );	
	TVD_WRITE32(tvd_base[channel_id] + 0x530, 0x0B410B09 );	
	TVD_WRITE32(tvd_base[channel_id] + 0x534, 0xF43E40F0 );	
	TVD_WRITE32(tvd_base[channel_id] + 0x538, 0x452414F0 );	
	TVD_WRITE32(tvd_base[channel_id] + 0x53C, 0x55C29325 );	

	TVD_WRITE32(tvd_base[channel_id] + 0x540, 0x30173F93);	
	TVD_WRITE32(tvd_base[channel_id] + 0x544, 0x21A41725);	
	TVD_WRITE32(tvd_base[channel_id] + 0x548, 0x408080FE);	
	TVD_WRITE32(tvd_base[channel_id] + 0x54C, 0x1245482B);	
	TVD_WRITE32(tvd_base[channel_id] + 0x550, 0xB4000023);	
	TVD_WRITE32(tvd_base[channel_id] + 0x554, 0x04AE0318);	
	TVD_WRITE32(tvd_base[channel_id] + 0x558, 0x106443D0);	
	TVD_WRITE32(tvd_base[channel_id] + 0x55C, 0x52836471);	
	TVD_WRITE32(tvd_base[channel_id] + 0x560, 0x29E518EF);	
	TVD_WRITE32(tvd_base[channel_id] + 0x564, 0x2010553C);	
	TVD_WRITE32(tvd_base[channel_id] + 0x568, 0x204CDEC9);	
	TVD_WRITE32(tvd_base[channel_id] + 0x56C, 0x204A9881);	
	TVD_WRITE32(tvd_base[channel_id] + 0x570, 0xFF29DEC5);	
	TVD_WRITE32(tvd_base[channel_id] + 0x574, 0xCA8B0F00);	
	TVD_WRITE32(tvd_base[channel_id] + 0x578, 0x797A8E61);	
	TVD_WRITE32(tvd_base[channel_id] + 0x57C, 0x420FA8A1);	
	TVD_WRITE32(tvd_base[channel_id] + 0x580, 0x208181D4);	
	TVD_WRITE32(tvd_base[channel_id] + 0x584, 0x28AF88C8);	
	TVD_WRITE32(tvd_base[channel_id] + 0x588, 0xEC844040);	
	TVD_WRITE32(tvd_base[channel_id] + 0x58C, 0x2854C531);	
	TVD_WRITE32(tvd_base[channel_id] + 0x590, 0x0206E61A);	
	TVD_WRITE32(tvd_base[channel_id] + 0x594, 0x4840310E);	
	TVD_WRITE32(tvd_base[channel_id] + 0x598, 0x21594088);	
	TVD_WRITE32(tvd_base[channel_id] + 0x59C, 0x29E2C0A5);	
	TVD_WRITE32(tvd_base[channel_id] + 0x5A0, 0x44800C80);	
	TVD_WRITE32(tvd_base[channel_id] + 0x5A4, 0x20010000);	
	TVD_WRITE32(tvd_base[channel_id] + 0x5A8, 0x0A4BC680);	
	TVD_WRITE32(tvd_base[channel_id] + 0x5AC, 0x400010C0); 

	TVD_WRITE32(tvd_base[channel_id] + 0x5B0, 0x20008400); // adjust  

	TVD_WRITE32(tvd_base[channel_id] + 0x5B4, 0x00F9BC00);	
	TVD_WRITE32(tvd_base[channel_id] + 0x5B8, 0x20002004);	
	TVD_WRITE32(tvd_base[channel_id] + 0x5BC, 0x17E0114F);	
	TVD_WRITE32(tvd_base[channel_id] + 0x5C0, 0x1F908420);	
	TVD_WRITE32(tvd_base[channel_id] + 0x5C4, 0x1A191515);	
	TVD_WRITE32(tvd_base[channel_id] + 0x5C8, 0x06658819);	
	TVD_WRITE32(tvd_base[channel_id] + 0x5CC, 0x72B91AB9);	
	TVD_WRITE32(tvd_base[channel_id] + 0x5D0, 0x5A1640DB);	
	TVD_WRITE32(tvd_base[channel_id] + 0x5D4, 0x55F401F4);	
	TVD_WRITE32(tvd_base[channel_id] + 0x5D8, 0x1001825E);	
	TVD_WRITE32(tvd_base[channel_id] + 0x5DC, 0x60506E60);	
	TVD_WRITE32(tvd_base[channel_id] + 0x5E0, 0xA0A793A6);	
	TVD_WRITE32(tvd_base[channel_id] + 0x5E4, 0xFACAD1A0);	
	TVD_WRITE32(tvd_base[channel_id] + 0x5E8, 0x7CB83BD4);	
	TVD_WRITE32(tvd_base[channel_id] + 0x5EC, 0x80DD43A9);	
	TVD_WRITE32(tvd_base[channel_id] + 0x5F0, 0x80554329);	
	TVD_WRITE32(tvd_base[channel_id] + 0x5F4, 0x1748A14B);	
	TVD_WRITE32(tvd_base[channel_id] + 0x5F8, 0x5040C67F);	
	TVD_WRITE32(tvd_base[channel_id] + 0x5FC, 0x71000480);	
	TVD_WRITE32(tvd_base[channel_id] + 0x600, 0x3D34035F);	
	TVD_WRITE32(tvd_base[channel_id] + 0x604, 0xF9228320);	
	TVD_WRITE32(tvd_base[channel_id] + 0x608, 0x32201501);	
	TVD_WRITE32(tvd_base[channel_id] + 0x60C, 0x5480A833);	
	TVD_WRITE32(tvd_base[channel_id] + 0x610, 0x000028AD);	
	TVD_WRITE32(tvd_base[channel_id] + 0x614, 0x55E8666E);	
	TVD_WRITE32(tvd_base[channel_id] + 0x618, 0xE6CB2224);	
	TVD_WRITE32(tvd_base[channel_id] + 0x61C, 0x84460080);	
	TVD_WRITE32(tvd_base[channel_id] + 0x620, 0x00400000);	
	TVD_WRITE32(tvd_base[channel_id] + 0x624, 0x7F084330);	
	TVD_WRITE32(tvd_base[channel_id] + 0x628, 0x08880001);	
	TVD_WRITE32(tvd_base[channel_id] + 0x62C, 0xBF488488);	
	TVD_WRITE32(tvd_base[channel_id] + 0x630, 0x0119812C);	
	TVD_WRITE32(tvd_base[channel_id] + 0x634, 0x01000844);	
	TVD_WRITE32(tvd_base[channel_id] + 0x638, 0x88020818);	
	TVD_WRITE32(tvd_base[channel_id] + 0x63C, 0x0FF0AA1F);	

	TVD_WRITE32(tvd_base[channel_id] + 0x640, 0x000B101F); 
	TVD_WRITE32(tvd_base[channel_id] + 0x644, 0x09A40600);	
	TVD_WRITE32(tvd_base[channel_id] + 0x648, 0x0A048BB1); 
	TVD_WRITE32(tvd_base[channel_id] + 0x64C, 0x2F360203); 
	TVD_WRITE32(tvd_base[channel_id] + 0x650, 0x03A80100); 
	TVD_WRITE32(tvd_base[channel_id] + 0x654, 0x3136400A); 
	TVD_WRITE32(tvd_base[channel_id] + 0x658, 0x02361E74); 
	TVD_WRITE32(tvd_base[channel_id] + 0x65C, 0x02441800); 
	TVD_WRITE32(tvd_base[channel_id] + 0x660, 0x01151139); 
	TVD_WRITE32(tvd_base[channel_id] + 0x664, 0x46F3C00B); 
	TVD_WRITE32(tvd_base[channel_id] + 0x668, 0x0A0AD303); 
	TVD_WRITE32(tvd_base[channel_id] + 0x66C, 0x0A0AD303); 
	TVD_WRITE32(tvd_base[channel_id] + 0x670, 0x000098C4); 
	TVD_WRITE32(tvd_base[channel_id] + 0x674, 0x00000007); 
	TVD_WRITE32(tvd_base[channel_id] + 0x678, 0x77777777);	
	TVD_WRITE32(tvd_base[channel_id] + 0x67C, 0x4A101030); 
	TVD_WRITE32(tvd_base[channel_id] + 0x680, 0x510A1E05);	
	TVD_WRITE32(tvd_base[channel_id] + 0x684, 0x6218040C);	
	TVD_WRITE32(tvd_base[channel_id] + 0x688, 0x8296321E);	
	TVD_WRITE32(tvd_base[channel_id] + 0x68C, 0x00107010);	
	TVD_WRITE32(tvd_base[channel_id] + 0x690, 0x0F01EF85);	
	TVD_WRITE32(tvd_base[channel_id] + 0x694, 0x1070500C);	
	TVD_WRITE32(tvd_base[channel_id] + 0x698, 0x00641400);	
	TVD_WRITE32(tvd_base[channel_id] + 0x69C, 0x00300500);	
	TVD_WRITE32(tvd_base[channel_id] + 0x6A0, 0x0000230A);	
	TVD_WRITE32(tvd_base[channel_id] + 0x6A4, 0x1E712808);	
	TVD_WRITE32(tvd_base[channel_id] + 0x6A8, 0x8006408A);	
	TVD_WRITE32(tvd_base[channel_id] + 0x6AC, 0x54280000);	
	TVD_WRITE32(tvd_base[channel_id] + 0x6B0, 0x200F0000);	
	TVD_WRITE32(tvd_base[channel_id] + 0x6B4, 0x00969600);	
	TVD_WRITE32(tvd_base[channel_id] + 0x6B8, 0x04044444);	
	TVD_WRITE32(tvd_base[channel_id] + 0x6BC, 0x0C010444);	
	TVD_WRITE32(tvd_base[channel_id] + 0x6C0, 0x00A444F3);	
	TVD_WRITE32(tvd_base[channel_id] + 0x6C4, 0x3E1A53D3);	
	TVD_WRITE32(tvd_base[channel_id] + 0x6C8, 0x064B1478);	
	TVD_WRITE32(tvd_base[channel_id] + 0x6CC, 0x1020D831);	
	TVD_WRITE32(tvd_base[channel_id] + 0x6D0, 0x11111111);	
	TVD_WRITE32(tvd_base[channel_id] + 0x6D4, 0xC008FC10);	
	TVD_WRITE32(tvd_base[channel_id] + 0x6D8, 0x00000000);	
	TVD_WRITE32(tvd_base[channel_id] + 0x6DC, 0xC0001020);	
	TVD_WRITE32(tvd_base[channel_id] + 0x6E0, 0x0F271858);	
	TVD_WRITE32(tvd_base[channel_id] + 0x6E4, 0x3220008F);	
	TVD_WRITE32(tvd_base[channel_id] + 0x6E8, 0xA02D0E28);	
	TVD_WRITE32(tvd_base[channel_id] + 0x6EC, 0x00000054);	
	TVD_WRITE32(tvd_base[channel_id] + 0x6F0, 0x43174537);	
	TVD_WRITE32(tvd_base[channel_id] + 0x6F4, 0x0045713C);	
	TVD_WRITE32(tvd_base[channel_id] + 0x6F8, 0xC0001A50);	
	TVD_WRITE32(tvd_base[channel_id] + 0x6FC, 0x4414050D);	
	TVD_WRITE32(tvd_base[channel_id] + 0x700, 0x11A00020);	
	TVD_WRITE32(tvd_base[channel_id] + 0x704, 0x64800450);	
	TVD_WRITE32(tvd_base[channel_id] + 0x708, 0x410A0028);	
	TVD_WRITE32(tvd_base[channel_id] + 0x70C, 0x00014455);	
	TVD_WRITE32(tvd_base[channel_id] + 0x710, 0x00000000);	
	TVD_WRITE32(tvd_base[channel_id] + 0x714, 0x31581E48); 
	TVD_WRITE32(tvd_base[channel_id] + 0x718, 0x8C9B3000); 
	TVD_WRITE32(tvd_base[channel_id] + 0x71C, 0x08101020); 
	TVD_WRITE32(tvd_base[channel_id] + 0x720, 0x08081E14); 
	TVD_WRITE32(tvd_base[channel_id] + 0x724, 0x0A100008); 
	TVD_WRITE32(tvd_base[channel_id] + 0x728, 0x14060208); 
	TVD_WRITE32(tvd_base[channel_id] + 0x72C, 0x0C0A2808); 
	TVD_WRITE32(tvd_base[channel_id] + 0x730, 0x0A0D9226); 
	TVD_WRITE32(tvd_base[channel_id] + 0x734, 0x33110029);	
	TVD_WRITE32(tvd_base[channel_id] + 0x738, 0x1311042A); 
	TVD_WRITE32(tvd_base[channel_id] + 0x73C, 0x8C03100A); 
	TVD_WRITE32(tvd_base[channel_id] + 0x740, 0x12345678); 
	TVD_WRITE32(tvd_base[channel_id] + 0x744, 0x02345678); 
	TVD_WRITE32_MASK(tvd_base[channel_id] + 0x748,0x00081414, 0xCFFFFFFF);
	TVD_WRITE32(tvd_base[channel_id] + 0x74C, 0x00030A10); 
	TVD_WRITE32(tvd_base[channel_id] + 0x750, 0x047C0558); 
	TVD_WRITE32(tvd_base[channel_id] + 0x754, 0x0088011A); 
	TVD_WRITE32(tvd_base[channel_id] + 0x758, 0x00009666); 
	TVD_WRITE32(tvd_base[channel_id] + 0x75C, 0x000781FF); 
	TVD_WRITE32(tvd_base[channel_id] + 0x760, 0x90000000); 
	TVD_WRITE32(tvd_base[channel_id] + 0x764, 0x00000000); 
	TVD_WRITE32(tvd_base[channel_id] + 0x768, 0x0F051818); 
	TVD_WRITE32(tvd_base[channel_id] + 0x76C, 0x00191014); 
	TVD_WRITE32(tvd_base[channel_id] + 0x770, 0x23645050); 
	TVD_WRITE32(tvd_base[channel_id] + 0x774, 0x64641432); 
	TVD_WRITE32(tvd_base[channel_id] + 0x778, 0x1E519696); 
	TVD_WRITE32(tvd_base[channel_id] + 0x77C, 0x10169696); 
	TVD_WRITE32(tvd_base[channel_id] + 0x780, 0x053C501E); 
	TVD_WRITE32(tvd_base[channel_id] + 0x784, 0x820A325A); 
	TVD_WRITE32(tvd_base[channel_id] + 0x788, 0xFFFF3359);	
	TVD_WRITE32(tvd_base[channel_id] + 0x78C, 0xFFFFFFFF); 
	TVD_WRITE32(tvd_base[channel_id] + 0x790, 0x00000000);	
	TVD_WRITE32(tvd_base[channel_id] + 0x794, 0x00000000);	
	TVD_WRITE32(tvd_base[channel_id] + 0x798, 0x00000000);	
	TVD_WRITE32(tvd_base[channel_id] + 0x79C, 0x00000000);	
	TVD_WRITE32(tvd_base[channel_id] + 0x7A0, 0x00000000);	
	TVD_WRITE32(tvd_base[channel_id] + 0x7A4, 0x00000000);	
	TVD_WRITE32(tvd_base[channel_id] + 0x7A8, 0x00000000);	
	TVD_WRITE32(tvd_base[channel_id] + 0x7AC, 0x00000000);	
	TVD_WRITE32(tvd_base[channel_id] + 0x7B0, 0x00000000);	
	TVD_WRITE32(tvd_base[channel_id] + 0x7B4, 0x100181FA);	
	TVD_WRITE32(tvd_base[channel_id] + 0x7B8, 0x00000000);	
	TVD_WRITE32(tvd_base[channel_id] + 0x7BC, 0x00000000);	
	TVD_WRITE32(tvd_base[channel_id] + 0x7D8, 0x020F42D0);	
	TVD_WRITE32(tvd_base[channel_id] + 0x7DC, 0x00000008);	
	TVD_WRITE32(tvd_base[channel_id] + 0x7E0, 0x0F0000A0);	
	TVD_WRITE32(tvd_base[channel_id] + 0x7E4, 0x11000020);	
	TVD_WRITE32(tvd_base[channel_id] + 0x7E8, 0xC000C000);	
	TVD_WRITE32(tvd_base[channel_id] + 0x7EC, 0x00CC0C50);	
	TVD_WRITE32(tvd_base[channel_id] + 0x7F0, 0x80008000);	
	TVD_WRITE32(tvd_base[channel_id] + 0x7F4, 0x00000000);	
	TVD_WRITE32(tvd_base[channel_id] + 0x7F8, 0x80000000);	
	TVD_WRITE32(tvd_base[channel_id] + 0x7FC, 0xD0090901);	
	TVD_WRITE32(tvd_base[channel_id] + 0x800, 0x00060604);	
	TVD_WRITE32(tvd_base[channel_id] + 0x804, 0x0B9000FF);	
	TVD_WRITE32(tvd_base[channel_id] + 0x808, 0x001400FC);	
	TVD_WRITE32(tvd_base[channel_id] + 0x80C, 0x005071AD);	
	TVD_WRITE32(tvd_base[channel_id] + 0x810, 0xC3080050);	
	TVD_WRITE32(tvd_base[channel_id] + 0x814, 0x0F120700);	
	TVD_WRITE32(tvd_base[channel_id] + 0x818, 0x02100578);	
	TVD_WRITE32(tvd_base[channel_id] + 0x81C, 0x65004020);	
	TVD_WRITE32(tvd_base[channel_id] + 0x820, 0x0000F00F);	
	TVD_WRITE32(tvd_base[channel_id] + 0x824, 0x14140F0F);	
	TVD_WRITE32(tvd_base[channel_id] + 0x828, 0x0A0A0A0A);	
	TVD_WRITE32(tvd_base[channel_id] + 0x82C, 0x77700000);	
	TVD_WRITE32(tvd_base[channel_id] + 0x830, 0x00A0A000);	
	TVD_WRITE32(tvd_base[channel_id] + 0x834, 0x00000000);	
	TVD_WRITE32(tvd_base[channel_id] + 0x838, 0x4400A00A);	
	TVD_WRITE32(tvd_base[channel_id] + 0x83C, 0x14000000);	
	TVD_WRITE32(tvd_base[channel_id] + 0x840, 0x00003010);	
	TVD_WRITE32(tvd_base[channel_id] + 0x844, 0x00201007);	
	TVD_WRITE32(tvd_base[channel_id] + 0x848, 0x00201007);	
	TVD_WRITE32(tvd_base[channel_id] + 0x84C, 0x0700100A);	
	TVD_WRITE32(tvd_base[channel_id] + 0x850, 0x00208858);	
	TVD_WRITE32(tvd_base[channel_id] + 0x854, 0x14001010);	
	TVD_WRITE32(tvd_base[channel_id] + 0x858, 0x00141414);	
	TVD_WRITE32(tvd_base[channel_id] + 0x85C, 0x10141414);	
	TVD_WRITE32(tvd_base[channel_id] + 0x860, 0x00000000);	
	TVD_WRITE32(tvd_base[channel_id] + 0x864, 0x00010000);	
	TVD_WRITE32(tvd_base[channel_id] + 0x868, 0x00000000);	
	TVD_WRITE32(tvd_base[channel_id] + 0x86C, 0xC0400C0A);	
	TVD_WRITE32(tvd_base[channel_id] + 0x870, 0x00020820);	
	TVD_WRITE32(tvd_base[channel_id] + 0x874, 0xC0016C01);	
	TVD_WRITE32(tvd_base[channel_id] + 0x878, 0x0089A022);	
	TVD_WRITE32(tvd_base[channel_id] + 0x87C, 0x00000000);	
	TVD_WRITE32(tvd_base[channel_id] + 0x880, 0x00060604);	
	TVD_WRITE32(tvd_base[channel_id] + 0x884, 0x000001FC);	
	TVD_WRITE32(tvd_base[channel_id] + 0x888, 0x005071AD);	
	TVD_WRITE32(tvd_base[channel_id] + 0x88C, 0x00002010);	
	TVD_WRITE32(tvd_base[channel_id] + 0x890, 0x0008F060);	
	TVD_WRITE32(tvd_base[channel_id] + 0x894, 0x0020A000);	
	TVD_WRITE32(tvd_base[channel_id] + 0x898, 0x00000000);	
	TVD_WRITE32(tvd_base[channel_id] + 0x89C, 0x00606200);	
	TVD_WRITE32(tvd_base[channel_id] + 0x8A0, 0xC000C000);	
	TVD_WRITE32(tvd_base[channel_id] + 0x8A4, 0x00000000);	
	TVD_WRITE32(tvd_base[channel_id] + 0x8A8, 0x0000000F);	
	TVD_WRITE32(tvd_base[channel_id] + 0x8AC, 0x00000000);	
	TVD_WRITE32(tvd_base[channel_id] + 0x8B0, 0x00000000);	
	TVD_WRITE32(tvd_base[channel_id] + 0x8B4, 0x00000078);	
	TVD_WRITE32(tvd_base[channel_id] + 0x8B8, 0xC0016C01);	
	TVD_WRITE32(tvd_base[channel_id] + 0x8BC, 0x6EC86EC8);	
	TVD_WRITE32(tvd_base[channel_id] + 0x8C0, 0x65031E04);	
	TVD_WRITE32(tvd_base[channel_id] + 0x8C4, 0x12884420);	
	TVD_WRITE32(tvd_base[channel_id] + 0x8C8, 0x0185C501);	
	TVD_WRITE32(tvd_base[channel_id] + 0x8CC, 0xC1CBA860);	
	TVD_WRITE32(tvd_base[channel_id] + 0x8D0, 0x70000BB8);	
	TVD_WRITE32(tvd_base[channel_id] + 0x8D4, 0x7F294A73);	
	TVD_WRITE32(tvd_base[channel_id] + 0x8D8, 0x83C29000);	
	TVD_WRITE32(tvd_base[channel_id] + 0x8DC, 0x74D08610);	
	TVD_WRITE32(tvd_base[channel_id] + 0x8E0, 0x195D1CCD);	
	TVD_WRITE32(tvd_base[channel_id] + 0x8E4, 0x2ACBBEF8);	
	TVD_WRITE32(tvd_base[channel_id] + 0x8E8, 0x03F81CCD);	
	TVD_WRITE32(tvd_base[channel_id] + 0x8EC, 0xF3935CD6);	
	TVD_WRITE32(tvd_base[channel_id] + 0x8F0, 0x532714C2);	
	TVD_WRITE32(tvd_base[channel_id] + 0x8F4, 0x471848D4);	
	TVD_WRITE32(tvd_base[channel_id] + 0x8F8, 0x80000000);	
	TVD_WRITE32(tvd_base[channel_id] + 0x8FC, 0x00000000);	
	TVD_WRITE32(tvd_base[channel_id] + 0x900, 0x62F32005);	
	TVD_WRITE32(tvd_base[channel_id] + 0x904, 0x120855A0);	
	TVD_WRITE32(tvd_base[channel_id] + 0x908, 0x0185D0B9);	
	TVD_WRITE32(tvd_base[channel_id] + 0x90C, 0xC1CAA800);	
	TVD_WRITE32(tvd_base[channel_id] + 0x910, 0x70000BB8);	
	TVD_WRITE32(tvd_base[channel_id] + 0x914, 0x7F284231);	
	TVD_WRITE32(tvd_base[channel_id] + 0x918, 0x83C28F00);	
	TVD_WRITE32(tvd_base[channel_id] + 0x91C, 0x74D08610);	
	TVD_WRITE32(tvd_base[channel_id] + 0x920, 0x195D1CCD);	
	TVD_WRITE32(tvd_base[channel_id] + 0x924, 0x413B0252);	
	TVD_WRITE32(tvd_base[channel_id] + 0x928, 0x0440283B);	
	TVD_WRITE32(tvd_base[channel_id] + 0x92C, 0xF403DCF7);	
	TVD_WRITE32(tvd_base[channel_id] + 0x930, 0x532714C2);	
	TVD_WRITE32(tvd_base[channel_id] + 0x934, 0x87184814);	
	TVD_WRITE32(tvd_base[channel_id] + 0x938, 0x08000800);	
	TVD_WRITE32(tvd_base[channel_id] + 0x93C, 0x00000800);	
	TVD_WRITE32(tvd_base[channel_id] + 0x940, 0x10281400);	
	TVD_WRITE32(tvd_base[channel_id] + 0x944, 0x04D0BB40);	
	TVD_WRITE32(tvd_base[channel_id] + 0x948, 0x10281400);	
	TVD_WRITE32(tvd_base[channel_id] + 0x94C, 0x04D0BB40);	
	TVD_WRITE32(tvd_base[channel_id] + 0x950, 0x00000000);	
	TVD_WRITE32(tvd_base[channel_id] + 0x954, 0x00000000);	
	TVD_WRITE32(tvd_base[channel_id] + 0x958, 0x00000000);	
	TVD_WRITE32(tvd_base[channel_id] + 0x95C, 0x00000000);	
	TVD_WRITE32(tvd_base[channel_id] + 0x960, 0x00000000);	
	TVD_WRITE32(tvd_base[channel_id] + 0x964, 0x00000000);	
	TVD_WRITE32(tvd_base[channel_id] + 0x968, 0x00000000);	
	TVD_WRITE32(tvd_base[channel_id] + 0x96C, 0x00000000);	
	TVD_WRITE32(tvd_base[channel_id] + 0x970, 0x00000000);	
	TVD_WRITE32(tvd_base[channel_id] + 0x974, 0x00000000);	
	TVD_WRITE32(tvd_base[channel_id] + 0x978, 0x00000000);	
	TVD_WRITE32(tvd_base[channel_id] + 0x97C, 0x00000000);	
	TVD_WRITE32(tvd_base[channel_id] + 0x980, 0x00000000);	
	TVD_WRITE32(tvd_base[channel_id] + 0x984, 0x00000000);	
	TVD_WRITE32(tvd_base[channel_id] + 0x988, 0x00000000);	
	TVD_WRITE32(tvd_base[channel_id] + 0x98C, 0x00000000);	
	TVD_WRITE32(tvd_base[channel_id] + 0x990, 0x00000000);	
	TVD_WRITE32(tvd_base[channel_id] + 0x994, 0x00000000);	
	TVD_WRITE32(tvd_base[channel_id] + 0x998, 0x00000000);	
	TVD_WRITE32(tvd_base[channel_id] + 0x99C, 0x00000000);	
	TVD_WRITE32(tvd_base[channel_id] + 0x9A0, 0x00000000);	
	TVD_WRITE32(tvd_base[channel_id] + 0x9A4, 0x00000000);	
	TVD_WRITE32(tvd_base[channel_id] + 0x9A8, 0x00000000);	
	TVD_WRITE32(tvd_base[channel_id] + 0x9AC, 0x00000000);	
	TVD_WRITE32(tvd_base[channel_id] + 0x9B0, 0x00000000);	
	TVD_WRITE32(tvd_base[channel_id] + 0x9B4, 0x00000000);	
	TVD_WRITE32(tvd_base[channel_id] + 0x9B8, 0x00000000);	
	TVD_WRITE32(tvd_base[channel_id] + 0x9BC, 0x00000000);	
	TVD_WRITE32(tvd_base[channel_id] + 0x9C0, 0x00000000);	
	TVD_WRITE32(tvd_base[channel_id] + 0x9C4, 0x00000000);	
	TVD_WRITE32(tvd_base[channel_id] + 0x9C8, 0x00000000);	
	TVD_WRITE32(tvd_base[channel_id] + 0x9CC, 0x00000000);	
	TVD_WRITE32(tvd_base[channel_id] + 0x9D0, 0x00000000);	
	TVD_WRITE32(tvd_base[channel_id] + 0x9D4, 0x00000000);	
	TVD_WRITE32(tvd_base[channel_id] + 0x9D8, 0x00000000);	
	TVD_WRITE32(tvd_base[channel_id] + 0x9DC, 0x00000000);	
	TVD_WRITE32(tvd_base[channel_id] + 0x9E0, 0x00000000);	
	TVD_WRITE32(tvd_base[channel_id] + 0x9E4, 0x00000000);	
	TVD_WRITE32(tvd_base[channel_id] + 0x9E8, 0x00000000);	
	TVD_WRITE32(tvd_base[channel_id] + 0x9EC, 0x00000000);	
	TVD_WRITE32(tvd_base[channel_id] + 0x9F0, 0x00000000);	
	TVD_WRITE32(tvd_base[channel_id] + 0x9F4, 0x00000000);	
	TVD_WRITE32(tvd_base[channel_id] + 0x9F8, 0x00000000);	
	TVD_WRITE32(tvd_base[channel_id] + 0x9FC, 0x00000000);	
	TVD_WRITE32(tvd_base[channel_id] + 0xA00, 0x00000000);	
	TVD_WRITE32(tvd_base[channel_id] + 0xA04, 0x00000000);	
	TVD_WRITE32(tvd_base[channel_id] + 0xA08, 0x00000000);	
	TVD_WRITE32(tvd_base[channel_id] + 0xA0C, 0x00000000);	
	TVD_WRITE32(tvd_base[channel_id] + 0xA10, 0x00000000);	
	TVD_WRITE32(tvd_base[channel_id] + 0xA14, 0x00000000);	
	TVD_WRITE32(tvd_base[channel_id] + 0xA18, 0x00000000);	
	TVD_WRITE32(tvd_base[channel_id] + 0xA1C, 0x00000000);	
	TVD_WRITE32(tvd_base[channel_id] + 0xA20, 0x00000000);	
	TVD_WRITE32(tvd_base[channel_id] + 0xA24, 0x00000000);	
	TVD_WRITE32(tvd_base[channel_id] + 0xA28, 0x00000000);	
	TVD_WRITE32(tvd_base[channel_id] + 0xA2C, 0x00000000);	
	TVD_WRITE32(tvd_base[channel_id] + 0xA30, 0x00000000);	
	TVD_WRITE32(tvd_base[channel_id] + 0xA34, 0x00000000);	
	TVD_WRITE32(tvd_base[channel_id] + 0xA38, 0x00000000);	
	TVD_WRITE32(tvd_base[channel_id] + 0xA3C, 0x00000000);	
	TVD_WRITE32(tvd_base[channel_id] + 0xA40, 0x00000000);	
	TVD_WRITE32(tvd_base[channel_id] + 0xA44, 0x00000000);	
	TVD_WRITE32(tvd_base[channel_id] + 0xA48, 0x00000000);	
	TVD_WRITE32(tvd_base[channel_id] + 0xA4C, 0x00000000);	
	TVD_WRITE32(tvd_base[channel_id] + 0xA50, 0x00000000);	
	TVD_WRITE32(tvd_base[channel_id] + 0xA54, 0x00000000);	
	TVD_WRITE32(tvd_base[channel_id] + 0xA58, 0x00000000);	
	TVD_WRITE32(tvd_base[channel_id] + 0xA5C, 0x00000000);	
	TVD_WRITE32(tvd_base[channel_id] + 0xA60, 0x00000000);	
	TVD_WRITE32(tvd_base[channel_id] + 0xA64, 0x00000000);	
	TVD_WRITE32(tvd_base[channel_id] + 0xA68, 0x00000000);	
	TVD_WRITE32(tvd_base[channel_id] + 0xA6C, 0x00000000);	
	TVD_WRITE32(tvd_base[channel_id] + 0xA70, 0x00000000);	
	TVD_WRITE32(tvd_base[channel_id] + 0xA74, 0x00000000);	
	TVD_WRITE32(tvd_base[channel_id] + 0xA78, 0x00000000);	
	TVD_WRITE32(tvd_base[channel_id] + 0xA7C, 0x00000000);	
	TVD_WRITE32(tvd_base[channel_id] + 0xA80, 0x0FF3FCFF);	
	TVD_WRITE32(tvd_base[channel_id] + 0xA84, 0x80008000);	
	TVD_WRITE32(tvd_base[channel_id] + 0xA88, 0x80000000);	
	TVD_WRITE32(tvd_base[channel_id] + 0xA8C, 0x00000000);	
	TVD_WRITE32(tvd_base[channel_id] + 0xA90, 0x00000000);	
	TVD_WRITE32(tvd_base[channel_id] + 0xA94, 0x00000000);	
	TVD_WRITE32(tvd_base[channel_id] + 0xA98, 0x00000000);	
	TVD_WRITE32(tvd_base[channel_id] + 0xA9C, 0x00000000);	
	TVD_WRITE32(tvd_base[channel_id] + 0xAA0, 0x00000000);	
	TVD_WRITE32(tvd_base[channel_id] + 0xAA4, 0x00000000);	
	TVD_WRITE32(tvd_base[channel_id] + 0xAA8, 0x00000000);	
	TVD_WRITE32(tvd_base[channel_id] + 0xAAC, 0x00000000);	
	TVD_WRITE32(tvd_base[channel_id] + 0xAB0, 0x22F08C03);	
	TVD_WRITE32(tvd_base[channel_id] + 0xAB4, 0x8F0070D4);	
	TVD_WRITE32(tvd_base[channel_id] + 0xAB8, 0x42400800);	
	TVD_WRITE32(tvd_base[channel_id] + 0xABC, 0x82000820);	
	
	TVD_WRITE32(tvd_base[channel_id] + 0x5aC, 0x000010C0);  //bit 31
	TVD_WRITE32(tvd_base[channel_id] + 0x59C, 0x21E2c0B2); //H start
	TVD_WRITE32(tvd_base[channel_id] + 0x5c4, 0x19191617);  //v start

	return;
}


void vPAL_60_2D_Setting(TVD_CHANNEL_ID channel_id)
{
	TVD_WRITE32(tvd_base[channel_id] + 0x410, 0x00818000); 
	TVD_WRITE32(tvd_base[channel_id] + 0x414, 0xFF080020); 
	TVD_WRITE32(tvd_base[channel_id] + 0x418, 0x10000000); 
	TVD_WRITE32(tvd_base[channel_id] + 0x41C, 0x12400010); 
	TVD_WRITE32(tvd_base[channel_id] + 0x420, 0x22788207); 
	TVD_WRITE32(tvd_base[channel_id] + 0x424, 0x000000FF); 
	TVD_WRITE32(tvd_base[channel_id] + 0x428, 0xFFFFFFFF); 
	TVD_WRITE32(tvd_base[channel_id] + 0x42C, 0x00000000); 
	TVD_WRITE32(tvd_base[channel_id] + 0x430, 0x00000000); 


	TVD_WRITE32(tvd_base[channel_id] + 0x434, 0xC4D00101); 



	TVD_WRITE32(tvd_base[channel_id] + 0x438, 0xB801404A); 
	TVD_WRITE32(tvd_base[channel_id] + 0x43C, 0x00800801); 
	TVD_WRITE32(tvd_base[channel_id] + 0x440, 0x00D01001); 
	TVD_WRITE32(tvd_base[channel_id] + 0x444, 0x1E000000); 
	TVD_WRITE32(tvd_base[channel_id] + 0x448, 0x80000114); 
	TVD_WRITE32(tvd_base[channel_id] + 0x44C, 0x2AAA8103); 
	TVD_WRITE32(tvd_base[channel_id] + 0x450, 0xF0000100); 
	TVD_WRITE32(tvd_base[channel_id] + 0x454, 0x1E000000); 
	TVD_WRITE32(tvd_base[channel_id] + 0x458, 0x80000114); 
	TVD_WRITE32(tvd_base[channel_id] + 0x45C, 0x2AAA8103); 
	TVD_WRITE32(tvd_base[channel_id] + 0x460, 0x47B00000); 
	TVD_WRITE32(tvd_base[channel_id] + 0x464, 0x1E000000); 
	TVD_WRITE32(tvd_base[channel_id] + 0x468, 0x80000114); 
	TVD_WRITE32(tvd_base[channel_id] + 0x46C, 0x2AAA8103); 
	TVD_WRITE32(tvd_base[channel_id] + 0x470, 0x1100001B); 
	TVD_WRITE32(tvd_base[channel_id] + 0x474, 0xAAA04EAB); 
	TVD_WRITE32(tvd_base[channel_id] + 0x478, 0x0888AAAA); 
	TVD_WRITE32(tvd_base[channel_id] + 0x47C, 0x00050000); 
	TVD_WRITE32(tvd_base[channel_id] + 0x480, 0x003E313D); 
	TVD_WRITE32(tvd_base[channel_id] + 0x484, 0x100F0480); 
	TVD_WRITE32(tvd_base[channel_id] + 0x488, 0x00000000); 
	TVD_WRITE32(tvd_base[channel_id] + 0x48C, 0x0200022D); 
	TVD_WRITE32(tvd_base[channel_id] + 0x490, 0x0AFFC588); 
	TVD_WRITE32(tvd_base[channel_id] + 0x494, 0x08000000); 
	TVD_WRITE32(tvd_base[channel_id] + 0x498, 0x00000000); 
	TVD_WRITE32(tvd_base[channel_id] + 0x49C, 0x0009C000); 
	TVD_WRITE32(tvd_base[channel_id] + 0x4A0, 0x00090000); 
	TVD_WRITE32(tvd_base[channel_id] + 0x4A4, 0x00000000); 
	TVD_WRITE32(tvd_base[channel_id] + 0x4A8, 0x00000000); 
	TVD_WRITE32(tvd_base[channel_id] + 0x4AC, 0x00000000); 
	TVD_WRITE32(tvd_base[channel_id] + 0x4B0, 0x00000000); 
	TVD_WRITE32(tvd_base[channel_id] + 0x4B4, 0x00000000); 
	TVD_WRITE32(tvd_base[channel_id] + 0x4B8, 0x00000000); 
	TVD_WRITE32(tvd_base[channel_id] + 0x4BC, 0x00000000); 
	TVD_WRITE32(tvd_base[channel_id] + 0x4C0, 0x51515C55); 
	TVD_WRITE32(tvd_base[channel_id] + 0x4C4, 0x04557845); 
	TVD_WRITE32(tvd_base[channel_id] + 0x4C8, 0x27F7595E); 
	TVD_WRITE32(tvd_base[channel_id] + 0x4CC, 0x8F340380); 
	TVD_WRITE32(tvd_base[channel_id] + 0x4D0, 0x158C0F80); 
	TVD_WRITE32(tvd_base[channel_id] + 0x4D4, 0x26144040); 
	TVD_WRITE32(tvd_base[channel_id] + 0x4D8, 0x0A00F600); 
	TVD_WRITE32(tvd_base[channel_id] + 0x4DC, 0xF41F0800); 
	TVD_WRITE32(tvd_base[channel_id] + 0x4E0, 0x13178080); 
	TVD_WRITE32(tvd_base[channel_id] + 0x4E4, 0x8AA7075E); 
	TVD_WRITE32(tvd_base[channel_id] + 0x4E8, 0xF08400A0); 
	TVD_WRITE32(tvd_base[channel_id] + 0x4EC, 0x08180C40); 
	TVD_WRITE32(tvd_base[channel_id] + 0x4F0, 0x2008C2C9); 
	TVD_WRITE32(tvd_base[channel_id] + 0x4F4, 0x7180001F); 
	TVD_WRITE32(tvd_base[channel_id] + 0x4F8, 0x8000306C); 
	TVD_WRITE32(tvd_base[channel_id] + 0x4FC, 0x420C33A0); 
	TVD_WRITE32(tvd_base[channel_id] + 0x500, 0x6838FA27); 
	TVD_WRITE32(tvd_base[channel_id] + 0x504, 0x0DFA2090); 
	TVD_WRITE32(tvd_base[channel_id] + 0x508, 0x08E06B40); 
	TVD_WRITE32(tvd_base[channel_id] + 0x50C, 0x83608016); 
	TVD_WRITE32(tvd_base[channel_id] + 0x510, 0x40204080); 
	TVD_WRITE32(tvd_base[channel_id] + 0x514, 0x80326485); 
	TVD_WRITE32(tvd_base[channel_id] + 0x518, 0x11A0A04C); 
	TVD_WRITE32(tvd_base[channel_id] + 0x51C, 0x8040A0C0); 
	TVD_WRITE32(tvd_base[channel_id] + 0x520, 0x7E35AF80); 
	TVD_WRITE32(tvd_base[channel_id] + 0x524, 0x0408F840); 
	TVD_WRITE32(tvd_base[channel_id] + 0x528, 0x5404E030); 
	TVD_WRITE32(tvd_base[channel_id] + 0x52C, 0x30900AA0); 
	TVD_WRITE32(tvd_base[channel_id] + 0x530, 0x0B410B09); 
	TVD_WRITE32(tvd_base[channel_id] + 0x534, 0xF43E40F0); 
	TVD_WRITE32(tvd_base[channel_id] + 0x538, 0x452414F0); 
	TVD_WRITE32(tvd_base[channel_id] + 0x53C, 0x55C29325); 
	TVD_WRITE32(tvd_base[channel_id] + 0x540, 0x30173E63); 
	TVD_WRITE32(tvd_base[channel_id] + 0x544, 0x21A41725); 
	TVD_WRITE32(tvd_base[channel_id] + 0x548, 0x30B080FE); 
	TVD_WRITE32(tvd_base[channel_id] + 0x54C, 0x1245482B); 
	TVD_WRITE32(tvd_base[channel_id] + 0x550, 0xB4000023); 
	TVD_WRITE32(tvd_base[channel_id] + 0x554, 0x04AE0318); 
	TVD_WRITE32(tvd_base[channel_id] + 0x558, 0x106441D0); 
	TVD_WRITE32(tvd_base[channel_id] + 0x55C, 0x57856973); 
	TVD_WRITE32(tvd_base[channel_id] + 0x560, 0x29E518EF); 
	TVD_WRITE32(tvd_base[channel_id] + 0x564, 0x2010573D); 
	TVD_WRITE32(tvd_base[channel_id] + 0x568, 0x204CDEC9); 
	TVD_WRITE32(tvd_base[channel_id] + 0x56C, 0x204A9881); 
	TVD_WRITE32(tvd_base[channel_id] + 0x570, 0xFF29DEC5); 
	TVD_WRITE32(tvd_base[channel_id] + 0x574, 0xCA8B0F00); 
	TVD_WRITE32(tvd_base[channel_id] + 0x578, 0x797A8E61); 
	TVD_WRITE32(tvd_base[channel_id] + 0x57C, 0x420FA8A1); 
	TVD_WRITE32(tvd_base[channel_id] + 0x580, 0x208181D4); 
	TVD_WRITE32(tvd_base[channel_id] + 0x584, 0x28AF88CA); 
	TVD_WRITE32(tvd_base[channel_id] + 0x588, 0xFC844040); 
	TVD_WRITE32(tvd_base[channel_id] + 0x58C, 0x2854C531); 
	TVD_WRITE32(tvd_base[channel_id] + 0x590, 0x0206E61A); 
	TVD_WRITE32(tvd_base[channel_id] + 0x594, 0x4840310E); 
	TVD_WRITE32(tvd_base[channel_id] + 0x598, 0x21594088); 
	TVD_WRITE32(tvd_base[channel_id] + 0x59C, 0x29E2C0A5); 
	TVD_WRITE32(tvd_base[channel_id] + 0x5A0, 0x44800C80); 
	TVD_WRITE32(tvd_base[channel_id] + 0x5A4, 0x20010000); 
	TVD_WRITE32(tvd_base[channel_id] + 0x5A8, 0x0A4BC6C1); 
	TVD_WRITE32(tvd_base[channel_id] + 0x5AC, 0x400010C0); 
	TVD_WRITE32(tvd_base[channel_id] + 0x5B0, 0x20180400); 
	TVD_WRITE32(tvd_base[channel_id] + 0x5B4, 0x00F9BC00); 
	TVD_WRITE32(tvd_base[channel_id] + 0x5B8, 0x20002004); 
	TVD_WRITE32(tvd_base[channel_id] + 0x5BC, 0x17E0114F); 
	TVD_WRITE32(tvd_base[channel_id] + 0x5C0, 0x1F908420); 
	TVD_WRITE32(tvd_base[channel_id] + 0x5C4, 0x1A191515); 
	TVD_WRITE32(tvd_base[channel_id] + 0x5C8, 0x06658819); 
	TVD_WRITE32(tvd_base[channel_id] + 0x5CC, 0x72B91AB9); 
	TVD_WRITE32(tvd_base[channel_id] + 0x5D0, 0x5A1640DB); 
	TVD_WRITE32(tvd_base[channel_id] + 0x5D4, 0x55F401F4); 
	TVD_WRITE32(tvd_base[channel_id] + 0x5D8, 0x100181FA); 
	TVD_WRITE32(tvd_base[channel_id] + 0x5DC, 0x60506E60); 
	TVD_WRITE32(tvd_base[channel_id] + 0x5E0, 0xA0A797A6); 
	TVD_WRITE32(tvd_base[channel_id] + 0x5E4, 0xFAC9D1A0); 
	TVD_WRITE32(tvd_base[channel_id] + 0x5E8, 0x7CB83BD4); 
	TVD_WRITE32(tvd_base[channel_id] + 0x5EC, 0x80DD43A9); 
	TVD_WRITE32(tvd_base[channel_id] + 0x5F0, 0x80554329); 
	TVD_WRITE32(tvd_base[channel_id] + 0x5F4, 0x1748A14B); 
	TVD_WRITE32(tvd_base[channel_id] + 0x5F8, 0x50E3C67F); 
	TVD_WRITE32(tvd_base[channel_id] + 0x5FC, 0x70D70480); 
	TVD_WRITE32(tvd_base[channel_id] + 0x600, 0x3D34035F); 
	TVD_WRITE32(tvd_base[channel_id] + 0x604, 0xF9228320); 
	TVD_WRITE32(tvd_base[channel_id] + 0x608, 0x32201501); 
	TVD_WRITE32(tvd_base[channel_id] + 0x60C, 0x5985A833); 
	TVD_WRITE32(tvd_base[channel_id] + 0x610, 0x000028AD); 
	TVD_WRITE32(tvd_base[channel_id] + 0x614, 0x55E86B73); 
	TVD_WRITE32(tvd_base[channel_id] + 0x618, 0xE6CB2224); 
	TVD_WRITE32(tvd_base[channel_id] + 0x61C, 0x84460080); 
	TVD_WRITE32(tvd_base[channel_id] + 0x620, 0x00400000); 
	TVD_WRITE32(tvd_base[channel_id] + 0x624, 0x7F084330); 
	TVD_WRITE32(tvd_base[channel_id] + 0x628, 0x08880001); 
	TVD_WRITE32(tvd_base[channel_id] + 0x62C, 0xBF488488); 
	TVD_WRITE32(tvd_base[channel_id] + 0x630, 0x0119812C); 
	TVD_WRITE32(tvd_base[channel_id] + 0x634, 0x01000844); 
	TVD_WRITE32(tvd_base[channel_id] + 0x638, 0x88020818); 
	TVD_WRITE32(tvd_base[channel_id] + 0x63C, 0x0FF0AA1F); 
	TVD_WRITE32(tvd_base[channel_id] + 0x640, 0x000B101F); 
	TVD_WRITE32(tvd_base[channel_id] + 0x644, 0x81A2F600); 
	TVD_WRITE32(tvd_base[channel_id] + 0x648, 0x0A045AFB); 
	TVD_WRITE32(tvd_base[channel_id] + 0x64C, 0x25030213); 
	TVD_WRITE32(tvd_base[channel_id] + 0x650, 0x00000000); 
	TVD_WRITE32(tvd_base[channel_id] + 0x654, 0x2F030438); 
	TVD_WRITE32(tvd_base[channel_id] + 0x658, 0x021E1E74); 
	TVD_WRITE32(tvd_base[channel_id] + 0x65C, 0x02641900); 
	TVD_WRITE32(tvd_base[channel_id] + 0x660, 0x01151139); 
	TVD_WRITE32(tvd_base[channel_id] + 0x664, 0x46F3C00B); 
	TVD_WRITE32(tvd_base[channel_id] + 0x668, 0x0A09052E); 
	TVD_WRITE32(tvd_base[channel_id] + 0x66C, 0x0A09052E); 
	TVD_WRITE32(tvd_base[channel_id] + 0x670, 0x00000000); 
	TVD_WRITE32(tvd_base[channel_id] + 0x674, 0x00000004); 
	TVD_WRITE32(tvd_base[channel_id] + 0x678, 0x393A393A); 
	TVD_WRITE32(tvd_base[channel_id] + 0x67C, 0x38081808); 
	TVD_WRITE32(tvd_base[channel_id] + 0x680, 0x090A1E00); 
	TVD_WRITE32(tvd_base[channel_id] + 0x684, 0x64180810); 
	TVD_WRITE32(tvd_base[channel_id] + 0x688, 0x64AA321E); 
	TVD_WRITE32(tvd_base[channel_id] + 0x68C, 0x32107023); 
	TVD_WRITE32(tvd_base[channel_id] + 0x690, 0x0F013F85); 
	TVD_WRITE32(tvd_base[channel_id] + 0x694, 0x0000B910); 
	TVD_WRITE32(tvd_base[channel_id] + 0x698, 0x0064C800); 
	TVD_WRITE32(tvd_base[channel_id] + 0x69C, 0x00480610); 
	TVD_WRITE32(tvd_base[channel_id] + 0x6A0, 0x0000900F); 
	TVD_WRITE32(tvd_base[channel_id] + 0x6A4, 0x10782880); 
	TVD_WRITE32(tvd_base[channel_id] + 0x6A8, 0x60084084); 
	TVD_WRITE32(tvd_base[channel_id] + 0x6AC, 0x1E0F1904); 
	TVD_WRITE32(tvd_base[channel_id] + 0x6B0, 0x10080344); 
	TVD_WRITE32(tvd_base[channel_id] + 0x6B4, 0x3E505F07); 
	TVD_WRITE32(tvd_base[channel_id] + 0x6B8, 0x04414444); 
	TVD_WRITE32(tvd_base[channel_id] + 0x6BC, 0x00110333); 
	TVD_WRITE32(tvd_base[channel_id] + 0x6C0, 0x00A404F3); 
	TVD_WRITE32(tvd_base[channel_id] + 0x6C4, 0x45195030); 
	TVD_WRITE32(tvd_base[channel_id] + 0x6C8, 0x0640285C); 
	TVD_WRITE32(tvd_base[channel_id] + 0x6CC, 0x11600811); 
	TVD_WRITE32(tvd_base[channel_id] + 0x6D0, 0x11111111); 
	TVD_WRITE32(tvd_base[channel_id] + 0x6D4, 0x00089C00); 
	TVD_WRITE32(tvd_base[channel_id] + 0x6D8, 0x00000000); 
	TVD_WRITE32(tvd_base[channel_id] + 0x6DC, 0x00071004); 
	TVD_WRITE32(tvd_base[channel_id] + 0x6E0, 0x141420FF); 
	TVD_WRITE32(tvd_base[channel_id] + 0x6E4, 0x32200060); 
	TVD_WRITE32(tvd_base[channel_id] + 0x6E8, 0x20A06000); 
	TVD_WRITE32(tvd_base[channel_id] + 0x6EC, 0x00000014); 
	TVD_WRITE32(tvd_base[channel_id] + 0x6F0, 0x43174537); 
	TVD_WRITE32(tvd_base[channel_id] + 0x6F4, 0x0045713C); 
	TVD_WRITE32(tvd_base[channel_id] + 0x6F8, 0xC0001A50); 
	TVD_WRITE32(tvd_base[channel_id] + 0x6FC, 0x4414050D); 
	TVD_WRITE32(tvd_base[channel_id] + 0x700, 0x11A00020); 
	TVD_WRITE32(tvd_base[channel_id] + 0x704, 0x64000450); 
	TVD_WRITE32(tvd_base[channel_id] + 0x708, 0x410A0028); 
	TVD_WRITE32(tvd_base[channel_id] + 0x70C, 0x00014455); 
	TVD_WRITE32(tvd_base[channel_id] + 0x710, 0x00000000); 
	TVD_WRITE32(tvd_base[channel_id] + 0x714, 0x314059C0); 
	TVD_WRITE32(tvd_base[channel_id] + 0x718, 0x801E0000); 
	TVD_WRITE32(tvd_base[channel_id] + 0x71C, 0x08001120); 
	TVD_WRITE32(tvd_base[channel_id] + 0x720, 0x08081E03); 
	TVD_WRITE32(tvd_base[channel_id] + 0x724, 0x00100008); 
	TVD_WRITE32(tvd_base[channel_id] + 0x728, 0x07060A10); 
	TVD_WRITE32(tvd_base[channel_id] + 0x72C, 0x10101022); 
	TVD_WRITE32(tvd_base[channel_id] + 0x730, 0x20B09000); 
	TVD_WRITE32(tvd_base[channel_id] + 0x734, 0x22110A0E); 
	TVD_WRITE32(tvd_base[channel_id] + 0x738, 0x1111043A); 
	TVD_WRITE32(tvd_base[channel_id] + 0x73C, 0x8000080A); 
	TVD_WRITE32(tvd_base[channel_id] + 0x740, 0x12345678); 
	TVD_WRITE32(tvd_base[channel_id] + 0x744, 0x02345678);  
	TVD_WRITE32_MASK(tvd_base[channel_id] + 0x748, 0x00080014, 0xCFFFFFFF);
	TVD_WRITE32(tvd_base[channel_id] + 0x74C, 0x00020A34); 
	TVD_WRITE32(tvd_base[channel_id] + 0x750, 0x047C0558); 
	TVD_WRITE32(tvd_base[channel_id] + 0x754, 0x0088011A); 
	TVD_WRITE32(tvd_base[channel_id] + 0x758, 0x00009666); 
	TVD_WRITE32(tvd_base[channel_id] + 0x75C, 0x000781FF); 
	TVD_WRITE32(tvd_base[channel_id] + 0x760, 0x90000000); 
	TVD_WRITE32(tvd_base[channel_id] + 0x764, 0x00000000); 
	TVD_WRITE32(tvd_base[channel_id] + 0x768, 0x0F051818); 
	TVD_WRITE32(tvd_base[channel_id] + 0x76C, 0x00191014); 
	TVD_WRITE32(tvd_base[channel_id] + 0x770, 0x23645050); 
	TVD_WRITE32(tvd_base[channel_id] + 0x774, 0x64641432); 
	TVD_WRITE32(tvd_base[channel_id] + 0x778, 0x1E519696); 
	TVD_WRITE32(tvd_base[channel_id] + 0x77C, 0x10169696); 
	TVD_WRITE32(tvd_base[channel_id] + 0x780, 0x053C501E); 
	TVD_WRITE32(tvd_base[channel_id] + 0x784, 0x820A325A); 
	TVD_WRITE32(tvd_base[channel_id] + 0x788, 0x6BFF3359); 
	TVD_WRITE32(tvd_base[channel_id] + 0x78C, 0xFFFFFFFF); 
	TVD_WRITE32(tvd_base[channel_id] + 0x790, 0x00000000); 
	TVD_WRITE32(tvd_base[channel_id] + 0x794, 0x00000000); 
	TVD_WRITE32(tvd_base[channel_id] + 0x798, 0x00000000); 
	TVD_WRITE32(tvd_base[channel_id] + 0x79C, 0x00000000); 
	TVD_WRITE32(tvd_base[channel_id] + 0x7A0, 0x00000000); 
	TVD_WRITE32(tvd_base[channel_id] + 0x7A4, 0x00000000); 
	TVD_WRITE32(tvd_base[channel_id] + 0x7A8, 0x00000000); 
	TVD_WRITE32(tvd_base[channel_id] + 0x7AC, 0x00000000); 
	TVD_WRITE32(tvd_base[channel_id] + 0x7B0, 0x00000000); 
	TVD_WRITE32(tvd_base[channel_id] + 0x7B4, 0x100181FA); 
	TVD_WRITE32(tvd_base[channel_id] + 0x7B8, 0x00000000); 
	TVD_WRITE32(tvd_base[channel_id] + 0x7BC, 0x00000000); 

	TVD_WRITE32(tvd_base[channel_id] + 0x7D8, 0x020F42D0); 
	TVD_WRITE32(tvd_base[channel_id] + 0x7DC, 0x00000008); 
	TVD_WRITE32(tvd_base[channel_id] + 0x7E0, 0x0F0000A0); 
	TVD_WRITE32(tvd_base[channel_id] + 0x7E4, 0x11000020); 
	TVD_WRITE32(tvd_base[channel_id] + 0x7E8, 0xC000C000); 
	TVD_WRITE32(tvd_base[channel_id] + 0x7EC, 0x00CC0C50); 
	TVD_WRITE32(tvd_base[channel_id] + 0x7F0, 0x80008000); 
	TVD_WRITE32(tvd_base[channel_id] + 0x7F4, 0x00000000); 
	TVD_WRITE32(tvd_base[channel_id] + 0x7F8, 0x80000000); 
	TVD_WRITE32(tvd_base[channel_id] + 0x7FC, 0xD0090901); 
	TVD_WRITE32(tvd_base[channel_id] + 0x800, 0x00060604); 
	TVD_WRITE32(tvd_base[channel_id] + 0x804, 0x0B9000FF); 
	TVD_WRITE32(tvd_base[channel_id] + 0x808, 0x001400FC); 
	TVD_WRITE32(tvd_base[channel_id] + 0x80C, 0x005071AD); 
	TVD_WRITE32(tvd_base[channel_id] + 0x810, 0xC3080050); 
	TVD_WRITE32(tvd_base[channel_id] + 0x814, 0x0F120700); 
	TVD_WRITE32(tvd_base[channel_id] + 0x818, 0x02100578); 
	TVD_WRITE32(tvd_base[channel_id] + 0x81C, 0x65004020); 
	TVD_WRITE32(tvd_base[channel_id] + 0x820, 0x0000F00F); 
	TVD_WRITE32(tvd_base[channel_id] + 0x824, 0x14140F0F); 
	TVD_WRITE32(tvd_base[channel_id] + 0x828, 0x0A0A0A0A); 
	TVD_WRITE32(tvd_base[channel_id] + 0x82C, 0x77700000); 
	TVD_WRITE32(tvd_base[channel_id] + 0x830, 0x00A0A000); 
	TVD_WRITE32(tvd_base[channel_id] + 0x834, 0x00000000); 
	TVD_WRITE32(tvd_base[channel_id] + 0x838, 0x4400A00A); 
	TVD_WRITE32(tvd_base[channel_id] + 0x83C, 0x14000000); 
	TVD_WRITE32(tvd_base[channel_id] + 0x840, 0x00003010); 
	TVD_WRITE32(tvd_base[channel_id] + 0x844, 0x00201007); 
	TVD_WRITE32(tvd_base[channel_id] + 0x848, 0x00201007); 
	TVD_WRITE32(tvd_base[channel_id] + 0x84C, 0x0700100A); 
	TVD_WRITE32(tvd_base[channel_id] + 0x850, 0x00208858); 
	TVD_WRITE32(tvd_base[channel_id] + 0x854, 0x14001010); 
	TVD_WRITE32(tvd_base[channel_id] + 0x858, 0x00141414); 
	TVD_WRITE32(tvd_base[channel_id] + 0x85C, 0x10141414); 
	TVD_WRITE32(tvd_base[channel_id] + 0x860, 0x00000000); 
	TVD_WRITE32(tvd_base[channel_id] + 0x864, 0x00010000); 
	TVD_WRITE32(tvd_base[channel_id] + 0x868, 0x00000000); 
	TVD_WRITE32(tvd_base[channel_id] + 0x86C, 0xC0400C0A); 
	TVD_WRITE32(tvd_base[channel_id] + 0x870, 0x00020820); 
	TVD_WRITE32(tvd_base[channel_id] + 0x874, 0xC0016C01); 
	TVD_WRITE32(tvd_base[channel_id] + 0x878, 0x0089A022); 
	TVD_WRITE32(tvd_base[channel_id] + 0x87C, 0x00000000); 
	TVD_WRITE32(tvd_base[channel_id] + 0x880, 0x00060604); 
	TVD_WRITE32(tvd_base[channel_id] + 0x884, 0x000001FC); 
	TVD_WRITE32(tvd_base[channel_id] + 0x888, 0x005071AD); 
	TVD_WRITE32(tvd_base[channel_id] + 0x88C, 0x00002010); 
	TVD_WRITE32(tvd_base[channel_id] + 0x890, 0x0008F060); 
	TVD_WRITE32(tvd_base[channel_id] + 0x894, 0x0020A000); 
	TVD_WRITE32(tvd_base[channel_id] + 0x898, 0x00000000); 
	TVD_WRITE32(tvd_base[channel_id] + 0x89C, 0x00606200); 
	TVD_WRITE32(tvd_base[channel_id] + 0x8A0, 0xC000C000); 
	TVD_WRITE32(tvd_base[channel_id] + 0x8A4, 0x00000000); 
	TVD_WRITE32(tvd_base[channel_id] + 0x8A8, 0x0000000F); 
	TVD_WRITE32(tvd_base[channel_id] + 0x8AC, 0x00000000); 
	TVD_WRITE32(tvd_base[channel_id] + 0x8B0, 0x00000000); 
	TVD_WRITE32(tvd_base[channel_id] + 0x8B4, 0x00000078); 
	TVD_WRITE32(tvd_base[channel_id] + 0x8B8, 0xC0016C01); 
	TVD_WRITE32(tvd_base[channel_id] + 0x8BC, 0x6EC86EC8); 
	TVD_WRITE32(tvd_base[channel_id] + 0x8C0, 0x67049E04); 
	TVD_WRITE32(tvd_base[channel_id] + 0x8C4, 0x120845A0); 
	TVD_WRITE32(tvd_base[channel_id] + 0x8C8, 0x0185C501); 
	TVD_WRITE32(tvd_base[channel_id] + 0x8CC, 0xC1CBA860); 
	TVD_WRITE32(tvd_base[channel_id] + 0x8D0, 0x70000BB8); 
	TVD_WRITE32(tvd_base[channel_id] + 0x8D4, 0x7F294A52); 
	TVD_WRITE32(tvd_base[channel_id] + 0x8D8, 0x83C28F00); 
	TVD_WRITE32(tvd_base[channel_id] + 0x8DC, 0x74D08610); 
	TVD_WRITE32(tvd_base[channel_id] + 0x8E0, 0x195D1CCD); 
	TVD_WRITE32(tvd_base[channel_id] + 0x8E4, 0x413B0231); 
	TVD_WRITE32(tvd_base[channel_id] + 0x8E8, 0x0440283B); 
	TVD_WRITE32(tvd_base[channel_id] + 0x8EC, 0xF3935CD6); 
	TVD_WRITE32(tvd_base[channel_id] + 0x8F0, 0x532714C2); 
	TVD_WRITE32(tvd_base[channel_id] + 0x8F4, 0x67184854); 
	TVD_WRITE32(tvd_base[channel_id] + 0x8F8, 0x80000000); 
	TVD_WRITE32(tvd_base[channel_id] + 0x8FC, 0x00000000); 
	TVD_WRITE32(tvd_base[channel_id] + 0x900, 0x62F32005); 
	TVD_WRITE32(tvd_base[channel_id] + 0x904, 0x120855A0); 
	TVD_WRITE32(tvd_base[channel_id] + 0x908, 0x0185D0B9); 
	TVD_WRITE32(tvd_base[channel_id] + 0x90C, 0xC1CAA800); 
	TVD_WRITE32(tvd_base[channel_id] + 0x910, 0x70000BB8); 
	TVD_WRITE32(tvd_base[channel_id] + 0x914, 0x7F29CE73); 
	TVD_WRITE32(tvd_base[channel_id] + 0x918, 0x83C28F00); 
	TVD_WRITE32(tvd_base[channel_id] + 0x91C, 0x74D08610); 
	TVD_WRITE32(tvd_base[channel_id] + 0x920, 0x195D1CCD); 
	TVD_WRITE32(tvd_base[channel_id] + 0x924, 0x413B0252); 
	TVD_WRITE32(tvd_base[channel_id] + 0x928, 0x0440283B); 
	TVD_WRITE32(tvd_base[channel_id] + 0x92C, 0xF403DCF7); 
	TVD_WRITE32(tvd_base[channel_id] + 0x930, 0x532714C2); 
	TVD_WRITE32(tvd_base[channel_id] + 0x934, 0x87184814); 
	TVD_WRITE32(tvd_base[channel_id] + 0x938, 0x08000800); 
	TVD_WRITE32(tvd_base[channel_id] + 0x93C, 0x00000800); 
	TVD_WRITE32(tvd_base[channel_id] + 0x940, 0x10281400); 
	TVD_WRITE32(tvd_base[channel_id] + 0x944, 0x04D0BB40); 
	TVD_WRITE32(tvd_base[channel_id] + 0x948, 0x10281400); 
	TVD_WRITE32(tvd_base[channel_id] + 0x94C, 0x04D0BB40); 
	TVD_WRITE32(tvd_base[channel_id] + 0x950, 0x00000000); 
	TVD_WRITE32(tvd_base[channel_id] + 0x954, 0x00000000); 
	TVD_WRITE32(tvd_base[channel_id] + 0x958, 0x00000000); 
	TVD_WRITE32(tvd_base[channel_id] + 0x95C, 0x00000000); 
	TVD_WRITE32(tvd_base[channel_id] + 0x960, 0x00000000); 
	TVD_WRITE32(tvd_base[channel_id] + 0x964, 0x00000000); 
	TVD_WRITE32(tvd_base[channel_id] + 0x968, 0x00000000); 
	TVD_WRITE32(tvd_base[channel_id] + 0x96C, 0x00000000); 
	TVD_WRITE32(tvd_base[channel_id] + 0x970, 0x00000000); 
	TVD_WRITE32(tvd_base[channel_id] + 0x974, 0x00000000); 
	TVD_WRITE32(tvd_base[channel_id] + 0x978, 0x00000000); 
	TVD_WRITE32(tvd_base[channel_id] + 0x97C, 0x00000000); 
	TVD_WRITE32(tvd_base[channel_id] + 0x980, 0x00000000); 
	TVD_WRITE32(tvd_base[channel_id] + 0x984, 0x00000000); 
	TVD_WRITE32(tvd_base[channel_id] + 0x988, 0x00000000); 
	TVD_WRITE32(tvd_base[channel_id] + 0x98C, 0x00000000); 
	TVD_WRITE32(tvd_base[channel_id] + 0x990, 0x00000000); 
	TVD_WRITE32(tvd_base[channel_id] + 0x994, 0x00000000); 
	TVD_WRITE32(tvd_base[channel_id] + 0x998, 0x00000000); 
	TVD_WRITE32(tvd_base[channel_id] + 0x99C, 0x00000000); 
	TVD_WRITE32(tvd_base[channel_id] + 0x9A0, 0x00000000); 
	TVD_WRITE32(tvd_base[channel_id] + 0x9A4, 0x00000000); 
	TVD_WRITE32(tvd_base[channel_id] + 0x9A8, 0x00000000); 
	TVD_WRITE32(tvd_base[channel_id] + 0x9AC, 0x00000000); 
	TVD_WRITE32(tvd_base[channel_id] + 0x9B0, 0x00000000); 
	TVD_WRITE32(tvd_base[channel_id] + 0x9B4, 0x00000000); 
	TVD_WRITE32(tvd_base[channel_id] + 0x9B8, 0x00000000); 
	TVD_WRITE32(tvd_base[channel_id] + 0x9BC, 0x00000000); 
	TVD_WRITE32(tvd_base[channel_id] + 0x9C0, 0x00000000); 
	TVD_WRITE32(tvd_base[channel_id] + 0x9C4, 0x00000000); 
	TVD_WRITE32(tvd_base[channel_id] + 0x9C8, 0x00000000); 
	TVD_WRITE32(tvd_base[channel_id] + 0x9CC, 0x00000000); 
	TVD_WRITE32(tvd_base[channel_id] + 0x9D0, 0x00000000); 
	TVD_WRITE32(tvd_base[channel_id] + 0x9D4, 0x00000000); 
	TVD_WRITE32(tvd_base[channel_id] + 0x9D8, 0x00000000); 
	TVD_WRITE32(tvd_base[channel_id] + 0x9DC, 0x00000000); 
	TVD_WRITE32(tvd_base[channel_id] + 0x9E0, 0x00000000); 
	TVD_WRITE32(tvd_base[channel_id] + 0x9E4, 0x00000000); 
	TVD_WRITE32(tvd_base[channel_id] + 0x9E8, 0x00000000); 
	TVD_WRITE32(tvd_base[channel_id] + 0x9EC, 0x00000000); 
	TVD_WRITE32(tvd_base[channel_id] + 0x9F0, 0x00000000); 
	TVD_WRITE32(tvd_base[channel_id] + 0x9F4, 0x00000000); 
	TVD_WRITE32(tvd_base[channel_id] + 0x9F8, 0x00000000); 
	TVD_WRITE32(tvd_base[channel_id] + 0x9FC, 0x00000000); 
	TVD_WRITE32(tvd_base[channel_id] + 0xA00, 0x00000000); 
	TVD_WRITE32(tvd_base[channel_id] + 0xA04, 0x00000000); 
	TVD_WRITE32(tvd_base[channel_id] + 0xA08, 0x00000000); 
	TVD_WRITE32(tvd_base[channel_id] + 0xA0C, 0x00000000); 
	TVD_WRITE32(tvd_base[channel_id] + 0xA10, 0x00000000); 
	TVD_WRITE32(tvd_base[channel_id] + 0xA14, 0x00000000); 
	TVD_WRITE32(tvd_base[channel_id] + 0xA18, 0x00000000); 
	TVD_WRITE32(tvd_base[channel_id] + 0xA1C, 0x00000000); 
	TVD_WRITE32(tvd_base[channel_id] + 0xA20, 0x00000000); 
	TVD_WRITE32(tvd_base[channel_id] + 0xA24, 0x00000000); 
	TVD_WRITE32(tvd_base[channel_id] + 0xA28, 0x00000000); 
	TVD_WRITE32(tvd_base[channel_id] + 0xA2C, 0x00000000); 
	TVD_WRITE32(tvd_base[channel_id] + 0xA30, 0x00000000); 
	TVD_WRITE32(tvd_base[channel_id] + 0xA34, 0x00000000); 
	TVD_WRITE32(tvd_base[channel_id] + 0xA38, 0x00000000); 
	TVD_WRITE32(tvd_base[channel_id] + 0xA3C, 0x00000000); 
	TVD_WRITE32(tvd_base[channel_id] + 0xA40, 0x00000000); 
	TVD_WRITE32(tvd_base[channel_id] + 0xA44, 0x00000000); 
	TVD_WRITE32(tvd_base[channel_id] + 0xA48, 0x00000000); 
	TVD_WRITE32(tvd_base[channel_id] + 0xA4C, 0x00000000); 
	TVD_WRITE32(tvd_base[channel_id] + 0xA50, 0x00000000); 
	TVD_WRITE32(tvd_base[channel_id] + 0xA54, 0x00000000); 
	TVD_WRITE32(tvd_base[channel_id] + 0xA58, 0x00000000); 
	TVD_WRITE32(tvd_base[channel_id] + 0xA5C, 0x00000000); 
	TVD_WRITE32(tvd_base[channel_id] + 0xA60, 0x00000000); 
	TVD_WRITE32(tvd_base[channel_id] + 0xA64, 0x00000000); 
	TVD_WRITE32(tvd_base[channel_id] + 0xA68, 0x00000000); 
	TVD_WRITE32(tvd_base[channel_id] + 0xA6C, 0x00000000); 
	TVD_WRITE32(tvd_base[channel_id] + 0xA70, 0x00000000); 
	TVD_WRITE32(tvd_base[channel_id] + 0xA74, 0x00000000); 
	TVD_WRITE32(tvd_base[channel_id] + 0xA78, 0x00000000); 
	TVD_WRITE32(tvd_base[channel_id] + 0xA7C, 0x00000000); 
	TVD_WRITE32(tvd_base[channel_id] + 0xA80, 0x0FF3FCFF); 
	TVD_WRITE32(tvd_base[channel_id] + 0xA84, 0x80008000); 
	TVD_WRITE32(tvd_base[channel_id] + 0xA88, 0x80000000); 
	TVD_WRITE32(tvd_base[channel_id] + 0xA8C, 0x00000000); 
	TVD_WRITE32(tvd_base[channel_id] + 0xA90, 0x00000000); 
	TVD_WRITE32(tvd_base[channel_id] + 0xA94, 0x00000000); 
	TVD_WRITE32(tvd_base[channel_id] + 0xA98, 0x00000000); 
	TVD_WRITE32(tvd_base[channel_id] + 0xA9C, 0x00000000); 
	TVD_WRITE32(tvd_base[channel_id] + 0xAA0, 0x00000000); 
	TVD_WRITE32(tvd_base[channel_id] + 0xAA4, 0x00000000); 
	TVD_WRITE32(tvd_base[channel_id] + 0xAA8, 0x00000000); 
	TVD_WRITE32(tvd_base[channel_id] + 0xAAC, 0x00000000); 
	TVD_WRITE32(tvd_base[channel_id] + 0xAB0, 0x22F08C03); 
	TVD_WRITE32(tvd_base[channel_id] + 0xAB4, 0x8F0070D4); 
	TVD_WRITE32(tvd_base[channel_id] + 0xAB8, 0x42400800); 
	TVD_WRITE32(tvd_base[channel_id] + 0xABC, 0x82000820); 
	
	TVD_WRITE32(tvd_base[channel_id] + 0x5aC, 0x000010C0);  //bit 31
	TVD_WRITE32(tvd_base[channel_id] + 0x59C, 0x21E2c0B2); //H start
	TVD_WRITE32(tvd_base[channel_id] + 0x5c4, 0x18181617);  //v start


	return;
}

void vPAL_M_2D_Setting(TVD_CHANNEL_ID channel_id)
{

	TVD_WRITE32(tvd_base[channel_id] + 0x410, 0x00818000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x414, 0xFF080020);  
	TVD_WRITE32(tvd_base[channel_id] + 0x418, 0x10000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x41C, 0x12400010);  
	TVD_WRITE32(tvd_base[channel_id] + 0x420, 0x22788207);  
	TVD_WRITE32(tvd_base[channel_id] + 0x424, 0x000000FF);  
	TVD_WRITE32(tvd_base[channel_id] + 0x428, 0xFFFFFFFF);  
	TVD_WRITE32(tvd_base[channel_id] + 0x42C, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x430, 0x00000000);  

	TVD_WRITE32(tvd_base[channel_id] + 0x434, 0xC4D00101);  

	TVD_WRITE32(tvd_base[channel_id] + 0x438, 0xB801404A);  
	TVD_WRITE32(tvd_base[channel_id] + 0x43C, 0x00800801);  
	TVD_WRITE32(tvd_base[channel_id] + 0x440, 0x00D01001);  
	TVD_WRITE32(tvd_base[channel_id] + 0x444, 0x1E000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x448, 0x80000114);  
	TVD_WRITE32(tvd_base[channel_id] + 0x44C, 0x2AAA8103);  
	TVD_WRITE32(tvd_base[channel_id] + 0x450, 0xF0000100);  
	TVD_WRITE32(tvd_base[channel_id] + 0x454, 0x1E000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x458, 0x80000114);  
	TVD_WRITE32(tvd_base[channel_id] + 0x45C, 0x2AAA8103);  
	TVD_WRITE32(tvd_base[channel_id] + 0x460, 0x47B00000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x464, 0x1E000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x468, 0x80000114);  
	TVD_WRITE32(tvd_base[channel_id] + 0x46C, 0x2AAA8103);  
	TVD_WRITE32(tvd_base[channel_id] + 0x470, 0x1100001B);  
	TVD_WRITE32(tvd_base[channel_id] + 0x474, 0xAAA04EAB);  
	TVD_WRITE32(tvd_base[channel_id] + 0x478, 0x0888AAAA);  
	TVD_WRITE32(tvd_base[channel_id] + 0x47C, 0x00050000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x480, 0x003E313D);  
	TVD_WRITE32(tvd_base[channel_id] + 0x484, 0x100F0480);  
	//TVD_WRITE32(0x488,0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x48C, 0x0200022D);  
	TVD_WRITE32(tvd_base[channel_id] + 0x490, 0x0AFFC588);  
	TVD_WRITE32(tvd_base[channel_id] + 0x494, 0x08000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x498, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x49C, 0x0009C000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x4A0, 0x00090000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x4A4, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x4A8, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x4AC, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x4B0, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x4B4, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x4B8, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x4BC, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x4C0, 0x51515C55);  
	TVD_WRITE32(tvd_base[channel_id] + 0x4C4, 0x04557845);  
	TVD_WRITE32(tvd_base[channel_id] + 0x4C8, 0x27F7595E);  
	TVD_WRITE32(tvd_base[channel_id] + 0x4CC, 0x8F340380);  
	TVD_WRITE32(tvd_base[channel_id] + 0x4D0, 0x158C0F80);  
	TVD_WRITE32(tvd_base[channel_id] + 0x4D4, 0x26144040);  
	TVD_WRITE32(tvd_base[channel_id] + 0x4D8, 0x0A00F600);  
	TVD_WRITE32(tvd_base[channel_id] + 0x4DC, 0xF41F0800);  
	TVD_WRITE32(tvd_base[channel_id] + 0x4E0, 0x13178080);  
	TVD_WRITE32(tvd_base[channel_id] + 0x4E4, 0x8AA7075E);  
	TVD_WRITE32(tvd_base[channel_id] + 0x4E8, 0xF08400A0);  
	TVD_WRITE32(tvd_base[channel_id] + 0x4EC, 0x08180C40);  
	TVD_WRITE32(tvd_base[channel_id] + 0x4F0, 0x2008C2C9);  
	TVD_WRITE32(tvd_base[channel_id] + 0x4F4, 0x7180001F);  
	TVD_WRITE32(tvd_base[channel_id] + 0x4F8, 0x8000306C);  
	TVD_WRITE32(tvd_base[channel_id] + 0x4FC, 0x420C33A0);  
	TVD_WRITE32(tvd_base[channel_id] + 0x500, 0x6838FA27);  
	TVD_WRITE32(tvd_base[channel_id] + 0x504, 0x0DFA2090);  
	TVD_WRITE32(tvd_base[channel_id] + 0x508, 0x08E06B40);  
	TVD_WRITE32(tvd_base[channel_id] + 0x50C, 0x83608016);  
	TVD_WRITE32(tvd_base[channel_id] + 0x510, 0x40204080);  
	TVD_WRITE32(tvd_base[channel_id] + 0x514, 0x80326485);  
	TVD_WRITE32(tvd_base[channel_id] + 0x518, 0x11A0A04C);  
	TVD_WRITE32(tvd_base[channel_id] + 0x51C, 0x8040A0C0);  
	TVD_WRITE32(tvd_base[channel_id] + 0x520, 0x7E35AF80);  
	TVD_WRITE32(tvd_base[channel_id] + 0x524, 0x0408F840);  
	TVD_WRITE32(tvd_base[channel_id] + 0x528, 0x5404E030);  
	TVD_WRITE32(tvd_base[channel_id] + 0x52C, 0x30900AA0);  
	TVD_WRITE32(tvd_base[channel_id] + 0x530, 0x0B410B09);  
	TVD_WRITE32(tvd_base[channel_id] + 0x534, 0xF43E40F0);  
	TVD_WRITE32(tvd_base[channel_id] + 0x538, 0x452414F0);  
	TVD_WRITE32(tvd_base[channel_id] + 0x53C, 0x55C29325);  
	TVD_WRITE32(tvd_base[channel_id] + 0x540, 0x30173E63);  
	TVD_WRITE32(tvd_base[channel_id] + 0x544, 0x21A41725);  
	TVD_WRITE32(tvd_base[channel_id] + 0x548, 0x30B080FE);  
	TVD_WRITE32(tvd_base[channel_id] + 0x54C, 0x1245482B);  
	TVD_WRITE32(tvd_base[channel_id] + 0x550, 0xB4000023);  
	TVD_WRITE32(tvd_base[channel_id] + 0x554, 0x04AE0318);  
	TVD_WRITE32(tvd_base[channel_id] + 0x558, 0x106443D0);  
	TVD_WRITE32(tvd_base[channel_id] + 0x55C, 0x57856973);  
	TVD_WRITE32(tvd_base[channel_id] + 0x560, 0x29E518EF);  
	TVD_WRITE32(tvd_base[channel_id] + 0x564, 0x2010573D);  
	TVD_WRITE32(tvd_base[channel_id] + 0x568, 0x204CDEC9);  
	TVD_WRITE32(tvd_base[channel_id] + 0x56C, 0x204A9881);  
	TVD_WRITE32(tvd_base[channel_id] + 0x570, 0xFF29DEC5);  
	TVD_WRITE32(tvd_base[channel_id] + 0x574, 0xCA8B0F00);  
	TVD_WRITE32(tvd_base[channel_id] + 0x578, 0x797A8E61);  
	TVD_WRITE32(tvd_base[channel_id] + 0x57C, 0x420FA8A1);  
	TVD_WRITE32(tvd_base[channel_id] + 0x580, 0x208181D4);  
	TVD_WRITE32(tvd_base[channel_id] + 0x584, 0x28AF88CA);  
	TVD_WRITE32(tvd_base[channel_id] + 0x588, 0xFC844040);  
	TVD_WRITE32(tvd_base[channel_id] + 0x58C, 0x2854C531);  
	TVD_WRITE32(tvd_base[channel_id] + 0x590, 0x0206E61A);  
	TVD_WRITE32(tvd_base[channel_id] + 0x594, 0x4840310E);  
	TVD_WRITE32(tvd_base[channel_id] + 0x598, 0x21594088);  
	TVD_WRITE32(tvd_base[channel_id] + 0x59C, 0x29E2F0B2);  
	TVD_WRITE32(tvd_base[channel_id] + 0x5A0, 0x44800C80);  
	TVD_WRITE32(tvd_base[channel_id] + 0x5A4, 0x20010000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x5A8, 0x0A4BC680);  
	TVD_WRITE32(tvd_base[channel_id] + 0x5AC, 0x400010C0);  
	TVD_WRITE32(tvd_base[channel_id] + 0x5B0, 0x20008400);  
	TVD_WRITE32(tvd_base[channel_id] + 0x5B4, 0x00F9BC00);  
	TVD_WRITE32(tvd_base[channel_id] + 0x5B8, 0x20002004);  
	TVD_WRITE32(tvd_base[channel_id] + 0x5BC, 0x17E0114F);  
	TVD_WRITE32(tvd_base[channel_id] + 0x5C0, 0x1F908420);  
	TVD_WRITE32(tvd_base[channel_id] + 0x5C4, 0x1A191515);  
	TVD_WRITE32(tvd_base[channel_id] + 0x5C8, 0x06658819);  
	TVD_WRITE32(tvd_base[channel_id] + 0x5CC, 0x72B91AB9);  
	TVD_WRITE32(tvd_base[channel_id] + 0x5D0, 0x5A1640DB);  
	TVD_WRITE32(tvd_base[channel_id] + 0x5D4, 0x55F401F4);  
	TVD_WRITE32(tvd_base[channel_id] + 0x5D8, 0x100181FA);  
	TVD_WRITE32(tvd_base[channel_id] + 0x5DC, 0x60506E60);  
	TVD_WRITE32(tvd_base[channel_id] + 0x5E0, 0xA0A793A6);  
	TVD_WRITE32(tvd_base[channel_id] + 0x5E4, 0xFAC9D1A0);  
	TVD_WRITE32(tvd_base[channel_id] + 0x5E8, 0x7CB83BD4);  
	TVD_WRITE32(tvd_base[channel_id] + 0x5EC, 0x80DD43A9);  
	TVD_WRITE32(tvd_base[channel_id] + 0x5F0, 0x80554329);  
	TVD_WRITE32(tvd_base[channel_id] + 0x5F4, 0x1748A14B);  
	TVD_WRITE32(tvd_base[channel_id] + 0x5F8, 0x50E3C67F);  
	TVD_WRITE32(tvd_base[channel_id] + 0x5FC, 0x70D70480);  
	TVD_WRITE32(tvd_base[channel_id] + 0x600, 0x3D34035F);  
	TVD_WRITE32(tvd_base[channel_id] + 0x604, 0xF9228320);  
	TVD_WRITE32(tvd_base[channel_id] + 0x608, 0x32201501);  
	TVD_WRITE32(tvd_base[channel_id] + 0x60C, 0x5985A833);  
	TVD_WRITE32(tvd_base[channel_id] + 0x610, 0x000028AD);  
	TVD_WRITE32(tvd_base[channel_id] + 0x614, 0x55E86B73);  
	TVD_WRITE32(tvd_base[channel_id] + 0x618, 0xE6CB2224);  
	TVD_WRITE32(tvd_base[channel_id] + 0x61C, 0x84460080);  
	TVD_WRITE32(tvd_base[channel_id] + 0x620, 0x00400000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x624, 0x7F084330);  
	TVD_WRITE32(tvd_base[channel_id] + 0x628, 0x08880001);  
	TVD_WRITE32(tvd_base[channel_id] + 0x62C, 0xBF488488);  
	TVD_WRITE32(tvd_base[channel_id] + 0x630, 0x0119812C);  
	TVD_WRITE32(tvd_base[channel_id] + 0x634, 0x01000844);  
	TVD_WRITE32(tvd_base[channel_id] + 0x638, 0x88020818);  
	TVD_WRITE32(tvd_base[channel_id] + 0x63C, 0x0FF0AA1F);  
	TVD_WRITE32(tvd_base[channel_id] + 0x640, 0x000B101F); 
	TVD_WRITE32(tvd_base[channel_id] + 0x644, 0x89A2F600);  
	TVD_WRITE32(tvd_base[channel_id] + 0x648, 0x8A045AF9);  
	TVD_WRITE32(tvd_base[channel_id] + 0x64C, 0x27030203); 
	TVD_WRITE32(tvd_base[channel_id] + 0x650, 0x02F40000); 
	TVD_WRITE32(tvd_base[channel_id] + 0x654, 0x29034404); 
	TVD_WRITE32(tvd_base[channel_id] + 0x658, 0x01C30A0C); 
	TVD_WRITE32(tvd_base[channel_id] + 0x65C, 0x01FA1600); 
	TVD_WRITE32(tvd_base[channel_id] + 0x660, 0x0111D107); 
	TVD_WRITE32(tvd_base[channel_id] + 0x664, 0x38D2F80B); 
	TVD_WRITE32(tvd_base[channel_id] + 0x668, 0x0A074829); 
	TVD_WRITE32(tvd_base[channel_id] + 0x66C, 0x20074829); 
	TVD_WRITE32(tvd_base[channel_id] + 0x670, 0x00000000); 
	TVD_WRITE32(tvd_base[channel_id] + 0x674, 0x00000007); 
	TVD_WRITE32(tvd_base[channel_id] + 0x678, 0x4C4C4C4C);  
	TVD_WRITE32(tvd_base[channel_id] + 0x67C, 0x4A101030); 
	TVD_WRITE32(tvd_base[channel_id] + 0x680, 0x480A1E2A);  
	TVD_WRITE32(tvd_base[channel_id] + 0x684, 0x7018040C);  
	TVD_WRITE32(tvd_base[channel_id] + 0x688, 0xC896321E);  
	TVD_WRITE32(tvd_base[channel_id] + 0x68C, 0x00107023);  
	TVD_WRITE32(tvd_base[channel_id] + 0x690, 0x0F00EF85);  
	TVD_WRITE32(tvd_base[channel_id] + 0x694, 0x0070500C);  
	TVD_WRITE32(tvd_base[channel_id] + 0x698, 0x00641400);  
	TVD_WRITE32(tvd_base[channel_id] + 0x69C, 0x00300500);  
	TVD_WRITE32(tvd_base[channel_id] + 0x6A0, 0x0000A00A);  
	TVD_WRITE32(tvd_base[channel_id] + 0x6A4, 0x1E702808);  
	TVD_WRITE32(tvd_base[channel_id] + 0x6A8, 0x80064078);  
	TVD_WRITE32(tvd_base[channel_id] + 0x6AC, 0x54280504);  
	TVD_WRITE32(tvd_base[channel_id] + 0x6B0, 0x20100000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x6B4, 0x00969600);  
	TVD_WRITE32(tvd_base[channel_id] + 0x6B8, 0x04044444);  
	TVD_WRITE32(tvd_base[channel_id] + 0x6BC, 0x0C010444);  
	TVD_WRITE32(tvd_base[channel_id] + 0x6C0, 0x00A444F3);  
	TVD_WRITE32(tvd_base[channel_id] + 0x6C4, 0x3E1A5353);  
	TVD_WRITE32(tvd_base[channel_id] + 0x6C8, 0x064B1478);  
	TVD_WRITE32(tvd_base[channel_id] + 0x6CC, 0x1020D831);  
	TVD_WRITE32(tvd_base[channel_id] + 0x6D0, 0x11111111);  
	TVD_WRITE32(tvd_base[channel_id] + 0x6D4, 0xC008FC10);  
	TVD_WRITE32(tvd_base[channel_id] + 0x6D8, 0x00000020);  
	TVD_WRITE32(tvd_base[channel_id] + 0x6DC, 0xC0001020);  
	TVD_WRITE32(tvd_base[channel_id] + 0x6E0, 0x0F371858);  
	TVD_WRITE32(tvd_base[channel_id] + 0x6E4, 0x3220008F);  
	TVD_WRITE32(tvd_base[channel_id] + 0x6E8, 0xB0210E28);  
	TVD_WRITE32(tvd_base[channel_id] + 0x6EC, 0x00000014);  
	TVD_WRITE32(tvd_base[channel_id] + 0x6F0, 0x43174537);  
	TVD_WRITE32(tvd_base[channel_id] + 0x6F4, 0x0045713C);  
	TVD_WRITE32(tvd_base[channel_id] + 0x6F8, 0xC0001A50);  
	TVD_WRITE32(tvd_base[channel_id] + 0x6FC, 0x4414050D);  
	TVD_WRITE32(tvd_base[channel_id] + 0x700, 0x11A00020);  
	TVD_WRITE32(tvd_base[channel_id] + 0x704, 0x64000450);  
	TVD_WRITE32(tvd_base[channel_id] + 0x708, 0x410A0028);  
	TVD_WRITE32(tvd_base[channel_id] + 0x70C, 0x00014455);  
	TVD_WRITE32(tvd_base[channel_id] + 0x710, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x714, 0x31581A40);
	TVD_WRITE32(tvd_base[channel_id] + 0x718, 0x849F0000);
	TVD_WRITE32(tvd_base[channel_id] + 0x71C, 0x08001020);
	TVD_WRITE32(tvd_base[channel_id] + 0x720, 0x08081E14);
	TVD_WRITE32(tvd_base[channel_id] + 0x724, 0x0A100008);
	TVD_WRITE32(tvd_base[channel_id] + 0x728, 0x50061608);
	TVD_WRITE32(tvd_base[channel_id] + 0x72C, 0x0C0A2808);
	TVD_WRITE32(tvd_base[channel_id] + 0x730, 0x0A149000);
	TVD_WRITE32(tvd_base[channel_id] + 0x734, 0x55000019);
	TVD_WRITE32(tvd_base[channel_id] + 0x738, 0x0311041A);
	TVD_WRITE32(tvd_base[channel_id] + 0x73C, 0x8C0A100A);
	TVD_WRITE32(tvd_base[channel_id] + 0x740, 0x01245678);
	TVD_WRITE32(tvd_base[channel_id] + 0x744, 0x02345678);
	TVD_WRITE32_MASK(tvd_base[channel_id] + 0x748,0x00080014, 0xCFFFFFFF);
	TVD_WRITE32(tvd_base[channel_id] + 0x74C, 0x00020A34);
	TVD_WRITE32(tvd_base[channel_id] + 0x750, 0x047C0558);
	TVD_WRITE32(tvd_base[channel_id] + 0x754, 0x0088011A);
	TVD_WRITE32(tvd_base[channel_id] + 0x758, 0x00009666);
	TVD_WRITE32(tvd_base[channel_id] + 0x75C, 0x000781FF);
	TVD_WRITE32(tvd_base[channel_id] + 0x760, 0x90000000);
	TVD_WRITE32(tvd_base[channel_id] + 0x764, 0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0x768, 0x0F051818);
	TVD_WRITE32(tvd_base[channel_id] + 0x76C, 0x00191014);
	TVD_WRITE32(tvd_base[channel_id] + 0x770, 0x23645050);
	TVD_WRITE32(tvd_base[channel_id] + 0x774, 0x64641432);
	TVD_WRITE32(tvd_base[channel_id] + 0x778, 0x1E519696);
	TVD_WRITE32(tvd_base[channel_id] + 0x77C, 0x10169696);
	TVD_WRITE32(tvd_base[channel_id] + 0x780, 0x053C501E);
	TVD_WRITE32(tvd_base[channel_id] + 0x784, 0x820A325A);
	TVD_WRITE32(tvd_base[channel_id] + 0x788, 0xFFFF3359);  
	TVD_WRITE32(tvd_base[channel_id] + 0x78C, 0xFFFFFFFF);
	TVD_WRITE32(tvd_base[channel_id] + 0x790, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x794, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x798, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x79C, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x7A0, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x7A4, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x7A8, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x7AC, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x7B0, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x7B4, 0x100181FA);  
	TVD_WRITE32(tvd_base[channel_id] + 0x7B8, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x7BC, 0x00000000); 

	TVD_WRITE32(tvd_base[channel_id] + 0x7D8, 0x020F42D0);  
	TVD_WRITE32(tvd_base[channel_id] + 0x7DC, 0x00000008);  
	TVD_WRITE32(tvd_base[channel_id] + 0x7E0, 0x0F0000A0);  
	TVD_WRITE32(tvd_base[channel_id] + 0x7E4, 0x11000020);  
	TVD_WRITE32(tvd_base[channel_id] + 0x7E8, 0xC000C000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x7EC, 0x00CC0C50);  
	TVD_WRITE32(tvd_base[channel_id] + 0x7F0, 0x80008000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x7F4, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x7F8, 0x80000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x7FC, 0xD0090901);  
	TVD_WRITE32(tvd_base[channel_id] + 0x800, 0x00060604);  
	TVD_WRITE32(tvd_base[channel_id] + 0x804, 0x0B9000FF);  
	TVD_WRITE32(tvd_base[channel_id] + 0x808, 0x001400FC);  
	TVD_WRITE32(tvd_base[channel_id] + 0x80C, 0x005071AD);  
	TVD_WRITE32(tvd_base[channel_id] + 0x810, 0xC3080050);  
	TVD_WRITE32(tvd_base[channel_id] + 0x814, 0x0F120700);  
	TVD_WRITE32(tvd_base[channel_id] + 0x818, 0x02100578);  
	TVD_WRITE32(tvd_base[channel_id] + 0x81C, 0x65004020);  
	TVD_WRITE32(tvd_base[channel_id] + 0x820, 0x0000F00F);  
	TVD_WRITE32(tvd_base[channel_id] + 0x824, 0x14140F0F);  
	TVD_WRITE32(tvd_base[channel_id] + 0x828, 0x0A0A0A0A);  
	TVD_WRITE32(tvd_base[channel_id] + 0x82C, 0x77700000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x830, 0x00A0A000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x834, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x838, 0x4400A00A);  
	TVD_WRITE32(tvd_base[channel_id] + 0x83C, 0x14000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x840, 0x00003010);  
	TVD_WRITE32(tvd_base[channel_id] + 0x844, 0x00201007);  
	TVD_WRITE32(tvd_base[channel_id] + 0x848, 0x00201007);  
	TVD_WRITE32(tvd_base[channel_id] + 0x84C, 0x0700100A);  
	TVD_WRITE32(tvd_base[channel_id] + 0x850, 0x00208858);  
	TVD_WRITE32(tvd_base[channel_id] + 0x854, 0x14001010);  
	TVD_WRITE32(tvd_base[channel_id] + 0x858, 0x00141414);  
	TVD_WRITE32(tvd_base[channel_id] + 0x85C, 0x10141414);  
	TVD_WRITE32(tvd_base[channel_id] + 0x860, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x864, 0x00010000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x868, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x86C, 0xC0400C0A);  
	TVD_WRITE32(tvd_base[channel_id] + 0x870, 0x00020820);  
	TVD_WRITE32(tvd_base[channel_id] + 0x874, 0xC0016C01);  
	TVD_WRITE32(tvd_base[channel_id] + 0x878, 0x0089A022);  
	TVD_WRITE32(tvd_base[channel_id] + 0x87C, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x880, 0x00060604);  
	TVD_WRITE32(tvd_base[channel_id] + 0x884, 0x000001FC);  
	TVD_WRITE32(tvd_base[channel_id] + 0x888, 0x005071AD);  
	TVD_WRITE32(tvd_base[channel_id] + 0x88C, 0x00002010);  
	TVD_WRITE32(tvd_base[channel_id] + 0x890, 0x0008F060);  
	TVD_WRITE32(tvd_base[channel_id] + 0x894, 0x0020A000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x898, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x89C, 0x00606200);  
	TVD_WRITE32(tvd_base[channel_id] + 0x8A0, 0xC000C000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x8A4, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x8A8, 0x0000000F);  
	TVD_WRITE32(tvd_base[channel_id] + 0x8AC, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x8B0, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x8B4, 0x00000078);  
	TVD_WRITE32(tvd_base[channel_id] + 0x8B8, 0xC0016C01);  
	TVD_WRITE32(tvd_base[channel_id] + 0x8BC, 0x6EC86EC8);  
	TVD_WRITE32(tvd_base[channel_id] + 0x8C0, 0x67049E04);  
	TVD_WRITE32(tvd_base[channel_id] + 0x8C4, 0x120845A0);  
	TVD_WRITE32(tvd_base[channel_id] + 0x8C8, 0x0185C501);  
	TVD_WRITE32(tvd_base[channel_id] + 0x8CC, 0xC1CBA860);  
	TVD_WRITE32(tvd_base[channel_id] + 0x8D0, 0x70000BB8);  
	TVD_WRITE32(tvd_base[channel_id] + 0x8D4, 0x7F294A52);  
	TVD_WRITE32(tvd_base[channel_id] + 0x8D8, 0x83C28F00);  
	TVD_WRITE32(tvd_base[channel_id] + 0x8DC, 0x74D08610);  
	TVD_WRITE32(tvd_base[channel_id] + 0x8E0, 0x195D1CCD);  
	TVD_WRITE32(tvd_base[channel_id] + 0x8E4, 0x413B0231);  
	TVD_WRITE32(tvd_base[channel_id] + 0x8E8, 0x0440283B);  
	TVD_WRITE32(tvd_base[channel_id] + 0x8EC, 0xF3935CD6);  
	TVD_WRITE32(tvd_base[channel_id] + 0x8F0, 0x532714C2);  
	TVD_WRITE32(tvd_base[channel_id] + 0x8F4, 0x67184854);  
	TVD_WRITE32(tvd_base[channel_id] + 0x8F8, 0x80000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x8FC, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x900, 0x62F32005);  
	TVD_WRITE32(tvd_base[channel_id] + 0x904, 0x120855A0);  
	TVD_WRITE32(tvd_base[channel_id] + 0x908, 0x0185D0B9);  
	TVD_WRITE32(tvd_base[channel_id] + 0x90C, 0xC1CAA800);  
	TVD_WRITE32(tvd_base[channel_id] + 0x910, 0x70000BB8);  
	TVD_WRITE32(tvd_base[channel_id] + 0x914, 0x7F29CE73);  
	TVD_WRITE32(tvd_base[channel_id] + 0x918, 0x83C28F00);  
	TVD_WRITE32(tvd_base[channel_id] + 0x91C, 0x74D08610);  
	TVD_WRITE32(tvd_base[channel_id] + 0x920, 0x195D1CCD);  
	TVD_WRITE32(tvd_base[channel_id] + 0x924, 0x413B0252);  
	TVD_WRITE32(tvd_base[channel_id] + 0x928, 0x0440283B);  
	TVD_WRITE32(tvd_base[channel_id] + 0x92C, 0xF403DCF7);  
	TVD_WRITE32(tvd_base[channel_id] + 0x930, 0x532714C2);  
	TVD_WRITE32(tvd_base[channel_id] + 0x934, 0x87184814);  
	TVD_WRITE32(tvd_base[channel_id] + 0x938, 0x08000800);  
	TVD_WRITE32(tvd_base[channel_id] + 0x93C, 0x00000800);  
	TVD_WRITE32(tvd_base[channel_id] + 0x940, 0x10281400);  
	TVD_WRITE32(tvd_base[channel_id] + 0x944, 0x04D0BB40);  
	TVD_WRITE32(tvd_base[channel_id] + 0x948, 0x10281400);  
	TVD_WRITE32(tvd_base[channel_id] + 0x94C, 0x04D0BB40);  
	TVD_WRITE32(tvd_base[channel_id] + 0x950, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x954, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x958, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x95C, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x960, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x964, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x968, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x96C, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x970, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x974, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x978, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x97C, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x980, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x984, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x988, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x98C, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x990, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x994, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x998, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x99C, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x9A0, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x9A4, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x9A8, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x9AC, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x9B0, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x9B4, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x9B8, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x9BC, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x9C0, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x9C4, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x9C8, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x9CC, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x9D0, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x9D4, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x9D8, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x9DC, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x9E0, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x9E4, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x9E8, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x9EC, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x9F0, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x9F4, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x9F8, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x9FC, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0xA00, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0xA04, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0xA08, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0xA0C, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0xA10, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0xA14, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0xA18, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0xA1C, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0xA20, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0xA24, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0xA28, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0xA2C, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0xA30, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0xA34, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0xA38, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0xA3C, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0xA40, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0xA44, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0xA48, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0xA4C, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0xA50, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0xA54, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0xA58, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0xA5C, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0xA60, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0xA64, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0xA68, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0xA6C, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0xA70, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0xA74, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0xA78, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0xA7C, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0xA80, 0x0FF3FCFF);  
	TVD_WRITE32(tvd_base[channel_id] + 0xA84, 0x80008000);  
	TVD_WRITE32(tvd_base[channel_id] + 0xA88, 0x80000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0xA8C, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0xA90, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0xA94, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0xA98, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0xA9C, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0xAA0, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0xAA4, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0xAA8, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0xAAC, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0xAB0, 0x22F08C03);  
	TVD_WRITE32(tvd_base[channel_id] + 0xAB4, 0x8F0070D4);  
	TVD_WRITE32(tvd_base[channel_id] + 0xAB8, 0x42400800);  
	TVD_WRITE32(tvd_base[channel_id] + 0xABC, 0x82000820);  

	TVD_WRITE32(tvd_base[channel_id] + 0x5aC, 0x000010C0);  //bit 31
	TVD_WRITE32(tvd_base[channel_id] + 0x59C, 0x21E2c0B2); //H start
	TVD_WRITE32(tvd_base[channel_id] + 0x5c4, 0x18181617);  //v start

	return;
}

void vPAL_N_2D_Setting(TVD_CHANNEL_ID channel_id)
{
	TVD_WRITE32(tvd_base[channel_id] + 0x410, 0x00818000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x414, 0xFF080020);  
	TVD_WRITE32(tvd_base[channel_id] + 0x418, 0x10000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x41C, 0x12400010);  
	TVD_WRITE32(tvd_base[channel_id] + 0x420, 0x22788207);  
	TVD_WRITE32(tvd_base[channel_id] + 0x424, 0x000000FF);  
	TVD_WRITE32(tvd_base[channel_id] + 0x428, 0xFFFFFFFF);  
	TVD_WRITE32(tvd_base[channel_id] + 0x42C, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x430, 0x00000000); 


	TVD_WRITE32(tvd_base[channel_id] + 0x434, 0xC4D00101);  

	TVD_WRITE32(tvd_base[channel_id] + 0x438, 0xB801404A);  
	TVD_WRITE32(tvd_base[channel_id] + 0x43C, 0x00800801);  
	TVD_WRITE32(tvd_base[channel_id] + 0x440, 0x00D01001);  
	TVD_WRITE32(tvd_base[channel_id] + 0x444, 0x1E000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x448, 0x80000114);  
	TVD_WRITE32(tvd_base[channel_id] + 0x44C, 0x2AAA8103);  
	TVD_WRITE32(tvd_base[channel_id] + 0x450, 0xF0000100);  
	TVD_WRITE32(tvd_base[channel_id] + 0x454, 0x1E000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x458, 0x80000114);  
	TVD_WRITE32(tvd_base[channel_id] + 0x45C, 0x2AAA8103);  
	TVD_WRITE32(tvd_base[channel_id] + 0x460, 0x47B00000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x464, 0x1E000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x468, 0x80000114);  
	TVD_WRITE32(tvd_base[channel_id] + 0x46C, 0x2AAA8103);  
	TVD_WRITE32(tvd_base[channel_id] + 0x470, 0x1100001B);  
	TVD_WRITE32(tvd_base[channel_id] + 0x474, 0xAAA04EAB);  
	TVD_WRITE32(tvd_base[channel_id] + 0x478, 0x0888AAAA);  
	TVD_WRITE32(tvd_base[channel_id] + 0x47C, 0x00050000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x480, 0x003E313D);  
	TVD_WRITE32(tvd_base[channel_id] + 0x484, 0x100F0480);  
	//TVD_WRITE32(0x488,0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x48C, 0x0200022D);  
	TVD_WRITE32(tvd_base[channel_id] + 0x490, 0x0AFFC588);  
	TVD_WRITE32(tvd_base[channel_id] + 0x494, 0x08000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x498, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x49C, 0x0009C000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x4A0, 0x00090000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x4A4, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x4A8, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x4AC, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x4B0, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x4B4, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x4B8, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x4BC, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x4C0, 0x51515C55);  
	TVD_WRITE32(tvd_base[channel_id] + 0x4C4, 0x04557845);  
	TVD_WRITE32(tvd_base[channel_id] + 0x4C8, 0x27F7595E);  
	TVD_WRITE32(tvd_base[channel_id] + 0x4CC, 0x8F3403A0);  
	TVD_WRITE32(tvd_base[channel_id] + 0x4D0, 0x15890F80);  
	TVD_WRITE32(tvd_base[channel_id] + 0x4D4, 0x26144040);  
	TVD_WRITE32(tvd_base[channel_id] + 0x4D8, 0x0A00F600);  
	TVD_WRITE32(tvd_base[channel_id] + 0x4DC, 0xF41F0800);  
	TVD_WRITE32(tvd_base[channel_id] + 0x4E0, 0x13178080);  
	TVD_WRITE32(tvd_base[channel_id] + 0x4E4, 0x8AA7075E);  
	TVD_WRITE32(tvd_base[channel_id] + 0x4E8, 0xF08400A0);  
	TVD_WRITE32(tvd_base[channel_id] + 0x4EC, 0x08180C40);  
	TVD_WRITE32(tvd_base[channel_id] + 0x4F0, 0x2008C2C9);  
	TVD_WRITE32(tvd_base[channel_id] + 0x4F4, 0x7180001F);  
	TVD_WRITE32(tvd_base[channel_id] + 0x4F8, 0x8000306C);  
	TVD_WRITE32(tvd_base[channel_id] + 0x4FC, 0x420C33A0);  
	TVD_WRITE32(tvd_base[channel_id] + 0x500, 0x6838FA27);  
	TVD_WRITE32(tvd_base[channel_id] + 0x504, 0x0DFA2090);  
	TVD_WRITE32(tvd_base[channel_id] + 0x508, 0x08E06B40);  
	TVD_WRITE32(tvd_base[channel_id] + 0x50C, 0x83608016);  
	TVD_WRITE32(tvd_base[channel_id] + 0x510, 0x40204080);  
	TVD_WRITE32(tvd_base[channel_id] + 0x514, 0x80326485);  
	TVD_WRITE32(tvd_base[channel_id] + 0x518, 0x11A0A04C);  
	TVD_WRITE32(tvd_base[channel_id] + 0x51C, 0x8040A0C0);  
	TVD_WRITE32(tvd_base[channel_id] + 0x520, 0x7E35AF80);  
	TVD_WRITE32(tvd_base[channel_id] + 0x524, 0x0408F840);  
	TVD_WRITE32(tvd_base[channel_id] + 0x528, 0x5404E030);  
	TVD_WRITE32(tvd_base[channel_id] + 0x52C, 0x30900AA0);  
	TVD_WRITE32(tvd_base[channel_id] + 0x530, 0x0B410B09);  
	TVD_WRITE32(tvd_base[channel_id] + 0x534, 0xF43E40F0);  
	TVD_WRITE32(tvd_base[channel_id] + 0x538, 0x452414F0);  
	TVD_WRITE32(tvd_base[channel_id] + 0x53C, 0x55C29325);  
	TVD_WRITE32(tvd_base[channel_id] + 0x540, 0x30173F93);  
	TVD_WRITE32(tvd_base[channel_id] + 0x544, 0x21A41725);  
	TVD_WRITE32(tvd_base[channel_id] + 0x548, 0x30B080FE);  
	TVD_WRITE32(tvd_base[channel_id] + 0x54C, 0x1245482B);  
	TVD_WRITE32(tvd_base[channel_id] + 0x550, 0xB4000023);  
	TVD_WRITE32(tvd_base[channel_id] + 0x554, 0x04AE0318);  
	TVD_WRITE32(tvd_base[channel_id] + 0x558, 0x106443D0);  
	TVD_WRITE32(tvd_base[channel_id] + 0x55C, 0x52836471);  
	TVD_WRITE32(tvd_base[channel_id] + 0x560, 0x29E518EF);  
	TVD_WRITE32(tvd_base[channel_id] + 0x564, 0x2010553C);  
	TVD_WRITE32(tvd_base[channel_id] + 0x568, 0x204CDEC9);  
	TVD_WRITE32(tvd_base[channel_id] + 0x56C, 0x204A9881);  
	TVD_WRITE32(tvd_base[channel_id] + 0x570, 0xFF29DEC5);  
	TVD_WRITE32(tvd_base[channel_id] + 0x574, 0xCA8B0F00);  
	TVD_WRITE32(tvd_base[channel_id] + 0x578, 0x797A8E61);  
	TVD_WRITE32(tvd_base[channel_id] + 0x57C, 0x420FA8A1);  
	TVD_WRITE32(tvd_base[channel_id] + 0x580, 0x208181D4);  
	TVD_WRITE32(tvd_base[channel_id] + 0x584, 0x28AF88C8);  
	TVD_WRITE32(tvd_base[channel_id] + 0x588, 0xEC844040);  
	TVD_WRITE32(tvd_base[channel_id] + 0x58C, 0x2854C531);  
	TVD_WRITE32(tvd_base[channel_id] + 0x590, 0x0206E61A);  
	TVD_WRITE32(tvd_base[channel_id] + 0x594, 0x4840310E);  
	TVD_WRITE32(tvd_base[channel_id] + 0x598, 0x21594088);  
	TVD_WRITE32(tvd_base[channel_id] + 0x59C, 0x29E2F0B2);  
	TVD_WRITE32(tvd_base[channel_id] + 0x5A0, 0x44800C80);  
	TVD_WRITE32(tvd_base[channel_id] + 0x5A4, 0x20010000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x5A8, 0x0A4BC680);  
	TVD_WRITE32(tvd_base[channel_id] + 0x5AC, 0x400010C0);  
	TVD_WRITE32(tvd_base[channel_id] + 0x5B0, 0x20008400);  
	TVD_WRITE32(tvd_base[channel_id] + 0x5B4, 0x00F9BC00);  
	TVD_WRITE32(tvd_base[channel_id] + 0x5B8, 0x20002004);  
	TVD_WRITE32(tvd_base[channel_id] + 0x5BC, 0x17E0114F);  
	TVD_WRITE32(tvd_base[channel_id] + 0x5C0, 0x1F908420);  
	TVD_WRITE32(tvd_base[channel_id] + 0x5C4, 0x1A191515);  
	TVD_WRITE32(tvd_base[channel_id] + 0x5C8, 0x06658819);  
	TVD_WRITE32(tvd_base[channel_id] + 0x5CC, 0x72B91AB9);  
	TVD_WRITE32(tvd_base[channel_id] + 0x5D0, 0x5A1640DB);  
	TVD_WRITE32(tvd_base[channel_id] + 0x5D4, 0x55F401F4);  
	TVD_WRITE32(tvd_base[channel_id] + 0x5D8, 0x1001825E);  
	TVD_WRITE32(tvd_base[channel_id] + 0x5DC, 0x60506E60);  
	TVD_WRITE32(tvd_base[channel_id] + 0x5E0, 0xA0A793A6);  
	TVD_WRITE32(tvd_base[channel_id] + 0x5E4, 0xFACAD1A0);  
	TVD_WRITE32(tvd_base[channel_id] + 0x5E8, 0x7CB83BD4);  
	TVD_WRITE32(tvd_base[channel_id] + 0x5EC, 0x80DD43A9);  
	TVD_WRITE32(tvd_base[channel_id] + 0x5F0, 0x80554329);  
	TVD_WRITE32(tvd_base[channel_id] + 0x5F4, 0x1748A14B);  
	TVD_WRITE32(tvd_base[channel_id] + 0x5F8, 0x5040C67F);  
	TVD_WRITE32(tvd_base[channel_id] + 0x5FC, 0x70D80480);  
	TVD_WRITE32(tvd_base[channel_id] + 0x600, 0x3D34035F);  
	TVD_WRITE32(tvd_base[channel_id] + 0x604, 0xF9228320);  
	TVD_WRITE32(tvd_base[channel_id] + 0x608, 0x32201501);  
	TVD_WRITE32(tvd_base[channel_id] + 0x60C, 0x5480A833);  
	TVD_WRITE32(tvd_base[channel_id] + 0x610, 0x000028AD);  
	TVD_WRITE32(tvd_base[channel_id] + 0x614, 0x55E8666E);  
	TVD_WRITE32(tvd_base[channel_id] + 0x618, 0xE6CB2224);  
	TVD_WRITE32(tvd_base[channel_id] + 0x61C, 0x84460080);  
	TVD_WRITE32(tvd_base[channel_id] + 0x620, 0x00400000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x624, 0x7F084330);  
	TVD_WRITE32(tvd_base[channel_id] + 0x628, 0x08880001);  
	TVD_WRITE32(tvd_base[channel_id] + 0x62C, 0xBF488488);  
	TVD_WRITE32(tvd_base[channel_id] + 0x630, 0x0119812C);  
	TVD_WRITE32(tvd_base[channel_id] + 0x634, 0x01000844);  
	TVD_WRITE32(tvd_base[channel_id] + 0x638, 0x88020818);  
	TVD_WRITE32(tvd_base[channel_id] + 0x63C, 0x0FF0AA1F);  
	TVD_WRITE32(tvd_base[channel_id] + 0x640, 0x000B101F);  
	TVD_WRITE32(tvd_base[channel_id] + 0x644, 0x89A40600);  
	TVD_WRITE32(tvd_base[channel_id] + 0x648, 0x8A04AAFB);  
	TVD_WRITE32(tvd_base[channel_id] + 0x64C, 0x2F360203);  
	TVD_WRITE32(tvd_base[channel_id] + 0x650, 0x02F40000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x654, 0x31364408);  
	TVD_WRITE32(tvd_base[channel_id] + 0x658, 0x01CC1A71);  
	TVD_WRITE32(tvd_base[channel_id] + 0x65C, 0x02611900);  
	TVD_WRITE32(tvd_base[channel_id] + 0x660, 0x01151139);  
	TVD_WRITE32(tvd_base[channel_id] + 0x664, 0x3953160B);  
	TVD_WRITE32(tvd_base[channel_id] + 0x668, 0x0A08BEC9);  
	TVD_WRITE32(tvd_base[channel_id] + 0x66C, 0x2008BEC9);  
	TVD_WRITE32(tvd_base[channel_id] + 0x670, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x674, 0x00000007);  
	TVD_WRITE32(tvd_base[channel_id] + 0x678, 0x63636363);  
	TVD_WRITE32(tvd_base[channel_id] + 0x67C, 0x4A101030);  
	TVD_WRITE32(tvd_base[channel_id] + 0x680, 0x480A1E2A);  
	TVD_WRITE32(tvd_base[channel_id] + 0x684, 0x7018040C);  
	TVD_WRITE32(tvd_base[channel_id] + 0x688, 0xC896321E);  
	TVD_WRITE32(tvd_base[channel_id] + 0x68C, 0x00107023);  
	TVD_WRITE32(tvd_base[channel_id] + 0x690, 0x0F00EF85);  
	TVD_WRITE32(tvd_base[channel_id] + 0x694, 0x0070500C);  
	TVD_WRITE32(tvd_base[channel_id] + 0x698, 0x00641400);  
	TVD_WRITE32(tvd_base[channel_id] + 0x69C, 0x00300500);  
	TVD_WRITE32(tvd_base[channel_id] + 0x6A0, 0x0000A00A);  
	TVD_WRITE32(tvd_base[channel_id] + 0x6A4, 0x1E702808);  
	TVD_WRITE32(tvd_base[channel_id] + 0x6A8, 0x80064078);  
	TVD_WRITE32(tvd_base[channel_id] + 0x6AC, 0x54280504);  
	TVD_WRITE32(tvd_base[channel_id] + 0x6B0, 0x20100000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x6B4, 0x00969600);  
	TVD_WRITE32(tvd_base[channel_id] + 0x6B8, 0x04044444);  
	TVD_WRITE32(tvd_base[channel_id] + 0x6BC, 0x0C010444);  
	TVD_WRITE32(tvd_base[channel_id] + 0x6C0, 0x00A444F3);  
	TVD_WRITE32(tvd_base[channel_id] + 0x6C4, 0x3E1A5353);  
	TVD_WRITE32(tvd_base[channel_id] + 0x6C8, 0x064B1478);  
	TVD_WRITE32(tvd_base[channel_id] + 0x6CC, 0x1020D831);  
	TVD_WRITE32(tvd_base[channel_id] + 0x6D0, 0x11111111);  
	TVD_WRITE32(tvd_base[channel_id] + 0x6D4, 0xC008FC10);  
	TVD_WRITE32(tvd_base[channel_id] + 0x6D8, 0x00000020);  
	TVD_WRITE32(tvd_base[channel_id] + 0x6DC, 0xC0001020);  
	TVD_WRITE32(tvd_base[channel_id] + 0x6E0, 0x0F371858);  
	TVD_WRITE32(tvd_base[channel_id] + 0x6E4, 0x3220008F);  
	TVD_WRITE32(tvd_base[channel_id] + 0x6E8, 0xB0210E28);  
	TVD_WRITE32(tvd_base[channel_id] + 0x6EC, 0x00000014);  
	TVD_WRITE32(tvd_base[channel_id] + 0x6F0, 0x43174537);  
	TVD_WRITE32(tvd_base[channel_id] + 0x6F4, 0x0045713C);  
	TVD_WRITE32(tvd_base[channel_id] + 0x6F8, 0xC0001A50);  
	TVD_WRITE32(tvd_base[channel_id] + 0x6FC, 0x4414050D);  
	TVD_WRITE32(tvd_base[channel_id] + 0x700, 0x11A00020);  
	TVD_WRITE32(tvd_base[channel_id] + 0x704, 0x64000450);  
	TVD_WRITE32(tvd_base[channel_id] + 0x708, 0x410A0028);  
	TVD_WRITE32(tvd_base[channel_id] + 0x70C, 0x00014455);  
	TVD_WRITE32(tvd_base[channel_id] + 0x710, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x714, 0x31581A40); 
	TVD_WRITE32(tvd_base[channel_id] + 0x718, 0x849F0000); 
	TVD_WRITE32(tvd_base[channel_id] + 0x71C, 0x08001020); 
	TVD_WRITE32(tvd_base[channel_id] + 0x720, 0x08081E14); 
	TVD_WRITE32(tvd_base[channel_id] + 0x724, 0x0A100008); 
	TVD_WRITE32(tvd_base[channel_id] + 0x728, 0x50061608); 
	TVD_WRITE32(tvd_base[channel_id] + 0x72C, 0x0C0A2808); 
	TVD_WRITE32(tvd_base[channel_id] + 0x730, 0x0A149000); 
	TVD_WRITE32(tvd_base[channel_id] + 0x734, 0x55000019); 
	TVD_WRITE32(tvd_base[channel_id] + 0x738, 0x0311041A); 
	TVD_WRITE32(tvd_base[channel_id] + 0x73C, 0x8C0A100A); 
	TVD_WRITE32(tvd_base[channel_id] + 0x740, 0x01245678); 
	TVD_WRITE32(tvd_base[channel_id] + 0x744, 0x02345678);
	TVD_WRITE32_MASK(tvd_base[channel_id] + 0x748, 0x00080014, 0xCFFFFFFF);
	TVD_WRITE32(tvd_base[channel_id] + 0x74C, 0x00020A34);
	TVD_WRITE32(tvd_base[channel_id] + 0x750, 0x047C0558);
	TVD_WRITE32(tvd_base[channel_id] + 0x754, 0x0088011A);
	TVD_WRITE32(tvd_base[channel_id] + 0x758, 0x00009666);
	TVD_WRITE32(tvd_base[channel_id] + 0x75C, 0x000781FF);
	TVD_WRITE32(tvd_base[channel_id] + 0x760, 0x90000000);
	TVD_WRITE32(tvd_base[channel_id] + 0x764, 0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0x768, 0x0F051818);
	TVD_WRITE32(tvd_base[channel_id] + 0x76C, 0x00191014);
	TVD_WRITE32(tvd_base[channel_id] + 0x770, 0x23645050);
	TVD_WRITE32(tvd_base[channel_id] + 0x774, 0x64641432);
	TVD_WRITE32(tvd_base[channel_id] + 0x778, 0x1E519696);
	TVD_WRITE32(tvd_base[channel_id] + 0x77C, 0x10169696);
	TVD_WRITE32(tvd_base[channel_id] + 0x780, 0x053C501E);
	TVD_WRITE32(tvd_base[channel_id] + 0x784, 0x820A325A);
	TVD_WRITE32(tvd_base[channel_id] + 0x788, 0xFFFF3359);  
	TVD_WRITE32(tvd_base[channel_id] + 0x78C, 0xFFFFFFFF);
	TVD_WRITE32(tvd_base[channel_id] + 0x790, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x794, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x798, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x79C, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x7A0, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x7A4, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x7A8, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x7AC, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x7B0, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x7B4, 0x100181FA);  
	TVD_WRITE32(tvd_base[channel_id] + 0x7B8, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x7BC, 0x00000000);

	TVD_WRITE32(tvd_base[channel_id] + 0x7D8, 0x020F42D0);  
	TVD_WRITE32(tvd_base[channel_id] + 0x7DC, 0x00000008);  
	TVD_WRITE32(tvd_base[channel_id] + 0x7E0, 0x0F0000A0);  
	TVD_WRITE32(tvd_base[channel_id] + 0x7E4, 0x11000020);  
	TVD_WRITE32(tvd_base[channel_id] + 0x7E8, 0xC000C000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x7EC, 0x00CC0C50);  
	TVD_WRITE32(tvd_base[channel_id] + 0x7F0, 0x80008000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x7F4, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x7F8, 0x80000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x7FC, 0xD0090901);  
	TVD_WRITE32(tvd_base[channel_id] + 0x800, 0x00060604);  
	TVD_WRITE32(tvd_base[channel_id] + 0x804, 0x0B9000FF);  
	TVD_WRITE32(tvd_base[channel_id] + 0x808, 0x001400FC);  
	TVD_WRITE32(tvd_base[channel_id] + 0x80C, 0x005071AD);  
	TVD_WRITE32(tvd_base[channel_id] + 0x810, 0xC3080050);  
	TVD_WRITE32(tvd_base[channel_id] + 0x814, 0x0F120700);  
	TVD_WRITE32(tvd_base[channel_id] + 0x818, 0x02100578);  
	TVD_WRITE32(tvd_base[channel_id] + 0x81C, 0x65004020);  
	TVD_WRITE32(tvd_base[channel_id] + 0x820, 0x0000F00F);  
	TVD_WRITE32(tvd_base[channel_id] + 0x824, 0x14140F0F);  
	TVD_WRITE32(tvd_base[channel_id] + 0x828, 0x0A0A0A0A);  
	TVD_WRITE32(tvd_base[channel_id] + 0x82C, 0x77700000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x830, 0x00A0A000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x834, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x838, 0x4400A00A);  
	TVD_WRITE32(tvd_base[channel_id] + 0x83C, 0x14000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x840, 0x00003010);  
	TVD_WRITE32(tvd_base[channel_id] + 0x844, 0x00201007);  
	TVD_WRITE32(tvd_base[channel_id] + 0x848, 0x00201007);  
	TVD_WRITE32(tvd_base[channel_id] + 0x84C, 0x0700100A);  
	TVD_WRITE32(tvd_base[channel_id] + 0x850, 0x00208858);  
	TVD_WRITE32(tvd_base[channel_id] + 0x854, 0x14001010);  
	TVD_WRITE32(tvd_base[channel_id] + 0x858, 0x00141414);  
	TVD_WRITE32(tvd_base[channel_id] + 0x85C, 0x10141414);  
	TVD_WRITE32(tvd_base[channel_id] + 0x860, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x864, 0x00010000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x868, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x86C, 0xC0400C0A);  
	TVD_WRITE32(tvd_base[channel_id] + 0x870, 0x00020820);  
	TVD_WRITE32(tvd_base[channel_id] + 0x874, 0xC0016C01);  
	TVD_WRITE32(tvd_base[channel_id] + 0x878, 0x0089A022);  
	TVD_WRITE32(tvd_base[channel_id] + 0x87C, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x880, 0x00060604);  
	TVD_WRITE32(tvd_base[channel_id] + 0x884, 0x000001FC);  
	TVD_WRITE32(tvd_base[channel_id] + 0x888, 0x005071AD);  
	TVD_WRITE32(tvd_base[channel_id] + 0x88C, 0x00002010);  
	TVD_WRITE32(tvd_base[channel_id] + 0x890, 0x0008F060);  
	TVD_WRITE32(tvd_base[channel_id] + 0x894, 0x0020A000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x898, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x89C, 0x00606200);  
	TVD_WRITE32(tvd_base[channel_id] + 0x8A0, 0xC000C000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x8A4, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x8A8, 0x0000000F);  
	TVD_WRITE32(tvd_base[channel_id] + 0x8AC, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x8B0, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x8B4, 0x00000078);  
	TVD_WRITE32(tvd_base[channel_id] + 0x8B8, 0xC0016C01);  
	TVD_WRITE32(tvd_base[channel_id] + 0x8BC, 0x6EC86EC8);  
	TVD_WRITE32(tvd_base[channel_id] + 0x8C0, 0x65031E04);  
	TVD_WRITE32(tvd_base[channel_id] + 0x8C4, 0x12884420);  
	TVD_WRITE32(tvd_base[channel_id] + 0x8C8, 0x0185C501);  
	TVD_WRITE32(tvd_base[channel_id] + 0x8CC, 0xC1CBA860);  
	TVD_WRITE32(tvd_base[channel_id] + 0x8D0, 0x70000BB8);  
	TVD_WRITE32(tvd_base[channel_id] + 0x8D4, 0x7F294A73);  
	TVD_WRITE32(tvd_base[channel_id] + 0x8D8, 0x83C29000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x8DC, 0x74D08610);  
	TVD_WRITE32(tvd_base[channel_id] + 0x8E0, 0x195D1CCD);  
	TVD_WRITE32(tvd_base[channel_id] + 0x8E4, 0x2ACBBEF8);  
	TVD_WRITE32(tvd_base[channel_id] + 0x8E8, 0x03F81CCD);  
	TVD_WRITE32(tvd_base[channel_id] + 0x8EC, 0xF3935CD6);  
	TVD_WRITE32(tvd_base[channel_id] + 0x8F0, 0x532714C2);  
	TVD_WRITE32(tvd_base[channel_id] + 0x8F4, 0x471848D4);  
	TVD_WRITE32(tvd_base[channel_id] + 0x8F8, 0x80000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x8FC, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x900, 0x62F32005);  
	TVD_WRITE32(tvd_base[channel_id] + 0x904, 0x120855A0);  
	TVD_WRITE32(tvd_base[channel_id] + 0x908, 0x0185D0B9);  
	TVD_WRITE32(tvd_base[channel_id] + 0x90C, 0xC1CAA800);  
	TVD_WRITE32(tvd_base[channel_id] + 0x910, 0x70000BB8);  
	TVD_WRITE32(tvd_base[channel_id] + 0x914, 0x7F284231);  
	TVD_WRITE32(tvd_base[channel_id] + 0x918, 0x83C28F00);  
	TVD_WRITE32(tvd_base[channel_id] + 0x91C, 0x74D08610);  
	TVD_WRITE32(tvd_base[channel_id] + 0x920, 0x195D1CCD);  
	TVD_WRITE32(tvd_base[channel_id] + 0x924, 0x413B0252);  
	TVD_WRITE32(tvd_base[channel_id] + 0x928, 0x0440283B);  
	TVD_WRITE32(tvd_base[channel_id] + 0x92C, 0xF403DCF7);  
	TVD_WRITE32(tvd_base[channel_id] + 0x930, 0x532714C2);  
	TVD_WRITE32(tvd_base[channel_id] + 0x934, 0x87184814);  
	TVD_WRITE32(tvd_base[channel_id] + 0x938, 0x08000800);  
	TVD_WRITE32(tvd_base[channel_id] + 0x93C, 0x00000800);  
	TVD_WRITE32(tvd_base[channel_id] + 0x940, 0x10281400);  
	TVD_WRITE32(tvd_base[channel_id] + 0x944, 0x04D0BB40);  
	TVD_WRITE32(tvd_base[channel_id] + 0x948, 0x10281400);  
	TVD_WRITE32(tvd_base[channel_id] + 0x94C, 0x04D0BB40);  
	TVD_WRITE32(tvd_base[channel_id] + 0x950, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x954, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x958, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x95C, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x960, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x964, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x968, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x96C, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x970, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x974, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x978, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x97C, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x980, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x984, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x988, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x98C, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x990, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x994, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x998, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x99C, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x9A0, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x9A4, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x9A8, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x9AC, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x9B0, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x9B4, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x9B8, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x9BC, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x9C0, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x9C4, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x9C8, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x9CC, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x9D0, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x9D4, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x9D8, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x9DC, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x9E0, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x9E4, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x9E8, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x9EC, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x9F0, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x9F4, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x9F8, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0x9FC, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0xA00, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0xA04, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0xA08, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0xA0C, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0xA10, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0xA14, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0xA18, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0xA1C, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0xA20, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0xA24, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0xA28, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0xA2C, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0xA30, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0xA34, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0xA38, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0xA3C, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0xA40, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0xA44, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0xA48, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0xA4C, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0xA50, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0xA54, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0xA58, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0xA5C, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0xA60, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0xA64, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0xA68, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0xA6C, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0xA70, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0xA74, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0xA78, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0xA7C, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0xA80, 0x0FF3FCFF);  
	TVD_WRITE32(tvd_base[channel_id] + 0xA84, 0x80008000);  
	TVD_WRITE32(tvd_base[channel_id] + 0xA88, 0x80000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0xA8C, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0xA90, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0xA94, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0xA98, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0xA9C, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0xAA0, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0xAA4, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0xAA8, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0xAAC, 0x00000000);  
	TVD_WRITE32(tvd_base[channel_id] + 0xAB0, 0x22F08C03);  
	TVD_WRITE32(tvd_base[channel_id] + 0xAB4, 0x8F0070D4);  
	TVD_WRITE32(tvd_base[channel_id] + 0xAB8, 0x42400800);  
	TVD_WRITE32(tvd_base[channel_id] + 0xABC, 0x82000820);  

	TVD_WRITE32(tvd_base[channel_id] + 0x5aC, 0x000010C0); //bit 31
	TVD_WRITE32(tvd_base[channel_id] + 0x59C, 0x21E2c0B2); //H start
	TVD_WRITE32(tvd_base[channel_id] + 0x5c4, 0x18181617); //v start
	
	return;
}
void vSECAM_2D_Setting(TVD_CHANNEL_ID channel_id)
{
	TVD_WRITE32(tvd_base[channel_id] + 0x410, 0x00818000);
	TVD_WRITE32(tvd_base[channel_id] + 0x414, 0xFF080020);
	TVD_WRITE32(tvd_base[channel_id] + 0x418, 0x10000000);
	TVD_WRITE32(tvd_base[channel_id] + 0x41C, 0x12400010);
	TVD_WRITE32(tvd_base[channel_id] + 0x420, 0x22788207);
	TVD_WRITE32(tvd_base[channel_id] + 0x424, 0x000000FF);
	TVD_WRITE32(tvd_base[channel_id] + 0x428, 0xFFFFFFFF);
	TVD_WRITE32(tvd_base[channel_id] + 0x42C, 0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0x430, 0x00000000);

	TVD_WRITE32(tvd_base[channel_id] + 0x434, 0xC4D00101);

	TVD_WRITE32(tvd_base[channel_id] + 0x438, 0xB801404A);
	TVD_WRITE32(tvd_base[channel_id] + 0x43C, 0x00800801);
	TVD_WRITE32(tvd_base[channel_id] + 0x440, 0x00D01001);
	TVD_WRITE32(tvd_base[channel_id] + 0x444, 0x1E000000);
	TVD_WRITE32(tvd_base[channel_id] + 0x448, 0x80000114);
	TVD_WRITE32(tvd_base[channel_id] + 0x44C, 0x2AAA8103);
	TVD_WRITE32(tvd_base[channel_id] + 0x450, 0xF0000100);
	TVD_WRITE32(tvd_base[channel_id] + 0x454, 0x1E000000);
	TVD_WRITE32(tvd_base[channel_id] + 0x458, 0x80000114);
	TVD_WRITE32(tvd_base[channel_id] + 0x45C, 0x2AAA8103);
	TVD_WRITE32(tvd_base[channel_id] + 0x460, 0x47B00000);
	TVD_WRITE32(tvd_base[channel_id] + 0x464, 0x1E000000);
	TVD_WRITE32(tvd_base[channel_id] + 0x468, 0x80000114);
	TVD_WRITE32(tvd_base[channel_id] + 0x46C, 0x2AAA8103);
	TVD_WRITE32(tvd_base[channel_id] + 0x470, 0x1100001B);
	TVD_WRITE32(tvd_base[channel_id] + 0x474, 0xAAA04EAB);
	TVD_WRITE32(tvd_base[channel_id] + 0x478, 0x0888AAAA);
	TVD_WRITE32(tvd_base[channel_id] + 0x47C, 0x00050000);
	TVD_WRITE32(tvd_base[channel_id] + 0x480, 0x003E313D);
	TVD_WRITE32(tvd_base[channel_id] + 0x484, 0x100F0480);

	TVD_WRITE32(tvd_base[channel_id] + 0x48C, 0x0200022D);
	TVD_WRITE32(tvd_base[channel_id] + 0x490, 0x0AFFC588);
	TVD_WRITE32(tvd_base[channel_id] + 0x494, 0x08000000);
	TVD_WRITE32(tvd_base[channel_id] + 0x498, 0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0x49C, 0x0009C000);
	TVD_WRITE32(tvd_base[channel_id] + 0x4A0, 0x00090000);
	TVD_WRITE32(tvd_base[channel_id] + 0x4A4, 0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0x4A8, 0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0x4AC, 0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0x4B0, 0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0x4B4, 0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0x4B8, 0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0x4BC, 0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0x4C0, 0x51515C55);
	TVD_WRITE32(tvd_base[channel_id] + 0x4C4, 0x04557845);
	TVD_WRITE32(tvd_base[channel_id] + 0x4C8, 0x27F7595E);
	TVD_WRITE32(tvd_base[channel_id] + 0x4CC, 0x8F3403A0);
	TVD_WRITE32(tvd_base[channel_id] + 0x4D0, 0x15890F80);
	TVD_WRITE32(tvd_base[channel_id] + 0x4D4, 0x26144040);
	TVD_WRITE32(tvd_base[channel_id] + 0x4D8, 0x0A00F600);
	TVD_WRITE32(tvd_base[channel_id] + 0x4DC, 0xF41F0800);
	TVD_WRITE32(tvd_base[channel_id] + 0x4E0, 0x33178080);
	TVD_WRITE32(tvd_base[channel_id] + 0x4E4, 0x8AA7075E);
	TVD_WRITE32(tvd_base[channel_id] + 0x4E8, 0xF08400A0);
	TVD_WRITE32(tvd_base[channel_id] + 0x4EC, 0x08180C40);
	TVD_WRITE32(tvd_base[channel_id] + 0x4F0, 0x2008C2C9);
	TVD_WRITE32(tvd_base[channel_id] + 0x4F4, 0x7180001F);
	TVD_WRITE32(tvd_base[channel_id] + 0x4F8, 0x8000306C);
	TVD_WRITE32(tvd_base[channel_id] + 0x4FC, 0x420C33A0);
	TVD_WRITE32(tvd_base[channel_id] + 0x500, 0x6838FA27);
	TVD_WRITE32(tvd_base[channel_id] + 0x504, 0x0DFA2090);
	TVD_WRITE32(tvd_base[channel_id] + 0x508, 0x08E06B40);
	TVD_WRITE32(tvd_base[channel_id] + 0x50C, 0x83608016);
	TVD_WRITE32(tvd_base[channel_id] + 0x510, 0x40204080);
	TVD_WRITE32(tvd_base[channel_id] + 0x514, 0x80326485);
	TVD_WRITE32(tvd_base[channel_id] + 0x518, 0x11A0A04C);
	TVD_WRITE32(tvd_base[channel_id] + 0x51C, 0x8040A0C0);
	TVD_WRITE32(tvd_base[channel_id] + 0x520, 0x7E35AF80);
	TVD_WRITE32(tvd_base[channel_id] + 0x524, 0x0408F840);
	TVD_WRITE32(tvd_base[channel_id] + 0x528, 0x5404E030);
	TVD_WRITE32(tvd_base[channel_id] + 0x52C, 0x30900AA0);
	TVD_WRITE32(tvd_base[channel_id] + 0x530, 0x0B410B09);
	TVD_WRITE32(tvd_base[channel_id] + 0x534, 0xF43E40F0);
	TVD_WRITE32(tvd_base[channel_id] + 0x538, 0x452414F0);
	TVD_WRITE32(tvd_base[channel_id] + 0x53C, 0x55C29325);
	TVD_WRITE32(tvd_base[channel_id] + 0x540, 0x30173F93);
	TVD_WRITE32(tvd_base[channel_id] + 0x544, 0x21A41725);
	TVD_WRITE32(tvd_base[channel_id] + 0x548, 0x30B080FE);
	TVD_WRITE32(tvd_base[channel_id] + 0x54C, 0x1245481B);
	TVD_WRITE32(tvd_base[channel_id] + 0x550, 0xB4000023);
	TVD_WRITE32(tvd_base[channel_id] + 0x554, 0x04AE0318);
	TVD_WRITE32(tvd_base[channel_id] + 0x558, 0x106543D0);
	TVD_WRITE32(tvd_base[channel_id] + 0x55C, 0x3040383A);
	TVD_WRITE32(tvd_base[channel_id] + 0x560, 0x29E518EF);
	TVD_WRITE32(tvd_base[channel_id] + 0x564, 0x2010553C);
	TVD_WRITE32(tvd_base[channel_id] + 0x568, 0x204CDEC9);
	TVD_WRITE32(tvd_base[channel_id] + 0x56C, 0x204A9881);
	TVD_WRITE32(tvd_base[channel_id] + 0x570, 0xFF29DEC5);
	TVD_WRITE32(tvd_base[channel_id] + 0x574, 0xCA8B0F00);
	TVD_WRITE32(tvd_base[channel_id] + 0x578, 0x797A8E61);
	TVD_WRITE32(tvd_base[channel_id] + 0x57C, 0x420FA8A1);
	TVD_WRITE32(tvd_base[channel_id] + 0x580, 0x208181D4);
	TVD_WRITE32(tvd_base[channel_id] + 0x584, 0x28AF88C8);
	TVD_WRITE32(tvd_base[channel_id] + 0x588, 0xEC844040);
	TVD_WRITE32(tvd_base[channel_id] + 0x58C, 0x2854C531);
	TVD_WRITE32(tvd_base[channel_id] + 0x590, 0x0206E61A);
	TVD_WRITE32(tvd_base[channel_id] + 0x594, 0x4840310E);
	TVD_WRITE32(tvd_base[channel_id] + 0x598, 0x21594088);
	TVD_WRITE32(tvd_base[channel_id] + 0x59C, 0x29E280A5);
	TVD_WRITE32(tvd_base[channel_id] + 0x5A0, 0x44800C80);
	TVD_WRITE32(tvd_base[channel_id] + 0x5A4, 0x20010000);
	TVD_WRITE32(tvd_base[channel_id] + 0x5A8, 0x0A4BC6C1);
	TVD_WRITE32(tvd_base[channel_id] + 0x5AC, 0x400010C0);
	TVD_WRITE32(tvd_base[channel_id] + 0x5B0, 0x20008400);
	TVD_WRITE32(tvd_base[channel_id] + 0x5B4, 0x00F9BC00);
	TVD_WRITE32(tvd_base[channel_id] + 0x5B8, 0x20002004);
	TVD_WRITE32(tvd_base[channel_id] + 0x5BC, 0x17E0114F);
	TVD_WRITE32(tvd_base[channel_id] + 0x5C0, 0x1F908420);
	TVD_WRITE32(tvd_base[channel_id] + 0x5C4, 0x1A191515);
	TVD_WRITE32(tvd_base[channel_id] + 0x5C8, 0x06658819);
	TVD_WRITE32(tvd_base[channel_id] + 0x5CC, 0x72B91AB9);
	TVD_WRITE32(tvd_base[channel_id] + 0x5D0, 0x5A1640DB);
	TVD_WRITE32(tvd_base[channel_id] + 0x5D4, 0x55F401F4);
	TVD_WRITE32(tvd_base[channel_id] + 0x5D8, 0x1001825E);
	TVD_WRITE32(tvd_base[channel_id] + 0x5DC, 0x60506E60);
	TVD_WRITE32(tvd_base[channel_id] + 0x5E0, 0xA0A797A6);
	TVD_WRITE32(tvd_base[channel_id] + 0x5E4, 0xDACAD1A0);
	TVD_WRITE32(tvd_base[channel_id] + 0x5E8, 0x7CB83BD4);
	TVD_WRITE32(tvd_base[channel_id] + 0x5EC, 0x80D543A9);
	TVD_WRITE32(tvd_base[channel_id] + 0x5F0, 0x80554329);
	TVD_WRITE32(tvd_base[channel_id] + 0x5F4, 0x1748A14B);
	TVD_WRITE32(tvd_base[channel_id] + 0x5F8, 0x5040C67F);
	TVD_WRITE32(tvd_base[channel_id] + 0x5FC, 0x70D80480);
	TVD_WRITE32(tvd_base[channel_id] + 0x600, 0x3D34035F);
	TVD_WRITE32(tvd_base[channel_id] + 0x604, 0xF9228320);
	TVD_WRITE32(tvd_base[channel_id] + 0x608, 0x32201501);
	TVD_WRITE32(tvd_base[channel_id] + 0x60C, 0x3040A833);
	TVD_WRITE32(tvd_base[channel_id] + 0x610, 0x000028AD);
	TVD_WRITE32(tvd_base[channel_id] + 0x614, 0x55E8383A);
	TVD_WRITE32(tvd_base[channel_id] + 0x618, 0xE6CB2224);
	TVD_WRITE32(tvd_base[channel_id] + 0x61C, 0x84460080);
	TVD_WRITE32(tvd_base[channel_id] + 0x620, 0x00400000);
	TVD_WRITE32(tvd_base[channel_id] + 0x624, 0x7F084330);
	TVD_WRITE32(tvd_base[channel_id] + 0x628, 0x08880001);
	TVD_WRITE32(tvd_base[channel_id] + 0x62C, 0xBF488488);
	TVD_WRITE32(tvd_base[channel_id] + 0x630, 0x0119812C);
	TVD_WRITE32(tvd_base[channel_id] + 0x634, 0x01000844);
	TVD_WRITE32(tvd_base[channel_id] + 0x638, 0x88020818);
	TVD_WRITE32(tvd_base[channel_id] + 0x63C, 0x0FF0AA1F);
	TVD_WRITE32(tvd_base[channel_id] + 0x640, 0x000B101F);
	TVD_WRITE32(tvd_base[channel_id] + 0x644, 0x09A40600);
	TVD_WRITE32(tvd_base[channel_id] + 0x648, 0x0A045AFB);
	TVD_WRITE32(tvd_base[channel_id] + 0x64C, 0x25030211);
	TVD_WRITE32(tvd_base[channel_id] + 0x650, 0x03980000);
	TVD_WRITE32(tvd_base[channel_id] + 0x654, 0x31034000);
	TVD_WRITE32(tvd_base[channel_id] + 0x658, 0x021E1E74);
	TVD_WRITE32(tvd_base[channel_id] + 0x65C, 0x02641900);
	TVD_WRITE32(tvd_base[channel_id] + 0x660, 0x01151139);
	TVD_WRITE32(tvd_base[channel_id] + 0x664, 0x46F3C00B);
	TVD_WRITE32(tvd_base[channel_id] + 0x668, 0x0A09073F);
	TVD_WRITE32(tvd_base[channel_id] + 0x66C, 0x0A09073F);
	TVD_WRITE32(tvd_base[channel_id] + 0x670, 0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0x674, 0x00000007);
	TVD_WRITE32(tvd_base[channel_id] + 0x678, 0x6C6C6B6A);
	TVD_WRITE32(tvd_base[channel_id] + 0x67C, 0x4A101030);
	TVD_WRITE32(tvd_base[channel_id] + 0x680, 0x580A1E05);
	TVD_WRITE32(tvd_base[channel_id] + 0x684, 0x6018040C);
	TVD_WRITE32(tvd_base[channel_id] + 0x688, 0xC896321E);
	TVD_WRITE32(tvd_base[channel_id] + 0x68C, 0x00107023);
	TVD_WRITE32(tvd_base[channel_id] + 0x690, 0x0F01EF85);
	TVD_WRITE32(tvd_base[channel_id] + 0x694, 0x0070500C);
	TVD_WRITE32(tvd_base[channel_id] + 0x698, 0x00641400);
	TVD_WRITE32(tvd_base[channel_id] + 0x69C, 0x00300500);
	TVD_WRITE32(tvd_base[channel_id] + 0x6A0, 0x0000230A);
	TVD_WRITE32(tvd_base[channel_id] + 0x6A4, 0x1E702808);
	TVD_WRITE32(tvd_base[channel_id] + 0x6A8, 0x80064078);
	TVD_WRITE32(tvd_base[channel_id] + 0x6AC, 0x54280504);
	TVD_WRITE32(tvd_base[channel_id] + 0x6B0, 0x200F0000);
	TVD_WRITE32(tvd_base[channel_id] + 0x6B4, 0x00969600);
	TVD_WRITE32(tvd_base[channel_id] + 0x6B8, 0x04444444);
	TVD_WRITE32(tvd_base[channel_id] + 0x6BC, 0x0C010444);
	TVD_WRITE32(tvd_base[channel_id] + 0x6C0, 0x00A444F3);
	TVD_WRITE32(tvd_base[channel_id] + 0x6C4, 0x3E1A5353);
	TVD_WRITE32(tvd_base[channel_id] + 0x6C8, 0x064B1478);
	TVD_WRITE32(tvd_base[channel_id] + 0x6CC, 0x1020D831);
	TVD_WRITE32(tvd_base[channel_id] + 0x6D0, 0x11111111);
	TVD_WRITE32(tvd_base[channel_id] + 0x6D4, 0xC008FC10);
	TVD_WRITE32(tvd_base[channel_id] + 0x6D8, 0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0x6DC, 0xC0001020);
	TVD_WRITE32(tvd_base[channel_id] + 0x6E0, 0x0F371858);
	TVD_WRITE32(tvd_base[channel_id] + 0x6E4, 0x3220008F);
	TVD_WRITE32(tvd_base[channel_id] + 0x6E8, 0xA02D0E28);
	TVD_WRITE32(tvd_base[channel_id] + 0x6EC, 0x00000054);
	TVD_WRITE32(tvd_base[channel_id] + 0x6F0, 0x43174537);
	TVD_WRITE32(tvd_base[channel_id] + 0x6F4, 0x0045713C);
	TVD_WRITE32(tvd_base[channel_id] + 0x6F8, 0x00001A50);
	TVD_WRITE32(tvd_base[channel_id] + 0x6FC, 0x4414050D);
	TVD_WRITE32(tvd_base[channel_id] + 0x700, 0x11A00020);
	TVD_WRITE32(tvd_base[channel_id] + 0x704, 0x64000450);
	TVD_WRITE32(tvd_base[channel_id] + 0x708, 0x410A0028);
	TVD_WRITE32(tvd_base[channel_id] + 0x70C, 0x00014455);
	TVD_WRITE32(tvd_base[channel_id] + 0x710, 0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0x714, 0x31501A48);
	TVD_WRITE32(tvd_base[channel_id] + 0x718, 0x8C9F0000);
	TVD_WRITE32(tvd_base[channel_id] + 0x71C, 0x08001020);
	TVD_WRITE32(tvd_base[channel_id] + 0x720, 0x08081E14);
	TVD_WRITE32(tvd_base[channel_id] + 0x724, 0x0A100008);
	TVD_WRITE32(tvd_base[channel_id] + 0x728, 0x14061608);
	TVD_WRITE32(tvd_base[channel_id] + 0x72C, 0x0C0A2808);
	TVD_WRITE32(tvd_base[channel_id] + 0x730, 0x0A149000);
	TVD_WRITE32(tvd_base[channel_id] + 0x734, 0x55000019);
	TVD_WRITE32(tvd_base[channel_id] + 0x738, 0x0311041A);
	TVD_WRITE32(tvd_base[channel_id] + 0x73C, 0x8C0A100A);
	TVD_WRITE32(tvd_base[channel_id] + 0x740, 0x01245678);
	TVD_WRITE32(tvd_base[channel_id] + 0x744, 0x02345678);
	TVD_WRITE32_MASK(tvd_base[channel_id] + 0x748, 0x00080014, 0xCFFFFFFF);
	TVD_WRITE32(tvd_base[channel_id] + 0x74C, 0x00020A34);
	TVD_WRITE32(tvd_base[channel_id] + 0x750, 0x047C0558);
	TVD_WRITE32(tvd_base[channel_id] + 0x754, 0x0088011A);
	TVD_WRITE32(tvd_base[channel_id] + 0x758, 0x00009666);
	TVD_WRITE32(tvd_base[channel_id] + 0x75C, 0x000781FF);
	TVD_WRITE32(tvd_base[channel_id] + 0x760, 0x90000000);
	TVD_WRITE32(tvd_base[channel_id] + 0x764, 0x00000001);
	TVD_WRITE32(tvd_base[channel_id] + 0x768, 0x0F051818);
	TVD_WRITE32(tvd_base[channel_id] + 0x76C, 0x00191014);
	TVD_WRITE32(tvd_base[channel_id] + 0x770, 0x23645050);
	TVD_WRITE32(tvd_base[channel_id] + 0x774, 0x64641432);
	TVD_WRITE32(tvd_base[channel_id] + 0x778, 0x1E519696);
	TVD_WRITE32(tvd_base[channel_id] + 0x77C, 0x10169696);
	TVD_WRITE32(tvd_base[channel_id] + 0x780, 0x053C501E);
	TVD_WRITE32(tvd_base[channel_id] + 0x784, 0x820A325A);
	TVD_WRITE32(tvd_base[channel_id] + 0x788, 0x45FF3359);
	TVD_WRITE32(tvd_base[channel_id] + 0x78C, 0xFFFFFFFF);
	TVD_WRITE32(tvd_base[channel_id] + 0x790, 0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0x794, 0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0x798, 0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0x79C, 0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0x7A0, 0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0x7A4, 0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0x7A8, 0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0x7AC, 0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0x7B0, 0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0x7B4, 0x100181FA);
	TVD_WRITE32(tvd_base[channel_id] + 0x7B8, 0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0x7BC, 0x00000000);

	TVD_WRITE32(tvd_base[channel_id] + 0x7D8, 0x020F42D0);
	TVD_WRITE32(tvd_base[channel_id] + 0x7DC, 0x00000008);
	TVD_WRITE32(tvd_base[channel_id] + 0x7E0, 0x0F0000A0);
	TVD_WRITE32(tvd_base[channel_id] + 0x7E4, 0x11000020);
	TVD_WRITE32(tvd_base[channel_id] + 0x7E8, 0xC000C000);
	TVD_WRITE32(tvd_base[channel_id] + 0x7EC, 0x00CC0C50);
	TVD_WRITE32(tvd_base[channel_id] + 0x7F0, 0x80008000);
	TVD_WRITE32(tvd_base[channel_id] + 0x7F4, 0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0x7F8, 0x80000000);
	TVD_WRITE32(tvd_base[channel_id] + 0x7FC, 0xD0090901);
	TVD_WRITE32(tvd_base[channel_id] + 0x800, 0x00060604);
	TVD_WRITE32(tvd_base[channel_id] + 0x804, 0x0B9000FF);
	TVD_WRITE32(tvd_base[channel_id] + 0x808, 0x001400FC);
	TVD_WRITE32(tvd_base[channel_id] + 0x80C, 0x005071AD);
	TVD_WRITE32(tvd_base[channel_id] + 0x810, 0xC3080050);
	TVD_WRITE32(tvd_base[channel_id] + 0x814, 0x0F120700);
	TVD_WRITE32(tvd_base[channel_id] + 0x818, 0x02100578);
	TVD_WRITE32(tvd_base[channel_id] + 0x81C, 0x65004020);
	TVD_WRITE32(tvd_base[channel_id] + 0x820, 0x0000F00F);
	TVD_WRITE32(tvd_base[channel_id] + 0x824, 0x14140F0F);
	TVD_WRITE32(tvd_base[channel_id] + 0x828, 0x0A0A0A0A);
	TVD_WRITE32(tvd_base[channel_id] + 0x82C, 0x77700000);
	TVD_WRITE32(tvd_base[channel_id] + 0x830, 0x00A0A000);
	TVD_WRITE32(tvd_base[channel_id] + 0x834, 0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0x838, 0x4400A00A);
	TVD_WRITE32(tvd_base[channel_id] + 0x83C, 0x14000000);
	TVD_WRITE32(tvd_base[channel_id] + 0x840, 0x00003010);
	TVD_WRITE32(tvd_base[channel_id] + 0x844, 0x00201007);
	TVD_WRITE32(tvd_base[channel_id] + 0x848, 0x00201007);
	TVD_WRITE32(tvd_base[channel_id] + 0x84C, 0x0700100A);
	TVD_WRITE32(tvd_base[channel_id] + 0x850, 0x00208858);
	TVD_WRITE32(tvd_base[channel_id] + 0x854, 0x14001010);
	TVD_WRITE32(tvd_base[channel_id] + 0x858, 0x00141414);
	TVD_WRITE32(tvd_base[channel_id] + 0x85C, 0x10141414);
	TVD_WRITE32(tvd_base[channel_id] + 0x860, 0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0x864, 0x00010000);
	TVD_WRITE32(tvd_base[channel_id] + 0x868, 0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0x86C, 0xC0400C0A);
	TVD_WRITE32(tvd_base[channel_id] + 0x870, 0x00020820);
	TVD_WRITE32(tvd_base[channel_id] + 0x874, 0xC0016C01);
	TVD_WRITE32(tvd_base[channel_id] + 0x878, 0x0089A022);
	TVD_WRITE32(tvd_base[channel_id] + 0x87C, 0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0x880, 0x00060604);
	TVD_WRITE32(tvd_base[channel_id] + 0x884, 0x000001FC);
	TVD_WRITE32(tvd_base[channel_id] + 0x888, 0x005071AD);
	TVD_WRITE32(tvd_base[channel_id] + 0x88C, 0x00002010);
	TVD_WRITE32(tvd_base[channel_id] + 0x890, 0x0008F060);
	TVD_WRITE32(tvd_base[channel_id] + 0x894, 0x0020A000);
	TVD_WRITE32(tvd_base[channel_id] + 0x898, 0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0x89C, 0x00606200);
	TVD_WRITE32(tvd_base[channel_id] + 0x8A0, 0xC000C000);
	TVD_WRITE32(tvd_base[channel_id] + 0x8A4, 0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0x8A8, 0x0000000F);
	TVD_WRITE32(tvd_base[channel_id] + 0x8AC, 0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0x8B0, 0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0x8B4, 0x00000078);
	TVD_WRITE32(tvd_base[channel_id] + 0x8B8, 0xC0016C01);
	TVD_WRITE32(tvd_base[channel_id] + 0x8BC, 0x6EC86EC8);
	TVD_WRITE32(tvd_base[channel_id] + 0x8C0, 0x65031E04);
	TVD_WRITE32(tvd_base[channel_id] + 0x8C4, 0x12884420);
	TVD_WRITE32(tvd_base[channel_id] + 0x8C8, 0x0185C501);
	TVD_WRITE32(tvd_base[channel_id] + 0x8CC, 0xC1CBA860);
	TVD_WRITE32(tvd_base[channel_id] + 0x8D0, 0x70000BB8);
	TVD_WRITE32(tvd_base[channel_id] + 0x8D4, 0x7F294A73);
	TVD_WRITE32(tvd_base[channel_id] + 0x8D8, 0x83C29000);
	TVD_WRITE32(tvd_base[channel_id] + 0x8DC, 0x74D08610);
	TVD_WRITE32(tvd_base[channel_id] + 0x8E0, 0x195D1CCD);
	TVD_WRITE32(tvd_base[channel_id] + 0x8E4, 0x2ACBBEF8);
	TVD_WRITE32(tvd_base[channel_id] + 0x8E8, 0x03F81CCD);
	TVD_WRITE32(tvd_base[channel_id] + 0x8EC, 0xF3935CD6);
	TVD_WRITE32(tvd_base[channel_id] + 0x8F0, 0x532714C2);
	TVD_WRITE32(tvd_base[channel_id] + 0x8F4, 0x471848D4);
	TVD_WRITE32(tvd_base[channel_id] + 0x8F8, 0x80000000);
	TVD_WRITE32(tvd_base[channel_id] + 0x8FC, 0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0x900, 0x62F32005);
	TVD_WRITE32(tvd_base[channel_id] + 0x904, 0x120855A0);
	TVD_WRITE32(tvd_base[channel_id] + 0x908, 0x0185D0B9);
	TVD_WRITE32(tvd_base[channel_id] + 0x90C, 0xC1CAA800);
	TVD_WRITE32(tvd_base[channel_id] + 0x910, 0x70000BB8);
	TVD_WRITE32(tvd_base[channel_id] + 0x914, 0x7F284231);
	TVD_WRITE32(tvd_base[channel_id] + 0x918, 0x83C28F00);
	TVD_WRITE32(tvd_base[channel_id] + 0x91C, 0x74D08610);
	TVD_WRITE32(tvd_base[channel_id] + 0x920, 0x195D1CCD);
	TVD_WRITE32(tvd_base[channel_id] + 0x924, 0x413B0252);
	TVD_WRITE32(tvd_base[channel_id] + 0x928, 0x0440283B);
	TVD_WRITE32(tvd_base[channel_id] + 0x92C, 0xF403DCF7);
	TVD_WRITE32(tvd_base[channel_id] + 0x930, 0x532714C2);
	TVD_WRITE32(tvd_base[channel_id] + 0x934, 0x87184814);
	TVD_WRITE32(tvd_base[channel_id] + 0x938, 0x08000800);
	TVD_WRITE32(tvd_base[channel_id] + 0x93C, 0x00000800);
	TVD_WRITE32(tvd_base[channel_id] + 0x940, 0x10281400);
	TVD_WRITE32(tvd_base[channel_id] + 0x944, 0x04D0BB40);
	TVD_WRITE32(tvd_base[channel_id] + 0x948, 0x10281400);
	TVD_WRITE32(tvd_base[channel_id] + 0x94C, 0x04D0BB40);
	TVD_WRITE32(tvd_base[channel_id] + 0x950, 0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0x954, 0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0x958, 0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0x95C, 0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0x960, 0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0x964, 0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0x968, 0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0x96C, 0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0x970, 0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0x974, 0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0x978, 0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0x97C, 0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0x980, 0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0x984, 0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0x988, 0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0x98C, 0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0x990, 0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0x994, 0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0x998, 0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0x99C, 0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0x9A0, 0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0x9A4, 0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0x9A8, 0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0x9AC, 0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0x9B0, 0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0x9B4, 0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0x9B8, 0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0x9BC, 0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0x9C0, 0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0x9C4, 0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0x9C8, 0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0x9CC, 0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0x9D0, 0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0x9D4, 0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0x9D8, 0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0x9DC, 0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0x9E0, 0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0x9E4, 0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0x9E8, 0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0x9EC, 0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0x9F0, 0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0x9F4, 0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0x9F8, 0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0x9FC, 0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0xA00, 0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0xA04, 0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0xA08, 0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0xA0C, 0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0xA10, 0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0xA14, 0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0xA18, 0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0xA1C, 0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0xA20, 0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0xA24, 0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0xA28, 0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0xA2C, 0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0xA30, 0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0xA34, 0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0xA38, 0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0xA3C, 0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0xA40, 0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0xA44, 0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0xA48, 0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0xA4C, 0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0xA50, 0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0xA54, 0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0xA58, 0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0xA5C, 0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0xA60, 0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0xA64, 0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0xA68, 0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0xA6C, 0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0xA70, 0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0xA74, 0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0xA78, 0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0xA7C, 0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0xA80, 0x0FF3FCFF);
	TVD_WRITE32(tvd_base[channel_id] + 0xA84, 0x80008000);
	TVD_WRITE32(tvd_base[channel_id] + 0xA88, 0x80000000);
	TVD_WRITE32(tvd_base[channel_id] + 0xA8C, 0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0xA90, 0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0xA94, 0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0xA98, 0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0xA9C, 0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0xAA0, 0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0xAA4, 0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0xAA8, 0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0xAAC, 0x00000000);
	TVD_WRITE32(tvd_base[channel_id] + 0xAB0, 0x22F08C03);
	TVD_WRITE32(tvd_base[channel_id] + 0xAB4, 0x8F0070D4);
	TVD_WRITE32(tvd_base[channel_id] + 0xAB8, 0x42400800);
	TVD_WRITE32(tvd_base[channel_id] + 0xABC, 0x82000820);
	
	TVD_WRITE32(tvd_base[channel_id] + 0x5aC, 0x000010C0);  //bit 31
	TVD_WRITE32(tvd_base[channel_id] + 0x59C, 0x21E2c0A7); //H start
	TVD_WRITE32(tvd_base[channel_id] + 0x5c4, 0x18181617);  //v start


	return; 
}

static void bTvd_Mode_Detect(TVD_CHANNEL_ID channel_id)
{
	u8 u1TvdMode;

	u1TvdMode = bHwTvdMode(tvd_base[channel_id]);

	switch (u1TvdMode) {
	case AV_PAL_N:
		TVD_LOG(TVD_LOG_LVL_WARN, "TVD mode auto detect: PAL_N.\n");
		if(channel_id == TVD_CH_0 && tvd3DComb == true){
			PAL_3D_setting(channel_id);
		}else{
			vPAL_N_2D_Setting(channel_id);
		}
		tvd_special_setting(channel_id);
		TVD_WRITE32(tvd_base[channel_id] + 0x540, (TVD_READ32(tvd_base[channel_id] + 0x540)& (~(0x1 << 4))) | (0x1 << 4));
		break;

	case AV_PAL:
		TVD_LOG(TVD_LOG_LVL_WARN, "TVD mode auto detect: PAL.\n");
		if(channel_id == TVD_CH_0 && tvd3DComb == true){
			PAL_3D_setting(channel_id);
		}else{
			vPAL_2D_Setting(channel_id);
		}
		tvd_special_setting(channel_id);
		TVD_WRITE32(tvd_base[channel_id] + 0x540, (TVD_READ32(tvd_base[channel_id] + 0x540)& (~(0x1 << 4))) | (0x1 << 4));
		break;

	case AV_PAL_M:
		TVD_LOG(TVD_LOG_LVL_WARN, "TVD mode auto detect: PAL_M.\n");
		if(channel_id == TVD_CH_0 && tvd3DComb == true){
			PAL_3D_setting(channel_id);
		}else{
			vPAL_M_2D_Setting(channel_id);
		}
		tvd_special_setting(channel_id);
		TVD_WRITE32(tvd_base[channel_id] + 0x540, (TVD_READ32(tvd_base[channel_id] + 0x540)& (~(0x1 << 4))) );
		break;

	case AV_NTSC:
		TVD_LOG(TVD_LOG_LVL_WARN, "TVD mode auto detect: NTSC.\n");
		if(channel_id == TVD_CH_0 && tvd3DComb == true){
			NTSC_3D_setting(channel_id);
		}else{
			vNTSC_2D_Setting(channel_id);
		}
		tvd_special_setting(channel_id);
		TVD_WRITE32(tvd_base[channel_id] + 0x540, (TVD_READ32(tvd_base[channel_id] + 0x540)& (~(0x1 << 4))) );
		break;

	case AV_SECAM:
		TVD_LOG(TVD_LOG_LVL_WARN, "TVD mode auto detect: SECAM.\n");
		//vTvd_SECAM_Setting(channel_id);
		vSECAM_2D_Setting(channel_id);
		tvd_special_setting(channel_id);
		TVD_WRITE32(tvd_base[channel_id] + 0x540, (TVD_READ32(tvd_base[channel_id] + 0x540)& (~(0x1 << 4))) ); 
		break;

	case AV_PAL_60:
		TVD_LOG(TVD_LOG_LVL_WARN, "TVD mode auto detect: PAL_60.\n");
		if(channel_id == TVD_CH_0 && tvd3DComb == true){
			PAL_3D_setting(channel_id);
		}else{
			vPAL_60_2D_Setting(channel_id);
		}
		tvd_special_setting(channel_id);
		TVD_WRITE32(tvd_base[channel_id] + 0x540, (TVD_READ32(tvd_base[channel_id] + 0x540)& (~(0x1 << 4))) ); 
		break;

	case AV_UNSTABLE:
		TVD_LOG(TVD_LOG_LVL_WARN, "TVD mode auto detect: unstable.\n");
		break;

	case AV_NTSC443:
		TVD_LOG(TVD_LOG_LVL_WARN, "TVD mode auto detect: NTSC443.\n");
		if(channel_id == TVD_CH_0 && tvd3DComb == true){
			NTSC_3D_setting(channel_id);
		}else{
			vNTSC443_2D_Setting(channel_id);
		}
		tvd_special_setting(channel_id);
		TVD_WRITE32(tvd_base[channel_id] + 0x540, (TVD_READ32(tvd_base[channel_id] + 0x540)& (~(0x1 << 4))) );
		break;

	default:
		TVD_LOG(TVD_LOG_LVL_WARN, "TVD mode auto detect: NONE.\n");
		break;
	}

	/*  vdo scan line front */
	/*   HAL_WRITE32((IO_BASE_VA+0x421E0), 0x008000B4); */
}

void Tvd_Core_SetMode(TVD_CHANNEL_ID channel_id, u32 u4TvdMode)
{
	switch (u4TvdMode) {
	case AV_PAL_N:
		TVD_LOG(TVD_LOG_LVL_WARN, "TVD mode auto detect: PAL_N.\n");
		if(channel_id == TVD_CH_0 && tvd3DComb == true){
			PAL_3D_setting(channel_id);
		}else{
			vPAL_N_2D_Setting(channel_id);
		}
		tvd_special_setting(channel_id);
		TVD_WRITE32(tvd_base[channel_id] + 0x540, (TVD_READ32(tvd_base[channel_id] + 0x540)& (~(0x1 << 4))) | (0x1 << 4));
		break;

	case AV_PAL:
		TVD_LOG(TVD_LOG_LVL_WARN, "TVD mode auto detect: PAL.\n");
		if(channel_id == TVD_CH_0 && tvd3DComb == true){
			PAL_3D_setting(channel_id);
		}else{
			vPAL_2D_Setting(channel_id);
		}
		tvd_special_setting(channel_id);
		TVD_WRITE32(tvd_base[channel_id] + 0x540, (TVD_READ32(tvd_base[channel_id] + 0x540)& (~(0x1 << 4))) | (0x1 << 4));
		break;

	case AV_PAL_M:
		TVD_LOG(TVD_LOG_LVL_WARN, "TVD mode auto detect: PAL_M.\n");
		if(channel_id == TVD_CH_0 && tvd3DComb == true){
			PAL_3D_setting(channel_id);
		}else{
			vPAL_M_2D_Setting(channel_id);
		}
		tvd_special_setting(channel_id);
		TVD_WRITE32(tvd_base[channel_id] + 0x540, (TVD_READ32(tvd_base[channel_id] + 0x540)& (~(0x1 << 4))) ); 
		break;

	case AV_NTSC:
		TVD_LOG(TVD_LOG_LVL_WARN, "TVD mode auto detect: NTSC.\n");
		if(channel_id == TVD_CH_0 && tvd3DComb == true){
			NTSC_3D_setting(channel_id);
		}else{
			vNTSC_2D_Setting(channel_id);
		}
		tvd_special_setting(channel_id);
		TVD_WRITE32(tvd_base[channel_id] + 0x540, (TVD_READ32(tvd_base[channel_id] + 0x540)& (~(0x1 << 4))) ); 
		break;

	case AV_SECAM:
		TVD_LOG(TVD_LOG_LVL_WARN, "TVD mode auto detect: SECAM.\n");
		vSECAM_2D_Setting(channel_id);
		tvd_special_setting(channel_id);
		TVD_WRITE32(tvd_base[channel_id] + 0x540, (TVD_READ32(tvd_base[channel_id] + 0x540)& (~(0x1 << 4))) );
		break;

	case AV_PAL_60:
		TVD_LOG(TVD_LOG_LVL_WARN, "TVD mode auto detect: PAL_60.\n");
		if(channel_id == TVD_CH_0 && tvd3DComb == true){
			PAL_3D_setting(channel_id);
		}else{
			vPAL_60_2D_Setting(channel_id);
		}
		tvd_special_setting(channel_id);
		TVD_WRITE32(tvd_base[channel_id] + 0x540, (TVD_READ32(tvd_base[channel_id] + 0x540)& (~(0x1 << 4))) ); 
		break;

	case AV_UNSTABLE:
		TVD_LOG(TVD_LOG_LVL_WARN, "TVD mode auto detect: unstable.\n");
		break;

	case AV_NTSC443:
		TVD_LOG(TVD_LOG_LVL_WARN, "TVD mode auto detect: NTSC443.\n");
		if(channel_id == TVD_CH_0 && tvd3DComb == true){
			NTSC_3D_setting(channel_id);
		}else{
			vNTSC443_2D_Setting(channel_id);
		}
		tvd_special_setting(channel_id);
		TVD_WRITE32(tvd_base[channel_id] + 0x540, (TVD_READ32(tvd_base[channel_id] + 0x540)& (~(0x1 << 4))) ); 
		break;

	default:
		TVD_LOG(TVD_LOG_LVL_WARN, "TVD mode auto detect: NONE.\n");
		break;
	}
}


u32 Tvd_Core_GetMode(TVD_CHANNEL_ID channel_id)
{
	u32 u4TvdMode = bHwTvdMode(tvd_base[channel_id]);

	switch (u4TvdMode) {
	case AV_PAL_N:
		TVD_LOG(TVD_LOG_LVL_WARN, "TVD Mode: PAL_N.\n");
		break;

	case AV_PAL:
		TVD_LOG(TVD_LOG_LVL_WARN, "TVD Mode: PAL.\n");
		break;

	case AV_PAL_M:
		TVD_LOG(TVD_LOG_LVL_WARN, "TVD Mode: PAL_M.\n");
		break;

	case AV_NTSC:
		TVD_LOG(TVD_LOG_LVL_WARN, "TVD Mode: NTSC.\n");
		break;

	case AV_SECAM:
		TVD_LOG(TVD_LOG_LVL_WARN, "TVD Mode: SECAM.\n");
		break;

	case AV_PAL_60:
		TVD_LOG(TVD_LOG_LVL_WARN, "TVD Mode: PAL_60.\n");
		break;

	case AV_UNSTABLE:
		TVD_LOG(TVD_LOG_LVL_WARN, "TVD Mode: unstable.\n");
		break;

	case AV_NTSC443:
		TVD_LOG(TVD_LOG_LVL_WARN, "TVD Mode: NTSC443.\n");
		break;

	default:
		TVD_LOG(TVD_LOG_LVL_WARN, "TVD Mode: NONE.\n");
		break;
	}

	return u4TvdMode;
}

/**************************************************************************

* @brief TVD VPRES interrupt status

*     Get the status of VPRES: VPRES  ON ( TVD SIGNAL COME)  /  VPRES OFF (TVD SIGNAL GO)

* @param .

* @return status of VPRES

**************************************************************************/

TVD_SIG_STATE_T tvd_core_get_signal_state(TVD_CHANNEL_ID channel_id)
{
	u32 vpres_on_off = TVD_READ32(tvd_base[channel_id] + STA_REG10);
	TVD_SIG_STATE_T signal_status = TVD_SIG_NONE;

	if ((vpres_on_off & VPRES_ON) && !(vpres_on_off & VPRES_OFF)) {
		signal_status = TVD_SIG_READY;
	} else if (!(vpres_on_off & VPRES_ON) && (vpres_on_off & VPRES_OFF)) {
		signal_status = TVD_SIG_LOST;
	} else {
		signal_status = TVD_SIG_READY;
		TVD_LOG(TVD_LOG_LVL_ERR, "Default Signal Status\n");
	}

	return signal_status;
}

/**************************************************************************

* IRQ Functions

**************************************************************************/



/**************************************************************************

* @brief TVD interrupt status clear

*     Clear TVD interrupt status

* @param TVD IRQ status, Wirte 1 to clear status.

* @return None

**************************************************************************/

void Tvd_Core_IrqClear(TVD_CHANNEL_ID channel_id, u32 u4IrqStatus)
{
	TVD_WRITE32(tvd_base[channel_id] + TVD_INTR_STA, u4IrqStatus);
}


/**************************************************************************

* @brief TVD interrupt enable

*     Only setting Vsync / TimerA / Mode Switching / VPRES for ac83xx.

* @param False is Disable, True is Enable

* @return None

**************************************************************************/

void Tvd_Core_IrqEnable(TVD_CHANNEL_ID channel_id, bool fgEnable)
{
	if (fgEnable) {
		TVD_CLR_BIT(tvd_base[channel_id] + TVD_INTR_EN, (INTR_TIMERA_TVD |INTR_VSYNC_TVD  | INTR_MODE_TVD | INTR_VPRES_TVD));
	}

	else {
		TVD_SET_BIT(tvd_base[channel_id] + TVD_INTR_EN, (INTR_TIMERA_TVD |INTR_VSYNC_TVD  | INTR_MODE_TVD | INTR_VPRES_TVD));
	}
}

u32 Tvd_Core_IrqStatus(TVD_CHANNEL_ID channel_id)
{
	u32 u4IrqEnable, u4IrqStatus = 0;

	u4IrqEnable = (~(TVD_READ32(tvd_base[channel_id] + TVD_INTR_EN))) &
		      (INTR_TIMERA_TVD | INTR_MODE_TVD | INTR_VPRES_TVD | INTR_VSYNC_TVD);

	u4IrqStatus = TVD_READ32(tvd_base[channel_id] + TVD_INTR_STA);
	TVD_LOG(TVD_LOG_LVL_DBG, "IrqEnable = 0x%x, IrqStatus = 0x%x\n", u4IrqEnable, u4IrqStatus);

	Tvd_Core_IrqClear(channel_id, u4IrqStatus);

	/* VERIFY(BIM_ClearIrq(VECTOR_VDOIN)); */

	u4IrqStatus &= u4IrqEnable;
	return u4IrqStatus;
}

#if defined(__ARM2__)
void CKGEN_AgtOnClk(e_CLK_T eAgt)
{
	u32 u4Tmp, u4Reset;

	if (eAgt >= e_CLK_PWM0 && eAgt <= e_CLK_PWM3) {
		u4Tmp = (*((volatile u32 *)(IO_UCV_BASE + REG_RW_CLKGATE_CFG3)));
		u4Reset = (*((volatile u32 *)(IO_UCV_BASE + REG_RW_SYNC_RESET_CFG3)));
	} else {
		u4Tmp = (*((volatile u32 *)(IO_UCV_BASE + REG_RW_CLKGATE_CFG6)));
		u4Reset = (*((volatile u32 *)(IO_UCV_BASE + REG_RW_SYNC_RESET_CFG6)));
	}

	if (eAgt == e_CLK_TVD1) {
		u4Tmp = u4Tmp | (CLK_PDN_TVD1);
		u4Reset = u4Reset | (CLK_RESET_TVD1);
	} else if (eAgt == e_CLK_TVD2) {
		u4Tmp = u4Tmp | (CLK_PDN_TVD2);
		u4Reset = u4Reset | (CLK_RESET_TVD1);
	} else if (eAgt == e_CLK_WRITE_CHANEL) {
		u4Tmp = u4Tmp | (CLK_PDN_WRITE_CHANEL);
		u4Reset = u4Reset | (CLK_RESET_WRITE_CHANEL);
	} else if (eAgt == e_CLK_WRITE_CHANEL_2) {
		u4Tmp = u4Tmp | (CLK_PDN_WRITE_CHANEL2);
		u4Reset = u4Reset | (CLK_RESET_WRITE_CHANEL2);
	} else if (eAgt == e_CLK_PWM0) {
		u4Tmp = u4Tmp | (CLK_PDN_PWM0);
		u4Reset = u4Reset | (CLK_RESET_PWM0);
	} else if (eAgt == e_CLK_PWM1) {
		u4Tmp = u4Tmp | (CLK_PDN_PWM1);
		u4Reset = u4Reset | (CLK_RESET_PWM1);
	} else if (eAgt == e_CLK_PWM2) {
		u4Tmp = u4Tmp | (CLK_PDN_PWM2);
		u4Reset = u4Reset | (CLK_RESET_PWM2);
	} else if (eAgt == e_CLK_PWM3) {
		u4Tmp = u4Tmp | (CLK_PDN_PWM3);
		u4Reset = u4Reset | (CLK_RESET_PWM3);
	}


	if (eAgt >= e_CLK_PWM0 && eAgt <= e_CLK_PWM3) {
		(*((volatile u32 *)(IO_UCV_BASE + REG_RW_CLKGATE_CFG3))) = u4Tmp;
		(*((volatile u32 *)(IO_UCV_BASE + REG_RW_SYNC_RESET_CFG3))) = u4Reset;
	} else {
		(*((volatile u32 *)(IO_UCV_BASE + REG_RW_CLKGATE_CFG6))) = u4Tmp;
		(*((volatile u32 *)(IO_UCV_BASE + REG_RW_SYNC_RESET_CFG6))) = u4Reset;
	}
}


void CKGEN_AgtOffClk(e_CLK_T eAgt)
{
	u32 u4Tmp, u4Reset;

	if (eAgt >= e_CLK_PWM0 && eAgt <= e_CLK_PWM3) {
		u4Tmp = (*((volatile u32 *)(IO_UCV_BASE + REG_RW_CLKGATE_CFG3)));
		u4Reset = (*((volatile u32 *)(IO_UCV_BASE + REG_RW_SYNC_RESET_CFG3)));
	} else {
		u4Tmp = (*((volatile u32 *)(IO_UCV_BASE + REG_RW_CLKGATE_CFG6)));
		u4Reset = (*((volatile u32 *)(IO_UCV_BASE + REG_RW_SYNC_RESET_CFG6)));
	}

	if (eAgt == e_CLK_TVD1) {
		u4Tmp = u4Tmp & (~CLK_PDN_TVD1);
		u4Reset = u4Reset & (~CLK_RESET_TVD1);
	} else if (eAgt == e_CLK_TVD2) {
		u4Tmp = u4Tmp & (~CLK_PDN_TVD2);
		u4Reset = u4Reset & (~CLK_RESET_TVD1);
	} else if (eAgt == e_CLK_WRITE_CHANEL) {
		u4Tmp = u4Tmp & (~CLK_PDN_WRITE_CHANEL);
		u4Reset = u4Reset & (~CLK_RESET_WRITE_CHANEL);
	} else if (eAgt == e_CLK_WRITE_CHANEL_2) {
		u4Tmp = u4Tmp & (~CLK_PDN_WRITE_CHANEL2);
		u4Reset = u4Reset & (~CLK_RESET_WRITE_CHANEL2);
	} else if (eAgt == e_CLK_PWM0) {
		u4Tmp = u4Tmp & (~CLK_PDN_PWM0);
		u4Reset = u4Reset & (~CLK_RESET_PWM0);
	} else if (eAgt == e_CLK_PWM1) {
		u4Tmp = u4Tmp & (~CLK_PDN_PWM1);
		u4Reset = u4Reset & (~CLK_RESET_PWM1);
	} else if (eAgt == e_CLK_PWM2) {
		u4Tmp = u4Tmp & (~CLK_PDN_PWM2);
		u4Reset = u4Reset & (~CLK_RESET_PWM2);
	} else if (eAgt == e_CLK_PWM3) {
		u4Tmp = u4Tmp & (~CLK_PDN_PWM3);
		u4Reset = u4Reset & (~CLK_RESET_PWM3);
	}

	if (eAgt >= e_CLK_PWM0 && eAgt <= e_CLK_PWM3) {
		(*((volatile u32 *)(IO_UCV_BASE + REG_RW_SYNC_RESET_CFG3))) = u4Reset;
		(*((volatile u32 *)(IO_UCV_BASE + REG_RW_CLKGATE_CFG3))) = u4Tmp;
	} else {
		(*((volatile u32 *)(IO_UCV_BASE + REG_RW_SYNC_RESET_CFG6))) = u4Reset;
		(*((volatile u32 *)(IO_UCV_BASE + REG_RW_CLKGATE_CFG6))) = u4Tmp;
	}
}

void CKGEN_AgtSelClk(e_CLK_T eAgt, u32 u4Sel)
{
	u32 u4Tmp;

	if (eAgt < e_CLK_SEL_MAX) {     /* CONFIG 10:*/
		u4Tmp = (*((volatile u32 *)(IO_UCV_BASE + REG_RW_AP_REG10)));

		switch (eAgt) {
		case e_CLK_SEL_PWM3:
			u4Sel = (u4Sel << CLK_REG10_PWM3_SEL_OFFSET) & CLK_REG10_PWM3_SEL_MASK;
			u4Tmp = u4Tmp & (~CLK_REG10_PWM3_SEL_MASK);
			u4Tmp = u4Tmp | u4Sel;
			break;

		case e_CLK_SEL_PWM2:
			u4Sel = (u4Sel << CLK_REG10_PWM2_SEL_OFFSET) & CLK_REG10_PWM2_SEL_MASK;
			u4Tmp = u4Tmp & (~CLK_REG10_PWM2_SEL_MASK);
			u4Tmp = u4Tmp | u4Sel;
			break;

		case e_CLK_SEL_PWM1:
			u4Sel = (u4Sel << CLK_REG10_PWM1_SEL_OFFSET) & CLK_REG10_PWM1_SEL_MASK;
			u4Tmp = u4Tmp & (~CLK_REG10_PWM1_SEL_MASK);
			u4Tmp = u4Tmp | u4Sel;
			break;

		case e_CLK_SEL_PWM0:
			u4Sel = (u4Sel << CLK_REG10_PWM0_SEL_OFFSET) & CLK_REG10_PWM0_SEL_MASK;
			u4Tmp = u4Tmp & (~CLK_REG10_PWM0_SEL_MASK);
			u4Tmp = u4Tmp | u4Sel;
			break;

		default:
			return;
		}

		(*((volatile u32 *)(IO_UCV_BASE + REG_RW_AP_REG10))) = u4Tmp;

	}

}


#endif

void Tvd_ClkOnOff(TVD_CHANNEL_ID channel_id, bool fgOn)
{
	if (fgOn) {
#ifndef __ARM2__
		
		switch(channel_id){
			case TVD_CH_0:
				//clk_prepare_enable(clk_ac8317_tvd1);
				//clk_prepare_enable(clk_ac8317_tvd2);
				TVD_ANA_WRITE32(0xB4, TVD_ANA_READ32(0xB4)|(0x1<<1)); // TVD1 clk,  enable.
				TVD_ANA_WRITE32(0xD0, TVD_ANA_READ32(0xD0)|(0x1<<1)); // TVD1 clk,  enable.TVD reset
				break;
				
			case TVD_CH_1:
				//clk_prepare_enable(clk_ac8317_tvd1);
				//clk_prepare_enable(clk_ac8317_tvd2);
				TVD_ANA_WRITE32(0xB4, TVD_ANA_READ32(0xB4)|(0x1<<2)); // TVD1 clk,  enable.
				TVD_ANA_WRITE32(0xD0, TVD_ANA_READ32(0xD0)|(0x1<<2)); // TVD1 clk,  enable.TVD reset
				break;
				
			case TVD_CH_2:
				//clk_prepare_enable(clk_ac8317_tvd1);
				//clk_prepare_enable(clk_ac8317_tvd2);
				TVD_ANA_WRITE32(0xB4, TVD_ANA_READ32(0xB4)|(0x1<<9)); // TVD1 clk,  enable.
				TVD_ANA_WRITE32(0xD0, TVD_ANA_READ32(0xD0)|(0x1<<9)); // TVD1 clk,  enable.TVD reset
				break;
				
			case TVD_CH_3:
				//clk_prepare_enable(clk_ac8317_tvd1);
				//clk_prepare_enable(clk_ac8317_tvd2);
				TVD_ANA_WRITE32(0xB4, TVD_ANA_READ32(0xB4)|(0x1<<12)); // TVD1 clk,  enable.
				TVD_ANA_WRITE32(0xD0, TVD_ANA_READ32(0xD0)|(0x1<<12)); // TVD1 clk,  enable.TVD reset
				break;
			default:
				break;
		}
#else
		TVD_ANA_WRITE32(0xB4, TVD_ANA_READ32(0xB4)|(0x1<<1)); // TVD1 clk,  enable.
		TVD_ANA_WRITE32(0xD0, TVD_ANA_READ32(0xD0)|(0x1<<1)); // TVD1 clk,  enable.TVD reset
#endif
	} else {
#ifndef __ARM2__
		
		switch(channel_id){
			case TVD_CH_0:
				//clk_disable_unprepare(clk_ac8317_tvd1);
				//clk_disable_unprepare(clk_ac8317_tvd2);
				TVD_ANA_WRITE32(0xD0, TVD_ANA_READ32(0xD0)&(~(0x1<<1))); // TVD1 clk,  disenable.TVD reset
				TVD_ANA_WRITE32(0xB4, TVD_ANA_READ32(0xB4)&(~(0x1<<1))); // TVD1 clk,  disenable.
				
				break;
				
			case TVD_CH_1:
				//clk_disable_unprepare(clk_ac8317_tvd1);
				//clk_disable_unprepare(clk_ac8317_tvd2);
				TVD_ANA_WRITE32(0xD0, TVD_ANA_READ32(0xD0)&(~(0x1<<2))); // TVD1 clk,  disenable.TVD reset
				TVD_ANA_WRITE32(0xB4, TVD_ANA_READ32(0xB4)&(~(0x1<<2))); // TVD1 clk,  disenable.
				
				break;
				
			case TVD_CH_2:
				//clk_disable_unprepare(clk_ac8317_tvd1);
				//clk_disable_unprepare(clk_ac8317_tvd2);
				TVD_ANA_WRITE32(0xD0, TVD_ANA_READ32(0xD0)&(~(0x1<<9))); // TVD1 clk,  disenable.TVD reset
				TVD_ANA_WRITE32(0xB4, TVD_ANA_READ32(0xB4)&(~(0x1<<9))); // TVD1 clk,  disenable.
				break;
				
			case TVD_CH_3:
				//clk_disable_unprepare(clk_ac8317_tvd1);
				//clk_disable_unprepare(clk_ac8317_tvd2);
				TVD_ANA_WRITE32(0xD0, TVD_ANA_READ32(0xD0)&(~(0x1<<12))); // TVD1 clk,  disenable.TVD reset
				TVD_ANA_WRITE32(0xB4, TVD_ANA_READ32(0xB4)&(~(0x1<<12))); // TVD1 clk,  disenable.
				
				break;
			default:
				break;
		}
#else
		TVD_ANA_WRITE32(0xD0, TVD_ANA_READ32(0xD0)&(~(0x1<<1))); // TVD1 clk,  disenable.TVD reset
		TVD_ANA_WRITE32(0xB4, TVD_ANA_READ32(0xB4)&(~(0x1<<1))); // TVD1 clk,  disenable.
		
#endif
	}
}

void Tvd_Register_Rst(TVD_CHANNEL_ID channel_id)
{
	//TVD_SET_BIT(tvd_base[channel_id] + 0x400, 0x1 << 0); /* reset register  */
	TVD_SET_BIT(tvd_base[channel_id] + 0x400, 0x1 << 2); /* reset TVD3D_core  */

	/*because it is level trigger,so we need to recovery it*/
	//TVD_CLR_BIT(tvd_base[channel_id] + 0x400, 0x1 << 0); /* reset register  */
	TVD_CLR_BIT(tvd_base[channel_id] + 0x400, 0x1 << 2); /* reset TVD3D_core  */
}

/**************************************************************************

* @brief TVD Initialization.

* @param

* @return None

**************************************************************************/
u32 Tvd_Core_Init(TVD_CHANNEL_ID channel_id)
{
	/* u32 uErrorCode = ERROR_NONE; */
	TVD_LOG(TVD_LOG_LVL_TRACE, "enter:channel_id = %d\n", channel_id);

	TVD_SET_BIT(tvd_base[channel_id] + 0x56c, 0x1 << 8); /* fix mode detect error  */

	/* the vCVBS_init is need in verification, and not need in emulation */
#if (!TVD_DRV_FPGA_BOARD)
	vCVBS_Init(channel_id);
	TVD_LOG(TVD_LOG_LVL_WARN, "ASIC.\n");
#else
	/* only FPGA to make clock inverse to stable sync level and blank level */
	TVD_WRITE32_MASK(tvd_base[channel_id] + REG_VFE_00, 0x0F, 0x02);
	TVD_LOG(TVD_LOG_LVL_WARN, "FPGA.\n");
#endif
	TVD_SET_BIT(tvd_base[channel_id] + REG_VSRC_07, RG_VSRC_INV_AIDX);  /* select CHA to TVD */
	TVD_WRITE32(tvd_base[channel_id] + REG_DFE_0E, 0x8000306C);/*This value from de fix vpres detect*/
	bTvd_Mode_Detect(channel_id);
	TVD_LOG(TVD_LOG_LVL_INFO, "leave\n");
	return ERROR_NONE;
}

/**************************************************************************

* @brief TVD Initialization.

* @param

* @return None

**************************************************************************/

void Tvd_Core_DeInit(TVD_CHANNEL_ID channel_id)
{
	TVD_LOG(TVD_LOG_LVL_OFF, "enter and disable Tvd Interrupt!\n");
	Tvd_Core_IrqEnable(channel_id, false);
	Tvd_Core_IrqClear(channel_id, 0xFFFFFFFF);
	TVD_LOG(TVD_LOG_LVL_OFF, "leave\n");
}

void Tvd_Color_Process(TVD_CORE_PREVIEW_CFG_T *prPreviewCfg)
{
#if SUPPORT_CALIBRATE_BRIGHTNESS

	u32 u4ColorProcessValS0;
	u32 u4ColorProcessValS1;
	u32 u4ColorProcessValS2;
	u32 u4ColorProcessValS3;

	u8 u1Mask = prPreviewCfg->u1Mask;
	u8 u1Ygain = prPreviewCfg->u1YGain;
	u8 u1Yoffset = prPreviewCfg->u1YOffset;
	u8 u1Uoffset = prPreviewCfg->u1UOffset;
	u8 u1Voffset = prPreviewCfg->u1VOffset;
	u8 u1Ucosgain = prPreviewCfg->u1UCosGain;
	u8 u1Usingain = prPreviewCfg->u1USinGain;
	u8 u1Vcosgain = prPreviewCfg->u1VCosGain;
	u8 u1Vsingain = prPreviewCfg->u1VSinGain;

	/* if you set YGAIN 0~7 bit */
	if (u1Mask & YGAIN_VALID_MASK) {
		u4ColorProcessValS0 = TVD_READ32(COLOR_PROCESS_S0);
		u4ColorProcessValS0 = u4ColorProcessValS0 & 0xFFFF0000;
		u1Ygain = u1Ygain & (u8)0xFFFF;
		u4ColorProcessValS0 = u4ColorProcessValS0 | u1Ygain;
		TVD_WRITE32(COLOR_PROCESS_S0, u4ColorProcessValS0);
		TVD_LOG(TVD_LOG_LVL_OFF, "u4ColorProcessValS0 %u\n", u4ColorProcessValS0);
	}

	/* if you set UCosgain 16~31 and USingain 0~15 */
	if ((u1Mask & UCOSGAIN_VALID_MASK) && (u1Mask & USINGAIN_VALID_MASK)) {
		u4ColorProcessValS1 = TVD_READ32(COLOR_PROCESS_S1);
		u4ColorProcessValS1 = u4ColorProcessValS1 & 0x0U;
		u4ColorProcessValS1 = (u32)((u32)u1Ucosgain << 16) | u1Usingain;
		TVD_WRITE32(COLOR_PROCESS_S1, u4ColorProcessValS1);
		TVD_LOG(TVD_LOG_LVL_OFF, "u4ColorProcessValS1 %u\n", u4ColorProcessValS1);
	}

	/* if you set VCosgain 16~31 and VSingain 0~15 */
	if ((u1Mask & VCOSGAIN_VALID_MASK) && (u1Mask & VSINGAIN_VALID_MASK)) {
		u4ColorProcessValS2 = TVD_READ32(COLOR_PROCESS_S2);
		u4ColorProcessValS2 = u4ColorProcessValS2 & 0x0U;
		u4ColorProcessValS2 = (u32)((u32)u1Vcosgain << 16) | u1Vsingain;
		TVD_WRITE32(COLOR_PROCESS_S2, u4ColorProcessValS2);
		TVD_LOG(TVD_LOG_LVL_OFF, "u4ColorProcessValS2 %u\n", u4ColorProcessValS2);
	}

	/* if you set u4Yoffset 16~23 and u4Uoffset 8~15 u4Voffset 0~7bit */
	if (u1Mask & (YOFFSET_VALID_MASK | UOFFSET_VALID_MASK | VOFFSET_VALID_MASK)) {
		u4ColorProcessValS3 = TVD_READ32(COLOR_PROCESS_S3);
		u4ColorProcessValS3 = u4ColorProcessValS3 & 0xFF000000U;
		u4ColorProcessValS3 |= (((u32)u1Yoffset << 16) &  0x00FF0000U);
		u4ColorProcessValS3 |= (((u32)u1Uoffset << 8) & 0x0000FF00U);
		u4ColorProcessValS3 |= (u1Voffset & 0x000000FFU);
		TVD_WRITE32(COLOR_PROCESS_S3, u4ColorProcessValS3);
		TVD_LOG(TVD_LOG_LVL_OFF, "u4ColorProcessValS3 %u\n", u4ColorProcessValS3);
	}

#endif
}



/**************************************************************************

* @brief TVD config.

* @param

* @return None

**************************************************************************/

void Tvd_Core_Config(TVD_CORE_PREVIEW_CFG_T *prPreviewCfg)
{
#if SUPPORT_CALIBRATE_BRIGHTNESS

	TVD_LOG(TVD_LOG_LVL_INFO, "u1Mask %x\n", prPreviewCfg->u1Mask);
	TVD_LOG(TVD_LOG_LVL_INFO, "u1YGain %x\n", prPreviewCfg->u1YGain);
	TVD_LOG(TVD_LOG_LVL_INFO, "u1YOffset %x\n", prPreviewCfg->u1YOffset);
#endif
#if (TVD_DRV_FPGA_BOARD)
	/*   vdo scan line */
	/*   HAL_WRITE32((IO_BASE_VA + 0x421E0), 0x008000B4); */
#endif

	if (NULL == prPreviewCfg) {
		TVD_LOG(TVD_LOG_LVL_WARN, "Tvd_Core_Config hasn't changed\n");
		return;
	}

#if SUPPORT_CALIBRATE_BRIGHTNESS
	Tvd_Color_Process(prPreviewCfg);
#endif
}

void TVD_Enable_ManualMode(void)
{
	TVD_SET_BIT(REG_CDET_00, TVD_MMODE);

	TVD_CLR_BIT(REG_CDET_00, SECAM_EN);
	TVD_CLR_BIT(REG_CDET_00, PAL60_EN);
	TVD_CLR_BIT(REG_CDET_00, PALM_EN);
	TVD_CLR_BIT(REG_CDET_00, PALN_EN);
	TVD_CLR_BIT(REG_CDET_00, NTSC443_EN);
}

void TVD_Disable_ManualMode(void)
{
	TVD_LOG(TVD_LOG_LVL_WARN, "Disable ManualMode!\n");
	TVD_CLR_BIT(REG_CDET_00, TVD_MMODE);
	/* TVD_CLR_BIT(REG_CDET_00, TVD_MODE()) */

	TVD_SET_BIT(REG_CDET_00, SECAM_EN);
	TVD_SET_BIT(REG_CDET_00, PAL60_EN);
	TVD_SET_BIT(REG_CDET_00, PALM_EN);
	TVD_SET_BIT(REG_CDET_00, PALN_EN);
	TVD_SET_BIT(REG_CDET_00, NTSC443_EN);
}

void _TVD_Set_ManualSigMode(u32 u4SigMode)
{
	TVD_LOG(TVD_LOG_LVL_WARN, "_TVD Set Manual signal mode as %u\n", u4SigMode);
	TVD_WRITE32_MASK(REG_CDET_00, u4SigMode, 0X7);
}


void Tvd_Snow_Mode(void)
{
	u32 u4SnowModeVal;

	u4SnowModeVal = TVD_READ32(SNOW_MODE);
	u4SnowModeVal = u4SnowModeVal & 0xFFFFFCFFU;
	u4SnowModeVal = u4SnowModeVal | (SNOW_MODE_ON);
	TVD_WRITE32(SNOW_MODE, u4SnowModeVal);
}

void Tvd_Snow_Mode_Alaways(void)
{
	u32 u4SnowModeVal;

	u4SnowModeVal = TVD_READ32(SNOW_MODE);
	u4SnowModeVal = u4SnowModeVal & 0xFFFFFCFFU;
	u4SnowModeVal = u4SnowModeVal | (SNOW_MODE_ALAWAYS_ON);
	TVD_WRITE32(SNOW_MODE, u4SnowModeVal);
}

void Tvd_Line_Average(bool fgOn)
{
	u32 u4LineAverageVal;

	if (fgOn) {
		u4LineAverageVal = TVD_READ32(LINE_AVERAGE);
		u4LineAverageVal = u4LineAverageVal & (0xEFFFFFFFU);
		u4LineAverageVal = u4LineAverageVal | (LINE_AVERAGE_ON);
		TVD_WRITE32(LINE_AVERAGE, u4LineAverageVal);
	} else {
		u4LineAverageVal = TVD_READ32(LINE_AVERAGE);
		u4LineAverageVal = u4LineAverageVal & (0xEFFFFFFFU);
		u4LineAverageVal = u4LineAverageVal | (LINE_AVERAGE_OFF);
		TVD_WRITE32(LINE_AVERAGE, u4LineAverageVal);
	}
}



void Tvd_Ana_IO_PowerOn(bool fgOnOff)
{
	u32 u4Tmp;

	TVD_LOG(TVD_LOG_LVL_DBG, "Tvd_Ana_IO_PowerOn is %d\n", fgOnOff);

	if (fgOnOff) {
		if (gfgAnaIOPwrOn) {
			return;
		}

		u4Tmp = (u32)(RG_CVBS_PWD | RG_PROT_PWD | RG_INMUX_PWD | RG_CLAMP_PWD);
		TVD_WRITE32_MASK(REG_VFE_00, (~u4Tmp), u4Tmp);
		TVD_CLR_BIT(REG_VFE_01, RG_CVBSADC_PWD);
		/*  AUADC GLB bias PWD */
		HAL_WRITE32(TVD_BASE + 0x340, (HAL_READ32(TVD_BASE + 0x340) & (~(1 << 14))));

		gfgAnaIOPwrOn = true;
	} else {
		if (!gfgAnaIOPwrOn) {
			return;
		}

		u4Tmp = (RG_CVBS_PWD | RG_PROT_PWD | RG_INMUX_PWD | RG_CLAMP_PWD);
		TVD_WRITE32_MASK(REG_VFE_00, (u4Tmp), u4Tmp);
		TVD_SET_BIT(REG_VFE_01, RG_CVBSADC_PWD);
		/*  AUADC GLB bias PWD */
		HAL_WRITE32(TVD_BASE + 0x340, (HAL_READ32(TVD_BASE + 0x340) | ((1 << 14))));

		gfgAnaIOPwrOn = false;
	}
}

/**************************************************************************
* @brief  CVBS (TVD Analog) initialize Setting.
*     The function for CVBS initialize, only realchip valid.
* @param
* @return None
**************************************************************************/
void vChannelA_Init(TVD_CHANNEL_ID channel_id)
{
	/* A channel enable */
	TVD_SET_BIT(tvd_base[channel_id] + REG_VFE_00, RG_UPDN); /*  cvbs enable clamp on  blank for CHA */
	TVD_SET_BIT(tvd_base[channel_id] + REG_VFE_00, RG_VAGSELA); /* cvbs channel A    PGA CM buffer input  0.5v */
	TVD_CLR_BIT(tvd_base[channel_id] + REG_VFE_00, RG_PGABUFNA_PWD); /* cvbs  channel A  input  BUFFER  power on */
	TVD_CLR_BIT(tvd_base[channel_id] + REG_VFE_00, RG_SHIFTA_PWD); /*  cvbs channel A shift power on */
	TVD_CLR_BIT(tvd_base[channel_id] + REG_VFE_00, RG_OFFCURA_PWD); /*  cvbs channel A offset current power on */
}

void vChannelB_Init(TVD_CHANNEL_ID channel_id)
{
	/*  B Channel enable */
	TVD_SET_BIT(tvd_base[channel_id] + REG_VFE_03, RG_BTM_EN); /*  cvbs enable clamp on bottom */
	TVD_SET_BIT(tvd_base[channel_id] + REG_VFE_03, RG_VAGSELB); /* cvbs channel B PGA CM buffer input 0.5v */
	TVD_CLR_BIT(tvd_base[channel_id] + REG_VFE_03, RG_PGABUFNB_PWD); /* cvbs channel input  BUFFER  power on */
	TVD_CLR_BIT(tvd_base[channel_id] + REG_VFE_03, RG_SHIFTB_PWD); /* cvbs channel B shift power on */
	TVD_CLR_BIT(tvd_base[channel_id] + REG_VFE_03, RG_OFFCURB_PWD); /*cvbs channel B offset current power on */

	TVD_WRITE32_MASK(tvd_base[channel_id] + REG_VFE_03, (0x04 << 16), 0x3F << 16); /* the setting is same with mt3360 */
}

/**************************************************************************
* @brief  CVBS (TVD Analog) initialize Setting.
*     The function for CVBS initialize, only realchip valid.
* @param
* @return None
**************************************************************************/
void vCVBS_Init(TVD_CHANNEL_ID channel_id)
{
	switch(channel_id) {
        case TVD_CH_0:
            //channel A   (choose 0P)
            TVD_ANA_WRITE32(REG_CVBS_CFG0,TVD_ANA_READ32(REG_CVBS_CFG0)&(~(RG_CVBS_PWD_CH0))); //cvbs power on
            TVD_ANA_WRITE32(REG_CVBS_CFG2,TVD_ANA_READ32(REG_CVBS_CFG2)&(~(RG_CVBSADC_PWD_CH0))); //cvbs adc power on
            TVD_ANA_WRITE32(REG_CVBS_CFG0,TVD_ANA_READ32(REG_CVBS_CFG0)&(~(RG_CLAMP_PWD_CH0)));//input clamp power on

            TVD_ANA_WRITE32(REG_CVBS_CFG0,TVD_ANA_READ32(REG_CVBS_CFG0)|(RG_UPDN_CH0));// cvbs enable clamp on blank for cha
            TVD_ANA_WRITE32(REG_CVBS_CFG0,TVD_ANA_READ32(REG_CVBS_CFG0)|(RG_VAGSELA_CH0));// cvbs channel A PGA cm buffer input 0.5V
            TVD_ANA_WRITE32(REG_CVBS_CFG0,TVD_ANA_READ32(REG_CVBS_CFG0)&(~(RG_PGABUFNA_PWD_CH0)));// cvbs channel A  input buffer power on
            TVD_ANA_WRITE32(REG_CVBS_CFG0,TVD_ANA_READ32(REG_CVBS_CFG0)&(~(RG_SHIFTA_PWD_CH0)));// cvbs channel A  shift power on
            TVD_ANA_WRITE32(REG_CVBS_CFG0,TVD_ANA_READ32(REG_CVBS_CFG0)&(~(RG_OFFCUROA_PWD_CH0)));// cvbs channel A  offset current power on

			#if 0
            //1p
            TVD_WRITE32(REG_CVBS_CFG2,TVD_READ32(REG_CVBS_CFG2)|((RG_CVBS0P_CHA_SEL_CH0)));// cvbs0p as channel A input
            TVD_WRITE32(REG_CVBS_CFG0,(TVD_READ32(REG_CVBS_CFG0)&(~(RG_AISEL_CH0))));// cvbs1p as channel A input
			#endif
			
            #if 0
            //2P        
            TVD_WRITE32(REG_CVBS_CFG2,TVD_READ32(REG_CVBS_CFG2)&(~(RG_CVBS0P_CHA_SEL_CH0)));// cvbs1p as channel A input
            TVD_WRITE32(REG_CVBS_CFG0,(TVD_READ32(REG_CVBS_CFG0)&(~(RG_AISEL_CH0)))|(0x4 << 20));// cvbs1p as channel A input
			#endif

			#if 0
            //0p
            TVD_ANA_WRITE32(REG_CVBS_CFG2,TVD_ANA_READ32(REG_CVBS_CFG2)&(~(RG_CVBS0P_CHA_SEL_CH0)));// cvbs2p as channel A input
            TVD_ANA_WRITE32(REG_CVBS_CFG0,(TVD_ANA_READ32(REG_CVBS_CFG0)&(~(RG_AISEL_CH0)))|(0x8 << 20));// cvbs2p as channel A input
            #endif

            TVD_ANA_WRITE32(0x6f0,(TVD_ANA_READ32(0x6f0) & (~(0x1 << 17))));
            TVD_ANA_WRITE32(REG_CVBS_CFG0,(TVD_ANA_READ32(REG_CVBS_CFG0) & (~(RG_INMUX_PWD_CH0)))); 
            TVD_ANA_WRITE32(REG_CVBS_CFG3,(TVD_ANA_READ32(REG_CVBS_CFG3) & (~(RG_CVBSADC_SEL_CKPLL_CH0)))); 

            #if 0
            //channel B   (choose 1P)
            TVD_WRITE32(REG_CVBS_CFG3,TVD_READ32(REG_CVBS_CFG3)|(RG_BTM_EN_CH0));// cvbs enable clamp onbottom for chb
            TVD_WRITE32(REG_CVBS_CFG3,TVD_READ32(REG_CVBS_CFG3)|(RG_VAGSELB_CH0));// cvbs channel b PGA cm buffer input 0.5V
            TVD_WRITE32(REG_CVBS_CFG3,TVD_READ32(REG_CVBS_CFG3)&(~(RG_PGABUFNB_PWD_CH0)));// cvbs channel b  input buffer power on
            TVD_WRITE32(REG_CVBS_CFG3,TVD_READ32(REG_CVBS_CFG3)&(~(RG_SHIFTB_PWD_CH0)));// cvbs channel b shift power on
            TVD_WRITE32(REG_CVBS_CFG3,TVD_READ32(REG_CVBS_CFG3)&(~(RG_OFFCUROB_PWD_CH0)));// cvbs channel b  offset current power on
            TVD_WRITE32(REG_CVBS_CFG0,(TVD_READ32(REG_CVBS_CFG0)&(~(RG_VIDEOBYPASS_CH0)))|(0x8<<24));// cvbs1p as channel b input
           // TVD_HAL_WRITE32(IO_BASE+REG_CVBS_CFG2,(TVD_HAL_READ32(IO_BASE+REG_CVBS_CFG2)&(~(RG_VIDEOBYPASS_CH0)))|(0x10<<24));// cvbs2p as channel b input
            #endif
            break;
        
        case TVD_CH_1:

            //channel A (choose 0P)
            TVD_ANA_WRITE32(REG_CVBS_CFG5,TVD_ANA_READ32(REG_CVBS_CFG5)&(~(RG_CVBS_PWD_CH1))); //cvbs power on
            TVD_ANA_WRITE32(REG_CVBS_CFG7,TVD_ANA_READ32(REG_CVBS_CFG7)&(~(RG_CVBSADC_PWD_CH1))); //cvbs adc power on
            TVD_ANA_WRITE32(REG_CVBS_CFG4,TVD_ANA_READ32(REG_CVBS_CFG4)&(~(RG_CLAMP_PWD_CH1)));//input clamp power on

            TVD_ANA_WRITE32(REG_CVBS_CFG4,TVD_ANA_READ32(REG_CVBS_CFG4)|(RG_UPDN_CH1));// cvbs enable clamp on blank for cha
            TVD_ANA_WRITE32(REG_CVBS_CFG4,TVD_ANA_READ32(REG_CVBS_CFG4)|(RG_VAGSELA_CH1));// cvbs channel A PGA cm buffer input 0.5V
            TVD_ANA_WRITE32(REG_CVBS_CFG5,TVD_ANA_READ32(REG_CVBS_CFG5)&(~(RG_PGABUFNA_PWD_CH1)));// cvbs channel A  input buffer power on
            TVD_ANA_WRITE32(REG_CVBS_CFG4,TVD_ANA_READ32(REG_CVBS_CFG4)&(~(RG_SHIFTA_PWD_CH1)));// cvbs channel A  shift power on
            TVD_ANA_WRITE32(REG_CVBS_CFG5,TVD_ANA_READ32(REG_CVBS_CFG5)&(~(RG_OFFCUROA_PWD_CH1)));// cvbs channel A  offset current power on
            TVD_ANA_WRITE32(REG_CVBS_CFG6,TVD_ANA_READ32(REG_CVBS_CFG6) | (RG_CVBS0P_CHA_SEL_CH1));// cvbs0p as channel A input
            TVD_ANA_WRITE32(REG_CVBS_CFG4,(TVD_ANA_READ32(REG_CVBS_CFG4)&(~RG_AISEL_CH1)) | (0x3 << 14));// cvbs0p as channel A input



            TVD_ANA_WRITE32(0x6f0,(TVD_ANA_READ32(0x6f0) & (~(0x1 << 17))));
            TVD_ANA_WRITE32(REG_CVBS_CFG5,(TVD_ANA_READ32(REG_CVBS_CFG5) & (~(RG_INMUX_PWD_CH1)))); 
            TVD_ANA_WRITE32(REG_CVBS_CFG7,(TVD_ANA_READ32(REG_CVBS_CFG7) & (~(RG_CVBSADC_SEL_CKPLL_CH1)))); 

			#if 0
            //channel B   (NO signal choose )
            TVD_ANA_WRITE32(REG_CVBS_CFG8,TVD_ANA_READ32(REG_CVBS_CFG8)|(RG_BTM_EN_CH1));// cvbs enable clamp on bottom for chb
            TVD_ANA_WRITE32(REG_CVBS_CFG8,TVD_ANA_READ32(REG_CVBS_CFG8)|(RG_VAGSELB_CH1));// cvbs channel b PGA cm buffer input 0.5V
            TVD_ANA_WRITE32(REG_CVBS_CFG7,TVD_ANA_READ32(REG_CVBS_CFG7)&(~(RG_PGABUFNB_PWD_CH1)));// cvbs channel b  input buffer power on
            TVD_ANA_WRITE32(REG_CVBS_CFG7,TVD_ANA_READ32(REG_CVBS_CFG7)&(~(RG_SHIFTB_PWD_CH1)));// cvbs channel b shift power on
            TVD_ANA_WRITE32(REG_CVBS_CFG7,TVD_ANA_READ32(REG_CVBS_CFG7)&(~(RG_OFFCUROB_PWD_CH1)));// cvbs channel b  offset current power on

            TVD_ANA_WRITE32(REG_CVBS_CFG4,(TVD_ANA_READ32(REG_CVBS_CFG4)&(~(RG_VIDEOBYPASS_CH1))));// NO signal  as channel b input

            //TVD_HAL_WRITE32(IO_BASE+REG_CVBS_CFG4,(TVD_HAL_READ32(IO_BASE+REG_CVBS_CFG4)&(~(RG_VIDEOBYPASS_CH1)))|(0x2<<18));// cvbs1p as channel b input
			#endif
            break;

        case TVD_CH_2:

            //channel A (choose 0P)
            TVD_ANA_WRITE32(REG_CVBS_CFG9,TVD_ANA_READ32(REG_CVBS_CFG9)&(~(RG_CVBS_PWD_CH2))); //cvbs power on
            TVD_ANA_WRITE32(REG_CVBS_CFG11,TVD_ANA_READ32(REG_CVBS_CFG11)&(~(RG_CVBSADC_PWD_CH2))); //cvbs adc power on
            TVD_ANA_WRITE32(REG_CVBS_CFG9,TVD_ANA_READ32(REG_CVBS_CFG9)&(~(RG_CLAMP_PWD_CH2)));//input clamp power on

            TVD_ANA_WRITE32(REG_CVBS_CFG8,TVD_ANA_READ32(REG_CVBS_CFG8)|(RG_UPDN_CH2));// cvbs enable clamp on blank for cha
            TVD_ANA_WRITE32(REG_CVBS_CFG9,TVD_ANA_READ32(REG_CVBS_CFG9)|(RG_VAGSELA_CH2));// cvbs channel A PGA cm buffer input 0.5V
            TVD_ANA_WRITE32(REG_CVBS_CFG9,TVD_ANA_READ32(REG_CVBS_CFG9)&(~(RG_PGABUFNA_PWD_CH2)));// cvbs channel A  input buffer power on
            TVD_ANA_WRITE32(REG_CVBS_CFG12,TVD_ANA_READ32(REG_CVBS_CFG12)&(~(RG_SHIFTA_PWD_CH2)));// cvbs channel A  shift power on
            TVD_ANA_WRITE32(REG_CVBS_CFG9,TVD_ANA_READ32(REG_CVBS_CFG9)&(~(RG_OFFCUROA_PWD_CH2)));// cvbs channel A  offset current power on

            TVD_ANA_WRITE32(REG_CVBS_CFG11,TVD_ANA_READ32(REG_CVBS_CFG11)|(RG_CVBS0P_CHA_SEL_CH2));// cvbs0p as channel A input
            TVD_ANA_WRITE32(REG_CVBS_CFG8,(TVD_ANA_READ32(REG_CVBS_CFG8)&(~(RG_AISEL_CH2))));// cvbs1p as channel b input


            TVD_ANA_WRITE32(0x6f0,(TVD_ANA_READ32(0x6f0) & (~(0x1 << 17))));
            TVD_ANA_WRITE32(REG_CVBS_CFG9,(TVD_ANA_READ32(REG_CVBS_CFG9) & (~(RG_INMUX_PWD_CH2)))); 
            TVD_ANA_WRITE32(REG_CVBS_CFG11,(TVD_ANA_READ32(REG_CVBS_CFG11) & (~(RG_CVBSADC_SEL_CKPLL_CH2)))); 


          
            //channel B   (NO signal choose )
            TVD_ANA_WRITE32(REG_CVBS_CFG12,TVD_ANA_READ32(REG_CVBS_CFG12)|(RG_BTM_EN_CH2));// cvbs enable clamp onbottom for chb
            TVD_ANA_WRITE32(REG_CVBS_CFG12,TVD_ANA_READ32(REG_CVBS_CFG12)|(RG_VAGSELB_CH2));// cvbs channel b PGA cm buffer input 0.5V
            TVD_ANA_WRITE32(REG_CVBS_CFG12,TVD_ANA_READ32(REG_CVBS_CFG12)&(~(RG_PGABUFNB_PWD_CH2)));// cvbs channel b  input buffer power on
            TVD_ANA_WRITE32(REG_CVBS_CFG12,TVD_ANA_READ32(REG_CVBS_CFG12)&(~(RG_SHIFTB_PWD_CH2)));// cvbs channel b shift power on
            TVD_ANA_WRITE32(REG_CVBS_CFG12,TVD_ANA_READ32(REG_CVBS_CFG12)&(~(RG_OFFCUROB_PWD_CH2)));// cvbs channel b  offset current power on

            TVD_ANA_WRITE32(REG_CVBS_CFG8,(TVD_ANA_READ32(REG_CVBS_CFG8)&(~(RG_VIDEOBYPASS_CH2)))|(0x00<<10));//NO signal as channel b input

            break;

        case TVD_CH_3:
                
            //channel A (choose 0P)
            TVD_ANA_WRITE32(REG_CVBS_CFG13,TVD_ANA_READ32(REG_CVBS_CFG13)&(~(RG_CVBS_PWD_CH3))); //cvbs power on
            TVD_ANA_WRITE32(REG_CVBS_CFG15,TVD_ANA_READ32(REG_CVBS_CFG15)&(~(RG_CVBSADC_PWD_CH3))); //cvbs adc power on
            TVD_ANA_WRITE32(REG_CVBS_CFG13,TVD_ANA_READ32(REG_CVBS_CFG13)&(~(RG_CLAMP_PWD_CH3)));//input clamp power on
            
            TVD_ANA_WRITE32(REG_CVBS_CFG13,TVD_ANA_READ32(REG_CVBS_CFG13)|(RG_UPDN_CH3));// cvbs enable clamp on blank for cha
            TVD_ANA_WRITE32(REG_CVBS_CFG13,TVD_ANA_READ32(REG_CVBS_CFG13)|(RG_VAGSELA_CH3));// cvbs channel A PGA cm buffer input 0.5V
            TVD_ANA_WRITE32(REG_CVBS_CFG13,TVD_ANA_READ32(REG_CVBS_CFG13)&(~(RG_PGABUFNA_PWD_CH3)));// cvbs channel A  input buffer power on
            TVD_ANA_WRITE32(REG_CVBS_CFG13,TVD_ANA_READ32(REG_CVBS_CFG13)&(~(RG_SHIFTA_PWD_CH3)));// cvbs channel A  shift power on
            TVD_ANA_WRITE32(REG_CVBS_CFG13,TVD_ANA_READ32(REG_CVBS_CFG13)&(~(RG_OFFCUROA_PWD_CH3)));// cvbs channel A  offset current power on

            TVD_ANA_WRITE32(REG_CVBS_CFG15,TVD_ANA_READ32(REG_CVBS_CFG15)|(RG_CVBS0P_CHA_SEL_CH3));// cvbs0p as channel A input
            TVD_ANA_WRITE32(REG_CVBS_CFG13,(TVD_ANA_READ32(REG_CVBS_CFG13)&(~(RG_AISEL_CH3))));// cvbs1p as channel b input


            TVD_ANA_WRITE32(0x6f0,(TVD_ANA_READ32(0x6f0) & (~(0x1 << 17))));
            TVD_ANA_WRITE32(REG_CVBS_CFG13,(TVD_ANA_READ32(REG_CVBS_CFG13) & (~(RG_INMUX_PWD_CH3)))); 
            TVD_ANA_WRITE32(REG_CVBS_CFG15,(TVD_ANA_READ32(REG_CVBS_CFG15) & (~(RG_CVBSADC_SEL_CKPLL_CH3)))); 



            //channel B   (NO signal choose )
            TVD_ANA_WRITE32(REG_CVBS_CFG16,TVD_ANA_READ32(REG_CVBS_CFG16)|(RG_BTM_EN_CH3));// cvbs enable clamp onbottom for chb
            TVD_ANA_WRITE32(REG_CVBS_CFG16,TVD_ANA_READ32(REG_CVBS_CFG16)|(RG_VAGSELB_CH3));// cvbs channel b PGA cm buffer input 0.5V
            TVD_ANA_WRITE32(REG_CVBS_CFG16,TVD_ANA_READ32(REG_CVBS_CFG16)&(~(RG_PGABUFNB_PWD_CH3)));// cvbs channel b  input buffer power on
            TVD_ANA_WRITE32(REG_CVBS_CFG16,TVD_ANA_READ32(REG_CVBS_CFG16)&(~(RG_SHIFTB_PWD_CH3)));// cvbs channel b shift power on
            TVD_ANA_WRITE32(REG_CVBS_CFG16,TVD_ANA_READ32(REG_CVBS_CFG16)&(~(RG_OFFCUROB_PWD_CH3)));// cvbs channel b  offset current power on
            TVD_ANA_WRITE32(REG_CVBS_CFG12,(TVD_ANA_READ32(REG_CVBS_CFG12)&(~(RG_VIDEOBYPASS_CH3)))|(0x0<<2));//NO signal as channel b input

            break;

        default:

            TVD_LOG(TVD_LOG_LVL_INFO, "Tvd Channel Number Err\n");
            break;

    }
}

void Tvd_CHA_PowerOn(TVD_CHANNEL_ID channel_id, bool fgOnOff)
{
	u32 u4Tmp;

	TVD_LOG(TVD_LOG_LVL_DBG, "Tvd_CHA_PowerOn is %d\n", fgOnOff);

	if (fgOnOff) {
		if (gfgCHAPwrOn) {
			return;
		}

		u4Tmp = (RG_PGABUFNA_PWD | RG_OFFCURA_PWD | RG_SHIFTA_PWD);
		//quzhi add
		//TVD_WRITE32_MASK(tvd_base[channel_id] + REG_VFE_00, (~u4Tmp), u4Tmp);

		gfgCHAPwrOn = true;
	} else {
		if (!gfgCHAPwrOn) {
			return;
		}

		u4Tmp = (RG_PGABUFNA_PWD | RG_OFFCURA_PWD | RG_SHIFTA_PWD);
		//quzhi add
		//TVD_WRITE32_MASK(tvd_base[channel_id] + REG_VFE_00, (u4Tmp), u4Tmp);

		gfgCHAPwrOn = false;
	}
}

void Tvd_CHB_PowerOn(TVD_CHANNEL_ID channel_id, bool fgOnOff)
{
	TVD_LOG(TVD_LOG_LVL_DBG, "Tvd_CHB_PowerOn is %d\n", fgOnOff);

	if (fgOnOff) {
		if (gfgCHBPwrOn) {
			return;
		}

		HAL_WRITE32(TVD_BASE + 0x280, (HAL_READ32(TVD_BASE + 0x280) & (~(0x7 << 21)))); /* CHB power up */
		/*  CHB VCM select 0.5V */
		HAL_WRITE32(TVD_BASE + 0x280, (HAL_READ32(TVD_BASE + 0x280) | (1 << 30)));
		/*  CHB PGA gain control */
		HAL_WRITE32(TVD_BASE + 0x260, (HAL_READ32(TVD_BASE + 0x260) | (((~0x3F << 24)) & (0x04 << 24))));

		gfgCHBPwrOn = true;
	} else {
		if (!gfgCHBPwrOn) {
			return;
		}

		HAL_WRITE32(TVD_BASE + 0x280, (HAL_READ32(TVD_BASE + 0x280) | ((0x7 << 21)))); /* CHB power down */

		gfgCHBPwrOn = false;
	}
}

void CVBS_ByPass_Sel(TVD_CHANNEL_ID channel_id, u32 u4Channel)
{
	if (u4Channel == TVD_CHA_BYPASS) {
		TVD_SET_BIT(tvd_base[channel_id] + REG_SYS_00_RST_CTRL, (u32)((u32)1 << 17)); /*  select CHA by pass to rear */

		TVD_LOG(TVD_LOG_LVL_HAL, "CHA by pass to rear.\r\n");
	} else if (u4Channel == TVD_CHB_BYPASS) {
		TVD_CLR_BIT(tvd_base[channel_id] + REG_SYS_00_RST_CTRL, (u32)((u32)1 << 17)); /* select CHB by pass to rear */

		TVD_LOG(TVD_LOG_LVL_HAL, "CHB by pass to rear.\r\n");
	} else {
		TVD_LOG(TVD_LOG_LVL_HAL, "CH NONE by pass to rear.\r\n");
	}
}

bool CVBS_By_Pass(TVD_CHANNEL_ID channel_id, u32 u4CH, u32 u4CVBSInP, u32 u4BypassChannel, u32 u4CfgType)
{
	u32 u4CHA_CVBSxP, u4CHB_CVBSxP;

	TVD_LOG(TVD_LOG_LVL_DBG, "CVBS_By_Pass!\n");
	/* Tvd_Ana_IO_PowerOn(true); */

	if (TVD_ANALOG_CFG_CLAMP == u4CfgType) {
		return true;
	}

	if (u4CH == TVD_CHA) {
		vChannelA_Init(channel_id);

		switch (u4CVBSInP) {
		case 1:
			TVD_LOG(TVD_LOG_LVL_HAL, "select cvbs1p to CHA !\r\n");
			TVD_SET_BIT(tvd_base[channel_id] + REG_VFE_02, RG_CVBS_REV_2); /*  select CVBS0P */
			TVD_WRITE32_MASK(tvd_base[channel_id] + REG_VFE_00, (0 << 8), RG_AISEL);
			break;

		case 2:
			TVD_LOG(TVD_LOG_LVL_HAL, "select cvbs2p to CHA !\r\n");
			TVD_CLR_BIT(tvd_base[channel_id] + REG_VFE_02, RG_CVBS_REV_2); /*  no select CVBS0P to CHA */
			TVD_WRITE32_MASK(tvd_base[channel_id] + REG_VFE_00, (1 << 8), RG_AISEL);     /* select CVBS1P to CHA */
			break;

		case 3:
			TVD_LOG(TVD_LOG_LVL_HAL, "select cvbs3p to CHA !\r\n");
			TVD_CLR_BIT(tvd_base[channel_id] + REG_VFE_02, RG_CVBS_REV_2); /* no select CVBS0P to CHA */
			TVD_WRITE32_MASK(tvd_base[channel_id] + REG_VFE_00, (2 << 8), RG_AISEL);     /* select CVBS2P to CHA */
			break;

		case 4:
			TVD_LOG(TVD_LOG_LVL_HAL, "select cvbs4p to CHA !\r\n");
			TVD_CLR_BIT(tvd_base[channel_id] + REG_VFE_02, RG_CVBS_REV_2); /* no select CVBS0P to CHA */
			TVD_WRITE32_MASK(tvd_base[channel_id] + REG_VFE_00, (4 << 8), RG_AISEL);     /* select CVBS3P to CHA */
			break;

		case 5:
			TVD_LOG(TVD_LOG_LVL_HAL, "select cvbs5p to CHA !\r\n");
			TVD_CLR_BIT(tvd_base[channel_id] + REG_VFE_02, RG_CVBS_REV_2); /*  no select CVBS0P to CHA */
			TVD_WRITE32_MASK(tvd_base[channel_id] + REG_VFE_00, (8 << 8), RG_AISEL);     /*  select CVBS4P to CHA */
			break;

		default:
			TVD_LOG(TVD_LOG_LVL_WARN, "CHA CVBS IN xP is NONE\r\n");
			TVD_LOG(TVD_LOG_LVL_WARN, "CHB CVBS IN xP is NONE\r\n");
			TVD_CLR_BIT(tvd_base[channel_id] + REG_VFE_02, RG_CVBS_REV_2);  /* no select CVBS0P to CHA */
			TVD_WRITE32_MASK(tvd_base[channel_id] + REG_VFE_00, 0, RG_AISEL);      /*  select CVBSNONE to CHA */

			/*TVD_SET_BIT(REG_SYS_00_RST_CTRL, 0x4);*/
			/*TVD_CLR_BIT(REG_SYS_00_RST_CTRL, 0x4);*/

			break;
		}

		TVD_LOG(TVD_LOG_LVL_HAL, "Select CHA and port num is %u\n", u4CVBSInP);
	} else {
		switch (u4CVBSInP) {
			vChannelB_Init(channel_id);

		case 0:
			TVD_WRITE32_MASK(tvd_base[channel_id] + REG_VFE_00, (1 << 0), RG_VIDEOBYPASS);     /*  select CVBS0P to CHB */
			break;

		case 1:
			TVD_WRITE32_MASK(tvd_base[channel_id] + REG_VFE_00, (2 << 0), RG_VIDEOBYPASS);     /*  select CVBS1P to CHB */
			break;

		case 2:
			TVD_WRITE32_MASK(tvd_base[channel_id] + REG_VFE_00, (4 << 0), RG_VIDEOBYPASS);     /*  select CVBS2P to CHB */
			break;

		case 3:
			TVD_WRITE32_MASK(tvd_base[channel_id] + REG_VFE_00, (0x8 << 0), RG_VIDEOBYPASS);     /*  select CVBS3P to CHB */
			break;

		case 4:
			TVD_WRITE32_MASK(tvd_base[channel_id] + REG_VFE_00, (0x10 << 0), RG_VIDEOBYPASS);     /*  select CVBS4P to CHB */
			break;

		default:
			TVD_LOG(TVD_LOG_LVL_WARN, "CHB CVBS IN xP is NONE\n");
			TVD_WRITE32_MASK(tvd_base[channel_id] + REG_VFE_00, 0, RG_VIDEOBYPASS);              /*  select CVBS0P to CHB */
			break;
		}

		TVD_LOG(TVD_LOG_LVL_HAL, "Select CHB and port num is %u\n", u4CVBSInP);
	}

	if ((TVD_READ32(tvd_base[channel_id] + REG_VFE_02) & RG_CVBS_REV_2) == RG_CVBS_REV_2) {
		u4CHA_CVBSxP = 0;
	} else {
		u4CHA_CVBSxP = (TVD_READ32(tvd_base[channel_id] + REG_VFE_00) & RG_AISEL) >> 8;

		if (0 == u4CHA_CVBSxP) {
			u4CHA_CVBSxP = CVBSIN_NONE;
		} else if (u4CHA_CVBSxP == 4) {
			u4CHA_CVBSxP = 3;
		} else if (u4CHA_CVBSxP == 8) {
			u4CHA_CVBSxP = 4;
		} else {
			/*nothing*/
		}
	}

	u4CHB_CVBSxP = (TVD_READ32(tvd_base[channel_id] + REG_VFE_00) & RG_VIDEOBYPASS);

	if (u4CHB_CVBSxP == 0) {
		u4CHB_CVBSxP = CVBSIN_NONE;
	} else if (u4CHB_CVBSxP == 1) {
		u4CHB_CVBSxP = 0;
	} else if (u4CHB_CVBSxP == 2) {
		u4CHB_CVBSxP = 1;
	} else if (u4CHB_CVBSxP == 4) {
		u4CHB_CVBSxP = 2;
	} else if (u4CHB_CVBSxP == 0x8) {
		u4CHB_CVBSxP = 3;
	} else if (u4CHB_CVBSxP == 0x10) {
		u4CHB_CVBSxP = 4;
	} else {
		/*nothing*/
	}

	if ((u4CHA_CVBSxP == u4CHB_CVBSxP) && (u4CHA_CVBSxP != CVBSIN_NONE)) {
		/*
		when there is no signal & the same input source,
		CHB BYPASS on->CHA on, CHB will be interfered
		because CLAMP on Bottom -> Blank.
		Add by MTK40136 for BUG: CNB00003119.
		 if (TVD_SIG_GET == gu4CurSignalStatus)
		*/
		TVD_CLR_BIT(tvd_base[channel_id] + REG_VFE_03,    RG_BTM_EN);            /* cvbs disable clamp on bottom */
	} else {
		TVD_SET_BIT(tvd_base[channel_id] + REG_VFE_03,    RG_BTM_EN);            /*  cvbs enable clamp on bottom */
		/* gfgNeedTurnClamp = false; */
	}

	CVBS_ByPass_Sel(channel_id, u4BypassChannel);

	return true;
}

