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
#ifndef S_MEM_FUNC_H
#define S_MEM_FUNC_H

//#include "section.h"

#if CONFIG_SECTION_BUILD
 /*-----------------------------------------------------------------------------
 * Name             - x_s_memmove
 * Description      - copies n bytes from memory area s2 to memory  area s1.
 *                    The memory areas may overlap.It can be used in secure/non-secure world
 * Input arguments  - s1 : destination
 *                  - s2 : source
 *                  - n :  n bytes
 * Output arguments - none 
 * Returns          -  returns a pointer to dest
 *---------------------------------------------------------------------------*/
extern void *x_s_memmove(void *s1,const void *s2, size_t n);

 /*-----------------------------------------------------------------------------
 * Name             - x_s_memset
 * Description      - fills  the  first  n  bytes of the memory area
                      pointed to by s with the constant byte c.
 * Input arguments  - s : point to the memory
 *                  - c : filled constant
 *                  - n : fill n bytes
 * Output arguments - none 
 * Returns          - returns a pointer to the memory area s.
 *---------------------------------------------------------------------------*/
extern void *x_s_memset(void *s,int c, size_t n); 

 /*-----------------------------------------------------------------------------
 * Name             - x_s_memset
 * Description      - copies n bytes from memory area s2 to memory area s1.
                      The memory areas may not overlap
 * Input arguments  - s1 : destination
 *                  - s2 : source
 *                  - n :  n bytes
 * Output arguments - none 
 * Returns          - returns a pointer to the memory area s.
 *---------------------------------------------------------------------------*/
extern void *x_s_memcpy(void *s1, const void *s2, size_t n);
#endif

#endif

