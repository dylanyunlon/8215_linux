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

#include <windows.h>

#include <linux/types.h>


extern void vRs232Write(uint32_t dwValue);


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
static u8 _szTransTbl[2][16] = {
	{'0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
		'a', 'b', 'c', 'd', 'e', 'f'},
	{'0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
		'A', 'B', 'C', 'D', 'E', 'F'},
};
static void OutputHex(unsigned long n, long depth, int low)
{
	if (depth) {
		depth--;
	}

	if ((n & ~0xf) || depth) {
		OutputHex(n >> 4, depth, low);
		n &= 0xf;
	}

	vRs232Write(_szTransTbl[low][n]);
}


/*****************************************************************************
*
*
*   @func   void | OutputNumber | Print the decimal representation of
*				a number through the monitor port.
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
static void OutputNumber(unsigned long n, long depth, int digit)
{
	if (depth) {
		depth--;
	}
	if ((n >= (unsigned long)digit) || depth) {
		OutputNumber(n / digit, depth, digit);
		n %= digit;
	}

	vRs232Write((unsigned char)(n + '0'));
}

/*****************************************************************************
*
*
*   @func   void    |   OutputString | Sends an unformatted string
				to the monitor port.
*
*   @rdesc  none
*
*   @parm   const unsigned char * |   s |
*               points to the string to be printed.
*
*   @comm
*           backslash n is converted to backslash r backslash n
*/
static void OutputString(const s8 *s)
{
	while (*s) {
		if (*s == TEXT('\n')) {
			vRs232Write('\r');
		}

		vRs232Write((u8)*s++);
	}
}

void UART_Printf(const char *sz, ...)
{
	u32      c;
	long     lMinNum;
	va_list  vl;

	va_start(vl, sz);

	while (*sz) {
		c = *sz++;
		switch (c) {
		case TEXT('%'):
			lMinNum = 0;
			c = *sz++;
			while ((c >= TEXT('0')) && (c <= TEXT('9'))) {
				lMinNum *= 10;
				lMinNum += c - TEXT('0');
				c = *sz++;
			}
			switch (c) {
			case TEXT('x'):
				OutputHex(va_arg(vl, unsigned long), lMinNum, 0);
				break;
			case TEXT('B'):
				OutputHex(va_arg(vl, unsigned long), lMinNum, 1);
				break;
			case TEXT('H'):
				OutputHex(va_arg(vl, unsigned long), lMinNum, 1);
				break;
			case TEXT('X'):
				OutputHex(va_arg(vl, unsigned long), lMinNum, 1);
				break;
			case TEXT('d'):
				{
					long    l;

					l = va_arg(vl, long);
					if (l < 0) {
						vRs232Write('-');
						l = -l;
					}
					OutputNumber((unsigned long)l, lMinNum, 10);
				}
				break;
			case TEXT('u'):
				OutputNumber(va_arg(vl, unsigned long), lMinNum, 10);
				break;
			case TEXT('s'):
				OutputString(va_arg(vl, s8 *));
				break;
			case TEXT('%'):
				vRs232Write('%');
				break;
			case TEXT('c'):
				c = va_arg(vl, u32);
				vRs232Write((u8)c);
				break;
			default:
				vRs232Write(' ');
				break;
			}
			break;
		case TEXT('\r'):
			if (*sz == TEXT('\n'))
				sz++;
			c = TEXT('\n');
		case TEXT('\n'):
			vRs232Write('\r');
		default:
			vRs232Write((u8)c);
		}
	}
	va_end(vl);
}


