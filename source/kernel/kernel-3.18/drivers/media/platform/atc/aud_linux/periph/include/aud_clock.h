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




#include "x_common.h"
//#include "x_typedef.h"
#include <linux/types.h>
#include <media/atc/drv_av_d.h>
#include "aud_3360_reg_rw.h"
#include "aud_pwmdac.h"
#include "aud_extdac.h"
#include "util.h"

#ifndef __AUD_CLOCK_H_
#define __AUD_CLOCK_H_

/*
Default Value of 0x2C0 for 270M
NCPO_PCW : 0x686C226 -> 109494822
Default Value of 0x2AC for 294M
NCPO_PCW : 0x73126E9 -> 120661737

    Fref:72M
    PREDIV:0
    NCPO_PCW:default value of 0x2C0 OR 0x2AC
    DIV2_EN:1
    POSDIV:2
Ckout     = Fref     / 2^PREDIV * (1 + NCPO_PCW  / 2^24)    * 2^DIV2_EN / 2^POSDIV
270950400 = 72000000 / 1   * (1 + 109494822.0 / 16777216.0) * 2         / 4
270950400 = 72000000 / 1   * (1 + 6.52639997)               * 2         / 4
270950400 = 72000000 / 1   * (7.52639997)                   * 2         / 4
270950400 = 72000000       * 7.52639997                     * 2         / 4
270950400 = 36000000       * 7.52639997
270950400 = 270950398.92

==> Ckout = 72000000 * (1 + x / 16777216.0) / 2
==> x = (Ckout / 36000000.0 - 1) * 16777216.0 

Ckout + 0.001 <--> x + 466
Ckout + 0.002 <--> x + 932

*/
#define APLL_ADJ_STEP                   1//466
#define APLL_PCW_MASK                   0x7FFFFFFF
#define APLL_PHASELATCH_MASK            0xFFFF
#define APLL_PHASELATCH_RANGE           200

#define PLL_SETTING_MASK                (0xFFFF)
#define APLL_CALIBRATION_TIMEOUT        0x10000

#define DIV_MCLK_256FS_8K               144 
#define DIV_MCLK_256FS_16K              72 
#define DIV_MCLK_256FS_22K              48
#define DIV_MCLK_256FS_24K              48 
#define DIV_MCLK_256FS_32K              36
#define DIV_MCLK_256FS_44K              24
#define DIV_MCLK_256FS_48K              24 
#define DIV_MCLK_256FS_64K              18
#define DIV_MCLK_256FS_88K              12 
#define DIV_MCLK_256FS_96K              12
#define DIV_MCLK_256FS_176K             6
#define DIV_MCLK_256FS_192K             6 

/*
#define PLL_FBSEL(v)    ((v & 0x3)<<14)
#define PLL_CKCTRL(v)   ((v & 0x3)<<12)
#define PLL_POSDIV(v)   ((v & 0x3)<<10)
#define PLL_PREDIV(v)   ((v & 0x3)<<8)
#define PLL_FBDIV(v)    ((v & 0x7F)<<1)
#define PLL_PWD         (1)
#define APLL_288MHz                     \
    (PLL_FBSEL(1) | PLL_CKCTRL(0) | PLL_POSDIV(2) | PLL_PREDIV(0) | PLL_FBDIV(7) | PLL_PWD)
*/


typedef enum {
    BCLK_CYCLE_16,
    BCLK_CYCLE_24,
    BCLK_CYCLE_32,
    BCLK_CYCLE_UNDEF
} BCLK_CYCLE_E;

typedef enum {
    AUD_FMT_RIGHT_JUSTIFIED,
    AUD_FMT_LEFT_JUSTIFIED,
    AUD_FMT_RESERVD,
    AUD_FMT_IIS,
    AUD_FMT_UNDEF_INTF
} AUD_FMT_INTF_E;

typedef enum{
    AUD_MCLK_IS_8K_256FS = 64,
    AUD_MCLK_IS_128_FS = 128,
    AUD_MCLK_IS_256_FS = 256,
    AUD_MCLK_IS_384_FS = 384,
    AUD_MCLK_IS_512_FS = 512,
    AUD_MCLK_IS_768_FS = 768,
    AUD_MCLK_UNDEF
} AUD_MCLK_SOURCE_TYPE_E;

typedef enum{
    AUD_AFE_CLK_FROM_AOUT1 ,
    AUD_AFE_CLK_FROM_AOUT2 ,
    AUD_AFE_CLK_FROM_MLINE ,
    AUD_AFE_CLK_FROM_MPHONE,
} AUD_AFE_CLOCK_SEL_E;

typedef enum {
    LRCLK_8K_HZ,
    LRCLK_32K_HZ,
    LRCLK_44K1_HZ,
    LRCLK_48K_HZ,
    LRCLK_88K2_HZ,
    LRCLK_96K_HZ,
    LRCLK_192K_HZ,
    LRCLK_UNDEF
} LRCLK_FREQ_E;

typedef enum {
    AUDIN_CLK_FROM_AOUT1,
    AUDIN_CLK_FROM_AOUT2,
    AUDIN_CLK_FROM_MLINE,
    AUDIN_CLK_FROM_MPHONE,
    AUDIN_LINEIN_CLK_FROM_INTERNAL_26M,
    AUDIN_LINEIN_CLK_FROM_INTERNAL_27M,
    AUDIN_MICIN_CLK_FROM_INTERNAL_26M,
    AUDIN_MICIN_CLK_FROM_INTERNAL_27M,
    AUDIN_DMICIN_CLK_FROM_INTERNAL_26M,
    AUDIN_CLK_UNDEF
} AUDIN_CLK_SOURCE_SEL_E;

typedef enum{
    AUD_LI_DT_16_BIT_MODE,
    AUD_LI_DT_24_BIT_MODE,
    AUD_LI_DT_UND_BIT_MODE
} AUD_LI_BIT_MODE_E;

typedef enum{
    AUD_APLL_ADJ_NORMAL,
    AUD_APLL_ADJ_UP,
    AUD_APLL_ADJ_DOWN
} AUD_APLL_ADJ_MODE_E;



#define AUD_AFE1_SRC_CLK_SEL_START               (2)
#define AUD_AFE1_SRC_CLK_SEL_NUM               (2)
#define AUD_AFE2_SRC_CLK_SEL_START               (4)
#define AUD_AFE2_SRC_CLK_SEL_NUM               (2)
#define AUD_AFE1_DST_CLK_SEL_START               (16)
#define AUD_AFE1_DST_CLK_SEL_NUM               (2)
#define AUD_AFE2_DST_CLK_SEL_START               (18)
#define AUD_AFE2_DST_CLK_SEL_NUM               (2)
#define AUD_AFE_CLK_AOUT1                       (0)
#define AUD_AFE_CLK_AOUT2                       (1)
#define AUD_AFE_CLK_MLINE                       (2)
#define AUD_AFE_CLK_MPHONE                       (3)

u32 Aud_APLLPowerOn(APLL_DOMAIN eDomain);
u32 Aud_APLLPowerDown(void);

void Aud_ApllDirectAdjust(AUD_APLL_ADJ_MODE_E eAdjMode, u32 u4StepNum);
extern void vDspSetClock(void);


#endif // __AUD_CLOCK_H_

