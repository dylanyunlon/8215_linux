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
#include "targetConfig.h"
#include "preloader_common.h"
#include "x_serial.h"

#ifdef CONFIG_ATC_USER
unsigned def_dbg_level = XLOG_ERR;
#else
unsigned def_dbg_level = XLOG_INFO; // INFO and lower log will be output to uart, and all log will be store in ram console.
#endif



int iPrintTimerOn = 1;
#define T64B_GET_LOW()	BIM_READ32(REG_RW_T64b_LO_0)


#define PAD_ZERO 1
INT32 pad;
static INT32 prints(const CHAR *string, INT32 width)
{
    register INT32 pc = 0, padchar = ' ';

    if (width > 0) {
        register INT32 len = 0;
        register const CHAR *ptr;
        for (ptr = string; *ptr; ++ptr) ++len;
        if (len >= width) width = 0;
        else width -= len;
        if (pad & PAD_ZERO) padchar = '0';
    }
    for ( ; width > 0; --width) {
        outbyte (padchar);
        ++pc;
    }    
    for ( ; *string ; ++string) {
        outbyte (*string);
        ++pc;
    }
    for ( ; width > 0; --width) {
        outbyte (padchar);
        ++pc;
    }

    return pc;
}

/* the following should be enough for 32 bit int */
#define PRINT_BUF_LEN 17

static INT32 printi(INT32 i, INT32 b, INT32 sg, INT32 width)
{
    CHAR print_buf[PRINT_BUF_LEN];
    register CHAR *s;
    register INT32 t;
    register INT32 neg = 0, pc = 0;
    register UINT32 u = i;

    if (sg && b == 10 && i < 0) {
        neg = 1;    		
        u = -i;
    }

    s = print_buf + PRINT_BUF_LEN-1;
    *s = '\0';

    if (u==0)
    {
        *--s = '0';
    }
    else
    {
        while (u) {
            /* div{10,16}: u for quotient and t for remainder */
            UINT32 ref;
            switch (b)
            {
            case 16:
                t = u & 0x0f;
                u = u >> 4;
                break;
            case 10:
                t = u;
                u = 0;
                ref = 0xa0000000;
                for (i=0; i<29; i++)
                {
                    u <<= 1;
                    if (t >= ref)
                    {
                        t -= ref;
                        u |= 1;
                    }
                    ref >>= 1;
                }
                break;
            default:
#if 0
                ASSERT("base should be 10 or 16 only");
#else
                /* fallback to base 16 and continue */
                t = u & 0x0f;
                u = u >> 4;
#endif
                break;
            }
            if( t >= 10 )
                t += 'a' - '0' - 10;
            *--s = t + '0';
        }

    }
    if (neg) {
        if( width && (pad & PAD_ZERO) ) {
            outbyte ('-');
            ++pc;
            --width;
        }
        else {
            *--s = '-';
        }
    }

    return pc + prints (s, width);
}

static INT32 print(INT32 *varg)
{
    register INT32 width;
    register INT32 pc = 0;
    register CHAR *format = (CHAR *)(*varg++);
    register UINT32 ch;

    for (ch = *format; ch != 0; ch=*++format) {
        if (ch == '%') {
            ch = *++format;
            width = pad = 0;
            if (ch == '\0') break;
            if (ch == '-') break;
            if (ch == '%') goto out;
            while (ch == '0') {
                ch = *++format;
                pad |= PAD_ZERO;
            }
            for ( ; ch >= '0' && ch <= '9'; ch=*++format) {
                width *= 10;
                width += ch - '0';
            }
            if( ch == 'x' || ch == 'X' ) {
                pc += printi (*varg++, 16, 0, width);
                continue;
            }
            if( ch == 'd' || ch == 'u' ) {
                pc += printi (*varg++, 10, (ch == 'd' ? 1 : 0), width);
                continue;
            }
            if( ch == 'c' ) {
                outbyte((CHAR)*varg++);
                ++pc;
                continue;
            }
        }
        else {
        out:
            outbyte (ch);
            ++pc;
        }
    }
    return pc;
}

/* assuming sizeof(void *) == sizeof(int) */
//int _divide(UINT32 dividend, UINT32 divisor, UINT32 *quotion, UINT32 *remainder);
//int printf(const char *format, ...)
#define TIMER_INTERVAL (189000)	//(189000000 / 1000)
#if (IS_FOR_LITTLE_SIZE==0)

INT32 str2num(char *numstr,UINT32 *num)
{
	INT32 isHex=0;
  INT32 isNeg=0;
  *num = 0;
	while(*numstr != 0)
	{
    if(isHex == 1)
    {
		  (*num) <<=4;
    }
    else
    {
      (*num) *= 10;
    }
    if((*num)==0)
    {
      if(*numstr=='0')
      {
        numstr++;
        if(*numstr =='x' || *numstr =='X')
        {
          isHex = 1;
          numstr++;
          
        }
        continue;
      }
      else if(*numstr=='-')
      {
        isNeg = 1;
        numstr++;
        continue;
      }
    }
    
		if('a' <= *numstr && *numstr <= 'f' && isHex==1)
		{
			(*num) += *numstr-'a'+10;
		}
		else if('A' <= *numstr && *numstr <= 'F' && isHex==1)
		{
			(*num) += *numstr-'A'+10;
		}
		else if('0' <= *numstr && *numstr <= '9')
		{
			(*num) += *numstr-'0';
		}
		else
		{
			return -1;
		}
		numstr++;
	}

	return 0;
	
}


INT32 UART_GetNum(UINT32 *num)
{
  INT32 ret=0;
  INT32 n=0;
  CHAR inB;
  CHAR numstr[20];
  SerTransparent();
  while(1)
  {
    
    while(SerInByte(&inB) == FALSE);
    outbyte (inB);
    if(inB == '\r' || inB =='\n')
    {
      numstr[n]=0;
      break;
    }
    else
    {
      numstr[n]=inB;
      n++;
    }
  }
  SerNormalMode();
  return str2num(numstr,num);
  
}
#endif

volatile static UINT32 uart_lock = 0;

INT32 UART_Printf(const CHAR *format, ...)
{
    INT32 nRet;
//    register INT32 *varg = (INT32 *)(&format);

	while(uart_lock);
	uart_lock = 1;
    SerTransparent();
	
    nRet = print((INT32 *)(&format));
    //SerNormalMode();
	uart_lock = 0;
    return nRet;
}

#if(IS_FOR_LITTLE_SIZE==0)

CHAR UART_GetChar(void)
{
  CHAR c;
  SerTransparent();
  c = inbyte();
  SerNormalMode();
  return c;
}

INT32 UART_GetString(CHAR *pStr,INT32 strSize)
{
  INT32 n=0;
  CHAR inB;
  SerTransparent();
  if(strSize != 0)
  {

    while(n<strSize)
    {
    
      while(SerInByte(&inB) == FALSE);
      outbyte (inB);
      if(inB == '\r' || inB =='\n')
      {
        pStr[n]=0;
        break;
      }
      else
      {
        pStr[n]=inB;
        n++;
      }
    }
    pStr[strSize-1]=0;
  }
  
  SerNormalMode();  
  return n;
}
#endif
//INT32 (*Printf)(const CHAR *format, ...) = _Printf;

