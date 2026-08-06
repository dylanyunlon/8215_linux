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


/*****************************************************************************
*  Audio Driver: Interface
*****************************************************************************/

#ifndef _MEMDBGCPP_H_
#define _MEMDBGCPP_H_

#include "memchk_cfg.h"

void *operator  new(size_t size, char *file, int line);
void operator delete(void *p, char *name, int line);
void *operator new[] (size_t size, char *file, int line);
void operator delete[] (void *p, char *name, int line);


#if MEMCHK_DEBUG

template < class Temp >
void Destroy(Temp * P)
{
	P->~Temp();
};

#define MM_NEW new((char *)__FILE__, __LINE__)
#define MM_DELETE(x)	 {Destroy(x); operator delete(x , (char *)__FILE__, __LINE__); }
#define MM_DELETE_V(x)	 {Destroy(x); operator delete[](x , (char *)__FILE__, __LINE__); }
#else
#define MM_NEW new
#define MM_DELETE delete	/* (x)   {Destroy(x); operator delete(x); } */
#define MM_DELETE_V delete[]	/* (x)       {Destroy(x); operator delete[](x); } */

#endif				/* MEMDBG_CHECK */

#endif				/* _MEMDBGCPP_H_ */
