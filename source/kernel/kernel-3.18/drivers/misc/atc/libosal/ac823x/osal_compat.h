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


#ifndef __OSAL_COMPAT_H__
#define __OSAL_COMPAT_H__
 
#ifdef CONFIG_COMPAT
long OsalDev_IOControl_Compat(struct file *file, unsigned int cmd, unsigned long arg);
#endif    //CONFIG_COMPAT

#endif	

