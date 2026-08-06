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

#ifndef __ARM2__
#include "linux/types.h"
#else
#include "x_types.h"
#endif
#include "aud_aout.h"
#include "reserved_memory.h"

#define     x_memset            memset

AOUT_HAL_T _rAoutHal;
PAOUT_HAL_T _prAout = NULL;
static RSV_MEM_T * dsp_rsv = NULL;
//==============================//
    #define CodeSight_AOUT_CLK
//==============================//

void IoClk_SetAout2Mclk()
{
    //select k4 as aout2 mclk source
    AUDREG_BITS_W(AUD_REG_AP_REG3, BIT_STR_AUD2_AP_SEL, BIT_NUM_AUD2_AP_SEL, AUD2_AP_ACLK_K4);
  
    //select apll for k4
    AUDREG_BITS_W(AUD_REG_AP_REG7, BIT_STR_SEL_APLL_K4, BIT_NUM_SEL_APLL_K4, AUD_DEF_CKGEN_APPL);
  
    //set mclk divider
    AUDREG_BITS_W(AUD_REG_AP_REG4, BIT_STR_CFG_REG_K4, BIT_NUM_CFG_REG_K4, AUD_DEF_MCLK_DIV);

    AUDLOG_INFO((T("[IoClk]SetAout2Mclk: MclkDiv(%d).\n"), AUD_DEF_MCLK_DIV));
}


//==============================//
    #define CodeSight_AOUT_HW
//==============================//

static void AoutHw_SetArmCtrl(BOOL fgEn)
{
    UINT32 u4Cfg = fgEn ? 0x36 : 0;
    AUDREG_BITS_W(REGENV_RGBK2_AOUT_CFG2, BIT_STR_AP_AOUT2_ARM_CTRL_CFG, BIT_NUM_AP_AOUT2_ARM_CTRL_CFG, u4Cfg);
}


static void AoutHw_Enable(BOOL fgEnable)
{
     AUDREG_BITS_W(REGENV_AOUT2_CTRL, BIT_STR_AOUT2_EN_PRE, BIT_NUM_AOUT2_EN_PRE, fgEnable);    
}


static void AoutHw_SetDataBitNum(UINT32 u4BitWidth)
{
    AUDREG_BITS_W(REGENV_MISC_CTRL, BIT_STR_AOUT2_DA_BNUM, BIT_NUM_AOUT2_DA_BNUM, u4BitWidth);
}
    

static void AoutHw_SetDataFmt(AUDFMT_INTF_E eFmt)
{
    AUDREG_BITS_W(REGENV_MISC_CTRL, BIT_STR_AOUT2_LEFT, BIT_NUM_AOUT2_LEFT, (eFmt & 0x1));
    AUDREG_BITS_W(REGENV_MISC_CTRL, BIT_STR_AOUT2_DELAY, BIT_NUM_AOUT2_DELAY, ((eFmt & 0x2) >> 1));
}


static void AoutHw_SetBckDivider()
{
    UINT32 u4MclkToBckRatio = AUD_DEF_MCLK / (32 * 4);
    AUDREG_BITS_W(REGENV_MISC_CTRL, BIT_STR_AOUT2_A2BCKX, BIT_NUM_AOUT2_A2BCKX, u4MclkToBckRatio);
}


static void AoutHw_SetLrckDivider(AUD_LRCK_CYC_T eCycle)
{
    AUDREG_BITS_W(REGENV_MISC_CTRL, BIT_STR_AOUT2_LRCK_CYC, BIT_NUM_AOUT2_LRCK_CYC, eCycle);
}


static void AoutHw_SetBckInvert(BOOL fgInvert)
{
    AUDREG_BITS_W(REGENV_MISC_CTRL, BIT_STR_INV_BCK2, BIT_NUM_INV_BCK2, fgInvert);
}


static void AoutHw_SetLrckInvert(BOOL fgInvert)
{
    AUDREG_BITS_W(REGENV_MISC_CTRL, BIT_STR_INV_LRCK2, BIT_NUM_INV_LRCK2, fgInvert);
}

static void AoutHw_SetDacSrc()
{
    AUDREG_BITS_W(REGENV_PWMTOP_CFG, BIT_STR_FRNT_PWM_IN_SEL, BIT_NUM_FRNT_PWM_IN_SEL, SRC_AOUT2);  // for PWM DAC
    AUDREG_BITS_W(REGENV_RGBK2_AOUT_CFG2, BIT_STR_FRNT_SRC_SEL, BIT_NUM_FRNT_SRC_SEL, SRC_AOUT2);   // for EXT DAC
}


static void AoutHw_SetBankAdr(UINT32 u4BankAdr)
{
    AUDREG_BITS_W(REGENV_AUD_DRAM_BANK, BIT_STR_ADSP_BANK, BIT_NUM_ADSP_BANK, u4BankAdr);
}


static void AoutHw_SetBlkAdr(UINT32 u4BlkAdr)
{
    AUDREG_BITS_W(REGENV_RWD_BLK67, BIT_STR_DRAM_SBLK7, BIT_NUM_DRAM_SBLK7, u4BlkAdr);
}


static void AoutHw_SetChAdr(UINT32 u4ChAdr)
{
    AUDREG_BITS_W(REGENV_AOUT2_CH1_BUF_SADR, BIT_STR_AOUT2_CH1_BUF_SADR, BIT_NUM_AOUT2_CH1_BUF_SADR, u4ChAdr);
}


static void AoutHw_SetChSize(UINT32 u4ChSize)
{
    AUDREG_BITS_W(REGENV_AOUT2_CH1_BUF_SIZE, BIT_STR_AOUT2_CH1_BUF_SIZE, BIT_NUM_AOUT2_CH1_BUF_SIZE, u4ChSize);
}


static void AoutHw_SetChNumber(UINT32 u4ChNum)
{   
    AUDREG_BITS_W(REGENV_AOUT2_CH_NUM, BIT_STR_AOUT2_CH_NUM, BIT_NUM_AOUT2_CH_NUM, u4ChNum);
}


static void AoutHw_SetChConfigure(UINT32 u4CfgId, UINT32 u4ChCfg)
{
    AUDREG_WRITE(REGENV_AOUT2_CH_CFG(u4CfgId), u4ChCfg);
}


static void AoutHw_SetNsadr(UINT32 u4ChIdx, UINT32 u4Nsadr)
{
    AUDREG_BITS_W(REGENV_AOUT2_CH_NSADR(u4ChIdx), BIT_STR_AOUT2_CH_NSADR, BIT_NUM_AOUT2_CH_NSADR, (u4Nsadr >> 2));
}


static void AoutHw_SetIntSize(UINT32 u4IntSize)
{
    AUDREG_BITS_W(REGENV_AOUT2_INTRSIZE, BIT_STR_AOUT2_INTRSIZE, BIT_NUM_AOUT2_INTRSIZE, u4IntSize);
}


static void AoutHw_SetNextSampleNum(UINT32 u4Nsnum)
{  
    AUDREG_BITS_W(REGENV_AOUT2_NSNUM, BIT_STR_AOUT2_NSNUM, BIT_NUM_AOUT2_NSNUM, u4Nsnum);
}


static void AoutHw_IntrClear()
{
    AUDREG_BITS_W(REGENV_RGBK2_CFG5, BIT_STR_AO2_INT_CLR, BIT_NUM_AO2_INT_CLR, 0);
    BIM_ClearIrq(VECTOR_AOUT_2ND_RC);
    AUDREG_BITS_W(REGENV_RGBK2_CFG5, BIT_STR_AO2_INT_CLR, BIT_NUM_AO2_INT_CLR, 1);
}


static void AoutHw_SetAoutAsDataPin(AUD_DAC_TYPE_T eDacType, ASDATA_PIN eAsdataPin, ASDATA_PIN_SRC ePinSrc)
{
    UINT32 u4SAdr, u4BitStart, u4BitNum;
    
    if (eDacType == AUD_DAC_PWM)
    {
        u4SAdr = REGENV_PWMIP_S_DAC_CH_CFG;
        u4BitNum = BIT_NUM_PWM2_CH_CFG;
        u4BitStart = BIT_STR_PWM2_CH_CFG(eAsdataPin - ASDATA_PIN0);
    }
    else
    {
        u4SAdr = REGENV_AOUT_CFG1;
        u4BitNum = BIT_NUM_AOSDATA;
        u4BitStart = BIT_STR_AOSDATA(eAsdataPin - ASDATA_PIN0);
    }
    
    AUDREG_BITS_W(u4SAdr, u4BitStart, u4BitNum, ePinSrc);
}

//==========================================//
    #define CodeSight_AOUT_HAL_Static
//==========================================//
 
static UINT32 AoutHal_RegSRCB(PFN_ISR_CB pfnCb)
{
    _prAout->pfnCb = pfnCb;
    
    if (pfnCb)
    {    
        AoutHw_SetNextSampleNum(_prAout->u4NSNum);
        AoutHw_SetIntSize(_prAout->u4IntrSz);
    }
    else
    {
        AoutHw_SetNsadr(0, _prAout->u4LSAdr);
        AoutHw_SetNsadr(1, _prAout->u4RSAdr);
        AoutHw_SetNextSampleNum(_prAout->u4ChBufSz >> (AOUT_DEF_BPS - 1));
    }

    return (AUD_RET_OK);
}


static VOID AoutHal_SetChCfg(UINT32 u4ChCfg0, UINT32 u4ChCfg1, UINT32 u4ChNum)
{
    _prAout->u4ChNum = u4ChNum;
    AoutHw_SetChNumber(_prAout->u4ChNum);

    AoutHw_SetChConfigure(0, u4ChCfg0);
    AoutHw_SetChConfigure(1, u4ChCfg1);
}


static VOID AoutHal_ClearBuf()
{
    x_memset((VOID *)(_prAout->u4LBuff), 0, _prAout->u4ChBufSz);
    x_memset((VOID *)(_prAout->u4RBuff), 0, _prAout->u4ChBufSz);
}


static VOID AudHal_UpdateNsadr(VOID)
{
    if (_prAout->u4RP != _prAout->u4PrevRP)
    {
        // Reset played buffer to zero (silence data in 16 bits)
        UINT32 u4Size1, u4Size2;
        
        if ((_prAout->u4PrevRP + _prAout->u4NSNumSz) > _prAout->u4ChBufSz) {
            u4Size1 = _prAout->u4ChBufSz  -  _prAout->u4PrevRP;
            u4Size2 = _prAout->u4NSNumSz - u4Size1;
        }  else {
            u4Size1 = _prAout->u4NSNumSz;
            u4Size2 = 0;
        }

        x_memset((VOID *)(_prAout->u4LBuff + _prAout->u4PrevRP), 0, u4Size1);
        x_memset((VOID *)(_prAout->u4RBuff + _prAout->u4PrevRP), 0, u4Size1);
        if (u4Size2) {
            x_memset((VOID *)(_prAout->u4LBuff), 0, u4Size2);
            x_memset((VOID *)(_prAout->u4RBuff), 0, u4Size2);
        }             
    }
    
    _prAout->u4PrevRP = _prAout->u4RP;
    _prAout->u4RP += _prAout->u4NSNumSz;
    if (_prAout->u4RP >= _prAout->u4ChBufSz) {
        _prAout->u4RP = 0;
    }
    _prAout->u4NextRP = _prAout->u4RP + _prAout->u4NSNumSz;
    if (_prAout->u4NextRP >= _prAout->u4ChBufSz) {
        _prAout->u4NextRP = 0;
    }
   
    AoutHw_SetNsadr(0, _prAout->u4LSAdr + _prAout->u4RP);
    AoutHw_SetNsadr(1, _prAout->u4RSAdr + _prAout->u4RP);
}


static VOID AoutHal_KernalInit(UINT32 u4SpeakerNum)
{   
	UINT32 addr = dsp_rsv->start_addr >> 20;
    AoutHw_SetArmCtrl(TRUE); 
    IoPinMux_SetCfg(AUD_DEF_DAC_TYPE);
    AoutHw_SetDacSrc(); 
      
    AoutHw_SetBankAdr(addr);
    AoutHw_SetBlkAdr(AOUT2_BLK_ADDR);
    AoutHw_SetChAdr(AOUT2_OFFSET >> 2);         //byte -> dword
    AoutHw_SetChSize(_prAout->u4ChBufSz);

    AoutHw_SetChConfigure(0, AUD_DEF_CH_CFG0);
    if (u4SpeakerNum != 4)
    {
        AoutHw_SetChConfigure(1, AUD_DEF_CH_CFG1);
    }
    else
	{
		AoutHw_SetChConfigure(1, AUD_DEF_CH_CFG1_2TO4);
	}
    AoutHw_SetChNumber(AUD_DEF_CH_NUM);

    AoutHw_SetNextSampleNum(_prAout->u4NSNum);
    AoutHw_SetIntSize(_prAout->u4IntrSz);
}


//==========================================//
    #define CodeSight_AOUT_HAL_Public
//==========================================//
#define AUD_REG_GPIO_EN5  0x88
#define AUD_REG_GPIO_OUT5 0xf4

void vAudMuteCircuitCtrl(BOOL fgMute)
{
    //first config GOIO162 GPIO FUNCTION
    AUD_CKGEN_CLRBITS(AUD_REG_PAD_MUX1, 0x3<<28);  //0x58[28:29] =0
    AUD_CKGEN_CLRBITS(AUD_REG_PAD_MUX4, 0x3<<2);  //0x64[2:3] = 0

    //output
    AUD_CKGEN_SETBITS(AUD_REG_GPIO_EN5, 0x1<<2);  //0x88[2] = 1

    if(TRUE == fgMute)
    {
        AUD_CKGEN_CLRBITS(AUD_REG_GPIO_OUT5, 0x1<<2);  //0xf4[2] = 0
    }
    else
    {
        AUD_CKGEN_SETBITS(AUD_REG_GPIO_OUT5, 0x1<<2);  //0xf4[2] = 1
    }
}

UINT32 AoutHal_New(PFN_ISR_CB pfnCb)
{
	UINT32 addr_ddr = dsp_rsv->start_addr >> 20;
	UINT32 addr_phy = (((addr_ddr) << 20) + ((AOUT2_BLK_ADDR)<<8) + AOUT2_OFFSET);
    if (_prAout == NULL) 
    {
        _prAout = &_rAoutHal;

        _prAout->u4NSNum = AOUT_DEF_FS / 1000 * AOUT_INTR_TIME; 
        _prAout->u4NSNumSz = _prAout->u4NSNum * AOUT_DEF_BPS;
        _prAout->u4IntrSz = _prAout->u4NSNum >> 1;
        _prAout->pfnCb = pfnCb;

        _prAout->u4ChBufSz = Aout2_BUFFER_SIZE >> 1;
        _prAout->u4LBuff = (UINT32) ARM1PHY2ARM2UCV(addr_phy);
        _prAout->u4RBuff = _prAout->u4LBuff + _prAout->u4ChBufSz;
        _prAout->u4LSAdr = AOUT2_OFFSET;
        _prAout->u4RSAdr = _prAout->u4LSAdr + _prAout->u4ChBufSz; 

        x_memset((VOID *)(_prAout->u4LBuff), 0, _prAout->u4ChBufSz);
        x_memset((VOID *)(_prAout->u4RBuff), 0, _prAout->u4ChBufSz);
        
        _prAout->u4State = AUD_STATE_UNINIT;
    }

    // Add for init pinmux in arm2 init process.
    IoPinMux_SetCfg(AUD_DEF_DAC_TYPE);

    return (_prAout->u4State);
}


UINT32 AoutHal_Init()
{
	dsp_rsv = get_rsv_mem_by_name("dsp");
	if (NULL == dsp_rsv) {
		return AUD_STATE_INIT_RSV_FAILED;
	}

    IoClk_SetAout2Mclk();
    
    if (AUD_DEF_DAC_TYPE == AUD_DAC_PWM) {
        DacHal_PwmStart(AUD_DEF_CH_NUM);  
    }

    AoutHw_SetDataBitNum(AOUT_DEF_BW);
    AoutHw_SetDataFmt(AUDFMT_IIS);
    AoutHw_SetBckDivider();
    AoutHw_SetLrckDivider(AUD_LRCK_CYC_32);
    AoutHw_SetBckInvert(TRUE);
    AoutHw_SetLrckInvert(FALSE);

    AoutHw_SetAoutAsDataPin(AUD_DEF_DAC_TYPE, ASDATA_PIN0, ASDATA_FLFR);
    AoutHw_SetAoutAsDataPin(AUD_DEF_DAC_TYPE, ASDATA_PIN1, ASDATA_RLRR);
    AoutHw_SetAoutAsDataPin(AUD_DEF_DAC_TYPE, ASDATA_PIN2, ASDATA_CLFE);
    AoutHw_SetAoutAsDataPin(AUD_DEF_DAC_TYPE, ASDATA_PIN3, ASDATA_FLFR);  

    AoutHw_SetArmCtrl(TRUE);
    _prAout->u4State = AUD_STATE_INITED;
    
    return (_prAout->u4State);
}


UINT32 AoutHal_UnInit(VOID)
{
    AoutHal_ClearBuf();
    AoutHw_SetNsadr(0, _prAout->u4LSAdr);
    AoutHw_SetNsadr(1, _prAout->u4RSAdr);

    AoutHw_SetArmCtrl(TRUE); 
    _prAout->u4State = AUD_STATE_UNINIT;
    
    return (_prAout->u4State);
}


BOOL AoutHal_Enable(BOOL fgEnable, UINT32 u4SpeakerNum)
{
    BOOL fgRet = FALSE;

    if (fgEnable != _prAout->fgEnable)
    {
        _prAout->u4RP = 0;
        _prAout->u4PrevRP = 0;

        if (fgEnable)
        {
            AoutHal_KernalInit(u4SpeakerNum);
            AoutHw_SetNsadr(0, _prAout->u4LSAdr);
            AoutHw_SetNsadr(1, _prAout->u4RSAdr);
            _prAout->u4NextRP = _prAout->u4RP + _prAout->u4NSNumSz;
            _prAout->u4LastIntr = GetARM2TickCount();       
            AoutHw_IntrClear();          
        }
        else
        {
            _prAout->u4NextRP = 0;
            AoutHal_ClearBuf();
        }

        AoutHw_Enable(fgEnable);
        _prAout->fgEnable = fgEnable;
        fgRet = TRUE;
    }

    return (fgRet);
}


UINT32 AOutHal_GetRP(VOID)
{
    return (_prAout->u4RP);
}


UINT32 AOutHal_GetBuf(DATA_BUFFER_T *prBuffer)
{
    prBuffer->u4Buf1 = _prAout->u4LBuff;
    prBuffer->u4Buf2 = _prAout->u4RBuff;
    prBuffer->u4Chn = 2;
    prBuffer->u4DataOff = 0;
    prBuffer->u4BufSize = _prAout->u4ChBufSz;
    prBuffer->u4DataSize = _prAout->u4ChBufSz;
    
    return AUD_RET_OK;
}


void AudHal_ISR(UINT16 u2Vector)
{    
    UINT32 u4Time = GetARM2TickCount();   
    if ((u4Time - _prAout->u4LastIntr) >= (AOUT_INTR_TIME + (AOUT_INTR_TIME >> 1)))
    {
        AUDLOG_INFO((T("Pcm Play Intr Interval(%d ms).\r\n"), u4Time - _prAout->u4LastIntr));
    }
    _prAout->u4LastIntr = u4Time;

    AudHal_UpdateNsadr();

    if (_prAout->pfnCb) {
        _prAout->pfnCb(_prAout->u4NSNumSz);
    }

    AoutHw_IntrClear();    
}


