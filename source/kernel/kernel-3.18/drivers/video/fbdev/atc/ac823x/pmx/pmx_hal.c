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
/*****************************************************************************
*  Plane Mixer: Interface
*****************************************************************************/

#ifndef _PMX_HAL_C_
#define _PMX_HAL_C_

#ifndef __ARM2__
#include <linux/module.h>
#include <linux/interrupt.h>
#include <media/atc/ac823x/pmx_hal.h>
#include <media/atc/drv_osd_if.h>
#include <media/atc/display_inc.h>
#include "windows.h"
#include "winutil.h"
#else
#include "x_types.h"
#include "assert.h"
#include "pmx_hal.h"
#include "drv_osd_if.h"
#include "display_inc.h"
#endif
#include "chip_ver.h"
#include "vdp_hw.h"
#include "vdp_hal.h"
#include "vdp.h"
#include "pmx_hw.h"
#include "log.h"
#include "pmx_vfy_hal.h"
#include "x_hal_ic.h"
#include "drv_config.h"
#include "irqs_vector.h"
//#include "drv_env.h"
#include "x_bim.h"
#include "x_os.h"

#define UTIL_Printf(a,...)

volatile bool PmxVerifyDoRestInVyncFront = FALSE;
volatile bool PmxVerifyDoRestInVyncRear = FALSE;
static bool _fgPmxMainIsrInited = FALSE;
static bool _fgPmxAuxIsrInited = FALSE;


/*** sw reg ***/
HAL_PMX_DISP_MAIN_UNION_T *pDcmPmxDispMainSwReg;
HAL_PMX_DISP_MAIN_UNION_T *pDcmPmxDispAuxSwReg;
//HAL_PMX_VDOUT_MAIN_UNION_T *pDcmPmxVdoutMainSwReg;
//HAL_PMX_VDOUT_AUX_UNION_T *pDcmPmxVdoutAuxSwReg;
HAL_PMX_DISP_MAIN_C_UNION_T *pDcmPmxDispMainCSwReg;
HAL_PMX_DISP_MAIN_C_UNION_T *pDcmPmxDispAuxCSwReg;
//HAL_PMX_SCLER_MAIN_UNION_T _rPmxSclerMainSwReg;
//HAL_PMX_SCLER_AUX_UNION_T _rPmxSclerAuxSwReg;
HAL_PMX_DISP_MAIN_UNION_T _rPmxDispMainSwReg;
HAL_PMX_DISP_MAIN_UNION_T _rPmxDispAuxSwReg;
//HAL_PMX_VDOUT_MAIN_UNION_T _rPmxVdoutMainSwReg;
//HAL_PMX_VDOUT_AUX_UNION_T _rPmxVdoutAuxSwReg;
HAL_PMX_DISP_MAIN_C_UNION_T _rPmxDispMainCSwReg;
HAL_PMX_DISP_MAIN_C_UNION_T _rPmxDispAuxCSwReg;
PMX_HAL_MIX_UNION_T   _rPmxHalMixSwReg;

/*** hw reg ***/
volatile HAL_PMX_DISP_MAIN_UNION_T*  _prPmxDispMainHwReg;// = (HAL_PMX_DISP_MAIN_UNION_T*)HAL_PMX_DISP_MAIN_REG;
volatile HAL_PMX_DISP_MAIN_UNION_T*  _prPmxDispAuxHwReg;// = (HAL_PMX_DISP_MAIN_UNION_T*)HAL_PMX_DISP_AUX_REG;
volatile HAL_PMX_DISP_MAIN_C_UNION_T*  _prPmxDispMainCHwReg;// = (HAL_PMX_DISP_MAIN_C_UNION_T*)HAL_PMX_DISP_MAIN_C_REG;
volatile HAL_PMX_DISP_MAIN_C_UNION_T*  _prPmxDispAuxCHwReg;// = (HAL_PMX_DISP_MAIN_C_UNION_T*)HAL_PMX_DISP_AUX_C_REG;
volatile PMX_HAL_MIX_UNION_T *  _prPmxHalMixHwReg;
volatile PMX_HAL_MIX_UNION_T *  _prPmxHalMix2HwReg;// = (PMX_HAL_MIX_UNION_T *)PMX_HAL_MIX_REG;

UINT64 *pPmxSclerMainRegMode;
UINT64 *pPmxSclerAuxRegMode;
UINT64 *pPmxDispMainRegMode;
UINT64 *pPmxDispAuxRegMode;
UINT64 *pPmxVdoutMainRegMode;
UINT64 *pPmxVdoutAuxRegMode;
UINT64 *pPmxDispMainCRegMode;
UINT64 *pPmxDispAuxCRegMode;
UCHAR _rPmxSclerMainRegMode[HAL_PMX_SCLER_MAIN_REG_NUM];
UCHAR _rPmxSclerAuxRegMode[HAL_PMX_SCLER_AUX_REG_NUM];
UCHAR _rPmxDispMainRegMode[HAL_PMX_DISP_MAIN_REG_NUM];
UCHAR _rPmxDispAuxRegMode[HAL_PMX_DISP_AUX_REG_NUM];
UCHAR _rPmxVdoutMainRegMode[HAL_PMX_VDOUT_MAIN_REG_NUM];
UCHAR _rPmxVdoutAuxRegMode[HAL_PMX_VDOUT_AUX_REG_NUM];
UCHAR _rPmxDispMainCRegMode[HAL_PMX_DISP_MAIN_C_REG_NUM];
UCHAR _rPmxDispAuxCRegMode[HAL_PMX_DISP_AUX_C_REG_NUM];
UCHAR _rPmxMixRegMode[PMX_HAL_MIX_REG_NUM];


extern unsigned long IO_BASE_BRINGUP;
#define REG_MASK(idx)           ((UINT64)1LL << (idx))
#define IS_REG_SET(r, mask)     ((r) & (mask))
#define REG_SET(r, mask)        ((r) |= (mask))
#define WriteREG(arg, val) (*(volatile __u32*)(IO_BASE_BRINGUP + (arg)) = val)
#define ReadREG(arg)       (*(volatile __u32*)(IO_BASE_BRINGUP + (arg)))
#define WriteREGMsk(arg, val, msk) WriteREG((arg), (ReadREG(arg) & (~(msk))) | ((val) & (msk)))
#define WriteRegMsk(arg, val, msk) WriteREG((arg), (ReadREG(arg) & (~(msk))) | ((val) & (msk)))

//[xzr]todo,ttl gpio interface

void getPmxHwToSw(void)
{
	int regcnt = 0;
	for (; regcnt < PMX_HAL_MIX_REG_NUM * 4; regcnt += 4) {
		_rPmxHalMixSwReg.au4Reg[regcnt] = _prPmxHalMixHwReg->au4Reg[regcnt];
		VDO_LOG(VDO_LOG_LVL_DBG, "[YZQ]---------------_rPmxHalMixSwReg.au4Reg[%d]:0x%x \r\n",
			regcnt, _rPmxHalMixSwReg.au4Reg[regcnt]); 
	}
}

//tmp solution for get hw ioreg instead of read from dts
void getpmxbaseaddr(void)       
{
	_prPmxHalMixHwReg = (PMX_HAL_MIX_UNION_T *)(IO_BASE_BRINGUP + 0x1f000);// yzq hw
	getPmxHwToSw();
	//_rPmxHalMixSwReg.rField.u4MIX_LAYER2_SEL = 0x1;
	_prPmxHalMixHwReg->au4Reg[0/4] = _rPmxHalMixSwReg.au4Reg[0/4];

	_prPmxHalMix2HwReg = (PMX_HAL_MIX_UNION_T *)(IO_BASE_BRINGUP + 0x1f054);
	_prPmxDispMainHwReg = (HAL_PMX_DISP_MAIN_UNION_T *)(IO_BASE_BRINGUP + 0x42000);
	_prPmxDispAuxHwReg = (HAL_PMX_DISP_MAIN_UNION_T *)(IO_BASE_BRINGUP + 0x43000);
	_prPmxDispMainCHwReg = (HAL_PMX_DISP_MAIN_C_UNION_T*)(IO_BASE_BRINGUP + 0x42d00);
	_prPmxDispAuxCHwReg = (HAL_PMX_DISP_MAIN_C_UNION_T*)(IO_BASE_BRINGUP + 0x43d00);
}

void PMX_HalSetupSoftwareRegister(void)
{
    // dispfmt
    pDcmPmxDispMainSwReg = &_rPmxDispMainSwReg;
    pPmxDispMainRegMode = (UINT64 *)&_rPmxDispMainRegMode;
    // dispfmt_pip
    pDcmPmxDispAuxSwReg = &_rPmxDispAuxSwReg;
    pPmxDispAuxRegMode = (UINT64 *)&_rPmxDispAuxRegMode;

    // vdout_fmt
    //pDcmPmxVdoutMainSwReg = &_rPmxVdoutMainSwReg;
    //pPmxVdoutMainRegMode = (UINT64 *)&_rPmxVdoutMainRegMode;
    // vdout_fmt_pip
    //pDcmPmxVdoutAuxSwReg = &_rPmxVdoutAuxSwReg;
    //pPmxVdoutAuxRegMode = (UINT64 *)&_rPmxVdoutAuxRegMode;

    //timing scler
    //pDcmPmxSclerMainSwReg = &_rPmxSclerMainSwReg;
    //pPmxSclerMainRegMode = (UINT64 *)&_rPmxSclerMainRegMode;

    //pDcmPmxSclerAuxSwReg = &_rPmxSclerAuxSwReg;
    //pPmxSclerAuxRegMode = (UINT64 *)&_rPmxSclerAuxRegMode;

    // dispfmt CS
    pDcmPmxDispMainCSwReg = &_rPmxDispMainCSwReg;
    pPmxDispMainCRegMode = (UINT64 *)&_rPmxDispMainCRegMode;
    // dispfmt_pip CS
    pDcmPmxDispAuxCSwReg = &_rPmxDispAuxCSwReg;
    pPmxDispAuxCRegMode = (UINT64 *)&_rPmxDispAuxCRegMode;
}

void vPmxHalInit(void)
{
	
}

void vPmxHalRstInVSync(__u8 ucPmxId)
{
	if (ucPmxId == PMX_1) {
		PmxVerifyDoRestInVyncFront = TRUE;
	} else {
		PmxVerifyDoRestInVyncRear = TRUE;
	}
}

void vPmxHalReset(__u8 ucPmxId)
{
	vPmxHalRstInVSync(ucPmxId);
}
EXPORT_SYMBOL(vPmxHalReset);

void vPmxHalSetMasterMode(bool fgEnable)
{
        //[xzr]
}
EXPORT_SYMBOL(vPmxHalSetMasterMode);

extern bool EnPmxRearNewSD144Mode;

void PMX_HalSetMode(UCHAR ucPmxId, UCHAR ucFmt)
{
	HAL_PMX_DISP_MAIN_UNION_T *pDcmPmxSwReg;
	volatile HAL_PMX_DISP_MAIN_UNION_T*  pDcmPmxHwReg = (ucPmxId==0) ? _prPmxDispMainHwReg : _prPmxDispAuxHwReg;

	if(ucPmxId==0){
		pDcmPmxSwReg = pDcmPmxDispMainSwReg;
	}else{
		pDcmPmxSwReg = pDcmPmxDispAuxSwReg;
	}

        // special case, don't wait VSYNC, since ISR may NOT happen
        if (ucPmxId == PMX_1)  //main vdo 0
        {
    	        //pDcmPmxSwReg->rField.fgDownScaleMode=1;
                pDcmPmxSwReg->rField.u2C4Default = 0x10EB;
                switch(ucFmt)
                {
		case RES_480P_800:
                        pDcmPmxSwReg->rField.fgHD_ON = 0;
                        pDcmPmxSwReg->rField.fgHD_TP = 0;
  		        pDcmPmxSwReg->rField.fgADJ_T = 1;
			pDcmPmxSwReg->rField.u2H_TOTAL = 0x3e8;
			//pDcmPmxSwReg->rField.u2H_TOTAL_MIX = 0x3b9;
			
			pDcmPmxSwReg->rField.u2V_TOTAL = 0x20d;
			//pDcmPmxSwReg->rField.u2V_TOTAL_MIX=0x20d;
			
		        pDcmPmxSwReg->rField.fgNOT_PST_D2 = 0;
		        pDcmPmxSwReg->rField.fgEN_FIRST_PXL_LEAD = 0;
		        pDcmPmxSwReg->rField.u1FIRST_PXL_LEAD = 0;
		        pDcmPmxSwReg->rField.fgFACT_PREC = 1;
		        pDcmPmxSwReg->rField.fgAverage422to444 = 1;

			pDcmPmxSwReg->rField.fgNEW_SD_MODE = 1; //new_sd_144m
			pDcmPmxSwReg->rField.fgDSDCLK_EANBLE = 1; //new_sd_144m

			pDcmPmxSwReg->rField.horizontal_use_3fs = 1;
			pDcmPmxSwReg->rField.new_hd= 0;	
			pDcmPmxSwReg->rField.fgNOT_PST_D2 = 0;			
			break;
		
                case RES_480I:
                case RES_480P:
                        pDcmPmxSwReg->rField.fgHD_ON = 0;
                        pDcmPmxSwReg->rField.fgHD_TP = 0;
                        pDcmPmxSwReg->rField.fgADJ_T = 0;
                        pDcmPmxSwReg->rField.fgADJ_T_MIX = 0;
                        pDcmPmxSwReg->rField.fgNOT_PST_D2 = 0;
                        pDcmPmxSwReg->rField.fgEN_FIRST_PXL_LEAD = 0;
                        pDcmPmxSwReg->rField.u1FIRST_PXL_LEAD = 0;
                        pDcmPmxSwReg->rField.fgFACT_PREC = 1;
                        pDcmPmxSwReg->rField.fgAverage422to444 = 1;
                        pDcmPmxSwReg->rField.fgAverage422to444 = 1;
                        break;
                        
                case RES_576I:
                case RES_576P:
                        pDcmPmxSwReg->rField.fgHD_ON = 0;
                        pDcmPmxSwReg->rField.fgHD_TP = 0;
                        pDcmPmxSwReg->rField.fgADJ_T = 0;
                        pDcmPmxSwReg->rField.fgADJ_T_MIX = 0;
                        pDcmPmxSwReg->rField.fgNOT_PST_D2 = 0;
                        pDcmPmxSwReg->rField.fgEN_FIRST_PXL_LEAD = 0;
                        pDcmPmxSwReg->rField.u1FIRST_PXL_LEAD = 0;
                        pDcmPmxSwReg->rField.fgFACT_PREC = 1;
                        pDcmPmxSwReg->rField.fgAverage422to444 = 1;
                        break;

                case RES_720P60HZ:
                case RES_720P50HZ:
                case RES_720P23HZ:
                case RES_720P24HZ:
                case RES_720P25HZ:
                case RES_720P30HZ:
                        pDcmPmxSwReg->rField.fgADJ_T = 0;
                        pDcmPmxSwReg->rField.fgADJ_T_MIX = 0;

                        if ((ucFmt == RES_720P23HZ) || (ucFmt == RES_720P24HZ)||(ucFmt == RES_3D_720P23HZ_TB) || (ucFmt == RES_3D_720P23HZ_SBS_HALF)||
                                (ucFmt == RES_3D_720P24HZ_TB) || (ucFmt == RES_3D_720P24HZ_SBS_HALF) || (ucFmt == RES_720P30HZ) ||
                                (ucFmt == RES_3D_720P30HZ_TB) || (ucFmt == RES_3D_720P30HZ_SBS_HALF))
                        {
                                pDcmPmxSwReg->rField.u2H_TOTAL = 3300;
                                pDcmPmxSwReg->rField.u2H_TOTAL_MIX = 3300;

                                pDcmPmxSwReg->rField.u2V_TOTAL = 750;
                                pDcmPmxSwReg->rField.u2V_TOTAL_MIX=750;
                                pDcmPmxSwReg->rField.fgADJ_T = 1;
                                pDcmPmxSwReg->rField.fgADJ_T_MIX = 1;
                        }
                        else if ((ucFmt == RES_720P25HZ) || (ucFmt == RES_3D_720P25HZ_TB)||(ucFmt == RES_3D_720P25HZ_SBS_HALF))
                        {
                                pDcmPmxSwReg->rField.u2H_TOTAL = 3960;
                                pDcmPmxSwReg->rField.u2H_TOTAL_MIX = 3960;

                                pDcmPmxSwReg->rField.u2V_TOTAL = 750;
                                pDcmPmxSwReg->rField.u2V_TOTAL_MIX=750;
                                pDcmPmxSwReg->rField.fgADJ_T = 1;
                                pDcmPmxSwReg->rField.fgADJ_T_MIX = 1;

                        }
		
                        pDcmPmxSwReg->rField.fgHD_ON = 1;
                        pDcmPmxSwReg->rField.fgHD_TP = 1;
                        //pDcmPmxSwReg->rField.fgNOT_PST_D2 = 0; //for 3365
                        pDcmPmxSwReg->rField.fgEN_FIRST_PXL_LEAD = 0;
                        pDcmPmxSwReg->rField.u1FIRST_PXL_LEAD = 0;
                        pDcmPmxSwReg->rField.fgFACT_PREC = 1;
			/*new add for 3365*/
			if(ucFmt == RES_720P60HZ){
				pDcmPmxSwReg->rField.u2V_TOTAL = 0x2ee;
				pDcmPmxSwReg->rField.u2H_TOTAL = 0x672;				

			}else if(ucFmt == RES_720P50HZ){
				pDcmPmxSwReg->rField.u2V_TOTAL = 0x2ee;
				pDcmPmxSwReg->rField.u2H_TOTAL = 0x7bc;				
			}
			pDcmPmxSwReg->rField.horizontal_use_2fs = 1;
			pDcmPmxSwReg->rField.new_hd= 1;	
			pDcmPmxSwReg->rField.fgNOT_PST_D2 = 1; 
			
                        pDcmPmxSwReg->rField.fgAverage422to444 = 1;
                        break;
                        
		case RES_600P_1024:			
			pDcmPmxSwReg->rField.fgADJ_T = 1;
			pDcmPmxSwReg->rField.fgADJ_T_MIX = 0;
					
			pDcmPmxSwReg->rField.fgHD_ON = 1;
			pDcmPmxSwReg->rField.fgHD_TP = 0;
			pDcmPmxSwReg->rField.fgEN_FIRST_PXL_LEAD = 0;
			pDcmPmxSwReg->rField.u1FIRST_PXL_LEAD = 0;
			
			pDcmPmxSwReg->rField.fgFACT_PREC = 1;
			pDcmPmxSwReg->rField.u2V_TOTAL = 0x271;
			pDcmPmxSwReg->rField.u2H_TOTAL = 0x540; 			

			pDcmPmxSwReg->rField.horizontal_use_2fs = 1;
			pDcmPmxSwReg->rField.new_hd= 1; 
			pDcmPmxSwReg->rField.fgNOT_PST_D2 = 1; 
			
			pDcmPmxSwReg->rField.fgAverage422to444 = 1;
			break;
                        
		case RES_600P_800:
			//pDcmPmxSwReg->rField.fgADJ_T = 0;
			pDcmPmxSwReg->rField.fgADJ_T_MIX = 0;						
			pDcmPmxSwReg->rField.fgHD_ON = 1;
			pDcmPmxSwReg->rField.fgHD_TP = 0;
			//pDcmPmxSwReg->rField.fgNOT_PST_D2 = 0; //for 3365
			pDcmPmxSwReg->rField.fgEN_FIRST_PXL_LEAD = 0;
			pDcmPmxSwReg->rField.u1FIRST_PXL_LEAD = 0;
			pDcmPmxSwReg->rField.fgFACT_PREC = 1;
			/*new add for 3365*/
			pDcmPmxSwReg->rField.fgADJ_T =1;
			
			pDcmPmxSwReg->rField.u2V_TOTAL = 0x294;
			pDcmPmxSwReg->rField.u2H_TOTAL = 0x3f2; 			

			pDcmPmxSwReg->rField.horizontal_use_4fs = 1;
			pDcmPmxSwReg->rField.new_hd= 1; 
			pDcmPmxSwReg->rField.fgNOT_PST_D2 = 1; 
			pDcmPmxSwReg->rField.fgAverage422to444 = 1;
			break;

		
                case RES_1080P60HZ:
                case RES_1080P50HZ:
                case RES_1080I60HZ:
                case RES_1080I50HZ:
                        pDcmPmxSwReg->rField.fgHD_ON = 1;
                        pDcmPmxSwReg->rField.fgHD_TP = 0;
                        pDcmPmxSwReg->rField.fgADJ_T = 0;
                        pDcmPmxSwReg->rField.fgADJ_T_MIX = 0;
                        //pDcmPmxSwReg->rField.fgNOT_PST_D2 = 0;
                        pDcmPmxSwReg->rField.fgEN_FIRST_PXL_LEAD = 0;
                        pDcmPmxSwReg->rField.u1FIRST_PXL_LEAD = 0;
                        pDcmPmxSwReg->rField.fgFACT_PREC = 1;
                        pDcmPmxSwReg->rField.fgAverage422to444 = 1;

			/*new add for 3365*/
			if(ucFmt == RES_1080P60HZ){
				pDcmPmxSwReg->rField.u2V_TOTAL = 1125;
				pDcmPmxSwReg->rField.u2H_TOTAL = 2200;				

			}else if(ucFmt == RES_1080P50HZ){
				pDcmPmxSwReg->rField.u2V_TOTAL = 1125;
				pDcmPmxSwReg->rField.u2H_TOTAL = 2640;				
			}
			pDcmPmxSwReg->rField.horizontal_use_fs = 1;
			pDcmPmxSwReg->rField.new_hd= 1;	
			pDcmPmxSwReg->rField.fgNOT_PST_D2 = 1;

                        break;

                case RES_1080P24HZ:
                case RES_1080P23_976HZ:
                        pDcmPmxSwReg->rField.fgHD_ON = 1;
                        pDcmPmxSwReg->rField.fgHD_TP = 1;
                        pDcmPmxSwReg->rField.u2V_TOTAL = 0x465;
                        pDcmPmxSwReg->rField.u2H_TOTAL = 0xABE;
                        pDcmPmxSwReg->rField.fgADJ_T = 1;
                        pDcmPmxSwReg->rField.u2V_TOTAL_MIX = 0x465;
                        pDcmPmxSwReg->rField.u2H_TOTAL_MIX = 0xABE;
                        pDcmPmxSwReg->rField.fgADJ_T_MIX = 1;
                        pDcmPmxSwReg->rField.fgNOT_PST_D2 = 0;
                        pDcmPmxSwReg->rField.fgEN_FIRST_PXL_LEAD = 0;
                        pDcmPmxSwReg->rField.u1FIRST_PXL_LEAD = 0;
                        pDcmPmxSwReg->rField.fgFACT_PREC = 1;
                        pDcmPmxSwReg->rField.fgAverage422to444 = 1;
                        break;

                case RES_1080P25HZ:
                        pDcmPmxSwReg->rField.fgHD_ON = 1;
                        pDcmPmxSwReg->rField.fgHD_TP = 1;
                        pDcmPmxSwReg->rField.u2V_TOTAL = 0x465;
                        pDcmPmxSwReg->rField.u2H_TOTAL = 0xA50;
                        pDcmPmxSwReg->rField.fgADJ_T = 1;
                        pDcmPmxSwReg->rField.u2V_TOTAL_MIX = 0x465;
                        pDcmPmxSwReg->rField.u2H_TOTAL_MIX = 0xA50;
                        pDcmPmxSwReg->rField.fgADJ_T_MIX = 1;
                        pDcmPmxSwReg->rField.fgNOT_PST_D2 = 0;
                        pDcmPmxSwReg->rField.fgEN_FIRST_PXL_LEAD = 0;
                        pDcmPmxSwReg->rField.u1FIRST_PXL_LEAD = 0;
                        pDcmPmxSwReg->rField.fgFACT_PREC = 1;
                        pDcmPmxSwReg->rField.fgAverage422to444 = 1;
                        break;

                case RES_1080P30HZ:
                case RES_1080P29_97HZ:
                        pDcmPmxSwReg->rField.fgHD_ON = 1;
                        pDcmPmxSwReg->rField.fgHD_TP = 1;
                        pDcmPmxSwReg->rField.u2V_TOTAL = 0x465;
                        pDcmPmxSwReg->rField.u2H_TOTAL = 0x898;
                        pDcmPmxSwReg->rField.fgADJ_T = 1;
                        pDcmPmxSwReg->rField.u2V_TOTAL_MIX = 0x465;
                        pDcmPmxSwReg->rField.u2H_TOTAL_MIX = 0x898;
                        pDcmPmxSwReg->rField.fgADJ_T_MIX = 1;
                        pDcmPmxSwReg->rField.fgNOT_PST_D2 = 0;
                        pDcmPmxSwReg->rField.fgEN_FIRST_PXL_LEAD = 0;
                        pDcmPmxSwReg->rField.u1FIRST_PXL_LEAD = 0;
                        pDcmPmxSwReg->rField.fgFACT_PREC = 1;
                        pDcmPmxSwReg->rField.fgAverage422to444 = 1;
                        break;

                        //ToDo: add settings for 1080P25HZ, 1080P30HZ, 1080P24HZ
                        //(1)0x30d4 bit28 ] 1, BNH_Total, V_total ](CEA-861-D spec)
                        //    1080P30:  H_total= 2200, V_total=1125 => 0x30d4= 0x18980465
                        //    1080P25:  H_total= 2640, V_total=1125 => 0x30d4= 0x1A500465
                        //    1080P24:  H_total= 2750, V_total=1125 => 0x30d4= 0x1ABE0465
                        //(2)0x30A0, 0x30A4, 0x30A8 ]1080P active region 1920x 1080
                        //    0x30a0= 0x00970817,
                        //    0x30a4=   0x002a0461,
                        //    0x30a8=   0x002a0461,
                        //(3)0x3094 , bit 13,bit 14,bit 15  ]1
                default:
                        //VERIFY(0);
                        VDO_LOG(VDO_LOG_LVL_ERR, "VERIFY Failed: %s, %d!\r\n", __FILE__, __LINE__); 
                        break;
                }

                switch(ucFmt)
                {
                case RES_1080I50HZ: // PMX mode is 1080P 50. P2I output 1080I
                case RES_1080I60HZ: // PMX mode is 1080P 60. P2I output 1080I
                case RES_1080P50HZ:
                case RES_1080P60HZ:
                case RES_720P60HZ:
                case RES_720P50HZ:	
                case RES_600P_800:
                case RES_600P_1024:
                	//For Front FMT, must be 1 when hd mode.
                        pDcmPmxSwReg->rField.fgNOT_PST_D2 = 1;
                        break;
                default:
                        pDcmPmxSwReg->rField.fgNOT_PST_D2 = 0;
                        break;
                }		

                if(fgIsHDRes(ucFmt) && (!fgIsFullHDRes(ucFmt)))
                {
                        pDcmPmxSwReg->rField.fgHD_C_FIR_EN = 1;
                }
                else
                {
                        pDcmPmxSwReg->rField.fgHD_C_FIR_EN = 0;
                }

                // for pana  pq request
                // repeat mode is better for s&m tt14(setup&evlauration->chroma multiburst),2013-06-18,yongqiang.niu,mtk40418

                pDcmPmxSwReg->rField.fgAverage422to444 = 0;

                #if 0 //xzr
                pDcmPmxHwReg->au4Reg[(0xd0/4)] = pDcmPmxSwReg->au4Reg[(0xd0/4)];
                pDcmPmxHwReg->au4Reg[(0xd4/4)] = pDcmPmxSwReg->au4Reg[(0xd4/4)];
                pDcmPmxHwReg->au4Reg[(0xc4/4)] = pDcmPmxSwReg->au4Reg[(0xc4/4)];
                pDcmPmxHwReg->au4Reg[(0xe4/4)] = pDcmPmxSwReg->au4Reg[(0xe4/4)];
                pDcmPmxHwReg->au4Reg[(0xf8/4)] = pDcmPmxSwReg->au4Reg[(0xf8/4)];
                #endif

                _rPmxDispMainRegMode[(0xd0 / 4)] |= PMX_HAL_REG_MODE_WRITE;
                _rPmxDispMainRegMode[(0xd4 / 4)] |= PMX_HAL_REG_MODE_WRITE;
                _rPmxDispMainRegMode[(0xc4 / 4)] |= PMX_HAL_REG_MODE_WRITE;
                _rPmxDispMainRegMode[(0xe4 / 4)] |= PMX_HAL_REG_MODE_WRITE;
                _rPmxDispMainRegMode[(0xf8 / 4)] |= PMX_HAL_REG_MODE_WRITE;

                if(fgIsTrueInterlaceSupport(ucFmt))
                {
                        pDcmPmxSwReg->rField.fgPRGS = 0;
                        pDcmPmxSwReg->rField.u1VSYNWIDTH = 8;
                }
                else
                {
                        UINT8 u1VsyncWidth = 12;

                        pDcmPmxSwReg->rField.fgPRGS = 1;
                        pDcmPmxSwReg->rField.u1VSYNWIDTH = u1VsyncWidth;
                }
                pDcmPmxSwReg->rField.u1HSYNWIDTH = 32;
                #if 0 //xzr
                pDcmPmxHwReg->au4Reg[(0x94/4)] = pDcmPmxSwReg->au4Reg[(0x94/4)];
                #endif
                _rPmxDispMainRegMode[(0x94 / 4)] |= PMX_HAL_REG_MODE_WRITE;

                pDcmPmxSwReg->rField.fgPFOFF = 1;
                pDcmPmxSwReg->rField.fgFMTM = 0;

                #if 0 //xzr
                pDcmPmxHwReg->au4Reg[(0xac/4)] = pDcmPmxSwReg->au4Reg[(0xac/4)];
                #endif
                _rPmxDispMainRegMode[(0xac / 4)] |= PMX_HAL_REG_MODE_WRITE;

        }else{  //vdo_id 1
                //fixme: should be new sd and scaler down 
                //acturally,if no scaler down, not use 144mode.
		if(EnPmxRearNewSD144Mode)
                        pDcmPmxSwReg->rField.fgDownScaleMode =1;
		
                pDcmPmxSwReg->rField.u2C4Default = 0x10EB;

                switch(ucFmt)
                {
		case RES_480P_800:
                        pDcmPmxSwReg->rField.fgHD_ON = 0;
                        pDcmPmxSwReg->rField.fgHD_TP = 0;
                        pDcmPmxSwReg->rField.fgADJ_T = 1;
                        pDcmPmxSwReg->rField.u2H_TOTAL = 0x3e8;
                        //pDcmPmxSwReg->rField.u2H_TOTAL_MIX = 0x3b9;

                        pDcmPmxSwReg->rField.u2V_TOTAL = 0x20d;
                        //pDcmPmxSwReg->rField.u2V_TOTAL_MIX=0x20d;


                        pDcmPmxSwReg->rField.fgNOT_PST_D2 = 0;
                        pDcmPmxSwReg->rField.fgEN_FIRST_PXL_LEAD = 0;
                        pDcmPmxSwReg->rField.u1FIRST_PXL_LEAD = 0;
                        pDcmPmxSwReg->rField.fgFACT_PREC = 1;
                        pDcmPmxSwReg->rField.fgAverage422to444 = 1;

                        pDcmPmxSwReg->rField.fgNEW_SD_MODE = 1; //new_sd_144m
                        pDcmPmxSwReg->rField.fgDSDCLK_EANBLE = 1; //new_sd_144m

                        pDcmPmxSwReg->rField.horizontal_use_3fs = 1;
                        pDcmPmxSwReg->rField.new_hd= 0;	
                        pDcmPmxSwReg->rField.fgNOT_PST_D2 = 0;			
                        break;
		
                case RES_480I:
                case RES_480P:
			if(EnPmxRearNewSD144Mode){
				pDcmPmxSwReg->rField.horizontal_use_3fs = 1;
				pDcmPmxSwReg->rField.fgNEW_SD_MODE = 1; //new_sd_144m
				pDcmPmxSwReg->rField.fgDSDCLK_EANBLE = 1; //new_sd_144m
			}
                        pDcmPmxSwReg->rField.fgHD_ON = 0;
                        pDcmPmxSwReg->rField.fgHD_TP = 0;
                        pDcmPmxSwReg->rField.fgADJ_T = 0;
                        pDcmPmxSwReg->rField.fgADJ_T_MIX = 0;
                        pDcmPmxSwReg->rField.fgNOT_PST_D2 = 0;
                        pDcmPmxSwReg->rField.fgEN_FIRST_PXL_LEAD = 0;
                        pDcmPmxSwReg->rField.u1FIRST_PXL_LEAD = 0;

                        pDcmPmxSwReg->rField.fgFACT_PREC = 1;
                        pDcmPmxSwReg->rField.fgAverage422to444 = 1;
                        pDcmPmxSwReg->rField.fgAverage422to444 = 1;
                        break;
                        
                case RES_576I:
                case RES_576P:
                        pDcmPmxSwReg->rField.fgHD_ON = 0;
                        pDcmPmxSwReg->rField.fgHD_TP = 0;
                        pDcmPmxSwReg->rField.fgADJ_T = 0;
                        pDcmPmxSwReg->rField.fgADJ_T_MIX = 0;
                        pDcmPmxSwReg->rField.fgNOT_PST_D2 = 0;
                        pDcmPmxSwReg->rField.fgEN_FIRST_PXL_LEAD = 0;
                        pDcmPmxSwReg->rField.u1FIRST_PXL_LEAD = 0;

                        pDcmPmxSwReg->rField.fgFACT_PREC = 1;
                        pDcmPmxSwReg->rField.fgAverage422to444 = 1;
                        break;

                case RES_720P60HZ:
                case RES_720P50HZ:
                case RES_720P23HZ:
                case RES_720P24HZ:
                case RES_720P25HZ:
                case RES_720P30HZ:
                        pDcmPmxSwReg->rField.fgADJ_T = 0;
                        pDcmPmxSwReg->rField.fgADJ_T_MIX = 0;

                        if ((ucFmt == RES_720P23HZ) || (ucFmt == RES_720P24HZ)||(ucFmt == RES_3D_720P23HZ_TB) || (ucFmt == RES_3D_720P23HZ_SBS_HALF)||
                                (ucFmt == RES_3D_720P24HZ_TB) || (ucFmt == RES_3D_720P24HZ_SBS_HALF) || (ucFmt == RES_720P30HZ) ||
                                (ucFmt == RES_3D_720P30HZ_TB) || (ucFmt == RES_3D_720P30HZ_SBS_HALF))
                        {
                                pDcmPmxSwReg->rField.u2H_TOTAL = 3300;
                                pDcmPmxSwReg->rField.u2H_TOTAL_MIX = 3300;

                                pDcmPmxSwReg->rField.u2V_TOTAL = 750;
                                pDcmPmxSwReg->rField.u2V_TOTAL_MIX=750;
                                pDcmPmxSwReg->rField.fgADJ_T = 1;
                                pDcmPmxSwReg->rField.fgADJ_T_MIX = 1;
                        }
                        else if ((ucFmt == RES_720P25HZ) || (ucFmt == RES_3D_720P25HZ_TB)||(ucFmt == RES_3D_720P25HZ_SBS_HALF))
                        {
                                pDcmPmxSwReg->rField.u2H_TOTAL = 3960;
                                pDcmPmxSwReg->rField.u2H_TOTAL_MIX = 3960;

                                pDcmPmxSwReg->rField.u2V_TOTAL = 750;
                                pDcmPmxSwReg->rField.u2V_TOTAL_MIX=750;
                                pDcmPmxSwReg->rField.fgADJ_T = 1;
                                pDcmPmxSwReg->rField.fgADJ_T_MIX = 1;
                        }

                        pDcmPmxSwReg->rField.fgHD_ON = 1;
                        pDcmPmxSwReg->rField.fgHD_TP = 1;
                        //pDcmPmxSwReg->rField.fgNOT_PST_D2 = 0; //for 3365
                        pDcmPmxSwReg->rField.fgEN_FIRST_PXL_LEAD = 0;
                        pDcmPmxSwReg->rField.u1FIRST_PXL_LEAD = 0;
                        pDcmPmxSwReg->rField.fgFACT_PREC = 1;
                	/*new add for 3365*/
                	if(ucFmt == RES_720P60HZ){
                		pDcmPmxSwReg->rField.u2V_TOTAL = 0x2ee;
                		pDcmPmxSwReg->rField.u2H_TOTAL = 0x672;				

                	}else if(ucFmt == RES_720P50HZ){
                		pDcmPmxSwReg->rField.u2V_TOTAL = 0x2ee;
                		pDcmPmxSwReg->rField.u2H_TOTAL = 0x7bc;				
                	}
                	pDcmPmxSwReg->rField.horizontal_use_2fs = 0;
                	pDcmPmxSwReg->rField.new_hd= 0;	
                	pDcmPmxSwReg->rField.fgNOT_PST_D2 = 1; 
                        	
                        pDcmPmxSwReg->rField.fgAverage422to444 = 1;
                        break;

		case RES_600P_800:
			//pDcmPmxSwReg->rField.fgADJ_T = 0;
			pDcmPmxSwReg->rField.fgADJ_T_MIX = 0;
				
			pDcmPmxSwReg->rField.fgHD_ON = 1;
			pDcmPmxSwReg->rField.fgHD_TP = 0;
			//pDcmPmxSwReg->rField.fgNOT_PST_D2 = 0; //for 3365
			pDcmPmxSwReg->rField.fgEN_FIRST_PXL_LEAD = 0;
			pDcmPmxSwReg->rField.u1FIRST_PXL_LEAD = 0;
			pDcmPmxSwReg->rField.fgFACT_PREC = 1;
			/*new add for 3365*/
			pDcmPmxSwReg->rField.fgADJ_T =1;
			
			pDcmPmxSwReg->rField.u2V_TOTAL = 0x294;
			pDcmPmxSwReg->rField.u2H_TOTAL = 0x3f2; 			

			pDcmPmxSwReg->rField.horizontal_use_4fs = 1;
			pDcmPmxSwReg->rField.new_hd= 1; 
			pDcmPmxSwReg->rField.fgNOT_PST_D2 = 1; 
			
			pDcmPmxSwReg->rField.fgAverage422to444 = 1;
			break;


                case RES_1080P60HZ:
                case RES_1080P50HZ:
                case RES_1080I60HZ:
                case RES_1080I50HZ:
                        pDcmPmxSwReg->rField.fgHD_ON = 1;
                        pDcmPmxSwReg->rField.fgHD_TP = 0;
                        pDcmPmxSwReg->rField.fgADJ_T = 0;
                        pDcmPmxSwReg->rField.fgADJ_T_MIX = 0;
                        //pDcmPmxSwReg->rField.fgNOT_PST_D2 = 0;
                        pDcmPmxSwReg->rField.fgEN_FIRST_PXL_LEAD = 0;
                        pDcmPmxSwReg->rField.u1FIRST_PXL_LEAD = 0;
                        pDcmPmxSwReg->rField.fgFACT_PREC = 1;
                        pDcmPmxSwReg->rField.fgAverage422to444 = 1;

			/*new add for 3365*/
			if(ucFmt == RES_1080P60HZ){
				pDcmPmxSwReg->rField.u2V_TOTAL = 1125;
				pDcmPmxSwReg->rField.u2H_TOTAL = 2200;				

			}else if(ucFmt == RES_1080P50HZ){
				pDcmPmxSwReg->rField.u2V_TOTAL = 1125;
				pDcmPmxSwReg->rField.u2H_TOTAL = 2640;				
			}
			pDcmPmxSwReg->rField.horizontal_use_fs = 0;
			pDcmPmxSwReg->rField.new_hd= 0;	
			pDcmPmxSwReg->rField.fgNOT_PST_D2 = 1;

                        break;

                case RES_1080P24HZ:
                case RES_1080P23_976HZ:
                        pDcmPmxSwReg->rField.fgHD_ON = 1;
                        pDcmPmxSwReg->rField.fgHD_TP = 1;
                        pDcmPmxSwReg->rField.u2V_TOTAL = 0x465;
                        pDcmPmxSwReg->rField.u2H_TOTAL = 0xABE;
                        pDcmPmxSwReg->rField.fgADJ_T = 1;
                        pDcmPmxSwReg->rField.u2V_TOTAL_MIX = 0x465;
                        pDcmPmxSwReg->rField.u2H_TOTAL_MIX = 0xABE;
                        pDcmPmxSwReg->rField.fgADJ_T_MIX = 1;
                        pDcmPmxSwReg->rField.fgNOT_PST_D2 = 0;
                        pDcmPmxSwReg->rField.fgEN_FIRST_PXL_LEAD = 0;
                        pDcmPmxSwReg->rField.u1FIRST_PXL_LEAD = 0;
                        pDcmPmxSwReg->rField.fgFACT_PREC = 1;
                        pDcmPmxSwReg->rField.fgAverage422to444 = 1;
                        break;

                case RES_1080P25HZ:
                        pDcmPmxSwReg->rField.fgHD_ON = 1;
                        pDcmPmxSwReg->rField.fgHD_TP = 1;
                        pDcmPmxSwReg->rField.u2V_TOTAL = 0x465;
                        pDcmPmxSwReg->rField.u2H_TOTAL = 0xA50;
                        pDcmPmxSwReg->rField.fgADJ_T = 1;
                        pDcmPmxSwReg->rField.u2V_TOTAL_MIX = 0x465;
                        pDcmPmxSwReg->rField.u2H_TOTAL_MIX = 0xA50;
                        pDcmPmxSwReg->rField.fgADJ_T_MIX = 1;
                        pDcmPmxSwReg->rField.fgNOT_PST_D2 = 0;
                        pDcmPmxSwReg->rField.fgEN_FIRST_PXL_LEAD = 0;
                        pDcmPmxSwReg->rField.u1FIRST_PXL_LEAD = 0;
                        pDcmPmxSwReg->rField.fgFACT_PREC = 1;
                        pDcmPmxSwReg->rField.fgAverage422to444 = 1;
                        break;

                case RES_1080P30HZ:
                case RES_1080P29_97HZ:
                        pDcmPmxSwReg->rField.fgHD_ON = 1;
                        pDcmPmxSwReg->rField.fgHD_TP = 1;
                        pDcmPmxSwReg->rField.u2V_TOTAL = 0x465;
                        pDcmPmxSwReg->rField.u2H_TOTAL = 0x898;
                        pDcmPmxSwReg->rField.fgADJ_T = 1;
                        pDcmPmxSwReg->rField.u2V_TOTAL_MIX = 0x465;
                        pDcmPmxSwReg->rField.u2H_TOTAL_MIX = 0x898;
                        pDcmPmxSwReg->rField.fgADJ_T_MIX = 1;
                        pDcmPmxSwReg->rField.fgNOT_PST_D2 = 0;
                        pDcmPmxSwReg->rField.fgEN_FIRST_PXL_LEAD = 0;
                        pDcmPmxSwReg->rField.u1FIRST_PXL_LEAD = 0;
                        pDcmPmxSwReg->rField.fgFACT_PREC = 1;
                        pDcmPmxSwReg->rField.fgAverage422to444 = 1;
                        break;

                //ToDo: add settings for 1080P25HZ, 1080P30HZ, 1080P24HZ
                //(1)0x30d4 bit28 ] 1, BNH_Total, V_total ](CEA-861-D spec)
                //    1080P30:  H_total= 2200, V_total=1125 => 0x30d4= 0x18980465
                //    1080P25:  H_total= 2640, V_total=1125 => 0x30d4= 0x1A500465
                //    1080P24:  H_total= 2750, V_total=1125 => 0x30d4= 0x1ABE0465
                //(2)0x30A0, 0x30A4, 0x30A8 ]1080P active region 1920x 1080
                //    0x30a0= 0x00970817,
                //    0x30a4=   0x002a0461,
                //    0x30a8=   0x002a0461,
                //(3)0x3094 , bit 13,bit 14,bit 15  ]1

                default:
                        //VERIFY(0);            
                        VDO_LOG(VDO_LOG_LVL_ERR, "VERIFY Failed: %s, %d!\r\n", __FILE__, __LINE__); 

                        break;
                }

                switch(ucFmt)
                {
                case RES_1080P50HZ:
                case RES_1080P60HZ:
                        //For Front FMT, must be 1 when hd mode.
                        pDcmPmxSwReg->rField.fgNOT_PST_D2 = 1;
                        break;
                default:
                        pDcmPmxSwReg->rField.fgNOT_PST_D2 = 0;
                        break;
                }		

	        if((ucPmxId==1)&&((ucFmt ==RES_720P60HZ) ||(ucFmt ==RES_720P50HZ))) {
                        pDcmPmxSwReg->rField.fgNOT_PST_D2 = 0;
	        }

	        if(fgIsHDRes(ucFmt) && (!fgIsFullHDRes(ucFmt)))
	        {
	                pDcmPmxSwReg->rField.fgHD_C_FIR_EN = 1;
	        }
	        else
	        {
	                pDcmPmxSwReg->rField.fgHD_C_FIR_EN = 0;
	        }

                // for pana  pq request
                // repeat mode is better for s&m tt14(setup&evlauration->chroma multiburst),2013-06-18,yongqiang.niu,mtk40418
                pDcmPmxSwReg->rField.fgAverage422to444 = 0;

                #if 0 //xzr
                pDcmPmxHwReg->au4Reg[(0xd0/4)] = pDcmPmxSwReg->au4Reg[(0xd0/4)];
                pDcmPmxHwReg->au4Reg[(0xd4/4)] = pDcmPmxSwReg->au4Reg[(0xd4/4)];
                pDcmPmxHwReg->au4Reg[(0xc4/4)] = pDcmPmxSwReg->au4Reg[(0xc4/4)];
                pDcmPmxHwReg->au4Reg[(0xe4/4)] = pDcmPmxSwReg->au4Reg[(0xe4/4)];
                pDcmPmxHwReg->au4Reg[(0xf8/4)] = pDcmPmxSwReg->au4Reg[(0xf8/4)];
                #endif
                _rPmxDispAuxRegMode[(0xd0 / 4)] |= PMX_HAL_REG_MODE_WRITE;
                _rPmxDispAuxRegMode[(0xd4 / 4)] |= PMX_HAL_REG_MODE_WRITE;
                _rPmxDispAuxRegMode[(0xc4 / 4)] |= PMX_HAL_REG_MODE_WRITE;
                _rPmxDispAuxRegMode[(0xe4 / 4)] |= PMX_HAL_REG_MODE_WRITE;
                _rPmxDispAuxRegMode[(0xf8 / 4)] |= PMX_HAL_REG_MODE_WRITE;

                if(fgIsTrueInterlaceSupport(ucFmt))
                {
                        pDcmPmxSwReg->rField.fgPRGS = 0;
                        pDcmPmxSwReg->rField.u1VSYNWIDTH = 8;
                }
                else
                {
                        UINT8 u1VsyncWidth = 12;

                        pDcmPmxSwReg->rField.fgPRGS = 1;
                        pDcmPmxSwReg->rField.u1VSYNWIDTH = u1VsyncWidth;
                }
                pDcmPmxSwReg->rField.u1HSYNWIDTH = 32;
                #if 0 //xzr
                pDcmPmxHwReg->au4Reg[(0x94/4)] = pDcmPmxSwReg->au4Reg[(0x94/4)];
                #endif
                _rPmxDispAuxRegMode[(0x94 / 4)] |= PMX_HAL_REG_MODE_WRITE;


                pDcmPmxSwReg->rField.fgPFOFF = 1;
                pDcmPmxSwReg->rField.fgFMTM = 0;

                #if 0 //xzr
                pDcmPmxHwReg->au4Reg[(0xac/4)] = pDcmPmxSwReg->au4Reg[(0xac/4)];
                #endif
                _rPmxDispAuxRegMode[(0xac / 4)] |= PMX_HAL_REG_MODE_WRITE;
	
        } //end if vdo 1
	
        VDO_LOG(VDO_LOG_LVL_DBG, " %s,no fast logo,HalResetInVSync(4) .\n",__FUNCTION__);

        //vPmxHalRstInVSync(ucPmxId);

    //  _PmxMainReset();
}

void PMX_HalSetTvType(UCHAR ucPmxId, UCHAR ucTvType)
{
    UINT32 u4Tvmode;

    if (ucPmxId == PMX_1)
    {
        u4Tvmode = 0;
        if(ucTvType == PMX_TV_TYPE_NTSC)
        {
            u4Tvmode = 0;
        }
        else if(ucTvType == PMX_TV_TYPE_PAL_M)
        {
            u4Tvmode = 1;
        }
        else if(ucTvType == PMX_TV_TYPE_PAL_N)
        {
            u4Tvmode = 2;
        }
        else if(ucTvType == PMX_TV_TYPE_PAL)
        {
            u4Tvmode = 3;
        }

        {
            char *szTvType[] =
            {
                "PMX_TV_TYPE_NTSC",
                "PMX_TV_TYPE_PAL_M",
                "PMX_TV_TYPE_PAL_N"
                "PMX_TV_TYPE_PAL",
            };

            if (u4Tvmode <= PMX_TV_TYPE_PAL)
            {
                UTIL_Printf("[PMX] set %s\n", &szTvType[u4Tvmode]);
            }
        }

        pDcmPmxDispMainSwReg->rField.u1TVMODE = u4Tvmode;
        //pDcmPmxDispAuxSwReg->rField.u1TVMODE = u4Tvmode;
        //pDcmPmxVdoutMainSwReg->rField.u1TVMODE = u4Tvmode;

        _prPmxDispMainHwReg->au4Reg[(0x94/4)] = pDcmPmxDispMainSwReg->au4Reg[(0x94/4)];
        //_prPmxDispAuxHwReg->au4Reg[(0x94/4)] = pDcmPmxDispAuxSwReg->au4Reg[(0x94/4)];
        //_prPmxVdoutMainHwReg->au4Reg[(0x94/4)] = pDcmPmxVdoutMainSwReg->au4Reg[(0x94/4)];
    }else if(ucPmxId == PMX_2){
			u4Tvmode = 0;
			if(ucTvType == PMX_TV_TYPE_NTSC)
			{
				u4Tvmode = 0;
			}
			else if(ucTvType == PMX_TV_TYPE_PAL_M)
			{
				u4Tvmode = 1;
			}
			else if(ucTvType == PMX_TV_TYPE_PAL_N)
			{
				u4Tvmode = 2;
			}
			else if(ucTvType == PMX_TV_TYPE_PAL)
			{
				u4Tvmode = 3;
			}
	
			{
				char *szTvType[] =
				{
					"PMX_TV_TYPE_NTSC",
						"PMX_TV_TYPE_PAL_M",
						"PMX_TV_TYPE_PAL_N"
						"PMX_TV_TYPE_PAL",
				};
	
				if (u4Tvmode <= PMX_TV_TYPE_PAL)
				{
					UTIL_Printf("[PMX] set %s \n", &szTvType[u4Tvmode]);
				}
			}
	
			//pDcmPmxDispMainSwReg->rField.u1TVMODE = u4Tvmode;
			pDcmPmxDispAuxSwReg->rField.u1TVMODE = u4Tvmode;
			//pDcmPmxVdoutMainSwReg->rField.u1TVMODE = u4Tvmode;
			WriteREGMsk(0x43124,u4Tvmode<<30,0x3<<30);  //scaler tv type.
			//_prPmxDispMainHwReg->au4Reg[(0x94/4)] = pDcmPmxDispMainSwReg->au4Reg[(0x94/4)];
			_prPmxDispAuxHwReg->au4Reg[(0x94/4)] = pDcmPmxDispAuxSwReg->au4Reg[(0x94/4)];
			//_prPmxVdoutMainHwReg->au4Reg[(0x94/4)] = pDcmPmxVdoutMainSwReg->au4Reg[(0x94/4)];
		}
}

void vPmxHalSetMode(__u8 ucPmxId, __u8 ucFmt)
{
}

void vPmxHalSetTvType(__u8 ucPmxId, __u8 ucTvType)
{
}


void vPmxHalSetAlpha(__u8 ucPmxId, __u32 ucInAlpha, __u32 ucOutAlpha, bool fgEnable)
{
}

void setPmxSwToHw(void)
{
#if 0 //old
	int u4RegIdx = 0;
	for (; u4RegIdx < PMX_HAL_MIX_REG_NUM; u4RegIdx++) {
		if (_rPmxMixRegMode[u4RegIdx] & PMX_HAL_REG_MODE_WRITE)
			_prPmxHalMixHwReg->au4Reg[u4RegIdx] = _rPmxHalMixSwReg.au4Reg[u4RegIdx];
			_rPmxMixRegMode[u4RegIdx] &= ~PMX_HAL_REG_MODE_WRITE;
	}
#endif
        __u32 u4RegIdx;
	__u32 tmp = 0;

	/* update plane mixer vdout register at vsync*/
	for (u4RegIdx = 0; u4RegIdx < PMX_HAL_MIX_REG_NUM; u4RegIdx++) {
		if (_rPmxMixRegMode[u4RegIdx] & PMX_HAL_REG_MODE_WRITE) {
			if ((u4RegIdx == 0) && (_rPmxMixRegMode[u4RegIdx] >> 2)) {
				/* Don't enable video layer and primary surface if delay write set*/
				/*tmp = (1 << PRIMARY_SURF_PLANE) | 0x3;*/
				tmp = 0x3;
				_prPmxHalMixHwReg->au4Reg[u4RegIdx] = _rPmxHalMixSwReg.au4Reg[u4RegIdx] & ~tmp;
				_rPmxMixRegMode[u4RegIdx] &= ~PMX_HAL_REG_MODE_WRITE;
			} else {
				_prPmxHalMixHwReg->au4Reg[u4RegIdx] = _rPmxHalMixSwReg.au4Reg[u4RegIdx];
				_rPmxMixRegMode[u4RegIdx] &= ~PMX_HAL_REG_MODE_WRITE;
			}
		}

		if (_rPmxMixRegMode[u4RegIdx] & PMX_HAL_REG_MODE_READ) {
			_rPmxHalMixSwReg.au4Reg[u4RegIdx] = _prPmxHalMixHwReg->au4Reg[u4RegIdx];
			_rPmxMixRegMode[u4RegIdx] &= ~PMX_HAL_REG_MODE_READ;
		}

#if !(PMX_HAL_REG_MODE_DELAY_WRITE == PMX_HAL_REG_MODE_WRITE)

		if (_rPmxMixRegMode[u4RegIdx] >= (PMX_HAL_REG_MODE_WRITE << 1)) {
			_rPmxMixRegMode[u4RegIdx] = _rPmxMixRegMode[u4RegIdx] >> 1;
			FB_PRINT(FB_LOG_LVL_INFO, "PMX", "MIX delay write mode %d [0x%x] 0x%x -> 0x%x\r\n"
				, (int)_rPmxMixRegMode[u4RegIdx], (unsigned int)(u4RegIdx << 2)
				, (unsigned int)_rPmxHalMixSwReg.au4Reg[u4RegIdx]
				, (unsigned int)_prPmxHalMixHwReg->au4Reg[u4RegIdx]);
		}

#endif
        }
}

void vPmxMixPlane(unsigned char ucPmxId, unsigned int u4Plane)
{
	if (ucPmxId == PMX_1)
	{
		switch (u4Plane)
		{
			case PMX_HW_VIDEO_MIX:
                                if (!_rPmxHalMixSwReg.rField.fgVIDEO_MIX_EN) {
        				_rPmxHalMixSwReg.rField.fgVIDEO_MIX_EN = 1;
        				_rPmxMixRegMode[(0 / 4)] |= PMX_HAL_REG_MODE_DELAY_WRITE;
			        }
				_rPmxHalMixSwReg.rField.fgVIDEO_MIX_EN = 1;
			break;
			case PMX_HW_OSD1_MIX:
				_rPmxHalMixSwReg.rField.fgOSD1_MIX_EN = 1;
                                _rPmxMixRegMode[(0 / 4)] |= PMX_HAL_REG_MODE_WRITE;
			break;
			case PMX_HW_OSD2_MIX:
				_rPmxHalMixSwReg.rField.fgOSD2_MIX_EN = 1;
                                _rPmxMixRegMode[(0 / 4)] |= PMX_HAL_REG_MODE_WRITE;
			break;
			case PMX_HW_OSD3_MIX:
				_rPmxHalMixSwReg.rField.fgOSD3_MIX_EN = 1;
                                _rPmxMixRegMode[(0 / 4)] |= PMX_HAL_REG_MODE_WRITE;
			break;
			case PMX_HW_PLANE_6:
				//_rPmxHalMixSwReg.rField.fgOSD4_MIX_EN = 1;
			break;
			case PMX_HW_OSD4_MIX:
				_rPmxHalMixSwReg.rField.fgOSD4_MIX_EN = 1;
                                _rPmxMixRegMode[(0 / 4)] |= PMX_HAL_REG_MODE_WRITE;
			break;
				
			case PMX_HW_PLANE_8:
			default:
			break;
		}
	
		//_rPmxMixRegMode[(0 / 4)] |= PMX_HAL_REG_MODE_WRITE;			
#ifdef __ARM2__
	//vPmxHalMainIsr(45,NULL);
#endif
	}
}
EXPORT_SYMBOL(vPmxMixPlane);

void vPmxNotMixPlane(unsigned char ucPmxId, unsigned int u4Plane)
{
	if (ucPmxId == PMX_1)
	{
		switch (u4Plane)
		{
			case PMX_HW_VIDEO_MIX:
                                if (_rPmxHalMixSwReg.rField.fgVIDEO_MIX_EN) {
        				_rPmxHalMixSwReg.rField.fgVIDEO_MIX_EN = 0;
        				_rPmxMixRegMode[(0 / 4)] |= PMX_HAL_REG_MODE_WRITE;
			        }
			break;
			case PMX_HW_OSD1_MIX:
				_rPmxHalMixSwReg.rField.fgOSD1_MIX_EN = 0;
                                _rPmxMixRegMode[(0 / 4)] |= PMX_HAL_REG_MODE_WRITE;
			break;
			case PMX_HW_OSD2_MIX:
				_rPmxHalMixSwReg.rField.fgOSD2_MIX_EN = 0;
                                _rPmxMixRegMode[(0 / 4)] |= PMX_HAL_REG_MODE_WRITE;
			break;
			case PMX_HW_OSD3_MIX:
				_rPmxHalMixSwReg.rField.fgOSD3_MIX_EN = 0;
                                _rPmxMixRegMode[(0 / 4)] |= PMX_HAL_REG_MODE_WRITE;
			break;
			case PMX_HW_OSD4_MIX:
				_rPmxHalMixSwReg.rField.fgOSD4_MIX_EN = 0;
                                _rPmxMixRegMode[(0 / 4)] |= PMX_HAL_REG_MODE_WRITE;
			break;
			case PMX_HW_PLANE_6:
				//_rPmxHalMixSwReg.rField.fgOSD5_MIX_EN = 0;
			break;
			
			case PMX_HW_PLANE_8:
			default:
			break;
		}

		//_rPmxMixRegMode[(0 / 4)] |= PMX_HAL_REG_MODE_WRITE;
#ifdef __ARM2__
	//vPmxHalMainIsr(45,NULL);
#endif
	}
}
EXPORT_SYMBOL(vPmxNotMixPlane);

void vPmxHalMixPlane(__u8 ucPmxId, __u32 u4Plane)
{
	
}
EXPORT_SYMBOL(vPmxHalMixPlane);

void vPmxHalNotMixPlane(__u8 ucPmxId, __u32 u4Plane)
{
	
}
EXPORT_SYMBOL(vPmxHalNotMixPlane);


bool fgPmxHalMixPlane(__u8 ucPmxId, __u32 u4Plane)
{
        bool fgMixPlane = FALSE;

	if (ucPmxId == PMX_1) {
		switch (u4Plane) {
		case PMX_HW_PLANE_1:
			fgMixPlane = (_prPmxHalMixHwReg->rField.fgVIDEO_MIX_EN == 1) ? TRUE : FALSE;
			break;

		case PMX_HW_PLANE_3:
			fgMixPlane = (_prPmxHalMixHwReg->rField.fgOSD1_MIX_EN == 1) ? TRUE : FALSE;
			break;

		case PMX_HW_PLANE_4:
			fgMixPlane = (_prPmxHalMixHwReg->rField.fgOSD2_MIX_EN == 1) ? TRUE : FALSE;
			break;

		case PMX_HW_PLANE_5:
			fgMixPlane = (_prPmxHalMixHwReg->rField.fgOSD3_MIX_EN == 1) ? TRUE : FALSE;
			break;

		case PMX_HW_PLANE_6:
			fgMixPlane = (_prPmxHalMixHwReg->rField.fgOSD4_MIX_EN == 1) ? TRUE : FALSE;
			break;

		default:
			break;
		}
	}

	return fgMixPlane;
}

void vPmxHalSetFullRange(__u8 ucPmxId, bool fgEnable, bool fg2352255)
{
        if (ucPmxId == PMX_1)
        {
                pDcmPmxDispMainSwReg->rField.fg235_TO_255_EN = fgEnable; //11:235->255 01:255->235 00:default
                pDcmPmxDispMainSwReg->rField.fgDATA_235_255 = fg2352255;
                //REG_SET(*pPmxDispMainRegMode, REG_MASK (0xf8 / 4));
                _rPmxDispMainRegMode[(0xf8 / 4)] |= PMX_HAL_REG_MODE_WRITE;
        }
        else
        {
                pDcmPmxDispAuxSwReg->rField.fg235_TO_255_EN = fgEnable;
                pDcmPmxDispAuxSwReg->rField.fgDATA_235_255 = fg2352255;
                //REG_SET(*pPmxDispAuxRegMode, REG_MASK (0xf8 / 4));
                _rPmxDispAuxRegMode[(0xf8 / 4)] |= PMX_HAL_REG_MODE_WRITE;
        }
}

void vPmxHalSet709To601(__u8 ucPmxId, bool fgEnable, bool fg7092601)
{

}

void vPmxHalSetFmtPhase(UCHAR ucPmxId, PMX_PHASE_TYPE_E eYPhase, PMX_PHASE_TYPE_E eCPhase)
{
    if (ucPmxId == PMX_1)
    {
        if ((PMX_PAHSE_TYPE_32 == eYPhase) && (PMX_PAHSE_TYPE_32 == eCPhase))
        {
            pDcmPmxDispMainSwReg->rField.fgUsePhase32 = 1;
        }
        else
        {
            if (PMX_PAHSE_TYPE_16 == eYPhase)
            {
                pDcmPmxDispMainSwReg->rField.fgUsePhase32 = 0;
                pDcmPmxDispMainSwReg->rField.fgPHASE_16 = 1;
            }
            else if (PMX_PAHSE_TYPE_8 == eYPhase) // 8 phase
            {
                pDcmPmxDispMainSwReg->rField.fgUsePhase32 = 0;
                pDcmPmxDispMainSwReg->rField.fgPHASE_16 = 0;
            }

            if (PMX_PAHSE_TYPE_16 == eCPhase)
            {
                pDcmPmxDispMainSwReg->rField.fgUsePhase32 = 0;
                pDcmPmxDispMainSwReg->rField.fgC_16_PHAEE = 1;
            }
            else if (PMX_PAHSE_TYPE_8 == eCPhase)// 8 phase
            {
                pDcmPmxDispMainSwReg->rField.fgUsePhase32 = 0;
                pDcmPmxDispMainSwReg->rField.fgC_16_PHAEE = 0;
            }
        }
        REG_SET(*pPmxDispMainRegMode, REG_MASK (0xe4 / 4));
        REG_SET(*pPmxDispMainRegMode, REG_MASK (0xb0 / 4));
        REG_SET(*pPmxDispMainRegMode, REG_MASK (0x48 / 4));
    }
    else
    {
        if ((PMX_PAHSE_TYPE_32 == eYPhase) && (PMX_PAHSE_TYPE_32 == eCPhase))
        {
            pDcmPmxDispAuxSwReg->rField.fgUsePhase32 = 1;
        }
        else
        {
            if (PMX_PAHSE_TYPE_16 == eYPhase)
            {
                pDcmPmxDispAuxSwReg->rField.fgUsePhase32 = 0;
                pDcmPmxDispAuxSwReg->rField.fgPHASE_16 = 1;
            }
            else // 8 phase
            {
                pDcmPmxDispAuxSwReg->rField.fgUsePhase32 = 0;
                pDcmPmxDispAuxSwReg->rField.fgPHASE_16 = 0;
            }

            if (PMX_PAHSE_TYPE_16 == eCPhase)
            {
                pDcmPmxDispAuxSwReg->rField.fgUsePhase32 = 0;
                pDcmPmxDispAuxSwReg->rField.fgC_16_PHAEE = 1;
            }
            else // 8 phase
            {
                pDcmPmxDispAuxSwReg->rField.fgUsePhase32 = 0;
                pDcmPmxDispAuxSwReg->rField.fgC_16_PHAEE = 0;
            }
        }
        REG_SET(*pPmxDispAuxRegMode, REG_MASK (0xe4 / 4));
        REG_SET(*pPmxDispAuxRegMode, REG_MASK (0xb0 / 4));
        REG_SET(*pPmxDispAuxRegMode, REG_MASK (0x48 / 4));
    }
}

void vPmxHalSetSDMode(UCHAR ucPmxId, PMX_SD_MODE_TYPE_E eSDModeType)
{

	HAL_PMX_DISP_MAIN_UNION_T *pDcmPmxDispSwReg_temp;
	UINT64 *pPmxDispRegMode_temp;
	
    if (ucPmxId == PMX_1){
		pDcmPmxDispSwReg_temp = pDcmPmxDispMainSwReg;
		pPmxDispRegMode_temp = pPmxDispMainRegMode;

	}else{
		pDcmPmxDispSwReg_temp = pDcmPmxDispAuxSwReg;
		pPmxDispRegMode_temp = pPmxDispAuxRegMode;
	}


    if (PMX_SD_MODE_TYPE_NEW_16 == eSDModeType)
    {
        pDcmPmxDispSwReg_temp->rField.fgDIRECT = 1;
        pDcmPmxDispSwReg_temp->rField.fgNEW_SD_MODE = 1;
        pDcmPmxDispSwReg_temp->rField.fgHD_ON = 0;
    }
    else if (PMX_SD_MODE_TYPE_NEW_8 == eSDModeType)
    {
        pDcmPmxDispSwReg_temp->rField.fgDIRECT = 0;
        pDcmPmxDispSwReg_temp->rField.fgNEW_SD_MODE = 1;
        pDcmPmxDispSwReg_temp->rField.fgHD_ON = 0;
    }
    else if (PMX_SD_MODE_TYPE_OLD_16 == eSDModeType)
    {
        pDcmPmxDispSwReg_temp->rField.fgDIRECT = 1;
        pDcmPmxDispSwReg_temp->rField.fgNEW_SD_MODE = 0;
        pDcmPmxDispSwReg_temp->rField.fgHD_ON = 0;
    }
    else
    {
        pDcmPmxDispSwReg_temp->rField.fgDIRECT = 0;
        pDcmPmxDispSwReg_temp->rField.fgNEW_SD_MODE = 0;
        pDcmPmxDispSwReg_temp->rField.fgHD_ON = 0;
    }

    REG_SET(*pPmxDispRegMode_temp, REG_MASK (0xe4 / 4));
    REG_SET(*pPmxDispRegMode_temp, REG_MASK (0x94 / 4));

}

extern volatile BOOL g_rearFieldSwith;
#ifndef __ARM2__
extern void vVdpIsr(__u32 u4VdpIdx);
extern void mt33xx_mask_ack_bim_irq(unsigned int virq);
#endif
/******************************************************************************
* Local Function
******************************************************************************/
irqreturn_t vPmxHalMainIsr(int u2Vector, void *dev_id)
{
        unsigned int u4RegIdx;
        //unsigned int u4Udf;
        UINT64 regMask;
        HAL_PMX_DISP_MAIN_UNION_T *pDcmPmxDispMainIsrSwReg;
        UINT64 *pDcmPmxDispMainIsrMode;
    
        HAL_PMX_DISP_MAIN_C_UNION_T *pDcmPmxDispMainCIsrSwReg;
        UINT64 *pDcmPmxDispMainCIsrMode;		

        vVdpIsr(0);

        if (PmxVerifyDoRestInVyncFront == TRUE) {
		_prPmxDispMainHwReg->rField.fgFTRST = 0;
		_prPmxDispMainHwReg->rField.fgFTRST = 1;
		_prPmxDispMainHwReg->rField.fgFTRST = 0;

		PmxVerifyDoRestInVyncFront = FALSE;
	}

        #if 0 //verify code
        if(TRUE == PmxVerifyDoRestInVyncFront){
		WriteREGMsk(0x420ac,(0x1<<10),0x1<<10); 
		WriteREGMsk(0xa46ac,(0x1<<10),0x1<<10); 
		
		WriteREG(0X4243C,0xFF);
		WriteREG(0X4243C,0x00);
		WriteREG(0X4243C,0xFF);
		WriteREG(0X4243C,0x00);
		VDO_LOG(VDO_LOG_LVL_IRQ, "rest vdo 0 in vysnc isr\n");
	}
        #endif

        pDcmPmxDispMainIsrSwReg = &_rPmxDispMainSwReg;
        pDcmPmxDispMainIsrMode = (UINT64 *)&_rPmxDispMainRegMode;
    
        // dispfmt CS
        pDcmPmxDispMainCIsrSwReg = &_rPmxDispMainCSwReg;
        pDcmPmxDispMainCIsrMode = (UINT64 *)&_rPmxDispMainCRegMode;

        /* update plane mixer display register at vsync*/
	for (u4RegIdx = 0; u4RegIdx < HAL_PMX_DISP_MAIN_REG_NUM; u4RegIdx++) {
		if (_rPmxDispMainRegMode[u4RegIdx] & PMX_HAL_REG_MODE_WRITE) {
			_prPmxDispMainHwReg->au4Reg[u4RegIdx] = _rPmxDispMainSwReg.au4Reg[u4RegIdx];
			_rPmxDispMainRegMode[u4RegIdx] &= ~PMX_HAL_REG_MODE_WRITE;
		}

		if (_rPmxDispMainRegMode[u4RegIdx] & PMX_HAL_REG_MODE_READ) {
			_rPmxDispMainSwReg.au4Reg[u4RegIdx] = _prPmxDispMainHwReg->au4Reg[u4RegIdx];
			_rPmxDispMainRegMode[u4RegIdx] &= ~PMX_HAL_REG_MODE_READ;
		}

#if !(PMX_HAL_REG_MODE_DELAY_WRITE == PMX_HAL_REG_MODE_WRITE)

		if (_rPmxDispMainRegMode[u4RegIdx] >= (PMX_HAL_REG_MODE_WRITE << 1)) {
			_rPmxDispMainRegMode[u4RegIdx] = _rPmxDispMainRegMode[u4RegIdx] >> 1;
			VDO_LOG(VDO_LOG_LVL_INFO, "FMTF delay write mode %d [0x%x] 0x%x -> 0x%x\r\n"
				, (int)_rPmxDispMainRegMode[u4RegIdx], (unsigned int)(u4RegIdx << 2)
				, (unsigned int)_rPmxDispMainSwReg.au4Reg[u4RegIdx]
				, (unsigned int)_prPmxDispMainHwReg->au4Reg[u4RegIdx]);
		}

#endif
	}
	setPmxSwToHw();
        #if 0
        for (u4RegIdx = 0, regMask = 1; u4RegIdx < HAL_PMX_DISP_MAIN_REG_NUM;
        u4RegIdx++, regMask <<= 1)
        {
            if (IS_REG_SET (*pDcmPmxDispMainIsrMode, regMask))
            {
                {
                    _prPmxDispMainHwReg->au4Reg[u4RegIdx] = pDcmPmxDispMainIsrSwReg->au4Reg[u4RegIdx];
                }
            }
        }
        *pDcmPmxDispMainIsrMode = 0;

        for (u4RegIdx = 0, regMask = 1; u4RegIdx < HAL_PMX_DISP_MAIN_C_REG_NUM;
        u4RegIdx++, regMask <<= 1)
        {
            if (IS_REG_SET (*pDcmPmxDispMainCIsrMode, regMask))
                _prPmxDispMainCHwReg->au4Reg[u4RegIdx] = pDcmPmxDispMainCIsrSwReg->au4Reg[u4RegIdx];
        }
    
        *pDcmPmxDispMainCIsrMode = 0;
        #endif
        
        // clear global isr
#ifndef __ARM2__
	mt33xx_mask_ack_bim_irq(u2Vector);
#else
	v_clear_bim_irq(u2Vector);
#endif

	return IRQ_HANDLED;
}

#ifndef __ARM2__
irqreturn_t vPmxHalAuxIsr(int u2Vector, void *dev_id)
{
        unsigned int u4RegIdx;
	UINT64 regMask;
	HAL_PMX_DISP_MAIN_UNION_T *pDcmPmxDispAuxIsrSwReg;
	UINT64 *pDcmPmxDispAuxIsrMode;
	HAL_PMX_DISP_MAIN_C_UNION_T *pDcmPmxDispAuxCIsrSwReg;
	UINT64 *pDcmPmxDispAuxCIsrMode;
        
	if(g_rearFieldSwith){
		/* switch field*/
		volatile unsigned int temp = (~(ReadREG(0x43430)>>5)) & 0x1;		
		WriteREGMsk(0x43430,1<<6,1<<6);
		WriteREGMsk(0x4341c,1<<13,1<<13);		
		WriteREGMsk(0x43430,temp<<5,1<<5);
					
	}

        vVdpIsr(1);
        
        if (PmxVerifyDoRestInVyncRear == TRUE) {
                        _prPmxDispAuxHwReg->rField.fgFTRST = 0;
                        _prPmxDispAuxHwReg->rField.fgFTRST = 1;
                        _prPmxDispAuxHwReg->rField.fgFTRST = 0;
        
                        PmxVerifyDoRestInVyncRear = FALSE;
        }

        #if 0 //verify code
	if(PmxVerifyDoRestInVyncRear==TRUE){
		
		WriteREGMsk(0x43430,0<<5,1<<5);
		WriteREGMsk(0x430ac,(0x1<<10),0x1<<10);	

		WriteREG(0x43120,0x403);
		WriteREG(0x43120,0x003);

		WriteREG(0X4343C,0xFF);
		WriteREG(0X4343C,0x00);
		
	}
        #endif
        
	// dispfmt_pip
	pDcmPmxDispAuxIsrSwReg = &_rPmxDispAuxSwReg;
	pDcmPmxDispAuxIsrMode = (UINT64 *)&_rPmxDispAuxRegMode;
	// dispfmt_pip CS
	pDcmPmxDispAuxCIsrSwReg = &_rPmxDispAuxCSwReg;
	pDcmPmxDispAuxCIsrMode = (UINT64 *)&_rPmxDispAuxCRegMode;

	for (u4RegIdx = 0, regMask = 1; u4RegIdx < HAL_PMX_DISP_AUX_REG_NUM;
	u4RegIdx++, regMask <<= 1)
	{
		if (IS_REG_SET (*pDcmPmxDispAuxIsrMode, regMask))
			_prPmxDispAuxHwReg->au4Reg[u4RegIdx] = pDcmPmxDispAuxIsrSwReg->au4Reg[u4RegIdx];
	}

	*pDcmPmxDispAuxIsrMode = 0;

	for (u4RegIdx = 0, regMask = 1; u4RegIdx < HAL_PMX_DISP_AUX_C_REG_NUM;
	u4RegIdx++, regMask <<= 1)
	{
		if (IS_REG_SET (*pDcmPmxDispAuxCIsrMode, regMask))
			_prPmxDispAuxCHwReg->au4Reg[u4RegIdx] = pDcmPmxDispAuxCIsrSwReg->au4Reg[u4RegIdx];
	}

	*pDcmPmxDispAuxCIsrMode = 0;


	// clear global isr
	mt33xx_mask_ack_bim_irq(u2Vector);

	return IRQ_HANDLED;
}
#endif
void vPmxHalIsrInit(void)
{
        //tmp hardcode
        fmtf_irq = 149;
        fmtr_irq = 151;

        //149 scl-f
        //151 scl-r
        //135 fmt-f
        //121 fmt-r
        //192 vdo front underrun
        //197 vdo rear underrun
        
        if (!_fgPmxMainIsrInited) {
#ifndef __ARM2__
		if (request_irq(fmtf_irq, vPmxHalMainIsr, 0, "FMTF_VSYNC", (void *)NULL) != OSR_OK) {
			VDO_LOG(VDO_LOG_LVL_ERR, "vPmxHalIsrInit fmtf irq reigster error\r\n");
			return;
		}
#endif
		/* init local variable*/
		_fgPmxMainIsrInited = TRUE;
	}

//now, fmtr dont init
#if 0
	if (!_fgPmxAuxIsrInited) {
#ifndef __ARM2__
		if (request_irq(fmtr_irq, vPmxHalAuxIsr, 0, "FMTR_VSYNC", (void *)NULL) != OSR_OK) {
			VDO_LOG(VDO_LOG_LVL_ERR, "vPmxHalIsrInit fmtr irq reigster error\r\n");
			return;
		}
		VDO_LOG(VDO_LOG_LVL_DBG, "PMX irq reigster %d %d\r\n", fmtf_irq, fmtr_irq);
#endif
		
		/* init local variable*/
		_fgPmxAuxIsrInited = TRUE;
	}
#endif        
}

void vPmxHalIsrStop(__u8 ucPmxId)
{
	if (_fgPmxMainIsrInited == TRUE) {
#ifndef __ARM2__
		free_irq(fmtf_irq, NULL);
#endif
		_fgPmxMainIsrInited = FALSE;
	}
}

void vPmxHalEnableFmt(__u8 ucPmxId)
{
	if (ucPmxId == PMX_1) {
		/*reset vdout fmt first*/
		_rPmxDispMainSwReg.rField.fgVDO_EN = 1;
		_rPmxDispMainSwReg.rField.fgFTRST = 1;
		/* FTRST is sw reset and write only register, so need read back after setting*/
		_rPmxDispMainRegMode[(0xac / 4)] |= PMX_HAL_REG_MODE_READ | PMX_HAL_REG_MODE_WRITE;
	} else {
		/*reset vdout fmt first*/
		_rPmxDispAuxSwReg.rField.fgVDO_EN = 1;
		_rPmxDispAuxSwReg.rField.fgFTRST = 1;
		/* FTRST is sw reset and write only register, so need read back after setting*/
		_rPmxDispAuxRegMode[(0xac / 4)] |= PMX_HAL_REG_MODE_READ | PMX_HAL_REG_MODE_WRITE;
	}
}
EXPORT_SYMBOL(vPmxHalEnableFmt);


void vPmxHalDisableFmt(__u8 ucPmxId)
{
        if (ucPmxId == PMX_1) {
		_prPmxDispMainHwReg->rField.fgVDO_EN = 0;
		_rPmxDispMainSwReg.rField.fgVDO_EN = _prPmxDispMainHwReg->rField.fgVDO_EN;
	} else {
		_prPmxDispAuxHwReg->rField.fgVDO_EN = 0;
		_rPmxDispAuxSwReg.rField.fgVDO_EN = _prPmxDispAuxHwReg->rField.fgVDO_EN;
	}
}
EXPORT_SYMBOL(vPmxHalDisableFmt);

bool vPmxHalGetFmtEn(__u8 ucPmxId)
{
	if (ucPmxId == PMX_1) {
		return _prPmxDispMainHwReg->rField.fgVDO_EN;
	} else {
		return _prPmxDispAuxHwReg->rField.fgVDO_EN;
	}
}

void vPmxHalSetPlaneOrder(__u8 ucPmxId, __u32 u4PlaneOrder)
{
	VDO_LOG(VDO_LOG_LVL_INFO, "vPmxHalSetPlaneOrder: id = %d, order = %x\r\n", ucPmxId, (unsigned int)u4PlaneOrder);
        //[xzr]todo    
}
EXPORT_SYMBOL(vPmxHalSetPlaneOrder);

__u32 vPmxHalGetPlaneOrder(__u8 ucPmxId)
{
	__u32 u4PlaneOrder = 0;

        //[xzr] todo    
	VDO_LOG(VDO_LOG_LVL_INFO, "vPmxHalGetPlaneOrder: id = %d, order = %x\r\n", ucPmxId, (unsigned int)u4PlaneOrder);

	return u4PlaneOrder;
}
EXPORT_SYMBOL(vPmxHalGetPlaneOrder);

void vPmxHalSetPlaneDstColorKey(__u8  ucPmxId, bool fgEnable)
{
	
}

void vPmxHalResume(__u8 ucPmxId)
{
	//[xzr]todo
	vVdpHalResume(ucPmxId);
}

void vPmxHalMixIsr(void)
{
	//[xzr]todo?
}

void vPmxHalDispFmtHFilter(__u8 ucVdoId, __u8 ucYC, __u8 ucCoef)
{
        unsigned int u4FmtCRegBase = 0;
        unsigned int u4FmtYRegBase = 0;
        unsigned int u4i = 0, u4Size = 0;

        if(ucVdoId == 0)
        {
                u4FmtYRegBase = 0x42000;
                u4FmtCRegBase = 0x42d00;
        }
        else
        {
                VDO_LOG(VDO_LOG_LVL_INFO, "vPmxHalDispFmtHFilter: now, vdo%d not support HFilter select\r\n", ucVdoId);
                return;
        }

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

        _rPmxDispMainSwReg.rField.fgHSLR = 0;
        _rPmxDispMainSwReg.rField.fgHD_C_FIR_EN = 1;

        _rPmxDispMainRegMode[(0xb0 / 4)] |= PMX_HAL_REG_MODE_WRITE;
        _rPmxDispMainRegMode[(0xe4 / 4)] |= PMX_HAL_REG_MODE_WRITE;
}

#endif /* _PMX_VSYNC_C_ */

