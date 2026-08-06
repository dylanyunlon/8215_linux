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

#ifndef __AUD_OAL_H__
#define __AUD_OAL_H__


#ifdef __linux__

#include <linux/mm.h>
#include <linux/fs.h>
#include <linux/stat.h>
#include <linux/module.h>
#include <linux/spinlock_types.h>
#include <asm/uaccess.h>
#include "x_assert.h"
#include "x_rtos.h"
#include "x_os.h"

#if CONFIG_DRV_AUD_AC83XX
#include <mach/83xx_irqs_vector.h>
#else
#include "mt3365_irqs_vector.h"
#endif
#include "winutil.h"
#include "windows.h"
#include "stc_hal.h"
#include "audiosys.h"
#include "aud_comm_os.h"


#define x_snwprintf     Aud_snprintf

#define AUD_ENABLE_UART_LOG(fgmode)                 \
{                                                   \
    return (TRUE);                                  \
}

#define AUD_BONDING_SUPPOR(bIsSupport)

#define AUD_SPRINTF(a, b, c)            snprintf(a, b, c)
#define AUD_STRLEN(a)                   strlen(a)
#define AUD_VSPRINTF(a, b, c, d)        vsnprintf(a, b, c, d)

#define ENTERCRITICALSECTION(lock, flags)           \
    spin_lock_irqsave(lock, flags)

#define LEAVECRITICALSECTION(lock, flags)           \
    spin_unlock_irqrestore(lock, flags)

#define InitializeCriticalSection(lock)
#define DeleteCriticalSection(lock)

#else // WinCE

#include "x_os.h"
#include <windows.h>
#include <msgqueue.h>
#include <ceddk.h>
#include <nkintr.h>
#include <pnp.h>
#include <guiddef.h>
#include <pm.h>
#include <oal.h>
#include <initguid.h>
#include <gpio.h>
#include <kfuncs.h>
#include <windev.h>
#include "ioctl_cfg.h"
#include "../aud/waveform/Audiosys.h"


#define AUD_ENABLE_UART_LOG(fgmode)                                                             \
{                                                                                               \
    if (!KernelIoControl(IOCTL_HAL_ENABLE_UART_LOG, &fgmode, sizeof(bool), NULL, 0, NULL))      \
    {                                                                                           \
        LOG(LOG_FEATURE, ("AudCliUartSwitch() IOCTL_HAL_ENABLE_UART_LOG call failed\r\n"));     \
        return FALSE;                                                                           \
    }                                                                                           \
    LOG(LOG_CTRLF, ("Uart switch ok! \r\n"));                                                   \
    return TRUE;                                                                                \
}

#define AUD_BONDING_SUPPOR(bIsSupport)                                                          \
{                                                                                               \
    u32 u4InputBufSize = sizeof(u32);                                                     \
    u32 u4OutputBufSize = sizeof(bool);                                                      \
    u32 u4RetSize = 0;                                                                       \
    if (!KernelIoControl(IOCTL_HAL_GET_CHIP_FEATURE,&u4FuncID,u4InputBufSize,                   \
                          &bIsSupport,u4OutputBufSize,(LPu32)&u4RetSize))                     \
    {                                                                                           \
        return FALSE;                                                                           \
    }                                                                                           \
}

#define AUD_SPRINTF(a, b, c)        swprintf_s(a, b, c)
#define AUD_STRLEN(a)               wcslen(a)
#define AUD_VSPRINTF(a, b, c, d)    vswprintf_s(a, b, c, d)

#define ENTERCRITICALSECTION(lock, flags)           \
    EnterCriticalSection(lock)

#define LEAVECRITICALSECTION(lock, flags)           \
    LeaveCriticalSection(lock)

#endif // #ifdef __linux__

#endif /* __AUD_OAL_H__ */

