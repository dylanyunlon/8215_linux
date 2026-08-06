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
#include "arm2pcmplay.h"

#define GAIN_MAP_SIZE       100

const UINT32 GainMap[GAIN_MAP_SIZE] =
{
    0xf1ad, // 0: -0.50 dB
    0xe429, // 1: -1.00 dB
    0xd765, // 2: -1.50 dB
    0xcb59, // 3: -2.00 dB
    0xbff9, // 4: -2.50 dB
    0xb53b, // 5: -3.00 dB
    0xab18, // 6: -3.50 dB
    0xa186, // 7: -4.00 dB
    0x987d, // 8: -4.50 dB
    0x8ff5, // 9: -5.00 dB
    0x87e8, // 10: -5.50 dB
    0x804d, // 11: -6.00 dB
    0x7920, // 12: -6.50 dB
    0x7259, // 13: -7.00 dB
    0x6bf4, // 14: -7.50 dB
    0x65ea, // 15: -8.00 dB
    0x6036, // 16: -8.50 dB
    0x5ad5, // 17: -9.00 dB
    0x55c0, // 18: -9.50 dB
    0x50f4, // 19: -10.00 dB
    0x4c6d, // 20: -10.50 dB
    0x4826, // 21: -11.00 dB
    0x441d, // 22: -11.50 dB
    0x404d, // 23: -12.00 dB
    0x3cb5, // 24: -12.50 dB
    0x394f, // 25: -13.00 dB
    0x361a, // 26: -13.50 dB
    0x3314, // 27: -14.00 dB
    0x3038, // 28: -14.50 dB
    0x2d86, // 29: -15.00 dB
    0x2afa, // 30: -15.50 dB
    0x2892, // 31: -16.00 dB
    0x264d, // 32: -16.50 dB
    0x2429, // 33: -17.00 dB
    0x2223, // 34: -17.50 dB
    0x203a, // 35: -18.00 dB
    0x1e6c, // 36: -18.50 dB
    0x1cb9, // 37: -19.00 dB
    0x1b1d, // 38: -19.50 dB
    0x1999, // 39: -20.00 dB
    0x182a, // 40: -20.50 dB
    0x16d0, // 41: -21.00 dB
    0x158a, // 42: -21.50 dB
    0x1455, // 43: -22.00 dB
    0x1332, // 44: -22.50 dB
    0x121f, // 45: -23.00 dB
    0x111c, // 46: -23.50 dB
    0x1027, // 47: -24.00 dB
    0x0f3f, // 48: -24.50 dB
    0x0e65, // 49: -25.00 dB
    0x0d97, // 50: -25.50 dB
    0x0cd4, // 51: -26.00 dB
    0x0c1c, // 52: -26.50 dB
    0x0b6f, // 53: -27.00 dB
    0x0acb, // 54: -27.50 dB
    0x0a31, // 55: -28.00 dB
    0x099f, // 56: -28.50 dB
    0x0915, // 57: -29.00 dB
    0x0893, // 58: -29.50 dB
    0x0818, // 59: -30.00 dB
    0x07a4, // 60: -30.50 dB
    0x0737, // 61: -31.00 dB
    0x06cf, // 62: -31.50 dB
    0x066e, // 63: -32.00 dB
    0x0612, // 64: -32.50 dB
    0x05bb, // 65: -33.00 dB
    0x0569, // 66: -33.50 dB
    0x051b, // 67: -34.00 dB
    0x04d2, // 68: -34.50 dB
    0x048d, // 69: -35.00 dB
    0x044c, // 70: -35.50 dB
    0x040e, // 71: -36.00 dB
    0x03d4, // 72: -36.50 dB
    0x039d, // 73: -37.00 dB
    0x0369, // 74: -37.50 dB
    0x0339, // 75: -38.00 dB
    0x030a, // 76: -38.50 dB
    0x02df, // 77: -39.00 dB
    0x02b6, // 78: -39.50 dB
    0x028f, // 79: -40.00 dB
    0x026a, // 80: -40.50 dB
    0x0248, // 81: -41.00 dB
    0x0227, // 82: -41.50 dB
    0x0208, // 83: -42.00 dB
    0x01eb, // 84: -42.50 dB
    0x01cf, // 85: -43.00 dB
    0x01b6, // 86: -43.50 dB
    0x019d, // 87: -44.00 dB
    0x0186, // 88: -44.50 dB
    0x0170, // 89: -45.00 dB
    0x015b, // 90: -45.50 dB
    0x0148, // 91: -46.00 dB
    0x0136, // 92: -46.50 dB
    0x0124, // 93: -47.00 dB
    0x0114, // 94: -47.50 dB
    0x0104, // 95: -48.00 dB
    0x00f6, // 96: -48.50 dB
    0x00e8, // 97: -49.00 dB
    0x00db, // 98: -49.50 dB
    0x00cf, // 99: -50.00 dB
};

/* add for arm2 aout init */
typedef enum {
    CLKPM_MPHONE,
    CLKPM_MLIN,
    CLKPM_MLIN2,
    CLKPM_IEC,
    CLKPM_AUD,
    CLKPM_AUD2,
    CLKPM_APLL_ADJ,
    CLKPM_APLL2_ADJ,
    CLKPM_STC_RISC,
    CLKPM_AXI,
    CLKPM_AP_ASRC,
    CLKPM_GPS_ASRC,
    CLKPM_AFE1_26M,
    CLKPM_AFE2_26M,
    CLKPM_FS_APLL,
    CLKPM_RS_APLL,
    CLKPM_MAX,
} AUD_CLK_POWER_CTL_MODULE_ID;

typedef struct {
    u8 u1BitCkgen;
    u8 u1BitRgbk2;
}AUD_CLK_PWCTL_T, *PAUD_CLK_PWCTL_T;

AUD_CLK_PWCTL_T AUDCLK_PWCTL_CKGEN_BIT[CLKPM_MAX] = {
    {8,    12},   //CLKPM_MPHONE
    {8,    13},   //CLKPM_MLIN
    {9,    0x20}, //CLKPM_MLIN2
    {10,   14},   //CLKPM_IEC
    {10,   0x20}, //CLKPM_AUD
    {11,   0x20}, //CLKPM_AUD2
    {0x20, 16},   //CLKPM_APLL_ADJ
    {0x20, 17},   //CLKPM_APLL2_ADJ
    {0x20, 10},   //CLKPM_STC_RISC
    {0x20, 9},    //CLKPM_AXI
    {12,   0x20}, //CLKPM_AP_ASRC
    {13,   0x20}, //CLKPM_GPS_ASRC
    {14,   18},   //CLKPM_AFE1_26M
    {14,   19},   //CLKPM_AFE2_26M
    {2,    0x20}, //CLKPM_FS_APLL
    {3,    0x20}, //CLKPM_RS_APLL
};
#define REGENV_RGBK2_CFG1           (AUD_REG_ENV_BASE2 + 0x084)

#define AUD_REG_TOP_MISC_BASE           (0x0000)

//audio peripher
#define AUD_REG_CLKGATE_CFG3            (AUD_REG_TOP_MISC_BASE + 0xA8)
#define CLK_PWCTL_INVALID   0x20
#define HAL_WRITE32(_reg_, _val_)    (*((volatile uint32_t*)(_reg_)) = (_val_))

#define IO_WRITE32(base, offset, value) HAL_WRITE32((base) + (offset), (value))
#define CKGEN_VIRT 0xF0000000

#define CKGEN_WRITE32(offset, value)  \
	IO_WRITE32(CKGEN_VIRT, (offset), (value))
#define BIT_STR_DSPA_CLK_PD 		0
#define BIT_STR_RISCA_BCK_PD		4
#define BIT_STR_DRAMA_CLK_PD		6
#define BIT_STR_DSPB_CLK_PD 		1
#define BIT_STR_RISCB_BCK_PD		5
#define BIT_STR_DRAMB_CLK_PD		7

#define ADSPA_CLK_PD ((1 << BIT_STR_DSPA_CLK_PD) | (1 << BIT_STR_RISCA_BCK_PD) | (1 << BIT_STR_DRAMA_CLK_PD))
#define ADSPB_CLK_PD ((1 << BIT_STR_DSPB_CLK_PD) | (1 << BIT_STR_RISCB_BCK_PD) | (1 << BIT_STR_DRAMB_CLK_PD))
#define AUD_REG_SYNC_RESET_CFG3         (AUD_REG_TOP_MISC_BASE + 0xC4)
#define BIT_STR_DSPA_RESET			0
#define BIT_STR_FS_PWMIP_RESET		2

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

static void IoClk_SetDspHwRest(void)
{
    AUDREG_BITS_W(AUD_REG_SYNC_RESET_CFG3, BIT_STR_DSPA_RESET, 2, 0);
    Sleep(1);
    AUDREG_BITS_W(AUD_REG_SYNC_RESET_CFG3, BIT_STR_DSPA_RESET, 2, 3);
}

static void IoClk_SetPwmHwRest(void)
{
    AUDREG_BITS_W(AUD_REG_SYNC_RESET_CFG3, BIT_STR_FS_PWMIP_RESET, 2, 0);
    Sleep(1);
    AUDREG_BITS_W(AUD_REG_SYNC_RESET_CFG3, BIT_STR_FS_PWMIP_RESET, 2, 3);
}

PCM_PLAY_T _rPcm;
PPCM_PLAY_T _prPcm = NULL;

extern void v_disable_bim_irq(UINT32 u4Id);
extern void v_enable_bim_irq(UINT32 u4Id);


//=====================================//
    #define CodeSight_Local_Fun
//=====================================//

static UINT32 InputData()
{       
    DATA_BUFFER_T rOutBuf;
    INT32 i4TmpData, i4Data;
    BYTE *pbDst[2];
    UINT32 i, u4FreeSz, u4AvaiBytes = 0;
    UINT32 u4OutRP, u4FillLen, u4FillOnceLen;
    UINT32 u4FadeOutStep = 0;
    UINT32 u4FadeOutSamples = 0;

    AOutHal_GetBuf(&rOutBuf);
    u4OutRP = AOutHal_GetRP();
    i = u4OutRP;

    if (u4OutRP <= _prPcm->u4OutWP) {
        u4OutRP += rOutBuf.u4BufSize;
    }
    u4FreeSz = u4OutRP - _prPcm->u4OutWP;
    u4FreeSz = (u4FreeSz > RESERVE_BUFFER) ? (u4FreeSz - RESERVE_BUFFER) : 0;

    u4FillLen = u4FreeSz / 3;   //(byte -> sample, 24bits/sample)
    u4FillLen <<= ((_prPcm->u4BW / 8 - 1) + (_prPcm->u4Chn - 1));  // bytes

    u4AvaiBytes = _prPcm->lpDataEnd - _prPcm->lpCurrData;    
    if (_prPcm->u4LoopCnt) {
        u4AvaiBytes += (_prPcm->lpDataEnd - _prPcm->lpDataStart) * (_prPcm->u4LoopCnt - 1);
    }
    
    if (u4FillLen > u4AvaiBytes) {
        u4FillLen = u4AvaiBytes;
    }  

    if (_prPcm->fgDoFadeOut) {
        u4FadeOutSamples = u4FillLen >> ((_prPcm->u4BW / 8 - 1) + (_prPcm->u4Chn - 1));
        u4FadeOutStep = u4FadeOutSamples;
    }

    while(u4FillLen)
    {
        u4FillOnceLen = _prPcm->lpDataEnd - _prPcm->lpCurrData; //buffer size by one time
        if (u4FillOnceLen > u4FillLen)  {
            u4FillOnceLen = u4FillLen;
        }
        u4FillLen -= u4FillOnceLen;
        
        while(u4FillOnceLen)
        {
            pbDst[0] = (BYTE *)(rOutBuf.u4Buf1 + _prPcm->u4OutWP); 
            pbDst[1] = (BYTE *)(rOutBuf.u4Buf2 + _prPcm->u4OutWP);

            if (_prPcm->u4BW == 8)
            {
                for (i = 0; i < _prPcm->u4Chn; i++)
                {
                    i4TmpData = ((INT32)(*_prPcm->lpCurrData)) - 128;
                    _prPcm->lpCurrData++;
                    i4Data = i4TmpData * _prPcm->u4FxGain[i];                    
                    i4Data >>= 8;
                    if (u4FadeOutSamples) {
                        i4Data = i4Data * u4FadeOutStep / u4FadeOutSamples;
                    }

                    *pbDst[i]++ = 0;
                    *pbDst[i]++ = 0;
                    *pbDst[i]++ = (BYTE)i4Data;
                    u4FillOnceLen--;
                }            
                if (1 == _prPcm->u4Chn)
                {
                    *pbDst[1]++ = 0;
                    *pbDst[1]++ = 0;
                    *pbDst[1]++ = (BYTE)i4Data;
                }
            }
            else
            {
                for (i = 0; i < _prPcm->u4Chn; i++)
                {
                    i4TmpData = (INT32)(*(INT16 *)_prPcm->lpCurrData);
                    _prPcm->lpCurrData += 2;     
                    i4Data = i4TmpData * _prPcm->u4FxGain[i];                    
                    i4Data >>= 16;
                    if (u4FadeOutSamples) {
                        i4Data = i4Data * u4FadeOutStep / u4FadeOutSamples;
                    }

                    *pbDst[i]++ = 0;
                    *pbDst[i]++ = (BYTE)i4Data;
                    *pbDst[i]++ = (BYTE)(i4Data >> 8);
                    u4FillOnceLen -= 2;
                }                   
                if (1 == _prPcm->u4Chn)
                {
                    *pbDst[i]++ = 0;
                    *pbDst[i]++ = (BYTE)i4Data;
                    *pbDst[i]++ = (BYTE)(i4Data >> 8);
                }
            }

            u4FadeOutStep--;
            
            _prPcm->u4OutWP += 3;
            if (_prPcm->u4OutWP >= rOutBuf.u4BufSize) {
                _prPcm->u4OutWP = 0;
            }
            
            if (_prPcm->lpCurrData == _prPcm->lpDataEnd)
            {
                if (_prPcm->u4LoopCnt > 1) {
                    _prPcm->u4LoopCnt --;
                    _prPcm->lpCurrData = _prPcm->lpDataStart;
                } else {
                    _prPcm->u4LoopCnt = 0;
                }
            }
            _prPcm->u4AOutLen += 3;
        } //while(u4FillOnceLen)
        
    }//    while(u4FillLen)

    return (AUD_RET_OK);
}


static VOID ARM2PCM_CB(UINT32 u4Param)
{
    UINT32 u4NSNumSz = u4Param;

    _prPcm->u4AOutLen = (_prPcm->u4AOutLen <= u4NSNumSz) ? (0) : (_prPcm->u4AOutLen - u4NSNumSz);
    InputData();
  
    if (!_prPcm->u4LoopCnt && !_prPcm->u4AOutLen && !_prPcm->fgDoFadeOut) {
        ARM2PCM_Stop();
    }  

    _prPcm->fgDoFadeOut = FALSE;
}


static VOID AudCfg_HWInit()
{
    AUDLOG_INFO((T("AudCfg_HWInit!\r\n")));

	AUD_CLK_POWER_CTL_MODULE_ID eClkPowerCtl;

	AUD_CKGEN_SETBITS(AUD_REG_CLKGATE_CFG3, (ADSPA_CLK_PD | ADSPB_CLK_PD));
    for (eClkPowerCtl = CLKPM_MPHONE; eClkPowerCtl < CLKPM_MAX; eClkPowerCtl++) {
        IoClk_SetModulePowerOn(eClkPowerCtl);
    }

    //audio related hw reset
    IoClk_SetDspHwRest();
    IoClk_SetPwmHwRest();

    if (AUD_DEF_DAC_TYPE == AUD_DAC_PWM)
    {
        DacHal_SetPwmBasicSetting();
    }

    //vAudMuteCircuitCtrl
    AUD_CKGEN_CLRBITS(AUD_REG_PAD_MUX1, 0x3 << 28);         
    AUD_CKGEN_CLRBITS(AUD_REG_PAD_MUX4, 0x3 << 2);          
    AUD_CKGEN_SETBITS(0x88, 0x1 << 2);                     
    AUD_CKGEN_SETBITS(0xf4, 0x1 << 2);                      

    AoutHal_Init();
}


static BOOL DoFadeOut()
{
    BOOL fgRet = TRUE;
    UINT32 u4Tick = GetARM2TickCount();
    
    if (_prPcm->eState == STR_RUN) 
    {
        if (_prPcm->fgDoFadeOut)
        {
            AUDLOG_INFO((T("Too stream wait for play! \r\n")));
            fgRet = FALSE;
        } 
        else 
        {          
            _prPcm->fgDoFadeOut = TRUE;         
            while (_prPcm->fgDoFadeOut && ((GetARM2TickCount() - u4Tick) <= AOUT_INTR_TIME)) 
            {}
            AUDLOG_INFO((T("DoFadeOut Finish (%d)ms! \r\n"), GetARM2TickCount() - u4Tick));
        }
    }

    return (fgRet);
}


//=====================================//
    #define CodeSight_Global_Fun
//=====================================//

UINT32 ARM2PCM_Start(PARM2PCM_FMT prFmt)
{
    AUDLOG_INFO((T("ARM2PCM Start (%d)! 2014-0926 \r\n"), _prPcm->eState));

    if ((prFmt->u4SampleRate != AOUT_DEF_FS) ||
        (prFmt->u4BitsPerSamples != 8 && prFmt->u4BitsPerSamples != 16) ||
        (prFmt->u4Channels > 2)) {
        AUDLOG_INFO((T("<***ERR***> Source Fmt Err: FS(%d) BW(%d) Chn(%d)\r\n"), 
            prFmt->u4SampleRate, prFmt->u4BitsPerSamples, prFmt->u4Channels));
    }

    if (DoFadeOut())
    {           
        _prPcm->u4BW = prFmt->u4BitsPerSamples;
        _prPcm->u4Chn = prFmt->u4Channels;
        _prPcm->lpDataStart = prFmt->lpData;
        _prPcm->lpCurrData = prFmt->lpData;
        _prPcm->lpDataEnd = prFmt->lpData + prFmt->u4Len;
        _prPcm->u4LoopCnt = prFmt->u4Loops;
       
        if (_prPcm->eState != STR_RUN)
        {
            _prPcm->u4AOutLen = 0;
            _prPcm->u4OutWP = 0;  
            InputData();
            AoutHal_Enable(TRUE, _prPcm->u4SpeakerNum);
            v_enable_bim_irq(VECTOR_AOUT_2ND_RC);
        }

        _prPcm->eState = STR_RUN;
    }

    return (_prPcm->eState);
}


UINT32 ARM2PCM_Stop()
{  
    if (_prPcm->eState == STR_RUN)
    {
        AUDLOG_INFO((T("ARM2PCM Stop!\r\n")));
        _prPcm->eState = STR_STOP;
    
        AoutHal_Enable(FALSE, _prPcm->u4SpeakerNum);
        v_disable_bim_irq(VECTOR_AOUT_2ND_RC);
    }

    return (_prPcm->eState);
}


VOID ARM2PCM_Init(UINT32 u4SpeakerNum)
{
    if (_prPcm) 
    {
        _prPcm->fgInitialized = FALSE;
    }
    else
    {
        _prPcm = &_rPcm;
    
        _prPcm->eState = STR_CLOSE;
        _prPcm->fgInitialized = FALSE;
        _prPcm->fgPowerOnByArm11 = FALSE;

        _prPcm->u4BW = 0;
        _prPcm->u4Chn = 0;
        _prPcm->lpCurrData = NULL;
        _prPcm->lpDataEnd = NULL;
        _prPcm->lpDataStart = NULL;
        _prPcm->u4LoopCnt = 0; 
        
        _prPcm->u4AOutLen = 0;
        _prPcm->u4OutWP = 0;

        _prPcm->u4FxGain[0] = AUD_DEF_LEFT_VOLUME; 
        _prPcm->u4FxGain[1] = AUD_DEF_RIGHT_VOLUME; 
        if ((u4SpeakerNum != 2) && (u4SpeakerNum != 4))
        {
            u4SpeakerNum = AUD_TWO_SPEAKERS;
        }
        _prPcm->u4SpeakerNum = u4SpeakerNum;

        AoutHal_New(ARM2PCM_CB);
    }
}

UINT32 ARM2PCM_Open()
{   
    UINT32 u4Time = GetARM2TickCount();

    AUDLOG_INFO((T("ARM2PCM Open (%d)!\r\n"), _prPcm->eState));
    ARM2PCM_Stop();
     
    if(!TAKE_AUD_POWER_ON_SEMAPHORE()) {
        AUDLOG_INFO((T("Power On By ARM 11!\r\n")));
        _prPcm->fgPowerOnByArm11 = TRUE; 
    }
    TAKE_GPS_AOUT_SEMAPHORE();
   
    if (!_prPcm->fgInitialized && !_prPcm->fgPowerOnByArm11) {  
        AudCfg_HWInit();             
    }  
    _prPcm->fgInitialized = TRUE;

    v_disable_bim_irq(VECTOR_AOUT_2ND_RC);
    _prPcm->eState = STR_OPEN;

    AUDLOG_INFO((T("ARM2PCM Open (%d ms)...\r\n"), GetARM2TickCount() - u4Time));
    
    return (_prPcm->eState);
}


UINT32 ARM2PCM_Close()
{
    if (_prPcm)
    {
        AUDLOG_INFO((T("ARM2PCM Close (%d)!\r\n"), _prPcm->eState));
        ARM2PCM_Stop();
        
        if (STR_CLOSE != _prPcm->eState)
        {
            RELEASE_GPS_AOUT_SEMAPHORE();
            RELEASE_AUD_POWER_ON_SEMAPHORE();
        }
        
        AoutHal_UnInit();
        _prPcm->eState = STR_CLOSE;
    }
    else
    {
        AUDLOG_INFO((T("ARM2PCM Close _prPcm = NULL\r\n")));
    }
    
    return (AUD_RET_OK);
}


