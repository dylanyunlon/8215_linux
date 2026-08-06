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

#include "x_serial.h"
#include "x_typedef.h"
#include "x_bim.h"
#include "dual_task.h"
#include <stdarg.h>
#include "printf.h"
#define VA_LIST             va_list
#define VA_START(ap, a1)    va_start((ap), (a1))
#define VA_ARG(ap, t)       va_arg(ap, t)
#define VA_END(ap)          va_end((ap))

const char *plevel[XLOG_LEVEL_MAX] = {
 	"[EMERG]",	// emerg
 	"[ALERT]",	// alert
 	"[CRIT]",	 // critical
 	"[E]",		// error
 	"[W]",		// warning
 	"[N",		 // notice
 	"[I]",		// info
 	"[D]"		 // debug
};
#ifdef CONFIG_ATC_USER
unsigned def_dbg_level = XLOG_ERR;
#else
unsigned def_dbg_level = XLOG_INFO; // INFO and lower log will be output to uart, and all log will be store in ram console.
#endif

__attribute__((weak)) void ac8015_putchar(int c)
{
	outbyte((char)c);
}

static void printchar(char **str, int c)
{
	if (str && *str) {
		**str = (char)c;
		++(*str);
	} else {
		ac8015_putchar(c);
	}
}

#define PAD_RIGHT 1
#define PAD_ZERO 2

static int prints(char **out, const char *string, int width, int pad)
{
	register int pc = 0, padchar = ' ';

	if (width > 0) {
		register int len = 0;
		register const char *ptr;
		for (ptr = string; *ptr; ++ptr) ++len;
		if (len >= width) width = 0;
		else width -= len;
		if (pad & PAD_ZERO) padchar = '0';
	}
	if (!(pad & PAD_RIGHT)) {
		for ( ; width > 0; --width) {
			printchar (out, padchar);
			++pc;
		}
	}
	for ( ; *string ; ++string) {
		printchar (out, *string);
		++pc;
	}
	for ( ; width > 0; --width) {
		printchar (out, padchar);
		++pc;
	}

	return pc;
}

/* the following should be enough for 32 bit int */
#define PRINT_BUF_LEN 17

static int printi(char **out, int i, int b, int sg, int width, int pad, int letbase)
{
	char print_buf[PRINT_BUF_LEN];
	register char *s;
	register int t, neg = 0, pc = 0;
	register unsigned int u = (unsigned int)i;

	if (i == 0) {
		print_buf[0] = '0';
		print_buf[1] = '\0';
		return prints (out, print_buf, width, pad);
	}

	if (sg && b == 10 && i < 0) {
		neg = 1;
		u = (unsigned int)-i;
	}

	s = print_buf + PRINT_BUF_LEN-1;
	*s = '\0';

	while (u) {
		t = (unsigned int)u % b;
		if( t >= 10 )
			t += letbase - '0' - 10;
		*--s = (char)(t + '0');
		u /= b;
	}

	if (neg) {
		if( width && (pad & PAD_ZERO) ) {
			printchar (out, '-');
			++pc;
			--width;
		}
		else {
			*--s = '-';
		}
	}

	return pc + prints (out, s, width, pad);
}

static int printl(char **out, unsigned long long i, int b, int sg, int width, int pad, int letbase)
{
	char print_buf[PRINT_BUF_LEN];
	register char *s;
	register int t, neg = 0, pc = 0;
	register unsigned long long u = i;
	(void)sg;

	if (b != 16) {
		/* do not support __aeabi_uldivmod, so we ignore all other base */
		return 0;
	}

	if (i == 0) {
		print_buf[0] = '0';
		print_buf[1] = '\0';
		return prints (out, print_buf, width, pad);
	}

	s = print_buf + PRINT_BUF_LEN-1;
	*s = '\0';

	while (u) {
		t = (unsigned int)(u & 0xF);
		if( t >= 10 )
			t += letbase - '0' - 10;
		*--s = (char)(t + '0');
		u >>= 4;
	}

	if (neg) {
		if( width && (pad & PAD_ZERO) ) {
			printchar (out, '-');
			++pc;
			--width;
		}
		else {
			*--s = '-';
		}
	}

	return pc + prints (out, s, width, pad);
}

static int print( char **out, const char *format, va_list args )
{
	register int width, pad;
	register int pc = 0;
	char scr[2];

	for (; *format != 0; ++format) {
		if (*format == '%') {
			++format;
			width = pad = 0;
			if (*format == '\0') break;
			if (*format == '%') goto out;
			if (*format == '-') {
				++format;
				pad = PAD_RIGHT;
			}
			while (*format == '0') {
				++format;
				pad |= PAD_ZERO;
			}
			for ( ; *format >= '0' && *format <= '9'; ++format) {
				width *= 10;
				width += *format - '0';
			}
			if( *format == 's' ) {
#ifdef __RTOS_A53__
				register char *s = (char *)va_arg( args, char * );
#else
				register char *s = (char *)(unsigned long)va_arg( args, int );
#endif
				pc += prints (out, s?s:"(null)", width, pad);
				continue;
			}
			if( *format == 'l' ) {
				++format;
				if( *format == 'd' ) {
					pc += printi (out, va_arg( args, long ), 10, 1, width, pad, 'a');
					continue;
				}
				if( *format == 'u' ) {
					pc += printi (out, va_arg( args, long ), 10, 0, width, pad, 'a');
					continue;
				}
				if( *format == 'l' ) {
					++format;
					if( *format == 'x' ) {
						pc += printl (out, va_arg( args, unsigned long long), 16, 1, width, pad, 'a');
						continue;
					}
					if( *format == 'd' ) {
						pc += printl (out, va_arg( args, unsigned long long), 10, 1, width, pad, 'a');
						continue;
					}
				}
			}
			if( *format == 'd' ) {
				pc += printi (out, va_arg( args, int ), 10, 1, width, pad, 'a');
				continue;
			}
			if( *format == 'x' ) {
				pc += printi (out, va_arg( args, int ), 16, 0, width, pad, 'a');
				continue;
			}
			if( *format == 'p' ) {
				pc += prints (out, "0x", width, pad);
				pc += printi (out, va_arg( args, int ), 16, 0, 8, PAD_ZERO, 'a');
				continue;
			}
			if( *format == 'X' ) {
				pc += printi (out, va_arg( args, int ), 16, 0, width, pad, 'A');
				continue;
			}
			if( *format == 'u' ) {
				pc += printi (out, va_arg( args, int ), 10, 0, width, pad, 'a');
				continue;
			}
			if( *format == 'c' ) {
				/* char are converted to int then pushed on the stack */
				scr[0] = (char)va_arg( args, int );
				scr[1] = '\0';
				pc += prints (out, scr, width, pad);
				continue;
			}
		}
		else {
		out:
			printchar (out, *format);
			++pc;
			if('\n' == *format) {
				printchar(out, '\r');
				++pc;
			}
		}
	}
	if (--format && (*format != '\n'))
		printchar(out, '\n');
	if (out) **out = '\0';
	va_end( args );
	return pc;
}
INT32 Printf(const CHAR *format, ...)
{
	int ret = 0;
	va_list args;
    BIM_GETHWSemaphore(HW_SEM_MSG_UART, 0);

	va_start( args, format );
	ret = print( 0, format, args );
    BIM_ReleaseHWSemaphore(HW_SEM_MSG_UART);

	return ret;
}

int printf(const char *format, ...)
{
	int ret = 0;
	va_list args;
    BIM_GETHWSemaphore(HW_SEM_MSG_UART, 0);

	va_start( args, format );
	ret = print( 0, format, args );
    BIM_ReleaseHWSemaphore(HW_SEM_MSG_UART);

	return ret;
}


INT32 printk(const CHAR *format, ...)
{
	int ret = 0;
#if 0
	va_list args;
    BIM_GETHWSemaphore(HW_SEM_MSG_UART, 0);

	va_start( args, format );
	ret = print( 0, format, args );
    BIM_ReleaseHWSemaphore(HW_SEM_MSG_UART);
	return ret;
#endif 
	return 0;
}

//with any other log ,original print
int printf_orig(const char *format, ...)
{
	int ret = 0;
	va_list args;
    BIM_GETHWSemaphore(HW_SEM_MSG_UART, 0);
	va_start( args, format );
	ret = print( 0, format, args );
    BIM_ReleaseHWSemaphore(HW_SEM_MSG_UART);

	return ret;
}

int puts (const char *c)
{
	char *p = (char*)c;
	int ret = 0;

	do {
		ac8015_putchar(*p);
		p++;
		ret++;
	} while(*p != '\0');

	ac8015_putchar('\n');

	return ret;
}

int sprintf(char *out, const char *format, ...)
{
	va_list args;
	va_start( args, format );
	return print( &out, format, args );
}

#ifdef __RTOS_A53__
int snprintf( char *buf, size_t count, const char *format, ... )
#else
int snprintf( char *buf, unsigned int count, const char *format, ... )
#endif
{
	va_list args;
	( void ) count;
	va_start( args, format );
	return print( &buf, format, args );
}
