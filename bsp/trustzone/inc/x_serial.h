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
/****************************************************************************

 Filename     : $Workfile: x_serial.h $
 Description  : Poll mode serial driver

 $Log: /emulation/include/x_serial.h $
 *
 * 1     04/05/26 3:22p Xavier
 *
 * 1     04/05/04 9:30p Xavier

 Copyright (C) 2004 MediaTek Incorporation. All Rights Reserved.

****************************************************************************/

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
EXTERN CHAR inbyte(void);
EXTERN BOOL SerInByte(UCHAR* puc);

EXTERN void SerIsrEnable(void);
EXTERN void SerIsrDisable(void);
EXTERN void SerIsrReg(void);
EXTERN UINT8 SerGetDebugPortNum(void);
EXTERN void SerSetoutbyte(BOOL fgSet);

EXTERN void DumpLogBuffer(void);
EXTERN void SerReset(void);

#endif	// X_SERIAL_H

