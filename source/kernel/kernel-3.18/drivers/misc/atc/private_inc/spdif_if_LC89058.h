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

#ifndef _SPDIF_IF_LC89058_H_
#define _SPDIF_IF_LC89058_H_

//#include "drv_aud_cfg.h"
#include "x_audin.h"
#include "drv_config.h"


// *********************************************************************
// SPDIF-IN DIR definitions
// *********************************************************************
// Write Data for LC89058

#define LC89058_SYS_SETTING_1           0x00
  //
  #define SYSRST                        (0x1<< 0)
  #define DOEN_HIZ                      (0x1<< 1)  // High impedence state
  #define DOEN_OUT                      (0x0<< 1)  // DO output state
  #define TESTM                         (0x1<< 1)  // Test mode

#define LC89058_SYS_SETTING_2           0x01
  //
  #define MOSEL                         (0x01<< 0)
  #define AOSEL                         (0x01<< 2)
  #define RXMON                         (0x01<< 3)
  #define FSLIM_NOLMT                   (0x00<< 4)
  #define FSLIM_96K                     (0x01<< 4)
  #define FSLIM_48K                     (0x02<< 4)

#define LC89058_MASTER_CLK_SETTING      0x02
  //
  #define XINSEL                        (0x01<< 1)  // 1: 24.576MHz ;  0: 12.288MHZ
  #define XMSEL_MUTE                    (0x03<< 2)
  #define PLLOPR_OPR                    (0x00<< 4) // Operate
  #define PLLOPR_STP                    (0x01<< 4) // Stop
  #define AMPOPR_PRMNT                  (0x00<< 6) // Permanent continuous operation

#define LC89058_R_SYS_CLK_SETTING       0x03
  //
  #define PRSEL_256FS                   (0x00<< 0)
  #define XRSEL_1X_XIN                  (0x00<< 2)
  #define XRBCK_3M_XIN                  (0x00<< 4)
  #define XRLRCK_48K_XIN                (0x00<< 6)

#define LC89058_S_SYS_CLK_SETTING       0x04
  //
  #define PSBCK_64FS                    (0x00<< 0)
  #define PSLRCK_FS                     (0x00<< 2)
  #define XSBCK_3M_XIN                  (0x00<< 4)
  #define XSLRCK_48K_XIN                (0x00<< 6)

#define LC89058_RDATA_OUT_SETTING       0x05
  //
  #define OCKSEL_0                      (0x00<< 1) // Use XIN clock as source while PLL is unlocked
  #define OCKSEL_1                      (0x01<< 0) // Use XIN clock as source regardless off PLL
  #define RDTSEL_MUTE                   (0x01<< 4) // Mute while PLL is unlocked
  #define RDTSTA_RDTSEL                 (0x00<< 5) // According to RDTSEL
  #define RDTMUT_RDTSEL                 (0x00<< 6) // According to RDTSEL

#define LC89058_DATA_IO_SETTING         0x06
  // Data demodulation input pin selection
  #define RISEL_RX0                     (0x00<< 0)
  #define RISEL_RX1                     (0x01<< 0)
  #define RISEL_RX2                     (0x02<< 0)
  #define RISEL_RX3                     (0x03<< 0)
  #define RISEL_RX4                     (0x04<< 0)
  #define RISEL_RX5                     (0x05<< 0)
  #define RISEL_RX6                     (0x06<< 0)
  #define RISEL_NONE                    (0x07<< 0)
  // RXOUT1 output data Settings
  #define RXOUT1_RX0                    (0x00<< 4)
  #define RXOUT1_RX1                    (0x01<< 4)
  #define RXOUT1_RX2                    (0x02<< 4)
  #define RXOUT1_RX3                    (0x03<< 4)
  #define RXOUT1_RX4                    (0x04<< 4)
  #define RXOUT1_RX5                    (0x05<< 4)
  #define RXOUT1_RX6                    (0x06<< 4)
  #define RXOUT1_L                      (0x07<< 4)

#define LC89058_DATA_OUT_FMT_SETTING    0x07
  //
  #define OFSEL_I2S                     (0x00<< 0) // I2S data output
  #define OFSEL_LJ                      (0x01<< 0) // Left-Justified 24bits MSB first data output
  #define RBCKP_FALL                    (0x00<< 4) // Falling RDATA data change
  #define RBCKP_RISE                    (0x01<< 4) // Rising RDATA data change
  #define RLRCKP_I2S                    (0x00<< 5) // RLRCK polarity for I2S
  #define RLRCKP_LJ                     (0x01<< 5) // RLRCK polarity for LJ
  #define SBCKP_FALL                    (0x00<< 6) // Falling RDATA data change
  #define SBCKP_RISE                    (0x01<< 6) // Rising RDATA data change
  #define SLRCKP_I2S                    (0x00<< 7) // SLRCK polarity for I2S
  #define SLRCKP_LJ                     (0x01<< 7) // SLRCK polarity for LJ

#define LC89058_INT_OUT_SETTING         0x08
  //
//  #define ERROR                       (0x01<< 0) // Output RERR pin status change
  #define INDET                         (0x01<< 1) // Output input data pin status change
  #define FSCHG                         (0x01<< 2) // Output updated flag of PLL lock freq. calculation result
  #define CSRNW                         (0x01<< 3) //
  #define UNPCM                         (0x01<< 4) //
  #define PCRNW                         (0x01<< 5) //
  #define GPIO                          (0x01<< 6) //
  #define EMPF                          (0x01<< 7) //

#define LC89058_RERR_OUT_SETTING        0x09
  //
  #define RESEL                         (0x00<< 0) // PLL lock error or data error
  #define REDER                         (0x00<< 1) // Output only when non-PCM data is recognized
  #define RESTA                         (0x00<< 4) // Output PLL status all the time
  #define FSERR                         (0x00<< 5) // Reflect fs changes to error flag
  #define ERWT_3                        (0x00<< 6) // Cancel error after preamble B is couned 3

#define LC89058_GPIO                    0x0A
  //
  #define PI0_L                         (0x00<< 4) //
  #define PI0_H                         (0x01<< 4) //
  #define PI1_L                         (0x00<< 5) //
  #define PI1_H                         (0x01<< 5) //
  #define PI2_L                         (0x00<< 6) //
  #define PI2_H                         (0x01<< 6) //
  #define PI3_L                         (0x00<< 7) //
  #define PI3_H                         (0x01<< 8) //

#define LC89058_SYS_SETTING_3           0x0C
  //
  #define PLLACC                        (0x01<< 0) // PLL clock lock freq setting - Automatic control
  #define PLLDV0_48K_512FS              (0x00<< 1) // suggeste (With PLLACC=1)
  #define PLLDV0_48K_256FS              (0x01<< 1)
  #define PLLDV1_96K_256FS              (0x00<< 2) // suggested (With PLLACC=1)
  #define PLLDV1_96K_512FS              (0x01<< 2)
  #define RMCKP_NRM                     (0x00<< 4) // DIR block RMCK output setting - Normal output
  #define RMCKP_INV                     (0x01<< 4)  // DIR block RMCK output setting - Inverted output
  #define CKST_HI                       (0x00<< 5) // CKST output polarity setting - Normal high output
  #define CKST_LO                       (0x01<< 5) // CKST output polarity setting - Normal low output


#define LC89058_DATA_IO_SETTING_2       0x0D
  // Settings of pin44~47 (GPIO0~3)
  #define GPIOS_GP                      (0x00<< 0) // General-Purpose I/O paralel input
  #define GPIOS_SI                      (0x01<< 0) // Selector input
  #define EXTSEL_DATA                   (0x00<< 1) // Output data and clock of DIR function
  #define EXTSEL_INPUT                  (0x01<< 1) // Output input signals (With GPIOS = 1)
  #define EMCKP_NRM                     (0x00<< 2) // GPIO0 output polarity setting - Normal output
  #define EMCKP_INV                     (0x01<< 2) // GPIO0 output polarity setting - Inverted output
  #define EDTMUT_NRM                    (0x00<< 3) // GPIO3 mute setting - Normal output
  #define EDTMUT_MUTE                   (0x01<< 3) // GPIO3 mute setting - Muted
  // RXOUT2 output data Settings
  #define RXOUT2_L                      (0x00<< 4)
  #define RXOUT2_RX0                    (0x01<< 4)
  #define RXOUT2_RX1                    (0x02<< 4)
  #define RXOUT2_RX2                    (0x03<< 4)
  #define RXOUT2_RX3                    (0x04<< 4)
  #define RXOUT2_RX4                    (0x05<< 4)
  #define RXOUT2_RX5                    (0x06<< 4)
  #define RXOUT2_RX6                    (0x07<< 4)

#define LC89058_PLL_CLK                 0x0E
  // Setting of clock switch wait time
  #define PTOXW                         (0x00<< 2) // Clock switching after 2.67ms from when the PLL clock lock status is identified
  // MOUT output contents setting (Ouput "L" when PLL unlock or when a value other than below is calculated)
  #define FSSEL_48K                     (0x00<< 6) // Output "H" when 32/44.1/48 KHz is calculated
  #define FSSEL_96K                     (0x01<< 6) // Output "H" when 64/88.2/96 KHz is calculated
  #define FSSEL_192K                    (0x02<< 6) // Output "H" when 128/176.4/192 KHz is calculated
  #define FSSEL_96K_H                   (0x03<< 6) // Output "H" when 64/88.2/96 KHz or higher is calculated

// Read Data for LC89058
#define LC89058_RXDET_INT               0xEA
 #define INPUT_DETCT_PART               0
 #define INT_SRC_PART                   1
 #define CH_STATUS                      2

  #define OERROR                        (0x01<< 0) // Output RERR pin status change
  #define OINDET                        (0x01<< 1) // Output input data pin status change
  #define OFSCHG                        (0x01<< 2) // Output updated flag of PLL lock freq. calculation result
  #define OCSRNW                        (0x01<< 3) //
  #define OUNPCM                        (0x01<< 4) //
  #define OPCRNW                        (0x01<< 5) //
  #define OGPIO                         (0x01<< 6) //
  #define OEMPF                         (0x01<< 7) //

  #define CSBIT1                        (0x01<< 0) //

#define LC89058_PO_FSC                  0xEB
 #define FSC_PART                       0

  #define FSC_MASK                      0xf0
  #define FSC_OUT_RANGE                 0x06
  #define FSC_32K                       0x07
  #define FSC_44K                       0x08
  #define FSC_48K                       0x09
  #define FSC_64K                       0x0A
  #define FSC_88K                       0x0B
  #define FSC_96K                       0x0C
  #define FSC_128K                      0x0D
  #define FSC_176K                      0x0E
  #define FSC_192K                      0x0F


#define LC89058_CS                      0xEC

#define LC89058_PC                      0xED
 #define DATA_TYPE_PART                 0


// *********************************************************************
// Export API
// *********************************************************************
// SPDIF-in/Line ine relative

extern void vLC89058Write(UINT8 ui1CmdAddr, UINT8 ui1Value);
extern UINT8 vLC89058Read(UINT8 ui1CCBAddr, UINT8 ui1ReadOutPart);
extern UINT8 u1SpdifDetSmpRate(void);

extern void vExtSPDIFIn_IRQHandler(UINT16 u2Vector);
extern void vExtSPDIFInIRQEnable(BOOL fgEnable);
extern void vExtSPDIFInIRQEnableByFunc(void);



#endif  //_SPDIF_IF_H_

