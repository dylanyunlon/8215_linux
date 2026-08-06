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


/**
 * @file aud_io_clock.c source file
 * 
 * aud io clock module driver
 * 
 * @author qiuhua.yin@autochips.com
 * 
 */

#include "aud_io_clock.h"
#include "aud_debug.h"


//==============================================//
#define CodeSight_IoClk_cmm_fun
//==============================================//

u32 MCLK_TO_FS_RATIO[AUD_MCLK_TYPE_MAX]=
{
    128, //MCLK_128FS
    192, //MCLK_192FS
    256, //MCLK_256FS
    384, //MCLK_384FS
    512, //MCLK_512FS
    768, //MCLK_768FS
    1024, //MCLK_1024FS
};

u32 FS_TYPE_TO_VALUE[FS_UNKNOWN + 1]=
{
    16000,//FS_16K,
    22050,//FS_22K,
    24000,//FS_24K,    
    32000,//FS_32K,     
    44100,//FS_44K,     
    48000,//FS_48K,     
    64000,//FS_64K,     
    88200,//FS_88K,     
    96000,//FS_96K,     
    176000,//FS_176K,   
    192000,//FS_192K,   
    44100,//FS512_44K,//for DSD
    768000,//FS_768K,
    44100,//FS128_44k,
    8000,//FS_8K, //for 8K flow.
    0,//FS_UNKNOWN
};

typedef struct
{
    u32 u4MclkDiv;
    AUD_CKGEN_APLL eApll;
}AUD_MCLK_INFO_T, *PAUD_MCLK_INFO_T;

typedef struct
{
    u8 u1BitCkgen;
    u8 u1BitRgbk2;
}AUD_CLK_PWCTL_T, *PAUD_CLK_PWCTL_T;

#define CLK_PWCTL_INVALID   0x20
AUD_CLK_PWCTL_T AUDCLK_PWCTL_CKGEN_BIT[CLKPM_MAX] =
{
    {BIT_STR_MP_MLIN_MCLK_PD,       BIT_STR_MPHONE_CLK_PDN},//CLKPM_MPHONE
    {BIT_STR_MP_MLIN_MCLK_PD,       BIT_STR_MLIN_CLK_PDN},//CLKPM_MLIN
    {BIT_STR_MLIN2_MCLK_PD,         CLK_PWCTL_INVALID},//CLKPM_MLIN2
    {BIT_STR_AUD_IEC_CLK_PD,        BIT_STR_IEC_CLK_PDN},//CLKPM_IEC
    {BIT_STR_AUD_IEC_CLK_PD,        CLK_PWCTL_INVALID},//CLKPM_AUD
    {BIT_STR_AUD2_CLK_PD,           CLK_PWCTL_INVALID},//CLKPM_AUD2
    {CLK_PWCTL_INVALID,             BIT_STR_APLL_ADJ_CLK_PDN},//CLKPM_APLL_ADJ
    {CLK_PWCTL_INVALID,             BIT_STR_APLL2_ADJ_CLK_PDN},//CLKPM_APLL2_ADJ
    {CLK_PWCTL_INVALID,             BIT_STR_STC_BCK_PDN},//CLKPM_STC_RISC
    {CLK_PWCTL_INVALID,             BIT_STR_CLK_AXI_PDN},//CLKPM_AXI
    {BIT_STR_APLL_CLK_AP_ASRC_PD,   CLK_PWCTL_INVALID},//CLKPM_AP_ASRC
    {BIT_STR_APLL_CLK_GPS_ASRC_PD,  CLK_PWCTL_INVALID},//CLKPM_GPS_ASRC
    {BIT_STR_AFE_26M_CLK_PD,        BIT_STR_AFE1_26M_CLK_PDN},//CLKPM_AFE1_26M
    {BIT_STR_AFE_26M_CLK_PD,        BIT_STR_AFE2_26M_CLK_PDN},//CLKPM_AFE2_26M
    {BIT_STR_FS_APLL_EN, CLK_PWCTL_INVALID}, //CLKPM_FS_APLL
    {BIT_STR_RS_APLL_EN, CLK_PWCTL_INVALID}, //CLKPM_RS_APLL
};


/**
 * function : transfer mclk type to mclk to fs ratio
 *
 * @param [in]  eMclkType : mlck type
 * @param [out] 
 *
 * @return  ratio(MCLK/FS)
 */
u32 IoClk_GetMclkToFsRatio(MCLK_TYPE_T eMclkType)
{
    return (MCLK_TO_FS_RATIO[eMclkType]);
}

/**
 * function : transfer fs type to value
 *
 * @param [in]  eFs : fs type
 * @param [out] 
 *
 * @return  fs value
 */
u32 IoClk_GetFsValue(AUDIO_SAMPLING_T eFs)
{
    return (FS_TYPE_TO_VALUE[eFs]);
}

/**
 * function : Get apll type
 *
 * @param [in]  eFs : Fs Type
 * @param [out] 
 *
 * @return Apll Type
 */
AUD_CKGEN_APLL IoClk_GetApllType(AUDIO_SAMPLING_T eFs)
{
    u32 u4Fs = IoClk_GetFsValue(eFs);

    AUD_CKGEN_APLL eApll  = (0 == (u4Fs % 11025)) ? CKGEN_APPL1 : CKGEN_APLL2;

    return eApll;
}

/**
 * function : Get mclk info
 *
 * @param [in]  eMclkType : mlck type, eFs : Fs Type, prMclkInfo: pointer to mclk info var
 * @param [out] 
 *
 * @return
 */
void IoClk_GetMclkInfo(MCLK_TYPE_T eMclkType, AUDIO_SAMPLING_T eFs, PAUD_MCLK_INFO_T prMclkInfo)
{
    u32 u4Apll, u4ApllDiv;

    u32 u4Mclk = IoClk_GetMclkToFsRatio(eMclkType);
    u32 u4Fs = IoClk_GetFsValue(eFs);
    
    if ((0 == u4Fs) || (0 == u4Mclk))
    {
        LOG(LOG_FAIL, _T("IoClk_GetMclkInfo Error, u4Fs = 0x%x, u4Mclk=0x%x.\r\n"), u4Fs, u4Mclk);
        return;
    }

    prMclkInfo->eApll  = (0 == (u4Fs % 11025)) ? CKGEN_APPL1 : CKGEN_APLL2;
    u4Apll = (CKGEN_APPL1 == prMclkInfo->eApll) ? APLL1_VALUE : APLL2_VALUE;
    u4ApllDiv = u4Apll / u4Mclk / u4Fs;

    prMclkInfo->u4MclkDiv = u4ApllDiv - 1;
}

//==============================================//
#define CodeSight_IoClk_Mphon_Fun
//==============================================//

/**
 * function : set mphone mclk info
 *
 * @param [in]  eMclkType : mlck type, eFs : Fs Type
 * @param [out] 
 *
 * @return
 */
void IoClk_SetMphMclk(MCLK_TYPE_T eMclkType, AUDIO_SAMPLING_T eFs)
{
    AUD_MCLK_INFO_T rMphMclkInfo;

    IoClk_GetMclkInfo(eMclkType, eFs, &rMphMclkInfo);
    
    //select k6 as mphone mclk source
    AUDREG_BITS_W(AUD_REG_AP_REG3, BIT_STR_MPHONE_AP_SEL, BIT_NUM_MPHONE_AP_SEL, MPHONE_ACK_K6);
  
    //select apll for k6
    AUDREG_BITS_W(AUD_REG_AP_REG7, BIT_STR_SEL_APLL_K6, BIT_NUM_SEL_APLL_K6, rMphMclkInfo.eApll);
  
    //set mclk divider
    AUDREG_BITS_W(AUD_REG_AP_REG5, BIT_STR_CFG_REG_K6, BIT_NUM_CFG_REG_K6, rMphMclkInfo.u4MclkDiv);
}

/**
 * function : set digital mic clk info
 *
 * @param [in]
 * @param [out] 
 *
 * @return
 */
void IoClk_SetDigMphClk(void)
{
    //select apll for k6
    AUDREG_BITS_W(AUD_REG_AP_REG8, BIT_STR_SEL_APLL_K14, BIT_NUM_SEL_APLL_K14, K4_SEL_26M_APLL);
  
    //set mclk divider
    AUDREG_BITS_W(AUD_REG_AP_REG7, BIT_STR_CFG_REG_K14, BIT_NUM_CFG_REG_K14, 0);
}


//==============================================//
#define CodeSight_IoClk_Adc_Fun
//==============================================//


/**
 * function : config 26M APll clock for ADC ????
 *
 * @param [in]
 * @param [out] 
 *
 * @return
 */

void IoClk_Set26mApll(void)
{

}


/**
 * function : set adc clk info  (6.5M)
 *
 * @param [in]
 * @param [out] 
 *
 * @return
 */
void IoClk_SetAdcClk(void)
{
    //select k10 as adc clk source
    AUDREG_BITS_W(AUD_REG_AP_REG8, BIT_STR_ADC_SEL, BIT_NUM_ADC_SEL, ADC_ACK_K10);

    //select apll for k10
    AUDREG_BITS_W(AUD_REG_AP_REG8, BIT_STR_SEL_APLL_K10, BIT_NUM_SEL_APLL_K10, K10_SEL_26M_APLL);
  
    //set mclk divider
    AUDREG_BITS_W(AUD_REG_AP_REG0, BIT_STR_CFG_REG_K10, BIT_NUM_CFG_REG_K10, 3);
}


//==============================================//
#define CodeSight_IoClk_Pcm_Fun
//==============================================//
/**
 * function : set pcm mclk info
 *
 * @param [in]  eSyncCycle : 32/64 Fs
 * @param [out] 
 *
 * @return
 */
void IoClk_SetPcmMclk(AUD_PCM_SYNC_CYCLE eSyncCycle, u32 u4SampleRate)
{
    u32 u4Divider;

    u4Divider = (PCM_CLK_CYCLE_32 == eSyncCycle) ? 32 : 64;
    u4Divider *= u4SampleRate;

    u4Divider = APLL2_VALUE / u4Divider - 1;
  
    //select k9 as pcm mclk
    AUDREG_BITS_W(AUD_REG_AP_REG3, BIT_STR_BT_PCM_SEL, BIT_NUM_BT_PCM_SEL, BT_PCM_ACK_K9);
  
    //select apll for k9
    AUDREG_BITS_W(AUD_REG_AP_REG8, BIT_STR_SEL_APLL_K9, BIT_NUM_SEL_APLL_K9, CKGEN_APLL2);
  
    //set divider for pcm clk
    AUDREG_BITS_W(AUD_REG_AP_REG6, BIT_STR_CFG_REG_K9, BIT_NUM_CFG_REG_K9, u4Divider);
}

//==============================================//
#define CodeSight_IoClk_Lin_Fun
//==============================================//

/**
 * function : set line in mclk info
 *
 * @param [in]  eMclkType : mlck type, eFs : Fs Type
 * @param [out] 
 *
 * @return
 */
void IoClk_SetLinMclk(MCLK_TYPE_T eMclkType, AUDIO_SAMPLING_T eFs)
{
    AUD_MCLK_INFO_T rLinMclkInfo;

    IoClk_GetMclkInfo(eMclkType, eFs, &rLinMclkInfo);

    //select k3 as line in mclk source
    AUDREG_BITS_W(AUD_REG_AP_REG8, BIT_STR_MLIN_SEL, BIT_NUM_MLIN_SEL, MLIN_ACK_K3);
  
    //select apll for k3
    AUDREG_BITS_W(AUD_REG_AP_REG7, BIT_STR_SEL_APLL_K3, BIT_NUM_SEL_APLL_K3, rLinMclkInfo.eApll);
  
    //set mclk divider
    AUDREG_BITS_W(AUD_REG_AP_REG4, BIT_STR_CFG_REG_K3, BIT_NUM_CFG_REG_K3, rLinMclkInfo.u4MclkDiv);
}

/**
 * function : set line in2 mclk info
 *
 * @param [in]  eMclkType : mlck type, eFs : Fs Type
 * @param [out] 
 *
 * @return
 */
void IoClk_SetLin2Mclk(MCLK_TYPE_T eMclkType, AUDIO_SAMPLING_T eFs)
{
    AUD_MCLK_INFO_T rLin2MclkInfo;

    IoClk_GetMclkInfo(eMclkType, eFs, &rLin2MclkInfo);

    //select k3 as line in mclk source
    AUDREG_BITS_W(AUD_REG_AP_REG8, BIT_STR_MLIN2_SEL, BIT_NUM_MLIN2_SEL, MLIN2_ACK_K12);
  
    //select apll for k12
    AUDREG_BITS_W(AUD_REG_AP_REG8, BIT_STR_SEL_APLL_K12, BIT_NUM_SEL_APLL_K12, rLin2MclkInfo.eApll);
  
    //set mclk divider
    AUDREG_BITS_W(AUD_REG_AP_REG6, BIT_STR_CFG_REG_K12, BIT_NUM_CFG_REG_K12, rLin2MclkInfo.u4MclkDiv);
}


//==============================================//
#define CodeSight_IoClk_Aout_Fun
//==============================================//

/**
 * function : set aout1 & Iec mclk info
 *
 * @param [in]  eMclkType : aout mclk type, eAoutFs : Aout Fs Type, eIecFs : IEC Fs Type
 * @param [out] 
 *
 * @return
 * 
 * Note: !!! Cause IEC Follow Aout1, so need to set mclk of iec and aout1 at the same time
 */
void IoClk_SetAout1IecMclk(MCLK_TYPE_T eAoutMclkType, AUDIO_SAMPLING_T eAoutFs, AUDIO_SAMPLING_T eIecFs)
{
    AUD_MCLK_INFO_T rAout1MclkInfo, rIecClkInfo;
    u32 u4Aout1IecMclkDiv;

    IoClk_GetMclkInfo(eAoutMclkType, eAoutFs, &rAout1MclkInfo);
    IoClk_GetMclkInfo(AUD_MCLK_128FS, eIecFs, &rIecClkInfo);

    AUDREG_BITS_W(AUD_REG_AP_REG4, BIT_STR_CFG_REG_K1, (BIT_NUM_CFG_REG_K1 + BIT_NUM_CFG_REG_K2), 0);

    //select k2 as aout mclk source
    AUDREG_BITS_W(AUD_REG_AP_REG2, BIT_STR_AUD_AP_SEL, BIT_NUM_AUD_AP_SEL, AUD_AP_ACLK_K2);
  
    //select apll for k2
    AUDREG_BITS_W(AUD_REG_AP_REG7, BIT_STR_SEL_APLL_K2, BIT_NUM_SEL_APLL_K2, rAout1MclkInfo.eApll);
  
    //iec clcok is from K1, select apll for k1
    AUDREG_BITS_W(AUD_REG_AP_REG7, BIT_STR_SEL_APLL_K1, BIT_NUM_SEL_APLL_K1, rIecClkInfo.eApll);

    u4Aout1IecMclkDiv = rIecClkInfo.u4MclkDiv | (rAout1MclkInfo.u4MclkDiv << 8);
      
    //set aout1 & IEC mclk divider
    AUDREG_BITS_W(AUD_REG_AP_REG4, BIT_STR_CFG_REG_K1, (BIT_NUM_CFG_REG_K1 + BIT_NUM_CFG_REG_K2), u4Aout1IecMclkDiv);
}

/**
 * function : set aout1 mclk info
 *
 * @param [in]  eMclkType : mlck type, eFs : Fs Type
 * @param [out] 
 *
 * @return
 */
void IoClk_SetAout1Mclk(MCLK_TYPE_T eMclkType, AUDIO_SAMPLING_T eFs)
{
    AUD_MCLK_INFO_T rAout1MclkInfo;

    IoClk_GetMclkInfo(eMclkType, eFs, &rAout1MclkInfo);

    //select k2 as aout mclk source
    AUDREG_BITS_W(AUD_REG_AP_REG2, BIT_STR_AUD_AP_SEL, BIT_NUM_AUD_AP_SEL, AUD_AP_ACLK_K2);
  
    //select apll for k2
    AUDREG_BITS_W(AUD_REG_AP_REG7, BIT_STR_SEL_APLL_K2, BIT_NUM_SEL_APLL_K2, rAout1MclkInfo.eApll);
  
    //set mclk divider
    AUDREG_BITS_W(AUD_REG_AP_REG4, BIT_STR_CFG_REG_K2, BIT_NUM_CFG_REG_K2, rAout1MclkInfo.u4MclkDiv);
}


/**
 * function : set aout2 mclk info
 *
 * @param [in]  eMclkType : mlck type, eFs : Fs Type
 * @param [out] 
 *
 * @return
 */
void IoClk_SetAout2Mclk(MCLK_TYPE_T eMclkType, AUDIO_SAMPLING_T eFs)
{
    AUD_MCLK_INFO_T rAout2MclkInfo;

    IoClk_GetMclkInfo(eMclkType, eFs, &rAout2MclkInfo);

    //select k4 as aout2 mclk source
    AUDREG_BITS_W(AUD_REG_AP_REG3, BIT_STR_AUD2_AP_SEL, BIT_NUM_AUD2_AP_SEL, AUD2_AP_ACLK_K4);
  
    //select apll for k4
    AUDREG_BITS_W(AUD_REG_AP_REG7, BIT_STR_SEL_APLL_K4, BIT_NUM_SEL_APLL_K4, rAout2MclkInfo.eApll);
  
    //set mclk divider
    AUDREG_BITS_W(AUD_REG_AP_REG4, BIT_STR_CFG_REG_K4, BIT_NUM_CFG_REG_K4, rAout2MclkInfo.u4MclkDiv);
}


//==============================================//
#define CodeSight_IoClk_Iec_Fun
//==============================================//
/**
 * function : set iec clock info
 *
 * @param [in]  eMclkType : mlck type, eFs : Fs Type
 * @param [out] 
 *
 * @return
 */
void IoClk_SetIecClk(MCLK_TYPE_T eMclkType, AUDIO_SAMPLING_T eFs)
{
    AUD_MCLK_INFO_T rIecClkInfo;

    IoClk_GetMclkInfo(eMclkType, eFs, &rIecClkInfo);

    //iec clcok is from K1, select apll for k1
    AUDREG_BITS_W(AUD_REG_AP_REG7, BIT_STR_SEL_APLL_K1, BIT_NUM_SEL_APLL_K1, rIecClkInfo.eApll);
      
    //set mclk divider
    AUDREG_BITS_W(AUD_REG_AP_REG4, BIT_STR_CFG_REG_K1, BIT_NUM_CFG_REG_K1, rIecClkInfo.u4MclkDiv);
}


//==============================================//
#define CodeSight_IoClk_Dvd_Aud_Fun
//==============================================//
/**
 * function : set dvd aout clock info
 *
 * @param [in]  eMclkType : mlck type, eFs : Fs Type
 * @param [out] 
 *
 * @return
 */
void IoClk_SetDvdAoutClk(MCLK_TYPE_T eMclkType, AUDIO_SAMPLING_T eFs)
{
    AUD_MCLK_INFO_T rDvdAudClkInfo;

    IoClk_GetMclkInfo(eMclkType, eFs, &rDvdAudClkInfo);

    //select k7 as dvd aout mclk source
    AUDREG_BITS_W(AUD_REG_DVD_REG1, BIT_STR_AUD_DVD_SEL, BIT_NUM_AUD_DVD_SEL, AUD_DVD_K7);

    //select apll for k7
    AUDREG_BITS_W(AUD_REG_AP_REG7, BIT_STR_SEL_APLL_K7, BIT_NUM_SEL_APLL_K7, rDvdAudClkInfo.eApll);

    //set mclk divider
    AUDREG_BITS_W(AUD_REG_AP_REG5, BIT_STR_CFG_REG_K7, BIT_NUM_CFG_REG_K7, rDvdAudClkInfo.u4MclkDiv);
}


//==============================================//
#define CodeSight_IoClk_AsrcFun
//==============================================//
/**
 * function : set ap asrc calibration clock info
 *
 * @param [in]  eMclkType : mlck type, eFs : Fs Type
 * @param [out] 
 *
 * @return
 */
void IoClk_SetApAsrcCliClk(MCLK_TYPE_T eMclkType, AUDIO_SAMPLING_T eFs)
{
    AUD_MCLK_INFO_T rApAsrcClkInfo;

    IoClk_GetMclkInfo(eMclkType, eFs, &rApAsrcClkInfo);

    //select A2 as dvd aout mclk source
    AUDREG_BITS_W(AUD_REG_AP_REG9, BIT_STR_AP_ASRC_CLI_SEL, BIT_NUM_AP_ASRC_CLI_SEL, AP_ASRC_CLI_ACLK_A2);

    //select apll for A2
    AUDREG_BITS_W(AUD_REG_AP_REG8, BIT_STR_SEL_APLL_A2, BIT_NUM_SEL_APLL_A2, rApAsrcClkInfo.eApll);

    //set mclk divider
    AUDREG_BITS_W(AUD_REG_AP_REG6, BIT_STR_CFG_REG_A2, BIT_NUM_CFG_REG_A2, 0);
}

/**
 * function : set gps asrc calibration clock info
 *
 * @param [in]  eMclkType : mlck type, eFs : Fs Type
 * @param [out] 
 *
 * @return
 */
void IoClk_SetGpsAsrcCliClk(MCLK_TYPE_T eMclkType, AUDIO_SAMPLING_T eFs)
{
    AUD_MCLK_INFO_T rGpsAsrcClkInfo;

    IoClk_GetMclkInfo(eMclkType, eFs, &rGpsAsrcClkInfo);

    //select A1 as dvd aout mclk source
    AUDREG_BITS_W(AUD_REG_AP_REG9, BIT_STR_GPS_ASRC_CLI_SEL, BIT_NUM_GPS_ASRC_CLI_SEL, AP_ASRC_CLI_ACLK_A1);

    //select apll for A1
    AUDREG_BITS_W(AUD_REG_AP_REG8, BIT_STR_SEL_APLL_A1, BIT_NUM_SEL_APLL_A1, rGpsAsrcClkInfo.eApll);

    //set mclk divider
    AUDREG_BITS_W(AUD_REG_AP_REG6, BIT_STR_CFG_REG_A1, BIT_NUM_CFG_REG_A1, 0);
}


//==============================================//
#define CodeSight_IoClk_PowerCtl_Fun
//==============================================//

/**
 * function : set adsp clock power on
 *
 * @param [in]
 * @param [out] 
 *
 * @return
 */
void IoClk_SetAdspPowerOn(void)
{    
    AUD_CKGEN_SETBITS(AUD_REG_CLKGATE_CFG3, (ADSPA_CLK_PD | ADSPB_CLK_PD));
}

/**
 * function : set adsp clock power down
 *
 * @param [in]
 * @param [out] 
 *
 * @return
 */
void IoClk_SetAdspPowerDown(void)
{
    AUD_CKGEN_CLRBITS(AUD_REG_CLKGATE_CFG3, (ADSPA_CLK_PD | ADSPB_CLK_PD));
}

/**
 * function : set io module clock power on
 *
 * @param [in]  eClkId : clock id
 * @param [out] 
 *
 * @return
 */
void IoClk_SetModulePowerOn(AUD_CLK_POWER_CTL_MODULE_ID eClkId)
{
    u8 u1BitCkgenStart = AUDCLK_PWCTL_CKGEN_BIT[eClkId].u1BitCkgen;
    u8 u1BitRgbk2Start = AUDCLK_PWCTL_CKGEN_BIT[eClkId].u1BitRgbk2;

    if (CLK_PWCTL_INVALID != u1BitRgbk2Start)
    {
        AUDREG_BITS_W(REGENV_RGBK2_CFG1, u1BitRgbk2Start, 1, 0);
    }
    
    if (CLK_PWCTL_INVALID != u1BitCkgenStart)
    {
        AUDREG_BITS_W(AUD_REG_CLKGATE_CFG3, u1BitCkgenStart, 1, 1);
    }
}

/**
 * function : set io module clock power down
 *
 * @param [in]  eClkId : clock id
 * @param [out] 
 *
 * @return
 */
void IoClk_SetModulePowerDown(AUD_CLK_POWER_CTL_MODULE_ID eClkId)
{
    u8 u1BitCkgenStart = AUDCLK_PWCTL_CKGEN_BIT[eClkId].u1BitCkgen;
    u8 u1BitRgbk2Start = AUDCLK_PWCTL_CKGEN_BIT[eClkId].u1BitRgbk2;
    
    if (CLK_PWCTL_INVALID != u1BitRgbk2Start)
    {
        AUDREG_BITS_W(REGENV_RGBK2_CFG1, u1BitRgbk2Start, 1, 1);
    }
    else if (CLK_PWCTL_INVALID != u1BitCkgenStart)
    {
        AUDREG_BITS_W(AUD_REG_CLKGATE_CFG3, u1BitCkgenStart, 1, 0);
    }
}

//==============================================//
#define CodeSight_IoClk_HwRest_Fun
//==============================================//

/**
 * function : dsp hw rest
 *
 * @param [in]
 * @param [out]
 *
 * @return
 */
void IoClk_SetDspHwRest(void)
{
    AUDREG_BITS_W(AUD_REG_SYNC_RESET_CFG3, BIT_STR_DSPA_RESET, 2, 0);
    //AudUtil_Delayus(1);
    Sleep(1);
    AUDREG_BITS_W(AUD_REG_SYNC_RESET_CFG3, BIT_STR_DSPA_RESET, 2, 3);
}

/**
 * function : pwm dac hw rest 
 *
 * @param [in]
 * @param [out]
 *
 * @return
 */
void IoClk_SetPwmHwRest(void)
{
    AUDREG_BITS_W(AUD_REG_SYNC_RESET_CFG3, BIT_STR_FS_PWMIP_RESET, 2, 0);
    Sleep(1);
    AUDREG_BITS_W(AUD_REG_SYNC_RESET_CFG3, BIT_STR_FS_PWMIP_RESET, 2, 3);
}


//==============================================//
#define CodeSight_IoClk_AsrcCli_Fun
//==============================================//

/**
 * function : Asrc auto trace mode clibration sig0 config
 *
 * @param [in]  eSig0Src : src clock can select
 * @param [out]
 *
 * @return
 */
void IoClk_SetAsrcCliSig0Src(AUD_ASRC_CLI_SIG0_SRC eSig0Src)
{
     AUDREG_BITS_W(REGENV_RGBK2_CFG5, BIT_STR_ASRC_CALI_SIG0_SEL, BIT_NUM_ASRC_CALI_SIG0_SEL, eSig0Src);
}

/**
 * function : Asrc auto trace mode clibration sig1 config
 *
 * @param [in]  eSig1Src : src clock can select
 * @param [out]
 *
 * @return
 */
void IoClk_SetAsrcCliSig1Src(AUD_ASRC_CLI_SIG1_SRC eSig1Src)
{
     AUDREG_BITS_W(REGENV_RGBK2_CFG5, BIT_STR_ASRC_CALI_SIG1_SEL, BIT_NUM_ASRC_CALI_SIG1_SEL, eSig1Src);
}

/**
 * function : Asrc auto trace mode clibration sig2 config
 *
 * @param [in]  eSig2Src : src clock can select
 * @param [out]
 *
 * @return
 */
void IoClk_SetAsrcCliSig2Src(AUD_ASRC_CLI_SIG2_SRC eSig2Src)
{
     AUDREG_BITS_W(REGENV_RGBK2_CFG5, BIT_STR_ASRC_CALI_SIG2_SEL, BIT_NUM_ASRC_CALI_SIG2_SEL, eSig2Src);
}

/**
 * function : Asrc auto trace mode clibration sig3 config
 *
 * @param [in]  eSig3Src : src clock can select
 * @param [out]
 *
 * @return
 */
void IoClk_SetAsrcCliSig3Src(AUD_ASRC_CLI_SIG3_SRC eSig3Src)
{
     AUDREG_BITS_W(REGENV_RGBK2_CFG5, BIT_STR_ASRC_CALI_SIG3_SEL, BIT_NUM_ASRC_CALI_SIG3_SEL, eSig3Src);
}

/**
 * function : Asrc auto trace mode clibration sig4 config
 *
 * @param [in]  eSig4Src : src clock can select
 * @param [out]
 *
 * @return
 */
void IoClk_SetAsrcCliSig4Src(AUD_ASRC_CLI_SIG4_SRC eSig4Src)
{
     AUDREG_BITS_W(REGENV_RGBK2_CFG5, BIT_STR_ASRC_CALI_SIG4_SEL, BIT_NUM_ASRC_CALI_SIG4_SEL, eSig4Src);
}


