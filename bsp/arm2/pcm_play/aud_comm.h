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


#ifndef _AUD_COMM_H
#define _AUD_COMM_H

#include "x_common.h"
#include "x_typedef.h"
#include "x_bim.h"
#include "x_printf.h"

#include "dual_hal.h"
#include "dual_callback.h"
#include "dual_task.h"
#include "image_cfg.h"

   
#ifdef __cplusplus
    extern "C"
    {
#endif


/**********************************************************************************
*
*   macros
*
**********************************************************************************/

typedef BYTE *      LPBYTE;

#define T(x) x

#define AUDLOG_INFO(exp)            Printf exp
#define AUDLOG_DBG(exp)             //Printf exp


/**********************************************************************************
*
*   Data Type
*
**********************************************************************************/

enum AUD_STATE_E
{
    AUD_STATE_UNINIT = 0,
    AUD_STATE_INITED,
    AUD_STATE_STOPPED,
    AUD_STATE_STARTED,
	AUD_STATE_INIT_RSV_FAILED,
};


enum AUD_RET_TYPE_E
{
    AUD_RET_OK  = 0,
    AUD_RET_FAIL,
    AUD_RET_PARAMS_ERR,
    AUD_RET_INVALID_STATE,
};


typedef struct
{
    UINT32 u4PhySddr;       // physical start address
    UINT32 u4VirSAdr;       // Virtual start address of buffer 
    UINT32 u4ChBufSz;       // Channel Buffer size (in byte)    
    UINT32 u4DataOff;       // Data start offset (in byte)
    UINT32 u4DataSize;      // Data size of every channels (in byte)
    UINT32 u4Chn;           // channel number
    UINT32 u4BW;            // bit width
}AUD_DATA_BUF_T, *PAUD_DATA_BUF_T;


/**********************************************************************************
*
*   Function
*
**********************************************************************************/


void Sleep(UINT32 u4MiniSecond);

void msleep(UINT32 u4Second);


UINT32 AudMisc_FifoFreeSize_Get(UINT32 u4Wp, UINT32 u4Rp, UINT32 BufLen);
UINT32 AudMisc_FifoDataSize_Get(UINT32 u4Wp, UINT32 u4Rp, UINT32 BufLen);

#ifdef __cplusplus
    }
#endif
            
            
#endif // _AUD_COMM_H

