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
* file dvp_fsStruct.h
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

#ifndef _DVP_FSStruct_H_
#define _DVP_FSStruct_H_


#include <windows.h>

#include "x_hal_83xx.h"
#include "dvp_protocol.h"

/*Max File num */
#define MAX_FILE_NUM  (6000)

/*default File num */
#define DEF_FILE_NUM  (2000)

#ifndef FS_RAM_CODE_MAX_FILENAME_BUF_SIZE
#define FS_RAM_CODE_MAX_FILENAME_BUF_SIZE  (82)
#endif


#ifndef CODEC_UNICODE
#define CODEC_UNICODE           0x00
#define CODEC_OTHER             0x01
#endif


enum E_FILE_FILTER {
    FILE_FILTER_ALL = 0x00,   /* 0 all*/
    FILE_FILTER_AUDIO,
    FILE_FILTER_VIDEO,
    FILE_FILTER_PIC,
    FILE_FILTER_TXT,
    FILE_FILTER_MAX,
};

enum FS_FTYPE {
    FS_FTYPE_UNKNOWN = 0xFF, /* unknown type */
    FS_FTYPE_AC3     = 0x0,
    FS_FTYPE_MP3     ,
    FS_FTYPE_MP2     ,
    FS_FTYPE_MP1     ,
    FS_FTYPE_WAV     = 0x04,
    FS_FTYPE_JPG     ,
    FS_FTYPE_MLP     ,
    FS_FTYPE_WMA     ,
    FS_FTYPE_ASF     ,
    FS_FTYPE_MPG     ,
    FS_FTYPE_DAT     ,
    FS_FTYPE_VOB     ,
    FS_FTYPE_AAC     ,
    FS_FTYPE_DSD     ,
    FS_FTYPE_MAP     = 0x0E,
    FS_FTYPE_CDA     ,
    FS_FTYPE_DIR     ,     /* directory */
    FS_FTYPE_DTS     ,
    FS_FTYPE_AVI     ,
    FS_FTYPE_QT      ,     /* for test 1 */
    FS_FTYPE_MP4     ,
    FS_FTYPE_3GP     ,
    FS_FTYPE_M4V     ,     /* for test 2 */
    FS_FTYPE_DST     ,
    FS_FTYPE_OGG     ,
    FS_FTYPE_RM      ,
    FS_FTYPE_RCV     ,
    FS_FTYPE_VC1     ,
    FS_FTYPE_MKV     ,
    FS_FTYPE_M4A     ,
    FS_FTYPE_OMA     ,
    FS_FTYPE_RA      ,
    FS_FTYPE_FLV     ,
    FS_FTYPE_OGM     ,
    FS_FTYPE_PDIR    ,
};

#pragma pack(push, 4)

struct DVPFsFileItem {
    u8      szName[FS_RAM_CODE_MAX_FILENAME_BUF_SIZE + 2];  /*Filename*/
    u8      uFType;                          /*File type*/
    u8      uCodeType;
};

struct FS_FILE_INS {
    u32      dwLBA;
    u32      dwLength;
    u8       eFType;
    u8       bNLen;
    u8       pbName[FS_RAM_CODE_MAX_FILENAME_BUF_SIZE];
};


struct FS_INS_LIST {
    u16 wCnt;
    u16 wLen;
    struct FS_FILE_INS *prList;
};


struct FS_DIR {
    u16 wDirIdx;
    struct FS_INS_LIST rFileList;
};


/*  file system super block */
struct FS_SB {
    u32 dwLba;           /* lb_addr of this dir */
    u16 wValidFileCnt;   /* number of the valid files */
    u16 wParent;         /* parent index */
    u16 wExt;            /* extension field */
    u8 *pbId;           /* ID of the super-block */
    /* other info */
};

#pragma pack(pop)

#endif /*_DVP_FSStruct_H_ */

