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

/******************************************************************************
*[File]                     aud_asrc_reg.h
*[Version]                  v1.0
*[Revision Date]            2014-03-10
*[Author]                   tongfa.luo@autochips.com 
*[Description]
*       asrc hw register definitions 
*
******************************************************************************/
#ifndef _AUD_ASRC_REG_H_
#define _AUD_ASRC_REG_H_

#include "aud_if_hw_asrc.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus


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
#define REG_ASRC_GEN_CONF                               (0x00)
#define REG_ASRC_GEN_CONF2                              (0x0C)

#define RegAsrc_GenConf_EN_R(rMap)                      AUDREG_BITS_R( rMap.GenConf[0], 8, 1) 
#define RegAsrc_GenConf_EN_W(rMap, val)                 AUDREG_BITS_W(rMap.GenConf[0], 8, 1, val)

#define RegAsrc_GenConf_AsrcBusy_R(rMap, idx)           AUDREG_BITS_R( rMap.GenConf[idx/4], 9, 1) 
#define RegAsrc_GenConf_AsrcBusy_W(rMap, idx, val)      AUDREG_BITS_W(rMap.GenConf[idx/4], 9, 1, val)

#define RegAsrc_GenConf_DspCtrlCoef_R(rMap)             AUDREG_BITS_R( rMap.GenConf[0], 11, 1) 
#define RegAsrc_GenConf_DspCtrlCoef_W(rMap, val)        AUDREG_BITS_W(rMap.GenConf[0], 11, 1, val)

#define RegAsrc_GenConf_ChEn_R(rMap, idx)               AUDREG_BITS_R( rMap.GenConf[idx/4], (12 + idx%4), 1)
#define RegAsrc_GenConf_ChEn_W(rMap, idx, val)          AUDREG_BITS_W(rMap.GenConf[idx/4], (12 + idx%4), 1, val)

#define RegAsrc_GenConf_ChClear_R(rMap, idx)            AUDREG_BITS_R( rMap.GenConf[idx/4], (16 + idx%4), 1)
#define RegAsrc_GenConf_ChClear_W(rMap, idx, val)       AUDREG_BITS_W(rMap.GenConf[idx/4], (16 + idx%4), 1, val)

#define RegAsrc_GenConf_ChCntxSwen_R(rMap, idx)         AUDREG_BITS_R( rMap.GenConf[idx/4], (20 + idx%4), 1)
#define RegAsrc_GenConf_ChCntxSwen_W(rMap, idx, val)    AUDREG_BITS_W(rMap.GenConf[idx/4], (20 + idx%4), 1, val)


/****************************************************************************************************
ASRC_IER                            Interrupt Enable Register 

[00 : 07]   resv0000                            
[08 : 11]   OBUF_AMOUNT_INTEN       Output buffer amount interupt enable for CH-set 0~3.    (4'b1111)
[12 : 15]   OBUF_OV_INTEN           Output buffer overflow interupt enable for CH-set 0~3
[16 : 19]   IBUF_AMOUNT_INTEN       Input buffer amount interrupt enable for CH-set 0~3     (4'b1111)
[20 : 23]   IBUF_EMPTY_INTEN        Input buffer empty interrupt enable for CH-set 0~3 
[24 : 31]   resv0024                        
*****************************************************************************************************/      
#define REG_ASRC_IER(eAsrcType)                         ((eAsrcType == GPS_ASRC) ? (0xCC) : (0x04))
#define REG_ASRC_IER2(eAsrcType)                        ((eAsrcType == GPS_ASRC) ? (0xD4) : (0xA4))   

#define RegAsrc_IER_READ(rMap, idx)                      AUDREG_READ( rMap.IER[idx/4])
#define RegAsrc_IER_WRITE(rMap, idx, val)                AUDREG_WRITE(rMap.IER[idx/4], val)

#define RegAsrc_IER_OBufAmountInten_R(rMap, idx)         AUDREG_BITS_R( rMap.IER[idx/4], (8 + idx%4), 1)
#define RegAsrc_IER_OBufAmountInten_W(rMap, idx, val)    AUDREG_BITS_W(rMap.IER[idx/4], (8 + idx%4), 1, val)

#define RegAsrc_IER_OBufOvInten_R(rMap, idx)             AUDREG_BITS_R( rMap.IER[idx/4], (12 + idx%4), 1)
#define RegAsrc_IER_OBufOvInten_W(rMap, idx, val)        AUDREG_BITS_W(rMap.IER[idx/4], (12 + idx%4), 1, val)

#define RegAsrc_IER_IBufAmountInten_R(rMap, idx)         AUDREG_BITS_R( rMap.IER[idx/4], (16 + idx%4), 1)
#define RegAsrc_IER_IBufAmountInten_W(rMap, idx, val)    AUDREG_BITS_W(rMap.IER[idx/4], (16 + idx%4), 1, val)

#define RegAsrc_IER_IBufEmptyInten_R(rMap, idx)          AUDREG_BITS_R( rMap.IER[idx/4], (20 + idx%4), 1)
#define RegAsrc_IER_IBufEmptyInten_W(rMap, idx, val)     AUDREG_BITS_W(rMap.IER[idx/4], (20 + idx%4), 1, val)


/***************************************************************************************************
ASRC_IFR                        Interrupt Enable Register 

[00 : 07]   resv0000                            
[08 : 11]   OBUF_AMOUNT_FLAG    Output Amount Reached Flag for CH-set
                                Each bit related to one channel pair, 1 means the related channel  
                                pair output amount meets requirement. Write the related bit will clear it.
[12 : 15]   OBUF_OV_FLAG        Output buffer full flag for CH-set
                                Each bit related to one channel pair, 1 means the related output buffer is full.
                                Write buffer full will make the whole system hang, please resolve it  
                                as soon as possible. Write the related bit will clear it.
[16 : 19]   IBUF_AMOUNT_FLAG    Input Left Amount Flag for chset
                                Each bit related to one channel pair, 1 means the related input buffer left amount 
                                reached the dedicated value. Write the related bit will clear it.
[20 : 23]   IBUF_EMPTY_FLAG     Input Buffer Empty Flag for CH-set
                                Each bit related to one channel pair, 1 means the related input buffer is empty. 
                                Write the related bit will clear it.
[24 : 31]   resv0024                        
***************************************************************************************************/ 
#define REG_ASRC_IFR                                    (0x08)
#define REG_ASRC_IFR2                                   (0xA8)

#define RegAsrc_IFR_READ(rMap, idx)                     AUDREG_READ( rMap.IFR[idx/4])
#define RegAsrc_IFR_WRITE(rMap, idx, val)               AUDREG_WRITE(rMap.IFR[idx/4], val)

#define RegAsrc_IFR_OBufAmountFlag_R(rMap, idx)         AUDREG_BITS_R( rMap.IFR[idx/4], (8 + idx%4), 1)
#define RegAsrc_IFR_OBufAmountFlag_W(rMap, idx, val)    AUDREG_BITS_W(rMap.IFR[idx/4], (8 + idx%4), 1, val)

#define RegAsrc_IFR_OBufOvFlag_R(rMap, idx)             AUDREG_BITS_R( rMap.IFR[idx/4], (12 + idx%4), 1)
#define RegAsrc_IFR_OBufOvFlag_W(rMap, idx, val)        AUDREG_BITS_W(rMap.IFR[idx/4], (12 + idx%4), 1, val)

#define RegAsrc_IFR_IBufAmountFlag_R(rMap, idx)         AUDREG_BITS_R( rMap.IFR[idx/4], (16 + idx%4), 1)
#define RegAsrc_IFR_IBufAmountFlag_W(rMap, idx, val)    AUDREG_BITS_W(rMap.IFR[idx/4], (16 + idx%4), 1, val

#define RegAsrc_IFR_IBufEmptyFlag_R(rMap, idx)          AUDREG_BITS_R( rMap.IFR[idx/4], (20 + idx%4), 1)
#define RegAsrc_IFR_IBufEmptyFlag_W(rMap, idx, val)     AUDREG_BITS_W(rMap.IFR[idx/4], (20 + idx%4), 1, val)

#define ASRC_OBUF_AMOUNT_BIT_VAL(idx)                   (1 << ( 8 + idx%4))
#define ASRC_OBUF_OV_BIT_VAL(idx)                       (1 << (12 + idx%4))
#define ASRC_IBUF_AMOUNT_BIT_VAL(idx)                   (1 << (16 + idx%4))
#define ASRC_IBUF_EMPTY_BIT_VAL(idx)                    (1 << (20 + idx%4))


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
#define REG_ASRC_CH01_CNFG                              (0x10)
#define REG_ASRC_CH23_CNFG                              (0x14)
#define REG_ASRC_CH45_CNFG                              (0x18)
#define REG_ASRC_CH67_CNFG                              (0x1C)
#define REG_ASRC_CH89_CNFG                              (0xAC)
#define REG_ASRC_CH1011_CNFG                            (0x100)

#define RegAsrc_ChCnfg_READ(rMap, idx)                  AUDREG_READ( rMap.ChCnfg[idx])
#define RegAsrc_ChCnfg_WRITE(rMap, idx, val)            AUDREG_WRITE(rMap.ChCnfg[idx], val)

#define RegAsrc_ChCnfg_IIRStage_R(rMap, idx)            AUDREG_BITS_R( rMap.ChCnfg[idx], 4, 3)
#define RegAsrc_ChCnfg_IIRStage_W(rMap, idx, val)       AUDREG_BITS_W(rMap.ChCnfg[idx], 4, 3, val)

#define RegAsrc_ChCnfg_IIREnable_R(rMap, idx)           AUDREG_BITS_R( rMap.ChCnfg[idx], 7, 1)
#define RegAsrc_ChCnfg_IIREnabke_W(rMap, idx, val)      AUDREG_BITS_W(rMap.ChCnfg[idx], 7, 1, val)

#define RegAsrc_ChCnfg_CalcAmount_R(rMap, idx)          AUDREG_BITS_R( rMap.ChCnfg[idx], 8, 8)
#define RegAsrc_ChCnfg_CalcAmount_W(rMap, idx, val)     AUDREG_BITS_W(rMap.ChCnfg[idx], 8, 8, val)
#define ASRC_CALC_AMOUNT_MAX                            (1 << 8)
#define ASRC_DEF_CACL_AMOUNT                            (3)

#define RegAsrc_ChCnfg_IFS_R(rMap, idx)                 AUDREG_BITS_R( rMap.ChCnfg[idx], 16, 2)
#define RegAsrc_ChCnfg_IFS_W(rMap, idx, val)            AUDREG_BITS_W(rMap.ChCnfg[idx], 16, 2, (val%4))

#define RegAsrc_ChCnfg_OFS_R(rMap, idx)                 AUDREG_BITS_R( rMap.ChCnfg[idx], 18, 2)
#define RegAsrc_ChCnfg_OFS_W(rMap, idx, val)            AUDREG_BITS_W(rMap.ChCnfg[idx], 18, 2, (val%4))

#define RegAsrc_ChCnfg_MONO_R(rMap, idx)                AUDREG_BITS_R( rMap.ChCnfg[idx], 20, 1)
#define RegAsrc_ChCnfg_MONO_W(rMap, idx, val)           AUDREG_BITS_W(rMap.ChCnfg[idx], 20, 1, val)

#define RegAsrc_ChCnfg_IBitWidth_R(rMap, idx)           AUDREG_BITS_R( rMap.ChCnfg[idx], 21, 1)
#define RegAsrc_ChCnfg_IBitWidth_W(rMap, idx, val)      AUDREG_BITS_W(rMap.ChCnfg[idx], 21, 1, val)

#define RegAsrc_ChCnfg_OBitWidth_R(rMap, idx)           AUDREG_BITS_R( rMap.ChCnfg[idx], 22, 1)
#define RegAsrc_ChCnfg_OBitWidth_W(rMap, idx, val)      AUDREG_BITS_W(rMap.ChCnfg[idx], 22, 1, val)

#define RegAsrc_ChCnfg_IIRBufClr_R(rMap, idx)           AUDREG_BITS_R( rMap.ChCnfg[idx], 22, 1)
#define RegAsrc_ChCnfg_IIRBufClr_W(rMap, idx, val)      AUDREG_BITS_W(rMap.ChCnfg[idx], 22, 1, val)


/*****************************************************************************
ASRC_FS                     Frequency palette 0 ~ 7   
                                                      
[00 : 23]   FREQUENCY       The frequency 'palette' for each channel set to 
                            define  its input frequency & output frequency
[24 : 31]   resv0024       
*****************************************************************************/
#define REG_ASRC_FREQUENCY0                             (0x20)          
#define REG_ASRC_FREQUENCY1                             (0x24)
#define REG_ASRC_FREQUENCY2                             (0x28)
#define REG_ASRC_FREQUENCY3                             (0x2C)
#define REG_ASRC_FREQUENCY4                             (0x114)
#define REG_ASRC_FREQUENCY5                             (0x118)
#define REG_ASRC_FREQUENCY6                             (0x11C)
#define REG_ASRC_FREQUENCY7                             (0x120)

#define RegAsrc_Frequency_R(rMap, idx)                  AUDREG_BITS_R( rMap.Fs[idx], 0, 24)
#define RegAsrc_Frequency_W(rMap, idx, val)             AUDREG_BITS_W(rMap.Fs[idx], 0, 24, val)
#define ASRC_FREQ_SUPPORT_MAX                           (1 << 24)


/******************************************************************************
ASRC_IBUF/OBUF_SADR         Input/Output Buffer Start Address.    
[00 : 03]   resv0000
[04 : 23]   SADR            The start address of input/output buffer, in 128-bit unit.
[24 : 31]   resv0024
******************************************************************************/
#define REG_ASRC_IBUF_SADR                              (0x30)
#define RegAsrc_IBufSAdr_R(rMap)                        AUDREG_BITS_R( rMap.IBufSAdr, 0, 24)
#define RegAsrc_IBufSAdr_W(rMap, val)                   AUDREG_BITS_W(rMap.IBufSAdr, 0, 24, val)

#define REG_ASRC_OBUF_SADR                              (0x38)
#define RegAsrc_OBufSAdr_R(rMap)                        AUDREG_BITS_R( rMap.OBufSAdr, 0, 24)
#define RegAsrc_OBufSAdr_W(rMap, val)                   AUDREG_BITS_W(rMap.OBufSAdr, 0, 24, val)

#define ASRC_BUF_SADR_MAX                               (1 << 24)


/******************************************************************************
ASRC_CH_IBUF/OBUT_SIZE      Input/Output Channel Size            
[00 : 03]   resv0000
[04 : 19]   SIZE            The input/output buffer size for each channel, in 128-bit unit. 
                            Each channel uses the same size and is circular.      
[20 : 31]   resv0020
*******************************************************************************/
#define REG_ASRC_IBUF_SIZE                              (0x34)
#define RegAsrc_ChIBufSize_R(rMap)                      AUDREG_BITS_R( rMap.IBufSize, 0, 20)
#define RegAsrc_ChIBufSize_W(rMap, val)                 AUDREG_BITS_W(rMap.IBufSize, 0, 20, val)

#define REG_ASRC_OBUF_SIZE                              (0x3C)
#define RegAsrc_ChOBufSize_R(rMap)                      AUDREG_BITS_R( rMap.OBufSize, 0, 20)
#define RegAsrc_ChOBufSize_W(rMap, val)                 AUDREG_BITS_W(rMap.OBufSize, 0, 20, val)

#define ASRC_BUF_SIZE_MAX                               (1 << 20)


#define ASRC_BUF_POINTER_MAX                            (1 << 24)
/**************************************************************************************
ASRC_CH01_IBUF_RDPNT        Input Buffer Read Address Register. (128bit alignmen)

[00 : 03]   resv0000
[04 : 23]   IBUF_RDPNT      Current input buffer read address for each channel pair. 
                            For stereo channel, this value is related to first channel.
[24 : 31]   resv0024
**************************************************************************************/
#define REG_ASRC_CH01_IBUF_RDPNT                        (0x40)
#define REG_ASRC_CH23_IBUF_RDPNT                        (0x44)
#define REG_ASRC_CH45_IBUF_RDPNT                        (0x48)
#define REG_ASRC_CH67_IBUF_RDPNT                        (0x4C)
#define REG_ASRC_CH89_IBUF_RDPNT                        (0xB0)
#define REG_ASRC_CH1011_IBUF_RDPNT                      (0x104)

#define RegAsrc_IBufRdpnt_R(rMap, idx)                  AUDREG_BITS_R( rMap.IBufRp[idx], 0, 24)
#define RegAsrc_IBufRdpnt_W(rMap, idx, val)             AUDREG_BITS_W(rMap.IBufRp[idx], 0, 24, val)


/**************************************************************************************
ASRC_CH01_IBUF_WRPNT        Input buffer write address register.  (128bit alignmen)

[00 : 03]   resv0000
[04 : 23]   IBUF_WRPNT      Current input buffer write address for each channel pair. 
                            For stereo channel, this value is related to first channel.
[24 : 31]   resv0024                                        
**************************************************************************************/
#define REG_ASRC_CH01_IBUF_WRPNT                        (0x50)
#define REG_ASRC_CH23_IBUF_WRPNT                        (0x54)
#define REG_ASRC_CH45_IBUF_WRPNT                        (0x58)
#define REG_ASRC_CH67_IBUF_WRPNT                        (0x5C)
#define REG_ASRC_CH89_IBUF_WRPNT                        (0xB4)
#define REG_ASRC_CH1011_IBUF_WRPNT                      (0x108)

#define RegAsrc_IBufWrpnt_R(rMap, idx)                  AUDREG_BITS_R( rMap.IBufWp[idx], 0, 24)
#define RegAsrc_IBufWrpnt_W(rMap, idx, val)             AUDREG_BITS_W(rMap.IBufWp[idx], 0, 24, val)


/**************************************************************************************
ASRC_CH**_OBUF_WRPNT        Output buffer write address register. (128bit alignmen)

[00 : 03]   resv0000
[04 : 23]   OBUF_WRPNT      Current output buffer write address for each channel pair. 
                            For stereo channel, this value is related to first channel.
[24 : 31]   resv0024
**************************************************************************************/
#define REG_ASRC_CH01_OBUF_WRPNT                        (0x60)
#define REG_ASRC_CH23_OBUF_WRPNT                        (0x64)
#define REG_ASRC_CH45_OBUF_WRPNT                        (0x68)
#define REG_ASRC_CH67_OBUF_WRPNT                        (0x6C)
#define REG_ASRC_CH89_OBUF_WRPNT                        (0xB8)
#define REG_ASRC_CH1011_OBUF_WRPNT                      (0x10c)

#define RegAsrc_OBufWrpnt_R(rMap, idx)                  AUDREG_BITS_R( rMap.OBufWp[idx], 0, 24)
#define RegAsrc_OBufWrpnt_W(rMap, idx, val)             AUDREG_BITS_W(rMap.OBufWp[idx], 0, 24, val)


/**************************************************************************************
ASRC_CH**_OBUF_RDPNT        Output buffer read address register. (128bit alignmen)

[00 : 03]   resv0000
[04 : 23]   OBUF_RDPNT      Current output buffer read address for each channel pair. 
                            For stereo channel, this value is related to first channel.
[24 : 31]   resv0024
**************************************************************************************/
#define REG_ASRC_CH01_OBUF_RDPNT                        (0x70)
#define REG_ASRC_CH23_OBUF_RDPNT                        (0x74)
#define REG_ASRC_CH45_OBUF_RDPNT                        (0x78)
#define REG_ASRC_CH67_OBUF_RDPNT                        (0x7C)
#define REG_ASRC_CH89_OBUF_RDPNT                        (0xBC)
#define REG_ASRC_CH1011_OBUF_RDPNT                      (0x110)

#define RegAsrc_OBufRdpnt_R(rMap, idx)                  AUDREG_BITS_R( rMap.OBufRp[idx], 0, 24)
#define RegAsrc_OBufRdpnt_W(rMap, idx, val)             AUDREG_BITS_W(rMap.OBufRp[idx], 0, 24, val)


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
#define REG_ASRC_IBUF_INTR_CNT0                         (0x80)
#define REG_ASRC_IBUF_INTR_CNT1                         (0x84)
#define REG_ASRC_IBUF_INTR_CNT2                         (0xC0)

#define RegAsrc_IBufIntrCnt_R(rMap, idx)                AUDREG_BITS_R( rMap.IBufIntrCnt[idx/2], (8 + 8*(idx%2)), 8)
#define RegAsrc_IBufIntrCnt_W(rMap, idx, val)           AUDREG_BITS_W(rMap.IBufIntrCnt[idx/2], (8 + 8*(idx%2)), 8, val)


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
#define REG_ASRC_OBUF_INTR_CNT0                         (0x88)
#define REG_ASRC_OBUF_INTR_CNT1                         (0x8C)
#define REG_ASRC_OBUF_INTR_CNT2                         (0xC4)

#define RegAsrc_OBufIntrCnt_R(rMap, idx)                AUDREG_BITS_R( rMap.OBufIntrCnt[idx/2], (8 + 8*(idx%2)), 8)
#define RegAsrc_OBufIntrCnt_W(rMap, idx, val)           AUDREG_BITS_W(rMap.OBufIntrCnt[idx/2], (8 + 8*(idx%2)), 8, val)


/******************************************************************************
ASRC_BAK  (0x90 => 3'b0)
[00 : 02]   RESULT_SEL          Output Selection
                                000: ASRC output
                                001: Data input
                                010: CMPF output
                                011: HBF1 output
                                100: HBF2 output
                                101: HBF3 output
                                110: HBF4 output
                                111: ASRC output
[03 : 31]   resv0003
******************************************************************************/
#define REG_ASRC_BAK                                    (0x90)

#define RegAsrc_Bak_ResultSel_R(rMap)                   AUDREG_BITS_R( rMap.BAK, 0, 3)
#define RegAsrc_Bak_ResultSel_W(rMap, val)              AUDREG_BITS_W(rMap.BAK, 0, 3, val)


/****************************************************************************************
ASRC_FREQ_CALI_CTRL  

[00 : 07]   resv0000
[08]        CALI_EN                 Set 1 to enable frequency clibrator.
                                    if auto restart is 0, this bit will be clear while one 
                                    clibration run is complete.
[09]        FS2_UPDATE              Update FS2 use frquency result.
                                    0: use period result             1: use frequency result
[10]        AUTO_RESTRT             Auto restart.
                                    set 1 make the calibrator auto restart new calibration.
[11]        AUTO_FS2_UPDATE         Set 1 to enable ASRC_FREQUENCY_2 auto update with the calibrator
                                    result once the calibrator complete one round.
[12 : 14]   PSLT_LSHFT              The Left shift amount for calibrator result auto load to 
                                    ASRC_FREQUENCY_2
[15]        CALIBRATOR_BYPASS       Set 1 to bypass the deglitch circuit for calibrator input
[16 : 17]   SRC_SEL                 The calibrator input selection register.
                                    00: use lrc                    01: use lrck_dec
                                    10: use splin_lrck          11: use dec2_splin_lrck
[18]        CLK_SEL                 Decide the calibrator reference clock.
                                    0: use apll;     1: use dsp clk;
[19]        COMP_FREQ_RES_EN        Frequency compenstation enabling register.                          
[20]        CAL_BUSY                The Frequency calculation is busy.
                                    For the calibration for frequency, the bit 8 and this bit become 0 
                                    means the frequency result on 0x5a8 is avaliable
[21 : 31]   resv0021

----------------------------------------------------------------------------------------
REG_ASRC_FREQ_CALI2_CTRL

[09]        FS3_UPDATE 
[11]        AUTO_FS3_UPDATE
****************************************************************************************/
#define REG_ASRC_FREQ_CALI_CTRL                                 (0x94)
#define REG_ASRC_FREQ_CALI2_CTRL                                (0x134)

#define RegAsrc_FreqCaliCtrl_CaliEn_R(rMap, idx)                AUDREG_BITS_R( rMap.FreqCaliCtrl[idx], 8, 1)
#define RegAsrc_FreqCaliCtrl_CaliEn_W(rMap, idx, val)           AUDREG_BITS_W(rMap.FreqCaliCtrl[idx], 8, 1, val)

#define RegAsrc_FreqCaliCtrl_Fs2Update_R(rMap, idx)             AUDREG_BITS_R( rMap.FreqCaliCtrl[idx], 9, 1)
#define RegAsrc_FreqCaliCtrl_Fs2Update_W(rMap, idx, val)        AUDREG_BITS_W(rMap.FreqCaliCtrl[idx], 9, 1, val)

#define RegAsrc_FreqCaliCtrl_AutoRestrt_R(rMap, idx)            AUDREG_BITS_R( rMap.FreqCaliCtrl[idx], 10, 1)
#define RegAsrc_FreqCaliCtrl_AutoRestrt_W(rMap, idx, val)       AUDREG_BITS_W(rMap.FreqCaliCtrl[idx], 10, 1, val)

#define RegAsrc_FreqCaliCtrl_AutoFs2Update_R(rMap, idx)         AUDREG_BITS_R( rMap.FreqCaliCtrl[idx], 11, 1)
#define RegAsrc_FreqCaliCtrl_AutoFs2Update_W(rMap, idx, val)    AUDREG_BITS_W(rMap.FreqCaliCtrl[idx], 11, 1, val)

#define RegAsrc_FreqCaliCtrl_PsltLShft_R(rMap, idx)             AUDREG_BITS_R( rMap.FreqCaliCtrl[idx], 12, 3)
#define RegAsrc_FreqCaliCtrl_PsltLShft_W(rMap, idx, val)        AUDREG_BITS_W(rMap.FreqCaliCtrl[idx], 12, 3, val)

#define RegAsrc_FreqCaliCtrl_CaliByPass_R(rMap, idx)            AUDREG_BITS_R( rMap.FreqCaliCtrl[idx], 15, 1)
#define RegAsrc_FreqCaliCtrl_CaliByPass_W(rMap, idx, val)       AUDREG_BITS_W(rMap.FreqCaliCtrl[idx], 15, 1, val)

#define RegAsrc_FreqCaliCtrl_SrcSel_R(rMap, idx)                AUDREG_BITS_R( rMap.FreqCaliCtrl[idx], 16, 2)
#define RegAsrc_FreqCaliCtrl_SrcSel_W(rMap, idx, val)           AUDREG_BITS_W(rMap.FreqCaliCtrl[idx], 16, 2, val)

#define RegAsrc_FreqCaliCtrl_ClkSel_R(rMap, idx)                AUDREG_BITS_R( rMap.FreqCaliCtrl[idx], 18, 1)
#define RegAsrc_FreqCaliCtrl_ClkSel_W(rMap, idx, val)           AUDREG_BITS_W(rMap.FreqCaliCtrl[idx], 18, 1, val)

#define RegAsrc_FreqCaliCtrl_CompFreqResEn_R(rMap, idx)         AUDREG_BITS_R( rMap.FreqCaliCtrl[idx], 19, 1)
#define RegAsrc_FreqCaliCtrl_CompFreqResEn_W(rMap, idx, val)    AUDREG_BITS_W(rMap.FreqCaliCtrl[idx], 19, 1, val)

#define RegAsrc_FreqCaliCtrl_CalBusy_R(rMap, idx)               AUDREG_BITS_R( rMap.FreqCaliCtrl[idx], 20, 1)
#define RegAsrc_FreqCaliCtrl_CalBusy_W(rMap, idx, val)          AUDREG_BITS_W(rMap.FreqCaliCtrl[idx], 20, 1, val)


/******************************************************************************
ASRC_FREQ_CALI_CYC  

[00 : 07]   resv0000
[08 : 19]   ASRC_FREQ_CALI_CYC      Asrc Frequency Calibrator Input Cycle Register
                                    Define how many input signal cycles the calibrator 
                                    calibrates in one round.
[20 : 31]   resv0020
******************************************************************************/
#define REG_ASRC_FREQ_CALI_CYC                      (0x98)

#define RegAsrc_FreqCaliCyc_R(rMap)                 AUDREG_BITS_R( REG_ASRC_FREQ_CALI_CYC, 8, 12)
#define RegAsrc_FreqCaliCyc_W(rMap, val)            AUDREG_BITS_W(REG_ASRC_FREQ_CALI_CYC, 8, 12, val)


/******************************************************************************
ASRC_PRD_CALI_RSLT   (0x9C)

[00 : 23]   ASRC_PRD_CALI_RSLT      Asrc Period Calibrator Result
                                    Record the calibration result of previous round.
                                    Write any value to this register will clear the result.
[24 : 31]   resv0024
******************************************************************************/
#define REG_ASRC_PRD_CALI_RSLT                      (0x9C)
#define REG_ASRC_PRD_CALI2_RSLT                     (0x138)

#define RegAsrc_PrdCaliCtrl_R(rMap, idx)            AUDREG_BITS_R( rMap.PrdCaliRslt[idx], 0, 24)
#define RegAsrc_PrdCaliCtrl_W(rMap, idx, val)       AUDREG_BITS_W(rMap.PrdCaliRslt[idx], 0, 24, val)


/**************************************************************************************************
ASRC_FREQ_CALI_RSLT  

[00 : 23]   ASRC_FREQ_CALI_RSLT     Asrc Frequency Calibrator Result
                                    This value is translated by{FREQ_TRANS_NUMERTOR} / ASRC_PRD_CALI_RSLT.
                                    If the result more than 1, the result will be 24'hFFFFFF
[24 : 31]   resv0024
***************************************************************************************************/
#define REG_ASRC_FREQ_CALI_RSLT                     (0xA0)
#define REG_ASRC_FREQ_CALI2_RSLT                    (0x13C)

#define RegAsrc_FreqCaliCtrl_R(rMap, idx)           AUDREG_BITS_R( rMap.FreqCaliRslt[idx], 0, 24)
#define RegAsrc_FreqCaliCtrl_W(rMap, idx, val)      AUDREG_BITS_W(rMap.FreqCaliRslt[idx], 0, 24, val)


/**************************************************************************************************
ASRC_FREQ_TRANS_NUM   
[00 : 23]   ASRC_FREQ_TRANS_NUMERATOR    
                    Define the numerator for frequency calibrator to get frequncy result from period result.
                    The frequency result will be the 24-bit fractional part of 
                    FREQ_TRANS_NUMERATOR / ASRC_PRD_CALI_RSLT
                    If the result more than 1, the result will be 24'hFFFFFF.
[24 : 31]   resv0024
***************************************************************************************************/
#define REG_ASRC_FREQ_TRANS_NUM                     (0xD8)

#define RegAsrc_FreqTransNum_R(rMap)                AUDREG_BITS_R( rMap.FreqTransNum, 0, 24)
#define RegAsrc_FreqTransNum_W(rMap, val)           AUDREG_BITS_W(rMap.FreqTransNum, 0, 24, val)


/*****************************************************************************************
ASM_MAX_OUTPUT_PER_IN*      ASM Maximum Output amount Per Input for channel set
                            Tell asrc each CH-set translation information to prevent output
                            buffer full this value should be "cell(OFS/IFS)"
                            the ASRC support 8x up-sampling an ~16x down-sample.
[00 : 07]   resv0000
[08 : 11]   MAX_OUT_PER_IN0
[12 : 15]   MAX_OUT_PER_IN1
[16 : 19]   MAX_OUT_PER_IN2
[20 : 23]   MAX_OUT_PER_IN3
[24 : 31]   resv0024
*****************************************************************************************/
#define REG_ASRC_MAX_OUTPUT_PER_IN0                 (0xE0)
#define REG_ASRC_MAX_OUTPUT_PER_IN1                 (0xE4)

#define RegAsrc_MaxOutPerIn_R(rMap, idx)            AUDREG_BITS_R( rMap.MaxOutPerIn[idx/4], (8 + 4*(idx%4)), 4)
#define RegAsrc_MaxOutPerIn_W(rMap, idx, val)       AUDREG_BITS_W(rMap.MaxOutPerIn[idx/4], (8 + 4*(idx%4)), 4, val)


/**************************************************************************************************
ASM_IIR_CRAM_ADDR                   ASM IIR coefficient SRAM address
[00 : 07]
[08 : 15]   ASM_IIR_CRAM_ADDR       ASM IIR coefficient SRAM address
                                    Determine the read/write address of IIR coefficent SRAM, read/write 
                                    ASM_IIR_CRA_WDATA will make this register increase by 1
[16 : 31]   resv0016
***************************************************************************************************/
#define REG_ASRC_IIR_CRAM_ADDR                      (0xF0)

#define RegAsrc_IIRCramAddr_R(rMap, idx)            AUDREG_BITS_R( rMap.IIRCramAddr, 8, 8)
#define RegAsrc_IIRCramAddr_W(rMap, idx, val)       AUDREG_BITS_W(rMap.IIRCramAddr, 8, 8, val)


/**************************************************************************************************
ASM_IIR_CRAM_DATA                   ASM IIR coefficient SRAM ata
[00 : 23]   ASM_IIR_CRAM_DATA       ASM IIR coefficient SRAM Read/Write Data port
                                    read/write data port to cofficient sram
                                    read data have one cycle delay
                                    the cofficients are filled stage by stage, each stage has 6 coefficient.
                                    For the 2nd order IIR filter with transfer function of one stage
                                    (2^(shift))*a0+(2^(shift))*a1*Z^-1+(2^(shift))*a2*Z^-2/1+(2^(shift))*b1*Z^-1+(2^(shift))*b2*Z^-2
                                    The coefficient SRAM should filled from low addr to high with a2 a1 a0-b1-b2 shift
[24 : 31]   resv0024
***************************************************************************************************/
#define REG_ASRC_IIR_CRAM_DATA                      (0xF4)

#define RegAsrc_IIRCramData_R(rMap)                 AUDREG_BITS_R( rMap.IIR_CRAM_DATA, 0, 24)
#define RegAsrc_IIRCramData_W(rMap, val)            AUDREG_BITS_W(rMap.IIR_CRAM_DATA, 0, 24, val)


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
#define REG_ASRC_DMA_CFG                            (0xFC)

#define RegAsrc_DmaCfg_READ(rMap)                   AUDREG_READ( rMap.DmaCfg)
#define RegAsrc_DmaCfg_WRITE(rMap, val)             AUDREG_WRITE(rMap.DmaCfg, val)

#define RegAsrc_DmaCfg_BufSize_R(rMap)              AUDREG_BITS_R( rMap.DmaCfg, 0, 2)
#define RegAsrc_DmaCfg_BufSize_W(rMap, val)         AUDREG_BITS_W(rMap.DmaCfg, 0, 2, val)
    
#define RegAsrc_DmaCfg_DmaBufEmpty_R(rMap)          AUDREG_BITS_R( rMap.DmaCfg, 4, 1)
#define RegAsrc_DmaCfg_DmaBufEmpty_W(rMap, val)     AUDREG_BITS_W(rMap.DmaCfg, 4, 1, val)

#define RegAsrc_DmaCfg_DmaBufFull_R(rMap)           AUDREG_BITS_R( rMap.DmaCfg, 5, 1)
#define RegAsrc_DmaCfg_DmaBufFull_W(rMap, val)      AUDREG_BITS_W(rMap.DmaCfg, 5, 1, val)

#define RegAsrc_DmaCfg_LastCfg_R(rMap)              AUDREG_BITS_R( rMap.DmaCfg, 8, 1)
#define RegAsrc_DmaCfg_LastCfg_W(rMap, val)         AUDREG_BITS_W(rMap.DmaCfg, 8, 1, val)

#define RegAsrc_DmaCfg_Reset_R(rMap)                AUDREG_BITS_R( rMap.DmaCfg, 16, 1)
#define RegAsrc_DmaCfg_Reset_W(rMap, val)           AUDREG_BITS_W(rMap.DmaCfg, 16, 1, val)


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
#define REG_ASRC_IFS_OFS_SEL                        (0x124)

#define RegAsrc_FsSel_IFS_R(rMap, idx)              AUDREG_BITS_R( rMap.IFsOFsSel, (8 + idx*2), 1)  
#define RegAsrc_FsSel_IFS_W(rMap, idx, val)         AUDREG_BITS_W(rMap.IFsOFsSel, (8 + idx*2), 1, (val/4))

#define RegAsrc_FsSel_OFS_R(rMap, idx)              AUDREG_BITS_R( rMap.IFsOFsSel, (9 + idx*2), 1)
#define RegAsrc_FsSel_OFS_W(rMap, idx, val)         AUDREG_BITS_W(rMap.IFsOFsSel, (9 + idx*2), 1, (val/4))


//===================================================================================================//

/*****************************************************************************************
AUD_REG_RGBK2_CFG0          0xA8080
[00 : 04]   RBANK_RGB2                  register wxpand for maping to dsp address
[06]        GPS_ASRC_RESTB              gps asrc soft reset
[07]        GPS_APLL_SEL                claculation clock sel
[08 : 18]   GPS_AIN_DMA_ADR_HIGH        gps asrc waddr high bits        
[20 : 30]   GPS_ASM_RD_ADR_HIGH         gps asrc raddr high bits    
*****************************************************************************************/

/*****************************************************************************************
AUD_DRAM_BANK           0x53A0  
[00 : 10]   AFIFO_BANK        AFIFO bank (in 2M bytes unit)        
[12 : 22]   ADSP_BANK         audio dsp working area bank (in 2M bytes unit)     
*****************************************************************************************/

/*****************************************************************************************
AUD_REG_RGBK2_AOUT_CFG2     0xA80C8   
[00 : 07]   AP_ASRC_ARM_CTRL_CFG            'h36 enable arm contorl           
*****************************************************************************************/
   
// bit 0~2 set as 7
#define RegAsrc_Reset_Reg()                         AUDREG_WRITE(0xC4, AUDREG_READ(0xC4) | 7)        


#define ASRC_BANK_ADDR_MAX                          (1 << 11)

#define RegAsrc_BankAddr_Input_W(eAsrcType, val)           \
    if (eAsrcType == GPS_ASRC)  {\
        AUDREG_BITS_W(0xA8080, 8, 11, val); \
    } else { \
        AUDREG_BITS_W(0x53A0,  0, 11, val); \
    }

#define RegAsrc_BankAddr_Output_W(eAsrcType, val)           \
    if (eAsrcType == GPS_ASRC)  {\
        AUDREG_BITS_W(0xA8080, 20, 11, val);  \
    } 


#define RegAsrc_Soft_Reset(eAsrcType)           \
    if (eAsrcType != GPS_ASRC) { \
        AUDREG_BITS_W(0xA80C8, 0, 8, 0x36); \
    } \
    AUDREG_WRITE(0xA8080, AUDREG_READ(0xA8080) | 0x41); 

    
#define RegAsrc_OthersReg_Read()                   \
    ASRCLOG_INFO((T("    0xA8080 : Val: 0x%08x.\r\n"), (u32)(AUDREG_READ(0xA8080))));\
    ASRCLOG_INFO((T("    0xA80C8 : Val: 0x%08x.\r\n"), (u32)(AUDREG_READ(0xA80C8))));\
    ASRCLOG_INFO((T("    0x53A0  : Val: 0x%08x.\r\n"), (u32)(AUDREG_READ(0x53A0)))); 



#ifdef __cplusplus
}
#endif // __cplusplus


#endif  //_AUD_ASRC_REG_H_

