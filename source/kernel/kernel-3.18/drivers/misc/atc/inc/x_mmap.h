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

#ifndef X_MMAP_H
#define X_MMAP_H


//#include <stddef.h>


//extern void reserve_static_memory(int channel, void **address, size_t *size);
//extern void *init_static_memory(const char *name, int channel, size_t size, size_t align);

#define DEFINE_CHANNEL1_MEMORY_AREA(NAME, TYPE, SIZE, ALIGN) \
    TYPE *NAME;

#define DEFINE_CHANNEL2_MEMORY_AREA(NAME, TYPE, SIZE, ALIGN) \
    TYPE *NAME;

#define RESERVE_CHANNEL1_MEMORY_AREA(NAME, TYPE, SIZE, ALIGN) \
    RESERVE_CHANNEL1_MEMORY_AREA_FRONT(NAME, TYPE, SIZE, ALIGN)

#define RESERVE_CHANNEL2_MEMORY_AREA(NAME, TYPE, SIZE, ALIGN) \
    RESERVE_CHANNEL2_MEMORY_AREA_FRONT(NAME, TYPE, SIZE, ALIGN)

#define RESERVE_CHANNEL1_MEMORY_AREA_FRONT(NAME, TYPE, SIZE, ALIGN) \
    { "+" #NAME, 0, sizeof(TYPE) * SIZE, ALIGN },

#define RESERVE_CHANNEL2_MEMORY_AREA_FRONT(NAME, TYPE, SIZE, ALIGN) \
    { "+" #NAME, 1, sizeof(TYPE) * SIZE, ALIGN },

#endif	// X_MMAP_H

