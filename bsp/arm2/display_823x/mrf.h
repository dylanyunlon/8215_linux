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
#ifndef  __MRF_H__
#define  __MRF_H__


#include "x_typedef.h"

#define  OBJ_TYPE_BITMAP    1
#define  OBJ_TYPE_BIN       2
#define  OBJ_TYPE_WAV       3

typedef  struct tagMRFFILEHEADER
{
    unsigned short  szFileSignature[4];
    unsigned int    u4Version;
    unsigned int    u4bitcount;
    unsigned int    u4frequence;
    unsigned int    u4picturecount;
    unsigned int    u4poswidth;
    unsigned int    u4posheight;
    unsigned int    u4OffsetHashTable;
    unsigned int    u4OffsetData;
}MRFHEADER;

typedef  struct  tagRCOBJECT 
{
    unsigned int    u4Size;
    unsigned int    objType;
    unsigned short  szImageFileName[8];
    unsigned int    u4FileID;
    unsigned int    u4OffsetData;
    unsigned int    u4SizeImage;
    unsigned int    u4OffsetNextImage;
}RCOBJECT;

typedef struct tagBITMAPOBJINFO
{
    unsigned int    u4Size;
    unsigned int    objType;
    unsigned short  szImageFileName[8];
    unsigned int    u4FileID;
    unsigned int    u4OffsetData;
    unsigned int    u4SizeImage;
    unsigned int    u4OffsetNextImage;
    unsigned int    u4Width;
    unsigned int    u4Height;
    unsigned int    u4BitCount;	
}BITMAPOBJINFO;

typedef struct tagBINFILEOBJINFO
{
    unsigned int    u4Size;
    unsigned int    objType;
    unsigned short  szImageFileName[8];
    unsigned int    u4FileID;
    unsigned int    u4OffsetData;
    unsigned int    u4SizeImage;
    unsigned int    u4OffsetNextImage;
}BINFILEOBJINFO;

typedef struct tagWAVOBJINFO
{
    unsigned int    u4Size;
    unsigned int    objType;
    unsigned short  szImageFileName[8];
    unsigned int    u4FileID;
    unsigned int    u4OffsetData;
    unsigned int    u4SizeImage;
    unsigned int    u4OffsetNextImage;
}WAVOBJINFO;

void* GetRCObjectMemAddr(HANDLE hMrf, RCOBJECT *pObjInfo);

BOOL GetWavResnfo(HANDLE hMrf, DWORD dwID, WAVOBJINFO *pInfo);

BOOL GetBitmapInfo(HANDLE hMrf, DWORD dwID, BITMAPOBJINFO *pInfo);

//HANDLE LoadMRF();

HANDLE LoadLogoMRF();

extern unsigned long logo_base;
extern unsigned long logo_size;
#define LOGO_MRF_BUFFER_PA_ADDR	(LOGO_RSV_MEM_START_ADDR_4ARM2)

#endif
