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
 * @file aud_aout_hw.c source file
 * 
 * aud io aout module hardware driver
 * 
 * @author qiuhua.yin@autochips.com
 * 
 */

#include "aud_aout_hal.h"
#include "aud_reg_env.h"
#include "aud_reg_rgbk2.h"
#include "aud_io_clock_if.h"


typedef struct 
{
    AOUT_HW_CLS_PUB rPub;
    
    AUD_AOUT_DEVID eAoutId;

}AOUT_HW_CLS, *PAOUT_HW_CLS;


static AUD_IO_REG_CTL aDacSrcCfgReg[AOUT_PATH_MAX][AUD_DAC_EXT + 1] = 
{
    //front seat
    {   
        //pwm dac      
        {REGENV_PWMTOP_CFG, BIT_STR_FRNT_PWM_IN_SEL, BIT_NUM_FRNT_PWM_IN_SEL, "front seat : Pwm DAC SRC CONFIG"},
        //ext dac
        {REGENV_RGBK2_AOUT_CFG2, BIT_STR_FRNT_SRC_SEL, BIT_NUM_FRNT_SRC_SEL,  "front seat : Ext DAC SRC CONFIG"}
    },
    //rear seat
    {   
        //pwm dac
        {REGENV_PWMTOP_CFG, BIT_STR_REAR_PWM_IN_SEL, BIT_NUM_REAR_PWM_IN_SEL, "rear seat  : Pwm DAC SRC CONFIG"},
        //ext dac
        {REGENV_RGBK2_AOUT_CFG2, BIT_STR_REAR_SRC_SEL, BIT_NUM_REAR_SRC_SEL,  "rear seat  : Ext DAC SRC CONFIG"}
    }
};

static AUD_IO_REG_CTL aAsdataPinSrcCfgReg[AUDID_AOUT_MAX][AUD_DAC_EXT + 1][ASDATA_PIN_MAX] = 
{
    {//aout1
        {//pwm dac
            {REGENV_AENV_BAK2, BIT_STR_PWM1_CH_CFGA, BIT_NUM_PWM1_CH_CFGA, "AOUT1 PWM DAC AOSDATA0"},
            {REGENV_AENV_BAK2, BIT_STR_PWM1_CH_CFGB, BIT_NUM_PWM1_CH_CFGB, "AOUT1 PWM DAC AOSDATA1"},
            {REGENV_AENV_BAK2, BIT_STR_PWM1_CH_CFGC, BIT_NUM_PWM1_CH_CFGC, "AOUT1 PWM DAC AOSDATA2"},
            {REGENV_AENV_BAK2, BIT_STR_PWM1_CH_CFGD, BIT_NUM_PWM1_CH_CFGD, "AOUT1 PWM DAC AOSDATA3"}
        },
        {//ext dac
            {REGENV_AOUT_CFG1, BIT_STR_AOSDATA0, BIT_NUM_AOSDATA0, "AOUT1 EXT DAC AOSDATA0"},
            {REGENV_AOUT_CFG1, BIT_STR_AOSDATA1, BIT_NUM_AOSDATA1, "AOUT1 EXT DAC AOSDATA1"},
            {REGENV_AOUT_CFG1, BIT_STR_AOSDATA2, BIT_NUM_AOSDATA2, "AOUT1 EXT DAC AOSDATA2"},
            {REGENV_AOUT_CFG1, BIT_STR_AOSDATA4, BIT_NUM_AOSDATA4, "AOUT1 EXT DAC AOSDATA4"}
        }
    },
    {//aout2
        {//pwm dac
            {REGENV_PWMIP_S_DAC_CH_CFG, BIT_STR_PWM2_CH_CFGA, BIT_NUM_PWM2_CH_CFGA, "AOUT2 PWM DAC AOSDATA0"},
            {REGENV_PWMIP_S_DAC_CH_CFG, BIT_STR_PWM2_CH_CFGB, BIT_NUM_PWM2_CH_CFGB, "AOUT2 PWM DAC AOSDATA0"},
            {REGENV_PWMIP_S_DAC_CH_CFG, BIT_STR_PWM2_CH_CFGC, BIT_NUM_PWM2_CH_CFGC, "AOUT2 PWM DAC AOSDATA0"},
            {REGENV_PWMIP_S_DAC_CH_CFG, BIT_STR_PWM2_CH_CFGD, BIT_NUM_PWM2_CH_CFGD, "AOUT2 PWM DAC AOSDATA0"}
        },
        {//ext dac
            {REGENV_AOUT_CFG1, BIT_STR_AOSDATA6, BIT_NUM_AOSDATA6,  "AOUT2 EXT DAC AOSDATA0"},
            {REGENV_AOUT_CFG1, BIT_STR_AOSDATA7, BIT_NUM_AOSDATA7,  "AOUT2 EXT DAC AOSDATA0"},
            {REGENV_AOUT_CFG1, BIT_STR_AOSDATA8, BIT_NUM_AOSDATA8,  "AOUT2 EXT DAC AOSDATA0"},
            {REGENV_AOUT_CFG1, BIT_STR_AOSDATA9, BIT_NUM_AOSDATA9,  "AOUT2 EXT DAC AOSDATA0"}
        }
    }
};


typedef enum
{
    AOUT_REG_BNUM,
    AOUT_REG_DAT_DLY,
    AOUT_REG_LEFT_ALN,
    AOUT_REG_A2CBCK,
    AOUT_REG_LRCK_CYC,
    AOUT_REG_INV_BCK,
    AOUT_REG_INV_LRCK,
    AOUT_REG_BLK_ADR,
    AOUT_REG_EN,
    AOUT_REG_INT_CLR,
    AOUT_REG_ARM_CTL,
    AOUT_REG_IDX_MAX,
}AOUT_CONFIG_REG_IDX;

static AUD_IO_REG_CTL aAoutConfigReg[AUDID_AOUT_MAX][AOUT_REG_IDX_MAX] = 
{
    {
        {REGENV_AOUT_CFG, BIT_STR_DA_BNUM, BIT_NUM_DA_BNUM, "AOUT1: DA_BNUM"},     //AOUT_REG_BNUM
        {REGENV_AOUT_CFG, BIT_STR_DAT_DLY, BIT_NUM_DAT_DLY, "AOUT1: DAT_DLY"},     //AOUT_REG_DAT_DLY
        {REGENV_AOUT_CFG, BIT_STR_LEFT_ALN, BIT_NUM_LEFT_ALN, "AOUT1: LEFT_ALN"},  //AOUT_REG_LEFT_ALN
        {REGENV_AOUT_CFG, BIT_STR_A2BCKX, BIT_NUM_A2BCKX, "AOUT1: A2BCKX"},        //AOUT_REG_A2CBCK
        {REGENV_AOUT_CFG, BIT_STR_LRCK_CYC, BIT_NUM_LRCK_CYC, "AOUT1: LRCK_CYC"},  //AOUT_REG_LRCK_CYC
        {REGENV_AOUT_CFG, BIT_STR_INV_BCK, BIT_NUM_INV_BCK, "AOUT1: INV_BCK"},     //AOUT_REG_INV_BCK
        {REGENV_AOUT_CFG, BIT_STR_INV_LRCK, BIT_NUM_INV_LRCK, "AOUT1: INV_LRCK"},  //AOUT_REG_INV_LRCK
        {REGENV_RWD_BLK45, BIT_STR_DRAM_SBLK5, BIT_NUM_DRAM_SBLK5, "AOUT1: SBLK5"}, //AOUT_REG_BLK_ADR
        {REGENV_AOUT1_CTRL, BIT_STR_AOUT1_EN_PRE, BIT_NUM_AOUT1_EN_PRE, "AOUT1: EN"}, //AOUT_REG_EN
        {REGENV_RGBK2_CFG5, BIT_STR_AO_INT_CLR, BIT_NUM_AO_INT_CLR, "AOUT1: INT clear"}, //AOUT_REG_INT_CLR
        {REGENV_AOUT_ARM_CTRL, BIT_STR_AP_AOUT_ARM_CTRL_CFG, BIT_NUM_AP_AOUT_ARM_CTRL_CFG, "AOUT1 : ARM_CTRL"}
    },
    {
        {REGENV_MISC_CTRL, BIT_STR_AOUT2_DA_BNUM, BIT_NUM_AOUT2_DA_BNUM, "AOUT2: DA_BNUM"},    //AOUT_REG_BNUM
        {REGENV_MISC_CTRL, BIT_STR_AOUT2_DELAY, BIT_NUM_AOUT2_DELAY, "AOUT2: DAT_DLY"},        //AOUT_REG_DAT_DLY
        {REGENV_MISC_CTRL, BIT_STR_AOUT2_LEFT, BIT_NUM_AOUT2_LEFT, "AOUT2: LEFT_ALN"},         //AOUT_REG_LEFT_ALN
        {REGENV_MISC_CTRL, BIT_STR_AOUT2_A2BCKX, BIT_NUM_AOUT2_A2BCKX, "AOUT2: A2BCKX"},       //AOUT_REG_A2CBCK
        {REGENV_MISC_CTRL, BIT_STR_AOUT2_LRCK_CYC, BIT_NUM_AOUT2_LRCK_CYC, "AOUT2: LRCK_CYC"}, //AOUT_REG_LRCK_CYC
        {REGENV_MISC_CTRL, BIT_STR_INV_BCK2, BIT_NUM_INV_BCK2, "AOUT2: INV_BCK2"},             //AOUT_REG_INV_BCK
        {REGENV_MISC_CTRL, BIT_STR_INV_LRCK2, BIT_NUM_INV_LRCK2, "AOUT2: INV_LRCK2"},          //AOUT_REG_INV_LRCK
        {REGENV_RWD_BLK67, BIT_STR_DRAM_SBLK7, BIT_NUM_DRAM_SBLK7, "AOUT2: SBLK7"},            //AOUT_REG_BLK_ADR
        {REGENV_AOUT2_CTRL, BIT_STR_AOUT2_EN_PRE, BIT_NUM_AOUT2_EN_PRE, "AOUT2: EN"},          //AOUT_REG_EN
        {REGENV_RGBK2_CFG5, BIT_STR_AO2_INT_CLR, BIT_NUM_AO2_INT_CLR, "AOUT2: INT clear"}, //AOUT_REG_INT_CLR
        {REGENV_RGBK2_AOUT_CFG2, BIT_STR_AP_AOUT2_ARM_CTRL_CFG, BIT_NUM_AP_AOUT2_ARM_CTRL_CFG, "AOUT2 : ARM_CTRL"}
    }
};

//macros for aout kernal  registers bits read / write
#define AOUT_REGKER_BITS_W(DevId, addr, start, bitNum, val) \
    AUDREG_BITS_W((addr + ((DevId == AUDID_AOUT1) ? 0 : (REGENV_AOUT2_CH1_BUF_SADR - REGENV_AOUT1_CH1_BUF_SADR))), \
                   start, bitNum, val)
#define AOUT_REGKER_BITS_R(DevId, addr, start, bitNum) \
    AUDREG_BITS_R((addr + ((DevId == AUDID_AOUT1) ? 0 : (REGENV_AOUT2_CH1_BUF_SADR - REGENV_AOUT1_CH1_BUF_SADR))), \
                   start, bitNum)
                   
//macros for aout kernal  registers  read / write
#define AOUT_REGKER_W(DevId, addr, val) \
    AUDREG_WRITE((addr + ((DevId == AUDID_AOUT1) ? 0 : (REGENV_AOUT2_CH1_BUF_SADR - REGENV_AOUT1_CH1_BUF_SADR))), val)
#define AOUT_REGKER_R(DevId, addr) \
    AUDREG_READ(addr + ((DevId == AUDID_AOUT1) ? 0 : (REGENV_AOUT2_CH1_BUF_SADR - REGENV_AOUT1_CH1_BUF_SADR)))

//macros for aout configure regosters read / write
#define AOUT_REGCFG_BITS_W(DevId, CfgRegId, val) \
    AUDREG_BITS_W((aAoutConfigReg[DevId][CfgRegId].u4Adr), (aAoutConfigReg[DevId][CfgRegId].u4BitStart), \
                  (aAoutConfigReg[DevId][CfgRegId].u4BitNum), val)
#define AOUT_REGCFG_BITS_R(DevId, CfgRegId) \
    AUDREG_BITS_R((aAoutConfigReg[DevId][CfgRegId].u4Adr), (aAoutConfigReg[DevId][CfgRegId].u4BitStart),\
                  (aAoutConfigReg[DevId][CfgRegId].u4BitNum))

static void AoutHw_SetNsadr(void * pThis, u32 u4ChNum, u32 u4Nsadr, u32 u4ChSize);
;

//=========================================================//
    #define CodeSight_AoutHw_Static_Func
//=========================================================//


/**
 * set aout hw control by arm or by dsp
 *
 * @param [in]  prThis : hw class; fgEn : 1 : ARM Control, 0 : DSP Control
 * @param [out] 
 *
 * @return
 */
static void AoutHw_SetArmCtrl(void * pThis, bool fgEn)
{
    u32 u4Cfg = fgEn ? 0x36 : 0;
    PAOUT_HW_CLS prThis = (PAOUT_HW_CLS)pThis;
    
    AOUT_REGCFG_BITS_W(prThis->eAoutId, AOUT_REG_ARM_CTL, u4Cfg);
}


static bool AoutHw_IsArmCtrl(void * pThis)
{
    PAOUT_HW_CLS prThis = (PAOUT_HW_CLS)pThis;
    bool fgArmCtrl = (0x36  == AOUT_REGCFG_BITS_R(prThis->eAoutId, AOUT_REG_ARM_CTL));
    
    return (fgArmCtrl);
}

/**
 * set aout data bit number to dac
 *
 * @param [in]  prThis : hw class; u4BitWidth : bit number
 * @param [out] 
 *
 * @return
 */
static void AoutHw_SetDataBitNum(PAOUT_HW_CLS prThis, u32 u4BitWidth)
{
    AOUT_REGCFG_BITS_W(prThis->eAoutId, AOUT_REG_BNUM, u4BitWidth);
}
    
/**
 * set aout data align mode
 *
 * @param [in]  prThis : hw class; eFmt : RJ, LJ, I2S
 * @param [out] 
 *
 * @return
 */
static void AoutHw_SetDataFmt(PAOUT_HW_CLS prThis, AUDFMT_INTF_E eFmt)
{
    AOUT_REGCFG_BITS_W(prThis->eAoutId, AOUT_REG_LEFT_ALN, (eFmt & 0x1));
    AOUT_REGCFG_BITS_W(prThis->eAoutId, AOUT_REG_DAT_DLY, ((eFmt & 0x2) >> 1));
}

/**
 * set bck divider, which used to generate bck from mclk
 *
 * @param [in]  prThis : hw class; eMclkType : mclk type
 * @param [out] 
 *
 * @return
 */
static void AoutHw_SetBckDivider(PAOUT_HW_CLS prThis, MCLK_TYPE_T eMclkType)
{
    u32 u4MclkToBckRatio;
    
    u4MclkToBckRatio = IoClk_GetMclkToFsRatio(eMclkType);
    u4MclkToBckRatio /= (32 * 4);

    AOUT_REGCFG_BITS_W(prThis->eAoutId, AOUT_REG_A2CBCK, u4MclkToBckRatio);
}

/**
 * set lrck divider, which used to generate lrck from bck
 *
 * @param [in]  prThis : hw class; eCycle : lrck cycle, default 32
 * @param [out] 
 *
 * @return
 */
static void AoutHw_SetLrckDivider(PAOUT_HW_CLS prThis, AUD_LRCK_CYC_T eCycle)
{
    AOUT_REGCFG_BITS_W(prThis->eAoutId, AOUT_REG_LRCK_CYC, eCycle);
}

/**
 * Invert bck or not
 *
 * @param [in]  prThis : hw class; fgInvert : yes or not
 * @param [out] 
 *
 * @return
 */
static void AoutHw_SetBckInvert(PAOUT_HW_CLS prThis, bool fgInvert)
{
    AOUT_REGCFG_BITS_W(prThis->eAoutId, AOUT_REG_INV_BCK, fgInvert);
}

/**
 * invert lrck or not
 *
 * @param [in]  prThis : hw class; fgInvert : yes or not
 * @param [out] 
 *
 * @return
 */
static void AoutHw_SetLrckInvert(PAOUT_HW_CLS prThis, bool fgInvert)
{
    AOUT_REGCFG_BITS_W(prThis->eAoutId, AOUT_REG_INV_LRCK, fgInvert);
}

/**
 * set bank address for aout hw
 *
 * @param [in]  prThis : hw class; u4BankAdr : aout bank address
 * @param [out] 
 *
 * @return
 */
#ifdef AOUT_ARM_CTL
static void AoutHw_SetBankAdr(PAOUT_HW_CLS prThis, u32 u4BankAdr)
{
    AUDREG_BITS_W(REGENV_AUD_DRAM_BANK, BIT_STR_ADSP_BANK, BIT_NUM_ADSP_BANK, u4BankAdr);
}

/**
 * set block address for aout hw
 *
 * @param [in]  prThis : hw class; u4BankAdr : aout blk address
 * @param [out] 
 *
 * @return
 */
static void AoutHw_SetBlkAdr(PAOUT_HW_CLS prThis, u32 u4BlkAdr)
{
    AOUT_REGCFG_BITS_W(prThis->eAoutId, AOUT_REG_BLK_ADR, u4BlkAdr);
}

/**
 * set CH1 start address
 *
 * @param [in]  prThis : hw class; u4ChAdr : start address of CH1
 * @param [out] 
 *
 * @return
 */
static void AoutHw_SetChAdr(PAOUT_HW_CLS prThis, u32 u4ChAdr)
{
    AOUT_REGKER_BITS_W(prThis->eAoutId, 
                       REGENV_AOUT1_CH1_BUF_SADR,
                       BIT_STR_AOUT1_CH1_BUF_SADR, 
                       BIT_NUM_AOUT1_CH1_BUF_SADR,
                       u4ChAdr);
}

/**
 * set aout channel size
 *
 * @param [in]  prThis : hw class; u4ChSize : aout channel size
 * @param [out] 
 *
 * @return
 */
static void AoutHw_SetChSize(PAOUT_HW_CLS prThis, u32 u4ChSize)
{
    AOUT_REGKER_BITS_W(prThis->eAoutId, 
                       REGENV_AOUT1_CH1_BUF_SIZE,
                       BIT_STR_AOUT1_CH1_BUF_SIZE, 
                       BIT_NUM_AOUT1_CH1_BUF_SIZE,
                       u4ChSize);
}

/**
 * set aout channel number
 *
 * @param [in]  prThis : hw class; u4ChNum : aout channel number
 * @param [out] 
 *
 * @return
 */
static void AoutHw_SetChNumber(PAOUT_HW_CLS prThis, u32 u4ChNum)
{
    AOUT_REGKER_BITS_W(prThis->eAoutId, 
                       REGENV_AOUT1_CH_NUM,
                       BIT_STR_AOUT1_CH_NUM, 
                       BIT_NUM_AOUT1_CH_NUM,
                       u4ChNum);
}

/**
 * aout channel buffer configure
 *
 * @param [in]  prThis : hw class; u4CfgId : CFG0/CFG1/CFG2;  u4ChCfg: config value
 * @param [out] 
 *
 * @return
 */
static void AoutHw_SetChConfigure(PAOUT_HW_CLS prThis, u32 u4CfgId, u32 u4ChCfg)
{
    AOUT_REGKER_W(prThis->eAoutId, (REGENV_AOUT1_CH_CFG0 + u4CfgId * 4), u4ChCfg);
}

/**
 * aout interrupt size configure
 *
 * @param [in]  prThis : hw class; u4IntSize : number of remained samples to generate aout interrupt
 * @param [out] 
 *
 * @return
 */
static void AoutHw_SetIntrSize(PAOUT_HW_CLS prThis, u32 u4IntSize)
{
    AOUT_REGKER_BITS_W(prThis->eAoutId, 
                       REGENV_AOUT1_INTRSIZE,
                       BIT_STR_AOUT1_INTRSIZE, 
                       BIT_NUM_AOUT1_INTRSIZE,
                       u4IntSize);
}

/**
 * sample number for next aout burst
 *
 * @param [in]  prThis : hw class; u4Nsnum : sample number
 * @param [out] 
 *
 * @return
 */
static void AoutHw_SetNextSampleNum(PAOUT_HW_CLS prThis, u32 u4Nsnum)
{
    AOUT_REGKER_BITS_W(prThis->eAoutId, 
                       REGENV_AOUT1_NSNUM,
                       BIT_STR_AOUT1_NSNUM, 
                       BIT_NUM_AOUT1_NSNUM,
                       u4Nsnum);
}
#endif

/**
 * Set Dac source
 *
 * @param [in]  prThis : hw class;
 * @param [out] 
 *
 * @return
 */
static void AoutHw_SetDacSrc(void * pThis, AUD_OUT_PATH_T eAOutPath, AUD_DAC_TYPE_T eDacType)
{
    AUD_AOUT_SRC eAoutSrc;
    PAOUT_HW_CLS prThis = (PAOUT_HW_CLS)pThis;
    PAUD_IO_REG_CTL prIoReg = &(aDacSrcCfgReg[eAOutPath][eDacType]);

    eAoutSrc =  (AUDID_AOUT1 == prThis->eAoutId) ? SRC_AOUT1 : ((AUDID_AOUT2 == prThis->eAoutId) ? SRC_AOUT2 : SRC_DVD);

    AUDREG_BITS_W(prIoReg->u4Adr, prIoReg->u4BitStart, prIoReg->u4BitNum, eAoutSrc);

    if(AUD_DAC_PWM == eDacType)//amute pin set by extern dac source.
    {
        prIoReg = &(aDacSrcCfgReg[eAOutPath][AUD_DAC_EXT]);
        AUDREG_BITS_W(prIoReg->u4Adr, prIoReg->u4BitStart, prIoReg->u4BitNum, eAoutSrc);
    }
}

/**
 * aout asdata pin config
 *
 * @param [in]  prThis : hw class;
 * @param [out] 
 *
 * @return
 */
static void AoutHw_SetAoutAsDataPin(PAOUT_HW_CLS prThis, AUD_DAC_TYPE_T eDacType, ASDATA_PIN eAsdataPin, ASDATA_PIN_SRC ePinSrc)
{
    PAUD_IO_REG_CTL prIoReg = &(aAsdataPinSrcCfgReg[prThis->eAoutId][eDacType][eAsdataPin]);

    AUDREG_BITS_W(prIoReg->u4Adr, prIoReg->u4BitStart, prIoReg->u4BitNum, ePinSrc);
}


//=========================================================//
    #define CodeSight_AoutHw_Pubilic_Func
//=========================================================//

/**
 * public interface : enable/disable aout hw
 *
 * @param [in]  prThis : hw class; fgEnable : enable or disable
 * @param [out] 
 *
 * @return
 */
static void AoutHw_Enable(void * pThis, bool fgEnable)
{
    PAOUT_HW_CLS prThis = (PAOUT_HW_CLS)pThis;
    
    AOUT_REGCFG_BITS_W(prThis->eAoutId, AOUT_REG_EN, fgEnable);
}

/**
 * public interface : aout configure
 *
 * @param [in]  prThis : hw class; prCfg : aout hw mode setting
 * @param [out] 
 *
 * @return
 */
static void AoutHw_InitCfg(void * pThis, void * pCfg)
{
    PAOUT_HW_CLS prThis = (PAOUT_HW_CLS)pThis;
    PAUD_AOUT_CFG_T prCfg = (PAUD_AOUT_CFG_T)pCfg;
    
    PAOUT_EXTPARAMS_T prExtCfg = &(prCfg->rExtCfg);
   #ifdef AOUT_ARM_CTL
    PAUD_DATA_BUF_T prBuf = &(prCfg->rBuf);
   #endif
    ASDATA_PIN_SRC ePin3Src;

    /***** part1 aout basic configure ******/
    AoutHw_SetDataBitNum(prThis, prCfg->u4DacBitNum);
    AoutHw_SetDataFmt(prThis, prCfg->eFmt);
    AoutHw_SetBckDivider(prThis, prCfg->eMclkType);
    AoutHw_SetLrckDivider(prThis, prCfg->eCycle);

    AoutHw_SetBckInvert(prThis, prCfg->fgInvertBck);
    AoutHw_SetLrckInvert(prThis, prCfg->fgInvertLrck);

  #ifdef AOUT_ARM_CTL
    /***** part2 aout kernal set ******/
  
    AoutHw_SetBankAdr(prThis, (prBuf->u4PhySddr>> 20) & 0x7ff);
    AoutHw_SetBlkAdr(prThis, (prBuf->u4PhySddr & 0x0FFF00) >> 8);
    AoutHw_SetChAdr(prThis, (prBuf->u4PhySddr & 0xFF) >> 2);

    AoutHw_SetChSize(prThis, prBuf->u4ChBufSz >> 2);
    AoutHw_SetChNumber(prThis, prBuf->u4Chn);

    AoutHw_SetChConfigure(prThis, 0, prExtCfg->u4ChCfg0);
    AoutHw_SetChConfigure(prThis, 1, prExtCfg->u4ChCfg1);

    if (AUDID_AOUT1 == prThis->eAoutId)
    {
        AoutHw_SetChConfigure(prThis, 2, prExtCfg->u4ChCfg2);
    }

    /***** part3 aout s32 related ******/ 
    AoutHw_SetNextSampleNum(prThis, prExtCfg->rIntCfg.u4NSNum);
    AoutHw_SetIntrSize(prThis, prExtCfg->rIntCfg.u4IntrSize);
   
    AoutHw_SetNsadr(prThis, prBuf->u4Chn, (u32)(prBuf->u4PhySddr & 0xFF), prBuf->u4ChBufSz);
  #endif
   
    /***** part4 dac source select ******/
    AoutHw_SetDacSrc(prThis, prExtCfg->eOutPath, prExtCfg->eDacType);

    /***** part5 ASDATA PIN source slect ******/
    ePin3Src = (AUDID_AOUT1 == prThis->eAoutId) ? ASDATA_CH9_10 : ASDATA_FLFR;
    
    AoutHw_SetAoutAsDataPin(prThis, prExtCfg->eDacType, ASDATA_PIN0, ASDATA_FLFR);
    AoutHw_SetAoutAsDataPin(prThis, prExtCfg->eDacType, ASDATA_PIN1, ASDATA_RLRR);
    AoutHw_SetAoutAsDataPin(prThis, prExtCfg->eDacType, ASDATA_PIN2, ASDATA_CLFE);
    AoutHw_SetAoutAsDataPin(prThis, prExtCfg->eDacType, ASDATA_PIN3, ePin3Src);
}

/**
 * start address for next aout burst
 *
 * @param [in]  prThis : hw class; u4ChId : which channel; u4Nsadr : next start address of according channel
 * @param [out] 
 *
 * @return
 */
static void AoutHw_SetNsadr(void * pThis, u32 u4ChNum, u32 u4Nsadr, u32 u4ChSize)
{
    u32 u4ChIdx;
    PAOUT_HW_CLS prThis = (PAOUT_HW_CLS)pThis;
    
    for (u4ChIdx = 0; u4ChIdx < u4ChNum; u4ChIdx ++)
    {
        AOUT_REGKER_BITS_W(prThis->eAoutId, 
                           (REGENV_AOUT1_CH1_NSADR + u4ChIdx * 4),
                           BIT_STR_AOUT1_CH_NSADR, 
                           BIT_NUM_AOUT1_CH_NSADR, 
                           (u4Nsadr >> 2));

        u4Nsadr += u4ChSize;
    }
}

/**
 * Set Aout Int Clear Bit
 *
 * @param [in]  prThis : hw class; fgOn : clear or raise
 * @param [out] 
 *
 * @return
 */
static void AoutHw_SetIntClrBit(void * pThis, bool fgOn)
{
    PAOUT_HW_CLS prThis = (PAOUT_HW_CLS)pThis;
    AOUT_REGCFG_BITS_W(prThis->eAoutId, AOUT_REG_INT_CLR, fgOn);
}

/**
 * Set Aout Data source
 *
 * @param [in]  prThis : hw class; eDacType : PWM/EXT; fgAdcBypasMode : from ADC(1) / DRAM(0)
 * @param [out] 
 *
 * @return
 */
static void AoutHw_SetDataSrc(void * pThis, AUD_DAC_TYPE_T eDacType, bool fgAdcBypasMode)
{
    ASDATA_PIN_SRC ePinSrc;
    PAOUT_HW_CLS prThis = (PAOUT_HW_CLS)pThis;

    if (AUDID_AOUT2 == prThis->eAoutId)
    {
        ePinSrc = fgAdcBypasMode ? ASDATA_CH7_8 : ASDATA_FLFR;
        AoutHw_SetAoutAsDataPin(prThis, eDacType, ASDATA_PIN3, ePinSrc);
    }	
    else if (AUDID_AOUT1 == prThis->eAoutId)
    {
        ePinSrc = fgAdcBypasMode ? ASDATA_CH9_10 : ASDATA_FLFR;
        AoutHw_SetAoutAsDataPin(prThis, eDacType, ASDATA_PIN0, ePinSrc);
    }
}


//===========================================//
    #define CodeSight_AoutHw_Create
//===========================================//

/**
 * delect a aout hw object
 *
 * @param [in]  prThis : pointer to the aout hw object
 * @param [out] 
 *
 * @return      0: OK; others: NG
 */
static u32 AoutHw_Delete(void * prThis)
{    
    AUD_CLASS_DELETE();
    
    return (0);
}

/**
 * creat a new aout hw object
 *
 * @param [in]  eAoutId : AOUT1 or AOUT2
 * @param [out] 
 *
 * @return  pointer to new object
 */
PAOUT_HW_CLS_PUB AoutHw_New(AUD_AOUT_DEVID eAoutId)
{
    PAOUT_HW_CLS prThis = AUD_CLASS_NEW(AOUT_HW_CLS);

    if (prThis)
    {
        prThis->eAoutId = eAoutId;
            
        prThis->rPub.Delete = AoutHw_Delete;
        
        prThis->rPub.InitCfg = AoutHw_InitCfg;
        prThis->rPub.Enable = AoutHw_Enable;
        prThis->rPub.SetNsadr = AoutHw_SetNsadr;
        prThis->rPub.SetIntClrBit = AoutHw_SetIntClrBit;
        prThis->rPub.SetDacSrc = AoutHw_SetDacSrc;
        prThis->rPub.SetDataSrc = AoutHw_SetDataSrc;
        prThis->rPub.SetArmCtrl = AoutHw_SetArmCtrl;
        prThis->rPub.IsArmCtrl = AoutHw_IsArmCtrl;
    }

    return ((PAOUT_HW_CLS_PUB)prThis);
}


