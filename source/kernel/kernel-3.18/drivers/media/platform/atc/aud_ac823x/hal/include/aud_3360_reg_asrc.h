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
#include <linux/types.h>
#ifndef _AUDIO_3360_REG_ASRC_H_
#define _AUDIO_3360_REG_ASRC_H_


/////////////////////////////////////////////////////////////////////////////
// There are two ASRC Hardware, one is the ASRC inside DSP, the other is work for GPS
/////////////////////////////////////////////////////////////////////////////
extern u32 g_u4ASRC_Reg_Base;


/////////////////////////////////////////////////////////////////////////////
//         GPS ASRC Register Accessed by RISC
/////////////////////////////////////////////////////////////////////////////
#define AUD_REG_GR_ASRC_BASE                  (0xA8600)

// bit8: ASRC_EN, ASRC Enable, The central enable signal, should be turned on after all configuration are set.
// bit12~15: CH_EN, Each Ch-set enable signal, This register controls which Ch-set should be execute.(0~3)
// bit16~19: CH_CLEAR, Each Ch-set clear signal, set to 1 means the Ch-set will clear history at next run.
// bit20~23: CH_CNTX_SWEN, Context switch disabler. Disable context switch of each Ch-set, remember to 
//           stop each Ch-set first by set related enable before using this registers.
#define AUD_REG_GR_ASRC_GEN_CFG               (g_u4ASRC_Reg_Base + 0x00)
#define AUD_REG_ASRC_EN(val)                  (((val) & 0x1) << 8)
#define AUD_REG_ASRC_CH_EN(val)               (((val) & 0xF) << 12)
#define AUD_REG_ASRC_CH_CLEAR(val)            (((val) & 0xF) << 16)
#define AUD_REG_ASRC_CH_CNTX_SWEN(val)        (((val) & 0xF) << 20)

// bit8~11: OBUF_AMOUNT_INTEN, output buffer amount interrupt enable Ch-set(0~3)
// bit12~15: OBUF_OV_INTEN, output buffer overflow interrupt enable Ch-set(0~3)
// bit16~19: IBUF_AMOUNT_INTEN, input buffer amount interrupt enable Ch-set(0~3)
// bit20~23: IBUF_EMPTY_INTEN, input buffer empty interrupt enable Ch-set(0~3)
#define AUD_REG_GR_ASRC_IER                   (g_u4ASRC_Reg_Base + 0xCC)
#define AUD_REG_ASRC_OBUF_CNT_INTEN(val)      (((val) & 0xF) << 8)
#define AUD_REG_ASRC_OBUF_OV_INTEN(val)       (((val) & 0xF) << 12)
#define AUD_REG_ASRC_IBUF_CNT_INTEN(val)      (((val) & 0xF) << 16)
#define AUD_REG_ASRC_IBUF_EMPY_INTEN(val)     (((val) & 0xF) << 20)

// bit8~11: OBUF_AMOUNT_INT, Output Amount Reached Flag for 0~3, Each bit related
//          to one channel pair, 1 means the related channel pair output amount meets requirement.
//          Write the related bit will clear it.
// bit12~15: OBUF_OV_INT, Output buffer full flag for 0~3.Each bit related
//           to one channel pair, 1 means the related output buffer is full.
//           Write buffer full will make the whole system hang, please resolve it 
//           as soon as possible. Write the related bit will clear it.
// bit16~19: IBUF_AMOUNT_INT, Input left amount reached flag for 0~3, Each bit related
//           to one channel pair, 1 means the related input buffer left amount reached
//           the dedicated value. Write the related bit will clear it.
// bit20~23: IBUT_EMPTY_INT, Input buffer empty flag for 0~3, Each bit related to one
//           channel pair, 1 means the related input buffer is empty.Write the related bit will clear it.
#define AUD_REG_GR_ASRC_IFR                   (g_u4ASRC_Reg_Base + 0x08)

// bit8~15: CALC_AMOUNT, Calculation amount.Define how many 128-bit output the related channel pair should calculate at each turn.
// bit16~17: IFS, Input sample rate selection. set 0,1,2 to choose the related frequency on palette.
// bit18~19: OFS, Output sample rate selection. set 0,1,2 to choose the related frequency on palette.
// bit20: MONO, Mono/Stereo selection register, 0 is stereo, 1 is mono.
// bit21: IBIT_WIDTH, Bit-width selection for input, 0 is 24-bit, 1 is 16-bit.
// bit22: OBIT_WIDTH, Bit-width selection for output, 0 is 24-bit, 1 is 16-bit.
#define AUD_REG_GR_ASRC_CH0_CFG               (g_u4ASRC_Reg_Base + 0x10)
#define AUD_REG_GR_ASRC_CH1_CFG               (g_u4ASRC_Reg_Base + 0x14)
#define AUD_REG_GR_ASRC_CH2_CFG               (g_u4ASRC_Reg_Base + 0x18)
#define AUD_REG_GR_ASRC_CH3_CFG               (g_u4ASRC_Reg_Base + 0x1C)
#define AUD_REG_ASRC_CALC_NUM(val)            (((val) & 0xFF) << 8)
#define AUD_REG_ASRC_IFS(val)                 (((val) & 0x3) << 16)
#define AUD_REG_ASRC_OFS(val)                 (((val) & 0x3) << 18)
#define AUD_REG_ASRC_MONO(val)                (((val) & 0x1) << 20)
#define AUD_REG_ASRC_IBW(val)                 (((val) & 0x1) << 21)
#define AUD_REG_ASRC_OBW(val)                 (((val) & 0x1) << 22)

// bit0~23: Frequency Palette, The frequency 'palette' for each channel set to
//          define its input frequency & output frequency
#define AUD_REG_GR_ASRC_FREQ_0                (g_u4ASRC_Reg_Base + 0x20)
#define AUD_REG_GR_ASRC_FREQ_1                (g_u4ASRC_Reg_Base + 0x24)
#define AUD_REG_GR_ASRC_FREQ_2                (g_u4ASRC_Reg_Base + 0x28)
#define AUD_REG_GR_ASRC_FREQ_3                (g_u4ASRC_Reg_Base + 0x2C)

// bit4~23:Input buffer start address, The start address of input buffer, in 128-bit unit.
#define AUD_REG_GR_ASRC_IBUF_SADR             (g_u4ASRC_Reg_Base + 0x30)

// bit4~19:Input channel size, the input buffer size for each channel, in 128-bit unit.
// Each channel uses the same size and is circular.
#define AUD_REG_GR_ASRC_IBUF_SIZE             (g_u4ASRC_Reg_Base + 0x34)

// bit4~23:Output buffer start address, The start address of output buffer, in 128-bit unit.
#define AUD_REG_GR_ASRC_OBUF_SADR             (g_u4ASRC_Reg_Base + 0x38)

// bit4~19:Output channel size, the output buffer size for each channel, in 128-bit unit.
// Each channel uses the same size and is circular.
#define AUD_REG_GR_ASRC_OBUF_SIZE             (g_u4ASRC_Reg_Base + 0x3C)

// bit4~23:Input buffer read address register. Current input buffer read address
//         for each channel pair. For stereo channel, this value is related to first channel.
#define AUD_REG_GR_ASRC_CH0_IBUF_RP           (g_u4ASRC_Reg_Base + 0x40)
#define AUD_REG_GR_ASRC_CH1_IBUF_RP           (g_u4ASRC_Reg_Base + 0x44)
#define AUD_REG_GR_ASRC_CH2_IBUF_RP           (g_u4ASRC_Reg_Base + 0x48)
#define AUD_REG_GR_ASRC_CH3_IBUF_RP           (g_u4ASRC_Reg_Base + 0x4C)

// bit4~23:Input buffer write address register. Current input buffer write address
//         for each channel pair. For stereo channel, this value is related to first channel.
#define AUD_REG_GR_ASRC_CH0_IBUF_WP           (g_u4ASRC_Reg_Base + 0x50)
#define AUD_REG_GR_ASRC_CH1_IBUF_WP           (g_u4ASRC_Reg_Base + 0x54)
#define AUD_REG_GR_ASRC_CH2_IBUF_WP           (g_u4ASRC_Reg_Base + 0x58)
#define AUD_REG_GR_ASRC_CH3_IBUF_WP           (g_u4ASRC_Reg_Base + 0x5C)

// bit4~23:Output buffer write address register. Current output buffer write address
//         for each channel pair. For stereo channel, this value is related to first channel.
#define AUD_REG_GR_ASRC_CH0_OBUF_WP           (g_u4ASRC_Reg_Base + 0x60)
#define AUD_REG_GR_ASRC_CH1_OBUF_WP           (g_u4ASRC_Reg_Base + 0x6A)
#define AUD_REG_GR_ASRC_CH2_OBUF_WP           (g_u4ASRC_Reg_Base + 0x6B)
#define AUD_REG_GR_ASRC_CH3_OBUF_WP           (g_u4ASRC_Reg_Base + 0x6C)

// bit4~23:Output buffer read address register. Current output buffer read address
//         for each channel pair. For stereo channel, this value is related to first channel.
#define AUD_REG_GR_ASRC_CH0_OBUF_RP           (g_u4ASRC_Reg_Base + 0x70)
#define AUD_REG_GR_ASRC_CH1_OBUF_RP           (g_u4ASRC_Reg_Base + 0x74)
#define AUD_REG_GR_ASRC_CH2_OBUF_RP           (g_u4ASRC_Reg_Base + 0x78)
#define AUD_REG_GR_ASRC_CH3_OBUF_RP           (g_u4ASRC_Reg_Base + 0x7C)

// bit16~23:Channel pair1 input buffer amount interrupt register. When the related
//          input buffer left less than this amount(in 384-bit unit) of input data.
//          It will rise the input buffer amount flag.
// bit8~15:Channel pair0 input buffer amount interrupt register. When the related
//         input buffer left less than this amount(in 384-bit unit) of input data.
//         It will rise the input buffer amount flag.
#define AUD_REG_GR_ASRC_CH01_IBUF_INTR        (g_u4ASRC_Reg_Base + 0x80)

// bit16~23:Channel pair3 input buffer amount interrupt register. When the related
//          input buffer left less than this amount(in 384-bit unit) of input data.
//          It will rise the input buffer amount flag.
// bit8~15:Channel pair2 input buffer amount interrupt register. When the related
//          input buffer left less than this amount(in 384-bit unit) of input data.
//          It will rise the input buffer amount flag.
#define AUD_REG_GR_ASRC_CH23_IBUF_INTR        (g_u4ASRC_Reg_Base + 0x84)

// bit16~23:Channel pair1 output buffer amount interrupt register. When the related
//          output buffer contain more than this amount(in 384-bit unit) of output data.
//          It will rise the output buffer amount flag.
// bit8~15:Channel pair0 output buffer amount interrupt register. When the related
//          output buffer contain more than this amount(in 384-bit unit) of output data.
//          It will rise the output buffer amount flag.
#define AUD_REG_GR_ASRC_CH01_OBUF_INTR        (g_u4ASRC_Reg_Base + 0x88)

// bit16~23:Channel pair3 output buffer amount interrupt register. When the related
//          output buffer contain more than this amount(in 384-bit unit) of output data.
//          It will rise the output buffer amount flag.
// bit8~15:Channel pair2 output buffer amount interrupt register. When the related
//          output buffer contain more than this amount(in 384-bit unit) of output data.
//          It will rise the output buffer amount flag.
#define AUD_REG_GR_ASRC_CH23_OBUF_INTR        (g_u4ASRC_Reg_Base + 0x8C)

// bit0~2:Output selection
// 000: ASRC output, 001: Data input, 010: CMPF output, 011: HBF1 output, 
// 100: HBF2 output, 101: HBF3 output, 110: HBF4 output, 111: ASRC output
#define AUD_REG_GR_ASRC_BAK                   (g_u4ASRC_Reg_Base + 0x90)

// bit8: Set 1 to enable frequency clibrator, if auto restart is 0, this bit 
//       will be clear while one clibration run is completed.
// bit9: The calibrator input selection register
//       0: Use aout_lrck or aout2_lrck
//       1: Use mphone_lrck or mphone_multi_lrck.
// bit10: Auto restart. set 1 make the calibrator auto restart new calibration.
// bit11: Set 1 to enable ASRC_FREQUENCY_2 auto update with the calibrator result once the calibrator complete one round.
// bit12~14: The left shift amount for calibrator result auto load to ASRC_FREQUENCY_2.
// bit15: 0 -- Use period calibration result to update FS2.
//        1 -- Use frequency calibration result to update FS2
// bit16: This bit shows if the frequency calculation is runing. For one round running case,
//        user should wait this bit and CALI_EN bit become low and then the frequency result will be ready.
#define AUD_REG_GR_ASRC_FREQ_CTRL             (g_u4ASRC_Reg_Base + 0x94)

// bit8~19: ASRC Frequency calibrator input cycle register define how many input signal
//          cycle the calibrator calibrates in one round.
#define AUD_REG_GR_ASRC_FREQ_CYC              (g_u4ASRC_Reg_Base + 0x98)

// bit0~23: ASRC period calibrator result record the calibration result of previous round. 
//          Write any value to this register will clear the result.
#define AUD_REG_GR_ASRC_PRD_RSLT              (g_u4ASRC_Reg_Base + 0x9C)

// bit0~23: ASRC frequency calibrator result. This values is translated by
//          (FREQ_TRANS_NUMERATOR,4'b0) / ASRC_PRD_CALI_RSLT.
//          If the result more then 1, the result will be 24'h FFFFFF.
#define AUD_REG_GR_ASRC_FREQ_RSLT             (g_u4ASRC_Reg_Base + 0xA0)

// bit8~11: Output buffer amount interrupt enable for DSPB for Ch-set0~3
// bit12~15: Output buffer overflow interrupt enable for DSPB for Ch-set0~3
// bit 16~19: Input buffer amount interrupt enable for DSPB for Ch-set0~3
// bit20~23: Input buffer empty interrupt enable for DSPB for Ch-set0~3
#define AUD_REG_GR_ASRC_DSP2_IER              (g_u4ASRC_Reg_Base + 0xC8)

// bit8~11: Output buffer amount interrupt enable for DSPC for Ch-set0~3
// bit12~15: Output buffer overflow interrupt enable for DSPC for Ch-set0~3
// bit16~19: Input buffer amount interrupt enable for DSPC for Ch-set0~3
// bit20~23: Input buffer empty interrupt enable for DSPC for Ch-set0~3
#define AUD_REG_GR_ASRC_DSP3_IER              (g_u4ASRC_Reg_Base + 0xCC)

// bit8~11: Maximum output amount per input for channel set 0
// bit12~15: Maximum output amount per input for channel set 1
// bit16~19: Maximum output amount per input for channel set 2
// bit20~23: Maximum output amount per input for channel set 3
#define AUD_REG_GR_MAX_OUTPUT_PER_IN          (g_u4ASRC_Reg_Base + 0xE0)

// bit0~23: Define the numerator for frequency calibrator to get frequency result 
//          from period result. The frequency result will be the 24-bit fractional part of 
//          FREQ_TRANS_NUMERATOR / ASRC_PRD_CALI_RSLT
#define AUD_REG_GR_ASRC_FREQ_TRANS_NUM        (g_u4ASRC_Reg_Base + 0xD8)

// bit0~1: ASRC DMA buffer size
//         00 -- 2X128bits, 01 -- 4X128bits
//         10 -- 8X128bits, 11 -- 16X128bits
// bit4: DMA buffer empty flag
// bit5: DMA buffer full flag
// bit8: LAST_CFG?
#define AUD_REG_GR_ASRC_DMA_CFG               (g_u4ASRC_Reg_Base + 0xFC)


/////////////////////////////////////////////////////////////////////////////
//         DSP ASRC Register Accessed by RISC
/////////////////////////////////////////////////////////////////////////////
#define AUD_REG_DR_ASRC_BASE                   (0xA8700)

#define AUD_REG_DR_ASRC_GEN_CFG                (g_u4ASRC_Reg_Base + 0x00)
#define AUD_REG_DR_ASRC_IER                    (g_u4ASRC_Reg_Base + 0x04)
#define AUD_REG_DR_ASRC_IFR                    (g_u4ASRC_Reg_Base + 0x08)
#define AUD_REG_DR_ASRC_CH0_CFG                (g_u4ASRC_Reg_Base + 0x10)
#define AUD_REG_DR_ASRC_CH1_CFG                (g_u4ASRC_Reg_Base + 0x14)
#define AUD_REG_DR_ASRC_CH2_CFG                (g_u4ASRC_Reg_Base + 0x18)
#define AUD_REG_DR_ASRC_CH3_CFG                (g_u4ASRC_Reg_Base + 0x1C)
#define AUD_REG_DR_ASRC_FREQ_0                 (g_u4ASRC_Reg_Base + 0x20)
#define AUD_REG_DR_ASRC_FREQ_1                 (g_u4ASRC_Reg_Base + 0x24)
#define AUD_REG_DR_ASRC_FREQ_2                 (g_u4ASRC_Reg_Base + 0x28)
#define AUD_REG_DR_ASRC_FREQ_3                 (g_u4ASRC_Reg_Base + 0x2C)
#define AUD_REG_DR_ASRC_IBUF_SADR              (g_u4ASRC_Reg_Base + 0x30)
#define AUD_REG_DR_ASRC_IBUF_SIZE              (g_u4ASRC_Reg_Base + 0x34)
#define AUD_REG_DR_ASRC_OBUF_SADR              (g_u4ASRC_Reg_Base + 0x38)
#define AUD_REG_DR_ASRC_OBUF_SIZE              (g_u4ASRC_Reg_Base + 0x3C)
#define AUD_REG_DR_ASRC_CH0_IBUF_RP            (g_u4ASRC_Reg_Base + 0x40)
#define AUD_REG_DR_ASRC_CH1_IBUF_RP            (g_u4ASRC_Reg_Base + 0x44)
#define AUD_REG_DR_ASRC_CH2_IBUF_RP            (g_u4ASRC_Reg_Base + 0x48)
#define AUD_REG_DR_ASRC_CH3_IBUF_RP            (g_u4ASRC_Reg_Base + 0x4C)
#define AUD_REG_DR_ASRC_CH0_IBUF_WP            (g_u4ASRC_Reg_Base + 0x50)
#define AUD_REG_DR_ASRC_CH1_IBUF_WP            (g_u4ASRC_Reg_Base + 0x54)
#define AUD_REG_DR_ASRC_CH2_IBUF_WP            (g_u4ASRC_Reg_Base + 0x58)
#define AUD_REG_DR_ASRC_CH3_IBUF_WP            (g_u4ASRC_Reg_Base + 0x5C)
#define AUD_REG_DR_ASRC_CH0_OBUF_WP            (g_u4ASRC_Reg_Base + 0x60)
#define AUD_REG_DR_ASRC_CH1_OBUF_WP            (g_u4ASRC_Reg_Base + 0x6A)
#define AUD_REG_DR_ASRC_CH2_OBUF_WP            (g_u4ASRC_Reg_Base + 0x6B)
#define AUD_REG_DR_ASRC_CH3_OBUF_WP            (g_u4ASRC_Reg_Base + 0x6C)
#define AUD_REG_DR_ASRC_CH0_OBUF_RP            (g_u4ASRC_Reg_Base + 0x70)
#define AUD_REG_DR_ASRC_CH1_OBUF_RP            (g_u4ASRC_Reg_Base + 0x74)
#define AUD_REG_DR_ASRC_CH2_OBUF_RP            (g_u4ASRC_Reg_Base + 0x78)
#define AUD_REG_DR_ASRC_CH3_OBUF_RP            (g_u4ASRC_Reg_Base + 0x7C)
#define AUD_REG_DR_ASRC_CH01_IBUF_INTR         (g_u4ASRC_Reg_Base + 0x80)
#define AUD_REG_DR_ASRC_CH23_IBUF_INTR         (g_u4ASRC_Reg_Base + 0x84)
#define AUD_REG_DR_ASRC_CH01_OBUF_INTR         (g_u4ASRC_Reg_Base + 0x88)
#define AUD_REG_DR_ASRC_CH23_OBUF_INTR         (g_u4ASRC_Reg_Base + 0x8C)
#define AUD_REG_DR_ASRC_BAK                    (g_u4ASRC_Reg_Base + 0x90)
#define AUD_REG_DR_ASRC_FREQ_CTRL              (g_u4ASRC_Reg_Base + 0x94)
#define AUD_REG_DR_ASRC_FREQ_CYC               (g_u4ASRC_Reg_Base + 0x98)
#define AUD_REG_DR_ASRC_PRD_RSLT               (g_u4ASRC_Reg_Base + 0x9C)
#define AUD_REG_DR_ASRC_FREQ_RSLT              (g_u4ASRC_Reg_Base + 0xA0)
#define AUD_REG_DR_ASRC_DSP2_IER               (g_u4ASRC_Reg_Base + 0xC8)
#define AUD_REG_DR_ASRC_DSP3_IER               (g_u4ASRC_Reg_Base + 0xCC)
#define AUD_REG_DR_ASRC_FREQ_TRANS_NUM         (g_u4ASRC_Reg_Base + 0xD8)
#define AUD_REG_DR_ASRC_DMA_CFG                (g_u4ASRC_Reg_Base + 0xFC)


/////////////////////////////////////////////////////////////////////////////
//         GPS ASRC Register Accessed by DSP
/////////////////////////////////////////////////////////////////////////////
#define AUD_REG_GD_ASRC_BASE                   (0x300)

#define AUD_REG_GD_ASRC_GEN_CFG                (AUD_REG_GD_ASRC_BASE + 0x00)
#define AUD_REG_GD_ASRC_IER                    (AUD_REG_GD_ASRC_BASE + 0x01)
#define AUD_REG_GD_ASRC_IFR                    (AUD_REG_GD_ASRC_BASE + 0x02)
#define AUD_REG_GD_ASRC_CH0_CFG                (AUD_REG_GD_ASRC_BASE + 0x04)
#define AUD_REG_GD_ASRC_CH1_CFG                (AUD_REG_GD_ASRC_BASE + 0x05)
#define AUD_REG_GD_ASRC_CH2_CFG                (AUD_REG_GD_ASRC_BASE + 0x06)
#define AUD_REG_GD_ASRC_CH3_CFG                (AUD_REG_GD_ASRC_BASE + 0x07)
#define AUD_REG_GD_ASRC_FREQ_0                 (AUD_REG_GD_ASRC_BASE + 0x08)
#define AUD_REG_GD_ASRC_FREQ_1                 (AUD_REG_GD_ASRC_BASE + 0x09)
#define AUD_REG_GD_ASRC_FREQ_2                 (AUD_REG_GD_ASRC_BASE + 0x0A)
#define AUD_REG_GD_ASRC_FREQ_3                 (AUD_REG_GD_ASRC_BASE + 0x0B)
#define AUD_REG_GD_ASRC_IBUF_SADR              (AUD_REG_GD_ASRC_BASE + 0x0C)
#define AUD_REG_GD_ASRC_IBUF_SIZE              (AUD_REG_GD_ASRC_BASE + 0x0D)
#define AUD_REG_GD_ASRC_OBUF_SADR              (AUD_REG_GD_ASRC_BASE + 0x0E)
#define AUD_REG_GD_ASRC_OBUF_SIZE              (AUD_REG_GD_ASRC_BASE + 0x0F)
#define AUD_REG_GD_ASRC_CH0_IBUF_RP            (AUD_REG_GD_ASRC_BASE + 0x10)
#define AUD_REG_GD_ASRC_CH1_IBUF_RP            (AUD_REG_GD_ASRC_BASE + 0x11)
#define AUD_REG_GD_ASRC_CH2_IBUF_RP            (AUD_REG_GD_ASRC_BASE + 0x12)
#define AUD_REG_GD_ASRC_CH3_IBUF_RP            (AUD_REG_GD_ASRC_BASE + 0x13)
#define AUD_REG_GD_ASRC_CH0_IBUF_WP            (AUD_REG_GD_ASRC_BASE + 0x14)
#define AUD_REG_GD_ASRC_CH1_IBUF_WP            (AUD_REG_GD_ASRC_BASE + 0x15)
#define AUD_REG_GD_ASRC_CH2_IBUF_WP            (AUD_REG_GD_ASRC_BASE + 0x16)
#define AUD_REG_GD_ASRC_CH3_IBUF_WP            (AUD_REG_GD_ASRC_BASE + 0x17)
#define AUD_REG_GD_ASRC_CH0_OBUF_WP            (AUD_REG_GD_ASRC_BASE + 0x18)
#define AUD_REG_GD_ASRC_CH1_OBUF_WP            (AUD_REG_GD_ASRC_BASE + 0x19)
#define AUD_REG_GD_ASRC_CH2_OBUF_WP            (AUD_REG_GD_ASRC_BASE + 0x1A)
#define AUD_REG_GD_ASRC_CH3_OBUF_WP            (AUD_REG_GD_ASRC_BASE + 0x1B)
#define AUD_REG_GD_ASRC_CH0_OBUF_RP            (AUD_REG_GD_ASRC_BASE + 0x1C)
#define AUD_REG_GD_ASRC_CH1_OBUF_RP            (AUD_REG_GD_ASRC_BASE + 0x1D)
#define AUD_REG_GD_ASRC_CH2_OBUF_RP            (AUD_REG_GD_ASRC_BASE + 0x1E)
#define AUD_REG_GD_ASRC_CH3_OBUF_RP            (AUD_REG_GD_ASRC_BASE + 0x1F)
#define AUD_REG_GD_ASRC_CH01_IBUF_INTR         (AUD_REG_GD_ASRC_BASE + 0x20)
#define AUD_REG_GD_ASRC_CH23_IBUF_INTR         (AUD_REG_GD_ASRC_BASE + 0x21)
#define AUD_REG_GD_ASRC_CH01_OBUF_INTR         (AUD_REG_GD_ASRC_BASE + 0x22)
#define AUD_REG_GD_ASRC_CH23_OBUF_INTR         (AUD_REG_GD_ASRC_BASE + 0x23)
#define AUD_REG_GD_ASRC_BAK                    (AUD_REG_GD_ASRC_BASE + 0x24)
#define AUD_REG_GD_ASRC_FREQ_CTRL              (AUD_REG_GD_ASRC_BASE + 0x25)
#define AUD_REG_GD_ASRC_FREQ_CYC               (AUD_REG_GD_ASRC_BASE + 0x26)
#define AUD_REG_GD_ASRC_PRD_RSLT               (AUD_REG_GD_ASRC_BASE + 0x27)
#define AUD_REG_GD_ASRC_FREQ_RSLT              (AUD_REG_GD_ASRC_BASE + 0x28)
//#define AUD_REG_GD_ASRC_IER2                   (AUD_REG_GD_ASRC_BASE + 0x29)
//#define AUD_REG_GD_ASRC_IFR2                   (AUD_REG_GD_ASRC_BASE + 0x2A)
//#define AUD_REG_GD_ASRC_CH4_CFG                (AUD_REG_GD_ASRC_BASE + 0x2B)
//#define AUD_REG_GD_ASRC_CH4_IBUF_RP            (AUD_REG_GD_ASRC_BASE + 0x2C)
//#define AUD_REG_GD_ASRC_CH4_IBUF_WP            (AUD_REG_GD_ASRC_BASE + 0x2D)
//#define AUD_REG_GD_ASRC_CH4_OBUF_RP            (AUD_REG_GD_ASRC_BASE + 0x2E)
//#define AUD_REG_GD_ASRC_CH4_OBUF_WP            (AUD_REG_GD_ASRC_BASE + 0x2F)
//#define AUD_REG_GD_ASRC_CH45_IBUF_INTR         (AUD_REG_GD_ASRC_BASE + 0x30)
//#define AUD_REG_GD_ASRC_CH45_OBUF_INTR         (AUD_REG_GD_ASRC_BASE + 0x31)
#define AUD_REG_GD_ASRC_DSP2_IER               (AUD_REG_GD_ASRC_BASE + 0x32)
#define AUD_REG_GD_ASRC_DSP3_IER               (AUD_REG_GD_ASRC_BASE + 0x33)
//#define AUD_REG_GD_ASRC_DSP2_IER2              (AUD_REG_GD_ASRC_BASE + 0x34)
//#define AUD_REG_GD_ASRC_DSP3_IER2              (AUD_REG_GD_ASRC_BASE + 0x35)
#define AUD_REG_GD_ASRC_FREQ_TRANS_NUM         (AUD_REG_GD_ASRC_BASE + 0x36)
#define AUD_REG_GD_ASRC_DMA_CFG                (AUD_REG_GD_ASRC_BASE + 0x3F)


/////////////////////////////////////////////////////////////////////////////
//         DSP ASRC Register Accessed by DSP
/////////////////////////////////////////////////////////////////////////////
#define AUD_REG_DD_ASRC_BASE                   0x500

#define AUD_REG_DD_ASRC_GEN_CFG                (AUD_REG_DD_ASRC_BASE + 0x80)
#define AUD_REG_DD_ASRC_IER                    (AUD_REG_DD_ASRC_BASE + 0x81)
#define AUD_REG_DD_ASRC_IFR                    (AUD_REG_DD_ASRC_BASE + 0x82)
#define AUD_REG_DD_ASRC_CH0_CFG                (AUD_REG_DD_ASRC_BASE + 0x84)
#define AUD_REG_DD_ASRC_CH1_CFG                (AUD_REG_DD_ASRC_BASE + 0x85)
#define AUD_REG_DD_ASRC_CH2_CFG                (AUD_REG_DD_ASRC_BASE + 0x86)
#define AUD_REG_DD_ASRC_CH3_CFG                (AUD_REG_DD_ASRC_BASE + 0x87)
#define AUD_REG_DD_ASRC_FREQ_0                 (AUD_REG_DD_ASRC_BASE + 0x88)
#define AUD_REG_DD_ASRC_FREQ_1                 (AUD_REG_DD_ASRC_BASE + 0x89)
#define AUD_REG_DD_ASRC_FREQ_2                 (AUD_REG_DD_ASRC_BASE + 0x8A)
#define AUD_REG_DD_ASRC_FREQ_3                 (AUD_REG_DD_ASRC_BASE + 0x8B)
#define AUD_REG_DD_ASRC_IBUF_SADR              (AUD_REG_DD_ASRC_BASE + 0x8C)
#define AUD_REG_DD_ASRC_IBUF_SIZE              (AUD_REG_DD_ASRC_BASE + 0x8D)
#define AUD_REG_DD_ASRC_OBUF_SADR              (AUD_REG_DD_ASRC_BASE + 0x8E)
#define AUD_REG_DD_ASRC_OBUF_SIZE              (AUD_REG_DD_ASRC_BASE + 0x8F)
#define AUD_REG_DD_ASRC_CH0_IBUF_RP            (AUD_REG_DD_ASRC_BASE + 0x90)
#define AUD_REG_DD_ASRC_CH1_IBUF_RP            (AUD_REG_DD_ASRC_BASE + 0x91)
#define AUD_REG_DD_ASRC_CH2_IBUF_RP            (AUD_REG_DD_ASRC_BASE + 0x92)
#define AUD_REG_DD_ASRC_CH3_IBUF_RP            (AUD_REG_DD_ASRC_BASE + 0x93)
#define AUD_REG_DD_ASRC_CH0_IBUF_WP            (AUD_REG_DD_ASRC_BASE + 0x94)
#define AUD_REG_DD_ASRC_CH1_IBUF_WP            (AUD_REG_DD_ASRC_BASE + 0x95)
#define AUD_REG_DD_ASRC_CH2_IBUF_WP            (AUD_REG_DD_ASRC_BASE + 0x96)
#define AUD_REG_DD_ASRC_CH3_IBUF_WP            (AUD_REG_DD_ASRC_BASE + 0x97)
#define AUD_REG_DD_ASRC_CH0_OBUF_WP            (AUD_REG_DD_ASRC_BASE + 0x98)
#define AUD_REG_DD_ASRC_CH1_OBUF_WP            (AUD_REG_DD_ASRC_BASE + 0x99)
#define AUD_REG_DD_ASRC_CH2_OBUF_WP            (AUD_REG_DD_ASRC_BASE + 0x9A)
#define AUD_REG_DD_ASRC_CH3_OBUF_WP            (AUD_REG_DD_ASRC_BASE + 0x9B)
#define AUD_REG_DD_ASRC_CH0_OBUF_RP            (AUD_REG_DD_ASRC_BASE + 0x9C)
#define AUD_REG_DD_ASRC_CH1_OBUF_RP            (AUD_REG_DD_ASRC_BASE + 0x9D)
#define AUD_REG_DD_ASRC_CH2_OBUF_RP            (AUD_REG_DD_ASRC_BASE + 0x9E)
#define AUD_REG_DD_ASRC_CH3_OBUF_RP            (AUD_REG_DD_ASRC_BASE + 0x9F)
#define AUD_REG_DD_ASRC_CH01_IBUF_INTR         (AUD_REG_DD_ASRC_BASE + 0xA0)
#define AUD_REG_DD_ASRC_CH23_IBUF_INTR         (AUD_REG_DD_ASRC_BASE + 0xA1)
#define AUD_REG_DD_ASRC_CH01_OBUF_INTR         (AUD_REG_DD_ASRC_BASE + 0xA2)
#define AUD_REG_DD_ASRC_CH23_OBUF_INTR         (AUD_REG_DD_ASRC_BASE + 0xA3)
#define AUD_REG_DD_ASRC_BAK                    (AUD_REG_DD_ASRC_BASE + 0xA4)
#define AUD_REG_DD_ASRC_FREQ_CTRL              (AUD_REG_DD_ASRC_BASE + 0xA5)
#define AUD_REG_DD_ASRC_FREQ_CYC               (AUD_REG_DD_ASRC_BASE + 0xA6)
#define AUD_REG_DD_ASRC_PRD_RSLT               (AUD_REG_DD_ASRC_BASE + 0xA7)
#define AUD_REG_DD_ASRC_FREQ_RSLT              (AUD_REG_DD_ASRC_BASE + 0xA8)
#define AUD_REG_DD_ASRC_DSP2_IER               (AUD_REG_DD_ASRC_BASE + 0xB2)
#define AUD_REG_DD_ASRC_DSP3_IER               (AUD_REG_DD_ASRC_BASE + 0xB3)
#define AUD_REG_DD_ASRC_FREQ_TRANS_NUM         (AUD_REG_DD_ASRC_BASE + 0xB6)
#define AUD_REG_DD_ASRC_DMA_CFG                (AUD_REG_DD_ASRC_BASE + 0xBF)


#endif // #ifndef _AUDIO_3360_REG_ASRC_H_
