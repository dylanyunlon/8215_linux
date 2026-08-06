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

#ifndef _MEMCHK_CFG_H_
#define _MEMCHK_CFG_H_

#include <linux/types.h>

#ifdef __cplusplus
extern "C" {

#endif	/* 
 */

#define MEMCHK_DEBUG 0

#define MEMDBG_MODE_NAME									 "MemManager"
#define MEMDBG_VER_MAJOR									 01
#define MEMDBG_VER_MINOR									 00
#define MEMDBG_VER_REV										 001

typedef struct _alloc_node {
	struct _alloc_node *lptr;
	struct _alloc_node *rptr;
	void    *pvAddr;
	size_t	len;
	char		file[256];
	__u32	  line;
} alloc_node;

static const __u8 g_guard[] = {
	0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD, 0xBE, 0xEF,
	0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD, 0xBE, 0xEF
};

/* Old C header file */
#ifdef __cplusplus
}
#endif	/* 
 */

#endif /* _MEMCHK_CFG_H_*/

