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

#include "dual_callback.h"
#include "dual_hal.h"

//============================================================================
// Config settings
//============================================================================

//============================================================================
// Constant definitions
//============================================================================

//============================================================================
// Static variables
//============================================================================
DUAL_ISR_CALLBACK_T _arDualIsrCallBack[DUAL_MAX_TASK];

//============================================================================
// Static functions
//============================================================================
void _vInitialDualCallBack(void)
{
    UINT32 u4Tmp1;
  
    for(u4Tmp1 = 0; u4Tmp1 < DUAL_MAX_TASK; u4Tmp1++)
    {
        _arDualIsrCallBack[u4Tmp1].u4MoudleID = 0;
        _arDualIsrCallBack[u4Tmp1].pDualCallBack = 0;
    }		
}

//============================================================================
// Public functions
//============================================================================
#if 0
void vDualIsr(void)
{
  UINT32 u4Tmp1;

  UINT32 u4MoudleID; 
  UINT32 u4P1;
  UINT32 u4P2;
  UINT32 u4P3;

  for(u4Tmp1=0; u4Tmp1<DUAL_MAX_REGISTER_COMMAND; u4Tmp1++)
  {
    if(HWGetMessage(u4Tmp1, &u4MoudleID, &u4P1, &u4P2, &u4P3)== FALSE)
    {
      //ASSERT(FALSE);
    }

    if(GETMESSAGEDIR(u4MoudleID) != ARM1TOARM2)
    {
      continue;
    }  
  	
    for(u4Tmp1=0; u4Tmp1<DUAL_MAX_REGISTER_COMMAND; u4Tmp1++)
    {
      if(_arDualIsrCallBack[u4Tmp1].u4MoudleID == GETMODULEID(u4MoudleID) && _arDualIsrCallBack[u4Tmp1].pDualCallBack != 0)
      {     
        //Execute Callack
        (_arDualIsrCallBack[u4Tmp1].pDualCallBack)(u4MoudleID, u4P1, u4P2, u4P3);
        //BIM_ClearIntFromARM2();
        //BIM_ClearIrq((UINT32)u2VectorId);   
        break;
      }
    }
  }
  //BIM_ClearIntFromARM2();
  //BIM_ClearIrq((UINT32)u2VectorId);
}
#endif

BOOL fgDualCallBackInit(void)
{
    _vInitialDualCallBack();
    fgClearGroup();
    
    return TRUE;
}	

BOOL fgRegisterCallBack(UINT32 u4MoudleID, void (*DUAL_CALLBACK)(UINT32 u4MoudleID, UINT32 u4P1, UINT32 u4P2, UINT32 u4P3))
{
    UINT32 u4Tmp1;
  	
    for(u4Tmp1 = 0; u4Tmp1 < DUAL_MAX_TASK; u4Tmp1++)
    {
        if(_arDualIsrCallBack[u4Tmp1].u4MoudleID == 0 && 
            _arDualIsrCallBack[u4Tmp1].pDualCallBack == 0)
        {
            _arDualIsrCallBack[u4Tmp1].u4MoudleID = u4MoudleID;
            _arDualIsrCallBack[u4Tmp1].pDualCallBack = DUAL_CALLBACK;
            
            return TRUE;
        }
    }
  
    return FALSE;
}	

BOOL fgReleaseCallBack(UINT32 u4CommandID)
{
    UINT32 u4Tmp1;
  	
    for(u4Tmp1 = 0; u4Tmp1 < DUAL_MAX_TASK; u4Tmp1++)
    {
        if(_arDualIsrCallBack[u4Tmp1].u4MoudleID == u4CommandID)
        {
            _arDualIsrCallBack[u4Tmp1].u4MoudleID = 0;
            _arDualIsrCallBack[u4Tmp1].pDualCallBack = 0;
            
            return TRUE;
        }
    }
  
    return FALSE;
}
