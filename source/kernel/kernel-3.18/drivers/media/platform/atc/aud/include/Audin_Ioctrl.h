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

#include "x_aud_dec.h"
#include "ose_mem.h"

#if 0 // mtk40043
#ifndef MT3360_AFIFO_VA
#define MT3360_DSP_WORKING_BUF		OSE_GetADSPStartAddr()    //74M
#define MT3360_AFIFO_VA             OSE_GetAFIFOStartAddr()
#endif



#define AUDIN_DEV_NAME                L"AIN1:"


#define AUDIN_IOCTRL_ID_START         0x1000

#define DEFINE_AUD_IOCTRL(ID)                       \
    CTL_CODE(FILE_DEVICE_UNKNOWN, ID, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_AUDIN_SET_ONOFF                      \
    DEFINE_AUD_IOCTRL(AUDIN_IOCTRL_ID_START + 0x00)

#define IOCTL_AUDIN_SET_ADDR                  \
    DEFINE_AUD_IOCTRL(AUDIN_IOCTRL_ID_START + 0x01)

#define IOCTL_AUDIN_GET_DEC_DATALEN                     \
    DEFINE_AUD_IOCTRL(AUDIN_IOCTRL_ID_START + 0x02)

#define IOCTL_AUDIN_GET_DEC_CFG_INFO                     \
		DEFINE_AUD_IOCTRL(AUDIN_IOCTRL_ID_START + 0x03)

#define IOCTL_AUDIN_REAR_VOL_GAIN_INFO       \
	DEFINE_AUD_IOCTRL(AUDIN_IOCTRL_ID_START + 0x04)

#define IOCTL_AUDIN_FRONT_VOL_GAIN_INFO   \
	DEFINE_AUD_IOCTRL(AUDIN_IOCTRL_ID_START + 0x05)

#endif


