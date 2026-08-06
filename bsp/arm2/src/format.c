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
/*!
 * @file format.c
 *
 * @par LEGAL DISCLAIMER
 *
 *
 */

//#include <windows.h>
//#include <blcommon.h>
#include <stdarg.h>
#include "x_serial.h"
#include "x_typedef.h"

#include "dual_task.h"


//
// Functional Prototypes
//
extern BOOL BIM_GETHWSemaphore(UINT32 u4Number, UINT32 u4TimeOut);
extern BOOL BIM_ReleaseHWSemaphore(UINT32 u4Number);

static void pOutputByte(unsigned char c);
static void OutputString(const unsigned char *s);
static void OutputNumber (unsigned long n, long depth, int digit);
static void OutputHex (unsigned long n, long depth, int low);

char *szSprintf = NULL;

#define OEMWriteDebugByte outbyte

//
// Routine starts
//
/*****************************************************************************
*
*
*   @func   void    | KITLOutputDebugString | Simple formatted debug output string routine
*
*   @rdesc  none
*
*   @parm   LPCSTR  |   sz,... |
*               Format String:
*
*               @flag Format string | type
*               @flag u | unsigned
*               @flag d | int
*               @flag c | char
*               @flag s | string
*               @flag x | 4-bit hex number
*               @flag B | 8-bit hex number
*               @flag H | 16-bit hex number
*               @flag X | 32-bit hex number
*
*   @comm
*           Same as FormatString, but output to serial port instead of buffer.
*/
//void KITLOutputDebugString (LPCSTR sz, ...)
void Printf_CE(char * sz, ...)
{
    unsigned char    c;
    va_list         vl;
    long lMinNum;
    
    va_start(vl, sz);

    BIM_GETHWSemaphore(HW_SEM_MSG_UART, 0);

    OEMWriteDebugByte('[');
    OEMWriteDebugByte('A');
    OEMWriteDebugByte('R');
    OEMWriteDebugByte('M');
    OEMWriteDebugByte('2');
    OEMWriteDebugByte(']');
    OEMWriteDebugByte(' ');
    
    while (*sz) {
        c = *sz++;
        switch (c) {
        case '%':
            lMinNum = 0;
            c = *sz++;
            while((c>= '0') && (c<= '9'))
            {
                lMinNum *= 10;
                lMinNum += c - '0';
                c = *sz ++;
            }
            switch (c) { 
            case 'x':
                OutputHex(va_arg(vl, unsigned long), lMinNum, 0);
                break;
            case 'B':
                OutputHex(va_arg(vl, unsigned long), lMinNum, 1);
                break;
            case 'H':
                OutputHex(va_arg(vl, unsigned long), lMinNum, 1);
                break;
            case 'X':
                OutputHex(va_arg(vl, unsigned long), lMinNum, 1);
                break;
            case 'd':
                {
                    long    l;
                
                    l = va_arg(vl, long);
                    if (l < 0) { 
                        pOutputByte('-');
                        l = - l;
                    }
                    OutputNumber((unsigned long)l, lMinNum, 10);
                }
                break;
            case 'u':
                OutputNumber(va_arg(vl, unsigned long), lMinNum, 10);
                break;
            case 's':
                OutputString(va_arg(vl, char *));
                break;
            case '%':
                pOutputByte('%');
                break;
            case 'c':
                //c = va_arg(vl, unsigned char);
                c = va_arg(vl, int);
                pOutputByte(c);
                break;
                
            default:
                pOutputByte(' ');
                break;
            }
            break;
        case '\r':
            if (*sz == '\n')
                sz ++;
            c = '\n';
            // fall through
        case '\n':
            pOutputByte('\r');
            // fall through
        default:
            pOutputByte(c);
        }
    }

    BIM_ReleaseHWSemaphore(HW_SEM_MSG_UART);
    
    va_end(vl);
}

/*****************************************************************************
*
*
*   @func   void    |   FormatString | Simple formatted output string routine
*
*   @rdesc  Returns length of formatted string
*
*   @parm   unsigned char * |   pBuf |
*               Pointer to string to return formatted output.  User must ensure
*               that buffer is large enough.
*
*   @parm   const unsigned char * |   sz,... |
*               Format String:
*
*               @flag Format string | type
*               @flag u | unsigned
*               @flag d | int
*               @flag c | char
*               @flag s | string
*               @flag x | 4-bit hex number
*               @flag B | 8-bit hex number
*               @flag H | 16-bit hex number
*               @flag X | 32-bit hex number
*
*   @comm
*           Same as OutputFormatString, but output to buffer instead of serial port.
*/
#if 0
unsigned int FormatString (unsigned char *pBuf, const unsigned char *sz, ...)
{
    unsigned char    c;
    long lMinNum;
    va_list         vl;
    
    va_start(vl, sz);
    
    szSprintf = pBuf;
    while (*sz) {
        c = *sz++;
        switch (c) {
        case '%':
            lMinNum = 0;
            c = *sz++;
            while((c>= '0') && (c<= '9'))
            {
                lMinNum *= 10;
                lMinNum += c - '0';
                c = *sz ++;
            }
            switch (c) { 
            case 'x':
                OutputHex(va_arg(vl, unsigned long), lMinNum, 0);
                break;
            case 'B':
                OutputHex(va_arg(vl, unsigned long), lMinNum, 1);
                break;
            case 'H':
                OutputHex(va_arg(vl, unsigned long), lMinNum, 1);
                break;
            case 'X':
                OutputHex(va_arg(vl, unsigned long), lMinNum, 1);
                break;
            case 'd':
                {
                    long    l;
                
                    l = va_arg(vl, long);
                    if (l < 0) { 
                        pOutputByte('-');
                        l = - l;
                    }
                    OutputNumber((unsigned long)l, lMinNum, 10);
                }
                break;
            case 'u':
                OutputNumber(va_arg(vl, unsigned long), lMinNum, 10);
                break;
            case 's':
                OutputString(va_arg(vl, char *));
                break;
            case '%':
                pOutputByte('%');
                break;
            case 'c':
                c = va_arg(vl, unsigned char);
                pOutputByte(c);
                break;
                
            default:
                pOutputByte(' ');
                break;
            }
            break;
        case '\r':
            if (*sz == '\n')
                sz ++;
            c = '\n';
            // fall through
        case '\n':
            pOutputByte('\r');
            // fall through
        default:
            pOutputByte(c);
        }
    }
    pOutputByte(0);
    c = szSprintf - pBuf;
    szSprintf = 0;
    va_end(vl);
    return (c);
}

#endif

/*****************************************************************************
*
*
*   @func   void    |   pOutputByte | Sends a byte out of the monitor port.
*
*   @rdesc  none
*
*   @parm   unsigned int |   c |
*               Byte to send.
*
*/
static void
pOutputByte(
    unsigned char c
)
{
    if (szSprintf)
        *szSprintf++ = c;
    else
        OEMWriteDebugByte(c);
}


/*****************************************************************************
*
*
*   @func   void    |   OutputHex | Print the hex representation of a number through the monitor port.
*
*   @rdesc  none
*
*   @parm   unsigned long |   n |
*               The number to print.
*
*   @parm   long | depth |
*               Minimum number of digits to print.
*   @parm   int | low |
*               0: lowercase output
*               1: uppercase outpu.
*
*/
static UINT8 _szTransTbl[2][16] = {
    {'0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f'},
    {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'},
};
static void OutputHex (unsigned long n, long depth, int low)
{
    if (depth) {
        depth--;
    }
    
    if ((n & ~0xf) || depth) {
        OutputHex(n >> 4, depth, low);
        n &= 0xf;
    }
    
    pOutputByte(_szTransTbl[low][n]);
}



/*****************************************************************************
*
*
*   @func   void    |   OutputNumber | Print the decimal representation of a number through the monitor port.
*
*   @rdesc  none
*
*   @parm   unsigned long |   n |
*               The number to print.

*   @parm   long | depth |
*               Minimum number of digits to print.
*   @parm   int | digit |
*               2: binary
*               8: octal
*               10: decimal
*
*/
static void OutputNumber (unsigned long n, long depth, int digit)
{
    if (depth) {
        depth--;
    }
    if ((n >= (unsigned long)digit) || depth) {
        OutputNumber(n / digit, depth, digit);
        n %= digit;
    }
    pOutputByte((unsigned char)(n + '0'));
}



/*****************************************************************************
*
*
*   @func   void    |   OutputString | Sends an unformatted string to the monitor port.
*
*   @rdesc  none
*
*   @parm   const unsigned char * |   s |
*               points to the string to be printed.
*
*   @comm
*           backslash n is converted to backslash r backslash n
*/
static void OutputString (const unsigned char *s)
{
    while (*s) {        
        if (*s == '\n') {
            OEMWriteDebugByte('\r');
        }
        OEMWriteDebugByte(*s++);
    }
}
