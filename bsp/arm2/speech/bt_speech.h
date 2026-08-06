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
#ifndef __AUDIO_SPEECH_H__
#define __AUDIO_SPEECH_H__

#include "bt_lib.h"

//event message
#define BT_SET_HW_RESOURCE      0x0001
#define BT_SET_PARAMETER        0x0002
#define BT_SCO_AUDIO_CONTROL    0x0003
#define BT_WRITE_FRAME          0x0004

#define BT_MSG_COMPLETED        0x0100
#define BT_FRAME_COMPLETED      0x0200
#define BT_STATE_CHANGED        0x0300
#define BT_ERROR                0x0400

// state
#define BT_STATE_UNINIT         0x0000
#define BT_STATE_INIT           0x0001
#define BT_STATE_IDLE           0x0002
#define BT_STATE_SCO            0x0003

// Parameter for message BT_SET_PARAMETER
#define BT_SPH_PARAMETER        0x0001
#define BT_DMNR_PARAMETER       0x0002
#define BT_ALL_PARAMETER        0x0007

// Parameter for message BT_STATE_SCO
#define BT_SUCCESS              0x00000000
#define BT_FAILURE              0xFFFFFFFF

#define SPEECH_FRAME_COUNT      10
#define SPEECH_FRAME_SAMPLES    160
#define SPEECH_FRAME_BYTES      320

#define DATA_REQ_POST_AEC       0x0100
#define DATA_REQ_POST_ABF       0x0200
#define DATA_REQ_POST_NDC       0x0400
#define DATA_REQ_ALL            0x0700

#define FRAME_OPT_DL            0x0001

#define FRAME_OPT_AEC           0x0002
#define FRAME_OPT_NDC           0x0004
#define FRAME_OPT_DMNR          0x0008
#define FRAME_OPT_PLC           0x0010
#define OPT_OUTPUT_LOG          0x0F00

#define BT_SCO_REQ_ALL      (FRAME_OPT_AEC|FRAME_OPT_NDC|FRAME_OPT_DMNR|FRAME_OPT_PLC)
#define BT_SCO_REQ_AECNDC   (FRAME_OPT_AEC|FRAME_OPT_NDC)


typedef struct 
{
    UINT32 u4Opt;
    UINT32 u4Param1;
    UINT32 u4Param2;
    UINT32 u4Param3;
    INT16 DLBuf[SPEECH_FRAME_SAMPLES];
    INT16 ULBuf1[SPEECH_FRAME_SAMPLES];
    INT16 ULBuf2[SPEECH_FRAME_SAMPLES];
}SPEECH_FRAME_T;


typedef struct
{
    UINT32 u4Size; 
    UINT32 u4Version;
    UINT32 u4State;
    UINT32 u4MaxFrame;
    UINT32 u4WriteIdx;
    UINT32 u4ReadIdx;
    
    SPH_ENH_ctrl_struct rSphParam;
    SPH_ENH_ctrl_struct rSphParam2;
    AEC_COM_RX_struct rAecRxParam;
    AEC_COM_TX_struct rAecTxParam;
    DMNR_PARAM_T rDmnrParam;
    
    SPEECH_FRAME_T rFrame[1];
}BT_SHARE_MEM_T;


typedef struct
{
    UINT32 u4Size; 
    UINT32 u4Version;
    UINT32 u4AECState;
    UINT32 u4MaxFrame;
    UINT32 u4WriteIdx;
    UINT32 u4ReadIdx;
    
    SPH_ENH_ctrl_struct rSphParam;
    SPH_ENH_ctrl_struct rSphParam2;
    AEC_COM_RX_struct rAecRxParam;
    AEC_COM_TX_struct rAecTxParam;
    DMNR_PARAM_T rDmnrParam;
    
    SPEECH_FRAME_T rFrame[SPEECH_FRAME_COUNT];
}BT_SHARE_MEM_EX_T;


#endif /* __AUDIO_SPEECH_H__ */
