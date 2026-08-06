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



#ifdef __KERNEL__
#include <linux/kernel.h>
#define restrict
#define strtoull simple_strtoull
#else // __KERNEL__
#include <stdlib.h>
#endif // __KERNEL__


long long strtoll(const char *restrict str, char **restrict endptr, int base)
{
    if (*str == '-')
    {
        return -(long long)strtoull(str + 1, endptr, base);
    }
    return (long long)strtoull(str, endptr, base);
}

