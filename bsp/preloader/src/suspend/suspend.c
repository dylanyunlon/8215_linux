/* Copyright Statement:
 *
 * This software/firmware and related documentation ("AutoChips Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to AutoChips Inc. and/or its licensors. Without
 * the prior written permission of AutoChips inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of AutoChips Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 * 
 * AutoChips Inc. (C) 2016. All rights reserved.
 * 
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("AUTOCHIPS SOFTWARE")
 * RECEIVED FROM AUTOCHIPS AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. AUTOCHIPS EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES AUTOCHIPS PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE AUTOCHIPS SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN AUTOCHIPS
 * SOFTWARE. AUTOCHIPS SHALL ALSO NOT BE RESPONSIBLE FOR ANY AUTOCHIPS SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND AUTOCHIPS'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE AUTOCHIPS SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT AUTOCHIPS'S OPTION, TO REVISE OR REPLACE THE
 * AUTOCHIPS SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO AUTOCHIPS FOR SUCH AUTOCHIPS SOFTWARE AT ISSUE.
 */

//=============================
// header files
//=============================
#include "targetConfig.h"

#include "auto_version.h" 
#include "preloader_common.h"
#include "chip_test.h"
#include "x_irq.h"
#include "msdc.h"
#include "boot.h"
#include "x_pdwnc.h"
#include "x_ckgen.h"

extern int loader(void);
extern void v_dram_bist(void);
void Menu_Config(void);
void SetStack2Mem(void);
extern void initRS232(void);
extern UINT32 _dramk_start;
extern UINT32 _loader_start;
extern UINT32 _loader_end;
extern unsigned int  ac8317_power_off[];
extern unsigned int  ac8317_power_off_end[];
extern unsigned int *calibration_address;
extern void power_off(unsigned int);

#ifdef __AndroidM__
struct quickboot_param qb_param = {0x1,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0};
#else
struct quickboot_param qb_param = {0x1,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0};
#endif

#define CPSR_MODE_IRQ  0x12 
#define	CPSR_MODE_SVC	0x13
#define CPSR_MODE_FIQ  0x11
#define CPSR_MODE_SYS  0x1F
#define CPSR_MODE_USR  0x10
#define CPSR_MODE_UND  0x1B
#define CPSR_MODE_ABT  0x17

#define	CPSR_BIT_I		0x80
#define	CPSR_BIT_F		0x40


//=============================
// define
//=============================
/* timer tick */

#define REG_RW_T64b_LO_0   0x0728
#define REG_RW_T64b_HI_0   0x072C
#define REG_RW_T64b_EN_0   0x0730
#define T64B_INIT()	\
	BIM_WRITE32(REG_RW_T64b_EN_0, 0); \
BIM_WRITE32(REG_RW_T64b_LO_0, 0); \
BIM_WRITE32(REG_RW_T64b_HI_0, 0); \
BIM_WRITE32(REG_RW_T64b_EN_0, 1)

void  Set_PDWN_GPIO_value(UINT32 u4Pin,UINT32 u4value)
{
	UINT32 u4Mode =0;
	UINT32 u4Tmp;
	UINT32 i = 0;

	do {
		//set pin to GPIO
		u4Tmp = PDWNC_READ32(REG_RW_PAD_PINMUX1);
		u4Tmp = u4Tmp | (1<<  u4Pin);
		PDWNC_WRITE32(REG_RW_PAD_PINMUX1, u4Tmp);
		i++;
	} while (0 == (PDWNC_READ32(REG_RW_PAD_PINMUX1) & (1U << u4Pin)));


	do {
	//GPIO direct
		u4Tmp = PDWNC_READ32(REG_RW_GPIOEN);

		if(u4Mode == 1)
			u4Tmp = u4Tmp & (~(1U << u4Pin));
		else
			u4Tmp = u4Tmp | ((1U << u4Pin));

		PDWNC_WRITE32(REG_RW_GPIOEN, u4Tmp);
		i++;
	} while (0 == (PDWNC_READ32(REG_RW_GPIOEN) & (1U << u4Pin)));

	do {
		//set output value
		u4Tmp = PDWNC_READ32(REG_RW_GPIOOUT);
		u4Tmp = u4Tmp & ( ~(1 << u4Pin));
		u4Tmp = u4Tmp | (u4value << u4Pin);
		if(u4Mode == 0)
			PDWNC_WRITE32(REG_RW_GPIOOUT,u4Tmp);
		i++;
	} while ((u4value << u4Pin) != (PDWNC_READ32(REG_RW_GPIOOUT) & (1U << u4Pin)));

	Printf("PIN(0x%x), EN(0x%x), OUT(0x%x), count(%d)\n",
	PDWNC_READ32(REG_RW_PAD_PINMUX1), PDWNC_READ32(REG_RW_GPIOEN), PDWNC_READ32(REG_RW_GPIOOUT), i);
}



/*
 *  0:AC ON
 *  1:DC ON
 *
 */
void vPDWNC_SetDCStatus(UINT32 u4Status)
{


	if(u4Status)
		PDWNC_WRITE32(REG_RW_SYSSTA,PDWNC_READ32(REG_RW_SYSSTA) | (1U << REG_RW_SYSSTA_ACDC_OFFSET));
	else
		PDWNC_WRITE32(REG_RW_SYSSTA,PDWNC_READ32(REG_RW_SYSSTA) | (0U << REG_RW_SYSSTA_ACDC_OFFSET));


}

void vPDWNC_SetWakeup_DelayCount(UINT32 u4Cnt)
{

	PDWNC_WRITE32(REG_RW_WKRSC,u4Cnt);

}
/*
 *the count is inital value,increase 1 every 333.3nm,
 *when reach to 0xFFFFFF,trigger system enter standby mode
 *

*/
void vPDWNC_SetGPIOPowerDown_Counter(UINT32 u4Cnt)
{
	PDWNC_WRITE32(REG_RW_PDIOCNT,u4Cnt);

}
void vPDWNC_SetGPIO_Wakeup(UINT32 u4Idx,BOOL bPolarity)
{

	UINT32 u4Tmp;
	u4Tmp = PDWNC_READ32(REG_RW_WAKEN);
	u4Tmp |=(1U << u4Idx);
	PDWNC_WRITE32(REG_RW_WAKEN,u4Tmp);

	u4Tmp = PDWNC_READ32(REG_RW_PDIO);
	if(bPolarity)
		u4Tmp |=(1U << u4Idx);
	else
		u4Tmp |=(0U << u4Idx);

	PDWNC_WRITE32(REG_RW_PDIO,u4Tmp);
	return;

}
static UINT32 GPIOWakeConfig(UINT32 u4Pin,UINT32 u4Prolarity)
{

	UINT32 u4Tmp = 0;

	u4Tmp = HAL_READ32(0xF0038004);

	/*mask sram boot bit*/
	HAL_WRITE32(0xF0038004,u4Tmp | (1U << 16));
	vPDWNC_SetGPIO_Wakeup(u4Pin,u4Prolarity);
	vPDWNC_SetDCStatus(1);
	vPDWNC_SetWakeup_DelayCount(0x170000);	
	return 0;

}

static void IRPowerDown()
{
	PDWNC_WRITE32(0x40, 0x6);
}
static void SRAMStandbyConfig()
{

	PDWNC_WRITE32(REG_RW_SRAM_CTL,0x8000);

}
static PLLPowerDown()
{
	UINT32 u4Tmp;

	CKGEN_WRITE32(0x4,0);
	CKGEN_WRITE32(0x8,0);
	CKGEN_WRITE32(0xc,0);
	CKGEN_WRITE32(0x10,0);
	CKGEN_WRITE32(0x14,0);
	CKGEN_WRITE32(0x18,0);
	CKGEN_WRITE32(0X1C,0);
	CKGEN_WRITE32(0x20,0);
	CKGEN_WRITE32(0x24,0);
	CKGEN_WRITE32(0x28,0);
	CKGEN_WRITE32(0x2C,0);
	CKGEN_WRITE32(0x30,0);
	CKGEN_WRITE32(0x34,0);
	CKGEN_WRITE32(0xD4,0);
	CKGEN_WRITE32(0xD8,0);
	CKGEN_WRITE32(0xDC,0);
	CKGEN_WRITE32(0x188,0);
	CKGEN_WRITE32(0x18C,0);
	CKGEN_WRITE32(0x190,0);

#if 0
	HAL_WRITE32(0xF0008448,0x0);
	HAL_WRITE32(0xF0055060,0x0);
	HAL_WRITE32(0xF0000594,0xBE4C2020);
	/*all pll power off @standby*/
#if 0
	u4Tmp = PDWNC_READ32(0x30);
	u4Tmp = u4Tmp | (1U << 7 );
	PDWNC_WRITE32(0x30,u4Tmp);
#endif

	/*ARMPLL Power off*/
	u4Tmp = PDWNC_READ32(0X184);
	u4Tmp =  (1U << 7 );
	PDWNC_WRITE32(0x184,u4Tmp);
	PDWNC_WRITE32(0x180,0xB5DFF);
	//decrease  pdwnc clock 
	//	 PDWNC_WRITE32(REG_RW_PDCKSEL,RW_PDCKSEL_250K);
#else

	PDWNC_WRITE32(0x180,0xB5DFB);
#endif
}
/*this code must execute@demux sram*/
void  EnterPowerDown()
{
	UINT32 u4tmp = 0;
	unsigned int *start= ac8317_power_off;
	unsigned int *dest = (unsigned int *)0xF4008000;
	vPDWNC_SetGPIOPowerDown_Counter(0x30000);

	//enable demux clock
	u4tmp = HAL_READ32(0xF00000A0);
	u4tmp &=0xFFFFFC7F;
	HAL_WRITE32(0xF00000A0,u4tmp);
	//demux reset enable
	u4tmp = HAL_READ32(0xF00000BC);
	u4tmp |=0x80;
	HAL_WRITE32(0xF00000BC,u4tmp);

	//remap
	u4tmp = HAL_READ32(0xF00120C0);
	u4tmp |=(0x1<<26);
	HAL_WRITE32(0xF00120C0,u4tmp);

	while(start < ac8317_power_off_end){
		*dest = *start;
		dest++;
		start++;
	}
	Set_PDWN_GPIO_value(qb_param.wakeup_sts_gpio,qb_param.wakeup_sts_polarity);
	asm("isb \r\n  \
			dsb \r\n  \
			ldr r0,=0xF4008000 \r\n \
			bx  r0 \r\n" );


}


int core_suspend(struct quickboot_param *param)
{
	/* timer init */

	UINT32 testData;
	UINT32 n;
	char outstr[30];
	initRS232();
	memcpy(&qb_param,param,sizeof(struct quickboot_param));
	Printf("-----start core suspend [v%d]-------\n", AUTO_VERSION);
	///
	set_opwrsb_mode(0);
	Printf("calibration:0x%x\n",qb_param.ddr_cal_addr);
	T64B_INIT();
	Printf("save dramc register address: 0x%x\n",(UINT32)&_loader_start);
	Printf("wakeup state gpio:%d,polarity:%d\n",qb_param.wakeup_sts_gpio,qb_param.wakeup_sts_polarity);

	DDR_EnterSuspend(&_loader_start);
        TIM_Start();
	GPIOWakeConfig(1,1);
	//	SRAMStandbyConfig();
	
	set_boot_type(QUICK_BOOT);
	Printf("ddr suspend finish\n");
	TIM_DelayUS(1000);

	PLLPowerDown();
	IRPowerDown();
	EnterPowerDown();
	while(1);
	return 0;  

}

unsigned set_opwrsb_mode(unsigned type)  // 0: power control  1: gpio
{
	UINT32 u4Tmp;
	
	if (type == 1) {
		u4Tmp = PDWNC_READ32(0X0F4);
		u4Tmp = u4Tmp | (1 << 3);
		Printf("set opwrsb as gpio \n");
		return PDWNC_WRITE32(0X0F4, u4Tmp); 
	} else if (type == 0) {
		u4Tmp = PDWNC_READ32(0X0F4);
		u4Tmp = u4Tmp & ~(1 << 3);
		Printf("set opwrsb as power control \n");
		return PDWNC_WRITE32(0X0F4, u4Tmp);
	} else {
		u4Tmp = PDWNC_READ32(0X0F4);
		u4Tmp = u4Tmp & ~(1 << 3);
		Printf("default set opwrsb as power control \n");
		return PDWNC_WRITE32(0X0F4, u4Tmp);
	}
}

unsigned set_opwrsb_function(unsigned fun, unsigned value)
{
	UINT32 u4Tmp;

	if (fun == 1) { // output
		u4Tmp = PDWNC_READ32(0X0D4);
		u4Tmp = u4Tmp | (1 << 3);

		PDWNC_WRITE32(0X0D4, u4Tmp);

		if (value == 1) { // output H
			u4Tmp = PDWNC_READ32(0X0D8);
			u4Tmp = u4Tmp | (1 << 3);
			Printf("opwrsb output H \n");
			PDWNC_WRITE32(0X0D8, u4Tmp);
		} else if (value == 0) {  //  output L
			u4Tmp = PDWNC_READ32(0X0D8);
			u4Tmp = u4Tmp & ~(1 << 3);
			Printf("opwrsb output L \n");
			PDWNC_WRITE32(0X0D8, u4Tmp);
		} else {

		}

		return 0;
	} else if (fun == 0) { // input
		u4Tmp = PDWNC_READ32(0X0D4);
		u4Tmp = u4Tmp & ~(1 << 3);
		Printf("opwrsb input \n");
		return PDWNC_WRITE32(0X0D4, u4Tmp);
	} else {
		return 0;
	}

}


unsigned get_boot_type()
{

	return PDWNC_READ32(0X160); 

}

unsigned set_boot_type(unsigned type)
{

	PDWNC_WRITE32(0x160,type);
}
