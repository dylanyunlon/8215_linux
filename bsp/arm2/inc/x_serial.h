/* Copyright Statement:
 *
 * This software/firmware and related documentation ("AutoChips Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to AutoChips Inc. and/or its licensors. Without
 * the prior written permission of AutoChips inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of AutoChips Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 * 
 * AutoChips Inc. (C) 2016. All rights reserved.
 * 
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("AUTOCHIPS SOFTWARE")
 * RECEIVED FROM AUTOCHIPS AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. AUTOCHIPS EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES AUTOCHIPS PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE AUTOCHIPS SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN AUTOCHIPS
 * SOFTWARE. AUTOCHIPS SHALL ALSO NOT BE RESPONSIBLE FOR ANY AUTOCHIPS SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND AUTOCHIPS'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE AUTOCHIPS SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT AUTOCHIPS'S OPTION, TO REVISE OR REPLACE THE
 * AUTOCHIPS SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO AUTOCHIPS FOR SUCH AUTOCHIPS SOFTWARE AT ISSUE.
 */


#ifndef X_SERIAL_H
#define X_SERIAL_H

#include "x_typedef.h"



#define ASCII_NULL						0x00
#define ASCII_ENTER						0x0D
#define ASCII_KEY_BS					0x08
#define ASCII_KEY_NL					0x0A
#define ASCII_KEY_CR					0x0D
#define ASCII_KEY_ESC					0x1B
#define ASCII_KEY_ARROW					0x5B
#define ASCII_KEY_UP					0x41
#define ASCII_KEY_DOWN					0x42
#define ASCII_KEY_RIGHT					0x43
#define ASCII_KEY_LEFT					0x44
#define ASCII_KEY_SPACE					0x20
#define ASCII_KEY_PRINTABLE_MIN			0x20
#define ASCII_KEY_PRINTABLE_MAX			0x7E

// for x-modem
#define ASCII_SOH  						0x01
#define ASCII_STX  						0x02
#define ASCII_EOT  						0x04
#define ASCII_ACK  						0x06
#define ASCII_NAK  						0x15
#define ASCII_CAN  						0x18

EXTERN void SerInit(void);
EXTERN void SerStart(void);
EXTERN void SerEnd(void);
EXTERN void SerTransparent(void);
EXTERN void SerNormalMode(void);
EXTERN void SerWaitTxBufClear(void);
EXTERN void SerEnableMergeMode(BOOL fgEnable);
EXTERN void SerOutputLogUseInterrupt(BOOL fgUseInterrupt);
EXTERN void SerPollPutChar(UINT8 cc);
EXTERN UINT32 SerGetRxDataCnt(void);
EXTERN UINT8 SerPollGetChar(void);
EXTERN UINT8 SerGetChar(void);
EXTERN UINT8 SerGetCharTimeout(UINT8* ucChar, UINT32 ui4Time);
EXTERN CHAR* SerGetS(CHAR* s);
EXTERN void outbyte(CHAR c);
EXTERN BOOL SerInByte(UCHAR* puc);

EXTERN void SerIsrEnable(void);
EXTERN void SerIsrDisable(void);
EXTERN void SerIsrReg(void);
EXTERN UINT8 SerGetDebugPortNum(void);
EXTERN void SerSetoutbyte(BOOL fgSet);

EXTERN void DumpLogBuffer(void);
#define DEBUG_SERIAL_READ_NODATA   (-1)


#endif	// X_SERIAL_H

