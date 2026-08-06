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

/***************************************************************************/
/**************************  Header File Include     ******************/
/***************************************************************************/
#include "aud_oal.h"
#include "x_assert.h"
#include "aud_debug.h"
#include "audin_if.h"
#include "aud_drv.h"
#include "aud_esm.h"
#include "aud_if.h"
#include "DspShm.h"
#include "DspFunc.h"

#include "aud_linein_hal_if.h"
#include "aud_bypass_hal_if.h"

typedef enum
{
    LinIn1Path = 0,
    LinIn2Path
}LinInPath;

PLIN_HAL_CLS_PUB  g_prLinIn1    = NULL;
LIN_EXTPARAMS_T   g_rLinIn1Params = {0};
PLIN_HAL_CLS_PUB  g_prLinIn2    = NULL;
LIN_EXTPARAMS_T   g_rLinIn2Params = {0};
PBYPS_HAL_CLS_PUB g_prLinInByps = NULL;
BYPS_EXTPARAMS_T  g_rByPassCfg = {0};

u32 g_u4ByPasGain = 0x7FFFFF;

extern u8 g_u1DecOutPath[];
/************************************************************************
Function    : static s32 Aud_Linein_GetWPtr()
Description : Get Line in Wptr
Parameter   : void
Return      : Wptr
Date        :  2011.6.27
************************************************************************/
u32 Aud_Linein_GetWPtr(void)
{   
    VERIFY(NULL != g_prLinIn1);
    return (g_prLinIn1->rHwIf.GetPoint(g_prLinIn1));
}

/************************************************************************
Function    : static s32 Aud_Linein2_GetWPtr()
Description : Get Line in Wptr
Parameter   : void
Return      : Wptr
Date        :  2011.6.27
************************************************************************/
u32 Aud_Linein2_GetWPtr(void)
{    
    VERIFY(NULL != g_prLinIn2);
    return (g_prLinIn2->rHwIf.GetPoint(g_prLinIn2));
}

static s32 Aud_Linein_SetByPasVol(u32 u4Vol)
{
    LOG(LOG_FEATURE, TEXT("u4Vol = 0x%x\n"),u4Vol);
    if (g_rByPassCfg.u4Gain != u4Vol)
    {
        g_rByPassCfg.u4Gain = u4Vol;
        
        if (g_prLinInByps == NULL)
        {
            LOG(LOG_FAIL, TEXT("g_prLinInByps is NULL.\n"));
            return (AUD_FAIL);
        }
        if (g_prLinInByps->rHwIf.CfgUpd == NULL)
        {
            LOG(LOG_FAIL, TEXT("g_prLinInByps->rHwIf.CfgUpd is NULL.\n"));
            return (AUD_FAIL);
        }

        g_prLinInByps->rHwIf.CfgUpd(g_prLinInByps, &g_rByPassCfg);
    }
    return (AUD_OK);
}

void Aud_Linein_GetDecInfo(AUDIN_INFO *pAudInfo)
{
    pAudInfo->e_audin_fmt = AUD_DEC_FMT_PCM;
    pAudInfo->e_audin_type = AUD_DEC_TYPE_STEREO;
    pAudInfo->ui1_bit_depth = 24;
    pAudInfo->ui2_pid = 0;
    pAudInfo->ui4_data_rate= 1536000;  //48000*2*16
    pAudInfo->ui4_sample_rate = 48000;
    pAudInfo->e_linendian = AUD_DEC_LIN_SAMLL_ENDIAN;

    pAudInfo->pcm_info.ePCM_Format = AUD_DEC_PCM_FMT_WAVE;
    pAudInfo->pcm_info.b_dlna_exist = FALSE;
    pAudInfo->pcm_info.u2BlockAlign= 4;
    pAudInfo->pcm_info.b_de_emphasis= FALSE;
}

void Aud_Linein_GetDataLen(AUDIN_DEC_DATA_LEN * pAudDecLinDataLen)
{
    pAudDecLinDataLen->u4LinDataLen = Aud_Linein_GetWPtr();
}

bool Aud_Linein_Init(void)
{
    AUD_LIN_DEVID eLineID = AUDID_LIN1;
    if (!g_prLinIn1)
    {
        g_prLinIn1 = LinHal_New(eLineID);
    }

    if (!g_prLinIn2)
    {
        eLineID = AUDID_LIN2;
        g_prLinIn2 = LinHal_New(eLineID);
    }

    if (!g_prLinInByps)
    {
        g_prLinInByps = BypsHal_New();
        g_rByPassCfg.u4Gain = g_u4ByPasGain;
    }

    return (TRUE);
}

bool Aud_Linein_DeInit(void)
{
    if (NULL!= g_prLinIn1)
    {
        g_prLinIn1->Delete(g_prLinIn1);
        g_prLinIn1 = NULL;
    }

    if(NULL!= g_prLinIn2)
    {
       g_prLinIn2->Delete(g_prLinIn2);
       g_prLinIn2 = NULL;
    }

    if(NULL!= g_prLinInByps)
    {    
        g_prLinInByps->Delete(g_prLinInByps);
        g_prLinInByps = NULL;
        
        g_u4ByPasGain = g_rByPassCfg.u4Gain;
    }

    return (TRUE);
}

static bool Aud_Linein_DspPath(LinInPath ePath, AUD_ADC_INPUT_SRC eLinGroup)
{
    PLIN_EXTPARAMS_T prExtCfg = NULL;    
    PLIN_HAL_CLS_PUB prLinCls = NULL;
    u8 u1LininOut = AUD_AOUT1;
    // Linein1 and Linein2 different    
    AUD_LIN_DEVID eLineID = AUDID_LIN1;

    uintptr_t u4Start = 0;
    u32 u4Size  = AUD_AFIFO_LINEIN_SIZE;

    if(LinIn1Path == ePath)
    {
        eLineID = AUDID_LIN1;        
        u4Start = AUD_AFIFO_LINEIN1_START;
        if(NULL == g_prLinIn1)
        {
            g_prLinIn1 = LinHal_New(eLineID);
        }
        prExtCfg = &g_rLinIn1Params;
        prLinCls = g_prLinIn1;
        u1LininOut = g_u1DecOutPath[SEC_DEC];
    }
    else if(LinIn2Path == ePath)
    {
        eLineID   = AUDID_LIN2;
        u4Start   = AUD_AFIFO_LINEIN2_START;
        if(NULL == g_prLinIn2)
        {
            g_prLinIn2 = LinHal_New(eLineID);
        }
        prExtCfg = &g_rLinIn2Params;        
        prLinCls = g_prLinIn2;
        u1LininOut = g_u1DecOutPath[TER_DEC];
    }

    if(AUD_STATE_STARTED == prLinCls->rHwIf.GetStatus(prLinCls))
    {
        LOG(LOG_FAIL, _T("Linein already started.\r\n"));
        return (FALSE);
    }  

    if(AUD_AOUT1 == u1LininOut)
    {
        prExtCfg->eIntClkSrc = LIN_CLK_AOUT1;
    }
    else if(AUD_AOUT2 == u1LininOut)
    {
        prExtCfg->eIntClkSrc = LIN_CLK_AOUT2;
    }
    else
    {
        LOG(LOG_FAIL, _T("Dsp Linein Clock = 0x%x, Error.\r\n"), u1LininOut);
    }
    
    LOG(LOG_FEATURE, T("[Audin]Internal LineIn.\r\n"));
    prExtCfg->eSrc = INT_LINEIN;
    prExtCfg->eGroup = eLinGroup;

    prExtCfg->u4BufPhyAdr = AFIFO_PHYSICAL(u4Start);
    prExtCfg->u4BufSz = u4Size;

    LOG(LOG_FEATURE, _T("Linein  adr: 0x%lx"), u4Start);

    prExtCfg->rIntCfg.fgOn = FALSE;
    prExtCfg->rIntCfg.eIntPeriod = LIN_INT_OFF;
    prExtCfg->rIntCfg.PFN_ISR_CB = NULL;    

    prLinCls->rHwIf.Setup(prLinCls, prExtCfg);    

    prLinCls->rHwIf.Start(prLinCls, 0);

    return (TRUE);
}


static bool Aud_Linein_StartByPass(AUD_ADC_INPUT_SRC eLinGroup, AUD_AOUT_DEVID eAoutId)
{
    PBYPS_HAL_CLS_PUB prBy = NULL;
    bool fgRet = TRUE;

    if (NULL == g_prLinInByps)
    {
        g_prLinInByps = BypsHal_New();
    }

    prBy = g_prLinInByps;
    if(AUD_STATE_STARTED != prBy->rHwIf.GetStatus(prBy))
    { 
        AudAout_ByPassMode(eAoutId, TRUE);
        g_rByPassCfg.eDst = eAoutId;
        g_rByPassCfg.eGainMode = BYPS_GAIN_LINER;
        g_rByPassCfg.eGroup = eLinGroup;
        g_rByPassCfg.u4Scale = 1;

        prBy->rHwIf.Setup(prBy, &g_rByPassCfg);
        prBy->rHwIf.Start(prBy, 0);
    }
    else
    {
        LOG(LOG_FAIL, _T("Lin Bypass already started.\r\n"));
        fgRet = FALSE;
    }
    return (fgRet);
}

static bool Aud_Linein_StopByPass(AUD_ADC_INPUT_SRC eLinGroup, AUD_AOUT_DEVID eAoutId)
{ 
    PBYPS_HAL_CLS_PUB prByps = g_prLinInByps;
    bool fgRet = TRUE;
    
    //close rear line in path
    if ((NULL != prByps)&&
        (AUD_STATE_STARTED==prByps->rHwIf.GetStatus(prByps)))
    {
        prByps->rHwIf.Stop(prByps, 0);
        g_rByPassCfg.eGroup = ADC_SRC_NON;
        AudAout_ByPassMode(eAoutId, FALSE);
    }
    else
    {
        LOG(LOG_FAIL, _T("Lin Bypass already stopped.\r\n"));
        fgRet = FALSE;
    } 
    
    return (fgRet);
}

static bool Aud_Linein_Start(LIN_MODE lMode, AUD_ADC_INPUT_SRC eLinGroup, u8 u1DecID)
{	
    LOG(LOG_CTRLF, _T("Audin lMode is 0x%x, Group is 0x%x.\r\n"), lMode, eLinGroup);

    switch(lMode)
    {
    case LIN_DRAM_FRONT:		
    case LIN_DRAM_REAR:
    {
        //audin dsp path
        LinInPath ePath = LinIn1Path;
        u8 u1Event 	= LINEIN1;
        if(LIN_DRAM_REAR == lMode)
        {
            ePath = LinIn2Path;
            u1Event =  LINEIN2;
        }
        
        if(TRUE == Aud_Linein_DspPath(ePath, eLinGroup))
        {
           i4AudEsm_SetAuEvent(u1Event);
        }
    }		
    break;
    //audin bypass path
    case LIN_BYPASS_FRONT:		
    case LIN_BYPASS_REAR:
    {
        AUD_AOUT_DEVID eOutId = AUDID_AOUT1;
        if(LIN_BYPASS_REAR == lMode)
        {
            eOutId = AUDID_AOUT2;
        }
        Aud_Linein_StartByPass(eLinGroup, eOutId);		
    }		
    break;
		
    default:
        LOG(LOG_FAIL, _T("[audin] unsupport mode.\r\n"));
        break;
    }
	
    return (TRUE);
}


static bool Aud_Linein_Stop(LIN_MODE lMode, AUD_ADC_INPUT_SRC eLinGroup, u8 u1DecID)
{
    //stop line in path
    PLIN_HAL_CLS_PUB prLinCls = g_prLinIn1;
    AUD_AOUT_DEVID eAoutId = AUDID_AOUT1;

    switch(lMode)
    {
    case LIN_DRAM_FRONT:
    case LIN_DRAM_REAR:
    //stop front linein
    if(LIN_DRAM_REAR == lMode)
    {
        prLinCls = g_prLinIn2;
    }
	
    if(NULL != prLinCls)
    {
    	if(AUD_STATE_STARTED == prLinCls->rHwIf.GetStatus(prLinCls))
        {
            prLinCls->rHwIf.Stop(prLinCls, 0);
        }  
    }		
    break;
		
    case LIN_BYPASS_FRONT:
    case LIN_BYPASS_REAR:
    if(LIN_BYPASS_REAR == lMode)
    {
        eAoutId = AUDID_AOUT2;
    }
    Aud_Linein_StopByPass(eLinGroup, eAoutId);
    break;
		
    default:
        LOG(LOG_FAIL, _T("[audin] unsupport mode.\r\n"));
        break;
    }
    
    return (TRUE);
}

/******************************************************************************
 * Function      : Aud_Linein_SetCtrl
 * Description   : Open/close linein
 * Parameter     : prLinInCmd: ctrl information
 * Return        : TRUE:success, FALSE: fail
 ******************************************************************************/
bool Aud_Linein_SetCtrl(AUDIN_SET_ONOFF* prLinInCmd, u8 u1DecID)
{
    bool fgRet = FALSE;
    AUD_ADC_INPUT_SRC eLinGroup = prLinInCmd->eLineINGroupSel;

    if((eLinGroup <= ADC_SRC_MICIN) ||(eLinGroup >= ADC_SRC_NON))
    {
        LOG(LOG_FAIL, _T("Line in Group out of range:0x%x"), eLinGroup);
        return (FALSE);
    }

    if(LIN_ON == prLinInCmd->fgAudInOnOff)
    {
        fgRet = Aud_Linein_Start(prLinInCmd->lMode, eLinGroup, u1DecID);       
    }
    else
    {
        fgRet = Aud_Linein_Stop(prLinInCmd->lMode, eLinGroup, u1DecID);
    }
    return (fgRet);
}

void Aud_Linein_VolCtrl(AUD_DEC_REAR_VOLUME_INFO_T * pRearChVol)
{
    u32 u4VolRearGain = (pRearChVol->ui1_level)*(0xFFFFFF/100);
    LOG(LOG_DATAF, TEXT("[AUD_CFG]u4VolRearGain = 0x%x,\n"),u4VolRearGain);
    Aud_Linein_SetByPasVol(u4VolRearGain);
}

void Aud_Linein_VolGainCtrl(AUD_DEC_REAR_VOLUME_GAIN_INFO_T * pRearChVol)
{
    u32 u4VolRearGain = 0;    
    LOG(LOG_DATAF, TEXT("Set audin rear bypass volume = 0x%x,\n"), pRearChVol->u4RearVolGain);
        
    if (pRearChVol->u4RearVolGain > 0x20000)
    {
        u4VolRearGain = 0xFFFFFF;
    }
    else
    {
        u4VolRearGain = (((pRearChVol->u4RearVolGain)*100)/0x20000)*(0xFFFFFF/100);
    }
    LOG(LOG_DATAF, TEXT(" u4Vol Rear Gain = 0x%x,\n"),u4VolRearGain);
    Aud_Linein_SetByPasVol(u4VolRearGain);
}

void Aud_Linein_ShowParams(u32 u4AudinId)
{
    if(u4AudinId == 0)
    {     
        LOG(LOG_DATAF, TEXT(" Linein1 params:\n"));       
        LOG(LOG_DATAF, TEXT(" Src = 0x%x,\n"),g_rLinIn1Params.eSrc);
        LOG(LOG_DATAF, TEXT(" clock = 0x%x,\n"),g_rLinIn1Params.eIntClkSrc);    
        LOG(LOG_DATAF, TEXT(" eGroup = 0x%x,\n"),g_rLinIn1Params.eGroup);    
        LOG(LOG_DATAF, TEXT(" u4BufPhyAdr = 0x%lx,\n"),g_rLinIn1Params.u4BufPhyAdr);
        LOG(LOG_DATAF, TEXT(" u4BufSz = 0x%x,\n"),g_rLinIn1Params.u4BufSz);
        if(EXT_LINEIN == g_rLinIn1Params.eSrc)
        {
            LOG(LOG_DATAF, TEXT(" Linein1 I2S config \n"));
        }
    }
    else if(u4AudinId == 1)
    {     
        LOG(LOG_DATAF, TEXT(" Linein2 params:\n"));       
        LOG(LOG_DATAF, TEXT(" Src = 0x%x,\n"),g_rLinIn2Params.eSrc);
        LOG(LOG_DATAF, TEXT(" clock = 0x%x,\n"),g_rLinIn2Params.eIntClkSrc);    
        LOG(LOG_DATAF, TEXT(" eGroup = 0x%x,\n"),g_rLinIn2Params.eGroup);    
        LOG(LOG_DATAF, TEXT(" u4BufPhyAdr = 0x%lx,\n"),g_rLinIn2Params.u4BufPhyAdr);
        LOG(LOG_DATAF, TEXT(" u4BufSz = 0x%x,\n"),g_rLinIn2Params.u4BufSz);
        if(EXT_LINEIN == g_rLinIn2Params.eSrc)
        {
            LOG(LOG_DATAF, TEXT(" Linein1 I2S config \n"));
        }
    }
    else if(u4AudinId == 2)
    {    
        LOG(LOG_DATAF, TEXT("Rear ByPass params:\n"));       
        LOG(LOG_DATAF, TEXT(" eDst= 0x%x,\n"),g_rByPassCfg.eDst);
        LOG(LOG_DATAF, TEXT(" eGainMode = 0x%x,\n"),g_rByPassCfg.eGainMode);    
        LOG(LOG_DATAF, TEXT(" eGroup = 0x%x,\n"),g_rByPassCfg.eGroup);    
        LOG(LOG_DATAF, TEXT(" u4Gain = 0x%x,\n"),g_rByPassCfg.u4Gain);    
        LOG(LOG_DATAF, TEXT(" u4Scale = 0x%x,\n"),g_rByPassCfg.u4Scale);        
    }
}

void Aud_Linein_SetInputType(u8 u1Input)
{    
    LOG(LOG_DATAF, _T("Audin input type = 0x%x.\n"), u1Input);
    vWriteDspShmBYTE(B_AUDIN_INPUT_TYPE, u1Input);    
}


static void Aud_Linein_SetIisParams(PLIN_EXTPARAMS_T prLinParams, AUD_IIS_CFG_INFO rI2sInfo)
{
    prLinParams->eSrc = EXT_LINEIN;

    prLinParams->rIntCfg.fgOn = FALSE;
    prLinParams->rIntCfg.eIntPeriod = LIN_INT_OFF;
    prLinParams->rIntCfg.PFN_ISR_CB = NULL;
    
    prLinParams->rI2sLinCfg.eClkMode = rI2sInfo.eMode;  
    prLinParams->rI2sLinCfg.rFmt.eCycle = rI2sInfo.rFmt.eCycle;
    prLinParams->rI2sLinCfg.rFmt.eDataFmt = rI2sInfo.rFmt.eDataFmt;
    prLinParams->rI2sLinCfg.rFmt.eFs = rI2sInfo.rFmt.eFs;
    prLinParams->rI2sLinCfg.rFmt.eMclkType = rI2sInfo.rFmt.eMclkType;    
    prLinParams->rI2sLinCfg.rFmt.u4SrcBitNum = rI2sInfo.rFmt.u4SrcBitNum;

    prLinParams->rI2sLinCfg.rFmt.eOutBitNum = LIN_16;
    prLinParams->rI2sLinCfg.rFmt.fgInvertBck = FALSE;
    prLinParams->rI2sLinCfg.rFmt.fgInvertLrck = FALSE;
}


static bool Aud_Linein_StartIisin(u8 u1DecId, LIN_MODE lMode, AUD_IIS_CFG_INFO rI2sInfo)
{
    PLIN_HAL_CLS_PUB  prLinCls    = NULL;
    PLIN_EXTPARAMS_T  prLinParams = NULL;
    uintptr_t u4Start = 0;
    bool fgRet = TRUE;
    u8 u1Event =  LINEIN1;

    LOG(LOG_CTRLF, _T("Mode = 0x%x, PinGrp = 0x%x.\r\n"), rI2sInfo.eMode, rI2sInfo.ePinGrp);

    LOG(LOG_DATAF, _T("Mclk = 0x%x, FS = 0x%x, Cycle = 0x%x.\r\n"), 
        rI2sInfo.rFmt.eMclkType, rI2sInfo.rFmt.eFs, rI2sInfo.rFmt.eCycle);
    LOG(LOG_DATAF, _T("DataFmt = 0x%x, SrcBitNum = 0x%x.\r\n"), 
        rI2sInfo.rFmt.eDataFmt, rI2sInfo.rFmt.u4SrcBitNum);
    
    if(SEC_DEC == u1DecId)
    {
        prLinCls = g_prLinIn1;
        prLinParams = &g_rLinIn1Params;
        u4Start = AUD_AFIFO_LINEIN1_START;
        u1Event =  LINEIN1;

        prLinParams->rI2sLinCfg.eGrpPin0 = rI2sInfo.ePinGrp;
        prLinParams->rI2sLinCfg.eGrpPin1 = PINMUX_I2SLIN1_DEFAULT;
    }
    else if(TER_DEC == u1DecId)
    {
        prLinCls = g_prLinIn2;
        prLinParams = &g_rLinIn2Params;
        u4Start = AUD_AFIFO_LINEIN2_START;
        u1Event =  LINEIN2;
        
        prLinParams->rI2sLinCfg.eGrpPin0 = PINMUX_I2SLIN0_DEFAULT;
        prLinParams->rI2sLinCfg.eGrpPin1 = rI2sInfo.ePinGrp;
    }
    else
    {
        LOG(LOG_CTRLF, _T("start iisin unsupport this Dec Id.\r\n"));
        return (FALSE);
    }

    if(AUD_STATE_STARTED == prLinCls->rHwIf.GetStatus(prLinCls))
    {        
        LOG(LOG_CTRLF, _T("Linein(%d) status is started, start it error.\r\n"), u1DecId);
        fgRet = FALSE;
    }
    else
    {
        //I2S params
        prLinParams->u4BufPhyAdr = AFIFO_PHYSICAL(u4Start);
        prLinParams->u4BufSz = AUD_AFIFO_LINEIN_SIZE;
        Aud_Linein_SetIisParams(prLinParams, rI2sInfo);
        if(lMode == LIN_DRAM_FRONT)
        {
            prLinParams->eIntClkSrc = LIN_CLK_AOUT1;
        }
        else
        {
            prLinParams->eIntClkSrc = LIN_CLK_AOUT2;
        }
        prLinParams->eGroup = ADC_SRC_NON;
        prLinCls->rHwIf.Setup(prLinCls, prLinParams);
        prLinCls->rHwIf.Start(prLinCls, 0);        

        i4AudEsm_SetAuEvent(u1Event);
        Sleep(100);
    }
    return (fgRet);    
}


static bool Aud_Linein_StopIisin(u8 u1DecId)
{
    bool fgRet = TRUE;
    if(SEC_DEC == u1DecId)
    {
        if(AUD_STATE_STARTED == g_prLinIn1->rHwIf.GetStatus(g_prLinIn1))
        {            
            g_prLinIn1->rHwIf.Stop(g_prLinIn1, 0);
        }        
    }
    else if(TER_DEC == u1DecId)
    {
        if(AUD_STATE_STARTED == g_prLinIn2->rHwIf.GetStatus(g_prLinIn2))
        {            
            g_prLinIn2->rHwIf.Stop(g_prLinIn2, 0);
        }  
    }
    else
    {
        LOG(LOG_CTRLF, _T("Iis in unsupport this Dec Id.\r\n"));
        fgRet = FALSE;
    }
    return (fgRet);
}

bool Aud_Linein_SetIisin(AUD_IIS_CTRL_INFO* prCtrl, u8 u1DecId)
{
    bool fgRet = TRUE;
    LOG(LOG_CTRLF,_T("Iis in On/Off is %d, lMode is %d.\r\n"), 
		prCtrl->fgAudInOnOff, prCtrl->lMode);
	
    if(LIN_ON == prCtrl->fgAudInOnOff)
    {
        fgRet = Aud_Linein_StartIisin(u1DecId, prCtrl->lMode, prCtrl->rI2sInfo);
    }
    else
    {
        fgRet = Aud_Linein_StopIisin(u1DecId);
    }
    return (fgRet);
}

void Aud_Linein_SetDecInfo(AUD_INFO_T *prInfo)
{
    prInfo->e_aud_fmt = AUD_DEC_FMT_PCM;
    prInfo->e_aud_type = AUD_DEC_TYPE_STEREO;	
    prInfo->ui1_bit_depth = 24;

    prInfo->ui2_pid = 0;
    prInfo->ui4_data_rate = 1536000;  //48000*2*16
    prInfo->ui4_sample_rate = 48000;
    prInfo->pcm_info.ePCM_Format = AUD_DEC_PCM_FMT_WAVE;
    prInfo->pcm_info.b_dlna_exist = FALSE;
    prInfo->pcm_info.b_de_emphasis = 4;
    prInfo->pcm_info.u2BlockAlign = FALSE;
}


bool Aud_Linein_AllocDecSource(AUD_DRV_CONTEXT *prContext, LIN_MODE eMode)
{    
    AUD_OUT_MEDIA_TYPE_T eType = AUD_OUT_MEDIA_LINE_IN;
    u8 u1Out = AUD_FRONT;
    switch(eMode)
    {
    case LIN_DRAM_FRONT:
        eType = AUD_OUT_MEDIA_LINE_IN;
        u1Out = AUD_FRONT;
        break; 
    case LIN_BYPASS_REAR:
        eType = AUD_OUT_MEDIA_UNDEF;
        u1Out = AUD_REAR;
        break;
    case LIN_BYPASS_FRONT:
        eType = AUD_OUT_MEDIA_UNDEF;
        u1Out = AUD_FRONT;
        break;
    case LIN_DRAM_REAR:
        eType = AUD_OUT_MEDIA_LINE_IN2;
        u1Out = AUD_REAR;
        break;

        default:
        LOG(LOG_CTRLF,_T("audio in flow, unsupport mode %d.\r\n"), eMode);
        return FALSE;
    }
    return (AudAllocDecResource(prContext, u1Out, eType));
}


void Aud_Linein_ConfigDecoder(AUD_DRV_CONTEXT *prContext)
{
    AUD_DRV_FMT_INFO_T rAudFmtInfo = {0};
    AUD_INFO_T rAudioInfo;
    AUD_DEC_CTRL_T eAudCtrl = AUD_DEC_CTRL_PLAY;

    prContext->fgPlaying = TRUE;
    //if(prContext->ePlayType == AUD_OUT_MEDIA_LINE_IN ||
    //   prContext->ePlayType == AUD_OUT_MEDIA_LINE_IN2)
    {
        rAudFmtInfo.e_fmt = AUD_DEC_FMT_PCM;
        AudSetFormat(prContext->u1DecId, &rAudFmtInfo);

        Aud_Linein_SetDecInfo(&rAudioInfo);	
        AudSetMwCodecInfo(prContext->u1DecId, &rAudioInfo);

        if(prContext->fgEnPlay == TRUE)
        {
            i4AudEsm_Connect(prContext->u1DecId);
            AudSetMwCtrl(prContext->u1DecId, eAudCtrl);
        }
    }
}

bool Aud_Linein_AdcInStart(AUD_DRV_CONTEXT *prContext, AUDIN_SET_ONOFF* prParam)
{
    u8 u1Input = AUD_ADC_IN;
    bool fgRet = FALSE;

    //vWriteDspShmBYTE(B_AUDIN_INPUT_TYPE, u1Input);
    //if(TRUE == Aud_Linein_AllocDecSource(prContext, prParam->lMode))
    //{
        Aud_Linein_ConfigDecoder(prContext);
        fgRet = Aud_Linein_Start(prParam->lMode, prParam->eLineINGroupSel, prContext->u1DecId);
    //}

    return (fgRet);
}


bool Aud_Linein_AdcInStop(AUD_DRV_CONTEXT *prContext, AUDIN_SET_ONOFF* prParam)
{
    AUD_DEC_CTRL_T eAudCtrl = AUD_DEC_CTRL_STOP;
    bool ret = TRUE;

    if(prContext->fgPlaying == TRUE)
    {
        if(prContext->u1DecId== SEC_DEC || prContext->u1DecId == TER_DEC)//dram mode
        {
            AudSetMwCtrl(prContext->u1DecId, eAudCtrl);
            i4AudEsm_Disconnect(prContext->u1DecId);
        }
        Aud_Linein_Stop(prParam->lMode, prParam->eLineINGroupSel, prContext->u1DecId);
        //AudReleaseDecResource(prContext);
    }
    else
    {
        LOG(LOG_FAIL,_T("AdcInStop, pcontext status is stop.\r\n"));
        ret = FALSE;
    }

    return ret;
}

bool Aud_Linein_AdcInFlowCtrl(AUD_DRV_CONTEXT *prContext, AUDIN_SET_ONOFF* prParam)
{
    if(prParam->fgAudInOnOff == LIN_ON)
    {
        return (Aud_Linein_AdcInStart(prContext, prParam));
    }
    else
    {
        return (Aud_Linein_AdcInStop(prContext, prParam));
    }
}


bool Aud_Linein_IisInStart(AUD_DRV_CONTEXT *prContext, AUD_IIS_CTRL_INFO* prParam)
{
    u8 u1Input = ADC_IIS_IN;
    bool fgRet = FALSE;

    //vWriteDspShmBYTE(B_AUDIN_INPUT_TYPE, u1Input);
    //if(TRUE == Aud_Linein_AllocDecSource(prContext, prParam->lMode))
    //{
        Aud_Linein_ConfigDecoder(prContext);
        fgRet = Aud_Linein_StartIisin(prContext->u1DecId, prParam->lMode, prParam->rI2sInfo);
    //}

    return (fgRet);
}



bool Aud_Linein_IisStop(AUD_DRV_CONTEXT *prContext, AUD_IIS_CTRL_INFO* prParam)
{
    AUD_DEC_CTRL_T eAudCtrl = AUD_DEC_CTRL_STOP;
    bool ret = TRUE;

    if(prContext->fgPlaying == TRUE)
    {
        if(prContext->u1DecId== SEC_DEC || prContext->u1DecId == TER_DEC)//dram mode
        {
            AudSetMwCtrl(prContext->u1DecId, eAudCtrl);
            i4AudEsm_Disconnect(prContext->u1DecId);
        }
        Aud_Linein_StopIisin(prContext->u1DecId);
        //AudReleaseDecResource(prContext);
    }
    else
    {
        LOG(LOG_FAIL,_T("iisStop, pcontext status is stop.\r\n"));
        ret = FALSE;
    }
    return ret;
}


bool Aud_Linein_IisInFlowCtrl(AUD_DRV_CONTEXT *prContext, AUD_IIS_CTRL_INFO* prParam)
{
    if(prParam->fgAudInOnOff == LIN_ON)
    {
        return (Aud_Linein_IisInStart(prContext, prParam));
    }
    else
    {
        return (Aud_Linein_IisStop(prContext, prParam));
    }
}


