/* Copyright Statement:
 *
 * This software/firmware and related documentation ("AutoChips Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to AutoChips Inc. and/or its licensors. Without
 * the prior written permission of AutoChips inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of AutoChips Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 * 
 * AutoChips Inc. (C) 2016. All rights reserved.
 * 
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("AUTOCHIPS SOFTWARE")
 * RECEIVED FROM AUTOCHIPS AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. AUTOCHIPS EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES AUTOCHIPS PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE AUTOCHIPS SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN AUTOCHIPS
 * SOFTWARE. AUTOCHIPS SHALL ALSO NOT BE RESPONSIBLE FOR ANY AUTOCHIPS SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND AUTOCHIPS'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE AUTOCHIPS SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT AUTOCHIPS'S OPTION, TO REVISE OR REPLACE THE
 * AUTOCHIPS SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO AUTOCHIPS FOR SUCH AUTOCHIPS SOFTWARE AT ISSUE.
 */

/** @file ddr_init.c
 *  ddr init flow control.
 */
#include "ddr_includes.h"
#include "ddr.h"

DRAM_DESC_T g_dram_desc;

void SetDDRAgtPRI()
{
    UINT32 u4Tmp = 0;
    DRAM_DMARB_WRITE32(DRAMB_REG_PRI0, DYNPRI0);
    DRAM_DMARB_WRITE32(DRAMB_REG_PRI1, DYNPRI1);
    
    u4Tmp = DRAM_DMARB_READ32(DRAMB_REG_DYNPRI);
    u4Tmp = u4Tmp | (BIT_PRIUPDAGE_EN);
    DRAM_DMARB_WRITE32(DRAMB_REG_DYNPRI, u4Tmp);
}


void ddr_init_dramc_dram(void)
{   
    // Init channel A DRAM.
    TCMSET_CHANNELA_ACTIVE();
    DDR_SetDramController();

#if 0
    // Init channel B DRAM.
    if (IS_DRAM_CHANNELB_SUPPORT())
    {
        mcSHOW_DBG_MSG("Channel B DRAM enable.\n");
		
        TCMSET_CHANNELB_ACTIVE();        
        DDR_SetDramController();
    }
#endif    
}


void vDDR_BLConfig(void)
{
    UINT32 u4Ret;
    UINT32 u4DramType = 0;
	UINT32 u4Val;
/*
      if(core off boot) loader from PDWNC SRAM else from BL
*/
    TCM_DRAM_SIZE   = 0;
    TCM_DRAM_FLAGS  = 0;
    TCM_DRAM_FLAGS1 = 0;
    TCM_DRAM_FLAGS2 = 0;
    TCM_DRAM_FLAGS3 = 0;
    TCM_DRAM_FLAGS4 = 0;
    TCM_DRAM_DLYCELL_PERT = 0x3131;
    TCM_DRAM_DATA_WIDTH = 0;

    TCMSET_CHANNELA_ACTIVE();
    
    // Set DRAM type.
    u4Ret = DRVCUST_InitGet(eDramType);
    TCM_DRAM_FLAGS |= ((u4Ret & 0xF) << TYPE_SHIFT);
    u4DramType = u4Ret;

    // Set DRAM clock.
    vDDR_LoadBLClock();                    //mtk40739 load clock info from sram
    //vDDR_DramCheckPCBVersion();   //mtk40739 mark for refine code size
    //TCM_DRAM_FLAGS |= ((DEFAULT_DDR_CLOCK / BASE_DDR_CLK) & DRAM_CLOCK_MASK);//mtk40739 manual fix clock here

    // Set DRAM DMSS on/off.
    if ((DRVCUST_InitGet(eDmpllSpectrumPermillage) != 0) &&
         (DRVCUST_InitGet(eDmpllSpectrumFrequency) != 0))
    {
        TCM_DRAM_FLAGS |= DRAM_DMSSON;
		
	//	LOG(1, "\nDRAM_DMSSON ON\n");
    }

	//dc-balance on/off.
	TCM_DRAM_FLAGS |=  (DRVCUST_InitGet(eFlagDDRDCBalance)) ? DRAM_DC_BALANCE : 0;
	// Default enter normal state
	TCM_DRAM_FLAGS &= ~DRAM_SUSPEND_STATE;

	if(FLAG_DDR_DCBALANCE == 1)
	{   
	    mcSHOW_DBG_MSG2("DC Balance ON with Resistance\n");
	}
	else
	{
	   mcSHOW_DBG_MSG2("DC Balance OFF without Resistance\n");
	}

    #if 0
    // Set if channel B is support.
    if ((u4DramType == DDR_II_x3) || (u4DramType == DDR_III_x3))
    {
        TCM_DRAM_FLAGS |= DRAM_CHANNELB_SUPPORT;
        TCM_DRAM_FLAGS |= DRAM_CHB_FORCE32;
    }
    else if((u4DramType == DDR_II_x4) || (u4DramType == DDR_III_x4))
    {
    	TCM_DRAM_FLAGS |= DRAM_CHANNELB_SUPPORT;
    }

    // Set 16 bit mode or 32 bit mode data bus.
    if ((u4DramType == DDR_II_x1) ||(u4DramType == DDR_III_x1))
    {
        TCM_DRAM_FLAGS |= DRAM_CHA_FORCE32;
    }

    // Set bus is x8 or x16.
    TCM_DRAM_FLAGS |=  DRVCUST_InitGet(eDdrBusX8) ? DRAM_BUSX8 : 0;
    #else
    //TCM_DRAM_FLAGS |= DRAM_CHA_FORCE32;
    TCMSET_DATA_WIDTH(DATA_WIDTH_16BIT);
    #endif

    // Set CAS latency by DRAM clock.
	if(TCMGET_DDR_CLK()<= CLK_1600MHZ)
    {
        TCM_DRAM_FLAGS |= ((11) << CL_SHIFT);
    }
	else if(TCMGET_DDR_CLK() <= CLK_1866MHZ)
    {
        TCM_DRAM_FLAGS |= ((13) << CL_SHIFT);
    }
    else
    {
        TCM_DRAM_FLAGS |= ((14) << CL_SHIFT);
    }
    SetDDRAgtPRI();
}


void DDR_Initialize(void)
{
	UINT32 u4Ret = 0;
	UINT32 u4Val;

	
    //LOG(1, "\ndramk , "__DATE__ " " __TIME__"\n");
    LOG(1, "dram setting version:" DRAM_SETTING_VERSION "\n");

    vDDR_BLConfig();
    //========mem init============
    DdrPhyInit(); //set mempll

    //========dramc & dram init============
    ddr_init_dramc_dram();

    //========dram calibration============
    //ddr_calibrate();
    
    //========test dram after calibration============
#ifdef DRAM_WRITE_READ_LOOP_AFTER_CALIBRATION
    dram_test_after_cali();
#endif

    //LOG(1, "\ndramk finish\n");

    //DDR_EnterSuspend(0);   
	//DDR_EnterResume(0);


	
#if 0 // dump dramc register
    {
    extern DRAM_STATUS_T DramcRegDump();
    DramcRegDump();
    while(1);
    }
#endif

}



