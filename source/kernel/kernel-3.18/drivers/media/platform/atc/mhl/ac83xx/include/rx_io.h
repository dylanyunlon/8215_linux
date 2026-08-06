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

#ifndef _RX_IO_H_
#define _RX_IO_H_

/*#if (DRV_SUPPORT_HDMI_RX) */
#include "x_typedef.h"
#include "typedef.h"
#include "x_hal_ic.h"
#include "hdmi_rx_hw.h"


BYTE HDMIRX_ReadI2C_Byte(BYTE RegAddr);
void Delay5MS(UINT32 count);
void vHalSetRxPort1HPDLevel(BOOL fgHighLevel);

void vHalSetRxSysResetPin(BOOL fgHighLevel);
void vHalEdid1SelPin(BOOL fgEnable);
void vHalSetSwitchRxPortHPDLevel(BOOL fgHighLevel);
BOOL fgRxPort1PWR5VStatus(void);
BOOL fgRxPort2PWR5VStatus(void);
#endif
