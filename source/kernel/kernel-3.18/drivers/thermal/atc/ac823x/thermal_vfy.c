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

/*****************************************************************************
 *
 * Filename:
 * ---------
 *   ts_thermal.c
 *
 * Project:
 * --------
 *   MT6575
 *
 * Description:
 * ------------
 *   None
 *
 * Author:
 * -------
 *   Chun-Wei Chen
 *
 *------------------------------------------------------------------------------
 * Upper this line, this part is controlled by PVCS VM. DO NOT MODIFY!!
 *============================================================================
 ****************************************************************************/
#if 0
#include "common.h"
#include "thermal.h"

#include "x_printf.h"
#include "x_debug.h"
#include "x_drv_cli.h"
#include "x_os.h"
#include "x_util.h"
#include "u_cli.h"
#include "../ckgen/inc/ckgen_drv.h"
#else
#include <linux/irq.h>
#include <asm/irq.h>

#include "thermal.h"
#include "x_drv_cli.h"
#include "x_serial.h"
#include "x_printf.h"
#include "x_os.h"
#include "x_assert.h"
#include "x_util.h"
#include "x_stl_lib.h"
#include "x_timer.h"
#include "x_debug.h"
#include "drv_config.h"
#include "chip_ver.h"

#include "slt_mod.h"

#endif

/*******************************************************************************
 * LOCAL MACRO DEFINATION
 ******************************************************************************/
#define REG_CHK(reg, check_value, result) do {  \
    if (reg != check_value)                     \
        {result = -1; UTIL_Printf("[FAIL]%s(),line=%d :0x%x = %x\r\n", __func__,__LINE__, reg, check_value);}   \
} while(0)

#ifndef atoi
#define atoi(str)      ((int)x_strtoll(str, NULL, 10))
#endif

/*******************************************************************************
 * LOCAL CONST DEFINATION
 ******************************************************************************/
#define CHECK_REG_DEFAULT_VALUE             0x0
#define CHECK_REG_READ_ONLY                 0x1
#define CHECK_REG_READ_WRITE                0x2

#define CHECK_HOT_INTERRUPT_THRESHOLD       0x0
#define CHECK_COLD_INTERRUPT_THRESHOLD      0x1
#define CHECK_H2N_INTERRUPT_THRESHOLD       0x2
#define CHECK_HOFFSET_INTERRUPT_THRESHOLD   0x3
#define CHECK_LOFFSET_INTERRUPT_THRESHOLD   0x4

#define CHECK_INTERRUPT_TRIGGER             0x0
#define CHECK_INTERRUPT_OCCURRANCE          0x1
#define CHECK_SENSING_FILTER_OPTION         0x2
#define CHECK_IMMEDIATE_MEASUREMENT         0x3
#define CHECK_AHB_TIMEOUT                   0x4
#define CHECK_FIRST_HOT_INTERRUPT           0x5
#define CHECK_FILTER_SAMPLE_INTERVAL        0x6
#define CHECK_DIFFERENT_THRESHOLD           0x7
#define CHECK_INTERRUPT_MASK                0x8
#define CHECK_TRIGGER_WDT_RESET             0x9

#define CHECK_ADC_HW_AUTO_MODE       		0x0
#define CHECK_ADC_SW_TRIGGER_MODE      		0x1
#define CHECK_ADC_IO_TRIGGER_MODE       	0x2
#define CHECK_ADC_CHANGE_SAMPLE_RATE   		0x3
#define CHECK_ADC_CALIBRATION       		0x4
#define CHECK_ADC_SNDR   					0x5


/*******************************************************************************
 * Funtion Definations
 ******************************************************************************/
static INT32 TS_THERMAL_Register_Test(INT32 i4Argc, const CHAR** aszArgv);
static INT32 TS_THERMAL_Interrupt_Test(INT32 i4Argc, const CHAR** aszArgv);
static INT32 TS_THERMAL_Module_Test(INT32 i4Argc, const CHAR** aszArgv);
static INT32 TS_THERMAL_Switch_Bank(INT32 i4Argc, const CHAR** aszArgv);
static INT32 TS_THERMAL_EXIT(INT32 i4Argc, const CHAR** aszArgv);
static INT32 TS_THERMAL_READ_REAL_TEMP(INT32 i4Argc, const CHAR** aszArgv);


extern int thermal_clk_select(UINT32 therm_ck, UINT32 therm_slow_ck);
extern int thermal_clk_enable(UINT32 therm_ck_flag, UINT32 therm_slow_ck_flag);
extern int thermal_clk_reset(UINT32 therm_reset);

#if 0
static void kal_sleep_task(kal_uint32 time_in_ticks)
{
    kal_uint32 i;
    if (time_in_ticks)
        for (i = 0; i < 1000 * time_in_ticks; i++) {}
}
#else
#define kal_sleep_task(us)  \
    do { \
        volatile int count = us * 1000; \
        while (count--); \
    }while(0)
#endif

static int THERMAL_Hot_Interrupt_Check(unsigned int sensor)
{
    int result = 1;
    kal_uint32 temp;

    thermal_real_interrupt_test_HCHL(sensor);

    while(1) {
        temp = thermal_get_interrupt_status();
		UTIL_Printf("ts_thermal: THERMAL_Hot_Interrupt_Check(), temp [0x%x], sensor [%d] \r\n", temp, sensor);

		switch (sensor)
        {
            case 0:  //THERMAL_ENABLE_SEN0:
                temp = temp & THERMAL_MON_HINTSTS0;
                break;
            case 1:  //THERMAL_ENABLE_SEN1:
                temp = temp & THERMAL_MON_HINTSTS1;
                break;
			case 2:  //THERMAL_ENABLE_SEN2:
                temp = temp & THERMAL_MON_HINTSTS2;
                break;
            case 3:  //THERMAL_ENABLE_SEN3:
                temp = temp & THERMAL_MON_HINTSTS3;
                break;
            default:
                UTIL_Printf("ts_thermal: unknown sensor value. sensor = %x\n", sensor);
        }

        if (temp)
            break;
        kal_sleep_task(1000);
    }

    UTIL_Printf("ts_thermal: THERMAL_Hot_Interrupt_Check() => PASS\r\n");

    return result;
}

static int THERMAL_Cold_Interrupt_Check(unsigned int sensor)
{
    int result = 1;
    kal_uint32 temp;

    thermal_real_interrupt_test_HCHL(sensor);

    while(1) {
        temp = thermal_get_interrupt_status();
		UTIL_Printf("ts_thermal: THERMAL_Cold_Interrupt_Check(), temp [0x%x], sensor [%d] \r\n", temp, sensor);
		
        switch (sensor)
        {
            case 0:
                temp = temp & THERMAL_MON_CINTSTS0;
                break;
            case 1:
                temp = temp & THERMAL_MON_CINTSTS1;
                break;
			case 2:
                temp = temp & THERMAL_MON_CINTSTS2;
                break;
            case 3:
                temp = temp & THERMAL_MON_CINTSTS3;
                break;
            default:
                UTIL_Printf("ts_thermal: unknown sensor value. sensor = %x\n", sensor);
        }

        if (temp)
            break;
        kal_sleep_task(1000);
    }

    UTIL_Printf("ts_thermal: THERMAL_Cold_Interrupt_Check() => PASS\r\n");

    return result;
}

static int THERMAL_H2N_Interrupt_Check(unsigned int sensor)
{
    int result = 1;
    kal_uint32 temp;

    thermal_real_interrupt_test_H2N(sensor);

    while(1) {
        temp = thermal_get_interrupt_status();
		UTIL_Printf("ts_thermal: THERMAL_H2N_Interrupt_Check(), temp [0x%x], sensor [%d] \r\n", temp, sensor);
		
        switch (sensor)
        {
            case 0:
                temp = temp & THERMAL_MON_NHINTSTS0;
                break;
            case 1:
                temp = temp & THERMAL_MON_NHINTSTS1;
                break;
			case 2:
                temp = temp & THERMAL_MON_NHINTSTS2;
                break;
            case 3:
                temp = temp & THERMAL_MON_NHINTSTS3;
                break;
            default:
                UTIL_Printf("ts_thermal: unknown sensor value. sensor = %x\n", sensor);
        }

        if (temp)
            break;
        kal_sleep_task(1000);
    }

    UTIL_Printf("ts_thermal: THERMAL_H2N_Interrupt_Check() => PASS\r\n");

    return result;
}

static int THERMAL_HOffset_Interrupt_Check(unsigned int sensor)
{
    int result = 1;
    kal_uint32 temp;

    thermal_real_interrupt_test_HCHL(sensor);

    while(1) {
        temp = thermal_get_interrupt_status();
		UTIL_Printf("ts_thermal: THERMAL_HOffset_Interrupt_Check(), temp [0x%x], sensor [%d] \r\n", temp, sensor);
		
        switch (sensor)
        {
            case 0:
                temp = temp & THERMAL_MON_HOINTSTS0;
                break;
            case 1:
                temp = temp & THERMAL_MON_HOINTSTS1;
                break;
			case 2:
                temp = temp & THERMAL_MON_HOINTSTS2;
                break;
            case 3:
                temp = temp & THERMAL_MON_HOINTSTS3;
                break;
            default:
                UTIL_Printf("ts_thermal: unknown sensor value. sensor = %x\n", sensor);
        }

        if (temp)
            break;
        kal_sleep_task(1000);
    }

    UTIL_Printf("ts_thermal: THERMAL_HOffset_Interrupt_Check() => PASS\r\n");

    return result;
}

static int THERMAL_LOffset_Interrupt_Check(unsigned int sensor)
{
    int result = 1;
    kal_uint32 temp;

    thermal_real_interrupt_test_HCHL(sensor);

    while(1) {
        temp = thermal_get_interrupt_status();
		UTIL_Printf("ts_thermal: THERMAL_LOffset_Interrupt_Check(), temp [0x%x], sensor [%d] \r\n", temp, sensor);
		
        switch (sensor)
        {
            case 0:
                temp = temp & THERMAL_MON_LOINTSTS0;
                break;
            case 1:
                temp = temp & THERMAL_MON_LOINTSTS1;
                break;
			case 2:
                temp = temp & THERMAL_MON_LOINTSTS2;
                break;
            case 3:
                temp = temp & THERMAL_MON_LOINTSTS3;
                break;
            default:
                UTIL_Printf("ts_thermal: unknown sensor value. sensor = %x\n", sensor);
        }

        if (temp)
            break;
        kal_sleep_task(1000);
    }

    UTIL_Printf("ts_thermal: THERMAL_LOffset_Interrupt_Check() => PASS\r\n");

    return result;
}

static INT32 TS_THERMAL_Interrupt_Test(INT32 i4Argc, const CHAR** aszArgv)
{
    int result = -1;
    int cond = 0, sensor = 0;
	
	if (i4Argc < 3 ) 
	{	 
		UTIL_Printf("Usage: int cond sensor\n");    
		UTIL_Printf("	0: Hot Interrupt Threshold\n");
		UTIL_Printf("	1: Cold Interrupt Threshold\n");
		UTIL_Printf("	2: Hot to Normal Interrupt Threshold\n");
		UTIL_Printf("	3: High Offset Threshold\n");
		UTIL_Printf("	4: Low Offset Threshold\n");
			
		return result;	
	}
		
	cond = atoi(aszArgv[1]);
	sensor = atoi(aszArgv[2]);
	UTIL_Printf("%s,cond: %d, sensor: %d \r\n",__func__, cond,sensor);
      
    switch (cond)
    {
        case CHECK_HOT_INTERRUPT_THRESHOLD:
            UTIL_Printf("ts_thermal: enter THERMAL_Hot_Interrupt_Check()\r\n");
            result = THERMAL_Hot_Interrupt_Check(sensor);
            break;
        case CHECK_COLD_INTERRUPT_THRESHOLD:
            UTIL_Printf("ts_thermal: enter THERMAL_Cold_Interrupt_Check()\r\n");
            result = THERMAL_Cold_Interrupt_Check(sensor);
            break;
        case CHECK_H2N_INTERRUPT_THRESHOLD:
            UTIL_Printf("ts_thermal: enter THERMAL_H2N_Interrupt_Check()\r\n");
            result = THERMAL_H2N_Interrupt_Check(sensor);
            break;
        case CHECK_HOFFSET_INTERRUPT_THRESHOLD:
            UTIL_Printf("ts_thermal: enter THERMAL_HOffset_Interrupt_Check()\r\n");
            result = THERMAL_HOffset_Interrupt_Check(sensor);
            break;
        case CHECK_LOFFSET_INTERRUPT_THRESHOLD:
            UTIL_Printf("ts_thermal: enter THERMAL_LOffset_Interrupt_Check()\r\n");
            result = THERMAL_LOffset_Interrupt_Check(sensor);
            break;
        default:
            result = -1;
            break;
    }
   

    return result;
}


static int THERMAL_ADC_Hw_Auto_Mode_Check(unsigned int channel)
{
	int result = 1;

	thermal_adc_hw_auto_mode(channel);

    return result;
}

static int THERMAL_ADC_Sw_Trigger_Mode_Check(unsigned int channel)
{
	int result = 1;

	thermal_adc_sw_trigger_mode(channel);

    return result;
}

static int THERMAL_ADC_Io_Trigger_Mode_Check(unsigned int channel)
{
	int result = 1;

	thermal_adc_io_trigger_mode(channel);

    return result;
}

static int THERMAL_ADC_Change_Sample_rate_Check(unsigned int channel)
{
	int result = 1;

	thermal_adc_change_sample_rate(channel);

    return result;
}

static int THERMAL_ADC_Calibration_Check(unsigned int channel)
{
	int result = 1;

	UTIL_Printf("open thermal clock start... \n");
	thermal_clk_select(1, 1);
	thermal_clk_enable(1, 1);
	thermal_clk_reset(0);
	thermal_clk_reset(1);
	UTIL_Printf("open thermal clock end \n");

	thermal_adc_calibration(channel);

    return result;
}

static int THERMAL_ADC_SNDR_Check(unsigned int channel)
{
	int result = 1;

	thermal_adc_sndr(channel);

    return result;
}


static INT32 TS_THERMAL_Adc_Test(INT32 i4Argc, const CHAR** aszArgv)
{
    int result = -1;
    unsigned int cond = 0, channel = 0;
	
	if (i4Argc < 3 )
	{	 
		UTIL_Printf("Usage: adc channel \n");    
		UTIL_Printf("	0: HW auto mode (65535 -> 0xFFFF -> open 16 channels)\n");
		UTIL_Printf("	1: SW trigger mode\n");
		UTIL_Printf("	2: IO trigger mode\n");
		UTIL_Printf("	3: change sample rate\n");
		UTIL_Printf("	4: calibration (opwrsb=1 && wakeup_sts=0)\n");
		UTIL_Printf("	5: SNDR\n");
		
		return result;	
	}
		
	cond = atoi(aszArgv[1]); 
	channel = atoi(aszArgv[2]);
	UTIL_Printf("%s,cond: %d, channel: 0x%x \r\n",__func__, cond, channel);
      
    switch (cond)
    {
        case CHECK_ADC_HW_AUTO_MODE:
            UTIL_Printf("ts_thermal: enter THERMAL_ADC_Hw_Auto_Mode_Check() \r\n");
            result = THERMAL_ADC_Hw_Auto_Mode_Check(channel);
            break;
        case CHECK_ADC_SW_TRIGGER_MODE:
            UTIL_Printf("ts_thermal: enter THERMAL_ADC_Sw_Trigger_Mode_Check \r\n");
            result = THERMAL_ADC_Sw_Trigger_Mode_Check(channel);
            break;
        case CHECK_ADC_IO_TRIGGER_MODE:
            UTIL_Printf("ts_thermal: enter THERMAL_ADC_Io_Trigger_Mode_Check \r\n");
            result = THERMAL_ADC_Io_Trigger_Mode_Check(channel);
            break;
        case CHECK_ADC_CHANGE_SAMPLE_RATE:
            UTIL_Printf("ts_thermal: enter THERMAL_ADC_Change_Sample_rate_Check \r\n");
            result = THERMAL_ADC_Change_Sample_rate_Check(channel);
            break;
		case CHECK_ADC_CALIBRATION:
			// to make system enter calibration mode when power on, set opwrsb=1 && wakeup_sts=0
            UTIL_Printf("ts_thermal: enter THERMAL_ADC_Calibration_Check \r\n");
            result = THERMAL_ADC_Calibration_Check(channel);
			break;
		case CHECK_ADC_SNDR:
			UTIL_Printf("ts_thermal: enter THERMAL_ADC_SNDR_Check \r\n");
			result = THERMAL_ADC_SNDR_Check(channel);
			break;
        default:
            result = -1;
            break;
    }
   

    return result;
}

static int THERMAL_Register_Default_Value_Check(void)
{
    int result = 1;

    REG_CHK((DRV_Reg32(TEMPMONCTL0) & 0x00000007),          0x00000000, result); if (result == -1) goto exit;
    REG_CHK((DRV_Reg32(TEMPMONCTL1) & 0x0000EFFF),          0x00000000, result); if (result == -1) goto exit;
    REG_CHK((DRV_Reg32(TEMPMONCTL2) & 0x03FF03FF),          0x00000000, result); if (result == -1) goto exit;
    REG_CHK((DRV_Reg32(TEMPMONINT) & 0x0007FFFF),           0x00000000, result); if (result == -1) goto exit;
    REG_CHK((DRV_Reg32(TEMPMONINTSTS) & 0x0000FFFF),        0x00000000, result); if (result == -1) goto exit;
    REG_CHK((DRV_Reg32(TEMPMONIDET0) & 0x000003FF),         0x00000000, result); if (result == -1) goto exit;
    REG_CHK((DRV_Reg32(TEMPMONIDET1) & 0x000003FF),         0x00000000, result); if (result == -1) goto exit;
    REG_CHK((DRV_Reg32(TEMPMONIDET2) & 0x000003FF),         0x00000000, result); if (result == -1) goto exit;
    REG_CHK((DRV_Reg32(TEMPH2NTHRE) & 0x000000FFF),         0x00000000, result); if (result == -1) goto exit;
    REG_CHK((DRV_Reg32(TEMPHTHRE) & 0x00000FFF),            0x00000000, result); if (result == -1) goto exit;
    REG_CHK((DRV_Reg32(TEMPCTHRE) & 0x00000FFF),            0x00000000, result); if (result == -1) goto exit;
    REG_CHK((DRV_Reg32(TEMPOFFSETH) & 0x00000FFF),          0x00000000, result); if (result == -1) goto exit;
    REG_CHK((DRV_Reg32(TEMPOFFSETL) & 0x00000FFF),          0x00000000, result); if (result == -1) goto exit;
    REG_CHK((DRV_Reg32(TEMPMSRCTL0) & 0x000001FF),          0x00000000, result); if (result == -1) goto exit;
    REG_CHK((DRV_Reg32(TEMPMSRCTL1) & 0x0000007F),          0x00000000, result); if (result == -1) goto exit;
    REG_CHK((DRV_Reg32(TEMPAHBPOLL) & 0x0000FFFF),          0x00000000, result); if (result == -1) goto exit;
    REG_CHK((DRV_Reg32(TEMPAHBTO) & 0xFFFFFFFF),            0xFFFFFFFF, result); if (result == -1) goto exit;
	REG_CHK((DRV_Reg32(TEMPADCPNP0) & 0xFFFFFFFF),          0x00000000, result); if (result == -1) goto exit;
    REG_CHK((DRV_Reg32(TEMPADCPNP1) & 0xFFFFFFFF),          0x00000000, result); if (result == -1) goto exit;
    REG_CHK((DRV_Reg32(TEMPADCPNP2) & 0xFFFFFFFF),          0x00000000, result); if (result == -1) goto exit;
    REG_CHK((DRV_Reg32(TEMPADCMUX) & 0xFFFFFFFF),           0x00000000, result); if (result == -1) goto exit;
    REG_CHK((DRV_Reg32(TEMPADCEXT) & 0xFFFFFFFF),           0x00000000, result); if (result == -1) goto exit;
    REG_CHK((DRV_Reg32(TEMPADCEXT1) & 0xFFFFFFFF),          0x00000000, result); if (result == -1) goto exit;
    REG_CHK((DRV_Reg32(TEMPADCEN) & 0xFFFFFFFF),            0x00000000, result); if (result == -1) goto exit;
    REG_CHK((DRV_Reg32(TEMPPNPMUXADDR) & 0xFFFFFFFF),       0x00000000, result); if (result == -1) goto exit;
    REG_CHK((DRV_Reg32(TEMPADCMUXADDR) & 0xFFFFFFFF),       0x00000000, result); if (result == -1) goto exit;
    REG_CHK((DRV_Reg32(TEMPADCEXTADDR) & 0xFFFFFFFF),       0x00000000, result); if (result == -1) goto exit;
    REG_CHK((DRV_Reg32(TEMPADCEXT1ADDR) & 0xFFFFFFFF),      0x00000000, result); if (result == -1) goto exit;
    REG_CHK((DRV_Reg32(TEMPADCENADDR) & 0xFFFFFFFF),        0x00000000, result); if (result == -1) goto exit;
    REG_CHK((DRV_Reg32(TEMPADCVALIDADDR) & 0xFFFFFFFF),     0x00000000, result); if (result == -1) goto exit;
    REG_CHK((DRV_Reg32(TEMPADCVOLTADDR) & 0xFFFFFFFF),      0x00000000, result); if (result == -1) goto exit;
    REG_CHK((DRV_Reg32(TEMPRDCTRL) & 0xFFFFFFFF),           0x00000000, result); if (result == -1) goto exit;
    REG_CHK((DRV_Reg32(TEMPADCVALIDMASK) & 0xFFFFFFFF),     0x00000000, result); if (result == -1) goto exit;
    REG_CHK((DRV_Reg32(TEMPADCVOLTAGESHIFT) & 0xFFFFFFFF),  0x00000000, result); if (result == -1) goto exit;
    REG_CHK((DRV_Reg32(TEMPADCWRITECTRL) & 0xFFFFFFFF),     0x00000000, result); if (result == -1) goto exit;
    REG_CHK((DRV_Reg32(TEMPMSR0) & 0x00000FFF),             0x00000000, result); if (result == -1) goto exit;
    REG_CHK((DRV_Reg32(TEMPMSR1) & 0x00000FFF),             0x00000000, result); if (result == -1) goto exit;
    REG_CHK((DRV_Reg32(TEMPMSR2) & 0x00000FFF),             0x00000000, result); if (result == -1) goto exit;
    REG_CHK((DRV_Reg32(TEMPIMMD0) & 0x00000FFF),            0x00000000, result); if (result == -1) goto exit;
    REG_CHK((DRV_Reg32(TEMPIMMD1) & 0x00000FFF),            0x00000000, result); if (result == -1) goto exit;
    REG_CHK((DRV_Reg32(TEMPIMMD2) & 0x00000FFF),            0x00000000, result); if (result == -1) goto exit;

    UTIL_Printf("ts_thermal: THERMAL_Register_Default_Value_Check() => PASS\r\n");
	
exit:
    return result;
}

static int THERMAL_Register_RW_Check(void)
{
    int result = 1;
    volatile kal_uint32 temp;

    temp = DRV_Reg32(TEMPMONCTL0);
    DRV_WriteReg32(TEMPMONCTL0, 0x00000007);
    REG_CHK((DRV_Reg32(TEMPMONCTL0) & 0x00000007),    0x00000007, result);      if (result == -1) goto exit;
    DRV_WriteReg32(TEMPMONCTL0, temp);

    temp = DRV_Reg32(TEMPMONCTL1);
    DRV_WriteReg32(TEMPMONCTL1, 0x0000E3FF);
    REG_CHK((DRV_Reg32(TEMPMONCTL1) & 0x0000E3FF),    0x0000E3FF, result);      if (result == -1) goto exit;
    DRV_WriteReg32(TEMPMONCTL1, temp);

    temp = DRV_Reg32(TEMPMONCTL2);
    DRV_WriteReg32(TEMPMONCTL2, 0x03FF03FF);
    REG_CHK((DRV_Reg32(TEMPMONCTL2) & 0x03FF03FF),    0x03FF03FF, result);      if (result == -1) goto exit;
    DRV_WriteReg32(TEMPMONCTL2, temp);

    temp = DRV_Reg32(TEMPMONINT);
    DRV_WriteReg32(TEMPMONINT, 0x003FFFFF);
    REG_CHK((DRV_Reg32(TEMPMONINT) & 0x003FFFFF),     0x003FFFFF, result);      if (result == -1) goto exit;
    DRV_WriteReg32(TEMPMONINT, temp);

    temp = DRV_Reg32(TEMPMONIDET0);
    DRV_WriteReg32(TEMPMONIDET0, 0x000003FF);
    REG_CHK((DRV_Reg32(TEMPMONIDET0) & 0x000003FF),   0x000003FF, result);      if (result == -1) goto exit;
    DRV_WriteReg32(TEMPMONIDET0, temp);

    temp = DRV_Reg32(TEMPMONIDET1);
    DRV_WriteReg32(TEMPMONIDET1, 0x000003FF);
    REG_CHK((DRV_Reg32(TEMPMONIDET1) & 0x000003FF),   0x000003FF, result);      if (result == -1) goto exit;
    DRV_WriteReg32(TEMPMONIDET1, temp);

    temp = DRV_Reg32(TEMPMONIDET2);
    DRV_WriteReg32(TEMPMONIDET2, 0x000003FF);
    REG_CHK((DRV_Reg32(TEMPMONIDET2) & 0x000003FF),   0x000003FF, result);      if (result == -1) goto exit;
    DRV_WriteReg32(TEMPMONIDET2, temp);

    temp = DRV_Reg32(TEMPH2NTHRE);
    DRV_WriteReg32(TEMPH2NTHRE, 0x00000FFF);
    REG_CHK((DRV_Reg32(TEMPH2NTHRE) & 0x00000FFF),    0x00000FFF, result);      if (result == -1) goto exit;
    DRV_WriteReg32(TEMPH2NTHRE, temp);

    temp = DRV_Reg32(TEMPHTHRE);
    DRV_WriteReg32(TEMPHTHRE, 0x00000FFF);
    REG_CHK((DRV_Reg32(TEMPHTHRE) & 0x00000FFF),      0x00000FFF, result);      if (result == -1) goto exit;
    DRV_WriteReg32(TEMPHTHRE, temp);

    temp = DRV_Reg32(TEMPCTHRE);
    DRV_WriteReg32(TEMPCTHRE, 0x00000FFF);
    REG_CHK((DRV_Reg32(TEMPCTHRE) & 0x00000FFF),      0x00000FFF, result);      if (result == -1) goto exit;
    DRV_WriteReg32(TEMPCTHRE, temp);

    temp = DRV_Reg32(TEMPOFFSETH);
    DRV_WriteReg32(TEMPOFFSETH, 0x00000FFF);
    REG_CHK((DRV_Reg32(TEMPOFFSETH) & 0x00000FFF),    0x00000FFF, result);      if (result == -1) goto exit;
    DRV_WriteReg32(TEMPOFFSETH, temp);

    temp = DRV_Reg32(TEMPOFFSETL);
    DRV_WriteReg32(TEMPOFFSETL, 0x00000FFF);
    REG_CHK((DRV_Reg32(TEMPOFFSETL) & 0x00000FFF),    0x00000FFF, result);      if (result == -1) goto exit;
    DRV_WriteReg32(TEMPOFFSETL, temp);

    temp = DRV_Reg32(TEMPMSRCTL0);
    DRV_WriteReg32(TEMPMSRCTL0, 0x000001FF);
    REG_CHK((DRV_Reg32(TEMPMSRCTL0) & 0x000001FF),    0x000001FF, result);      if (result == -1) goto exit;
    DRV_WriteReg32(TEMPMSRCTL0, temp);

    temp = DRV_Reg32(TEMPMSRCTL1);
    DRV_WriteReg32(TEMPMSRCTL1, 0x0000007E);
    REG_CHK((DRV_Reg32(TEMPMSRCTL1) & 0x0000007E),    0x0000007E, result);      if (result == -1) goto exit;
    DRV_WriteReg32(TEMPMSRCTL1, temp);

    temp = DRV_Reg32(TEMPAHBPOLL);
    DRV_WriteReg32(TEMPAHBPOLL, 0xFFFFFFFF);
    REG_CHK((DRV_Reg32(TEMPAHBPOLL) & 0xFFFFFFFF),    0xFFFFFFFF, result);      if (result == -1) goto exit;
    DRV_WriteReg32(TEMPAHBPOLL, temp);

    temp = DRV_Reg32(TEMPAHBTO);
    DRV_WriteReg32(TEMPAHBTO, 0x12345678);
    REG_CHK((DRV_Reg32(TEMPAHBTO) & 0xFFFFFFFF),      0x12345678, result);      if (result == -1) goto exit;
    DRV_WriteReg32(TEMPAHBTO, temp);

    temp = DRV_Reg32(TEMPADCPNP0);
    DRV_WriteReg32(TEMPADCPNP0, 0xFFFFFFFF);
    REG_CHK((DRV_Reg32(TEMPADCPNP0) & 0xFFFFFFFF),      0xFFFFFFFF, result);    if (result == -1) goto exit;
    DRV_WriteReg32(TEMPADCPNP0, temp);

    temp = DRV_Reg32(TEMPADCPNP1);
    DRV_WriteReg32(TEMPADCPNP1, 0xFFFFFFFF);
    REG_CHK((DRV_Reg32(TEMPADCPNP1) & 0xFFFFFFFF),      0xFFFFFFFF, result);    if (result == -1) goto exit;
    DRV_WriteReg32(TEMPADCPNP1, temp);

    temp = DRV_Reg32(TEMPADCPNP2);
    DRV_WriteReg32(TEMPADCPNP2, 0xFFFFFFFF);
    REG_CHK((DRV_Reg32(TEMPADCPNP2) & 0xFFFFFFFF),      0xFFFFFFFF, result);    if (result == -1) goto exit;
    DRV_WriteReg32(TEMPADCPNP2, temp);

    temp = DRV_Reg32(TEMPADCMUX);
    DRV_WriteReg32(TEMPADCMUX, 0xFFFFFFFF);
    REG_CHK((DRV_Reg32(TEMPADCMUX) & 0xFFFFFFFF),      0xFFFFFFFF, result);     if (result == -1) goto exit;
    DRV_WriteReg32(TEMPADCMUX, temp);

    temp = DRV_Reg32(TEMPADCEXT);
    DRV_WriteReg32(TEMPADCEXT, 0xFFFFFFFF);
    REG_CHK((DRV_Reg32(TEMPADCEXT) & 0xFFFFFFFF),      0xFFFFFFFF, result);     if (result == -1) goto exit;
    DRV_WriteReg32(TEMPADCEXT, temp);

    temp = DRV_Reg32(TEMPADCEXT1);
    DRV_WriteReg32(TEMPADCEXT1, 0xFFFFFFFF);
    REG_CHK((DRV_Reg32(TEMPADCEXT1) & 0xFFFFFFFF),      0xFFFFFFFF, result);    if (result == -1) goto exit;
    DRV_WriteReg32(TEMPADCEXT1, temp);

    temp = DRV_Reg32(TEMPADCEN);
    DRV_WriteReg32(TEMPADCEN, 0xFFFFFFFF);
    REG_CHK((DRV_Reg32(TEMPADCEN) & 0xFFFFFFFF),      0xFFFFFFFF, result);      if (result == -1) goto exit;
    DRV_WriteReg32(TEMPADCEN, temp);

    temp = DRV_Reg32(TEMPPNPMUXADDR);
    DRV_WriteReg32(TEMPPNPMUXADDR, 0xFFFFFFFF);
    REG_CHK((DRV_Reg32(TEMPPNPMUXADDR) & 0xFFFFFFFF),      0xFFFFFFFF, result); if (result == -1) goto exit;
    DRV_WriteReg32(TEMPPNPMUXADDR, temp);

    temp = DRV_Reg32(TEMPADCMUXADDR);
    DRV_WriteReg32(TEMPADCMUXADDR, 0xFFFFFFFF);
    REG_CHK((DRV_Reg32(TEMPADCMUXADDR) & 0xFFFFFFFF), 0xFFFFFFFF, result);      if (result == -1) goto exit;
    DRV_WriteReg32(TEMPADCMUXADDR, temp);

    temp = DRV_Reg32(TEMPADCEXTADDR);
    DRV_WriteReg32(TEMPADCEXTADDR, 0xFFFFFFFF);
    REG_CHK((DRV_Reg32(TEMPADCEXTADDR) & 0xFFFFFFFF), 0xFFFFFFFF, result);      if (result == -1) goto exit;
    DRV_WriteReg32(TEMPADCEXTADDR, temp);

    temp = DRV_Reg32(TEMPADCEXT1ADDR);
    DRV_WriteReg32(TEMPADCEXT1ADDR, 0xFFFFFFFF);
    REG_CHK((DRV_Reg32(TEMPADCEXT1ADDR) & 0xFFFFFFFF), 0xFFFFFFFF, result);     if (result == -1) goto exit;
    DRV_WriteReg32(TEMPADCEXT1ADDR, temp);

    temp = DRV_Reg32(TEMPADCENADDR);
    DRV_WriteReg32(TEMPADCENADDR, 0xFFFFFFFF);
    REG_CHK((DRV_Reg32(TEMPADCENADDR) & 0xFFFFFFFF),  0xFFFFFFFF, result);      if (result == -1) goto exit;
    DRV_WriteReg32(TEMPADCENADDR, temp);

    temp = DRV_Reg32(TEMPADCVALIDADDR);
    DRV_WriteReg32(TEMPADCVALIDADDR, 0xFFFFFFFF);
    REG_CHK((DRV_Reg32(TEMPADCVALIDADDR) & 0xFFFFFFFF),  0xFFFFFFFF, result);   if (result == -1) goto exit;
    DRV_WriteReg32(TEMPADCVALIDADDR, temp);

    temp = DRV_Reg32(TEMPADCVOLTADDR);
    DRV_WriteReg32(TEMPADCVOLTADDR, 0xFFFFFFFF);
    REG_CHK((DRV_Reg32(TEMPADCVOLTADDR) & 0xFFFFFFFF),  0xFFFFFFFF, result);    if (result == -1) goto exit;
    DRV_WriteReg32(TEMPADCVOLTADDR, temp);

    temp = DRV_Reg32(TEMPRDCTRL);
    DRV_WriteReg32(TEMPRDCTRL, 0x00000001);
    REG_CHK((DRV_Reg32(TEMPRDCTRL) & 0x00000001),  0x00000001, result);         if (result == -1) goto exit;
    DRV_WriteReg32(TEMPRDCTRL, temp);

    temp = DRV_Reg32(TEMPADCVALIDMASK);
    DRV_WriteReg32(TEMPADCVALIDMASK, 0x0000003F);
    REG_CHK((DRV_Reg32(TEMPADCVALIDMASK) & 0x0000003F),  0x0000003F, result);   if (result == -1) goto exit;
    DRV_WriteReg32(TEMPADCVALIDMASK, temp);

    temp = DRV_Reg32(TEMPADCVOLTAGESHIFT);
    DRV_WriteReg32(TEMPADCVOLTAGESHIFT, 0x0000001F);
    REG_CHK((DRV_Reg32(TEMPADCVOLTAGESHIFT) & 0x0000001F),0x0000001F, result);  if (result == -1) goto exit;
    DRV_WriteReg32(TEMPADCVOLTAGESHIFT, temp);

    temp = DRV_Reg32(TEMPADCWRITECTRL);
    DRV_WriteReg32(TEMPADCWRITECTRL, 0x0000000F);
    REG_CHK((DRV_Reg32(TEMPADCWRITECTRL) & 0x0000000F),0x0000000F, result);     if (result == -1) goto exit;
    DRV_WriteReg32(TEMPADCWRITECTRL, temp);


    UTIL_Printf("ts_thermal: THERMAL_Register_RW_Check() => PASS\r\n");
	
exit:
    return result;
}

static int THERMAL_Register_RO_Check(void)
{
    int result = 1;
    volatile kal_uint32 temp;

    temp = DRV_Reg32(TEMPMONINTSTS);
    DRV_WriteReg32(TEMPMONINTSTS, 0x00123456);
    REG_CHK((DRV_Reg32(TEMPMONINTSTS) & 0x00FFFFFF),  temp, result);    if (result == -1) goto exit;

    temp = DRV_Reg32(TEMPMSRCTL1);
    DRV_WriteReg32(TEMPMSRCTL1, 0x00000001);
    REG_CHK((DRV_Reg32(TEMPMSRCTL1) & 0x00000001),    temp, result);    if (result == -1) goto exit;

    temp = DRV_Reg32(TEMPMSR0);
    DRV_WriteReg32(TEMPMSR0, 0x00000123);
    REG_CHK((DRV_Reg32(TEMPMSR0) & 0x00000FFF),       (temp& 0x00000FFF), result);    if (result == -1) goto exit;

    temp = DRV_Reg32(TEMPMSR1);
    DRV_WriteReg32(TEMPMSR1, 0x00000123);
    REG_CHK((DRV_Reg32(TEMPMSR1) & 0x00000FFF),       (temp& 0x00000FFF), result);    if (result == -1) goto exit;

    temp = DRV_Reg32(TEMPMSR2);
    DRV_WriteReg32(TEMPMSR2, 0x00000123);
    REG_CHK((DRV_Reg32(TEMPMSR2) & 0x00000FFF),       (temp& 0x00000FFF), result);    if (result == -1) goto exit;

    temp = DRV_Reg32(TEMPIMMD0);
    DRV_WriteReg32(TEMPIMMD0, 0x00000123);
    REG_CHK((DRV_Reg32(TEMPIMMD0) & 0x00000FFF),      temp, result);    if (result == -1) goto exit;

    temp = DRV_Reg32(TEMPIMMD1);
    DRV_WriteReg32(TEMPIMMD1, 0x00000123);
    REG_CHK((DRV_Reg32(TEMPIMMD1) & 0x00000FFF),      temp, result);    if (result == -1) goto exit;

    temp = DRV_Reg32(TEMPIMMD2);
    DRV_WriteReg32(TEMPIMMD2, 0x00000123);
    REG_CHK((DRV_Reg32(TEMPIMMD2) & 0x00000FFF),      temp, result);    if (result == -1) goto exit;

    UTIL_Printf("ts_thermal: THERMAL_Register_RO_Check() => PASS\r\n");
	
exit:
    return result;
}

static INT32 TS_THERMAL_Register_Test(INT32 i4Argc, const CHAR** aszArgv)
{
    int result = -1;	
	int select = 0;

	if (i4Argc < 2 ) 
	{    
		UTIL_Printf("Usage: reg xx\n");  
		UTIL_Printf("	0:Default value\n");
		UTIL_Printf("	1:Read-Only\n");
		UTIL_Printf("	2:Read-Write Check\n");
		
	    UTIL_Printf("0:defaut,1:read-only,2:rw-check \n");  
	    return result;  
	}
	
	// UTIL_Printf("[0]%s, [1]%s \r\n",aszArgv[0], aszArgv[1]);
	
	select = atoi(aszArgv[1]);
    UTIL_Printf("%s, select: %d, CKGEN_BASE_EX = [0x%lx] \r\n",__func__, select, CKGEN_BASE_EX);
    switch (select) 
    {
        case CHECK_REG_DEFAULT_VALUE:
			///
			UTIL_Printf("open thermal clock start... \n");
			thermal_clk_select(1, 1);
			thermal_clk_enable(1, 1);
			thermal_clk_reset(0);
			thermal_clk_reset(1);
			UTIL_Printf("open thermal clock end \n");
			
            UTIL_Printf("ts_thermal: enter THERMAL_Register_Default_Value_Check()\r\n");
            result = THERMAL_Register_Default_Value_Check();
            break;
        case CHECK_REG_READ_ONLY:
            UTIL_Printf("ts_thermal: enter THERMAL_Register_RO_Check()\r\n");
            result = THERMAL_Register_RO_Check();
            break;
        case CHECK_REG_READ_WRITE:
            UTIL_Printf("ts_thermal: enter THERMAL_Register_RW_Check()\r\n");
            result = THERMAL_Register_RW_Check();
            break;
        default:
            result = -1;
            break;
    }

    return result;
}

static INT32 TS_THERMAL_EXIT(INT32 i4Argc, const CHAR** aszArgv)
{
	thermal_exit();

	///
	UTIL_Printf("close thermal clock start... \n");
	thermal_clk_enable(0, 0);
	thermal_clk_reset(0);
	UTIL_Printf("close thermal clock end \n");
	
    return 0;
}

extern void vDrvThermal_Cal_Prepare(void);
extern void vDrvThermal_Cal_Prepare_2(void);

static INT32 TS_THERMAL_READ_REAL_TEMP(INT32 i4Argc, const CHAR** aszArgv)
{
	UTIL_Printf("read real temperature: TS_THERMAL_READ_REAL_TEMP \r\n");

	//UTIL_Printf("open thermal clock start... \n");
	thermal_clk_select(1, 1);
	thermal_clk_enable(1, 1);
	thermal_clk_reset(0);
	thermal_clk_reset(1);
	//UTIL_Printf("open thermal clock end \n");

	vDrvThermal_Cal_Prepare();
	vDrvThermal_Cal_Prepare_2();

	mtktscpu_get_hw_temp();

    return 0;
}


static INT32 TS_THERMAL_Switch_Bank(INT32 i4Argc, const CHAR** aszArgv)
{
    int result = -1;	
	int select = 0;

	if (i4Argc < 2 ) 
	{    
		UTIL_Printf("Usage: sb xx\n");    
	    UTIL_Printf("	0: Swtich BANK0\n");
		UTIL_Printf("	1: Swtich BANK1\n");
		UTIL_Printf("	2: Swtich BANK2\n");
			
	    return result;  
	}
	
	select = atoi(aszArgv[0]);
    UTIL_Printf("%s,select: %d\r\n",__func__, select);

    switch (select)
    {
        case THERMAL_BANK0:
            UTIL_Printf("ts_thermal: enter THERMAL_BANK0\r\n");
            result = mtktscpu_switch_bank(THERMAL_BANK0);
            break;
        case THERMAL_BANK1:
            UTIL_Printf("ts_thermal: enter THERMAL_BANK1\r\n");
            result = mtktscpu_switch_bank(THERMAL_BANK1);
            break;
        case THERMAL_BANK2:
            UTIL_Printf("ts_thermal: enter THERMAL_BANK0\r\n");
            result = mtktscpu_switch_bank(THERMAL_BANK2);
            break;
        default:
            result = -1;
            break;
    }

    
    return result;
}


int THERMAL_Interrupt_Trigger_Check(void)
{
    int result = 1;
   // volatile kal_uint32 temp;

   result= thermal_interrupt_trigger_test();
//    thermal_interrupt_trigger_to_SPM_test();

    return result;
}

int THERMAL_Interrupt_Occurrance_Check(void)
{
    int result = 1;

    UTIL_Printf("\r\n");
    UTIL_Printf("THERMAL_Interrupt_Occurrance_Check: 1 times occurrance check\r\n");
    thermal_interrupt_occurrance_test(0x00000000); // 1 times occurrance
    UTIL_Printf("\r\n");

    UTIL_Printf("\r\n");
    UTIL_Printf("THERMAL_Interrupt_Occurrance_Check: 2 times occurrance check\r\n");
    thermal_interrupt_occurrance_test(0x00000155); // 2 times occurrance
    UTIL_Printf("\r\n");

    UTIL_Printf("\r\n");
    UTIL_Printf("THERMAL_Interrupt_Occurrance_Check: 3 times occurrance check\r\n");
    thermal_interrupt_occurrance_test(0x000002AA); // 3 times occurrance
    UTIL_Printf("\r\n");

    UTIL_Printf("\r\n");
    UTIL_Printf("THERMAL_Interrupt_Occurrance_Check: 4 times occurrance check\r\n");
    thermal_interrupt_occurrance_test(0x000003FF); // 4 times occurrance
    UTIL_Printf("\r\n");

    return result;
}

int THERMAL_Sensing_Filter_Option_Check(void)
{
    int result = 1;

	#if 1
    UTIL_Printf("THERMAL_Sensing_Filter_Option_Check: one sampling check\r\n");
    thermal_sensing_filter_option_test(0x00000000);

    UTIL_Printf("THERMAL_Sensing_Filter_Option_Check: average 2 sampling check\r\n");
    thermal_sensing_filter_option_test(0x00000049);
	#endif
	
    UTIL_Printf("THERMAL_Sensing_Filter_Option_Check: 4 sampling check\r\n");
    thermal_sensing_filter_option_test(0x000000A2);

	#if 1
    UTIL_Printf("THERMAL_Sensing_Filter_Option_Check: 6 sampling check\r\n");
    thermal_sensing_filter_option_test(0x000000DB);

    UTIL_Printf("THERMAL_Sensing_Filter_Option_Check: 10 sampling check\r\n");
    thermal_sensing_filter_option_test(0x0000124);

    UTIL_Printf("THERMAL_Sensing_Filter_Option_Check: 18 sampling check\r\n");
    thermal_sensing_filter_option_test(0x000016D);
	#endif

    return result;
}

int THERMAL_Immediate_Measurement_Check(void)
{
    int result = 1;

    thermal_immediate_measurement_test();

    return result;
}

int THERMAL_AHB_Timeout_Check(void)
{
    int result = 1;

    UTIL_Printf("THERMAL_AHB_Timeout_Check: Timeout interrupt will occur if sampling voltage is not valid\r\n\r\n");

    UTIL_Printf("THERMAL_AHB_Timeout_Check: AHB polling time > AHB polling timeout\r\n");
	// AHB polling time > AHB polling timeout, should has AHB timeout interrupt
	thermal_ahb_timeout_test(0x0000FFFF, 0x0000000F);

    UTIL_Printf("THERMAL_AHB_Timeout_Check: AHB polling time < AHB polling timeout\r\n");
	// AHB polling time < AHB polling timeout, should NO AHB timeout interrupt
    thermal_ahb_timeout_test(0x0000000F, 0x000000FF);
    
    UTIL_Printf("THERMAL_AHB_Timeout_Check: AHB polling time > AHB polling timeout\r\n");
	// AHB polling time > AHB polling timeout, should has AHB timeout interrupt
	thermal_ahb_timeout_test(0xFFFFFFFF, 0x0000FFFF);
	
    return result;
}

int THERMAL_First_Hot_Interrupt_Check(void)
{
    int result = 1;

    UTIL_Printf("THERMAL_First_Hot_Interrupt_Check: First Hot Interrupt Enable\r\n");
    thermal_first_hot_interrupt_test(true);

    UTIL_Printf("THERMAL_First_Hot_Interrupt_Check: First Hot Interrupt Disable\r\n");
    thermal_first_hot_interrupt_test(false);


    return result;
}

int THERMAL_Filter_Sample_Interval_Check(void)
{
    int result = 1;

    UTIL_Printf("THERMAL_Filter_Sample_Interval_Check: set to extra large interval\r\n");
    thermal_filter_sample_interval_test(0x00000100, 0x000003FF);

    UTIL_Printf("THERMAL_Filter_Sample_Interval_Check: set to normal interval\r\n");
    thermal_filter_sample_interval_test(0x000000FF, 0x000000FF);

    UTIL_Printf("THERMAL_Filter_Sample_Interval_Check: set to extra small interval\r\n");
    thermal_filter_sample_interval_test(0x00000000, 0x0000000F);

    return result;
}

int THERMAL_Different_Threshold_Check(void)
{
    int result = 1;

    UTIL_Printf("THERMAL_Different_Threshold_Check: HOT         HIGHOFFSET  HOT2NORMAL  LOWOFFSET   COLD\r\n");
    UTIL_Printf("THERMAL_Different_Threshold_Check: 0x000000AA, 0x000000BB, 0x000000FF, 0x00000111, 0x00000133\r\n");
    thermal_different_threshold_test(0x000000AA, 0x000000BB, 0x000000FF, 0x00000111, 0x00000133);

    UTIL_Printf("THERMAL_Different_Threshold_Check: HOT         HIGHOFFSET  HOT2NORMAL  LOWOFFSET   COLD\r\n");
    UTIL_Printf("THERMAL_Different_Threshold_Check: 0x00000055, 0x00000066, 0x00000088, 0x00000333, 0x00000444\r\n");
    thermal_different_threshold_test(0x00000055, 0x00000066, 0x00000088, 0x00000333, 0x00000444);

    UTIL_Printf("THERMAL_Different_Threshold_Check: HOT         HIGHOFFSET  HOT2NORMAL  LOWOFFSET   COLD\r\n");
    UTIL_Printf("THERMAL_Different_Threshold_Check: 0x000000111, 0x000000222, 0x00000333, 0x00000666, 0x00000777\r\n");
    thermal_different_threshold_test(0x00000111, 0x00000222, 0x00000333, 0x00000666, 0x00000777);

    return result;
}

int THERMAL_Interrupt_Mask_Check(void)
{
    int result = 1;

    UTIL_Printf("THERMAL_Interrupt_Mask_Check: mask all interrupt\r\n");
    thermal_interrupt_mask_test();

    return result;
}

int THERMAL_Trigger_WDT_Reset(void)
{
    int result = 1;

    UTIL_Printf("THERMAL_Trigger_WDT_Reset: check wdt reset \r\n");
    thermal_trigger_wdt_reset();

    return result;
}

static INT32 TS_THERMAL_Module_Test(INT32 i4Argc, const CHAR** aszArgv)
{
    int result = -1;	
	int select = 0;

	if (i4Argc < 2 ) 
	{    
		UTIL_Printf("Usage: mod xx\n");    
	    UTIL_Printf("	1: interrupt occurance\n");
		UTIL_Printf("	2: Sensing/Filter Option\n");
		UTIL_Printf("	3: immediate measurement\n");
		UTIL_Printf("	4: AHB Timeout\n");
		UTIL_Printf("	5: First Hot Threshold\n");
		UTIL_Printf("	6: Different Threshold\n");
		UTIL_Printf("	7: Filter sample interval\n");
		UTIL_Printf("	8: Interrupt Mask\n");
		UTIL_Printf("	9: Trigger WDT Reset\n");
		
	    return result;  
	}
	
	select = atoi(aszArgv[1]);
    UTIL_Printf("%s,select: %d\r\n",__func__, select);
	
	if (select != CHECK_IMMEDIATE_MEASUREMENT) {
		thermal_init();
	}

    switch (select) 
    {
        case CHECK_INTERRUPT_TRIGGER:
            UTIL_Printf("ts_thermal: enter THERMAL_Interrupt_Trigger_Check()\r\n");
            result = THERMAL_Interrupt_Trigger_Check();
            break;
        case CHECK_INTERRUPT_OCCURRANCE:
            UTIL_Printf("ts_thermal: enter THERMAL_Interrupt_Occurrance_Check()\r\n");
            result = THERMAL_Interrupt_Occurrance_Check();
            break;
        case CHECK_SENSING_FILTER_OPTION:
            UTIL_Printf("ts_thermal: enter THERMAL_Sensing_Filter_Option_Check()\r\n");
            result = THERMAL_Sensing_Filter_Option_Check();
            break;
        case CHECK_IMMEDIATE_MEASUREMENT:
            UTIL_Printf("ts_thermal: enter THERMAL_Immediate_Measurement_Check()\r\n");
            result = THERMAL_Immediate_Measurement_Check();
            break;
        case CHECK_AHB_TIMEOUT:
            UTIL_Printf("ts_thermal: enter THERMAL_AHB_Timeout_Check()\r\n");
            result = THERMAL_AHB_Timeout_Check();
            break;
        case CHECK_FIRST_HOT_INTERRUPT:
            UTIL_Printf("ts_thermal: enter THERMAL_First_Hot_Interrupt_Check()\r\n");
            result = THERMAL_First_Hot_Interrupt_Check();
            break;
        case CHECK_DIFFERENT_THRESHOLD:
            UTIL_Printf("ts_thermal: enter THERMAL_Different_Threshold_Check()\r\n");
            result = THERMAL_Different_Threshold_Check();
            break;
        case CHECK_FILTER_SAMPLE_INTERVAL:
            UTIL_Printf("ts_thermal: enter THERMAL_Filter_Sample_Interval_Check()\r\n");
            result = THERMAL_Filter_Sample_Interval_Check();
            break;
        case CHECK_INTERRUPT_MASK:
            UTIL_Printf("ts_thermal: enter THERMAL_Interrupt_Mask_Check()\r\n");
            result = THERMAL_Interrupt_Mask_Check();
            break;
		case CHECK_TRIGGER_WDT_RESET:
			UTIL_Printf("ts_thermal: enter THERMAL_Trigger_WDT_Reset()\r\n");
			result = THERMAL_Trigger_WDT_Reset();
			break;
        default:
            result = -1;
            break;
    }

    return result;
}


/*register CLI cmd*/
static CLI_EXEC_T _arThermalCmdTbl[] =
{
    {"Register R/W",	"reg",  TS_THERMAL_Register_Test, NULL, 
    "Register setting check", CLI_GUEST },
    
    
    {"Module Test",	"mod", TS_THERMAL_Module_Test, NULL,
    	"Module function Test", CLI_GUEST },     

    {"Interrupt Test",	"int", TS_THERMAL_Interrupt_Test, NULL,
    	"Interrupt Test", CLI_GUEST }, 

    {"TSAUXADC Test",	"adc", TS_THERMAL_Adc_Test, NULL,
    	"TSAUXADC Test", CLI_GUEST },  
    
    {"Get Real Temp",	"t",  TS_THERMAL_READ_REAL_TEMP, NULL,
    	"Get MTK real CPU Temp" ,CLI_GUEST },
    	
    {"Switch Bank",	"sb",  TS_THERMAL_Switch_Bank,  NULL,
    	" Switch Bank", CLI_GUEST },    

	{"EXIT",	"exit", TS_THERMAL_EXIT  ,  NULL,
	  	" thermal_exit", CLI_GUEST },
	  	
    {NULL, NULL, NULL, NULL, NULL, CLI_GUEST}
};


CLI_EXEC_T _rThermalVfyCmdTbl =
{
  "thm",
  NULL,
  NULL,
  _arThermalCmdTbl,
  "thermal verify command",
  CLI_GUEST
};

CLI_EXEC_T* GetThermalVfyCmdTbl(void)
{
  return &_rThermalVfyCmdTbl;
}
//EXPORT_SYMBOL(GetThermalVfyCmdTbl);

#if 0
double test_fun(void)
{

	return 0.0;
}
#endif

/* thermal export API for SLT */
long thermal_slt_proc(void *param)
{
	unsigned int temp = 0;

	//UTIL_Printf("open thermal clock start... \n");
	thermal_clk_select(1, 1);
	thermal_clk_enable(1, 1);
	thermal_clk_reset(0);
	thermal_clk_reset(1);
	//UTIL_Printf("open thermal clock end \n");

	vDrvThermal_Cal_Prepare();
	vDrvThermal_Cal_Prepare_2();

	temp = mtktscpu_get_hw_temp();
	UTIL_Printf("got tmp [0x%x](%d) \n", temp, temp);

	temp = temp / 1000;

	if ((temp>10) && (temp<80)) {
		return SLT_RET_SUCCESS;		
	} else {
		return SLT_RET_FAIL;
	}
}

/*CLI add end */
