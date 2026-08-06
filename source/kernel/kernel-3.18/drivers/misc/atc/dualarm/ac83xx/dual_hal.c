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

//============================================================================
// Include files
//============================================================================

#include <linux/jiffies.h>
#include <linux/delay.h>
#include "x_ioopt.h"
#include "ac83xx_io_macros.h"
#include "x_bim.h"
#include "x_ckgen.h"
#include "dual_hal.h"
#include "base_regs.h"
#include "drv_dual.h"
#include "irqs_vector.h"
#include "reg_interrupter.h"

//============================================================================
// Config settings
//============================================================================

//============================================================================
// Constant definitions
//============================================================================

//============================================================================
// Static variables
//============================================================================

//============================================================================
// Static functions
//============================================================================
#ifndef __linux__
#define WAIT_BITS_ZERO(reg, bitsmask) \
{\
    DWORD dwTotal = 0; \
    DWORD dwTime = GetTickCount();\
    while((bitsmask) & BIM2_READ32(reg)) \
    {\
        DWORD dwTmp = GetTickCount();\
        if ((dwTmp - dwTime ) > 1000)\
        {\
            dwTotal += dwTmp - dwTime;\
            pr_err("[Dualarm] [dual_hal.c][%s][%d] Reg(0x%x)&0x%x is not zero for time(%d)\r\n", __func__, __LINE__, reg, bitsmask,dwTotal);\
            dwTime = dwTmp;\
            msleep(10);  \
        }\
    };\
}
#endif
#define WAIT_BITS_ZERO(reg, bitsmask) \
{\
    uint32_t dwTotal = 0; \
    uint32_t dwTime = 1000*jiffies/HZ;\
    while((bitsmask) & BIM2_READ32(reg)) \
    {\
        uint32_t dwTmp = 1000*jiffies/HZ;\
        if ((dwTmp - dwTime ) > 1000)\
        {\
            dwTotal += dwTmp - dwTime;\
            pr_err("[Dualarm] [dual_hal.c][%s][%d] Reg(0x%x)&0x%x is not zero for time(%d)\r\n", __func__, __LINE__, reg, bitsmask,dwTotal);\
            dwTime = dwTmp;\
            msleep(10);  \
        }\
    }\
}
static BOOL _fgDualHALSetParmeterGroup0(UINT32 u4P1, UINT32 u4P2, UINT32 u4P3, UINT32 u4P4)
{
  //while(MESSAGEDIRMASK & BIM2_READ32(REG_RW_SINFO0_REG));
  WAIT_BITS_ZERO(REG_RW_SINFO0_REG, MESSAGEDIRMASK);
  BIM2_WRITE32(REG_RW_SINFO1_REG, u4P2); 
  BIM2_WRITE32(REG_RW_SINFO2_REG, u4P3);  
  BIM2_WRITE32(REG_RW_SINFO3_REG, u4P4);
  BIM2_WRITE32(REG_RW_SINFO0_REG, u4P1); //must be last
  return TRUE;  
}

static BOOL _fgDualHALGetParmeterGroup0(UINT32 *pu4P1, UINT32 *pu4P2, UINT32 *pu4P3, UINT32 *pu4P4)
{
  *pu4P1 = BIM2_READ32(REG_RW_SINFO0_REG);
  *pu4P2 = BIM2_READ32(REG_RW_SINFO1_REG);
  *pu4P3 = BIM2_READ32(REG_RW_SINFO2_REG);
  *pu4P4 = BIM2_READ32(REG_RW_SINFO3_REG);     

  //clear message
  BIM2_WRITE32(REG_RW_SINFO0_REG, 0);
  return TRUE;    
}	

static BOOL _fgDualHALSetParmeterGroup1(UINT32 u4P1, UINT32 u4P2, UINT32 u4P3, UINT32 u4P4)
{
  //while(MESSAGEDIRMASK & BIM2_READ32(REG_RW_SINFO4_REG));
  WAIT_BITS_ZERO(REG_RW_SINFO4_REG, MESSAGEDIRMASK);
  BIM2_WRITE32(REG_RW_SINFO5_REG, u4P2); 
  BIM2_WRITE32(REG_RW_SINFO6_REG, u4P3);  
  BIM2_WRITE32(REG_RW_SINFO7_REG, u4P4); 
  BIM2_WRITE32(REG_RW_SINFO4_REG, u4P1);
  return TRUE;  
}
#if 0
static BOOL _fgDualHALGetParmeterGroup1(UINT32 *pu4P1, UINT32 *pu4P2, UINT32 *pu4P3, UINT32 *pu4P4)
{
  *pu4P1 = BIM2_READ32(REG_RW_SINFO4_REG);
  *pu4P2 = BIM2_READ32(REG_RW_SINFO5_REG);
  *pu4P3 = BIM2_READ32(REG_RW_SINFO6_REG);
  *pu4P4 = BIM2_READ32(REG_RW_SINFO7_REG);     

  //clear message
  BIM2_WRITE32(REG_RW_SINFO4_REG, 0);
  return TRUE;    
}

static BOOL _fgDualHALSetParmeterGroup2(UINT32 u4P1, UINT32 u4P2, UINT32 u4P3, UINT32 u4P4)
{
  //while(MESSAGEDIRMASK & BIM2_READ32(REG_RW_SINFO8_REG));
  WAIT_BITS_ZERO(REG_RW_SINFO8_REG, MESSAGEDIRMASK);
  BIM2_WRITE32(REG_RW_SINFO9_REG, u4P2); 
  BIM2_WRITE32(REG_RW_SINFOA_REG, u4P3);  
  BIM2_WRITE32(REG_RW_SINFOB_REG, u4P4);
  BIM2_WRITE32(REG_RW_SINFO8_REG, u4P1);
  return TRUE;  
}
#endif
static BOOL _fgDualHALGetParmeterGroup2(UINT32 *pu4P1, UINT32 *pu4P2, UINT32 *pu4P3, UINT32 *pu4P4)
{
  *pu4P1 = BIM2_READ32(REG_RW_SINFO8_REG);
  *pu4P2 = BIM2_READ32(REG_RW_SINFO9_REG);
  *pu4P3 = BIM2_READ32(REG_RW_SINFOA_REG);
  *pu4P4 = BIM2_READ32(REG_RW_SINFOB_REG);     

  //clear message
  BIM2_WRITE32(REG_RW_SINFO8_REG, 0);
  return TRUE;    
}
#if 0
static BOOL _fgDualHALSetParmeterGroup3(UINT32 u4P1, UINT32 u4P2, UINT32 u4P3, UINT32 u4P4)
{
//  while(MESSAGEDIRMASK & BIM2_READ32(REG_RW_SINFOC_REG));
  WAIT_BITS_ZERO(REG_RW_SINFOC_REG, MESSAGEDIRMASK);
  BIM2_WRITE32(REG_RW_SINFOD_REG, u4P2); 
  BIM2_WRITE32(REG_RW_SINFOE_REG, u4P3);  
  BIM2_WRITE32(REG_RW_SINFOF_REG, u4P4); 
  BIM2_WRITE32(REG_RW_SINFOC_REG, u4P1);
  return TRUE;  
}
#endif
static BOOL _fgDualHALGetParmeterGroup3(UINT32 *pu4P1, UINT32 *pu4P2, UINT32 *pu4P3, UINT32 *pu4P4)
{
  *pu4P1 = BIM2_READ32(REG_RW_SINFOC_REG);
  *pu4P2 = BIM2_READ32(REG_RW_SINFOD_REG);
  *pu4P3 = BIM2_READ32(REG_RW_SINFOE_REG);
  *pu4P4 = BIM2_READ32(REG_RW_SINFOF_REG);     

  //clear message
  BIM2_WRITE32(REG_RW_SINFOC_REG, 0);
  return TRUE;    
}

//============================================================================
// Public functions
//============================================================================
BOOL fgDualHALInit(void)
{
  return TRUE;
}	

BOOL fgDualHALStart(void)
{
  UINT32 u4Tmp1;
  	
  //Step.1 Reset ARM2
  u4Tmp1 = BIM_READ32(REG_RW_RISCRST);  
  u4Tmp1 = u4Tmp1 | RISCRST_RISC1_RESET;
  BIM_WRITE32(REG_RW_RISCRST, u4Tmp1);
  
  return TRUE;
}

BOOL fgDualHALStop(void)
{
  // Step.1 Turn off ARM2
  BIM_WRITE32(REG_RW_RISCRST, RISCRST_PASSWD);
  BIM_WRITE32(REG_RW_RISCRST, RISCRST_RISC1_PASSWD);

  return TRUE;
}

BOOL fgDualHALSetRemap(void)
{
  BIM2_WRITE32(REG_RW_REMAP, BIM2_REG32(REG_RW_REMAP) | REMAP_BIT);
  return TRUE;
}


BOOL fgDualHALSetOffset(UINT32 u4Offset)
{
  BIM2_WRITE32(REG_RW_DRAMB_OFF, u4Offset);
  return TRUE;
}	

UINT32 u4DualHALOffsetAlignment(void)
{
  return DUAL_DRAMB_OFFSET_ALIGNMENT;
}	



BOOL fgDualHALSetBootUpParameter(UINT32 u4P1, UINT32 u4P2, UINT32 u4P3, UINT32 u4P4)
{
  return _fgDualHALSetParmeterGroup0(u4P1, u4P2, u4P3, u4P4);
}

	
BOOL fgDualHALGetBootUpParameter(UINT32 *pu4P1, UINT32 *pu4P2, UINT32 *pu4P3, UINT32 *pu4P4)
{
  return _fgDualHALGetParmeterGroup0(pu4P1, pu4P2, pu4P3, pu4P4);
}



BOOL fgDualHALSetSendCommandParameter(UINT32 u4P1, UINT32 u4P2, UINT32 u4P3, UINT32 u4P4)
{
  return _fgDualHALSetParmeterGroup1(u4P1, u4P2, u4P3, u4P4);
}

		
BOOL fgDualHALINTEachOther(void)
{
  int count = 0;
  while (ARM2_REG_IRQ_STATUS & (1U << 23))
  {
	  count++;
	  if (count > 500) {
		  count = 0;
		  msleep(10);
	  }
  }
  BIM_WRITE32(REG_RW_TOCORISC, TOCORISC_INTR);  
  	
  return TRUE;
}	

BOOL fgDualHALGetReturnParameter(UINT32 *pu4P1, UINT32 *pu4P2, UINT32 *pu4P3, UINT32 *pu4P4)
{	
  return _fgDualHALGetParmeterGroup2(pu4P1, pu4P2, pu4P3, pu4P4);	
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
  
  u4MessageHeader = (u4MessageHeader | (ARM1TOARM2<<16));
  if (u4ModuleID == MODULE_AEC)
  {
    _fgDualHALSetParmeterGroup0(u4MessageHeader, u4P2, u4P3, u4P4);
  }
  else if (u4ModuleID == MODULE_BCAR)
  {
 //   _fgDualHALSetParmeterGroup1(u4MessageHeader, u4P2, u4P3, u4P4);
  }
  else if (u4ModuleID == MODULE_ARM2SYSTEMSERVICE)
  {
    _fgDualHALSetParmeterGroup1(u4MessageHeader, u4P2, u4P3, u4P4);
  }
  else if (u4ModuleID == MODULE_BOOTANIMATION)
  {
    _fgDualHALSetParmeterGroup0(u4MessageHeader, u4P2, u4P3, u4P4);
  }
  //else if(u4ModuleID == MODULE_SMEM)
  //{
  //  _fgDualHALSetParmeterGroup2(u4MessageHeader, u4P2, u4P3, u4P4);
  //}
  //else if(u4ModuleID == MOUDLE_REV2)
  //{
  //  _fgDualHALSetParmeterGroup3(u4MessageHeader, u4P2, u4P3, u4P4);
  //}
  
  fgDualHALINTEachOther();

  return(TRUE);
}

BOOL HWGetMessage(UINT32 u4ModuleID, UINT32 *pu4P1, UINT32 *pu4P2, UINT32 *pu4P3, UINT32 *pu4P4)
{
  if (u4ModuleID == MODULE_AEC)
  {
    _fgDualHALGetParmeterGroup2(pu4P1, pu4P2, pu4P3, pu4P4);
  }
  else if (u4ModuleID == MODULE_ARM2SYSTEMSERVICE)
  {
    _fgDualHALGetParmeterGroup3(pu4P1, pu4P2, pu4P3, pu4P4);
  }
  else if (u4ModuleID == MODULE_BOOTANIMATION)
  {
    _fgDualHALGetParmeterGroup2(pu4P1, pu4P2, pu4P3, pu4P4);
  }
 // else if(u4ModuleID == 2)
 // {
 //   _fgDualHALGetParmeterGroup2(pu4P1, pu4P2, pu4P3, pu4P4);
 // }
 // else if(u4ModuleID == 3)
 // {
 //   _fgDualHALGetParmeterGroup3(pu4P1, pu4P2, pu4P3, pu4P4);
 // }

  return(TRUE);
}

