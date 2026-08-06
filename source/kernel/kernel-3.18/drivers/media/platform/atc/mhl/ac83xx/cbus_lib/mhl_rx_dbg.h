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

#ifndef _MHL_DBG_H_
#define _MHL_DBG_H_
#if 0/*DRV_SUPPORT_MHL_RX*/
#include <hdmi_debug.h>
extern size_t mhl_rx_log_on;

#define hdmicbuslog         (0x01)
#define hdmicbustxrx         (0x02)
#define hdmicbusint         (0x04)
#define hdmicbuserr         (0x08)

#define hdmialllog   (0xffff)

/*******************************************DRV************************/
#define SINK_CBUS_LOG(fmt, arg...) \
	do { \
		if (mhl_rx_log_on&hdmicbuslog) { \
			HDMI_LOG(HDMI_LOG_DEBUG, "[mhls] "); \
			HDMI_LOG(HDMI_LOG_DEBUG, fmt, ##arg); } \
	} while (0)
#define SINK_CBUS_ERR(fmt, arg...) \
			do { \
				if (mhl_rx_log_on&hdmicbuserr) { \
					HDMI_LOG(HDMI_LOG_DEBUG, "[mhls_err] "); \
					HDMI_LOG(HDMI_LOG_DEBUG, fmt, ##arg); } \
			} while (0)
#define SINK_CBUS_TXRX(fmt, arg...) \
			do { \
				if (mhl_rx_log_on&hdmicbustxrx) { \
					HDMI_LOG(HDMI_LOG_DEBUG, "[mhls_tr] "); \
					HDMI_LOG(HDMI_LOG_DEBUG, fmt, ##arg); } \
			} while (0)
#define SINK_CBUS_INT(fmt, arg...) \
			do { \
				if (mhl_rx_log_on&hdmicbusint) { \
					HDMI_LOG(HDMI_LOG_DEBUG, "[mhls_int] "); \
					HDMI_LOG(HDMI_LOG_DEBUG, fmt, ##arg); } \
			} while (0)
#define SINK_CBUS_FUNC()    \
	do { \
		if (mhl_rx_log_on&hdmicbuslog) { \
			HDMI_LOG(HDMI_LOG_DEBUG, "[mhls] %s\n", __func__); } \
	} while (0)

#endif
#endif
