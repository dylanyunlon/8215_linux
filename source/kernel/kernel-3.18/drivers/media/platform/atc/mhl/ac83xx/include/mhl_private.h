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

#ifndef _MHL_PRIVATE_H_
#define _MHL_PRIVATE_H_

#define Printf printk

/* typedef  unsigned int HAL_TIME_T; */

/* #define ACP_TYPE_GENERAL_AUDIO  0 */

/* #define Printf  printk */
/* #define ACP_LOST_DISABLE 0 */


#define CRYSTAL 0

#define DDCCI_EDID1_ENABLE              0x608
#define DDCCI_EDID_DOWNLOAD_MODE  0x61c
#define DDCCI_EDID_DATA                       0x650
#define DDCCI_EDID0_DATA_REMAP                       0x660
#define DDCCI_EDID1_DATA_REMAP                       0x668

#define DDCCI_EDID0_ENABLE              0x600
#define DDCCI_EDID_CHECKSUM              0x604
#define DDCCI_EDID0_CHECKSUM              0x604
#define DDCCI_EDID1_CHECKSUM              0x60C

/*#define REG_RW_CLK_CFG4        0x008C    //Clock Selection Configuration 4 : HDMI rx */

/* #define VECTOR_HDMIRXINT 62 */

#define VECTOR_INT_P_CBUS_SRCEN 0
/*#define PDWNC_WRITE8(offset, value)               IO_WRITE8(PDWNC_BASE, offset, (value))
#define PDWNC_WRITE16(offset, value)              IO_WRITE16(PDWNC_BASE, offset, (value))
#define PDWNC_WRITE32(offset, value)              IO_WRITE32(PDWNC_BASE, offset, (value))*/




#define CONFIG_DRV_CUSTOM_JXF 0

#define CONFIG_DRV_CUSTOM_JSN 0

#define CONFIG_DRV_CUSTOM_SFLP 0

#define CONFIG_FASTBOOT_MULTI_PHASE_INIT_DRIVER_EN 0

#define HDMI_RX_IGNORE_ACP_PACKET 0

#define VECTOR_HDMIRXINT 0

#define DRV_SUPPORT_HDMI_RX_POWEROFF 0

#define CONFIG_DRV_HDMI_SUPPORT_HDCP_BDS_X80 0

#define PIN_HTPLG_RX 0

#define PIN_HTPLG_RX2 0

#define CONFIG_DRV_OPPO_SUPPORT 0

#define CONFIG_DRV_CUSTOM_CBBG 0


#endif

