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




#include "x_bim.h"
#include "aud_config.h"
#include "aud_extdac.h"
#include "aud_3360_reg_misc.h"
#include "aud_3360_reg_rw.h"
#include "aud_pwmdac.h"
#include "util.h"
#include <media/atc/aud_output.h>
#include "aud_clock.h"
#include "aud_hal.h"
#include <media/atc/drv_aud.h>
#include "aud_debug.h"
#include "aud_aout_hal_if.h"
#include "aud_io_clock_if.h"
#include "aud_io_manager_if.h"
#include "aud_reg_ckgen.h"
#include "aud_reg_rgbk2.h"
#include "audin_if.h"
#include "aud_oal.h"

void AudAout_Init(AUD_CLOCK_T eAoutId, AUDIO_SAMPLING_T eSample);
void vAudMuteCircuitCtrl(bool fgMute);
PAOUT_HAL_CLS_PUB AudAout_GetHalClass(AUD_AOUT_DEVID eAoutID);
PAOUT_EXTPARAMS_T AudAout_GetHalParam(AUD_AOUT_DEVID eAoutID);
/****************************************************************************
** Local definitions
****************************************************************************/
#define TAKE_AUD_POWER_ON_SEMAPHORE() BIM_GETHWSemaphore(HWSMPHE_AUD_POWER_ON, 10)

/****************************************************************************
** Local variable
****************************************************************************/
extern bool g_fgAPLLPowerOnByArm9;

static bool  g_fgEnSpdifOut = FALSE;
PAOUT_HAL_CLS_PUB g_prAudAout1 = NULL;
PAOUT_HAL_CLS_PUB g_prAudAout2 = NULL;
AOUT_EXTPARAMS_T  g_rAout1Parmas = {0};
AOUT_EXTPARAMS_T  g_rAout2Parmas = {0};

#ifdef __linux__
s32 f_dac_type = AUD_DAC_PWM;
s32 f_pin_grp = PINMUX_FS_I2SOUT_GROUP1;
s32 r_dac_type = AUD_DAC_PWM;
s32 r_pin_grp = PINMUX_RS_I2SOUT_GROUP3;
module_param(f_dac_type, int, S_IRUSR | S_IWUSR | S_IWGRP | S_IRGRP | S_IROTH); /* rw-rw-r-- */
module_param(f_pin_grp, int, S_IRUSR | S_IWUSR | S_IWGRP | S_IRGRP | S_IROTH); /* rw-rw-r-- */
module_param(r_dac_type, int, S_IRUSR | S_IWUSR | S_IWGRP | S_IRGRP | S_IROTH); /* rw-rw-r-- */
module_param(r_pin_grp, int, S_IRUSR | S_IWUSR | S_IWGRP | S_IRGRP | S_IROTH); /* rw-rw-r-- */
#endif


/****************************************************************************
** Local functions
**************************************************************************/
void AudCfg_SetAoutSR(u8 u1DecId, u8 u1SmpRate)
{
    AUD_CLOCK_T eAoutID = 0;
    if (u1DecId == PRI_DEC)
    {
        eAoutID = AUD_CLK_AOUT1;
    }
    else
    {
        eAoutID = AUD_CLK_AOUT2;
    }
    
    AudAout_SampleSet(u1SmpRate, eAoutID);
}


void AudCfg_SetDVPRS(AUDIO_SAMPLING_T eSmpRate)
{
    //Play DVD. Switch AOUT1 to Front and AOUT2 to Rear.
    PAOUT_HAL_CLS_PUB prAout = AudAout_GetHalClass(AUDID_AOUT1);
    PAOUT_EXTPARAMS_T prExtCfg = AudAout_GetHalParam(AUDID_AOUT1);
    prExtCfg->eOutPath = AOUT_FS;
    prAout->SetAoutPath(prAout, AOUT_FS);

    prAout = AudAout_GetHalClass(AUDID_AOUT2);
    prExtCfg = AudAout_GetHalParam(AUDID_AOUT2);
    prExtCfg->eOutPath = AOUT_RS;
    prAout->SetAoutPath(prAout, AOUT_RS);    
}

/****************************************************************************
** Global functions
****************************************************************************/
void AudCfg_HWInit(void)
{
    if(!TAKE_AUD_POWER_ON_SEMAPHORE()) 
    {
        g_fgAPLLPowerOnByArm9 = TRUE;
    } 
    else 
    {          
        g_fgAPLLPowerOnByArm9 = FALSE;       
    }

    LOG(LOG_CTRLF, TEXT("[AUD_CFG] Power On by ARM9(%d)!\n"), g_fgAPLLPowerOnByArm9);

    AudIoMg_InitHw(g_fgAPLLPowerOnByArm9);
    vDspSetClock();

    AudAout_Init(AUD_CLK_AOUT1, FS_48K);
    AudAout_Init(AUD_CLK_AOUT2, FS_48K);    
    
    AudCfg_SpdifEnable(AUD_AOUT1);

    //Enable DVD DTS license
    CKGEN_SETBITS(0x94,0x80);

    // this pin is used to ctrl anolog out OP chip in general board.
    // if customer used this pin for other function, please disable below code in customer branch.
    LOG(LOG_DAC, TEXT("[AUD_CFG] open the 12v power to control audio anolog out OP circuit!\n"));
    vAudMuteCircuitCtrl(FALSE);
}

bool AudCfg_ChgOutCfg(AUD_SOURCE_CFG_T   *prAudSrcParam,
                   AUD_OUTPUT_SETTING_CFG_T   *prAudOutParam,
                   AUD_OUTPUT_SETTING_CFG_T   *prAudHdmiOutParam)
{
    bool fgSfChanged = TRUE;

    LOG(LOG_DATAF, TEXT("[AUD_CFG] AudCfg_ChgOutCfg \n"));

    prAudOutParam->u1Sampling_Rate = prAudSrcParam->u1Aud_Sampling_Rate;
    prAudOutParam->eIec_Cfg = AUD_IEC_CFG_PCM;
    memset(&prAudOutParam->rChan_Delay, 0, sizeof(prAudOutParam->rChan_Delay));

    prAudHdmiOutParam->u1Sampling_Rate = prAudSrcParam->u1Aud_Sampling_Rate;;
    prAudHdmiOutParam->eIec_Cfg = AUD_IEC_CFG_PCM;
    prAudHdmiOutParam->u1Aud_Output_Chan_Cnt = prAudSrcParam->u1Aud_Input_Chan_Cnt;
    prAudHdmiOutParam->fgHBROutI2S = FALSE;
    memset(&prAudHdmiOutParam->rChan_Delay, 0, sizeof(prAudHdmiOutParam->rChan_Delay));

    return fgSfChanged;
}

void AudCfg_RestoreAoutRegs(void)
{
    PAOUT_HAL_CLS_PUB prAout = AudAout_GetHalClass(AUDID_AOUT2);
    
    if (prAout->IsArmCtrl(prAout))
    {       
        PAOUT_EXTPARAMS_T prExtCfg = AudAout_GetHalParam(AUDID_AOUT2);
        LOG(0, TEXT("[Aud_Cfg]Restore Aout Regs !\n"));
        prAout->rHwIf.Setup(prAout, prExtCfg);
        
        AUDREG_BITS_W(REGENV_AOUT2_CH1_BUF_SADR, BIT_STR_AOUT2_CH1_BUF_SADR, 
                      BIT_NUM_AOUT2_CH1_BUF_SADR, 0x2000);   // Update Start Address
        AUDREG_BITS_W(REGENV_AOUT2_CH1_BUF_SIZE, BIT_STR_AOUT2_CH1_BUF_SIZE, 
                      BIT_NUM_AOUT2_CH1_BUF_SIZE, 0x2730);   // Update Size of Buffer
        AUDREG_BITS_W(REGENV_AOUT2_NSNUM, BIT_STR_AOUT2_NSNUM, 
                      BIT_NUM_AOUT2_NSNUM, 0x40);            // Set Sample Num of One Frame       
        AUDREG_BITS_W(REGENV_AOUT2_INTRSIZE, BIT_STR_AOUT2_INTRSIZE, 
                      BIT_NUM_AOUT2_INTRSIZE, 0x3);         // Set Interrupt Size

        prAout->rHwIf.Start(prAout, 0);
        prAout->SetArmCtrl(prAout, FALSE);

        //Reset front pwm&ext dac select aout1
        prAout = AudAout_GetHalClass(AUDID_AOUT1);
        prExtCfg = AudAout_GetHalParam(AUDID_AOUT1);
        prExtCfg->eOutPath = AOUT_FS;
        prAout->SetAoutPath(prAout, AOUT_FS);
    }
}

void AudCfg_SwitchAout(u32 dwParam)
{
    LOG(LOG_DATAF, TEXT("[AUD_CFG] AudCfg_SwitchAout unused.\n"));
    
}

void AudCfg_SpdifEnable(AUD_OUT_TYPE_T eSrcID)
{
    if ((eSrcID == AUD_AOUT1) || (eSrcID == AUD_DVD_OUT)|| (eSrcID == 4))
    {
        if (!g_fgEnSpdifOut)
        {
            // Pinmux select for SPDIF out
            IoPinMux_SetSpdif(PINMUX_SPDIF_GROUP1);
            //IoClk_SetIecClk(AUD_MCLK_128FS, FS_48K); //remove, set with aout1 together 
            g_fgEnSpdifOut = TRUE;
        }
        AudCfg_MuteSPDIF(FALSE);
        
        //SPDIF Setting
        AudCfg_IECSelect(eSrcID);
    }
    else
    {
        // Disable SPDIF        
        LOG(LOG_DATAF, TEXT("[SPDIF][AUD_CFG]Mute SPDIF.\n"));
        AudCfg_MuteSPDIF(TRUE);
        g_fgEnSpdifOut = FALSE;
    }
}

// Select source for SPDIF output
void AudCfg_MuteSPDIF(bool fgMute)
{
    if (fgMute)
    {
        // Mute IEC
        CKGEN_SETBITS(0x5014, 0x1 << 12);
        LOG(LOG_DATAF, TEXT("[AUD_CFG] AudCfg_MuteSPDIF - Mute SPDIF\n"));
    }
    else
    {
        // Unmute IEC
        CKGEN_CLRBITS(0x5014, 0x1 << 12);
        LOG(LOG_DATAF, TEXT("[AUD_CFG] AudCfg_MuteSPDIF - Unmute SPDIF\n"));
    }
}


void AudCfg_IECSelect(AUD_OUT_TYPE_T eSrcID)
{
    if (eSrcID == AUD_AOUT1)
    {
        CKGEN_CLRBITS(0x5014, 0x4 << 8);
        CKGEN_CLRBITS(AUD_REG_RGBK2_CFG1, 0x1 << 8);
        LOG(LOG_DATAF, TEXT("[SPDIF][AUD_CFG] AudCfg_IECSelect - AP to SPDIF Out\n"));
    }
    else if (eSrcID == AUD_DVD_OUT)
    {
        CKGEN_CLRBITS(0x5014, 0x4 << 8);
        CKGEN_SETBITS(AUD_REG_RGBK2_CFG1, 0x1 << 8);
        LOG(LOG_DATAF, TEXT("[SPDIF][AUD_CFG] AudCfg_IECSelect - DVP to SPDIF Out\n"));
    }
    else if (eSrcID == 4)//Line in buffer
    {
        CKGEN_SETBITS(0x5014, 0x4 << 8);
        LOG(LOG_DATAF, TEXT("[SPDIF][AUD_CFG] AudCfg_IECSelect - Line in to SPDIF Out\n"));
    }
}


void AudAout_ShowStatus(u8 u1ClkId)
{
    PAOUT_EXTPARAMS_T prAoutParam = NULL;
    AUD_AOUT_DEVID eAoutID = 0;

    if(u1ClkId == AUD_AOUT1)
    {    
        LOG(LOG_DATAF, TEXT("Show AOut1 configure:\n"));
        eAoutID = AUDID_AOUT1;
    }
    else if(u1ClkId == AUD_AOUT2)
    {
        LOG(LOG_DATAF, TEXT("Show AOut2 configure:\n"));
        eAoutID = AUDID_AOUT2;
    }
    else
    {
        LOG(LOG_DATAF, TEXT("Unsupport Aout ID.\n"));
        return;
    }
    //prAoutCls = AudAout_GetHalClass(eAoutID);        
    prAoutParam = AudAout_GetHalParam(eAoutID);
    
    LOG(LOG_DATAF, TEXT("eOutPath = 0x%x, eDacType = 0x%x, fgAdcBypasMode = 0x%x.\n"),
        prAoutParam->eOutPath, prAoutParam->eDacType, prAoutParam->fgAdcBypasMode);    
    LOG(LOG_DATAF, TEXT("eFs = 0x%x, u4Bps = 0x%x.\n"), prAoutParam->eFs, prAoutParam->u4Bps);

    if(AUD_DAC_EXT == prAoutParam->eDacType)
    {
        LOG(LOG_DATAF, TEXT("Front Ext Dac pinmux = 0x%x, Rear pinmux = 0x%x.\n"),
        prAoutParam->ePinMuxFsExtDac, prAoutParam->ePinMuxRsExtDac);
    }    
}

/**
 * Get aout params
 *
 * @param [in]  eAoutID : AUDID_AOUT1/AUDID_AOUT2
 * @param [out] aout params pointer
 *
 * @return
 */
PAOUT_EXTPARAMS_T AudAout_GetHalParam(AUD_AOUT_DEVID eAoutID)
{
    PAOUT_EXTPARAMS_T pAoutParam = NULL;
    
    if(AUDID_AOUT1 == eAoutID)
    {
        pAoutParam = &g_rAout1Parmas;
    }
    else if(AUDID_AOUT2 == eAoutID)
    {
        pAoutParam = &g_rAout2Parmas;
    }
    else
    {
        LOG(LOG_FAIL, _T("GetAoutClass Fail,AoutID(0x%x) out of range."), eAoutID);
        return (NULL);
    }
    return (pAoutParam);
}


/**
 * Get aout hal class
 *
 * @param [in]  eAoutID : AUDID_AOUT1/AUDID_AOUT2
 * @param [out] aout hal class pointer
 *
 * @return
 */

PAOUT_HAL_CLS_PUB AudAout_GetHalClass(AUD_AOUT_DEVID eAoutID)
{
    if(AUDID_AOUT1 == eAoutID)
    {
        return (g_prAudAout1);
    }
    else if(AUDID_AOUT2 == eAoutID)
    {
        return (g_prAudAout2);
    }
    else
    {
        LOG(LOG_FAIL, _T("GetAoutClass Fail,AoutID(0x%x) out of range."), eAoutID);
        return (NULL);
    }
}


/**
 * New aout hal class
 *
 * @param [in]  eAoutID : AUDID_AOUT1/AUDID_AOUT2
 * @param [out] True: success, False: fail
 *
 * @return
 */
 
bool AudAout_NewHalClass(AUD_AOUT_DEVID eAoutID)
{
    bool fgRet = TRUE;
    
    ASSERT((eAoutID == AUDID_AOUT1) ||(eAoutID == AUDID_AOUT2));
    
    if(NULL == AudAout_GetHalClass(eAoutID))
    {        
        if(AUDID_AOUT1 == eAoutID)
        {
            g_prAudAout1 = AoutHal_New(eAoutID);
            if (NULL == g_prAudAout1)
            {
                LOG(LOG_FAIL,_T("Audio New Aout1 Obj Fail !!!  \r\n"));
                fgRet = FALSE;
            }
            else
            {
                LOG(LOG_DAC,_T("Audio New Aout1 Obj success, g_prAudAout1 0x%x\r\n"), (u32)g_prAudAout1);
            }
        }
        else if(AUDID_AOUT2 == eAoutID)
        {
            g_prAudAout2 = AoutHal_New(eAoutID);
            if (NULL == g_prAudAout2)
            {
                LOG(LOG_FAIL,_T("Audio New Aout2 Obj Fail !!!  \r\n"));
                fgRet = FALSE;
            }
            else
            {
                LOG(LOG_DAC,_T("Audio New Aout2 Obj success, g_prAudAout2 0x%x\r\n"), (u32)g_prAudAout2);
            }
        }
        else
        {
            LOG(LOG_FAIL, _T("NewAoutClass Fail,AoutID(0x%x) out of range."), eAoutID);
            fgRet = FALSE;
        }
    }
    else
    {    
        LOG(LOG_FAIL,_T("Audio Aout(0x%x) already new. \r\n"), eAoutID);
        fgRet = FALSE;
    }

    return (fgRet);
}

bool AudAout_StartHalClass(AUD_AOUT_DEVID eAoutID, AUDIO_SAMPLING_T eSample)
{
    AUD_OUT_PATH_T eAoutPath = AOUT_FS;

    PAOUT_EXTPARAMS_T prExtCfg = NULL;
    PAOUT_HAL_CLS_PUB prTmpAout = NULL;
    
    prExtCfg = AudAout_GetHalParam(eAoutID);
    prTmpAout = AudAout_GetHalClass(eAoutID);
    
    if(NULL == prExtCfg|| NULL == prTmpAout)
    {
        LOG(LOG_FAIL, _T("SetupAoutClass prExtCfg/prTmpAout is NULL.\r\n"));
        return (FALSE);
    }

    if(AUDID_AOUT1 == eAoutID)
    {
        eAoutPath = g_fgAPLLPowerOnByArm9 ? AOUT_RS : AOUT_FS;
        #ifndef __linux__
        AudOS_Regkey_GetDword(AUD_DRV, (s8 *)L"AOUT_FS_DAC_TYPE", 
            (u32 *) &(prExtCfg->eDacType), AUD_DAC_PWM);
        #else
        prExtCfg->eDacType = f_dac_type;
        LOG(LOG_CTRLF, _T("Front DAC Type is : %d .\r\n"), f_dac_type);
        #endif
    }
    else if(AUDID_AOUT2 == eAoutID)
    {
        eAoutPath = AOUT_RS;
        #ifndef __linux__
        AudOS_Regkey_GetDword(AUD_DRV, (s8 *)L"AOUT_RS_DAC_TYPE", 
           (u32 *) &(prExtCfg->eDacType), AUD_DAC_PWM);
        #else
        prExtCfg->eDacType = r_dac_type;
        LOG(LOG_CTRLF, _T("Rear DAC Type is : %d .\r\n"), r_dac_type);
        #endif
    }
    else
    {
        LOG(LOG_CTRLF, _T("AudAout_StartHalClass only aout1/aout2 .\r\n"));
    }
    
    prExtCfg->eOutPath = eAoutPath;
    prExtCfg->fgAdcBypasMode = FALSE;
    prExtCfg->eFs = eSample;
    prExtCfg->u4Bps = 24;

    #ifndef __linux__
    AudOS_Regkey_GetDword(AUD_DRV, (s8 *)L"AOUT_FS_EXTDAC_GRP", 
            (u32 *) &(prExtCfg->ePinMuxFsExtDac), PINMUX_FS_I2SOUT_GROUP1);
    AudOS_Regkey_GetDword(AUD_DRV, (s8 *)L"AOUT_RS_EXTDAC_GRP", 
            (u32 *) &(prExtCfg->ePinMuxRsExtDac), PINMUX_RS_I2SOUT_GROUP3);
    #else
    prExtCfg->ePinMuxFsExtDac = f_pin_grp;
    prExtCfg->ePinMuxRsExtDac = r_pin_grp;
    LOG(LOG_DAC, _T("Front/Rear I2S output Group is %d and %d .\r\n"), f_pin_grp, r_pin_grp);
    #endif
    
    prExtCfg->u4BufPhyAdr = 0;
    prExtCfg->u4BufSz = 0;

    prExtCfg->rIntCfg.u4NSNum = 0;
    prExtCfg->rIntCfg.u4IntrSize = 0;
    prExtCfg->rIntCfg.PFN_ISR_CB = NULL;

    if (AUDID_AOUT1 == eAoutID) 
    {
        prTmpAout->SetArmCtrl(prTmpAout, FALSE);
        prTmpAout->rHwIf.Setup(prTmpAout, prExtCfg);
        prTmpAout->rHwIf.Start(prTmpAout, 0);
    }
    else if(AUDID_AOUT2 == eAoutID)
    {
        if(g_fgAPLLPowerOnByArm9)
        {
            prTmpAout->IntConfigInit(prTmpAout);
        }
        else
        {
            prTmpAout->SetArmCtrl(prTmpAout, FALSE);
            prTmpAout->rHwIf.Setup(prTmpAout, prExtCfg);
            prTmpAout->rHwIf.Start(prTmpAout, 0);
        }
    }
    else
    {
        LOG(LOG_FAIL, _T("AudAout_StartHalClass aout ID error.\r\n"));
    }
    
    return (TRUE);
}


/**
 * arm init aout
 *
 * @param [in]  eAoutId : AUD_CLK_AOUT1/AUD_CLK_AOUT2, eSample: audio sample rate
 * @param [out] 
 *
 * @return
 */
void AudAout_Init(AUD_CLOCK_T eAoutClkId, AUDIO_SAMPLING_T eSample)
{
    AUD_AOUT_DEVID eAoutId = 0;
    LOG(LOG_DAC, _T("Audio Aout ID(%d), sample rate(%d) \r\n"), eAoutClkId, eSample);   

    if(AUD_CLK_AOUT1 == eAoutClkId)
    {
        eAoutId = AUDID_AOUT1;
    }
    else if(AUD_CLK_AOUT2 == eAoutClkId)
    {
        eAoutId = AUDID_AOUT2;
    }
    else
    {
        LOG(LOG_FAIL, _T("AudAout_Init eAoutClkId(%d) error.\r\n"), eAoutClkId);
        return;
    }    

    ASSERT(FALSE != AudAout_NewHalClass(eAoutId));
    ASSERT(FALSE != AudAout_StartHalClass(eAoutId, eSample));
}

void AudAout_DeInit(AUD_CLOCK_T eAoutClkId)
{
    if(AUD_CLK_AOUT1 == eAoutClkId)
    {
        if(NULL != g_prAudAout1)
        {
            LOG(LOG_CTRLF, _T("AudAout_DeInit eAoutClkId(%d), g_prAudAout1 0x%x, g_prAudAout1->Delete 0x%x\r\n\r\n"),
                eAoutClkId, (u32)g_prAudAout1, (u32)(g_prAudAout1->Delete));
            g_prAudAout1->Delete(g_prAudAout1);
            g_prAudAout1 = NULL;        
        }
    }
    else if(AUD_CLK_AOUT2 == eAoutClkId)
    {
        if(NULL != g_prAudAout2)
        {
            LOG(LOG_CTRLF, _T("AudAout_DeInit eAoutClkId(%d), g_prAudAout2 0x%x, g_prAudAout2->Delete 0x%x\r\n\r\n"),
                eAoutClkId, (u32)g_prAudAout2, (u32)(g_prAudAout2->Delete));
            g_prAudAout2->Delete(g_prAudAout2);
            g_prAudAout2 = NULL;
        }
    }
    else
    {
        LOG(LOG_FAIL, _T("AudAout_Init eAoutClkId(%d) error.\r\n"), eAoutClkId);
    }
    return;
}


/**
 * set aout1/aout2 sample rate
 *
 * @param [in]  AUDIO_SAMPLING_T : audio sample rate, eType: aout1/aout2
 * @param [out] 
 *
 * @return
 */
void AudAout_SampleSet( AUDIO_SAMPLING_T eFS, AUD_CLOCK_T eType)
{
    PAOUT_HAL_CLS_PUB pAoutCls = NULL;
    PAOUT_EXTPARAMS_T pAoutParma = NULL;
    
    if(AUD_CLK_AOUT1 == eType)
    {
        pAoutCls = AudAout_GetHalClass(AUDID_AOUT1);
        pAoutParma = AudAout_GetHalParam(AUDID_AOUT1);       
    }
    else if(AUD_CLK_AOUT2 == eType)
    {
        pAoutCls= AudAout_GetHalClass(AUDID_AOUT2);
        pAoutParma = AudAout_GetHalParam(AUDID_AOUT2);                
    }
    else
    {        
        LOG(LOG_FAIL,_T("Audio Aout ID = 0x%x, error.\r\n"),eType);
        return;
    }
    

    if((NULL != pAoutCls)&&(NULL != pAoutParma))
    {
        pAoutParma->eFs = eFS;
        pAoutCls->rHwIf.CfgUpd(pAoutCls, pAoutParma);
    }
    else
    {         
        LOG(LOG_FAIL, _T("AudAout_SampleSet get aout class is null"));

    }         

    return;
}


/**
 * set aout path
 *
 * @param [in]  prPath : aout path
 * @param [out] 
 *
 * @return
 */
bool AudAout_PathSet(AUD_OUTPUT_PATH_T *prPath)
{
    bool fgRet = TRUE;
    PAOUT_HAL_CLS_PUB prAoutHal = NULL;
    PAOUT_EXTPARAMS_T prAoutParam = NULL;
    
    if(NULL == prPath)
    {
        LOG(LOG_FAIL, _T("AudAout_PathSet param NULL"));
        return (FALSE);
    }

    //set spdif out
    if (AUD_SPDIF == prPath->eOut)
    {
        // Without DAC Select.
        if (prPath->eSrc == AUD_AOUT1)
        {
            CKGEN_CLRBITS(AUD_REG_RGBK2_CFG1, 0x1 << 8);
        }
        else if (prPath->eSrc == AUD_DVD_OUT)
        {
            CKGEN_SETBITS(AUD_REG_RGBK2_CFG1, 0x1 << 8);
        }
        LOG(LOG_FAIL, TEXT("[AUD_CFG] Out to SPDIF.\n"));
        return (TRUE);
    }

    //set iis out
    if(AUD_AOUT1 == prPath->eSrc)
    {
        prAoutHal = AudAout_GetHalClass(AUDID_AOUT1);
        prAoutParam = AudAout_GetHalParam(AUDID_AOUT1); 
    }
    else if(AUD_AOUT2 == prPath->eSrc)
    {        
        prAoutHal = AudAout_GetHalClass(AUDID_AOUT2);
        prAoutParam = AudAout_GetHalParam(AUDID_AOUT2); 
    }
    else
    {
        LOG(LOG_FAIL, _T("AudAout_PathSet path src = 0x%x,error\n"), prPath->eOut);
        fgRet = FALSE;
    }

    if(NULL != prAoutHal)
    {        
        if(AUD_FRONT == prPath->eOut)
        {
            prAoutParam->eOutPath = AOUT_FS;
            prAoutHal->rHwIf.CfgUpd(prAoutHal, prAoutParam);        
        }
        else if(AUD_REAR == prPath->eOut)
        {
            prAoutParam->eOutPath = AOUT_RS;
            //check if by pass mode?
            prAoutHal->rHwIf.CfgUpd(prAoutHal, prAoutParam);
        }
        else
        {
            LOG(LOG_FAIL, _T("AudAout_PathSet Aout1 out error"));
            fgRet = FALSE;
        }
    }
    return (fgRet);
}


/**
 * set front/rear dac type
 *
 * @param [in]  pDacType : Dac type
 * @param [out] 
 *
 * @return
 */

bool AudAout_DacTypeSet(AUD_DAC_TYPE_SEL_T *pDacType)
{   
    bool fgDacChange = FALSE;
    PAOUT_HAL_CLS_PUB prAoutHal = NULL;
    PAOUT_EXTPARAMS_T prAoutParam = NULL;
    
    ASSERT((pDacType->eOut == AUD_FRONT) || (pDacType->eOut == AUD_REAR));
    
    if(AUD_FRONT == pDacType->eOut)
    {        
        prAoutHal = AudAout_GetHalClass(AUDID_AOUT1);
        prAoutParam = AudAout_GetHalParam(AUDID_AOUT1);
        if(AOUT_FS == prAoutParam->eOutPath)
        {
            if(prAoutParam->eDacType != pDacType->eDacType)
			{
				prAoutParam->eDacType = pDacType->eDacType;
				prAoutHal->rHwIf.CfgUpd(prAoutHal, prAoutParam);
                fgDacChange = TRUE;
			}            
        }
        else
        {
            prAoutParam = AudAout_GetHalParam(AUDID_AOUT2);
            if(AOUT_FS == prAoutParam->eOutPath)
            {
                if(prAoutParam->eDacType != pDacType->eDacType)
                {
                    prAoutParam->eDacType = pDacType->eDacType;                    
                    prAoutHal = AudAout_GetHalClass(AUDID_AOUT2);
                    prAoutHal->rHwIf.CfgUpd(prAoutHal, prAoutParam);
                    fgDacChange = TRUE;
                }
            }
        }
    }
    else if(AUD_REAR == pDacType->eOut)
    {    
        prAoutHal = AudAout_GetHalClass(AUDID_AOUT2);
        prAoutParam = AudAout_GetHalParam(AUDID_AOUT2);
        if(AOUT_RS == prAoutParam->eOutPath)
        {
            if(prAoutParam->eDacType != pDacType->eDacType)
			{
				prAoutParam->eDacType = pDacType->eDacType;
				prAoutHal->rHwIf.CfgUpd(prAoutHal, prAoutParam);
                fgDacChange = TRUE;
			}            
        }
        else
        {
            prAoutParam = AudAout_GetHalParam(AUDID_AOUT1);
            if(AOUT_FS == prAoutParam->eOutPath)
            {
                if(prAoutParam->eDacType != pDacType->eDacType)
                {
                    prAoutParam->eDacType = pDacType->eDacType;                    
                    prAoutHal = AudAout_GetHalClass(AUDID_AOUT1);
                    prAoutHal->rHwIf.CfgUpd(prAoutHal, prAoutParam);
                    fgDacChange = TRUE;
                }
            }
        }    
        
    }
    else
    {
        LOG(LOG_FAIL, _T("AudAout_DacTypeSet error"));
    }
    
    if(FALSE == fgDacChange)
    {        
        LOG(LOG_DAC, _T(" Seat(%d) Dac type (%d) unchanged."),
            pDacType->eOut, pDacType->eDacType);
    }
    else
    {
        LOG(LOG_CTRLF, _T(" Seat(%d) Dac change to type(%d) ."),
            pDacType->eOut, pDacType->eDacType);
    }

	return (TRUE);
}


/**
 * set rear bypass mode
 *
 * @param [in]  fgByPass : true/false
 * @param [out] 
 *
 * @return
 */

void AudAout_ByPassMode(AUD_AOUT_DEVID eAoutId, bool fgByPass)
{
    PAOUT_HAL_CLS_PUB prAoutHal = AudAout_GetHalClass(eAoutId);
    PAOUT_EXTPARAMS_T prAoutParam = AudAout_GetHalParam(eAoutId);
    AUD_OUT_PATH_T ePath = AOUT_FS;
	
    if(AUDID_AOUT1 == eAoutId)
    {
        ePath = AOUT_FS;
    }
    else if(AUDID_AOUT2 == eAoutId)
    {	
        ePath = AOUT_RS;
    }

    if(NULL == prAoutHal)
    {
        LOG(LOG_FAIL, _T("[ByPass] get Aout Cls NULL.\r\n"));
        return;
    }

    //check aout2 go bypass
    if(ePath == prAoutParam->eOutPath)
    {
        if(prAoutParam->fgAdcBypasMode != fgByPass)
        {
            prAoutParam->fgAdcBypasMode = fgByPass;
            prAoutHal->rHwIf.CfgUpd(prAoutHal, prAoutParam);
        }
    }   
}



void AudAout_APLLA1Sel(void)
{
    CKGEN_SETBITS(0x2C, 0x1000);//SEL_APLL_A1 as Apll2
}


#define AUD_REG_GPIO_EN5  0x88
#define AUD_REG_GPIO_OUT5 0xf4

void vAudMuteCircuitCtrl(bool fgMute)
{
    //first config GOIO162 GPIO FUNCTION
    CKGEN_CLRBITS(AUD_REG_PAD_MUX1, 0x3<<28);  //0x58[28:29] =0
    CKGEN_CLRBITS(AUD_REG_PAD_MUX4, 0x3<<2);  //0x64[2:3] = 0

     //output
    CKGEN_SETBITS(AUD_REG_GPIO_EN5, 0x1<<2);  //0x88[2] = 1

    if(TRUE == fgMute)
    {
        CKGEN_CLRBITS(AUD_REG_GPIO_OUT5, 0x1<<2);  //0xf4[2] = 0
    }
    else
    {
        CKGEN_SETBITS(AUD_REG_GPIO_OUT5, 0x1<<2);  //0xf4[2] = 1
    }
}

void AudPower_Init(void)
{
    Aud_Linein_Init();
    //AudAout_Init(AUD_CLK_AOUT1, FS_48K);
    //AudAout_Init(AUD_CLK_AOUT2, FS_48K);
}

void AudPower_Deinit(void)
{
    Aud_Linein_DeInit();
    AudAout_DeInit(AUD_CLK_AOUT1);
    AudAout_DeInit(AUD_CLK_AOUT2);
}

void AudPower_ErrRecover_Init(void)
{
    Aud_Linein_Init();
    AudAout_Init(AUD_CLK_AOUT1, FS_48K);
    AudAout_Init(AUD_CLK_AOUT2, FS_48K);
}

