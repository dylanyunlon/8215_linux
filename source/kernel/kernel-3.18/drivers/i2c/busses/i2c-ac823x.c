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

#include <linux/module.h>
#include <linux/delay.h>
#include <linux/i2c.h>
#include <linux/err.h>
#include <linux/interrupt.h>
#include <linux/completion.h>
#include <linux/platform_device.h>
#include <linux/clk.h>
#include <linux/io.h>
#include <linux/slab.h>
#include <linux/pm_runtime.h>
#include "i2c-ac823x.h"
#include <linux/gpio.h>
#include <linux/printk.h>
#include <linux/gpio/consumer.h>
#include <linux/pinctrl/consumer.h>
#include <linux/printk.h>
#include <generated/atc_project.h>

#define I2C_DEBUG 1
//#define I2C_TEST  1

#define PRINT_STRING	"[I2C] "
#ifdef I2C_DEBUG
#define I2C_DBG(fmt, ...)				\
	({							\
		pr_info(fmt, ##__VA_ARGS__); \
	})
#else
	#define I2C_DBG(fmt, ...)
#endif
/* version info */
#define I2C_VER_NAME    "I2C"
#define I2C_VER_MAIN     01
#define I2C_VER_MINOR    00
#define I2C_VER_REV      00

/* timeout waiting for the controller to respond */
#define  AC83XX_I2C_TIMEOUT    (msecs_to_jiffies(1000))
#define  CONFIG_SUSPEND_I2C    ((unsigned int)1)
/*****************************************************************************
* Local variable
*****************************************************************************/
static int i2c_nr = 0;
struct gpio_desc * pI2c0Gpio1, * pI2c0Gpio2;
struct gpio_desc * pI2c1Gpio1, * pI2c1Gpio2;
struct pinctrl * pinctrl_i2c0;
struct pinctrl * pinctrl_i2c1;
struct pinctrl_state * pins_i2c0_st1;
struct pinctrl_state * pins_i2c0_st2;
struct pinctrl_state * pins_i2c0_st3;
struct pinctrl_state * pins_i2c0_st4;
struct pinctrl_state * pins_i2c1_st1;
struct pinctrl_state * pins_i2c1_st2;
struct pinctrl_state * pins_i2c1_st3;
struct pinctrl_state * pins_i2c1_st4;
/*========================================================================*/

struct ac83xx_i2c_dev {
	struct device       *dev;
	int                  irq;
	u32                  speed;/* Speed of bus in Khz */
	u32                  suspended;
	u8                   *buf;
	u8                   *regs;
    int                  id;
	size_t               buf_len;
	struct i2c_adapter   adapter;
};

/********************for i2c test***************************/
#ifdef I2C_TEST

#define REG_CONFIG_ADDR   0x8047
#define REG_ADDR_LEN      2

static void i2c_transfer_data(struct i2c_client *client, u8 *buf, int len)
{
    int ret = 0;
    u8 retries;
    
    struct i2c_msg msgs[2] = {
        {
            .flags = !I2C_M_RD,
            .addr  = client->addr,
            .len   = REG_ADDR_LEN,
            .buf   = &buf[0],
        },
        {
            .flags = I2C_M_RD,
            .addr  = client->addr,
            .len   = len - REG_ADDR_LEN,
            .buf   = &buf[REG_ADDR_LEN],
        },
    };
    for (retries = 0; retries < 1; retries ++) {
        ret = i2c_transfer(client->adapter, msgs, 2);
        if (ret == 2) {
            break;
        }
        pr_info("i2c retry:%d\n", retries);
    }
}

static int i2c_test(int id)
{
    struct i2c_adapter *adapter;
    struct i2c_client client;
	  int i, j;
    u8 buf[3] = {REG_CONFIG_ADDR >> 8, REG_CONFIG_ADDR & 0xff, 0};
    
    pr_info("i2c_init\n");
		j=5;
		while(j--)
		{
			//for(i=0; i<1; i++)
			i=id;
			{
			    adapter = i2c_get_adapter(i);/*get i2c master 0/1 to transfer data*/
			    client.flags = 0;
			    client.addr = 0x5D;
			    client.adapter = adapter;
			    i2c_transfer_data(&client, buf, 3);
			    pr_info("adapter i2c%d, read data=0x%x\n", i, buf[2]); 
			    if (buf[2] != 0)
			        pr_info("i2c%d test successfully\n", i);
			    else
			        pr_info("i2c%d test failed\n", i);
			}
		}
    
    return 0;
}
#endif
/*******************************************************/
static int SIFM_TrigMode(u32 u4Mode)
{
	SIFM_SIF_MODE_WRITE(u4Mode);
	SIF_SET_BIT(SIF_SIFM0CTL1, SIFM_TRI);
	while (IS_SIF_BIT(SIF_SIFM0CTL1, SIFM_TRI))
		;

	return 0;
}
static int SIFM1_TrigMode(u32 u4Mode)
{
	SIFM1_SIF_MODE_WRITE(u4Mode);
	SIF_SET_BIT(SIF_SIFM1CTL1, SIFM_TRI);
	while (IS_SIF_BIT(SIF_SIFM1CTL1, SIFM_TRI))
		;

	return 0;
}

static int ac83xx_i2c_init(struct ac83xx_i2c_dev *dev)
{
	u32 u4Tmp = 0;
	int ret = 0;

	//I2C_DBG(KERN_ERR " ac823x_i2c_init\n");
    
	/* select master0 & master1 */
	u4Tmp = SIF_IO_READ32(SIF_SEL);
	u4Tmp |= SIF_SEL_M0M1;
	SIF_IO_WRITE32(SIF_SEL, u4Tmp);

	if (dev->id == 0) {    /* master0 set pinmux and clock*/
		pinctrl_i2c0 = devm_pinctrl_get(dev->dev);
		if (IS_ERR(pinctrl_i2c0)) {
			pr_err("pinctrl_i2c0 init failed!");
		}

		/* gpio_request(PIN_112_SCL0,"PIN_112_SCL0"); */
		pI2c0Gpio1 = __gpiod_get(dev->dev, "i2c01", GPIOD_ASIS);
		if (IS_ERR(pI2c0Gpio1)) {
			pr_err("get gpio i2c01 failed");
		}
		/* GPIO_MultiFun_Set(PIN_112_SCL0, I2C0_SEL); */
		pins_i2c0_st1 = pinctrl_lookup_state(pinctrl_i2c0, "i2c0_state_1");
		if (IS_ERR(pins_i2c0_st1)) {
			pr_err("lookup pins_i2c0_st1 failed!");
		}
		ret = pinctrl_select_state(pinctrl_i2c0, pins_i2c0_st1);
		if (ret) {
			pr_err("select pins_i2c0_st1 failed!");
		}

		/* gpio_request(PIN_117_SDA0,"PIN_117_SDA0"); */
		pI2c0Gpio2 = __gpiod_get(dev->dev, "i2c02", GPIOD_ASIS);
		if (IS_ERR(pI2c0Gpio2)) {
			pr_err("get gpio i2c02 failed");
		}
		/* GPIO_MultiFun_Set(PIN_117_SDA0, I2C0_SEL); */
		pins_i2c0_st3 = pinctrl_lookup_state(pinctrl_i2c0, "i2c0_state_3");
		if (IS_ERR(pins_i2c0_st3)) {
			pr_err("lookup pins_i2c0_st3 failed!");
		}
		ret = pinctrl_select_state(pinctrl_i2c0, pins_i2c0_st3);
		if (ret) {
			pr_err("select pins_i2c0_st3 failed!");
		}
		
		u4Tmp = SIF_IO_READ32(SIF_CLOCK);
		u4Tmp |= SIFM0_CLOCK;
		u4Tmp &= ~SIFS0_CLOCK;
		SIF_IO_WRITE32(SIF_CLOCK, u4Tmp);

		u4Tmp = SIF_IO_READ32(SIF_RESET);
		u4Tmp |= SIFM0_RESET;
		SIF_IO_WRITE32(SIF_RESET, u4Tmp);

		/* enable sif master0 */
		SIF_SET_BIT(SIF_SIFM0CTL0, SIFM_SM0EN);
		/* output pull-high */
		SIF_SET_BIT(SIF_SIFM0CTL0, SIFM_ODRAIN);
		/* init SCL line value */
		SIF_SET_BIT(SIF_SIFM0CTL0, SIFM_SCL_STATE);
		/* init SDA line value */
		SIF_SET_BIT(SIF_SIFM0CTL0, SIFM_SDA_STATE);
	}
	if (dev->id == 1) {    /* master1 set pinmux and clock */
		pinctrl_i2c1 = devm_pinctrl_get(dev->dev);
		if (IS_ERR(pinctrl_i2c1)) {
			pr_err("pinctrl_i2c1 init failed!");
		}

		/* gpio_request(PIN_113_SCL1,"PIN_113_SCL1"); */
		pI2c1Gpio1 = __gpiod_get(dev->dev, "i2c11", GPIOD_ASIS);
		if (IS_ERR(pI2c1Gpio1)) {
			pr_err("get gpio i2c11 failed");
		}
		/* GPIO_MultiFun_Set(PIN_113_SCL1, I2C1_SEL); */
		pins_i2c1_st1 = pinctrl_lookup_state(pinctrl_i2c1, "i2c1_state_1");
		if (IS_ERR(pins_i2c1_st1)) {
			pr_err("lookup pins_i2c1_st1 failed!");
		}
		ret = pinctrl_select_state(pinctrl_i2c1, pins_i2c1_st1);
		if (ret) {
			pr_err("select pins_i2c1_st1 failed!");
		}

		/* gpio_request(PIN_118_SDA1,"PIN_118_SDA1"); */
		pI2c1Gpio2 = __gpiod_get(dev->dev, "i2c12", GPIOD_ASIS);
		if (IS_ERR(pI2c1Gpio2)) {
			pr_err("get gpio i2c12 failed");
		}
		/* GPIO_MultiFun_Set(PIN_118_SDA1, I2C1_SEL); */
		pins_i2c1_st3 = pinctrl_lookup_state(pinctrl_i2c1, "i2c1_state_3");
		if (IS_ERR(pins_i2c1_st3)) {
			pr_err("lookup pins_i2c1_st3 failed!");
		}
		ret = pinctrl_select_state(pinctrl_i2c1, pins_i2c1_st3);
		if (ret) {
			pr_err("select pins_i2c1_st3 failed!");
		}
		#if defined(CONFIG_ATC_PRJ_ac823x_evb)
		u4Tmp = SIF_IO_READ32(SIF_CLOCK);
		u4Tmp |= SIFM1_CLOCK;
		SIF_IO_WRITE32(SIF_CLOCK, u4Tmp);
		#elif defined (CONFIG_ATC_PRJ_ac823x_adas)
		u4Tmp = SIF_IO_READ32(SIF_CLOCK);
		u4Tmp &= ~SIFM1_CLOCK;
		u4Tmp &= ~SIFS1_CLOCK;
		SIF_IO_WRITE32(SIF_CLOCK, u4Tmp);
		#endif
		u4Tmp = SIF_IO_READ32(SIF_RESET);
		u4Tmp |= SIFM1_RESET;
		SIF_IO_WRITE32(SIF_RESET, u4Tmp);

		/* enable sif master1 */
		SIF_SET_BIT(SIF_SIFM1CTL0, SIFM_SM0EN);
		/* output pull-high */
		SIF_SET_BIT(SIF_SIFM1CTL0, SIFM_ODRAIN);
		/* init SCL line value */
		SIF_SET_BIT(SIF_SIFM1CTL0, SIFM_SCL_STATE);
		/* init SDA line value */
		SIF_SET_BIT(SIF_SIFM1CTL0, SIFM_SDA_STATE);
	}

	SIF_CLR_BIT(SIFM_INTEN, 1);
	SIF_SET_BIT(SIFM_INTCLR, 1);

	SIF_CLR_BIT(SIFM1_INTEN, 1);
	SIF_SET_BIT(SIFM1_INTCLR, 1);

	/* set cloclk speed,default is 400k */
	SIFM_CLK_DIV_WRITE(68);
	SIFM1_CLK_DIV_WRITE(900);

	/* enable gloabl ISR */
	/* SIF_SET_BIT(SIF_INTEN, SIFM_INTEN); */


	return 0;
}

/*
 * Waiting on Bus Busy
 */
static int ac83xx_i2c_wait_for_bb(struct ac83xx_i2c_dev *dev)
{
	unsigned long timeout;
	timeout = jiffies + AC83XX_I2C_TIMEOUT;
	if (dev->adapter.nr == 0) {
		while (SIF_READ32(SIF_SIFM0CTL1) & SIFM_BUSY) {
			if (time_after(jiffies, timeout)) {
				I2C_DBG("[I2C]wait for bus ready:timeout\n");
				return -ETIMEDOUT;
			}
			msleep(20);
		}
	}
	if (dev->adapter.nr == 1) {
		while (SIF_READ32(SIF_SIFM1CTL1) & SIFM_BUSY) {
			if (time_after(jiffies, timeout)) {
				I2C_DBG("[I2C]wait for bus ready:timeout\n");
				return -ETIMEDOUT;
			}
			msleep(20);
		}
	}
	return 0;
}

static int SifMRead(u32 ucDev, u8 *pucValue, u32 u4Count, u32 NoRDAck)
{
	u32 u4Ack, ucReadCount, ucIdx, ucAckCount, ucAckFinal, ucTmpCount;
	if ((pucValue == NULL) || (u4Count == 0)) {
		I2C_DBG(KERN_INFO "Data is not right\n");
		return -EIO;
	}

	ucIdx = 0;

	SIFM_DATA0_WRITE(((ucDev<<1) + 1));
	SIFM_PGLEN_WRITE(0x00);
	SIFM_TrigMode(SIFM_WRITE_DATA);
	u4Ack = SIFM_ACK_READ();
	if (u4Ack != 0x1) {
		pr_debug("MASTER0 READ ACK FAILURE\n");
		return -EIO;
	}

	ucAckCount = (u4Count-1)/8;
	ucAckFinal = 0;
	while (u4Count > 0) {
		if (ucAckCount > 0) {
			ucReadCount = 8;
			ucAckFinal = 0;
			ucAckCount--;
		} else {
			ucReadCount = u4Count;
			ucAckFinal = 1;
		}

		SIFM_PGLEN_WRITE((ucReadCount - 1));
		if (NoRDAck)
			SIFM_TrigMode(SIFM_READ_DATA_NO_ACK);
		else {
			SIFM_TrigMode((ucAckFinal == 1)
						? SIFM_READ_DATA_NO_ACK
						: SIFM_READ_DATA_ACK);

			u4Ack = SIFM_ACK_READ();
			for (ucTmpCount = 0;
				((u4Ack & (1 << ucTmpCount)) != 0) && \
				(ucTmpCount < 8); ucTmpCount++) {
				;
			}

			if (((ucAckFinal == 1)
				&& ((ucTmpCount) != (ucReadCount-1)))
				|| ((ucAckFinal == 0)
				&& (ucTmpCount != ucReadCount))) {
				break;
			}
		}

		switch (ucReadCount) {
		case 8:
			pucValue[ucIdx + 7] = SIFM_DATA7_READ();
		case 7:
			pucValue[ucIdx + 6] = SIFM_DATA6_READ();
		case 6:
			pucValue[ucIdx + 5] = SIFM_DATA5_READ();
		case 5:
			pucValue[ucIdx + 4] = SIFM_DATA4_READ();
		case 4:
			pucValue[ucIdx + 3] = SIFM_DATA3_READ();
		case 3:
			pucValue[ucIdx + 2] = SIFM_DATA2_READ();
		case 2:
			pucValue[ucIdx + 1] = SIFM_DATA1_READ();
		case 1:
			pucValue[ucIdx + 0] = SIFM_DATA0_READ();
		default:
			break;
		}

		u4Count -= ucReadCount;
		ucIdx += ucReadCount;
	}

	//I2C_DBG(KERN_INFO "MASTER0 READ OKAY\n");
	return 0;
}

static int SifM1Read(u32 ucDev,  u8 *pucValue, u32 u4Count, u32 NoRDAck)
{
	u32 u4Ack, ucReadCount, ucIdx, ucAckCount, ucAckFinal, ucTmpCount;

	if ((pucValue == NULL) || (u4Count == 0)) {
		I2C_DBG(KERN_INFO "Data is not right\n");
		return -EIO;
	}

	ucIdx = 0;

	SIFM1_DATA0_WRITE(((ucDev<<1) + 1));
	SIFM1_PGLEN_WRITE(0x00);
	SIFM1_TrigMode(SIFM_WRITE_DATA);
	u4Ack = SIFM1_ACK_READ();
	if (u4Ack != 0x1) {
		I2C_DBG(KERN_INFO "MASTER1 READ ACK FAILURE\n");
		return -EIO;
	}

	ucAckCount = (u4Count-1)/8;
	ucAckFinal = 0;
	while (u4Count > 0) {
		if (ucAckCount > 0) {
			ucReadCount = 8;
			ucAckFinal = 0;
			ucAckCount--;
		} else {
			ucReadCount = u4Count;
			ucAckFinal = 1;
		}

		SIFM1_PGLEN_WRITE((ucReadCount - 1));
		if (NoRDAck)
			SIFM1_TrigMode(SIFM_READ_DATA_NO_ACK);
		else {
			SIFM1_TrigMode((ucAckFinal == 1)
						? SIFM_READ_DATA_NO_ACK
						: SIFM_READ_DATA_ACK);

			u4Ack = SIFM1_ACK_READ();
			for (ucTmpCount = 0;
				((u4Ack & (1 << ucTmpCount)) != 0)
				&& (ucTmpCount < 8); ucTmpCount++) {
				;
			}

			if (((ucAckFinal == 1)
				&& ((ucTmpCount) != (ucReadCount-1)))
				|| ((ucAckFinal == 0)
				&& (ucTmpCount != ucReadCount))) {
				break;
			}
		}

		switch (ucReadCount) {
		case 8:
			pucValue[ucIdx + 7] = SIFM1_DATA7_READ();
		case 7:
			pucValue[ucIdx + 6] = SIFM1_DATA6_READ();
		case 6:
			pucValue[ucIdx + 5] = SIFM1_DATA5_READ();
		case 5:
			pucValue[ucIdx + 4] = SIFM1_DATA4_READ();
		case 4:
			pucValue[ucIdx + 3] = SIFM1_DATA3_READ();
		case 3:
			pucValue[ucIdx + 2] = SIFM1_DATA2_READ();
		case 2:
			pucValue[ucIdx + 1] = SIFM1_DATA1_READ();
		case 1:
			pucValue[ucIdx + 0] = SIFM1_DATA0_READ();
		default:
			break;
		}

		u4Count -= ucReadCount;
		ucIdx += ucReadCount;
	}

	I2C_DBG(KERN_INFO "MASTER1 READ OKAY\n");
	return 0;
}

static int SifMWrite(u32 ucDev, const u8 *pucValue, u32 u4Count)
{
	u32 u4Ack, ucWriteCount, ucIdx, ucTmpCount;

	ucIdx = 0;

	if ((pucValue == NULL) || (u4Count == 0)) {
		return -EIO;
	}

	SIFM_DATA0_WRITE((ucDev<<1));
	SIFM_PGLEN_WRITE(0x00);
	SIFM_TrigMode(SIFM_WRITE_DATA);
	u4Ack = SIFM_ACK_READ();
	if (u4Ack != 0x1) {
		pr_debug("MASTER0 WRITE ACK FAILURE\n");
		return -EIO;
	}

	while (u4Count > 0) {
		ucWriteCount = (u4Count > 8) ? 8 : (u4Count);

		switch (ucWriteCount) {
		case 8:
			SIFM_DATA7_WRITE(pucValue[ucIdx + 7]);
		case 7:
			SIFM_DATA6_WRITE(pucValue[ucIdx + 6]);
		case 6:
			SIFM_DATA5_WRITE(pucValue[ucIdx + 5]);
		case 5:
			SIFM_DATA4_WRITE(pucValue[ucIdx + 4]);
		case 4:
			SIFM_DATA3_WRITE(pucValue[ucIdx + 3]);
		case 3:
			SIFM_DATA2_WRITE(pucValue[ucIdx + 2]);
		case 2:
			SIFM_DATA1_WRITE(pucValue[ucIdx + 1]);
		case 1:
			SIFM_DATA0_WRITE(pucValue[ucIdx + 0]);
		default:
			break;
		}

		SIFM_PGLEN_WRITE((ucWriteCount - 1));
		SIFM_TrigMode(SIFM_WRITE_DATA);

		u4Ack = SIFM_ACK_READ();
		for (ucTmpCount = 0;
			((u4Ack & (1 << ucTmpCount)) != 0)
			&& (ucTmpCount < 8); ucTmpCount++) {
			;
		}
		if (ucTmpCount != ucWriteCount) {
			break;
		}

		u4Count -= ucWriteCount;
		ucIdx += ucWriteCount;
	}

	//I2C_DBG(KERN_INFO "MASTER0 WRITE OKAY\n");
	return 0;
}

static int SifM1Write(u32 ucDev, const u8 *pucValue, u32 u4Count)
{
	u32 u4Ack, ucWriteCount, ucIdx, ucTmpCount;

	ucIdx = 0;

	if ((pucValue == NULL) || (u4Count == 0)) {
		return -EIO;
	}

	SIFM1_DATA0_WRITE((ucDev<<1));
	SIFM1_PGLEN_WRITE(0x00);
	SIFM1_TrigMode(SIFM_WRITE_DATA);
	u4Ack = SIFM1_ACK_READ();
	if (u4Ack != 0x1) {
		I2C_DBG(KERN_INFO "MASTER1 WRITE ACK FAILURE\n");
		return -EIO;
	}

	while (u4Count > 0) {
		ucWriteCount = (u4Count > 8) ? 8 : (u4Count);

		switch (ucWriteCount) {
		case 8:
			SIFM1_DATA7_WRITE(pucValue[ucIdx + 7]);
		case 7:
			SIFM1_DATA6_WRITE(pucValue[ucIdx + 6]);
		case 6:
			SIFM1_DATA5_WRITE(pucValue[ucIdx + 5]);
		case 5:
			SIFM1_DATA4_WRITE(pucValue[ucIdx + 4]);
		case 4:
			SIFM1_DATA3_WRITE(pucValue[ucIdx + 3]);
		case 3:
			SIFM1_DATA2_WRITE(pucValue[ucIdx + 2]);
		case 2:
			SIFM1_DATA1_WRITE(pucValue[ucIdx + 1]);
		case 1:
			SIFM1_DATA0_WRITE(pucValue[ucIdx + 0]);
		default:
			break;
		}

		SIFM1_PGLEN_WRITE((ucWriteCount - 1));
		SIFM1_TrigMode(SIFM_WRITE_DATA);

		u4Ack = SIFM1_ACK_READ();
		for (ucTmpCount = 0;
			((u4Ack & (1 << ucTmpCount)) != 0)
			&& (ucTmpCount < 8); ucTmpCount++) {
			;
		}

		if (ucTmpCount != ucWriteCount) {
			break;
		}

		u4Count -= ucWriteCount;
		ucIdx += ucWriteCount;
	}

	I2C_DBG(KERN_INFO "MASTER1 WRITE OKAY\n");
	return 0;
}

/*
 * Low level master read/write transaction.
 */
static int ac83xx_i2c_xfer_msg(struct i2c_adapter *adap,
				struct i2c_msg *msg, int stop)
{
	struct ac83xx_i2c_dev *dev = i2c_get_adapdata(adap);
	u32 NoStart, NoRdAck;
	int ret = -1;

	//I2C_DBG("addr: 0x%04x, len: %d, flags: 0x%x, stop: %d\n",
		//msg->addr, msg->len, msg->flags, stop);

	if (msg->len == 0)
		return -EINVAL;
	if (dev == NULL) {
		pr_err("i2c_dev is null, get_adapdate fail\n");
		return -EIO;
	}
	if (dev->suspended)
		return -EIO;

	dev->buf = msg->buf;
	dev->buf_len = msg->len;
	NoStart = ((msg->flags) & I2C_M_NOSTART);
	NoRdAck = ((msg->flags) & I2C_M_NO_RD_ACK);

	/************************************************************/
	if (adap->nr == 0) {
		/* start bit */
		if (0 == NoStart)
			SIFM_TrigMode(SIFM_START);

		if ((msg->flags)&I2C_M_RD)
			ret = SifMRead(msg->addr, dev->buf,
						dev->buf_len, NoRdAck);
		else
			ret = SifMWrite(msg->addr, dev->buf, dev->buf_len);

		/* stop bit */
		if (stop)
			SIFM_TrigMode(SIFM_STOP);
		if (!ret)
			return msg->len;
		else
			return ret;
	}
	if (adap->nr == 1) {
		/* start bit */
		if (0 == NoStart)
			SIFM1_TrigMode(SIFM_START);

		if ((msg->flags)&I2C_M_RD)
			ret = SifM1Read(msg->addr, dev->buf,
						dev->buf_len, NoRdAck);
		else
			ret = SifM1Write(msg->addr, dev->buf, dev->buf_len);

		/* stop bit */
		if (stop)
			SIFM1_TrigMode(SIFM_STOP);
		if (!ret)
			return msg->len;
		else
			return ret;
	}
	return ret;
}

/*
 * Prepare controller for a transaction and call ac83xx_i2c_xfer_msg
 * to do the work during IRQ processing.
 */
static int ac83xx_i2c_xfer(struct i2c_adapter *adap,
		struct i2c_msg msgs[], int num)
{
	struct ac83xx_i2c_dev *dev = i2c_get_adapdata(adap);
	int i;
	int r;
	int count = 0;

	if (dev == NULL) {
		pr_err("dev is null, i2c_get_adapdata fail\n");
		return -EIO;
	}
	r = ac83xx_i2c_wait_for_bb(dev);
	if (r < 0)
		goto out;

	for (i = 0; i < num; i++) {
		r = ac83xx_i2c_xfer_msg(adap, &msgs[i], (i == (num - 1)));
		if (r < 0)
			return -EIO;
		count += r;
	}

	r = ac83xx_i2c_wait_for_bb(dev);
	if (r == 0)
		r = num;
out:
	return r;
}

static u32 ac83xx_i2c_func(struct i2c_adapter *adap)
{
	return I2C_FUNC_I2C | I2C_FUNC_SMBUS_EMUL
			| I2C_FUNC_PROTOCOL_MANGLING;
}

static const struct i2c_algorithm ac83xx_i2c_algo = {
	.master_xfer    = ac83xx_i2c_xfer,
	.functionality  = ac83xx_i2c_func,
};



static int ac83xx_i2c_probe(struct platform_device *pdev)
{
	struct ac83xx_i2c_dev *dev;
	struct i2c_adapter  *adap;
	int r;

	pdev->id = i2c_nr++;

	dev = kzalloc(sizeof(struct ac83xx_i2c_dev), GFP_KERNEL);
	if (!dev) {
		r = -ENOMEM;
		goto err_free_mem;
	}

	dev->speed = 400;   /* 400kHz */
	dev->dev = &pdev->dev;
	dev->suspended = 0;
    dev->id = pdev->id ;

	platform_set_drvdata(pdev, dev);
	ac83xx_i2c_init(dev);

	adap = &dev->adapter;
	i2c_set_adapdata(adap, dev);
	adap->owner = THIS_MODULE;
	adap->class = I2C_CLASS_HWMON;
	strlcpy(adap->name, "ac83xx I2C adapter", sizeof(adap->name));
	adap->algo = &ac83xx_i2c_algo;
	adap->dev.parent = &pdev->dev;
	adap->dev.of_node = pdev->dev.of_node;

	/* i2c device drivers may be active on return from add_adapter() */
	adap->nr = pdev->id;
	r = i2c_add_numbered_adapter(adap);
	if (r) {
		I2C_DBG("failure adding adapter\n");
		goto err_free_irq;
	}
	I2C_DBG("ac83xx_i2c probe sucess\n");

	#ifdef I2C_TEST
		printk("[I2C]test i2c master %d\n", pdev->id);
		i2c_test(pdev->id);
	#endif
	
	return 0;

err_free_irq:
	/*    free_irq(dev->irq, dev);    */
err_free_mem:
	platform_set_drvdata(pdev, NULL);
	kfree(dev);

	return r;
}

static int ac83xx_i2c_remove(struct platform_device *pdev)
{
	int ret = 0;
	struct ac83xx_i2c_dev *i2c = platform_get_drvdata(pdev);

	platform_set_drvdata(pdev, NULL);
	i2c_del_adapter(&i2c->adapter);
	kfree(i2c);

	/* clear pinmux setting */
	pins_i2c0_st2 = pinctrl_lookup_state(pinctrl_i2c0, "i2c0_state_2");
	if (IS_ERR(pins_i2c0_st2)) {
		pr_err("lookup pins_i2c0_st2 failed!");
	}
	ret = pinctrl_select_state(pinctrl_i2c0, pins_i2c0_st2);
	if (ret) {
		pr_err("select pins_i2c0_st2 failed!");
	}

	pins_i2c0_st4 = pinctrl_lookup_state(pinctrl_i2c0, "i2c0_state_4");
	if (IS_ERR(pins_i2c0_st4)) {
		pr_err("lookup pins_i2c0_st4 failed!");
	}
	ret = pinctrl_select_state(pinctrl_i2c0, pins_i2c0_st4);
	if (ret) {
		pr_err("select pins_i2c0_st4 failed!");
	}

	pins_i2c1_st2 = pinctrl_lookup_state(pinctrl_i2c1, "i2c1_state_2");
	if (IS_ERR(pins_i2c1_st2)) {
		pr_err("lookup pins_i2c1_st2 failed!");
	}
	ret = pinctrl_select_state(pinctrl_i2c1, pins_i2c1_st2);
	if (ret) {
		pr_err("select pins_i2c1_st2 failed!");
	}

	pins_i2c1_st4 = pinctrl_lookup_state(pinctrl_i2c1, "i2c1_state_4");
	if (IS_ERR(pins_i2c1_st4)) {
		pr_err("lookup pins_i2c1_st4 failed!");
	}
	ret = pinctrl_select_state(pinctrl_i2c1, pins_i2c1_st4);
	if (ret) {
		pr_err("select pins_i2c1_st4 failed!");
	}

	/* release gpio */
	gpiod_put(pI2c0Gpio1);
	gpiod_put(pI2c0Gpio2);
	gpiod_put(pI2c1Gpio1);
	gpiod_put(pI2c1Gpio2);

	return 0;
}

#ifdef CONFIG_SUSPEND_I2C
static int ac83xx_i2c_suspend(struct device *dev)
{
	struct platform_device *pdev = to_platform_device(dev);
	struct ac83xx_i2c_dev *i2c = platform_get_drvdata(pdev);
	if (i2c == NULL) {
		pr_err("i2c_dev is null, get_drvdate fail\n");
		return -EIO;
	}
	i2c->suspended = 1;
	pr_debug("[I2C] suspend\n");

	return 0;
}

static int ac83xx_i2c_resume(struct device *dev)
{
	struct platform_device *pdev = to_platform_device(dev);
	struct ac83xx_i2c_dev  *i2c = platform_get_drvdata(pdev);
	if (i2c == NULL) {
		pr_err("i2c_dev is null, get_drvdate fail\n");
		return -EIO;
	}
	i2c->suspended = 0;
	ac83xx_i2c_init(i2c);
	pr_debug("[I2C] resume\n");

	return 0;
}

static const struct dev_pm_ops ac83xx_i2c_pm_ops = {
	.suspend = ac83xx_i2c_suspend,
	.resume = ac83xx_i2c_resume,
};
#define ac83xx_I2C_PM_OPS (&ac83xx_i2c_pm_ops)
#else
#define ac83xx_I2C_PM_OPS NULL
#endif

#if 0
static struct platform_driver ac83xx_i2c_driver = {
	.probe      = ac83xx_i2c_probe,
	.remove     = ac83xx_i2c_remove,
	.driver     = {
		.name   = "ac83xx_i2c",
		.owner  = THIS_MODULE,
		.pm     = ac83xx_I2C_PM_OPS,
	},
};
#else

static const struct of_device_id ac83xx_i2c_of_match[] = {
	{
		.compatible = "autochips,i2c0",
	},
	{
		.compatible = "autochips,i2c1",
	},
	{ },
};

static struct platform_driver ac83xx_i2c_driver = {
	.probe		= ac83xx_i2c_probe,
	.remove		= ac83xx_i2c_remove,
	.driver		= {
		.name	= "ac83xx_i2c",
		.owner	= THIS_MODULE,
		.pm	    = ac83xx_I2C_PM_OPS,
		.of_match_table = of_match_ptr(ac83xx_i2c_of_match),
	},
};

#endif

/* I2C may be needed to bring up other drivers */
static int __init
ac83xx_i2c_init_driver(void)
{
	int ret;

	I2C_DBG(KERN_ERR "i2c-ac83xx: probe\n");
	ret = platform_driver_register(&ac83xx_i2c_driver);
	if (ret)
		I2C_DBG(KERN_ERR "i2c-ac83xx: probe failed: %d\n", ret);

	I2C_DBG(KERN_ERR "i2c-ac83xx: probe OKAY\n");

	return ret;
}
subsys_initcall(ac83xx_i2c_init_driver);

static void __exit ac83xx_i2c_exit_driver(void)
{
	platform_driver_unregister(&ac83xx_i2c_driver);
}
module_exit(ac83xx_i2c_exit_driver);

MODULE_AUTHOR("ATC");
MODULE_DESCRIPTION("ATC ac83xx I2C bus adapter");
MODULE_LICENSE("GPL");
