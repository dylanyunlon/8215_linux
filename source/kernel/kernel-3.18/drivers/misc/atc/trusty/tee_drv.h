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

#include <linux/types.h>

struct resvd_mem_info {
	phys_addr_t  base;
	void        *virt_addr;
	phys_addr_t  size;
	
};

extern struct resvd_mem_info tz_share_rsv_mem;

extern int LoadHDCPKeyToSRAM(unsigned char* data, unsigned int len);