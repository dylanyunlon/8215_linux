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

/** @file atc_hcd_cfg.c
 *  This C file implements the atc83xx USB host controller driver.
 */

//---------------------------------------------------------------------------
// Include files
//---------------------------------------------------------------------------
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/ioport.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/timer.h>
#include <linux/list.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/usb.h>
#include <linux/usb/quirks.h>

#include <asm/byteorder.h>
#include <asm/io.h>
#include <asm/irq.h>
//#include <asm/system.h>
#include <asm/unaligned.h>
#include <asm/uaccess.h>

#include <linux/dma-mapping.h>
#include <linux/random.h>
#include <linux/proc_fs.h>
#include <linux/string.h>
#include <linux/ctype.h> 
#include <linux/version.h>

#if defined(UNIFIED_USB)
#include <mach/ic_version.h>
#include <mach/hardware.h>
#include <mach/cache_operation.h>
#endif


MODULE_DESCRIPTION("ATC 83XX USB Host Controller Driver");
MODULE_LICENSE("GPL");

//---------------------------------------------------------------------------
// Configurations
//---------------------------------------------------------------------------
#define DRIVER_VERSION    "12 Nov 2009"

//---------------------------------------------------------------------------
// Constant definitions
//---------------------------------------------------------------------------
#ifdef USB_SINGLE_PORT
    #define MUC_NUM_PLATFORM_DEV (1)
#else 
    #define MUC_NUM_PLATFORM_DEV (2)
#endif  

#define USB_READ_WRITE_TEST   1

#define USB_AUTOK_SLEWRATE  0

#ifdef USB_READ_WRITE_TEST
#define USB_DMA_BUFFER_SIZE     (64*1024)
#endif

#define EPRX (0)
#define EPTX (1)
#define EP0 (0)
//---------------------------------------------------------------------------
// Type definitions
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Macro definitions
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Imported variables
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Imported functions
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Static function forward declarations
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
// Static variables
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// Static functions
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------
int MUC_usb_config_suspend(void)
{
#ifdef  CONFIG_USB_SUSPEND
  return (1);
#else
  return (0);
#endif  
}

