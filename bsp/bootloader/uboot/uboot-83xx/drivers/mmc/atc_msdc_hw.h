/********************************************************************************************
 *     LEGAL DISCLAIMER
 *
 *     (Header of Autochips Software/Firmware Release or Documentation)
 *
 *     BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *     THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("AUTOCHIPS SOFTWARE") RECEIVED
 *     FROM AUTOCHIPS AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS
 *     ONLY. AUTOCHIPS EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED,
 *     INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
 *     A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES AUTOCHIPS PROVIDE ANY WARRANTY
 *     WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE AUTOCHIPS SOFTWARE, AND BUYER AGREES TO LOOK
 *     ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. AUTOCHIPS SHALL ALSO
 *     NOT BE RESPONSIBLE FOR ANY AUTOCHIPS SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION
 *     OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 *     BUYER'S SOLE AND EXCLUSIVE REMEDY AND AUTOCHIPS'S ENTIRE AND CUMULATIVE LIABILITY WITH
 *     RESPECT TO THE AUTOCHIPS SOFTWARE RELEASED HEREUNDER WILL BE, AT AUTOCHIPS'S OPTION,
 *     TO REVISE OR REPLACE THE AUTOCHIPS SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO AUTOCHIPS FOR SUCH AUTOCHIPS SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/

#ifndef _ATC_MSDC_HW_ACCESS_H_
#define _ATC_MSDC_HW_ACCESS_H_

//---------------------------------------------------------------------------
// Include files
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Type definitions
//---------------------------------------------------------------------------
static inline unsigned int uffs(unsigned int x)
{
    unsigned int r = 1;

    if (!x)
        return 0;
    if (!(x & 0xffff)) {
        x >>= 16;
        r += 16;
    }
    if (!(x & 0xff)) {
        x >>= 8;
        r += 8;
    }
    if (!(x & 0xf)) {
        x >>= 4;
        r += 4;
    }
    if (!(x & 3)) {
        x >>= 2;
        r += 2;
    }
    if (!(x & 1)) {
        x >>= 1;
        r += 1;
    }
    return r;
}

/**
* Base address definition
*/
#define IO_BASE						0xF0000000
#define ATC_MSDC0_BASE				(IO_BASE + 0x0B000)
#define ATC_MSDC1_BASE              (IO_BASE + 0x21000)
#define ATC_MSDC2_BASE              (IO_BASE + 0x0A000)

#define MSDC_WRITE32(addr, value)   	(*(volatile unsigned int *)(addr)) = (value)
#define MSDC_READ32(addr)           	(*(volatile unsigned int *)(addr))
#define MSDC_SETBIT(addr, dBit)        	MSDC_WRITE32(addr, MSDC_READ32(addr) | (dBit))
#define MSDC_CLRBIT(addr, dBit)        	MSDC_WRITE32(addr, MSDC_READ32(addr) & (~(dBit)))


#define MSDC_SET_FIELD(addr, field, val) \
    do {	\
        volatile unsigned int tv = MSDC_READ32(addr);	\
        tv &= ~(field); \
        tv |= ((val) << (uffs((unsigned int)field) - 1)); \
        MSDC_WRITE32(addr, tv); \
    } while(0)
    
#define MSDC_GET_FIELD(addr, field, val) \
    do {	\
        volatile unsigned int tv = MSDC_READ32(addr);	\
        val = ((tv & (field)) >> (uffs((unsigned int)field) - 1)); \
    } while(0)
//---------------------------------------------------------------------------
// Inter-file functions
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Public functions
//---------------------------------------------------------------------------


#endif // _ATC_MSDC_HW_ACCESS_H_