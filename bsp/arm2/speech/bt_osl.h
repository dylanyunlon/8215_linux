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

#ifndef ___BT_OS_LAYER_H___
#define ___BT_OS_LAYER_H___

#include "x_typedef.h"
#include "x_bim.h"
#include "x_printf.h"
#include "dual_hal.h"
#include "dual_callback.h"
#include "dual_task.h"


#define T(x)                x

#define SPH_NAME            T("ARM2SPEECH")

#define ENABLE_AEC          1
#define ENABLE_NDC          1
#define ENABLE_DMNR         1
#define ENABLE_PLC          1

#define ENABLE_PERFORMANCE_STAT     1
#define ENABLE_CSV_OUTPUT           0
#define USE_REAL_AEC_LIB            1

typedef CHAR MCHAR;
//typedef unsigned long long          UINT64;

//void Printf(LPCSTR sz, ...);

#define SPHLOG(cond, exp)     if(cond){Printf(T("[ARM2SPEECH]")); Printf exp;}
#define SPHLOG_INFO(exp)      if(g_u4SphLog & 0x1){Printf exp;}
#define SPHLOG_DBG(exp)       if(g_u4SphLog & 0x2){Printf exp;}
#define SPHLOG_TEST(exp)      if(g_u4SphLog & 0x4){Printf exp;} 
#define SPHLOG_DETAIL(exp)    if(g_u4SphLog & 0x8){Printf exp;}

#include "bt_speech.h"

#define TAKE_BT_HW_SEMAPHORE()          BIM_GETHWSemaphore(_u4HwSemaphore,0)
#define RELEASE_BT_HW_SEMAPHORE()       BIM_ReleaseHWSemaphore(_u4HwSemaphore)
#define BTGetSysFrequery    GetSysFrequery
#define BTGetSysTick        GetSysTick

#define AECSendMessage(p1, p2, p3, p4)  HWSendMessage(MSG_COMBINE(MODULE_AEC, p1), p2, p3, p4)

#if defined(__cplusplus)
extern "C" {
#endif   // __cplusplus


UINT32 BT_MemoryInit(VOID);
UINT32 BT_MemoryUninit(VOID);
void * BT_Malloc(UINT32 u4Size);
void  BT_Free(void *pvMemory);

#define BTMemCopy   memcpy


#if defined(__cplusplus)
}
#endif   // __cplusplus

#endif /* ___BT_OS_LAYER_H___ */
