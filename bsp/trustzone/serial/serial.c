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
/*-----------------------------------------------------------------------------
 *
 * $Author: emily.zhang $
 * $Date: 2015/10/19 $
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
#include "../inc/tz_common.h"
#include "../inc/x_serial.h"

//============================================================================
// Constant definitions
//============================================================================
//Local Define
#define UART_PORT_0                 0
#define UART_PORT_1                 1

#define DBG_PORT                    UART_PORT_0

#define REG_SER_PORT                IO_REG8(RS232_BASE, 0x00)
#define REG_SER_STATUS              IO_REG32(RS232_BASE, 0x04)
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


  if ((REG_SER_STATUS & SER_READ_ALLOW) == 0)
  {
    return FALSE;
  }
//  ASSERT(puc!=NULL);
  *puc = REG_SER_PORT;


  return TRUE;
}

/*----------------------------------------------------------------------------
 * void putchar(UINT8) put char, poll mode
 *---------------------------------------------------------------------------*/
void SerPollPutChar(UINT8 cc)
{
  while((REG_SER_STATUS & SER_WRITE_ALLOW) != SER_WRITE_ALLOW) 
  {
  }
  
  REG_SER_PORT = cc;
}

/*----------------------------------------------------------------------------
 * void SerTransparent(void) set Serial port as transparent mode \n
 *      (RISC output debug message)
 *---------------------------------------------------------------------------*/
void SerTransparent(void)
{
#if 0
  UINT32 reg;
  //Set to transparent mode
  reg = REG_SER_STATUS;
  reg &= (~0xFF);
  reg |= 0xE2;
  REG_SER_STATUS = reg;       	
  SerSetoutbyte(TRUE);
#endif
}

/*----------------------------------------------------------------------------
 * void SerTransparent(void) set Serial port as normal mode \n
 *      (for MTK tools, DSP tools).
 *---------------------------------------------------------------------------*/
void SerNormalMode(void)
{
#if 0
  //Set to normal mode
  UINT32 reg;
  //Set to transparent mode
  reg = REG_SER_STATUS;
  reg &= (~0xFF);
  REG_SER_STATUS = reg;      	
#endif
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

  if(!_fgSetoutbyte)
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

CHAR inbyte(void)
{
  CHAR inB;
  while(SerInByte(&inB) == FALSE);
  return inB;
}

void SerReset(void)
{

}
