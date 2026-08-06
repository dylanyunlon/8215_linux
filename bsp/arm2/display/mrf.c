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
#include "mrf.h"
#include "backcar_cfg.h"

void* GetRCObjectMemAddr(HANDLE hMrf, RCOBJECT *pObjInfo)
{
    return (void* )((BYTE *)hMrf + pObjInfo->u4OffsetData);     
}

BOOL GetWavResnfo(HANDLE hMrf, DWORD dwID, WAVOBJINFO *pInfo)
{
    RCOBJECT    *pObjInfo;
    LPVOID      lpBuf = (LPVOID)hMrf;
    MRFHEADER   *pMrfHeader = (MRFHEADER *)lpBuf;
	
    pObjInfo = (RCOBJECT *)((BYTE *)lpBuf + pMrfHeader->u4OffsetData);

    do
    {
        if (pObjInfo->u4FileID == dwID)
        {
            memcpy(pInfo, pObjInfo, sizeof(WAVOBJINFO));
            return TRUE;
        }		 	
        pObjInfo = (RCOBJECT *)((BYTE *)lpBuf + pObjInfo->u4OffsetNextImage);
    } while (pObjInfo != lpBuf);

    return FALSE;   
}

BOOL GetBitmapInfo(HANDLE hMrf, DWORD dwID, BITMAPOBJINFO *pInfo)
{
    BITMAPOBJINFO   *pBitmapInfo;
    LPVOID          lpBuf = (LPVOID)hMrf;
    MRFHEADER       *pMrfHeader = (MRFHEADER *)lpBuf;
	
    pBitmapInfo = (BITMAPOBJINFO *)((BYTE *)lpBuf + pMrfHeader->u4OffsetData);

    do
	{
        if (pBitmapInfo->u4FileID == dwID)
        {
            memcpy(pInfo, pBitmapInfo, sizeof(BITMAPOBJINFO));
            return TRUE;
        }		 	
        pBitmapInfo = (BITMAPOBJINFO *)((BYTE *)lpBuf + pBitmapInfo->u4OffsetNextImage);
    }while (pBitmapInfo != lpBuf);

    return FALSE;
}

//extern char backcar_data[]; //remove rvc resource obj

HANDLE LoadMRF()
{
//    void* lpBuf = backcar_data;
//    return (HANDLE)lpBuf;
    return NULL;
}

HANDLE LoadLogoMRF()
{
    void* lpBuf = ARM1PHY2ARM2UCV(LOGO_MRF_BUFFER_PA_ADDR); 
    return (HANDLE)lpBuf;	
}

