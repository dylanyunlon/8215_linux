/*
* Copyright (c) 2016 AutoChips Inc.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License version 2 as
* published by the Free Software Foundation.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
*/

#include "x_lint.h"

//#include "x_typedef.h"
#include <linux/types.h>
#include "x_assert.h"
#include "aud_drv.h"

#include "DspDrvInc.h"
#include "DspFunc.h"

#include "aud_raw.h"
#include "aud_debug.h"
#include "aud_if.h"

/****************************************************************************
** Local definitions
****************************************************************************/
#define SRAM_DEC1_RP          (0x20b)
#define SRAM_DEC1_WP          (0x250)
#define SRAM_DEC2_RP          (0x20e)
#define SRAM_DEC2_WP          (0x251)

/****************************************************************************
** Local structures and enumerations
****************************************************************************/

#define DATASZ(wp, rp, fifosize) \
        (((wp) >= (rp)) ? ((wp) - (rp)) : (((fifosize) + (wp)) - (rp)))

/****************************************************************************
** Function prototypes
****************************************************************************/
extern void AudDrvGetDecStatus(u8 u1DecId, AUD_DECODER_T * prAudDec);
extern u32 u4CliDbgReadDspSram(u8 u1DspId, u32 u4Addr);

/****************************************************************************
** Local variable
****************************************************************************/

static s8 * _paszDecState[] =
{
    _T("AUD_DEC_STOP"),
    _T("AUD_DEC_PLAYING"),
    _T("AUD_DEC_INIT"),    
    _T("AUD_DEC_PAUSING"),
    _T("AUD_DEC_PAUSED"),
    _T("AUD_DEC_RESUMING"),
};



static s8 * _paszDecFormat[] =
{
    _T("AUD_DEC_FMT_UNKNOWN"),
    _T("AUD_DEC_FMT_MPEG"),
    _T("AUD_DEC_FMT_AC3"),
    _T("AUD_DEC_FMT_PCM"),
    _T("AUD_DEC_FMT_MP3"),
    _T("AUD_DEC_FMT_AAC"),
    _T("AUD_DEC_FMT_DTS"),
    _T("AUD_DEC_FMT_WMA"),
    _T("AUD_DEC_FMT_RA"),
    _T("AUD_DEC_FMT_HDCD"),
    _T("AUD_DEC_FMT_MLP")
};




void AudShowDspStatus(void)
{
    u32 u4Data[23] = {0};
    //DspGetDspStatus(&u4Data);
 
    LOG(LOG_FEATURE, TEXT("Aud DSPA PC = 0x%08x\n"),u4Data[0]);
    LOG(LOG_FEATURE, TEXT("Aud DSPB PC = 0x%08x\n"),u4Data[1]);
    LOG(LOG_FEATURE, TEXT("Aud DSPC PC = 0x%08x\n"),u4Data[2]);

    LOG(LOG_FEATURE, TEXT("=============== Primary Sram variable ====================\n"));
    LOG(LOG_FEATURE, TEXT("Aud CONNECT_CONTROL(0x%08x) = 0x%08x\n"),u4Data[3], u4Data[4]);
    LOG(LOG_FEATURE, TEXT("Aud CONNECT_STATUS(0x%08x) = 0x%08x\n"), u4Data[5],u4Data[6]);
    LOG(LOG_FEATURE, TEXT("Aud AOUT_CONTROL(0x%08x) = 0x%08x\n"),u4Data[7], u4Data[8]);
    LOG(LOG_FEATURE, TEXT("Aud AOUT_STATUS (0x%08x)= 0x%08x\n"),u4Data[9], u4Data[10]);
    LOG(LOG_FEATURE, TEXT("Aud APUT_BANK(0x%08x) = 0x%08x\n"), u4Data[11],u4Data[12]);
    LOG(LOG_FEATURE, TEXT("Aud AOUT_BANK (0x%08x)= 0x%08x\n"),u4Data[13], u4Data[14]);
    LOG(LOG_FEATURE, TEXT("Aud DECODING_APUT_BANK(0x%08x) = 0x%08x\n"),u4Data[15], u4Data[16]);
    LOG(LOG_FEATURE, TEXT("Aud DECODING_AOUT_BANK(0x%08x) = 0x%08x\n"),u4Data[17], u4Data[18]);
    LOG(LOG_FEATURE, TEXT("Aud B_STATE_DECODER(0x%08x) = 0x%08x\n"),u4Data[19], u4Data[20]);
    LOG(LOG_FEATURE, TEXT("Aud B_DECODER_CONTROL(0x%08x) = 0x%08x\n"),u4Data[21], u4Data[22]);
}

void AudShowConfig(void)
{
    AUD_SOURCE_CFG_T* prParam = DspGetSrcParam();
    AUD_OUTPUT_SETTING_CFG_T* prOutParam = DspGetOutParam();
    AUD_OUTPUT_SETTING_CFG_T* prHdmiOutParam = DspGetOutHdmiParam();
    LOG(LOG_FEATURE, TEXT("===DspSrcParam=== \n"));
    LOG(LOG_FEATURE, TEXT("SrcParam.u1Aud_DecId: 0x%08x\n"),prParam->u1Aud_DecId);
    LOG(LOG_FEATURE, TEXT("SrcParam.u1Aud_Sampling_Rate: 0x%08x\n"),prParam->u1Aud_Sampling_Rate);
    LOG(LOG_FEATURE, TEXT("SrcParam.u1Aud_Iec_Frame_Rate: 0x%08x\n"),prParam->u1Aud_Iec_Frame_Rate);
    LOG(LOG_FEATURE, TEXT("SrcParam.u1Aud_Codec_Fmt: 0x%08x\n"),prParam->u1Aud_Codec_Fmt);
    LOG(LOG_FEATURE, TEXT("SrcParam.u1Aud_Input_Chan_Cnt: 0x%08x\n"),prParam->u1Aud_Input_Chan_Cnt);
    LOG(LOG_FEATURE, TEXT("SrcParam.u1Aud_Reencode_Fmt: 0x%08x\n"),prParam->u1Aud_Reencode_Fmt);
    LOG(LOG_FEATURE, TEXT("SrcParam.rAud_Ui_Setting.eAud_Iec_Ui_Select: 0x%08x\n"),prParam->rAud_Ui_Setting.eAud_Iec_Ui_Select);
    LOG(LOG_FEATURE, TEXT("SrcParam.rAud_Ui_Setting.eAud_Iec_Max_Sampling_Rate: 0x%08x\n"),prParam->rAud_Ui_Setting.eAud_Iec_Max_Sampling_Rate);
    LOG(LOG_FEATURE, TEXT("SrcParam.rAud_Ui_Setting.eAud_Hdmi_Ui_Select: 0x%08x\n"),prParam->rAud_Ui_Setting.eAud_Hdmi_Ui_Select);
    LOG(LOG_FEATURE, TEXT("SrcParam.rAud_Ui_Setting.u4Aud_Speaker_Config: 0x%08x\n"),prParam->rAud_Ui_Setting.u4Aud_Speaker_Config);
    LOG(LOG_FEATURE, TEXT("SrcParam.rAud_Ui_Setting.eAud_IEC_Chan_Select: 0x%08x\n"),prParam->rAud_Ui_Setting.eAud_IEC_Chan_Select);
    LOG(LOG_FEATURE, TEXT("SrcParam.fgReEncStatus: 0x%08x\n"),prParam->fgReEncStatus);
    LOG(LOG_FEATURE, TEXT("SrcParam.u1Hdmi_Max_Channel: 0x%08x\n"),prParam->u1Hdmi_Max_Channel);
    LOG(LOG_FEATURE, TEXT("SrcParam.fgHdmiEnableHdAudioOutput: %s\n"),prParam->fgHdmiEnableHdAudioOutput?"HD":"SD");
	
    LOG(LOG_FEATURE, TEXT("== DspOutputParam=== \n"));
    LOG(LOG_FEATURE, TEXT("DspOutputParam.u1Sampling_Rate: 0x%08x\n"),prOutParam->u1Sampling_Rate);
    LOG(LOG_FEATURE, TEXT("DspOutputParam.eIec_Cfg: 0x%08x\n"),prOutParam->eIec_Cfg);
    LOG(LOG_FEATURE, TEXT("DspOutputParam.u1Aud_Output_Chan_Cnt: 0x%08x\n"),prOutParam->u1Aud_Output_Chan_Cnt);
    LOG(LOG_FEATURE, TEXT("DspOutputParam.u1Aud_Dec_Fmt: 0x%08x\n"),prOutParam->u1Aud_Dec_Fmt);
		
    LOG(LOG_FEATURE, TEXT("===DspOutputHdmiParam=== \n"));
    LOG(LOG_FEATURE, TEXT("DspOutputHdmiParam.u1Sampling_Rate: 0x%08x\n"),prHdmiOutParam->u1Sampling_Rate);
    LOG(LOG_FEATURE, TEXT("DspOutputHdmiParam.eIec_Cfg: 0x%08x\n"),prHdmiOutParam->eIec_Cfg);
    LOG(LOG_FEATURE, TEXT("DspOutputHdmiParam.u1Aud_Output_Chan_Cnt: 0x%08x\n"),prHdmiOutParam->u1Aud_Output_Chan_Cnt);
    LOG(LOG_FEATURE, TEXT("DspOutputHdmiParam.u1Aud_Dec_Fmt: 0x%08x\n"),prHdmiOutParam->u1Aud_Dec_Fmt);
    LOG(LOG_FEATURE, TEXT("DspOutputHdmiParam.u1HdmiMaxSupCh: 0x%08x\n"),prHdmiOutParam->u1HdmiMaxSupCh);	  //HDMI_DMX_RMP_COEFF_BY_DEC
    LOG(LOG_FEATURE, TEXT("DspOutputHdmiParam.u1HdmiSpeakAllocat: 0x%08x\n"),prHdmiOutParam->u1HdmiSpeakAllocat); //HDMI_DMX_RMP_COEFF_BY_DEC
    LOG(LOG_FEATURE, TEXT("DspOutputHdmiParam.fgHDMIConnectStatus: 0x%08x\n"),prHdmiOutParam->fgHDMIConnectStatus);//HDMI_DMX_RMP_COEFF_BY_DEC
}

void AudShowStatus(u8 u1DecId)
{
    AUD_DECODER_T arAudDecoder[2];
    u32 u4RpDec1, u4WpDec1;
    u32 u4DataSZDec1;
    u32 u4DecFrameCount = LOG_FEATURE;
    u32 u4DecErrorCount = 0;

    VERIFY(u1DecId == PRI_DEC);

    // Get read/write pointer
    AUD_GetRWPtr(u1DecId, &u4RpDec1, &u4WpDec1, &u4DataSZDec1);

    AudDrvGetDecStatus(u1DecId, &arAudDecoder[u1DecId]);
    LOG(1, TEXT("Aud(%d) status = %s \n"),
        u1DecId, _paszDecState[arAudDecoder[u1DecId].eDecState]);
    LOG(1, TEXT("Aud(%d) format = %d \n"),
        u1DecId, arAudDecoder[u1DecId].eDecFormat);
    LOG(1, TEXT("Aud(%d) rp,wp,size = [0x%08x, 0x%08x, %u]\n"),
        u1DecId,u4RpDec1,u4WpDec1,u4DataSZDec1);
    LOG(1, TEXT("Aud(%d) frame count, error count = [%u, %u]\n"),
        u1DecId,u4DecFrameCount,u4DecErrorCount);

    // Fix -O2 warning message
    UNUSED(u4DecFrameCount);
    UNUSED(u4DecErrorCount);
    UNUSED(_paszDecState);
    UNUSED(_paszDecFormat);
}

extern u8 _u1AudDspState;
extern u8 _u1DspAState;
extern u8 _u1DspAoutState;
extern u8 _u1DspAout2State;
extern u8 _u1DspBDec1State;
extern u8 _u1DspReencState;

extern u32 g_u4DspAIntIdx;
extern u32 g_u4DspBIntIdx;
extern TDspUopInt g_tDspAIntHist[MAX_DSP_CMD_NS];
extern TDspUopInt g_tDspBIntHist[MAX_DSP_CMD_NS];


s8 _arAudCmdDspDrvState[9][64] =
{
    {_T("POWER-OFF")},
    {_T("INIT")},
    {_T("READY")},
    {_T("PLAYING")},
    {_T("PLAY OK")},
    {_T("STOPPING")},
    {_T("PAUSING")},
    {_T("PAUSED")},
    {_T("RESUMING")},
};


s8 _arAudCmdDspAState[11][64] =
{
    {_T("POWER-OFF")},
    {_T("INIT")},
    {_T("AOUT OFF")},
    {_T("AOUT STARTING")},
    {_T("AOUT ON")},
    {_T("AOUT STOPPING")},
    {_T("DISCONNECTED")},
    {_T("CONNECTING")},
    {_T("CONNECTED")},
    {_T("DISCONNECTING")},
    {_T("STEPPING")},
};

s8 _arAudCmdDspBState[9][64] =
{
    {_T("POWER-OFF")},
    {_T("INIT")},
    {_T("READY")},
    {_T("PARSING")},
    {_T("WAIT CFG ACK")},
    {_T("DECODER INIT")},
    {_T("DECODING")},
    {_T("EOS")},
    {_T("STOPPING")},
};

s8 _arAudCmdReencState[6][64] =
{
    {_T("POWER-OFF")},
    {_T("INIT")},
    {_T("STOP")},
    {_T("STARTING")},
    {_T("START")},
    {_T("STOPPING")},
};


void AudDispStates(void)
{
    LOG(LOG_DECINFO, TEXT("[AUD] Aud DRV state = %s \n"), _arAudCmdDspDrvState[_u1AudDspState]);
    LOG(LOG_DECINFO, TEXT("[AUD] Aud DSP A state = %s \n"), _arAudCmdDspAState[_u1DspAState]);
    LOG(LOG_DECINFO, TEXT("[AUD] Aud DSP A Out state = %s \n"), _arAudCmdDspAState[_u1DspAoutState]);
    LOG(LOG_DECINFO, TEXT("[AUD] Aud DSP A Out2 state = %s \n"), _arAudCmdDspAState[_u1DspAout2State]);
    LOG(LOG_DECINFO, TEXT("[AUD] Aud DSP B Primary = %s \n"), _arAudCmdDspBState[_u1DspBDec1State]);
    LOG(LOG_DECINFO, TEXT("[AUD] Aud DSP Encoder = %s \n"), _arAudCmdReencState[_u1DspReencState]);
}

extern TDspCmd _tDspCmd;
extern TDspCmd _tDspCmdH;

void AudDispUopHistory(void)
{
    u32 i=0;

    LOG(1, TEXT("[AUD] ========= RISC UOP list:=========\n"));
    LOG(1, TEXT("[AUD] read index = %d, write index = %d\n"), _tDspCmd.bRdIdx, _tDspCmd.bWrIdx);
    for( i=0; i< MAX_DSP_CMD_NS; i++)
    {
        LOG(1, TEXT("[AUD] [%d] UOP = 0x%X\n"), i, _tDspCmd.pu4Cmd[i]);
    }

}

void AudDispRC2DIntHistory(void)
{
    u32 i=0;

    LOG(1, TEXT("[AUD] ============= RISC to DSP INT ==============\n"));
    LOG(1, TEXT("[AUD] =========Send to DSP A UOP history list:=========\n"));
    LOG(1, TEXT("[AUD] Current index = %d\n"), g_u4DspAIntIdx-1);
    for( i=0; i< MAX_DSP_CMD_NS; i++)
    {
        LOG(1, TEXT("[AUD] Idx=%d, DspId=%d, IntAddr=0x%X, IntSD=0x%X, IntLD=0x%X\n"), i, g_tDspAIntHist[i].fgDspId,
            g_tDspAIntHist[i].u4DspIntAddr, g_tDspAIntHist[i].u4DspRIntSD, g_tDspAIntHist[i].u4DspRIntLD);
    }

    LOG(1, TEXT("[AUD] =========Send to DSP B UOP history list:=========\n"));
    LOG(1, TEXT("[AUD] Current index = %d\n"), g_u4DspBIntIdx-1);
    for( i=0; i< MAX_DSP_CMD_NS; i++)
    {
        LOG(1, TEXT("[AUD] Idx=%d, DspId=%d, IntAddr=0x%X, IntSD=0x%X, IntLD=0x%X\n"), i, g_tDspBIntHist[i].fgDspId,
            g_tDspBIntHist[i].u4DspIntAddr, g_tDspBIntHist[i].u4DspRIntSD, g_tDspBIntHist[i].u4DspRIntLD);
    }
}

void AudDispIntHistory(void)
{
    u32 i=0;

    LOG(1, TEXT("[AUD] ============= DSP to RISC INT ==============\n"));

    LOG(1, TEXT("[AUD] read index = %d, write index = %d, CmdNum = %d\n"),
         _tDspCmdH.bRdIdx, _tDspCmdH.bWrIdx, _tDspCmdH.bCmdNs);
    for( i=0; i<MAX_DSP_CMD_NS; i++)
    {
        LOG(1, TEXT("[AUD] Idx=%d, pu4Cmd=0x%X, DspId=%d, IntSD=0x%X, IntLD=0x%X\n"),
              i, _tDspCmdH.pu4Cmd[i], _tDspCmdH.prCmd[i].fgDspId,
              _tDspCmdH.prCmd[i].u4DspRIntSD, _tDspCmdH.prCmd[i].u4DspRIntLD);
    }

    AudDispRC2DIntHistory();
}


typedef struct _IEC_REGISTER
{
    u16 u2RegIndex;
    s8   *pszName;
} IEC_REGISTER_T;

static const IEC_REGISTER_T _arIECRegisters[] = {
    {0x2A0, "REG_IEC_BURST_INFO"},
    {0x2A1, "REG_IEC_INTR_SIZE"},
    {0x2A2, "REG_IEC_NSNUM"},
    {0x2A3, "REG_IEC_NEXT_LENGTH"},
    {0x2A4, "REG_IEC_NSADR"},
    {0x2A5, "REG_IEC_NEXT_UADR"},
    {0x2A6, "REG_IEC_CTRL"},
    {0x2A7, "REG_IEC_CHANNEL_CFG"},
    {0x2A8, "REG_IEC_BS_SBLK"},
    {0x2A9, "REG_IEC_BS_EBLK"},
    {0x2AA, "REG_IEC_CHL_STATUS0"},
    {0x2AB, "REG_IEC_CHL_STATUS1"},
    {0x2AC, "REG_IEC_CHL_STATUS2"},
    {0x2AD, "REG_IEC_CHR_STATUS0"},
    {0x2AE, "REG_IEC_CHR_STATUS1"},
    {0x2AF, "REG_IEC_CHR_STATUS2"},
    {0x2B0, "REG_IEC2_BURST_INFO"},
    {0x2B1, "REG_IEC2_INTR_SIZE"},
    {0x2B2, "REG_IEC2_NSNUM"},
    {0x2B3, "REG_IEC2_NEXT_LENGTH"},
    {0x2B4, "REG_IEC2_NSADR"},
    {0x2B5, "REG_IEC2_NEXT_UADR"},
    {0x2B6, "REG_IEC2_CTRL"},
    {0x2B7, "REG_IEC2_CHANNEL_CFG"},
    {0x2B8, "REG_IEC2_BS_SBLK"},
    {0x2B9, "REG_IEC2_BS_EBLK"},
    {0x2BA, "REG_IEC2_CHL_STATUS0"},
    {0x2BB, "REG_IEC2_CHL_STATUS1"},
    {0x2BC, "REG_IEC2_CHL_STATUS2"},
    {0x2BD, "REG_IEC2_CHR_STATUS0"},
    {0x2BE, "REG_IEC2_CHR_STATUS1"},
    {0x2BF, "REG_IEC2_CHR_STATUS2"}
 };

void AudDispIECRegisters(void)
{
    u16 u2Index = 0;
    u32 u4RegValue = 0;

    LOG(1, TEXT("[AUD] ========== Digital Output Registers ===========\n"));

    for (u2Index = 0; u2Index < sizeof(_arIECRegisters)/sizeof(IEC_REGISTER_T); u2Index++)
    {
        u4RegValue = u4CliDbgReadDspSram(DSPA_ID, _arIECRegisters[u2Index].u2RegIndex);
        LOG(1, TEXT("[AUD] 0x%03X = 0x%06X %s\n"), _arIECRegisters[u2Index].u2RegIndex, u4RegValue, _arIECRegisters[u2Index].pszName);
    }
}


void AUD_GetRWPtr(u8 u1DecId, u32 * pu4Rp, u32 * pu4Wp, u32 *pu4Size)
{
    u32 u4FifoSADec1 = 0;
    u32 u4FifoEADec1 = 0;
    u32 u4FifoSADec2 = 0;
    u32 u4FifoEADec2 = 0;
    u32 u4DataSZDec1 = 0;
    u32 u4DataSZDec2 = 0;
    u32 u4FifoSZDec1 = 0;
    u32 u4FifoSZDec2 = 0;

    VERIFY(u1DecId == PRI_DEC);
    // Get read/write pointer
    if (u1DecId == PRI_DEC)
    {
        if ((pu4Rp != NULL) && (pu4Wp != NULL))
        {
            *pu4Rp = u4CliDbgReadDspSram(DSPA_ID,SRAM_DEC1_RP);
            *pu4Wp = u4CliDbgReadDspSram(DSPA_ID,SRAM_DEC1_WP);
        }
    }

    // Get Fifo address and fifo size
    VERIFY(AUD_GetAudFifo(&u4FifoSADec1, &u4FifoEADec1, &u4FifoSADec2, &u4FifoEADec2) == AUD_OK);
    VERIFY(u4FifoEADec1 > u4FifoSADec1);
    VERIFY(u4FifoEADec2 > u4FifoSADec2);
    u4FifoSZDec1 = u4FifoEADec1 - u4FifoSADec1;
    u4FifoSZDec2 = u4FifoEADec2 - u4FifoSADec2;
    if ((pu4Rp != NULL) && (pu4Wp != NULL))
    {
        u4DataSZDec1 = DATASZ(*pu4Wp, *pu4Rp, u4FifoSZDec1);
        u4DataSZDec2 = DATASZ(*pu4Wp, *pu4Rp, u4FifoSZDec2);
    }

    if (pu4Size != NULL)
    {
        *pu4Size = (u1DecId == PRI_DEC) ? u4DataSZDec1 : u4DataSZDec2;
    }
}


void AudGetSpectrumInfo(AUD_DEC_SPECTRUM_INFO_T * ptAudSpectrumInfo)
{
    DspGetSpectrumInfo(ptAudSpectrumInfo);
}

void AUD_GetDspVersionNumber(AUD_DEC_DSP_VERSION_T *prDspVer)
{
    DspGetDspVersion(prDspVer);
}

void AUD_GetPbInfo(u8 u1DecId, PBINF_A *ptAudPbInfo)
{
       
    ptAudPbInfo->u8DspPlayBackTime = u4DspGetPlaybackTime();

}


