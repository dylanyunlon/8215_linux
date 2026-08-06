#include "targetConfig.h"
#include "x_typedef.h"
#include "Simulation_log.h"
#include "x_hal_io.h"


#if(SIMULATION_LOG)

#if(CONFIG_CHIP_VER_CURR == CONFIG_CHIP_VER_MT3360)
#define _MemLogAddr_ 0x7000a004
#define _MemLogST_End_Addr_ 0x7000a000
#elif(CONFIG_CHIP_VER_CURR == CONFIG_CHIP_VER_MT3356)
#define _MemLogAddr_ 0x700081E0
#define _MemLogST_End_Addr_ 0x700081E4
#elif(CONFIG_CHIP_VER_CURR == CONFIG_CHIP_VER_MT3363)
#define _MemLogAddr_ 0xF00081E0
#define _MemLogST_End_Addr_ 0xF00081E4
#endif
#define _MemLogStartID_ 0x0
#define _MemLogEndID_ 0x1
#define MemLogBin2Char(dPara) (((dPara) < 10)? ((dPara) + '0') : ((dPara) - 10 + 'a'))
#define LOG_BUF_SIZE    100
UINT8 logBuf[LOG_BUF_SIZE];



CHAR *outbuf;

#define OUTBYTE(char)        *outbuf++=char
#define PAD_ZERO 1
INT32 pad;

static INT32 sprints(const CHAR *string, INT32 width)
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
        OUTBYTE (padchar);
        ++pc;
    }    
    for ( ; *string ; ++string) {
        OUTBYTE (*string);
        ++pc;
    }
    for ( ; width > 0; --width) {
        OUTBYTE (padchar);
        ++pc;
    }

    return pc;
}

/* the following should be enough for 32 bit int */
#define PRINT_BUF_LEN 10

static INT32 sprinti(INT32 i, INT32 b, INT32 sg, INT32 width)
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
            OUTBYTE ('-');
            ++pc;
            --width;
        }
        else {
            *--s = '-';
        }
    }

    return pc + sprints (s, width);
}

static INT32 sprint(CHAR *pstr,INT32 *varg)
{
  register INT32 width;
  register INT32 pc = 0;
  register CHAR *format = (CHAR *)(*varg++);
  register UINT32 ch;
  outbuf = pstr;
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
              pc += sprinti (*varg++, 16, 0, width);
              continue;
          }
          if( ch == 'd' || ch == 'u' ) {
              pc += sprinti (*varg++, 10, (ch == 'd' ? 1 : 0), width);
              continue;
          }
          if( ch == 'c' ) {
              OUTBYTE((CHAR)*varg++);
              ++pc;
              continue;
          }
      }
      else {
      out:
          OUTBYTE (ch);
          ++pc;
      }
  }
  OUTBYTE('0');
  return pc;

}



BOOL bSendLog_ROREG(UINT8 bType, UINT32 dPara)
{
	UINT8 *pStr;	
	UINT32 i;
	UINT8 isEnd;
	UINT32 dTmpBuf;
	switch(bType)	
	{
	case(SI_RLOG_TYPE_DWRD):
	{

		HAL_WRITE32(_MemLogST_End_Addr_, _MemLogStartID_);
		dTmpBuf = 0;
		dTmpBuf |= MemLogBin2Char((dPara >> 28) & 0xf);
		dTmpBuf |= MemLogBin2Char((dPara >> 24) & 0xf) << 8;
		dTmpBuf |= MemLogBin2Char((dPara >> 20) & 0xf) << 16;
		dTmpBuf |= MemLogBin2Char((dPara >> 16) & 0xf) << 24;
		HAL_WRITE32(_MemLogAddr_, dTmpBuf);
		dTmpBuf = 0;
		dTmpBuf |= MemLogBin2Char((dPara >> 12) & 0xf);
		dTmpBuf |= MemLogBin2Char((dPara >> 8) & 0xf) << 8;
		dTmpBuf |= MemLogBin2Char((dPara >> 4) & 0xf) << 16;
		dTmpBuf |= MemLogBin2Char(dPara  & 0xf) << 24;
		HAL_WRITE32(_MemLogAddr_, dTmpBuf);
		HAL_WRITE32(_MemLogST_End_Addr_, _MemLogEndID_);
	}
	break;

	
	case(SI_RLOG_TYPE_STR): //print string to read only register
	{
		pStr = (UINT8*)dPara;
		i = 0;
		dTmpBuf = 0;
		isEnd = FALSE;
		HAL_WRITE32(_MemLogST_End_Addr_, _MemLogStartID_);
		while(TRUE) 
		{
			if(isEnd == TRUE) dTmpBuf |= ' ' << ((i & 3) << 3);
			else if(pStr[i]) dTmpBuf |= pStr[i] << ((i & 3) << 3);
			else 
			{
				dTmpBuf |= ' ' << ((i & 3) << 3);
				isEnd = TRUE;
			}

			i++;				
			if((i & 3) == 0) 
			{
				HAL_WRITE32(_MemLogAddr_, dTmpBuf);
				dTmpBuf = 0;

				if(isEnd == TRUE) 
				{
					HAL_WRITE32(_MemLogST_End_Addr_, _MemLogEndID_);
					break;
				}
			}
		}
	}
	break;

	
	default: break;
	}
return(TRUE);
}


void SIM_Printf(const CHAR *format,...)
{

  sprint(logBuf,(INT32 *)(&format));
  bSendLog_ROREG(SI_RLOG_TYPE_STR, (DWRD)(logBuf));  

}

#endif

