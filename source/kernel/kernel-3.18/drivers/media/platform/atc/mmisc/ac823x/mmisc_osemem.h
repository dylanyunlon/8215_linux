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

#ifndef MMISC_OSEMEM_H
#define MMISC_OSEMEM_H

#include <linux/types.h>

struct res_mem_info {
	phys_addr_t phys_addr;
	void *virt_addr;
	phys_addr_t size;
};

extern struct res_mem_info *g_rsvmem_info;

bool OSE_Init(void);
void OSE_Uninit(void);


#endif				/* MMISC_OSEMEM_H */
