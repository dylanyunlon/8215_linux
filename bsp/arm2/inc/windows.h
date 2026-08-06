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
#ifndef __WINDOWS_H__
#define __WINDOWS_H__

#ifndef __KERNEL__
#error "This header file can only be used in driver"
#endif

#if defined(MODULE) || defined(__MODULE__)

#include "windev.h"
//#include "types.h"
#include "linux/string.h"
#include "linux/kernel.h"
#include "linux/slab.h"
#include "linux/delay.h"
//#include "x_queue.h"
#include "x_typedef.h"
#include <linux/vmalloc.h>

#define WIN32 


#define FAILED(x) ( (x)<0 )
#define TEXT(x)  x 
#define _T(x)  x

#define RETAILMSG(fg, x... )  \
do { \
    if (fg) \
        printk x ;\
} while ( 0 )


#define DEBUGMSG(fg, x... )  \
do { \
    if (fg) \
        printk x ;\
} while ( 0 )
#define Sleep  msleep 


#define LPTR 0x0040 
static inline void* LocalAlloc(unsigned int flag , size_t size)
{
	//return kzalloc(size, GFP_KERNEL);
	void *p = vmalloc( size );
	if ( p )
	{
		memset( p , 0 , size );
	}
	return p;
}

static inline void* LocalFree(void *p)
{
	//kfree(p);
	if ( p )
	{
		vfree( p );
	}
	return NULL;
}


/*
#include "ose_mem.h"
static inline BOOL OSE_MemInit(OSE_MEM_REGION eMemRegion)
{
	return TRUE;
}

static VOID OSE_MemUninit(OSE_MEM_REGION eMemRegion)
{
	return ;
}
*/

#endif 

#endif

