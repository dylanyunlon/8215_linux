/********************************************************************************************
 *     LEGAL DISCLAIMER
 *
 *     (Header of AutoChips Software/Firmware Release or Documentation)
 *
 *     BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *     THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("AutoChips SOFTWARE") RECEIVED
 *     FROM AutoChips AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS
 *     ONLY. AutoChips EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED,
 *     INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
 *     A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES AutoChips PROVIDE ANY WARRANTY
 *     WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE AutoChips SOFTWARE, AND BUYER AGREES TO LOOK
 *     ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. AutoChips SHALL ALSO
 *     NOT BE RESPONSIBLE FOR ANY AutoChips SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION
 *     OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 *     BUYER'S SOLE AND EXCLUSIVE REMEDY AND AutoChips'S ENTIRE AND CUMULATIVE LIABILITY WITH
 *     RESPECT TO THE AutoChips SOFTWARE RELEASED HEREUNDER WILL BE, AT AutoChips'S OPTION,
 *     TO REVISE OR REPLACE THE AutoChips SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO AutoChips FOR SUCH AutoChips SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/

/******************************************************************************
*[File]                     apcm_asrc_reg.h
*[Author]                   tongfa.luo@autochips.com
*[Description]
*
******************************************************************************/
#ifndef _APCM_ASRC_REG_H_
#define _APCM_ASRC_REG_H_

#include "apcm_asrc.h"

#define ASRC_REGBASE                0xA8600

/*******************************************************************************************
ASRC_GEN_CONF                   ASM General Configuration Register

[00 : 07]   resv0000
[08]        EN                  ASRC Enable.         (ASRC_GEN_CONF2 not has)
                                The central enable signal, should be turned on after all configration are set.
[09]        ASRC_BUSY           ASRC busy flag for CH-set 0~3   / 4~5
                                1 means chset is running
[10]        resv0010
[11]        DSP_CTRL_COEF       DSP control cofficient sram         (ASRC_GEN_CONF2 not has)
                                1: dsp can access coefficient SRAM, Through
                                ASM_IIR_CRAM_ADDR(0x5bc) and ASM_IIR_CRAM_DATA(05bd)
[12 : 15]   CH_EN               Ecach CH-set enable signal for CH-set 0~3 / 4~5.
                                This register controls which CH-set 0~3 / 4~5 should be execute.
[16 : 19]   CH_CLEAR            Ecach CH-set clear signal for CH-set 0~3 / 4~5.
                                Set to 1 means the chset will clear history at next run.
[20 : 23]   CH_CNTX_SWEN        Context switch disabler.
                                Disable context switch of each Ch-set for CH set 0~3 / 4~5, remember to stop
                                each Ch-set first by set related enable before using this registers.
[24 : 31]   resv0024
*******************************************************************************************/
#define REG_ASRC_GEN_CONF		(ASRC_REGBASE + 0x00)
#define REG_ASRC_GEN_CONF2		(ASRC_REGBASE + 0x0C)

#define REG_ASRC_ENABLE(val)		AUDREG_BITS_W(REG_ASRC_GEN_CONF, 8, 1, val)
#define REG_ASRC_ENABLE_CHSET(idx, val)	AUDREG_BITS_W(_asrc_reg.gen_conf[idx/4], (12 + idx%4), 1, val)
#define REG_ASRC_CLEAR(idx)		AUDREG_BITS_W(_asrc_reg.gen_conf[idx/4], (16 + idx%4), 1, 1)


/*****************************************************************************************
ASRC_IFS_OFS_SEL
[00 : 07]   resv0000
[08]        CS0_IFS_SEL_2           Channel set 0 IFS select bit 2
[09]        CS0_OFS_SEL_2           Channel set 0 OFS select bit 2
[10]        CS1_IFS_SEL_2           Channel set 1 IFS select bit 2
[11]        CS1_OFS_SEL_2           Channel set 1 OFS select bit 2
[12]        CS2_IFS_SEL_2           Channel set 2 IFS select bit 2
[13]        CS2_OFS_SEL_2           Channel set 2 OFS select bit 2
[14]        CS3_IFS_SEL_2           Channel set 3 IFS select bit 2
[15]        CS3_OFS_SEL_2           Channel set 3 OFS select bit 2
[16]        CS4_IFS_SEL_2           Channel set 4 IFS select bit 2
[17]        CS4_OFS_SEL_2           Channel set 4 OFS select bit 2
[18]        CS5_IFS_SEL_2           Channel set 5 IFS select bit 2
[19]        CS5_OFS_SEL_2           Channel set 5 OFS select bit 2
*****************************************************************************************/

/********************************************************************************************************
ASRC_CH**_CNFG      (0x10, 0x14, 0x18, 0x1C, 0xAC, 0x100)   Channel Set 0 ~ 5 Configuration Register

[00 : 03]   resv0000
[04 : 06]   IIR_STAGE       Anti - alias IIR filter stage
                            Define how many 2-order IIR stage are cascaded for the anti-alias filter. This value
                            should be "real stage amunt" minus 1, which up to 8 stage and 16 order is supported.
[07]        IIR_ENABLE      Anti -alias IIR filter enable.
                            set 1 to run on the anti-alias IIR filter.
[08 : 15]   CLAC_AMOUNT     Calculation amount.
                            Define how many 128-bit output the related channel pair should calculate at each turn.
[16 : 17]   IFS             Input Sample Rate Selection.  set 0, 1, 2 to choose realted frequency on palette
[18 : 19]   OFS             Output Sample Rate Selection.  set 0, 1, 2 to choose realted frequency on palette
[20]        MONO            Mono/Stereo Selection Regsiter    ||  0: stereo. / 1:  mono.
[21]        IBIT_WIDTH      Bit-width Selection for Input    ||  0: 24-bit / 1: 16-bit
[22]        OBIT_WIDTH      Bit-width Selection for Input    ||  0: 24-bit / 1: 16-bit
[23]        IIR_BUF_CLR     Set 1 to clear current IIR output history buffer for IIR limit -cycle problem prevention
                            this register will aut_clear once the history are cleared
[24 : 31]   resv0024
*********************************************************************************************************/
#define REG_ASRC_CH01_CNFG		(ASRC_REGBASE + 0x10)
#define REG_ASRC_CH23_CNFG		(ASRC_REGBASE + 0x14)
#define REG_ASRC_CH45_CNFG		(ASRC_REGBASE + 0x18)
#define REG_ASRC_CH67_CNFG		(ASRC_REGBASE + 0x1C)
#define REG_ASRC_CH89_CNFG 		(ASRC_REGBASE + 0xAC)
#define REG_ASRC_CH1011_CNFG		(ASRC_REGBASE + 0x100)
#define REG_ASRC_IFS_OFS_SEL		(ASRC_REGBASE + 0x124)

#define REG_ASRC_DEF_CACL_AMOUNT	(3)
#define REG_ASRC_SET_CALC_AMOUNT(idx)	AUDREG_BITS_W(_asrc_reg.ch_conf[idx], 8, 8, REG_ASRC_DEF_CACL_AMOUNT)

#define REG_ASRC_SET_MONO(idx)		AUDREG_BITS_W(_asrc_reg.ch_conf[idx], 20, 1, false)
#define REG_ASRC_SET_IBW(idx, val)	AUDREG_BITS_W(_asrc_reg.ch_conf[idx], 21, 1, ((val == 16) ? 1 : 0))
#define REG_ASRC_SET_OBW(idx, val)	AUDREG_BITS_W(_asrc_reg.ch_conf[idx], 22, 1, ((val == 16) ? 1 : 0))

#define REG_ASRC_SET_IFS(idx, val)	AUDREG_BITS_W(_asrc_reg.ch_conf[idx], 16, 2, (val%4));   \
					AUDREG_BITS_W(REG_ASRC_IFS_OFS_SEL, (8 + idx*2), 1, (val/4));

#define REG_ASRC_SET_OFS(idx, val)	AUDREG_BITS_W(_asrc_reg.ch_conf[idx], 18, 2, (val%4));   \
					AUDREG_BITS_W(REG_ASRC_IFS_OFS_SEL, (9 + idx*2), 1, (val/4));


/*****************************************************************************
ASRC_FS                     Frequency palette 0 ~ 7

[00 : 23]   FREQUENCY       The frequency 'palette' for each channel set to
                            define  its input frequency & output frequency
[24 : 31]   resv0024
*****************************************************************************/
#define REG_ASRC_FREQUENCY0		(ASRC_REGBASE + 0x20)
#define REG_ASRC_FREQUENCY1		(ASRC_REGBASE + 0x24)
#define REG_ASRC_FREQUENCY2		(ASRC_REGBASE + 0x28)
#define REG_ASRC_FREQUENCY3		(ASRC_REGBASE + 0x2C)
#define REG_ASRC_FREQUENCY4		(ASRC_REGBASE + 0x114)
#define REG_ASRC_FREQUENCY5		(ASRC_REGBASE + 0x118)
#define REG_ASRC_FREQUENCY6		(ASRC_REGBASE + 0x11C)
#define REG_ASRC_FREQUENCY7		(ASRC_REGBASE + 0x120)

#define REG_ASRC_SET_FREQ(idx, val)	AUDREG_BITS_W(_asrc_reg.fs[idx], 0, 24, val)


/*****************************************************************************************
AUD_REG_RGBK2_CFG0          0xA8080
[00 : 04]   RBANK_RGB2                  register wxpand for maping to dsp address
[06]        GPS_ASRC_RESTB              gps asrc soft reset
[07]        GPS_APLL_SEL                claculation clock sel
[08 : 18]   GPS_AIN_DMA_ADR_HIGH        gps asrc waddr high bits
[20 : 30]   GPS_ASM_RD_ADR_HIGH         gps asrc raddr high bits
*****************************************************************************************/
/****************************************************************************************
ASRC_CH_IBUF/OBUT_SIZE      Input/Output Channel Size
[00 : 03]   resv0000
[04 : 19]   SIZE            The input/output buffer size for each channel, in 128-bit unit.
                            Each channel uses the same size and is circular.
[20 : 31]   resv0020
*****************************************************************************************/
/*****************************************************************************************
ASRC_IBUF/OBUF_SADR         Input/Output Buffer Start Address.
[00 : 03]   resv0000
[04 : 23]   SADR            The start address of input/output buffer, in 128-bit unit.
[24 : 31]   resv0024
*****************************************************************************************/
#define REG_ASRC_IBUF_SADR		(ASRC_REGBASE + 0x30)
#define REG_ASRC_IBUF_SIZE		(ASRC_REGBASE + 0x34)
#define REG_ASRC_OBUF_SADR		(ASRC_REGBASE + 0x38)
#define REG_ASRC_OBUF_SIZE		(ASRC_REGBASE + 0x3C)
#define REG_ASRC_BANK_ADDR		(0xA8080)

#define REG_ASRC_SET_IBUF_INFO(addr, size)	\
 	do {	\
 		AUDREG_BITS_W(REG_ASRC_BANK_ADDR, 8, 11, (addr >> 20)); \
		AUDREG_BITS_W(REG_ASRC_IBUF_SADR, 0, 24, (addr & 0xFFFFF)); \
		AUDREG_BITS_W(REG_ASRC_IBUF_SIZE, 0, 20, size); \
	} while (0);

#define REG_ASRC_SET_OBUF_INFO(addr, size)	\
 	do {	\
 		AUDREG_BITS_W(REG_ASRC_BANK_ADDR, 20, 11, (addr >> 20)); \
		AUDREG_BITS_W(REG_ASRC_OBUF_SADR, 0, 24, (addr & 0xFFFFF)); \
		AUDREG_BITS_W(REG_ASRC_OBUF_SIZE, 0, 20, size); \
	} while (0);


/**************************************************************************************
ASRC_CH01_IBUF_RDPNT        Input Buffer Read Address Register. (128bit alignmen)

[00 : 03]   resv0000
[04 : 23]   IBUF_RDPNT      Current input buffer read address for each channel pair.
                            For stereo channel, this value is related to first channel.
[24 : 31]   resv0024
**************************************************************************************/
#define REG_ASRC_CH01_IBUF_RDPNT	(ASRC_REGBASE + 0x40)
#define REG_ASRC_CH23_IBUF_RDPNT	(ASRC_REGBASE + 0x44)
#define REG_ASRC_CH45_IBUF_RDPNT	(ASRC_REGBASE + 0x48)
#define REG_ASRC_CH67_IBUF_RDPNT	(ASRC_REGBASE + 0x4C)
#define REG_ASRC_CH89_IBUF_RDPNT	(ASRC_REGBASE + 0xB0)
#define REG_ASRC_CH1011_IBUF_RDPNT	(ASRC_REGBASE + 0x104)

#define REG_ASRC_GET_IBUF_RP(idx)	(AUDREG_BITS_R(_asrc_reg.ibuf_rp[idx], 0, 24) & 0xFFFFF0)
#define REG_ASRC_SET_IBUF_RP(idx, val)	AUDREG_BITS_W(_asrc_reg.ibuf_rp[idx], 0, 24, ((val) & 0xFFFFF0))


/**************************************************************************************
ASRC_CH01_IBUF_WRPNT        Input buffer write address register.  (128bit alignmen)

[00 : 03]   resv0000
[04 : 23]   IBUF_WRPNT      Current input buffer write address for each channel pair.
                            For stereo channel, this value is related to first channel.
[24 : 31]   resv0024
**************************************************************************************/
#define REG_ASRC_CH01_IBUF_WRPNT	(ASRC_REGBASE + 0x50)
#define REG_ASRC_CH23_IBUF_WRPNT	(ASRC_REGBASE + 0x54)
#define REG_ASRC_CH45_IBUF_WRPNT	(ASRC_REGBASE + 0x58)
#define REG_ASRC_CH67_IBUF_WRPNT	(ASRC_REGBASE + 0x5C)
#define REG_ASRC_CH89_IBUF_WRPNT	(ASRC_REGBASE + 0xB4)
#define REG_ASRC_CH1011_IBUF_WRPNT	(ASRC_REGBASE + 0x108)

#define REG_ASRC_GET_IBUF_WP(idx)	(AUDREG_BITS_R(_asrc_reg.ibuf_wp[idx], 0, 24) & 0xFFFFF0)
#define REG_ASRC_SET_IBUF_WP(idx, val)	AUDREG_BITS_W(_asrc_reg.ibuf_wp[idx], 0, 24, ((val) & 0xFFFFF0))


/**************************************************************************************
ASRC_CH**_OBUF_WRPNT        Output buffer write address register. (128bit alignmen)

[00 : 03]   resv0000
[04 : 23]   OBUF_WRPNT      Current output buffer write address for each channel pair.
                            For stereo channel, this value is related to first channel.
[24 : 31]   resv0024
**************************************************************************************/
#define REG_ASRC_CH01_OBUF_WRPNT	(ASRC_REGBASE + 0x60)
#define REG_ASRC_CH23_OBUF_WRPNT	(ASRC_REGBASE + 0x64)
#define REG_ASRC_CH45_OBUF_WRPNT	(ASRC_REGBASE + 0x68)
#define REG_ASRC_CH67_OBUF_WRPNT	(ASRC_REGBASE + 0x6C)
#define REG_ASRC_CH89_OBUF_WRPNT	(ASRC_REGBASE + 0xB8)
#define REG_ASRC_CH1011_OBUF_WRPNT	(ASRC_REGBASE + 0x10c)

#define REG_ASRC_GET_OBUF_WP(idx)	(AUDREG_BITS_R(_asrc_reg.obuf_wp[idx], 0, 24) & 0xFFFFF0)
#define REG_ASRC_SET_OBUF_WP(idx, val)	AUDREG_BITS_W(_asrc_reg.obuf_wp[idx], 0, 24, ((val) & 0xFFFFF0))


/**************************************************************************************
ASRC_CH**_OBUF_RDPNT        Output buffer read address register. (128bit alignmen)

[00 : 03]   resv0000
[04 : 23]   OBUF_RDPNT      Current output buffer read address for each channel pair.
                            For stereo channel, this value is related to first channel.
[24 : 31]   resv0024
**************************************************************************************/
#define REG_ASRC_CH01_OBUF_RDPNT	(ASRC_REGBASE + 0x70)
#define REG_ASRC_CH23_OBUF_RDPNT	(ASRC_REGBASE + 0x74)
#define REG_ASRC_CH45_OBUF_RDPNT	(ASRC_REGBASE + 0x78)
#define REG_ASRC_CH67_OBUF_RDPNT	(ASRC_REGBASE + 0x7C)
#define REG_ASRC_CH89_OBUF_RDPNT	(ASRC_REGBASE + 0xBC)
#define REG_ASRC_CH1011_OBUF_RDPNT	(ASRC_REGBASE + 0x110)

#define REG_ASRC_GET_OBUF_RP(idx)	(AUDREG_BITS_R(_asrc_reg.obuf_rp[idx], 0, 24) & 0xFFFFF0)
#define REG_ASRC_SET_OBUF_RP(idx, val)	AUDREG_BITS_W(_asrc_reg.obuf_rp[idx], 0, 24, ((val) & 0xFFFFF0))


/*****************************************************************************************
ASM_MAX_OUT_PER_IN*         ASM Maximum Output amount Per Input for channel set
                            Tell asrc each CH-set translation information to prevent output buffer full
                            this value should be "cell(OFS/IFS)"
                            the ASRC support 8x up-sampling an ~16x down-sample.
[00 : 07]   resv0000
[08 : 11]   MAX_OUT_PER_IN0
[12 : 15]   MAX_OUT_PER_IN1
[16 : 19]   MAX_OUT_PER_IN2
[20 : 23]   MAX_OUT_PER_IN3
[24 : 31]   resv0024
*****************************************************************************************/
#define REG_ASRC_MAX_OUT_PER_IN0		(ASRC_REGBASE + 0xE0)
#define REG_ASRC_MAX_OUT_PER_IN1		(ASRC_REGBASE + 0xE4)

#define REG_ASRC_SET_MAX_OUT_PER_IN(idx, val)	AUDREG_BITS_W(_asrc_reg.max_out_per_in[idx/4], (8 + 4*(idx%4)), 4, val)


/*****************************************************************************************
DMA CFG   (0xFC)
[00 : 01]   BUF_SIZE            ASRC DMA buffer size
                                00:  2X128bits      01:  4X128bits
                                10:  8X128bits      11:  16X128bits
[02 : 03]   resv0002
[04]        DMA_BUF_EMPTY       DMA buffer empty flag
[05]        DMA_BUF_FULL        DMA buffer full flag
[06 : 07]   resv0006
[08]        LAST_CFG
[09 : 15]   resv0009
[16]        RESET
[17 : 31]   resv0024
*****************************************************************************************/
#define REG_ASRC_DMA_CFG		(ASRC_REGBASE + 0xFC)

#define REG_ASRC_RESET_REG()		\
 	do {	\
 		AUDREG_WRITE(0xA8080, AUDREG_READ(0xA8080) | 0x41); \
		AUDREG_WRITE(0xC4, AUDREG_READ(0xC4) | 7); \
 		AUDREG_BITS_W(REG_ASRC_DMA_CFG, 16, 1, 1); \
 	} while (0);


/****************************************************************************************************
ASRC_IER                            Interrupt Enable Register

[00 : 07]   resv0000
[08 : 11]   OBUF_AMOUNT_INTEN       Output buffer amount interupt enable for CH-set 0~3.    (4'b1111)
[12 : 15]   OBUF_OV_INTEN           Output buffer overflow interupt enable for CH-set 0~3
[16 : 19]   IBUF_AMOUNT_INTEN       Input buffer amount interrupt enable for CH-set 0~3     (4'b1111)
[20 : 23]   IBUF_EMPTY_INTEN        Input buffer empty interrupt enable for CH-set 0~3
[24 : 31]   resv0024
*****************************************************************************************************/
/***************************************************************************************************
ASRC_IFR                            Interrupt Enable Register

[00 : 07]   resv0000
[08 : 11]   OBUF_AMOUNT_FLAG        Output Amount Reached Flag for CH-set
                                    Each bit related to one channel pair, 1 means the related channel
                                    pair output amount meets requirement. Write the related bit will clear it.
[12 : 15]   OBUF_OV_FLAG            Output buffer full flag for CH-set
                                    Each bit related to one channel pair, 1 means the related output buffer is full.
                                    Write buffer full will make the whole system hang, please resolve it
                                    as soon as possible. Write the related bit will clear it.
[16 : 19]   IBUF_AMOUNT_FLAG        Input Left Amount Flag for chset
                                    Each bit related to one channel pair, 1 means the related input buffer left amount
                                    reached the dedicated value. Write the related bit will clear it.
[20 : 23]   IBUF_EMPTY_FLAG         Input Buffer Empty Flag for CH-set
                                    Each bit related to one channel pair, 1 means the related input buffer is empty.
                                    Write the related bit will clear it.
[24 : 31]   resv0024
***************************************************************************************************/

#define REG_ASRC_IER				(ASRC_REGBASE + 0xCC)
#define REG_ASRC_IER2				(ASRC_REGBASE + 0xD4)

#define REG_ASRC_IFR				(ASRC_REGBASE + 0x08)
#define REG_ASRC_IFR2				(ASRC_REGBASE + 0xA8)

#define REG_ASRC_READ_IER(idx)			AUDREG_READ(_asrc_reg.ier[idx/4])
#define REG_ASRC_WRITE_IER(idx, val)		AUDREG_WRITE(_asrc_reg.ier[idx/4], val)

#define REG_ASRC_READ_IFR(idx)			AUDREG_READ(_asrc_reg.ifr[idx/4])
#define REG_ASRC_WRITE_IFR(idx, val)		AUDREG_WRITE(_asrc_reg.ifr[idx/4], val)

#define REG_ASRC_GET_OBUF_AMOUNT_FLAG(idx)	AUDREG_BITS_R(_asrc_reg.ier[idx/4], (8 + idx%4), 1) & \
						AUDREG_BITS_R(_asrc_reg.ifr[idx/4], (8 + idx%4), 1)

#define REG_ASRC_GET_OBUF_OV_FLAG(idx)		AUDREG_BITS_R(_asrc_reg.ier[idx/4], (12 + idx%4), 1) & \
						AUDREG_BITS_R(_asrc_reg.ifr[idx/4], (12 + idx%4), 1)

#define REG_ASRC_GET_IBUF_AMOUNT_FLAG(idx)	AUDREG_BITS_R(_asrc_reg.ier[idx/4], (16 + idx%4), 1) & \
						AUDREG_BITS_R(_asrc_reg.ifr[idx/4], (16 + idx%4), 1)

#define REG_ASRC_GET_IBUF_EMPTY_FLAG(idx)	AUDREG_BITS_R(_asrc_reg.ier[idx/4], (20 + idx%4), 1) & \
						AUDREG_BITS_R(_asrc_reg.ifr[idx/4], (20 + idx%4), 1)

#define REG_ASRC_OBUF_AMOUNT_BIT_VAL(idx)	(1 << ( 8 + idx%4))
#define REG_ASRC_OBUF_OV_BIT_VAL(idx)		(1 << (12 + idx%4))
#define REG_ASRC_IBUF_AMOUNT_BIT_VAL(idx)	(1 << (16 + idx%4))
#define REG_ASRC_IBUF_EMPTY_BIT_VAL(idx) 	(1 << (20 + idx%4))


/*********************************************************************************************
ASRC_IBUF_INTR_CNT0             Iutput Buffer Amount Interrupt Register

[00 : 07]   resv0000
[08 : 15]   CH0_IBUF_INTR_CNT   Channel pair0/2 input buffer amount interrupt register.
                                When the related input buffer left less than this amount(in 384-bit unit)
                                of input data. It will rise the input buffer amount flag.
[16 : 23]   CH1_IBUF_INTR_CNT   Channel pair1/3 input buffer amount interrupt register.
                                When the related input buffer left less than this amount(in 384-bit unit)
                                of input data. It will rise the input buffer amount flag.
[24 : 31]        resv0024
*********************************************************************************************/
#define REG_ASRC_IBUF_INTR_CNT0			(ASRC_REGBASE + 0x80)
#define REG_ASRC_IBUF_INTR_CNT1 		(ASRC_REGBASE + 0x84)
#define REG_ASRC_IBUF_INTR_CNT2			(ASRC_REGBASE + 0xC0)

#define REG_ASRC_SET_IBUF_INTR_CNT(idx, val)	AUDREG_BITS_W(_asrc_reg.ibuf_intr_cnt[idx/2], (8 + 8*(idx%2)), 8, val)


/********************************************************************************************
ASRC_OBUF_INTR_CNT*             Output Buffer Amount Interrupt Register

[00 : 07]   resv0000
[08 : 15]   CH0_OBUF_INTR_CNT   Channel pair0 output buffer amount interrupt register.
                                When the related output buffer contain more than this amount
                                (in 384-bit unit) of output data. It will rise the output buffer amount flag.
[16 : 23]   CH1_OBUF_INTR_CNT   Channel pair1 output buffer amount interrupt register.
                                When the related output buffer contain more than this amount
                                (in 384-bit unit) of output data. It will rise the output buffer amount flag.
[24 : 31]   resv0024
********************************************************************************************/
#define REG_ASRC_OBUF_INTR_CNT0			(ASRC_REGBASE + 0x88)
#define REG_ASRC_OBUF_INTR_CNT1			(ASRC_REGBASE + 0x8C)
#define REG_ASRC_OBUF_INTR_CNT2 		(ASRC_REGBASE + 0xC4)

#define REG_ASRC_SET_OBUF_INTR_CNT(idx, val)	AUDREG_BITS_W(_asrc_reg.obuf_intr_cnt[idx/2], (8 + 8*(idx%2)), 8, val)

#endif  //_APCM_ASRC_REG_H_

