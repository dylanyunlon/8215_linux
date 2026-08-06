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
/*-----------------------------------------------------------------------------
 *
 * Description:
 *
 *---------------------------------------------------------------------------*/

#ifndef X_ASSERT_H
#define X_ASSERT_H

#ifndef __ARM2__
#include "x_typedef.h"
#else
#include "x_types.h"
#endif
//EXTERN void Assert(const CHAR* szExpress, const CHAR* szFile, INT32 i4Line);

#ifndef NDEBUG

  #if defined(__KERNEL__)  && defined(__linux__)
  #include "linux/kernel.h"
  #define ASSERT(s) do{if (!(s)) panic( "%s:%d line=>%s", __FILE__, __LINE__,  #s);} while(0)
  #define VERIFY(s) do{if (!(s)) panic( "%s:%d line=>%s", __FILE__, __LINE__,  #s);} while(0)
  #endif 

#else	// NDEBUG

  #ifdef ASSERT
  #undef ASSERT
  #endif

  #ifdef VERIFY
  #undef VERIFY
  #endif

  #define ASSERT(x)		do { \
		if (!(x))	\
			RETAILMSG(1, (TEXT("DBGCHK Failed: %s, %d!\r\n"), TEXT(__FILE__), (INT32)__LINE__)); \
	} while (0)
	
  #define VERIFY(x)		do { \
		if (!(x))	\
			RETAILMSG(1, (TEXT("DBGCHK Failed: %s, %d!\r\n"), TEXT(__FILE__), (INT32)__LINE__)); \
	} while (0)
#endif	// NDEBUG

#endif	// X_ASSERT_H
