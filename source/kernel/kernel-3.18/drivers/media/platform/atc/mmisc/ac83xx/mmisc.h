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

#ifndef MMISC_H
#define MMISC_H

#include <linux/types.h>

/* Success, return 0, otherwise, return -1 */
int mm_copy_from_user(void *dest, const void *src, unsigned long count);

/* Success, return 0, otherwise, return -1 */
int mm_copy_to_user(void *dest, const void *src, unsigned long count);

#endif				/*  */
