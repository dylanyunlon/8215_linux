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
 * @file aud_dac_hw.c source file
 * 
 * aud io dac module hardware driver
 * 
 * @author qiuhua.yin@autochips.com
 * 
 */

#include "aud_dac_hal.h"
#include "aud_reg_env.h"
#include "aud_reg_rgbk2.h"
#include "aud_reg_pwm.h"
#include "aud_reg_pwmip.h"


typedef struct 
{
    DAC_HW_CLS_PUB rPub;
    
    AUD_OUT_PATH_T eOutPath;
    AUD_DAC_TYPE_T eDacType;

}DAC_HW_CLS, *PDAC_HW_CLS;

typedef struct 
{
    u32 u4AddrReg;
    u32 u4DataReg;
}PWM_INDREG_DESC, *PPWM_INDREG_DESC;

PWM_INDREG_DESC aPwmIndRegDesc[PWM_SET_MAX] =
{
    {REGENV_PWMIP_NDRECT_ADDR_G1, REGENV_PWMIP_NDRECT_DATA_G1},
    {REGENV_PWMIP_NDRECT_ADDR_G2, REGENV_PWMIP_NDRECT_DATA_G2},
    {REGENV_PWMIP_NDRECT_ADDR_G3, REGENV_PWMIP_NDRECT_DATA_G3},
    {REGENV_PWMIP_NDRECT_ADDR_G4, REGENV_PWMIP_NDRECT_DATA_G4}
};

#define DACHW_PWM_YRAM_SIZE     215
#define DACHW_PWM_YRAM_IDX      5

//Fsout = 192.00 kHz, FsIn = 48.00 kHz, bw = 20.00 kHz, target att. = -100.00 dB
// 3-23 NS coefs bw=30k Hinf=16 order=5 QLEV=4, 24 intp gain, 25-72 intp coefs
u32 sg_yram_x03_20_ns40[DACHW_PWM_YRAM_SIZE] ={ 
0x3eb60, 0x2b9a0, 0x249f0, 0x0d900, 0xfde90, 0x00000, 0x00000, 0x0d1a00,
0xf2e60, 0x10000, 0x10000, 0x10000, 0x10000, 0x00000, 0x00000, 0x100000,
0x10000, 0x10000, 0xfee30, 0xfce30, 0x00000, 0x0c000, 0xfff70, 0xffed00,
0xfff60, 0x001a0, 0x00370, 0x001c0, 0xffc70, 0xff870, 0xffc20, 0x006e00,
0x00e70, 0x00780, 0xff410, 0xfe6d0, 0xff2d0, 0x01380, 0x02920, 0x015b00,
0xfe1b0, 0xfbf80, 0xfdda0, 0x02de0, 0x062c0, 0x03570, 0xfbb00, 0xf68e00,
0xfac40, 0x06a20, 0x0f060, 0x08b10, 0xf4d90, 0xe4c70, 0xee740, 0x199700,
0x55210, 0x7fff0, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x000000,
0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x000000,
0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x000000,
0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x000000,
0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x000000,
0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x000000,
0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x000000,
0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x000000,
0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x000000,
0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x000000,
0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x000000,
0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x000000,
0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x000000,
0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x000000,
0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x000000,
0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x000000,
0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x000000,
0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x000000,
0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x000000,
0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x0c000
};

//Fsout = 192.00 kHz, FsIn = 48.00 kHz, bw = 20.00 kHz, target att. = -100.00 dB
// 3-23 NS coefs bw=30k Hinf=16 order=5 QLEV=4, 24 intp gain, 25-72 intp coefs
u32 sg_yram_x03_20_ns40_adjGain[DACHW_PWM_YRAM_SIZE] ={
0x3eb60, 0x2b9a0, 0x249f0, 0x0d900, 0xfde90, 0x00000, 0x00000, 0x0d1a00,
0xf2e60, 0x10000, 0x10000, 0x10000, 0x10000, 0x00000, 0x00000, 0x100000,
0x10000, 0x10000, 0xfee30, 0xfce30, 0x00000, 0x0c000, 0xfff70, 0xffed00,
0xfff60, 0x001a0, 0x00370, 0x001c0, 0xffc70, 0xff870, 0xffc20, 0x006e00,
0x00e70, 0x00780, 0xff410, 0xfe6d0, 0xff2d0, 0x01380, 0x02920, 0x015b00,
0xfe1b0, 0xfbf80, 0xfdda0, 0x02de0, 0x062c0, 0x03570, 0xfbb00, 0xf68e00,
0xfac40, 0x06a20, 0x0f060, 0x08b10, 0xf4d90, 0xe4c70, 0xee740, 0x199700,
0x55210, 0x7fff0, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x000000,
0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x000000,
0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x000000,
0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x000000,
0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x000000,
0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x000000,
0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x000000,
0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x000000,
0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x000000,
0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x000000,
0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x000000,
0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x000000,
0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x000000,
0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x000000,
0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x000000,
0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x000000,
0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x000000,
0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x000000,
0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x000000,
0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x0c000,
};

// Fsout =192.00 kHz, FsIn = 48.00 kHz, bw = 20.00 kHz, target att. = -100.00 dB
// 3-23 NS coefs bw=30k Hinf=16 order=5 QLEV=4, 24 ch0 intp gain, 25-72 intp coefs,217 ch1 intp gain
u32 sg_yram_x04_20_ns40[DACHW_PWM_YRAM_SIZE] ={
0x3eb60, 0x2b9a0, 0x249f0, 0x0d900, 0xfde90, 0x00000, 0x00000, 0x0d1a00,
0xf2e60, 0x10000, 0x10000, 0x10000, 0x10000, 0x00000, 0x00000, 0x100000,
0x10000, 0x10000, 0xfee30, 0xfce30, 0x00000, 0x0dc20, 0xfffe0, 0xffff00,
0x00080, 0x00160, 0x00230, 0x00220, 0x00080, 0xffda0, 0xffae0, 0xffa300,
0xffd30, 0x00360, 0x009d0, 0x00c30, 0x00760, 0xffc00, 0xfef60, 0xfe9a00,
0xff0a0, 0x003c0, 0x019f0, 0x025b0, 0x01c50, 0xffe50, 0xfd990, 0xfc4100,
0xfcf70, 0xffc80, 0x036c0, 0x05c10, 0x04fd0, 0x00dd0, 0xfb360, 0xf73f00,
0xf7e60, 0xfde60, 0x06c90, 0x0dac0, 0x0d9f0, 0x04a40, 0xf5a90, 0xe83900,
0xe5920, 0xf4560, 0x14c40, 0x3f7a0, 0x67ac0, 0x7fff0, 0x00000, 0x000000,
0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x000000,
0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x000000,
0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x000000,
0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x000000,
0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x000000,
0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x000000,
0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x000000,
0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x000000,
0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x000000,
0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x000000,
0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x000000,
0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x000000,
0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x000000,
0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x000000,
0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x000000,
0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x000000,
0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x000000,
0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x0dc20
};

// Fsout =192.00 kHz, FsIn = 48.00 kHz, bw = 20.00 kHz, target att. = -100.00 dB
// 3-23 NS coefs bw=30k Hinf=16 order=5 QLEV=4, 24 ch0 intp gain, 25-72 intp coefs,217 ch1 intp gain
u32 sg_yram_x04_20_ns40_adjGain[DACHW_PWM_YRAM_SIZE] ={
0x3eb600, 0x2b9a00, 0x249f00, 0x0d9000, 0xfde900, 0x000000, 0x000000, 0x0d1a00,
0xf2e600, 0x100000, 0x100000, 0x100000, 0x100000, 0x000000, 0x000000, 0x100000,
0x100000, 0x100000, 0xfee300, 0xfce300, 0x000000, 0x0c0000, 0xfffe00, 0xffff00,
0x000800, 0x001600, 0x002300, 0x002200, 0x000800, 0xffda00, 0xffae00, 0xffa300,
0xffd300, 0x003600, 0x009d00, 0x00c300, 0x007600, 0xffc000, 0xfef600, 0xfe9a00,
0xff0a00, 0x003c00, 0x019f00, 0x025b00, 0x01c500, 0xffe500, 0xfd9900, 0xfc4100,
0xfcf700, 0xffc800, 0x036c00, 0x05c100, 0x04fd00, 0x00dd00, 0xfb3600, 0xf73f00,
0xf7e600, 0xfde600, 0x06c900, 0x0dac00, 0x0d9f00, 0x04a400, 0xf5a900, 0xe83900,
0xe59200, 0xf45600, 0x14c400, 0x3f7a00, 0x67ac00, 0x7fff00, 0x00000, 0x000000,
0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x000000,
0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x000000,
0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x000000,
0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x000000,
0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x000000,
0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x000000,
0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x000000,
0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x000000,
0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x000000,
0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x000000,
0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x000000,
0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x000000,
0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x000000,
0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x000000,
0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x000000,
0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x000000,
0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x000000,
0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x0c0000
};

// Fsout =240.00 kHz, FsIn = 48.00 kHz, bw = 20.00 kHz, target att. = -100.00 dB
// 3-23 NS coefs bw=30k Hinf=16 order=5 QLEV=4, 24 ch0 intp gain, 25-84 intp coefs,217 ch1 intp gain
u32 sg_yram_x05_20_ns40[DACHW_PWM_YRAM_SIZE] ={
0x3eb60, 0x2b9a0, 0x249f0, 0x0d900, 0xfde90, 0x00000, 0x00000, 0x0d1a00,
0xf2e60, 0x10000, 0x10000, 0x10000, 0x10000, 0x00000, 0x00000, 0x100000,
0x10000, 0x10000, 0xfee30, 0xfce30, 0x00000, 0x0dc20, 0xfffe0, 0xffff00,
0x00030, 0x000c0, 0x00180, 0x00220, 0x00240, 0x00170, 0xfffb0, 0xffd500,
0xffb20, 0xffa20, 0xffb50, 0xffee0, 0x00420, 0x00940, 0x00c00, 0x00a900,
0x00450, 0xffa90, 0xff080, 0xfea60, 0xfebb0, 0xff5a0, 0x00630, 0x017f00,
0x023c0, 0x02380, 0x014b0, 0xffa60, 0xfdd10, 0xfc810, 0xfc5b0, 0xfda900,
0x00290, 0x03120, 0x05480, 0x05c40, 0x04010, 0x004b0, 0xfbc60, 0xf81e00,
0xf6f70, 0xf9450, 0xfec40, 0x05df0, 0x0c0c0, 0x0e9b0, 0x0bb50, 0x034200,
0xf74a0, 0xebb20, 0xe5420, 0xe83f0, 0xf6f60, 0x10c40, 0x31dc0, 0x540800,
0x70250, 0x7fff0, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x000000,
0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x000000,
0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x000000,
0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x000000,
0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x000000,
0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x000000,
0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x000000,
0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x000000,
0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x000000,
0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x000000,
0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x000000,
0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x000000,
0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x000000,
0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x000000,
0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x000000,
0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x000000,
0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x0dc20
};

// Fsout =240.00 kHz, FsIn = 48.00 kHz, bw = 20.00 kHz, target att. = -100.00 dB
// 3-23 NS coefs bw=30k Hinf=16 order=5 QLEV=4, 24 ch0 intp gain, 25-84 intp coefs,217 ch1 intp gain
u32 sg_yram_x05_20_ns40_adjGain[DACHW_PWM_YRAM_SIZE] ={
0x3eb60, 0x2b9a0, 0x249f0, 0x0d900, 0xfde90, 0x00000, 0x00000, 0x0d1a00,
0xf2e60, 0x10000, 0x10000, 0x10000, 0x10000, 0x00000, 0x00000, 0x100000,
0x10000, 0x10000, 0xfee30, 0xfce30, 0x00000, 0x0c000, 0xfffe0, 0xffff00,
0x00030, 0x000c0, 0x00180, 0x00220, 0x00240, 0x00170, 0xfffb0, 0xffd500,
0xffb20, 0xffa20, 0xffb50, 0xffee0, 0x00420, 0x00940, 0x00c00, 0x00a900,
0x00450, 0xffa90, 0xff080, 0xfea60, 0xfebb0, 0xff5a0, 0x00630, 0x017f00,
0x023c0, 0x02380, 0x014b0, 0xffa60, 0xfdd10, 0xfc810, 0xfc5b0, 0xfda900,
0x00290, 0x03120, 0x05480, 0x05c40, 0x04010, 0x004b0, 0xfbc60, 0xf81e00,
0xf6f70, 0xf9450, 0xfec40, 0x05df0, 0x0c0c0, 0x0e9b0, 0x0bb50, 0x034200,
0xf74a0, 0xebb20, 0xe5420, 0xe83f0, 0xf6f60, 0x10c40, 0x31dc0, 0x540800,
0x70250, 0x7fff0, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x000000,
0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x000000,
0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x000000,
0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x000000,
0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x000000,
0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x000000,
0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x000000,
0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x000000,
0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x000000,
0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x000000,
0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x000000,
0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x000000,
0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x000000,
0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x000000,
0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x000000,
0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x000000,
0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x0c000
};

// Fsout =384.00 kHz, FsIn = 48.00 kHz, bw = 20.00 kHz, target att. = -100.00 dB
// 3-23 NS coefs bw=30k Hinf=16 order=5 QLEV=4, 24 ch0 intp gain, 25-120 intp coefs,217 ch1 intp gain
u32 sg_yram_x08_20_ns40_adjGain[DACHW_PWM_YRAM_SIZE] ={
0x3eb600, 0x2b9a00, 0x249f00, 0x0d9000, 0xfde900, 0x000000, 0x000000, 0x0d1a00,
0xf2e600, 0x100000, 0x100000, 0x100000, 0x100000, 0x000000, 0x000000, 0x100000,
0x100000, 0x100000, 0xfee300, 0xfce300, 0x000000, 0x0c0000, 0xfffe00, 0xfffe00,
0xffff00, 0x000200, 0x000600, 0x000c00, 0x001300, 0x001a00, 0x002000, 0x002400,
0x002300, 0x001c00, 0x000f00, 0xfffd00, 0xffe600, 0xffcd00, 0xffb700, 0xffa800,
0xffa300, 0xffad00, 0xffc500, 0xffeb00, 0x001d00, 0x005300, 0x008600, 0x00ad00,
0x00bf00, 0x00b600, 0x008f00, 0x004a00, 0xffef00, 0xff8800, 0xff2400, 0xfed400,
0xfea700, 0xfeaa00, 0xfee300, 0xff5100, 0xffea00, 0x009c00, 0x014d00, 0x01e100,
0x023d00, 0x024c00, 0x020000, 0x015a00, 0x006900, 0xff4b00, 0xfe2600, 0xfd2700,
0xfc7d00, 0xfc4a00, 0xfca700, 0xfd9400, 0xfefd00, 0x00b800, 0x028900, 0x042900,
0x055300, 0x05c900, 0x056600, 0x041f00, 0x020f00, 0xff7200, 0xfc9f00, 0xfa0100,
0xf80600, 0xf71100, 0xf76500, 0xf91b00, 0xfc1a00, 0x001100, 0x048200, 0x08ca00,
0x0c3c00, 0x0e3300, 0x0e2f00, 0x0bee00, 0x077600, 0x012500, 0xf9ad00, 0xf20200,
0xeb4700, 0xe6ac00, 0xe54900, 0xe7f900, 0xef3c00, 0xfb1b00, 0x0b2400, 0x1e6800,
0x338f00, 0x48fa00, 0x5ce600, 0x6d9f00, 0x79ae00, 0x7fff00, 0x000000, 0x000000,
0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000,
0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000,
0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000,
0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000,
0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000,
0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000,
0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000,
0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000,
0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000,
0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000,
0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000,
0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x0c0000, 
};





//=========================================================//
    #define CODESIGHT_DACHW_STATIC_FUNC
//=========================================================//
/**
 * pwm indirect register read
 *
 * @param [in]  ePwmIdx : pwm set index; u4Addr : register address;  pu4Val : pointer to return value
 * @param [out] 
 *
 * @return
 */
static void DacHw_IndReadPwmReg(AUD_PWM_DAC_ID ePwmIdx, u32 u4Addr, u32 *pu4Val)
{
    AUDREG_WRITE(aPwmIndRegDesc[ePwmIdx].u4AddrReg, u4Addr);
    *pu4Val = AUDREG_READ(aPwmIndRegDesc[ePwmIdx].u4DataReg);
}

/**
 * pwm indirect register write
 *
 * @param [in]  ePwmIdx : pwm set index; u4Addr : register address;  pu4Val : writer value
 * @param [out] 
 *
 * @return
 */
static void DacHw_IndWritePwmReg(AUD_PWM_DAC_ID ePwmIdx, u32 u4Addr, u32 u4Val)
{
    AUDREG_WRITE(aPwmIndRegDesc[ePwmIdx].u4AddrReg, u4Addr);
    AUDREG_WRITE(aPwmIndRegDesc[ePwmIdx].u4DataReg, u4Val);

    // Check Write correct...
    if ((u4Addr != 0xf08) && (u4Addr != 0xf09))
    {
        u32 u4ReadVal = 0x12345678;
        DacHw_IndReadPwmReg(ePwmIdx, u4Addr, &u4ReadVal);
        if (u4ReadVal != u4Val)
        {
            DACLOG_ERR(T("Write PWMDAC Register Fail!!!   (0x%x : 0x%x : 0x%x) \r\n"), (u32)u4Addr, (u32)u4Val, (u32)u4ReadVal);
        }
    }
}

/**
 * pwm dac digital risc domain set
 *
 * @param [in]  prThis : hw class;
 * @param [out] 
 *
 * @return
 */
static void DacHw_SetPwmDigRiscDomain(PDAC_HW_CLS prThis)
{
    u32 u4RegAdr, u4PwmSetNum;
    u32 u4Tmp;

    u4RegAdr = (AOUT_FS == prThis->eOutPath) ? REGENV_PWMCFG0_PG1 : REGENV_PWMCFG0_PG4;
    u4PwmSetNum = (AOUT_FS == prThis->eOutPath) ? 3 : 1;

    for (u4Tmp = 0; u4Tmp < u4PwmSetNum; u4Tmp++)
    {
        AUDREG_WRITE(u4RegAdr, 0xfa029000);
        u4RegAdr +=4;
    }

    AUDREG_WRITE(REGENV_PWMIP_MISC1, 0xf8000000);
}

/**
 * pwm dac digital IPG set
 *
 * @param [in]  prThis : hw class;
 * @param [out] 
 *
 * @return
 */
static void DacHw_SetPwmDigIpg(PDAC_HW_CLS prThis, AUD_PWM_DAC_ID ePwmChId, u32 u4YramDataType)
{
    u32 i;
    u32 *pu4YramData;
    u32 u4YramDataNum;

    DacHw_IndWritePwmReg(ePwmChId, AUD_REG_PWMIP_P0PIN, 0x607);
    DacHw_IndWritePwmReg(ePwmChId, AUD_REG_PWMIP_P1PIN, 0x607);
    DacHw_IndWritePwmReg(ePwmChId, AUD_REG_PWMIP_P2PIN, 0x600);
    DacHw_IndWritePwmReg(ePwmChId, AUD_REG_PWMIP_P3PIN, 0x600);
    DacHw_IndWritePwmReg(ePwmChId, AUD_REG_PWMIP_POE, 0x3);
    DacHw_IndWritePwmReg(ePwmChId, AUD_REG_PWMIP_PGCTRL1, 0x40FF27);
    DacHw_IndWritePwmReg(ePwmChId, AUD_REG_PWMIP_PGCTRL0, 0x540FE);
    DacHw_IndWritePwmReg(ePwmChId, AUD_REG_PWMIP_PIC, 0x0001);

    switch (u4YramDataType)
    {
    case 0:
        pu4YramData = sg_yram_x03_20_ns40;
        u4YramDataNum = DACHW_PWM_YRAM_SIZE;
        break;

    case 1:
        pu4YramData = sg_yram_x03_20_ns40_adjGain;
        u4YramDataNum = DACHW_PWM_YRAM_SIZE;
        break;

    case 2:
        pu4YramData = sg_yram_x04_20_ns40;
        u4YramDataNum = DACHW_PWM_YRAM_SIZE;
        break;

    case 3:
        pu4YramData = sg_yram_x04_20_ns40_adjGain;
        u4YramDataNum = DACHW_PWM_YRAM_SIZE;
        break;

    case 4:
        pu4YramData = sg_yram_x05_20_ns40;
        u4YramDataNum = DACHW_PWM_YRAM_SIZE;
        break;

    case 5:
        pu4YramData = sg_yram_x08_20_ns40_adjGain;
        u4YramDataNum = DACHW_PWM_YRAM_SIZE;
        break;
        
    default:
        pu4YramData = sg_yram_x05_20_ns40_adjGain;
        u4YramDataNum = DACHW_PWM_YRAM_SIZE;
        break;
    }

    //write xram data
    DacHw_IndWritePwmReg(ePwmChId, AUD_REG_PWMIP_PCADDR, 0);
    for (i = 0; i < 73; i++)
    {
        DacHw_IndWritePwmReg(ePwmChId, AUD_REG_PWMIP_PCDATA, 0);
    }
    Sleep(10);

    //write yram data
    DacHw_IndWritePwmReg(ePwmChId, AUD_REG_PWMIP_PCADDR, 0x100);
    for (i = 0; i < u4YramDataNum; i++)
    {
        DacHw_IndWritePwmReg(ePwmChId, AUD_REG_PWMIP_PCDATA, pu4YramData[i]);
    }
    Sleep(10);
}

/**
 * pwm digital part Apll domain select
 *
 * @param [in]  prThis : hw class; channel set index; fg294M : APLL1(270M) / APLL2(294M)
 * @param [out] 
 *
 * @return
 */
static void DacHw_SetPwmDigApllDomain(PDAC_HW_CLS prThis, bool fg294M)
{
    u32 u4ValBitStart, u4ValBitNum;

    u4ValBitStart = (AOUT_FS == prThis->eOutPath) ? BIT_STR_PWM_FRNT_APLL_MUX : BIT_STR_PWM_REAR_APLL_MUX;
    u4ValBitNum = (AOUT_FS == prThis->eOutPath) ? BIT_NUM_PWM_FRNT_APLL_MUX : BIT_NUM_PWM_REAR_APLL_MUX;

    AUDREG_BITS_W(REGENV_PWMTOP_CFG, u4ValBitStart, u4ValBitNum, fg294M);
}

/**
 * pwm dac digital part start
 *
 * @param [in]  prThis : hw class;
 * @param [out] 
 *
 * @return
 */
static void DacHw_SetPwmDigPartStart(PDAC_HW_CLS prThis, PDAC_EXTPARAMS_T prCfg)
{
    AUD_PWM_DAC_ID ePwmChId;
    u32 u4PwmSetNum, u4Tmp;
    bool fg294M;

    //apll select
    fg294M = (CKGEN_APLL2 == prCfg->eApll) ? TRUE : FALSE;
    DacHw_SetPwmDigApllDomain(prThis, fg294M);

    //risc domain cofig
    DacHw_SetPwmDigRiscDomain(prThis);

    //pwmip config
    ePwmChId = (AOUT_FS == prThis->eOutPath) ? PWM_SET0 : PWM_SET3;
    u4PwmSetNum = (AOUT_FS == prThis->eOutPath) ? 3 : 1;
    for (u4Tmp = 0; u4Tmp < u4PwmSetNum; u4Tmp++)
    {
        DacHw_SetPwmDigIpg(prThis, ePwmChId, DACHW_PWM_YRAM_IDX);
        ePwmChId++;
    }

    //pwmip on
    ePwmChId = (AOUT_FS == prThis->eOutPath) ? PWM_SET0 : PWM_SET3;
    DacHw_IndWritePwmReg(ePwmChId, AUD_REG_PWMIP_PGCTRL0, 0x540FF);
}

/**
 * pwm dac digital part stop
 *
 * @param [in]  prThis : hw class;
 * @param [out] 
 *
 * @return
 */
static void DacHw_SetPwmDigPartStop(PDAC_HW_CLS prThis)
{
    AUD_PWM_DAC_ID ePwmChId;

    ePwmChId = (AOUT_FS == prThis->eOutPath) ? PWM_SET0 : PWM_SET3;

    DacHw_IndWritePwmReg(ePwmChId, AUD_REG_PWMIP_PGCTRL0, 0x540FE);
}

/**
 * pwm analog part VCM enable
 *
 * @param [in]  prThis : hw class; fgEn : ON(1)/OFF(0)
 * @param [out] 
 *
 * @return
 */
static void DacHw_SetPwmAnaVcmEn(PDAC_HW_CLS prThis, bool fgEn)
{
    AUDREG_BITS_W(AUD_REG_PWM_CFG9, BIT_STR_ADAC_VCM_EN, BIT_NUM_ADAC_VCM_EN, fgEn);
}

/**
 * pwm analog part high impedance mode enable
 *
 * @param [in]  ePwmChId : channel set index;  fgEn : ON(1)/OFF(0)
 * @param [out] 
 *
 * @return
 */
static void DacHw_SetPwmAnaHizEn(AUD_PWM_DAC_ID ePwmChId, bool fgEn)
{
    AUDREG_BITS_W(AUD_REG_PWM_CFG10, 
                  (BIT_STR_HB_HIZ + ePwmChId * 2), 
                  BIT_NUM_PWM_2CH_CTL, 
                  (fgEn * 3));
}

/**
 * pwm dac analog part channel set enable
 *
 * @param [in]  prThis : hw class; ePwmChId : channel set index;  fgEn : ON(1)/OFF(0)
 * @param [out] 
 *
 * @return
 */
static void DacHw_SetPwmAnaChEn(PDAC_HW_CLS prThis, AUD_PWM_DAC_ID ePwmChId, bool fgEn)
{
    AUDREG_BITS_W(AUD_REG_PWM_CFG2, 
                  (BIT_STR_HB_ENVO_CH0 + ePwmChId * 4), 
                  BIT_NUM_HB_ENVO_CH0, 
                  fgEn);
}

/**
 * pwm dac analog part channel set analog or digital function config
 *
 * @param [in] ePwmChId : channel set index;  fgEn : GPO(1)/Analog(0)
 * @param [out] 
 *
 * @return
 */
void DacHw_SetPwmAnaGpioFun(AUD_PWM_DAC_ID ePwmChId, bool fgEn)
{
    bool fgAnalogFun =  fgEn ? PWM_GPIO_FUNCTION : PWM_ANALOG_FUNCTON;
    bool fgGPO = fgEn ? PWM_GPO_EN : PWM_GPI_EN;

    //HIZ config
    DacHw_SetPwmAnaHizEn(ePwmChId, fgEn);

    //enable analog or digital function
    AUDREG_BITS_W(AUD_REG_PWM_CFG7,
                  (BIT_STR_GPIO_PWM_G + ePwmChId * 2),
                  BIT_NUM_PWM_2CH_CTL,
                  (fgAnalogFun * 3));

    //gpi or gpo config
    AUDREG_BITS_W(AUD_REG_PWM_CFG5, 
                  (BIT_STR_GPIO_PWM_EN + ePwmChId * 2), 
                  BIT_NUM_PWM_2CH_CTL, 
                  (fgGPO * 3));
}

/**
 * pwm analog part Apll domain select
 *
 * @param [in]  prThis : hw class; channel set index; fg294M : APLL1(270M) / APLL2(294M)
 * @param [out] 
 *
 * @return
 */
static void DacHw_SetPwmAnaApllDomain(PDAC_HW_CLS prThis, AUD_PWM_DAC_ID ePwmChId, bool fg294M)
{
    bool fgPwmClkDomain;

    fgPwmClkDomain = fg294M ? PWM_294M : PWM_270M;

    AUDREG_BITS_W(AUD_REG_PWM_CFG0, 
                  (BIT_STR_HB_CLK_294M_SEL + ePwmChId * 2), 
                  BIT_NUM_PWM_2CH_CTL, 
                  (fgPwmClkDomain * 3));

    AUDREG_BITS_W(AUD_REG_PWM_CFG1, 
                  (BIT_STR_HB_PS_CLK_294M_SEL + ePwmChId * 2), 
                  BIT_NUM_PWM_2CH_CTL, 
                  (fgPwmClkDomain * 3));
}

/**
 * pwm analog part default setting, for each pwm set
 *
 * @param [in]  prThis : hw class;
 * @param [out] 
 *
 * @return
 */
static void DacHw_SetPwmAnaDefaultConfig(PDAC_HW_CLS prThis)
{
    AUDREG_BITS_W(AUD_REG_PWM_CFG3, BIT_STR_HB_ENPWRDET, BIT_NUM_HB_ENPWRDET, 0);
    AUDREG_BITS_W(AUD_REG_PWM_CFG5, BIT_STR_HB_REV0, BIT_NUM_HB_REV0, 0);
    AUDREG_BITS_W(AUD_REG_PWM_CFG8, BIT_STR_AUD_PWMDAC_REV0, BIT_NUM_AUD_PWMDAC_REV0, 0);
    AUDREG_BITS_W(AUD_REG_PWM_CFG8, BIT_STR_AUD_PWMDAC_REV1, BIT_NUM_AUD_PWMDAC_REV1, 0);
    AUDREG_BITS_W(AUD_REG_PWM_CFG2, BIT_STR_HB_DFC_CH, BIT_NUM_HB_DFC_CH, 0);
}

/**
 * pwm analog part set basic power on
 *
 * @param [in]  prThis : hw class;
 * @param [out] 
 *
 * @return
 */
void DacHw_SetPwmAnalogPartBasicPowerOn(void)
{
    DAC_HW_CLS rDacObj;
    PDAC_HW_CLS prThis;
    AUD_PWM_DAC_ID ePwmSetStart;
    
    prThis = &rDacObj;  //no useful, so no need init

    //step1: default setting
    DacHw_SetPwmAnaDefaultConfig(prThis);

    //step2: apll select & gpio config
    for (ePwmSetStart = PWM_SET0; ePwmSetStart < PWM_SET_MAX; ePwmSetStart++)
    {
        DacHw_SetPwmAnaApllDomain(prThis, ePwmSetStart, TRUE);  //default 294M
        DacHw_SetPwmAnaGpioFun(ePwmSetStart, FALSE);  //default analog function
    }

    //step3: VCM Enable  ->Be care need about 1 second to finish this step
    DacHw_SetPwmAnaVcmEn(prThis, TRUE);
}

/**
 * pwm analog part set start
 *
 * @param [in]  prThis : hw class; prCfg : external config
 * @param [out] 
 *
 * @return
 */
static void DacHw_SetPwmAnalogPartStart(PDAC_HW_CLS prThis, PDAC_EXTPARAMS_T prCfg)
{
    u32 u4Tmp;
    u32 u4PwmSetNum;
    AUD_PWM_DAC_ID ePwmSetStart;
    bool fg294M;

    u4PwmSetNum = (AOUT_FS == prThis->eOutPath) ? 3 : 1;
    ePwmSetStart = (AOUT_FS == prThis->eOutPath) ? PWM_SET0 : PWM_SET3;
    fg294M = (CKGEN_APLL2 == prCfg->eApll) ? TRUE : FALSE;

    //apll select & gpio config
    for (u4Tmp = 0; u4Tmp < u4PwmSetNum; u4Tmp++)
    {
        DacHw_SetPwmAnaApllDomain(prThis, ePwmSetStart, fg294M);
        DacHw_SetPwmAnaGpioFun(ePwmSetStart, FALSE);
        
        ePwmSetStart++;
    }

    //ENVO Enable
    ePwmSetStart = (AOUT_FS == prThis->eOutPath) ? PWM_SET0 : PWM_SET3;
    for (u4Tmp = 0; u4Tmp < u4PwmSetNum; u4Tmp++)
    {
        DacHw_SetPwmAnaChEn(prThis, ePwmSetStart, TRUE);

        ePwmSetStart++;
    }
}

/**
 * pwm analog part set stop
 *
 * @param [in]  prThis : hw class;
 * @param [out] 
 *
 * @return
 */
static void DacHw_SetPwmAnalogPartStop(PDAC_HW_CLS prThis)
{
    u32 u4Tmp;
    u32 u4PwmSetNum;
    AUD_PWM_DAC_ID ePwmSetStart;

    u4PwmSetNum = (AOUT_FS == prThis->eOutPath) ? 3 : 1;
    ePwmSetStart = (AOUT_FS == prThis->eOutPath) ? PWM_SET0 : PWM_SET3;

    for (u4Tmp = 0; u4Tmp < u4PwmSetNum; u4Tmp++)
    {
        DacHw_SetPwmAnaChEn(prThis, ePwmSetStart, FALSE);

        ePwmSetStart++;
    }
}


//=========================================================//
    #define CODESIGHT_DACHW_PUBLIC_FUNC
//=========================================================//

/**
 * public interface : pwm dac start
 *
 * @param [in]  prThis : hw class; prCfg : adc hw mode setting
 * @param [out] 
 *
 * @return
 */
static void DacHw_PwmStart(void * pThis, void * pCfg)
{
    PDAC_HW_CLS prThis = (PDAC_HW_CLS)pThis;
    PDAC_EXTPARAMS_T prCfg = (PDAC_EXTPARAMS_T)pCfg;
    
    DACLOG_INFO(T(" DacHw_PwmStart \r\n"));

    //config anlaog part
    DacHw_SetPwmAnalogPartStart(prThis, prCfg);

    //config digitable part
    DacHw_SetPwmDigPartStart(prThis, prCfg);
}
    
/**
 * public interface : pwm dac stop
 *
 * @param [in]  prThis : hw class;
 * @param [out] 
 *
 * @return
 */
static void DacHw_PwmStop(void * pThis)
{
    PDAC_HW_CLS prThis = (PDAC_HW_CLS)pThis;
    
    DACLOG_INFO(T(" DacHw_PwmStop \r\n"));

    //config digitable part
    DacHw_SetPwmDigPartStop(prThis);

    //config anlaog part
    DacHw_SetPwmAnalogPartStop(prThis);
}

/**
 * public interface : ext dac start
 *
 * @param [in]  prThis : hw class; prCfg : adc hw mode setting
 * @param [out] 
 *
 * @return
 */
static void DacHw_ExtStart(void * pThis, void * pCfg)
{
    DACLOG_INFO(T(" DacHw_ExtStart \r\n"));
}
    
/**
 * public interface : ext dac stop
 *
 * @param [in]  prThis : hw class;
 * @param [out] 
 *
 * @return
 */
static void DacHw_ExtStop(void * pThis)
{
    DACLOG_INFO(T(" DacHw_ExtStop \r\n"));
}


//===========================================//
    #define CODESIGHT_DACHW_CREATE
//===========================================//

/**
 * delect a dac hw object
 *
 * @param [in]  prThis : pointer to the dac hw object
 * @param [out] 
 *
 * @return      0: OK; others: NG
 */
static u32 DacHw_Delete(void * pThis)
{
    PDAC_HW_CLS prThis = (PDAC_HW_CLS)pThis;
    
    AUD_CLASS_DELETE();
    
    return (0);
}

/**
 * create a new adc hw object
 *
 * @param [in] 
 * @param [out] 
 *
 * @return  pointer to new object
 */
PDAC_HW_CLS_PUB DacHw_New(AUD_OUT_PATH_T eOutPath, AUD_DAC_TYPE_T eDacType)
{
    PDAC_HW_CLS prThis = AUD_CLASS_NEW(DAC_HW_CLS);

    if (prThis)
    {
        prThis->eDacType = eDacType;
        prThis->eOutPath = eOutPath;
            
        prThis->rPub.Delete = DacHw_Delete;

        if (AUD_DAC_PWM == eDacType)
        {
            prThis->rPub.Start = DacHw_PwmStart;
            prThis->rPub.Stop = DacHw_PwmStop;
        }
        else
        {
            prThis->rPub.Start = DacHw_ExtStart;
            prThis->rPub.Stop = DacHw_ExtStop;
        }
    }

    return ((PDAC_HW_CLS_PUB)prThis);
}


