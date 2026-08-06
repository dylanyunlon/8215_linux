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
/*
 *Department: ATC-SD-SD2
 *
 *Aurthor: yezhiqiang(atc0064)
 *
 *Date: 2017-02-27
 */
#include "pp_reg.h"
#include "pp_log.h"
#include "pp_drv.h"
#include "pp_hal.h"

unsigned long _IO_BASE_ = 0x10000000;/*init for arm2*/
unsigned long PP_BASE;

POST_DFT_QTY aDefaultQtyTbl[] =
{
	// 1. TDSHARP

	//zxk
	    {0x40,	0xff,	0x7f,	FROM_DFT, QUALITY_TDSHARP_GAIN1},
	    {0x40,	0xff,	0x7f,	FROM_DFT, QUALITY_TDSHARP_GAIN4},
	    {0x40,	0xff,	0x7f,	FROM_DFT, QUALITY_TDSHARP_GAIN5},
	    {0x40,	0xff,	0x7f,	FROM_DFT, QUALITY_TDSHARP_GAIN6},
	    {0x40,	0xff,	0x7f,	FROM_DFT, QUALITY_TDSHARP_GAIN8},
	    {0x40,	0xff,	0x7f,	FROM_DFT, QUALITY_TDSHARP_GAIN9},
	    
	    {0x00,	0x00,	0x00,	FROM_DFT, QUALITY_TDSHARP_CORING1},
	    {0x00,	0x00,	0x00,	FROM_DFT, QUALITY_TDSHARP_CORING4},
	    {0x00,	0x00,	0x00,	FROM_DFT, QUALITY_TDSHARP_CORING5},
	    {0x00,	0x00,	0x00,	FROM_DFT, QUALITY_TDSHARP_CORING6},
	    {0x00,	0x00,	0x00,	FROM_DFT, QUALITY_TDSHARP_CORING8},
	    {0x00,	0x00,	0x00,	FROM_DFT, QUALITY_TDSHARP_CORING9},
	    
	    {0x00,	0xff,	0x7f,	FROM_DFT, QUALITY_TDSHARP_LIMIT_POS_ALL},
	    {0x00,	0xff,	0x7f,	FROM_DFT, QUALITY_TDSHARP_LIMIT_NEG_ALL},  
	    
	    {0x00,	0xff,	0x7f,	FROM_DFT, QUALITY_TDSHARP_LIMIT_POS1},
	    {0x00,	0xff,	0x7f,	FROM_DFT, QUALITY_TDSHARP_LIMIT_POS4},
	    {0x00,	0xff,	0x7f,	FROM_DFT, QUALITY_TDSHARP_LIMIT_POS5},
	    {0x00,	0xff,	0x7f,	FROM_DFT, QUALITY_TDSHARP_LIMIT_POS6},
	    {0x00,	0xff,	0x7f,	FROM_DFT, QUALITY_TDSHARP_LIMIT_POS8},
	    {0x00,	0xff,	0x7f,	FROM_DFT, QUALITY_TDSHARP_LIMIT_POS9},
	    
	    {0x00,	0xff,	0x7f,	FROM_DFT, QUALITY_TDSHARP_LIMIT_NEG1},
	    {0x00,	0xff,	0x7f,	FROM_DFT, QUALITY_TDSHARP_LIMIT_NEG4},
	    {0x00,	0xff,	0x7f,	FROM_DFT, QUALITY_TDSHARP_LIMIT_NEG5},
	    {0x00,	0xff,	0x7f,	FROM_DFT, QUALITY_TDSHARP_LIMIT_NEG6},
	    {0x00,	0xff,	0x7f,	FROM_DFT, QUALITY_TDSHARP_LIMIT_NEG8},
	    {0x00,	0xff,	0x7f,	FROM_DFT, QUALITY_TDSHARP_LIMIT_NEG9},
	    
	    {0x02,	0x02,	0x02,	FROM_DFT, QUALITY_TDSHARP_CLIP_EN1},
	    {0x02,	0x02,	0x02,	FROM_DFT, QUALITY_TDSHARP_CLIP_EN4},
	    {0x02,	0x02,	0x02,	FROM_DFT, QUALITY_TDSHARP_CLIP_EN5},
	    {0x02,	0x02,	0x02,	FROM_DFT, QUALITY_TDSHARP_CLIP_EN6},
	    {0x02,	0x02,	0x02,	FROM_DFT, QUALITY_TDSHARP_CLIP_EN8},
	    {0x02,	0x02,	0x02,	FROM_DFT, QUALITY_TDSHARP_CLIP_EN9},
	    
	    {0x00,	0xff,	0x7f,	FROM_DFT, QUALITY_TDSHARP_CLIP_THPOS1},
	    {0x00,	0xff,	0x7f,	FROM_DFT, QUALITY_TDSHARP_CLIP_THNEG1},
	    {0x00,	0xff,	0x7f,	FROM_DFT, QUALITY_TDSHARP_CLIP_THPOS4},
	    {0x00,	0xff,	0x7f,	FROM_DFT, QUALITY_TDSHARP_CLIP_THNEG4},
	    {0x00,	0xff,	0x7f,	FROM_DFT, QUALITY_TDSHARP_CLIP_THPOS5},
	    {0x00,	0xff,	0x7f,	FROM_DFT, QUALITY_TDSHARP_CLIP_THNEG5},
	    {0x00,	0xff,	0x7f,	FROM_DFT, QUALITY_TDSHARP_CLIP_THPOS6},
	    {0x00,	0xff,	0x7f,	FROM_DFT, QUALITY_TDSHARP_CLIP_THNEG6},
	    {0x00,	0xff,	0x7f,	FROM_DFT, QUALITY_TDSHARP_CLIP_THPOS8},
	    {0x00,	0xff,	0x7f,	FROM_DFT, QUALITY_TDSHARP_CLIP_THNEG8},
	    {0x00,	0xff,	0x7f,	FROM_DFT, QUALITY_TDSHARP_CLIP_THPOS9},
	    {0x00,	0xff,	0x7f,	FROM_DFT, QUALITY_TDSHARP_CLIP_THNEG9},

	
	// 3. CTI
	//zxk (CONFIG_CHIP_VER_CURR == CONFIG_CHIP_VER_MT8530)||(CONFIG_DRV_ENABLE_SIMP_POST_PROC == 1)

		{0x00,	0xff,	0x75,	FROM_DFT, QUALITY_ECTI_FLAT_SEL},
		{0x00,	0x00,	0x02,	FROM_DFT, QUALITY_ECTI_FLAT_OFST},
		{0x00,	0x07,	0x06,	FROM_DFT, QUALITY_ECTI_T_SEL},
		{0x00,	0x0f,	0x0A,	FROM_DFT, QUALITY_ECTI_ADPT_SEL},
		{0x00,	0x00,	0x01,	FROM_DFT, QUALITY_ECTI_ADPT_OFST},
		{0x00,	0x00,	0x00,	FROM_DFT, QUALITY_ECTI_LPF_SEL}, // 0 is good for corbar
		{0x10,	0x80,	0x10,	FROM_DFT, QUALITY_ECTI_STB_SEL},  // >>1
		{0x00,	0x00,	0x01,	FROM_DFT, QUALITY_ECTI_STB_OFST}, // >>2
	
    {0xFF,	0xFF,	0xFF,	0xFF, 0xFF},	//End of array
};

unsigned char _bQtyTbl[QUALITY_MAX] = //Store Quality settings
{
    #if 1//zxk (CONFIG_CHIP_VER_CURR == CONFIG_CHIP_VER_MT8530)||(CONFIG_DRV_ENABLE_SIMP_POST_PROC == 1)
    0x40, //QUALITY_TDSHARP_GAIN1 
    0x40, //QUALITY_TDSHARP_GAIN4 
    0x40, //QUALITY_TDSHARP_GAIN5 
    0x40, //QUALITY_TDSHARP_GAIN6 
    0x50, //QUALITY_TDSHARP_GAIN8 
    0x40, //QUALITY_TDSHARP_GAIN9 
    
    0x03, //QUALITY_TDSHARP_CORING1 
    0x03, //QUALITY_TDSHARP_CORING4 
    0x03, //QUALITY_TDSHARP_CORING5 
    0x03, //QUALITY_TDSHARP_CORING6 
    0x03, //QUALITY_TDSHARP_CORING8 
    0x03, //QUALITY_TDSHARP_CORING9 
    
    0xff, //QUALITY_TDSHARP_LIMIT_POS_ALL 
    0xff, //QUALITY_TDSHARP_LIMIT_NEG_ALL   
    
    0x0A, //QUALITY_TDSHARP_LIMIT_POS1 
    0x0A, //QUALITY_TDSHARP_LIMIT_POS4 
    0x0A, //QUALITY_TDSHARP_LIMIT_POS5 
    0x0A, //QUALITY_TDSHARP_LIMIT_POS6 
    0x0A, //QUALITY_TDSHARP_LIMIT_POS8 
    0x0A, //QUALITY_TDSHARP_LIMIT_POS9 
    
    0x08, //QUALITY_TDSHARP_LIMIT_NEG1 
    0x08, //QUALITY_TDSHARP_LIMIT_NEG4 
    0x08, //QUALITY_TDSHARP_LIMIT_NEG5 
    0x08, //QUALITY_TDSHARP_LIMIT_NEG6 
    0x08, //QUALITY_TDSHARP_LIMIT_NEG8 
    0x08, //QUALITY_TDSHARP_LIMIT_NEG9 
    
    0x01, //QUALITY_TDSHARP_CLIP_EN1 
    0x00, //QUALITY_TDSHARP_CLIP_EN4 
    0x01, //QUALITY_TDSHARP_CLIP_EN5 
    0x01, //QUALITY_TDSHARP_CLIP_EN6 
    0x01, //QUALITY_TDSHARP_CLIP_EN8 
    0x00, //QUALITY_TDSHARP_CLIP_EN9 
    
    0x08, //QUALITY_TDSHARP_CLIP_THPOS1 
    0x08, //QUALITY_TDSHARP_CLIP_THNEG1 
    0x00, //QUALITY_TDSHARP_CLIP_THPOS4 
    0x00, //QUALITY_TDSHARP_CLIP_THNEG4 
    0x08, //QUALITY_TDSHARP_CLIP_THPOS5 
    0x08, //QUALITY_TDSHARP_CLIP_THNEG5 
    0x10, //QUALITY_TDSHARP_CLIP_THPOS6 
    0x10, //QUALITY_TDSHARP_CLIP_THNEG6 
    0x08, //QUALITY_TDSHARP_CLIP_THPOS8 
    0x08, //QUALITY_TDSHARP_CLIP_THNEG8 
    0x00, //QUALITY_TDSHARP_CLIP_THPOS9 
    0x00, //QUALITY_TDSHARP_CLIP_THNEG9 		
#endif
	
#if 1//zxk (CONFIG_CHIP_VER_CURR == CONFIG_CHIP_VER_MT8530)||(CONFIG_DRV_ENABLE_SIMP_POST_PROC == 1)
		0x15, //QUALITY_ECTI_FLAT_SEL 
		0x02, //QUALITY_ECTI_FLAT_OFST 
		0x06, //QUALITY_ECTI_T_SEL 
		0x0A, //QUALITY_ECTI_ADPT_SEL 
		0x01, //QUALITY_ECTI_ADPT_OFST 
		0x02, //QUALITY_ECTI_LPF_SEL 
		0x10, //QUALITY_ECTI_STB_SEL   // >>1
		0x01, //QUALITY_ECTI_STB_OFST  // >>2 
#endif
};

#define vReadQualityTable(wAddr)	 _bQtyTbl[wAddr]
#define vWriteQualityTable(wAddr, wData) (_bQtyTbl[wAddr] = wData)

#define TDSHARP_ON_STATUS         0x1 << 0
#define TDSHARP_OFF_STATUS        0x0 << 0
#define CTI_ON_STATUS             0x1 << 1
#define CTI_OFF_STATUS            0x0 << 1

#define SV_ON	           (int)(1)
#define SV_OFF            (int)(0)
#define POST_OK	           (int)(0)
#define POST_FAIL            (int)(-1)

unsigned int _u1PostStatus;

void _getPPaddr(unsigned char VideoPath)
{
	if (0 == VideoPath) {
		PP_BASE = 0x1B000;
	} else if (1 == VideoPath) {
		PP_BASE = 0x1C000;
	} else {
		PP_LOG(PP_LOG_LVL_ERR, "_getPPaddr wrong VideoPath:%d \n", VideoPath);
	}
}

void _PpInit(unsigned char VideoPath, PP_DISPLAY_MODE_E eDisplayMode)
{
	unsigned int Scaler_h, Scaler_v;
	unsigned int HActive, VActive;
	unsigned int HTotal, VTotal;
	unsigned int dwTmp;
	unsigned int PP_Delay, PP_Ctrl;

	if (0 == VideoPath) {
		PP_WRITE32MSK(0xB4, 0x02000000, 0x02000000); //Bit 25
		PP_WRITE32MSK(0xD0, 0x02000000, 0x02000000);

		PP_WRITE32MSK(0x1f080, 0x1, 0x1); //enable PP&vcp

		Scaler_h = PP_HAL_READ32(0xA46A0);
		Scaler_v = PP_HAL_READ32(0xA46A4);
		HActive = (Scaler_h & 0x0000FFFF) - ((Scaler_h & 0xFFFF0000) >> 16) + 1;
		VActive = (Scaler_v & 0x0000FFFF) - ((Scaler_v & 0xFFFF0000) >> 16) + 1;

		dwTmp = PP_HAL_READ32(0xA468C); 			//[27:16]:HTotal & [10:0]:VTotal
		HTotal = (dwTmp & 0x0FFF0000) >> 16;
		VTotal = (dwTmp & 0x000007FF) >> 0;

		PP_Ctrl = (((HActive)/2 -1) << 20) | (0x3 << 16) | (((HActive)/2 -1) << 2);
		PP_WRITE32MSK(0x1f084, PP_Ctrl,0xffffffff); //pp_front_ctrl

		PP_WRITE32MSK(0x42130, Scaler_h,0xffffffff); //PP_H_s_e
		PP_WRITE32MSK(0x42134, Scaler_v,0xffffffff); //PP_V_s_e
		PP_WRITE32MSK(0x42138, Scaler_v,0xffffffff); //PP_V_s_e

		PP_Delay = (0x80040000 | (HTotal - 0x59));
		PP_WRITE32MSK(0x4213C, PP_Delay,0xffffffff);

		switch (eDisplayMode) {
		case RES_1080P:	//1080P
			PP_WRITE32MSK(0x42130, 0x00ba0839, 0xffffffff); //H/V_Sync Delay
			PP_WRITE32MSK(0x420EC, 0xC00807eb, 0xffffffff); //H/V_Sync Delay
			break;

		case RES_1024X600:	//1024x600
			PP_WRITE32MSK(0x42130, 0x00ae04ad, 0xffffffff); //H/V_Sync Delay
			PP_WRITE32MSK(0x420E8, 0x800009b7, 0xffffffff); //H/V_Sync Delay
			PP_WRITE32MSK(0x420EC, 0x800404b0, 0xffffffff); //H/V_Sync Delay
			break;

		case RES_800X480:	//800x480
			PP_WRITE32MSK(0x42130, 0x009603b5, 0xffffffff); //H/V_Sync Delay
			PP_WRITE32MSK(0x420E8, 0x80040982, 0xffffffff); //H/V_Sync Delay
			PP_WRITE32MSK(0x420EC, 0xC0020383, 0xffffffff); //H/V_Sync Delay
			break;

		case RES_800X600:	//800x600
			PP_WRITE32MSK(0x42130, 0x008203a1, 0xffffffff); //H/V_Sync Delay
			PP_WRITE32MSK(0x4213c, 0x8004039a, 0xffffffff); //H/V_Sync Delay
			PP_WRITE32MSK(0x420E8, 0x80000e38, 0xffffffff); //H/V_Sync Delay
			PP_WRITE32MSK(0x420EC, 0xC004038f, 0xffffffff); //H/V_Sync Delay
			break;

		default:
			break;
		}
	} else if(1 == VideoPath){

	} else {
		PP_LOG(PP_LOG_LVL_ERR, "[PpInit] wrong ChannelId:%d\n", VideoPath);

	}
}

void _PostFmt(unsigned char VideoPath, unsigned int PicSize)
{
	unsigned int Scaler_h, Scaler_v;
	unsigned int Fmt_h, Fmt_v;
	unsigned int HActive, VActive;
	unsigned int HTotal, VTotal;
	unsigned int dwTmp;
	unsigned int PP_Delay, PP_Ctrl;

	if(0 == VideoPath)
	{
		PP_BASE	= 0x1B000;	//front pp

		// set clock enable 
		PP_WRITE32MSK(0xB4, 0x02000000, 0x02000000);//Bit 25
		PP_WRITE32MSK(0xD0, 0x02000000, 0x02000000);

		PP_WRITE32MSK(0x1f080, 0x1,0x1); //No Bypass PP

		Scaler_h = PP_HAL_READ32(0xA46A0);
		Scaler_v = PP_HAL_READ32(0xA46A4);
		HActive = (Scaler_h & 0x0000FFFF) - ((Scaler_h & 0xFFFF0000) >> 16) + 1;
		VActive = (Scaler_v & 0x0000FFFF) - ((Scaler_v & 0xFFFF0000) >> 16) + 1;

		dwTmp = PP_HAL_READ32(0xA468C);				//[27:16]:HTotal & [10:0]:VTotal
		HTotal = (dwTmp & 0x0FFF0000) >> 16;
		VTotal = dwTmp & 0x000007FF >> 0;

		PP_LOG(PP_LOG_LVL_INFO, "HActive = 0x%x(%d), VActive = 0x%x(%d) \n", HActive,HActive, VActive,VActive);
		PP_LOG(PP_LOG_LVL_INFO, "HTotal  = 0x%x(%d), VTotal  = 0x%x(%d) \n", HTotal,HTotal, VTotal,VTotal);

		PP_Ctrl = (((HActive)/2 -1) << 20) | (0x3 << 16) | (((HActive)/2 -1) << 2);
		PP_WRITE32MSK(0x1f084, PP_Ctrl,0xffffffff); //pp_front_ctrl

		PP_WRITE32MSK(0x42130, Scaler_h,0xffffffff); //PP_H_s_e
		PP_WRITE32MSK(0x42134, Scaler_v,0xffffffff); //PP_V_s_e
		PP_WRITE32MSK(0x42138, Scaler_v,0xffffffff); //PP_V_s_e

		PP_Delay = (0x80040000 | (HTotal - 0x59));
		PP_WRITE32MSK(0x4213C, PP_Delay,0xffffffff);

		switch(PicSize)
		{
			case 1:	//1080P
				PP_WRITE32MSK(0x42130, 0x00ba0839, 0xffffffff); //H/V_Sync Delay
				PP_WRITE32MSK(0x420EC, 0xC00807eb, 0xffffffff); //H/V_Sync Delay
				break;

			case 3:	//1024x600
				PP_WRITE32MSK(0x42130, 0x00ae04ad, 0xffffffff); //H/V_Sync Delay
				PP_WRITE32MSK(0x420E8, 0x800009b7, 0xffffffff); //H/V_Sync Delay
				PP_WRITE32MSK(0x420EC, 0x800404b0, 0xffffffff); //H/V_Sync Delay
				break;

			case 4:	//800x480
				PP_WRITE32MSK(0x42130, 0x009603b5, 0xffffffff); //H/V_Sync Delay
				PP_WRITE32MSK(0x420E8, 0x80040982, 0xffffffff); //H/V_Sync Delay
				PP_WRITE32MSK(0x420EC, 0xC0020383, 0xffffffff); //H/V_Sync Delay
				break;

			case 5:	//800x600
				PP_WRITE32MSK(0x42130, 0x008203a1, 0xffffffff); //H/V_Sync Delay
				PP_WRITE32MSK(0x4213c, 0x8004039a, 0xffffffff); //H/V_Sync Delay
				PP_WRITE32MSK(0x420E8, 0x80000e38, 0xffffffff); //H/V_Sync Delay
				PP_WRITE32MSK(0x420EC, 0xC004038f, 0xffffffff); //H/V_Sync Delay
				break;

			default:
				break;
		}		
	}
	else if(1 == VideoPath)
	{
		PP_BASE = 0x1C000;  //rear pp

		PP_WRITE32MSK(0x000B4, 0x08000000, 0x08000000); //Bit 27
		PP_WRITE32MSK(0x000D0, 0x08000000, 0x08000000);

		PP_WRITE32MSK(0x1f090, 0x70300000, 0xffffffff); //No Bypass rear  PP
		PP_WRITE32MSK(0x1f080, 0x10,0x10); //No Bypass rear  PP

		Fmt_h = PP_HAL_READ32(0x430A0);
		Fmt_v = PP_HAL_READ32(0x430A4);
		PP_WRITE32MSK(0x43130, Fmt_h,0xffffffff);
		PP_WRITE32MSK(0x43134, Fmt_v,0xffffffff);
		PP_WRITE32MSK(0x43138, Fmt_v,0xffffffff); 

		switch(PicSize)
		{
			case 1:	//1080P
				HActive = 1920;
				PP_WRITE32MSK(0x43130, 0x00B20831, 0xffffffff); 
				PP_WRITE32MSK(0x4313C, 0x00030823, 0xffffffff); //0x8004084E
				PP_WRITE32MSK(0x430E8, 0x0002082F, 0xffffffff); //0x8004082E
				PP_WRITE32MSK(0x2804,  0x0A1D2400, 0xffffffff);
				PP_WRITE32MSK(0x4310C, 0x00E10680, 0xffffffff);

				PP_LOG(PP_LOG_LVL_INFO, "1080P Rear \n");
				break;

			case 2:	//720p
				HActive = 1280;
				PP_WRITE32MSK(0x4313C, 0x00030608, 0xffffffff); //0x8004061A
				PP_WRITE32MSK(0x430E8, 0x00020608, 0xffffffff); //0x80040609
				PP_WRITE32MSK(0x2804,  0x0A1D2500, 0xffffffff);
				PP_WRITE32MSK(0x4310C, 0x00E10680, 0xffffffff);
				break;

			case 6:	//480p
				PP_WRITE32MSK(0x430E8, 0x000202E8, 0xffffffff); 

				PP_WRITE32MSK(0x43130, 0x00760355, 0xffffffff); 
				PP_WRITE32MSK(0x4313C, 0x800302F6, 0xffffffff);

				PP_WRITE32MSK(0x430A4, 0x00280207, 0xffffffff);
				PP_WRITE32MSK(0x430A8, 0x00280207, 0xffffffff);
				PP_WRITE32MSK(0x43134, 0x00280207, 0xffffffff);
				PP_WRITE32MSK(0x43138, 0x00280207, 0xffffffff);
				break;

			case 7:	//480I
				PP_WRITE32MSK(0x430E8, 0x0002064A, 0xffffffff); 
				PP_WRITE32MSK(0x4313C, 0x000206AF, 0xffffffff);

				PP_WRITE32MSK(0x430A0, 0x00DF067E, 0xffffffff);
				PP_WRITE32MSK(0x43130, 0x008D062F, 0xffffffff); 
				PP_WRITE32MSK(0x43138, 0x011C020B, 0xffffffff);

				PP_WRITE32MSK(0x43094, 0x00000620, 0xffffffff);
				PP_WRITE32MSK(0x43124, 0x00000620, 0xffffffff);
				PP_WRITE32MSK(0x430A4, 0x00110100, 0xffffffff);
				PP_WRITE32MSK(0x43134, 0x001101F1, 0xffffffff);
				break;

			case 8:	//576p
				PP_WRITE32MSK(0x430E8, 0x800002FB, 0xffffffff); 
				PP_WRITE32MSK(0x4313C, 0x80020010, 0xffffffff);

				PP_WRITE32MSK(0x43130, 0x001102E0, 0xffffffff); 
				PP_WRITE32MSK(0x43134, 0x0030026f, 0xffffffff);
				PP_WRITE32MSK(0x43138, 0x0030026f, 0xffffffff);

				PP_WRITE32MSK(0x43034, 0x00310270, 0xffffffff);
				PP_WRITE32MSK(0x43038, 0x00310270, 0xffffffff);
				break;

			case 9:	//576I
				PP_WRITE32MSK(0x430E8, 0x00080666, 0xffffffff); 
				PP_WRITE32MSK(0x4313C, 0x00080672, 0xffffffff);

				PP_WRITE32MSK(0x43130, 0x00C0068F, 0xffffffff); 
				PP_WRITE32MSK(0x430A0, 0x00E6068F, 0xffffffff); 

				PP_WRITE32MSK(0x430A4, 0x00110130, 0xffffffff); 
				PP_WRITE32MSK(0x430A8, 0x014A0269, 0xffffffff); 
				PP_WRITE32MSK(0x43134, 0x00110130, 0xffffffff);
				PP_WRITE32MSK(0x43138, 0x014A0269, 0xffffffff);

				PP_WRITE32MSK(0x43124, 0xC0000420, 0xffffffff);
				PP_WRITE32MSK(0x43094, 0xC0000420, 0xffffffff);
				break;

			case 10:	//1080i
				PP_WRITE32MSK(0x430E8, 0x0002083F, 0xffffffff);
				PP_WRITE32MSK(0x4313C, 0x0003083F, 0xffffffff); 
				PP_WRITE32MSK(0x43134, 0x02480463, 0xffffffff); 
				break;

			default:
				break;
		}	

		PP_Ctrl = (((HActive)/2 -1) << 20) | (0x3 << 16) | (((HActive)/2 -1) << 2);
		PP_WRITE32MSK(0x1f088, PP_Ctrl,0xffffffff); //pp_front_ctrl

	}	
	else
	{
		PP_LOG(PP_LOG_LVL_ERR, "VideoPath Error\n");
	}
}

int u2SearchQtyItem(int u2QtyItem)
{
    int u2SearchIndex;

    // Search the item in aDefaultQtyTbl.
    for (u2SearchIndex = 0; u2SearchIndex < QUALITY_MAX; u2SearchIndex++)
    {
        // Find a match entry.
        if (aDefaultQtyTbl[u2SearchIndex].wDftQtyItem == u2QtyItem)
        {
            return u2SearchIndex;
        }

        // Search to the end of table. No found.
        if ((aDefaultQtyTbl[u2SearchIndex].bDftQtyMin == 0xFF)
            && (aDefaultQtyTbl[u2SearchIndex].bDftQtyMax == 0xFF)
            && (aDefaultQtyTbl[u2SearchIndex].bDftQtyDft == 0xFF))
        {
            return 0xFFFF;      // Can not find valid quality value.
        }
    }
    return 0xFFFF;
}

int u2QtyMapping(int u2QtyItem, int i2UIMin, int i2UIMax, 
                                     int i2UIDft, int i2UICur)
{
    // search correct item by timing & index first
    int u2HwMin = 0;
    int u2HwMax = 0;
    int u2HwDft = 0;
    int u2SearchIndex;

    u2SearchIndex = u2SearchQtyItem(u2QtyItem);
    if (u2SearchIndex == 0xFFFF){ // Not found in default quality table!!! Quality table should be updated!!!
        return 0;
    }
		
    u2HwMin = aDefaultQtyTbl[u2SearchIndex].bDftQtyMin;
    u2HwMax = aDefaultQtyTbl[u2SearchIndex].bDftQtyMax;
    u2HwDft = aDefaultQtyTbl[u2SearchIndex].bDftQtyDft;		
		//LOG(2,"Item = %d: HWMin = 0x%x, HWmax = 0x%x, HWDtf = 0x%x \n",u2SearchIndex,u2HwMin,u2HwMax,u2HwDft);
		
    // Some qty_item does not need mapping, return default HW value.
    // or UI_cur == UI_dft, return default HW value.
    //1.
    if (i2UIMin == i2UIMax)
    {
        return u2HwDft;
    }
    
    if ((u2HwMin == u2HwMax) || ( i2UICur == i2UIDft))
    {
        return u2HwDft;
    }

	//2.
    if(i2UICur > i2UIMax)
    {
        return u2HwMax;
    }

	//3.
    if(i2UICur < i2UIMin)
    {
        return u2HwMin;
    }

    //4. Some qty_item use 2-level setting as min/max/default, don't mapping.
    if (i2UIMax == 2)           // UI_max=2 means UI option is: off, low, high.
    {
        if (i2UICur == 0)
        {
            return u2HwMin;
        }
        else if (i2UICur == 1)
        {
            return u2HwDft;
        }
        else if (i2UICur == 2)
        {
            return u2HwMax;
        }
    }

    //5. Some qty_item use 3-level setting as min/max/default, don't mapping.
    if (i2UIMax == 3)           // UI_max=3 means UI option is: off, low, mid, hi.
    {
        if (i2UICur == 1)
        {
            return u2HwMin;
        }
        else if (i2UICur == 2)
        {
            return u2HwDft;
        }
        else if (i2UICur == 3)
        {
            return u2HwMax;
        }
    }

	//Maping
    //6.1 Get HW value for UI_current > UI_default.
    if (i2UICur > i2UIDft)
    {
        // Prevent UI_max, UI_dft miss setting, and div=0.
        if ((i2UIMax - i2UIDft) <= 0)
        {
            return u2HwDft;
        }
        else
        {
            return (unsigned short) ((i2UICur - i2UIDft) * (u2HwMax - u2HwDft) / 
                           (i2UIMax - i2UIDft) + u2HwDft);
        }
    }
    //6.2 Get HW value for UI_current < UI_default.
    else
    {
        // Prevent UI_dft, UI_min miss setting, and div=0.
        if ((i2UIDft - i2UIMin) <= 0)
        {
            return i2UIDft;
        }
        else
        {
            return (unsigned short) ((i2UICur - i2UIMin) * (u2HwDft - u2HwMin) /
                            (i2UIDft - i2UIMin) + u2HwMin);
        }
    }
}

void vDrvPostSharpOnOff(unsigned char bOnOff)
{
    if(bOnOff > 0)
    {
        _u1PostStatus |= TDSHARP_ON_STATUS;
    }
    else
    {
        _u1PostStatus &= ~TDSHARP_ON_STATUS;
    }
	#if 1//zxk (CONFIG_CHIP_VER_CURR == CONFIG_CHIP_VER_MT8530)||(CONFIG_DRV_ENABLE_SIMP_POST_PROC == 1)
    	PP_WRITE32MSK(TDPROC_10, (bOnOff > 0) ? TDSHARP_EN : 0, TDSHARP_EN);   
	#endif
}

void vDrvPostSharpParam(void)
{      
#if 1//zxk (CONFIG_CHIP_VER_CURR == CONFIG_CHIP_VER_MT8530)||(CONFIG_DRV_ENABLE_SIMP_POST_PROC == 1)||CONFIG_ENABLE_POSTPROC_DELAY
	//Gain Setup
	//Band 1
    PP_WRITE32MSK(TDPROC_00, 
        (vReadQualityTable(QUALITY_TDSHARP_GAIN1)<<24)|
        (vReadQualityTable(QUALITY_TDSHARP_LIMIT_POS1)<<16)|
        (vReadQualityTable(QUALITY_TDSHARP_LIMIT_NEG1)<<8)|
        (vReadQualityTable(QUALITY_TDSHARP_CORING1)),
        TDSHARP_GAIN1|TDSHARP_LIMIT_POS1|TDSHARP_LIMIT_NEG1|TDSHARP_CORING1
        );
	//Band 4
    PP_WRITE32MSK(TDPROC_06, 
        (vReadQualityTable(QUALITY_TDSHARP_GAIN4)<<24)|
        (vReadQualityTable(QUALITY_TDSHARP_LIMIT_POS4)<<16)|
        (vReadQualityTable(QUALITY_TDSHARP_LIMIT_NEG4)<<8)|
        (vReadQualityTable(QUALITY_TDSHARP_CORING4)),
        TDSHARP_GAIN4|TDSHARP_LIMIT_POS4|TDSHARP_LIMIT_NEG4|TDSHARP_CORING4
        );
	//Band 5
	PP_WRITE32MSK(TDPROC_08, 
        (vReadQualityTable(QUALITY_TDSHARP_GAIN5)<<24)|
        (vReadQualityTable(QUALITY_TDSHARP_LIMIT_POS5)<<16)|
        (vReadQualityTable(QUALITY_TDSHARP_LIMIT_NEG5)<<8)|
        (vReadQualityTable(QUALITY_TDSHARP_CORING5)),
        TDSHARP_GAIN5|TDSHARP_LIMIT_POS5|TDSHARP_LIMIT_NEG5|TDSHARP_CORING5
        );
	//Band 6
	PP_WRITE32MSK(TDPROC_0A, 
        (vReadQualityTable(QUALITY_TDSHARP_GAIN6)<<24)|
        (vReadQualityTable(QUALITY_TDSHARP_LIMIT_POS6)<<16)|
        (vReadQualityTable(QUALITY_TDSHARP_LIMIT_NEG6)<<8)|
        (vReadQualityTable(QUALITY_TDSHARP_CORING6)),
        TDSHARP_GAIN6|TDSHARP_LIMIT_POS6|TDSHARP_LIMIT_NEG6|TDSHARP_CORING6
        );
	//Band 8
	PP_WRITE32MSK(TDPROC_0E, 
        (vReadQualityTable(QUALITY_TDSHARP_GAIN8)<<24)|
        (vReadQualityTable(QUALITY_TDSHARP_LIMIT_POS8)<<16)|
        (vReadQualityTable(QUALITY_TDSHARP_LIMIT_NEG8)<<8)|
        (vReadQualityTable(QUALITY_TDSHARP_CORING8)),
        TDSHARP_GAIN8|TDSHARP_LIMIT_POS8|TDSHARP_LIMIT_NEG8|TDSHARP_CORING8
        );
	//Band 9
	PP_WRITE32MSK(TDPROC_14, 
        (vReadQualityTable(QUALITY_TDSHARP_GAIN9)<<24)|
        (vReadQualityTable(QUALITY_TDSHARP_LIMIT_POS9)<<16)|
        (vReadQualityTable(QUALITY_TDSHARP_LIMIT_NEG9)<<8)|
        (vReadQualityTable(QUALITY_TDSHARP_CORING9)),
        TDSHARP_GAIN9|TDSHARP_LIMIT_POS9|TDSHARP_LIMIT_NEG9|TDSHARP_CORING9
        );


    //Limit setup
    PP_WRITE32MSK(TDPROC_10, 
                (TDSHARP_HMASK)|
                (vReadQualityTable(QUALITY_TDSHARP_LIMIT_POS_ALL)<<16)|
                (vReadQualityTable(QUALITY_TDSHARP_LIMIT_NEG_ALL)<<8),
                TDSHARP_HMASK|TDSHARP_LIMIT_POS_ALL|TDSHARP_LIMIT_NEG_ALL);

	//Band 1
   PP_WRITE32MSK(TDPROC_01, 
               (vReadQualityTable(QUALITY_TDSHARP_CLIP_EN1)<<6)|
               (vReadQualityTable(QUALITY_TDSHARP_CLIP_THPOS1)<<24)|
               (vReadQualityTable(QUALITY_TDSHARP_CLIP_THNEG1)<<16)|
               ((0x01<<0))|   //[-1 0 2 0 -1]
               ((0x01<<8)),   //[-1 0 2 0 -1]
               TDSHARP_CLIP_EN1|TDSHARP_CLIP_THPOS1|TDSHARP_CLIP_THNEG1|TDSHARP_CLIP_BAND_SEL1|TDSHARP_ATTENUATE_SEL1);  
	//Band 4
    PP_WRITE32MSK(TDPROC_07, 
                (vReadQualityTable(QUALITY_TDSHARP_CLIP_EN4)<<6)|
                (vReadQualityTable(QUALITY_TDSHARP_CLIP_THPOS4)<<24)|
                (vReadQualityTable(QUALITY_TDSHARP_CLIP_THNEG4)<<16)|
                (0x04<<0)|   //[-1 0 2 0 -1]T
                (0x01<<8),   //[-1 0 2 0 -1]T
                TDSHARP_CLIP_EN4|TDSHARP_CLIP_THPOS4|TDSHARP_CLIP_THNEG4|TDSHARP_CLIP_BAND_SEL4|TDSHARP_ATTENUATE_SEL4);  
	//Band 5
    PP_WRITE32MSK(TDPROC_09, 
                (vReadQualityTable(QUALITY_TDSHARP_CLIP_EN5)<<6)|
                (vReadQualityTable(QUALITY_TDSHARP_CLIP_THPOS5)<<24)|
                (vReadQualityTable(QUALITY_TDSHARP_CLIP_THNEG5)<<16)|
                (0x05<<0)|   //[-1 0 -1; 0 4 0; -1 0 -1]
                (0x02<<8),  //[-1 0 -1; 0 4 0; -1 0 -1]
                TDSHARP_CLIP_EN5|TDSHARP_CLIP_THPOS5|TDSHARP_CLIP_THNEG5|TDSHARP_CLIP_BAND_SEL5|TDSHARP_ATTENUATE_SEL5);
	//Band 6
    PP_WRITE32MSK(TDPROC_0B, 
                (vReadQualityTable(QUALITY_TDSHARP_CLIP_EN6)<<6)|
                (vReadQualityTable(QUALITY_TDSHARP_CLIP_THPOS6)<<24)|
                (vReadQualityTable(QUALITY_TDSHARP_CLIP_THNEG6)<<16)|
                (0x06<<0)|   //[-1 0 0 0 -1; 0 0 4 0 0; -1 0 0 0 -1]
                (0x02<<8),  //[-1 0 0 0 -1; 0 0 4 0 0; -1 0 0 0 -1]
                TDSHARP_CLIP_EN6|TDSHARP_CLIP_THPOS6|TDSHARP_CLIP_THNEG6|TDSHARP_CLIP_BAND_SEL6|TDSHARP_ATTENUATE_SEL6);
	//Band 8
    PP_WRITE32MSK(TDPROC_0F, 
                (vReadQualityTable(QUALITY_TDSHARP_CLIP_EN8)<<6)|
                (vReadQualityTable(QUALITY_TDSHARP_CLIP_THPOS8)<<24)|
                (vReadQualityTable(QUALITY_TDSHARP_CLIP_THNEG8)<<16)|
                (0x02<<0)|   //[-1 -1 1 2 1 -1 -1]
                (0x02<<8),  //[-1 -1 1 2 1 -1 -1]
                TDSHARP_CLIP_EN8|TDSHARP_CLIP_THPOS8|TDSHARP_CLIP_THNEG8|TDSHARP_CLIP_BAND_SEL8|TDSHARP_ATTENUATE_SEL8);
	//Band 9
    PP_WRITE32MSK(TDPROC_15, 
                (vReadQualityTable(QUALITY_TDSHARP_CLIP_EN9)<<6)|
                (vReadQualityTable(QUALITY_TDSHARP_CLIP_THPOS9)<<24)|
                (vReadQualityTable(QUALITY_TDSHARP_CLIP_THNEG9)<<16)|
                (0x02<<0)|   //[-6 -6 -4 1 4 7 8 7 4 1 -4 -6 -6]
                (0x05<<8)|   //[-6 -6 -4 1 4 7 8 7 4 1 -4 -6 -6]
                (TDSHARP_MASK_SP),         //[-6 -6 -4 1 4 7 8 7 4 1 -4 -6 -6]  
                TDSHARP_CLIP_EN9|TDSHARP_CLIP_THPOS9|TDSHARP_CLIP_THNEG9|TDSHARP_CLIP_BAND_SEL9|TDSHARP_ATTENUATE_SEL9|TDSHARP_MASK_SP);
#endif
}

void vDrvCTIROnOff(unsigned char bOnOff)
{
    if(bOnOff > 0)
    {
        _u1PostStatus |= CTI_ON_STATUS;
    }
    else
    {
        _u1PostStatus &= ~CTI_ON_STATUS;
    }
	
  	#if 1//zxk (CONFIG_CHIP_VER_CURR == CONFIG_CHIP_VER_MT8530)||(CONFIG_DRV_ENABLE_SIMP_POST_PROC == 1)	
    	PP_WRITE32MSK(ECTI_02, ((bOnOff > 0) ? ECTI_ENA : 0), ECTI_ENA);
  	#endif
}

void vDrvCTIRParam(void)
{
#if 1//zxk (CONFIG_CHIP_VER_CURR == CONFIG_CHIP_VER_MT8530)||(CONFIG_DRV_ENABLE_SIMP_POST_PROC == 1)
    PP_WRITE32MSK(ECTI_00, 
                (vReadQualityTable(QUALITY_ECTI_FLAT_SEL)<<16)|(vReadQualityTable(QUALITY_ECTI_FLAT_OFST)<<8)|
                (vReadQualityTable(QUALITY_ECTI_T_SEL)<<4)|vReadQualityTable(QUALITY_ECTI_ADPT_SEL)|ECTI_HUE_TIE, 
                ECTI_HUE_TIE|ECTI_FLAT_SEL|ECTI_FLAT_OFST|ECTI_T_SEL|ECTI_ADPT_SEL);

    PP_WRITE32MSK(ECTI_01, 
		        ((vReadQualityTable(QUALITY_ECTI_STB_SEL) << 1)<<12)|(vReadQualityTable(QUALITY_ECTI_STB_OFST) << 2), 
		        ECTI_STB_SEL|ECTI_STB_OFST);     // Only 8-bit MSB is stored in qty tbl.
		        
    PP_WRITE32MSK(ECTI_02, vReadQualityTable(QUALITY_ECTI_LPF_SEL)<<12, ECTI_LPF_SEL);

    PP_WRITE32MSK(ECTI_03, 
		        ECTI_VMASK|ECTI_PRT_ENA|ECTI_SGN_PRT|vReadQualityTable(QUALITY_ECTI_ADPT_OFST), 
		        ECTI_VMASK|ECTI_PRT_ENA|ECTI_SGN_PRT|ECTI_ADPT_OFST);    
#endif
}

int i4PostVideoProc(POST_UI_ITEM_T e_UI_Item, int i2UIMin, 
                      int i2UIMax, int i2UIDft, int i2UICur)
{
	int u2QtyItem;
	int MapValue; //1126
	PP_LOG(PP_LOG_LVL_INFO,"Set Video %d, %d, %d, %d, %d \n", (unsigned int)e_UI_Item, i2UIMin, i2UIMax, i2UIDft, i2UICur);

    switch(e_UI_Item)
    {
	   case POST_VIDEO_SHARPNESS:
	   	
		   if(i2UIMin == 0){
			   i2UIMin = 1;
		   }

		   for(u2QtyItem = QUALITY_TDSHARP_BEGIN; u2QtyItem <= QUALITY_TDSHARP_END; u2QtyItem++)
		   {
				MapValue = u2QtyMapping(u2QtyItem, i2UIMin, i2UIMax, i2UIDft, i2UICur);
			    vWriteQualityTable(u2QtyItem, MapValue); 
		   }
				
		   
		   if(i2UICur == 0)
		   {
			    vDrvPostSharpOnOff(SV_OFF); //0
			    vDrvPostSharpParam();
		   }
		   else
		   {
		    	vDrvPostSharpOnOff(SV_ON);
			    vDrvPostSharpParam();
		   }
			 
		   PP_LOG(PP_LOG_LVL_INFO,"Set Sharpness, Level = %d \n", i2UICur);
		   break;		

        case POST_VIDEO_CTI: 
            for(u2QtyItem = QUALITY_CTI_BEGIN; u2QtyItem <= QUALITY_CTI_END; u2QtyItem++)
            {
                MapValue = u2QtyMapping(u2QtyItem, i2UIMin, i2UIMax, i2UIDft, i2UICur);
			    vWriteQualityTable(u2QtyItem, MapValue); 
            }

            if(i2UICur == 0)
            {
                vDrvCTIROnOff(SV_OFF);
				vDrvCTIRParam(); 
            }
            else
            {
                vDrvCTIROnOff(SV_ON);
                vDrvCTIRParam();          
            }

            PP_LOG(PP_LOG_LVL_INFO, "Set CTI, Level = %d \n", i2UICur);
            break;

		default:
            return POST_FAIL;
    }

    return POST_OK;
}

int _PostSharpness (unsigned char UiMin, unsigned char UiMax, unsigned char UiDft, unsigned char UiCur)
{
    i4PostVideoProc(POST_VIDEO_SHARPNESS, UiMin, UiMax, UiDft, UiCur);

	return 0;
}

int _PostCTI (unsigned char UiMin, unsigned char UiMax, unsigned char UiDft, unsigned char UiCur)
{

    i4PostVideoProc(POST_VIDEO_CTI, UiMin, UiMax, UiDft, UiCur);

	return 0;
}

void vDrvPostSharpParaSet(POST_SHN_BAND_PARA eBandPara)
{
    switch (eBandPara.eShnBand)
    {
    #if 1//zxk (CONFIG_CHIP_VER_CURR == CONFIG_CHIP_VER_MT8530)||(CONFIG_DRV_ENABLE_SIMP_POST_PROC == 1)    
        case SHN_BAND_H1: //Band 1
          PP_WRITE32MSK(TDPROC_00, 
                       (eBandPara.bGain<<24)|
                       (eBandPara.bLimitPos<<16)|
                       (eBandPara.bLimitNeg<<8)|
                       (eBandPara.bCoring),
                       TDSHARP_GAIN1|TDSHARP_LIMIT_POS1|TDSHARP_LIMIT_NEG1|TDSHARP_CORING1); 
          
          PP_WRITE32MSK(TDPROC_01, 
                      (eBandPara.bClipThPos<<24)|
                      (eBandPara.bClipThNeg<<16)|
                      (eBandPara.bClipEn<<7),
                      TDSHARP_CLIP_THPOS1|TDSHARP_CLIP_THNEG1|TDSHARP_CLIP_EN1);
        break;
            
        case SHN_BAND_V: //Band 4
		  PP_WRITE32MSK(TDPROC_06, 
                      (eBandPara.bGain<<24)|
                      (eBandPara.bLimitPos<<16)|
                      (eBandPara.bLimitNeg<<8)|
                      (eBandPara.bCoring),
                      TDSHARP_GAIN4|TDSHARP_LIMIT_POS4|TDSHARP_LIMIT_NEG4|TDSHARP_CORING4); 

          PP_WRITE32MSK(TDPROC_07, 
                      (eBandPara.bClipThPos<<24)|
                      (eBandPara.bClipThNeg<<16)|
                      (eBandPara.bClipEn<<7),
                      TDSHARP_CLIP_THPOS4|TDSHARP_CLIP_THNEG4|TDSHARP_CLIP_EN4);  
        break;
            
        case SHN_BAND_X1: //Band 5
          PP_WRITE32MSK(TDPROC_08, 
                      (eBandPara.bGain<<24)|
                      (eBandPara.bLimitPos<<16)|
                      (eBandPara.bLimitNeg<<8)|
                      (eBandPara.bCoring),
                      TDSHARP_GAIN5|TDSHARP_LIMIT_POS5|TDSHARP_LIMIT_NEG5|TDSHARP_CORING5); 

          PP_WRITE32MSK(TDPROC_09, 
                      (eBandPara.bClipThPos<<24)|
                      (eBandPara.bClipThNeg<<16)|
                      (eBandPara.bClipEn<<7),
                      TDSHARP_CLIP_THPOS5|TDSHARP_CLIP_THNEG5|TDSHARP_CLIP_EN5);  
        break;
            
        case SHN_BAND_X2: //Band 6
          PP_WRITE32MSK(TDPROC_0A, 
                      (eBandPara.bGain<<24)|
                      (eBandPara.bLimitPos<<16)|
                      (eBandPara.bLimitNeg<<8)|
                      (eBandPara.bCoring),
                      TDSHARP_GAIN6|TDSHARP_LIMIT_POS6|TDSHARP_LIMIT_NEG6|TDSHARP_CORING6); 

          PP_WRITE32MSK(TDPROC_0B, 
                      (eBandPara.bClipThPos<<24)|
                      (eBandPara.bClipThNeg<<16)|
                      (eBandPara.bClipEn<<7),
                      TDSHARP_CLIP_THPOS6|TDSHARP_CLIP_THNEG6|TDSHARP_CLIP_EN6);  
        break;
            
        case SHN_BAND_H2: //Band 8
          PP_WRITE32MSK(TDPROC_0E, 
                      (eBandPara.bGain<<24)|
                      (eBandPara.bLimitPos<<16)|
                      (eBandPara.bLimitNeg<<8)|
                      (eBandPara.bCoring),
                      TDSHARP_GAIN8|TDSHARP_LIMIT_POS8|TDSHARP_LIMIT_NEG8|TDSHARP_CORING8); 

          PP_WRITE32MSK(TDPROC_0F, 
                      (eBandPara.bClipThPos<<24)|
                      (eBandPara.bClipThNeg<<16)|
                      (eBandPara.bClipEn<<7),
                      TDSHARP_CLIP_THPOS8|TDSHARP_CLIP_THNEG8|TDSHARP_CLIP_EN8);  
        break;
            
        case SHN_BAND_H3: //Band 9
          PP_WRITE32MSK(TDPROC_14, 
                      (eBandPara.bGain<<24)|
                      (eBandPara.bLimitPos<<16)|
                      (eBandPara.bLimitNeg<<8)|
                      (eBandPara.bCoring),
                      TDSHARP_GAIN9|TDSHARP_LIMIT_POS9|TDSHARP_LIMIT_NEG9|TDSHARP_CORING9); 

          PP_WRITE32MSK(TDPROC_15, 
                      (eBandPara.bClipThPos<<24)|
                      (eBandPara.bClipThNeg<<16)|
                      (eBandPara.bClipEn<<7),
                      TDSHARP_CLIP_THPOS9|TDSHARP_CLIP_THNEG9|TDSHARP_CLIP_EN9);  
            break;
     #endif
	
        default:
            break;
    }
}

void vDrvPostSharpCtrlSet(POST_SHN_CTRL_PARA eCtrlPara)
{
#if 1//zxk (CONFIG_CHIP_VER_CURR == CONFIG_CHIP_VER_MT8530)||(CONFIG_DRV_ENABLE_SIMP_POST_PROC == 1)
	PP_WRITE32MSK(TDPROC_10, 
				 (eCtrlPara.fgShnEn ? TDSHARP_EN : 0)|
				 (eCtrlPara.bLimitAllPos<<16)|
				 (eCtrlPara.bLimitAllNeg<<8),
				 TDSHARP_EN|TDSHARP_LIMIT_POS_ALL|TDSHARP_LIMIT_NEG_ALL); 
#endif
}

void eDrvCTIRParamSet(POST_CTI_CTRL_PARA eRetPara)
{
	//ZXK (CONFIG_CHIP_VER_CURR == CONFIG_CHIP_VER_MT8530)||(CONFIG_DRV_ENABLE_SIMP_POST_PROC == 1)
	PP_WRITE32MSK(ECTI_01, (eRetPara.bECTIStbSel<<12), ECTI_STB_SEL); 
	PP_WRITE32MSK(ECTI_02, (eRetPara.bECTIFlpfSel<<12)|(eRetPara.fgCtiEn<<28), ECTI_ENA|ECTI_LPF_SEL); 

	PP_LOG(PP_LOG_LVL_INFO, "CTIStb = 0x%x, CTILpfSel = 0x%x, CTIEnable = %d \n", eRetPara.bECTIStbSel, eRetPara.bECTIFlpfSel, eRetPara.fgCtiEn);
}

