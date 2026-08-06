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
/*
 *Department: ATC-SD-SD2
 *
 *Aurthor: yezhiqiang(atc0064)
 *
 *Date: 2016-12-05
 */

#ifndef _WCH_IF_H_
#define _WCH_IF_H_

#include "wch_drv.h"



#ifdef __ARM2__

typedef int irqreturn_t;
#define IRQ_HANDLED 0x1;

typedef int (*irq_handler_t)(int u4Vector, void * dev_id);
#endif

unsigned int OpenWch(unsigned char u1wchId, WCH_SRC_APP_ID_E eWchSrcId);
WCH_SRC_APP_ID_E getWhichSrcUseWch(unsigned char u1wchId);
unsigned int ConfigWch(PWCH_CFG_T pWchCtlParam);
unsigned int StartWch(unsigned char u1WchId, WCH_SRC_APP_ID_E eWchSrcId);
unsigned int StopWch(unsigned char u1WchId, WCH_SRC_APP_ID_E eWchSrcId);
unsigned int CloseWch(unsigned char u1WchId, WCH_SRC_APP_ID_E eWchSrcId);
unsigned int WchGetBufferAddress(PWCH_BUF_T pWchGetBuf);
#if WCH_SYNC_BUFFER
unsigned int WchReturnBuffer(unsigned char u1WchId, unsigned int u4BufIdx);
#endif
#endif

