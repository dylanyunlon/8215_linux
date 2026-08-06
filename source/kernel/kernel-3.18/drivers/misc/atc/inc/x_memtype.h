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

/* These macros are to be declared in pointers in middleware/driver interfaces,
   to indicate the nature of the memory pointed by this pointer.

  Example: (rm_set parameter)

    typedef struct
    {
        UINT32 abc[32];  // Arrays don't need memory type macros
        UINT8* __cross_space__ pu1buf0;   // The driver can access addr_user_to_kernel(_pu1buf0) directly
        struct SOME_STRUCT_T* __local_space__ ptSome;  // The driver must do copy_from_user to access the content of this structure.
        VOID* __opaque__ pvTag; // pvTag is used for driver to pass it back to middleware.  Driver won't check the contents of pvTag.
    } DRV_FOO_SET_INFO_T;

*/


/* Add this macro to indicate that the memory pointed by this pointer is allocated by x_mem_alloc or static allocation.
   This memory buffer, after calling addr_user_to_kernel or addr_kernel_to_user, is accessible in the other space. */
#define __cross_space__

/* Add this macro to indicate that the memory pointed by this pointer is a local or global variable, or is allocated by new or malloc.
   This memory buffer is not accessible in the the other space unless doing copy_from_user or copy_to_user. */
#define __local_space__

/* Add this macro to indicate that the memory pointed by this pointer will not be dereferenced in the other space.
   Therefore, it doesn't matter how it's allocated. */
#define __opaque__

