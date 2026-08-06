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
* file dvp_fsIo.h
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

#ifndef _DVP_FSIO_H_
#define _DVP_FSIO_H_

#include "dvp_fs.h"


void DVPPFs_Init(void);
void DVPPFs_DeInit(void);

/**
*Get File Info From FS
*@param [out] pFileItem, Return File Info to pFileItem
*@param [in] u4ReadIndx, To Read the File Index
**/
bool DVPFs_ReadFileInfo(struct DVPFsFileItem *pFileItem, u32 u4ReadIndex);


/**
*Set File Filter to FS
*
*@param [in] eFilter, when ReadFileInfo From FS, Only return this Filter.
**/
bool DVPFs_SetFilter(enum E_FILE_FILTER eFilter);


/**
*Set File Filter to FS
*
*@param [in] u4DirIndex, Set Fs WorkDir
**/
bool DVPFs_SetWorkDir(u16 u2DirIndex);

/**
*Set Valid
*
*@param [in] dwFsAddr, File Table Addr, dwFilter DVP Support file
*
**/
bool DVPFs_SetFsValid(u32 dwFsAddr, u32 dwFilter);


/**
*Set Valid
*
*Invaild the File System
**/
bool DVPFs_SetFsInvalid(void);


/**
*u2Codec
*
*@param [in]  u3Code,   CODEC_UNICODE or CODEC_OTHER
*Set File Exten name Codec
**/

bool DVPFs_SetFLCodec(u8 u2Codec);

#endif



