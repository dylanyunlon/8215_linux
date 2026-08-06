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

#ifndef _ARM2SYSTEM_SERVICE_H_
#define _ARM2SYSTEM_SERVICE_H_

typedef enum arm2system_service_msg
{
    ARM2SYSTEM_SERVICE_HEARTBEAT,
    ARM2SYSTEM_SERVICE_KERNEL_PANIC,
    ARM2SYSTEM_SERVICE_HEARTBEAT_START,
    ARM2SYSTEM_SERVICE_REBOOT,
    ARM2SYSTEM_SERVICE_AWTK_START,
    ARM2SYSTEM_SERVICE_DISPLAY_VSYNC,
	ARM2SYSTEM_SERVIC_SHUTDOWN,
	ARM2SYSTEM_SERVICE_MAX,
}arm2system_service_msg_t;

#endif				/* AEE_COMMON_H */
