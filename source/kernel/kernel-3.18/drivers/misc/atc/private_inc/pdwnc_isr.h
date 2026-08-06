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

#ifndef PDWNC_ISR_H
#define PDWNC_ISR_H

#include "x_typedef.h"

#define PDWNC_MAX_INT 17

// PDWNC int Result
#define PDWNC_OK          ((INT32)  0)
#define PDWNC_INV_ARG     ((INT32)  -1)

typedef void (*PDWNC_CALLBACK)(UINT32 u4Vector);

typedef struct
{
  PDWNC_CALLBACK pPdwncCallBack;
} PDWNC_ISR_CALLBACK_T;

typedef enum
{
    e_PDWNC_INT_GPIO0,			// 0
    e_PDWNC_INT_GPIO1,			// 1
    e_PDWNC_INT_GPIO2,			// 2
    e_PDWNC_INT_GPIO3,			// 3
    e_PDWNC_INT_GPIO4,			// 4
    e_PDWNC_INT_GPIO5,			// 5
    e_PDWNC_INT_GPIO6,			// 6
    e_PDWNC_INT_GPIO7,			// 7
    e_PDWNC_INT_UNKNOWN8,		// 8
    e_PDWNC_INT_DBG_UART,		// 9
    e_PDWNC_INT_SIFS,			// 10
    e_PDWNC_INT_CEC,			// 11
    e_PDWNC_INT_ETNET,			// 12
    e_PDWNC_INT_IR,				// 13
    e_PDWNC_INT_SIFM,			// 14
    e_PDWNC_INT_DDCCI,			// 15
    e_PDWNC_INT_SFFE,			// 16
    e_PDWNC_INT_MAX
} e_PDWNC_INT_T;

EXTERN BOOL fgPDWNC_InitIsr(void);
EXTERN void vPDWNC_UninitIsr(void);
EXTERN BOOL fgRegisterPDWNCCallBack(UINT16 u2Vector, PDWNC_CALLBACK pvCallBackFuctionEntry);
EXTERN BOOL fgReleasePDWNCCallBack(UINT16 u2Vector);
EXTERN INT32 i4PDWNC_Init(void);
#endif  // PDWNC_ISR_H
