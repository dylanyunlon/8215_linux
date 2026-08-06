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

#ifndef __X_UTIL_H__
#define __X_UTIL_H__

#include "x_typedef.h"

typedef enum {
    eNO_TYPE,
    eRD_ONLY,
    eWR_ONLY,
    eRD_WR
} REGTYPE;

typedef struct strucRegTest {
    UINT32     u4Addr;		// the register offset
    REGTYPE    eRegType;
    INT32      iRegLen;		// 1, 2, or 4; number of byte
	UINT32     u4Mask;		// the valid bit mask
	INT32      fgWDfV;		// set 0 if without reset value.
	UINT32     u4DfVal;		// the reset value.
} REG_TEST_T;


/* Log export function. */
extern INT32 UTIL_LogThreadInit(void);
extern INT32 UTIL_Log(const CHAR *szFmt, ...);

/* Cli util functions. */
extern INT32 UTIL_MemOrder(UINT32 u4Addr, UINT32 u4ByteUnit, UINT32 u4Len, UINT32 u4Step);
extern INT32 UTIL_MemCmp(UINT32 u4Addr1, UINT32 u4Addr2, UINT32 u4Len);

/* Reg test functions. */
INT32 UTIL_AllSpaceRWTest(UINT32 u4BaseAddr, UINT32 u4Length);
INT32 UTIL_RegDefChk(UINT32 u4BaseAddr, REG_TEST_T *psrgtRegList);
INT32 UTIL_RegRWTest(UINT32 u4BaseAddr, REG_TEST_T *psrgtRegList);


#endif /* __X_UTIL_H__ */

