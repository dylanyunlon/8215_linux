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
/*-----------------------------------------------------------------------------
 *
 * $Author: jianghong.lin $
 * $Date: 2015/07/06 $
 * $RCSfile: serial.c,v $
 * $Revision: #1 $
 *
 *---------------------------------------------------------------------------*/

/** @file serial.c
 *  In serial.c, it provides basic functions for input/output through UART.
 *  This is a poll mode serial driver for boot start package(bsp).  It also
 *  can use the advanced uart driver to run in interrupt mode.
 */

//============================================================================
// Include files
//============================================================================
#include "x_serial.h"
#include "x_bim.h"
#include "x_hal_ic.h"

//============================================================================
// Constant definitions
//============================================================================
//Local Define
#define UART_PORT_0                 0
#define UART_PORT_1                 1

#define DBG_PORT                    UART_PORT_0
#if defined(CONFIG_ATC_PLATFORM_ac823x)
	#define	RS232_BASE				(0x1000C000)
#endif
#define REG_SER_PORT                IO_REG8(RS232_BASE, 0x00)
#define REG_SER_STATUS              IO_REG16(RS232_BASE, 0x04)
  #define SER_READ_ALLOW              0x1
  #define SER_WRITE_ALLOW             0x2
   
#define REG_SER01_PORT              IO_REG8(RS232_BASE, 0x00)
#define REG_SER01_STATUS            IO_REG32(RS232_BASE, 0xCC)

#define REG_SER_INT_EN              IO_REG32(RS232_BASE, 0x0C)
#define REG_SER_INT_STATUS          IO_REG32(RS232_BASE, 0x10)
  #define REG_SER_INT_EN_TX_D_ACEPTED (UINT32)(0x1<<6)
  #define REG_SER_INT_EN_READ_ALLOW   (UINT32)(0x1<<7)

//For UART Interrupt mode
#define UART_INT_BUF_SIZE           1024
#define SER_PORT_0                  (0)

//============================================================================
// Imported variables
//============================================================================
EXTERN volatile UINT32 g_u4EndMagic;

//============================================================================
// Static variables
//============================================================================
static UINT8 _u1DebugPort = DBG_PORT;
static BOOL _fgSetoutbyte = TRUE;

//============================================================================
// Static functions
//============================================================================

//-----------------------------------------------------------------------------
// Public functions
//-----------------------------------------------------------------------------

/*----------------------------------------------------------------------------
 * void SerInByte(UCHAR *puc) read RX character.
 *  @param puc a pointer to unsigned char, it cannot be NULL.
 *  @retval TRUE read a char from RX buffer and put into *puc.
 *  @retval FALSE no char in RX buffer.
 *---------------------------------------------------------------------------*/
BOOL SerInByte(UCHAR* puc)
{
    SerTransparent();

    if ((REG_SER_STATUS & SER_READ_ALLOW) == 0)
    {
        return FALSE;
    }
    //  ASSERT(puc!=NULL);
    *puc = REG_SER_PORT;

    SerNormalMode();
    
    return TRUE;
}

/*----------------------------------------------------------------------------
 * void putchar(UINT8) put char, poll mode
 *---------------------------------------------------------------------------*/
void SerPollPutChar(UINT8 cc)
{
	int count = 0;
	while ((REG_SER_STATUS & SER_WRITE_ALLOW) != SER_WRITE_ALLOW)
	{
		count++;
		if (count > 500) {
			count = 0;
			Printf("arm2,SerPollPutChar, while count > 500 \n");
		}
	}
    //REG_SER_PORT = cc;
    (*((volatile UINT8*)(RS232_BASE + 0x00))) = cc;
}

/*----------------------------------------------------------------------------
 * void SerTransparent(void) set Serial port as transparent mode \n
 *      (RISC output debug message)
 *---------------------------------------------------------------------------*/
void SerTransparent(void)
{
    //Set to transparent mode	
    //REG_SER_STATUS = 0xE2;  
    (*((volatile UINT8*)(RS232_BASE + 0x04))) = 0xE2;     	
    SerSetoutbyte(TRUE);
}

/*----------------------------------------------------------------------------
 * void SerTransparent(void) set Serial port as normal mode \n
 *      (for MTK tools, DSP tools).
 *---------------------------------------------------------------------------*/
void SerNormalMode(void)
{
    //Set to normal mode
    //REG_SER_STATUS = 0; 
	(*((volatile UINT8*)(RS232_BASE + 0x04))) = 0;
}

/*----------------------------------------------------------------------------
 * void SerStart(void) start to enable RISC to uart.
 *---------------------------------------------------------------------------*/
void SerStart(void)
{
    SerSetoutbyte(TRUE);
}

/*----------------------------------------------------------------------------
 * void SerEnd(void) stop Serial port routine.
 *---------------------------------------------------------------------------*/
void SerEnd(void)
{
    //Back to normal mode	
    SerNormalMode();         
    SerSetoutbyte(FALSE);
}

/*----------------------------------------------------------------------------
 * UINT8 SerGetDebugPortNum() get debug port number
 *  @reval debug port number (0 or 1)
 *---------------------------------------------------------------------------*/
UINT8 SerGetDebugPortNum(void)
{
    return _u1DebugPort;
}

/*----------------------------------------------------------------------------
 * UINT8 SerSetoutbyte() set outbyte function enable/disable
 *  @parameter fgSet TRUE: enable outbyte(), FALSE: disable outbyte()
 *---------------------------------------------------------------------------*/
void SerSetoutbyte(BOOL fgSet)
{
    _fgSetoutbyte = fgSet;
}

/*----------------------------------------------------------------------------
 * void outbyte(int c) out char, poll mode, I/F with Print() (printf)
 *  @param c a character which is going to put into TX buffer.
 *---------------------------------------------------------------------------*/
void outbyte(CHAR c)
{
    static CHAR prev = 0;

    if (!_fgSetoutbyte)
    {
        return;
    }

    if((c < ' ') && (c != '\r') && (c != '\n') && (c != '\t') && (c != '\b'))
    {
        return;
    }

    if((c == '\n') && (prev != '\r'))
    {
        SerPollPutChar('\r');
    }
    prev = c;

    SerPollPutChar((UINT8)c);
}


void WriteDebugByte(UINT8 ch) 
{
    outbyte(ch);
}

INT32 ReadDebugByte(void) 
{
    INT32 ch;

    //if (REG_SER_STATUS(SerialBaseV) & SER_READ_ALLOW)        // There is received data
    if (REG_SER_STATUS & SER_READ_ALLOW)
    {
        //ch = (int)(REG_SER_PORT(SerialBaseV));
        ch = (int)(REG_SER_PORT);
    }
    else        // There no data in RX Buffer;
    {
        ch = DEBUG_SERIAL_READ_NODATA;
    }

    return ch;
}


UINT32 GetInputNumber(USHORT MaxLength)
{
    USHORT cwNumChars = 0;
    INT32 InChar = 0;
    BOOL bHex = FALSE;
    UINT32 number = 0;
	
    if( MaxLength == 0 )
        MaxLength = 0xFFFF;
    
    while(!((InChar == 0x0d) || (InChar == 0x0a)))
    {
        InChar = ReadDebugByte();
		
        if (InChar != DEBUG_SERIAL_READ_NODATA ) 
        {
            if( bHex == FALSE && cwNumChars < MaxLength )
            {
                if( (InChar >= '0' && InChar <='9' ) )
                {
                    cwNumChars++;
                    number = (number * 10) + (InChar - '0');
                    WriteDebugByte((BYTE)InChar);
                }
                else if( InChar == 'x'  || InChar == 'X')  // Hex input
                {
                    cwNumChars++;
                    bHex = TRUE;
                    number = 0;
                    WriteDebugByte((BYTE)InChar);
                }
            }
            else if( bHex == TRUE && (cwNumChars < (MaxLength+2)) ) 
            {
                if( (InChar >= '0' && InChar <='9' ) )
                {
                    cwNumChars++;
                    number = (number * 16) + (InChar - '0');
                    WriteDebugByte((BYTE)InChar);
                }
                else if ((InChar >= 'a' && InChar <='f' ))
                {
                    cwNumChars++;
                    number = (number * 16) + (InChar - 'a') + 10;
                    WriteDebugByte((BYTE)InChar);
                }
                else if ((InChar >= 'A' && InChar <='F' ))
                {
                    cwNumChars++;
                    number = (number * 16) + (InChar - 'A') + 10;
                    WriteDebugByte((BYTE)InChar);
                }               
            }
            
			// If it's a backspace, back up.
			//
			if (InChar == 8) 
			{
                if (cwNumChars > 0) 
                {
                    cwNumChars--;
					
                    if( bHex == FALSE )
                        number /= 10;
                    else
                        number /= 16;
					
              WriteDebugByte((BYTE)InChar);
              WriteDebugByte((BYTE)0x20);//add a space char for recover the old char
              WriteDebugByte((BYTE)InChar);
                }
				
                if( cwNumChars <= 1 )
                {
                    bHex = FALSE;
                    number = 0;
                }
			}
        }
    }
	
    return number;
}


