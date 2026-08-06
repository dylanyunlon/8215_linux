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

#ifndef __AUD_AOUT_H
#define __AUD_AOUT_H

#include "aud_pinmux.h"
#include "arm2pcmplay_if.h"

#ifdef __cplusplus
extern "C"
{
#endif

//===================================================================================================//

#define AOUT_DEF_FS             48000
#define AOUT_DEF_BW             24
#define AOUT_DEF_BPS            3               //BPS: bytes per sample = (AOUT_DEF_BW / 8) 

#define AUD_DEF_MCLK            256             //AUD_MCLK_256FS

#define APLL1_VALUE             270950400
#define APLL2_VALUE             294912000
            
#define AUD_DEF_APLL_VALUE      APLL2_VALUE
#define AUD_DEF_CKGEN_APPL      CKGEN_APLL2
    
#define AUD_DEF_MCLK_DIV       (AUD_DEF_APLL_VALUE / AUD_DEF_MCLK / AOUT_DEF_FS - 1)

//===================================================================================================//

#define AOUT_INTR_TIME          15

//#define AOUT2_BANK_ADDR         (AUDIO_DSP_MEM_PA >> 20)//  CE : ((RESERVED_AUDIO_BASE_VA - 0x8C000000) >> 20)

#ifdef MEMORY_SIZE_128M
#define AOUT2_BLK_ADDR          0x32b0
#else
#define AOUT2_BLK_ADDR          0x42b0
#endif

#define AOUT2_OFFSET            0x3B400

#define Aout2_BUFFER_SIZE       (8640*2)         //? 48 * 60 * 24 / 8 ?


//===================================================================================================//

typedef VOID (*PFN_ISR_CB) (UINT32 u4Param);


typedef enum
{
    SRC_AOUT1,
    SRC_AOUT2,
    SRC_DVD,
    SRC_MAX
}AUD_AOUT_SRC;


typedef enum
{
    ASDATA_PIN0,
    ASDATA_PIN1,
    ASDATA_PIN2,
    ASDATA_PIN3,
    ASDATA_PIN_MAX,
}ASDATA_PIN;


typedef enum
{
    ASDATA_FLFR,   //front seat left channel, right channel
    ASDATA_RLRR,   //rear seat left channel, right channel
    ASDATA_CLFE,   //Center, LFE channel
    ASDATA_CH7_8,  //downmix channel 7 & 8
    ASDATA_CH9_10, //downmix channel 9 & 10
    ASDATA_CH11_12,
    ASDATA_SRC_MAX,
}ASDATA_PIN_SRC;


typedef enum {
    AUDFMT_RIGHT_JUSTIFIED,
    AUDFMT_LEFT_JUSTIFIED,
    AUDFMT_RESERVD,
    AUDFMT_IIS,
    AUDFMT_UNDEF_INTF
} AUDFMT_INTF_E;


typedef enum
{ 
    AUD_LRCK_CYC_16,
    AUD_LRCK_CYC_24,
    AUD_LRCK_CYC_32
} AUD_LRCK_CYC_T;


typedef struct 
{
    UINT32 u4Buf1;          // Virtual start address of buffer of L Ch
    UINT32 u4Buf2;          // Virtual start address of buffer of R Ch
    UINT32 u4BufSize;       // Buffer size (in byte)
    UINT32 u4DataOff;       // Data start offset (in byte)
    UINT32 u4DataSize;      // Data size of every channels (in byte)
    UINT32 u4Chn;           // Number of  validate buffers. (1 or 2)
}DATA_BUFFER_T, *PDATA_BUFFER_T;


typedef struct
{                                                        //BIT CFG: 8 ~ 11,    12 ~ 15,    16 ~ 19,    20 ~ 23
    UINT32 u4ChCfg0;            //channel configure0                 FL          FR          C          CH7
    UINT32 u4ChCfg1;            //channel configure1                 RL          RR         LFE         CH8
    UINT32 u4ChNum;             //channel number

    void (*PFN_ISR_CB)(UINT32 u4Param);

}AOUT_EXTPARAMS_T, *PAOUT_EXTPARAMS_T;


typedef struct
{
    UINT32 u4State;
    BOOL fgEnable;

    UINT32 u4LBuff;         //vir addr
    UINT32 u4RBuff;
    UINT32 u4LSAdr;         //phy addr offset
    UINT32 u4RSAdr;
    UINT32 u4ChBufSz;
    UINT32 u4ChNum;

    UINT32 u4PrevRP;
    UINT32 u4RP;
    UINT32 u4NextRP;

    PFN_ISR_CB pfnCb;
    UINT32 u4NSNum;
    UINT32 u4NSNumSz;
    UINT32 u4IntrSz;

    UINT32 u4LastIntr;
    
}AOUT_HAL_T, *PAOUT_HAL_T;


UINT32 AoutHal_New(PFN_ISR_CB pfnCb);
UINT32 AoutHal_Init();
UINT32 AoutHal_UnInit();
BOOL AoutHal_Enable(BOOL fgEnable, UINT32 u4SpeakerNum);

UINT32 AOutHal_GetRP(VOID);
UINT32 AOutHal_GetBuf(DATA_BUFFER_T *prBuffer);

void vAudMuteCircuitCtrl(BOOL fgMute);

#ifdef __cplusplus
}
#endif


#endif /* __AUD_AOUT_H */

