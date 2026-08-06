/*
* Copyright (c) 2021 AutoChips Inc.
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

// this header file is must include after wi_left.h

#include <linux/version.h>

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0)
#pragma GCC diagnostic pop
#endif
