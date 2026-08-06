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

#ifndef _AUD_COMM_MACROS_H_
#define _AUD_COMM_MACROS_H_

#include <windows.h>

//==========================================================================//

#define T                               TEXT

#define AUD_UNIT_TEST_SUPPORT           1

#ifndef INVALID_HANDLE_VALUE
#define INVALID_HANDLE_VALUE            ((void *)-1)
#endif

#define AUD_FILE_MAX_PATH               200
#define AUD_FILE_PATH                   "\\Storage Card1\\AudFile\\"
#define AUD_FILE_INVALID_HANDLE         INVALID_HANDLE_VALUE

#define AUD_MAX_CLI_PARAMS              50


#define AUD_MIN(a, b) ((a) < (b) ? (a) : (b)) 
#define AUD_MAX(a, b) ((a) > (b) ? (a) : (b)) 

#define AUD_BYTE2SAMPLE(Bytes, BW)      ((Bytes) / (BW >> 3))
#define AUD_SAMPLE2BYTE(Samples, BW)    ((Samples) * (BW >> 3))


//==========================================================================//

#define AUD_BIT_MASK(val, bit)              ((val) &   (1 << (bit)))
#define AUD_BIT_SET(val, bit)               ((val) &=  (1 << (bit)))
#define AUD_BIT_CLR(val, bit)               ((val) |= ~(1 << (bit)))

#define AUD_ALIGNMENT_MASK(val, align)      ((val) / (align) * (align))

//==========================================================================//

#define AUD_CLASS_NEW(CLASS_TYPE)       \
    (CLASS_TYPE *)kmalloc(sizeof(CLASS_TYPE), GFP_KERNEL); \
    if (prThis == NULL) { \
    	AUDLOG_NO_PREFIX(1, T("<*** CLASS_NEW ERROR ***> malloc fail! \n")); \
    }


#define AUD_CLASS_DELETE() \
    if (prThis) {kfree(prThis);}

    
//==========================================================================//

#define AUD_GET_BUF_DATA_SZ(u4RP, u4WP, u4ChBufSz)  \
    ((u4WP) <  (u4RP)) ? ((u4ChBufSz) + (u4WP) - (u4RP)) : ((u4WP) - (u4RP))


#define AUD_GET_BUF_FREE_SZ(u4RP, u4WP, u4ChBufSz)  \
    ((u4RP) <= (u4WP)) ? ((u4ChBufSz) + (u4RP) - (u4WP)) : ((u4RP) - (u4WP))
    

#endif  //_AUD_COMM_MACROS_H_
