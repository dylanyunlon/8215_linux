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

#include <linux/err.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/input.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <linux/clk.h>
#include <linux/io.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/delay.h>
#include <asm/irq.h>
#include <linux/timer.h>
#include <linux/module.h>

#include "mhl_private.h"
#include "types.h"
#include "x_typedef.h"
#include "x_os.h"
#include "x_printf.h"
#include "x_stl_lib.h"
#include "x_assert.h"

#include "drv_thread.h"
#include "x_timer.h"
#include "x_bim.h"

/* #include "x_gpio.h"*/
/* #include "d_drv_cus_model.h"*/
/* #include "x_bsp.h"*/
/* #include "pm.h"*/
#include "mhl_drv_if.h"

#include "mhl_rx_cbus.h"
#include "mhl_rx_cbus_hw.h"
#include "mhl_rx_dbg.h"
#include<hdmi_debug.h>
#include "mhl_rx_cbus_ctrl.h"
#include "hdmi_rx_ctrl.h"
/* #include "drv_ir.h"*/
#include "drv_config.h"
#include "hdmi_rx_hw.h"

#if 1

SINK_CBUS_LINK_STATE sink_link_state     = SINK_CBUS_LINK_FLOAT_CBUS;
SINK_CBUS_LINK_STATE sink_link_state_bak = 0xFF;
SINK_DEVICE_CONTROL_ST st_dev_ctrl;
SINK_TEST_MODE sink_test_mode;


#define MHL_PHONE_LIST_MAX  6

static MHL_PHONE_CAP dev_cap_list[MHL_PHONE_LIST_MAX] = {
	{"HONOR6",      false,    {0x00, 0x20, 0x02, 0x01, 0x42, 0x3F, 0x03, 0x00, 0x8E,
		0x00, 0x07, 0x87, 0x88, 0x10, 0x33, 0x00} },
	{"samsung s3",  true,     {0x00, 0x11, 0x02, 0x01, 0x41, 0x03, 0x03, 0x00, 0x80,
	0x0F, 0x07, 0x00, 0x00, 0x10, 0x33, 0x00} },
	{"sony L3",     true,     {0x00, 0x12, 0x02, 0x03, 0xA7, 0x31, 0x01, 0x00, 0x80,
	0x00, 0x07, 0x04, 0x04, 0x10, 0x33, 0x00} },
	{"xiaomi 2",    false,    {0x00, 0x11, 0x02, 0x01, 0x42, 0x01, 0x01, 0x00, 0x82,
	0x0f, 0x07, 0x92, 0x44, 0x10, 0x33, 0x00} },
	{"samsung s4",  true,     {0x00, 0x20, 0x12, 0x01, 0x41, 0x09, 0x01, 0x00, 0x80,
	0x0f, 0x07, 0x00, 0x00, 0x10, 0x33, 0x00} },
	{"none",        false,    {0} },
};

BOOL readDevcapDone = FALSE;
int sink_get_phoneid_by_devcap(void)
{
	UINT32 i = 0, j = 0;

	if (!readDevcapDone)
		return -1;

	for (; i < MHL_PHONE_LIST_MAX; i++) {
		for (j = 0; j < 16; j++) {
			if (dev_cap_list[i].devCap[j] != src_dev_reg[j])
				break;
			else if (j == 15)
				return i;
		}
	}

	return -1;
}


BOOL sink_fg_phone_support_hdcp(void)
{
	int idx = -1;

	if (!readDevcapDone || !IsMhlMode()) {
		HDMI_LOG(HDMI_LOG_DEBUG, "*");
		return TRUE;
	}

	idx = sink_get_phoneid_by_devcap();

	if (idx == -1 || idx >= MHL_PHONE_LIST_MAX) {
		HDMI_LOG(HDMI_LOG_DEBUG, ".");
		return TRUE;
	}

	return dev_cap_list[idx].fgHdcp;
}

void sink_read_DeviceCaps(UINT32 u4Index)
{
	stSinkCbusDbgRead.fgIsValid = TRUE;
	stSinkCbusDbgRead.u1Val = u4Index;
}


/*******************************************************

 device

********************************************************/
void vSinkCbusInitDevice(void)
{
	memset(my_dev_reg, 0, SINK_CBUS_DEVICE_LENGTH);
	memset(src_dev_reg, 0, SINK_CBUS_DEVICE_LENGTH);
	/* init my device*/
#if 1
#if 0
	my_dev_reg[0] = 0x00; /*  0: DEV_STATE       x */
	my_dev_reg[1] = 0x21; /*  1: MHL_VERSION     -*/
	my_dev_reg[2] = 0x31; /*  2: DEV_CAT         - POW[4], DEV_TYPE[3:0], SINK Imin=900mA*/
	my_dev_reg[3] = 0x00; /*  3: ADOPTER_ID_H*/
	my_dev_reg[4] = 0x00; /*  4: ADOPTER_ID_L*/
	my_dev_reg[5] = 0x0F; /*  5: VID_LINK_MODE   - SUPP_VGA[5], SUPP_ISLANDS[4],
	SUPP_PPIXEL[3], SUPP_YCBCR422[2], SUPP_YCBCR444[1], SUPP_RGB444[0]*/
	my_dev_reg[6] = 0x03; /*  6: AUD_LINK_MODE   - AUD_8CH[1], AUD_2CH[0]*/
	my_dev_reg[7] = 0x00;/*  7: VIDEO_TYPE       x SUPP_VT[7], VT_GAME[3],
	VT_CINEMA[2], VT_PHOTO[1], VT_GRAPHICS[0]*/
	my_dev_reg[8] = 0x81; /*  8: LOG_DEV_MAP     - LD_GUI[7], LD_SPEAKER[6],
	LD_RECORD[5], LD_TUNER[4], LD_MEDIA[3], LD_AUDIO[2], LD_VIDEO[1], LD_DISPLAY[0]*/
	my_dev_reg[9] = 0x0F;/*  9: BANDWIDTH        x*/
	my_dev_reg[10] = 0x07; /*  A: FEATURE_FLAG   - UCP_RECV_SUPPORT[4],
	UCP_SEND_SUPPORT[3], SP_SUPPORT[2], RAP_SUPPORT[1], RCP_SUPPORT[0]*/
	my_dev_reg[11] = 0x00;/*  B: DEVICE_ID_H     -*/
	my_dev_reg[12] = 0x00; /*  C: DEVICE_ID_L        -*/
	my_dev_reg[13] = 0x10; /*  D: SCRATCHPAD_SIZE    -*/
	my_dev_reg[14] = 0x33; /*  E: INT_STAT_SIZE  - STAT_SIZE[7:4], INT_SIZE[3:0]*/
	my_dev_reg[15] = 0x00; /*  F: Reserved*/
	my_dev_reg[SINK_CBUS_MSC_STATUS_CONNECTED_RDY] = SINK_CBUS_MSC_STATUS_CONNECTED_RDY_DCAP_RDY;
	my_dev_reg[SINK_CBUS_MSC_STATUS_LINK_MODE] = SINK_CBUS_MSC_STATUS_LINK_MODE_CLK_MODE__Normal;
#endif

	my_dev_reg[0] = 0x00; /*  0: DEV_STATE       x   at spec 146 mhl 2.0*/
	my_dev_reg[1] = 0x21; /*  1: MHL_VERSION     -*/
	my_dev_reg[2] = 0x31; /*  2: DEV_CAT         - POW[4], DEV_TYPE[3:0], SINK Imin=900mA*/
	my_dev_reg[3] = 0x01; /*  3: ADOPTER_ID_H*/
	my_dev_reg[4] = 0x01; /*  4: ADOPTER_ID_L*/
	my_dev_reg[5] = 0x3F; /*  5: VID_LINK_MODE   - SUPP_VGA[5], SUPP_ISLANDS[4],
	SUPP_PPIXEL[3], SUPP_YCBCR422[2], SUPP_YCBCR444[1], SUPP_RGB444[0]*/
	my_dev_reg[6] = 0x03; /*  6: AUD_LINK_MODE   - AUD_8CH[1], AUD_2CH[0]*/
	my_dev_reg[7] = 0x8f;/*  7: VIDEO_TYPE       x SUPP_VT[7], VT_GAME[3],
	VT_CINEMA[2], VT_PHOTO[1], VT_GRAPHICS[0]*/
	my_dev_reg[8] = 0x81; /*  8: LOG_DEV_MAP     - LD_GUI[7], LD_SPEAKER[6],
	LD_RECORD[5], LD_TUNER[4], LD_MEDIA[3], LD_AUDIO[2], LD_VIDEO[1], LD_DISPLAY[0]*/
	my_dev_reg[9] = 0x0F;/*  9: BANDWIDTH        x*/
	my_dev_reg[10] = 0x17; /*  A: FEATURE_FLAG   - UCP_RECV_SUPPORT[4],
	UCP_SEND_SUPPORT[3], SP_SUPPORT[2], RAP_SUPPORT[1], RCP_SUPPORT[0]*/
	my_dev_reg[11] = 0x00;/*  B: DEVICE_ID_H     -*/
	my_dev_reg[12] = 0x01; /*  C: DEVICE_ID_L        -*/
	my_dev_reg[13] = 0x10; /*  D: SCRATCHPAD_SIZE    -*/
	my_dev_reg[14] = 0x33; /*  E: INT_STAT_SIZE  - STAT_SIZE[7:4], INT_SIZE[3:0]*/
	my_dev_reg[15] = 0x00; /*  F: Reserved*/
	my_dev_reg[SINK_CBUS_MSC_STATUS_CONNECTED_RDY] = SINK_CBUS_MSC_STATUS_CONNECTED_RDY_DCAP_RDY;
	my_dev_reg[SINK_CBUS_MSC_STATUS_LINK_MODE] = SINK_CBUS_MSC_STATUS_LINK_MODE_CLK_MODE__Normal;

#else
	my_dev_reg[0] = 0x00; /*  0: DEV_STATE       x*/
	my_dev_reg[1] = 0x12; /*  1: MHL_VERSION     -*/
	my_dev_reg[2] = 0x31; /*  2: DEV_CAT         - POW[4], DEV_TYPE[3:0],SINK Imin=900mA*/
	my_dev_reg[3] = 0x00; /*  3: ADOPTER_ID_H*/
	my_dev_reg[4] = 0x00; /*  4: ADOPTER_ID_L*/
	my_dev_reg[5] = 0x01; /*  5: VID_LINK_MODE   - SUPP_VGA[5], SUPP_ISLANDS[4],
	SUPP_PPIXEL[3], SUPP_YCBCR422[2], SUPP_YCBCR444[1], SUPP_RGB444[0]*/
	my_dev_reg[6] = 0x03; /*  6: AUD_LINK_MODE   - AUD_8CH[1], AUD_2CH[0]*/
	my_dev_reg[7] = 0x00;/*  7: VIDEO_TYPE       x SUPP_VT[7], VT_GAME[3],
	VT_CINEMA[2], VT_PHOTO[1], VT_GRAPHICS[0]*/
	my_dev_reg[8] = 0x81; /*  8: LOG_DEV_MAP     - LD_GUI[7], LD_SPEAKER[6],
	LD_RECORD[5], LD_TUNER[4], LD_MEDIA[3], LD_AUDIO[2], LD_VIDEO[1], LD_DISPLAY[0]*/
	my_dev_reg[9] = 0x0F;/*  9: BANDWIDTH        x*/
	my_dev_reg[10] = 0x07; /*  A: FEATURE_FLAG   - UCP_RECV_SUPPORT[4],
	UCP_SEND_SUPPORT[3], SP_SUPPORT[2], RAP_SUPPORT[1], RCP_SUPPORT[0]*/
	my_dev_reg[11] = 0x00;/*  B: DEVICE_ID_H     -*/
	my_dev_reg[12] = 0x00; /*  C: DEVICE_ID_L        -*/
	my_dev_reg[13] = 0x10; /*  D: SCRATCHPAD_SIZE    -*/
	my_dev_reg[14] = 0x33; /*  E: INT_STAT_SIZE  - STAT_SIZE[7:4], INT_SIZE[3:0]*/
	my_dev_reg[15] = 0x00; /*  F: Reserved*/
	my_dev_reg[SINK_CBUS_MSC_STATUS_CONNECTED_RDY] = SINK_CBUS_MSC_STATUS_CONNECTED_RDY_DCAP_RDY;
	my_dev_reg[SINK_CBUS_MSC_STATUS_LINK_MODE] = SINK_CBUS_MSC_STATUS_LINK_MODE_CLK_MODE__Normal;
#endif

	/* init sink device*/
	src_dev_reg[0] = 0x00; /*  0: DEV_STATE      x*/
	src_dev_reg[1] = 0x10; /*  1: MHL_VERSION        -*/
	src_dev_reg[2] = 0x31; /*  2: DEV_CAT            - POW[4], DEV_TYPE[3:0],900mA*/
	src_dev_reg[3] = 0x00; /*  3: ADOPTER_ID_H*/
	src_dev_reg[4] = 0x00; /*  4: ADOPTER_ID_L*/
	src_dev_reg[5] = 0x00; /*  5: VID_LINK_MODE  - SUPP_VGA[5], SUPP_ISLANDS[4],
	SUPP_PPIXEL[3], SUPP_YCBCR422[2], SUPP_YCBCR444[1], SUPP_RGB444[0]*/
	src_dev_reg[6] = 0x01; /*  6: AUD_LINK_MODE  - AUD_8CH[1], AUD_2CH[0]*/
	src_dev_reg[7] = 0x00;/*  7: VIDEO_TYPE      x SUPP_VT[7], VT_GAME[3],
	VT_CINEMA[2], VT_PHOTO[1], VT_GRAPHICS[0]*/
	src_dev_reg[8] = 0x06; /*  8: LOG_DEV_MAP        - LD_GUI[7], LD_SPEAKER[6],
	LD_RECORD[5], LD_TUNER[4], LD_MEDIA[3], LD_AUDIO[2], LD_VIDEO[1], LD_DISPLAY[0]*/
	src_dev_reg[9] = 0x0F;/*  9: BANDWIDTH       x*/
	src_dev_reg[10] = 0x04; /*  A: FEATURE_FLAG  - UCP_RECV_SUPPORT[4],
	UCP_SEND_SUPPORT[3], SP_SUPPORT[2], RAP_SUPPORT[1], RCP_SUPPORT[0]*/
	src_dev_reg[11] = 0x00;/*  B: DEVICE_ID_H        -*/
	src_dev_reg[12] = 0x00; /*  C: DEVICE_ID_L       -*/
	src_dev_reg[13] = 0x00; /*  D: SCRATCHPAD_SIZE   -*/
	src_dev_reg[14] = 0x33; /*  E: INT_STAT_SIZE - STAT_SIZE[7:4], INT_SIZE[3:0]*/
	src_dev_reg[15] = 0x00; /*  F: Reserved*/

	memset(&st_dev_ctrl, 0, sizeof(SINK_DEVICE_CONTROL_ST));

}
unsigned char sink_3D_brust_info[] = {
	0x40,
	0x00,
	0x10,
	0xcb,
	0x0b,
	0x01,
	0x05,
	0x00,
	0x07,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x06,
	0x00,
	0x07
};

unsigned char sink_3D_brust_info1[] = {
	0x40,
	0x00,
	0x10,
	0xc0,
	0x0b,
	0x02,
	0x05,
	0x00,
	0x06,
	0x00,
	0x06,
	0x00,
	0x06,
	0x00,
	0x06,
	0x00,
	0x06
};

unsigned char sink_3D_brust_info2[] = {
	0x40 ,
	0x00 ,
	0x10 ,
	0xdb ,
	0x0b ,
	0x03 ,
	0x01 ,
	0x00 ,
	0x06
};

unsigned char sink_3D_brust_info3[] = {
	0x40,
	0x00,
	0x11,
	0xc5,
	0x06,
	0x01,
	0x05,
	0x00,
	0x06,
	0x00,
	0x06,
	0x00,
	0x06,
	0x00,
	0x06,
	0x00,
	0x06,

};


unsigned char sink_3D_brust_info4[] = {
	0x40,
	0x00,
	0x11,
	0xe0,
	0x06,
	0x02,
	0x01,
	0x00,
	0x06,
};



unsigned char sink_dscr_data[17] = {0x40, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};

BOOL is_sink_attached = FALSE;

void vSinkSetAttachStatus(BOOL en)
{
	is_sink_attached = en;

	if (is_sink_attached) {
		vSinkCbusInit();
		vSinkCbusInitDevice();
	}

	vSinkCbusSelectR(SINK_CBUS_R_FLOAT);
	vSinkCbusReset();

	if (is_sink_attached) {
		HDMI_LOG(HDMI_LOG_INFO, "[mhls]attached\n");
	} else {
		HDMI_LOG(HDMI_LOG_INFO, "[mhls]unattached\n");
	}

	SinkSetAttachMode(en);
}
BOOL fgSinkAttachSource(void)
{
	return is_sink_attached;
}
BOOL is_sink_hpd_on = FALSE;
EXPORT_SYMBOL(is_sink_hpd_on);

BOOL fgSinkHPDReady(void)
{
	return is_sink_hpd_on;
}
void vSinkTaskSetHPD(void)
{
	is_sink_hpd_on = TRUE;
}
EXPORT_SYMBOL(vSinkTaskSetHPD);
void vSinkTaskClrHPD(void)
{
	is_sink_hpd_on = TRUE;
}
EXPORT_SYMBOL(vSinkTaskClrHPD);
BOOL is_sink_cbus_stuch_low = FALSE;
void vSetSinkCbusStuchLow(void)
{
	is_sink_cbus_stuch_low = TRUE;
}
unsigned char sink_cbus_low_force = 0;
BOOL fgSinkCbusStuchLow(void)
{
	if (sink_cbus_low_force == 1)
		return FALSE;
	else
		return is_sink_cbus_stuch_low;
}
BOOL is_wakeup_err = FALSE;
void vSetSinkWakeupErr(void)
{
	is_wakeup_err = TRUE;
}
BOOL fgSinkWakeupErr(void)
{
	return is_wakeup_err;
}
BOOL is_discovery_ok = FALSE;
void vSetSinkDiscoveryOk(void)
{
	is_discovery_ok = TRUE;
}
BOOL fgSinkDiscoveryOk(void)
{
	return is_discovery_ok;
}
BOOL is_sink_mhl_standby = TRUE;
void vSinkTaskSetStandby(BOOL en)
{
	if (en) {
		HDMI_LOG(HDMI_LOG_DEBUG, "[mhl_rx]standby\n");
		/* SINK_CBUS_LOG("standby\n");*/
	} else {
		HDMI_LOG(HDMI_LOG_DEBUG, "un-standby\n");
		/* SINK_CBUS_LOG("un-standby\n");*/
	}

	is_sink_mhl_standby = en;
}
BOOL fgSinkStandby(void)
{
	return FALSE;
	/* return is_sink_mhl_standby;*/
}
void vSinkResetLink(void)
{
	is_sink_attached = FALSE;
	is_sink_cbus_stuch_low = FALSE;
	is_wakeup_err = FALSE;
	is_discovery_ok = FALSE;
}
BOOL fgIsSinkCbusConnected(void)
{
	if ((sink_link_state == SINK_CBUS_LINK_STATE_CONNECTED)
	    || (sink_link_state == SINK_CBUS_LINK_STATE_CONTENT_ON))
		return TRUE;

	return FALSE;
}
BOOL fgIsSinkCbusContentOn(void)
{
	if (sink_link_state == SINK_CBUS_LINK_STATE_CONTENT_ON)
		return TRUE;

	return FALSE;
}
#define SINK_KEYCODE_NUM  128
const UINT32 sink_keycode_map[SINK_KEYCODE_NUM] = {
#if 0        /*Ke Xu*/
	/*00*/ BTN_SELECT,
	/*01*/ BTN_CURSOR_UP,
	/*02*/ BTN_CURSOR_DOWN,
	/*03*/ BTN_CURSOR_LEFT,
	/*04*/ BTN_CURSOR_RIGHT,
	/*0x05~0x08*/BTN_NONE, BTN_NONE, BTN_NONE, BTN_NONE,
	/*09*/ BTN_MENU,
	/*0x0A~0x0C*/BTN_NONE, BTN_NONE, BTN_NONE,
	/*0D*/ BTN_EXIT,
	/*0x0E~0x0F*/BTN_NONE, BTN_NONE,

	/*0x10~0x1F*/BTN_NONE, BTN_NONE, BTN_NONE, BTN_NONE, BTN_NONE, BTN_NONE, BTN_NONE,
	BTN_NONE, BTN_NONE, BTN_NONE, BTN_NONE, BTN_NONE, BTN_NONE, BTN_NONE, BTN_NONE, BTN_NONE,

	/*20*/ BTN_DIGIT_0,
	/*21*/ BTN_DIGIT_1,
	/*22*/ BTN_DIGIT_2,
	/*23*/ BTN_DIGIT_3,
	/*24*/ BTN_DIGIT_4,
	/*25*/ BTN_DIGIT_5,
	/*26*/ BTN_DIGIT_6,
	/*27*/ BTN_DIGIT_7,
	/*28*/ BTN_DIGIT_8,
	/*29*/ BTN_DIGIT_9,
	/*2A*/ BTN_NONE,
	/*2B*/ BTN_PLAY_ENTER,
	/*2C*/ BTN_CLEAR,
	/*0D~0F*/BTN_NONE, BTN_NONE, BTN_NONE,

	/*30*/ BTN_PRG_UP,
	/*31*/ BTN_PRG_DOWN,
	/*32*/ BTN_PREV_PRG,
	/*33~3F*/BTN_NONE, BTN_NONE, BTN_NONE, BTN_NONE, BTN_NONE, BTN_NONE, BTN_NONE,
	BTN_NONE, BTN_NONE, BTN_NONE, BTN_NONE, BTN_NONE, BTN_NONE,

	/*40~43*/BTN_NONE, BTN_NONE, BTN_NONE, BTN_NONE,
	/*44*/ BTN_PLAY,
	/*45*/ BTN_STOP,
	/*46*/ BTN_PAUSE,
	/*47*/ BTN_RECORD,
	/*48*/ BTN_FR,
	/*49*/ BTN_FF,
	/*4A*/ BTN_EJECT,
	/*4B*/ BTN_SF,
	/*4C*/ BTN_SR,
	/*4D~4F*/BTN_NONE, BTN_NONE, BTN_NONE,

	/*0x50~0x5F*/BTN_NONE, BTN_NONE, BTN_NONE, BTN_NONE, BTN_NONE, BTN_NONE, BTN_NONE,
	BTN_NONE, BTN_NONE, BTN_NONE, BTN_NONE, BTN_NONE, BTN_NONE, BTN_NONE, BTN_NONE, BTN_NONE,

	/*60*/ BTN_PLAY_PAUSE,
	/*61*/ BTN_PAUSE_STEP,
	/*62*/ BTN_STOP_RESUME,
	/*63~6F*/BTN_NONE, BTN_NONE, BTN_NONE, BTN_NONE, BTN_NONE, BTN_NONE, BTN_NONE, BTN_NONE,
	BTN_NONE, BTN_NONE, BTN_NONE, BTN_NONE, BTN_NONE,

	/*70~7F*/BTN_NONE, BTN_NONE, BTN_NONE, BTN_NONE, BTN_NONE, BTN_NONE, BTN_NONE,
	BTN_NONE, BTN_NONE, BTN_NONE, BTN_NONE, BTN_NONE, BTN_NONE, BTN_NONE, BTN_NONE, BTN_NONE
#endif
};
int sink_get_keycode_from_ir(UINT32 keycode)
{
	unsigned char cbus_keycode;

	/* SINK_CBUS_LOG("keycode input:%x\n",(unsigned int)keycode);*/
	for (cbus_keycode = 0; cbus_keycode < SINK_KEYCODE_NUM; cbus_keycode++) {
		if (sink_keycode_map[cbus_keycode] == keycode) {
			st_dev_ctrl.st_rcp.fgIsValid = TRUE;
			st_dev_ctrl.st_rcp.u2Code = SINK_MSC_MSG_RCP;
			st_dev_ctrl.st_rcp.u1Val = cbus_keycode;
			/* SINK_CBUS_LOG("keycode:%x,%x\n",(unsigned int)keycode,cbus_keycode);*/
			return 0;
		}
	}

	/* SINK_CBUS_LOG("no keycode:%x\n",(unsigned int)keycode);*/
	return -1;
}
void sink_send_keycode_to_ir(unsigned char keycode)
{
	UINT32 key;

	key = sink_keycode_map[keycode];
#if ((!CONFIG_DRV_ONLY) && (!CONFIG_DRV_VERIFY_SUPPORT))

	if (key != BTN_NONE)
		IRRX_SendMtkIr(key);

#endif
}
unsigned int sink_delay_hpd = 0;
void sink_delay_set_hpd(void)
{
	if (fgIsSinkCbusConnected() == TRUE) {
		if (st_dev_ctrl.st_hpd.fgHPDOn == FALSE) {
			st_dev_ctrl.st_hpd.fgHPDOn = TRUE;
			st_dev_ctrl.st_hpd.fgHPDChanged = TRUE;
			/* SINK_CBUS_LOG("delay set HPD\n");*/
		}
	}
}
void sink_discovery_ok(void)
{
	if (sink_link_state == SINK_CBUS_LINK_ATTACHED) {
		vSinkCbusInitDevice();
		vSinkCbusReset();
		vSinkCbusSelectR(SINK_CBUS_R_100K);
		sink_link_state = SINK_CBUS_LINK_STATE_CONNECTED;
		/* st_dev_ctrl.st_hpd.fgHPDOn = TRUE;*/
		/* st_dev_ctrl.st_hpd.fgHPDChanged = TRUE;*/
		sink_delay_hpd = 500 / MHL_LINK_TIME;
		HDMI_LOG(HDMI_LOG_DEBUG, "[mhls]cbus connected\r\n");
	} else
		HDMI_LOG(HDMI_LOG_DEBUG, "[mhls]cbus err connected\n");
}
void sink_cbus_low_err(void)
{
	if ((sink_link_state == SINK_CBUS_LINK_STATE_CONNECTED) ||
		(sink_link_state == SINK_CBUS_LINK_STATE_CONTENT_ON)) {
		vSinkCbusSelectR(SINK_CBUS_R_1K);
		sink_link_state = SINK_CBUS_LINK_ATTACHED;
	}
}
unsigned int sink_cbus_link_live = 0;
void vSinkLinkState(void)
{
	unsigned int tmp;

	sink_cbus_link_live++;

	if (u1ForceSinkCbusStop == 0xA5) {
		if (fgSinkCbusStop)
			return;
	}

	switch (sink_link_state) {

	case SINK_CBUS_LINK_UNATTACHED:
		HDMI_LOG(HDMI_LOG_DEBUG, "No attached");

		if (fgSinkStandby() == TRUE) {
			vSinkCbusSelectR(SINK_CBUS_R_FLOAT);
			sink_link_state = SINK_CBUS_LINK_POWER_OFF;
		}

		else if (fgSinkAttachSource() == TRUE) {
			msleep(100);/*mhl spec. (Tsink cbus float>=50ms)*/
			vSinkCbusSelectR(SINK_CBUS_R_1K);
			sink_link_state = SINK_CBUS_LINK_ATTACHED;
		}

		break;

	case SINK_CBUS_LINK_ATTACHED:
		HDMI_LOG(HDMI_LOG_DEBUG, "attached \r\n");

		if (fgSinkStandby() == TRUE) {
			vSinkCbusSelectR(SINK_CBUS_R_FLOAT);
			sink_link_state = SINK_CBUS_LINK_POWER_OFF;
		}

		else if (fgSinkAttachSource() == FALSE) {
			vSinkCbusSelectR(SINK_CBUS_R_FLOAT);
			sink_link_state = SINK_CBUS_LINK_UNATTACHED;
		}

		else if (fgSinkWakeupErr()) {
			sink_link_timer = 0;
			vSinkCbusSelectR(SINK_CBUS_R_FLOAT);
			sink_link_state = SINK_CBUS_LINK_FLOAT_CBUS;
		}

		/*
		else if(fgSinkDiscoveryOk())
		{
		sink_link_state = SINK_CBUS_LINK_STATE_CONNECTED;
		st_dev_ctrl.st_hpd.fgHPDOn = TRUE;
		st_dev_ctrl.st_hpd.fgHPDChanged = TRUE;
		printk("[mhls]cbus connected\n");
		}*/
		break;

	case SINK_CBUS_LINK_STATE_CONNECTED:

		/* printk("connected");*/
		if (fgSinkStandby() == TRUE) {
			vSinkCbusSelectR(SINK_CBUS_R_FLOAT);
			sink_link_state = SINK_CBUS_LINK_POWER_OFF;
		}

		else if (fgSinkAttachSource() == FALSE) {
			vSinkCbusSelectR(SINK_CBUS_R_FLOAT);
			sink_link_state = SINK_CBUS_LINK_UNATTACHED;
		}
		/*
		else if(fgSinkCbusStuchLow() == TRUE)
		{
		vSinkCbusSelectR(SINK_CBUS_R_1K);
		sink_link_state = SINK_CBUS_LINK_ATTACHED;
		}*/
		else {
			sink_link_state = SINK_CBUS_LINK_STATE_CONTENT_ON;
			my_dev_reg[SINK_CBUS_MSC_STATUS_LINK_MODE] |= SINK_CBUS_MSC_STATUS_LINK_MODE_PATH_EN;
			st_dev_ctrl.st_path.fgPATHChanged = TRUE;

			st_dev_ctrl.st_dcap.fgDCAPReady = TRUE;
			st_dev_ctrl.st_dcap.fgDCAPChanged = TRUE;
		}

		break;

	case SINK_CBUS_LINK_STATE_CONTENT_ON:

		/* printk("connected ON\n");*/
		if (fgSinkStandby() == TRUE) {
			vSinkCbusSelectR(SINK_CBUS_R_FLOAT);
			sink_link_state = SINK_CBUS_LINK_POWER_OFF;
		}

		else if (fgSinkAttachSource() == FALSE) {
			vSinkCbusSelectR(SINK_CBUS_R_FLOAT);
			sink_link_state = SINK_CBUS_LINK_UNATTACHED;
		}

		/*
		else if(fgSinkCbusStuchLow() == TRUE)
		{
		vSinkCbusSelectR(SINK_CBUS_R_1K);
		sink_link_state = SINK_CBUS_LINK_ATTACHED;
		}*/
		break;

	case SINK_CBUS_LINK_POWER_OFF:
		HDMI_LOG(HDMI_LOG_DEBUG, "power off");

		if (fgSinkStandby() == FALSE) {
			if (fgSinkAttachSource() == TRUE) {
				vSinkCbusSelectR(SINK_CBUS_R_1K);
				sink_link_state = SINK_CBUS_LINK_ATTACHED;
			} else {
				vSinkCbusSelectR(SINK_CBUS_R_FLOAT);
				sink_link_state = SINK_CBUS_LINK_UNATTACHED;
			}
		}

		break;

	case SINK_CBUS_LINK_FLOAT_CBUS:
		HDMI_LOG(HDMI_LOG_DEBUG, "float");

		if (fgSinkStandby() == TRUE) {
			vSinkCbusSelectR(SINK_CBUS_R_FLOAT);
			sink_link_state = SINK_CBUS_LINK_POWER_OFF;
		}

		else if (sink_link_timer > (50 / MHL_LINK_TIME)) {
			if (fgSinkAttachSource() == TRUE) {
				vSinkCbusSelectR(SINK_CBUS_R_1K);
				sink_link_state = SINK_CBUS_LINK_ATTACHED;
			} else {
				vSinkCbusSelectR(SINK_CBUS_R_FLOAT);
				sink_link_state = SINK_CBUS_LINK_UNATTACHED;
			}
		}

		break;

	default:
		break;
	}

	if (sink_link_state != sink_link_state_bak) {
		/* SINK_CBUS_LOG("link state (new : %d|old:%d)\n",sink_link_state,sink_link_state_bak);*/
		sink_link_state_bak = sink_link_state;
	}

	#if 0
	if (sink_test_mode.test_mode == 0xA5) {
		if (sink_test_mode_timer > 1500) {
			//sink_test_mode_timer = 0;
			sink_test_mode_timer_reset = true;

			if (sink_test_mode.counter > 0) {
				sink_test_mode.counter--;
				HDMI_LOG(HDMI_LOG_DEBUG, "[mhls_log]begin link:%d\n", sink_test_mode.counter);

				vIO32WriteFldAlign(MHL_HDCP_CTRL_1, 0x0, RISC_ADDR_PAGE);
				sink_link_state = SINK_CBUS_LINK_FLOAT_CBUS;
				//sink_link_timer = 0;
				sink_link_timer_reset = true;
				vSinkCbusInit();
				vSinkCbusInitDevice();
				vSinkCbusSelectR(SINK_CBUS_R_FLOAT);
				vSinkCbusReset();
				tmp = 500;

				while (tmp > 0) {
					tmp--;
					msleep(10);
					vIO32WriteFldAlign(MHL_HDCP_CTRL_1, 0x0, RISC_ADDR_PAGE);
				}
			}
		}
	}
	#endif
}
EXPORT_SYMBOL(vSinkLinkState);

unsigned int sink_cbus_cmd_live = 0;
void sink_dscr_updata(void)
{
	if (st_dev_ctrl.st_burst.type == SINK_BURST_TYPE_NONE)
		st_dev_ctrl.st_burst.type = SINK_BURST_TYPE_DSCR;
	else {
		/* SINK_CBUS_LOG("burst busy\n");*/
		HDMI_LOG(HDMI_LOG_DEBUG, "burst busy\n");
	}
}

void vTrigRCPMsg(unsigned int key)
{
	st_dev_ctrl.st_rcp.fgIsValid = TRUE;
	st_dev_ctrl.st_rcp.u1Val = key;
	st_dev_ctrl.st_rcp.u2Code = SINK_MSC_MSG_RCP;
}

/* for requster command process when responder is idle*/
void vSinkCommandState(void)
{
	sink_cbus_cmd_live++;

	if (fgSinkCbusStop)
		return;

	if ((fgIsSinkCbusConnected() == TRUE) && (sink_cbus_req_delay_by_discovery == 0)) {

		if (st_dev_ctrl.st_hpd.fgHPDChanged) {
			if (st_dev_ctrl.st_hpd.fgHPDOn)
				fgSinkCbusReqSetHPDCmd();
			else
				fgSinkCbusReqClrHPDCmd();

			st_dev_ctrl.st_hpd.fgHPDChanged = FALSE;
		}

		else if (st_dev_ctrl.st_path.fgPATHChanged) {
			fgSinkCbusReqWriteStatCmd(SINK_CBUS_MSC_STATUS_LINK_MODE,
				my_dev_reg[SINK_CBUS_MSC_STATUS_LINK_MODE]);
			st_dev_ctrl.st_path.fgPATHChanged = FALSE;
		}

		else if (st_dev_ctrl.st_dcap.fgDCAPChanged) {
			if (st_dev_ctrl.st_dcap.fgDCAPReady)
				my_dev_reg[SINK_CBUS_MSC_STATUS_CONNECTED_RDY] |=
				SINK_CBUS_MSC_STATUS_CONNECTED_RDY_DCAP_RDY;

			else
				my_dev_reg[SINK_CBUS_MSC_STATUS_CONNECTED_RDY] &=
				~SINK_CBUS_MSC_STATUS_CONNECTED_RDY_DCAP_RDY;

			fgSinkCbusReqWriteStatCmd(SINK_CBUS_MSC_STATUS_CONNECTED_RDY,
				my_dev_reg[SINK_CBUS_MSC_STATUS_CONNECTED_RDY]);
			fgSinkCbusReqWriteStatCmd(SINK_CBUS_MSC_RCHANGE_INT, SINK_CBUS_MSC_RCHANGE_INT_DCAP_CHG);
			st_dev_ctrl.st_dcap.fgDCAPChanged = FALSE;
		}

		else if (st_dev_ctrl.st_edid.fgEDIDChanged)
			fgSinkCbusReqWriteStatCmd(SINK_CBUS_MSC_DCHANGE_INT, SINK_CBUS_MSC_DCHANGE_INT_EDID_CHG);

		else if (st_dev_ctrl.st_rcp.fgIsValid) {
			if ((st_dev_ctrl.st_rcp.u2Code & 0xff) == (SINK_MSC_MSG_RCP & 0xff))
				fgSinkCbusReqMscMsgCmd(SINK_MSC_MSG_RCP, st_dev_ctrl.st_rcp.u1Val);

			st_dev_ctrl.st_rcp.fgIsValid = FALSE;
		}

		else if (st_dev_ctrl.fgIs3DReq) {
			if (st_dev_ctrl.st_burst.type == SINK_BURST_TYPE_NONE) {
				st_dev_ctrl.st_burst.type =     SINK_BURST_TYPE_3DINFO;
				st_dev_ctrl.st_burst.st_3dinfo.state = SINK_3DINFO_REQ_STATE_REQ;
				st_dev_ctrl.fgIs3DReq = FALSE;
			}
		}

		else if (stSinkCbusMscMsg.fgIsValid) {
			if (fgIsSinkMscMsg(stSinkCbusMscMsg.u2Code) == FALSE)
				fgSinkCbusReqMscMsgCmd(SINK_MSC_MSG_MSGE, 1);

			else if ((stSinkCbusMscMsg.u2Code & 0xff) == (SINK_MSC_MSG_RCP & 0xff)) {
				fgSinkCbusReqMscMsgCmd(SINK_MSC_MSG_RCPK, stSinkCbusMscMsg.u1Val);
				sink_send_keycode_to_ir(stSinkCbusMscMsg.u1Val);
			}

			stSinkCbusMscMsg.fgIsValid = FALSE;
		}

		else if (stSinkCbusDbgWrite.fgIsValid) {
			fgSinkCbusReqWriteStatCmd(stSinkCbusDbgWrite.u1Val, stSinkCbusDbgWrite.u2Code);
			stSinkCbusDbgWrite.fgIsValid = FALSE;
			HDMI_LOG(HDMI_LOG_DEBUG, "device reg write done\n");
		}

		else if (stSinkCbusDbgRead.fgIsValid) {
			fgSinkCbusReqReadDevCapCmd(stSinkCbusDbgRead.u1Val);
			stSinkCbusDbgRead.fgIsValid = FALSE;
			/* printk("device reg read done,reg offset:%x,data:%x\n",
			stSinkCbusDbgRead.u1Val,src_dev_reg[stSinkCbusDbgRead.u1Val]);*/
		}

		/* 3dinfo*/
		if (st_dev_ctrl.st_burst.type == SINK_BURST_TYPE_3DINFO) {
			static unsigned int count_3d;

			if (st_dev_ctrl.st_burst.st_3dinfo.state == SINK_3DINFO_REQ_STATE_REQ) {
				fgSinkCbusReqWriteStatCmd(SINK_CBUS_MSC_RCHANGE_INT, SINK_CBUS_MSC_RCHANGE_INT_REQ_WRT);
				st_dev_ctrl.st_burst.st_3dinfo.state = SINK_3DINFO_REQ_STATE_GRT;
				vSetSinkCbusBurstReqWaitTmr(1000 / MHL_LINK_TIME);
			}

			else if (st_dev_ctrl.st_burst.st_3dinfo.state == SINK_3DINFO_REQ_STATE_GRT) {
				if (st_dev_ctrl.fgIsGrtWrt) {
					st_dev_ctrl.fgIsGrtWrt = FALSE;
					vClrSinkCbusBurstReqWaitTmr();

					if (count_3d == 0)
						fgSinkCbusReqWriteBrustCmd(&sink_3D_brust_info[0], 17);
					else if (count_3d == 1)
						fgSinkCbusReqWriteBrustCmd(&sink_3D_brust_info1[0], 17);
					else if (count_3d == 2)
						fgSinkCbusReqWriteBrustCmd(&sink_3D_brust_info2[0], 9);
					else if (count_3d == 3)
						fgSinkCbusReqWriteBrustCmd(&sink_3D_brust_info3[0], 17);
					else if (count_3d == 4)
						fgSinkCbusReqWriteBrustCmd(&sink_3D_brust_info4[0], 9);

					st_dev_ctrl.st_burst.st_3dinfo.state = SINK_3DINFO_REQ_STATE_IDLE;
					/*st_dev_ctrl.st_burst.type = SINK_BURST_TYPE_DSCR;For debugdegbug!!!!*/
					/*st_dev_ctrl.st_burst.type = SINK_BURST_TYPE_NONE;*/
					st_dev_ctrl.st_burst.st_3dinfo.state = 3;
				}
			} else if (st_dev_ctrl.st_burst.st_3dinfo.state == 3) {
				vClrSinkCbusBurstReqWaitTmr();
				fgSinkCbusReqWriteStatCmd(SINK_CBUS_MSC_RCHANGE_INT,
					SINK_CBUS_MSC_RCHANGE_INT_DSCR_CHG);

				if (count_3d == 4) {
					count_3d = 0;
					st_dev_ctrl.st_burst.type = SINK_BURST_TYPE_NONE;
					/* st_dev_ctrl.st_burst.type = SINK_BURST_TYPE_3DINFO;*/
				} else {
					count_3d++;
					HDMI_LOG(HDMI_LOG_DEBUG, "request brust again");
					st_dev_ctrl.st_burst.type = SINK_BURST_TYPE_3DINFO;
					st_dev_ctrl.st_burst.st_3dinfo.state = SINK_3DINFO_REQ_STATE_REQ;
				}
			}
		}

		else if (st_dev_ctrl.st_burst.type == SINK_BURST_TYPE_DSCR) {
			if (st_dev_ctrl.st_burst.st_dscr.state == SINK_DSCR_REQ_STATE_REQ) {
				fgSinkCbusReqWriteStatCmd(SINK_CBUS_MSC_RCHANGE_INT, SINK_CBUS_MSC_RCHANGE_INT_REQ_WRT);
				st_dev_ctrl.st_burst.st_dscr.state = SINK_DSCR_REQ_STATE_GRT;
				vSetSinkCbusBurstReqWaitTmr(1000 / MHL_LINK_TIME);
			}

			else if (st_dev_ctrl.st_burst.st_dscr.state == SINK_DSCR_REQ_STATE_GRT) {
				if (st_dev_ctrl.fgIsGrtWrt) {
					st_dev_ctrl.fgIsGrtWrt = FALSE;
					vClrSinkCbusBurstReqWaitTmr();
					fgSinkCbusReqWriteBrustCmd(&sink_dscr_data[0], 17);
					fgSinkCbusReqWriteStatCmd(SINK_CBUS_MSC_RCHANGE_INT,
						SINK_CBUS_MSC_RCHANGE_INT_DSCR_CHG);
					st_dev_ctrl.st_burst.st_dscr.state = SINK_DSCR_REQ_STATE_IDLE;
					st_dev_ctrl.st_burst.type = SINK_BURST_TYPE_NONE;
				}
			}
		}
	}
}
EXPORT_SYMBOL(vSinkCommandState);

/* unsigned int mhl_rx_log_on = 0;*/
void sink_cbus_cmd(unsigned int cmd, unsigned int arg0, unsigned int arg1, unsigned int arg2)
{
	unsigned int temp;

	switch (cmd) {
	case 0:
		/* mhl_rx_log_on = arg0;*/
		/* printk("[mhls]log setting=%x\n",mhl_rx_log_on);*/
		break;

	case 1:
		vSinkCbusCmdStatus();
		break;

	case 2:
		HDMI_LOG(HDMI_LOG_DEBUG, "sink timer live : %d\n", sink_cbus_timer_live);
		HDMI_LOG(HDMI_LOG_DEBUG, "sink int live : %d\n", sink_cbus_int_live);
		HDMI_LOG(HDMI_LOG_DEBUG, "sink link live : %d\n", sink_cbus_link_live);
		HDMI_LOG(HDMI_LOG_DEBUG, "sink cmd live : %d\n", sink_cbus_cmd_live);
		HDMI_LOG(HDMI_LOG_DEBUG, "sink_link_state:%d\n", sink_link_state);
		/*printk("fgSinkAttachSource():%d\n",fgSinkAttachSource());
		printk("fgSinkStandby():%d\n",fgSinkStandby());
		printk("fgSinkWakeupErr():%d\n",fgSinkWakeupErr());
		printk("fgSinkDiscoveryOk():%d\n",fgSinkDiscoveryOk());
		printk("fgSinkCbusStuchLow():%d\n",fgSinkCbusStuchLow());*/
		break;

	case 3:
		if (arg0 == 0) {
			HDMI_LOG(HDMI_LOG_INFO, "un attach\n");
			vSinkSetAttachStatus(FALSE);
		} else {
			HDMI_LOG(HDMI_LOG_INFO, "attach\n");
			vSinkSetAttachStatus(TRUE);
		}

		break;

	case 4:
		sink_100k_r = arg0;
		HDMI_LOG(HDMI_LOG_DEBUG, "100K:%x\n", sink_100k_r);
		break;

	case 5:
		sink_cbus_low_force = arg0;
		HDMI_LOG(HDMI_LOG_DEBUG, "cbus low %d\n", sink_cbus_low_force);
		break;

	case 6:
		sink_test_mode.test_mode = arg0;
		sink_test_mode.counter = arg1;
		HDMI_LOG(HDMI_LOG_DEBUG, "test mode:%x,%d\n", sink_test_mode.test_mode, sink_test_mode.counter);
		sink_test_mode_timer = 0;
		fgSinkCbusStop = FALSE;
		u1ForceSinkCbusStop = arg0;
		vSinkCbusInit();
		vSinkCbusInitDevice();
		vSinkCbusSelectR(SINK_CBUS_R_FLOAT);
		vSinkCbusReset();
		vMhlSinkUsePinTrigForDebug(0);
		break;

	case 7:
		/* u1SinkGetEfuseConfig();*/
		break;

	case 8:
		/* u1TestGetEfuseConfig(arg0);*/
		break;

	case 9:
		stSinkCbusMscMsg.fgIsValid = TRUE;
		stSinkCbusMscMsg.u2Code = SINK_MSC_MSG_RCP;
		stSinkCbusMscMsg.u1Val = (unsigned char)arg0;
		HDMI_LOG(HDMI_LOG_DEBUG, "RCP send : %x\n", stSinkCbusMscMsg.u1Val);
		break;

	case 50:
		temp = (unsigned char)arg0;

		if ((temp >= 0x20) && (temp < 0x40)) {
			stSinkCbusDbgWrite.fgIsValid = TRUE;
			stSinkCbusDbgWrite.u1Val = temp;
			stSinkCbusDbgWrite.u2Code = (unsigned char)arg1;
			HDMI_LOG(HDMI_LOG_DEBUG, "device reg write, device reg offset:%x, write data:%x\n",
				stSinkCbusDbgWrite.u1Val, stSinkCbusDbgWrite.u2Code);
		} else
			HDMI_LOG(HDMI_LOG_DEBUG, "device reg write, device reg offset error\n");

		break;

	case 51:
		temp = (unsigned char)arg0;

		if (temp < 0x10) {
			stSinkCbusDbgRead.fgIsValid = TRUE;
			stSinkCbusDbgRead.u1Val = temp;
			/* printk("device reg read, device reg offset:%x\n",stSinkCbusDbgRead.u1Val);*/
		} else
			HDMI_LOG(HDMI_LOG_DEBUG, "device reg read, device reg offset error\n");

		break;

	default:
		break;
	}
}
#endif
