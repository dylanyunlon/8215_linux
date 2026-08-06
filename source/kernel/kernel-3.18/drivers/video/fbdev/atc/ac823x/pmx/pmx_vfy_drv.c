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
//#include <media/atc/drv_av_d.h>
#include <media/atc/ac823x/pmx_hal.h>
#include "windows.h"
#include "x_debug.h"
#include "x_stl_lib.h"
#else
#include "x_types.h"
//#include "drv_av_d.h"
#include "pmx_hal.h"
#endif
#include "drv_config.h"
#include "x_os.h"
#include "x_rtos.h"
#include "x_assert.h"
#include "x_util.h"
#include "x_printf.h"
#include "x_hal_ic.h"

#include "log.h"
#include "pmx_vfy_drv.h"
/*#include "pmx_vfy_sys.h"*/
#include "pmx_vfy_hal.h"
#include "sys_config.h"
#include <generated/atc_project.h>

const CHAR* szResStr[] =
{
    "RES_480I",
    "RES_576I",
    "RES_480P",
    "RES_576P",
    "RES_480P_800",
    "RES_600P_800",
    "RES_600P_1064",
    "RES_576P_2880",
    "RES_720P60HZ",
    "RES_720P50HZ",
    "RES_1080I60HZ",
    "RES_1080I50HZ",
    "RES_1080P60HZ",
    "RES_1080P50HZ",
    "RES_1080P30HZ",
    "RES_1080P25HZ",
    "RES_480I_2880",
    "RES_576I_2880",
    "RES_1080P24HZ",
    "RES_1080P23_976HZ",
    "RES_1080P29_97HZ",
    "RES_3D_1080P23HZ",
    "RES_3D_1080P24HZ",
    "RES_3D_720P60HZ",
    "RES_3D_720P50HZ",
    "RES_3D_720P30HZ",
    "RES_3D_720P25HZ",
    "RES_3D_576P50HZ",
    "RES_3D_480P60HZ",
    "RES_3D_1080I60HZ",
    "RES_3D_1080I50HZ",
    "RES_3D_1080I30HZ",
    "RES_3D_1080I25HZ",
    "RES_3D_576I25HZ",
    "RES_3D_480I30HZ",
    "RES_3D_576I50HZ",
    "RES_3D_480I60HZ",
    "RES_2D_480I60HZ",
    "RES_2D_576I50HZ",
    "RES_2D_640x480HZ",
    "RES_PANEL_AUO_B089AW01",
    "RES_3D_720P60HZ_TB",
    "RES_3D_720P50HZ_TB",
    "RES_3D_1080I60HZ_SBS_HALF",
    "RES_3D_1080I50HZ_SBS_HALF",
    "RES_3D_1080P23HZ_TB",
    "RES_3D_1080P24HZ_TB",

    "RES_2160P_23_976HZ",
    "RES_2160P_24HZ",
    "RES_2160P_25HZ",
    "RES_2160P_29_97HZ",
    "RES_2160P_30HZ",
    "RES_2161P_24HZ",

    "RES_720P30HZ",
    "RES_720P25HZ",
    "RES_720P24HZ",
    "RES_720P23HZ",

    "RES_3D_1080P60HZ",
    "RES_3D_1080P50HZ",
    "RES_3D_1080P30HZ",
    "RES_3D_1080P29HZ",
    "RES_3D_1080P25HZ",
    "RES_3D_720P24HZ",
    "RES_3D_720P23HZ",

    "RES_3D_1080P60HZ_TB",
    "RES_3D_1080P50HZ_TB",
    "RES_3D_1080P30HZ_TB",
    "RES_3D_1080P29HZ_TB",
    "RES_3D_1080P25HZ_TB",
    "RES_3D_1080I60HZ_TB",
    "RES_3D_1080I50HZ_TB",
    "RES_3D_1080I30HZ_TB",
    "RES_3D_1080I25HZ_TB",
    "RES_3D_720P30HZ_TB",
    "RES_3D_720P25HZ_TB",
    "RES_3D_720P24HZ_TB",
    "RES_3D_720P23HZ_TB",
    "RES_3D_576P50HZ_TB",
    "RES_3D_576I25HZ_TB",
    "RES_3D_576I50HZ_TB",
    "RES_3D_480P60HZ_TB",
    "RES_3D_480I30HZ_TB",
    "RES_3D_480I60HZ_TB",

    "RES_3D_1080P60HZ_SBS_HALF",
    "RES_3D_1080P50HZ_SBS_HALF",
    "RES_3D_1080P30HZ_SBS_HALF",
    "RES_3D_1080P29HZ_SBS_HALF",
    "RES_3D_1080P25HZ_SBS_HALF",
    "RES_3D_1080P24HZ_SBS_HALF",
    "RES_3D_1080P23HZ_SBS_HALF",
    "RES_3D_1080I30HZ_SBS_HALF",
    "RES_3D_1080I25HZ_SBS_HALF",
    "RES_3D_720P60HZ_SBS_HALF",
    "RES_3D_720P50HZ_SBS_HALF",
    "RES_3D_720P30HZ_SBS_HALF",
    "RES_3D_720P25HZ_SBS_HALF",
    "RES_3D_720P24HZ_SBS_HALF",
    "RES_3D_720P23HZ_SBS_HALF",
    "RES_3D_576P50HZ_SBS_HALF",
    "RES_3D_576I25HZ_SBS_HALF",
    "RES_3D_576I50HZ_SBS_HALF",
    "RES_3D_480P60HZ_SBS_HALF",
    "RES_3D_480I30HZ_SBS_HALF",
    "RES_3D_480I60HZ_SBS_HALF",
    "RES ERROR,OVERFLOW",

};

#define  PMX_VRF_IO_SEMI_HOSTING    0
#define  PMX_VRF_IO_RVD_UTIL        1
#define  PMX_VRF_IO_HDD             2
#define  PMX_VRF_IO_USB             3
#define  PMX_VRF_FILE_IO_TYPE       PMX_VRF_IO_SEMI_HOSTING

static unsigned int _i4PmxVerifyFrmBufIdx = 0;
static bool _fgPmxVerifyDrvInit = FALSE;
unsigned int _u4PmxVdoIndex;
bool EnPmxRearNewSD144Mode = FALSE;
volatile BOOL g_fgNeedEnableXHalf = FALSE;
volatile bool g_rearFieldSwith = FALSE;


//volatile unsigned int g_rearFieldSwith = 0;

extern unsigned long IO_BASE_BRINGUP;
#define WriteREG(arg, val) (*(volatile __u32*)(IO_BASE_BRINGUP + (arg)) = val)
#define ReadREG(arg)       (*(volatile __u32*)(IO_BASE_BRINGUP + (arg)))
#define WriteREGMsk(arg, val, msk) WriteREG((arg), (ReadREG(arg) & (~(msk))) | ((val) & (msk)))
#define WriteRegMsk(arg, val, msk) WriteREG((arg), (ReadREG(arg) & (~(msk))) | ((val) & (msk)))

#define UTIL_Printf(a,...)

void PmxVerifyDrvClkInit(unsigned char ucVdoId)
{
	//fixme:enable all clk is not good.
	if(0==ucVdoId){
		WriteREGMsk(0xd0, 0x00000000, 1<<6);
		WriteREGMsk(0xb4, 0x00000000, 1<<6);

	}else{
		WriteREGMsk(0xd0, 0x00000000, 1<<7);
		WriteREGMsk(0xb4, 0x00000000, 1<<7);
	}
#ifndef CONFIG_ATC_PRJ_ac823x_adas
	WriteREGMsk(0xd0, 0xffffffff, 0xffffffff); 
	WriteREGMsk(0xb4, 0xffffffff, 0xffffffff);
#else
	WriteREGMsk(0xd0, 0x1fe0039, 0x1fe0039); 
	WriteREGMsk(0xb4, 0x1fe0039, 0x1fe0039);
#endif
#if 0	
	//rear clk path
	WriteREGMsk(0x000000d8,1,0);
	WriteREGMsk(0x0001f034,1,0);
	WriteREGMsk(0x0001f034,1<<31,0);
	WriteREGMsk(0x0001f038,1<<25,0);
	//front clk path
	WriteREGMsk(0x00000034,1<<4,0);
	WriteREGMsk(0x000000d8,1<<19,1<<19);
	WriteREGMsk(0x000a48e0,1,1);
	WriteREGMsk(0x0001f034,1<<31,0);
	WriteREGMsk(0x000000d8,(0x1 << 2),(0));

	WriteREGMsk(0x5c,(0x1<<22),(0x1<<22));
	WriteREGMsk(0x94,(0x1<<6),(0x1<<6));
	WriteREGMsk(0x5c,(0xf<<6),(0xf<<6));
	WriteREGMsk(0x298,(0xf<<28),(0xf<<28));

	//cav clk ref
	WriteREGMsk(0x73c,(0x7<<3),(0));	//dec enable
	WriteREGMsk(0xd8,(0x1<<4),(1<<4)); //ckgen en.
#else
	//rear clk path
	WriteREGMsk(0x000000d8,0,1);
	WriteREGMsk(0x0001f034,0,1);
	WriteREGMsk(0x0001f034,0,1<<31);	
	if(EnPmxRearNewSD144Mode){
		WriteREGMsk(0x1f038,1<<25,1<<25);
	}else{
		WriteREGMsk(0x1f038,0<<25,1<<25);
	}
	//front clk path
	WriteREGMsk(0x00000034,0,1<<4);
	WriteREGMsk(0x000000d8,1<<19,1<<19);
	WriteREGMsk(0x000a48e0,1,1);
	WriteREGMsk(0x0001f034,0,1<<31);
	WriteREGMsk(0x000000d8,0,(0x1 << 2));

	WriteREGMsk(0x5c,(0x1<<22),(0x1<<22));
	WriteREGMsk(0x94,(0x1<<6),(0x1<<6));
	WriteREGMsk(0x5c,(0xf<<6),(0xf<<6));
	WriteREGMsk(0x298,(0xf<<28),(0xf<<28));

	//cvbs
	WriteREGMsk(0x73c,0,(0x1<<31));	//dec enable
	
	//cav clk ref
	WriteREGMsk(0x73c,0,(0x7<<3));	//dec enable
	WriteREGMsk(0xd8,(0x1<<4),(1<<4)); //ckgen en.

#endif
}


void PmxVerifyDrvInit(bool fgHwReset)
{
        if (!_fgPmxVerifyDrvInit) {
                if (fgHwReset) {
                        PMX_HalSetupSoftwareRegister();	
                        //vPmxVerifyHalSysInit(); //8317
                }
#ifndef __ARM2__
                vPmxHalIsrInit();
#endif
                //vPmxHalInit(); //8317
                _fgPmxVerifyDrvInit = TRUE;
        }

        //tmp location:register scl isr
        //move mix control register to scl isr
}

void PmxVerifyCAVSetup(unsigned int u4Mode)
{
	switch(u4Mode)
	{
		case RES_480I:
		   vPmxVerifyHalLoadSetting(&_ArCavSetting_T[0]);
		  break;
		case RES_480P:
		  vPmxVerifyHalLoadSetting(&_ArCavSetting_T[1]);
		  break;
		case RES_576I:
		  vPmxVerifyHalLoadSetting(&_ArCavSetting_T[2]);
		  break;
		case RES_576P:
		  vPmxVerifyHalLoadSetting(&_ArCavSetting_T[3]);
		  break;
		case RES_720P60HZ:
		  vPmxVerifyHalLoadSetting(&_ArCavSetting_T[4]);
		  break;
		case RES_1080I60HZ:
		  vPmxVerifyHalLoadSetting(&_ArCavSetting_T[5]);
		  break;
		case RES_1080P60HZ:
		  vPmxVerifyHalLoadSetting(&_ArCavSetting_T[6]);
		  break;		
		case RES_720P50HZ:
		  vPmxVerifyHalLoadSetting(&_ArCavSetting_T[7]);
		  break;
		case RES_1080I50HZ:
		  vPmxVerifyHalLoadSetting(&_ArCavSetting_T[8]);
		  break;
		case RES_1080P50HZ:
		  vPmxVerifyHalLoadSetting(&_ArCavSetting_T[9]);
		  break;			  
		default:
		  break;
	}

    //UTIL_Printf("[CAV] 0x2034 = %x\n",  ReadREG(0x2034));
#if 0
    switch(u4Mode)
    {
    case 480:
        if(ucInterlace)
            vPmxVerifyHalLoadCavSetting(&_ArCavSetting[6], TRUE);
        else
            vPmxVerifyHalLoadCavSetting(&_ArCavSetting[0], FALSE);
        break;
    case 576:
        if(ucInterlace)
            vPmxVerifyHalLoadCavSetting(&_ArCavSetting[7], TRUE);
        else
            vPmxVerifyHalLoadCavSetting(&_ArCavSetting[1], FALSE);
        break;
    case 720:
        if(ucTvType == PMX_TV_TYPE_NTSC) //60Hz
            vPmxVerifyHalLoadCavSetting(&_ArCavSetting[2], FALSE);
        else //50Hz
            vPmxVerifyHalLoadCavSetting(&_ArCavSetting[4], FALSE);
        break;
    case 1080:
        if(ucTvType == PMX_TV_TYPE_NTSC) //60Hz
        {
            if(ucInterlace)
                vPmxVerifyHalLoadCavSetting(&_ArCavSetting[8], FALSE);
            else
                vPmxVerifyHalLoadCavSetting(&_ArCavSetting[3], FALSE);
        }
        else
        {
            if(ucInterlace)
                vPmxVerifyHalLoadCavSetting(&_ArCavSetting[9], FALSE);
            else
                vPmxVerifyHalLoadCavSetting(&_ArCavSetting[5], FALSE);
        }
        break;
    default:
        break;
    }
#endif
}



void PmxVerifyCVBSSetup(unsigned int u4Mode, unsigned char ucTvType, unsigned char ucInterlace)
{
    switch(u4Mode)
    {
    case 480:
        if(ucInterlace)
            vPmxVerifyHalLoadCvbsSetting(&_ArCvbsSetting[6], TRUE);
        else
            vPmxVerifyHalLoadCvbsSetting(&_ArCvbsSetting[0], FALSE);
        break;
    case 576:
        if(ucInterlace)
            vPmxVerifyHalLoadCvbsSetting(&_ArCvbsSetting[7], TRUE);
        else
            vPmxVerifyHalLoadCvbsSetting(&_ArCvbsSetting[1], FALSE);
        break;
    case 720:
        if(ucTvType == PMX_TV_TYPE_NTSC) //60Hz
            vPmxVerifyHalLoadCvbsSetting(&_ArCvbsSetting[2], FALSE);
        else //50Hz
            vPmxVerifyHalLoadCvbsSetting(&_ArCvbsSetting[4], FALSE);
        break;
    case 1080:
        if(ucTvType == PMX_TV_TYPE_NTSC) //60Hz
        {
            if(ucInterlace)
                vPmxVerifyHalLoadCvbsSetting(&_ArCvbsSetting[8], FALSE);
            else
                vPmxVerifyHalLoadCvbsSetting(&_ArCvbsSetting[3], FALSE);
        }
        else
        {
            if(ucInterlace)
                vPmxVerifyHalLoadCvbsSetting(&_ArCvbsSetting[9], FALSE);
            else
                vPmxVerifyHalLoadCvbsSetting(&_ArCvbsSetting[5], FALSE);
        }
        break;
    default:
        break;
    }

}

void PmxVerifyCVBSSetup2(unsigned char ucVdoId, unsigned int ucPmxMode)
{
	switch(ucPmxMode)
	{
		case RES_480I:
		   vPmxVerifyHalLoadSetting(&_ArCvbsSetting_T[0]);
		  break;
		case RES_480P:
		  vPmxVerifyHalLoadSetting(&_ArCvbsSetting_T[1]);
		  break;
		case RES_576I:
		  vPmxVerifyHalLoadSetting(&_ArCvbsSetting_T[2]);
		  break;
		case RES_576P:
		  vPmxVerifyHalLoadSetting(&_ArCvbsSetting_T[3]);
		  break;
		default:
		  break;
	}
}

void PmxVerifySclerSetup(unsigned char ucVdoId, unsigned int ucPmxMode)
{
	if(ucVdoId == 0){
		switch(ucPmxMode)
		{
			case RES_480P_800:
			   vPmxVerifyHalLoadSetting(&_ArSclerTimingSetting[0]);
			  break;
			case RES_600P_800:
			  vPmxVerifyHalLoadSetting(&_ArSclerTimingSetting[11]);
			  break;
			case RES_600P_1024:
			  vPmxVerifyHalLoadSetting(&_ArSclerTimingSetting[12]);
			  break;			  
			case RES_720P60HZ:
			  vPmxVerifyHalLoadSetting(&_ArSclerTimingSetting[1]);
			  break;
			case RES_1080P60HZ:
			  vPmxVerifyHalLoadSetting(&_ArSclerTimingSetting[2]);
			  break;
			default:
			  break;
		}
	}else{
		switch(ucPmxMode)
		{
			case RES_480I:
			   vPmxVerifyHalLoadSetting(&_ArSclerTimingSetting[3]);
			  break;
			case RES_480P:
			  vPmxVerifyHalLoadSetting(&_ArSclerTimingSetting[4]);
			  break;
			case RES_576I:
			  vPmxVerifyHalLoadSetting(&_ArSclerTimingSetting[5]);
			  break;
			case RES_576P:
			  vPmxVerifyHalLoadSetting(&_ArSclerTimingSetting[6]);
			  break;
			case RES_720I: // no test case;no path.
			   vPmxVerifyHalLoadSetting(&_ArSclerTimingSetting[7]);
			  break;
			case RES_720P60HZ:
			  vPmxVerifyHalLoadSetting(&_ArSclerTimingSetting[8]);
			  break;
			case RES_1080I60HZ:
			  vPmxVerifyHalLoadSetting(&_ArSclerTimingSetting[9]);
			  break;
			case RES_1080I50HZ:
			  vPmxVerifyHalLoadSetting(&_ArSclerTimingSetting[9]);
			  break;
			case RES_1080P60HZ:
			  vPmxVerifyHalLoadSetting(&_ArSclerTimingSetting[10]);
			  break;			  
		    case RES_720P50HZ:
			  vPmxVerifyHalLoadSetting(&_ArSclerTimingSetting[8]);
			  break;
			case RES_1080P50HZ:
			  vPmxVerifyHalLoadSetting(&_ArSclerTimingSetting[10]);
			  break;
			  
			default:
			  break;
		}
	}
}


void PmxVerifySetVDOPtr(unsigned char ucVdoId, unsigned char ucBufIdx, unsigned int u4AddrY, unsigned int u4AddrC)
{
        //UINT64 u8YBuf, u8CBuf;
        unsigned int u4VdoRegBase, u4RegVal;
        
        //u8YBuf = AddrY - 0x100000000UL;
        //u8CBuf = AddrC - 0x100000000UL;
	
        //vPmxVerifyHalVdoPtr(ucVdoId, u8YBuf, u8CBuf);
        if(ucVdoId == 0)
                u4VdoRegBase = 0x42400;
        else if(ucVdoId == 1)
                u4VdoRegBase = 0x43400;
        else
                u4VdoRegBase = 0x43a00;

        u4RegVal = u4AddrY >> 2;
        WriteREG((u4VdoRegBase + 0x00), u4RegVal); // Y
        WriteREG((u4VdoRegBase + 0x08), u4RegVal); // X
        WriteREG((u4VdoRegBase + 0x80), u4RegVal); // W
        WriteREG((u4VdoRegBase + 0x84), u4RegVal); // Z
        WriteREG((u4VdoRegBase + 0xEC), u4RegVal); // A
        u4RegVal = u4AddrC >> 2;
        WriteREG((u4VdoRegBase + 0x04), u4RegVal); // Y
        WriteREG((u4VdoRegBase + 0x0C), u4RegVal); // X
        WriteREG((u4VdoRegBase + 0xFC), u4RegVal); // Z
}

void PmxVerifyVDOSetup(__u8 ucVdoId, __u32 u4Mode)
{
        //discard by verify? 
}


void PmxVerifyVDOSetup_WithSrcType(unsigned char ucVdoId, unsigned int u4Mode,unsigned char ucSrcType)
{
	if(ucVdoId == 0) //main vdo
	{
		switch(u4Mode)
		{
		case 576:
			vPmxVerifyHalLoadSetting(&_ArVdoSetting[1]);
			break;
		case 720:
			vPmxVerifyHalLoadSetting(&_ArVdoSetting[2]);
			break;
		case 1080:
			vPmxVerifyHalLoadSetting(&_ArVdoSetting[3]);
			break;			
		case 480:
			if(ucSrcType ==PMX_VDO_800X480 ){
				vPmxVerifyHalLoadSetting(&_ArVdoSetting[19]);
			}else{
				vPmxVerifyHalLoadSetting(&_ArVdoSetting[0]);
			}
			break;
		case 600:
			if(ucSrcType ==PMX_VDO_1024X600 ){
				vPmxVerifyHalLoadSetting(&_ArVdoSetting[21]);
			}else{
				vPmxVerifyHalLoadSetting(&_ArVdoSetting[20]);
			}
			break;
		default:
			vPmxVerifyHalLoadSetting(&_ArVdoSetting[0]);
			break;
		}
	}
	else if(ucVdoId == 1)//sub vdo
	{
		switch(u4Mode)
		{
		case 576:
			vPmxVerifyHalLoadSetting(&_ArVdoSetting[5]);
			break;
		case 720:
			vPmxVerifyHalLoadSetting(&_ArVdoSetting[6]);
			break;
		case 1080:
			vPmxVerifyHalLoadSetting(&_ArVdoSetting[7]);
			break;
		case 480:
		default:
			vPmxVerifyHalLoadSetting(&_ArVdoSetting[4]);
			break;
		}
	}


}

void PmxVerifyVDOActRegionSetup(unsigned char ucVdoId, unsigned int u4Mode, unsigned char ucTvType)
{
        if(ucVdoId == 0) //main vdo
        {
                switch(u4Mode)
                {
                case 576:
                    vPmxVerifyHalLoadSetting(&_ArVdoActRegionSetting[1]);
                    break;
                case 720:
                    if(ucTvType == PMX_TV_TYPE_NTSC)
                        vPmxVerifyHalLoadSetting(&_ArVdoActRegionSetting[2]);
                    else
                        vPmxVerifyHalLoadSetting(&_ArVdoActRegionSetting[18]);
                    break;
                case 1080:
                    if(ucTvType == PMX_TV_TYPE_NTSC_1080P_23_9)
                        vPmxVerifyHalLoadSetting(&_ArVdoActRegionSetting[8]);
                    else if(ucTvType == PMX_TV_TYPE_PAL_1080P_24)
                        vPmxVerifyHalLoadSetting(&_ArVdoActRegionSetting[9]);
                    else if(ucTvType == PMX_TV_TYPE_PAL_1080P_25)
                        vPmxVerifyHalLoadSetting(&_ArVdoActRegionSetting[10]);
                    else if(ucTvType == PMX_TV_TYPE_NTSC_1080P_29_9)
                        vPmxVerifyHalLoadSetting(&_ArVdoActRegionSetting[11]);
                    else if(ucTvType == PMX_TV_TYPE_PAL_1080P_30)
                        vPmxVerifyHalLoadSetting(&_ArVdoActRegionSetting[12]);
                    else if(ucTvType == PMX_TV_TYPE_NTSC)
                        vPmxVerifyHalLoadSetting(&_ArVdoActRegionSetting[3]);
                    else
                        vPmxVerifyHalLoadSetting(&_ArVdoActRegionSetting[19]);
                    break;
        			
        		case 480:
        			if(ucTvType == PMX_TV_TYPE_800X480){
        				vPmxVerifyHalLoadSetting(&_ArVdoActRegionSetting[39]);
        			}else{
        				vPmxVerifyHalLoadSetting(&_ArVdoActRegionSetting[0]);
        			}
        			break;
        
        		case 600:
        			if(ucTvType == PMX_TV_TYPE_1024X600){
        				vPmxVerifyHalLoadSetting(&_ArVdoActRegionSetting[41]);
        			}else{ //800*600
        				vPmxVerifyHalLoadSetting(&_ArVdoActRegionSetting[40]);
        			}
        			break;
                default:
                    vPmxVerifyHalLoadSetting(&_ArVdoActRegionSetting[0]);
                    break;
                }
        }
        else if(ucVdoId == 1)//sub vdo
        {     
         
        }
}

void PmxVerifyDispFmtHFilter(unsigned char ucVdoId, unsigned char ucYC, unsigned char ucCoef)
{
        vPmxVerifyHalDispFmtHFilter(ucVdoId, ucYC, ucCoef);
}

void vDispVdoReset(void)
{
    //DispTestAPIEntry();
    //VDO Reset
        /*  never do it here.    
            WriteREG(0x4243C, 0xFF);
            WriteREG(0x4243C, 0x00);
            WriteREG(0x4343C, 0xFF);
            WriteREG(0x4343C, 0x00);
        */
    //DispTestAPILeave();
}

void PmxVerifySetMode(unsigned char ucVdoId, unsigned int u4SrcFmt, unsigned int u4PmxFmt, unsigned int u4OutFmt, unsigned int u4CavFmt, 
        unsigned char ucTvType, unsigned char ucFit, unsigned char ucInterlace, unsigned char ucVdoInterlace ,unsigned char ucSrcType,unsigned char uc3d)
{
        unsigned char ucPmxMode = 0;
        unsigned char ucOutMode = 0;
	_u4PmxVdoIndex = ucVdoId;

	UTIL_Printf("  Input Arguments : ---> \n");
	UTIL_Printf("              ucVdoId %d:\n", ucVdoId);
	UTIL_Printf("              u4SrcFmt %d:\n", u4SrcFmt);
	UTIL_Printf("              u4PmxFmt %d:\n", u4PmxFmt);
	UTIL_Printf("              u4OutFmt %d:\n", u4OutFmt);
	UTIL_Printf("              u4CavFmt %d:\n", u4CavFmt);
	UTIL_Printf("              ucTvType %d:\n", ucTvType);
	UTIL_Printf("              ucFit %d:\n", ucFit);
	UTIL_Printf("              ucInterlace %d:\n", ucInterlace);
	UTIL_Printf("              ucVdoInterlace %d:\n", ucVdoInterlace);
	UTIL_Printf("              ucSrcType %d:\n", ucSrcType);
	UTIL_Printf("              ucColorbar %d:\n", uc3d);

	if(EnPmxRearNewSD144Mode){
		UTIL_Printf("\n Note: ----EnPmxRearNewSD144Mode Enabled----\n");
	}
        // Only 1080P output need use XHalf.
        // XHALF only support H 1/2 scale.
        if (1080 == u4PmxFmt && 0 == ucInterlace)
        {
            g_fgNeedEnableXHalf = TRUE;
        }
        else
        {
            g_fgNeedEnableXHalf = FALSE;
        }

	if(ucInterlace){
		g_rearFieldSwith = TRUE;
	}else{
		g_rearFieldSwith = FALSE;
	}

	/*
		ckg enable & vpll & vpll1 seting
	*/	
	PmxVerifyDrvClkInit(ucVdoId);
	vPmxVerifyPanelSizeSel(ucVdoId, u4PmxFmt, ucTvType);
	vPmxVerifyHalSysInit();		
	vPmxVerifyHalXfsInit(ucVdoId,u4PmxFmt,ucTvType,ucInterlace);

        PmxVerifyDrvInit(TRUE);
		
	////PmxVerifySetEmuDispFPGAenv(ucVdoId);

	//PmxVerifySetVDOPtr(ucVdoId, ucVdoId); //[xzr] because Y/C addr in verify allocated by cli test , here comment ,attention

        PmxVerifyVDOActRegionSetup(ucVdoId, u4PmxFmt, ucTvType);
        //PmxVerifyVDOSetup(ucVdoId, u4SrcFmt);
        PmxVerifyVDOSetup_WithSrcType(ucVdoId, u4SrcFmt,ucSrcType);

	if(EnPmxRearNewSD144Mode){
		if(u4PmxFmt == 480){
			PmxVerifyHal480PSD144Mode();
		}
	}	
        if(ucFit)
        {
                //vPmxVerifyHalVdoFit(u4SrcFmt, u4PmxFmt, ucVdoId);
    		vPmxVerifyHalVdoFit2(u4SrcFmt, u4PmxFmt, ucVdoId,ucSrcType,ucTvType, 0); //default 0 is block mode
        }

        switch(u4PmxFmt)
        {
                case 480:
            		if(ucTvType == PMX_TV_TYPE_800X480){
            			ucPmxMode = RES_480P_800;
            		}else{
            			ucPmxMode = RES_480P;
            		}
            	break;
                case 576:
            	ucPmxMode = RES_576P;
            	break;
            	case 600:
            		if(ucTvType == PMX_TV_TYPE_1024X600){
            			ucPmxMode = RES_600P_1024;
            		}else{
            			ucPmxMode = RES_600P_800;
            		}
            		break;	
                case 720:
            	if(ucTvType == PMX_TV_TYPE_NTSC)
            	    ucPmxMode = RES_720P60HZ;
            	else
            	    ucPmxMode = RES_720P50HZ;
            	break;
                case 1080:
            	if(ucTvType == PMX_TV_TYPE_NTSC){
            	    if(ucInterlace)
            		ucPmxMode = RES_1080I60HZ;
            	    else
            		ucPmxMode = RES_1080P60HZ;
            	}
            	else if(ucTvType == PMX_TV_TYPE_NTSC_1080P_23_9)
            	    ucPmxMode = RES_1080P23_976HZ;
            	else if(ucTvType == PMX_TV_TYPE_NTSC_1080P_29_9)
            	    ucPmxMode = RES_1080P29_97HZ;
            	else if(ucTvType == PMX_TV_TYPE_PAL_1080P_24)
            	    ucPmxMode = RES_1080P24HZ;
            	else if(ucTvType == PMX_TV_TYPE_PAL_1080P_25)
            	    ucPmxMode = RES_1080P25HZ;
            	else if(ucTvType == PMX_TV_TYPE_PAL_1080P_30)
            	    ucPmxMode = RES_1080P30HZ;
            	else{
            	    if(ucInterlace)
            		ucPmxMode = RES_1080I50HZ;
            	    else
            		ucPmxMode = RES_1080P50HZ;
            		}			
            	break;
                case 2160:
            	ucPmxMode = RES_2160P_30HZ;
            	break;
                case 2161:
            	ucPmxMode = RES_2161P_24HZ;
            	break;
                default:
            	break;
        }
    
        UTIL_Printf("[PMX] PmxMode=%s (%d)\n", szResStr[ucPmxMode], ucPmxMode);
    
        switch(u4OutFmt)
        {
                case 480:
            		if(ucTvType == PMX_TV_TYPE_800X480){
            			ucOutMode = RES_480P_800;
            		}else{	//only rear.
            			if(ucInterlace) 
            				{ucOutMode = RES_480I;} 
            			else
            				{ucOutMode = RES_480P;}
            		}
            	break;
            
                case 576:
            	if(ucInterlace)
            	    ucOutMode = RES_576I;
            	else
            	    ucOutMode = RES_576P;
            	break;
            
            	case 600:
            		if(ucTvType == PMX_TV_TYPE_1024X600){
            			ucOutMode = RES_600P_1024;
            		}else{
            			ucOutMode = RES_600P_800;
            		}
            		break;		
                case 720: //no interlace test
            	if(ucTvType == PMX_TV_TYPE_NTSC)
            	    ucOutMode = RES_720P60HZ;
            	else
            	    ucOutMode = RES_720P50HZ;
            	break;
                case 1080:
            	if(ucTvType == PMX_TV_TYPE_NTSC)
            	{
            	    if(ucInterlace)
            		ucOutMode = RES_1080I60HZ;
            	    else
            		ucOutMode = RES_1080P60HZ;
            	}
            	else if(ucTvType == PMX_TV_TYPE_PAL)
            	{
            	    if(ucInterlace)
            		ucOutMode = RES_1080I50HZ;
            	    else
            		ucOutMode = RES_1080P50HZ;
            	}
            	else if(ucTvType == PMX_TV_TYPE_PAL_1080P_24)
            	    ucOutMode = RES_1080P24HZ;
            	else if(ucTvType == PMX_TV_TYPE_PAL_1080P_25)
            	    ucOutMode = RES_1080P25HZ;
            	else if(ucTvType == PMX_TV_TYPE_PAL_1080P_30)
            	    ucOutMode = RES_1080P30HZ;
            	else if(ucTvType == PMX_TV_TYPE_NTSC_1080P_23_9)
            	    ucOutMode = RES_1080P23_976HZ;
            	else if(ucTvType == PMX_TV_TYPE_NTSC_1080P_29_9)
            	    ucOutMode = RES_1080P29_97HZ;
            	break;
                case 2160:
            	ucOutMode = RES_2160P_30HZ;
            	break;
                case 2161:
            	ucOutMode = RES_2161P_24HZ;
            	break;
                default:
            	break;
        }
    
        UTIL_Printf("[PMX] OutMode=%s (%d)\n", szResStr[ucOutMode], ucOutMode);

	if (1== ucVdoId)
	{
		if(u4CavFmt){
			PmxVerifyCAVSetup(ucOutMode);
		}else{		
			PmxVerifyCVBSSetup2(ucVdoId, ucOutMode);
		}
	}

	PmxVerifySclerSetup(ucVdoId, ucOutMode);
	
        PmxVerifyDispFmtHFilter(ucVdoId, 0, 2); //"Arg: ucVdoId, ucYC(1-Y; 2-C; 3-YC), ucCoef\n"


	if(ucVdoId ==0){
	    if((ucTvType == PMX_TV_TYPE_NTSC_1080P_23_9)||(ucTvType == PMX_TV_TYPE_NTSC_1080P_29_9))
		PMX_HalSetTvType(PMX_1, PMX_TV_TYPE_NTSC);
	    else if((ucTvType == PMX_TV_TYPE_PAL_1080P_24)||(ucTvType == PMX_TV_TYPE_PAL_1080P_25)||(ucTvType == PMX_TV_TYPE_PAL_1080P_30))
		PMX_HalSetTvType(PMX_1, PMX_TV_TYPE_PAL);
	    else
		PMX_HalSetTvType(PMX_1, ucTvType);
	}else{
	    if((ucTvType == PMX_TV_TYPE_NTSC_1080P_23_9)||(ucTvType == PMX_TV_TYPE_NTSC_1080P_29_9))
		PMX_HalSetTvType(PMX_2, PMX_TV_TYPE_NTSC);
	    else if((ucTvType == PMX_TV_TYPE_PAL_1080P_24)||(ucTvType == PMX_TV_TYPE_PAL_1080P_25)||(ucTvType == PMX_TV_TYPE_PAL_1080P_30))
		PMX_HalSetTvType(PMX_2, PMX_TV_TYPE_PAL);
	    else
		PMX_HalSetTvType(PMX_2, ucTvType);
	}

        PMX_HalSetMode(ucVdoId, ucPmxMode);
        vPmxVerifyHalEnablePMX(ucVdoId, 1);
        //PMX_HalReset(ucVdoId);
    
    	vPmxVerifyHalEnableColorBar(ucVdoId,uc3d);
    
        //vdo interlace 480i or 1080i
        if(ucVdoInterlace)
        {
            	unsigned int vFact,vdoBase;
            	if((ucPmxMode == RES_1080P60HZ) || (ucPmxMode == RES_1080I60HZ) || 
            			(ucPmxMode == RES_1080P50HZ) || (ucPmxMode == RES_1080I50HZ))
            	    PmxVerifyHal1080iExtra(ucVdoId);
            	else if(ucPmxMode == RES_480P)
            	    PmxVerifyHal480iExtra(ucVdoId);
            	else if(ucPmxMode == RES_576P)
            	    PmxVerifyHal576iExtra(ucVdoId);
            		
            		vdoBase = 0x42400+ucVdoId*0X1000;
            		vFact  = ReadREG(vdoBase+0x14) & 0xffff;		
            		WriteREGMsk((vdoBase+0x14),vFact<<1, 0xFFFF);
        }
    	
    
        if(ucVdoId == 0) //main vdo
        {
    		vPmxMixPlane(PMX_1,    PMX_HW_VIDEO_MIX);
    		vPmxNotMixPlane(PMX_1, PMX_HW_OSD1_MIX);
    		vPmxNotMixPlane(PMX_1, PMX_HW_OSD2_MIX);
    		vPmxNotMixPlane(PMX_1, PMX_HW_OSD3_MIX);
    		vPmxNotMixPlane(PMX_1, PMX_HW_OSD4_MIX);
        }
    
    
        //vDispVdoutFmtReset();
        //vDispDispFmtReset();
        vPmxHalReset(ucVdoId);
        vDispVdoReset();
}


void PmxVerifySetMode_OSD(unsigned char ucVdoId,
				unsigned int u4SrcFmt,
				unsigned int u4PmxFmt,
				unsigned int u4OutFmt,
				unsigned int u4CavFmt,
				unsigned char ucTvType,
				unsigned char ucFit,
				unsigned char ucInterlace,
				unsigned char ucVdoInterlace,
				unsigned char ucSrcType,
				unsigned char uc3d,
				bool fginit)
{
    unsigned char ucPmxMode = 0;
    unsigned char ucOutMode = 0;

	if (fginit) {
		PmxVerifyDrvClkInit(ucVdoId);
		vPmxVerifyPanelSizeSel(ucVdoId, u4PmxFmt, ucTvType);
		vPmxVerifyHalSysInit();		
		//vPmxVerifyHalXfsInit(ucVdoId,u4PmxFmt,ucTvType,ucInterlace);
	} else {
		vPmxVerifyPanelSizeSel(ucVdoId, u4PmxFmt, ucTvType);
	}

    switch(u4PmxFmt)
    {
    case 480:
	if(ucTvType == PMX_TV_TYPE_800X480){
		ucPmxMode = RES_480P_800;
	}else{
		ucPmxMode = RES_480P;
	}
        break;
    case 576:
        ucPmxMode = RES_576P;
        break;
    case 600:
	if(ucTvType == PMX_TV_TYPE_1024X600){
		ucPmxMode = RES_600P_1024;
	}else{
		ucPmxMode = RES_600P_800;
	}
	break;	
    case 720:
        if(ucTvType == PMX_TV_TYPE_NTSC)
            ucPmxMode = RES_720P60HZ;
        else
            ucPmxMode = RES_720P50HZ;
        break;
    case 1080:
        if(ucTvType == PMX_TV_TYPE_NTSC){
            if(ucInterlace)
                ucPmxMode = RES_1080I60HZ;
            else
                ucPmxMode = RES_1080P60HZ;
        }
        else if(ucTvType == PMX_TV_TYPE_NTSC_1080P_23_9)
            ucPmxMode = RES_1080P23_976HZ;
        else if(ucTvType == PMX_TV_TYPE_NTSC_1080P_29_9)
            ucPmxMode = RES_1080P29_97HZ;
        else if(ucTvType == PMX_TV_TYPE_PAL_1080P_24)
            ucPmxMode = RES_1080P24HZ;
        else if(ucTvType == PMX_TV_TYPE_PAL_1080P_25)
            ucPmxMode = RES_1080P25HZ;
        else if(ucTvType == PMX_TV_TYPE_PAL_1080P_30)
            ucPmxMode = RES_1080P30HZ;
        else{
            if(ucInterlace)
                ucPmxMode = RES_1080I50HZ;
            else
                ucPmxMode = RES_1080P50HZ;
		}			
        break;
    case 2160:
        ucPmxMode = RES_2160P_30HZ;
        break;
    case 2161:
        ucPmxMode = RES_2161P_24HZ;
        break;
    default:
        break;
    }

    switch(u4OutFmt)
    {

    case 480:
	if(ucTvType == PMX_TV_TYPE_800X480){
		ucOutMode = RES_480P_800;
	}else{  //only rear.
		if(ucInterlace) 
			{ucOutMode = RES_480I;} 
		else
			{ucOutMode = RES_480P;}
	}
        break;

    case 576:
        if(ucInterlace)
            ucOutMode = RES_576I;
        else
            ucOutMode = RES_576P;
        break;

	case 600:
		if(ucTvType == PMX_TV_TYPE_1024X600){
			ucOutMode = RES_600P_1024;
		}else{
			ucOutMode = RES_600P_800;
		}
		break;		
    case 720: //no interlace test
        if(ucTvType == PMX_TV_TYPE_NTSC)
            ucOutMode = RES_720P60HZ;
        else
            ucOutMode = RES_720P50HZ;
        break;
    case 1080:
        if(ucTvType == PMX_TV_TYPE_NTSC)
        {
            if(ucInterlace)
                ucOutMode = RES_1080I60HZ;
            else
                ucOutMode = RES_1080P60HZ;
        }
        else if(ucTvType == PMX_TV_TYPE_PAL)
        {
            if(ucInterlace)
                ucOutMode = RES_1080I50HZ;
            else
                ucOutMode = RES_1080P50HZ;
        }
        else if(ucTvType == PMX_TV_TYPE_PAL_1080P_24)
            ucOutMode = RES_1080P24HZ;
        else if(ucTvType == PMX_TV_TYPE_PAL_1080P_25)
            ucOutMode = RES_1080P25HZ;
        else if(ucTvType == PMX_TV_TYPE_PAL_1080P_30)
            ucOutMode = RES_1080P30HZ;
        else if(ucTvType == PMX_TV_TYPE_NTSC_1080P_23_9)
            ucOutMode = RES_1080P23_976HZ;
        else if(ucTvType == PMX_TV_TYPE_NTSC_1080P_29_9)
            ucOutMode = RES_1080P29_97HZ;
        break;
    case 2160:
        ucOutMode = RES_2160P_30HZ;
        break;
    case 2161:
        ucOutMode = RES_2161P_24HZ;
        break;
    default:
        break;
    }

    //UTIL_Printf("[PMX] OutMode=%s (%d)\n", szResStr[ucOutMode], ucOutMode);

    if (1== ucVdoId)
    {
    	if(u4CavFmt){
    		PmxVerifyCAVSetup(ucOutMode);
    	}else{
    		PmxVerifyCVBSSetup2(ucVdoId, ucOutMode);
    	}
    }

    if (fginit)
    	PmxVerifySclerSetup(ucVdoId, ucOutMode);

    if(ucVdoId == 0) //main vdo
    {
    	if (fginit){
			vPmxNotMixPlane(PMX_1, PMX_HW_VIDEO_MIX);
			vPmxNotMixPlane(PMX_1, PMX_HW_OSD1_MIX);
			vPmxNotMixPlane(PMX_1, PMX_HW_OSD2_MIX);
			vPmxMixPlane(PMX_1, PMX_HW_OSD3_MIX);
			vPmxMixPlane(PMX_1, PMX_HW_OSD4_MIX);
    	} else {
			//vPmxMixPlane(PMX_1, PMX_HW_VIDEO_MIX);
			vPmxNotMixPlane(PMX_1, PMX_HW_OSD1_MIX);
			//vPmxMixPlane(PMX_1, PMX_HW_OSD2_MIX);
			//vPmxMixPlane(PMX_1, PMX_HW_OSD3_MIX);
			//vPmxMixPlane(PMX_1, PMX_HW_OSD4_MIX);
		}
    }
}


//now only for avin:yuv420,raster scan color mode
void PmxVerifySetMode_720480Vdo(unsigned char ucVdoId,
				unsigned int u4SrcFmt,
				unsigned int u4PmxFmt,
				unsigned int u4OutFmt,
				unsigned int u4CavFmt,
				unsigned char ucTvType,
				unsigned char ucFit,
				unsigned char ucInterlace,
				unsigned char ucVdoInterlace,
				unsigned char ucSrcType,
				unsigned char uc3d,
				unsigned int ucScanline,
				unsigned int u4AddrY,
				unsigned int u4AddrC
				)
{
        unsigned char ucPmxMode = 0;
        unsigned char ucOutMode = 0;

        if(EnPmxRearNewSD144Mode){
		VDO_LOG(VDO_LOG_LVL_DBG, "\n Note: ----EnPmxRearNewSD144Mode Enabled----\n");
	}
        
        if (1080 == u4PmxFmt && 0 == ucInterlace)
        {
                g_fgNeedEnableXHalf = TRUE;
        }
        else
        {
                g_fgNeedEnableXHalf = FALSE;
        }

        if(ucInterlace){
		g_rearFieldSwith = TRUE;
	}else{
		g_rearFieldSwith = FALSE;
	}

        /*
		ckg enable & vpll & vpll1 seting
	*/	
	//PmxVerifyDrvClkInit3365(ucVdoId);
	//vPmxVerifyPanelSizeSel3365(ucVdoId, u4PmxFmt, ucTvType);
	//vPmxVerifyHalSysInit3365();		
	//vPmxVerifyHalXfsInit3365(ucVdoId,u4PmxFmt,ucTvType,ucInterlace);

        //PmxVerifyDrvInit3365();

        PmxVerifySetVDOPtr(ucVdoId, ucVdoId, u4AddrY, u4AddrC);
        PmxVerifyVDOActRegionSetup(ucVdoId, u4PmxFmt, ucTvType);
        PmxVerifyVDOSetup_WithSrcType(ucVdoId, u4SrcFmt,ucSrcType);

        if(EnPmxRearNewSD144Mode){
		if(u4PmxFmt == 480){
			//PmxVerifyHal480PSD144Mode(); //todo
		}
	}

        if(ucFit)
        {
                //vPmxVerifyHalVdoFit(u4SrcFmt, u4PmxFmt, ucVdoId);
		vPmxVerifyHalVdoFit2(u4SrcFmt, u4PmxFmt, ucVdoId, ucSrcType, ucTvType, ucScanline);
        }

        switch(u4PmxFmt)
        {
            case 480:
        		if(ucTvType == PMX_TV_TYPE_800X480){
        			ucPmxMode = RES_480P_800;
        		}else{
        			ucPmxMode = RES_480P;
        		}
                break;
            case 576:
                ucPmxMode = RES_576P;
                break;
        	case 600:
        		if(ucTvType == PMX_TV_TYPE_1024X600){
        			ucPmxMode = RES_600P_1024;
        		}else{
        			ucPmxMode = RES_600P_800;
        		}
        		break;	
            case 720:
                if(ucTvType == PMX_TV_TYPE_NTSC)
                    ucPmxMode = RES_720P60HZ;
                else
                    ucPmxMode = RES_720P50HZ;
                break;
            case 1080:
                if(ucTvType == PMX_TV_TYPE_NTSC){
                    if(ucInterlace)
                        ucPmxMode = RES_1080I60HZ;
                    else
                        ucPmxMode = RES_1080P60HZ;
                }
                else if(ucTvType == PMX_TV_TYPE_NTSC_1080P_23_9)
                    ucPmxMode = RES_1080P23_976HZ;
                else if(ucTvType == PMX_TV_TYPE_NTSC_1080P_29_9)
                    ucPmxMode = RES_1080P29_97HZ;
                else if(ucTvType == PMX_TV_TYPE_PAL_1080P_24)
                    ucPmxMode = RES_1080P24HZ;
                else if(ucTvType == PMX_TV_TYPE_PAL_1080P_25)
                    ucPmxMode = RES_1080P25HZ;
                else if(ucTvType == PMX_TV_TYPE_PAL_1080P_30)
                    ucPmxMode = RES_1080P30HZ;
                else{
                    if(ucInterlace)
                        ucPmxMode = RES_1080I50HZ;
                    else
                        ucPmxMode = RES_1080P50HZ;
        		}			
                break;
            case 2160:
                ucPmxMode = RES_2160P_30HZ;
                break;
            case 2161:
                ucPmxMode = RES_2161P_24HZ;
                break;
            default:
                break;
        }

        VDO_LOG(VDO_LOG_LVL_DBG, "[PMX] PmxMode=%s (%d)\n", szResStr[ucPmxMode], ucPmxMode);

        switch(u4OutFmt)
        {

            case 480:
        		if(ucTvType == PMX_TV_TYPE_800X480){
        			ucOutMode = RES_480P_800;
        		}else{  //only rear.
        			if(ucInterlace) 
        				{ucOutMode = RES_480I;} 
        			else
        				{ucOutMode = RES_480P;}
        		}
                break;

            case 576:
                if(ucInterlace)
                    ucOutMode = RES_576I;
                else
                    ucOutMode = RES_576P;
                break;

        	case 600:
        		if(ucTvType == PMX_TV_TYPE_1024X600){
        			ucOutMode = RES_600P_1024;
        		}else{
        			ucOutMode = RES_600P_800;
        		}
        		break;		
            case 720: //no interlace test
                if(ucTvType == PMX_TV_TYPE_NTSC)
                    ucOutMode = RES_720P60HZ;
                else
                    ucOutMode = RES_720P50HZ;
                break;
            case 1080:
                if(ucTvType == PMX_TV_TYPE_NTSC)
                {
                    if(ucInterlace)
                        ucOutMode = RES_1080I60HZ;
                    else
                        ucOutMode = RES_1080P60HZ;
                }
                else if(ucTvType == PMX_TV_TYPE_PAL)
                {
                    if(ucInterlace)
                        ucOutMode = RES_1080I50HZ;
                    else
                        ucOutMode = RES_1080P50HZ;
                }
                else if(ucTvType == PMX_TV_TYPE_PAL_1080P_24)
                    ucOutMode = RES_1080P24HZ;
                else if(ucTvType == PMX_TV_TYPE_PAL_1080P_25)
                    ucOutMode = RES_1080P25HZ;
                else if(ucTvType == PMX_TV_TYPE_PAL_1080P_30)
                    ucOutMode = RES_1080P30HZ;
                else if(ucTvType == PMX_TV_TYPE_NTSC_1080P_23_9)
                    ucOutMode = RES_1080P23_976HZ;
                else if(ucTvType == PMX_TV_TYPE_NTSC_1080P_29_9)
                    ucOutMode = RES_1080P29_97HZ;
                break;
            case 2160:
                ucOutMode = RES_2160P_30HZ;
                break;
            case 2161:
                ucOutMode = RES_2161P_24HZ;
                break;
            default:
                break;
        }

        VDO_LOG(VDO_LOG_LVL_DBG, "[PMX] OutMode=%s (%d)\n", szResStr[ucOutMode], ucOutMode);

        if (1== ucVdoId)
	{
		if(u4CavFmt){
			PmxVerifyCAVSetup(ucOutMode);
		}else{		
			PmxVerifyCVBSSetup2(ucVdoId, ucOutMode);
		}
	}

        //PmxVerifySclerSetup(ucVdoId, ucOutMode);

        PmxVerifyDispFmtHFilter(ucVdoId, 0, 2); //"Arg: ucVdoId, ucYC(1-Y; 2-C; 3-YC), ucCoef\n"

        if(ucVdoId ==0) {
	        if((ucTvType == PMX_TV_TYPE_NTSC_1080P_23_9)||(ucTvType == PMX_TV_TYPE_NTSC_1080P_29_9))
	            PMX_HalSetTvType(PMX_1, PMX_TV_TYPE_NTSC);
	        else if((ucTvType == PMX_TV_TYPE_PAL_1080P_24)||(ucTvType == PMX_TV_TYPE_PAL_1080P_25)||(ucTvType == PMX_TV_TYPE_PAL_1080P_30))
	            PMX_HalSetTvType(PMX_1, PMX_TV_TYPE_PAL);
	        else
	            PMX_HalSetTvType(PMX_1, ucTvType);
	} else {
	        if((ucTvType == PMX_TV_TYPE_NTSC_1080P_23_9)||(ucTvType == PMX_TV_TYPE_NTSC_1080P_29_9))
	            PMX_HalSetTvType(PMX_2, PMX_TV_TYPE_NTSC);
	        else if((ucTvType == PMX_TV_TYPE_PAL_1080P_24)||(ucTvType == PMX_TV_TYPE_PAL_1080P_25)||(ucTvType == PMX_TV_TYPE_PAL_1080P_30))
	            PMX_HalSetTvType(PMX_2, PMX_TV_TYPE_PAL);
	        else
	            PMX_HalSetTvType(PMX_2, ucTvType);
	}

        PMX_HalSetMode(ucVdoId, ucPmxMode);
        vPmxVerifyHalEnablePMX(ucVdoId, 1);
        //PMX_HalReset(ucVdoId);
    
    	vPmxVerifyHalEnableColorBar(ucVdoId,uc3d);
    
        //vdo interlace 480i or 1080i
        if(ucVdoInterlace) 
        {
        #if 0 //todo interlace
        	unsigned int vFact,vdoBase;
                if((ucPmxMode == RES_1080P60HZ) || (ucPmxMode == RES_1080I60HZ) || 
        			(ucPmxMode == RES_1080P50HZ) || (ucPmxMode == RES_1080I50HZ))
                    PmxVerifyHal1080iExtra(ucVdoId);
                else if(ucPmxMode == RES_480P)
                    PmxVerifyHal480iExtra(ucVdoId);
                else if(ucPmxMode == RES_576P)
                    PmxVerifyHal576iExtra(ucVdoId);
    		
    		vdoBase = 0x42400+ucVdoId*0X1000;
    		vFact  = ReadREG(vdoBase+0x14) & 0xffff;		
    		WriteREGMsk((vdoBase+0x14),vFact<<1, 0xFFFF);
        #endif                
        }
	

        if(ucVdoId == 0) //main vdo
        {
    		vPmxMixPlane(PMX_1,    PMX_HW_VIDEO_MIX);
    		//vPmxNotMixPlane(PMX_1, PMX_HW_OSD1_MIX);
    		//vPmxNotMixPlane(PMX_1, PMX_HW_OSD2_MIX);
    		//vPmxNotMixPlane(PMX_1, PMX_HW_OSD3_MIX);
    		//vPmxNotMixPlane(PMX_1, PMX_HW_OSD4_MIX);
        }
    
    
        //vDispVdoutFmtReset();
        //vDispDispFmtReset();
        //vPmxHalReset(ucVdoId);
        vDispVdoReset();
}

