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

//============================================================================
// Include files
//============================================================================

#include "x_bim.h"
#include "x_printf.h"
#include "dual_hal.h"
#include "dual_callback.h"
#include "dual_task.h"
#include "ac83xx.h"

//============================================================================
// Config settings
//============================================================================

//============================================================================
// Constant definitions
//============================================================================

//============================================================================
// Static variables
//============================================================================
UINT32 g_u4Arm2MemroyOffset;
UINT32 g_u4MemorySize;

#define WAIT_BITS_ZERO(reg, bitsmask) \
{\
    DWORD dwTotal = 0; \
    DWORD dwTime = GetARM2TickCount();\
    while((bitsmask) & BIM2_READ32(reg)) \
    {\
        DWORD dwTmp = GetARM2TickCount();\
        if ((dwTmp - dwTime ) > 1000)\
        {\
            dwTotal += dwTmp - dwTime;\
            Printf("[Error] Reg(0x%x)&0x%x is not zero for time(%d)\r\n",reg, bitsmask,dwTotal);\
            dwTime = dwTmp;\
        }\
    };\
}


//============================================================================
// Static functions
//============================================================================
BOOL _fgDualHALSetParmeterGroup0(UINT32 u4P1, UINT32 u4P2, UINT32 u4P3, UINT32 u4P4)
{
    //  while(MESSAGEDIRMASK & BIM2_READ32(REG_RW_SINFO0_REG));
    WAIT_BITS_ZERO(REG_RW_SINFO0_REG, MESSAGEDIRMASK);
    BIM2_WRITE32(REG_RW_SINFO1_REG, u4P2);
    BIM2_WRITE32(REG_RW_SINFO2_REG, u4P3);
    BIM2_WRITE32(REG_RW_SINFO3_REG, u4P4);
    BIM2_WRITE32(REG_RW_SINFO0_REG, u4P1); //must be last

    return TRUE;
}

BOOL _fgDualHALGetParmeterGroup0(UINT32 *pu4P1, UINT32 *pu4P2, UINT32 *pu4P3, UINT32 *pu4P4)
{
    *pu4P1 = BIM2_READ32(REG_RW_SINFO0_REG);
    *pu4P2 = BIM2_READ32(REG_RW_SINFO1_REG);
    *pu4P3 = BIM2_READ32(REG_RW_SINFO2_REG);
    *pu4P4 = BIM2_READ32(REG_RW_SINFO3_REG);

    //clear message
    BIM2_WRITE32(REG_RW_SINFO0_REG, 0);

    return TRUE;
}

BOOL _fgDualHALSetParmeterGroup1(UINT32 u4P1, UINT32 u4P2, UINT32 u4P3, UINT32 u4P4)
{
    //while(MESSAGEDIRMASK & BIM2_READ32(REG_RW_SINFO4_REG));
    WAIT_BITS_ZERO(REG_RW_SINFO4_REG, MESSAGEDIRMASK);
    BIM2_WRITE32(REG_RW_SINFO5_REG, u4P2);
    BIM2_WRITE32(REG_RW_SINFO6_REG, u4P3);
    BIM2_WRITE32(REG_RW_SINFO7_REG, u4P4);
    BIM2_WRITE32(REG_RW_SINFO4_REG, u4P1);

    return TRUE;
}

BOOL _fgDualHALGetParmeterGroup1(UINT32 *pu4P1, UINT32 *pu4P2, UINT32 *pu4P3, UINT32 *pu4P4)
{
    *pu4P1 = BIM2_READ32(REG_RW_SINFO4_REG);
    *pu4P2 = BIM2_READ32(REG_RW_SINFO5_REG);
    *pu4P3 = BIM2_READ32(REG_RW_SINFO6_REG);
    *pu4P4 = BIM2_READ32(REG_RW_SINFO7_REG);

    //clear message
    BIM2_WRITE32(REG_RW_SINFO4_REG, 0);

    return TRUE;
}

BOOL _fgDualHALSetParmeterGroup2(UINT32 u4P1, UINT32 u4P2, UINT32 u4P3, UINT32 u4P4)
{
    while(MESSAGEDIRMASK & BIM2_READ32(REG_RW_SINFO8_REG));
    WAIT_BITS_ZERO(REG_RW_SINFO8_REG, MESSAGEDIRMASK);
    BIM2_WRITE32(REG_RW_SINFO9_REG, u4P2);
    BIM2_WRITE32(REG_RW_SINFOA_REG, u4P3);
    BIM2_WRITE32(REG_RW_SINFOB_REG, u4P4);
    BIM2_WRITE32(REG_RW_SINFO8_REG, u4P1);

    return TRUE;
}

BOOL _fgDualHALGetParmeterGroup2(UINT32 *pu4P1, UINT32 *pu4P2, UINT32 *pu4P3, UINT32 *pu4P4)
{
    *pu4P1 = BIM2_READ32(REG_RW_SINFO8_REG);
    *pu4P2 = BIM2_READ32(REG_RW_SINFO9_REG);
    *pu4P3 = BIM2_READ32(REG_RW_SINFOA_REG);
    *pu4P4 = BIM2_READ32(REG_RW_SINFOB_REG);

    //clear message
    BIM2_WRITE32(REG_RW_SINFO8_REG, 0);

    return TRUE;
}

BOOL _fgDualHALSetParmeterGroup3(UINT32 u4P1, UINT32 u4P2, UINT32 u4P3, UINT32 u4P4)
{
	int count = 0;
	while (MESSAGEDIRMASK & BIM2_READ32(REG_RW_SINFOC_REG))
	{
		count++;
		if (count > 500) {
			count = 0;
			Printf("arm2,_fgDualHALSetParmeterGroup3, while count > 500 \n");
		}
	}

    WAIT_BITS_ZERO(REG_RW_SINFOC_REG, MESSAGEDIRMASK);
    BIM2_WRITE32(REG_RW_SINFOD_REG, u4P2);
    BIM2_WRITE32(REG_RW_SINFOE_REG, u4P3);
    BIM2_WRITE32(REG_RW_SINFOF_REG, u4P4);
    BIM2_WRITE32(REG_RW_SINFOC_REG, u4P1);

    return TRUE;
}

BOOL _fgDualHALGetParmeterGroup3(UINT32 *pu4P1, UINT32 *pu4P2, UINT32 *pu4P3, UINT32 *pu4P4)
{
    *pu4P1 = BIM2_READ32(REG_RW_SINFOC_REG);
    *pu4P2 = BIM2_READ32(REG_RW_SINFOD_REG);
    *pu4P3 = BIM2_READ32(REG_RW_SINFOE_REG);
    *pu4P4 = BIM2_READ32(REG_RW_SINFOF_REG);

    //clear message
    BIM2_WRITE32(REG_RW_SINFOC_REG, 0);

    return TRUE;
}

ARGS_TO_ARM2_P args_to_arm2;

ARGS_TO_ARM2_P args_from_bootloader = (ARGS_TO_ARM2_P)(0x0);

UINT32 fgDualHALGetDtbStatus(void)
{
	Flush_Invalid_Cache(args_from_bootloader, sizeof(*args_from_bootloader));
	UINT32 status = args_from_bootloader->dtb_status;
    return(status);
}

UINT32 fgDualHALSetDtbStatus(unsigned int status)
{
	args_from_bootloader->arm2_dtb_status = status;
	Flush_Cache(args_from_bootloader, sizeof(*args_from_bootloader));
    return(status);
}

//============================================================================
// Public functions
//============================================================================
UINT32 fgDualHALGetOffset(void)
{
    g_u4Arm2MemroyOffset = BIM2_READ32(REG_RW_DRAMB_OFF);
    //0x800 must snyc with EBoot or UBoot
	Flush_Invalid_Cache(args_from_bootloader, sizeof(*args_from_bootloader));
    g_u4MemorySize = args_from_bootloader->dram_size;
    Printf("g_u4Arm2MemroyOffset 0x%x\n", g_u4Arm2MemroyOffset);
    Printf("g_u4MemorySize 0x%x\n", g_u4MemorySize);

    return(g_u4Arm2MemroyOffset);
}

UINT32 fgDualHALGetUpgradeMode(void)
{
    //0x400 must snyc with EBoot or UBoot
	Flush_Invalid_Cache(args_from_bootloader, sizeof(*args_from_bootloader));
    UINT32 g_u4UpgradeMode = args_from_bootloader->upgrade_mode;

    return(g_u4UpgradeMode);
}

BOOL fgDualHALGetCmdQParameter(UINT32 *pu4P1, UINT32 *pu4P2, UINT32 *pu4P3, UINT32 *pu4P4)
{
    return _fgDualHALGetParmeterGroup1(pu4P1, pu4P2, pu4P3, pu4P4);
}

UINT32 fgSystemIndex(void)
{
	Flush_Invalid_Cache(args_from_bootloader, sizeof(*args_from_bootloader));
	UINT32 status = args_from_bootloader->system_index;
    return(status);
}

#if ATC_AB_PARTITION_SUPPORT
UINT32 fgSlotSuffix(void)
{
	Flush_Invalid_Cache(args_from_bootloader, sizeof(*args_from_bootloader));
	UINT32 status = args_from_bootloader->ab_slot;
    return(status);
}
#endif

BOOL fgDualHALINTEachOther(void)
{
	int count = 0;
	while (ARM1_REG_IRQ_STATUS & (1U << 23))
	{
		count++;
		if (count > 500) {
			count = 0;
			Printf("arm2,fgDualHALINTEachOther, while count > 500 \n");
		}
	}
    BIM2_WRITE32(REG_RW_TOCORISC, TOCORISC_INTR);

    return TRUE;
}

BOOL fgDualHALGetReturnParameter(UINT32 *pu4P1, UINT32 *pu4P2, UINT32 *pu4P3, UINT32 *pu4P4)
{
    return _fgDualHALGetParmeterGroup2(pu4P1, pu4P2, pu4P3, pu4P4);
}

BOOL HWGetMessage(UINT32 u4ModuleID, UINT32 *pu4P1, UINT32 *pu4P2, UINT32 *pu4P3, UINT32 *pu4P4)
{
  //	Printf("HWGetMessage:%u enter\n", u4ModuleID);
    if (u4ModuleID == MODULE_AEC)
    {
        _fgDualHALGetParmeterGroup0(pu4P1, pu4P2, pu4P3, pu4P4);
        BIM2_WRITE32(REG_RW_SINFO0_REG, (*pu4P1) & (~MESSAGEDIRMASK));
    }
    else if (u4ModuleID == MODULE_ARM2SYSTEMSERVICE)
    {
        _fgDualHALGetParmeterGroup1(pu4P1, pu4P2, pu4P3, pu4P4);
        BIM2_WRITE32(REG_RW_SINFO4_REG, (*pu4P1)&(~MESSAGEDIRMASK));
    }
    else if (u4ModuleID == MODULE_BOOTANIMATION)
    {
        _fgDualHALGetParmeterGroup0(pu4P1, pu4P2, pu4P3, pu4P4);
        BIM2_WRITE32(REG_RW_SINFO0_REG, (*pu4P1)&(~MESSAGEDIRMASK));
    }
    //else if (u4ModuleID == 2)
    //{
        //  _fgDualHALGetParmeterGroup2(pu4P1, pu4P2, pu4P3, pu4P4);
        //  BIM2_WRITE32(REG_RW_SINFO8_REG, (*pu4P1)&(~MESSAGEDIRMASK));
    //}
    //else if (u4ModuleID == 3)
    //{
    //  _fgDualHALGetParmeterGroup3(pu4P1, pu4P2, pu4P3, pu4P4);
    //  BIM2_WRITE32(REG_RW_SINFOC_REG, (*pu4P1)&(~MESSAGEDIRMASK));
    //}

  //	Printf("HWGetMessage:%u out\n", u4ModuleID);
    return(TRUE);
}

void fgClearGroup(void)
{
    BIM2_WRITE32(REG_RW_SINFO0_REG, 0);
    BIM2_WRITE32(REG_RW_SINFO4_REG, 0);
    BIM2_WRITE32(REG_RW_SINFO8_REG, 0);
    BIM2_WRITE32(REG_RW_SINFOC_REG, 0);
}

BOOL HWSendMessage(UINT32 u4MessageHeader, UINT32 u4P2, UINT32 u4P3, UINT32 u4P4)
{
    UINT32 u4ModuleID = GETMODULEID(u4MessageHeader);

  	Printf("arm2 send message to arm1, module is %d message %d\n", u4ModuleID, u4P2);
    u4MessageHeader = (u4MessageHeader | (ARM2TOARM1<<16));
    if (u4ModuleID == MODULE_AEC)
    {
        _fgDualHALSetParmeterGroup2(u4MessageHeader, u4P2, u4P3, u4P4);
    }
    else if (u4ModuleID == MODULE_ARM2SYSTEMSERVICE)
    {
        _fgDualHALSetParmeterGroup3(u4MessageHeader, u4P2, u4P3, u4P4);
    }
    else if (u4ModuleID == MODULE_BOOTANIMATION)
    {
        _fgDualHALSetParmeterGroup2(u4MessageHeader, u4P2, u4P3, u4P4);
    }
    //else if(u4ModuleID == MODULE_SMEM)
    //{
        //_fgDualHALSetParmeterGroup2(u4MessageHeader, u4P2, u4P3, u4P4);
    //}
    //else if(u4ModuleID == MOUDLE_REV2)
    //{
        //_fgDualHALSetParmeterGroup3(u4MessageHeader, u4P2, u4P3, u4P4);
    //}

    fgDualHALINTEachOther();

    return(TRUE);
}

