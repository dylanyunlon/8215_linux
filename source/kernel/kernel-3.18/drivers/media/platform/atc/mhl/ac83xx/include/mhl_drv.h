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

#ifndef _MHL_DRV_H_
#define _MHL_DRV_H_
#include "mhl_rx_cbus_hw.h"


extern unsigned int hdmi_rx_log_on;
#define hdmierr         (0x01)
#define hdmiwrn         (0x02)
#define hdmidbg         (0x04)

#define hdmialllog   (0xffff)


#define hlog_e(fmt, arg...) \
			do { \
				if (hdmi_rx_log_on&hdmierr) { \
					HDMI_LOG(HDMI_LOG_ERROR, "[h_e] "); \
					HDMI_LOG(HDMI_LOG_ERROR, fmt, ##arg); } \
			} while (0)

#define hlog_w(fmt, arg...) \
			do { \
				if (hdmi_rx_log_on&hdmiwrn) { \
					HDMI_LOG(HDMI_LOG_DEBUG, "[h_w] "); \
					HDMI_LOG(HDMI_LOG_DEBUG, fmt, ##arg); } \
			} while (0)

#define hlog_d(fmt, arg...) \
			do { \
				if (hdmi_rx_log_on&hdmidbg) { \
					HDMI_LOG(HDMI_LOG_DEBUG, "[h_d] "); \
					HDMI_LOG(HDMI_LOG_DEBUG, fmt, ##arg); } \
			} while (0)

#define BASE_READ32(offset) (*(volatile unsigned int *)((g_IO_VBASE_VA + (offset))))
#define BASE_WRITE32(offset, val)    ((*(volatile unsigned int *)(g_IO_VBASE_VA + (offset))) = val)

#define PRINT_REG(offset) \
			do { \
				unsigned int val = (g_IO_VBASE_VA + offset); \
				HDMI_LOG(HDMI_LOG_DEBUG, "addr: 0x%08x, val: 0x%08x", \
				val, BASE_READ32((offset))); \
			} while (0)

#define BIT_SET(offset, bit)        BASE_WRITE32(offset, (BASE_READ32(offset)|(1<<bit)))
#define BIT_CLR(offset, bit)        BASE_WRITE32(offset, (BASE_READ32(offset)&(~(1<<bit))))
#define BIT_VAL(offset, bit)        (BASE_READ32((offset)) & (1<<(bit)))

#define CBUS_BIT_SET(offset, bit)  BIT_SET((IO_BASE_MHL_SINK_CBUS_OFFSET + (offset)), (bit))
#define CBUS_BIT_CLR(offset, bit)  BIT_CLR((IO_BASE_MHL_SINK_CBUS_OFFSET + (offset)), (bit))
#define CBUS_BIT_VAL(offset, bit)  (BIT_VAL((IO_BASE_MHL_SINK_CBUS_OFFSET + (offset)), (bit)))

#define ANA_BIT_SET(offset, bit)  BIT_SET((HDMI_ANA_REG_BASE_OFFSET+ (offset)), (bit))
#define ANA_BIT_CLR(offset, bit)  BIT_CLR((HDMI_ANA_REG_BASE_OFFSET + (offset)), (bit))
#define ANA_BIT_VAL(offset, bit)  (BIT_VAL((HDMI_ANA_REG_BASE_OFFSET + (offset)), (bit)))


#define vRxWriteReg(dAddr, dVal) \
	{*(unsigned int *)(g_IO_VBASE_VA + dAddr) = dVal; } /* IO_WRITE32(IO_BASE, dAddr, dVal)
			(volatile DWRD *)(IO_BASE_ADDRESS + HDMI_SYS_REG_OFFSET + dAddr) = dVal */
#define vRxReadReg(dAddr) (*(volatile unsigned int*)(g_IO_VBASE_VA + dAddr))
#define vRxWriteRegMsk(dAddr, dVal, dMsk) vRxWriteReg((dAddr), (vRxReadReg(dAddr) & (~(dMsk))) | ((dVal) & (dMsk)))

#define vANAWriteReg(dAddr, dVal) \
	{*(unsigned int *)(g_IO_VBASE_VA + HDMI_ANA_REG_BASE_OFFSET + dAddr) = dVal; }
#define vANAReadReg(dAddr) (*(volatile unsigned int *)(g_IO_VBASE_VA + HDMI_ANA_REG_BASE_OFFSET + dAddr))
#define vANAWriteRegMsk(dAddr, dVal, dMsk) vANAWriteReg((dAddr), (vANAReadReg(dAddr) & (~(dMsk))) | ((dVal) & (dMsk)))
#endif


