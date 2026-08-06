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

//============================================================================
// Include files
//============================================================================

#include "bt_osl.h"
#include "bt_perf_stat.h"


SPH_ENH_ctrl_struct Sph_Enh_ctrl = 
{
    {
        96  ,   // AEC NLP
        224 ,   // AEC control word
        5256,   // AEC Echo suppression
        30  ,   // NDC UL control word
        57351,  // NDC NR
        31   ,  // NDC DL control word
        400  ,  // NDC calibration
        0    ,  // Digital Gain
        80   ,  // NDC NR
        4325 ,  // NDC NR aggressive mode
        4195 ,  // NDC RINI
        0   ,
        20488,  // AEC AES
        0 ,   // ABF control (0 - ABF off)
        0 ,   // ABF Post filtering (0 - ABF off)
        0 ,
        0 ,
        0 ,
        0 , 
        32767,  // Clipping
        32769,
        0,
        0,
        0,
        0,
        0,
        0,
        0
    },
    0
};

static Word16 ABF_cal_data[96] = 
{
    9392,       0,      0,      0,      0,      0,      0,      0,
       0,       0,   5706,   -462,    675,  -4827,   3236,  -4161,
    4984,   -2334,  -1140,  -1183,   4484,   -722,     32,  -8171,
    4010,    4948,   3567,  -4141,   -264,   -582,   6085,  -1284,
    5499,   -1377,   -826,   2772,   2988,    -36,   1084,  -2452,
    7351,   2197,      24,   -403,   -613,   -888,  -2409,   1464,
    -193,    4704,   4393,   3641,   -942,  -2038,   1143,  -1773,
    4498,     238,   1388,    788,   8976,  -7475,  -9758,  -3220,
    -425,    -353,   1300,   2073,   4677,    570,   9315,  -6417,
   -8499,   -3277,  -6701,    935,   6436,   1392,   1980,   -425,
       7,   20000,  20000,  20000,  20000,  20000,  20000,  20000,
   20000,       1,  21000,      0,      0,      0,      0,      0
};
static Word16 aec_com_rx[22] = {32767, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static Word16 aec_com_tx[22] = {32767, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};


static UINT32 _u4HwSemaphore = 0;
static UINT32 _u4State = BT_STATE_UNINIT;

static BT_SHARE_MEM_T    *g_prShareMem   = NULL;
static BT_SHARE_MEM_EX_T *g_prShareMemEx = NULL;

extern  BT_SHARE_MEM_T *g_prShareMem;

static UINT32 _u4BTDataReq = 0;
static SPEECH_FRAME_T *_prFrame = NULL;

static INT16 *_pi2SMDL = NULL;
static INT16 *_pi2SMUL = NULL;
static INT16 *_pi2SMUL2 = NULL;

UINT32 g_u4SphLog = 0;

#define INCREASE_READ_IDX  (g_prShareMem->u4ReadIdx  = (g_prShareMem->u4ReadIdx  + 1) % (g_prShareMem->u4MaxFrame << 1))
#define INCREASE_WRITE_IDX (g_prShareMem->u4WriteIdx = (g_prShareMem->u4WriteIdx + 1) % (g_prShareMem->u4MaxFrame << 1))
#define IS_FRAME_FOR_AEC   (g_prShareMem->u4ReadIdx != g_prShareMem->u4WriteIdx)
#define NUM_FRAME_FOR_AEC  ((g_prShareMem->u4WriteIdx  >= g_prShareMem->u4ReadIdx) ? \
                            (g_prShareMem->u4WriteIdx - g_prShareMem->u4ReadIdx) : \
                            (g_prShareMem->u4WriteIdx + (g_prShareMem->u4MaxFrame << 1) - g_prShareMem->u4ReadIdx))
#define IS_FRAME_FOR_WRITE (NUM_FRAME_FOR_AEC < g_prShareMem->u4MaxFrame)


struct BT_DELAYED_MSG
{
    UINT32 u4MsgId;
    UINT32 u4P1;
    UINT32 u4P2;
    UINT32 u4P3;

    BOOL fgHasDelayed;
} _rDelayedMsg = {0, 0, 0, 0, FALSE};

static UINT16 _ai2BufDL[SPEECH_FRAME_SAMPLES];
static UINT16 _ai2BufUL[SPEECH_FRAME_SAMPLES];
static UINT16 _ai2BufUL2[SPEECH_FRAME_SAMPLES];

static UINT32 _u4DbgMiniSecond = 0;

extern UINT32 GetARM2TickCount(VOID);

//==========================================//
  #define CodeSight_Speech_AEC
//==========================================//

static UINT32 AECInit()
{
    int  i, iAECMemSz = 0;
    int* piAECMemPtr = NULL;
    unsigned char *pbSignSAdr = (unsigned char *)0XAC0FF000; //Start Address for signature.

    iAECMemSz = ENH_API_Get_Memory(&Sph_Enh_ctrl);
    piAECMemPtr = (int *)BT_Malloc(iAECMemSz);
    if (NULL == piAECMemPtr)
    {
        SPHLOG(1, (T("[ARM2SPEECH]AECInit malloc piAECMemPtr error!\r\n")));
        return (AEC_RESULT_FAILED);
    }
    SPHLOG_INFO((T("[ARM2SPEECH]AEC Init: SAdr(0x%x) size(%d)\r\n"), piAECMemPtr, iAECMemSz));
    
    pbSignSAdr = (unsigned char *)piAECMemPtr;

    for (i = 0; i < iAECMemSz; i++)
    {
        *pbSignSAdr++ = 0;
    }
    ENH_API_Alloc(&Sph_Enh_ctrl, piAECMemPtr);

    ENH_API_Init_AEC(&Sph_Enh_ctrl, &aec_com_rx[0], &aec_com_tx[0]);
    ENH_API_Init_ABF(&Sph_Enh_ctrl, ABF_cal_data);
    ENH_API_AGC_Init( 14 );
    ENH_API_Init_PLC();

    SPHLOG_INFO((T("[ARM2SPEECH]AEC Init successs! \r\n")));

    return (AEC_RESULT_SUCESS);
}


static UINT32 AECULProcess(INT16 * pi2UL_sp, INT16* pi2DL_sp, INT16 * pi2UL2_sp)
{
    ENH_API_AGC_2(pi2UL_sp, pi2UL2_sp);
    ENH_API_Run_Aec_UL(&Sph_Enh_ctrl, pi2UL_sp, pi2DL_sp, pi2UL2_sp);   // run UL  AEC + ABF

    return (AEC_RESULT_SUCESS);
}


static UINT32 AECDLProcess(INT16* pi2DL_sp)
{
    ENH_API_Run_Aec_DL(&Sph_Enh_ctrl,pi2DL_sp);           
   
    return (AEC_RESULT_SUCESS);
}


static UINT32 AECDeinit(VOID)
{
    ENH_API_Free_AEC();

    return (AEC_RESULT_SUCESS);
}


//==========================================//
  #define CodeSight_Speech_NDC
//==========================================//

static UINT32 NDCInit()
{
    NDC_Com_Init(Sph_Enh_ctrl.enhance_pars);
    NDC_UL_Init();
    NDC_DL_Init();
    
    return (AEC_RESULT_SUCESS);
}


static UINT32 NDCULProcess(INT16 *pwLinkBuffer)
{
    NDC_UL_MAIN(pwLinkBuffer);

    return (AEC_RESULT_SUCESS);
}


static UINT32 NDCDLProcess(INT16 *pwLinkBuffer)
{
    NDC_DL_MAIN(pwLinkBuffer);

    return (AEC_RESULT_SUCESS);
}


static UINT32 NDCDeinit(VOID)
{
    return (AEC_RESULT_SUCESS);
}


//==========================================//
  #define CodeSight_Speech_Process_Srv
//==========================================//

static BOOL InitFramePointers(VOID)
{
    BOOL fgRet = FALSE;
    
    TAKE_BT_HW_SEMAPHORE();
    if (IS_FRAME_FOR_AEC)
    {
        UINT32 u4Idx = g_prShareMem->u4ReadIdx % g_prShareMem->u4MaxFrame;
        _prFrame = g_prShareMem->rFrame + u4Idx;

        _pi2SMUL  = _prFrame->ULBuf1;
        _pi2SMDL  = _prFrame->DLBuf;
        _pi2SMUL2 = _prFrame->ULBuf2;
        
        fgRet = TRUE;
    }
    RELEASE_BT_HW_SEMAPHORE();
    
    return (fgRet);
}


static BOOL UpdateReadIndex()
{
    UINT32 u4ReadIdx;
    TAKE_BT_HW_SEMAPHORE();
    u4ReadIdx = g_prShareMem->u4ReadIdx;
    INCREASE_READ_IDX;
    RELEASE_BT_HW_SEMAPHORE();
    AECSendMessage(BT_FRAME_COMPLETED, u4ReadIdx, 0, 0);

    return (TRUE);
}

UINT32 u4Cnt = 0;

static BOOL SpeechDLProcess()
{
    INT16 *pi2DL = _ai2BufDL;

    SPHLOG_DETAIL((T("[ARM2SPEECH]SpeechDLProcess Start %dms\r\n"), GetARM2TickCount()));

    BTMemCopy(pi2DL, _pi2SMDL, SPEECH_FRAME_BYTES);

    TimeStatEnter(STAT_IDX_DL);

    if (_prFrame->u4Opt & FRAME_OPT_PLC)
    {
        TimeStatEnter(STAT_IDX_DL_PLC);
        ENH_API_PLC(pi2DL);
        TimeStatLeave(STAT_IDX_DL_PLC);
    }

    if (_prFrame->u4Opt & FRAME_OPT_NDC)
    {
        TimeStatEnter(STAT_IDX_DL_NDC);
        NDCDLProcess(pi2DL);
        TimeStatLeave(STAT_IDX_DL_NDC);
    }

    if (_prFrame->u4Opt & DATA_REQ_POST_NDC)    // Copy data after NDC back to  UL Buffer1
    {      
        BTMemCopy(_prFrame->ULBuf1, pi2DL, SPEECH_FRAME_BYTES);
    }

    if (_prFrame->u4Opt & FRAME_OPT_AEC)
    {
        TimeStatEnter(STAT_IDX_DL_AEC);
        AECDLProcess(pi2DL);
        TimeStatLeave(STAT_IDX_DL_AEC);
    }

    TimeStatLeave(STAT_IDX_DL);

    // Copy final data back to DL buffer.
    BTMemCopy(_pi2SMDL, pi2DL, SPEECH_FRAME_BYTES);

    UpdateReadIndex();
    
    return (TRUE);
}


static BOOL SpeechULProcess()
{
    INT16 *pi2DL  = (INT16 *)_ai2BufDL;
    INT16 *pi2UL  = (INT16 *)_ai2BufUL;
    INT16 *pi2UL2 = (INT16 *)_ai2BufUL2;

    BTMemCopy(pi2UL,  _pi2SMUL,  SPEECH_FRAME_BYTES);
    BTMemCopy(pi2DL,  _pi2SMDL,  SPEECH_FRAME_BYTES);
    BTMemCopy(pi2UL2, _pi2SMUL2, SPEECH_FRAME_BYTES);
    
    TimeStatEnter(STAT_IDX_UL);

    if (_prFrame->u4Opt & FRAME_OPT_AEC)  // UL AEC Process
    {        
        TimeStatEnter(STAT_IDX_UL_AEC);
        AECULProcess(pi2UL, pi2DL, pi2UL2);
        TimeStatLeave(STAT_IDX_UL_AEC);
    }

    if (_prFrame->u4Opt & DATA_REQ_POST_AEC)    // Copy data after AEC to UL Buffer
    {      
        BTMemCopy(_pi2SMDL, pi2UL, SPEECH_FRAME_BYTES);
    }
    
    if (_prFrame->u4Opt & FRAME_OPT_NDC)    // UL NDC process
    {
        TimeStatEnter(STAT_IDX_UL_NDC);
        NDCULProcess(pi2UL);
        TimeStatLeave(STAT_IDX_UL_NDC);
    }
    
    TimeStatLeave(STAT_IDX_UL);

    // Copy final data back to DL Buffer
    BTMemCopy(_pi2SMUL, pi2UL, SPEECH_FRAME_BYTES);

    UpdateReadIndex();
    SPHLOG_DETAIL((T("[ARM2SPEECH]SpeechULProcess End %dms\r\n"), GetARM2TickCount()));
    
    return (TRUE);
}


static BOOL SpeechFrameProcess()
{
    UINT32 u4MaxProFrame = 1;
    UINT32 u4Idx = g_prShareMem->u4ReadIdx;
    
    TimeStatEnter(STAT_IDX_AEC_NDC);
    while(u4MaxProFrame && InitFramePointers())
    {
        u4Cnt++;
        if(u4Cnt > 100 && u4Cnt < 120)
            SPHLOG_INFO((T("[ARM2SPEECH]SpeechFrameProcess(%d) Opt(0x%x)\r\n"), u4Idx, _prFrame->u4Opt));
        if (_prFrame->u4Opt & FRAME_OPT_DL)
        {
            SpeechDLProcess();
        }
        else
        {
            SpeechULProcess();
        }
        u4MaxProFrame --;
    }
    TimeStatLeave(STAT_IDX_AEC_NDC);

    return (u4MaxProFrame < 10);
}


//==========================================//
  #define CodeSight_Speech_EventSrv
//==========================================//

static UINT32 BTSetHWResource(UINT32 u4PhyAddr, UINT32 u4Size, UINT32 u4HwSema)
{
    UINT32 u4Ret = BT_SUCCESS;
    
    if ((BT_STATE_SCO == _u4State) || (BT_STATE_UNINIT == _u4State))
    {   
         u4Ret = (BT_FAILURE);  // Can't handle BT_SET_HW_RESOURCE in these states
    }
    else if (!u4PhyAddr && !u4Size && !u4HwSema)
    {     
         u4Ret = (BT_FAILURE); // Invalid resource
    }
    else
    {
        g_prShareMem = (BT_SHARE_MEM_T *)ARM1PHY2ARM2UCV(u4PhyAddr);
        _u4HwSemaphore = u4HwSema;
        if (BT_STATE_INIT == _u4State)
        {
            _u4State = BT_STATE_IDLE;
            g_prShareMem->u4State = _u4State;
            SPHLOG(1, (T("[ARM2SPEECH]Enter IDLE State. g_prShareMem(0x%x).\r\n"), g_prShareMem));         
        }
        AECSendMessage(BT_MSG_COMPLETED, BT_SET_HW_RESOURCE, _u4State, 0);
        u4Ret = BT_SUCCESS;
    }
    
    return (u4Ret);
}


static UINT32 BTSetParameter(UINT32 u4Which)
{
    UINT32 u4Ret = BT_SUCCESS;
    if ((BT_STATE_UNINIT == _u4State) || (BT_STATE_SCO == _u4State))
    {        
        u4Ret = (BT_FAILURE);   // Don't handle this message in these state.
    }
    else
    {
        TAKE_BT_HW_SEMAPHORE();
        if (BT_SPH_PARAMETER & u4Which) {
            SPH_PARAM_T *prSphParam = &g_prShareMem->rSphParam;
        }
        if (BT_DMNR_PARAMETER & u4Which) {
            DMNR_PARAM_T *prDmnrParam = &g_prShareMem->rDmnrParam;;
        }
        RELEASE_BT_HW_SEMAPHORE();
    }
    return (u4Ret);
}


static UINT32 BTWriteFrame(UINT32 u4FrameIdx)
{
    return (BT_SUCCESS);
}


static UINT32 BTEnterSCO(BOOL fgEnter, UINT32 u4Request)
{
    UINT32 u4Ret = BT_SUCCESS;
    
    if ((fgEnter && (BT_STATE_IDLE != _u4State)) || (!fgEnter && (BT_STATE_SCO !=  _u4State)))
    {      
        u4Ret = BT_FAILURE; //Error
        goto EXIT;
    }
    u4Cnt = 0;
    if (fgEnter)    // Enter SCO State. 
    {      
        UINT32 i, u4Return;
        
        BT_MemoryInit();
        _u4BTDataReq = u4Request;
        g_u4SphLog = (u4Request & OPT_OUTPUT_LOG) >> 8;

        if (u4Request & FRAME_OPT_DMNR) {
            BTMemCopy(&Sph_Enh_ctrl, &g_prShareMem->rSphParam2, sizeof(SPH_ENH_ctrl_struct));
        } else {
            BTMemCopy(&Sph_Enh_ctrl, &g_prShareMem->rSphParam,  sizeof(SPH_ENH_ctrl_struct));
        }

        BTMemCopy(ABF_cal_data, g_prShareMem->rDmnrParam.dmnrParm, sizeof(DMNR_PARAM_T));
        SPHLOG_INFO((T("[ARM2SPEECH]AEC Parameters Request(0x%x)\n"), u4Request));
        for (i = 0; i < AEC_NDC_PARAM_NUM; i++)
        {          
            if (!(AEC_NDC_PARAM_NUM % 10)) {
                SPHLOG_INFO((T("\r\n")));
            }
            SPHLOG_INFO((T("par[%d] = %d "),  i, Sph_Enh_ctrl.enhance_pars[i]));
        }
        SPHLOG_INFO((T("\r\n")));

        if (u4Request & FRAME_OPT_AEC)
        {
            BTMemCopy(&aec_com_rx, &g_prShareMem->rAecRxParam, sizeof(AEC_COM_RX_struct));
            BTMemCopy(&aec_com_tx, &g_prShareMem->rAecTxParam, sizeof(AEC_COM_TX_struct));
            u4Return = AECInit();
            SPHLOG_INFO((T("[ARM2SPEECH]AEC Init. Return(0x%x)\r\n"), u4Return));
        }

        if (u4Request & FRAME_OPT_NDC)
        {
            u4Return = NDCInit();
            SPHLOG_INFO((T("[ARM2SPEECH]NDCInit. Return(0x%x)\r\n"),  u4Return));
        }

        TimeStatInit();        
        SPHLOG_INFO((T("[ARM2SPEECH]g_prShareMem(0x%x) MaxFrame(%d) ReadIdx(%d) WriteIdx(%d)\r\n"), 
                    g_prShareMem, g_prShareMem->u4MaxFrame, 
                    g_prShareMem->u4ReadIdx, g_prShareMem->u4WriteIdx));
        _u4State = BT_STATE_SCO;
        SPHLOG_INFO((T("[ARM2SPEECH]Change state from IDLD to SCO.\r\n")));
    }
    else    // Leave SCO State. 
    {       
        if (_u4BTDataReq & FRAME_OPT_AEC) {
            AECDeinit();
        }
        if (_u4BTDataReq & FRAME_OPT_NDC) {
            NDCDeinit();
        }
        
        TimeStatUnInit();      
        _u4BTDataReq = 0;
        _u4State = BT_STATE_IDLE;
        BT_MemoryUninit();
        SPHLOG_INFO((T("[ARM2SPEECH]Change state from SCO to IDLE.\r\n")));
    }
    
    g_prShareMem->u4State = _u4State;
    AECSendMessage(BT_STATE_CHANGED, _u4State, 0, 0);

EXIT:
    return (u4Ret);
}


static UINT32 BTHandleMsg(UINT32 u4MsgID, UINT32 u4P1, UINT32 u4P2, UINT32 u4P3)
{
    UINT32 u4Ret = BT_SUCCESS;
    
    switch(u4MsgID & 0xFFFF)
    {
    case BT_SET_HW_RESOURCE:
        u4Ret = BTSetHWResource(u4P1, u4P2, u4P3);
        break;
        
    case BT_SET_PARAMETER:
        u4Ret = BTSetParameter(u4P1);
        break;
        
    case BT_SCO_AUDIO_CONTROL:
        u4Ret = BTEnterSCO(u4P1, u4P2);
        break;
        
    case BT_WRITE_FRAME:
        u4Ret = BTWriteFrame(u4P1);
        break;
        
    default:
        break;
    }
    
    return (u4Ret);
}


//==========================================//
  #define CodeSight_Speech_Interface
//==========================================//

UINT32 SpeechInit(VOID)
{
    _u4State = BT_STATE_INIT;
    return (0);
}


UINT32 SpeechStateMachine(VOID)
{
    UINT32 u4Ret = TASK_IDLE;
    
#ifdef BSP_ARM2
    UINT32 u4Temp = GetARM2TickCount();
    if (((u4Temp - _u4DbgMiniSecond) > 30000) && (BT_STATE_SCO ==  _u4State))
    {
        SPHLOG_INFO((T("[ARM2SPEECH]SpeechStateMachine (%d ms)\r\n"), u4Temp));
        _u4DbgMiniSecond = u4Temp;
    }
#endif

    if (_rDelayedMsg.fgHasDelayed)
    {
        BTHandleMsg(_rDelayedMsg.u4MsgId, _rDelayedMsg.u4P1, _rDelayedMsg.u4P2, _rDelayedMsg.u4P3);
        _rDelayedMsg.fgHasDelayed = FALSE;
        u4Ret = TASK_BUSY;
    }
    
    switch(_u4State)
    {
    case BT_STATE_SCO:
        if (IS_FRAME_FOR_AEC){
            SpeechFrameProcess();
        }
        u4Ret = TASK_BUSY;
        break;
        
    case BT_STATE_IDLE:
    case BT_STATE_INIT:
    case BT_STATE_UNINIT:
    default:
        break;
    }
    
    return (u4Ret);
}


UINT32 SpeechCB(UINT32 u4MsgID, UINT32 u4P1, UINT32 u4P2, UINT32 u4P3) 
{
    UINT32 u4Ret = BT_SUCCESS;
    SPHLOG_DETAIL((T("[ARM2SPEECH]SpeechCB: MsgID(0x%x) Params(0x%x 0x%x 0x%x)\r\n"), 
            u4MsgID, u4P1, u4P2, u4P3));
            
    if ((BT_SCO_AUDIO_CONTROL == (u4MsgID & 0xFFFF))) //68031  && (FALSE == u4P1))
    {        
        _rDelayedMsg.u4MsgId = u4MsgID;
        _rDelayedMsg.u4P1 = u4P1;
        _rDelayedMsg.u4P2 = u4P2;
        _rDelayedMsg.u4P3 = u4P3;
        _rDelayedMsg.fgHasDelayed = TRUE;
    }
    else
    {
        u4Ret = BTHandleMsg(u4MsgID, u4P1, u4P2, u4P3);
    }
    
    return (u4Ret);
}


