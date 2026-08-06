/********************************************************************************************
 *     LEGAL DISCLAIMER 
 *
 *     (Header of MediaTek Software/Firmware Release or Documentation)
 *
 *     BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES 
 *     THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE") RECEIVED 
 *     FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS 
 *     ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED, 
 *     INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR 
 *     A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY 
 *     WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, 
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK 
 *     ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
 *     NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION 
 *     OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *     
 *     BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE LIABILITY WITH 
 *     RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, 
 *     TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE 
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE. 
 *     
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS 
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.  
 ************************************************************************************************/
/******************************************************************************
* [File]			atc_nfiecc.h
* [Version]			v1.0
* [Revision Date]	2007-12-27
* [Author]			Meng-Chang Liu, mengchang.Liu@mediatek.com, 26615, 2007-12-27
* [Description]
*	AC83XX Download Agent NFI ECC include file
* [Copyright]
*	Copyright (C) 2007 MediaTek Incorporation. All Rights Reserved.
******************************************************************************/
#ifndef _ATC_NFIECC_H
#define _ATC_NFIECC_H
#include "atc_nfi.h"

//#include "x_typedef.h"
#ifdef USE_60BIT
#define NFIECC_BASE        0xf001E800
#else
#define NFIECC_BASE        0xf001EC00
#endif
#define NFIECC_ENCCON      ((volatile u16 *)(NFIECC_BASE+0x0000))
    #define ENC_EN      0x01
    
#define NFIECC_ENCCNFG  ((volatile u32 *)(NFIECC_BASE+0x0004))
#ifdef USE_60BIT
    #define ENC_TNUM(x)			(((u32)x>24)?(((u32)x/4)+4):(((u32)x/2)-2))
    #define ENC_NFI_MODE		0x01 << 5
#else
    #define ENC_TNUM(x)                ((((u32) x / 2) - 2))
    #define ENC_NFI_MODE             0x01 << 4
#endif
    #define ENC_MS(x)                    (((u32) x &0x3FFF) << 16)

#define NFIECC_ENCDIADDR      ((volatile u32 *)(NFIECC_BASE+0x0008))

#define NFIECC_ENCIDLE          ((volatile u16 *)(NFIECC_BASE+0x000C))
    #define ENC_IDLE                    0x01

#define NFIECC_DECCON      ((volatile u16 *)(NFIECC_BASE+0x0100))
    #define DEC_EN      0x01

#define NFIECC_DECCNFG  ((volatile u32 *)(NFIECC_BASE+0x0104))
#ifdef USE_60BIT
    #define DEC_TNUM(x)                (((u32)x>24)?(((u32)x/4)+4):(((u32)x/2)-2))
    #define DEC_NFI_MODE             0x01 << 5
#else
    #define DEC_TNUM(x)                (((((u32) x) / 2) - 2))
    #define DEC_NFI_MODE             0x01 << 4
#endif
    #define DEC_CON(x)                    (((u32) x &0x03) << 12)     
    #define DEC_CS(x)                    ((((u32) x )&0x3FFF) << 16)
    #define DEC_EMPTY_EN             0x80000000

#define NFIECC_DECDIADDR      ((volatile u32 *)(NFIECC_BASE+0x0108))

#define NFIECC_DECIDLE          ((volatile u16 *)(NFIECC_BASE+0x010C))
    #define DEC_IDLE                    0x01

#define NFIECC_DECFER            ((volatile u16 *)(NFIECC_BASE+0x0110))

#define NFIECC_DECENUM            ((volatile u32 *)(NFIECC_BASE+0x0150))
#define NFIECC_DECENUM2            ((volatile u32 *)(NFIECC_BASE+0x0154))
#ifdef USE_60BIT
    #define DECENUM_MASK     0x3F
#else
    #define DECENUM_MASK	 0x1F
#endif

#define NFIECC_DECDONE        ((volatile u16 *)(NFIECC_BASE+0x0118))

#define NFIECC_DECEL0           ((volatile u32 *)(NFIECC_BASE+0x160))

#define NFIECC_DECIRQEN       ((volatile u16 *)(NFIECC_BASE+0x0134))
    #define DEC_IRQEN                0x01

#define NFIECC_DECIRQSTA     ((volatile u16 *)(NFIECC_BASE+0x0138))
    #define DEC_IRQSTA                0x01

#define NFIECC_FDMADDR        ((volatile u32 *)(NFIECC_BASE+0x013C))

#define NFIECC_FDMSTA        ((volatile u32 *)(NFIECC_BASE+0x0140))


#define NFIECC_DECNFIDI ((volatile u32 *)(NFIECC_BASE+0x0148))

typedef enum {
   ECC_4_BITS = 4,
   ECC_6_BITS = 6,
   ECC_8_BITS = 8,
   ECC_10_BITS = 10,
   ECC_12_BITS = 12,
   ECC_22_BITS = 22,
   ECC_24_BITS = 24,
#ifdef USE_60BIT
   ECC_28_BITS = 28,
   ECC_32_BITS = 32,
   ECC_36_BITS = 36,
   ECC_40_BITS = 40,
   ECC_44_BITS = 44,
   ECC_48_BITS = 48,
   ECC_52_BITS = 52,
   ECC_56_BITS = 56,
   ECC_60_BITS = 60
#endif
} ECC_Level_t;

typedef enum {
   ECC_DEC_NONE,
   ECC_DEC_DETECT,
   ECC_DEC_LOCATE,
   ECC_DEC_CORRECT
} ECC_Decode_Type_t;

#endif //_ATC_NFIECC_H
