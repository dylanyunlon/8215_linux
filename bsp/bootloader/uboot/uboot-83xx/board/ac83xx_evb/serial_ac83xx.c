/*
 * (C) Copyright 2000
 * Rob Taylor, Flying Pig Systems. robt@flyingpig.com.
 *
 * (C) Copyright 2004
 * ARM Ltd.
 * Philippe Robin, <philippe.robin@arm.com>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/* Simple U-Boot driver for the PrimeCell PL011 UARTs on the IntegratorCP */
/* Should be fairly simple to make it work with the PL010 as well */

#include <common.h>
#include <asm/arch/x_bim.h>

//#ifdef CFG_AC83XX_SERIAL

#include <asm/arch/serial_ac83xx.h>
#include <asm/arch/x_typedef.h>


//-----------------------------------------------------------------------------
// Static variables
//-----------------------------------------------------------------------------
static UINT32 _u4SerIntMode = 0;

static CHAR _aszUartIntBuf[UART_INT_BUF_SIZE];
static UINT32 _aszUartIntBufRead = 0;
static UINT32 _aszUartIntBufWrite = 0;

static UINT8 _u1DebugPort = DBG_PORT;
static BOOL _fgIsMT5371 = TRUE;
static BOOL _fgSetoutbyte = TRUE;

UINT32 dwRWTEST1 = 0x11111111;
UINT32 dwRWTEST2 = 0x22222222;
UINT32 dwZITEST1;
UINT32 dwZITEST2;

static void ac83xx_putc (UINT8 c);
static int ac83xx_getc ();
static int ac83xx_tstc();
static UINT8 _BspSerGetRxDataCnt()
{
  UINT8 u1RxCnt;
  u1RxCnt = (UINT8)(REG_SER_STATUS & SER_READ_ALLOW);
  //u1RxCnt = 1;
  return u1RxCnt;
}

//-----------------------------------------------------------------------------
/** _BspSerIntGetChar() provides polling mode to get the character.
 *  _BspSerIntGetChar() will return 0xff if no character input, otherwise it
 *  poll and get the input character.
 *  @return the char which polled, or 0xff no character.
 */
//-----------------------------------------------------------------------------
static UINT8 _BspSerIntGetChar(void)
{
  UINT8 uReturnValue;

  if (_aszUartIntBufRead == _aszUartIntBufWrite)
  {
    return NULL;
  }

  uReturnValue = (UINT8)_aszUartIntBuf[_aszUartIntBufRead];

  _aszUartIntBufRead = (_aszUartIntBufRead + 1) % UART_INT_BUF_SIZE;

  return uReturnValue;
}

//-----------------------------------------------------------------------------
/** UINT8 SerSetoutbyte() set outbyte function enable/disable
 *  @parameter fgSet TRUE: enable outbyte(), FALSE: disable outbyte()
 */
//-----------------------------------------------------------------------------
void SerSetoutbyte(BOOL fgSet)
{
  _fgSetoutbyte = fgSet;
}

void SerTransparent(void)
{
//#ifndef CONFIG_MACH_AC8317FPGA
  REG_SER_STATUS = 0x2207E2;     	// Set to transparent mode
//#else
//  REG_SER_STATUS = 0xE2;
//#endif
  SerSetoutbyte(TRUE);
}

void SerIsrEnable(void)
{
  //Enable RS232 interrupt
  REG_SER_INT_EN = REG_SER_INT_EN_TX_D_ACEPTED | REG_SER_INT_EN_READ_ALLOW;
  
  //Enable UART interrupt
//  VERIFY(BIM_EnableIrq(VECTOR_RS232_1));
  
  //_u4SerIntMode = 1;  
}
//-----------------------------------------------------------------------------
/** void putchar(UINT8) put char, poll mode
 *
 */
//-----------------------------------------------------------------------------
void SerPollPutChar(UINT8 cc)
{
   BIM_GETHWSemaphore(HSMPHE_UART1, 0);
   
   while((REG_SER_STATUS & SER_WRITE_ALLOW) != SER_WRITE_ALLOW) {}
   
   REG_SER_PORT = cc;
   
   BIM_ReleaseHWSemaphore(HSMPHE_UART1);
}
//-----------------------------------------------------------------------------
/** void SerInit(void) init Serial port routine.
 *
 */
//-----------------------------------------------------------------------------
int serial_init(void)
{
 
    SerTransparent();   
    /* Enable RS232 interrupt */
//    REG_SER_INT_EN = REG_SER_INT_EN_TX_D_ACEPTED | REG_SER_INT_EN_READ_ALLOW;
//    REG_SER_IRQ = REG_SER_IRQ | REG_SER_IRQ_EN;
    SerIsrEnable();
  
	return 0;
}

void serial_putc (const char c)
{
	if (c == '\n')
		ac83xx_putc ('\r');

	ac83xx_putc (c);
}

void serial_puts (const char *s)
{
	while (*s) {
		serial_putc (*s++);
	}
}

int serial_getc (void)
{
	return ac83xx_getc ();
}

int serial_tstc (void)
{
	return ac83xx_tstc ();
}

void serial_setbrg (void)
{
}

static void ac83xx_putc (UINT8 c)
{
	static CHAR prev = 0;

	if (!_fgSetoutbyte)
	{
		return;
	}

	if ((c < ' ') && (c != '\r') && (c != '\n') && (c != '\t') && (c != '\b'))
	{
		return;
	}

	if ((c == '\n') && (prev != '\r'))
	{
		SerPollPutChar('\r');
	}
	prev = c;

	SerPollPutChar((UINT8)c);
}

static int ac83xx_getc ()
{
	if (_u4SerIntMode)
	{
		return _BspSerIntGetChar();
	}

	// if data NOT ready, return 0xFF
	if (_BspSerGetRxDataCnt() == 0)
	{
		return NULL;
	}

	return REG_SER01_PORT;
}

static int ac83xx_tstc()
{
	if(_BspSerGetRxDataCnt() > 0)
                return 1;
        else
                return 0;
}
//#endif

