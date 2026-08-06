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

#ifdef audio_clock_standardization
#include <linux/clk.h>
#include <linux/clk-private.h>

typedef struct
{
    struct clk* clock;
    int  index;
    char* clk_name;
}AUD_CLK_T;

typedef enum
{
    AUD_CLK_AUD_DVD     = 0,
    AUD_CLK_ADSP,
    AUD_CLK_AUD,
    AUD_CLK_AUD2,
    AUD_CLK_MPHON,
    AUD_CLK_ARM_AUD     = 5,
    AUD_CLK_BT_MIC_AUD,
    AUD_CLK_AUDIO_K1,
    AUD_CLK_AUDIO_K2,
    AUD_CLK_AUDIO_K3,
    AUD_CLK_AUDIO_K4    = 10,
    AUD_CLK_AUDIO_K5,
    AUD_CLK_AUDIO_K6,
    AUD_CLK_AUDIO_K7,
    AUD_CLK_AUDIO_K8,
    AUD_CLK_AUDIO_K9    = 15,
    AUD_CLK_AUDIO_K10,
    AUD_CLK_AUDIO_K11,
    AUD_CLK_AUDIO_K12,
    AUD_CLK_AUDIO_K13,
    AUD_CLK_AUDIO_K14   = 20,
    AUD_CLK_AUDIO_A1,
    AUD_CLK_AUDIO_A2,
    AUD_CLK_AUDIO_A3,
    AUD_CLK_AUD_ADC,
    AUD_CLK_AUD_PWM     = 25,
    AUD_CLK_MLIN2,
    AUD_CLK_AUD_MPH,
    AUD_CLK_MLIN,
    AUD_CLK_AUD_A1_TST,
    AUD_CLK_AUD_A2_TST  = 30,
    AUD_CLK_TOP_MUX_NUM
}AUD_TOP_MUX_CLK_ID;

static AUD_CLK_T aud_top_mux_clk[AUD_CLK_TOP_MUX_NUM] =
{
    {NULL, AUD_CLK_AUD_DVD,     "aud_dvd_sel"},  // index 0
    {NULL, AUD_CLK_ADSP,        "adsp_sel"},
    {NULL, AUD_CLK_AUD,         "aud_sel"},
    {NULL, AUD_CLK_AUD2,        "aud2_sel"},
    {NULL, AUD_CLK_MPHON,       "mphon_sel"},
    {NULL, AUD_CLK_ARM_AUD,     "arm_aud_sel"},   // index 5
    {NULL, AUD_CLK_BT_MIC_AUD,  "bt_mic_aud_sel"},
    {NULL, AUD_CLK_AUDIO_K1,    "audio_k1_sel"},
    {NULL, AUD_CLK_AUDIO_K2,    "audio_k2_sel"},
    {NULL, AUD_CLK_AUDIO_K3,    "audio_k3_sel"},
    {NULL, AUD_CLK_AUDIO_K4,    "audio_k4_sel"},   // index 10
    {NULL, AUD_CLK_AUDIO_K5,    "audio_k5_sel"},
    {NULL, AUD_CLK_AUDIO_K6,    "audio_k6_sel"},
    {NULL, AUD_CLK_AUDIO_K7,    "audio_k7_sel"},
    {NULL, AUD_CLK_AUDIO_K8,    "audio_k8_sel"},
    {NULL, AUD_CLK_AUDIO_K9,    "audio_k9_sel"},   // index 15
    {NULL, AUD_CLK_AUDIO_K10,   "audio_k10_sel"},
    {NULL, AUD_CLK_AUDIO_K11,   "audio_k11_sel"},
    {NULL, AUD_CLK_AUDIO_K12,   "audio_k12_sel"},
    {NULL, AUD_CLK_AUDIO_K13,   "audio_k13_sel"},
    {NULL, AUD_CLK_AUDIO_K14,   "audio_k14_sel"},  // index 20
    {NULL, AUD_CLK_AUDIO_A1,    "audio_a1_sel"},
    {NULL, AUD_CLK_AUDIO_A2,    "audio_a2_sel"},
    {NULL, AUD_CLK_AUDIO_A3,    "audio_a3_sel"},
    {NULL, AUD_CLK_AUD_ADC,     "aud_adc_sel"},
    {NULL, AUD_CLK_AUD_PWM,     "aud_pwm_sel"},   // index 25
    {NULL, AUD_CLK_MLIN2,       "mlin2_sel"},
    {NULL, AUD_CLK_AUD_MPH,     "aud_mph_sel"},
    {NULL, AUD_CLK_MLIN,        "mlin_sel"},
    {NULL, AUD_CLK_AUD_A1_TST,  "aud_a1_tst_sel"},
    {NULL, AUD_CLK_AUD_A2_TST,  "aud_a2_tst_sel"}
};


typedef enum
{
    AUD_CLK_ACLK_K1,
    AUD_CLK_ACLK_K2,
    AUD_CLK_ACLK_K3,
    AUD_CLK_ACLK_K4,
    AUD_CLK_ACLK_K5,
    AUD_CLK_ACLK_K6,
    AUD_CLK_ACLK_K7,
    AUD_CLK_ACLK_K8,
    AUD_CLK_ACLK_K9,
    AUD_CLK_ACLK_K10,
    AUD_CLK_ACLK_K11,
    AUD_CLK_ACLK_K12,
    AUD_CLK_ACLK_K13,
    AUD_CLK_ACLK_K14,
    AUD_CLK_ACLK_A1,
    AUD_CLK_ACLK_A2,
    AUD_CLK_ACK_K9,
    AUD_CLK_ACK_K6,
    AUD_CLK_ACLK_NUM
}AUD_TOP_MUX_PARENT_CLK_ID;

static AUD_CLK_T aud_top_mux_parent_clk[AUD_CLK_TOP_MUX_NUM] =
{
    {NULL, AUD_CLK_ACLK_K1,  "aclk_k1"},
    {NULL, AUD_CLK_ACLK_K2,  "aclk_k2"},
    {NULL, AUD_CLK_ACLK_K3,  "aclk_k3"},
    {NULL, AUD_CLK_ACLK_K4,  "aclk_k4"},
    {NULL, AUD_CLK_ACLK_K5,  "aclk_k5"},
    {NULL, AUD_CLK_ACLK_K6,  "aclk_k6"},
    {NULL, AUD_CLK_ACLK_K7,  "aclk_k7"},
    {NULL, AUD_CLK_ACLK_K8,  "aclk_k8"},
    {NULL, AUD_CLK_ACLK_K9,  "aclk_k9"},
    {NULL, AUD_CLK_ACLK_K10, "aclk_k10"},
    {NULL, AUD_CLK_ACLK_K11, "aclk_k11"},
    {NULL, AUD_CLK_ACLK_K12, "aclk_k12"},
    {NULL, AUD_CLK_ACLK_K13, "aclk_k13"},
    {NULL, AUD_CLK_ACLK_K14, "aclk_k14"},
    {NULL, AUD_CLK_ACLK_A1, "aclk_a1"},
    {NULL, AUD_CLK_ACLK_A2, "aclk_a2"},
    {NULL, AUD_CLK_ACK_K9, "ack_k9"},
    {NULL, AUD_CLK_ACK_K6, "ack_k6"}
};

typedef enum
{
    SYS_CLK_APLL1   = 0,
    SYS_CLK_APLL2   = 1,
    SYS_CLK_APLL_NUM
}SYSTEM_TOP_MUX_CLK_ID;

static AUD_CLK_T sys_top_clk[] =
{
    {NULL, SYS_CLK_APLL1, "apll1_ck"},
    {NULL, SYS_CLK_APLL2, "apll2_ck"}
};

typedef enum
{
    AUD_K1_RATIO   = 0,
    AUD_K2_RATIO,
    AUD_K3_RATIO,
    AUD_K4_RATIO,
    AUD_K5_RATIO,
    AUD_K6_RATIO,
    AUD_K7_RATIO,
    AUD_K8_RATIO,
    AUD_K9_RATIO,
    AUD_K10_RATIO,
    AUD_K11_RATIO,
    AUD_K12_RATIO,
    AUD_K13_RATIO,
    AUD_K14_RATIO,
    AUD_A1_RATIO,
    AUD_A2_RATIO,
    AUD_A3_RATIO,
    AUD_RATIO_CLK_NUM
}AUD_DIVIDER_CLK_ID;

static AUD_CLK_T aud_div_clk[] =
{
    {NULL, AUD_K1_RATIO,  "aud_k1_ratio"},
    {NULL, AUD_K2_RATIO,  "aud_k2_ratio"},
    {NULL, AUD_K3_RATIO,  "aud_k3_ratio"},
    {NULL, AUD_K4_RATIO,  "aud_k4_ratio"},
    {NULL, AUD_K5_RATIO,  "aud_k5_ratio"},
    {NULL, AUD_K6_RATIO,  "aud_k6_ratio"},
    {NULL, AUD_K7_RATIO,  "aud_k7_ratio"},
    {NULL, AUD_K8_RATIO,  "aud_k8_ratio"},
    {NULL, AUD_K9_RATIO,  "aud_k9_ratio"},
    {NULL, AUD_K10_RATIO, "aud_k10_ratio"},
    {NULL, AUD_K11_RATIO, "aud_k11_ratio"},
    {NULL, AUD_K12_RATIO, "aud_k12_ratio"},
    {NULL, AUD_K13_RATIO, "aud_k13_ratio"},
    {NULL, AUD_K14_RATIO, "aud_k14_ratio"},
    {NULL, AUD_A1_RATIO,  "aud_a1_ratio"},
    {NULL, AUD_A2_RATIO,  "aud_a2_ratio"},
    {NULL, AUD_A3_RATIO,  "aud_a3_ratio"}
};


static AUD_CLK_T aud_arm_gate_clk[] =
{
    {NULL, AUD_GATE_FS_PWM_CLK,            "audio_peri_b02"},
    {NULL, AUD_GATE_RS_PWM_CLK,            "audio_peri_b03"},
    {NULL, AUD_GATE_MP_MLIN_MCLK,          "audio_peri_b08"},
    {NULL, AUD_GATE_MLIN2_MCLK,            "audio_peri_b09"},
    {NULL, AUD_GATE_AUD_IEC_CLK,           "audio_peri_b10"},
    {NULL, AUD_GATE_AUD2_CLK,              "audio_peri_b11"},
    {NULL, AUD_GATE_APLL_CLK_AP_ASRC_CLK,  "audio_peri_b12"},
    {NULL, AUD_GATE_APLL_CLK_GPS_ASRC_CLK, "audio_peri_b13"},
    {NULL, AUD_GATE_AFE_26M_CLK,           "audio_peri_b14"}
};

typedef enum
{
    AUD_GATE_DSPA_CLK      = 0,
    AUD_GATE_DSPB_CLK,
    AUD_GATE_RISCA_CLK,
    AUD_GATE_RISCB_CLK,
    AUD_GATE_DRAMA_CLK,
    AUD_GATE_DRAMB_CLK,
    AUD_DSP_GATE_CLK_NUM
}AUD_GATE_CLK_ID;

static AUD_CLK_T aud_dsp_gate_clk[] =
{
    {NULL, AUD_GATE_DSPA_CLK,              "audio_peri_b00"},
    {NULL, AUD_GATE_DSPB_CLK,              "audio_peri_b01"},
    {NULL, AUD_GATE_RISCA_CLK,             "audio_peri_b04"},
    {NULL, AUD_GATE_RISCB_CLK,             "audio_peri_b05"},
    {NULL, AUD_GATE_DRAMA_CLK,             "audio_peri_b06"},
    {NULL, AUD_GATE_DRAMB_CLK,             "audio_peri_b07"}
};

s32 IoClk_getAudioClock(void)
{
    u32 u4Ind = 0;
    // system apll clock
    for(u4Ind =0; u4Ind < SYS_CLK_APLL_NUM; u4Ind++){
        sys_top_clk[u4Ind].clock = clk_get(NULL, sys_top_clk[u4Ind].clk_name);

        if(IS_ERR(sys_top_clk[u4Ind].clock)){
            LOG(LOG_FAIL, "%s sys_top_clk %s error.", __func__, sys_top_clk[u4Ind].clk_name);
        }
    }

    // audio top mux parent clock
    for(u4Ind =0; u4Ind < AUD_CLK_ACLK_NUM; u4Ind++){
        aud_top_mux_parent_clk[u4Ind].clock = clk_get(NULL, aud_top_mux_parent_clk[u4Ind].clk_name);

        if(IS_ERR(aud_top_mux_parent_clk[u4Ind].clock)){
            LOG(LOG_FAIL, "%s sys_top_clk %s error.", __func__, aud_top_mux_parent_clk[u4Ind].clk_name);
        }
    }

    // audio top mux clock
    for(u4Ind =0; u4Ind < AUD_CLK_TOP_MUX_NUM; u4Ind++){
        aud_top_mux_clk[u4Ind].clock = clk_get(NULL, aud_top_mux_clk[u4Ind].clk_name);

        if(IS_ERR(aud_top_mux_clk[u4Ind].clock)){
            LOG(LOG_FAIL, "%s aud_top_mux_clk %s error.", __func__, aud_top_mux_clk[u4Ind].clk_name);
        }
    }

    // audio divider clock
    for(u4Ind =0; u4Ind < AUD_RATIO_CLK_NUM; u4Ind++){
        aud_div_clk[u4Ind].clock = clk_get(NULL, aud_div_clk[u4Ind].clk_name);

        if(IS_ERR(aud_div_clk[u4Ind].clock)){
            LOG(LOG_FAIL, "%s aud_div_clk %s error.", __func__, aud_div_clk[u4Ind].clk_name);
        }
    }

    // audio arm gate clock
    for(u4Ind =0; u4Ind < AUD_ARM_GATE_CLK_NUM; u4Ind++){
        aud_arm_gate_clk[u4Ind].clock = clk_get(NULL, aud_arm_gate_clk[u4Ind].clk_name);

        if(IS_ERR(aud_arm_gate_clk[u4Ind].clock)){
            LOG(LOG_FAIL, "%s aud_arm_gate_clk %s error.", __func__, aud_arm_gate_clk[u4Ind].clk_name);
        }
    }

    // audio dsp gate clock
    for(u4Ind =0; u4Ind < AUD_DSP_GATE_CLK_NUM; u4Ind++){
        aud_dsp_gate_clk[u4Ind].clock = clk_get(NULL, aud_dsp_gate_clk[u4Ind].clk_name);

        if(IS_ERR(aud_dsp_gate_clk[u4Ind].clock)){
            LOG(LOG_FAIL, "%s aud_dsp_gate_clk %s error.", __func__, aud_dsp_gate_clk[u4Ind].clk_name);
        }
    }
}

#endif

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


#ifndef audio_clock_standardization
    //select k6 as mphone mclk source
    AUDREG_BITS_W(AUD_REG_AP_REG3, BIT_STR_MPHONE_AP_SEL, BIT_NUM_MPHONE_AP_SEL, MPHONE_ACK_K6);

    //select apll for k6
    AUDREG_BITS_W(AUD_REG_AP_REG7, BIT_STR_SEL_APLL_K6, BIT_NUM_SEL_APLL_K6, rMphMclkInfo.eApll);

    //set mclk divider
    AUDREG_BITS_W(AUD_REG_AP_REG5, BIT_STR_CFG_REG_K6, BIT_NUM_CFG_REG_K6, rMphMclkInfo.u4MclkDiv);

#else
    //select k6 as mphone mclk source
    if(clk_set_parent(aud_top_mux_clk[AUD_CLK_MPHON].clock, aud_top_mux_parent_clk[AUD_CLK_ACK_K6].clock))
        LOG(LOG_FAIL, "%s setting aud top mux clock id(%d) error.\r\n", __func__, AUD_CLK_MPHON);

    //select apll for k6
    if(clk_set_parent(aud_top_mux_clk[AUD_CLK_AUDIO_K6].clock, sys_top_clk[rMphMclkInfo.eApll].clock))
        LOG(LOG_FAIL, "%s setting aud top mux clock id(%d) error.\r\n", __func__, AUD_CLK_AUDIO_K6);


    // k6 divider select
    if(clk_set_rate(aud_div_clk[AUD_K6_RATIO].clock, rMphMclkInfo.u4MclkDiv))
        LOG(LOG_FAIL, "%s setting aud top mux divider id(%d) error.\r\n", __func__, AUD_K6_RATIO);

#endif
}

/**
 * function : set digital mic clk info
 *
 * @param [in]
 * @param [out]
 *
 * @return
 */
 // unused function
void IoClk_SetDigMphClk(void)
{
#ifndef audio_clock_standardization
    //select apll for k14
    AUDREG_BITS_W(AUD_REG_AP_REG8, BIT_STR_SEL_APLL_K14, BIT_NUM_SEL_APLL_K14, K4_SEL_26M_APLL);

    //set mclk divider
    AUDREG_BITS_W(AUD_REG_AP_REG7, BIT_STR_CFG_REG_K14, BIT_NUM_CFG_REG_K14, 0);
#else
    int i4Ret = 0;
    //select apll for k14
    if(clk_set_parent(aud_top_mux_clk[AUD_CLK_AUDIO_K14].clock, sys_top_clk[SYS_CLK_APLL1].clock))
        LOG(LOG_FAIL, "%s setting aud top mux clock id(%d) error.\r\n", __func__, AUD_CLK_AUDIO_K14);

    //set mclk divider
    if(clk_set_rate(aud_div_clk[AUD_CLK_AUDIO_K14].clock, 0))
        LOG(LOG_FAIL, "%s setting aud top mux clock id(%d) error.\r\n", __func__, AUD_CLK_AUDIO_K14);
#endif
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

#ifndef audio_clock_standardization
    //select k10 as adc clk source
    AUDREG_BITS_W(AUD_REG_AP_REG8, BIT_STR_ADC_SEL, BIT_NUM_ADC_SEL, ADC_ACK_K10);

    //select apll for k10
    AUDREG_BITS_W(AUD_REG_AP_REG8, BIT_STR_SEL_APLL_K10, BIT_NUM_SEL_APLL_K10, K10_SEL_26M_APLL);

    //set mclk divider
    AUDREG_BITS_W(AUD_REG_AP_REG0, BIT_STR_CFG_REG_K10, BIT_NUM_CFG_REG_K10, 3);
#else
    int i4Ret = 0;
    //select k10 as adc clk source
    if(clk_set_parent(aud_top_mux_clk[AUD_CLK_AUD_ADC].clock, aud_top_mux_parent_clk[AUD_CLK_ACLK_K10].clock))
        LOG(LOG_FAIL, "%s setting aud top mux clock id(%d) error.\r\n", __func__, AUD_CLK_AUD_ADC);

    //select apll for k10
    if(clk_set_parent(aud_top_mux_clk[AUD_CLK_AUDIO_K10].clock, sys_top_clk[SYS_CLK_APLL1].clock))
        LOG(LOG_FAIL, "%s setting aud top mux clock id(%d) error.\r\n", __func__, AUD_CLK_AUDIO_K10);

    // divider select
    if(clk_set_rate(aud_div_clk[AUD_K10_RATIO].clock, 3))
        LOG(LOG_FAIL, "%s setting aud top mux divider id(%d) error.\r\n", __func__, AUD_K10_RATIO);
#endif
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

#ifndef audio_clock_standardization
    //select k9 as pcm mclk
    AUDREG_BITS_W(AUD_REG_AP_REG3, BIT_STR_BT_PCM_SEL, BIT_NUM_BT_PCM_SEL, BT_PCM_ACK_K9);

    //select apll for k9
    AUDREG_BITS_W(AUD_REG_AP_REG8, BIT_STR_SEL_APLL_K9, BIT_NUM_SEL_APLL_K9, CKGEN_APLL2);

    //set divider for pcm clk
    AUDREG_BITS_W(AUD_REG_AP_REG6, BIT_STR_CFG_REG_K9, BIT_NUM_CFG_REG_K9, u4Divider);

#else
    //select k9 as pcm mclk
    if(clk_set_parent(aud_top_mux_clk[AUD_CLK_BT_MIC_AUD].clock, aud_top_mux_parent_clk[AUD_CLK_ACK_K9].clock))
        LOG(LOG_FAIL, "%s setting aud top mux clock id(%d) error.\r\n", __func__, AUD_CLK_BT_MIC_AUD);

    //select apll for k9
    if(clk_set_parent(aud_top_mux_clk[AUD_CLK_AUDIO_K9].clock, sys_top_clk[SYS_CLK_APLL2].clock))
        LOG(LOG_FAIL, "%s setting aud top mux clock id(%d) error.\r\n", __func__, AUD_CLK_AUDIO_K9);

    // divider select
    if(clk_set_rate(aud_div_clk[AUD_K9_RATIO].clock, u4Divider))
        LOG(LOG_FAIL, "%s setting aud top mux divider id(%d) error.\r\n", __func__, AUD_K9_RATIO);

#endif
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

#ifndef audio_clock_standardization
    //select k3 as line in mclk source
    AUDREG_BITS_W(AUD_REG_AP_REG8, BIT_STR_MLIN_SEL, BIT_NUM_MLIN_SEL, MLIN_ACK_K3);

    //select apll for k3
    AUDREG_BITS_W(AUD_REG_AP_REG7, BIT_STR_SEL_APLL_K3, BIT_NUM_SEL_APLL_K3, rLinMclkInfo.eApll);

    //set mclk divider
    AUDREG_BITS_W(AUD_REG_AP_REG4, BIT_STR_CFG_REG_K3, BIT_NUM_CFG_REG_K3, rLinMclkInfo.u4MclkDiv);

#else
    //select k3 as line in mclk source
    if(clk_set_parent(aud_top_mux_clk[AUD_CLK_MLIN].clock, aud_top_mux_parent_clk[AUD_CLK_ACLK_K3].clock))
        LOG(LOG_FAIL, "%s setting aud top mux clock id(%d) error.\r\n", __func__, AUD_CLK_MLIN);

    //select apll for k3
    if(clk_set_parent(aud_top_mux_clk[AUD_CLK_AUDIO_K3].clock, sys_top_clk[rLinMclkInfo.eApll].clock))
        LOG(LOG_FAIL, "%s setting aud top mux clock id(%d) error.\r\n", __func__, AUD_CLK_AUDIO_K3);

    // divider select
    if(clk_set_rate(aud_div_clk[AUD_K4_RATIO].clock, rLinMclkInfo.u4MclkDiv))
        LOG(LOG_FAIL, "%s setting aud top mux divider id(%d) error.\r\n", __func__, AUD_K4_RATIO);

#endif
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

#ifndef audio_clock_standardization
    //select k3 as line in mclk source
    AUDREG_BITS_W(AUD_REG_AP_REG8, BIT_STR_MLIN2_SEL, BIT_NUM_MLIN2_SEL, MLIN2_ACK_K12);
    //select apll for k12
    AUDREG_BITS_W(AUD_REG_AP_REG8, BIT_STR_SEL_APLL_K12, BIT_NUM_SEL_APLL_K12, rLin2MclkInfo.eApll);
    //set mclk divider
    AUDREG_BITS_W(AUD_REG_AP_REG6, BIT_STR_CFG_REG_K12, BIT_NUM_CFG_REG_K12, rLin2MclkInfo.u4MclkDiv);

#else
    //select k3 as line in mclk source
    if(clk_set_parent(aud_top_mux_clk[AUD_CLK_MLIN2].clock, aud_top_mux_parent_clk[AUD_CLK_ACLK_K12].clock))
        LOG(LOG_FAIL, "%s setting aud top mux clock id(%d) error.\r\n", __func__, AUD_CLK_MLIN2);

    //select apll for k3
    if(clk_set_parent(aud_top_mux_clk[AUD_CLK_AUDIO_K12].clock, sys_top_clk[rLin2MclkInfo.eApll].clock))
        LOG(LOG_FAIL, "%s setting aud top mux clock id(%d) error.\r\n", __func__, AUD_CLK_AUDIO_K12);

    // divider select
    if(clk_set_rate(aud_div_clk[AUD_K12_RATIO].clock, rLin2MclkInfo.u4MclkDiv))
        LOG(LOG_FAIL, "%s setting aud top mux divider id(%d) error.\r\n", __func__, AUD_K12_RATIO);

#endif
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

#ifndef audio_clock_standardization
    //select k2 as aout mclk source
    AUDREG_BITS_W(AUD_REG_AP_REG2, BIT_STR_AUD_AP_SEL, BIT_NUM_AUD_AP_SEL, AUD_AP_ACLK_K2);

    //select apll for k2
    AUDREG_BITS_W(AUD_REG_AP_REG7, BIT_STR_SEL_APLL_K2, BIT_NUM_SEL_APLL_K2, rAout1MclkInfo.eApll);

    //set mclk divider
    AUDREG_BITS_W(AUD_REG_AP_REG4, BIT_STR_CFG_REG_K2, BIT_NUM_CFG_REG_K2, rAout1MclkInfo.u4MclkDiv);

#else
    //select k2 as aout mclk source
    if(clk_set_parent(aud_top_mux_clk[AUD_CLK_AUD].clock, aud_top_mux_parent_clk[AUD_CLK_ACLK_K2].clock))
        LOG(LOG_FAIL, "%s setting aud top mux clock id(%d) error.\r\n", __func__, AUD_CLK_AUD);

    //select apll for k2
    if(clk_set_parent(aud_top_mux_clk[AUD_CLK_AUDIO_K2].clock, sys_top_clk[rAout1MclkInfo.eApll].clock))
        LOG(LOG_FAIL, "%s setting aud top mux clock id(%d) error.\r\n", __func__, AUD_CLK_AUDIO_K2);

    //set mclk divider
    if(clk_set_rate(aud_div_clk[AUD_K2_RATIO].clock, rAout1MclkInfo.u4MclkDiv))
        LOG(LOG_FAIL, "%s setting aud top mux divider id(%d) error.\r\n", __func__, AUD_K2_RATIO);

#endif

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

#ifndef audio_clock_standardization
    //select k4 as aout2 mclk source
    AUDREG_BITS_W(AUD_REG_AP_REG3, BIT_STR_AUD2_AP_SEL, BIT_NUM_AUD2_AP_SEL, AUD2_AP_ACLK_K4);

    //select apll for k4
    AUDREG_BITS_W(AUD_REG_AP_REG7, BIT_STR_SEL_APLL_K4, BIT_NUM_SEL_APLL_K4, rAout2MclkInfo.eApll);

    //set mclk divider
    AUDREG_BITS_W(AUD_REG_AP_REG4, BIT_STR_CFG_REG_K4, BIT_NUM_CFG_REG_K4, rAout2MclkInfo.u4MclkDiv);

#else
    //select k4 as aout mclk source
    if(clk_set_parent(aud_top_mux_clk[AUD_CLK_AUD2].clock, aud_top_mux_parent_clk[AUD_CLK_ACLK_K4].clock))
        LOG(LOG_FAIL, "%s setting aud top mux clock id(%d) error.\r\n", __func__, AUD_CLK_AUD2);

    //select apll for k4
    if(clk_set_parent(aud_top_mux_clk[AUD_CLK_AUDIO_K4].clock, sys_top_clk[rAout2MclkInfo.eApll].clock))
        LOG(LOG_FAIL, "%s setting aud top mux clock id(%d) error.\r\n", __func__, AUD_CLK_AUDIO_K4);

    //set mclk divider
    if(clk_set_rate(aud_div_clk[AUD_K4_RATIO].clock, rAout2MclkInfo.u4MclkDiv))
        LOG(LOG_FAIL, "%s setting aud top mux divider id(%d) error.\r\n", __func__, AUD_K4_RATIO);

#endif
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

#ifndef audio_clock_standardization
    //iec clcok is from K1, select apll for k1
    AUDREG_BITS_W(AUD_REG_AP_REG7, BIT_STR_SEL_APLL_K1, BIT_NUM_SEL_APLL_K1, rIecClkInfo.eApll);

    //set mclk divider
    AUDREG_BITS_W(AUD_REG_AP_REG4, BIT_STR_CFG_REG_K1, BIT_NUM_CFG_REG_K1, rIecClkInfo.u4MclkDiv);

#else
    //iec clcok is from K1, select apll for k1
    if(clk_set_parent(aud_top_mux_clk[AUD_CLK_AUDIO_K1].clock, sys_top_clk[rIecClkInfo.eApll].clock))
        LOG(LOG_FAIL, "%s setting aud top mux clock id(%d) error.\r\n", __func__, AUD_CLK_AUDIO_K1);

    //set mclk divider
    if(clk_set_rate(aud_div_clk[AUD_K1_RATIO].clock, rIecClkInfo.u4MclkDiv))
        LOG(LOG_FAIL, "%s setting aud top mux divider id(%d) error.\r\n", __func__, AUD_K1_RATIO);

#endif
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

#ifndef audio_clock_standardization
    //select k7 as dvd aout mclk source
    AUDREG_BITS_W(AUD_REG_DVD_REG1, BIT_STR_AUD_DVD_SEL, BIT_NUM_AUD_DVD_SEL, AUD_DVD_K7);

    //select apll for k7
    AUDREG_BITS_W(AUD_REG_AP_REG7, BIT_STR_SEL_APLL_K7, BIT_NUM_SEL_APLL_K7, rDvdAudClkInfo.eApll);

    //set mclk divider
    AUDREG_BITS_W(AUD_REG_AP_REG5, BIT_STR_CFG_REG_K7, BIT_NUM_CFG_REG_K7, rDvdAudClkInfo.u4MclkDiv);

#else
    //select k7 as dvd aout mclk source
    if(clk_set_parent(aud_top_mux_clk[AUD_CLK_AUD_DVD].clock, aud_top_mux_clk[AUD_CLK_ACLK_K7].clock))
        LOG(LOG_FAIL, "%s setting aud top mux clock id(%d) error.\r\n", __func__, AUD_CLK_AUD_DVD);

    //select apll for k7
    if(clk_set_parent(aud_top_mux_clk[AUD_CLK_AUDIO_K7].clock, sys_top_clk[rDvdAudClkInfo.eApll].clock))
        LOG(LOG_FAIL, "%s setting aud top mux clock id(%d) error.\r\n", __func__, AUD_CLK_AUDIO_K7);

    //set mclk divider
    if(clk_set_rate(aud_div_clk[AUD_K7_RATIO].clock, rDvdAudClkInfo.u4MclkDiv))
        LOG(LOG_FAIL, "%s setting aud top mux divider id(%d) error.\r\n", __func__, AUD_K7_RATIO);

#endif
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
void IoClk_SetGpsAsrcCliClk(MCLK_TYPE_T eMclkType, AUDIO_SAMPLING_T eFs)
{
    AUD_MCLK_INFO_T rApAsrcClkInfo;
    IoClk_GetMclkInfo(eMclkType, eFs, &rApAsrcClkInfo);

#ifndef audio_clock_standardization
    //select A2 as dvd aout mclk source
    AUDREG_BITS_W(AUD_REG_AP_REG9, BIT_STR_AP_ASRC_CLI_SEL, BIT_NUM_AP_ASRC_CLI_SEL, AP_ASRC_CLI_ACLK_A2);

    //select apll for A2
    AUDREG_BITS_W(AUD_REG_AP_REG8, BIT_STR_SEL_APLL_A2, BIT_NUM_SEL_APLL_A2, rApAsrcClkInfo.eApll);

    //set mclk divider
    AUDREG_BITS_W(AUD_REG_AP_REG6, BIT_STR_CFG_REG_A2, BIT_NUM_CFG_REG_A2, 0);

#else
    //select k10 as adc clk source
    if(clk_set_parent(aud_top_mux_clk[AUD_CLK_AUD_A2_TST].clock, aud_top_mux_parent_clk[AUD_CLK_ACLK_A2].clock))
        LOG(LOG_FAIL, "%s setting aud top mux clock id(%d) error.\r\n", __func__, AUD_CLK_AUD_A2_TST);

    //select apll for k10
    if(clk_set_parent(aud_top_mux_clk[AUD_CLK_AUDIO_A2].clock, sys_top_clk[rApAsrcClkInfo.eApll].clock))
        LOG(LOG_FAIL, "%s setting aud top mux clock id(%d) error.\r\n", __func__, AUD_CLK_AUDIO_A2);

    // divider select
    if(clk_set_rate(aud_div_clk[AUD_A2_RATIO].clock, 0))
        LOG(LOG_FAIL, "%s setting aud top mux divider id(%d) error.\r\n", __func__, AUD_CLK_AUDIO_A2);

#endif
}

/**
 * function : set gps asrc calibration clock info
 *
 * @param [in]  eMclkType : mlck type, eFs : Fs Type
 * @param [out]
 *
 * @return
 */
void IoClk_SetApAsrcCliClk(MCLK_TYPE_T eMclkType, AUDIO_SAMPLING_T eFs)
{
    AUD_MCLK_INFO_T rGpsAsrcClkInfo;
    IoClk_GetMclkInfo(eMclkType, eFs, &rGpsAsrcClkInfo);

#ifndef audio_clock_standardization
    //select A1 as dvd aout mclk source
    AUDREG_BITS_W(AUD_REG_AP_REG9, BIT_STR_GPS_ASRC_CLI_SEL, BIT_NUM_GPS_ASRC_CLI_SEL, AP_ASRC_CLI_ACLK_A1);

    //select apll for A1
    AUDREG_BITS_W(AUD_REG_AP_REG8, BIT_STR_SEL_APLL_A1, BIT_NUM_SEL_APLL_A1, rGpsAsrcClkInfo.eApll);

    //set mclk divider
    AUDREG_BITS_W(AUD_REG_AP_REG6, BIT_STR_CFG_REG_A1, BIT_NUM_CFG_REG_A1, 0);

#else
    //select k10 as adc clk source
    if(clk_set_parent(aud_top_mux_clk[AUD_CLK_AUD_A1_TST].clock, aud_top_mux_parent_clk[AUD_CLK_ACLK_A1].clock))
        LOG(LOG_FAIL, "%s setting aud top mux clock id(%d) error.\r\n", __func__, AUD_CLK_AUD_A1_TST);

    //select apll for k10
    if(clk_set_parent(aud_top_mux_clk[AUD_CLK_AUDIO_A1].clock, sys_top_clk[rGpsAsrcClkInfo.eApll].clock))
        LOG(LOG_FAIL, "%s setting aud top mux clock id(%d) error.\r\n", __func__, AUD_CLK_AUDIO_A1);

    // divider select
    if(clk_set_rate(aud_div_clk[AUD_A1_RATIO].clock, 0))
        LOG(LOG_FAIL, "%s setting aud top mux divider id(%d) error.\r\n", __func__, AUD_A1_RATIO);
#endif
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
#ifndef audio_clock_standardization
    AUD_CKGEN_SETBITS(AUD_REG_CLKGATE_CFG3, (ADSPA_CLK_PD | ADSPB_CLK_PD));
#else
    u32 u4Ind = 0;
    int i4Ret = 0;
    for(; u4Ind <AUD_DSP_GATE_CLK_NUM; u4Ind++){
        i4Ret = clk_prepare(aud_dsp_gate_clk[u4Ind].clock);
        if(i4Ret != 0){
            LOG(LOG_FAIL, "%s dsp gate clock id(%d) error.\r\n", __func__, u4Ind);
        }
    }
#endif
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
#ifndef audio_clock_standardization
    AUD_CKGEN_CLRBITS(AUD_REG_CLKGATE_CFG3, (ADSPA_CLK_PD | ADSPB_CLK_PD));
#else
    u32 u4Ind = 0;
    for(; u4Ind < AUD_DSP_GATE_CLK_NUM; u4Ind++){
        clk_unprepare(aud_dsp_gate_clk[u4Ind].clock);
    }
#endif
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
#ifndef audio_clock_standardization
    u8 u1BitCkgenStart = AUDCLK_PWCTL_CKGEN_BIT[eClkId].u1BitCkgen;
#endif
    u8 u1BitRgbk2Start = AUDCLK_PWCTL_CKGEN_BIT[eClkId].u1BitRgbk2;

    if (CLK_PWCTL_INVALID != u1BitRgbk2Start){
        AUDREG_BITS_W(REGENV_RGBK2_CFG1, u1BitRgbk2Start, 1, 0);
    }

#ifndef audio_clock_standardization
    if (CLK_PWCTL_INVALID != u1BitCkgenStart){
        AUDREG_BITS_W(AUD_REG_CLKGATE_CFG3, u1BitCkgenStart, 1, 1);
    }
#endif
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
#ifndef audio_clock_standardization
    u8 u1BitCkgenStart = AUDCLK_PWCTL_CKGEN_BIT[eClkId].u1BitCkgen;
#endif
    u8 u1BitRgbk2Start = AUDCLK_PWCTL_CKGEN_BIT[eClkId].u1BitRgbk2;

    if (CLK_PWCTL_INVALID != u1BitRgbk2Start){
        AUDREG_BITS_W(REGENV_RGBK2_CFG1, u1BitRgbk2Start, 1, 1);
    }

#ifndef audio_clock_standardization
    if(CLK_PWCTL_INVALID != u1BitCkgenStart){
        AUDREG_BITS_W(AUD_REG_CLKGATE_CFG3, u1BitCkgenStart, 1, 0);
    }
#endif
}

/**
 * function : set io module clock power down
 *
 * @param [in]  eClkId : clock id
 * @param [out]
 *
 * @return
 */

#ifdef audio_clock_standardization
void IoClk_SetSubGateClock(AUD_ARM_GATE_CLK_ID eClkId, u32 u4ctrl)
{
    int i4Ret;
    if(0 != u4ctrl){
        i4Ret = clk_prepare(aud_arm_gate_clk[eClkId].clock);
        if(i4Ret != 0){
            LOG(LOG_FAIL, "%s clk_prepare clock id(%d) error.\r\n", __func__, eClkId);
        }
    }
    else{
        clk_unprepare(aud_arm_gate_clk[eClkId].clock);
    }
}

void IoClk_SetGateClock(u32 u4ctrl)
{
    u32 u4Ind = 0;
    for(;u4Ind < AUD_ARM_GATE_CLK_NUM; u4Ind++){
        IoClk_SetSubGateClock(u4Ind, u4ctrl);
    }
}

#endif


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
#ifndef audio_clock_standardization
    AUDREG_BITS_W(AUD_REG_SYNC_RESET_CFG3, BIT_STR_DSPA_RESET, 2, 0);
    Sleep(1);
    AUDREG_BITS_W(AUD_REG_SYNC_RESET_CFG3, BIT_STR_DSPA_RESET, 2, 3);
#else

    clk_disable(aud_dsp_gate_clk[AUD_GATE_DSPA_CLK].clock);
    clk_disable(aud_dsp_gate_clk[AUD_GATE_DSPB_CLK].clock);

    Sleep(1);
    if(clk_enable(aud_dsp_gate_clk[AUD_GATE_DSPA_CLK].clock))
        LOG(LOG_FAIL, "%s clk_enable clock id(%d) error.\r\n", __func__, AUD_GATE_DSPA_CLK);

    if(clk_enable(aud_dsp_gate_clk[AUD_GATE_DSPB_CLK].clock))
        LOG(LOG_FAIL, "%s clk_enable clock id(%d) error.\r\n", __func__, AUD_GATE_DSPB_CLK);
 #endif
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
#ifndef audio_clock_standardization
    AUDREG_BITS_W(AUD_REG_SYNC_RESET_CFG3, BIT_STR_FS_PWMIP_RESET, 2, 0);
    Sleep(1);
    AUDREG_BITS_W(AUD_REG_SYNC_RESET_CFG3, BIT_STR_FS_PWMIP_RESET, 2, 3);
#else
    int i4Ret = 0;
    clk_disable(aud_arm_gate_clk[AUD_GATE_FS_PWM_CLK].clock);
    clk_disable(aud_arm_gate_clk[AUD_GATE_RS_PWM_CLK].clock);
    Sleep(1);
    if(clk_enable(aud_arm_gate_clk[AUD_GATE_FS_PWM_CLK].clock))
        LOG(LOG_FAIL, "%s clk_enable clock id(%d) error.\r\n", __func__, AUD_GATE_FS_PWM_CLK);

    if(clk_enable(aud_arm_gate_clk[AUD_GATE_RS_PWM_CLK].clock))
        LOG(LOG_FAIL, "%s clk_enable clock id(%d) error.\r\n", __func__, AUD_GATE_RS_PWM_CLK);

 #endif
}

#ifdef audio_clock_standardization
void IoClk_SetGateResetEnable(u32 u4ctrl)
{
    int i4Ret = 0;
    if(0 != u4ctrl){
        if(clk_enable(aud_dsp_gate_clk[AUD_GATE_DSPA_CLK].clock))
            LOG(LOG_FAIL, "%s clk_enable clock id(%d) error.\r\n", __func__, AUD_GATE_DSPA_CLK);

        if(clk_enable(aud_dsp_gate_clk[AUD_GATE_DSPB_CLK].clock))
            LOG(LOG_FAIL, "%s clk_enable clock id(%d) error.\r\n", __func__, AUD_GATE_DSPB_CLK);

        if(clk_enable(aud_arm_gate_clk[AUD_GATE_FS_PWM_CLK].clock))
            LOG(LOG_FAIL, "%s clk_enable clock id(%d) error.\r\n", __func__, AUD_GATE_FS_PWM_CLK);

        if(clk_enable(aud_arm_gate_clk[AUD_GATE_RS_PWM_CLK].clock))
            LOG(LOG_FAIL, "%s clk_enable clock id(%d) error.\r\n", __func__, AUD_GATE_RS_PWM_CLK);
    }
    else{
        clk_disable(aud_arm_gate_clk[AUD_GATE_FS_PWM_CLK].clock);
        clk_disable(aud_arm_gate_clk[AUD_GATE_RS_PWM_CLK].clock);

        clk_disable(aud_dsp_gate_clk[AUD_GATE_DSPA_CLK].clock);
        clk_disable(aud_dsp_gate_clk[AUD_GATE_DSPB_CLK].clock);
    }
}
#endif

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


/**
 * function : set dsp clcok repace function "vDspSetClock"
 *
 * @param [in]
 * @param [out]
 *
 * @return
 */

#ifdef audio_clock_standardization
u32 IoClk_SetDspClock(void)
{
    int ret = 0;
    struct clk * pDspParent = clk_get(NULL, "syspll_d2");
    ret= clk_set_parent(aud_top_mux_clk[AUD_CLK_ADSP].clock, pDspParent);

    if(ret)
        LOG(LOG_FAIL, "%s clk_set_parent clock id(%d) error.\r\n", __func__, AUD_CLK_ADSP);

    return ret;
}
#endif


