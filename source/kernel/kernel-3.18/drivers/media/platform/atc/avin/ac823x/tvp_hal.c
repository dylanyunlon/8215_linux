/*************************************************************************************
 *LEGAL DISCLAIMER
 *
 * (Header of AutoChips Software/Firmware Release or Documentation)
 *
 * BY OPENING OR USING THIS FILE, USER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND
 * AGREES THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("AUTOCHIPS SOFTWARE")
 * ARE PROVIDED TO USER ON AN "AS-IS" BASIS ONLY. AUTOCHIPS EXPRESSLY DISCLAIMS
 * ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES AUTOCHIPS PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED
 * IN, OR SUPPLIED WITH THE AUTOCHIPS SOFTWARE, AND USER AGREES TO LOOK ONLY TO
 * SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. AUTOCHIPS SHALL
 * ALSO NOT BE RESPONSIBLE FOR ANY AUTOCHIPS SOFTWARE RELEASES MADE TO USER'S
 * SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 * USER HEREBY ACKNOWLEDGES THE CONFIDENTIALITY OF AUTOCHIPS SOFTWARE AND AGREES
 * NOT TO DISCLOSE OR PERMIT DISCLOSURE OF ANY AUTOCHIPS SOFTWARE TO ANY THIRD
 * PARTY OR TO ANY OTHER PERSON, EXCEPT TO DIRECTORS, OFFICERS, EMPLOYEES OF
 * USER WHO ARE REQUIRED TO HAVE THE INFORMATION TO CARRY OUT THE PURPOSE OF
 * OPENING OR USING THIS FILE.
*************************************************************************************/

#include <linux/module.h>
#include <linux/string.h>
#include <linux/kthread.h>
#include <linux/types.h>
#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/io.h>
#include "x_os.h"
#include "x_typedef.h"
#include "tvp_hal.h"
#include "tvp5158_patch_v02_03_02.h"
#include "wch_if.h"
#include "wch_drv.h"
#include "avin_common.h"


struct mutex g_WchLock;
static WCH_BUF_T g_rTvpWchBufInfo = {0};
static WCH_CTL_PARAM_T g_rTvpWchCtrl;
static bool g_fgExitThread = false;
static HANDLE_T g_hSendBufMsgQ = NULL_HANDLE;
static struct task_struct *g_Thread_task = NULL;
static int g_TvpStartCnt = 0;
static int g_WchStartCnt = 0;

static struct i2c_client *g_pClient = NULL;
static uint32_t g_u4LogCnt = 0;
struct tvp_data g_tvp_data[TVP_MAX_VIDEO_DEVS];

extern int avin_buffer_complete(enum avin_device_type device_type, const struct capture_priv *data);


static int tvp_i2c_read(uint8_t *reg, uint8_t *buffer, uint8_t count, uint8_t dataSize)
{
	int err = 0;
	uint8_t idx = 0;
	uint8_t retry = 0;
	struct i2c_msg msg;
	unsigned char data[8] = {0};

	if ((NULL == reg) || (NULL == buffer) || (dataSize <= 0) || (dataSize > 2)) {
		pr_err("[AVIN]%s: param reg/buffer/datasize(%d) error!\r\n", __func__, dataSize);
		return -1;
	}

	for (idx = 0; idx < count; idx++) {
		memset(&msg, 0, sizeof(struct i2c_msg));
		msg.addr = g_pClient->addr;
		msg.flags = 0;
		msg.len = 1;
		msg.buf = data;
		data[0] = reg[idx];
		for (retry = 0; retry < TVP_I2C_RETRY_TIME; retry++) {
			err = i2c_transfer(g_pClient->adapter, &msg, 1);
			if (err >= 0)
				break;
		}
		if (TVP_I2C_RETRY_TIME == retry) {
			pr_err("[AVIN]%s: i2c_transfer(0x%x, %x) write error(%d) with idx(%d)!\r\n",
				__func__, msg.addr, msg.buf[0], err, idx);
			return err;
		}

		msg.flags = I2C_M_RD;
		msg.len = dataSize;
		memset(data, 0, sizeof(data));
		for (retry = 0; retry < TVP_I2C_RETRY_TIME; retry++) {
			err = i2c_transfer(g_pClient->adapter, &msg, 1);
			if (err >= 0) {
				if (dataSize == 1) {
					buffer[idx] = data[0];
				} else if (dataSize == 2) {
					buffer[2 * idx] = data[1];
					buffer[2 * idx + 1] = data[0];
				}
				break;
			}
		}
		if (TVP_I2C_RETRY_TIME == retry) {
			pr_err("[AVIN]%s: i2c_transfer(0x%x, %x) read error(%d) with idx(%d)!\r\n",
				__func__, msg.addr, msg.buf[0], err, idx);
			return err;
		}
	}

	return (0);
}

static int tvp_i2c_write(uint8_t *reg, uint8_t *buffer, uint8_t count, uint8_t dataSize)
{
	int err = 0;
	uint8_t idx = 0;
	uint8_t retry = 0;
	struct i2c_msg msg;
	unsigned char data[8] = {0};

	if ((NULL == reg) || (NULL == buffer) || (dataSize <= 0) || (dataSize > 2)) {
		pr_err("[AVIN]%s: param dataSize(%d) error!\r\n", __func__, dataSize);
		return -1;
	}

	for (idx = 0; idx < count; idx++) {
		memset(&msg, 0, sizeof(struct i2c_msg));
		msg.addr = g_pClient->addr;
		msg.flags = 0;
		msg.buf = data;

		data[0] = reg[idx];

		if (dataSize == 1) {
			data[1] = buffer[idx];
			msg.len = 2;
		}
		else if (dataSize == 2) {
			data[1] = buffer[2 * idx + 1];
			data[2] = buffer[2 * idx];
			msg.len = 3;
		}
		for (retry = 0; retry < TVP_I2C_RETRY_TIME; retry++) {
			err = i2c_transfer(g_pClient->adapter, &msg, 1);
			if (err >= 0)
				break;
		}
		if (TVP_I2C_RETRY_TIME == retry) {
			pr_err("[AVIN]%s: i2c_transfer(0x%x, %x) write error(%d) with idx(%d)!\r\n",
				__func__, msg.addr, msg.buf[0], err, idx);
			return err;
		}
	}

	return 0;
}

static int tvp_vbus_write(uint32_t vbus_addr, uint8_t val, uint8_t len)
{
	uint8_t idx = 0;
	uint8_t datasize = 0;
	uint8_t regAddr[4];
	uint8_t regVal[4];

	idx = 0;
	datasize = 1;
	regAddr[idx] = TVP_REG_VBUS_ADDRESS_ACCESS_1;
	regVal[idx] = (uint8_t)((vbus_addr >> 0) & 0xFF);
	idx++;

	regAddr[idx] = TVP_REG_VBUS_ADDRESS_ACCESS_2;
	regVal[idx] = (uint8_t)((vbus_addr >> 8) & 0xFF);
	idx++;

	regAddr[idx] = TVP_REG_VBUS_ADDRESS_ACCESS_3;
	regVal[idx] = (uint8_t)((vbus_addr >> 16) & 0xFF);
	idx++;

	if (len) {
		regAddr[idx] = TVP_REG_VBUS_DATA_ACCESS_NO_ADDR_INCREMENT;
		regVal[idx]  = val;
		idx++;
	}

	return (tvp_i2c_write(regAddr, regVal, idx, datasize));
}

static int tvp_select_write(uint8_t value)
{
	uint8_t regAddr = 0;
	uint8_t regVal = 0;
	uint8_t count = 0;
	uint8_t datasize = 0;

	regAddr = TVP_REG_DECODER_WRITE_ENABLE;
	regVal = value;
	count = 1;
	datasize = 1;

	return (tvp_i2c_write(&regAddr, &regVal, count, datasize));
}

static int tvp_select_read(uint8_t value)
{
	uint8_t regAddr = 0;
	uint8_t regVal = 0;
	uint8_t count = 0;
	uint8_t datasize = 0;

	regAddr = TVP_REG_DECODER_READ_ENABLE;
	regVal = value;
	count = 1;
	datasize = 1;

	return (tvp_i2c_write(&regAddr, &regVal, count, datasize));
}

static int tvp_download_patch(void)
{
	int ret = 0;
	uint8_t idx = 0;
	uint8_t vbusStatus = 0;
	uint8_t regVal[8];
	uint8_t regAddr[254];
	int32_t wrSize = 0;
	uint8_t *patchAddr = 0;
	uint32_t patchSize = 0;

	patchAddr = (uint8_t*)gTVP5158_patch;
	patchSize = sizeof(gTVP5158_patch);

	ret = tvp_select_write(0xF);
	if (ret) {
		pr_err("[AVIN]%s(line%d): tvp_select_write error with ret(%d)\r\n",
			__func__, __LINE__, ret);
		return ret;
	}

	idx = 0;
	regAddr[idx] = TVP_REG_VBUS_ADDRESS_ACCESS_1;
	regVal[idx] = 0x60;
	idx++;

	regAddr[idx] = TVP_REG_VBUS_ADDRESS_ACCESS_2;
	regVal[idx] = 0x00;
	idx++;

	regAddr[idx] = TVP_REG_VBUS_ADDRESS_ACCESS_3;
	regVal[idx] = 0xB0;
	idx++;

	ret = tvp_i2c_write(regAddr, regVal, idx, 1);
	if (ret) {
		pr_err("[AVIN]%s(line%d): tvp_i2c_write error with ret(%d)\r\n",
			__func__, __LINE__, ret);
		return ret;
	}

	idx = 0;
	regAddr[idx] = 0xE0;
	regVal[idx] = 0;
	idx++;

	ret = tvp_i2c_read(regAddr, regVal, idx, 1);
	if (ret) {
		pr_err("[AVIN]%s(line%d): tvp_i2c_read error with ret(%d)\r\n",
			__func__, __LINE__, ret);
		return (ret);
	}

	vbusStatus = regVal[0];
	if (vbusStatus & 0x2) {
		pr_err("[AVIN]%s: Patch is already running.\r\n", __func__);
		return ret;
	}
	else
	{
		pr_err("[AVIN]%s: Patch is downloading...\r\n", __func__);
	}

	ret = tvp_select_write(0xF);
	if (ret) {
		pr_err("[AVIN]%s(line%d): tvp_select_write error with ret(%d)\r\n",
			__func__, __LINE__, ret);
		return ret;
	}

	vbusStatus |= 0x1;
	ret = tvp_vbus_write(0xB00060, vbusStatus, 1);
	if (ret) {
		pr_err("[AVIN]%s(line%d): tvp_vbus_write error with ret(%d)\r\n",
			__func__, __LINE__, ret);
		return ret;
	}

	ret = tvp_vbus_write(0x400000, 0, 0);
	if (ret) {
		pr_err("[AVIN]%s(line%d): tvp_vbus_write error with ret(%d)\r\n",
			__func__, __LINE__, ret);
		return ret;
	}

	memset(regAddr, 0xE1, sizeof(regAddr));
	while (patchSize) {
		if(patchSize < sizeof(regAddr)) {
			wrSize = patchSize;
		} else {
			wrSize = sizeof(regAddr);
		}
		ret = tvp_i2c_write(regAddr, patchAddr, wrSize, 1);
		if (ret) {
			pr_err("[AVIN]%s(line%d): tvp_i2c_write error with ret(%d)\r\n",
				__func__, __LINE__, ret);
			return ret;
		}
		patchAddr += wrSize;
		patchSize -= wrSize;
	}
	pr_err("[AVIN]%s: Patch is downloaded.  Soft reset\r\n", __func__);

	vbusStatus |= 0x3;
	ret = tvp_vbus_write(0xB00060, vbusStatus, 1);
	if (ret) {
		pr_err("[AVIN]%s(line%d): tvp_vbus_write error with ret(%d)\r\n",
			__func__, __LINE__, ret);
		return ret;
	}

	vbusStatus &= ~(0x1);
	ret = tvp_vbus_write(0xB00060, vbusStatus, 1);
	if (ret) {
		pr_err("[AVIN]%s(line%d): tvp_vbus_write error with ret(%d)\r\n",
			__func__, __LINE__, ret);
		return (ret);
	}
	//msleep(300);

	return 0;
}

static int tvp_check_chip(void)
{
	int ret = 0;
	uint8_t idx = 0;
	uint8_t datasize = 0;
	uint8_t regAddr[2] = {0};
	uint8_t regVal[2] = {0};

	idx = 0;
	regAddr[idx] = TVP_REG_CHIP_ID_MSB;
	regVal[idx] = 0;
	idx++;

	regAddr[idx] = TVP_REG_CHIP_ID_LSB;
	regVal[idx] = 0;
	idx++;
	datasize = 1;

	ret = tvp_i2c_read(regAddr, regVal, idx, datasize);
	if (ret) {
		pr_err("[AVIN]%s: tvp_i2c_read error with ret(%d)\r\n", __func__, ret);
		return ret;
	}

	if ((regVal[0] != ((TVP_CHIP_ID & 0xFF00) >> 8)) ||
		(regVal[1] != (TVP_CHIP_ID & 0xFF))) {
		pr_err("[AVIN]%s: tvp_i2c_read error with regVal(0x%02x, 0x%02x)\r\n",
			__func__, regVal[0], regVal[1]);
		return -1;
	}

	return 0;
}

/*
static int tvp_check_input(tvp_input_standard *pInputStandard)
{
	int ret = 0;
	uint8_t count = 0;
	uint8_t datasize = 0;
	uint8_t regAddr = 0;
	uint8_t regValue[2] = {0};

	regAddr = TVP_REG_VIDEO_STANDARD_STATUS;
	count = 1;
	datasize = 2;

	//read decoder 1 & 2 increment
	ret = tvp_select_read(0x13);
	if (ret) {
		pr_err("[AVIN]%s(line%d): tvp_select_read error with ret(%d)\r\n",
			__func__, __LINE__, ret);
		return (ret);
	}
	ret = tvp_i2c_read(&regAddr, regValue, count, datasize);
	if (ret) {
		pr_err("[AVIN]%s(line%d): tvp_i2c_read error with ret(%d)\r\n",
			__func__, __LINE__, ret);
		return ret;
	}
	pInputStandard->dec1_std = regValue[1];
	pInputStandard->dec2_std = regValue[0];

	//read decoder 3 & 4 increment
	ret = tvp_select_read(0x1C);
	if (ret) {
		pr_err("[AVIN]%s(line%d): tvp_select_read error with ret(%d)\r\n",
			__func__, __LINE__, ret);
		return ret;
	}

	ret = tvp_i2c_read(&regAddr, regValue, count, datasize);
	if (ret) {
		pr_err("[AVIN]%s(line%d): tvp_i2c_read error with ret(%d)\r\n",
			__func__, __LINE__, ret);
		return ret;
	}
	pInputStandard->dec3_std = regValue[1];
	pInputStandard->dec4_std = regValue[0];

	ret = tvp_select_read(0x1);
	if (ret) {
		pr_err("[AVIN]%s(line%d): tvp_select_read error with ret(%d)\r\n",
			__func__, __LINE__, ret);
		return ret;
	}

	pr_err("[AVIN]%s: dec1_std(%d) dec2_std(%d) dec3_std(%d) dec4_std(%d)\r\n",
			__func__, pInputStandard->dec1_std, pInputStandard->dec2_std,
			pInputStandard->dec3_std, pInputStandard->dec4_std);

	return 0;
}
*/

static int tvp_reset_ofm(void)
{
	int ret = 0;
	uint8_t count = 0;
	uint8_t datasize = 0;
	uint8_t regAddr = 0;
	uint8_t regVal = 0;

	ret = tvp_select_write(0x1);
	if (ret) {
		pr_err("[AVIN]%s: tvp_select_write error with ret(%d)\r\n",
			__func__, ret);
		return ret;
	}

	regAddr = TVP_REG_OFM_MODE_CONTROL;
	count = 1;
	regVal = 0;
	datasize = 1;

	ret = tvp_i2c_write(&regAddr, &regVal, count, datasize);
	if (ret) {
		pr_err("[AVIN]%s: tvp_i2c_write error with ret(%d)\r\n",
			__func__, ret);
		return ret;
	}

	msleep(10);

	return 0;
}

static int tvp_soft_reset(void)
{
	int ret = 0;
	uint8_t data = 0;
	uint8_t idx = 0;
	uint8_t datasize = 0;
	uint8_t regAddr[8] = {0};
	uint8_t regVal[8] = {0};

	ret = tvp_select_write(0xF);
	if (ret) {
		pr_err("[AVIN]%s(line%d): tvp_select_write error with ret(%d)\r\n",
			__func__, __LINE__, ret);
		return ret;
	}

	idx = 0;
	datasize = 1;
	regAddr[idx] = TVP_REG_VBUS_ADDRESS_ACCESS_1;
	regVal[idx] = 0x60;
	idx++;

	regAddr[idx] = TVP_REG_VBUS_ADDRESS_ACCESS_2;
	regVal[idx] = 0x00;
	idx++;

	regAddr[idx] = TVP_REG_VBUS_ADDRESS_ACCESS_3;
	regVal[idx] = 0xB0;
	idx++;

	ret = tvp_i2c_write(regAddr, regVal, idx, datasize);
	if (ret) {
		pr_err("[AVIN]%s(line%d): tvp_i2c_write error with ret(%d)\r\n",
			__func__, __LINE__, ret);
		return ret;
	}

	idx = 0;
	regAddr[idx] = TVP_REG_VBUS_DATA_ACCESS_NO_ADDR_INCREMENT;
	regVal[idx] = 0;
	idx++;

	data = (uint8_t)tvp_i2c_read(regAddr, regVal, idx, datasize);
	if (ret) {
		pr_err("[AVIN]%s(line%d): tvp_i2c_read error with ret(%d)\r\n",
			__func__, __LINE__, ret);
		return ret;
	}

	data = regVal[0] | 1;
	ret = tvp_vbus_write(0xB00060, data, 1);
	if (ret) {
		pr_err("[AVIN]%s(line%d): tvp_vbus_write error with ret(%d)\r\n",
			__func__, __LINE__, ret);
		return ret;
	}

	data &= ~(0x1);
	ret = tvp_vbus_write(0xB00060, data, 1);
	if (ret) {
		pr_err("[AVIN]%s(line%d): tvp_vbus_write error with ret(%d)\r\n",
			__func__, __LINE__, ret);
		return ret;
	}

	msleep(300);

	return 0;
}

int tvp_set_attribute(uint8_t chid, uint32_t control, int32_t value)
{
	int ret = 0;
	uint8_t regAddr = 0;
	uint8_t regVal = 0;
	uint8_t count = 0;
	uint8_t datasize = 0;

	if (chid >= TVP_MAX_VIDEO_DEVS) {
		pr_err("[AVIN]%s: param error with chid(%d)!\r\n", __func__, chid);
		return -1;
	}
	regVal = (uint8_t)value;
	count = 1;
	datasize = 1;
	switch (control)
	{
	case V4L2_CID_BRIGHTNESS:
		regAddr = TVP_REG_LUMINANCE_BRIGHTNESS;
		break;

	case V4L2_CID_CONTRAST:
		regAddr = TVP_REG_LUMINANCE_CONTRAST;
		break;

	case V4L2_CID_SATURATION:
		regAddr = TVP_REG_CHROMINANCE_SATURATION;
		break;

	case V4L2_CID_HUE:
		regAddr = TVP_REG_CHROMINANCE_HUE;
		break;

	default:
		pr_err("[AVIN]%s: param control(%d) error\r\n", __func__, control);
		return -1;
	}

	ret = tvp_select_write(1 << chid);
	if (ret) {
		pr_err("[AVIN]%s: tvp_select_write error with ret(%d)\r\n", __func__, ret);
		return ret;
	}

	return (tvp_i2c_write(&regAddr, &regVal, count, datasize));
}

int tvp_get_attribute(uint8_t chid, uint32_t control, int32_t *pvalue)
{
	int ret = 0;
	uint8_t regAddr = 0;
	uint8_t count = 0;
	uint8_t datasize = 0;

	if (chid >= TVP_MAX_VIDEO_DEVS) {
		pr_err("[AVIN]%s: param error with chid(%d)!\r\n", __func__, chid);
		return -1;
	}
	if (NULL == pvalue) {
		pr_err("[AVIN]%s: param pvalue is NULL!\r\n", __func__);
		return -1;
	}
	count = 1;
	datasize = 1;
	switch (control)
	{
	case V4L2_CID_BRIGHTNESS:
		regAddr = TVP_REG_LUMINANCE_BRIGHTNESS;
		break;

	case V4L2_CID_CONTRAST:
		regAddr = TVP_REG_LUMINANCE_CONTRAST;
		break;

	case V4L2_CID_SATURATION:
		regAddr = TVP_REG_CHROMINANCE_SATURATION;
		break;

	case V4L2_CID_HUE:
		regAddr = TVP_REG_CHROMINANCE_HUE;
		break;

	default:
		pr_err("[AVIN]%s: param control(%d) error\r\n", __func__, control);
		return -1;
	}

	ret = tvp_select_read(1 << chid);
	if (ret) {
		pr_err("[AVIN]%s: tvp_select_read error with ret(%d)\r\n", __func__, ret);
		return ret;
	}

	return (tvp_i2c_read(&regAddr, (uint8_t *)pvalue, count, datasize));
}

int tvp_set_std(uint8_t chid, uint8_t std_type)
{
	int ret = 0;
	uint8_t regAddr = 0;
	uint8_t regVal = 0;
	uint8_t count = 0;
	uint8_t datasize = 0;

	if (chid >= TVP_MAX_VIDEO_DEVS) {
		pr_err("[AVIN]%s: param error with chid(%d)!\r\n", __func__, chid);
		return -1;
	}

	count = 1;
	datasize = 1;
	regAddr = TVP_REG_VIDEO_STANDARD_SELECT;
	regVal = std_type;
	ret = tvp_select_write(1 << chid);
	if (ret) {
		pr_err("[AVIN]%s: tvp_select_write error with ret(%d)\r\n", __func__, ret);
		return ret;
	}

	return (tvp_i2c_write(&regAddr, &regVal, count, datasize));
}


int tvp_get_std(uint8_t chid, v4l2_std_id *pstd_type)
{
	int ret = 0;
	uint8_t regAddr = 0;
	uint8_t regVal = 0;
	uint8_t count = 0;
	uint8_t datasize = 0;

	if (chid >= TVP_MAX_VIDEO_DEVS) {
		pr_err("[AVIN]%s: param error with chid(%d)!\r\n", __func__, chid);
		return -1;
	}
	if (NULL == pstd_type) {
		pr_err("[AVIN]%s: param pstd_type is NULL!\r\n", __func__);
		return -1;
	}
	count = 1;
	datasize = 1;
	regAddr = TVP_REG_VIDEO_STANDARD_STATUS;
	ret = tvp_select_read(1 << chid);
	if (ret) {
		pr_err("[AVIN]%s: tvp_select_read error with ret(%d)\r\n", __func__, ret);
		return ret;
	}
	ret = tvp_i2c_read(&regAddr, &regVal, count, datasize);
	if (ret) {
		pr_err("[AVIN]%s: tvp_i2c_read error with ret(%d)\r\n", __func__, ret);
		return ret;
	}

	switch (regVal & 0x07)
	{
	case VIDEO_STD_NTSC_MJ:
	case VIDEO_STD_NTSC_443:
		*pstd_type = V4L2_STD_NTSC;
		break;

	case VIDEO_STD_PAL_BDGHIN:
	case VIDEO_STD_PAL_M:
	case VIDEO_STD_PAL_COMBINATION_N:
	case VIDEO_STD_PAL_60:
		*pstd_type = V4L2_STD_PAL;
		break;

	default:
		pr_err("[AVIN]%s: get error standards(0x%02x)!\r\n", __func__, regVal);
		*pstd_type = V4L2_STD_UNKNOWN;
		return -1;
	}

	return 0;
}

int tvp_set_reg_value(uint8_t chid, uint8_t reg, uint8_t value)
{
	int ret = 0;
	uint8_t count = 0;
	uint8_t datasize = 0;

	if (chid >= TVP_MAX_VIDEO_DEVS) {
		pr_err("[AVIN]%s: param error with chid(%d)!\r\n", __func__, chid);
		return -1;
	}

	count = 1;
	datasize = 1;
	ret = tvp_select_write(1 << chid);
	if (ret) {
		pr_err("[AVIN]%s: tvp_select_write error with ret(%d)\r\n", __func__, ret);
		return ret;
	}

	return (tvp_i2c_write(&reg, &value, count, datasize));
}

int tvp_get_reg_value(uint8_t chid, uint8_t reg, uint8_t *pvalue)
{
	int ret = 0;
	uint8_t count = 0;
	uint8_t datasize = 0;

	if (chid >= TVP_MAX_VIDEO_DEVS) {
		pr_err("[AVIN]%s: param error with chid(%d)!\r\n", __func__, chid);
		return -1;
	}
	if (NULL == pvalue) {
		pr_err("[AVIN]%s: param pvalue is NULL!\r\n", __func__);
		return -1;
	}
	count = 1;
	datasize = 1;
	ret = tvp_select_read(1 << chid);
	if (ret) {
		pr_err("[AVIN]%s: tvp_select_read error with ret(%d)\r\n", __func__, ret);
		return ret;
	}

	return (tvp_i2c_read(&reg, pvalue, count, datasize));
}

int tvp_i2c_start(int ofmMode)
{
	int ret = 0;
	uint8_t idx = 0;
	uint8_t datasize = 0;
	uint8_t regAddr[32];
	uint8_t regVal[32];
	uint8_t B0, B1, B2, B3, B4, B5, B6, B7, B8, B9;

	if (g_TvpStartCnt) {
		pr_debug("[AVIN]%s: tvp has started!\r\n", __func__);
		g_TvpStartCnt++;
		return 0;
	}

	ret = tvp_check_chip();
	if (ret) {
		pr_err("[AVIN]%s: tvp_check_chip error with ret(%d)\r\n",
			__func__, ret);
		return ret;
	}

	ret = tvp_reset_ofm();
	if (ret) {
		pr_err("[AVIN]%s: tvp_reset_ofm error with ret(%d)\r\n",
			__func__, ret);
		return ret;
	}

	B1 = 0x10;
	B2 = 0x05;
	B3 = 0xE4;
	B4 = 0xE4;
	B5 = 0x00;
	B6 = 0x1B;
	B7 = 0x04;

	switch (ofmMode)
	{
	case TVP_VIDEO_DECODER_MODE_1CH_D1_PORT_A:
		B0 = 0x00;
		//B1 = 0x90;
		B8 = 0xF8;
		B9 = 0x10;
		break;

	case TVP_VIDEO_DECODER_MODE_1CH_HALF_D1_PORT_A_CROP:
		B0 = 0x02;
		B1 = 0x50;
		B8 = 0x70;
		B9 = 0x10;
		break;

	case TVP_VIDEO_DECODER_MODE_4CH_D1:
		B0 = 0xA0;
		B8 = 0xF8;
		B9 = 0x10;
		break;

	case TVP_VIDEO_DECODER_MODE_4CH_D1_CROP:
		B0 = 0xA0;
		B1 = 0x50;
		B8 = 0x18;
		B9 = 0x11;
		break;

	case TVP_VIDEO_DECODER_MODE_4CH_HALF_D1:
	case TVP_VIDEO_DECODER_MODE_4CH_HALF_D1_PROG:
		pr_err("[AVIN]%s: TVP_VIDEO_DECODER_MODE_4CH_HALF_D1\r\n",
			__func__);
		B0 = 0xA2;
		B8 = 0x70;
		B9 = 0x10;
		break;

	case TVP_VIDEO_DECODER_MODE_4CH_HALF_D1_CROP:
	case TVP_VIDEO_DECODER_MODE_4CH_HALF_D1_PROG_CROP:
		pr_err("[AVIN]%s: TVP_VIDEO_DECODER_MODE_4CH_HALF_D1_CROP\r\n",
			__func__);
		B0 = 0xA2;
		B1 = 0x50;
		B8 = 0x80;
		B9 = 0x10;
		break;

	case TVP_VIDEO_DECODER_MODE_4CH_CIF:
		B0 = 0xA3;
		B8 = 0x70;
		B9 = 0x10;
		break;

	case TVP_VIDEO_DECODER_MODE_4CH_CIF_CROP:
		B0 = 0xA3;
		B1 = 0x50;
		B8 = 0x80;
		B9 = 0x10;
		break;

	default:
		pr_err("[AVIN]%s: error with unsupported ofm mode(%d)\r\n",
			__func__, ret);
		return -1;
	}

	ret = tvp_select_write(0xF);
	if (ret) {
		pr_err("[AVIN]%s(line%d): tvp_select_write error with ret(%d)\r\n",
			__func__, __LINE__, ret);
		return ret;
	}

	idx = 0;
	datasize = 1;
	regAddr[idx] = TVP_REG_AVD_OUTPUT_CONTROL_1;
	regVal[idx] = B0;
	idx++;

	regAddr[idx] = TVP_REG_AVD_OUTPUT_CONTROL_2;
	regVal[idx] = B1;
	idx++;

	ret = tvp_i2c_write(regAddr, regVal, idx, datasize);
	if (ret) {
		pr_err("[AVIN]%s(line%d): tvp_i2c_write error with ret(%d)\r\n",
			__func__, __LINE__, ret);
		return ret;
	}

	ret = tvp_select_write(0x1);
	if (ret) {
		pr_err("[AVIN]%s(line%d): tvp_select_write error with ret(%d)\r\n",
			__func__, __LINE__, ret);
		return ret;
	}

	idx = 0;
	regAddr[idx] = TVP_REG_OFM_MODE_CONTROL;
	regVal[idx] = B2;
	idx++;

	regAddr[idx] = TVP_REG_OFM_CHANNEL_SELECT_1;
	regVal[idx] = B3;
	idx++;

	regAddr[idx] = TVP_REG_OFM_CHANNEL_SELECT_2;
	regVal[idx] = B4;
	idx++;

	regAddr[idx] = TVP_REG_OFM_CHANNEL_SELECT_3;
	regVal[idx] = B5;
	idx++;

	regAddr[idx] = TVP_REG_OFM_SUPER_FRAME_SIZE_1;
	regVal[idx] = B6;
	idx++;

	regAddr[idx] = TVP_REG_OFM_SUPER_FRAME_SIZE_2;
	regVal[idx] = B7;
	idx++;

	ret = tvp_i2c_write(regAddr, regVal, idx, datasize);
	if (ret) {
		pr_err("[AVIN]%s(line%d): tvp_i2c_write error with ret(%d)\r\n",
			__func__, __LINE__, ret);
		return ret;
	}
	msleep(20);

	//do an OFM_reset after setup any mode.
	idx = 0;
	regAddr[idx] = TVP_REG_MISC_OFM_CONTROL;
	regVal[idx] = 0x01;
	idx++;
	ret = tvp_i2c_write(regAddr, regVal, idx, datasize);
	if (ret) {
		pr_err("[AVIN]%s(line%d): tvp_i2c_write error with ret(%d)\r\n",
			__func__, __LINE__, ret);
		return ret;
	}

	g_TvpStartCnt++;

	return 0;
}

int tvp_i2c_stop(void)
{
	int ret = 0;

	if (!g_TvpStartCnt) {
		pr_debug("[AVIN]%s: tvp has not started!\r\n", __func__);
		return 0;
	}

	if (g_TvpStartCnt > 1) {
		pr_debug("[AVIN]%s: tvp don't need to stop!\r\n", __func__);
		g_TvpStartCnt--;
		return 0;
	}

	ret = tvp_soft_reset();
	if (ret) {
		pr_err("[AVIN]%s: tvp_soft_reset error with ret(%d)\r\n",
			__func__, ret);
		return ret;
	}

	g_TvpStartCnt--;

	return 0;
}

static void tvp_get_wch_buf_idx(u32 *pu4BufIdx)
{
	int i4Ret = 0;

	if (NULL == pu4BufIdx)
	{
		pr_err("[AVIN]%s: param pu4BufIdx is NULL!\r\n", __func__);
		return;
	}

	i4Ret = x_msg_q_send(g_hSendBufMsgQ, pu4BufIdx, sizeof(uint32_t), 1);
	if (OSR_OK != i4Ret)
	{
		pr_err("[AVIN]%s: x_msg_q_send fail(%d)!\r\n", __func__, i4Ret);
	}
}

static struct framebuf * tvp_get_next_valid_buf(uint32_t u4ChIdx, int32_t i4CurrIdx)
{
	int32_t i4NextIdx = 0;
	struct framebuf *pBuf = NULL;

	if (0 == g_tvp_data[u4ChIdx].num_frames) {
		g_tvp_data[u4ChIdx].cur_frames = 0;
		return NULL;
	}

	pBuf = &(g_tvp_data[u4ChIdx].buffers[i4CurrIdx]);
	if ((NULL != pBuf) && (NULL != pBuf->data) && (FRAME_EMPTY == pBuf->status))
	{
		g_tvp_data[u4ChIdx].cur_frames = i4CurrIdx;
		return pBuf;
	} else {
		g_u4LogCnt++;
		if (!(g_u4LogCnt % 50)) {
			g_u4LogCnt = 0;
			pr_err("[AVIN]%s: ch(%d) next buf(%d) is invalid with status(%d)!\r\n",
				__func__, u4ChIdx, i4NextIdx, pBuf->status);
		}
		return NULL;
	}
}

static int tvp_parser_thread(void *data)
{
	int32_t i4Ret = 0;
	uint32_t u4LineIdx = 0;
	uint8_t *pSrcYAddr = NULL;
	uint8_t *pSrcCAddr = NULL;
	uint8_t *pDstAddr = NULL;
	uint32_t u4ChId = 0;
	uint32_t u4LineNum = 0;
	uint32_t u4Field = 0;
	uint32_t u4Blank = 0;
	uint8_t  uStartCode[TVP_START_CODE_SIZE] = {0};
	uint32_t u4PreLine[TVP_MAX_VIDEO_DEVS] = {0};
	bool fgStart[TVP_MAX_VIDEO_DEVS] = {0};
	uint32_t u4WchBufIdx = 0;
	uint32_t u4BufWidth = 0;
	UINT16 u2MsgIdx = 0;
	SIZE_T z_msg_size = sizeof(uint32_t);
	struct framebuf *pBuf[TVP_MAX_VIDEO_DEVS];

	if (g_tvp_data[0].pix.width) {
		u4BufWidth = g_tvp_data[0].pix.width;
	} else if (g_tvp_data[1].pix.width) {
		u4BufWidth = g_tvp_data[1].pix.width;
	} else if (g_tvp_data[2].pix.width) {
		u4BufWidth = g_tvp_data[2].pix.width;
	} else if (g_tvp_data[3].pix.width) {
		u4BufWidth = g_tvp_data[3].pix.width;
	} else {
		pr_err("[AVIN]%s: tvp data width error!\r\n", __func__);
		return -1;
	}

	pBuf[0] = &(g_tvp_data[0].buffers[0]);
	pBuf[1] = &(g_tvp_data[1].buffers[0]);
	pBuf[2] = &(g_tvp_data[2].buffers[0]);
	pBuf[3] = &(g_tvp_data[3].buffers[0]);
	while (TRUE) {
		i4Ret = x_msg_q_receive(&u2MsgIdx, &u4WchBufIdx, &z_msg_size, &g_hSendBufMsgQ,
			1, X_MSGQ_OPTION_WAIT);
		if (OSR_OK != i4Ret) {
			pr_err("[AVIN]%s: x_msg_q_receive fail!\r\n", __func__);
			break;
		}
		if (0xFF == u4WchBufIdx) {
			break;
		}

		mutex_lock(&g_WchLock);
		if (true == g_fgExitThread) {
			mutex_unlock(&g_WchLock);
			continue;
		}

		for (u4LineIdx = 0; u4LineIdx < TVP_SUPER_FRAME_HEIGHT_4CH_HALF_D1; u4LineIdx++) {
			pSrcYAddr = (uint8_t *)(g_rTvpWchBufInfo.tWchBuf.u4YBuf[u4WchBufIdx] +
				u4LineIdx * u4BufWidth);
			pSrcCAddr = (uint8_t *)(g_rTvpWchBufInfo.tWchBuf.u4CBuf[u4WchBufIdx] +
				u4LineIdx * u4BufWidth);

			uStartCode[3] = *pSrcYAddr;
			uStartCode[2] = *(pSrcYAddr + 1);
			uStartCode[1] = *(pSrcYAddr + 2);
			uStartCode[0] = *(pSrcYAddr + 3);
			if ((uStartCode[3] == 0x01) && (uStartCode[2] == 0x01) &&
				(uStartCode[1] == 0x01) && (uStartCode[0] == 0x01)) {
				continue;
			}
			if (((uStartCode[3] & 0x80) != 0x80) || ((uStartCode[2] & 0x80) != 0x00) ||
				((uStartCode[0] & 0x90) != 0x80)) {
				//pr_err("[AVIN]%s: Error startcode with SC3:SC1 = 0x%02x%02x%02x%02x\r\n",
					//__func__, uStartCode[3], uStartCode[2], uStartCode[1], uStartCode[0]);
				continue;
			}

			u4ChId = uStartCode[3] & 0x03;
			u4LineNum = (uStartCode[2] & 0x3) << 7;
			u4LineNum += uStartCode[1] & 0x7F;
			u4Field = (uStartCode[0] & 0x40) >> 6;
			u4Blank = (uStartCode[0] & 0x20) >> 5;

			if (1 != g_tvp_data[u4ChId].streaming) {
				continue;
			}

			if ((0 == u4LineNum) && (0 == u4Field)) {
				fgStart[u4ChId] = true;
			}

			if (true == fgStart[u4ChId]) {
				if (((480 == g_tvp_data[u4ChId].pix.height) ||
					(576 == g_tvp_data[u4ChId].pix.height)) &&
					(0 == u4Blank) && (u4LineNum < g_tvp_data[u4ChId].pix.height / 2)) {
					if ((NULL != pBuf[u4ChId]) && ((NULL != pBuf[u4ChId]->data))) {
						if (0 == u4Field) {
							pDstAddr = (uint8_t *)(pBuf[u4ChId]->data +
								u4LineNum * 2 * g_tvp_data[u4ChId].pix.width);
						} else {
							pDstAddr = (uint8_t *)(pBuf[u4ChId]->data +
								(u4LineNum * 2 + 1) * g_tvp_data[u4ChId].pix.width);
						}
						memcpy(pDstAddr, pSrcYAddr + TVP_START_CODE_SIZE,
							g_tvp_data[u4ChId].pix.width - TVP_START_CODE_SIZE);
						memset(pDstAddr + g_tvp_data[u4ChId].pix.width - TVP_START_CODE_SIZE,
							0x10, TVP_START_CODE_SIZE);
						if (0 == (u4LineNum % 2)) {
							if (0 == u4Field) {
								pDstAddr = (uint8_t *)(pBuf[u4ChId]->data +
									(g_tvp_data[u4ChId].pix.height + u4LineNum) * g_tvp_data[u4ChId].pix.width);
							} else {
								pDstAddr = (uint8_t *)(pBuf[u4ChId]->data +
									(g_tvp_data[u4ChId].pix.height + u4LineNum + 1) * g_tvp_data[u4ChId].pix.width);
							}
							memcpy(pDstAddr, pSrcCAddr + TVP_START_CODE_SIZE,
								g_tvp_data[u4ChId].pix.width - TVP_START_CODE_SIZE);
							memset(pDstAddr + g_tvp_data[u4ChId].pix.width - TVP_START_CODE_SIZE,
								0x80, TVP_START_CODE_SIZE);
						}
					} else {
						pBuf[u4ChId] = tvp_get_next_valid_buf(u4ChId, g_tvp_data[u4ChId].cur_frames);
						continue;
					}
				} else if (((240 == g_tvp_data[u4ChId].pix.height) ||
					(288 == g_tvp_data[u4ChId].pix.height)) && (0 == u4Blank) &&
					(0 == u4Field) && (u4LineNum < g_tvp_data[u4ChId].pix.height)) {
					if ((NULL != pBuf[u4ChId]) && ((NULL != pBuf[u4ChId]->data))) {
						pDstAddr = (uint8_t *)(pBuf[u4ChId]->data +
							u4LineNum * g_tvp_data[u4ChId].pix.width);
						memcpy(pDstAddr, pSrcYAddr + TVP_START_CODE_SIZE,
							g_tvp_data[u4ChId].pix.width - TVP_START_CODE_SIZE);
						memset(pDstAddr + g_tvp_data[u4ChId].pix.width - TVP_START_CODE_SIZE,
							0x10, TVP_START_CODE_SIZE);
						if (0 == (u4LineNum % 2)) {
							pDstAddr = (uint8_t *)(pBuf[u4ChId]->data +
								(g_tvp_data[u4ChId].pix.height + u4LineNum / 2) * g_tvp_data[u4ChId].pix.width);
							memcpy(pDstAddr, pSrcCAddr + TVP_START_CODE_SIZE,
								g_tvp_data[u4ChId].pix.width - TVP_START_CODE_SIZE);
							memset(pDstAddr + g_tvp_data[u4ChId].pix.width - TVP_START_CODE_SIZE,
								0x80, TVP_START_CODE_SIZE);
						}
					} else {
						pBuf[u4ChId] = tvp_get_next_valid_buf(u4ChId, g_tvp_data[u4ChId].cur_frames);
						continue;
					}
				}
				if ((NULL != pBuf[u4ChId]) && (0 == u4Field) &&
					(u4LineNum < u4PreLine[u4ChId]) && (0 == u4LineNum)) {
					if (0 == u4ChId) {
						i4Ret = avin_buffer_complete(AVIN_TYPE_AVM_FRONT, (struct capture_priv *)pBuf[u4ChId]);
					} else if (1 == u4ChId) {
						i4Ret = avin_buffer_complete(AVIN_TYPE_AVM_REAR, (struct capture_priv *)pBuf[u4ChId]);
					} else if (2 == u4ChId) {
						i4Ret = avin_buffer_complete(AVIN_TYPE_AVM_LEFT, (struct capture_priv *)pBuf[u4ChId]);
					} else {
						i4Ret = avin_buffer_complete(AVIN_TYPE_AVM_RIGHT, (struct capture_priv *)pBuf[u4ChId]);
					}
					if (i4Ret) {
						pr_debug("[AVIN]%s: ch(%d) reuse buf idx(%d)!\r\n",
							__func__, u4ChId, g_tvp_data[u4ChId].cur_frames);
					} else {
						g_tvp_data[u4ChId].cur_frames = (g_tvp_data[u4ChId].cur_frames + 1) %
							g_tvp_data[u4ChId].num_frames;
					}
					pBuf[u4ChId] = tvp_get_next_valid_buf(u4ChId, g_tvp_data[u4ChId].cur_frames);
				}
				u4PreLine[u4ChId] = u4LineNum;
			}
		}
		mutex_unlock(&g_WchLock);
	}

	g_fgExitThread = false;
	pr_info("[AVIN]%s: thread exit success!\r\n", __func__);


	return 0;
}

int tvp_wch_start(E_TVP_TYPE_T eTVPFmt)
{
	int i4Ret = 0;
	int i4Idx = 0;
	u32 u4Size = 0;
	uint32_t u4WchBufIdx = 0xFF;

	if (g_WchStartCnt) {
		pr_debug("[AVIN]%s: wch has started!\r\n", __func__);
		g_WchStartCnt++;
		return 0;
	}

	g_Thread_task = kthread_create(tvp_parser_thread, NULL, "TVP_Parser_Thread");
	if (IS_ERR(g_Thread_task))
	{
		i4Ret = PTR_ERR(g_Thread_task);
		pr_err("[AVIN]%s: kthread_create fail(%d)!\r\n", __func__, i4Ret);
		g_Thread_task = NULL;
		return i4Ret;
	}
	g_fgExitThread = false;
	wake_up_process(g_Thread_task);

	switch (eTVPFmt)
	{
	case TVP_TYPE_656_P_480:
		g_rTvpWchCtrl.tWchCfg.fgVSyncPolarity = 0; // FALSE is LOW level present sync.
		g_rTvpWchCtrl.tWchCfg.fgHSyncPolarity = 0; // TRUE is High.

		g_rTvpWchCtrl.tWchCfg.eOutputFmt = DATA_FMT_YUV420;
		g_rTvpWchCtrl.tWchCfg.eInputFmt = DATA_FMT_BT656;
		g_rTvpWchCtrl.tWchCfg.u4SrcWidth = 720; //mWidth;
		g_rTvpWchCtrl.tWchCfg.u4SrcHeight = 480; //mHeight;

		g_rTvpWchCtrl.tWchCfg.u4SrcStartX = 0;//may change cause of different hw
		g_rTvpWchCtrl.tWchCfg.u4SrcStartYTop = 0;//above
		g_rTvpWchCtrl.tWchCfg.u4SrcStartYBot = 0x0;//above

		g_rTvpWchCtrl.tWchCfg.u4DstWidth = 720; //mWidth;
		g_rTvpWchCtrl.tWchCfg.u4DstHeight = 480; //mHeight;
		g_rTvpWchCtrl.tWchCfg.fgProgressive = 1;

		g_rTvpWchCtrl.tWchCfg.u1YSel = 1;//may change cause of different hw11
		g_rTvpWchCtrl.tWchCfg.u1USel = 1;//above
		g_rTvpWchCtrl.tWchCfg.u1VSel = 3;//above
		break;

	case TVP_TYPE_656_P_576:
		g_rTvpWchCtrl.tWchCfg.fgVSyncPolarity = 0; // FALSE is LOW level present sync.
		g_rTvpWchCtrl.tWchCfg.fgHSyncPolarity = 0; // TRUE is High.

		g_rTvpWchCtrl.tWchCfg.eOutputFmt = DATA_FMT_YUV420;
		g_rTvpWchCtrl.tWchCfg.eInputFmt = DATA_FMT_BT656;
		g_rTvpWchCtrl.tWchCfg.u4SrcWidth = 720; //mWidth;
		g_rTvpWchCtrl.tWchCfg.u4SrcHeight = 576; //mHeight;

		g_rTvpWchCtrl.tWchCfg.u4SrcStartX = 0;//may change cause of different hw
		g_rTvpWchCtrl.tWchCfg.u4SrcStartYTop = 0;//above
		g_rTvpWchCtrl.tWchCfg.u4SrcStartYBot = 0;//above

		g_rTvpWchCtrl.tWchCfg.u4DstWidth = 720; //mWidth;
		g_rTvpWchCtrl.tWchCfg.u4DstHeight = 576; //mHeight;
		g_rTvpWchCtrl.tWchCfg.fgProgressive = 1;

		g_rTvpWchCtrl.tWchCfg.u1YSel = 1;//may change cause of different hw
		g_rTvpWchCtrl.tWchCfg.u1USel = 1;//above
		g_rTvpWchCtrl.tWchCfg.u1VSel = 3;//above
		break;

	case TVP_TYPE_656_I_480:
		g_rTvpWchCtrl.tWchCfg.fgVSyncPolarity = 0; // FALSE is LOW level present sync.
		g_rTvpWchCtrl.tWchCfg.fgHSyncPolarity = 0; // TRUE is High.

		g_rTvpWchCtrl.tWchCfg.eOutputFmt = DATA_FMT_YUV420;
		g_rTvpWchCtrl.tWchCfg.eInputFmt = DATA_FMT_BT656;
		g_rTvpWchCtrl.tWchCfg.u4SrcWidth = 720; //mWidth;
		g_rTvpWchCtrl.tWchCfg.u4SrcHeight = 480; //mHeight;

		g_rTvpWchCtrl.tWchCfg.u4SrcStartX = 0;//may change cause of different hw
		g_rTvpWchCtrl.tWchCfg.u4SrcStartYTop = 4;//above
		g_rTvpWchCtrl.tWchCfg.u4SrcStartYBot = 4;//above

		g_rTvpWchCtrl.tWchCfg.u4DstWidth = 720; //mWidth;
		g_rTvpWchCtrl.tWchCfg.u4DstHeight = 480; //mHeight;
		g_rTvpWchCtrl.tWchCfg.fgProgressive = 0;

		g_rTvpWchCtrl.tWchCfg.u1YSel = 1;//may change cause of different hw
		g_rTvpWchCtrl.tWchCfg.u1USel = 1;//above
		g_rTvpWchCtrl.tWchCfg.u1VSel = 3;//above
		break;

	case TVP_TYPE_656_I_576:
		g_rTvpWchCtrl.tWchCfg.fgVSyncPolarity = 0; // FALSE is LOW level present sync.
		g_rTvpWchCtrl.tWchCfg.fgHSyncPolarity = 0; // TRUE is High.

		g_rTvpWchCtrl.tWchCfg.eOutputFmt = DATA_FMT_YUV420;
		g_rTvpWchCtrl.tWchCfg.eInputFmt = DATA_FMT_BT656;
		g_rTvpWchCtrl.tWchCfg.u4SrcWidth = 720; //mWidth;
		g_rTvpWchCtrl.tWchCfg.u4SrcHeight = 576; //mHeight;
		g_rTvpWchCtrl.tWchCfg.u4SrcStartX = 0;//may change cause of different hw
		g_rTvpWchCtrl.tWchCfg.u4SrcStartYTop = 0;//above
		g_rTvpWchCtrl.tWchCfg.u4SrcStartYBot = 0;//above

		g_rTvpWchCtrl.tWchCfg.u4DstWidth = 720; //mWidth;
		g_rTvpWchCtrl.tWchCfg.u4DstHeight = 576; //mHeight;
		g_rTvpWchCtrl.tWchCfg.fgProgressive = 0;

		g_rTvpWchCtrl.tWchCfg.u1YSel = 1;//may change cause of different hw
		g_rTvpWchCtrl.tWchCfg.u1USel = 1;//above
		g_rTvpWchCtrl.tWchCfg.u1VSel = 3;//above
		break;

	case TVP_TYPE_656_I_352_480:
		pr_err("[AVIN]%s: TVP_TYPE_656_I_352_480\r\n", __func__);
		g_rTvpWchCtrl.tWchCfg.fgVSyncPolarity = 0; // FALSE is LOW level present sync.
		g_rTvpWchCtrl.tWchCfg.fgHSyncPolarity = 0; // TRUE is High.

		g_rTvpWchCtrl.tWchCfg.eOutputFmt = DATA_FMT_YUV420;
		g_rTvpWchCtrl.tWchCfg.eInputFmt = DATA_FMT_BT656;
		g_rTvpWchCtrl.tWchCfg.u4SrcWidth = 352; //mWidth;
		g_rTvpWchCtrl.tWchCfg.u4SrcHeight = 480; //mHeight;

		g_rTvpWchCtrl.tWchCfg.u4SrcStartX = 0;//may change cause of different hw
		g_rTvpWchCtrl.tWchCfg.u4SrcStartYTop = 4;//above
		g_rTvpWchCtrl.tWchCfg.u4SrcStartYBot = 4;//above

		g_rTvpWchCtrl.tWchCfg.u4DstWidth = 352; //mWidth;
		g_rTvpWchCtrl.tWchCfg.u4DstHeight = 480; //mHeight;
		g_rTvpWchCtrl.tWchCfg.fgProgressive = 0;

		g_rTvpWchCtrl.tWchCfg.u1YSel = 1;//may change cause of different hw
		g_rTvpWchCtrl.tWchCfg.u1USel = 1;//above
		g_rTvpWchCtrl.tWchCfg.u1VSel = 3;//above
		break;

	case TVP_TYPE_656_P_352_2100:
		pr_err("[AVIN]%s: TVP_TYPE_656_P_352_2100\r\n", __func__);
		g_rTvpWchCtrl.tWchCfg.fgVSyncPolarity = 0; // FALSE is LOW level present sync.
		g_rTvpWchCtrl.tWchCfg.fgHSyncPolarity = 0; // TRUE is High.

		g_rTvpWchCtrl.tWchCfg.eOutputFmt = DATA_FMT_YUV422;
		g_rTvpWchCtrl.tWchCfg.eInputFmt = DATA_FMT_BT656;
		g_rTvpWchCtrl.tWchCfg.u4SrcWidth = 352; //mWidth;
		g_rTvpWchCtrl.tWchCfg.u4SrcHeight = 2100;//mHeight;

		g_rTvpWchCtrl.tWchCfg.u4SrcStartX = 0;//may change cause of different hw
		g_rTvpWchCtrl.tWchCfg.u4SrcStartYTop = 0;//above
		g_rTvpWchCtrl.tWchCfg.u4SrcStartYBot = 0;//above

		g_rTvpWchCtrl.tWchCfg.u4DstWidth = 352; //mWidth;
		g_rTvpWchCtrl.tWchCfg.u4DstHeight = 2100;//mHeight;
		g_rTvpWchCtrl.tWchCfg.fgProgressive = 1;

		g_rTvpWchCtrl.tWchCfg.u1YSel = 1;//may change cause of different hw
		g_rTvpWchCtrl.tWchCfg.u1USel = 1;//above
		g_rTvpWchCtrl.tWchCfg.u1VSel = 3;//above
		break;

	default:
		pr_err("[AVIN]%s: Error TVP Fmt: %d\r\n", __func__, eTVPFmt);
		goto error;
	}

	g_rTvpWchCtrl.tWchCfg.eInputSrc = DATA_SRC_DGI;
	g_rTvpWchCtrl.tWchCfg.fgBotFieldFirst = 0;
	g_rTvpWchCtrl.tWchCfg.GetWchBufIndx = tvp_get_wch_buf_idx;
	g_rTvpWchCtrl.eSrcId = SRC_APP_AVM;

	if (ConfigWch(&g_rTvpWchCtrl)) {
		pr_err("[AVIN]%s: ConfigWch fail!\r\n", __func__);
		i4Ret = x_msg_q_send(g_hSendBufMsgQ, &u4WchBufIdx, sizeof(uint32_t), 1);
		if (OSR_OK != i4Ret) {
			pr_err("[AVIN]%s: x_msg_q_send fail(%d)!\r\n", __func__, i4Ret);
		}
		return -1;
	}
	pr_debug("[AVIN]%s: ConfigWch success!\r\n", __func__);

	if (!g_rTvpWchBufInfo.tWchBuf.u4YBuf[0]) {
		g_rTvpWchBufInfo.eSrcId = SRC_APP_AVM;
		if (WchGetBufferAddress(&g_rTvpWchBufInfo)) {
			pr_err("[AVIN]%s:WchGetBufferAddress fail!", __func__);
			goto error;
		}
		pr_debug("[AVIN]%s: WchGetBufferAddress success!\r\n", __func__);
		for (i4Idx = 0; i4Idx < WCH_BUF_MAX_CNT; i4Idx++) {
			pr_debug("[AVIN]%s: Physic Y(%08x) C(%08x)\r\n", __func__,
				g_rTvpWchBufInfo.tWchBuf.u4YBuf[i4Idx], g_rTvpWchBufInfo.tWchBuf.u4CBuf[i4Idx]);
			u4Size = WCH_AVM_YBUF_SIZE;
			g_rTvpWchBufInfo.tWchBuf.u4YBuf[i4Idx] =
				(u32)ioremap(g_rTvpWchBufInfo.tWchBuf.u4YBuf[i4Idx], u4Size);
			u4Size = WCH_AVM_CBUF_SIZE;
			g_rTvpWchBufInfo.tWchBuf.u4CBuf[i4Idx] =
				(u32)ioremap(g_rTvpWchBufInfo.tWchBuf.u4CBuf[i4Idx], u4Size);
			pr_debug("[AVIN]%s: virtual Y(%08x) C(%08x)\r\n", __func__,
				g_rTvpWchBufInfo.tWchBuf.u4YBuf[i4Idx], g_rTvpWchBufInfo.tWchBuf.u4CBuf[i4Idx]);
		}
	}

	if (StartWch(g_rTvpWchCtrl.eSrcId)) {
		pr_err("[AVIN]%s: StartWch fail!\r\n", __func__);
		goto error;
	}
	pr_debug("[AVIN]%s: StartWch success!\r\n", __func__);
	g_WchStartCnt++;

	return 0;

error:
	for (i4Idx = 0; i4Idx < WCH_BUF_MAX_CNT; i4Idx++) {
		if (g_rTvpWchBufInfo.tWchBuf.u4YBuf[i4Idx]) {
			iounmap((void *)g_rTvpWchBufInfo.tWchBuf.u4YBuf[i4Idx]);
			g_rTvpWchBufInfo.tWchBuf.u4YBuf[i4Idx] = 0;
		}
		if (g_rTvpWchBufInfo.tWchBuf.u4CBuf[i4Idx]) {
			iounmap((void *)g_rTvpWchBufInfo.tWchBuf.u4CBuf[i4Idx]);
			g_rTvpWchBufInfo.tWchBuf.u4YBuf[i4Idx] = 0;
		}
	}

	if (CloseWch(g_rTvpWchCtrl.eSrcId)) {
		pr_err("[AVIN]%s: CloseWch fail!\r\n", __func__);
	}
	i4Ret = x_msg_q_send(g_hSendBufMsgQ, &u4WchBufIdx, sizeof(uint32_t), 1);
	if (OSR_OK != i4Ret) {
		pr_err("[AVIN]%s: x_msg_q_send fail(%d)!\r\n", __func__, i4Ret);
	}

	return -1;
}

int tvp_wch_stop(void)
{
	int i4Ret = 0;
	uint32_t u4WchBufIdx = 0xFF;

	if (!g_WchStartCnt) {
		pr_debug("[AVIN]%s: tvp has not started!\r\n", __func__);
		return 0;
	}

	if (g_WchStartCnt > 1) {
		pr_debug("[AVIN]%s: tvp don't need to stop!\r\n", __func__);
		g_WchStartCnt--;
		return 0;
	}

	i4Ret = x_msg_q_send(g_hSendBufMsgQ, &u4WchBufIdx, sizeof(uint32_t), 1);
	if (OSR_OK != i4Ret) {
		pr_err("[AVIN]%s: x_msg_q_send fail(%d)!\r\n", __func__, i4Ret);
		return -1;
	}

	mutex_lock(&g_WchLock);
	if (StopWch(g_rTvpWchCtrl.eSrcId)) {
		pr_err("[AVIN]%s: StopWch fail!\r\n", __func__);
		mutex_unlock(&g_WchLock);
		return -1;
	}
	pr_debug("[AVIN]%s: StopWch success!\r\n", __func__);

	if (CloseWch(g_rTvpWchCtrl.eSrcId)) {
		pr_err("[AVIN]%s: CloseWch fail!\r\n", __func__);
		mutex_unlock(&g_WchLock);
		return -1;
	}
	pr_debug("[AVIN]%s: CloseWch success!\r\n", __func__);
	g_fgExitThread = true;
	mutex_unlock(&g_WchLock);

	g_WchStartCnt--;

	return 0;
}

int tvp_init(unsigned short addr)
{
	struct i2c_board_info info;
	struct i2c_adapter *pAdapter;

	memset(&info, 0, sizeof(struct i2c_board_info));
	pAdapter = i2c_get_adapter(0);
	if (NULL == pAdapter) {
		pr_err("[AVIN]%s: i2c_get_adapter failed!\r\n", __func__);
		return -1;
	}

	info.addr = addr;
	g_pClient = i2c_new_device(pAdapter, &info);
	if (NULL == g_pClient) {
		pr_err("[AVIN]%s: i2c_new_device failed!\r\n", __func__);
		return -1;
	}
	i2c_put_adapter(pAdapter);

	if (!tvp_check_chip()) {
		tvp_download_patch();
	}

	mutex_init(&g_WchLock);
	if (OSR_OK != x_msg_q_create(&g_hSendBufMsgQ, "SendBuf_MSGQ", sizeof(uint32_t), 40)) {
		pr_err("[AVIN]%s: x_msg_q_create fail!\r\n", __func__);
		if (g_pClient) {
			i2c_unregister_device(g_pClient);
			g_pClient = NULL;
		}
		return -1;
	}

	g_TvpStartCnt = 0;
	g_WchStartCnt = 0;
	memset(&g_rTvpWchBufInfo, 0, sizeof(WCH_BUF_T));

	return 0;
}

int tvp_deinit(void)
{
	int i4Idx = 0;

	for (i4Idx = 0; i4Idx < WCH_BUF_MAX_CNT; i4Idx++) {
		if (g_rTvpWchBufInfo.tWchBuf.u4YBuf[i4Idx]) {
			iounmap((void *)g_rTvpWchBufInfo.tWchBuf.u4YBuf[i4Idx]);
			g_rTvpWchBufInfo.tWchBuf.u4YBuf[i4Idx] = 0;
		}
		if (g_rTvpWchBufInfo.tWchBuf.u4CBuf[i4Idx]) {
			iounmap((void *)g_rTvpWchBufInfo.tWchBuf.u4CBuf[i4Idx]);
			g_rTvpWchBufInfo.tWchBuf.u4CBuf[i4Idx] = 0;
		}
	}

	if (g_pClient) {
		i2c_unregister_device(g_pClient);
		g_pClient = NULL;
	}

	if (NULL_HANDLE != g_hSendBufMsgQ) {
		if (OSR_OK != x_msg_q_delete(g_hSendBufMsgQ)) {
			pr_err("[AVIN]%s: x_msg_q_delete fail!\r\n", __func__);
		}
		g_hSendBufMsgQ = NULL_HANDLE;
	}

	g_TvpStartCnt = 0;
	g_WchStartCnt = 0;

	return 0;
}

