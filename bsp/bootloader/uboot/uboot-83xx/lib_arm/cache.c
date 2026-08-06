/*
 * (C) Copyright 2002
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/* for now: just dummy functions to satisfy the linker */

#include <common.h>


void Flush_Cache(unsigned int u4Start, unsigned int u4Len);
void Flush_Invalid_Cache(unsigned int u4Start, unsigned int u4Len);

void  flush_cache (unsigned int dummy1, unsigned int dummy2)
{
#ifdef CONFIG_OMAP2420
	void arm1136_cache_flush(void);

	arm1136_cache_flush();
#endif
	Flush_Cache(dummy1, dummy2);
	return;
}

void flush_invalid_cache (unsigned int dummy1, unsigned int dummy2)
{
	Flush_Invalid_Cache(dummy1, dummy2);
}
