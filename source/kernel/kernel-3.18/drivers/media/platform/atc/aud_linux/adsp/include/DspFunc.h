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


 
#ifndef _H_DSP_FUNC_
#define _H_DSP_FUNC_

// IC Type Distinguishing Config
#include "aud_drv_config.h"
#include "chip_ver.h"
#include <media/atc/drv_aud.h>
#include <media/atc/drv_av_d.h>
#include "DspStruct.h"
#include "drv_dsp_cfg.h"
#include "aud_drv.h"
#include "aud_esm.h"
#include "GpsMix_mw.h"

/*
 * Dsp Code Internal use only
 * This files only contains DSP Internal use function
 */
#ifdef __cplusplus
extern "C"
{
#endif                          /* __cplusplus */


#define DOLBY_REROUTING_SUPPORT         1
#define DOWNMIX_BY_RISC                 1

typedef enum
{
    AUDSE_ats = 0,
	AUDSE_NEO6,
	AUDSE_pl2,
	AUDSE_csii,
	AUDSE_dh2,
	AUDSE_dvs2,	
	AUDSE_mvs	
}AUD_SE_DSP_TYPE;

#define BASSM_CUT_FREQ_MIN    0
#define BASSM_CUT_FREQ_MAX    300
#define IS_NORMAL_CUT_FREQ_VALUE(val)    (((val>=BASSM_CUT_FREQ_MIN)&&(val<=BASSM_CUT_FREQ_MAX)    \
                                         &&(val%5==0))? TRUE:FALSE)


/******************************************************************************
*    Extern function of DspInit.c
******************************************************************************/
#ifndef _DSP_INIT_C
    #define _DSP_INIT_C
    extern void vDspStateInit (void);
    extern void vDspCommCodeInit(void);

    extern void vDspInit (void);
    extern void vDspErrRecoverOnInit(u32 u4Cnt);
  #if CONFIG_AUD_POWER_MANAGEMENT_SUPPORT
    extern void vDspHibernationOnInit(void);
  #endif
    extern void vDspShareInfoInit (void);
    extern void vDspDramInit (void);
    extern void vDspPlayCommonInit (void);
    extern void vReEncCommonInit (void);
    extern void vDspMetadataInit (void);
    extern bool fgAudFeatureSupport(u32 u4Feature);
#endif


/******************************************************************************
*    Extern function of DspState.c
******************************************************************************/
#ifndef _DSP_STATE_C
#define _DSP_STATE_C
    extern void vDspPowerOn (void);
    extern void vDspState (void);

#endif



/******************************************************************************
*    Extern function of DspInt.c
******************************************************************************/
#ifndef _DSP_INT_C
#define _DSP_INT_C
    extern void vDspSetTable (u32 u4FreqIdx);
    extern void vDspIntSvc (u32 u4DspUopMsg);
    //extern u8 uDspGetDecodingChBitDescriptor(void);
#endif


/******************************************************************************
*    Extern function of DspUop.c
******************************************************************************/
#ifndef _DSP_UOP_C
#define _DSP_UOP_C
    extern void vDspUopSvc (u32 u4DspUop);
// For microphone use
#endif


/******************************************************************************
*    Extern function of DspIntf.c
******************************************************************************/
#ifndef _DSP_INTF_C
#define _DSP_INTF_C
    extern void vSendDspCmd(u32 u4Cmd);
    extern void vDspAdacFmt (u32 u4ClkFmt, bool fgFirstDecoder);
    extern bool fgIsDolbyDigitalKaraoke(void);
    // Aout2 relate Settings
    extern void vDspUpdateSpkCfg(AUD_CH_NUM_T  eAout2ChCnt,bool fgAoutSelect);
    extern void vDspUpdateAout2SamplingFreq(AUDIO_SAMPLING_T  eAout2SamplingRate);
    extern void vDspSamplingFreqTransformAvdToDsp(AUDIO_SAMPLING_T eAvdSamplingRate,AUD_SFREQ_IDX_T *eDspSampl,u32 *u4DspTableSampl);
    extern void vAudUpdateIecCfg(bool fgHbr4sd,AUD_IEC_CFG_T eAout2IecCfg,bool fgAoutSelect);
    extern void vAudSetChannelStatus(AUDIO_SAMPLING_T u1SamplingRate);
    extern void vAudUpdateChDelay(AUD_CH_DELAY_SETTING_T  rAout2ChDelay,bool fgAoutSelect);
    extern void vDspCodecBonding(void);
    extern bool fgMT3360AudBondingSupport(u32 u4FuncID); //add for efuse control
    extern void vDspAACBonding(void);
    extern void vDspSACDBonding(void);
    extern void vDspBManagementBonding(void);

    //AC3_OUTMODE, Orho
    extern s16 i2AC3SpkCfgTblToOutMode(u32 u4Spkcfg1, u32 u4Spkcfg2);
    extern s16 i2AC3SpkCfgTblToLFEMode(u32 u4Spkcfg1, u32 u4Spkcfg2);
#endif                          // _DSP_INTF_C


/******************************************************************************
*    Extern function of DspOp.c
******************************************************************************/
#ifndef _DSP_OP_C
#define _DSP_OP_C
    extern void vSetSpeakerConfig (bool fgFirstDecoder);
    extern void vDspSetFreq (u32 u4FreqIdx, bool fgResetDAC, bool fgFirstDecoder);
    extern bool fgDspChkSampleRateChange (void);
    extern bool fgDspChkFreqSetting (void);
    extern void vDspSetFreqDone (void);
    extern void vSetDRCInfo (void);
    extern bool fgGetIecSetting (u16 * u2IecFlag, bool fgFirstDecoder);
    extern void vGetAudCfgSetting (u32 u4FreqIdx, AUDIO_CFG_T * prAudCfg, bool fgFirstDecoder);
    extern void vDspCmd (u32 u4Cmd);
    extern void vDspMetadataToMixingPara(void);
#endif                          // _DSP_OP_C


/******************************************************************************
*    Extern function of DspIrq.c
******************************************************************************/
#ifndef _DSP_IRQ_C
#define _DSP_IRQ_C
    extern void vDspAIRQSvc (void);
    extern void vDspBIRQSvc (void);
    extern void vDspCIRQSvc (void);
#endif


/******************************************************************************
*    Extern function of DspCodecInit.c
******************************************************************************/
#ifndef _DSP_CODEC_INIT_C
#define _DSP_CODEC_INIT_C
    extern void vDecCommonInit (void);
    extern void vDecCommonInitDec2 (void);
    extern void vDecCommonInitDec4 (void);
    extern void vDecCommonInitDec5 (void);
#endif


/******************************************************************************
*    Extern function of DspCmd.c
******************************************************************************/
#ifndef _DSP_CMD_C
#define _DSP_CMD_C
    extern void vDspCmdInit (void);
    extern void vDspCmdUnInit (void);
    extern void vSendDspCmd(u32 u4Cmd);
    extern void vDspCmdDispatch(void);
    extern void vDspHCmdDispatch(void);
    extern void vSendDspISR(u8 u1DspRIntAddr, u32 u4DspRIntSD, u32 u4DspRIntLD, bool fgDspId);

#endif


/******************************************************************************
*    Extern function of DspMem.c
******************************************************************************/
#ifndef _DSP_MEM_C
#define _DSP_MEM_C
    extern void vDspMemInit(u8* puWorkingBuffer);
#endif

/******************************************************************************
*    Extern function of DspLoad.c
******************************************************************************/
#ifndef _DSP_LOAD_C
#define _DSP_LOAD_C_
extern void vDspLoadDecTable (u32 u4Type);
#endif

#ifdef DOLBY_REROUTING_SUPPORT
extern void vDolbyReroutingConfig(void);
extern void vSetDefDownmixCoef(bool fgHdmi, bool bForceDolbyDefaultCoef);
#endif

/******************************************************************************
*    Extern function of Asrc
******************************************************************************/
extern void DspSetDvdAsrcCtrl(AUDIO_SAMPLING_T eSmpRate);
extern u32 DspSetAsrcMode(u8 u1DecId, u32 u4FreqIdx);
extern u32 DspStartAsrcAutoTracMode(u8 u1DecId);
extern void DspStopAsrcAutoTracMode(u8 u1DecId);
extern u32 DspSetAsrcBypass(u8 u1DecId, bool fgVal);


extern s32  DspLoadAdspSeTable (u32 u4Type);
extern s32  DspLoadAdspCode(u8 u1DecId, AUD_DRV_FMT_T eStreamFormat);
extern void vAdspEnableASRC(u8 u1DecID,bool fgAsrcEnable);
extern s32 i4AudSetPlaySpeed(u8 u1DecId, AUD_DEC_PB_SPEED_TYPE_T tSpeed);
extern void vAdspDecMute(bool fgIsMute);
extern void DspCfgSetSpkLayout(AUD_DEC_SPEAKER_LAYOUT_T* prLayout);
extern void DspCfgSetVolumeGain(AUD_CH_T eChannel, u32 u4Value);
extern void DspCfgModBInfo(AUD_DEC_MODULE_BMANAGEMENT_CHANNEL_INFO_T* eInfo);
extern u8 DspCfgGetFrontMediaType(void);
extern u8 DspCfgGetRearMediaType(void);
extern void DspCfgSetFrontMediaType(u8 u1Type);
extern void DspCfgSetRearMediaType(u8 u1Type);
extern void DspCfgSetIecFlags(u8 u1Flag);
extern void DspCfgSetIecMute(u8 u1Mute);

extern void DspCfgSetFuncOption(AUD_FUNC_OPTION_T *prVal);
extern void DspCfgSetDrcLowRange(u8 u1Dec, u32 u4Val);
extern void DspCfgSetDrcHighRange(u8 u1Dec, u32 u4Val);
extern s32 DspCfgSetFeatureInfo(AUD_DEC_FEATURE_INFO_T* peInfo);

extern void DspCfgGetOutputVal(AUD_OUTPUT_VOL *prChVol);
extern bool DspCfgSetLrMix(AUD_DEC_LRMIX_OUTPUT_T eMode);
extern bool DspDecSetDec4Info(AUD_DEC4_INFO_T* prInfo);
extern bool DspDecSetDec5Info(u8 u1Dec, AUD_DEC_AUD_INFO_T* prInfo);
extern void DspCfgSetTestToneCh(u8 u1Seat, u32 u4ChSet);
extern void DspCfgSetTestToneFlag(u8 u1Seat, u32 u4Flag);
extern u8 DspCfgGetInputBitRate(u8 u1DecId);
extern u32 DspCfgInputSampRateDecimal(u8 u1DecId);



//set Dram function
extern void DspSetArm2FsReady(u8 u1Val);

extern void DspGetSpectrumInfo(AUD_DEC_SPECTRUM_INFO_T * prInfo);
extern void DspGetDspVersion(AUD_DEC_DSP_VERSION_T *prDspVer);
extern u32 DspGetDelayValue(void);
//pts value
extern void DspGetUpdatePtsValue(u8 u1DecId, u32* pu4High, u32* pu4Low);
extern void DspSetUpdatePtsValue(u8 u1DecId, u32 u4High, u32 u4Low);

extern void DspGetSpeakerConfig(u32* pu4Spk, u32* pu4Spk2);
extern void DspSetSpeaker_subOutCtrl(u32 u4Val);
extern void DspGetTestToneFlag(u32* pu4frnFlag, u32* pu4rearFlag);
extern void DspSetSpdifInFlag(u32 u4Val);
extern void DspGetSpdifInFlag(u32* pu4Val);



extern u32 DspGetCommCodeVersion(void);
extern u32 DspGetDecVersion(void);

extern void DspSetMiracast_skipDataFlag(u32 u4Val);
extern void DspSetEmphasisFlag(u32 u4Val);
extern void DspSetLpcmDrcValue(u32 u4Val);
extern void DspSetCgmsInfo(u32 u4Val);

extern void DspSetAOutMediaType(u8 u1Aout, u16 u2Type);
extern void DspGetAOutMediaType(u8 u1Aout, u16* u2Type);

extern void DspSetLineDmxRemapping(u8 u1Aout, u16 u2Type);

/******************************************************************************
*    codec set functions
******************************************************************************/
extern void DspCodecSet_WmaInfo(AUD_DEC_AUD_INFO_T  *prDecInfo);
extern void DspCodecSet_CookInfo(AUD_DEC_AUD_INFO_T  *prDecInfo);
extern void DspCodecSet_ApeInfo(AUD_DEC_AUD_INFO_T  *prDecInfo);
extern void DspCodecSet_FlacInfo(AUD_DEC_AUD_INFO_T  *prDecInfo);
extern void DspSetApeSeekInfo(u8 devId, APE_SEEKINFO_INFO_T * pApeSeekInfo);



/******************************************************************************
*    asrc set functions
******************************************************************************/
extern void DspGetAsrcSample(u8 u1DecId, u32* u4InSample, u32* u4OutSample);
extern void DspGetAsrcControl(u32* u4Ctrl);
extern void DspSetAsrcAoutStatusToDvp(u8 u1Val);
extern void DspGetAsrcIecFlag(u32* u4Flag);

/******************************************************************************
*    dsp configure set functions
******************************************************************************/
extern void DspSetChDelayFactor(u32 u4FreqIdx);
extern void DspSetBassManageTable(u32 u4FreqIdx);
extern void DspSetupDownMix(u32 u4FreqIdx, bool fgFirstDecoder);
extern void DspGetSourceConfig(AUD_SOURCE_CFG_T* prSrcParam);
extern void DspGetFourFiveDecConfig(u8 u1DecId, AUD_SOURCE_CFG_T* prSrcParam);
extern void DspCfgSetPlayMode(AUD_SOURCE_CFG_T* prSrcParam);
extern void DspResetSpectrumInfo(void);
extern void DspSetDvdSoftMute(void);
extern void DspCfgSetDvdPlayMode(void);
extern void DspGetInputChanCountFromDvd(void);
extern void DspGetSamplingRateFromDvd(AUDIO_SAMPLING_T eSmpRate);



/******************************************************************************
*    dsp debug functions
******************************************************************************/
extern void DspSetUnderRun(bool fgOn);
extern void vCliDbgWriteDspSram(u8 u1DspId,u32 u4Addr, u32 u4Value);


extern void DspGetPtsQueueInfo(AUD_PTS_QUEUE_INFO_T *prInfo);
extern void DspSetPtsNormalMode(AUD_PTS_QUEUE_INFO_T *prInfo, u32 u4PtsAddr, u32 u4PtsVal);
extern void DspSetFirstPtsValue(u32 u4PtsVal);
extern void DspSetPtsWptr(u32 u4PtsVal);


/******************************************************************************
*    share memory functions
******************************************************************************/
extern u32 g_u4DspShareMem;

#define uReadDspShmBYTE(addr) (*((volatile u8*)(g_u4DspShareMem+addr)))
#define u2ReadDspShmWORD(addr) (*((volatile u16*)(g_u4DspShareMem+addr)))
#define u4ReadDspShmDWRD(addr) (*((volatile u32*)(g_u4DspShareMem+addr)))

#define vWriteDspShmBYTE(addr,value) *((volatile u8*)(g_u4DspShareMem + addr))=(u8)(value)
#define vWriteDspShmWORD(addr,value) *((volatile u16*)(g_u4DspShareMem + addr))=(u16)(value)
#define vWriteDspShmDWRD(addr,value) *((volatile u32*)(g_u4DspShareMem + addr))=(u32)(value)

extern u8 uReadShmUINT8(u16 u2Addr);
extern u16 u2ReadShmUINT16(u16 u2Addr);
extern u32 u4ReadShmUINT32(u16 u2Addr);

extern void vWriteShmUINT16(u16 u2Addr,u16 u2Value);
extern void vWriteShmUINT32(u16 u2Addr,u32 u4Value);
extern void vWriteShmUINT8(u16 u2Addr,u32 uValue);

extern u32 dReadDspDram(u32 u4PageId,u32 addr);

extern void DspGetGpsMixConsumedData(u32* pu4Size);
extern void DspGetMicBufInfo(AUD_MIC_BUF_FOR_BT_INFO *prBufInfo, u32 u4wkPhyAdr,u32  u4PagePhyAdr);
extern void DspGetMicBufTotal(u32* u4ToSize);


extern void DspGetDec1StrType(u8* u1Type);
extern AUD_SOURCE_CFG_T* DspGetSrcParam(void);
extern AUD_OUTPUT_SETTING_CFG_T* DspGetOutParam(void);
extern AUD_OUTPUT_SETTING_CFG_T* DspGetOutHdmiParam(void);


//extern void DspGetDspStatus(u32** prData);
extern void DspGetInputChCfg(u32* pu4InCh);
extern void DspSetPrologicIIConfig(void);

extern void DspSetApSpdOut(AUD_SOURCE_CFG_T* prSrcCfg);
extern void DspSetDvdSpdOut(AUD_SOURCE_CFG_T* prSrcCfg, AUDIO_SAMPLING_T eSamp);
extern void vDecideSpdifOutput(AUD_SOURCE_CFG_T *prSrcParam, AUD_OUTPUT_SETTING_CFG_T *prOutParam);
extern AUDIO_BITSTREAM_TYPE_T DspGetDvdCodecFmt(void);
extern void DspSetForce2ChDownmix(bool fgDownmix, AUD_DSP_FORCE_2CH_DOWNMIX_T eSource);

extern void DspReSetDownmixDram(void);

extern void vDspASendInt(void);
extern void vDspBSendInt(void);
extern bool fgMakeVorbisCodebook(void);

extern u32 DspAwbGetSadr(void);
extern u32 DspAwbGetChSize(void);
extern u32 DspAwbGetWptr(void);

/******************************************************************************
*    Extern function of Others
******************************************************************************/

#ifdef __cplusplus
}
#endif                          /* __cplusplus */

#endif
