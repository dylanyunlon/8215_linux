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
#include <linux/string.h>
#else // __KERNEL__
#include <string.h>
#endif // __KERNEL__

#include <linux/types.h>



void *memrchr(const void *s, char c, size_t n)
{
    char *pui1_mem;

    //pui1_mem = ((const char *)s) + n - 1;
    pui1_mem = &(((char *)s)[n - 1]);

    while (n--)
    {
        if (*pui1_mem == c)
        {
            return (void *)pui1_mem;
        }

        pui1_mem--;
    }

    return NULL;
}



