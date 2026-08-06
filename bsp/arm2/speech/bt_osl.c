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

/******************************************************************************
*[File]             bt_memory.c
*[Version]          v0.1
*[Revision Date]    2010-07-20
*[Author]           Zeng Zhang
*[Description]
*    Implementation for memory malloc/free for AEC/NDC
*
*
******************************************************************************/
#include "bt_osl.h"

#define BT_FAST_MEM_SIZE  0x8000
#define BT_MEM_SIZE       0

static BYTE BT_FAST_MEM_START[BT_FAST_MEM_SIZE];

static BYTE *_pbFastMemory = NULL;
static UINT32 _u4FastMallocSize = 0;
static UINT32 _u4FastMaxSize = 0;

static BYTE *_pbMemory = NULL;
static UINT32 _u4MaxSize = 0;
static UINT32 _u4MallocSize = 0;


UINT32 BT_MemoryInit()
{
    _pbFastMemory = (BYTE *)BT_FAST_MEM_START;
    _u4FastMaxSize = BT_FAST_MEM_SIZE;
    _u4FastMallocSize = 0;
    
    _pbMemory = NULL;
    _u4MaxSize = BT_MEM_SIZE;
    _u4MallocSize = 0;

    return 0;
}


UINT32 BT_MemoryUninit()
{
    _u4MaxSize = 0;
    _u4MallocSize = 0;

    return 0;
}


void * BT_Malloc(UINT32 u4Size)
{
    void *pvRet = NULL; 
    
    u4Size +=3;
    u4Size &= 0xFFFFFFFC;
    
    if (_pbFastMemory && (u4Size + _u4FastMallocSize <= _u4FastMaxSize))
    {
        _u4FastMallocSize += u4Size;
        pvRet = (_pbFastMemory + _u4FastMallocSize - u4Size);
    }   
    else if ((u4Size + _u4MallocSize) <= _u4MaxSize)
    {
        _u4MallocSize += u4Size;
        pvRet = (_pbMemory + _u4MallocSize - u4Size);
    }
    else
    {   
        SPHLOG(1, (T("BT_Malloc return failed. u4Size (%d)\r\n"), u4Size));
    }
    
    return (pvRet);
}


void BT_Free(void *pvMemory)
{
}





