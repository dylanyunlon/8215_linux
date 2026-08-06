/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of AutoChips Inc. (C) 2008 AutoChips Inc.
*
*  BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
*  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("AUTOCHIPS SOFTWARE")
*  RECEIVED FROM AUTOCHIPS AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON
*  AN "AS-IS" BASIS ONLY. AUTOCHIPS EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
*  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
*  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
*  NEITHER DOES AUTOCHIPS PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
*  SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
*  SUPPLIED WITH THE AUTOCHIPS SOFTWARE, AND BUYER AGREES TO LOOK ONLY TO SUCH
*  THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. AUTOCHIPS SHALL ALSO
*  NOT BE RESPONSIBLE FOR ANY AUTOCHIPS SOFTWARE RELEASES MADE TO BUYER'S
*  SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*
*  BUYER'S SOLE AND EXCLUSIVE REMEDY AND AUTOCHIPS'S ENTIRE AND CUMULATIVE
*  LIABILITY WITH RESPECT TO THE AUTOCHIPS SOFTWARE RELEASED HEREUNDER WILL BE,
*  AT AUTOCHIPS'S OPTION, TO REVISE OR REPLACE THE AUTOCHIPS SOFTWARE AT ISSUE,
*  OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY BUYER TO
*  AUTOCHIPS FOR SUCH AUTOCHIPS SOFTWARE AT ISSUE.
*
*  THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE
*  WITH THE LAWS OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF
*  LAWS PRINCIPLES.  ANY DISPUTES, CONTROVERSIES OR CLAIMS ARISING THEREOF AND
*  RELATED THERETO SHALL BE SETTLED BY ARBITRATION IN SAN FRANCISCO, CA, UNDER
*  THE RULES OF THE INTERNATIONAL CHAMBER OF COMMERCE (ICC).
*
*****************************************************************************/

/*******************************************************************************
*
* Filename:
* ---------
* file dvp_fs.h
*
* Project:
* --------
*   CNB
*
* Description:
* ------------
*
*
*
*
*------------------------------------------------------------------------------
*
*******************************************************************************/

#ifndef _DVP_FS_H_
#define _DVP_FS_H_

#include "dvp_fsStruct.h"


struct FileItem {
    u32 u4FileAddr;
    u8  uCodeType;
};


/**
*To Save File Table from DVP rFsSys
*
*
**/

struct DFileTable {
    u32        m_u4TableLen;
    u32        m_u4MaxLen;
    struct FileItem   *m_pItemTable;
    enum E_FILE_FILTER m_eType;
};

/**
*File System IO
*
*
**/

struct DFileSysIO {
    struct DFileTable   *m_pAudioTable;
    struct DFileTable   *m_pVideoTable;
    struct DFileTable   *m_pPicTable;
    u32   m_dwBaseAddr;
    u32   m_dwDVPSupportFile;
    u32   m_dwVideoFilter;
    u32   m_dwAudioFilter;
    u32   m_dwPicFilter;
    u16   m_u2WorkDir;
    bool    m_bValid;
    bool    m_u2Codec;
    enum E_FILE_FILTER m_eFilter;
};

void DFileSysIOInit(void);
void DFileSysIODeInit(void);
bool ReadFileInfo(struct DVPFsFileItem *FileItem, u32 u4ReadIndex);
bool ReadAllFileInfo(struct DVPFsFileItem *FileItem, u32 u4ReadIndex);
bool SetFilter(enum E_FILE_FILTER eFilter);
bool SetWorkDir(u16 u2DirIndex);
bool SetFsValid(u32 dwFsAddr, u32 dwFilter);
bool SetFsInvalid(void);
bool SetFLCodec(u8 u2Codec);
bool GetFsValid(void);
u32 GetWorkDir(void);
enum E_FILE_FILTER GetFilter(void);

bool InitTable(void);
bool DeInitTable(void);
bool IsSupportFile(BYTE bFilter);
bool FFillsKept(BYTE bFType, u32 dwFilter);
bool IsVaildDir(const u32 dwLBA);

EXTERN u32 GetDvpMemBaseAddr(void);
EXTERN u16 OtherCodeToUtf8(u8 *puData, u16 u2DataLen, u8  *szStr);
EXTERN u16 BigEndiaDataUcs2ToUtf8(u8 *puData, u16 u2DataLen, u8 *szStr);
EXTERN u16 BigEndiaDataUcs4ToUtf8(u8 *puData, u16 u2DataLen, u8 *szStr);
extern u32 changePhyToVirtualAddress(u32 address);


#endif


