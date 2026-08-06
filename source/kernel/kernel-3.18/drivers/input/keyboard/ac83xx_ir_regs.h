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

#ifndef __IR_REGS_H__
#define __IR_REGS_H__

#include <mach/base_regs.h>
#include <mach/ac83xx_basic.h>
#include <linux/kernel.h>


#define ZONE_FUNCTION   0
#define ZONE_WARN       0
#define ZONE_ERROR      1

#define IO_BASE_VA         IO_UCV_BASE

/**************************************************
    IR protocol & user define
    ************************************************/
#define CONFIG_ARM2_EJECT  0
#define IRRX_USE_27M  1

#define IRRX_RC_NEC  (0x00)
#define IRRX_RC_RC6  (0x01)
#define IRRX_RC_RC5  (0x02)
#define IRRX_RC_PAN  (0X03)
#define IRRX_RC_SIRC (0x04)
#define IRRX_RC_JVC  (0x05) 


//(IRRX_RC_PROTOCOL == IRRX_RC_RC6)

#if (IRRX_USE_27M)
#define IRRX_SAPERIOD_RC6 (unsigned int)(0x2e)
#else
#define IRRX_SAPERIOD_RC6 (unsigned int)(0xA6)
#endif
#define IRRX_RC6_CONFIG   (IRRX_CH_END_15 | IRRX_CH_IGSYN | IRRX_CH_HWIR | IRRX_CH_ORDINV | IRRX_CH_RC5)
#define IRRX_RC6_BITCNT   (unsigned int)(0x1e)
#define IRRX_RC6_LEADER   (unsigned int)(0x8)
#define IRRX_RC6_TOGGLE0  (unsigned int)(0x1)
#define IRRX_RC6_TOGGLE1  (unsigned int)(0x2)
#define IRRX_RC6_CUSTOM (unsigned int)(0x32)

#define IRRX_RC6_GET_LEADER(bdata0) ((bdata0>>4))
#define IRRX_RC6_GET_TOGGLE(bdata0) ((bdata0 & 0xc)>>2)
#define IRRX_RC6_GET_CUSTOM(bdata0,bdata1) (((bdata0 & 0x3) << 6) |bdata1 >> 2)
#define IRRX_RC6_GET_KEYCODE(bdata1,bdata2)  \
                (((bdata2>>2) | ((bdata1 & 0x3)<<6)) & 0xff)

#define IRRX_RC6_MAX_MAP_ENTRY   (0xFF)

//(IRRX_RC_PROTOCOL == IRRX_RC_SIRC)

#if (IRRX_USE_27M)
#define IRRX_SAPERIOD_SIRC (unsigned int)(0x30)  
#else
#define IRRX_SAPERIOD_SIRC (unsigned int)(0xAB)
#endif
#define IRRX_SIRC_CONFIG   (IRRX_CH_END_15 | IRRX_CH_ORDINV | IRRX_CH_HWIR)
#define IRRX_SIRC_BITCNT12 (unsigned int)(0xc)
#define IRRX_SIRC_12B_DEVICE    ((unsigned char)(0x01))

#define IRRX_SIRC_BITCNT15 (unsigned int)(0xf)
#define IRRX_SIRC_15B_DEVICE    ((unsigned char)(0x01))

#define IRRX_SIRC_BITCNT20 (unsigned int)(0x14)
#define IRRX_SIRC_20B_DEVICE    ((unsigned char)(0x1A))
#define IRRX_SIRC_20B_EXTENDED  ((unsigned char)(0xE2))

#define IRRX_SIRC_MAX_MAP_ENTRY   (0x80)

//(IRRX_RC_PROTOCOL == IRRX_RC_NEC)

//NEC config
#define ATC_IRRX_CONFIG         (IRRX_CH_END_15 + IRRX_CH_IGSYN + IRRX_CH_HWIR)
#if (IRRX_USE_27M)
//NEC sampling period 0x32 = 560us/9.5us
#define ATC_IRRX_SAPERIOD       (0x0032)
#else
#define ATC_IRRX_SAPERIOD       (0x00B4)
#endif
#define ATC_NEC_MAX_MAP_ENTRY   (0xFF)
//#define ATC_NEC_MAX_MAP_ENTRY   (0x60)

#define ATC_IRRX_1st_Plus_REPEAT  (3)
#define ATC_IRRX_BITCNT_NORMAL    (33)
#define ATC_IRRX_BITCNT_REPEAT    (1)
#define ATC_IRRX_BIT8_VERIFY      (0xff)

#define ATC_IRRX_GRPID_DVD    (0xff00)

//(IRRX_RC_PROTOCOL == IRRX_RC_JVC)

#if (IRRX_USE_27M)
#define IRRX_SAPERIOD_JVC (unsigned int)(0x30)
#else
#define IRRX_SAPERIOD_JVC (unsigned int)(0xB0)
#endif

#define IRRX_JVC_CONFIG            (IRRX_CH_END_15 + IRRX_CH_HWIR)

#define IRRX_JVC_BITCNT_NORMAL     (unsigned int)(0x11)
#define IRRX_JVC_BITCNT_REPEAT     (unsigned int)(0x10)
#define IRRX_JVC_1ST_PULSE_NORMAL  (unsigned int)(0x8)
#define IRRX_JVC_1ST_PULSE_REPEAT  (unsigned int)(0x2)
#define IRRX_JVC_CUSTOM            (unsigned int)(0xEF)

#define IRRX_JVC_MAX_MAP_ENTRY   (0xFF)

//(IRRX_RC_PROTOCOL == IRRX_RC_PAN)

#if (IRRX_USE_27M)
#define IRRX_SAPERIOD_PAN (unsigned int)(0x26)
#else
#define IRRX_SAPERIOD_PAN (unsigned int)(0x8A)
#endif

#define IRRX_PAN_CONFIG            (IRRX_CH_END_15 + IRRX_CH_IGSYN +  +IRRX_CH_HWIR)

#define IRRX_PAN_BITCNT_NORMAL     (unsigned int)(0x31)
#define IRRX_PAN_BITCNT_REPEAT     (unsigned int)(0x10)
#define IRRX_PAN_1ST_PULSE_NORMAL  (unsigned int)(0x8)
#define IRRX_PAN_1ST_PULSE_REPEAT  (unsigned int)(0x2)
#define IRRX_PAN_CUSTOM            (unsigned int)(0x2002)

#define IRRX_PAN_MAX_MAP_ENTRY   (0xFF)

//(IRRX_RC_PROTOCOL == IRRX_RC_RC5)

#if (IRRX_USE_27M)
#define IRRX_SAPERIOD_RC5 (unsigned int)(0x60)
#else
#define IRRX_SAPERIOD_RC5 (unsigned int)(0xB0)
#endif
#define IRRX_RC5_CONFIG   (IRRX_CH_IGSYN | IRRX_CH_HWIR | IRRX_CH_ORDINV | IRRX_CH_RC5)

#define IRRX_RC5_BITCNT   (unsigned int)(0x10)
#define IRRX_RC5_CUSTOM   (unsigned int)(0x00)

#define IRRX_RC5_GET_TOGGLE(bdata0) (((~(bdata0)) & 0x80) >> 7)
#define IRRX_RC5_GET_CUSTOM(bdata0) (((~(bdata0)) & 0x7C) >> 2)
#define IRRX_RC5_GET_KEYCODE(bdata0,bdata1)  \
                ((((~(bdata0)) & 0x03) << 4) | (((~(bdata1)) & 0xF0) >> 4))

#define IRRX_RC5_MAX_MAP_ENTRY   (0x40)



//set deglitch with the max number.
#define ATC_IRRX_THRESHOLD      (0x0201)    

/* if there is no key in 400ms, poll function will get a 0xffffffff key. */
#define ATC_IRRX_TIMESLICE      (400)   
#define ATC_IRRX_PRIORITY       (100)
#define ATC_IRRX_ITEMCNT        (16)

#define MAX_IRRX_DATA        (4)

/**************************************************
    IR clock select
    ************************************************/
#define IRRX_UP_CFG              0x188
  #define FAST_CK_EN           ((unsigned int)0x1<<28)//1: 27M; 0:3M

#define IRRX_CLKPDN              0x040
#define IRRXPD                 (0x01<<1) //IR receiver module clock stop
#define IRDECODERPD            (0x1<<2)
  
#define IRRX_IRCKSEL             0x044
#define IRCLKSEL_MASK          0xf

#define CLK_SEL_IR_DIV_1_1     0x0 
#define CLK_SEL_IR_DIV_1_2     0x1
#define CLK_SEL_IR_DIV_1_4     0x2
#define CLK_SEL_IR_DIV_1_8     0x3
#define CLK_SEL_IR_DIV_1_16    0x4
#define CLK_SEL_IR_DIV_1_32    0x5
#define CLK_SEL_IR_DIV_1_64    0x6
#define CLK_SEL_IR_DIV_1_128   0x7
#define CLK_SEL_IR_DIV_1_256   0x8

/**************************************************
  IR interrupt , level 2 (level 1 is VECTOR_PWDNC)
  ************************************************/
#define IRRX_INTSTA             0x140
#define IR_INT                (0x01<<13)

#define IRRX_INTEN              0x144
#define IR_INTEN              (0x01<<13)

#define IRRX_INTCLR             0x148
#define IR_INTCLR             (0x01<<13)

/**************************************************
    IRRX register define
    ************************************************/
#define IRRX_COUNT_HIGH_REG        0x200  
#define IRRX_CH_BITCNT_MASK         0x0000003f
#define IRRX_CH_BITCNT_BITSFT       0
#define IRRX_CH_1ST_PULSE_MASK      0x0000ff00
#define IRRX_CH_1ST_PULSE_BITSFT    8
#define IRRX_CH_2ND_PULSE_MASK      0x00ff0000
#define IRRX_CH_2ND_PULSE_BITSFT    16
#define IRRX_CH_3RD_PULSE_MASK      0xff000000
#define IRRX_CH_3RD_PULSE_BITSFT    24
  
#define IRRX_COUNT_MID_REG         0x204
#define IRRX_COUNT_LOW_REG         0x208

#define IRRX_CONFIG_HIGH_REG     0x20c
#define IRRX_CH_DISPD        ((unsigned int)(1 << 15)) 
#define IRRX_CH_IGB0         ((unsigned int)(1 << 14))
#define IRRX_CH_END_7        ((unsigned int)(0x00 << 8))
#define IRRX_CH_END_15       ((unsigned int)(0x0F << 16))
#define IRRX_CH_END_23       ((unsigned int)(0x17 << 8))
#define IRRX_CH_END_31       ((unsigned int)(0x1f << 16))
#define IRRX_CH_END_39       ((unsigned int)(0x27 << 16))
#define IRRX_CH_END_47       ((unsigned int)(0x2f << 16))
#define IRRX_CH_END_55       ((unsigned int)(0x37 << 16))
#define IRRX_CH_END_63       ((unsigned int)(0x3f << 16))
#define IRRX_CH_DISCH        ((unsigned int)(1 << 7))
#define IRRX_CH_DISCL        ((unsigned int)(1 << 6))
#define IRRX_CH_IGSYN        ((unsigned int)(1 << 5))
#define IRRX_CH_ORDINV       ((unsigned int)(1 << 4))
#define IRRX_CH_RC5_1ST      ((unsigned int)(1 << 3))
#define IRRX_CH_RC5          ((unsigned int)(1 << 2))
#define IRRX_CH_IRI          ((unsigned int)(1 << 1))
#define IRRX_CH_HWIR         ((unsigned int)(1 << 0))

#define IRRX_CONFIG_LOW_REG       (0x210)
#define IRRX_THRESHOLD_REG        (0x214)
#define IRRX_GD_DEL_MASK      (0x0300)
#define IRRX_ICLR             (0x80)
#define IRRX_THRESHOLD_MASK   (0x7f)

#define IRRX_IRCLR                (0x218)
#define IRCLR                 (0x1)

#define IRRX_WAKEN             0x080
  #define IR_WAKEN             (0x1<<8)
  
#define IRRX_PDSTAT            0x088
  #define IR_IR_WAK            (0x1<<8)

#define IRRX_PDSTCLR            0x08C

#define IRRX_IREXP_EN           ((unsigned int)(0x240))
#define PD_IREXPEN_IR0        (((unsigned int)(1))<<10)
#define PD_IREXPEN_IR1        (((unsigned int)(1))<<11)
#define PD_IRPDWN_EN          (((unsigned int)(1))<<9)
#define WU_BCEPEN             (((unsigned int)(1))<<8)
#define WU_IREXPEN_IR0        (((unsigned int)(1))<<0)
#define WU_IREXPEN_IR1        (((unsigned int)(1))<<1)
#define WU_IREXPEN_IR2        (((unsigned int)(1))<<2)
#define WU_IREXPEN_IR3        (((unsigned int)(1))<<3)
#define WU_IREXPEN_IR4        (((unsigned int)(1))<<4)
#define WU_IREXPEN_IR5        (((unsigned int)(1))<<5)
#define WU_IREXPEN_IR6        (((unsigned int)(1))<<6)
#define WU_IREXPEN_IR7        (((unsigned int)(1))<<7)

#define IRRX_ENEXP_IRM          ((unsigned int)(0x244))
#define IRRX_ENEXP_IRL          ((unsigned int)(0x248))

#define IRRX_EXP_BCNT           ((unsigned int)(0x24C))

#define IRRX_PDWNCNT            ((unsigned int)(0x250))

#define IRRX_EXP_IRM0          ((unsigned int)(0x280))
#define IRRX_EXP_IRL0          ((unsigned int)(0x284))
#define IRRX_EXP_IRM1          ((unsigned int)(0x288))
#define IRRX_EXP_IRL1          ((unsigned int)(0x28C))
#define IRRX_EXP_IRM2          ((unsigned int)(0x290))
#define IRRX_EXP_IRL2          ((unsigned int)(0x294))
#define IRRX_EXP_IRM3          ((unsigned int)(0x298))
#define IRRX_EXP_IRL3          ((unsigned int)(0x29C))
#define IRRX_EXP_IRM4          ((unsigned int)(0x2A0))
#define IRRX_EXP_IRL4          ((unsigned int)(0x2A4))
#define IRRX_EXP_IRM5          ((unsigned int)(0x2A8))
#define IRRX_EXP_IRL5          ((unsigned int)(0x2AC))
#define IRRX_EXP_IRM6          ((unsigned int)(0x2B0))
#define IRRX_EXP_IRL6          ((unsigned int)(0x2B4))
#define IRRX_EXP_IRM7          ((unsigned int)(0x2B8))
#define IRRX_EXP_IRL7          ((unsigned int)(0x2BC))
#define IRRX_EXPD_IRM0          ((unsigned int)(0x2C0))
#define IRRX_EXPD_IRL0          ((unsigned int)(0x2C4))
#define IRRX_EXPD_IRM1          ((unsigned int)(0x2C8))
#define IRRX_EXPD_IRL1          ((unsigned int)(0x2CC))

/**************************************************
    IR device register read/write macros
    ************************************************/
#define IR_REGISTER_BASE         (IO_BASE_VA+ 0x24000)
#define IR_BASE                  (IO_BASE_VA+ 0x24000)

#define IR_WRITE32(i4Addr, u4Val)  IO_WRITE32(IR_REGISTER_BASE, i4Addr, u4Val)   
#define IR_READ32(i4Addr)          IO_READ32(IR_REGISTER_BASE, i4Addr)

#endif /* __IRRX_VRF_HW_H__ */

 

