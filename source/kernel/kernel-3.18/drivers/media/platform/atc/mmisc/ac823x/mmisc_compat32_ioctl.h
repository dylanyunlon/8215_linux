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
#include <linux/compat.h>
#include <linux/module.h>
#if CONFIG_COMPAT

extern long mmiscdev_compat32_ioctl(struct file *file, unsigned int cmd, unsigned long arg);

#endif	/* CONFIG_COMPAT */
