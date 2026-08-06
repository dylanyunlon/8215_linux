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


#include "aud_comm.h"

void Sleep(UINT32 u4MiniSecond)
{
    UINT32 u4Tick = GetARM2TickCount();
    AUDLOG_DBG((T("Sleep(%d ms) Tick(0x%x)  >>>>>>>.\r\n"), u4MiniSecond, u4Tick));
    while (GetARM2TickCount() - u4Tick < u4MiniSecond);
    AUDLOG_DBG((T("Sleep(%d ms) Tick(0x%x)  <<<<<<<.\r\n"), u4MiniSecond, GetARM2TickCount()));
}

void msleep(UINT32 u4Second)
{
      Sleep(u4Second);
}

UINT32 AudMisc_FifoFreeSize_Get(UINT32 u4Wp, UINT32 u4Rp, UINT32 BufLen)
{
    UINT32 u4FreeSize;
    
    if (u4Rp >= u4Wp)
    {
        u4FreeSize = u4Rp - u4Wp;
    }
    else if (u4Rp < u4Wp)
    {
        u4FreeSize = BufLen - u4Wp + u4Rp;
    }

    return u4FreeSize;
}

UINT32 AudMisc_FifoDataSize_Get(UINT32 u4Wp, UINT32 u4Rp, UINT32 BufLen)
{
    UINT32 u4UsedSize;
    
    if (u4Rp > u4Wp)
    {
        u4UsedSize = BufLen - u4Rp + u4Wp;
    }
    else if (u4Rp <= u4Wp)
    {
        u4UsedSize = u4Wp - u4Rp;
    }

    return u4UsedSize;
}



