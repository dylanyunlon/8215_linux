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

/* #include "x_typedef.h" */
#include <linux/types.h>

#ifndef _DRV_IMGRESZ_ERRCODE_H_
#define _DRV_IMGRESZ_ERRCODE_H_

#define DRL_MODULE_IMGRESZ                      0x73L   ///< Image resizer


#define IMGRESZ_OKCODE(modid, okid)           \
  ((s32)                             \
    ((s32)(0x00000000) |             \
     (s32)((modid & 0x7f) << 24) |   \
     (s32)(okid & 0xffff))           \
  )


#define IMGRESZ_ERRCODE(modid, errid)         \
  ((s32)                             \
    ((s32)(0x80000000) |             \
     (s32)((modid & 0x7f) << 24) |   \
     (s32)(errid & 0xffff))          \
  )


#define IMGRESZ_DRV_UOKCODE(group, okcode)  \
    (                                       \
        IMGRESZ_OKCODE(DRL_MODULE_IMGRESZ,         \
            ((u32)((group & 0xff) << 8) | \
            (u32)(okcode & 0xff)))        \
    )


#define IMGRESZ_DRV_UERRCODE(group, errcode)\
    (                                       \
        IMGRESZ_ERRCODE(DRL_MODULE_IMGRESZ,        \
            ((u32)((group & 0xff) << 8) | \
            (u32)(errcode & 0xff)))       \
    )


#define IMGRESZ_DRV_GROUP_GENERAL         0


#define S_IMGRESZ_DRV_OK                      IMGRESZ_DRV_UOKCODE(IMGRESZ_DRV_GROUP_GENERAL,0x00)
#define S_IMGRESZ_DRV_RESIZE_FINISH           IMGRESZ_DRV_UOKCODE(IMGRESZ_DRV_GROUP_GENERAL,0x01)
#define S_IMGRESZ_DRV_RESIZE_STOP             IMGRESZ_DRV_UOKCODE(IMGRESZ_DRV_GROUP_GENERAL,0x02)
#define S_IMGRESZ_DRV_RESIZE_PIC_MODE_READY   IMGRESZ_DRV_UOKCODE(IMGRESZ_DRV_GROUP_GENERAL,0x03)

#define E_IMGRESZ_DRV_FAIL                    IMGRESZ_DRV_UERRCODE(IMGRESZ_DRV_GROUP_GENERAL,0x00)
#define E_IMGRESZ_DRV_LOCK_FAIL               IMGRESZ_DRV_UERRCODE(IMGRESZ_DRV_GROUP_GENERAL,0x01)
#define E_IMGRESZ_DRV_RESIZE_TIMEOUT          IMGRESZ_DRV_UERRCODE(IMGRESZ_DRV_GROUP_GENERAL,0x02)


#endif // #ifndef _DRV_IMGRESZ_ERRCODE_H_


