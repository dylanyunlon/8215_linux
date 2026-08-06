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

#ifndef __WINDOWS_H__
#define __WINDOWS_H__

#ifndef __KERNEL__
#error "This header file can only be used in driver"
#endif

//#if defined(MODULE) || defined(__MODULE__)

#include "windev.h"
#include "types.h"
#include "linux/string.h"
#include "linux/kernel.h"
#include "linux/slab.h"
#include "linux/delay.h"

#include <linux/vmalloc.h>

#define WIN32 


#define FAILED(x) ( (x)<0 )
#define TEXT(x)  x   //here do not use (x) to instead, or will cause call printk error
#define _T(x)  x

#define RETAILMSG(fg, x... )  \
do { \
    if (fg) \
        printk x; \
} while (0)


#define DEBUGMSG(fg, x... )  \
do { \
    if (fg) \
        printk x ;\
} while (0)
#define Sleep  msleep 


#define LPTR 0x0040 
static inline void* LocalAlloc(unsigned int flag , size_t size)
{
	void *p = vmalloc( (unsigned long)size );
	if (p != NULL)
		memset(p, 0, size);

	return p;
}

static inline void* LocalFree(const void *p)
{
	if (p != NULL)
		vfree(p);

	return NULL;
}

//#endif 

#endif

