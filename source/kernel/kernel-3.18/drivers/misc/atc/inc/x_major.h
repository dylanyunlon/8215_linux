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


#ifndef _X_MAJOR_H_
#define _X_MAJOR_H_


/*

    For the 1st parameter (major number) of register_chrdev, please use a
    number ranging from 240 to 254.  These are reserved for local/experimental
    use and won't conflict with other devices even if we change kernel
    versions.  Please see

        http://www.lanana.org/docs/device-list/devices-2.6+.txt

    for a list of offiical device major number registries.

*/
#define INFLATE_MAJOR        237

#define DUALCORE_MAJOR        239
#define TZ_INTF_MAJOR        238
#define SLT_MAJOR       239
#define BSP_MAJOR       240
#define OSAI_MAJOR      241
#define CBAGENT_MAJOR   242
#define RM_MAJOR        243
#define CLI_MAJOR       244
#define CHIP_CAP_MAJOR        245
#define WA_MAJOR        246
#define USR_STATIC_MEM_MAJOR        247

#define GPIO_MAJOR      248
#define I2C_MAJOR       249

/* major number 250 is used by Fusion (www.directfb.org).  Please do not use it. */

#define UART2_MAJOR     251
#define DBG_MAJOR       252
#define NAT_MAJOR       253
#define CIPHSV_MAJOR    255
#define KMEM_MAJOR      256


#endif /* _X_MAJOR_H_ */

