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

#ifdef _GPIO_UBOOT_

#include <mach/ac83xx_gpio_pinmux.h>
#include <mach/ac83xx_pinmux_table.h>
#include <linux/module.h>

#else
#include <linux/gpio.h>
#include <mach/ac83xx_basic.h>

#include <mach/ac83xx_gpio_pinmux.h>
#include <mach/ac83xx_pinmux_table.h>

#include <mach/pinmux.h>
#include <mach/pinmux_reg.h>

#include <linux/module.h>
#include <linux/printk.h>

#endif

/*============================================================================
** CKGEN Registers
============================================================================*/
#define REG_RW_TST_CFG0                0x005C

#define REG_RW_PINMUX_OFFSET           0x054  /* Pinmux offset      */
#define REG_RW_GPIO_EN_OFFSET          0x074  /* GPIO Enable offset */
#define REG_RW_GPIO_OUT_OFFSET         0x0E0  /* GPIO OUT offset    */
#define REG_RW_GPIO_IN_OFFSET          0x100  /* GPIO IN offset     */
#define REG_RW_GPIO_PU_OFFSET          0x400  /* GPIO UP offset     */
#define REG_RW_GPIO_PD_OFFSET          0x418  /* GPIO DOWN offset   */

#define REG_RW_MSDC_CONFIG_OFFSET      0x308  /* GPIO MSDC CONFIG   */
#define REG_RW_TTL_CONFIG_OFFSET       0x298  /* GPIO TTL CONFIG    */
#define REG_RW_VB_CONFIG_OFFSET        0x94   /* VB0 VB1 CONFIG     */
#define REG_R_TRAPPING_MODE_OFFSET     0x180  /* TRAPPING MODE      */

#ifdef _GPIO_UBOOT_

#define CKGEN_VIRT 0xF0000000

#define HAL_WRITE32(_reg_, _val_)  (*((volatile uint32_t*)(_reg_)) = (_val_))
#define HAL_READ32(_reg_)          (*((volatile uint32_t*)(_reg_)))

#define IO_READ32(base, offset)    HAL_READ32((base) + (offset))
#define IO_WRITE32(base, offset, value)  \
	HAL_WRITE32((base) + (offset), (value))
#define	IO_REG32(base, offset)	   HAL_READ32((base) + (offset))

#define CKGEN_READ32(offset)       IO_READ32(CKGEN_VIRT, (offset))
#define CKGEN_WRITE32(offset, value)  \
	IO_WRITE32(CKGEN_VIRT, (offset), (value))

#endif

#define GPIO_EN_WRITE(idx, val)    \
	CKGEN_WRITE32((REG_RW_GPIO_EN_OFFSET+(4*(idx))), (val))
#define GPIO_OUT_WRITE(idx, val)   \
	CKGEN_WRITE32((REG_RW_GPIO_OUT_OFFSET+(4*(idx))), (val))
#define GPIO_IN_WRITE(idx, val)    \
	CKGEN_WRITE32((REG_RW_GPIO_IN_OFFSET+(4*(idx))), (val))
#define GPIO_OUT_REG(idx)   CKGEN_READ32(REG_RW_GPIO_OUT_OFFSET+(4*(idx)))
#define GPIO_EN_REG(idx)    CKGEN_READ32(REG_RW_GPIO_EN_OFFSET+(4*(idx)))
#define GPIO_IN_REG(idx)    CKGEN_READ32(REG_RW_GPIO_IN_OFFSET+(4*(idx)))

#define GPIO_MSDC_CONFIG_WR(val)  \
	CKGEN_WRITE32(REG_RW_MSDC_CONFIG_OFFSET, (val))
#define GPIO_MSDC_CONFIG_RD()     CKGEN_READ32(REG_RW_MSDC_CONFIG_OFFSET)

#define GPIO_TTL_CONFIG_WR(val) CKGEN_WRITE32(REG_RW_TTL_CONFIG_OFFSET, (val))
#define GPIO_TTL_CONFIG_RD()    CKGEN_READ32(REG_RW_TTL_CONFIG_OFFSET)

#define GPIO_VB_CONFIG_WR(val)  CKGEN_WRITE32(REG_RW_VB_CONFIG_OFFSET, (val))
#define GPIO_VB_CONFIG_RD()     CKGEN_READ32(REG_RW_VB_CONFIG_OFFSET)


#define PINMUX_WRITE(idx, val)   \
	CKGEN_WRITE32((REG_RW_PINMUX_OFFSET+(4*(idx))), (val))
#define PINMUX_REG(idx)         CKGEN_READ32(REG_RW_PINMUX_OFFSET+(4*(idx)))

#define TRAPPING_MODE_RD()      CKGEN_READ32(REG_R_TRAPPING_MODE_OFFSET)

/*---------------------------Set GPIO Pull Up-------------------------*/
#define REG_RW_GPIO_PU_OFFSET                       0x400
#define GPIO_PU_REG(idx)      CKGEN_READ32(REG_RW_GPIO_PU_OFFSET+(4*(idx)))
#define GPIO_PU_WRITE(idx, val)    \
	CKGEN_WRITE32((REG_RW_GPIO_PU_OFFSET+(4*(idx))), (val))

/*---------------------------Set GPIO Pull Down-----------------------*/
#define REG_RW_GPIO_PD_OFFSET                       0x418
#define GPIO_PD_REG(idx)      CKGEN_READ32(REG_RW_GPIO_PD_OFFSET+(4*(idx)))
#define GPIO_PD_WRITE(idx, val)    \
	CKGEN_WRITE32((REG_RW_GPIO_PD_OFFSET+(4*(idx))), (val))

static inline unsigned gpio_get_addridx(unsigned gpio_pinmux)
{
	return gpio_pinmux / 32;
}

static inline unsigned gpio_get_addroffset(unsigned gpio_pinmux)
{
	return gpio_pinmux % 32;
}

static inline int valid_gpio(unsigned gpio)
{
	return gpio <= TOTAL_GPIO_NUM;
}

static inline int valid_pinmux(unsigned pinmux_sel, unsigned function)
{
	return ((function >= 0) &&
			(function <= MAX_PINMUX_FUNCTION) &&
			(function <= _au1PinmuxFunctionMasks[pinmux_sel]) &&
			(pinmux_sel >= 0) &&
			(pinmux_sel <= MAX_PINMUX_SEL) &&
			(_au1PinmuxFunctionMasks[pinmux_sel] != 0));
}

int ac83xx_gpio_inout_sel_reg(unsigned gpio, int dir)
{
	unsigned val, idx, offset;
	int ret = -1;

	if (gpio <= PIN_191_VR5) { /* 191 */
		offset = gpio_get_addroffset(gpio);
		idx = gpio_get_addridx(gpio);

		val = GPIO_EN_REG(idx);
		val = (dir == OUTPUT) ? (val | (1U << offset)) :
			(val & ~(1U << offset));
		GPIO_EN_WRITE(idx, val);

		ret = 0;
	} else if (gpio <= TOTAL_GPIO_NUM) { /* 192~195 */
		offset = gpio_get_addroffset(gpio) + 16;
		idx = gpio_get_addridx(gpio);
		idx = idx + 1; /* Reg_en not be used */
		val = GPIO_EN_REG(idx);
		val = (dir == OUTPUT) ? (val | (1U << offset)) :
			(val & ~(1U << offset));
		GPIO_EN_WRITE(idx, val);
		ret = 0;
	}

	return ret;
}

int ac83xx_gpio_get_value_reg(unsigned gpio)
{
	unsigned val, idx, offset;

	if (valid_gpio(gpio)) {
		if (gpio <= PIN_204_VGA_VSYNC_GPI) {
			idx = gpio_get_addridx(gpio);
			offset = gpio_get_addroffset(gpio);

			val = GPIO_IN_REG(idx);

			return (val & (1U << offset)) ? 1 : 0;
		}
	}

	return -1;
}

int ac83xx_gpio_set_value_reg(unsigned gpio, int value)
{
	unsigned val, idx, offset;

	if (valid_gpio(gpio)) {
		if (gpio <= PIN_204_VGA_VSYNC_GPI) {
			offset = gpio_get_addroffset(gpio);
			idx = gpio_get_addridx(gpio);
			val = GPIO_OUT_REG(idx);
			val = (value == 1) ? (val | (1U << offset)) :
					(val & ~(1U << offset));
			GPIO_OUT_WRITE(idx, val);
		}
		return 0;
	} else
		return -1;
}

int ac83xx_gpio_direction_input_reg(unsigned gpio)
{
	int ret = -1;

	if (valid_gpio(gpio)) {
		ret = ac83xx_gpio_inout_sel_reg(gpio, INPUT);
		if (ret == 0) {
			ret = ac83xx_gpio_get_value_reg(gpio);
		}

		return ret;
	} else
		return -1;
}

int ac83xx_gpio_direction_output_reg(unsigned gpio, int value)
{
	int ret = 0;

	if (valid_gpio(gpio)) {
		ret = ac83xx_gpio_set_value_reg(gpio, value);
		ret |= ac83xx_gpio_inout_sel_reg(gpio, OUTPUT);

		return ret;
	} else
		return -1;
}

int __bsp_pinset(unsigned pinmux_sel, unsigned function)
{
	unsigned mask, offset, idx, val;
	if (valid_pinmux(pinmux_sel, function)) {
		offset = gpio_get_addroffset((pinmux_sel));
		idx = gpio_get_addridx((pinmux_sel));
		mask = ~((unsigned)_au1PinmuxFunctionMasks[pinmux_sel] <<
				offset);
		val = PINMUX_REG(idx);
		val &= mask;
		val |= ((unsigned)function << offset);
		PINMUX_WRITE(idx, val);

		return 0;
	} else
		return -1;
}

int __bsp_pinget(unsigned pinmux_sel)
{
	unsigned mask, offset, idx, val;

	if (valid_pinmux(pinmux_sel, 0)) {
		offset = gpio_get_addroffset((pinmux_sel));
		idx = gpio_get_addridx((pinmux_sel));
		mask = _au1PinmuxFunctionMasks[pinmux_sel];

		val = PINMUX_REG(idx) >> offset;
		val &= mask;

		return (int)val;
	} else
		return -1;
}

unsigned int GPIO_MultiFun_Get(unsigned int i4GpioNum)
{
	int u4Val = INVALID_PIN_FUNCTION;
	unsigned int u4Tmp = 0;
	unsigned int u4Group = 0;
	if (i4GpioNum >= TOTAL_GPIO_NUM) {
		return -1;
	}
	do {
		u4Val = ((unsigned int)_au1PinmuxFunctionSel[i4GpioNum][u4Tmp]);
		u4Group = ((unsigned int)_au1PinmuxFunctionSel\
						[i4GpioNum][u4Tmp+1]);
		if (u4Val == PINMUX_LEVEL_INVALID_FLAG) {
			;
		} else if (__bsp_pinget(u4Val) == u4Group) {
			return u4Val;
		}
		u4Tmp += 2;
	} while (u4Val != PINMUX_LEVEL_GPIO_END_FLAG);

	return u4Val;
}
EXPORT_SYMBOL(GPIO_MultiFun_Get);

void Extern_Uart_set(unsigned int i4GpioNum,  unsigned int i4FuncSel)
{
	unsigned int u4ValTemp = 0;
	unsigned int u4MaskTmp = 0;
	bool   fgSpeUart = false;
	unsigned int u4SetValue = 0;

	if (!((i4FuncSel == DVD_RS232_SEL_ADDITION) ||
			(i4FuncSel == AP_RS232_SEL_ADDITION))) {
		return;
	}
	if ((i4GpioNum == PIN_174_VG6) || (i4GpioNum == PIN_175_VG7)) {
		u4SetValue = 0x0;
		fgSpeUart = true;
	} else if ((i4GpioNum == PIN_68_AO2N) || (i4GpioNum == PIN_69_AO2P)) {
		u4SetValue = 0x1;
		fgSpeUart = true;
	} else if ((i4GpioNum == PIN_47_HSYNC_IN) ||
			(i4GpioNum == PIN_46_VSYNC_IN)) {
		u4SetValue = 0x2;
		fgSpeUart = true;
	}
	if (fgSpeUart) {
		if (i4FuncSel == DVD_RS232_SEL_ADDITION) {
			i4FuncSel = DVD_RS232_SEL;
		} else {
			i4FuncSel = AP_RS232_SEL;
		}
		u4MaskTmp = ~((unsigned int)_au1PinmuxFunctionMasks\
					[i4FuncSel] << (i4FuncSel % 32));
		u4ValTemp = PINMUX_REG(i4FuncSel / 32);
		u4ValTemp &= u4MaskTmp;
		u4ValTemp |= u4SetValue << (i4FuncSel%32);
		PINMUX_WRITE((i4FuncSel / 32), (u4ValTemp));
	}

	return;
}

int GPIO_MultiFun_Set(int i4GpioNum,  int i4FuncSel)
{
	unsigned int u4Val = 0;
	unsigned int u4Tmp = 0;
	unsigned int  u4Group = 0;

	if (i4GpioNum == 74) {
		pr_debug("BT CLOCK GPIO:%d; FuncSel:%d\n", i4GpioNum,
				i4FuncSel);
	}
	if (i4GpioNum >= TOTAL_GPIO_NUM) {
		return -1;
	}
	u4Tmp = 0;
	do {
		/* The function Array */
		u4Val = ((unsigned int)_au1PinmuxFunctionSel[i4GpioNum][u4Tmp]);
		/* the value you set into reg:bits */
		u4Group = ((unsigned int)_au1PinmuxFunctionSel\
						[i4GpioNum][u4Tmp+1]);
		/* o, i find it */
		if ((u4Val == i4FuncSel) &&
				u4Val != PINMUX_LEVEL_GPIO_END_FLAG) {
			/* set the function you want */
			if (-1 == __bsp_pinset(u4Val, u4Group)) {
				return -1;
			}
		} else if (u4Val == PINMUX_LEVEL_INVALID_FLAG) {
			;
		} else {
			/* if Pin Func has been set as other Function */
			if (__bsp_pinget(u4Val) == u4Group) {
				/* add warning message */

				/* clear the function */
				__bsp_pinset(u4Val, PINMUX_FUNCTION0);
			}
		}

		if ((i4GpioNum > 79 && i4GpioNum < 84) || (i4GpioNum > 195 &&
				i4GpioNum < 198)) {
			unsigned int msdcconfig = 0x0;
			msdcconfig = GPIO_MSDC_CONFIG_RD();
			if (i4FuncSel == PINMUX_LEVEL_GPIO_END_FLAG) {
				msdcconfig = msdcconfig | 0x3f;
			} else {
				msdcconfig = msdcconfig & 0xffffffc0;
			}
			GPIO_MSDC_CONFIG_WR(msdcconfig);
		}

		if (i4GpioNum == 161 || (i4GpioNum > 162 && i4GpioNum < 168)
				|| (i4GpioNum > 169 && i4GpioNum < 176)
				|| (i4GpioNum > 187 && i4GpioNum < 194)) {
			unsigned int ttl6_8config = 0x0;
			ttl6_8config = GPIO_TTL_CONFIG_RD();
			if (i4FuncSel == PINMUX_LEVEL_GPIO_END_FLAG) {
				ttl6_8config = ttl6_8config & 0x7FFFFFFF;
			} else {
				ttl6_8config = ttl6_8config | 0x80000000;
			}
			GPIO_TTL_CONFIG_WR(ttl6_8config);
		}

		if ((i4GpioNum > 158 && i4GpioNum < 161)
				|| (i4GpioNum > 167 && i4GpioNum < 170)
				|| (i4GpioNum > 185 && i4GpioNum < 188)) {
			unsigned int ttl_8config = 0x0;
			ttl_8config = GPIO_TTL_CONFIG_RD();
			if (i4FuncSel == PINMUX_LEVEL_GPIO_END_FLAG) {
				ttl_8config = ttl_8config & 0xBFFFFFFF;
			} else {
				ttl_8config = ttl_8config | 0x40000000;
			}
			GPIO_TTL_CONFIG_WR(ttl_8config);
		}

		if (i4GpioNum == 159 || i4GpioNum == 160
				|| i4GpioNum == 163) { /* vb0 vb1 */
			unsigned int vb_config = 0x0;
			vb_config = GPIO_VB_CONFIG_RD();
			if (i4FuncSel == PINMUX_LEVEL_GPIO_END_FLAG) {
				vb_config = vb_config | 0x40;
			} else {
				vb_config = vb_config & 0xFFFFFFBF;
			}
			GPIO_VB_CONFIG_WR(vb_config);
			vb_config = GPIO_VB_CONFIG_RD();
		}

		u4Tmp += 2;
	} while (u4Val != PINMUX_LEVEL_GPIO_END_FLAG);
	Extern_Uart_set(i4GpioNum, i4FuncSel);
	return 0;
}
EXPORT_SYMBOL(GPIO_MultiFun_Set);

int AC_BoardType_Get(void)
{
	unsigned int trappingmode = 0x0;
	trappingmode = TRAPPING_MODE_RD();
	trappingmode = trappingmode & 0x80;
	return trappingmode;
}
EXPORT_SYMBOL(AC_BoardType_Get);

void GPIO_DriveCurrent_Set(int i4GpioNum,  int i4Current)
{
	pr_debug("pin %d current is set to %d\n", i4GpioNum, i4Current);
}
EXPORT_SYMBOL(GPIO_DriveCurrent_Set);

void GPIO_PullUp(int i4GpioNum, GPIO_PUD ePullUD)
{
	unsigned int u1Offset = 0;
	unsigned int u4Val = 0;

	pr_debug("pin %d pull to %d\n", i4GpioNum, ePullUD);

	if ((i4GpioNum > TOTAL_GPIO_NUM)
		|| ((unsigned int)PIN_PULL_UP_OR_DOWN_OFFSET[i4GpioNum]
		== INVALID_PUD_GPIO)) {
		return;
	}
	u1Offset = (unsigned int)PIN_PULL_UP_OR_DOWN_OFFSET[i4GpioNum];
	if (ePullUD == PULLUP) {
		u4Val = GPIO_PU_REG(u1Offset / 32);
		u4Val = u4Val | (1 << (u1Offset % 32));
		GPIO_PU_WRITE(u1Offset / 32, u4Val);
		u4Val = GPIO_PD_REG(u1Offset / 32);
		u4Val = u4Val & ~(1 << (u1Offset % 32));
		GPIO_PD_WRITE(u1Offset / 32, u4Val);
	} else {
		u4Val = GPIO_PU_REG(u1Offset / 32);
		u4Val = u4Val & ~(1 << (u1Offset % 32));
		GPIO_PU_WRITE(u1Offset / 32, u4Val);
	}
}

void GPIO_PullDown(int i4GpioNum, GPIO_PUD ePullUD)
{
	unsigned int u1Offset;
	unsigned int u4Val;
	if ((i4GpioNum > TOTAL_GPIO_NUM) ||
		((unsigned int)PIN_PULL_UP_OR_DOWN_OFFSET[i4GpioNum]
		== INVALID_PUD_GPIO)) {
		return;
	}
	u1Offset = (unsigned int)PIN_PULL_UP_OR_DOWN_OFFSET[i4GpioNum];
	if (ePullUD == PULLDOWN) {
		u4Val = GPIO_PD_REG(u1Offset / 32);
		u4Val = u4Val | (1 << (u1Offset % 32));
		GPIO_PD_WRITE(u1Offset / 32, u4Val);

		u4Val = GPIO_PU_REG(u1Offset / 32);
		u4Val = u4Val & ~(1 << (u1Offset % 32)) ;
		GPIO_PU_WRITE(u1Offset / 32, u4Val);
	} else { /* normal */
		u4Val = GPIO_PD_REG(u1Offset / 32);
		u4Val = u4Val & ~(1 << (u1Offset%32));
		GPIO_PD_WRITE(u1Offset / 32, u4Val);
	}
}

void GPIO_Pull_UpDown(int i4GpioNum, unsigned PullUpOrDown)
{
	unsigned char u1PullUD = PullUpOrDown;

	if ((i4GpioNum >= TOTAL_GPIO_NUM)) {
		return;
	}

	switch (u1PullUD) {
	case PULLUP:
		GPIO_PullDown(i4GpioNum, NORMAL);
		GPIO_PullUp(i4GpioNum, PULLUP);
		break;

	case PULLDOWN:
		GPIO_PullUp(i4GpioNum, NORMAL);
		GPIO_PullDown(i4GpioNum, PULLDOWN);
		break;

	case NORMAL:
		GPIO_PullDown(i4GpioNum, NORMAL);
		GPIO_PullUp(i4GpioNum, NORMAL);
		break;

	default:
		break;
	}
	return;
}
EXPORT_SYMBOL(GPIO_Pull_UpDown);

