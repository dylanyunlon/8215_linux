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

#ifndef _X_MODUTIL_H_
#define _X_MODUTIL_H_

#include "u_common.h"

#define DRV_SPLITTER_MODULE_FLAG  (1 << 0)
#define SYS_NAT_MODULE_FLAG       (1 << 1)
#define DRV_NTFS_MODULE_FLAG         (1 << 2)
#define DRV_UDF_MODULE_FLAG  (1 << 3)
#define DRV_SR_MODULE_FLAG  (1 << 4)
#define DRV_SATA_MODULE_FLAG  (1 << 5)
#define DRV_LIBATA_MODULE_FLAG  (1 << 6)
#define DRV_CDROM_MODULE_FLAG  (1 << 7)

extern int insert_kernel_module(const char *filepath, const char *filename, const char *options, UINT32 u4ModuleFlag);
extern int remove_kernel_module(const char *module, UINT32 u4ModuleFlag);

#endif /* _X_MODUTIL_H_ */

