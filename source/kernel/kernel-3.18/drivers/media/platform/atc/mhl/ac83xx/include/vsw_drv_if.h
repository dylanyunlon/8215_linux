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

#ifndef VSW_DRV_IF_H
#define VSW_DRV_IF_H

#include "x_typedef.h"

#define VSW_RET_OK 0

typedef enum {
	VSW_COMP_INT_HDMIRX = 0,
	VSW_COMP_EXT_HDMIRX,
	VSW_COMP_EXT_TVD

} VSW_HANDLE_COMP_ID_T;

typedef enum {
	VSW_COMP_NFY_ERROR = -1,
	VSW_COMP_NFY_LOCK = 1,
	VSW_COMP_NFY_UNLOCK,
	VSW_COMP_NFY_RESOLUTION_CHGING,
	VSW_COMP_NFY_RESOLUTION_CHG_DONE,
	VSW_COMP_NFY_ASPECT_CHG,
	VSW_COMP_NFY_COLOR_SPACE_CHG,
	VSW_COMP_NFY_JPEG_CHG,
	VSW_COMP_NFY_CINEMA_CHG
} VSW_NFY_COND_T;

typedef struct _VSW_NFY_INFO_T {
	VSW_HANDLE_COMP_ID_T  eCompId;
	VSW_NFY_COND_T eNfyCond;

} VSW_NFY_INFO_T;

/* Nfy function for HDMIRX/TVD */
UINT32 u4Vsw_SetCompNfy(void *pt_nfy_info);

#endif
