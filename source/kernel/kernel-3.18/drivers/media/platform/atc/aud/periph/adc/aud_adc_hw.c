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
 * @file aud_adc_hw.c source file
 * 
 * aud io adc module hardware driver
 * 
 * @author qiuhua.yin@autochips.com
 * 
 */

#include "aud_adc_hal.h"
#include "aud_reg_env.h"
#include "aud_reg_rgbk2.h"
#include "aud_reg_adc.h"
#include "aud_io_clock_if.h"

typedef struct 
{
    ADC_HW_CLS_PUB rPub;
    
    AUD_ADC_ID eAdcId;

    AUD_ADC_INPUT_SRC eInput;

}ADC_HW_CLS, *PADC_HW_CLS;

typedef enum
{
    ADC_REG_PREAMPLON,
    ADC_REG_PREAMPRON,
    ADC_REG_PREAMPLINPUTSEL,
    ADC_REG_PREAMPRINPUTSEL,
    ADC_REG_ADCLPWRUP,
    ADC_REG_ADCRPWRUP,
    ADC_REG_ADCLINPUTSEL,
    ADC_REG_ADCRINPUTSEL,
    ADC_REG_PWDB_MBIAS,
    ADC_REG_LSBUFLGAIN,
    ADC_REG_LSBUFRGAIN,
    ADC_REG_LSBUFLPWRUP,
    ADC_REG_LSBUFRPWRUP,
    ADC_REG_LSBUFLINPUTSEL,
    ADC_REG_LSBUFRINPUTSEL,
    ADC_REG_AFE_BCK_SEL,
    ADC_REG_AFE_LRCK_SEL,
    ADC_REG_AFE_LIN2_CLK_SEL,
    ADC_REG_FL_LSBUF_CLAMP,
    ADC_REG_FR_LSBUF_CLAMP,
    ADC_REG_IDX_MAX,
}AOUT_CONFIG_REG_IDX;


static AUD_IO_REG_CTL aAdcConfigReg[AUD_ADC_MAX - 1][ADC_REG_IDX_MAX] = 
{
    {
        {AUD_REG_ADC_CFG0, BIT_STR_PREAMPL1ON, BIT_NUM_PREAMPL1ON, "ADC1: PREAMPL1ON"},
        {AUD_REG_ADC_CFG0, BIT_STR_PREAMPR1ON, BIT_NUM_PREAMPR1ON, "ADC1: PREAMPR1ON"},
        {AUD_REG_ADC_CFG0, BIT_STR_PREAMPLINPUTSEL1, BIT_NUM_PREAMPLINPUTSEL1, "ADC1: PREAMPLINPUTSEL1"},
        {AUD_REG_ADC_CFG0, BIT_STR_PREAMPRINPUTSEL1, BIT_NUM_PREAMPRINPUTSEL1, "ADC1: PREAMPRINPUTSEL1"},  
        {AUD_REG_ADC_CFG1, BIT_STR_ADCLPWRUP, BIT_NUM_ADCLPWRUP, "ADC1: ADCLPWRUP"},
        {AUD_REG_ADC_CFG1, BIT_STR_ADCRPWRUP, BIT_NUM_ADCRPWRUP, "ADC1: ADCRPWRUP"}, 
        {AUD_REG_ADC_CFG3, BIT_STR_ADCLINPUTSEL1, BIT_NUM_ADCLINPUTSEL1, "ADC1: ADCLINPUTSEL1"},
        {AUD_REG_ADC_CFG3, BIT_STR_ADCRINPUTSEL1, BIT_NUM_ADCRINPUTSEL1, "ADC1: ADCRINPUTSEL1"}, 
        {AUD_REG_ADC_CFG7, BIT_STR_PWDB_MBIAS1, BIT_NUM_PWDB_MBIAS1, "ADC1: PWDB_MBIAS1"}, 
        {AUD_REG_ADC_CFG10, BIT_STR_LSBUFLGAIN1, BIT_NUM_LSBUFLGAIN1, "ADC1: LSBUFLGAIN1"},
        {AUD_REG_ADC_CFG10, BIT_STR_LSBUFRGAIN1, BIT_NUM_LSBUFRGAIN1, "ADC1: LSBUFRGAIN1"}, 
        {AUD_REG_ADC_CFG11, BIT_STR_LSBUFLPWRUP1, BIT_NUM_LSBUFLPWRUP1, "ADC1: LSBUFLPWRUP1"},
        {AUD_REG_ADC_CFG11, BIT_STR_LSBUFRPWRUP1, BIT_NUM_LSBUFRPWRUP1, "ADC1: LSBUFRPWRUP1"}, 
        {AUD_REG_ADC_CFG11, BIT_STR_LSBUFLINPUTSEL1, BIT_NUM_LSBUFLINPUTSEL1, "ADC1: LSBUFLINPUTSEL1"},
        {AUD_REG_ADC_CFG11, BIT_STR_LSBUFRINPUTSEL1, BIT_NUM_LSBUFRINPUTSEL1, "ADC1: LSBUFRINPUTSEL1"},
        {REGENV_AFE_TOP_CFG0, BIT_STR_AFE1_BCK_SEL, BIT_NUM_AFE1_BCK_SEL, "AFE1: Bck select"},
        {REGENV_AFE_TOP_CFG0, BIT_STR_AFE1_LRCK_SEL, BIT_NUM_AFE1_LRCK_SEL, "AFE1: Lrck select"},
        {REGENV_AFE_TOP_CFG0, BIT_STR_AFE1_CLK_USE_LIN2, BIT_NUM_AFE1_CLK_USE_LIN2, "AFE1: Lin2 CLK select"},
        {AUD_REG_ADC_CFG5, BIT_STR_ADCLSBUFLCLAMP1_EN, BIT_NUM_ADCLSBUFLCLAMP1_EN, "ADC1: ADC_REG_FL_LSBUF_CLAMP"},
        {AUD_REG_ADC_CFG5, BIT_STR_ADCLSBUFRCLAMP1_EN, BIT_NUM_ADCLSBUFRCLAMP1_EN, "ADC1: ADC_REG_FR_LSBUF_CLAMP"}
    },
    {
        {AUD_REG_ADC_CFG0, BIT_STR_PREAMPL2ON, BIT_NUM_PREAMPL2ON, "ADC2: PREAMPL1ON"},
        {AUD_REG_ADC_CFG0, BIT_STR_PREAMPR2ON, BIT_NUM_PREAMPR2ON, "ADC2: PREAMPR2ON"},
        {AUD_REG_ADC_CFG0, BIT_STR_PREAMPLINPUTSEL2, BIT_NUM_PREAMPLINPUTSEL2, "ADC2: PREAMPLINPUTSEL2"},
        {AUD_REG_ADC_CFG0, BIT_STR_PREAMPRINPUTSEL2, BIT_NUM_PREAMPRINPUTSEL2, "ADC2: PREAMPRINPUTSEL2"},  
        {AUD_REG_ADC_CFG1, BIT_STR_ADCLPWRUP2, BIT_NUM_ADCLPWRUP2, "ADC2: ADCLPWRUP2"},
        {AUD_REG_ADC_CFG1, BIT_STR_ADCRPWRUP2, BIT_NUM_ADCRPWRUP2, "ADC2: ADCRPWRUP2"}, 
        {AUD_REG_ADC_CFG3, BIT_STR_ADCLINPUTSEL2, BIT_NUM_ADCLINPUTSEL2, "ADC2: ADCLINPUTSEL2"},
        {AUD_REG_ADC_CFG3, BIT_STR_ADCRINPUTSEL2, BIT_NUM_ADCRINPUTSEL2, "ADC2: ADCRINPUTSEL2"},
        {AUD_REG_ADC_CFG7, BIT_STR_PWDB_MBIAS1, BIT_NUM_PWDB_MBIAS1, "ADC2: PWDB_MBIAS2"}, 
        {AUD_REG_ADC_CFG10, BIT_STR_LSBUFLGAIN2, BIT_NUM_LSBUFLGAIN2, "ADC2: LSBUFLGAIN2"},
        {AUD_REG_ADC_CFG10, BIT_STR_LSBUFRGAIN2, BIT_NUM_LSBUFRGAIN2, "ADC2: LSBUFRGAIN2"}, 
        {AUD_REG_ADC_CFG11, BIT_STR_LSBUFLPWRUP2, BIT_NUM_LSBUFLPWRUP2, "ADC2: LSBUFLPWRUP2"},
        {AUD_REG_ADC_CFG11, BIT_STR_LSBUFRPWRUP2, BIT_NUM_LSBUFRPWRUP2, "ADC2: LSBUFRPWRUP2"}, 
        {AUD_REG_ADC_CFG11, BIT_STR_LSBUFLINPUTSEL2, BIT_NUM_LSBUFLINPUTSEL2, "ADC2: LSBUFLINPUTSEL2"},
        {AUD_REG_ADC_CFG11, BIT_STR_LSBUFRINPUTSEL2, BIT_NUM_LSBUFRINPUTSEL2, "ADC2: LSBUFRINPUTSEL2"},
        {REGENV_AFE_TOP_CFG0, BIT_STR_AFE2_BCK_SEL, BIT_NUM_AFE2_BCK_SEL, "AFE2: Bck select"},
        {REGENV_AFE_TOP_CFG0, BIT_STR_AFE2_LRCK_SEL, BIT_NUM_AFE2_LRCK_SEL, "AFE2: Lrck select"},
        {REGENV_AFE_TOP_CFG0, BIT_STR_AFE2_CLK_USE_LIN2, BIT_NUM_AFE2_CLK_USE_LIN2, "AFE2: Lin2 CLK select"},
        {AUD_REG_ADC_CFG5, BIT_STR_ADCLSBUFLCLAMP2_EN, BIT_NUM_ADCLSBUFLCLAMP2_EN, "ADC2: ADC_REG_FL_LSBUF_CLAMP"},
        {AUD_REG_ADC_CFG5, BIT_STR_ADCLSBUFRCLAMP2_EN, BIT_NUM_ADCLSBUFRCLAMP2_EN, "ADC2: ADC_REG_FR_LSBUF_CLAMP"}
    }
};

//macros for aout configure regosters read / write
#define ADC_REGCFG_BITS_W(AdcId, CfgRegId, val) \
    AUDREG_BITS_W((aAdcConfigReg[AdcId][CfgRegId].u4Adr), (aAdcConfigReg[AdcId][CfgRegId].u4BitStart), \
                  (aAdcConfigReg[AdcId][CfgRegId].u4BitNum), val)
#define ADC_REGCFG_BITS_R(AdcId, CfgRegId) \
    AUDREG_BITS_R((aAdcConfigReg[AdcId][CfgRegId].u4Adr), (aAdcConfigReg[AdcId][CfgRegId].u4BitStart),\
                  (aAdcConfigReg[AdcId][CfgRegId].u4BitNum))


//=========================================================//
    #define CodeSight_AdcHw_Static_Func
//=========================================================//


/**
 * set adc preamplifiter on/off
 *
 * @param [in]  prThis : hw class; fgOn : on/off
 * @param [out] 
 *
 * @return
 */
static void AdcHw_SetPreAmpOn(PADC_HW_CLS prThis, bool fgOn)
{
    bool fgAmpCtrl;

    fgAmpCtrl = (fgOn) ? PREAMP_NORMAL : PREAMP_RESET;
    
    ADC_REGCFG_BITS_W(prThis->eAdcId, ADC_REG_PREAMPLON, fgAmpCtrl);
    ADC_REGCFG_BITS_W(prThis->eAdcId, ADC_REG_PREAMPRON, fgAmpCtrl);
}

/**
 * set adc preamplifiter input
 *
 * @param [in]  prThis : hw class
 * @param [out] 
 *
 * @return
 */
static void AdcHw_SetPreAmpInput(PADC_HW_CLS prThis)
{
    ADC_REGCFG_BITS_W(prThis ->eAdcId, ADC_REG_PREAMPLINPUTSEL, PREAMP_AIN1);
    ADC_REGCFG_BITS_W(prThis ->eAdcId, ADC_REG_PREAMPRINPUTSEL, PREAMP_AIN1);
}

/**
 * set adc power mode
 *
 * @param [in]  prThis : hw class; fgOn : power on/off
 * @param [out] 
 *
 * @return
 */
static void AdcHw_SetAdcPowerMode(PADC_HW_CLS prThis, bool fgOn)
{
    bool fgAdcpowerMode;

    fgAdcpowerMode = (fgOn) ? ADC_PWP : ADC_PWD;
    
    ADC_REGCFG_BITS_W(prThis->eAdcId, ADC_REG_ADCLPWRUP, fgAdcpowerMode);
    ADC_REGCFG_BITS_W(prThis->eAdcId, ADC_REG_ADCRPWRUP, fgAdcpowerMode);
}

/**
 * set adc input
 *
 * @param [in]  prThis : hw class; eAdcInput : adc input source (mic in / line in)
 * @param [out] 
 *
 * @return
 */
static void AdcHw_SetAdcInput(PADC_HW_CLS prThis, AUD_ADC_INPUT_SRC eAdcInput)
{
    u32 u4AdcInput;

    u4AdcInput = (ADC_SRC_MICIN == eAdcInput) ? ADC_IN_PREAMP : ADC_IN_LSBUF;
    
    ADC_REGCFG_BITS_W(prThis->eAdcId, ADC_REG_ADCLINPUTSEL, u4AdcInput);
    ADC_REGCFG_BITS_W(prThis->eAdcId, ADC_REG_ADCRINPUTSEL, u4AdcInput);
}

/**
 * set mic bias power on / off
 *
 * @param [in]  prThis : hw class; fgOn : power on / off
 * @param [out] 
 *
 * @return
 */
static void AdcHw_SetMicBiasPowerMode(PADC_HW_CLS prThis, bool fgOn)
{
    bool fgMicBiasPowerMode;

    fgMicBiasPowerMode = (fgOn) ? MBIAS_PWP : MBIAS_PWD;
    
    ADC_REGCFG_BITS_W(prThis->eAdcId, ADC_REG_PWDB_MBIAS, fgMicBiasPowerMode);
}

/**
 * set lsbuf gain
 *
 * @param [in]  prThis : hw class; eLsbufGain : gain value
 * @param [out] 
 *
 * @return
 */
static void AdcHw_SetLsbufGain(PADC_HW_CLS prThis, AUD_LSBUF_GAIN eLsbufGain)
{
    ADC_REGCFG_BITS_W(prThis->eAdcId, ADC_REG_LSBUFLGAIN, eLsbufGain);
    ADC_REGCFG_BITS_W(prThis->eAdcId, ADC_REG_LSBUFRGAIN, eLsbufGain);
}


/**
 * set lsbuf clamp mode
 *
 * @param [in]  prThis : hw class; fgEn : if enale lsbuf clamp 
 * @param [out] 
 *
 * @return
 */
static void AdcHw_SetLsbufClampMode(PADC_HW_CLS prThis, bool fgEn)
{
    ADC_REGCFG_BITS_W(prThis->eAdcId, ADC_REG_FL_LSBUF_CLAMP, fgEn);
    ADC_REGCFG_BITS_W(prThis->eAdcId, ADC_REG_FR_LSBUF_CLAMP, fgEn);
}


/**
 * set lsbuf gain
 *
 * @param [in]  prThis : hw class; fgOn : on /off
 * @param [out] 
 *
 * @return
 */
static void AdcHw_SetLsbufPowerMode(PADC_HW_CLS prThis, bool fgOn)
{
    bool fgLsbufPowerMode;
    
    fgLsbufPowerMode = (fgOn) ? LSBUF_PWP : LSBUF_PWD;
    
    ADC_REGCFG_BITS_W(prThis->eAdcId, ADC_REG_LSBUFLPWRUP, fgLsbufPowerMode);
    ADC_REGCFG_BITS_W(prThis->eAdcId, ADC_REG_LSBUFRPWRUP, fgLsbufPowerMode);
}

/**
 * set lsbuf input
 *
 * @param [in]  prThis : hw class; eAdcInput : line in source
 * @param [out] 
 *
 * @return
 */
static void AdcHw_SetLsbufinput(PADC_HW_CLS prThis, AUD_ADC_INPUT_SRC eAdcInput)
{   
    ADC_REGCFG_BITS_W(prThis->eAdcId, ADC_REG_LSBUFLINPUTSEL, eAdcInput);
    ADC_REGCFG_BITS_W(prThis->eAdcId, ADC_REG_LSBUFRINPUTSEL, eAdcInput);
}

/**
 * set global bias power mode
 *
 * @param [in]  prThis : hw class; fgOn : on /off
 * @param [out] 
 *
 * @return
 */
static void AdcHw_SetGlbPowerMode(PADC_HW_CLS prThis, bool fgOn)
{
    bool fgGlbPowerMode;
    
    fgGlbPowerMode = (fgOn) ? GLB_PWP : GLB_PWD;

    AUDREG_BITS_W(AUD_REG_ADC_CFG16,
                  BIT_STR_GLB_PWD,
                  BIT_NUM_GLB_PWD,
                  fgGlbPowerMode);
}

/**
 * set adc clock reset 
 *
 * @param [in]  prThis : hw class; fgReset : reset or not
 * @param [out] 
 *
 * @return
 */
static void AdcHw_SeAdcClkReset(PADC_HW_CLS prThis, bool fgReset)
{
    bool fgAdcClkReset;
    
    fgAdcClkReset = (fgReset) ? CLK_RESET_EN : CLK_NO_RESET;

    AUDREG_BITS_W(AUD_REG_ADC_CFG16,
                  BIT_STR_RESET_CLK,
                  BIT_NUM_RESET_CLK,
                  fgAdcClkReset);
}

/**
 * set adc clock freq type
 *
 * @param [in]  prThis : hw class;
 * @param [out] 
 *
 * @return
 */
static void AdcHw_SeAdcClkType(PADC_HW_CLS prThis)
{
    AUDREG_BITS_W(AUD_REG_ADC_CFG16,
                  BIT_STR_SEL_CLK_FREQ,
                  BIT_NUM_SEL_CLK_FREQ,
                  CLK_13M);
}

/**
 * set mic gain
 *
 * @param [in]  prThis : hw class; u4MicGain : gain value
 * @param [out] 
 *
 * @return
 */
static void AdcHw_SetMicGain(PADC_HW_CLS prThis, u32 u4MicGain)
{
    u32 u4MicGainReg;

    u4MicGainReg = (AUD_ADC1 == prThis->eAdcId) ? REGENV_AFE1_CFG19 : REGENV_AFE2_CFG19;

    AUDREG_BITS_W(u4MicGainReg, BIT_STR_CH2_MANU_GAIN_SEL, BIT_NUM_CH2_MANU_GAIN_SEL, MANU_GAIN);
    AUDREG_BITS_W(u4MicGainReg, BIT_STR_CH1_MANU_GAIN_SEL, BIT_NUM_CH1_MANU_GAIN_SEL, MANU_GAIN);

    AUDREG_BITS_W(u4MicGainReg, BIT_STR_CH2_MANU_GAIN, BIT_NUM_CH2_MANU_GAIN, u4MicGain);
    AUDREG_BITS_W(u4MicGainReg, BIT_STR_CH1_MANU_GAIN, BIT_NUM_CH1_MANU_GAIN, u4MicGain);
}

/**
 * afe config according : clock source & sample rate
 *
 * @param [in]  prThis : hw class; eFs : sample rate; eClkSrc : afe clock source
 * @param [out] 
 *
 * @return
 */
void AdcHw_SetAfe(PADC_HW_CLS prThis, AUDIO_SAMPLING_T eFs, AUD_AFE_CLK_SEL_E eClkSrc)
{
    u32 u4AfeCfgReg = 0;
    u32 u4InitVal = 0;

    //AFE clock source config    
    if (AFE_CLK_MLIN2 == eClkSrc)
    {
       ADC_REGCFG_BITS_W(prThis->eAdcId, ADC_REG_AFE_LIN2_CLK_SEL, 3); 
    }
    else
    {
        ADC_REGCFG_BITS_W(prThis->eAdcId, ADC_REG_AFE_LIN2_CLK_SEL, 0); 
        ADC_REGCFG_BITS_W(prThis->eAdcId, ADC_REG_AFE_BCK_SEL, eClkSrc); 
        ADC_REGCFG_BITS_W(prThis->eAdcId, ADC_REG_AFE_LRCK_SEL, eClkSrc); 
    }

    //AFE sample rate config
    u4AfeCfgReg = (AUD_ADC1 == prThis->eAdcId) ? REGENV_AFE1_CFG20 : REGENV_AFE2_CFG20;
    
    //BIT [17:16] & [15:14], ch voice mode 
    //0:8K, 1:16K, 3:48K
    u4InitVal = (FS_48K == eFs) ? 0x203c001 : ((FS_16K == eFs) ? 0x2014001 : 0x2000001);

    AUDREG_WRITE(u4AfeCfgReg, u4InitVal);
    AUDREG_BITS_W(u4AfeCfgReg, BIT_STR_MCU_UL_SRC_ON, BIT_NUM_MCU_UL_SRC_ON, 1);
    AUDREG_BITS_W(u4AfeCfgReg, BIT_STR_ANA_FIFO_CS, BIT_NUM_ANA_FIFO_CS, 1);
}

/**
 * set line in input pin ain0_l ~ ain4_l, ain0_r ~ ain4_r, anlog input or gpi function
 *
 * @param [in]  ePinIdx : input pin group; fgGpiFunEn : if set gpi function
 * @param [out] 
 *
 * @return
 */
void AdcHw_SetInputPinGpioFun(AUD_LIN_PIN_IDX ePinIdx, bool fgGpiFunEn)
{
    AUDREG_BITS_W(AUD_REG_ADC_CFG12, (ePinIdx + BIT_STR_RCH_GPIO_PIN_CFG), 1, fgGpiFunEn); //RCH GPIO EN
    AUDREG_BITS_W(AUD_REG_ADC_CFG14, (ePinIdx + BIT_STR_LCH_GPIO_PIN_CFG), 1, fgGpiFunEn); //LCH GPIO EN

    AUDREG_BITS_W(AUD_REG_ADC_CFG12, (ePinIdx + BIT_STR_RCH_GPIO_PIN_CTL), 1, DIGITAL_INPUT); //RCH GPI CFG
    AUDREG_BITS_W(AUD_REG_ADC_CFG14, (ePinIdx + BIT_STR_LCH_GPIO_PIN_CTL), 1, DIGITAL_INPUT); //LCH GPI CFG
}


//=========================================================//
    #define CodeSight_AdcHw_Pubilic_Func
//=========================================================//


/**
 * public interface : adc start
 *
 * @param [in]  prThis : hw class; prCfg : adc hw mode setting
 * @param [out] 
 *
 * @return
 */
static void AdcHw_Start(void * pThis, void * pCfg)
{
    PADC_HW_CLS prThis = (PADC_HW_CLS)pThis;
    PADC_EXTPARAMS_T prCfg =(PADC_EXTPARAMS_T)pCfg;
    
    ADCLOG_INFO(T(" AdcHw_Start(%d) \r\n"), prThis->eAdcId);

    prThis->eInput = prCfg->eInput;
    
    AdcHw_SetGlbPowerMode(prThis, TRUE);
    AdcHw_SetAdcPowerMode(prThis, TRUE);
    
    AdcHw_SeAdcClkType(prThis);
    AdcHw_SeAdcClkReset(prThis, FALSE);
    
    AdcHw_SetAdcInput(prThis,  prCfg->eInput);
    
    if (ADC_SRC_MICIN == prCfg->eInput)
    {
        AdcHw_SetLsbufPowerMode(prThis, TRUE);   // -> fix line in crosstalk to mic issue
        
        AdcHw_SetMicBiasPowerMode(prThis, TRUE);
        AdcHw_SetPreAmpOn(prThis, TRUE);
        
        AdcHw_SetPreAmpInput(prThis);
        AdcHw_SetMicGain(prThis, prCfg->u4MicGain);
    }
    else
    {
        AdcHw_SetLsbufPowerMode(prThis, TRUE);
        AdcHw_SetLsbufGain(prThis, prCfg->eLinGain);
        AdcHw_SetLsbufinput(prThis, prCfg->eInput);
        
        AdcHw_SetLsbufClampMode(prThis, TRUE);   // -> fix mic crosstalk to line in issue
    }

    AdcHw_SetAfe(prThis, prCfg->eFs, prCfg->eClkSrc);
}
    
/**
 * public interface : adc stop
 *
 * @param [in]  prThis : hw class;
 * @param [out] 
 *
 * @return
 */
static void AdcHw_Stop(void * pThis)
{
    PADC_HW_CLS prThis = (PADC_HW_CLS)pThis;
    
    if (ADC_SRC_MICIN == prThis->eInput)
    {
        AdcHw_SetMicBiasPowerMode(prThis, FALSE);
        AdcHw_SetPreAmpOn(prThis, FALSE); 
    }
    else
    {
        AdcHw_SetLsbufinput(prThis, ADC_SRC_NON);
        AdcHw_SetLsbufPowerMode(prThis, FALSE);
    }
	
    AdcHw_SetAdcPowerMode(prThis, FALSE);    
    ADCLOG_INFO(T(" AdcHw_Stop(%d) \r\n"), prThis->eAdcId);
}


//===========================================//
    #define CodeSight_AdcHw_Create
//===========================================//

/**
 * delect a adc hw object
 *
 * @param [in]  prThis : pointer to the adc hw object
 * @param [out] 
 *
 * @return      0: OK; others: NG
 */
static u32 AdcHw_Delete(void * pThis)
{
    PADC_HW_CLS prThis = (PADC_HW_CLS)pThis;
    AUD_CLASS_DELETE();
    
    return (0);
}

/**
 * creat a new adc hw object
 *
 * @param [in] 
 * @param [out] 
 *
 * @return  pointer to new object
 */
PADC_HW_CLS_PUB AdcHw_New(AUD_ADC_ID eAdcId)
{
    PADC_HW_CLS prThis = AUD_CLASS_NEW(ADC_HW_CLS);

    if (prThis)
    {
        prThis ->eAdcId = eAdcId;
            
        prThis ->rPub.Delete = AdcHw_Delete;
        prThis ->rPub.Start = AdcHw_Start;
        prThis ->rPub.Stop = AdcHw_Stop;
    }

    return ((PADC_HW_CLS_PUB)prThis);
}


