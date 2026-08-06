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
#include "windows.h"
#include "x_os.h"
#include "x_common.h"
#include "x_cli.h"
#include "cli.h"
#include "_cli.h"

#include <linux/types.h>

#include "winutil.h"

#include "btdrv.h"

#define _ttoi(a)   simple_strtol(a, NULL, 10)



static s32 _CLI_BTSniffer(s32 i4Argc, const s8 **szArgv)
{
	u32 cmd = 0;
	u32 arg1 = BT_CAPTURE_CFG_NUM;
	BT_CLI_CFG cli_cfg = {0};

	if (i4Argc < 2) {
		pr_err(TEXT("[cli_bt]_CLI_BTSniffer:Parameter Error.\r\n"));
		return 0;
	}

	cmd = _ttoi(szArgv[1]);
	switch (cmd) {
	case BT_CAPTURE_MODE:
	case BT_CAPTURE_FLUSH_TIME:
	case BT_CAPTURE_BUFFER_SIZE:
		if (i4Argc == 2) {
			/* get capture config */
			cli_cfg.eBtCliType = BT_CLI_CMD_Get_Capture_CFG;
			cli_cfg.itemId = cmd;
			bt_cli_cfg(&cli_cfg);
			pr_info(TEXT("[cli_bt]_CLI_BTSniffer:get %d (%d)\n"),
					cmd, cli_cfg.bt_cfg.capture[cmd]);
		} else if (i4Argc == 3) {
			/* set capture config */
			cli_cfg.eBtCliType = BT_CLI_CMD_Set_Capture_CFG;
			cli_cfg.itemId = cmd;
			cli_cfg.bt_cfg.capture[cmd] = _ttoi(szArgv[2]);
			bt_cli_cfg(&cli_cfg);
			pr_info(TEXT("[cli_bt]_CLI_BTSniffer:set %d (%d)\n"),
					cmd, cli_cfg.bt_cfg.capture[cmd]);
		}
		break;

	default:
		break;
	}

	return 0;
}

static s32 _CLI_BTMac(s32 i4Argc, const s8 **szArgv)
{
	BT_CLI_CFG cli_cfg = {0};
	int i = 0;

	switch (i4Argc) {
	case 1:
		/* get bt mac address */
		cli_cfg.eBtCliType = BT_CLI_CMD_Get_MAC_CFG;
		bt_cli_cfg(&cli_cfg);
		pr_info(TEXT("[cli_bt]_CLI_BTMac:get(%02x:%02x:%02x:%02x:%02x:%02x)\n"),
			cli_cfg.bt_cfg.mac_id[0], cli_cfg.bt_cfg.mac_id[1],
			cli_cfg.bt_cfg.mac_id[2], cli_cfg.bt_cfg.mac_id[3],
			cli_cfg.bt_cfg.mac_id[4], cli_cfg.bt_cfg.mac_id[5]);
		break;

	case 7:
		/* set bt mac address */
		cli_cfg.eBtCliType = BT_CLI_CMD_Set_MAC_CFG;
		for (i = 0; i < 6; i++) {
			sscanf(szArgv[1+i], "%x", &cli_cfg.bt_cfg.mac_id[i]);
		}
		bt_cli_cfg(&cli_cfg);
		pr_info(TEXT("[cli_bt]_CLI_BTMac:set(%02x:%02x:%02x:%02x:%02x:%02x)\n"),
			cli_cfg.bt_cfg.mac_id[0], cli_cfg.bt_cfg.mac_id[1],
			cli_cfg.bt_cfg.mac_id[2], cli_cfg.bt_cfg.mac_id[3],
			cli_cfg.bt_cfg.mac_id[4], cli_cfg.bt_cfg.mac_id[5]);
		break;

	default:
		break;
	}

	return 0;
}

static s32 _CLI_BTLog(s32 i4Argc, const s8 **szArgv)
{
	pr_info(TEXT("[cli_bt]_CLI_BTLog:%d-%s\n"), i4Argc, *szArgv);
	return 0;
}

CLI_EXEC_T _arBTSnifferTbl[] = {
	{
		TEXT("Sniffer"),
		TEXT("cfa"),
		_CLI_BTSniffer,
		NULL,
		TEXT("BT Sniffer control(0:Capture, 1:Flush time, 2:Buffer size)"),
		CLI_GUEST
	},
	{
		TEXT("BT MAC"),
		TEXT("mac"),
		_CLI_BTMac,
		NULL,
		TEXT("BT MAC address"),
		CLI_GUEST
	},

	/* last cli command record, NULL */
	{
		NULL, NULL, NULL, NULL, NULL, CLI_SUPERVISOR
	}
};

CLI_EXEC_T _arBTCfgTbl[] = {
	{
		TEXT("BT Log"),
		TEXT("log"),
		_CLI_BTLog,
		NULL,
		TEXT("BT debug log(0:GAP,1:HFP,2:PBAP,3:A2DP,4:AVRCP,5:HID)"),
		CLI_GUEST
	},

	/* last cli command record, NULL */
	{
		NULL, NULL, NULL, NULL, NULL, CLI_SUPERVISOR
	}
};


/******************************************************************************
* CLI Commond for Bluetooth
******************************************************************************/
CLI_EXEC_T _arBluetoothCmdTbl[] = {
	{
		TEXT("BT Sniffer"),
		TEXT("sniffer"),
		NULL,
		_arBTSnifferTbl,
		TEXT("BT Sniffer control"),
		CLI_GUEST
	},
	{
		TEXT("BT Config"),
		TEXT("btcfg"),
		NULL,
		_arBTCfgTbl,
		TEXT("BT Config"),
		CLI_GUEST
	},
	/* last cli command record, NULL */
	{
		NULL, NULL, NULL, NULL, NULL, CLI_SUPERVISOR
	}
};

