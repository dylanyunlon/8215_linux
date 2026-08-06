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

#include "x_typedef.h"
#include "u_uerrcode.h"

#ifndef _DRV_KM_ERRCODE_H_
#define _DRV_KM_ERRCODE_H_

/* success message 0xA600XXXX */
#define S_KM_OK                     UOKCODE(DRL_MODULE_KM, 0)

/* error message 0xE600XXXX */
#define E_KM_UNEXPECT               UERRCODE(DRL_MODULE_KM, 0x0000)
#define E_KM_INVALID_HANDLE         UERRCODE(DRL_MODULE_KM, 0x0001)
#define E_KM_PARAM_WRONG            UERRCODE(DRL_MODULE_KM, 0x0002)
#define E_KM_OS_OPERA_FAIL          UERRCODE(DRL_MODULE_KM, 0x0003)
#define E_KM_NO_INIT                UERRCODE(DRL_MODULE_KM, 0x0004)
#define E_KM_OVER_LIMIT             UERRCODE(DRL_MODULE_KM, 0x0005)
#define E_KM_OPERATE_FORBID         UERRCODE(DRL_MODULE_KM, 0x0006)
#define E_KM_NO_MEM                 UERRCODE(DRL_MODULE_KM, 0x0007)
#define E_KM_CMD_FAIL               UERRCODE(DRL_MODULE_KM, 0x0008)
#define E_KM_UNIMPLEMENT            UERRCODE(DRL_MODULE_KM, 0x0009)
#define E_KM_NOT_EXIST              UERRCODE(DRL_MODULE_KM, 0x000A)
#define E_KM_MODE0_ERR              UERRCODE(DRL_MODULE_KM, 0x000B)
#define E_KM_MODE2_ERR              UERRCODE(DRL_MODULE_KM, 0x000C)
#define E_KM_RAW_READ_FAIL          UERRCODE(DRL_MODULE_KM, 0x000D)
#define E_KM_RAW_WRITE_FAIL         UERRCODE(DRL_MODULE_KM, 0x000E)
#define E_KM_RAW_PATH_FAIL          UERRCODE(DRL_MODULE_KM, 0x000F)
#define E_KM_UPG_KB_VERIFY_FAIL     UERRCODE(DRL_MODULE_KM, 0x0010)
#define E_KM_UPG_KB_TYPE_ERR        UERRCODE(DRL_MODULE_KM, 0x0011)

#define E_KM_UPG_KB_0               UERRCODE(DRL_MODULE_KM, 0x0020)
#define E_KM_UPG_KB_1               UERRCODE(DRL_MODULE_KM, 0x0021)
#define E_KM_UPG_KB_2               UERRCODE(DRL_MODULE_KM, 0x0022)
#define E_KM_UPG_KB_3               UERRCODE(DRL_MODULE_KM, 0x0023)
#define E_KM_UPG_KB_4               UERRCODE(DRL_MODULE_KM, 0x0024)

#define E_KM_UPG_KB_DEC0            UERRCODE(DRL_MODULE_KM, 0x0030)
#define E_KM_UPG_KB_DEC1            UERRCODE(DRL_MODULE_KM, 0x0031)
#define E_KM_UPG_KB_DEC2            UERRCODE(DRL_MODULE_KM, 0x0032)
#define E_KM_UPG_KB_DEC3            UERRCODE(DRL_MODULE_KM, 0x0033)
#define E_KM_UPG_KB_DEC4            UERRCODE(DRL_MODULE_KM, 0x0034)
#define E_KM_UPG_KB_DEC5            UERRCODE(DRL_MODULE_KM, 0x0035)
#define E_KM_UPG_KB_DEC6            UERRCODE(DRL_MODULE_KM, 0x0036)

#define E_KM_UPG_KB_TX0             UERRCODE(DRL_MODULE_KM, 0x0040)
#define E_KM_UPG_KB_TX1             UERRCODE(DRL_MODULE_KM, 0x0041)
#define E_KM_UPG_KB_TX2             UERRCODE(DRL_MODULE_KM, 0x0042)
#define E_KM_UPG_KB_TX3             UERRCODE(DRL_MODULE_KM, 0x0043)
#define E_KM_UPG_KB_TX4             UERRCODE(DRL_MODULE_KM, 0x0044)
#define E_KM_UPG_KB_TX5             UERRCODE(DRL_MODULE_KM, 0x0045)

#define E_KM_UPG_KB_TXCHIPK0        UERRCODE(DRL_MODULE_KM, 0x0050)

#define E_KM_UPG_KB_TXKBI0          UERRCODE(DRL_MODULE_KM, 0x0060)
#define E_KM_UPG_KB_TXKBI1          UERRCODE(DRL_MODULE_KM, 0x0061)
#define E_KM_UPG_KB_TXKBI2          UERRCODE(DRL_MODULE_KM, 0x0062)
#define E_KM_UPG_KB_TXKBI3          UERRCODE(DRL_MODULE_KM, 0x0063)

#define E_KM_UPG_KB_NVM0            UERRCODE(DRL_MODULE_KM, 0x0070)
#define E_KM_UPG_KB_NVM1            UERRCODE(DRL_MODULE_KM, 0x0071)
#define E_KM_UPG_KB_NVM2            UERRCODE(DRL_MODULE_KM, 0x0072)

#endif /* #ifndef _DRV_KM_ERRCODE_H_ */
