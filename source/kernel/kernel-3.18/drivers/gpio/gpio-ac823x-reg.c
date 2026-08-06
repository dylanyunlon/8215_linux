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
#include "gpio_ac823x_pinmux.h"
#include "gpio_ac823x_pinmux_table.h"
#include "ac823x_pinmux.h"
#include "ac823x_pinmux_reg.h"


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

//#ifdef _GPIO_UBOOT_
#if 0

#define GPIO_823X_VIRT 0xF0000000

#define HAL_WRITE32(_reg_, _val_)  (*((volatile uint32_t*)(_reg_)) = (_val_))
#define HAL_READ32(_reg_)          (*((volatile uint32_t*)(_reg_)))

#define GPIOIO_READ32(base, offset)    HAL_READ32((base) + (offset))
#define GPIOIO_WRITE32(base, offset, value)  \
	HAL_WRITE32((base) + (offset), (value))
#define	IO_REG32(base, offset)	   HAL_READ32((base) + (offset))

#define GPIO_823X_READ32(offset)       GPIOIO_READ32(GPIO_823X_VIRT, (offset))
#define GPIO_823X_WRITE32(offset, value)  \
	GPIOIO_WRITE32(GPIO_823X_VIRT, (offset), (value))

#endif

#define GPIO_EN_WRITE(idx, val)		GPIO_823X_WRITE32((REG_RW_GPIO_EN_OFFSET+(4*(idx))), (val))
#define GPIO_OUT_WRITE(idx, val)   \
	GPIO_823X_WRITE32((REG_RW_GPIO_OUT_OFFSET+(4*(idx))), (val))
#define GPIO_IN_WRITE(idx, val)    \
	GPIO_823X_WRITE32((REG_RW_GPIO_IN_OFFSET+(4*(idx))), (val))
#define GPIO_OUT_REG(idx)   GPIO_823X_READ32(REG_RW_GPIO_OUT_OFFSET+(4*(idx)))
#define GPIO_EN_REG(idx)    GPIO_823X_READ32(REG_RW_GPIO_EN_OFFSET+(4*(idx)))
#define GPIO_IN_REG(idx)    GPIO_823X_READ32(REG_RW_GPIO_IN_OFFSET+(4*(idx)))

#define GPIO_MSDC_CONFIG_WR(val)  \
	GPIO_823X_WRITE32(REG_RW_MSDC_CONFIG_OFFSET, (val))
#define GPIO_MSDC_CONFIG_RD()     GPIO_823X_READ32(REG_RW_MSDC_CONFIG_OFFSET)

#define GPIO_TTL_CONFIG_WR(val) GPIO_823X_WRITE32(REG_RW_TTL_CONFIG_OFFSET, (val))
#define GPIO_TTL_CONFIG_RD()    GPIO_823X_READ32(REG_RW_TTL_CONFIG_OFFSET)

#define GPIO_VB_CONFIG_WR(val)  GPIO_823X_WRITE32(REG_RW_VB_CONFIG_OFFSET, (val))
#define GPIO_VB_CONFIG_RD()     GPIO_823X_READ32(REG_RW_VB_CONFIG_OFFSET)


#define PINMUX_WRITE(idx, val)   \
	GPIO_823X_WRITE32((REG_RW_PINMUX_OFFSET+(4*(idx))), (val))
#define PINMUX_REG(idx)         GPIO_823X_READ32(REG_RW_PINMUX_OFFSET+(4*(idx)))

#define TRAPPING_MODE_RD()      GPIO_823X_READ32(REG_R_TRAPPING_MODE_OFFSET)

/*---------------------------Set GPIO Pull Up-------------------------*/
#define REG_RW_GPIO_PU_OFFSET                       0x400
#define GPIO_PU_REG(idx)      GPIO_823X_READ32(REG_RW_GPIO_PU_OFFSET+(4*(idx)))
#define GPIO_PU_WRITE(idx, val)    \
	GPIO_823X_WRITE32((REG_RW_GPIO_PU_OFFSET+(4*(idx))), (val))

/*---------------------------Set GPIO Pull Down-----------------------*/
#define REG_RW_GPIO_PD_OFFSET                       0x418
#define GPIO_PD_REG(idx)      GPIO_823X_READ32(REG_RW_GPIO_PD_OFFSET+(4*(idx)))
#define GPIO_PD_WRITE(idx, val)    \
	GPIO_823X_WRITE32((REG_RW_GPIO_PD_OFFSET+(4*(idx))), (val))

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

int atc_set_gpio_dir(unsigned gpio, int dir)
{
	unsigned val, idx, offset;
	int ret = -1;
	if ((gpio == PIN_197_GPIO7) || (gpio == PIN_219_GPIO8) || (gpio == PIN_220_GPIO9)) {
		offset = (gpio == PIN_197_GPIO7) ? (3):(gpio - 215);
		GPIO_823X_WRITE32(0x240F4, (GPIO_823X_READ32(0x240F4) | (1<<offset))); //enable gpio
		val = GPIO_823X_READ32(0x240D4);
		val = (dir == OUTPUT) ? (val | (1U << offset)) :
			(val & ~(1U << offset));
		GPIO_823X_WRITE32(0x240D4, val); //set input/output
		
	}else if (gpio <= PIN_191_YIN2) { /* 191 */
		offset = gpio_get_addroffset(gpio);
		idx = gpio_get_addridx(gpio);

		val = GPIO_EN_REG(idx);
		val = (dir == OUTPUT) ? (val | (1U << offset)) :
			(val & ~(1U << offset));
		GPIO_EN_WRITE(idx, val);

		ret = 0;
	} else if (gpio <= TOTAL_GPIO_NUM) { /* 192~195 */
		offset = gpio_get_addroffset(gpio);
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
EXPORT_SYMBOL(atc_set_gpio_dir);

int atc_get_gpio_in(unsigned gpio)
{
	unsigned val, idx, offset;

	if (valid_gpio(gpio)) {
		if ((gpio == PIN_197_GPIO7) || (gpio == PIN_219_GPIO8) || (gpio == PIN_220_GPIO9)) {
			val = GPIO_823X_READ32(0x240D0);
			offset = (gpio == PIN_197_GPIO7) ? (3):(gpio - 215);
			return (val & (1U << offset)) ? 1 : 0;
			
		}else if (gpio <= PIN_220_GPIO9) {
			idx = gpio_get_addridx(gpio);
			offset = gpio_get_addroffset(gpio);

			val = GPIO_IN_REG(idx);

			return (val & (1U << offset)) ? 1 : 0;
		}
	}

	return -1;
}
EXPORT_SYMBOL(atc_get_gpio_in);

int atc_set_gpio_out(unsigned gpio, int value)
{
	unsigned val, idx, offset;

	//printk("set value gpio=%d, value=%d", gpio, value);
	if (valid_gpio(gpio)) {
		if ((gpio == PIN_197_GPIO7) || (gpio == PIN_219_GPIO8) || (gpio == PIN_220_GPIO9)) {
			offset = (gpio == PIN_197_GPIO7) ? (3):(gpio - 215);
			val = GPIO_823X_READ32(0x240D8);
			val = (value == 1) ? (val | (1U << offset)) :
					(val & ~(1U << offset));
			GPIO_823X_WRITE32(0x240D8, val);
		}else if (gpio <= PIN_220_GPIO9) {
			offset = gpio_get_addroffset(gpio);
			idx = gpio_get_addridx(gpio);
			val = GPIO_OUT_REG(idx);
			val = (value == 1) ? (val | (1U << offset)) :
					(val & ~(1U << offset));
			//printk("set value idx=%d, val=%d", idx, val);
			GPIO_OUT_WRITE(idx, val);
		}
		return 0;
	} else
		return -1;
}
EXPORT_SYMBOL(atc_set_gpio_out);

int ac83xx_gpio_direction_input_reg(unsigned gpio)
{
	int ret = -1;

	if (valid_gpio(gpio)) {
		ret = atc_set_gpio_dir(gpio, INPUT);
		if (ret == 0) {
			ret = atc_get_gpio_in(gpio);
		}

		return ret;
	} else
		return -1;
}

int ac83xx_gpio_direction_output_reg(unsigned gpio, int value)
{
	int ret = 0;

	if (valid_gpio(gpio)) {
		ret = atc_set_gpio_out(gpio, value);
		ret |= atc_set_gpio_dir(gpio, OUTPUT);

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

unsigned int GPIO_MultiFun_Get(unsigned int gpionum)
{
	int u4Val = INVALID_PIN_FUNCTION;
	unsigned int u4Tmp = 0;
	unsigned int u4Group = 0;
	if (gpionum >= TOTAL_GPIO_NUM) {
		return -1;
	}
	do {
		u4Val = ((unsigned int)_au1PinmuxFunctionSel[gpionum][u4Tmp]);
		u4Group = ((unsigned int)_au1PinmuxFunctionSel\
						[gpionum][u4Tmp+1]);
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

void Extern_Uart_set(unsigned int gpionum,  unsigned int funcsel)
{
	unsigned int u4ValTemp = 0;
	unsigned int u4MaskTmp = 0;
	bool   fgSpeUart = false;
	unsigned int u4SetValue = 0;

	/*
	if (!((funcsel == DVD_RS232_SEL_ADDITION) ||
			(funcsel == AP_RS232_SEL_ADDITION))) {
		return;
	}
	*/

	/*
	if ((gpionum == PIN_174_VG6) || (gpionum == PIN_175_VG7)) {
		u4SetValue = 0x0;
		fgSpeUart = true;
	} else if ((gpionum == PIN_68_AO2N) || (gpionum == PIN_69_AO2P)) {
		u4SetValue = 0x1;
		fgSpeUart = true;
	} else if ((gpionum == PIN_47_HSYNC_IN) ||
			(gpionum == PIN_46_VSYNC_IN)) {
		u4SetValue = 0x2;
		fgSpeUart = true;
	}
	*/
	
	if (fgSpeUart) {
		/*
		if (funcsel == DVD_RS232_SEL_ADDITION) {
			funcsel = DVD_RS232_SEL;
		} else {
			funcsel = AP_RS232_SEL;
		}
		*/
		u4MaskTmp = ~((unsigned int)_au1PinmuxFunctionMasks\
					[funcsel] << (funcsel % 32));
		u4ValTemp = PINMUX_REG(funcsel / 32);
		u4ValTemp &= u4MaskTmp;
		u4ValTemp |= u4SetValue << (funcsel%32);
		PINMUX_WRITE((funcsel / 32), (u4ValTemp));
	}

	return;
}

int atc_set_gpio_pinmux(int gpionum,  int funcsel)
{
	unsigned int u4Val = 0;
	unsigned int u4Tmp = 0;
	unsigned int  u4Group = 0;

	
	if (gpionum >= TOTAL_GPIO_NUM) {
		return -1;
	}
	u4Tmp = 0;
	do {
		/* The function Array */
		u4Val = ((unsigned int)_au1PinmuxFunctionSel[gpionum][u4Tmp]);
		/* the value you set into reg:bits */
		u4Group = ((unsigned int)_au1PinmuxFunctionSel\
						[gpionum][u4Tmp+1]);
		/* o, i find it */
		if ((u4Val == funcsel) &&
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

		/*if ((gpionum > 79 && gpionum < 84) || (gpionum > 195 &&
				gpionum < 198)) {
			unsigned int msdcconfig = 0x0;
			msdcconfig = GPIO_MSDC_CONFIG_RD();
			if (funcsel == PINMUX_LEVEL_GPIO_END_FLAG) {
				msdcconfig = msdcconfig | 0x3f;
			} else {
				msdcconfig = msdcconfig & 0xffffffc0;
			}
			GPIO_MSDC_CONFIG_WR(msdcconfig);
		}
		*/
		if (gpionum == 161 || (gpionum > 162 && gpionum < 168)
				|| (gpionum > 169 && gpionum < 176)
				|| (gpionum > 187 && gpionum < 194)) {
			unsigned int ttl6_8config = 0x0;
			ttl6_8config = GPIO_TTL_CONFIG_RD();
			if (funcsel == PINMUX_LEVEL_GPIO_END_FLAG) {
				ttl6_8config = ttl6_8config & 0x7FFFFFFF;
			} else {
				ttl6_8config = ttl6_8config | 0x80000000;
			}
			GPIO_TTL_CONFIG_WR(ttl6_8config);
		}

		if ((gpionum > 158 && gpionum < 161)
				|| (gpionum > 167 && gpionum < 170)
				|| (gpionum > 185 && gpionum < 188)) {
			unsigned int ttl_8config = 0x0;
			ttl_8config = GPIO_TTL_CONFIG_RD();
			if (funcsel == PINMUX_LEVEL_GPIO_END_FLAG) {
				ttl_8config = ttl_8config & 0xBFFFFFFF;
			} else {
				ttl_8config = ttl_8config | 0x40000000;
			}
			GPIO_TTL_CONFIG_WR(ttl_8config);
		}

		if (gpionum == 159 || gpionum == 160
				|| gpionum == 163) { /* vb0 vb1 */
			unsigned int vb_config = 0x0;
			vb_config = GPIO_VB_CONFIG_RD();
			if (funcsel == PINMUX_LEVEL_GPIO_END_FLAG) {
				vb_config = vb_config | 0x40;
			} else {
				vb_config = vb_config & 0xFFFFFFBF;
			}
			GPIO_VB_CONFIG_WR(vb_config);
			vb_config = GPIO_VB_CONFIG_RD();
		}

		u4Tmp += 2;
	} while (u4Val != PINMUX_LEVEL_GPIO_END_FLAG);
	Extern_Uart_set(gpionum, funcsel);
	return 0;
}
EXPORT_SYMBOL(atc_set_gpio_pinmux);

int AC_BoardType_Get(void)
{
	unsigned int trappingmode = 0x0;
	trappingmode = TRAPPING_MODE_RD();
	trappingmode = trappingmode & 0x80;
	return trappingmode;
}
EXPORT_SYMBOL(AC_BoardType_Get);

void GPIO_DriveCurrent_Set(int gpionum,  int i4Current)
{
	pr_debug("pin %d current is set to %d\n", gpionum, i4Current);
}
EXPORT_SYMBOL(GPIO_DriveCurrent_Set);

void GPIO_PullUp(int gpionum, GPIO_PUD ePullUD)
{
	unsigned int u1Offset = 0;
	unsigned int u4Val = 0;
	if(gpionum == 197)  {
		if(ePullUD == PULLUP) {
			u4Val = GPIO_823X_READ32(0x240e8);
			u4Val &= ~(1 << 12);
			u4Val |= 1 << 11; 
			GPIO_823X_WRITE32(0x240e8,u4Val);
		}
		else {
			u4Val = GPIO_823X_READ32(0x240e8);
			u4Val &= ~(1 << 11);
			GPIO_823X_WRITE32(0x240e8,u4Val);
		}
	}
	else if(gpionum == 219)  {
		if(ePullUD == PULLUP) {
			u4Val = GPIO_823X_READ32(0x240e8);
			u4Val &= ~(1 << 20);
			u4Val |= 1 << 19; 
			GPIO_823X_WRITE32(0x240e8,u4Val);
		}
		else {
			u4Val = GPIO_823X_READ32(0x240e8);
			u4Val &= ~(1 << 19);
			GPIO_823X_WRITE32(0x240e8,u4Val);
		}
	}
	else if(gpionum == 220)  {
		if(ePullUD == PULLUP) {
			u4Val = GPIO_823X_READ32(0x240e8);
			u4Val &= ~(1 << 28);
			u4Val |= 1 << 27; 
			GPIO_823X_WRITE32(0x240e8,u4Val);
		}
		else {
			u4Val = GPIO_823X_READ32(0x240e8);
			u4Val &= ~(1 << 27);
			GPIO_823X_WRITE32(0x240e8,u4Val);
		}
	}
	else {
		if ((gpionum > TOTAL_GPIO_NUM)
		|| ((unsigned int)PIN_PULL_UP_OR_DOWN_OFFSET[gpionum]
		== INVALID_PUD_GPIO)) {
			return;
		}
		u1Offset = (unsigned int)PIN_PULL_UP_OR_DOWN_OFFSET[gpionum];
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
}

void GPIO_PullDown(int gpionum, GPIO_PUD ePullUD)
{
	unsigned int u1Offset;
	unsigned int u4Val;
	
	if(gpionum == 197)  {
		if(ePullUD == PULLDOWN) {
			u4Val = GPIO_823X_READ32(0x240e8);
			u4Val &= ~(1 << 11);
			u4Val |= 1 << 12; 
			GPIO_823X_WRITE32(0x240e8,u4Val);
		}
		else {
			u4Val = GPIO_823X_READ32(0x240e8);
			u4Val &= ~(1 << 12);
			GPIO_823X_WRITE32(0x240e8,u4Val);
		}
	}
	else if(gpionum == 219)  {
		if(ePullUD == PULLDOWN) {
			u4Val = GPIO_823X_READ32(0x240e8);
			u4Val &= ~(1 << 19);
			u4Val |= 1 << 20; 
			GPIO_823X_WRITE32(0x240e8,u4Val);
		}
		else {
			u4Val = GPIO_823X_READ32(0x240e8);
			u4Val &= ~(1 << 20);
			GPIO_823X_WRITE32(0x240e8,u4Val);
		}
	}
	else if(gpionum == 220)  {
		if(ePullUD == PULLDOWN) {
			u4Val = GPIO_823X_READ32(0x240e8);
			u4Val &= ~(1 << 27);
			u4Val |= 1 << 28; 
			GPIO_823X_WRITE32(0x240e8,u4Val);
		}
		else {
			u4Val = GPIO_823X_READ32(0x240e8);
			u4Val &= ~(1 << 28);
			GPIO_823X_WRITE32(0x240e8,u4Val);
		}
	}
	else {
		if ((gpionum > TOTAL_GPIO_NUM) ||
			((unsigned int)PIN_PULL_UP_OR_DOWN_OFFSET[gpionum]
			== INVALID_PUD_GPIO)) {
			return;
		}
		u1Offset = (unsigned int)PIN_PULL_UP_OR_DOWN_OFFSET[gpionum];
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
	
}

void GPIO_Pull_UpDown(int gpionum, unsigned PullUpOrDown)
{
	unsigned char u1PullUD = PullUpOrDown;

	if ((gpionum >= TOTAL_GPIO_NUM)) {
		return;
	}

	switch (u1PullUD) {
	case PULLUP:
		GPIO_PullDown(gpionum, NORMAL);
		GPIO_PullUp(gpionum, PULLUP);
		break;

	case PULLDOWN:
		GPIO_PullUp(gpionum, NORMAL);
		GPIO_PullDown(gpionum, PULLDOWN);
		break;

	case NORMAL:
		GPIO_PullDown(gpionum, NORMAL);
		GPIO_PullUp(gpionum, NORMAL);
		break;

	default:
		break;
	}
	return;
}
EXPORT_SYMBOL(GPIO_Pull_UpDown);

