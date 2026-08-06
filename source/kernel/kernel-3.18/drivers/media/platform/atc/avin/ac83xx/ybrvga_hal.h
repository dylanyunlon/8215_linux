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

#ifndef YBRVGA_HAL_H_
#define YBRVGA_HAL_H_

#include <linux/module.h>
#include <linux/init.h>
#include <linux/types.h>

enum VIDE_SIGNAL_STATUS_LIST/* Video Signal Status*/
{
    SV_VDO_NOSIGNAL        = 0,
    SV_VDO_NOSUPPORT,
    SV_VDO_UNKNOWN,        /*Still doing mode detect*/
    SV_VDO_STABLE
};

int ybrvga_init_video(int index);
int ybrvga_set_auto(void );
int ybrvga_select_video(int index);
int ybrvga_start_video(int index) ;
int ybrvga_stop_video(void);
//extern WCH_SRC_APP_ID_E    mSrcAppId;
//extern WCH_CFG_T mWchCfg;
//extern YBR_VGA_CFG  mYbrVgaCfg;
//extern int mWidth;
//extern int mHeight;

#endif
