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
#ifndef __ARM2__
#include <linux/mm.h>
#include <linux/delay.h>
#include <mach/pinmux.h>
#include <mach/ac83xx_gpio_pinmux.h>
#include <mach/ac83xx_pinmux_table.h>
//#include <media/atc/drv_av_d.h>
#include <media/atc/display_inc.h>
#include <media/atc/ac823x/pmx_hal.h>
#include "x_stl_lib.h"
#include "x_debug.h"
#else
#include "x_types.h"
//#include "ac83xx_gpio_pinmux.h"
//#include "ac83xx_pinmux_table.h"
#include "x_ckgen_8317.h"
//#include "drv_av_d.h"
#include "display_inc.h"
#include "pmx_hal.h"
#endif
#include "x_os.h"
#include "x_rtos.h"
#include "x_assert.h"
#include "x_util.h"
#include "x_printf.h"
#include "x_bim.h"

#include "pmx_vfy_drv.h"
#include "pmx_vfy_hal.h"
#include "tcon.h"
#include "log.h"

extern unsigned long IO_BASE_BRINGUP;
#define WriteREG(arg, val) (*(volatile __u32*)(IO_BASE_BRINGUP + (arg)) = val)
#define ReadREG(arg)       (*(volatile __u32*)(IO_BASE_BRINGUP + (arg)))
#define WriteREGMsk(arg, val, msk) WriteREG((arg), (ReadREG(arg) & (~(msk))) | ((val) & (msk)))
#define WriteRegMsk(arg, val, msk) WriteREG((arg), (ReadREG(arg) & (~(msk))) | ((val) & (msk)))

#define UTIL_Printf(a,...)

int _i4VerifyRoutineBreakCnt = 0;
volatile unsigned int g_u4PmxVfyDelayTime = 1000; // default 1s.
extern volatile BOOL g_fgNeedEnableXHalf;
volatile unsigned int fpd_panel_size =0;
void vPmxVerifyHalVdoRXSkip(unsigned int u4RXSkip, UCHAR ucVdoId);


//picture mode
char *g_szPmxVfyPicMode[] =
{
    "PMX_VFY_VDO_FLD",
    "PMX_VFY_VDO_FRM",
    "PMX_VFY_VDO_4MA",
    "PMX_VFY_VDO_4MEMA",
    "PMX_VFY_VDO_4FUSION",
    "PMX_VFY_VDO_4FUSION_CS",
};

//source format
char *g_szPmxVfySrcFormat[] =
{
    "PMX_VFY_VDO_SRC_420_MB",
    "PMX_VFY_VDO_SRC_422_MB",
    "PMX_VFY_VDO_SRC_420_SL",
    "PMX_VFY_VDO_SRC_422_SL",
};

//definition of ZOOM type
char *g_szPmxVfyHZoomType[] =
{
    "null",
    "PMX_VFY_H_ZOOM_1X",
    "PMX_VFY_H_ZOOM_2X",
    "PMX_VFY_H_ZOOM_3X",
    "PMX_VFY_H_ZOOM_4X",
    "PMX_VFY_H_ZOOM_A2X",
    "PMX_VFY_H_ZOOM_A3X",
    "PMX_VFY_H_ZOOM_A4X",
    "PMX_VFY_H_ZOOM_1D2X",
    "PMX_VFY_H_ZOOM_1D3X",
    "PMX_VFY_H_ZOOM_1D4X",
    "PMX_VFY_H_ZOOM_A1D2X",
    "PMX_VFY_H_ZOOM_A1D3X",
    "PMX_VFY_H_ZOOM_A1D4X",
};

char *g_szPmxVfyVZoomType[] =
{
    "null",
    "PMX_VFY_V_ZOOM_1X",
    "PMX_VFY_V_ZOOM_2X",
    "PMX_VFY_V_ZOOM_3X",
    "PMX_VFY_V_ZOOM_4X",
    "PMX_VFY_V_ZOOM_A2X",
    "PMX_VFY_V_ZOOM_A3X",
    "PMX_VFY_V_ZOOM_A4X",
    "PMX_VFY_V_ZOOM_1D2X",
    "PMX_VFY_V_ZOOM_1D3X",
    "PMX_VFY_V_ZOOM_1D4X",
    "PMX_VFY_V_ZOOM_A1D2X",
    "PMX_VFY_V_ZOOM_A1D3X",
    "PMX_VFY_V_ZOOM_A1D4X",
};

char *g_szPmxVfyDemoBdr[] =
{
    "PMX_VFY_DEMO_BDR_0D8",
    "PMX_VFY_DEMO_BDR_1D8",
    "PMX_VFY_DEMO_BDR_2D8",
    "PMX_VFY_DEMO_BDR_3D8",
    "PMX_VFY_DEMO_BDR_4D8",
    "PMX_VFY_DEMO_BDR_5D8",
    "PMX_VFY_DEMO_BDR_6D8",
    "PMX_VFY_DEMO_BDR_7D8",
    "PMX_VFY_DEMO_BDR_8D8",
};
extern unsigned int _u4PmxVdoIndex;
extern REG_SET_T _ArOutputInitSetting[];
extern REG_SET_T _ArEMUfpdFPGADISP[];

extern void vFpdInit (unsigned int dty_cyc, unsigned int pType);
void vPmxVerifyPanelSizeSel(unsigned int u4VdoId,unsigned int u4PmxFmt,UCHAR ucTvType)
{
	if(u4VdoId==0){
		switch(u4PmxFmt)
	    {
	    case 480:
			fpd_panel_size = PANEL_SIZE_800_480;
	        break;
		case 600:
			if(ucTvType == PMX_TV_TYPE_1024X600){
				fpd_panel_size = PANEL_SIZE_1024_600;
			}else{
				fpd_panel_size = PANEL_SIZE_800_600;
			}
			break;	
		case 1080:
			fpd_panel_size = PANEL_SIZE_1920_1080;
			break;

		default:
			fpd_panel_size =PANEL_SIZE_800_480;
			break;
		}
	}else{}//none
}

void vPmxVerifyHalXfsInit(unsigned int u4VdoId,unsigned int u4PmxFmt,UCHAR ucTvType,UCHAR ucInterlace){
	//unsigned int xfs_width;
	//unsigned int fs_width;
	//unsigned int extern_crital;
#if PMX_REALCHIP_EN
#else
	if(u4VdoId ==0){
		switch(u4PmxFmt){
			case 720:
				WriteREG(0x1f0a0,0x80ffff28); 
				WriteREG(0x1f0a0,0xa0ffff28); 
				WriteREG(0x1f0a0,0x80ffff28); 
				//2fs
				WriteREG(0x1f0a0,0x80179e08); 				
				WriteREG(0x1f0a0,0xa0179e08);
				WriteREG(0x1f0a0,0x80179e08);
				//fs
				WriteREG(0x1f0a0,0x801f3c0a); 
				WriteREG(0x1f0a0,0xa01f3c0a);
				WriteREG(0x1f0a0,0x801f3c0a);
				
				WriteREG(0x1f0a0,0x00000000); 				
				break;				
			case 480:
				/*
				//800*480
				WriteREG(0x1f0a0,0x80ffff28); 
				WriteREG(0x1f0a0,0xa0ffff28); 
				WriteREG(0x1f0a0,0x80ffff28); 
				//6fs
				WriteREG(0x1f0a0,0x80128b08); 				
				WriteREG(0x1f0a0,0xa0128b08);
				WriteREG(0x1f0a0,0x80128b08);
				//fs
				WriteREG(0x1f0a0,0x801f3c0a); 
				WriteREG(0x1f0a0,0xa01f3c0a);
				WriteREG(0x1f0a0,0x801f3c0a);
				
				WriteREG(0x1f0a0,0x00000000); 
				*/
				break;
			case 1080:
				WriteREG(0x1f0a0,0x80ffff28); 
				WriteREG(0x1f0a0,0xa0ffff28); 
				WriteREG(0x1f0a0,0x80ffff28); 
				//1fs
				WriteREG(0x1f0a0,0x801f3c08); 				
				WriteREG(0x1f0a0,0xa01f3c08);
				WriteREG(0x1f0a0,0x801f3c08);
				//fs
				WriteREG(0x1f0a0,0x801f3c0a); 
				WriteREG(0x1f0a0,0xa01f3c0a);
				WriteREG(0x1f0a0,0x801f3c0a);
				
				WriteREG(0x1f0a0,0x00000000); 
				break;
			case 600: 
				if(ucTvType == PMX_TV_TYPE_1024X600){
					//2fs
					WriteREG(0x1f0a0,0x80ffff28); 
					WriteREG(0x1f0a0,0xa0ffff28); 
					WriteREG(0x1f0a0,0x80ffff28); 
					//2fs
					WriteREG(0x1f0a0,0x80179e08);				
					WriteREG(0x1f0a0,0xa0179e08);
					WriteREG(0x1f0a0,0x80179e08);
					//fs
					WriteREG(0x1f0a0,0x801f3c0a); 
					WriteREG(0x1f0a0,0xa01f3c0a);
					WriteREG(0x1f0a0,0x801f3c0a);
					
					WriteREG(0x1f0a0,0x00000000);				
				}else{
					// 800x600 do 4fs.
					WriteREG(0x1f0a0,0x80ffff28); 
					WriteREG(0x1f0a0,0xa0ffff28); 
					WriteREG(0x1f0a0,0x80ffff28); 
					//4fx
					WriteREG(0x1f0a0,0x8013cf08);				
					WriteREG(0x1f0a0,0xa013cf08);
					WriteREG(0x1f0a0,0x8013cf08);
					//fs 6M(12M cristal)
					WriteREG(0x1f0a0,0x801f3c0a); 
					WriteREG(0x1f0a0,0xa01f3c0a);
					WriteREG(0x1f0a0,0x801f3c0a);
					
					WriteREG(0x1f0a0,0x00000000); 
					// 6f6: low= high=36
				}
				break;
			default:
				break;				
		}		
		//WriteREG(0x1f034,0x40000000); //u2 2fs
	}
#endif	
	if(u4VdoId == 1){		
	     switch(u4PmxFmt){
			case 720:
				WriteREGMsk(0x1f038,1<<26,1<<26); //720p xfs ckgen enable;
#if PMX_REALCHIP_EN
#else				
				WriteREGMsk(0x1f038,1<<31,1<<31); //no need in real chip
#endif				
				break;
			case 1080:
				if(ucInterlace == 1){
#if PMX_REALCHIP_EN
#else				
					WriteREGMsk(0x1f038,1<<31,1<<31);//no need in real chip	
#endif					
					WriteREGMsk(0x1f038,1<<26,1<<26); 
				}else{
#if PMX_REALCHIP_EN
#else				
					WriteREGMsk(0x1f038,0<<31,1<<31);		
#endif
					WriteREGMsk(0x1f038,0<<26,1<<26); 
				}
				break;
			default:
#if PMX_REALCHIP_EN
#else				
				WriteREGMsk(0x1f038,0<<31,1<<31);		
#endif
				WriteREGMsk(0x1f038,0<<26,1<<26); 
				break;			
		 }



	}

	
}

extern void vFpdInit (unsigned int dty_cyc, unsigned int pType);
extern void vPanelLVDSInit(unsigned int LinkMode);

void vPmxVerifyHalSysInit(void)
	{	
	//FIXME: temp code.
	switch(fpd_panel_size){
		//
		case(PANEL_SIZE_800_480):{
			vFpdInit(100,32000); //1st seg:50 fix(0~100)  2nd seg: DPIX CLK KHZ
			WriteREG(0x00000608,0x2eda4028); //refclk:32M						
			break;
		}
		case(PANEL_SIZE_800_600):{
			vFpdInit(100,40000); //1st seg:50 fix(0~100)  2nd seg: DPIX CLK KHZ			
			WriteREG(0x00000608,0x3ed84028); //refclk:40M	
			WriteREGMsk(0xDC,0<<22,1<<22);//FPD Clock Clock Invert Used for Clock & Data Phase not match
			break;
		}
		case(PANEL_SIZE_1024_600):{
			vFpdInit(52,55000); //1st seg:50 fix(0~100)  2nd seg: DPIX CLK KHZ			
			vPanelLVDSInit(0);
			WriteREG(0x00000608,0x1eda402a); //refclk:55M						
			break;
		}
		case(PANEL_SIZE_1920_1080):{
			vFpdInit(100,148500); //1st seg:50 fix(0~100)  2nd seg: DPIX CLK KHZ
			vPanelLVDSInit(1);
			WriteREGMsk(0x000000d8,(0x1 << 2),(1<<2));			
			break;
		}		
		default:{
			break;
		}

	}
	
	WriteREGMsk(0x000a4700,1<<19,1<<19); //del if xiaojian has update his code. 				
#if 1 //just for 3365 emu fpgadisp path.
	//vPmxVerifyHalLoadSetting(&_ArEMUfpdFPGADISP[0]);
#else
		//jg:fixme
		//WriteREGMsk(0x0005c, (0xf<<6), (0xf<<6)); //TTL sync select
		//WriteREGMsk(0x000b4, (0x10180e7<<0), (0x10180e7<<0)); //tve,ypbpr,tvd,scler,DVD_MIX_2AP Clock Enable
		//WriteREGMsk(0x000d0, (0x10180e7<<0), (0x10180e7<<0)); //tve,ypbpr,tvd,scler,DVD_MIX_2APReset Enable    
		//WriteREGMsk(0x000b0, (1<<0), (1<<0)); //LVDS clock enable
		//WriteREGMsk(0x000cc, (1<<0), (1<<0)); //LVDS reset enable
			
	if(_u4PmxVdoIndex == 0){   
#if (PANEL_TYPE_SELECT == PANNEL_TTL) // TTL Panel
		if(PANEL_SIZE_SELECT==PANEL_SIZE_800_480){
		  vPmxVerifyHalLoadSetting(&_ArOutputInitSetting[0]);
		}
#elif (PANEL_TYPE_SELECT == PANNEL_LVDS)	// LVDS Panel   
		//1024x600 panel init
		vPmxVerifyHalLoadSetting(&_ArOutputInitSetting[3]);
#endif
	}else{}//fmtr using tve.
	
#if (PANEL_TYPE_SELECT == PANNEL_TTL) // TTL Panel
	  switch(PANEL_SIZE_SELECT){
		 case(PANEL_SIZE_800_480):{
			 vFpdInit(100,1);
			 break;
		 }
		 case(PANEL_SIZE_800_600):{
			 vFpdInit(100,2);
			 break;
		 }
		 default:{
			 break;
		 }
	  }
#else if(PANEL_TYPE_SELECT == PANNEL_LVDS)
	   switch(PANEL_SIZE_SELECT){
		 case(PANEL_SIZE_1024_600):{
			 vFpdInit(100,0);
			 break;
		 }
		 case(PANEL_SIZE_1280_800):{
			 vFpdInit(100,3);		
			 break;
		 }
		 case(PANEL_SIZE_1280_720):{
			 vFpdInit(100,4);	
			 break;
		 }
		 default:
			break;
		}
#endif
#endif
	}


void vPmxVerifyHalLoadSetting(REG_SET_T *prRegSet)
{
    unsigned int u4Idx;
    unsigned int u4RegAddr, u4RegVal;
    unsigned int *pu4Array = prRegSet->pu4RegSetting;
    unsigned int u4Size = prRegSet->size;

    //UTIL_Printf("[pmx] %s: %s\n", __FUNCTION__, prRegSet->szArrayName);

    u4Size = u4Size/2;

    for(u4Idx =0; u4Idx<u4Size; u4Idx++)
    {
        u4RegAddr = pu4Array[u4Idx*2];
        u4RegVal = pu4Array[u4Idx*2+1];
        WriteREG(u4RegAddr, u4RegVal);
    }

    return;
}

void vPmxVerifyHalLoadCavSetting(REG_SET_T *prRegSet, BOOL fgIndex) //load full 0x00~0xFF
{
    unsigned int u4Idx;
    unsigned int u4RegVal;
    unsigned int *pu4Array = prRegSet->pu4RegSetting;

    //UTIL_Printf("[pmx] %s: %s\n", __FUNCTION__, prRegSet->szArrayName);

    /*if(fgIndex)
    {
    WriteREG(0x20604, 0x4A0104A0);
    WriteREG(0x20678, 0x00000202);
    }
    else
    {
    WriteREG(0x20604, 0x00000003);
    WriteREG(0x20678, 0x00000000);
    }
    */

    for(u4Idx =0; u4Idx< (0xDC/4); u4Idx++)
    {
        u4RegVal = pu4Array[u4Idx];
        WriteREG((0x3600 + u4Idx*4), u4RegVal);
    }

    for(u4Idx =0; u4Idx< (0x20/4); u4Idx++)
    {
        u4RegVal = pu4Array[(0xDC/4) + u4Idx];
        WriteREG((0x3700 + u4Idx*4), u4RegVal);
    }

    u4RegVal = pu4Array[16*4-1];
    WriteREG(0x36E0, u4RegVal);

    return;
}

void vPmxVerifyHalLoadCvbsSetting(REG_SET_T *prRegSet, BOOL fgIndex) //load full 0x00~0xFF
{
    unsigned int u4Idx;
    unsigned int u4RegVal;
    unsigned int *pu4Array = prRegSet->pu4RegSetting;

    //UTIL_Printf("[pmx] %s: %s\n", __FUNCTION__, prRegSet->szArrayName);

    for(u4Idx =0; u4Idx< (0x84/4); u4Idx++)
    {
        u4RegVal = pu4Array[u4Idx];
        WriteREG((0x2000 + u4Idx*4), u4RegVal);
    }

    for(u4Idx =0; u4Idx< (0x1C/4); u4Idx++)
    {
        u4RegVal = pu4Array[(0x84/4) + u4Idx];
        WriteREG((0x20700 + u4Idx*4), u4RegVal);
    }

    return;
}

void vPmxVerifyHalVdoPtr(UCHAR ucVdoId, UINT64 u4YBuf, UINT64 u4CBuf)
{
    unsigned int u4VdoRegBase, u4RegVal;

    if(ucVdoId == 0)
        u4VdoRegBase = 0x42400;
    else if(ucVdoId == 1)
        u4VdoRegBase = 0x43400;
    else
        u4VdoRegBase = 0x43a00;

    u4RegVal = u4YBuf >> 2;
    WriteREG((u4VdoRegBase + 0x00), u4RegVal); // Y
    WriteREG((u4VdoRegBase + 0x08), u4RegVal); // X
    WriteREG((u4VdoRegBase + 0x80), u4RegVal); // W
    WriteREG((u4VdoRegBase + 0x84), u4RegVal); // Z
    WriteREG((u4VdoRegBase + 0xEC), u4RegVal); // A
    u4RegVal = u4CBuf >> 2;
    WriteREG((u4VdoRegBase + 0x04), u4RegVal); // Y
    WriteREG((u4VdoRegBase + 0x0C), u4RegVal); // X
    WriteREG((u4VdoRegBase + 0xFC), u4RegVal); // Z
}

void vPmxVerifyHalSclDRAMPtr(UCHAR ucVdoId)
{
    unsigned int u4YAddr;
    unsigned int u4CAddr;
    unsigned int u4VdoRegBase;

    if(ucVdoId == 0)
    {
        u4VdoRegBase = 0x42400;
    }
    else if(ucVdoId == 1)
    {
        u4VdoRegBase = 0x43400;
    }
    else
    {
        u4VdoRegBase = 0x43a00;
    }

    //change scaler DRAM mode read pointer to MVDO frame buffer pointer for DRAM 420 read mode
    u4YAddr = ReadREG(u4VdoRegBase + 0x00);
    u4CAddr = ReadREG(u4VdoRegBase + 0x04);
    WriteREG(0x3c00, u4YAddr);
    WriteREG(0x3c04, u4CAddr);
}

void vPmxVerifyHalAVIFrame(UCHAR ucRes, UCHAR ucColor)
{
    UINT8 u1Data = 0, u1Data2, u1Data3, u1Data4, u1Data5, u1Chksum;
    unsigned int u4Ind;

    WriteREGMsk(0x2101c, 0, (1<<6));
    WriteREG(u4HdmiAviInfoReg[0], u4HdmiAviInfoReg[1]);
    WriteREG(u4HdmiAviInfoReg[2], u4HdmiAviInfoReg[3]);
    WriteREG(u4HdmiAviInfoReg[4], u4HdmiAviInfoReg[5]);
    if(ucColor == 1)//RGB
    {
        u1Data = 0x00;
    }
    else  if((ucColor == 2)||(ucColor == 4))//ycbcr444
    {
        u1Data = 0x40;
    }
    else  if(ucColor == 3)//ycbcr422
    {
        u1Data = 0x20;
    }

    u1Data2 = 0;
    if(((ucRes >= RES_720P60HZ) && (ucRes <= RES_1080P25HZ))||
        ((ucRes >= RES_1080P24HZ) && (ucRes <= RES_1080P29_97HZ)))//HD
    {
        if(ucColor == 4)
            u1Data2 |= ((3 <<6) | (2<<4));//other, 16:9
        else
            u1Data2 |= ((2 <<6) | (2<<4));//709, 16:9
    }
    else
    {
        if(ucColor == 4)
            u1Data2 |= ((3 <<6) | (1<<4));//other, 16:9
        else
            u1Data2 |= ((1 <<6) | (1<<4));//601, 4:3
    }

    if(ucColor == 4)
    {
        if(((ucRes >= RES_720P60HZ) && (ucRes <= RES_1080P25HZ))||
            ((ucRes >= RES_1080P24HZ) && (ucRes <= RES_1080P29_97HZ)))//HD
            u1Data3 = 0x10;
        else
            u1Data3 = 0x00;
    }
    else
        u1Data3 = 0;

    u1Data4 = u1HdmiVideoID[ucRes - RES_480I];

    u1Data5 = 0; //no repeat
    if((ucRes == RES_480I) || (ucRes == RES_576I) || (ucRes == RES_480P_1440) || (ucRes == RES_576P_1440))
    {
        u1Data5 = 1; //repeat 2 times
    }
    else if((ucRes == RES_480P_2880) || (ucRes == RES_576P_2880) ||
        (ucRes == RES_480I_2880) || (ucRes == RES_576I_2880))
    {
        u1Data5 = 3; //repeat 4 times
    }

    u1Chksum = (UINT8)(u4HdmiAviInfoReg[1]) + (UINT8)(u4HdmiAviInfoReg[3]) + (UINT8)(u4HdmiAviInfoReg[5]) + u1Data + u1Data2 + u1Data3 + u1Data4 + u1Data5;
    u1Chksum =0x100 - u1Chksum;

    WriteREG(0x21188, u1Chksum);
    WriteREG(0x21188, u1Data);
    WriteREG(0x21188, u1Data2);
    WriteREG(0x21188, u1Data3);
    WriteREG(0x21188, u1Data4);//data4
    WriteREG(0x21188, u1Data5);

    for(u4Ind = 0; u4Ind < 8 ; u4Ind ++)
        WriteREG(0x21188, 0);
    WriteREGMsk(0x2101c, (1<<6), (1<<6));
}

#define PHASE_NO    8
#define H_Y_TAP_NO  10
#define H_C_TAP_NO  8

unsigned int _u4PmxVerifySclHRegIdxTblY[PHASE_NO][H_Y_TAP_NO]= {
    {0x31F0,0x3190,0x3190,0x3190,  0x3190,0x3194,  0x3194,0x3194,0x3194,0x31F0},//phase 0
    {0x31F0,0x3198,0x3198,0x3198,  0x3198,0x319c,  0x319c,0x319c,0x319c,0x31F0},//phase 1
    {0x31F4,0x31c0,0x31c0,0x31c0,  0x31c0,0x31c4,  0x31c4,0x31c4,0x31c4,0x31F4},//phase 2
    {0x31F4,0x31c8,0x31c8,0x31c8,  0x31c8,0x31cc,  0x31cc,0x31cc,0x31cc,0x31F4},//phase 3
    {0x31F8,0x31d0,0x31d0,0x31d0,  0x31d0,0x31d4,  0x31d4,0x31d4,0x31d4,0x31F8},//phase 4
    {0x31F8,0x31d8,0x31d8,0x31d8,  0x31d8,0x31dc,  0x31dc,0x31dc,0x31dc,0x31F8},//phase 5
    {0x3240,0x31e0,0x31e0,0x31e0,  0x31e0,0x31e4,  0x31e4,0x31e4,0x31e4,0x3240},//phase 6
    {0x3240,0x31e8,0x31e8,0x31e8,  0x31e8,0x31ec,  0x31ec,0x31ec,0x31ec,0x3240} //phase 7
};

unsigned int _u4PmxVerifySclHRegBitLShiftTblY[PHASE_NO][H_Y_TAP_NO]= {
    {0,  0,  8,  16,  24,  0,  8,  16, 24,  8},//phase 0
    {16, 0,  8,  16,  24,  0,  8,  16, 24, 24},//phase 1
    {0,  0,  8,  16,  24,  0,  8,  16, 24,  8},//phase 2
    {16, 0,  8,  16,  24,  0,  8,  16, 24, 24},//phase 3
    {0,  0,  8,  16,  24,  0,  8,  16, 24,  8},//phase 4
    {16, 0,  8,  16,  24,  0,  8,  16, 24, 24},//phase 5
    {0,  0,  8,  16,  24,  0,  8,  16, 24,  8},//phase 6
    {16, 0,  8,  16,  24,  0,  8,  16, 24, 24} //phase 7
};

unsigned int PMX_VFY_SCL_Y_H_COEF_SCLDS[6][PHASE_NO][H_Y_TAP_NO] = {
    {//filter 0 nuttall158 fc 0.0345 720p
        {0x00,0xfd,0xfb,0x40,  0x90,0x76,  0x11,0xf9,0x00,0x00},//phase 0
        {0x00,0xfe,0xfa,0x39,  0x8f,0x47,  0xfc,0xfd,0x00,0x00},//phase 1
        {0x00,0xfe,0xf9,0x33,  0x8c,0x4f,  0xfe,0xfd,0x00,0x00},//phase 2
        {0x00,0xff,0xf9,0x2c,  0x8a,0x56,  0x00,0xfc,0x00,0x00},//phase 3
        {0x00,0xff,0xf9,0x26,  0x88,0x5d,  0x02,0xfb,0x00,0x00},//phase 4
        {0x00,0xff,0xf9,0x20,  0x84,0x64,  0x05,0xfb,0x00,0x00},//phase 5
        {0x00,0xff,0xf9,0x1a,  0x81,0x6a,  0x09,0xfa,0x00,0x00},//phase 6
        {0x00,0x00,0xf9,0x15,  0x7c,0x70,  0x0c,0xfa,0x00,0x00} //phase 7
    },
    {//filter 1 nuttall158 fc 0.0233 1080i
        {0x00,0xff,0x0c,0x41,  0x68,0x5d,  0x22,0x02,0xff,0x00},//phase 0
        {0x00,0xff,0x0a,0x3d,  0x68,0x45,  0x0e,0xff,0x00,0x00},//phase 1
        {0x00,0xff,0x09,0x39,  0x66,0x49,  0x11,0xff,0x00,0x00},//phase 2
        {0x00,0xff,0x07,0x35,  0x66,0x4d,  0x13,0xff,0x00,0x00},//phase 3
        {0x00,0xff,0x06,0x31,  0x65,0x50,  0x16,0x00,0xff,0x00},//phase 4
        {0x00,0xff,0x04,0x2d,  0x64,0x54,  0x19,0x00,0xff,0x00},//phase 5
        {0x00,0xff,0x03,0x2a,  0x62,0x57,  0x1c,0x00,0xff,0x00},//phase 6
        {0x00,0xff,0x02,0x26,  0x60,0x5a,  0x1f,0x01,0xff,0x00} //phase 7
    },
    {//filter 1 nuttall158 fc 0.0233 1080p
        {0x00,0xff,0x0c,0x41,  0x68,0x5d,  0x22,0x02,0xff,0x00},//phase 0
        {0x00,0xff,0x0a,0x3d,  0x68,0x45,  0x0e,0xff,0x00,0x00},//phase 1
        {0x00,0xff,0x09,0x39,  0x66,0x49,  0x11,0xff,0x00,0x00},//phase 2
        {0x00,0xff,0x07,0x35,  0x66,0x4d,  0x13,0xff,0x00,0x00},//phase 3
        {0x00,0xff,0x06,0x31,  0x65,0x50,  0x16,0x00,0xff,0x00},//phase 4
        {0x00,0xff,0x04,0x2d,  0x64,0x54,  0x19,0x00,0xff,0x00},//phase 5
        {0x00,0xff,0x03,0x2a,  0x62,0x57,  0x1c,0x00,0xff,0x00},//phase 6
        {0x00,0xff,0x02,0x26,  0x60,0x5a,  0x1f,0x01,0xff,0x00} //phase 7
    },
    {
        //10 tap 8 phase odd
        {0x00,0xf3,0x00,0x4c,  0x82,0x4c,  0x00,0xf3,0x00,0x00},//phase 0
        {0x01,0xf4,0xfb,0x40,  0x81,0x56,  0x06,0xf2,0xff,0x02},//phase 1
        {0x02,0xf6,0xf7,0x35,  0x7d,0x60,  0x0e,0xf1,0xfd,0x03},//phase 2
        {0x02,0xf8,0xf4,0x2a,  0x78,0x69,  0x17,0xf1,0xfc,0x03},//phase 3
        {0x02,0xfa,0xf2,0x20,  0x72,0x72,  0x20,0xf2,0xfa,0x02},//phase 4
        {0x03,0xfc,0xf1,0x17,  0x69,0x78,  0x2a,0xf4,0xf8,0x02},//phase 5
        {0x03,0xfd,0xf1,0x0e,  0x60,0x7d,  0x35,0xf7,0xf6,0x02},//phase 6
        {0x02,0xff,0xf2,0x06,  0x56,0x81,  0x40,0xfb,0xf4,0x01} //phase 7
    }
#if 0
    {//filter 2 nuttall158 fc 0.0455 1080p
        {0x00,0x02,0xf0,0x31,  0xba,0x8c,  0xf7,0xfc,0x01,0x00},//phase 0 //M1, 0 ,1, 2,3,4,5,6,7,8, reg 0x3190,0x3194,0x31f0
        {0x00,0x02,0xf2,0x27,  0xb9,0x3c,  0xef,0x01,0x00,0x00},//phase 1
        {0x00,0x02,0xf3,0x1d,  0xb7,0x47,  0xef,0x01,0x00,0x00},//phase 2
        {0x00,0x02,0xf5,0x14,  0xb2,0x53,  0xef,0x01,0x00,0x00},//phase 3
        {0x00,0x02,0xf6,0x0d,  0xae,0x5e,  0xef,0x00,0x00,0x00},//phase 4
        {0x00,0x01,0xf8,0x06,  0xa8,0x6a,  0xf0,0xff,0x00,0x00},//phase 5
        {0x00,0x01,0xf9,0x00,  0xa0,0x76,  0xf1,0xfe,0x01,0x00},//phase 6
        {0x00,0x01,0xfb,0xfb,  0x96,0x81,  0xf4,0xfd,0x01,0x00} //phase 7
    }
#endif
};

unsigned int PMX_VFY_SCL_Y_H_MSB_SCLDS[6][4] = {
    {//filter 0
        0x00,0x00,0x00,0x00//phase 7-0
    },
    {//filter 1
        0x00,0x00,0x00,0x00//phase 7-0
    },
    {//filter 2
        0x00,0x00,0x00,0x00//phase 7-0
    },
    {
        0x00,0x00,0x00,0x00
    }
};

unsigned int _u4PmxVerifySclHRegIdxTblC[PHASE_NO][H_C_TAP_NO]= {
    {0x3200,0x3200,0x3200,  0x3200,0x3204,  0x3204,0x3204,0x3204},//phase 0
    {0x3208,0x3208,0x3208,  0x3208,0x320c,  0x320c,0x320c,0x320c},//phase 1
    {0x3210,0x3210,0x3210,  0x3210,0x3214,  0x3214,0x3214,0x3214},//phase 2
    {0x3218,0x3218,0x3218,  0x3218,0x321c,  0x321c,0x321c,0x321c},//phase 3
    {0x3220,0x3220,0x3220,  0x3220,0x3224,  0x3224,0x3224,0x3224},//phase 4
    {0x3228,0x3228,0x3228,  0x3228,0x322c,  0x322c,0x322c,0x322c},//phase 5
    {0x3230,0x3230,0x3230,  0x3230,0x3234,  0x3234,0x3234,0x3234},//phase 6
    {0x3238,0x3238,0x3238,  0x3238,0x323c,  0x323c,0x323c,0x323c} //phase 7
};

unsigned int _u4PmxVerifySclHRegBitLShiftTblC[PHASE_NO][H_C_TAP_NO]= {
    {0,  8,  16,  24,  0,  8,  16, 24},//phase 0
    {0,  8,  16,  24,  0,  8,  16, 24},//phase 1
    {0,  8,  16,  24,  0,  8,  16, 24},//phase 2
    {0,  8,  16,  24,  0,  8,  16, 24},//phase 3
    {0,  8,  16,  24,  0,  8,  16, 24},//phase 4
    {0,  8,  16,  24,  0,  8,  16, 24},//phase 5
    {0,  8,  16,  24,  0,  8,  16, 24},//phase 6
    {0,  8,  16,  24,  0,  8,  16, 24} //phase 7
};

//horizontal c filter coef [filter][phase][n]
unsigned int PMX_VFY_SCL_C_H_COEF_SCLDS[6][PHASE_NO][H_C_TAP_NO] = {
    {//filter 0 nuttall126 fc 0.0173 720p
        {0x00,0x0d,0x3f,  0x68,0x5c,  0x21,0x03,0x00},//phase 0
        {0x00,0x0b,0x3b,  0x67,0x43,  0x0f,0x01,0x00},//phase 1
        {0x00,0x09,0x37,  0x67,0x47,  0x11,0x01,0x00},//phase 2
        {0x00,0x08,0x33,  0x66,0x4b,  0x13,0x01,0x00},//phase 3
        {0x00,0x07,0x2f,  0x65,0x4f,  0x15,0x01,0x00},//phase 4
        {0x00,0x06,0x2c,  0x62,0x52,  0x18,0x02,0x00},//phase 5
        {0x00,0x05,0x28,  0x60,0x56,  0x1b,0x02,0x00},//phase 6
        {0x00,0x04,0x24,  0x5e,0x59,  0x1e,0x03,0x00} //phase 7
    },
    {//filter 1 nuttall126 fc 0.0117 1080i
        {0x01,0x10,0x3f,  0x60,0x57,  0x24,0x05,0x00},//phase 0
        {0x01,0x0e,0x3b,  0x61,0x42,  0x12,0x01,0x00},//phase 1
        {0x01,0x0d,0x38,  0x5f,0x45,  0x14,0x02,0x00},//phase 2
        {0x01,0x0b,0x35,  0x5d,0x49,  0x17,0x02,0x00},//phase 3
        {0x00,0x0a,0x31,  0x5d,0x4c,  0x19,0x03,0x00},//phase 4
        {0x00,0x08,0x2e,  0x5c,0x4f,  0x1c,0x03,0x00},//phase 5
        {0x00,0x07,0x2b,  0x5b,0x51,  0x1e,0x04,0x00},//phase 6
        {0x00,0x06,0x27,  0x59,0x54,  0x21,0x05,0x00} //phase 7
    },
    {//filter 2 nuttall126 fc 0.0228 1080p
        {0x00,0x08,0x3f,  0x72,0x63,  0x1c,0x01,0x00},//phase 0
        {0x00,0x07,0x3a,  0x71,0x44,  0x0a,0x00,0x00},//phase 1
        {0x00,0x05,0x36,  0x70,0x49,  0x0c,0x00,0x00},//phase 2
        {0x00,0x04,0x31,  0x70,0x4d,  0x0e,0x00,0x00},//phase 3
        {0x00,0x03,0x2c,  0x6f,0x52,  0x10,0x00,0x00},//phase 4
        {0x00,0x02,0x28,  0x6c,0x57,  0x13,0x00,0x00},//phase 5
        {0x00,0x02,0x24,  0x69,0x5b,  0x16,0x00,0x00},//phase 6
        {0x00,0x01,0x20,  0x66,0x5f,  0x19,0x01,0x00} //phase 7
    },
    {
        //8 tap 8 phase odd
        {0xeb,0x00,0x52,  0x86,0x52,  0x00,0xeb,0x00},//phase 0
        {0xec,0xf9,0x48,  0x86,0x5e,  0x08,0xea,0xfd},//phase 1
        {0xef,0xf4,0x3d,  0x84,0x68,  0x11,0xe9,0xfa},//phase 2
        {0xf1,0xef,0x31,  0x81,0x72,  0x1b,0xea,0xf7},//phase 3
        {0xf4,0xec,0x26,  0x7a,0x7a,  0x26,0xec,0xf4},//phase 4
        {0xf7,0xea,0x1b,  0x72,0x81,  0x31,0xef,0xf1},//phase 5
        {0xfa,0xe9,0x11,  0x68,0x84,  0x3d,0xf4,0xef},//phase 6
        {0xfd,0xea,0x08,  0x5e,0x86,  0x48,0xf9,0xec} //phase 7
    }
};

unsigned int PMX_VFY_SCL_C_H_MSB_SCLDS[6][4] = {
    {//filter 0
        0x00,0x00,0x00,0x00//phase 10, 32, 54, 76
    },
    {//filter 1
        0x00,0x00,0x00,0x00//phase 10, 32, 54, 76
    },
    {//filter 2
        0x00,0x00,0x00,0x00//phase 10, 32, 54, 76
    },
    {
        0x00,0x00,0x00,0x00
    }
};

#define V_Y_TAP_NO 4
#define V_C_TAP_NO 4

unsigned int _u4PmxVerifySclVRegIdxTblY[PHASE_NO][V_Y_TAP_NO]= {
    {0x3140,  0x3140,0x3140,  0x3140},//phase 0
    {0x3144,  0x3144,0x3144,  0x3144},//phase 1
    {0x3148,  0x3148,0x3148,  0x3148},//phase 2
    {0x314c,  0x314c,0x314c,  0x314c},//phase 3
    {0x3150,  0x3150,0x3150,  0x3150},//phase 4
    {0x3154,  0x3154,0x3154,  0x3154},//phase 5
    {0x3158,  0x3158,0x3158,  0x3158},//phase 6
    {0x315c,  0x315c,0x315c,  0x315c} //phase 7
};

unsigned int _u4PmxVerifySclVRegBitLShiftTblY[PHASE_NO][V_Y_TAP_NO]= {
    {0,  8,  16,  24},//phase 0
    {0,  8,  16,  24},//phase 1
    {0,  8,  16,  24},//phase 2
    {0,  8,  16,  24},//phase 3
    {0,  8,  16,  24},//phase 4
    {0,  8,  16,  24},//phase 5
    {0,  8,  16,  24},//phase 6
    {0,  8,  16,  24} //phase 7
};

//vertical y filter coef [filter][phase][n] for scaler downscaling
unsigned int PMX_VFY_SCL_Y_V_COEF_SCLDS[6][PHASE_NO][V_Y_TAP_NO] = {
    {//filter 0 nuttall62 fc 0.0416 720p60
        //hamming_0416
        //4 tap 16 phase odd
        {0x26,  0xb4,0x80,  0x00},//phase 0
        {0x1f,  0xb4,0x30,  0xfd},//phase 1
        {0x17,  0xb2,0x3a,  0xfd},//phase 2
        {0x11,  0xad,0x45,  0xfd},//phase 3
        {0x0c,  0xa5,0x51,  0xfe},//phase 4
        {0x08,  0x9e,0x5c,  0xfe},//phase 5
        {0x04,  0x96,0x68,  0xfe},//phase 6
        {0x02,  0x8b,0x74,  0xff} //phase 7
    },
    {//filter 1 nuttall62 fc 0.0556 1080i60
        //hamming_0556
        //4 tap 16 phase odd
        {0x0e,  0xe4,0x89,  0xf7},//phase 0
        {0x06,  0xe4,0x19,  0xfd},//phase 1
        {0x00,  0xde,0x25,  0xfd},//phase 2
        {0xfb,  0xd6,0x33,  0xfc},//phase 3
        {0xf8,  0xcb,0x42,  0xfb},//phase 4
        {0xf7,  0xbc,0x53,  0xfa},//phase 5
        {0xf6,  0xad,0x64,  0xf9},//phase 6
        {0xf6,  0x9b,0x77,  0xf8} //phase 7
    },
    {//filter 2 nuttall62 fc 0.0278 1080p60
        //hamming_0278
        //4 tap 16 phase odd
        {0x36,  0x94,0x74,  0x0c},//phase 0
        {0x2e,  0x93,0x3d,  0x02},//phase 1
        {0x28,  0x91,0x45,  0x02},//phase 2
        {0x22,  0x8e,0x4d,  0x03},//phase 3
        {0x1c,  0x8b,0x55,  0x04},//phase 4
        {0x17,  0x87,0x5d,  0x05},//phase 5
        {0x13,  0x81,0x65,  0x07},//phase 6
        {0x0f,  0x7b,0x6d,  0x09} //phase 7
    },
    {//filter 3 nuttall62 fc 0.0500 720p50
        //hamming_05
        //4 tap 16 phase odd
        {0x19,  0xce,0x86,  0xfa},//phase 0
        {0x11,  0xcf,0x23,  0xfd},//phase 1
        {0x0a,  0xcb,0x2f,  0xfc},//phase 2
        {0x04,  0xc4,0x3c,  0xfc},//phase 3
        {0x00,  0xbc,0x49,  0xfb},//phase 4
        {0xfd,  0xb0,0x58,  0xfb},//phase 5
        {0xfb,  0xa4,0x67,  0xfa},//phase 6
        {0xfa,  0x95,0x77,  0xfa} //phase 7
    },
    {//filter 4 nuttall62 fc 0.0625 1080i50
        //hamming_0625
        //4 tap 16 phase odd
        {0x00,  0x00,0x8a,  0xf6},//phase 0
        {0xf9,  0xfe,0x0a,  0xff},//phase 1
        {0xf4,  0xf7,0x16,  0xff},//phase 2
        {0xf2,  0xec,0x24,  0xfe},//phase 3
        {0xf0,  0xdd,0x36,  0xfd},//phase 4
        {0xf1,  0xcb,0x49,  0xfb},//phase 5
        {0xf2,  0xb6,0x5e,  0xfa},//phase 6
        {0xf4,  0xa0,0x74,  0xf8} //phase 7
    },
    {//filter 5 nuttall62 fc 0.0333 1080p50
        //hamming_0333
        //4 tap 16 phase odd
        {0x31,  0x9e,0x79,  0x07},//phase 0
        {0x29,  0x9e,0x39,  0x00},//phase 1
        {0x22,  0x9c,0x42,  0x00},//phase 2
        {0x1c,  0x98,0x4b,  0x01},//phase 3
        {0x16,  0x95,0x54,  0x01},//phase 4
        {0x11,  0x8f,0x5e,  0x02},//phase 5
        {0x0d,  0x89,0x67,  0x03},//phase 6
        {0x0a,  0x81,0x70,  0x05} //phase 7
    }
};

unsigned int PMX_VFY_SCL_Y_V_MSB_SCLDS[6][4] = {
    {//filter 0
        0x00,0x00,0x00,0x00//phase 10, 32, 54, 76
    },
    {//filter 1
        0x00,0x00,0x00,0x00//phase 10, 32, 54, 76
    },
    {//filter 2
        0x00,0x00,0x00,0x00//phase 10, 32, 54, 76
    },
    {//filter 3
        0x00,0x00,0x00,0x00//phase 10, 32, 54, 76
    },
    {//filter 4
        0x01,0x00,0x00,0x00//phase 10, 32, 54, 76
    },
    {//filter 5
        0x00,0x00,0x00,0x00//phase 10, 32, 54, 76
    }
};


unsigned int _u4PmxVerifySclVRegIdxTblC[PHASE_NO][V_C_TAP_NO]= {
    {0x3160,  0x3160,0x3160,  0x3160},//phase 0
    {0x3164,  0x3164,0x3164,  0x3164},//phase 1
    {0x3168,  0x3168,0x3168,  0x3168},//phase 2
    {0x316c,  0x316c,0x316c,  0x316c},//phase 3
    {0x3170,  0x3170,0x3170,  0x3170},//phase 4
    {0x3174,  0x3174,0x3174,  0x3174},//phase 5
    {0x3178,  0x3178,0x3178,  0x3178},//phase 6
    {0x317c,  0x317c,0x317c,  0x317c} //phase 7
};

unsigned int _u4PmxVerifySclVRegBitLShiftTblC[PHASE_NO][V_C_TAP_NO]= {
    {0,  8,  16,  24},//phase 0
    {0,  8,  16,  24},//phase 1
    {0,  8,  16,  24},//phase 2
    {0,  8,  16,  24},//phase 3
    {0,  8,  16,  24},//phase 4
    {0,  8,  16,  24},//phase 5
    {0,  8,  16,  24},//phase 6
    {0,  8,  16,  24} //phase 7
};

//vertical c filter coef [filter][phase][n]
unsigned int PMX_VFY_SCL_C_V_COEF_SCLDS[6][PHASE_NO][V_C_TAP_NO] = {
    {//filter 0 nuttall62 fc 0.0416 720p60
        //hamming_0416
        //4 tap 16 phase odd
        {0x26,  0xb4,0x80,  0x00},//phase 0
        {0x1f,  0xb4,0x30,  0xfd},//phase 1
        {0x17,  0xb2,0x3a,  0xfd},//phase 2
        {0x11,  0xad,0x45,  0xfd},//phase 3
        {0x0c,  0xa5,0x51,  0xfe},//phase 4
        {0x08,  0x9e,0x5c,  0xfe},//phase 5
        {0x04,  0x96,0x68,  0xfe},//phase 6
        {0x02,  0x8b,0x74,  0xff} //phase 7
    },
    {//filter 1 nuttall62 fc 0.0556 1080i60
        //hamming_0278
        //4 tap 16 phase odd
        //hamming_0556
        //4 tap 16 phase odd
        {0x0e,  0xe4,0x89,  0xf7},//phase 0
        {0x06,  0xe4,0x19,  0xfd},//phase 1
        {0x00,  0xde,0x25,  0xfd},//phase 2
        {0xfb,  0xd6,0x33,  0xfc},//phase 3
        {0xf8,  0xcb,0x42,  0xfb},//phase 4
        {0xf7,  0xbc,0x53,  0xfa},//phase 5
        {0xf6,  0xad,0x64,  0xf9},//phase 6
        {0xf6,  0x9b,0x77,  0xf8} //phase 7
    },
    {//filter 2 nuttall62 fc 0.0278 1080p60
        //hamming_0278
        //4 tap 16 phase odd
        {0x36,  0x94,0x74,  0x0c},//phase 0
        {0x2e,  0x93,0x3d,  0x02},//phase 1
        {0x28,  0x91,0x45,  0x02},//phase 2
        {0x22,  0x8e,0x4d,  0x03},//phase 3
        {0x1c,  0x8b,0x55,  0x04},//phase 4
        {0x17,  0x87,0x5d,  0x05},//phase 5
        {0x13,  0x81,0x65,  0x07},//phase 6
        {0x0f,  0x7b,0x6d,  0x09} //phase 7
    },
    {//filter 3 nuttall62 fc 0.0500 720p50
        //hamming_05
        //4 tap 16 phase odd
        {0x19,  0xce,0x86,  0xfa},//phase 0
        {0x11,  0xcf,0x23,  0xfd},//phase 1
        {0x0a,  0xcb,0x2f,  0xfc},//phase 2
        {0x04,  0xc4,0x3c,  0xfc},//phase 3
        {0x00,  0xbc,0x49,  0xfb},//phase 4
        {0xfd,  0xb0,0x58,  0xfb},//phase 5
        {0xfb,  0xa4,0x67,  0xfa},//phase 6
        {0xfa,  0x95,0x77,  0xfa} //phase 7
    },
    {//filter 4 nuttall62 fc 0.0625 1080i50
        //hamming_0625
        //4 tap 16 phase odd
        {0x00,  0x00,0x8a,  0xf6},//phase 0
        {0xf9,  0xfe,0x0a,  0xff},//phase 1
        {0xf4,  0xf7,0x16,  0xff},//phase 2
        {0xf2,  0xec,0x24,  0xfe},//phase 3
        {0xf0,  0xdd,0x36,  0xfd},//phase 4
        {0xf1,  0xcb,0x49,  0xfb},//phase 5
        {0xf2,  0xb6,0x5e,  0xfa},//phase 6
        {0xf4,  0xa0,0x74,  0xf8} //phase 7
    },
    {//filter 5 nuttall62 fc 0.0333 1080p50
        //hamming_0333
        //4 tap 16 phase odd
        {0x31,  0x9e,0x79,  0x07},//phase 0
        {0x29,  0x9e,0x39,  0x00},//phase 1
        {0x22,  0x9c,0x42,  0x00},//phase 2
        {0x1c,  0x98,0x4b,  0x01},//phase 3
        {0x16,  0x95,0x54,  0x01},//phase 4
        {0x11,  0x8f,0x5e,  0x02},//phase 5
        {0x0d,  0x89,0x67,  0x03},//phase 6
        {0x0a,  0x81,0x70,  0x05} //phase 7
    }
};

unsigned int PMX_VFY_SCL_C_V_MSB_SCLDS[6][4] = {
    {//filter 0
        0x00,0x00,0x00,0x00//phase 10, 32, 54, 76
    },
    {//filter 1
        0x00,0x00,0x00,0x00//phase 10, 32, 54, 76
    },
    {//filter 2
        0x00,0x00,0x00,0x00//phase 10, 32, 54, 76
    },
    {//filter 3
        0x00,0x00,0x00,0x00//phase 10, 32, 54, 76
    },
    {//filter 4
        0x01,0x00,0x00,0x00//phase 10, 32, 54, 76
    },
    {//filter 5
        0x00,0x00,0x00,0x00//phase 10, 32, 54, 76
    }
};

void vPmxVerifyHalSclFilter(UCHAR ucOn, unsigned int u4CoefIdx)
{
    unsigned int u4PhaseIdx, u4TapIdx;
    unsigned int u4Reg, u4IdxReg, u4BitLShift;

    if(!ucOn)
    {
        u4Reg = ReadREG(0x32ec)&(~0x00000006);
        u4Reg |= 0x00000006;
        WriteREG(0x32ec, u4Reg);
        u4Reg = ReadREG(0x311c)&(~0x00010030); //disable y FIR and set to 8 tap
        u4Reg |= 0x00000030;
        WriteREG(0x311c, u4Reg);
        return;
    }

    //load down scaling H Y filter coeffficients
    for(u4PhaseIdx=0; u4PhaseIdx<PHASE_NO; u4PhaseIdx++)
    {
        for(u4TapIdx=0; u4TapIdx<H_Y_TAP_NO; u4TapIdx++)
        {
            u4IdxReg = _u4PmxVerifySclHRegIdxTblY[u4PhaseIdx][u4TapIdx];
            u4BitLShift = _u4PmxVerifySclHRegBitLShiftTblY[u4PhaseIdx][u4TapIdx];
            u4Reg = ReadREG(u4IdxReg)&(~(0xff<<u4BitLShift));
            u4Reg |= ((PMX_VFY_SCL_Y_H_COEF_SCLDS[u4CoefIdx][u4PhaseIdx][u4TapIdx])<<u4BitLShift);
            WriteREG(u4IdxReg, u4Reg);
        }
    }
    WriteREG(0x31fc, (PMX_VFY_SCL_Y_H_MSB_SCLDS[u4CoefIdx][3] << 24) | (PMX_VFY_SCL_Y_H_MSB_SCLDS[u4CoefIdx][2] << 16) | (PMX_VFY_SCL_Y_H_MSB_SCLDS[u4CoefIdx][1] << 8) | (PMX_VFY_SCL_Y_H_MSB_SCLDS[u4CoefIdx][0]));

    //load down scaling H C filter coeffficients
    for(u4PhaseIdx=0; u4PhaseIdx<PHASE_NO; u4PhaseIdx++)
    {
        for(u4TapIdx=0; u4TapIdx<H_C_TAP_NO; u4TapIdx++)
        {
            u4IdxReg = _u4PmxVerifySclHRegIdxTblC[u4PhaseIdx][u4TapIdx];
            u4BitLShift = _u4PmxVerifySclHRegBitLShiftTblC[u4PhaseIdx][u4TapIdx];
            u4Reg = ReadREG(u4IdxReg)&(~(0xff<<u4BitLShift));
            u4Reg |= ((PMX_VFY_SCL_C_H_COEF_SCLDS[u4CoefIdx][u4PhaseIdx][u4TapIdx])<<u4BitLShift);
            WriteREG(u4IdxReg, u4Reg);
        }
    }
    WriteREG(0x324c, (PMX_VFY_SCL_C_H_MSB_SCLDS[u4CoefIdx][3] << 24) | (PMX_VFY_SCL_C_H_MSB_SCLDS[u4CoefIdx][2] << 16) | (PMX_VFY_SCL_C_H_MSB_SCLDS[u4CoefIdx][1] << 8) | (PMX_VFY_SCL_C_H_MSB_SCLDS[u4CoefIdx][0]));

    //load down scaling V Y filter coeffficients
    for(u4PhaseIdx=0; u4PhaseIdx<PHASE_NO; u4PhaseIdx++)
    {
        for(u4TapIdx=0; u4TapIdx<V_Y_TAP_NO; u4TapIdx++)
        {
            u4IdxReg = _u4PmxVerifySclVRegIdxTblY[u4PhaseIdx][u4TapIdx];
            u4BitLShift = _u4PmxVerifySclVRegBitLShiftTblY[u4PhaseIdx][u4TapIdx];
            u4Reg = ReadREG(u4IdxReg)&(~(0xff<<u4BitLShift));
            u4Reg |= ((PMX_VFY_SCL_Y_V_COEF_SCLDS[u4CoefIdx][u4PhaseIdx][u4TapIdx])<<u4BitLShift);
            WriteREG(u4IdxReg, u4Reg);
        }
    }
    WriteREG(0x3180, (PMX_VFY_SCL_Y_V_MSB_SCLDS[u4CoefIdx][3] << 24) | (PMX_VFY_SCL_Y_V_MSB_SCLDS[u4CoefIdx][2] << 16) | (PMX_VFY_SCL_Y_V_MSB_SCLDS[u4CoefIdx][1] << 8) | (PMX_VFY_SCL_Y_V_MSB_SCLDS[u4CoefIdx][0]));

    //load down scaling V C filter coeffficients
    for(u4PhaseIdx=0; u4PhaseIdx<PHASE_NO; u4PhaseIdx++)
    {
        for(u4TapIdx=0; u4TapIdx<V_C_TAP_NO; u4TapIdx++)
        {
            u4IdxReg = _u4PmxVerifySclVRegIdxTblC[u4PhaseIdx][u4TapIdx];
            u4BitLShift = _u4PmxVerifySclVRegBitLShiftTblC[u4PhaseIdx][u4TapIdx];
            u4Reg = ReadREG(u4IdxReg)&(~(0xff<<u4BitLShift));
            u4Reg |= ((PMX_VFY_SCL_C_V_COEF_SCLDS[u4CoefIdx][u4PhaseIdx][u4TapIdx])<<u4BitLShift);
            WriteREG(u4IdxReg, u4Reg);
        }
    }
    WriteREG(0x3184, (PMX_VFY_SCL_C_V_MSB_SCLDS[u4CoefIdx][3] << 24) | (PMX_VFY_SCL_C_V_MSB_SCLDS[u4CoefIdx][2] << 16) | (PMX_VFY_SCL_C_V_MSB_SCLDS[u4CoefIdx][1] << 8) | (PMX_VFY_SCL_C_V_MSB_SCLDS[u4CoefIdx][0]));

    if(ucOn)
    {
        u4Reg = ReadREG(0x32ec)&(~0x00000006);
        u4Reg |= 0x00000000;
        WriteREG(0x32ec, u4Reg);
        u4Reg = ReadREG(0x311c)&(~0x00010030);
        u4Reg |= 0x00010000; //enable y FIR and set to 16 tap
        WriteREG(0x311c, u4Reg);
        return;
    }

    return;
}

void vPmxVerifyHalDispFmtHFilter(UCHAR ucVdoId, UCHAR ucYC, UCHAR ucCoef)
{
    unsigned int u4FmtCRegBase = 0;
    unsigned int u4FmtYRegBase = 0;
    unsigned int u4i = 0, u4Size = 0;

    if(ucVdoId == 0)
    {
        u4FmtYRegBase = 0x42000;
        u4FmtCRegBase = 0x42d00;
    }
    else if(ucVdoId == 1)
    {
        u4FmtYRegBase = 0x43000;
        u4FmtCRegBase = 0x43d00;
    }
    else
    {
        u4FmtYRegBase = 0x42000;
        u4FmtCRegBase = 0x42d00;
    }
    //Load horiztonal filter coefficients
    //UTIL_Printf("[pmx] %s(%d,%d,%d): %s\n", __FUNCTION__, ucVdoId, ucYC, ucCoef, _ArVdoHCoefSetting[ucCoef].szArrayName);

    u4Size = _ArVdoHCoefSetting[ucCoef].size;

    if ((ucYC & (1<<0)) == 1)
    {
        for(u4i = 0; u4i < u4Size-1; u4i ++)
        {
            WriteREG((u4FmtYRegBase + (u4i*4)), _ArVdoHCoefSetting[ucCoef].pu4RegSetting[u4i]);
        }

        WriteREG((u4FmtYRegBase + 0x4C), _ArVdoHCoefSetting[ucCoef].pu4RegSetting[u4Size-1]);
    }
    if ((ucYC & (1<<1)) == (1<<1))
    {
        for(u4i = 0; u4i < u4Size-1; u4i ++)
        {
            WriteREG((u4FmtCRegBase + (u4i*4)), _ArVdoHCoefSetting[ucCoef].pu4RegSetting[u4i]);
        }

        WriteREG((u4FmtCRegBase + 0x4C), _ArVdoHCoefSetting[ucCoef].pu4RegSetting[u4Size-1]);
    }

    WriteREGMsk((u4FmtYRegBase + 0xb0), 0, 1<<1);
    WriteREGMsk((u4FmtYRegBase + 0xe4), 1<<8, 1<<8);
}

void vPmxVerifyHalDispFmtPhaseSel2(UCHAR ucVdoId, UCHAR ucYPhase, UCHAR ucCPhase,bool bYEven,bool bCEven){
    unsigned int u4DispFmtRegBase = 0;

    UTIL_Printf("[pmx] %s(%d, %d, %d)\n", __FUNCTION__, ucVdoId, ucYPhase, ucCPhase);

    if(ucVdoId == 0)
    {
        u4DispFmtRegBase = 0x42000;
    }
    else if(ucVdoId == 1)
    {
        u4DispFmtRegBase = 0x43000;
    }
    else
    {
        u4DispFmtRegBase = 0x42000;
    }

    if ((32 == ucYPhase) && (32 == ucCPhase))
    {
        WriteREGMsk((u4DispFmtRegBase + 0xe4), 1<<12, 1<<12);
    }
    else
    {
        if (16 == ucYPhase)
        {
            WriteREGMsk((u4DispFmtRegBase + 0xe4), 0, 1<<12);
            WriteREGMsk((u4DispFmtRegBase + 0xb0), 1<<31, 1<<31);
			if(bYEven==TRUE){
				WriteREGMsk((u4DispFmtRegBase + 0xb0), 1<<30, 1<<30);				
			}else{
				WriteREGMsk((u4DispFmtRegBase + 0xb0), 0<<30, 1<<30);				
			}
        }
        else if (8 == ucYPhase)
        { // 8 phase
            WriteREGMsk((u4DispFmtRegBase + 0xe4), 0, 1<<12);
            WriteREGMsk((u4DispFmtRegBase + 0xb0), 0, 1<<31);
        }
        if (16 == ucCPhase)
        {
            WriteREGMsk((u4DispFmtRegBase + 0xe4), 0, 1<<12);
            WriteREGMsk((u4DispFmtRegBase + 0x48), 1<<10, 1<<10);
			if(bCEven==TRUE){
				WriteREGMsk((u4DispFmtRegBase + 0x48), 1<<11, 1<<11);				
			}else{
				WriteREGMsk((u4DispFmtRegBase + 0x48), 0<<11, 1<<11);				
			}

        }
        else if (8 == ucCPhase)
        { // 8 phase
            WriteREGMsk((u4DispFmtRegBase + 0xe4), 0, 1<<12);
            WriteREGMsk((u4DispFmtRegBase + 0x48), 0, 1<<10);
        }
    }


}

void vPmxVerifyHalDispFmtPhaseSel(UCHAR ucVdoId, UCHAR ucYPhase, UCHAR ucCPhase)
{
    unsigned int u4DispFmtRegBase = 0;

    UTIL_Printf("[pmx] %s(%d, %d, %d)\n", __FUNCTION__, ucVdoId, ucYPhase, ucCPhase);

    if(ucVdoId == 0)
    {
        u4DispFmtRegBase = 0x42000;
    }
    else if(ucVdoId == 1)
    {
        u4DispFmtRegBase = 0x43000;
    }
    else
    {
        u4DispFmtRegBase = 0x42000;
    }

    if ((32 == ucYPhase) && (32 == ucCPhase))
    {
        WriteREGMsk((u4DispFmtRegBase + 0xe4), 1<<12, 1<<12);
    }
    else
    {
        if (16 == ucYPhase)
        {
            WriteREGMsk((u4DispFmtRegBase + 0xe4), 0, 1<<12);
            WriteREGMsk((u4DispFmtRegBase + 0xb0), 1<<31, 1<<31);
        }
        else if (8 == ucYPhase)
        { // 8 phase
            WriteREGMsk((u4DispFmtRegBase + 0xe4), 0, 1<<12);
            WriteREGMsk((u4DispFmtRegBase + 0xb0), 0, 1<<31);
        }

        if (16 == ucCPhase)
        {
            WriteREGMsk((u4DispFmtRegBase + 0xe4), 0, 1<<12);
            WriteREGMsk((u4DispFmtRegBase + 0x48), 1<<10, 1<<10);
        }
        else if (8 == ucCPhase)
        { // 8 phase
            WriteREGMsk((u4DispFmtRegBase + 0xe4), 0, 1<<12);
            WriteREGMsk((u4DispFmtRegBase + 0x48), 0, 1<<10);
        }
    }
}

void vPmxVerifyHalDispFmtSDModeSel(UCHAR ucVdoId, UCHAR ucSDMode)
{
    // Update register in Vsync.
    vPmxHalSetSDMode(ucVdoId, ucSDMode);
}
extern bool EnPmxRearNewSD144Mode;

void vPmxVerifyHalMvdoVFilter(UCHAR ucVdoId, UCHAR ucV4Tap, UCHAR ucCoef)
{
    unsigned int u4V8RegBase = 0;
    unsigned int u4i = 0, u4Size = 0;

    if(ucVdoId == 0)
    {
        u4V8RegBase = 0x42200;
    }
    else if(ucVdoId == 1)
    {
        u4V8RegBase = 0x43200;
    }
    else
    {
        u4V8RegBase = 0x42200;
    }

    //UTIL_Printf("[pmx] %s(%d,%d,%d): %s\n", __FUNCTION__, ucVdoId, ucV4Tap, ucCoef, _ArVdoV4CoefSetting[ucCoef].szArrayName);

    if (ucV4Tap)
    {
        u4Size = _ArVdoV4CoefSetting[ucCoef].size;
        for(u4i = 0; u4i < (u4Size - 1); u4i ++)
        {
            WriteREG((u4V8RegBase + (u4i*4)), _ArVdoV4CoefSetting[ucCoef].pu4RegSetting[u4i]);
            WriteREG((u4V8RegBase + (u4i*4) + 0x50), _ArVdoV4CoefSetting[ucCoef].pu4RegSetting[u4i]);
        }
        WriteREG((u4V8RegBase + 0x40), _ArVdoV4CoefSetting[ucCoef].pu4RegSetting[u4Size - 1]);
        WriteREG((u4V8RegBase + 0x90), _ArVdoV4CoefSetting[ucCoef].pu4RegSetting[u4Size - 1]);
    }
    else
    {
        u4Size = _ArVdoV8CoefSetting[ucCoef].size;
        for(u4i = 0; u4i < u4Size; u4i ++)
        {
            WriteREG((u4V8RegBase + (u4i*4)), _ArVdoV8CoefSetting[ucCoef].pu4RegSetting[u4i]);
            WriteREG((u4V8RegBase + (u4i*4) + 0x50), _ArVdoV8CoefSetting[ucCoef].pu4RegSetting[u4i]);
        }
    }
}

void vPmxVerifyHalVdoFit2(unsigned int u4In, unsigned int u4Out, UCHAR ucVdoId, UCHAR ucSrcType, UCHAR ucTvType, unsigned int ucScanline)
{
	unsigned int u4HFactor = 0, u4VFactor = 0;
	unsigned int u4HFactor_inverse = 0;
	unsigned int u4VdoRegBase = 0, u4FmtRegBase = 0;
	unsigned int adj_hact_vdo=0;
	unsigned int adj_hact_hd=0;
	

	switch(u4Out)
	{
	case 480:
		if(ucTvType ==  PMX_TV_TYPE_800X480)
		{
			if((u4In == 480)&&(ucSrcType == PMX_VDO_800X480))
			{
				u4HFactor = (unsigned int)(4096);
				u4VFactor = (unsigned int)(2048);

			}else if(u4In == 480){
				u4HFactor = (unsigned int)(720*4096/800);
				u4VFactor = (unsigned int)(480*2048/480);
			}
			if((u4In == 600)&&(ucSrcType == PMX_VDO_1024X600))
			{
				u4HFactor = (unsigned int)(1024*4096/800);
				u4VFactor = (unsigned int)(600*2048/480);
			
			}else if(u4In == 600){
				u4HFactor = (unsigned int)(800*4096/800);
				u4VFactor = (unsigned int)(600*2048/480);
			}
			else if(u4In == 576)
			{
				u4HFactor = (unsigned int)(720 * 4096 / 800);
				u4VFactor = (unsigned int)(576 * 2048 /480);
			}
			else if(u4In == 1080)
			{
				u4HFactor = (unsigned int)(1920 * 4096 / 800);
				u4VFactor = (unsigned int)(1080 * 2048 / 480);
			}
			else if(u4In == 720)
			{
				u4HFactor = (unsigned int)(1280 * 4096 /800);
				u4VFactor = (unsigned int)(720 * 2048 /480);
			}
		}else{ //480p		
			if((u4In == 480)&&(ucSrcType == PMX_VDO_800X480))
			{
				u4HFactor = (unsigned int)(800*4096/720);
				u4VFactor = (unsigned int)(480*2048/480);
			
			}else if(u4In == 480){
				u4HFactor = (unsigned int)(720*4096/720);
				u4VFactor = (unsigned int)(480*2048/480);
			}
			if((u4In == 600)&&(ucSrcType == PMX_VDO_1024X600))
			{
				u4HFactor = (unsigned int)(1024*4096/720);
				u4VFactor = (unsigned int)(600*2048/480);
			
			}else if(u4In == 600){
				u4HFactor = (unsigned int)(800*4096/720);
				u4VFactor = (unsigned int)(600*2048/480);
			}
			else if(u4In == 576)
			{
				u4HFactor = (unsigned int)(720 * 4096 / 720);
				u4VFactor = (unsigned int)(576 * 2048 /480);
			}
			else if(u4In == 1080)
			{
				u4HFactor = (unsigned int)(1920 * 4096 / 720);
				u4VFactor = (unsigned int)(1080 * 2048 / 480);
			}
			else if(u4In == 720)
			{
				u4HFactor = (unsigned int)(1280 * 4096 /720);
				u4VFactor = (unsigned int)(720 * 2048 /480);
			}
		}		
		break;	
	case 600:
		if(ucTvType ==  PMX_TV_TYPE_1024X600)
		{
			if((u4In == 480)&&(ucSrcType == PMX_VDO_800X480))
			{
				u4HFactor = (unsigned int)(800*4096/1024);
				u4VFactor = (unsigned int)(480*2048/600);

			}else if(u4In == 480){
				u4HFactor = (unsigned int)(720*4096/1024);
				u4VFactor = (unsigned int)(480*2048/600);
			}
			if((u4In == 600)&&(ucSrcType == PMX_VDO_1024X600))
			{
				u4HFactor = (unsigned int)(1024*4096/1024);
				u4VFactor = (unsigned int)(600*2048/600);
			
			}else if(u4In == 600){
				u4HFactor = (unsigned int)(800*4096/1024);
				u4VFactor = (unsigned int)(600*2048/600);
			}
			else if(u4In == 576)
			{
				u4HFactor = (unsigned int)(720 * 4096 / 1024);
				u4VFactor = (unsigned int)(576 * 2048 /600);
			}
			else if(u4In == 1080)
			{
				u4HFactor = (unsigned int)(1920 * 4096 / 1024);
				u4VFactor = (unsigned int)(1080 * 2048 / 600);
			}
			else if(u4In == 720)
			{
				u4HFactor = (unsigned int)(1280 * 4096 /1024);
				u4VFactor = (unsigned int)(720 * 2048 /600);
			}
		}else{ //800x600		
			if((u4In == 480)&&(ucSrcType == PMX_VDO_800X480))
			{
				u4HFactor = (unsigned int)(800*4096/800);
				u4VFactor = (unsigned int)(480*2048/600);
			
			}else if(u4In == 480){
				u4HFactor = (unsigned int)(720*4096/800);
				u4VFactor = (unsigned int)(480*2048/600);
			}
			if((u4In == 600)&&(ucSrcType == PMX_VDO_1024X600))
			{
				u4HFactor = (unsigned int)(1024*4096/800);
				u4VFactor = (unsigned int)(600*2048/600);
			
			}else if(u4In == 600){
				u4HFactor = (unsigned int)(800*4096/800);
				u4VFactor = (unsigned int)(600*2048/600);
			}
			else if(u4In == 576)
			{
				u4HFactor = (unsigned int)(720 * 4096 / 800);
				u4VFactor = (unsigned int)(576 * 2048 /600);
			}
			else if(u4In == 1080)
			{
				u4HFactor = (unsigned int)(1920 * 4096 / 800);
				u4VFactor = (unsigned int)(1080 * 2048 / 600);
			}
			else if(u4In == 720)
			{
				u4HFactor = (unsigned int)(1280 * 4096 /800);
				u4VFactor = (unsigned int)(720 * 2048 /600);
			}
		}		
		break;	
	case 576:
		if((u4In == 480)&&(ucSrcType == PMX_VDO_800X480))
		{
			u4HFactor = (unsigned int)(800*4096/720);
			u4VFactor = (unsigned int)(480*2048/576);
		
		}else if(u4In == 480){
			u4HFactor = (unsigned int)(720*4096/720);
			u4VFactor = (unsigned int)(480*2048/576);
		}
		if((u4In == 600)&&(ucSrcType == PMX_VDO_1024X600))
		{
			u4HFactor = (unsigned int)(1024*4096/720);
			u4VFactor = (unsigned int)(600*2048/576);
		
		}else if(u4In == 600){
			u4HFactor = (unsigned int)(800*4096/720);
			u4VFactor = (unsigned int)(600*2048/576);
		}
		else if(u4In == 576)
		{
			u4HFactor = (unsigned int)4096;
			u4VFactor = (unsigned int)2048;
		}
		else if(u4In == 1080)
		{
			u4HFactor = (unsigned int)(1920 * 4096 /720);
			u4VFactor = (unsigned int)(1080 * 2048 /576);
		}
		else if(u4In == 720)
		{
			u4HFactor = (unsigned int)(1280 * 4096 /720);
			u4VFactor = (unsigned int)(720 * 2048 /576);
		}
		break;
	case 720:
		if((u4In == 480)&&(ucSrcType == PMX_VDO_800X480))
		{
			u4HFactor = (unsigned int)(800*4096/1280);
			u4VFactor = (unsigned int)(480*2048/720);
		
		}else if(u4In == 480){
			u4HFactor = (unsigned int)(720*4096/1280);
			u4VFactor = (unsigned int)(480*2048/720);
		}
		if((u4In == 600)&&(ucSrcType == PMX_VDO_1024X600))
		{
			u4HFactor = (unsigned int)(1024*4096/1280);
			u4VFactor = (unsigned int)(600*2048/720);
		
		}else if(u4In == 600){
			u4HFactor = (unsigned int)(800*4096/1280);
			u4VFactor = (unsigned int)(600*2048/720);
		}
		else if(u4In == 576)
		{
			u4HFactor = (unsigned int)(720 * 4096 /1280);
			u4VFactor = (unsigned int)(576 * 2048 /720);
		}
		else if(u4In == 1080)
		{
			u4HFactor = (unsigned int)(1920 * 4096 /1280);
			u4VFactor = (unsigned int)(1080 * 2048 /720);
		}
		else if(u4In == 720)
		{
			u4HFactor = (unsigned int)4096;
			u4VFactor = (unsigned int)2048;
		}
		break;
	case 1080:
		if((u4In == 480)&&(ucSrcType == PMX_VDO_800X480))
		{
			u4HFactor = (unsigned int)(800*4096/1920);
			u4VFactor = (unsigned int)(480*2048/1080);
		
		}else if(u4In == 480){
			u4HFactor = (unsigned int)(720*4096/1920);
			u4VFactor = (unsigned int)(480*2048/1080);
		}
		if((u4In == 600)&&(ucSrcType == PMX_VDO_1024X600))
		{
			u4HFactor = (unsigned int)(1024*4096/1920);
			u4VFactor = (unsigned int)(600*2048/1080);
		
		}else if(u4In == 600){
			u4HFactor = (unsigned int)(800*4096/1920);
			u4VFactor = (unsigned int)(600*2048/1080);
		}
		else if(u4In == 576)
		{
			u4HFactor = (unsigned int)(720 * 4096 /1920);
			u4VFactor = (unsigned int)(576 * 2048 /1080);
		}
		else if(u4In == 720)
		{
			u4HFactor = (unsigned int)(1280 * 4096 /1920);
			u4VFactor = (unsigned int)(720 * 2048 /1080);
		}
		else if(u4In == 1080)
		{
			u4HFactor = (unsigned int)4096;
			u4VFactor = (unsigned int)2048;
		}
		break;
	case 2160:
		if(u4In == 480)
		{
			u4HFactor = (unsigned int)(720*4096/3840);
			u4VFactor = (unsigned int)(480*2048/2160);
		}
		else if(u4In == 720)
		{
			u4HFactor = (unsigned int)(1280*4096/3840);
			u4VFactor = (unsigned int)(720*2048/2160);
		}
		else if(u4In == 1080)
		{

			u4HFactor = (unsigned int)(1920*4096/3840); //3840x2160
			u4VFactor = (unsigned int)(1080*2048/2160);
		}
		else if(u4In == 2160)
		{

			u4HFactor = (unsigned int)(3840*4096/3840); //3840x2160
			u4VFactor = (unsigned int)(2160*2048/2160);
		}
		else if(u4In == 2161)
		{
			u4HFactor = (unsigned int)(4096*4096/3840); //3840x2160
			u4VFactor = (unsigned int)(2160*2048/2160);
		}
		break;
	case 2161:
		if(u4In == 480)
		{
			u4HFactor = (unsigned int)(720*4096/4096);
			u4VFactor = (unsigned int)(480*2048/2160);
		}
		else if(u4In == 720)
		{
			u4HFactor = (unsigned int)(1280*4096/4096);
			u4VFactor = (unsigned int)(720*2048/2160);
		}
		else if(u4In == 1080)
		{
			u4HFactor = (unsigned int)(1920*4096/4096); //4096x2160
			u4VFactor = (unsigned int)(1080*2048/2160);
		}
		else if(u4In == 2160)
		{

			u4HFactor = (unsigned int)(3840*4096/4096); //4096x2160
			u4VFactor = (unsigned int)(2160*2048/2160);
		}
		else if(u4In == 2161)
		{

			u4HFactor = (unsigned int)(4096*4096/4096); //4096x2160
			u4VFactor = (unsigned int)(2160*2048/2160);
		}
		break;

	default:
		VDO_LOG(VDO_LOG_LVL_INFO, "[PmxVfy] default \n");
		break;
  }

  VDO_LOG(VDO_LOG_LVL_INFO, "[PmxVfy] u4HFactor=0x%x, u4VFactor=0x%x \n", u4HFactor, u4VFactor);

  if(ucVdoId == 0)
  {
	  u4VdoRegBase = 0x42400;
	  u4FmtRegBase = 0x42000;
  }
  else if (ucVdoId == 1)
  {
	  u4VdoRegBase = 0x43400;
	  u4FmtRegBase = 0x43000;
  }
  else
  {
	  u4VdoRegBase = 0x43A00;
	  u4FmtRegBase = 0x42000;
  }
   if((u4In == 600)&&(ucSrcType == PMX_VDO_1024X600))
  {
			WriteREG((u4VdoRegBase+0x10), 0x0258c880);
			WriteREGMsk((u4VdoRegBase+0xe0), 0x100, 0x1FF);
			WriteREG((u4FmtRegBase+0x9c), 0x00000400);

  }
  else if(u4In == 600) //800x600
  {
	  WriteREG((u4VdoRegBase+0x10), 0x0258c864);
	  WriteREGMsk((u4VdoRegBase+0xe0), 0xc8, 0x1FF);
	  WriteREG((u4FmtRegBase+0x9c), 0x00000320);
  }
  else if((u4In == 480)&&(ucSrcType == PMX_VDO_800X480))
  {
			WriteREG((u4VdoRegBase+0x10), 0x01e0c864);
			WriteREGMsk((u4VdoRegBase+0xe0), 0xc8, 0x1FF);
			WriteREG((u4FmtRegBase+0x9c), 0x00000320);

  }
  else if(u4In == 480) //720x480
  {
	  WriteREG((u4VdoRegBase+0x10), 0x01e0b45a);
	  WriteREGMsk((u4VdoRegBase+0xe0), 0xb4, 0x1FF);
	  WriteREG((u4FmtRegBase+0x9c), 0x000002d0);
  }
  else if(u4In == 576)
  {
	  WriteREG((u4VdoRegBase+0x10), 0x0240b45a);
	  WriteREGMsk((u4VdoRegBase+0xe0), 0xb4, 0x1FF);
	  WriteREG((u4FmtRegBase+0x9c), 0x000002d0);
  }
  else if(u4In == 720)
  {
	  WriteREG((u4VdoRegBase+0x10), 0x02d040a0);
	  WriteREGMsk((u4VdoRegBase+0xe0), 0x140, 0x1FF);
	  WriteREG((u4FmtRegBase+0x9c), 0x00000500);
  }
  else if(u4In == 1080)
  {
	  WriteREG((u4VdoRegBase+0x10), 0x0438b4f0);
	  WriteREGMsk((u4VdoRegBase+0xe0), 0x1E0, 0x1FF);
	  WriteREG((u4FmtRegBase+0x9c), 0x00000780);
  }
  else if(u4In == 2160)
  {
	  WriteREG((u4VdoRegBase+0x10), 0x1870b4e0);
	  WriteREGMsk((u4VdoRegBase+0xe0), 0x3c0, 0x1FF);
	  WriteREG((u4FmtRegBase+0x9c), 0x00000f00);
  }
  else if(u4In == 2161)
  {
	  WriteREG((u4VdoRegBase+0x10), 0x2870b400);
	  WriteREGMsk((u4VdoRegBase+0xe0), 0x400, 0x1FF);
	  WriteREG((u4FmtRegBase+0x9c), 0x00001000);
  }

  if(0 == ucVdoId){
		if(480 == u4Out){
			u4VFactor = u4VFactor / 2; //new sd162 mode.
		}
  }else{
  		if((EnPmxRearNewSD144Mode==TRUE) && (480 == u4Out)){
			u4VFactor = u4VFactor / 2; //new sd mode.
		}
		//fixme: if new sd mode. also need /2 operation.
  }
	
  if(0 == ucVdoId){
		if(u4HFactor > 0x1000){
			WriteREGMsk((u4FmtRegBase+0xB0), (0x1000<< 16), 0xFFFF0000);
			WriteREGMsk((u4FmtRegBase+0xB0),1, 1);
			WriteREGMsk((u4FmtRegBase+0x6c), (u4HFactor << 16)| 0x821, 0xFFFFFFFF);

			//use acc.
			u4HFactor_inverse = (u4HFactor*1000)/0x1000;
			u4HFactor_inverse = (0x1000 * 1000) / u4HFactor_inverse;
			WriteREGMsk((u4FmtRegBase+0xe4),1<<29, 1<<29);
			WriteREGMsk((u4FmtRegBase+0xcc),u4HFactor_inverse,0xffff);
			
		    adj_hact_vdo = (ReadREG(u4FmtRegBase+0xa0) >> 16) & 0xfff;
		    adj_hact_hd = (ReadREG(u4FmtRegBase+0x70) >> 16) & 0xfff;

			switch(u4In){
				case(720):
					adj_hact_vdo = 1280 +adj_hact_vdo +1;
					adj_hact_hd = 1280 + adj_hact_hd +1;					
					break;
				case(1080):
					adj_hact_vdo = 1920 + adj_hact_vdo +1;
					adj_hact_hd =  1920 + adj_hact_hd +1;					
					break;
				case(600):
					if(ucSrcType == PMX_VDO_1024X600){
						adj_hact_vdo = 1024 + adj_hact_vdo +1;
						adj_hact_hd =  1024 + adj_hact_hd +1;					
					}else{
						adj_hact_vdo = 800 + adj_hact_vdo +1;
						adj_hact_hd =  800 + adj_hact_hd +1;					
					}
					break;				
				default:
					adj_hact_vdo = 1920 + adj_hact_vdo +1;
					adj_hact_hd =  1920 + adj_hact_hd +1;					
					break;
			}
			WriteREGMsk((u4FmtRegBase+0xa0),adj_hact_vdo, 0x1fff);
			WriteREGMsk((u4FmtRegBase+0x70),adj_hact_hd, 0x1fff);

			/*tiny fix,may not need in realchip*/
			//for 1080->800x480p
			//if((ucVdoId==0)&&(u4In==1080)&&(u4Out==480))
				//WriteREGMsk((u4FmtRegBase+0x70),(adj_hact_vdo<<16)| (adj_hact_hd+2), 0xfff1fff);				
			
				
		}else{			
			WriteREGMsk((u4FmtRegBase+0xB0), (u4HFactor << 16), 0xFFFF0000);
			WriteREGMsk((u4VdoRegBase+0x1C), 0, (0x1 << 11));			
			WriteREGMsk((u4FmtRegBase+0xB0),1, 1);

			if(u4HFactor == 0x1000){ //fixme;
				//use acc.
				u4HFactor_inverse = (u4HFactor*1000)/0x1000;
				u4HFactor_inverse = (0x1000 * 1000) / u4HFactor_inverse;
				WriteREGMsk((u4FmtRegBase+0xe4),1<<29, 1<<29);
				WriteREGMsk((u4FmtRegBase+0xcc),u4HFactor_inverse,0xffff);
			}			

		}
  }else{
  	  if(EnPmxRearNewSD144Mode)
	  {
		if(u4HFactor > 0x1000){
			WriteREGMsk((u4FmtRegBase+0xB0), (0x1000<< 16), 0xFFFF0000);
			WriteREGMsk((u4FmtRegBase+0xB0),1, 1);
			WriteREGMsk((u4FmtRegBase+0x6c), (u4HFactor << 16)| 0x821, 0xFFFFFFFF);

			//use acc.
			u4HFactor_inverse = (u4HFactor*1000)/0x1000;
			u4HFactor_inverse = (0x1000 * 1000) / u4HFactor_inverse;
			WriteREGMsk((u4FmtRegBase+0xe4),1<<29, 1<<29);
			WriteREGMsk((u4FmtRegBase+0xcc),u4HFactor_inverse,0xffff);


		    adj_hact_vdo = (ReadREG(u4FmtRegBase+0xa0) >> 16) & 0xfff;
		    adj_hact_hd = (ReadREG(u4FmtRegBase+0x70) >> 16) & 0xfff;

			switch(u4In){
				case(720):
					adj_hact_vdo = 1280 +adj_hact_vdo +1;
					adj_hact_hd = 1280 + adj_hact_hd +1;					
					break;
				case(1080):
					adj_hact_vdo = 1920 + adj_hact_vdo +1;
					adj_hact_hd =  1920 + adj_hact_hd +1;					
					break;
				case(600):
					if(ucSrcType == PMX_VDO_1024X600){
						adj_hact_vdo = 1024 + adj_hact_vdo +1;
						adj_hact_hd =  1024 + adj_hact_hd +1;					
					}else{
						adj_hact_vdo = 800 + adj_hact_vdo +1;
						adj_hact_hd =  800 + adj_hact_hd +1;					
					}
					break;				
				default:
					adj_hact_vdo = 1920 + adj_hact_vdo +1;
					adj_hact_hd =  1920 + adj_hact_hd +1;					
					break;
			}
			WriteREGMsk((u4FmtRegBase+0xa0),adj_hact_vdo, 0x1fff);
			WriteREGMsk((u4FmtRegBase+0x70),adj_hact_hd, 0x1fff);
				
		}else{
			WriteREGMsk((u4FmtRegBase+0xB0), (u4HFactor << 16), 0xFFFF0000);
			WriteREGMsk((u4VdoRegBase+0x1C), 0, (0x1 << 11));			
			WriteREGMsk((u4FmtRegBase+0xB0),1, 1);
		}
  }else{
	  	   //fixme: if new sd mode. do hdown scaler if must.
		  if (u4HFactor > 0x2000 || ((u4HFactor == 0x2000) && g_fgNeedEnableXHalf))
		  {
			  WriteREGMsk((u4FmtRegBase+0xB0), (u4HFactor << 15), 0xFFFF0000);
			  WriteREGMsk((u4VdoRegBase+0x1C), (0x1 << 11), (0x1 << 11));
		  }
		  else
		  {
			  WriteREGMsk((u4FmtRegBase+0xB0), (u4HFactor << 16), 0xFFFF0000);
			  WriteREGMsk((u4VdoRegBase+0x1C), 0, (0x1 << 11));
		  }
		  WriteREGMsk((u4FmtRegBase+0xB0),1, 1);
	  }
  }

  	
  WriteREGMsk((u4VdoRegBase+0x14), u4VFactor, 0xFFFF);
  WriteREG((u4FmtRegBase + 0x48), 0x5);

  //[xzr]set yuv420,raster scan, tmp for demo version
  WriteREGMsk((u4VdoRegBase + 0x30), 0, 0x1 << 21 ); //30, bit 21, 0:420, 1:422
  WriteREGMsk((u4VdoRegBase + 0xe0), (ucScanline << 23),  0x1 << 23); //e0, bit 23, 0:block, 1:raster scan
}


void vPmxVerifyHalVdoFit(unsigned int u4In, unsigned int u4Out, UCHAR ucVdoId)
{
    unsigned int u4HFactor = 0, u4VFactor = 0;
    unsigned int u4VdoRegBase = 0, u4FmtRegBase = 0;

    switch(u4Out)
    {
	case 480800: //800x480 output.
		if(u4In == 480800)
		{
			u4HFactor = (unsigned int)(4096);
			u4VFactor = (unsigned int)(2048);
		}
		if(u4In == 480)
		{
			u4HFactor = (unsigned int)(720*4096/800);
			u4VFactor = (unsigned int)(480*2048/480);
		}
		else if(u4In == 576)
		{
			u4HFactor = (unsigned int)(720 * 4096 / 800);
			u4VFactor = (unsigned int)(576 * 2048 /480);
		}
		else if(u4In == 1080)
		{
			u4HFactor = (unsigned int)(1920 * 4096 / 800);
			u4VFactor = (unsigned int)(1080 * 2048 / 480);
		}
		else if(u4In == 720)
		{
			u4HFactor = (unsigned int)(1280 * 4096 /800);
			u4VFactor = (unsigned int)(720 * 2048 /480);
		}
		break;
	
    case 480:
		if(u4In == 480800)
        {
            u4HFactor = (unsigned int)(800 * 4096 / 720);
            u4VFactor = (unsigned int)(480 * 2048 /480);
        }
        if(u4In == 480)
        {
            u4HFactor = (unsigned int)4096;
            u4VFactor = (unsigned int)2048;
        }
        else if(u4In == 576)
        {
            u4HFactor = (unsigned int)(720 * 4096 / 720);
            u4VFactor = (unsigned int)(576 * 2048 /480);
        }
        else if(u4In == 1080)
        {
            u4HFactor = (unsigned int)(1920 * 4096 / 720);
            u4VFactor = (unsigned int)(1080 * 2048 / 480);
        }
        else if(u4In == 720)
        {
            u4HFactor = (unsigned int)(1280 * 4096 /720);
            u4VFactor = (unsigned int)(720 * 2048 /480);
        }
        break;
    case 576:		
		if(u4In == 480800)
        {
            u4HFactor = (unsigned int)(800 * 4096 / 720);
            u4VFactor = (unsigned int)(480 * 2048 /576);
        }
        if(u4In == 480)
        {
            u4HFactor = (unsigned int)(720 * 4096 /720);
            u4VFactor = (unsigned int)(480 * 2048 /576);
        }
        else if(u4In == 576)
        {
            u4HFactor = (unsigned int)4096;
            u4VFactor = (unsigned int)2048;
        }
        else if(u4In == 1080)
        {
            u4HFactor = (unsigned int)(1920 * 4096 /720);
            u4VFactor = (unsigned int)(1080 * 2048 /576);
        }
        else if(u4In == 720)
        {
            u4HFactor = (unsigned int)(1280 * 4096 /720);
            u4VFactor = (unsigned int)(720 * 2048 /576);
        }
        break;
    case 720:
		if(u4In == 480800)
        {
            u4HFactor = (unsigned int)(800 * 4096 / 1280);
            u4VFactor = (unsigned int)(480 * 2048 /720);
        }
        if(u4In == 480)
        {
            u4HFactor = (unsigned int)(720 * 4096 /1280);
            u4VFactor = (unsigned int)(480 * 2048 /720);
        }
        else if(u4In == 576)
        {
            u4HFactor = (unsigned int)(720 * 4096 /1280);
            u4VFactor = (unsigned int)(576 * 2048 /720);
        }
        else if(u4In == 1080)
        {
            u4HFactor = (unsigned int)(1920 * 4096 /1280);
            u4VFactor = (unsigned int)(1080 * 2048 /720);
        }
        else if(u4In == 720)
        {
            u4HFactor = (unsigned int)4096;
            u4VFactor = (unsigned int)2048;
        }
        break;
    case 1080:
		if(u4In == 480800)
        {
            u4HFactor = (unsigned int)(800 * 4096 / 1920);
            u4VFactor = (unsigned int)(480 * 2048 /1080);
        }
        if(u4In == 480)
        {
            u4HFactor = (unsigned int)(720 * 4096 /1920);
            u4VFactor = (unsigned int)(480 * 2048 /1080);
        }
        else if(u4In == 576)
        {
            u4HFactor = (unsigned int)(720 * 4096 /1920);
            u4VFactor = (unsigned int)(576 * 2048 /1080);
        }
        else if(u4In == 720)
        {
            u4HFactor = (unsigned int)(1280 * 4096 /1920);
            u4VFactor = (unsigned int)(720 * 2048 /1080);
        }
        else if(u4In == 1080)
        {
            u4HFactor = (unsigned int)4096;
            u4VFactor = (unsigned int)2048;
        }
        break;
    case 2160:
        if(u4In == 480)
        {
            u4HFactor = (unsigned int)(720*4096/3840);
            u4VFactor = (unsigned int)(480*2048/2160);
        }
        else if(u4In == 720)
        {
            u4HFactor = (unsigned int)(1280*4096/3840);
            u4VFactor = (unsigned int)(720*2048/2160);
        }
        else if(u4In == 1080)
        {

            u4HFactor = (unsigned int)(1920*4096/3840); //3840x2160
            u4VFactor = (unsigned int)(1080*2048/2160);
        }
        else if(u4In == 2160)
        {

            u4HFactor = (unsigned int)(3840*4096/3840); //3840x2160
            u4VFactor = (unsigned int)(2160*2048/2160);
        }
        else if(u4In == 2161)
        {

            u4HFactor = (unsigned int)(4096*4096/3840); //3840x2160
            u4VFactor = (unsigned int)(2160*2048/2160);
        }
        break;
    case 2161:
        if(u4In == 480)
        {
            u4HFactor = (unsigned int)(720*4096/4096);
            u4VFactor = (unsigned int)(480*2048/2160);
        }
        else if(u4In == 720)
        {
            u4HFactor = (unsigned int)(1280*4096/4096);
            u4VFactor = (unsigned int)(720*2048/2160);
        }
        else if(u4In == 1080)
        {
            u4HFactor = (unsigned int)(1920*4096/4096); //4096x2160
            u4VFactor = (unsigned int)(1080*2048/2160);
        }
        else if(u4In == 2160)
        {

            u4HFactor = (unsigned int)(3840*4096/4096); //4096x2160
            u4VFactor = (unsigned int)(2160*2048/2160);
        }
        else if(u4In == 2161)
        {

            u4HFactor = (unsigned int)(4096*4096/4096); //4096x2160
            u4VFactor = (unsigned int)(2160*2048/2160);
        }
        break;

    default:
        UTIL_Printf("[PmxVfy] default \n");
        break;
  }

  UTIL_Printf("[PmxVfy] u4HFactor=0x%x, u4VFactor=0x%x \n", u4HFactor, u4VFactor);

  if(ucVdoId == 0)
  {
      u4VdoRegBase = 0x42400;
      u4FmtRegBase = 0x42000;
  }
  else if (ucVdoId == 1)
  {
      u4VdoRegBase = 0x43400;
      u4FmtRegBase = 0x43000;
  }
  else
  {
      u4VdoRegBase = 0x43A00;
      u4FmtRegBase = 0x42000;
  }
  if(u4In == 480800){
  	    //fixme : setting uncorrectly.
		WriteREG((u4VdoRegBase+0x10), 0x01e0b45a);
		WriteREGMsk((u4VdoRegBase+0xe0), 0xb4, 0x1FF);
		WriteREG((u4FmtRegBase+0x9c), 0x000002d0);
  }
  if(u4In == 480)
  {
      WriteREG((u4VdoRegBase+0x10), 0x01e0b45a);
      WriteREGMsk((u4VdoRegBase+0xe0), 0xb4, 0x1FF);
      WriteREG((u4FmtRegBase+0x9c), 0x000002d0);
  }
  else if(u4In == 576)
  {
      WriteREG((u4VdoRegBase+0x10), 0x0240b45a);
      WriteREGMsk((u4VdoRegBase+0xe0), 0xb4, 0x1FF);
      WriteREG((u4FmtRegBase+0x9c), 0x000002d0);
  }
  else if(u4In == 720)
  {
      WriteREG((u4VdoRegBase+0x10), 0x02d040a0);
      WriteREGMsk((u4VdoRegBase+0xe0), 0x140, 0x1FF);
      WriteREG((u4FmtRegBase+0x9c), 0x00000500);
  }
  else if(u4In == 1080)
  {
      WriteREG((u4VdoRegBase+0x10), 0x0438b4f0);
      WriteREGMsk((u4VdoRegBase+0xe0), 0x1E0, 0x1FF);
      WriteREG((u4FmtRegBase+0x9c), 0x00000780);
  }
  else if(u4In == 2160)
  {
      WriteREG((u4VdoRegBase+0x10), 0x1870b4e0);
      WriteREGMsk((u4VdoRegBase+0xe0), 0x3c0, 0x1FF);
      WriteREG((u4FmtRegBase+0x9c), 0x00000f00);
  }
  else if(u4In == 2161)
  {
      WriteREG((u4VdoRegBase+0x10), 0x2870b400);
      WriteREGMsk((u4VdoRegBase+0xe0), 0x400, 0x1FF);
      WriteREG((u4FmtRegBase+0x9c), 0x00001000);
  }

  if(u4HFactor > 0x2000)
  {
      WriteREGMsk((u4FmtRegBase+0xB0), (u4HFactor << 15), 0xFFFF0000);
      WriteREGMsk((u4VdoRegBase+0x1C), (0x1 << 11), (0x1 << 11));
  }
  else
  {
      WriteREGMsk((u4FmtRegBase+0xB0), (u4HFactor << 16), 0xFFFF0000);
      WriteREGMsk((u4VdoRegBase+0x1C), 0, (0x1 << 11));
  }
  WriteREGMsk((u4FmtRegBase+0xB0),1, 1);
  WriteREGMsk((u4VdoRegBase+0x14), u4VFactor, 0xFFFF);
  WriteREG((u4FmtRegBase + 0x48), 0x5);
}

void vPmxVerifyHalVdoFitEx(unsigned int u4PicW, unsigned int u4PicH, BOOL fgVFit, BOOL fgHFit, UCHAR ucVdoId)
{
    unsigned int u4HFactor = 0, u4VFactor = 0;
    unsigned int u4VdoRegBase = 0, u4FmtRegBase = 0;
    unsigned int u4HBegin, u4HEnd, u4VBegin, u4VEnd, u4OutH, u4OutW;

    if(ucVdoId == 0)
    {
        u4VdoRegBase = 0x42400;
        u4FmtRegBase = 0x42000;
    }
    else if (ucVdoId == 1)
    {
        u4VdoRegBase = 0x43400;
        u4FmtRegBase = 0x43000;
    }
    else
    {
        u4VdoRegBase = 0x43A00;
        u4FmtRegBase = 0x42000;
    }

    u4HBegin = (ReadREG(u4FmtRegBase + 0xA0) >> 16) & 0x0FFF;
    u4HEnd = ReadREG(u4FmtRegBase + 0xA0) & 0x0FFF;
    u4VBegin = (ReadREG(u4FmtRegBase + 0xA4) >> 16) & 0xFFF;
    u4VEnd = ReadREG(u4FmtRegBase + 0xA4) & 0xFFF;

    u4OutW = u4HEnd - u4HBegin + 1;
    u4OutH = u4VEnd - u4VBegin + 1;

    if (fgVFit)
    {
        u4VFactor = 2048 * u4PicH / u4OutH; // 2048 / (u4OutH/u4PicH)
    }
    else
    {
        u4VFactor = 2048;
    }

    if (fgHFit)
    {
        u4HFactor = 4096 * u4PicW / u4OutW; // 4096 / (u4OutW/u4PicW)
    }
    else
    {
        u4HFactor = 4096;
    }

    UTIL_Printf("[Pmx] FitEx: (%d,%d)->(%d, %d) HFactor = 0x%X, VFactor = 0x%X\n",
        u4PicW, u4PicH, (fgHFit ? u4OutW : u4PicW), (fgVFit ? u4OutH : u4PicH), u4HFactor, u4VFactor);

    WriteREG((u4FmtRegBase+0x9c), u4PicW);

    if(u4HFactor > 0x2000)
    {
        WriteREGMsk((u4FmtRegBase+0xB0), (u4HFactor << 15), 0xFFFF0000);
        WriteREGMsk((u4VdoRegBase+0x1C), (0x1 << 11), (0x1 << 11));
    }
    else
    {
        WriteREGMsk((u4FmtRegBase+0xB0), (u4HFactor << 16), 0xFFFF0000);
        WriteREGMsk((u4VdoRegBase+0x1C), 0, (0x1 << 11));
    }

    WriteREGMsk((u4VdoRegBase+0x14), u4VFactor, 0xFFFF);
    WriteREG((u4FmtRegBase + 0x48), 0x5);
}


void vPmxVerifyHalEnableColorBar(UCHAR ucVdoId,UCHAR ucOn)
{
	unsigned int u4DispFmtRegBase;
	unsigned int u4Temp;
	if(ucVdoId == 0)
        {
                u4DispFmtRegBase = 0x42000;
        }
        else if(ucVdoId == 1)
        {
                u4DispFmtRegBase = 0x43000;
        }

        u4Temp= (ucOn==0) ? 0 : 1;
	WriteREGMsk((u4DispFmtRegBase + 0x118),0x130000,0xffffffff);	//colorbar width.
	WriteREGMsk((u4DispFmtRegBase + 0x118),u4Temp,0x1);

	if(u4Temp){
		VDO_LOG(VDO_LOG_LVL_DBG, "[PMX] set FMT %s color bar enable \n",(!ucVdoId) ? "Front" : "Rear");
	}
}

void vPmxVerifyHalEnablePMX(UCHAR ucVdoId, UCHAR ucOn)
{
    unsigned int u4DispFmtRegBase;
	//unsigned int u4VdoutFmtRegBase;

    //u4VdoutFmtRegBase = 0x3000;
    if(ucVdoId == 0)
    {
        u4DispFmtRegBase = 0x42000;
    }
    else if(ucVdoId == 1)
    {
        u4DispFmtRegBase = 0x43000;
    }
    else
    {
        u4DispFmtRegBase = 0x42000;
    }

    WriteREGMsk((u4DispFmtRegBase + 0xA0), 0x1<<31, 0x1<<31);

    if(ucOn)
    {
        WriteREGMsk((u4DispFmtRegBase + 0xAC), 0x1, 0x1);
        //WriteREGMsk((u4VdoutFmtRegBase + 0xAC), 0x1, 0x1);
    }
    else
    {
        WriteREGMsk((u4DispFmtRegBase + 0xAC), 0x0, 0x1);
        //WriteREGMsk((u4VdoutFmtRegBase + 0xAC), 0x0, 0x1);
    }
}

void vPmxVerifyHalVdoXSkip(unsigned int u4XSkip, UCHAR ucVdoId)
{
    unsigned int u4VdoRegBase;
    unsigned int u4V8RegBase;

    UTIL_Printf("[PmxVfy] u4XSkip=%d \n", u4XSkip);

    if(ucVdoId == 0)
    {
        u4VdoRegBase = 0x42400;
        u4V8RegBase = 0x42200;
    }
    else if(ucVdoId == 1)
    {
        u4VdoRegBase = 0x43400;
        u4V8RegBase = 0x43200;
    }
    else
    {
        u4VdoRegBase = 0x43a00;
        u4V8RegBase = 0x42200;
    }

    WriteREGMsk((u4VdoRegBase+0x14), (u4XSkip << 16), 0xFFFF0000);
    vPmxVerifyHalVdoRXSkip(u4XSkip, ucVdoId); //last pixel for direct shrink mode.
    WriteREGMsk((u4V8RegBase+0xb8), (u4XSkip << 28), 0xF0000000);//just for right skip of H_SHARP
}

unsigned int _ucCsHoriActRegion = 0;	

void vPmxVerifyHalV4Tap(UCHAR uc4Tap, UCHAR ucCoef, UCHAR ucSrcHD, UCHAR ucVdoId)
{
    unsigned int u4VdoRegBase = 0, u4FmtRegBase = 0, u4V8RegBase = 0, u4RegVal = 0, u4Res = 0;
    UCHAR uc4MA = 0;
    unsigned int u4i, u4Size;

    if(ucVdoId == 0)
    {
        u4VdoRegBase = 0x42400;
        u4V8RegBase = 0x42200;
        u4FmtRegBase = 0x42000;
    }
    else if(ucVdoId == 1)
    {
        u4VdoRegBase = 0x43400;
        u4V8RegBase = 0x43200;
        u4FmtRegBase = 0x43000;
    }
    else
    {
        u4VdoRegBase = 0x43a00;
        u4V8RegBase = 0x42200;
        u4FmtRegBase = 0x42000;
    }

    uc4MA = (ReadREG(u4VdoRegBase + 0x88) & (0x1 << 24)) >> 24;

    if(uc4Tap == 1) //Turn on 4Taps
    {
        if (0xff != ucCoef)
        {
            //UTIL_Printf("[pmx] %s(%d,%d,%d,%d): %s\n", __FUNCTION__, uc4Tap, ucCoef, ucSrcHD, ucVdoId, _ArVdoV4CoefSetting[ucCoef].szArrayName);

            u4Size = _ArVdoV4CoefSetting[ucCoef].size;
            for(u4i = 0; u4i < (u4Size - 1); u4i ++)
            {
                WriteREG((u4V8RegBase + (u4i*4)), _ArVdoV4CoefSetting[ucCoef].pu4RegSetting[u4i]);
                WriteREG((u4V8RegBase + (u4i*4) + 0x50), _ArVdoV4CoefSetting[ucCoef].pu4RegSetting[u4i]);
            }
            WriteREG((u4V8RegBase + 0x40), _ArVdoV4CoefSetting[ucCoef].pu4RegSetting[u4Size - 1]);
            WriteREG((u4V8RegBase + 0x90), _ArVdoV4CoefSetting[ucCoef].pu4RegSetting[u4Size - 1]);
        }
        if (uc4MA == 1)
            WriteREGMsk((u4VdoRegBase + 0x88), (0x1 << 24), (0x1 << 24));
        else
            WriteREGMsk((u4VdoRegBase + 0x88), 0x0, (0x1 << 24));

        WriteREGMsk((u4VdoRegBase + 0x7C), (0x3 << 20), (0xF << 20));

        WriteREGMsk((u4VdoRegBase + 0x78), 0x1, 0x1);
        WriteREGMsk((u4VdoRegBase + 0x78), (1<<4), ((1<<4)));

        u4RegVal = ReadREG(u4FmtRegBase + 0x9c);
        if(u4RegVal == 720)
        {
            u4Res = 480;//fixeme : add 800x480 800x600 for front
        }
        else if(u4RegVal == 1280)
        {
            u4Res = 720;
        }
        else if(u4RegVal == 1920)
        {
            u4Res = 1080;
        }else if(u4RegVal == 800){        
			u4Res = 480;  //800x600?
		}else if(u4RegVal == 1024){        
			u4Res = 600;  
		}

        if (u4Res == 1080)
            WriteREGMsk((u4VdoRegBase + 0x78), (0x1 << 7), 0x80);
        else
            WriteREGMsk((u4VdoRegBase + 0x78), 0, 0x80);
        WriteREGMsk((u4VdoRegBase + 0x78), (0<<16), ((1<<16))); //8561 new feature 42478[16]down_scl_4tap_force
        if (uc4MA != 1)
        { // frame - 4taps
            if(ucSrcHD)
            {
                if(u4Res == 1080)
                {
                    WriteREGMsk((u4VdoRegBase + 0xE0), (0x1 << 21) | (0x1 << 22) | (0x1 << 29) | (0x1 << 31), (0x1 << 21) | (0x1 << 22) | (0x1 << 24) | (0x1 << 29) | (0x1 << 31));
                }
                else
                {
                    WriteREGMsk((u4VdoRegBase + 0xE0), (0x1 << 22) | (0x1 << 29) | (0x1 << 31), (0x1 << 21) | (0x1 << 22) | (0x1 << 24) | (0x1 << 29) | (0x1 << 31));
                }
            }
            else
            {
                WriteREGMsk((u4VdoRegBase + 0xE0), (0x1 << 29) | (0x1 << 31), (0x1 << 21) | (0x1 << 22) | (0x1 << 24) | (0x1 << 29) | (0x1 << 31));
            }
        }

        //start line & sub line
        if(ucVdoId == 0)
        {
            //start line & sub line 
#if FRONT_ALL_HD_MEM_EN 		            
			if((u4Res == 480) ||(u4Res == 720)||(u4Res == 1080) || (u4RegVal==800) || (u4Res == 600)) //720p/1080p 4-tap special case :(800x480) 
#else
			if((u4Res == 720)||(u4Res == 1080) || (u4RegVal==800) || (u4Res == 600)) //720p/1080p 4-tap special case :(800x480) 
#endif
            {
                WriteREG((u4VdoRegBase + 0x20), 0x07fb07fa);
                WriteREG((u4VdoRegBase + 0x24), 0x07fd07fc);
                WriteREG((u4VdoRegBase + 0x50), 0x07fa07f9);
                WriteREG((u4VdoRegBase + 0x54), 0x07fe07fd);
                WriteREG((u4VdoRegBase + 0x28), 0x80800000);
                WriteREG((u4VdoRegBase + 0x2c), 0x2060a0e0);
            }
            else
            {
                if(ReadREG(u4VdoRegBase + 0x30) & 0x200000)//422
                {
                    WriteREG((u4VdoRegBase + 0x20), 0x07fd07fc);
                    WriteREG((u4VdoRegBase + 0x24), 0x07fb07fa);
                    WriteREG((u4VdoRegBase + 0x50), 0x07fc07fb);
                    WriteREG((u4VdoRegBase + 0x54), 0x07fc07fb);
                    WriteREG((u4VdoRegBase + 0x28), 0x80800000);
                    WriteREG((u4VdoRegBase + 0x2c), 0x00008080);
                }
                else
                { 
                    WriteREG((u4VdoRegBase + 0x20), 0x07fd07fc);
                    WriteREG((u4VdoRegBase + 0x24), 0x07ff07fe);
                    WriteREG((u4VdoRegBase + 0x50), 0x07fc07fb);
                    WriteREG((u4VdoRegBase + 0x54), 0x07fe07fd);
                    WriteREG((u4VdoRegBase + 0x28), 0x80800000);
                    WriteREG((u4VdoRegBase + 0x2c), 0xa0e02060);
                }
            }
			//
			if(ReadREG(u4VdoRegBase + 0x30) & 0x200000)//422
			{
				WriteREG((u4VdoRegBase + 0x20), 0x07fd07fc);
				WriteREG((u4VdoRegBase + 0x24), 0x07fb07fa);
				WriteREG((u4VdoRegBase + 0x50), 0x07fc07fb);
				WriteREG((u4VdoRegBase + 0x54), 0x07fc07fb);
				WriteREG((u4VdoRegBase + 0x28), 0x80800000);
				WriteREG((u4VdoRegBase + 0x2c), 0x00008080);

			}
			
        }
        else  //vdo2
        {
            //start line & sub line
            if((u4Res == 720)||(u4Res == 1080)) //720p/1080p 4-tap special case
            {
                WriteREG((u4VdoRegBase + 0x20), 0x07fb07fa);
                WriteREG((u4VdoRegBase + 0x24), 0x07fd07fc);
                WriteREG((u4VdoRegBase + 0x50), 0x07fa07f9);
                WriteREG((u4VdoRegBase + 0x54), 0x07fe07fd);
                WriteREG((u4VdoRegBase + 0x28), 0x80800000);
                WriteREG((u4VdoRegBase + 0x2c), 0x2060a0e0);
            }
            else
            {
                if(ReadREG(u4VdoRegBase + 0x30) & 0x200000)//422
                {
                    WriteREG((u4VdoRegBase + 0x20), 0x07fd07fc);
                    WriteREG((u4VdoRegBase + 0x24), 0x07fb07fa);
                    WriteREG((u4VdoRegBase + 0x50), 0x07fc07fb);
                    WriteREG((u4VdoRegBase + 0x54), 0x07fc07fb);
                    WriteREG((u4VdoRegBase + 0x28), 0x80800000);
                    WriteREG((u4VdoRegBase + 0x2c), 0x00008080);
                }
                else
                {
                    WriteREG((u4VdoRegBase + 0x20), 0x07fd07fc);
                    WriteREG((u4VdoRegBase + 0x24), 0x07ff07fe);
                    WriteREG((u4VdoRegBase + 0x50), 0x07fc07fb);
                    WriteREG((u4VdoRegBase + 0x54), 0x07fe07fd);
                    WriteREG((u4VdoRegBase + 0x28), 0x80800000);
                    WriteREG((u4VdoRegBase + 0x2c), 0xa0e02060);
                }
            }
        }
  }
  else //Turn off 4Taps
  {
      if(uc4MA == 1)
      {
          WriteREGMsk((u4VdoRegBase + 0x88), (0x1 << 24), (0x1 << 24));
          WriteREGMsk((u4VdoRegBase + 0x7C), (0x3 << 20), (0xF << 20));
          //if(ucSrcHD == 1)
          //  WriteREGMsk((u4VdoRegBase + 0xE0), 0xA0400000, 0xFF600000);
          //else
          //  WriteREGMsk((u4VdoRegBase + 0xE0), 0xA0000000, 0xFF600000);
      }
      else
      {
          WriteREGMsk((u4VdoRegBase + 0x88), 0x0, (0x1 << 24));
          WriteREGMsk((u4VdoRegBase + 0x7C), 0x0, (0xF << 20));
          //if(ucSrcHD == 1)
          //  WriteREGMsk((u4VdoRegBase + 0xE0), 0x01000000, 0xFF600000);
          //else
          //  WriteREGMsk((u4VdoRegBase + 0xE0), 0x00000000, 0xFF600000);
      }
      WriteREGMsk((u4VdoRegBase + 0x78), 0x0, 0x1);
  }
}

void vPmxVerifyHalV8Tap(UCHAR uc8Tap, UCHAR ucCoef, UCHAR ucSuper, UCHAR ucSrcHD, UCHAR ucVdoId)
{
    unsigned int u4V8RegBase, u4VdoRegBase,u4FmtRegBase;
    unsigned int u4i, u4Size;
    if(ucVdoId == 0)
    {
        u4V8RegBase = 0x42200;
        u4VdoRegBase = 0x42400;
        u4FmtRegBase = 0x42000;
    }
    else if (ucVdoId == 1)
    {
        u4V8RegBase = 0x43200;
        u4VdoRegBase = 0x43400;
        u4FmtRegBase = 0x43000;
    }
    else
    {
        u4V8RegBase = 0x42200;
        u4VdoRegBase = 0x43a00;
        u4FmtRegBase = 0x42000;
    }

    if(uc8Tap == 1) //Turn on 8Taps
    {
        if (0xff != ucCoef)
        {
            //UTIL_Printf("[pmx] %s(%d,%d,%d,%d,%d): %s\n", __FUNCTION__, uc8Tap, ucCoef, ucSuper, ucSrcHD, ucVdoId, _ArVdoV8CoefSetting[ucCoef].szArrayName);

            u4Size = _ArVdoV8CoefSetting[ucCoef].size;

            for(u4i = 0; u4i < u4Size; u4i ++)
            {
                WriteREG((u4V8RegBase + (u4i*4)), _ArVdoV8CoefSetting[ucCoef].pu4RegSetting[u4i]);
                WriteREG((u4V8RegBase + (u4i*4) + 0x50), _ArVdoV8CoefSetting[ucCoef].pu4RegSetting[u4i]);
            }
        }
        //if(ucSuper == 1) //means cs+8tap mode.
        if ((ReadREG(0x42800)&0x10) == 0x10) //cs+8taps
        {
            //_ucCsHoriActRegion = _ucCsHoriActRegion+0x000c000c;
            WriteREG((u4FmtRegBase+0xa0), _ucCsHoriActRegion);
            WriteREGMsk((u4FmtRegBase + 0xe4), 0, (1<<30)); // get sharp
        }
        else
        {
            WriteREGMsk((u4FmtRegBase + 0xe4), (1<<30), (1<<30)); // sharp
        }
        WriteREGMsk((u4V8RegBase + 0x4C), ((0x1 << 13) | (0x1 << 24) | (0x1 << 25)), ((0x1 << 0) | (0x1 << 13) | (0x1 << 24) | (0x1 << 25)));
        WriteREG((u4V8RegBase + 0x94), 0x07ff0000); //8 tap efect range.

		if(ucSrcHD == 1)
		{
			if(ucVdoId ==1){
				WriteREG((u4VdoRegBase + 0x20), 0x07fb07fa);
				WriteREG((u4VdoRegBase + 0x24), 0x07ff07fe);
				WriteREG((u4VdoRegBase + 0x50), 0x07fa07f9);
				WriteREG((u4VdoRegBase + 0x54), 0x07fe07fd);
				WriteREG((u4VdoRegBase + 0x28), 0x80800000);
				WriteREG((u4VdoRegBase + 0x2c), 0xa0e02060);				
				if(ReadREG(u4VdoRegBase + 0x30) & 0x200000)//422
				{
				WriteREG((u4VdoRegBase + 0x20), 0x07fd07fc);
				WriteREG((u4VdoRegBase + 0x24), 0x07fb07fa);
				WriteREG((u4VdoRegBase + 0x50), 0x07fc07fb);
				WriteREG((u4VdoRegBase + 0x54), 0x07fc07fb);
				WriteREG((u4VdoRegBase + 0x28), 0x80800000);
				WriteREG((u4VdoRegBase + 0x2c), 0x00008080);				
				}
			}else
			{			
				WriteREG((u4VdoRegBase + 0x20), 0x07fb07fa);
				WriteREG((u4VdoRegBase + 0x24), 0x07ff07fe);
				WriteREG((u4VdoRegBase + 0x50), 0x07fa07f9);
				WriteREG((u4VdoRegBase + 0x54), 0x07fe07fd);
				WriteREG((u4VdoRegBase + 0x28), 0x80800000);
				WriteREG((u4VdoRegBase + 0x2c), 0xa0e02060);
				if(ReadREG(u4VdoRegBase + 0x30) & 0x200000)//422
				{
				WriteREG((u4VdoRegBase + 0x20), 0x07fd07fc);
				WriteREG((u4VdoRegBase + 0x24), 0x07fb07fa);
				WriteREG((u4VdoRegBase + 0x50), 0x07fc07fb);
				WriteREG((u4VdoRegBase + 0x54), 0x07fc07fb);
				WriteREG((u4VdoRegBase + 0x28), 0x80800000);
				WriteREG((u4VdoRegBase + 0x2c), 0x00008080);				
				}

				
			}
		}

    }
    else //Turn off 8Taps
    {
        WriteREGMsk((u4V8RegBase + 0x4C), (0x1 << 13), ((0x1 << 0) | (0x1 << 13) | (0x1 << 24) | (0x1 << 25)));
    }
}

void vPmxVerifyHalDeIntMode(UCHAR ucVdoId, UCHAR ucMode)
{
    unsigned int u4VdoRegBase;
    unsigned int u4RegVal = 0;
    UCHAR ucSrcHD = 0;
    UCHAR ucSrc1080P = 0;

    if(ucVdoId == 0)
        u4VdoRegBase = 0x42400;
    else if(ucVdoId == 1)
        u4VdoRegBase = 0x43400;
    else
        u4VdoRegBase = 0x43a00;

    u4RegVal = ((((ReadREG(u4VdoRegBase + 0xE0) & (0x18000000))>>27)<<9) |(ReadREG(u4VdoRegBase + 0xE0) & (0x1FF))) << 2;
    if(u4RegVal > 1280)
    {
        ucSrcHD = 1;
        ucSrc1080P = 1;
    }
    else if(u4RegVal > 720)
    {
        ucSrcHD = 1;
        ucSrc1080P = 0;
    }
    else
    {
        ucSrcHD = 0;
        ucSrc1080P = 0;
    }

	//jgao:fixeme for 3365,front,panel >=800x480, HD mem always 1.
	if(ucVdoId==0){
#if FRONT_ALL_HD_MEM_EN 		
        ucSrcHD = 1;
#endif        
	}

    switch(ucMode)
    {
    case PMX_VFY_VDO_FLD:
        UTIL_Printf("[PMX] FLD\n");
        WriteREGMsk((u4VdoRegBase + 0x7C), 0, (0x1 << 20) | (0x1 << 21));
        WriteREGMsk((u4VdoRegBase + 0x30), 0, (0x1 << 0) | (0x1 << 1));
        WriteREGMsk((u4VdoRegBase + 0x88), 0, (0x1 << 24));
        WriteREGMsk((u4VdoRegBase + 0xC0), 0, (0x1 << 4));
        WriteREGMsk((u4VdoRegBase + 0xD0), 0, (0x1 << 0));
        WriteREG((u4VdoRegBase + 0xEC), ReadREG(u4VdoRegBase + 0x00));
        WriteREGMsk(0x42800, 0, (0x1 << 0));
        WriteREGMsk(0x42870, (0x3 << 28), (0x3 << 28));

        if(ucSrcHD)
        {
            WriteREGMsk((u4VdoRegBase + 0xE0), (0x1 << 24), (0x1 << 21) | (0x1 << 22) | (0x1 << 24) | (0x1 << 29) | (0x1 << 31));
        }
        else
        {
            WriteREGMsk((u4VdoRegBase + 0xE0), 0, (0x1 << 21) | (0x1 << 22) | (0x1 << 24) | (0x1 << 29) | (0x1 << 31));
        }

        if ((ReadREG(u4VdoRegBase + 0x30) & (1<<21))) //422
        {
            //subline
            WriteREG((u4VdoRegBase + 0x20), 0x0ffb0ffa);
            WriteREG((u4VdoRegBase + 0x24), 0x07fb0ffa);
            WriteREG((u4VdoRegBase + 0x50), 0x07fa0ff9);
            WriteREG((u4VdoRegBase + 0x54), 0x0ffa0ff9);
            WriteREG((u4VdoRegBase + 0x28), 0x80800000);
            WriteREG((u4VdoRegBase + 0x2C), 0x80800000);
        }
        else //420
        {
            //subline
            WriteREG((u4VdoRegBase + 0x20), 0x0ffb0ffa);
            WriteREG((u4VdoRegBase + 0x24), 0x0ffd0ffc);
            WriteREG((u4VdoRegBase + 0x50), 0x0ffc0ffa);
            WriteREG((u4VdoRegBase + 0x54), 0x0ffe0ffd);
            WriteREG((u4VdoRegBase + 0x28), 0x80800000);
            WriteREG((u4VdoRegBase + 0x2C), 0x2060a0e0);
        }
        break;
    case PMX_VFY_VDO_FRM:
        UTIL_Printf("[PMX] FRM\n");
        WriteREGMsk((u4VdoRegBase + 0x7C), 0, (0x1 << 20) | (0x1 << 21));
        WriteREGMsk((u4VdoRegBase + 0x30), (0x1 << 0) | (0x1 << 1), (0x1 << 0) | (0x1 << 1));
        WriteREGMsk((u4VdoRegBase + 0x88), 0, (0x1 << 24));
        WriteREGMsk((u4VdoRegBase + 0xC0), 0, (0x1 << 4));
        WriteREGMsk((u4VdoRegBase + 0xD0), 0, (0x1 << 0));
        WriteREG((u4VdoRegBase + 0xEC), ReadREG(u4VdoRegBase + 0x00));
        WriteREGMsk(0x42800, 0, (0x1 << 0));
        WriteREGMsk(0x42870, (0x3 << 28), (0x3 << 28));
        
        if(ucSrc1080P&&((ReadREG(u4VdoRegBase + 0x78) & (0x1<<0))))
        {
            WriteREGMsk((u4VdoRegBase + 0xE0), (0x1 << 22)|(0x1 << 21), (0x1 << 21) | (0x1 << 22) | (0x1 << 24) | (0x1 << 29) | (0x1 << 31));
        }
        else if(ucSrcHD)
        {
            WriteREGMsk((u4VdoRegBase + 0xE0), (0x1 << 24), (0x1 << 21) | (0x1 << 22) | (0x1 << 24) | (0x1 << 29) | (0x1 << 31));
        }
        else
        {
            WriteREGMsk((u4VdoRegBase + 0xE0), 0, (0x1 << 22) | (0x1 << 21) | (0x1 << 24) | (0x1 << 29) | (0x1 << 31));
        }

        if ((ReadREG(u4VdoRegBase + 0x30) & (1<<21))) //422
        {
            //subline
            WriteREG((u4VdoRegBase + 0x20), 0x0ffa0ff9);
            WriteREG((u4VdoRegBase + 0x24), 0x0ffa0ff9);
            WriteREG((u4VdoRegBase + 0x50), 0x0ffa0ff9);
            WriteREG((u4VdoRegBase + 0x54), 0x0ffa0ff9);
            WriteREG((u4VdoRegBase + 0x28), 0x00000000);
            WriteREG((u4VdoRegBase + 0x2C), 0x00000000);
        }
        else //420
        {
            //subline
            WriteREG((u4VdoRegBase + 0x20), 0x0ff90ff9);
            WriteREG((u4VdoRegBase + 0x24), 0x0ffd0ffc);
            WriteREG((u4VdoRegBase + 0x50), 0x0ff90ff9);
            WriteREG((u4VdoRegBase + 0x54), 0x0ffd0ffc);
            WriteREG((u4VdoRegBase + 0x28), 0x00000000);
            WriteREG((u4VdoRegBase + 0x2C), 0x20602060);
        }
        break;
    case PMX_VFY_VDO_4MA:
        UTIL_Printf("[PMX] 4MA\n");
        WriteREGMsk((u4VdoRegBase + 0x7C), (0x1 << 20) | (0x1 << 21), (0x1 << 20) | (0x1 << 21));
        WriteREGMsk((u4VdoRegBase + 0x30), 0, (0x1 << 0) | (0x1 << 1));
        WriteREGMsk((u4VdoRegBase + 0x88), (0x1 << 24), (0x1 << 24));
        WriteREGMsk((u4VdoRegBase + 0xC0), 0, (0x1 << 4));
        WriteREGMsk((u4VdoRegBase + 0xD0), 0, (0x1 << 0));
        WriteREG((u4VdoRegBase + 0xEC), ReadREG(u4VdoRegBase + 0x00));
        WriteREGMsk(0x42800, 0, (0x1 << 0));
        WriteREGMsk(0x42870, (0x3 << 28), (0x3 << 28));

        if(ucSrcHD)
        {
            if (ucSrc1080P)
            {
                WriteREGMsk((u4VdoRegBase + 0xE0), (0x1 << 21) | (0x1 << 22) | (0x1 << 29) | (0x1 << 31), (0x1 << 21) | (0x1 << 22) | (0x1 << 24) | (0x1 << 29) | (0x1 << 31));
            }
            else
            {
                WriteREGMsk((u4VdoRegBase + 0xE0), (0x1 << 22) | (0x1 << 29) | (0x1 << 31), (0x1 << 21) | (0x1 << 22) | (0x1 << 24) | (0x1 << 29) | (0x1 << 31));
            }
        }
        else
        {
            WriteREGMsk((u4VdoRegBase + 0xE0), (0x1 << 29) | (0x1 << 31), (0x1 << 21) | (0x1 << 22) | (0x1 << 24) | (0x1 << 29) | (0x1 << 31));
        }

        if ((ReadREG(u4VdoRegBase + 0x30) & (1<<21))) //422
        {
            //subline
            WriteREG((u4VdoRegBase + 0x20), 0x0ffb0ffa);
            WriteREG((u4VdoRegBase + 0x24), 0x0ffb0ffa);
            WriteREG((u4VdoRegBase + 0x50), 0x0ffa0ff9);
            WriteREG((u4VdoRegBase + 0x54), 0x0ffa0ff9);
            WriteREG((u4VdoRegBase + 0x28), 0x80800000);
            WriteREG((u4VdoRegBase + 0x2C), 0x80800000);
        }
        else //420
        {
            //subline
            WriteREG((u4VdoRegBase + 0x20), 0x0ffb0ffa);
            WriteREG((u4VdoRegBase + 0x24), 0x0ffd0ffc);
            WriteREG((u4VdoRegBase + 0x50), 0x0ffc0ffa);
            WriteREG((u4VdoRegBase + 0x54), 0x0ffe0ffd);
            WriteREG((u4VdoRegBase + 0x28), 0x80800000);
            WriteREG((u4VdoRegBase + 0x2C), 0x2060a0e0);
        }
        break;

    case PMX_VFY_VDO_4FUSION:
        UTIL_Printf("[PMX] 4FUSION\n");
        WriteREGMsk((u4VdoRegBase + 0x7C), (0x1 << 20) | (0x1 << 21), (0x1 << 20) | (0x1 << 21));
        WriteREGMsk((u4VdoRegBase + 0x30), 0, (0x1 << 0) | (0x1 << 1));
        WriteREGMsk((u4VdoRegBase + 0x88), (0x1 << 24), (0x1 << 24));
        WriteREGMsk((u4VdoRegBase + 0xC0), (0x1 << 4), (0x1 << 4));
        WriteREGMsk((u4VdoRegBase + 0xD0), (0x1 << 0), (0x1 << 0));
        WriteREG((u4VdoRegBase + 0xEC), ReadREG(u4VdoRegBase + 0x04));
        WriteREGMsk(0x42800, (0x1 << 0), (0x1 << 0));
        WriteREGMsk(0x42870, (0x3 << 28), (0x3 << 28));
        if(ucSrcHD)
        {
            if (ucSrc1080P)
            {
                WriteREGMsk((u4VdoRegBase + 0xE0), (0x1 << 21) | (0x1 << 22) | (0x1 << 29) | (0x1 << 31), (0x1 << 21) | (0x1 << 22) | (0x1 << 24) | (0x1 << 29) | (0x1 << 31));
            }
            else
            {
                WriteREGMsk((u4VdoRegBase + 0xE0), (0x1 << 22) | (0x1 << 29) | (0x1 << 31), (0x1 << 21) | (0x1 << 22) | (0x1 << 24) | (0x1 << 29) | (0x1 << 31));
            }
        }
        else
        {
            WriteREGMsk((u4VdoRegBase + 0xE0), (0x1 << 29) | (0x1 << 31), (0x1 << 21) | (0x1 << 22) | (0x1 << 24) | (0x1 << 29) | (0x1 << 31));
        }
        if ((ReadREG(u4VdoRegBase + 0x30) & (1<<21))) //422
        {
            //subline
            WriteREG((u4VdoRegBase + 0x20), 0x0ffb0ffa);
            WriteREG((u4VdoRegBase + 0x24), 0x0ffb0ffa);
            WriteREG((u4VdoRegBase + 0x50), 0x0ffa0ff9);
            WriteREG((u4VdoRegBase + 0x54), 0x0ffa0ff9);
            WriteREG((u4VdoRegBase + 0x28), 0x80800000);
            WriteREG((u4VdoRegBase + 0x2C), 0x80800000);
        }
        else //420
        {
            //subline
            WriteREG((u4VdoRegBase + 0x20), 0x0ffb0ffa);
            WriteREG((u4VdoRegBase + 0x24), 0x0ffd0ffc);
            WriteREG((u4VdoRegBase + 0x50), 0x0ffc0ffa);
            WriteREG((u4VdoRegBase + 0x54), 0x0ffe0ffd);
            WriteREG((u4VdoRegBase + 0x28), 0x80800000);
            WriteREG((u4VdoRegBase + 0x2C), 0x2060a0e0);
        }
        break;

    default:
        break;
  }
}

unsigned int _u4PmxVfyOsdRegBase[5] =
{0x20100, 0x20200, 0x20300, 0x20a00, 0x20b00};

void vPmxVerifyHalOSDMixRatio(UCHAR ucOsdId, unsigned int u4MixRatio)
{
    unsigned int u4RegBase;
    unsigned int u4RegVal;

    if((ucOsdId > 5) || (ucOsdId < 1))
    {
        return; //error osd id
    }
    else
    {
        u4RegBase = _u4PmxVfyOsdRegBase[ucOsdId - 1];
    }

    u4RegVal = ReadREG(u4RegBase + 0x08) & (~0xFF000000);
    u4RegVal |= ((u4MixRatio & 0xFF) << 24);
    WriteREG(u4RegBase + 0x08, u4RegVal);

    return;
}

void vPmxVerifyHalSrcFmt(UCHAR ucVdoId, UCHAR ucSrcFmt)
{
    unsigned int u4VdoRegBase;
    unsigned int u4Reg424E0, u4Reg42430;

    if(ucVdoId == 0)
    {
        u4VdoRegBase = 0x42400;
    }
    else if(ucVdoId == 1)
    {
        u4VdoRegBase = 0x43400;
    }
    else
    {
        u4VdoRegBase = 0x43a00;
        //return; //error
    }

    u4Reg42430 = ReadREG(u4VdoRegBase + 0x30) & (~0x00200000);
    u4Reg424E0 = ReadREG(u4VdoRegBase + 0xE0) & (~0x00800000);

    switch (ucSrcFmt)
    {
    case PMX_VFY_VDO_SRC_420_MB: //YCbCr 420 MB
        UTIL_Printf("[PMX] SrcFmt: 420 MB\n");
        u4Reg42430 |= 0x0;
        u4Reg424E0 |= 0x0;
        break;

    case PMX_VFY_VDO_SRC_422_MB: //YCbCr 422 MB
        UTIL_Printf("[PMX] SrcFmt: 422 MB\n");
        u4Reg42430 |= 0x00200000;
        u4Reg424E0 |= 0x0;
        break;

    case PMX_VFY_VDO_SRC_420_SL: //YCbCr 420 scan-line
        UTIL_Printf("[PMX] SrcFmt: 420 RS\n");
        u4Reg42430 |= 0x0;
        u4Reg424E0 |= 0x00800000;
        break;

    case PMX_VFY_VDO_SRC_422_SL: //YCbCr 422 scan-line
        UTIL_Printf("[PMX] SrcFmt: 422 RS\n");
        u4Reg42430 |= 0x00200000;
        u4Reg424E0 |= 0x00800000;
        break;

    default:
        break;
    }

    WriteREG(u4VdoRegBase + 0x30, u4Reg42430);
    WriteREG(u4VdoRegBase + 0xe0, u4Reg424E0);

    return;
}

BOOL _fgHdownScalerOn = FALSE;

/*
	now ok for fmt front.
*/
void vPmxVerifyHalHDownScaler2(UCHAR ucOn, UCHAR ucVdoId,unsigned int u4HScaleUp, unsigned int u4HScaleDiv,unsigned int Mode){
	//adjust 4206c 4207c a46a0
    unsigned int u4DispFmtRegBase;
	unsigned int u4ZoomCoef;
	unsigned int u4temp;
	
    if(ucVdoId == 0) //Dispfmt 0
    {
        u4DispFmtRegBase = 0x42000;
    }
    else if(ucVdoId == 1)
    {
        u4DispFmtRegBase = 0x43000;
    }

	u4temp =  ReadREG(u4DispFmtRegBase + 0x6c);
	if(u4temp & 0x1){
		u4ZoomCoef = (u4temp >>16) & 0xffff;
	}else{
		u4ZoomCoef = 0x1000;
	}

	if(Mode==1){
		//acc
	    u4ZoomCoef = (u4HScaleUp*0x1000)/u4HScaleDiv;
	    WriteREGMsk(u4DispFmtRegBase + 0xcc,u4ZoomCoef,0x1fff);
		
		WriteREGMsk(u4DispFmtRegBase + 0x6c,1,0x1);		 
		WriteREGMsk(u4DispFmtRegBase + 0xe4,1<<29,1<<29);
	}else{ //fir

	   WriteREGMsk(u4DispFmtRegBase + 0xe4,0<<29,1<<29);
	   u4ZoomCoef = (u4HScaleDiv*0x1000)/u4HScaleUp;
	   
	   WriteREGMsk(u4DispFmtRegBase + 0x6c,(u4ZoomCoef<<16)|(ucOn&1),0xffff0001);		
	}




	
}

void vPmxVerifyHalHDownScaler(UCHAR ucOn, UCHAR ucVdoId, unsigned int u4DownSrcSel, unsigned int u4HScaleUp, unsigned int u4HScaleDiv)
{
    unsigned int u4SrcHstart, u4SrcHend, u4SrcVstart, u4SrcVend;
    unsigned int u4TgtHstart, u4TgtHend, u4TgtVstart, u4TgtVend;
    unsigned int u4SrcWidth, u4TgtWidth, u4Hscale;
    unsigned int u4Reg;
    unsigned int u4DispFmtRegBase;
    unsigned int u4VSyncWidth;
    unsigned int u4Mode = 480;
    unsigned int u4AccuFactor = 0;

    if(ucVdoId == 0) //Dispfmt 1
    {
        u4DispFmtRegBase = 0x42000;
    }
    else if(ucVdoId == 1)
    {
        u4DispFmtRegBase = 0x43000;
    }
    else
    {
        u4DispFmtRegBase = 0x42000;
    }

    u4SrcHstart = (ReadREG(u4DispFmtRegBase + 0xA0) >> 16) & 0x0FFF;
    u4SrcHend = ReadREG(u4DispFmtRegBase + 0xA0) & 0x0FFF;
    u4SrcVstart = (ReadREG(u4DispFmtRegBase + 0xA4) >> 16) & 0xFFF;
    u4SrcVend = ReadREG(u4DispFmtRegBase + 0xA4) & 0xFFF;

    if(ucOn && (_fgHdownScalerOn == FALSE))
    {
        u4SrcVstart -= 1;
        WriteREG((u4DispFmtRegBase + 0xA4), ((u4SrcVstart << 16) | u4SrcVend));
        WriteREG((u4DispFmtRegBase + 0xA8), ((u4SrcVstart << 16) | u4SrcVend));

        u4VSyncWidth = ReadREG(u4DispFmtRegBase + 0x94) & 0x1F00;
        u4VSyncWidth -= 0x100;
        WriteREG((u4DispFmtRegBase + 0x94), (ReadREG(u4DispFmtRegBase + 0x94) & (~0x1F00)) | u4VSyncWidth);
        _fgHdownScalerOn = TRUE;
    }
    else if((ucOn == 0) && (_fgHdownScalerOn == TRUE))
    {
        u4SrcVstart += 1;
        WriteREG((u4DispFmtRegBase + 0xA4), ((u4SrcVstart << 16) | u4SrcVend));
        WriteREG((u4DispFmtRegBase + 0xA8), ((u4SrcVstart << 16) | u4SrcVend));

        u4VSyncWidth = ReadREG(u4DispFmtRegBase + 0x94) & 0x1F00;
        u4VSyncWidth += 0x100;
        WriteREG((u4DispFmtRegBase + 0x94), (ReadREG(u4DispFmtRegBase + 0x94) & (~0x1F00)) | u4VSyncWidth);
        _fgHdownScalerOn = FALSE;
    }
    u4SrcWidth = u4SrcHend - u4SrcHstart;
    u4TgtWidth = (u4SrcWidth * u4HScaleUp)/u4HScaleDiv;

    u4TgtHstart = u4SrcHstart + (u4SrcWidth - u4TgtWidth)/2;
    u4TgtHend = u4TgtHstart + u4TgtWidth;
    u4TgtVstart = u4SrcVstart;
    u4TgtVend = u4SrcVend;

    u4Hscale = (0x1000 * u4HScaleDiv)/u4HScaleUp;

    u4Reg = ReadREG(u4DispFmtRegBase + 0xd4)&0x1FFF0000;

    if (u4Reg == 0x157c0000)
    {
        u4Mode = 2161;
    }
    else if (u4Reg == 0x11300000)
    {
        u4Mode = 2160;
    }
    else
    {
        u4Reg = ReadREG(u4DispFmtRegBase + 0x94);
        if(u4Reg &  (0x1 << 14)) //HD_ON
        {
            if(u4Reg &  (0x1 << 13)) //720
            {
                u4Mode = 720;
            }
            else //1080
            {
                u4Mode = 1080;
            }
        }
        else //480
        {
            u4Mode = 480;
        }
    }

    UTIL_Printf("[PmxVfy] ucOn=%d, u4Mode=%d, u4HScaleUp=%d, u4HScaleDiv=%d \n", ucOn, u4Mode, u4HScaleUp, u4HScaleDiv);

    switch(u4Mode)
    {
    case 480:
    case 576:
        WriteREG((u4DispFmtRegBase + 0x70), ((u4SrcHstart + 2) << 16) | (u4SrcHend + 5));
        WriteREG((u4DispFmtRegBase + 0x7C), ((u4TgtHstart + 2) << 16) | (u4TgtHend + 5));
        break;
    case 720:
    case 1080:
        WriteREG((u4DispFmtRegBase + 0x70), ((u4SrcHstart + 3) << 16) | (u4SrcHend + 6));
        WriteREG((u4DispFmtRegBase + 0x7C), ((u4TgtHstart + 3) << 16) | (u4TgtHend + 5));
        break;
    default:
        break;
    }

    WriteREG((u4DispFmtRegBase + 0x74), (u4TgtVstart << 16) | u4TgtVend);
    WriteREG((u4DispFmtRegBase + 0x78), (u4TgtVstart << 16) | u4TgtVend);

    u4Reg = ReadREG(0x30F4) & (~0x0F000000);
    u4Reg |= ((0 & 0xF)<<24); //DownSrcSel, no use in 8530
    WriteREG(0x30F4, u4Reg);

    if(ucOn)
    {
        u4Reg = (0x00000801) | (u4Hscale << 16);
    }
    else
    {
        u4Reg = 0;
    }
    WriteREG((u4DispFmtRegBase + 0x6C), u4Reg);

    WriteREGMsk((u4DispFmtRegBase + 0xe4), (1 << 29),  (1 << 29)); // select accumulation mode.
    u4AccuFactor = (4096*4096)/u4Hscale;
    WriteREGMsk((u4DispFmtRegBase + 0xcc), u4AccuFactor, 0x1fff);

    // If enable shadow register, need trigger.
    if (ReadREG(u4DispFmtRegBase + 0xa0) & 0x20000000)
    {
        WriteREGMsk((u4DispFmtRegBase + 0xa0), (1 << 31),  (1 << 31));
    }

    //  WriteREG(0x420b0, ReadREG(0x420b0)|0x01); //attention, important, but be set to 1 or no video output
    //  WriteREG(0x423b0, ReadREG(0x423b0)|0x01); //attention, important, but be set to 1 or no video output

    //register 0x30ec?

    return;
}

#if 1 // (CONFIG_CHIP_VER_PMXVDO == CONFIG_CHIP_VER_MT8563)

/*
 * MT8563 new scale mode, 480P,720P,1080I, 1080P support.
 *  there ar 4 scale group. Guoxing suggests pre-scale use 8TAP, so there are 2 groups need be taken care.
 *  In software view, all the 4 groups need be emulated.
 *
 *  g_fgPreScaleUseLinear  g_fgPostScaleUseAcc        need_care
 *
 *            0                    0                      v
 *
 *            0                    1                      v
 *
 *            1                    0                      x
 *
 *            1                    1                      x
 *
 */

/*
 * Please note FMT timing 480I, 576I not support new scale mode.
 * In real use case, fmt timing is 480P, 576P, and P2I convert to I timing, so it should be support.
*/

// fgNewScaleMode: corresponding to 420C8[0] or 423C8[0]
// fgPostScaleUseAcc: corresponding to 420C8[1] or 423C8[1]
// fgPreScaleUseLinear: corresponding to 420B0[1], 420E4[8] o4 423B0[1], 423E4[8].

volatile PMX_VFY_NEW_HDSL_H_OFST_T g_rNDSLOFST_480 = {-5, -4, 4, 5};
volatile PMX_VFY_NEW_HDSL_H_OFST_T g_rNDSLOFST_576 = {-5, -4, 4, 5};
volatile PMX_VFY_NEW_HDSL_H_OFST_T g_rNDSLOFST_720 = {-8, -6, 5, 6};
volatile PMX_VFY_NEW_HDSL_H_OFST_T g_rNDSLOFST_1080 = {-8, -6, 5, 6};


void vPmxVerifyHalNewHDownScaler(UCHAR ucOn, UCHAR ucVdoId, unsigned int u4DownSrcSel, unsigned int u4HScaleUp, unsigned int u4HScaleDiv,
    BOOL fgNewScale, BOOL fgPreScaleUseLinear, BOOL fgPostScaleUseAcc)
{
    unsigned int u4SrcHstart, u4SrcHend, u4SrcVstart, u4SrcVend;
    unsigned int u4TgtHstart, u4TgtHend, u4TgtVstart, u4TgtVend;
    unsigned int u4SrcWidth, u4TgtWidth, u4Hscale;
    unsigned int u4Reg, u4Reg420E4, u4Reg420B0, u4Reg4206C;
    unsigned int u4DispFmtRegBase;
    unsigned int u4VSyncWidth;
    unsigned int u4Mode = 480;
    unsigned int u4AccuFactor = 0;

    if(ucVdoId == 0) //Dispfmt 1
    {
        u4DispFmtRegBase = 0x42000;
    }
    else if(ucVdoId == 1)
    {
        u4DispFmtRegBase = 0x43000;
    }
    else
    {
        u4DispFmtRegBase = 0x42000;
    }

    /*
     * 420A0: Horizontal active zone register
     * [12:0]   -- HACTEND
     * [28:16]  -- HACTBGN
     */
    u4Reg = ReadREG(u4DispFmtRegBase + 0xA0);
    u4SrcHstart = (u4Reg >> 16) & 0x0FFF;
    u4SrcHend = u4Reg & 0x0FFF;

    /*
     * 420A4: Vertical odd active zone register
     * [11:0]   -- VOACTEND
     * [27:16]  -- VOACTBGN
     */
    u4Reg = ReadREG(u4DispFmtRegBase + 0xA4);
    u4SrcVstart = (u4Reg >> 16) & 0xFFF;
    u4SrcVend = u4Reg & 0xFFF;

    if(ucOn && (_fgHdownScalerOn == FALSE))
    {
        u4SrcVstart -= 1;
        WriteREG((u4DispFmtRegBase + 0xA4), ((u4SrcVstart << 16) | u4SrcVend));
        WriteREG((u4DispFmtRegBase + 0xA8), ((u4SrcVstart << 16) | u4SrcVend));

        u4Reg = ReadREG(u4DispFmtRegBase + 0x94);
        u4VSyncWidth = u4Reg & 0x1F00; // 42094[12:8]vsynwidth
        u4VSyncWidth -= 0x100; // vsync width subtract 1.
        WriteREG((u4DispFmtRegBase + 0x94), (u4Reg & (~0x1F00)) | u4VSyncWidth);
        _fgHdownScalerOn = TRUE;
    }
    else if((ucOn == 0) && (_fgHdownScalerOn == TRUE))
    {
        u4SrcVstart += 1;
        WriteREG((u4DispFmtRegBase + 0xA4), ((u4SrcVstart << 16) | u4SrcVend));
        WriteREG((u4DispFmtRegBase + 0xA8), ((u4SrcVstart << 16) | u4SrcVend));

        u4Reg = ReadREG(u4DispFmtRegBase + 0x94);
        u4VSyncWidth = u4Reg & 0x1F00;
        u4VSyncWidth += 0x100; // vsync width add 1.
        WriteREG((u4DispFmtRegBase + 0x94), (u4Reg & (~0x1F00)) | u4VSyncWidth);
        _fgHdownScalerOn = FALSE;
    }

    u4SrcWidth = u4SrcHend - u4SrcHstart;
    u4TgtWidth = (u4SrcWidth * u4HScaleUp)/u4HScaleDiv;

    u4TgtHstart = u4SrcHstart + (u4SrcWidth - u4TgtWidth)/2; // Make the picture in middle of screen.
    u4TgtHend = u4TgtHstart + u4TgtWidth;
    u4TgtVstart = u4SrcVstart;
    u4TgtVend = u4SrcVend;

    u4Hscale = (0x1000 * u4HScaleDiv)/u4HScaleUp;

    /*
     * 420D4: Horizontal/Vertical total pixel.
     * [11:0]   -- V_TOTAL
     * [28:16]  -- H_TOTAL
     */
    u4Reg = ReadREG(u4DispFmtRegBase + 0xd4)&0x1FFF0000;

    if (u4Reg == 0x157c0000)
    {
        u4Mode = 2161; // 4096*2160, only 8580 support.
    }
    else if (u4Reg == 0x11300000)
    {
        u4Mode = 2160; // 3840*2160, only 8580 support.
    }
    else
    {
        /*
         * 42094: Mode control register
         * [7:0]   -- HSYNC WIDTH
         * [12:8]  -- VSYNC WIDTH
         * [13]    -- HD_TP, 1:HD_720, 0:HD_1080
         * [14]    -- HD_ON, 1:HD mode, 0:SD mode.
         * [15]    -- PRGS
         */
        u4Reg = ReadREG(u4DispFmtRegBase + 0x94);
        if(u4Reg &  (0x1 << 14)) //HD_ON
        {
            if(u4Reg &  (0x1 << 13)) //720
            {
                u4Mode = 720;
            }
            else //1080
            {
                u4Mode = 1080;
            }
        }
        else //480
        {
            u4Mode = 480;
        }
    }

    UTIL_Printf("[PmxVfy] ucOn=%d, u4Mode=%d, new=%d, pre=%s, post=%s, HScale=%d/%d\n",
        ucOn, u4Mode, fgNewScale,
        (fgPreScaleUseLinear ? "Lnr" : "8Tap"),
        (fgPostScaleUseAcc ? "Acc" : "Lnr"),
        u4HScaleUp, u4HScaleDiv);

    /*
     * 42070: Down scaler range, horizontal active zone register
     * [11:0]   -- HDWN_HEND
     * [27:16]  -- HDWN_HBGN
     *
     * 4207C: Down scaler output range, horizontal active zone register
     * [11:0]   -- HDWN_HO_END
     * [27:16]  -- HDWN_HO_BGN
     */
    switch(u4Mode)
    {
    case 480:
        if (fgNewScale)
        {
            WriteREG((u4DispFmtRegBase + 0x70), (((int)u4SrcHstart + g_rNDSLOFST_480.i4HBGN) << 16) | ((int)u4SrcHend + g_rNDSLOFST_480.i4HEND));
            WriteREG((u4DispFmtRegBase + 0x7C), (((int)u4TgtHstart + g_rNDSLOFST_480.i4HOBGN) << 16) | ((int)u4TgtHend + g_rNDSLOFST_480.i4HOEND));
        }
        else
        {
            WriteREG((u4DispFmtRegBase + 0x70), ((u4SrcHstart + 2) << 16) | (u4SrcHend + 5));
            WriteREG((u4DispFmtRegBase + 0x7C), ((u4TgtHstart + 2) << 16) | (u4TgtHend + 5));
        }
        break;

    case 576:
        if (fgNewScale)
        {
            WriteREG((u4DispFmtRegBase + 0x70), (((int)u4SrcHstart + g_rNDSLOFST_576.i4HBGN) << 16) | ((int)u4SrcHend + g_rNDSLOFST_576.i4HEND));
            WriteREG((u4DispFmtRegBase + 0x7C), (((int)u4TgtHstart + g_rNDSLOFST_576.i4HOBGN) << 16) | ((int)u4TgtHend + g_rNDSLOFST_576.i4HOEND));
        }
        else
        {
            WriteREG((u4DispFmtRegBase + 0x70), ((u4SrcHstart + 2) << 16) | (u4SrcHend + 5));
            WriteREG((u4DispFmtRegBase + 0x7C), ((u4TgtHstart + 2) << 16) | (u4TgtHend + 5));
        }
        break;

    case 720:
        if (fgNewScale)
        {
            WriteREG((u4DispFmtRegBase + 0x70), (((int)u4SrcHstart + g_rNDSLOFST_720.i4HBGN) << 16) | ((int)u4SrcHend + g_rNDSLOFST_720.i4HEND));
            WriteREG((u4DispFmtRegBase + 0x7C), (((int)u4TgtHstart + g_rNDSLOFST_720.i4HOBGN) << 16) | ((int)u4TgtHend + g_rNDSLOFST_720.i4HOEND));
        }
        else
        {
            WriteREG((u4DispFmtRegBase + 0x70), ((u4SrcHstart + 3) << 16) | (u4SrcHend + 6));
            WriteREG((u4DispFmtRegBase + 0x7C), ((u4TgtHstart + 3) << 16) | (u4TgtHend + 5));
        }
        break;

    case 1080:
        if (fgNewScale)
        {
            WriteREG((u4DispFmtRegBase + 0x70), (((int)u4SrcHstart + g_rNDSLOFST_1080.i4HBGN) << 16) | ((int)u4SrcHend + g_rNDSLOFST_1080.i4HEND));
            WriteREG((u4DispFmtRegBase + 0x7C), (((int)u4TgtHstart + g_rNDSLOFST_1080.i4HOBGN) << 16) | ((int)u4TgtHend + g_rNDSLOFST_1080.i4HOEND));
        }
        else
        {
            WriteREG((u4DispFmtRegBase + 0x70), ((u4SrcHstart + 3) << 16) | (u4SrcHend + 6));
            WriteREG((u4DispFmtRegBase + 0x7C), ((u4TgtHstart + 3) << 16) | (u4TgtHend + 5));
        }
        break;
    default:
        break;
    }

    /*
     * 42074: Down scaler range, vertical odd active zone register
     * [11:0]   -- HDWN_VOEND
     * [27:16]  -- HDWN_VOBGN
     *
     * 42078: Down scaler range, horizontal even active zone register
     * [11:0]   -- HDWN_VEEND
     * [27:16]  -- HDWN_VEBGN
     */
    WriteREG((u4DispFmtRegBase + 0x74), (u4TgtVstart << 16) | u4TgtVend);
    WriteREG((u4DispFmtRegBase + 0x78), (u4TgtVstart << 16) | u4TgtVend);

    /* 30F4[27:24]  -- down src */
    u4Reg = ReadREG(0x30F4) & (~0x0F000000);
    u4Reg |= ((0 & 0xF)<<24); //DownSrcSel, no use in 8530
    WriteREG(0x30F4, u4Reg);

    if(ucOn)
    {
        if (fgNewScale) // MT8563 new added.
        {
            u4Reg420E4 = ReadREG(u4DispFmtRegBase+0xE4);
            u4Reg420B0 = ReadREG(u4DispFmtRegBase+0xB0);

            u4Reg420B0 |= 1 << 0; // HSON, horizontal scaling enable.

            if (0 == (ReadREG(u4DispFmtRegBase + 0x94) &  (0x1 << 14))) //HD_ON
            {
                // SD ouput
                u4Reg420E4 |= 1 << 1; // new scale mode need set 420E4[1]: DIRECT = 1
            }

            u4Reg420E4 |= 1 << 4; // Y_ALL_8TAP_OUT, fro new scale mode, need set to 1.
            u4Reg420E4 |= 1 << 7; // C_ALL_8TAP_OUT, for new scale mode, need set to 1.

            if (fgPreScaleUseLinear)
            {
                u4Reg420E4 &= ~(1 << 8); // HD_C_FIR_EN=0, control C. For new scale mode, both SD and HD need set.
                u4Reg420B0 |= 1 << 1;    // HSLR=1, control Y
            }
            else // 8TAP, DE suggest use 8TAP.
            {
                u4Reg420E4 |= 1 << 8; // HD_C_FIR_EN=1, control C. For new scale mode, both SD and HD need set.
                u4Reg420B0 &= ~(1 << 1); // HSLR=0, control Y
            }

            WriteREG(u4DispFmtRegBase+0xE4, u4Reg420E4);
            WriteREG(u4DispFmtRegBase+0xB0, u4Reg420B0);

            // DO NOT set DOWN_EN to 1.
            // DO NOT NEED set DOWN_444 to 1 after DE change it in next bitfile.[2013-5-13]
            u4Reg4206C = (0x00000800) | (u4Hscale << 16);
        }
        else
        {
            u4Reg4206C = (0x00000801) | (u4Hscale << 16);
        }
    }
    else
    {
        if (fgNewScale)
        {
            u4Reg420E4 = ReadREG(u4DispFmtRegBase+0xE4);
            u4Reg420B0 = ReadREG(u4DispFmtRegBase+0xB0);

            if (0 == (ReadREG(u4DispFmtRegBase + 0x94) &  (0x1 << 14))) //HD_ON
            {
                // SD ouput
                u4Reg420E4 &= ~(1 << 1); // new scale mode need set 420E4[1]: DIRECT = 1
            }

            u4Reg420E4 &= ~(1 << 4); // Y_ALL_8TAP_OUT, fro new scale mode, need set to 1.
            u4Reg420E4 &= ~(1 << 7); // C_ALL_8TAP_OUT, for new scale mode, need set to 1.

            u4Reg420E4 |= 1 << 8; // HD_C_FIR_EN=1, control C. For new scale mode, both SD and HD need set.
            u4Reg420B0 &= ~(1 << 1); // HSLR=0, control Y

            WriteREG(u4DispFmtRegBase+0xE4, u4Reg420E4);
            WriteREG(u4DispFmtRegBase+0xB0, u4Reg420B0);
        }

        u4Reg4206C = 0;
    }

    /*
     * 4206C: Down control
     * [0]   -- DOWN_EN
     * [8]   -- DOWN_422
     * [11]  -- DOWN_444
     * [31:16] -- DOWN_FACTOR
     */
    WriteREG((u4DispFmtRegBase + 0x6C), u4Reg4206C);//4206c[0] down_en [8]down_422 [11] down_444[31:16]down_factor

    if (!fgNewScale
        || (fgNewScale && fgPostScaleUseAcc))
    {
        WriteREGMsk((u4DispFmtRegBase + 0xe4), (1 << 29),  (1 << 29)); // select accumulation mode to zoom out

        // linear mode, do not care this factor.
        u4AccuFactor = (4096*4096)/u4Hscale;
        /*
         * 420CC: Dispfmt configure
         * [12:0]   -- scaler_factor_acc, accumulation mode scaler factor. It's 13 bit, register map define is 12 bit, that's wrong.
         */
        WriteREGMsk((u4DispFmtRegBase + 0xcc), u4AccuFactor, 0x1fff);
    }

    if (fgNewScale)
    {
        u4Reg = 0;

        if (ucOn)
        {
            u4Reg |= 1 << 0;
        }

        if (fgPostScaleUseAcc)
        {
            u4Reg |= 1 << 1;
        }

        /*
         * 420C8: New scale control, [MT8563 new added register]
         * [0]   -- new_scale_mode, 1: enable
         * [1]   -- post_scale_use_acc, 1: accumulation, 0: linear.
         */
        WriteREG((u4DispFmtRegBase + 0xC8), u4Reg); // 420C8
    }


    // If enable shadow register, need trigger.
    if (ReadREG(u4DispFmtRegBase + 0xa0) & 0x20000000)
    {
        WriteREGMsk((u4DispFmtRegBase + 0xa0), (1 << 31),  (1 << 31));
    }

    //  WriteREG(0x420b0, ReadREG(0x420b0)|0x01); //attention, important, but be set to 1 or no video output
    //  WriteREG(0x423b0, ReadREG(0x423b0)|0x01); //attention, important, but be set to 1 or no video output

    //register 0x30ec?

    return;
}

#endif

void vPmxVerifyHalLumaKey(UCHAR ucVdoId, UCHAR ucOn, unsigned int u4LumaKeyVal)
{
    unsigned int u4VdoRegBase, u4DispfmtRegBase, u4VdoutfmtRegBase;
    unsigned int u4Reg;

    u4VdoutfmtRegBase = 0x3000;
    if(ucVdoId == 0)
    {
        u4VdoRegBase = 0x42400;
        u4DispfmtRegBase = 0x42000;
    }
    else if(ucVdoId == 1)
    {
        u4VdoRegBase = 0x43400;
        u4DispfmtRegBase = 0x43000;
    }
    else
    {
        u4VdoRegBase = 0x43a00;
        u4DispfmtRegBase = 0x42000;
    }

    u4Reg = ReadREG(u4VdoutfmtRegBase + 0xFC) & (~0xFF040000);
    u4Reg |= (ucOn << 18);
    u4Reg |= (u4LumaKeyVal << 24);
    WriteREG((u4VdoutfmtRegBase + 0xFC), u4Reg);

    u4Reg = (u4LumaKeyVal << 4);
    WriteREG((u4DispfmtRegBase + 0xFC), u4Reg);

    u4Reg = ReadREG(u4DispfmtRegBase + 0xE4)&(~0x00000008);
    if(ucOn)
    {
        u4Reg |= 0x00000008;
    }
    WriteREG((u4DispfmtRegBase + 0xE4), u4Reg);

    u4Reg = ReadREG(u4VdoRegBase + 0xC8) & (~0x020000FF);
    u4Reg |= (ucOn << 25);
    u4Reg |= (u4LumaKeyVal);
    WriteREG((u4VdoRegBase + 0xC8), u4Reg);

    return;
}


void vPmxVerifyHalNonLinear(UCHAR ucVdoId, UCHAR ucDispFmt)
{
    unsigned int u4DispfmtRegBase;
    unsigned int u4Reg420E4;

    if(ucVdoId == 0)
    {
        u4DispfmtRegBase = 0x42000;
    }
    else if(ucVdoId == 1)
    {
        u4DispfmtRegBase = 0x43000;
    }
    else
    {
        u4DispfmtRegBase = 0x42000;
    }
    if(ucDispFmt) //dispfmt nonlinear stretc
    {
        /* It's good habit to write register with mask. */
        // MT8563 new change: 2D 1920P50/60 output need set 420E4[6] = 1,
        // It has been updated in array u4DispFPGAInitVdo1Regs1080p_src1080p.
        // If write with mask, it is keeped and no need set again here.
        u4Reg420E4 = ReadREG(u4DispfmtRegBase + 0xE4);
        u4Reg420E4 |= 1 << 0; // MULTI_RATIO
        u4Reg420E4 |= 1 << 5; // FACT_PREC.
        u4Reg420E4 &= ~(1 << 7); // C_ALL_8TAP_OUT

        WriteREG((u4DispfmtRegBase + 0xE4), u4Reg420E4);
        // 8563 CACC_ST should bee set to 0.
        WriteREG((u4DispfmtRegBase + 0xB0), 0x08000001);
        WriteREG((u4DispfmtRegBase + 0xD8), 0x0223A2BA);
        WriteREG((u4DispfmtRegBase + 0xDC), 0x205FA67A);
    }
    else //scaler nonlinear stretch
    {
        // CACC_ST = 0
        WriteREG((u4DispfmtRegBase + 0xB0), 0x04000000); //420b0[0] horizontal scaling enable

        /* It's good habit to write register with mask. */
        // MT8563 new change: 2D 1920P50/60 output need set 420E4[6] = 1,
        // It has been updated when in setmode.
        // If write with mask, it is keeped and no need set again here.
        u4Reg420E4 = ReadREG(u4DispfmtRegBase + 0xE4);
        u4Reg420E4 &= ~(1 << 0); // MULTI_RATIO
        WriteREG((u4DispfmtRegBase + 0xE4), u4Reg420E4);
        // CACC_ST = 0
        WriteREG((u4DispfmtRegBase + 0xB0), 0x06000001);
    }
}

void vPmxVerifyHalShiftLine(UCHAR ucVdoId, UCHAR ucHD, UCHAR ucLine)
{
    unsigned int u4VdoRegBase = 0;
    if(ucVdoId == 0)
    {
        u4VdoRegBase = 0x42400;
    }
    else if(ucVdoId == 1)
    {
        u4VdoRegBase = 0x43400;
    }
    else
    {
        u4VdoRegBase = 0x43a00;
    }

    if(ucHD)
    {
        switch(ucLine)
        {
        case 149:
            WriteREG((u4VdoRegBase + 0x20), 0x008F008E);
            WriteREG((u4VdoRegBase + 0x24), 0x00450046);
            WriteREG((u4VdoRegBase + 0x28), 0xC0004080);
            WriteREG((u4VdoRegBase + 0x2C), 0x40A0C020);
            WriteREG((u4VdoRegBase + 0x50), 0x008E008F);
            WriteREG((u4VdoRegBase + 0x54), 0x00460045);
            WriteREG((u4VdoRegBase + 0x14), 0x000002AA);
            break;
        case 150:
            WriteREG((u4VdoRegBase + 0x20), 0x008F0090);
            WriteREG((u4VdoRegBase + 0x24), 0x00490048);
            WriteREG((u4VdoRegBase + 0x28), 0x4080C000);
            WriteREG((u4VdoRegBase + 0x2C), 0x80E00060);
            WriteREG((u4VdoRegBase + 0x50), 0x0090008F);
            WriteREG((u4VdoRegBase + 0x54), 0x00480047);
            WriteREG((u4VdoRegBase + 0x14), 0x000002AA);
            break;
        case 151:
            WriteREG((u4VdoRegBase + 0x20), 0x00910090);
            WriteREG((u4VdoRegBase + 0x24), 0x00490048);
            WriteREG((u4VdoRegBase + 0x28), 0xC0004080);
            WriteREG((u4VdoRegBase + 0x2C), 0xC02040A0);
            WriteREG((u4VdoRegBase + 0x50), 0x00900091);
            WriteREG((u4VdoRegBase + 0x54), 0x00480049);
            WriteREG((u4VdoRegBase + 0x14), 0x000002AA);
            break;
        case 152:
            WriteREG((u4VdoRegBase + 0x20), 0x00910092);
            WriteREG((u4VdoRegBase + 0x24), 0x00490048);
            WriteREG((u4VdoRegBase + 0x28), 0x4080C000);
            WriteREG((u4VdoRegBase + 0x2C), 0xC06080E0);
            WriteREG((u4VdoRegBase + 0x50), 0x00920091);
            WriteREG((u4VdoRegBase + 0x54), 0x004A0049);
            WriteREG((u4VdoRegBase + 0x14), 0x000002AA);
            break;
        default:
            break;
        }
    }
    else
    {
        switch(ucLine)
        {
        case 149:
            WriteREG((u4VdoRegBase + 0x20), 0x00910090);
            WriteREG((u4VdoRegBase + 0x24), 0x00490048);
            WriteREG((u4VdoRegBase + 0x28), 0xA9602A80);
            WriteREG((u4VdoRegBase + 0x2C), 0xB52035A0);
            WriteREG((u4VdoRegBase + 0x50), 0x00900091);
            WriteREG((u4VdoRegBase + 0x54), 0x00480049);
            WriteREG((u4VdoRegBase + 0x14), 0x000002AA);
            break;
        case 150:
            WriteREG((u4VdoRegBase + 0x20), 0x00910092);
            WriteREG((u4VdoRegBase + 0x24), 0x00490048);
            WriteREG((u4VdoRegBase + 0x28), 0x2A80AA00);
            WriteREG((u4VdoRegBase + 0x2C), 0xF56075E0);
            WriteREG((u4VdoRegBase + 0x50), 0x00920091);
            WriteREG((u4VdoRegBase + 0x54), 0x00480049);
            WriteREG((u4VdoRegBase + 0x14), 0x000002AA);
            break;
        case 151:
            WriteREG((u4VdoRegBase + 0x20), 0x00930092);
            WriteREG((u4VdoRegBase + 0x24), 0x00470048);
            WriteREG((u4VdoRegBase + 0x28), 0xAA002A80);
            WriteREG((u4VdoRegBase + 0x2C), 0x35A0B520);
            WriteREG((u4VdoRegBase + 0x50), 0x00920093);
            WriteREG((u4VdoRegBase + 0x54), 0x00480047);
            WriteREG((u4VdoRegBase + 0x14), 0x000002AA);
            break;
        case 152:
            WriteREG((u4VdoRegBase + 0x20), 0x00930094);
            WriteREG((u4VdoRegBase + 0x24), 0x0049004A);
            WriteREG((u4VdoRegBase + 0x28), 0x2A80AA00);
            WriteREG((u4VdoRegBase + 0x2C), 0x75E0F560);
            WriteREG((u4VdoRegBase + 0x50), 0x00940093);
            WriteREG((u4VdoRegBase + 0x54), 0x004A0049);
            WriteREG((u4VdoRegBase + 0x14), 0x000002AA);
            break;
        default:
            break;
        }
    }
}

void vPmxVerifyHalDispClearChkSum(UCHAR ucVdoId)
{
    unsigned int u4DispfmtRegBase;

    if(ucVdoId == 0)
    {
        u4DispfmtRegBase = 0x42000;
    }
    else if(ucVdoId == 1)
    {
        u4DispfmtRegBase = 0x43000;
    }
    else
    {
        u4DispfmtRegBase = 0x42000;
    }
    WriteREG((u4DispfmtRegBase + 0x50), (0x1 << 1));
}

void vPmxVerifyHalDispInitChkSum(UCHAR ucVdoId)
{
    unsigned int u4DispfmtRegBase;
    if(ucVdoId == 0)
    {
        u4DispfmtRegBase = 0x42000;
    }
    else if(ucVdoId == 1)
    {
        u4DispfmtRegBase = 0x43000;
    }
    else
    {
        u4DispfmtRegBase = 0x42000;
    }
    WriteREG((u4DispfmtRegBase + 0x50), (0x1 << 0));
}

unsigned int u4PmxVerifyHalDispGetChkSum(UCHAR ucVdoId)
{
    unsigned int u4DispfmtRegBase, u4ReturnVal;
    if(ucVdoId == 0)
    {
        u4DispfmtRegBase = 0x42000;
    }
    else if(ucVdoId == 1)
    {
        u4DispfmtRegBase = 0x43000;
    }
    else
    {
        u4DispfmtRegBase = 0x42000;
    }

    u4ReturnVal = ReadREG(u4DispfmtRegBase + 0x50) >> 8;
    return (u4ReturnVal);
}


void vPmxVerifyHalSetDataSource(UCHAR sel_hdmi, UCHAR sel_cav, UCHAR sel_cvbs)
{
    unsigned int VdoutRegBase = 0x3500;

    unsigned int old_value = ReadREG(VdoutRegBase + 0x4);

    old_value &= ~(0x0000003d);
    old_value |= ((unsigned int) sel_hdmi << 4)|((unsigned int) sel_cav << 2)|((unsigned int) sel_cvbs);

    WriteREG(VdoutRegBase + 0x4, old_value);
}

void vPmxVerifyHalSetClockSource(UCHAR sel_hdmi, UCHAR sel_cav, UCHAR sel_cvbs)
{
    unsigned int VdoutRegBase = 0x3500;

    unsigned int old_value = ReadREG(VdoutRegBase + 0x8);

    old_value &= ~(0x00000700);
    old_value |= ((unsigned int) sel_hdmi << 10)|((unsigned int) sel_cav << 9)|((unsigned int) sel_cvbs << 8);

    WriteREG(VdoutRegBase + 0x8, old_value);
}

void vPmxVerifyHalSetScalerClkSet(UCHAR vclk, UCHAR hclk)
{
    unsigned int VdoutRegBase = 0x3500;

    unsigned int old_value = ReadREG(VdoutRegBase + 0x8);

    old_value &= ~(0x20000000);
    old_value |= ((unsigned int) vclk << 29);

    WriteREG(VdoutRegBase + 0x8, old_value);
}

void PmxVerifyHalFixCVBS(void)
{
    unsigned int v;

    v = ReadREG(0x3c1c);

    v |= 0x00000001;

    WriteREG(0x3c1c, v);

    v &= ~1;

    WriteREG(0x3c1c, v);
}

void vPmxVerifyHalFmt2Timing(unsigned int u4PmxMode)
{
    switch (u4PmxMode)
    {
    case RES_1080P60HZ:
    case RES_1080P50HZ: //148Hz
        WriteREGMsk(0x3578, (1<<12)|(1<<13), (1<<12)|(1<<13));
        break;

    case RES_480P:
    case RES_576P: //27Hz
        WriteREGMsk(0x3578, 0, (1<<12)|(1<<13));
        break;

    default: //74Hz
        WriteREGMsk(0x3578, (1<<12), (1<<12)|(1<<13));
        break;
    }

    WriteREG(0x3394, ReadREG(0x3094));
    WriteREGMsk(0x33AC, 3, 3);
    //WriteREGMsk(0x359c, 0, (1<<8)|(1<<9));

    WriteREGMsk(0x30AC, 0, 1<<10);
    WriteREGMsk(0x30AC, 1<<10, 1<<10);
    WriteREGMsk(0x30AC, 0, 1<<10);
    WriteREGMsk(0x33AC, 0, 1<<10);
    WriteREGMsk(0x33AC, 1<<10, 1<<10);
    WriteREGMsk(0x33AC, 0, 1<<10);
}

void vPmxVerifyHalModeSel(unsigned int u4PmxFmtNo, unsigned int u4Rbg2HdmiNo)
{
    if (!u4PmxFmtNo)
    {
        WriteREGMsk(0x359c, (1<<8)|(1<<9), (1<<8)|(1<<9));
    }
    else
    {
        WriteREGMsk(0x359c, 0, (1<<8)|(1<<9));
    }

    if (!u4Rbg2HdmiNo)
    {
        WriteREGMsk(0x3528, (1<<1)|(1<<2), (1<<1)|(1<<2));
    }
    else
    {
        WriteREGMsk(0x3528, 0, (1<<1)|(1<<2));
    }
}

void PmxVerifyHal480iExtra(UCHAR ucVdoId)
{
    unsigned int u4VdoRegBase, u4DispfmtRegBase;

    if(ucVdoId == 0)
    {
        //u4VdoutfmtRegBase = 0x3000;
        u4DispfmtRegBase = 0x42000;
        u4VdoRegBase = 0x42400;
    }
    else if(ucVdoId == 1)
    {
        //u4VdoutfmtRegBase = 0x3300;
        u4DispfmtRegBase = 0x43000;
        u4VdoRegBase = 0x43400;
    }
    else
    {
        //u4VdoutfmtRegBase = 0x3000;
        u4DispfmtRegBase = 0x42000;
        u4VdoRegBase = 0x43a00;
        //return; //error
    }
    
    WriteREG((u4DispfmtRegBase + 0xa0), 0x00ee068d);
  // WriteREG((u4DispfmtRegBase + 0xa4), 0x00150104);
    WriteREG((u4DispfmtRegBase + 0xa4), 0x00140103);  
    WriteREG((u4DispfmtRegBase + 0xa8), 0x011c020b);
    //WriteREG((u4VdoutfmtRegBase + 0xa0), 0x00c10660);
    //WriteREG((u4VdoutfmtRegBase + 0xa4), 0x00150104);
    //WriteREG((u4VdoutfmtRegBase + 0xa8), 0x011c020b);
    WriteREG((u4DispfmtRegBase + 0xa0), 0x00CF066E);
    //WriteREG((u4VdoutfmtRegBase + 0xa0), 0x00EF068E);

    WriteREG(u4DispfmtRegBase+0x94, 0x00000820);
    //WriteREG(u4DispfmtRegBase+0xe8, 0x80020677);
    WriteREGMsk((u4DispfmtRegBase + 0xe4), (0x1 << 28), (0x1 << 6) | (0x1 << 28));
    WriteREG((u4DispfmtRegBase + 0xc4), 0x880010eb);
    //WriteREG((u4VdoRegBase + 0x14), 0x00001000);
    WriteREG(u4DispfmtRegBase+0x94, 0x00000820);
    //WriteREG(0x3094, 0x00000820);
    //WriteREGMsk(0x20678, 0, (0x1 << 9));
    //WriteREG(0x3504, 0);
    //WriteREG(0x3820, 0x00ef068e);
    //WriteREG(0x3824, 0x00150104);
    //WriteREG(0x3828, 0x011c020b);
    //WriteREG(0x38b4, 0x00010007);
    //WriteREG(0x38b4, 0x0001002f);
    //WriteREG(u4DispfmtRegBase+0xe8, 0x00010030);
    WriteREG(u4DispfmtRegBase+0xe8, 0x00010001);
    //WriteREG(0x423e8, 0x800206A4);
}

void PmxVerifyHal576iExtra(UCHAR ucVdoId)
{
    unsigned int u4VdoRegBase, u4DispfmtRegBase;

    if(ucVdoId == 0)
    {
        //u4VdoutfmtRegBase = 0x3000;
        u4DispfmtRegBase = 0x42000;
        u4VdoRegBase = 0x42400;
    }
    else if(ucVdoId == 1)
    {
        //u4VdoutfmtRegBase = 0x3300;
        u4DispfmtRegBase = 0x43000;
        u4VdoRegBase = 0x43400;
    }
    else
    {
        //u4VdoutfmtRegBase = 0x3000;
        u4DispfmtRegBase = 0x42000;
        u4VdoRegBase = 0x43a00;
        //return; //error
    }

    WriteREG((u4DispfmtRegBase + 0xa0), 0x00ef068e);
    WriteREG((u4DispfmtRegBase + 0xa4), 0x00170136);
    WriteREG((u4DispfmtRegBase + 0xa8), 0x0150026f);
    //WriteREG((u4VdoutfmtRegBase + 0xa0), 0x00ef068e);
    //WriteREG((u4VdoutfmtRegBase + 0xa4), 0x00170136);
    //WriteREG((u4VdoutfmtRegBase + 0xa8), 0x0150026f);
    
    WriteREG((u4DispfmtRegBase + 0xa0), 0xa0e6068f); //0x00ef068e -->e6068f
    WriteREG((0x2004), 0xa190e00); //add only for 576i fullscreen.

    //WriteREG(u4DispfmtRegBase+0x94, 0x00000820);
    WriteREG(u4DispfmtRegBase+0xe8, 0x00010001); //  0x00010030 --> 0x00010001
    WriteREGMsk((u4DispfmtRegBase + 0xe4), (0x1 << 28), (0x1 << 6) | (0x1 << 28));
    WriteREG((u4DispfmtRegBase + 0xc4), 0x880010eb);
    //WriteREG((u4VdoRegBase + 0x14), 0x00001000);
    //WriteREG(0x3094, 0x80000820);
    WriteREGMsk(u4DispfmtRegBase+0x94, 0x0820,0xffff);
    //WriteREGMsk(0x20678, 0, (0x1 << 9));
    //WriteREG(0x3504, 0);
    //WriteREG(0x3820, 0x00ef068e);
    //WriteREG(0x3824, 0x00170136);
    //WriteREG(0x3828, 0x0150026f);
    //WriteREG(0x38b4, 0x00010007);
    //mtk40568
    //WriteREG(0x38b4, 0x0001002f);
}

void PmxVerifyHal480PSD144Mode(void){
	WriteREG(0x43094, 0x18008d20);
	WriteREG(0x43124, 0x00008d20);

	//WriteREG(0x430e8, 0x08300a08);
	//WriteREG(0x430ec, 0x04180357);

	WriteREG(0x430e8, 0x083009c8);
	WriteREG(0x430ec, 0x0418034b);

	
	WriteREG(0x4309c, 0x000002d0);
	WriteREG(0x430a0, 0x01680438);
	WriteREG(0x430a4, 0x002b020a);
	WriteREG(0x430a8, 0x002b020a);
	WriteREG(0x430e4, 0x00000020);
	WriteREG(0x430d4, 0x035a220d);
	WriteREG(0x43070, 0x016d043d); //
	WriteREG(0x43074, 0x002b020c);
	WriteREG(0x43078, 0x002b020c);
	WriteREG(0x4307c, 0x0264b354);
	WriteREG(0x430c0, 0x00780347);
	WriteREG(0x4306c, 0x10000820);
		
}


void PmxVerifyHal1080iExtra(UCHAR ucVdoId)
{
    unsigned int u4VdoRegBase, u4DispfmtRegBase;
	
    if(ucVdoId == 0)
    {
        //u4VdoutfmtRegBase = 0x3000;
        u4DispfmtRegBase = 0x42000;
        u4VdoRegBase = 0x42400;
    }
    else if(ucVdoId == 1)
    {
        //u4VdoutfmtRegBase = 0x3300;
        u4DispfmtRegBase = 0x43000;
        u4VdoRegBase = 0x43400;
    }
    else
    {
        //u4VdoutfmtRegBase = 0x3000;
        u4DispfmtRegBase = 0x42000;
        u4VdoRegBase = 0x43a00;
        //return; //error
    }

    WriteREG((u4DispfmtRegBase + 0xa0), 0x00c10840);
    WriteREG((u4DispfmtRegBase + 0xa4), 0x00150230);
    WriteREG((u4DispfmtRegBase + 0xa8), 0x02480463);
    //WriteREG((u4VdoutfmtRegBase + 0xa0), 0x00c10840);
    //WriteREG((u4VdoutfmtRegBase + 0xa4), 0x00150230);
    //WriteREG((u4VdoutfmtRegBase + 0xa8), 0x02480463);

    //WriteREG(u4DispfmtRegBase+0x94, 0x00004820);
    //WriteREG(u4DispfmtRegBase+0xe8, 0x08C8088B);
    WriteREGMsk((u4DispfmtRegBase + 0xe4), (0x1 << 6), (0x1 << 6));
    //WriteREG((u4VdoRegBase + 0x14), 0x00001000);
    //WriteREG(u4DispfmtRegBase+0x94, 0x00004820);
    WriteREGMsk(u4DispfmtRegBase+0x94,0x00004820,0xffff);
	
    //WriteREG(0x3504, 0x301);
    //WriteREGMsk(0x3508, (0x1 << 30), (0x1 << 30));
    //WriteREG(0x3820, 0x00c10840);
    //WriteREG(0x3824, 0x00150230);
    //WriteREG(0x3828, 0x02480463);
    //WriteREG(0x38b4, 0x00010007);
   // WriteREG(0x38b4, 0x0001002E);
}
void PmxVerifyHal480I3dTiming(UCHAR ucVdoId)
{
    unsigned int u4HdmiRegBase=0x3800;
    WriteREG((u4HdmiRegBase + 0x20), 0x00EF068E);
    WriteREG((u4HdmiRegBase + 0x24), 0x00150418);
    WriteREG((u4HdmiRegBase + 0x28), 0x00150418);

    WriteREG(0x3084, 0x86b441a1);  //fmt master mode
    WriteREG(0x3504, 0x00000400);  //hdmi master mode
    WriteREG(0x352c, 0x00004827);  //switch syncs to RGB2HDMI output

    WriteREG(0x3024, 0x00400000);  //test 3d mode


    WriteREGMsk(0x3508, (0x0<<11), (0x1 << 11));  // hdmi_prgs27m
    WriteREGMsk(0x350c, (0x1<<9), (0x1 << 9));  // hdmi_hdaud
    WriteREGMsk(0x351c, ((((ReadREG(0x351c) & 0x30000000)>>28)-1)<<28), (0x3 << 28));  //120hz enable
    WriteREGMsk(0x352c, (0x1<<0), (0x1 << 0));  //120hz enable
    WriteREGMsk(0x352c, (0x1<<14), (0x1 << 14));  //set fmtter clock to 120hz mode

    WriteREG(0x30ac, 0x08000003);
    WriteREG(0x30ac, 0x08000403);
    WriteREG(0x30ac, 0x08000003);
    WriteREGMsk(0x3084, (0x0 << 30), (0x1 << 30));
    WriteREGMsk(0x3084, (0x1 << 30), (0x1 << 30));
    WriteREGMsk(0x3084, (0x0 << 30), (0x1 << 30));

}
void PmxVerifyHal480P3dTiming(UCHAR ucVdoId)
{
    unsigned int u4HdmiRegBase=0x3800;
    WriteREG((u4HdmiRegBase + 0x20), 0x007B034A);
    WriteREG((u4HdmiRegBase + 0x24), 0x002b0417);
    WriteREG((u4HdmiRegBase + 0x28), 0x002b0417);

    WriteREG(0x3084, 0x835a41a9);  //fmt master mode
    WriteREG(0x3504, 0x00000400);  //hdmi master mode
    WriteREG(0x352c, 0x00004827);  //switch syncs to RGB2HDMI output

    WriteREG(0x3024, 0x00400000);  //test 3d mode


    WriteREGMsk(0x3508, (0x0<<11), (0x1 << 11));  // hdmi_prgs27m
    WriteREGMsk(0x351c, ((((ReadREG(0x351c) & 0x30000000)>>28)-1)<<28), (0x3 << 28));  //120hz enable
    WriteREGMsk(0x352c, (0x1<<0), (0x1 << 0));  //120hz enable
    WriteREGMsk(0x352c, (0x1<<14), (0x1 << 14));  //set fmtter clock to 120hz mode

    WriteREG(0x30ac, 0x08000003);
    WriteREG(0x30ac, 0x08000403);
    WriteREG(0x30ac, 0x08000003);
    WriteREGMsk(0x3084, (0x0 << 30), (0x1 << 30));
    WriteREGMsk(0x3084, (0x1 << 30), (0x1 << 30));
    WriteREGMsk(0x3084, (0x0 << 30), (0x1 << 30));

}
void PmxVerifyHal576I3dTiming(UCHAR ucVdoId)
{
    unsigned int u4HdmiRegBase=0x3800;
    WriteREG((u4HdmiRegBase + 0x20), 0x00EF068E);
    WriteREG((u4HdmiRegBase + 0x24), 0x001704e0);
    WriteREG((u4HdmiRegBase + 0x28), 0x001704e0);

    WriteREG(0x3084, 0x86c04e21);  //fmt master mode
    WriteREG(0x3504, 0x00000400);  //hdmi master mode
    WriteREG(0x352c, 0x00004827);  //switch syncs to RGB2HDMI output

    WriteREG(0x3024, 0x00400000);  //test 3d mode


    WriteREGMsk(0x3508, (0x0<<11), (0x1 << 11));  // hdmi_prgs27m
    WriteREGMsk(0x350c, (0x1<<9), (0x1 << 9));  // hdmi_hdaud
    WriteREGMsk(0x351c, ((((ReadREG(0x351c) & 0x30000000)>>28)-1)<<28), (0x3 << 28));  //120hz enable
    WriteREGMsk(0x352c, (0x1<<0), (0x1 << 0));  //120hz enable
    WriteREGMsk(0x352c, (0x1<<14), (0x1 << 14));  //set fmtter clock to 120hz mode

    WriteREG(0x30ac, 0x08000003);
    WriteREG(0x30ac, 0x08000403);
    WriteREG(0x30ac, 0x08000003);
    WriteREGMsk(0x3084, (0x0 << 30), (0x1 << 30));
    WriteREGMsk(0x3084, (0x1 << 30), (0x1 << 30));
    WriteREGMsk(0x3084, (0x0 << 30), (0x1 << 30));

}
void PmxVerifyHal576P3dTiming(UCHAR ucVdoId)
{
    unsigned int u4HdmiRegBase=0x3800;
    WriteREG((u4HdmiRegBase + 0x20), 0x00850354);
    WriteREG((u4HdmiRegBase + 0x24), 0x002d04dc);
    WriteREG((u4HdmiRegBase + 0x28), 0x002d04dc);

    WriteREG(0x3084, 0x83604e29);  //fmt master mode
    WriteREG(0x3504, 0x00000400);  //hdmi master mode
    WriteREG(0x352c, 0x00004827);  //switch syncs to RGB2HDMI output

    WriteREG(0x3024, 0x00400000);  //test 3d mode

    WriteREGMsk(0x3508, (0x0<<11), (0x1 << 11));  // hdmi_prgs27m
    WriteREGMsk(0x351c, ((((ReadREG(0x351c) & 0x30000000)>>28)-1)<<28), (0x3 << 28));  //120hz enable
    WriteREGMsk(0x352c, (0x1<<0), (0x1 << 0));  //120hz enable
    WriteREGMsk(0x352c, (0x1<<14), (0x1 << 14));  //set fmtter clock to 120hz mode

    WriteREG(0x30ac, 0x08000003);
    WriteREG(0x30ac, 0x08000403);
    WriteREG(0x30ac, 0x08000003);
    WriteREGMsk(0x3084, (0x0 << 30), (0x1 << 30));
    WriteREGMsk(0x3084, (0x1 << 30), (0x1 << 30));
    WriteREGMsk(0x3084, (0x0 << 30), (0x1 << 30));

}
void PmxVerifyHal720P60HZ3dTiming(UCHAR ucVdoId)
{
    unsigned int u4HdmiRegBase=0x3800;

    WriteREG(0x3084, 0x86725dcf);  //fmt master mode
    WriteREG(0x3504, 0x00000400);  //hdmi master mode
    WriteREGMsk(0x351c, ((((ReadREG(0x351c) & 0x30000000)>>28)-1)<<28), (0x3 << 28));  //120hz enable

    WriteREGMsk(0x3508, (0x0<<30), (0x1 << 30));//vdout_fmt_2fs clock 148mhz

    WriteREGMsk(0x3508, (0x1<<15), (0x1 << 15)); //enable self_option_hdmi
    WriteREGMsk(0x3508, (0x7<<20), (0x7 << 20)); //3508[20]hdmi_hd 3508[22]hdmi_1080p 148mhz

    WriteREG((u4HdmiRegBase + 0x20), 0x01050604);
    WriteREG((u4HdmiRegBase + 0x24), 0x001A05d7);
    WriteREG((u4HdmiRegBase + 0x28), 0x001A05d7);

    WriteREGMsk(0x3508, (0x1<<16), (0x1 << 16)); //mvdo_hd_clk
    WriteREGMsk(0x3508, (0x1<<23), (0x1 << 23));

    WriteREGMsk(0x420e4, (0x1<<6), (0x1 << 6)); //disfmt path set
    WriteREG(0x3024, 0x00400000);  //test 3d mode

    WriteREG(0x30ac, 0x08000003);
    WriteREG(0x30ac, 0x08000403);
    WriteREG(0x30ac, 0x08000003);
    WriteREGMsk(0x3084, (0x0 << 30), (0x1 << 30));
    WriteREGMsk(0x3084, (0x1 << 30), (0x1 << 30));
    WriteREGMsk(0x3084, (0x0 << 30), (0x1 << 30));
}
void PmxVerifyHal720P50HZ3dTiming(UCHAR ucVdoId)
{

    unsigned int u4HdmiRegBase=0x3800;

    WriteREG(0x3084, 0x87bc5dcf);  //fmt master mode
    WriteREG(0x3504, 0x00000400);  //hdmi master mode
    WriteREGMsk(0x351c, ((((ReadREG(0x351c) & 0x30000000)>>28)-1)<<28), (0x3 << 28));  //120hz enable

    WriteREGMsk(0x3508, (0x0<<30), (0x1 << 30));//vdout_fmt_2fs clock 148mhz

    WriteREGMsk(0x3508, (0x1<<15), (0x1 << 15)); //enable self_option_hdmi
    WriteREGMsk(0x3508, (0x7<<20), (0x7 << 20)); //3508[20]hdmi_hd 3508[22]hdmi_1080p 148mhz

    WriteREG((u4HdmiRegBase + 0x20), 0x01050604);
    WriteREG((u4HdmiRegBase + 0x24), 0x001A05d7);
    WriteREG((u4HdmiRegBase + 0x28), 0x001A05d7);

    WriteREGMsk(0x3508, (0x1<<16), (0x1 << 16)); //mvdo_hd_clk
    WriteREGMsk(0x3508, (0x1<<23), (0x1 << 23));

    WriteREGMsk(0x420e4, (0x1<<6), (0x1 << 6)); //disfmt path set
    WriteREG(0x3024, 0x00400000);  //test 3d mode

    WriteREG(0x30ac, 0x08000003);
    WriteREG(0x30ac, 0x08000403);
    WriteREG(0x30ac, 0x08000003);
    WriteREGMsk(0x3084, (0x0 << 30), (0x1 << 30));
    WriteREGMsk(0x3084, (0x1 << 30), (0x1 << 30));
    WriteREGMsk(0x3084, (0x0 << 30), (0x1 << 30));

}
void PmxVerifyHal1080I60HZ3dTiming(UCHAR ucVdoId)
{
    unsigned int u4HdmiRegBase=0x3800;

    WriteREG(0x3084, 0x88988cab);  //fmt master mode
    WriteREG(0x3504, 0x00000400);  //hdmi master mode

    WriteREGMsk(0x351c, ((((ReadREG(0x351c) & 0x30000000)>>28)-1)<<28), (0x3 << 28));  //120hz enable
    WriteREGMsk(0x3508, (0x0<<30), (0x1 << 30));//vdout_fmt_2fs clock 148mhz

    WriteREGMsk(0x3508, (0x1<<15), (0x1 << 15)); //enable self_option_hdmi
    WriteREGMsk(0x3508, (0x7<<20), (0x7 << 20)); //3508[20]hdmi_hd 3508[22]hdmi_1080p 148mhz

    WriteREG((u4HdmiRegBase + 0x20), 0x00C10840);
    WriteREG((u4HdmiRegBase + 0x24), 0x001508c8);
    WriteREG((u4HdmiRegBase + 0x28), 0x001508c8);

    WriteREGMsk(0x3508, (0x1<<16), (0x1 << 16)); //mvdo_hd_clk
    WriteREGMsk(0x3508, (0x1<<23), (0x1 << 23));

    WriteREGMsk(0x420e4, (0x1<<6), (0x1 << 6)); //disfmt path set
    WriteREG(0x3024, 0x00400000);  //test 3d mode

    WriteREG(0x30ac, 0x08000003);
    WriteREG(0x30ac, 0x08000403);
    WriteREG(0x30ac, 0x08000003);
    WriteREGMsk(0x3084, (0x0 << 30), (0x1 << 30));
    WriteREGMsk(0x3084, (0x1 << 30), (0x1 << 30));
    WriteREGMsk(0x3084, (0x0 << 30), (0x1 << 30));

}
void PmxVerifyHal1080I50HZ3dTiming(UCHAR ucVdoId)
{

    unsigned int u4HdmiRegBase=0x3800;

    WriteREG(0x3084, 0x8a508ca3);  //fmt master mode
    WriteREG(0x3504, 0x00000400);  //hdmi master mode

    WriteREGMsk(0x351c, ((((ReadREG(0x351c) & 0x30000000)>>28)-1)<<28), (0x3 << 28));  //120hz enable
    WriteREGMsk(0x3508, (0x0<<30), (0x1 << 30));//vdout_fmt_2fs clock 148mhz

    WriteREGMsk(0x3508, (0x1<<15), (0x1 << 15)); //enable self_option_hdmi
    WriteREGMsk(0x3508, (0x7<<20), (0x7 << 20)); //3508[20]hdmi_hd 3508[22]hdmi_1080p 148mhz

    WriteREG((u4HdmiRegBase + 0x20), 0x00C10840);
    WriteREG((u4HdmiRegBase + 0x24), 0x002a08c6);
    WriteREG((u4HdmiRegBase + 0x28), 0x002a08c6);

    WriteREGMsk(0x3508, (0x1<<16), (0x1 << 16)); //mvdo_hd_clk
    WriteREGMsk(0x3508, (0x1<<23), (0x1 << 23));

    WriteREGMsk(0x420e4, (0x1<<6), (0x1 << 6)); //disfmt path set
    WriteREG(0x3024, 0x00400000);  //test 3d mode

    WriteREG(0x30ac, 0x08000003);
    WriteREG(0x30ac, 0x08000403);
    WriteREG(0x30ac, 0x08000003);
    WriteREGMsk(0x3084, (0x0 << 30), (0x1 << 30));
    WriteREGMsk(0x3084, (0x1 << 30), (0x1 << 30));
    WriteREGMsk(0x3084, (0x0 << 30), (0x1 << 30));


}
void PmxVerifyHal1080P25HZ3dTiming(UCHAR ucVdoId)
{}
void PmxVerifyHal1080P24HZ3dTiming(UCHAR ucVdoId)
{
    unsigned int u4HdmiRegBase=0x3800;

    WriteREG(0x3084, 0x8abe8cab);  //fmt master mode
    WriteREG(0x3504, 0x00000400);  //hdmi master mode
    WriteREG(0x30d4, 0x8abe0465);
    WriteREGMsk(0x351c, ((((ReadREG(0x351c) & 0x30000000)>>28)-1)<<28), (0x3 << 28));  //120hz enable
    WriteREGMsk(0x3508, (0x0<<30), (0x1 << 30));//vdout_fmt_2fs clock 148mhz

    WriteREGMsk(0x3508, (0x1<<15), (0x1 << 15)); //enable self_option_hdmi
    WriteREGMsk(0x3508, (0x7<<20), (0x7 << 20)); //3508[20]hdmi_hd 3508[22]hdmi_1080p 148mhz

    WriteREG((u4HdmiRegBase + 0x20), 0x00C10840);
    WriteREG((u4HdmiRegBase + 0x24), 0x002A08c6);
    WriteREG((u4HdmiRegBase + 0x28), 0x002A08c6);

    WriteREGMsk(0x3508, (0x1<<16), (0x1 << 16)); //mvdo_hd_clk
    WriteREGMsk(0x3508, (0x1<<23), (0x1 << 23));

    WriteREGMsk(0x420e4, (0x1<<6), (0x1 << 6)); //disfmt path set
    WriteREG(0x3024, 0x00400000);  //test 3d mode

    WriteREG(0x30ac, 0x08000003);
    WriteREG(0x30ac, 0x08000403);
    WriteREG(0x30ac, 0x08000003);
    WriteREGMsk(0x3084, (0x0 << 30), (0x1 << 30));
    WriteREGMsk(0x3084, (0x1 << 30), (0x1 << 30));
    WriteREGMsk(0x3084, (0x0 << 30), (0x1 << 30));


}
void PmxVerifyHal1080P30HZ3dTiming(UCHAR ucVdoId)
{}


void vPmxVerifyHalSetVHLnrMode(unsigned int u4VdoId, unsigned int u4VLnrOn, unsigned int u4HLnrOn)
{
    unsigned int u4DispFmtRegBase = 0;
    unsigned int u4V8RegBase = 0;

    UTIL_Printf("[PMX] VDO%d V Lnr:%s, H Lnr:%s\n", u4VdoId, (u4VLnrOn ? "On" : "Off"), (u4HLnrOn ? "On" : "Off"));

    if(u4VdoId == 0)
    {
        u4V8RegBase = 0x42200;
        u4DispFmtRegBase = 0x42000;
    }
    else if(u4VdoId == 1)
    {
        u4V8RegBase = 0x43200;
        u4DispFmtRegBase = 0x43000;
    }
    else
    {
        u4V8RegBase = 0x42200;
        u4DispFmtRegBase = 0x42000;
    }

    if (u4VLnrOn)
    {
        WriteREGMsk((u4V8RegBase + 0x4c), (0x3<<26), (0x3<<26));
    }
    else
    {
        WriteREGMsk((u4V8RegBase + 0x4c), 0, (0x3<<26));
    }

    if (u4HLnrOn)
    {
        WriteREGMsk((u4DispFmtRegBase + 0xe4), 0, (0x1<<8));
        WriteREGMsk((u4DispFmtRegBase + 0xb0), (0x1<<1), (0x1<<1));
    }
    else
    {
        WriteREGMsk((u4DispFmtRegBase + 0xe4), (0x1<<8), (0x1<<8));
        WriteREGMsk((u4DispFmtRegBase + 0xb0), 0, (0x1<<1));
    }
}


void vPmxVerifyHalDering(unsigned int ucVdoId, unsigned int u4Enable, unsigned int u4ThredY, unsigned int u4ThredC, unsigned int u4TransY)
{
    unsigned int u4DispfmtRegBase;

    if(ucVdoId == 0)
    {
        u4DispfmtRegBase = 0x42000;
    }
    else if(ucVdoId == 1)
    {
        u4DispfmtRegBase = 0x43000;
    }
    else
    {
        u4DispfmtRegBase = 0x42000;
    }

    WriteREG((u4DispfmtRegBase + 0x2a4), ((u4TransY & 0xF)<<16) | ((u4ThredC & 0xFF)<<8) | (u4ThredY & 0xFF));

    if (u4Enable)
    {
        WriteREGMsk((u4DispfmtRegBase + 0x2a0), 0x30, 0x30);
        WriteREG((u4DispfmtRegBase + 0x54), 0x38019064);
    }
    else
    {
        WriteREG((u4DispfmtRegBase + 0x54), 0);
        WriteREGMsk((u4DispfmtRegBase + 0x2a0), 0x00, 0x30);
    }

}

void vPmxVerifyHalCfgV4Coff(unsigned int u4VdoId, unsigned int u4CoefIdx)
{
    unsigned int u4VdoRegBase, u4V4RegBase;
    unsigned int u4i, u4Size;

    if(u4VdoId == 0)
    {
        u4VdoRegBase = 0x42400;
        u4V4RegBase  = 0x42100;
    }
    else if(u4VdoId == 1)
    {
        u4VdoRegBase = 0x43400;
        u4V4RegBase = 0x43100;
    }
    else
    {
        u4VdoRegBase = 0x43a00;
        u4V4RegBase  = 0x42100;
    }

    {
        //UTIL_Printf("[pmx] %s(%d,%d): %s\n", __FUNCTION__, u4VdoId, u4CoefIdx, _ArVdoV4CoefSetting[u4CoefIdx].szArrayName);

        u4Size = _ArVdoV4CoefSetting[u4CoefIdx].size;
        for(u4i = 0; u4i < u4Size-1; u4i ++)
        {
            WriteREG((u4V4RegBase + (u4i*4)), _ArVdoV4CoefSetting[u4CoefIdx].pu4RegSetting[u4i]);
        }
        WriteREG((u4V4RegBase + 0x40), _ArVdoV4CoefSetting[u4CoefIdx].pu4RegSetting[u4Size - 1]);
    }

    WriteREGMsk((u4VdoRegBase+0x78), (1<<4), (7<<2));
}

void vPmxVerifyHalBuidinColor(unsigned int u4Mvdo, unsigned int u4Enable, unsigned int u4BIY, unsigned int u4BICb, unsigned int u4BICr)
{
    unsigned int u4FmtRegBase;

    if(u4Mvdo == 0)
    {
        u4FmtRegBase = 0x42000;
    }
    else if(u4Mvdo == 1)
    {
        u4FmtRegBase = 0x43000;
    }
    else
    {
        u4FmtRegBase = 0x42000;
    }

    if (u4Enable)
    {
        WriteREGMsk((u4FmtRegBase+0xac), 0, 0x1);

        WriteREGMsk((u4FmtRegBase+0xb4), (u4BIY&0xf)|((u4BICb&0xf)<<12)|((u4BICr&0xf)<<20), 0x00F0F0F0);
    }
    else
    {
        WriteREGMsk((u4FmtRegBase+0xac), 1, 0x1);
    }
}

void vPmxVerifyHalBoardColor(unsigned int u4Mvdo,unsigned int u4module, unsigned int u4Enable, unsigned int u4XWidth, unsigned int u4YWidth, unsigned int u4BDY, unsigned int u4BDCb, unsigned int u4BDCr)
{
    unsigned int u4FmtRegBase, u4VdoutfmtRegBase;
    unsigned int u4RegVal;

    if(u4Mvdo == 0)
    {
        u4VdoutfmtRegBase = 0x3000;
        u4FmtRegBase = 0x42000;
    }
    else if(u4Mvdo == 1)
    {
        u4VdoutfmtRegBase = 0x3300;
        u4FmtRegBase = 0x43000;
    }
    else
    {
        u4VdoutfmtRegBase = 0x3000;
        u4FmtRegBase = 0x42000;
    }

    if(u4module == 0)
    {
        if(u4Mvdo == 0)
        {
            if (u4Enable)
            {
                u4RegVal = ReadREG((u4VdoutfmtRegBase+0xa0));
                u4RegVal = u4RegVal + (u4XWidth<<16) - u4XWidth;
                WriteREG((u4VdoutfmtRegBase+0xa0), u4RegVal);

                u4RegVal = ReadREG((u4VdoutfmtRegBase+0xa4));
                u4RegVal = u4RegVal + (u4YWidth<<16) - u4YWidth;
                WriteREG((u4VdoutfmtRegBase+0xa4), u4RegVal);

                u4RegVal = ReadREG((u4VdoutfmtRegBase+0xa8));
                u4RegVal = u4RegVal + (u4YWidth<<16) - u4YWidth;
                WriteREG((u4VdoutfmtRegBase+0xa8), u4RegVal);

                WriteREGMsk((u4VdoutfmtRegBase+0x5c), ((u4BDY&0x3f)<<2)|((u4BDCb&0x3f)<<10)|((u4BDCr&0x3f)<<18), 0x00FCFCFC);

                WriteREG((u4VdoutfmtRegBase+0x58), (u4YWidth<<24)|(u4XWidth<<16));
                WriteREGMsk((u4VdoutfmtRegBase+0x0c), (0x1<<4), (0x1<<4));
            }
            else
            {
                WriteREGMsk((u4VdoutfmtRegBase+0x0c), 0, (0x1<<4));
            }
        }
        else
        {
            if (u4Enable)
            {
                u4RegVal = ReadREG((u4VdoutfmtRegBase+0xa0));
                u4RegVal = u4RegVal + (u4XWidth<<16) - u4XWidth;
                WriteREG((u4VdoutfmtRegBase+0xa0), u4RegVal);

                u4RegVal = ReadREG((u4VdoutfmtRegBase+0xa4));
                u4RegVal = u4RegVal + (u4YWidth<<16) - u4YWidth;
                WriteREG((u4VdoutfmtRegBase+0xa4), u4RegVal);

                u4RegVal = ReadREG((u4VdoutfmtRegBase+0xa8));
                u4RegVal = u4RegVal + (u4YWidth<<16) - u4YWidth;
                WriteREG((u4VdoutfmtRegBase+0xa8), u4RegVal);

                WriteREGMsk((u4VdoutfmtRegBase+0x74), ((u4BDY&0x3f)<<2)|((u4BDCb&0x3f)<<10)|((u4BDCr&0x3f)<<18), 0x00FCFCFC);

                WriteREG((u4VdoutfmtRegBase+0x70), (u4YWidth<<24)|(u4XWidth<<16));
                WriteREGMsk((0x3000+0x0c), (0x1<<5), (0x1<<5));
            }
            else
            {
                WriteREGMsk((0x3000+0x0c), 0, (0x1<<5));
            }

        }
    }
    else
    {
        if (u4Enable)
        {
            u4RegVal = ReadREG((u4FmtRegBase+0xa0));
            u4RegVal = u4RegVal + (u4XWidth<<16) - u4XWidth;
            WriteREG((u4FmtRegBase+0xa0), u4RegVal);

            u4RegVal = ReadREG((u4FmtRegBase+0xa4));
            u4RegVal = u4RegVal + (u4YWidth<<16) - u4YWidth;
            WriteREG((u4FmtRegBase+0xa4), u4RegVal);

            u4RegVal = ReadREG((u4FmtRegBase+0xa8));
            u4RegVal = u4RegVal + (u4YWidth<<16) - u4YWidth;
            WriteREG((u4FmtRegBase+0xa8), u4RegVal);

            WriteREGMsk((u4FmtRegBase+0x5c), ((u4BDY&0x3f)<<2)|((u4BDCb&0x3f)<<10)|((u4BDCr&0x3f)<<18), 0x00FCFCFC);

            WriteREG((u4FmtRegBase+0x58), 0x80000000|(u4YWidth<<24)|(u4XWidth<<16));
        }
        else
        {
            WriteREG((u4FmtRegBase+0x58), 0);
        }
    }
}

void PmxVerifyHalDemoMode(unsigned int u4VdoId, unsigned int u4Enable, unsigned int u4BdrLine, unsigned int u4LeftMode)
{
    unsigned int u4VdoRegBase, u4FmtRegBase;
    unsigned int u4BdrValue = 0;
    unsigned int u4RegVal = 0;
    unsigned int u4W=0;

    if(u4VdoId == 0)
    {
        u4FmtRegBase = 0x42000;
        u4VdoRegBase = 0x42400;
    }
    else if(u4VdoId == 1)
    {
        u4FmtRegBase = 0x43000;
        u4VdoRegBase = 0x43400;
    }
    else
    {
        u4FmtRegBase = 0x42000;
        u4VdoRegBase = 0x43A00;
    }

    u4RegVal = ReadREG(u4FmtRegBase + 0xa0) & 0xfff0fff;

    u4W = (u4RegVal & 0xfff) + ((u4RegVal & 0xfff0000) >> 16);
	//UTIL_Printf("[jg@debug: u4RegVal = %d \n" ,u4RegVal );

	
    switch (u4BdrLine)
    {
    case PMX_VFY_DEMO_BDR_0D8:
        u4BdrValue = 0;
        break;
    case PMX_VFY_DEMO_BDR_1D8:
        u4BdrValue = (unsigned int) (1*u4W/8);
        break;
    case PMX_VFY_DEMO_BDR_2D8:
        u4BdrValue = (unsigned int) (2*u4W/8);
        break;
    case PMX_VFY_DEMO_BDR_3D8:
        u4BdrValue = (unsigned int) (3*u4W/8);
        break;
    case PMX_VFY_DEMO_BDR_4D8:
        u4BdrValue = (unsigned int) (4*u4W/8);
        break;
    case PMX_VFY_DEMO_BDR_5D8:
        u4BdrValue = (unsigned int) (5*u4W/8);
        break;
    case PMX_VFY_DEMO_BDR_6D8:
        u4BdrValue = (unsigned int) (6*u4W/8);
        break;
    case PMX_VFY_DEMO_BDR_7D8:
        u4BdrValue = (unsigned int) (7*u4W/8);
        break;
    case PMX_VFY_DEMO_BDR_8D8:
        u4BdrValue = (unsigned int) (u4W);
        break;

    default:
        break;
    }

	//jg:add 
	if(u4LeftMode==0){
		u4BdrValue = u4W-u4BdrValue;
	}
    WriteREGMsk((u4FmtRegBase+0xcc), ((u4BdrValue&0xfff)<<20)|(u4LeftMode<<17), 0xfff20000);
    WriteREGMsk((u4VdoRegBase + 0x78), 0, ((1<<4))); //42478[4]luma_4tap filter coefficient programbale 0:hardware write

    UTIL_Printf("[PMX] Enable=%d, u4W=%d, u4BrdValue=%d, DemoBdr=%d/8, Left=%d\n", u4Enable, u4W, u4BdrValue, u4BdrLine, u4LeftMode);

    if ((u4Enable&(0x1<<0))== (0x1<<0))
    { //veritical demo 8taps vs. 4taps
        WriteREGMsk((u4FmtRegBase+0xcc), (1<<16), (1<<16)); //turn on demo mode
        WriteREGMsk((u4FmtRegBase+0x2a0), (1<<18), (1<<18));//421a0[18] vertical demo 1:8 tap(coefficient self_build) 4tap build_in
        WriteREGMsk((u4FmtRegBase+0xb0), (3<<0), (3<<0));//horizontal scaling enable,using linear interpolation
        // WriteREGMsk((u4FmtRegBase+0xe4), (0<<15), (1<<15));//closed horizontal demo  FIR
        WriteREGMsk((u4VdoRegBase+0x78), (0<<4), (0x1<<4)); //  open vertical 8tap,
        WriteREGMsk((u4FmtRegBase+0x24c), (3<<24), (3<<24)); //  open vertical 8tap,
    }
    else
    {
        WriteREGMsk((u4FmtRegBase+0x2a0), 0, (1<<18));
    }

    if ((u4Enable&(0x1<<1))== (0x1<<1))
    { //horizontal demo
        WriteREGMsk((u4FmtRegBase+0xcc), (1<<16), (1<<16));
        WriteREGMsk((u4FmtRegBase+0xb0), (1<<0), (3<<0));//horizontal scaling enable
        WriteREGMsk((u4FmtRegBase+0xe4), (1<<15), (1<<15));//horizontal demo  FIR
        WriteREGMsk((u4FmtRegBase+0x24c), (0<<24), (3<<24)); //  closed vertical 8tap,
        WriteREGMsk((u4FmtRegBase+0x2a0), (0<<18), (1<<18));//421a0[18] vertical demo 1:8 tap(coefficient self_build) 4tap build_in
    }
    else
    {
        WriteREGMsk((u4FmtRegBase+0xe4), 0, (1<<15));//horizontal demo  Linear
    }

    if ((u4Enable&(0x1<<2))== (0x1<<2))
    { //veritical demo 4taps vs. 3taps
        WriteREGMsk((u4FmtRegBase+0xcc), (1<<16), (1<<16));
        WriteREGMsk((u4VdoRegBase + 0x78), ((1<<0)), ((1<<0)));
        WriteREGMsk((u4VdoRegBase + 0x78), ((1<<4)), ((1<<4))); //YFIR_CF_PRG

        WriteREGMsk((u4FmtRegBase+0x29c), (0x1<<8)|(0x8<<0), (0x1<<8)|(0xf<<0)); //enable 3-tap and threshold
        WriteREGMsk((u4FmtRegBase+0x29c), (0x1<<9), (0x1<<9)); //enbale demo mode and
    }
    else
    {
        WriteREGMsk((u4FmtRegBase+0x29c), 0, (0x3<<8));
    }

    if (!u4Enable)
    {
        WriteREGMsk((u4FmtRegBase+0xcc), (0<<16), (1<<16)); //turn off demo mode
    }

}

void PmxVerifyHalNewSDEn(unsigned int u4VdoId, unsigned int u4Enable, unsigned int u4Type)
{
    unsigned int u4FmtRegBase;

    if(u4VdoId == 0)
    {
        u4FmtRegBase = 0x42000;
    }
    else if(u4VdoId == 1)
    {
        u4FmtRegBase = 0x43000;
    }
    else
    {
        u4FmtRegBase = 0x42000;
    }

    if (u4Enable)
    {
        WriteREGMsk((u4FmtRegBase+0x94), (1<<28)|(u4Type<<27), (1<<27)|(1<<28)|(1<<29));
    }
    else
    {
        WriteREGMsk((u4FmtRegBase+0x94), 0, (1<<27)|(1<<28)|(1<<29));
    }

    if (u4Type)
    {
        WriteREGMsk((u4FmtRegBase+0xe4), (1<<29), (1<<29));
    }
    else
    {
        WriteREGMsk((u4FmtRegBase+0xe4), 0, (1<<29));
    }
}

void PmxVerifyHalColorConv(unsigned int u4VdoId, unsigned int u4Enable, unsigned int u4B4, unsigned int u4TgtMode)
{
    unsigned int u4VDoutFmtRegBase = 0x3000;

    if (!u4B4)
    {
        if (!u4Enable)
        {
            if (601 == u4TgtMode)
            {
                WriteREGMsk((u4VDoutFmtRegBase+0xf4), (1<<8), (1<<8)|(1<<10));
            }
            else
            {
                WriteREGMsk((u4VDoutFmtRegBase+0xf4), 0, (1<<8)|(1<<10));
            }
        }
        else
        {
            if (601 == u4TgtMode)
            {
                WriteREGMsk((u4VDoutFmtRegBase+0xf4), (1<<8)|(1<<10), (1<<8)|(1<<10));
            }
            else
            {
                WriteREGMsk((u4VDoutFmtRegBase+0xf4), (1<<10), (1<<8)|(1<<10));
            }
        }
    }
    else
    {
        if (0 == u4VdoId)
        {
            if (!u4Enable)
            {
                if (601 == u4TgtMode)
                {
                    WriteREGMsk((u4VDoutFmtRegBase+0xf8), (1<<3), (1<<2)|(1<<3));
                }
                else
                {
                    WriteREGMsk((u4VDoutFmtRegBase+0xf8), 0, (1<<2)|(1<<3));
                }
            }
            else
            {
                if (601 == u4TgtMode)
                {
                    WriteREGMsk((u4VDoutFmtRegBase+0xf8), (1<<3)|(1<<2), (1<<2)|(1<<3));
                }
                else
                {
                    WriteREGMsk((u4VDoutFmtRegBase+0xf8), (1<<2), (1<<2)|(1<<3));
                }
            }
        }
        else
        {
            if (!u4Enable)
            {
                if (601 == u4TgtMode)
                {
                    WriteREGMsk((u4VDoutFmtRegBase+0xf8), (1<<7), (1<<6)|(1<<7));
                }
                else
                {
                    WriteREGMsk((u4VDoutFmtRegBase+0xf8), 0, (1<<6)|(1<<7));
                }
            }
            else
            {
                if (601 == u4TgtMode)
                {
                    WriteREGMsk((u4VDoutFmtRegBase+0xf8), (1<<7)|(1<<6), (1<<6)|(1<<7));
                }
                else
                {
                    WriteREGMsk((u4VDoutFmtRegBase+0xf8), (1<<6), (1<<6)|(1<<7));
                }
            }
        }
    }
}

unsigned int _u4DummyData[72] =
{
    0x48, 0x39, 0x5c, 0x07, 0x39, 0x1c, 0x07, 0x00,
    0x48, 0x36, 0x18, 0x05, 0x3d, 0x1f, 0x09, 0x00,
    0x47, 0x32, 0x15, 0x04, 0x3f, 0x23, 0x0b, 0x01,
    0x46, 0x2e, 0x12, 0x03, 0x42, 0x27, 0x0d, 0x01,
    0x45, 0x2a, 0x0f, 0x02, 0x45, 0x2a, 0x0f, 0x02,
    0x42, 0x27, 0x0d, 0x01, 0x46, 0x2e, 0x12, 0x03,
    0x3f, 0x23, 0x0b, 0x01, 0x47, 0x32, 0x15, 0x04,
    0x3d, 0x1f, 0x09, 0x00, 0x48, 0x36, 0x18, 0x05,
    0xd3, 0xf1, 0x90, 0x00, 0x84, 0x63, 0x81, 0x50,
};

#define PP_DELAY   1000

void PmxVerifyHalPostYLevTest(void)
{
    unsigned int u4Idx, u4Loop;
    unsigned int u4ReadValue, u4WriteValue;

    for (u4Idx=0; u4Idx<72; u4Idx++)
    {
        u4WriteValue  = (u4Idx<<4) | ((_u4DummyData[u4Idx]<<12));
        //u4WriteValue  = (u4Idx<<4) | ((u4Idx<<12));
        u4WriteValue |= 0x00000008;

        WriteREG(0x5b0b0, u4WriteValue);

        //write trigger
        WriteREG(0x5b0b0, (u4WriteValue | 2));
        //UTIL_Printf("YLev: Delay 1 \n");
        for (u4Loop=0; u4Loop<PP_DELAY; u4Loop++)
        {
            //dummy register
            WriteREG(0x42100, u4Loop);
        }
        WriteREG(0x5b0b0, (u4WriteValue & (~2)));
        for (u4Loop=0; u4Loop<PP_DELAY; u4Loop++)
        {
            //dummy register
            WriteREG(0x42100, u4Loop);
        }
    }
    for (u4Idx=0; u4Idx<72; u4Idx++)
    {
        //Read
        u4ReadValue = u4Idx<<4;
        u4ReadValue |= 0x00000001;
        WriteREG(0x5b0b0, u4ReadValue);

        //UTIL_Printf("YLev: Delay 2 \n");
        for (u4Loop=0; u4Loop<PP_DELAY; u4Loop++)
        {
            //dummy register
            WriteREG(0x42100, u4Loop);
        }
        u4ReadValue = (ReadREG(0x5b0a4) & 0x00ff0000) >> 16 ;

        if (u4ReadValue != _u4DummyData[u4Idx])
            //if (u4ReadValue != u4Idx)
        {
            UTIL_Printf("YLev Failed: Index : u4Value = 0x%2x : 0x%2x \n", u4Idx, u4ReadValue);
        }
    }
}

void PmxVerifyHalPostYLevBurstTest(void)
{
    unsigned int u4Idx, u4Loop;
    unsigned int u4ReadValue, u4WriteValue;

    //dummmy
    WriteREG(0x5b0b0, 0x00000FFC);
    for (u4Loop=0; u4Loop<PP_DELAY; u4Loop++)
    {
        //dummy register
        WriteREG(0x42100, u4Loop);
    }

    for (u4Idx=0; u4Idx<72; u4Idx++)
    {
        u4WriteValue  = (u4Idx<<4) | ((_u4DummyData[(71-u4Idx)]<<12));
        u4WriteValue |= 0x0000000C;

        WriteREG(0x5b0b0, u4WriteValue);
    }
    for (u4Idx=0; u4Idx<72; u4Idx++)
    {
        //Read
        u4ReadValue = u4Idx<<4;
        u4ReadValue |= 0x00000001;
        WriteREG(0x5b0b0, u4ReadValue);

        //UTIL_Printf("YLev: Delay 1 \n");
        for (u4Loop=0; u4Loop<PP_DELAY; u4Loop++)
        {
            //dummy register
            WriteREG(0x42100, u4Loop);
        }
        u4ReadValue = (ReadREG(0x5b0a4) & 0x00ff0000) >> 16 ;

        if (u4ReadValue != _u4DummyData[(71-u4Idx)])
        {
            UTIL_Printf("YLev Failed: Index : u4Value = 0x%2x : 0x%2x \n", u4Idx, u4ReadValue);
        }
    }
}

void PmxVerifyHalPostLClipTest(void)
{
    unsigned int u4Idx, u4Loop;
    unsigned int u4ReadValue, u4WriteValue;

    for (u4Idx=0; u4Idx<72; u4Idx++)
    {
        u4WriteValue  = u4Idx | (u4Idx<<8);
        WriteREG(0x5b0a0, u4WriteValue);
        for (u4Loop=0; u4Loop<PP_DELAY; u4Loop++)
        {
            //dummy register
            WriteREG(0x42100, u4Loop);
        }

        WriteREG(0x5b09c, 0x00000009);
        for (u4Loop=0; u4Loop<PP_DELAY; u4Loop++)
        {
            //dummy register
            WriteREG(0x42100, u4Loop);
        }

        WriteREG(0x5b09c, 0x0000000B);
        for (u4Loop=0; u4Loop<PP_DELAY; u4Loop++)
        {
            //dummy register
            WriteREG(0x42100, u4Loop);
        }


        WriteREG(0x5b09c, 0x00000009);
        for (u4Loop=0; u4Loop<PP_DELAY; u4Loop++)
        {
            //dummy register
            WriteREG(0x42100, u4Loop);
        }

        WriteREG(0x5b09c, 0x00000001);
        for (u4Loop=0; u4Loop<PP_DELAY; u4Loop++)
        {
            //dummy register
            WriteREG(0x42100, u4Loop);
        }

        /*
        //u4WriteValue  = (u4Idx<<0) | ((_u4DummyData[u4Idx]<<8));


        //write trigger
        WriteREG(0x5b09c, (u4WriteValue | 2));

        //UTIL_Printf("LClip: Delay 1 \n");
        //x_thread_delay(2);
        //UTIL_Printf("LClip: Delay 2 \n");
        for (u4Loop=0; u4Loop<PP_DELAY; u4Loop++)
        {
        //dummy register
        WriteREG(0x42100, u4Loop);
        WriteREG(0x5b09c, (u4WriteValue | 2));
        }
        WriteREG(0x5b09c, (u4WriteValue & (~2)));
        for (u4Loop=0; u4Loop<PP_DELAY; u4Loop++)
        {
        //dummy register
        WriteREG(0x42100, u4Loop);
        WriteREG(0x5b09c, (u4WriteValue & (~2)));
        }
        */
    }

    WriteREG(0x5b09c, 0x0);
    for (u4Loop=0; u4Loop<PP_DELAY; u4Loop++)
    {
        //dummy register
        WriteREG(0x42100, u4Loop);
    }

    for (u4Idx=0; u4Idx<72; u4Idx++)
    {
        // set read address
        u4WriteValue  = u4Idx;//(u4Idx<<0) | ((u4Idx<<8));
        WriteREG(0x5b0a0, u4WriteValue);
        for (u4Loop=0; u4Loop<PP_DELAY; u4Loop++)
        {
            //dummy register
            WriteREG(0x42100, u4Loop);
        }

        // enable read
        WriteREG(0x5b09c, 0x00000001);
        for (u4Loop=0; u4Loop<PP_DELAY; u4Loop++)
        {
            //dummy register
            WriteREG(0x42100, u4Loop);
        }

        u4ReadValue = ReadREG(0x5b0a4);
        for (u4Loop=0; u4Loop<PP_DELAY; u4Loop++)
        {
            //dummy register
            WriteREG(0x42100, u4Loop);
        }
        UTIL_Printf("Read Index : u4Value = 0x%2x : 0x%2x \n", u4Idx, u4ReadValue);

        /*
        WriteREG(0x5b09c, 0x00000001);
        WriteREG(0x5b0a0, u4Idx);

        //UTIL_Printf("LClip: Delay 3 \n");
        for (u4Loop=0; u4Loop<PP_DELAY; u4Loop++)
        {
        //dummy register
        WriteREG(0x42100, u4Loop);
        u4ReadValue = (ReadREG(0x5b0a4) & 0x00ff);
        }

        u4ReadValue = (ReadREG(0x5b0a4) & 0x00ff);

        //if (u4ReadValue != _u4DummyData[u4Idx])
        //if (u4ReadValue != u4Idx)
        {
        UTIL_Printf("LClip Failed: Index : u4Value = 0x%2x : 0x%2x \n", u4Idx, u4ReadValue);
        }
        */
    }

    // turn off read
    WriteREG(0x5b09c, 0x0);
    for (u4Loop=0; u4Loop<PP_DELAY; u4Loop++)
    {
        //dummy register
        WriteREG(0x42100, u4Loop);
    }
}

void PmxVerifyHalPostLClipBurstTest(void)
{
    unsigned int u4Idx, u4Loop;
    unsigned int u4ReadValue, u4WriteValue;

    //dummmy
    WriteREG(0x5b09c, 0x0000000C);
    WriteREG(0x5b0a0, 0x000000ff);

    for (u4Loop=0; u4Loop<PP_DELAY; u4Loop++)
    {
        //dummy register
        WriteREG(0x42100, u4Loop);
    }
    for (u4Idx=0; u4Idx<72; u4Idx++)
    {
        u4WriteValue  = (u4Idx<<0) | ((_u4DummyData[(71-u4Idx)]<<8));
        WriteREG(0x5b0a0, u4WriteValue);
    }
    for (u4Loop=0; u4Loop<PP_DELAY; u4Loop++)
    {
        //dummy register
        WriteREG(0x42100, u4Loop);
    }

    for (u4Idx=0; u4Idx<72; u4Idx++)
    {
        //Read
        WriteREG(0x5b09c, 0x00000001);
        WriteREG(0x5b0a0, u4Idx);

        //UTIL_Printf("LClip: Delay 1 \n");
        for (u4Loop=0; u4Loop<PP_DELAY; u4Loop++)
        {
            //dummy register
            WriteREG(0x42100, u4Loop);
        }

        u4ReadValue = (ReadREG(0x5b0a4) & 0x00ff);

        if (u4ReadValue != _u4DummyData[(71-u4Idx)])
        {
            UTIL_Printf("LClip Failed: Index : u4Value = 0x%2x : 0x%2x \n", u4Idx, u4ReadValue);
        }
    }
}

void vPmxVerifyHalVdoRXSkip(unsigned int u4RXSkip, UCHAR ucVdoId)
{
    unsigned int u4DispfmtRegBase;

    if(ucVdoId == 0)
    {
        u4DispfmtRegBase = 0x42000;
    }
    else if(ucVdoId == 1)
    {
        u4DispfmtRegBase = 0x43000;
    }
    else
    {
        u4DispfmtRegBase = 0x42000;
    }

    WriteREGMsk((u4DispfmtRegBase+0x9c), (u4RXSkip << 16), (0xf << 16));
    WriteREGMsk((u4DispfmtRegBase+0xe4), (0 << 31), (0x1 << 31));  //padding use last pixel
    //WriteREGMsk((u4DispfmtRegBase+0xe4), (1 << 31), (0x1 << 31));  //padding use background color
}

void vPmxVerifyHalSecArea(unsigned int u4Mvdo, unsigned int u4Enable, unsigned int u4XShift)
{
    unsigned int u4VdoutfmtRegBase;
    unsigned int u4RegVal;

    if(u4Mvdo == 0)
    {
        u4VdoutfmtRegBase = 0x3000;
    }
    else if(u4Mvdo == 1)
    {
        u4VdoutfmtRegBase = 0x3300;
    }
    else
    {
        u4VdoutfmtRegBase = 0x3000;
    }

    if (u4Enable)
    {
        u4RegVal = ReadREG((u4VdoutfmtRegBase+0xa0));
        u4RegVal = u4RegVal + (u4XShift<<16) - u4XShift;
        WriteREG((u4VdoutfmtRegBase+0x60), u4RegVal);                //3060 vdo1 h seconf active area

        WriteREGMsk((u4VdoutfmtRegBase+0x24), (0x1<<28), (0x1<<28)); //3024[28]h_act_2
    }
    else
    {
        WriteREGMsk((u4VdoutfmtRegBase+0x24), 0, (0x1<<28));
    }

}

void vPmxVerifyHalSetCsMode(UCHAR u4VdoId, unsigned int u4On)
{
    if (u4On)
    {
        WriteREG(0x42800, 0x11);
        WriteREG(0x4287c, 0x30380000);

        _ucCsHoriActRegion = ReadREG(0x420A0); // backup for CS h-zoom out.
    }
    else
    {
        WriteREG(0x42800, 0);
        WriteREG(0x4287c, 0x30380000);
    }
}

void vPmxVerifyHalSetHSharp(unsigned int u4VdoId, unsigned int u4Tap8, unsigned int u4On)
{
    unsigned int u4DispFmtRegBase = 0;
    unsigned int u4V8RegBase = 0;

    if(u4VdoId == 0)
    {
        u4V8RegBase = 0x42200;
        u4DispFmtRegBase = 0x42000;
    }
#if 1
    else
    {
        u4V8RegBase = 0x43200;
        u4DispFmtRegBase = 0x43000;
    }
#else
    else if(u4VdoId == 1)
    {
        u4V8RegBase = 0x43100;
        u4DispFmtRegBase = 0x42300;
    }
    else
    {
        u4V8RegBase = 0x42100;
        u4DispFmtRegBase = 0x42000;
    }
#endif

    if (u4Tap8)
    {
        WriteREGMsk((u4DispFmtRegBase + 0xe4), (1<<30), (1<<30));
    }
    else
    {
        WriteREGMsk((u4DispFmtRegBase + 0xe4), 0, (1<<30));
    }
    if (u4On)
    {
        WriteREGMsk((u4V8RegBase + 0xb4), (1<<23), (1<<23));  //0x421b4[23] sharpness enable
    }
    else
    {
        WriteREGMsk((u4V8RegBase + 0xb4), 0, (1<<23));
    }
}

void PmxVerifyHalPostColorConv(unsigned int u4VdoId, unsigned int u4Enable, unsigned int u4TgtMode)
{
    unsigned int u4VDoutFmtRegBase = 0x3000;


    if (0 == u4VdoId)
    {
        if (!u4Enable)
        {
            if (601 == u4TgtMode)
            {
                WriteREGMsk((u4VDoutFmtRegBase+0x18), (1<<4), (1<<3)|(1<<4));
            }
            else
            {
                WriteREGMsk((u4VDoutFmtRegBase+0x18), 0, (1<<3)|(1<<4));
            }
        }
        else
        {
            if (601 == u4TgtMode)
            {
                WriteREGMsk((u4VDoutFmtRegBase+0x18), (1<<3)|(1<<4), (1<<3)|(1<<4));
            }
            else
            {
                WriteREGMsk((u4VDoutFmtRegBase+0x18), (1<<3), (1<<3)|(1<<4));
            }
        }
    }
    else
    {
        if (!u4Enable)
        {
            if (601 == u4TgtMode)
            {
                WriteREGMsk((u4VDoutFmtRegBase+0x18), (1<<6), (1<<5)|(1<<6));
            }
            else
            {
                WriteREGMsk((u4VDoutFmtRegBase+0x18), 0, (1<<5)|(1<<6));
            }
        }
        else
        {
            if (601 == u4TgtMode)
            {
                WriteREGMsk((u4VDoutFmtRegBase+0x18), (1<<5)|(1<<6), (1<<5)|(1<<6));
            }
            else
            {
                WriteREGMsk((u4VDoutFmtRegBase+0x18), (1<<5), (1<<5)|(1<<6));
            }
        }

    }
}

void PmxVerifyHalFpgaDispEn(unsigned int u4On)
{
    WriteREGMsk((0x35f0), ((!u4On)<<28), (1<<28));
    WriteREGMsk((0x52004), (2<<16), (0x1f<<16));
}

void PmxVerifyHalSelSrcPoint(unsigned int u4SrcFmt, unsigned int u4SelSrc, unsigned int u4DramBufPtr)
{
    switch(u4SelSrc)
    {
    case PMX_VFY_RGB2HDMI:
        WriteREGMsk((0x357c), (1<<4)|(1<<3), 0x1E);
        WriteREGMsk((0x35f0), u4DramBufPtr, 0x07FFFFFF);
        break;
    case PMX_VFY_RGB2HDMI_SUB:
        WriteREGMsk((0x357c), (1<<1)|(1<<3), 0x1E);
        WriteREGMsk((0x35f0), u4DramBufPtr, 0x07FFFFFF);
        break;
    case PMX_VFY_TVE:
        WriteREGMsk((0x357c), (1<<3), 0x1E);
        WriteREGMsk((0x35f0), u4DramBufPtr, 0x07FFFFFF);
        break;
    case PMX_VFY_FMT:
    default:
        WriteREGMsk((0x357c), (1<<2)|(1<<3), 0x1E);
        WriteREGMsk((0x35f0), u4DramBufPtr, 0x07FFFFFF);
        break;
    }
    switch (u4SrcFmt)
    {
    case 720:
        WriteREG(0x35f4, 0x8ff00000);
        WriteREG(0x35f8, 0x00000020);
        WriteREG(0x35fc, 0x22ee0672);
        break;
#if 0
    case 720@50Hz:
        WriteREG(0x35f4, 0x8ff00000);
        WriteREG(0x35f8, 0x00000020);
        WriteREG(0x35fc, 0x22ee07bc);
        break;
    case 1080p@24Hz:
        WriteREG(0x352c, 0x00400000); //Select RGB2HDMI timming.
        WriteREG(0x35f4, 0x8ff00000);
        WriteREG(0x35f8, 0x00000020);
        WriteREG(0x35fc, 0x04650ac0);
        WriteREG(0x30d4, 0x8ac00465);
        break;
    case 1080p@24Hz@3D:
        WriteREG(0x352c, 0x00400000); //Select RGB2HDMI timming.
        WriteREG(0x35f4, 0x8ff00000);
        WriteREG(0x35f8, 0x00000020);
        WriteREG(0x35fc, 0x00CA8ac0);
        WriteREG(0x30d4, 0x8ac00465);
        WriteREGMsk(0x351c, 1, (1<<28));
        WriteREGMsk(0x3508, 0, (1<<30));
        WriteREG(0x3084, 0x8ac08cab);
        break;
#endif
    case 1080:
        WriteREG(0x35f4, 0x8ff00000);
        WriteREG(0x35f8, 0x00000020);
        WriteREG(0x35fc, 0x3c650898);
        break;
    case 2160:
        WriteREG(0x35f4, 0x8ff00000);
        WriteREG(0x35f8, 0x00000020);
        WriteREG(0x35fc, 0x30ca9130);
        break;
    case 2161:
        WriteREG(0x35f4, 0x8ff00000);
        WriteREG(0x35f8, 0x00000020);
        WriteREG(0x35fc, 0x30ca957c);
        break;
    case 480:
    default:
        WriteREG(0x35f4, 0x8ff00000);
        WriteREG(0x35f8, 0x00000020);
        WriteREG(0x35fc, 0x020d06b4);
        break;
    }
}


void PmxVerifyHalSetPrgsForOsd3d(unsigned int u4Prgs_for_osd_3d)
{
    WriteREGMsk(0x30E4, u4Prgs_for_osd_3d << 15, 1 << 15);

    UTIL_Printf("[PMX] set Prgs_for_osd_3d = %d\n", u4Prgs_for_osd_3d);
}

void PmxVerifyHalSetSrcSize (UCHAR ucVdpId, unsigned int u4Width, unsigned int u4Height)
{
    unsigned int u4Reg42410;
    unsigned int u4VdoRegBase;

    if(ucVdpId == 0)
    {
        u4VdoRegBase = 0x42400;
    }
    else if(ucVdpId == 1)
    {
        u4VdoRegBase = 0x43400;
    }
    else
    {
        u4VdoRegBase = 0x43A00;
    }

    /*
     * 42410: HBLOCK, Picture size
     * [7:0]   -- HBLOCK[7:0], the number of blocks per line. (frame buffer width / 8)
     * [15:8]  -- DW_NEED[7:0], how many double-words of pixels should be displayed for SD source picture.
     * [26:16] -- PICHEIGHT[10:0], the height of picture.
     * [29:28] -- HBLOCK[9:8], extend for 4Kx2K source.
     */

    u4Reg42410 = ReadREG(u4VdoRegBase + 0x10);

    u4Reg42410 &= ~0xFF;
    u4Reg42410 |= ((u4Width + 7) / 8) & 0xFF;

    if (u4Height > 0x7FF)
    {
        u4Height = 0x7FF;
        UTIL_Printf("[PMX] Picture height overflow. %u\n", u4Height);
    }

    u4Reg42410 &= ~(0x7FF << 16);
    u4Reg42410 |= ((u4Height & 0x7FF) << 16);

    WriteREG(u4VdoRegBase + 0x10, u4Reg42410);
}

void PmxVerifyHalVdo2PowerOnOff (BOOL fgOn)
{
#ifndef __ARM2__
    if (fgOn)
    {
        // 1. Power on 1
        WriteREGMsk(0x388, 0x1 << 0, 0x1 << 0);
        // 2. Delay 1us
        msleep(1);
        // 3. Power on 2
        WriteREGMsk(0x388, 0x1 << 1, 0x1 << 1);
        // 4. Delay 1us
        msleep(1);
        // 5. Memory(SRAM) power on
        WriteREG(0x3A8, 0x00000000);
        // 6. Delay 1us
        msleep(1);
        // 7. RESET de-assert
        WriteREGMsk(0x388, 0x1 << 2, 0x1 << 2);
        // 8. ISOLATE disable
        WriteREGMsk(0x388, 0x0 << 3, 0x1 << 3);
        // 9. Clock on
        WriteREGMsk(0x388, 0x0 << 4, 0x1 << 4);
    }
    else
    {
        // 1. Clock off
        WriteREGMsk(0x388, 0x1 << 4, 0x1 << 4);
        // 2. ISOLATE enable
        WriteREGMsk(0x388, 0x1 << 3, 0x1 << 3);
        // 3. RESET assert
        WriteREGMsk(0x388, 0x0 << 2, 0x1 << 2);
        // 4. Memory (SRAM) power off
        WriteREG(0x3A8, 0xFFFFFFFF);
        // 5. Power off 1
        WriteREGMsk(0x388, 0x0 << 0, 0x1 << 0);
        // 6. Power off 2
        WriteREGMsk(0x388, 0x0 << 1, 0x1 << 1);
    }
#endif
}

void PmxVerifyHalUsingStandardTiming(unsigned int u4PmxId,unsigned int u4SrcWidth,unsigned int u4SrcHight,unsigned int u4DstWidth,unsigned int u4DstHight,unsigned int u4adj1,unsigned int u4adj2,unsigned int u4adj3){
	unsigned int u4VdoRegBase,u4FmtRegBase;
	unsigned int	htotal = 0;
	unsigned int	vtotal = 0;
	unsigned int	hend,hact = 0;
	unsigned int	vend,vact = 0;
	unsigned int  h_xfs,total_xfs;
	unsigned int  src_hact,src_hend;
	
	if(u4PmxId == 0)
	{
		u4VdoRegBase = 0x42400;
		u4FmtRegBase = 0x42000;
	}
	else
	{
		u4VdoRegBase = 0x43400;
		u4FmtRegBase = 0x43000;
	}

	switch(u4DstHight){
		case(480):
		{
			h_xfs = 3;
			total_xfs = 6;
			break;
		}
		case(600):
			{				
			h_xfs = 4;
			total_xfs = 1;
			break;
			}

		case(720):
			{
			
			h_xfs = 2;
			total_xfs = 2;
			break;
			}

		case(1080):
			{
			h_xfs = 1;
			total_xfs = 1;
			break;
			}
		default:
		{
			h_xfs = 3;
			total_xfs = 6;
			break;
		}
	} //end switch?

	htotal = ((ReadREG(0xa468c)) >> 16) & 0xfff;
	vtotal = ((ReadREG(0xa468c))) & 0xfff;
	hact = ((ReadREG(0xa46a0)) >> 16) & 0xfff;
	vact = ((ReadREG(0xa46a4)) >> 16) & 0xfff;

	hact = hact -5;
	
	hend = hact + u4DstWidth -1;
	vend = vact + u4DstHight -1;
	
	WriteREGMsk(0xa46a4,vend,0xfff);
	WriteREGMsk(0xa46a8,vend,0xfff);
	WriteREGMsk(0xa46a0,hend,0xfff);


	/*
	 h v sync adjust.
	*/
	WriteREG(0x420e8,(h_xfs*htotal-4)| 0x00040000);
	WriteREG(0x420ec,(htotal-4) | 0x40020000);
	
#define LineOutDelay_n 2
	/*
	v adjust.
	*/
	WriteREGMsk(0x42074,((vact-LineOutDelay_n)<<16),0xfff0000);
	WriteREGMsk(0x42074,((vend-LineOutDelay_n)),0xfff);
	WriteREGMsk(0x42078,((vact-LineOutDelay_n)<<16),0xfff0000);
	WriteREGMsk(0x42078,((vend-LineOutDelay_n)),0xfff);
	
	WriteREGMsk(0x420a4,((vact)<<16),0xfff0000);
	WriteREGMsk(0x420a4,((vend)),0xfff);
	WriteREGMsk(0x420a8,((vact)<<16),0xfff0000);
	WriteREGMsk(0x420a8,((vend)),0xfff);
	
	/*
	h adjust.
	*/
	if(u4SrcWidth < u4DstWidth){

		src_hact = hact*h_xfs;
	    src_hend = hact*h_xfs + u4DstWidth -1;
		
		WriteREGMsk(0x420a0,((src_hact)<<16),0xfff0000);
		WriteREGMsk(0x420a0,(src_hact),0xfff);

		WriteREGMsk(0x42070,((src_hact+5)<<16),0xfff0000);
		WriteREGMsk(0x42070,(src_hend+5),0xfff);

		WriteREGMsk(0x4207c,((hact*total_xfs)<<16),0xfff0000);
		WriteREGMsk(0x4207c,(hend*total_xfs),0x1fff);
		
		WriteREGMsk(0x420c0,((hact+15)<<16),0xfff0000);
		WriteREGMsk(0x420c0,(hend+15),0xfff);

		WriteREGMsk(0xa46a0,((src_hact+15+5)<<16),0xfff0000);
		WriteREGMsk(0xa46a0,(src_hend+15+5),0xfff);	

	}else{ // scaler down . h down enable;
	    // fixme: get a good src_hact.
	    src_hact = hact +u4adj1;
	    src_hend = hact + u4SrcWidth -1;
		
		WriteREGMsk(0x420a0,((src_hact)<<16),0xfff0000);
		WriteREGMsk(0x420a0,(src_hend),0xfff);

		WriteREGMsk(0x42070,((src_hact+5)<<16),0xfff0000);
		WriteREGMsk(0x42070,(src_hend+5),0xfff);

		WriteREGMsk(0x4207c,((hact*total_xfs)<<16),0xfff0000);
		WriteREGMsk(0x4207c,(hend*total_xfs),0x1fff);
		
		WriteREGMsk(0x420c0,((hact)<<16),0xfff0000);
		WriteREGMsk(0x420c0,(hend),0xfff);

		WriteREGMsk(0xa46a0,((src_hact+5)<<16),0xfff0000);
		WriteREGMsk(0xa46a0,(src_hend+5),0xfff);		
		
	}

	UTIL_Printf("reg[0xa46a0] = %8x \n", ReadREG(0xa46a0));
	UTIL_Printf("reg[0xa46a4] = %8x \n", ReadREG(0xa46a4));
	UTIL_Printf("reg[0xa46a8] = %8x \n", ReadREG(0xa46a8));
	UTIL_Printf("reg[0xa46e8] = %8x \n", ReadREG(0xa46e8));
	UTIL_Printf("reg[0xa468c] = %8x \n\n", ReadREG(0xa468c));
	
	UTIL_Printf("reg[0x42070] = %8x \n", ReadREG(0x42070));
	UTIL_Printf("reg[0x42074] = %8x \n", ReadREG(0x42074));
	UTIL_Printf("reg[0x42078] = %8x \n", ReadREG(0x42078));

	UTIL_Printf("reg[0x4207c] = %8x \n", ReadREG(0x4207c));
	UTIL_Printf("reg[0x420c0] = %8x \n\n", ReadREG(0x420c0));

	UTIL_Printf("reg[0x420a0] = %8x \n", ReadREG(0x420a0));
	UTIL_Printf("reg[0x420a4] = %8x \n", ReadREG(0x420a4));
	UTIL_Printf("reg[0x420a8] = %8x \n", ReadREG(0x420a8));

	UTIL_Printf("reg[0x420e8] = %8x \n", ReadREG(0x420e8));
	UTIL_Printf("reg[0x420ec] = %8x \n", ReadREG(0x420ec));
	
}


