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

#ifndef _EEPROM_H_
#define _EEPROM_H_

/* #include "x_gpio.h" kenny temply mark */
#include "x_hal_ic.h"
#include "edid_data.h"
#if 0
#define DDC_CLK_DEFAULT 100 /* 71KHZ //200//110 khz  if EEPROM VCC=5V, Max =400Khz, But Vcc=3.3 Max VCC=100Khz */


#define EDID_EEPROM1 0
#define EDID_EEPROM2 1

/*  EDID device ID */
#define EDIDID     0x50  /*  0xA0 */


#define GPIO_REG_OFFSET 0x24000

#define GPIO_GRL_BASE (IO_BASE+GPIO_REG_OFFSET)

/* DDC of HDMI Rx Port2 as GPIO , for I2C master mode */

#define GPIO2_EN1 0x320
#define HDMI2_SCK_DIR_OUT (1<<15)
#define HDMI2_SD_DIR_OUT  (1<<16)
#define HDMI_Rx_SCK_OUT_EN (1<<19)
#define HDMI_Rx_SD_OUT_EN  (1<<20)

#define GPIO2_OUT1 0x328
#define HDMI2_SCK_OUT_ONE (1<<15)
#define HDMI2_SD_OUT_ONE (1<<16)
#define HDMI_Rx_SCK_OUT (1<<19)
#define HDMI_Rx_SD_OUT  (1<<20)

#define GPIO2_IN1 0x324
#define HDMI2_SCK_IN (1<<15)
#define HDMI2_SD_IN  (1<<16)
#define HDMI_Rx_SCK_IN (1<<19)
#define HDMI_Rx_SD_IN  (1<<20)

#define vWriteGPIOCTRL(dAddr, dVal)  \
	IO_WRITE32(GPIO_GRL_BASE, dAddr, dVal)
/*(volatile UINT32 *)(IO_BASE_ADDRESS + HDMI_GRL_REG_OFFSET + bAddr) = bVal */
#define ReadGPIOCTRL(bAddr)         \
	IO_READ32(GPIO_GRL_BASE, bAddr)
/* (UINT8)((*(volatile UINT32 *)(IO_BASE_ADDRESS + HDMI_GRL_REG_OFFSET + bAddr))&0xff) */
#define vWriteGPIOMsk(dAddr, dVal, dMsk) vWriteGPIOCTRL((dAddr), (ReadGPIOCTRL(dAddr) & (~(dMsk))) | ((dVal) & (dMsk)))


#define SET_Rx2_DDCD_OUT (vWriteGPIOMsk(GPIO2_EN1, HDMI2_SD_DIR_OUT, HDMI2_SD_DIR_OUT))
#define SET_Rx2_DDCD_IN  (vWriteGPIOMsk(GPIO2_EN1, 0, HDMI2_SD_DIR_OUT))
/* define SET_DDCC_OUT (vWriteGPIOMsk(GPIO_EN1, HDMI_SD_DIR_OUT, HDMI_SD_DIR_OUT)) */
#define SET_Rx2_DDCC_OUT (vWriteGPIOMsk(GPIO2_EN1, HDMI2_SCK_DIR_OUT, HDMI2_SCK_DIR_OUT))
#define SET_Rx2_DDCC_IN  (vWriteGPIOMsk(GPIO2_EN1, 0, HDMI2_SCK_DIR_OUT))



#define SET_Rx2_DDCD  (vWriteGPIOMsk(GPIO2_EN1, 0, HDMI2_SD_DIR_OUT))/* set input */
#define SET_Rx2_DDCC  (vWriteGPIOMsk(GPIO2_EN1, 0, HDMI2_SCK_DIR_OUT))/* set input  */

#define CLR_Rx2_DDCD {vWriteGPIOMsk(GPIO2_OUT1, 0, HDMI2_SD_OUT_ONE); \
			vWriteGPIOMsk(GPIO2_EN1, HDMI2_SD_DIR_OUT, HDMI2_SD_DIR_OUT); } /* set output */
#define CLR_Rx2_DDCC {vWriteGPIOMsk(GPIO2_OUT1, 0, HDMI2_SCK_OUT_ONE); \
			vWriteGPIOMsk(GPIO2_EN1, HDMI2_SCK_DIR_OUT, HDMI2_SCK_DIR_OUT); } /* set output */

#define Rx2_DDCD     ((ReadGPIOCTRL(GPIO2_IN1) & HDMI2_SD_IN))/* temply */
#define Rx2_DDCC     ((ReadGPIOCTRL(GPIO2_IN1) & HDMI2_SCK_IN))/* temply */

/* DDC of HDMI Rx Port2 as GPIO , for I2C master mode */

#define SET_Rx_DDCD_OUT (vWriteGPIOMsk(GPIO2_EN1, HDMI_Rx_SD_OUT_EN, HDMI_Rx_SD_OUT_EN))
#define SET_Rx_DDCD_IN  (vWriteGPIOMsk(GPIO2_EN1, 0, HDMI_Rx_SD_OUT_EN))

#define SET_Rx_DDCC_OUT (vWriteGPIOMsk(GPIO2_EN1, HDMI_Rx_SCK_OUT_EN, HDMI_Rx_SCK_OUT_EN))
#define SET_Rx_DDCC_IN  (vWriteGPIOMsk(GPIO2_EN1, 0, HDMI_Rx_SCK_OUT_EN))



#define SET_Rx_DDCD  (vWriteGPIOMsk(GPIO2_EN1, 0, HDMI_Rx_SD_OUT))/* set input */
#define SET_Rx_DDCC  (vWriteGPIOMsk(GPIO2_EN1, 0, HDMI_Rx_SCK_OUT))/* set input */

#define CLR_Rx_DDCD {vWriteGPIOMsk(GPIO2_OUT1, 0, HDMI_Rx_SD_OUT); \
			vWriteGPIOMsk(GPIO2_EN1, HDMI_Rx_SD_OUT_EN, HDMI_Rx_SD_OUT_EN); } /* set output */
#define CLR_Rx_DDCC {vWriteGPIOMsk(GPIO2_OUT1, 0, HDMI_Rx_SCK_OUT); \
			vWriteGPIOMsk(GPIO2_EN1, HDMI_Rx_SCK_OUT_EN, HDMI_Rx_SCK_OUT_EN); } /* set output */

#define Rx_DDCD     ((ReadGPIOCTRL(GPIO2_IN1) & HDMI_Rx_SD_IN))/* temply */
#define Rx_DDCC     ((ReadGPIOCTRL(GPIO2_IN1) & HDMI_Rx_SCK_IN))/* temply */




/*********************************************************************
Exported API
*********************************************************************
#if (DRV_SUPPORT_HDMI_RX)
*/
extern BOOL fgEeprom1DataRead(BYTE bDevice, BYTE bData_Addr, BYTE bDataCount,
			BYTE *prBuffer);
extern BOOL fgEeprom2DataRead(BYTE bDevice, BYTE bData_Addr, BYTE bDataCount,
			BYTE *prBuffer);

extern BOOL fgEeprom1DataWrite(BYTE bDevice, BYTE bData_Addr,
			BYTE bDataCount, BYTE *prData);

extern BOOL fgEeprom2DataWrite(BYTE bDevice, BYTE bData_Addr,
			BYTE bDataCount, BYTE *prData);

#define fgEeprom1ByteWrite(bDevice, bData_Addr, bData) \
			fgEeprom1DataWrite(bDevice, bData_Addr, 1, &bData)

#define fgEeprom2ByteWrite(bDevice, bData_Addr, bData) \
			fgEeprom2DataWrite(bDevice, bData_Addr, 1, &bData)

#define fgEeprom1ByteRead(bDevice, bData_Addr, pbData) \
			fgEeprom1DataRead(bDevice, bData_Addr, 1, pbData)

#define fgEeprom2ByteRead(bDevice, bData_Addr, pbData) \
			fgEeprom2DataRead(bDevice, bData_Addr, 1, pbData)


extern void vSWResetEEPROM(BYTE bEEPROEMNo);

void vInitEepromI2cLine(void);
void vSetEepromClock(BYTE bClck);
#if (EDID_SUPPORT_MODE == INTERAL_MODE) || (EDID_SUPPORT_MODE == MIX_MODE)
void vEnableEDIDDLMODE(BOOL fgEnable);
void vWriteEDIDCHKSUM(UINT8 u1DevNum, UINT8 u1Val);
void vWriteEDIDPA(UINT8 u1DevNum, UINT16 u2Val, UINT8 u1Offset);
void vSetEDIDDLADD(UINT8 u1ADD);
void vWriteEDIDRam(UINT32 u4Val);
UINT32 u4ReadEDIDRam(void);
#endif
void vHDMIRxDDCChgToMaster(BOOL fgMaster);


/*#endif
*********************************************************************
Define I2C Read/Write Flag
*********************************************************************/
#define   FG_SEQREAD    1
#define   FG_RANDREAD   0
#endif
#endif /* _DDC_I2C_H_ */
