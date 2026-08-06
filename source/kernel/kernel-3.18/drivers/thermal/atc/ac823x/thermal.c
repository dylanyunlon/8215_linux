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

#include <linux/delay.h>
#include <linux/kthread.h>
//#include "common.h"
#include "thermal.h" 
#include "sync_write.h"

#define OUTPUT_LOG  0

#if 1
extern unsigned long thm_reg_base;
extern unsigned long pdwnc_reg_base;
extern unsigned long ckgen_reg_base;
extern unsigned long efuse_reg_base;
#endif

#if 1
//#define kal_uint32 UINT32
//#define kal_int32 INT32

typedef enum 
{
    MTK_THERMAL_SENSOR_CPU0 = 0, 
    MTK_THERMAL_SENSOR_CPU1, 
    MTK_THERMAL_SENSOR_CPU2, 
    
    MTK_THERMAL_SENSOR_COUNT
} MTK_THERMAL_SENSOR_ID;


#define TS_ADC_INPUT_RANGE 28 // MT5890/5861: 2.8V / MT8135:1.5V
#define TS_BUFFER_GAIN 32 // MT5890/5861:3.2X  / MT8135:1.8x
#define TS_LOW_CRITERIAL 3192 //MT5890/5861: 4096*0.682*3.2/2.8=3192  / MT8135:4096*0.682*1.8/1.5=3352(3350)

static kal_int32 g_adc_ge = 0;  // ADC Gain Error + 512 (Uint: 1/4096) 
static kal_int32 g_adc_oe = 0;  // ADC Offset Error + 512  (Uint: 1/4096)
static kal_int32 g_o_vtsmcu0 = 0; // TSU0 Offset (Uint: 1/4096)
static kal_int32 g_o_vtsmcu1 = 0; // TSU1 Offset (Uint: 1/4096)
static kal_int32 g_o_vtsmcu2 = 0; // TSU2 Offset (Uint: 1/4096)
static kal_int32 g_o_vtsmcu3 = 0; // TSU3 Offset (Uint: 1/4096)
static kal_int32 g_degc_cali = 0;  // Environment tempature (Unit: degree C) * 2
static kal_int32 g_adc_cali_en = 0; // Gain and Offset calibration
static kal_int32 g_o_slope_sign = 0; // Buffer Gain 3.2 calibration slop (1.xx>=1.65, O_SLOPE_SIGN=0, 1.xx<1.65, O_SLOPE_SIGN=1)
static kal_int32 g_o_slope = 0; // Buffer Gain 3.2 calibration value
static kal_int32 g_id = 0;

static kal_int32 g_ge = 0;  // ADC Real Gain Error  (Uint: 1*10000/4096) 
static kal_int32 g_oe = 0;  // ADC Real Offset Error  (Uint: 1*10000/4096) 
static kal_int32 g_gain = 0;

static kal_int32 g_x_roomt[4] = {0, 0, 0, 0};

//static int* tz_last_values[MTK_THERMAL_SENSOR_COUNT] = {NULL};
//static int TS_curr_raw[4] = {0, 0, 0, 0};
#endif

bool thermal_real_test = false;
bool thermal_intr_flag = false;

static struct task_struct *g_getTempThread;
volatile int fgStartGetTemp = 0;

extern INT32 UTIL_Printf(const CHAR *ps_format, ...);

typedef VOID (*x_os_isr_fct) (UINT16  ui2_vector_id);

#if 0
extern INT32 x_reg_isr(UINT16         ui2_vec_id,
          x_os_isr_fct   pf_isr,
          x_os_isr_fct   *ppf_old_isr);
#else
extern __s32 x_reg_isr(__u16         ui2_vec_id,
          x_os_isr_fct   pf_isr,
          x_os_isr_fct   *ppf_old_isr);
#endif

extern BOOL BIM_ClearIrq(UINT32 u4Vector);
extern BOOL BIM_EnableIrq(UINT32 u4Vector);
extern BOOL BIM_DisableIrq(UINT32 u4Vector);
static void vDrvThermalISR(UINT16 u2Vector);

static void tscpu_reset_thermal(void);

#define TAG() UTIL_Printf("%s,%d\n", __func__, __LINE__)

#ifndef MIN
#define MIN(_a_, _b_) ((_a_) > (_b_) ? (_b_) : (_a_))
#endif 

#ifndef MAX
#define MAX(_a_, _b_) ((_a_) > (_b_) ? (_a_) : (_b_))
#endif 

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


#if 0
#define THERMAL_WRAP_WR32(val,addr)        mt65xx_reg_sync_writel((val), ((void *)addr))

#define thermal_readl(addr)         DRV_Reg32(addr)

#define thermal_writel(addr, val)   mt65xx_reg_sync_writel((val), ((void *)addr))

#define thermal_setl(addr, val)     mt65xx_reg_sync_writel(thermal_readl(addr) | (val), ((void *)addr))

#define thermal_clrl(addr, val)     mt65xx_reg_sync_writel(thermal_readl(addr) & ~(val), ((void *)addr))

#else

#define THERMAL_WRAP_WR32(val,addr)        DRV_WriteReg32(addr, val)

#define thermal_readl(addr)         DRV_Reg32(addr)

#define thermal_writel(addr, val)   THERMAL_WRAP_WR32((val), (addr))

#define thermal_setl(addr, val)     THERMAL_WRAP_WR32((thermal_readl(addr) | (val)), (addr))

#define thermal_clrl(addr, val)     THERMAL_WRAP_WR32((thermal_readl(addr) & ~(val)), (addr))

#endif

static void thermal_initial_all_bank(void);
static void tscpu_config_all_tc_hw_protect( int temperature, int temperature2);

static INT32 raw_to_temperature_roomt(UINT32 ret, thermal_sensor_name ts_name);

static void print_reg_setting(void)
{
		UTIL_Printf("***************thermal setting: \n");
#if 0
		UTIL_Printf("PTPSPARE0=0x%x, TEMPAHBPOLL=0x%x,PTPCORESEL=0x%x\n0x90=0x%x, 0x94=0x%x, 0x98=0x%x\n", 
							DRV_Reg32(PTPSPARE0),DRV_Reg32(TEMPAHBPOLL), 
							DRV_Reg32(PTPCORESEL), DRV_Reg32(TEMPMSR0), DRV_Reg32(TEMPMSR1), DRV_Reg32(TEMPMSR2));
#endif
		UTIL_Printf("TEMPPNPMUXADDR=0x%x, TEMPADCMUXADDR=0x%x, TEMPADCENADDR=0x%x, TEMPADCVALIDADDR = 0x%x, TEMPADCVOLTADDR=0x%x\n", 
							DRV_Reg32(TEMPPNPMUXADDR), DRV_Reg32(TEMPADCMUXADDR), DRV_Reg32(TEMPADCENADDR), 
							DRV_Reg32(TEMPADCVALIDADDR), DRV_Reg32(TEMPADCVOLTADDR));

		UTIL_Printf("***************auxadc setting: \n");
		UTIL_Printf("PDWNC_SRVCFG0=0x%x, PDWNC_SRVSWT=0x%x\n", DRV_Reg32(PDWNC_SRVCFG0), DRV_Reg32(PDWNC_SRVSWT));
		UTIL_Printf("***************thermal/auxadc setting end *******************\n");
}

static void Print_Thermal_Reg(void)
{
    UINT32 count, temp;
    UTIL_Printf("********************************\n");
    for (count = 0; count < 0x100;)
    {
        temp = DRV_Reg32(THERM_CTRL_BASE + count);
        UTIL_Printf("0x%08X | %08X ", THERM_CTRL_BASE + count, temp);
        temp = DRV_Reg32(THERM_CTRL_BASE + count + 4);
        UTIL_Printf("%08X ", temp);
        temp = DRV_Reg32(THERM_CTRL_BASE + count + 8);
        UTIL_Printf("%08X ", temp);
        temp = DRV_Reg32(THERM_CTRL_BASE + count + 12);
        UTIL_Printf("%08X\n", temp);
        count += 0x10;
    }
    UTIL_Printf("********************************\n");
}


/*
Bank0 : CPU (TS_MCU1,TS_MCU2)        (TS3, TS4)
Bank1 : GPU (TS_MCU3)                (TS5)
Bank2 : SOC (TS_MCU4,TS_MCU2,TS_MCU3)(TS1, TS4, TS5)

TS_MCU1: TS3 (9464.54, 65.8)
TS_MCU2: TS4 (6745.06, 2790.2)
TS_MCU3: TS5 (5673.50, 6392.4)
TS_MCU4: TS1 (3349.22, 4163.6)
TS_ABB:  TS2

*/
static int CPU_TS_MCU1_T=0,CPU_TS_MCU2_T=0;
static int GPU_TS_MCU3_T=0,ABB_TS_ABB_T=0;
static int SOC_TS_MCU4_T=0,SOC_TS_MCU2_T=0,SOC_TS_MCU3_T=0;

static int CPU_TS_MCU1_R=0,CPU_TS_MCU2_R=0;
static int GPU_TS_MCU3_R=0;
static int SOC_TS_MCU4_R=0,SOC_TS_MCU2_R=0,SOC_TS_MCU3_R=0;

#if 1
/* therm_ck: 0 -> 27M, 1-> usbpll_d8, 2->syspll_d12, 3->syspll_d16
    therm_slow_ck: 0 -> 27M, 1-> 27M_d512, 2->27M_d1024, 3->27M_d2048 */
int thermal_clk_select(UINT32 therm_ck, UINT32 therm_slow_ck)
{
    UINT32 temp = 0;

	// select therm_ck for thermal
	temp = DRV_Reg32(CKGEN_BASE_EX + 0x18);
	temp &= (~(3 << 22));
	temp |= (therm_ck << 22);
	DRV_WriteReg32(CKGEN_BASE_EX + 0x18, temp);

	temp = DRV_Reg32(CKGEN_BASE_EX + 0x18);
	//UTIL_Printf("thermal_clk_select, 1, 0x18 = [0x%x] \n", temp);

	// select therm_slow_ck for thermal
	temp = DRV_Reg32(CKGEN_BASE_EX + 0x18);
	temp &= (~(3 << 20));
	temp |= (therm_slow_ck << 20);
	DRV_WriteReg32(CKGEN_BASE_EX + 0x18, temp);

	temp = DRV_Reg32(CKGEN_BASE_EX + 0x18);
	//UTIL_Printf("thermal_clk_select, 2, 0x18 = [0x%x] \n", temp);

    return 0;
}

/* therm_ck_flag: 0 -> disable, 1-> enable
    therm_slow_ck_flag: 0 -> disable, 1-> enable */
int thermal_clk_enable(UINT32 therm_ck_flag, UINT32 therm_slow_ck_flag)
{
    UINT32 temp = 0;

	// enable therm_ck for thermal
	if (therm_ck_flag) {
		temp = DRV_Reg32(CKGEN_BASE_EX + 0xA0);
		temp |= (therm_ck_flag << 18);
		DRV_WriteReg32(CKGEN_BASE_EX + 0xA0, temp);
	} else {
		temp = DRV_Reg32(CKGEN_BASE_EX + 0xA0);
		temp &= (~(therm_ck_flag << 18));
		DRV_WriteReg32(CKGEN_BASE_EX + 0xA0, temp);
	}

	temp = DRV_Reg32(CKGEN_BASE_EX + 0xA0);
	//UTIL_Printf("thermal_clk_enable, 1, 0xA0 = [0x%x] \n", temp);

	// enable therm_slow_ck for thermal
	if (therm_slow_ck_flag) {
		temp = DRV_Reg32(CKGEN_BASE_EX + 0xA0);
		temp |= (therm_slow_ck_flag << 19);
		DRV_WriteReg32(CKGEN_BASE_EX + 0xA0, temp);
	} else {
		temp = DRV_Reg32(CKGEN_BASE_EX + 0xA0);
		temp &= (~(therm_slow_ck_flag << 19));
		DRV_WriteReg32(CKGEN_BASE_EX + 0xA0, temp);
	}

	temp = DRV_Reg32(CKGEN_BASE_EX + 0xA0);
	//UTIL_Printf("thermal_clk_enable, 2, 0xA0 = [0x%x] \n", temp);

    return 0;
}

/*  therm_clock: 0 -> clear 0, 1-> reset */
int thermal_clk_reset(UINT32 therm_reset)
{
    UINT32 temp = 0;

	// reset therm_clock for thermal
	if (therm_reset) {
		temp = DRV_Reg32(CKGEN_BASE_EX + 0xBC);
		temp |= (3 << 18);
		DRV_WriteReg32(CKGEN_BASE_EX + 0xBC, temp);
	} else {
		temp = DRV_Reg32(CKGEN_BASE_EX + 0xBC);
		temp &= (~(3 << 18));
		DRV_WriteReg32(CKGEN_BASE_EX + 0xBC, temp);
	}

	temp = DRV_Reg32(CKGEN_BASE_EX + 0xBC);
	//UTIL_Printf("thermal_clk_reset, 0xBC = [0x%x] \n", temp);

    return 0;
}
#endif

void thermal_controller_Reset(void)
{

	////UTIL_Printf("==============================================\n");
    ////UTIL_Printf("Select&enable clock and Reset module for Thermal Controller \n");
	////UTIL_Printf("==============================================\n");

	//UTIL_Printf("open & reset thermal clock start... \n");
	thermal_clk_select(1, 1);
	thermal_clk_enable(1, 1);
	thermal_clk_reset(0);
	thermal_clk_reset(1);
	//UTIL_Printf("open thermal clock end \n");

	return;
}


/*******************************************************************************
 * FUNCTION
 *	thermal_get_interrupt_status
 *
 * DESCRIPTION
 *	This function will return the interrupt status
 *
 * PARAMETERS
 *  None
 *
 * RETURNS
 *	The interrupt status
 ******************************************************************************/
kal_uint32 thermal_get_interrupt_status(void)
{
    // UTIL_Printf("thermal_get_interrupt_status: get interrupt status = 0x%x \n", DRV_Reg32(TEMPMONINTSTS));
    return DRV_Reg32(TEMPMONINTSTS);
}

/*******************************************************************************
 * FUNCTION
 *	thermal_lisr
 *
 * DESCRIPTION
 *	This function will handle and identify interrupt trigger type
 *
 * PARAMETERS
 *  None
 *
 * RETURNS
 *	None
 ******************************************************************************/
void thermal_lisr(void)
{
    kal_uint32 temp = 0, ret = 0;
   
    ret = DRV_Reg32(TEMPMONINTSTS);
    // UTIL_Printf("[thermal_lisr] thermal interrupt trigger, TEMPMONINTSTS = 0x%x\n", ret);

    if(ret & THERMAL_tri_SPM_State0)
    	UTIL_Printf("[thermal_lisr] Thermal state0 to trigger SPM state0- cold \n");
    if(ret & THERMAL_tri_SPM_State1)
    	UTIL_Printf("[thermal_lisr] Thermal state1 to trigger SPM state1- normal \n");
    if(ret & THERMAL_tri_SPM_State2)
    {
    	UTIL_Printf("[thermal_lisr] Thermal state2 to trigger SPM state2 - High \n");
        UTIL_Printf("[thermal_lisr] watch dog status:  0xf0024004=0x%x, 0xf0024140=0x%x\n", DRV_Reg32(0xfd024004), DRV_Reg32(0xfd024140));
    }

    if (ret & THERMAL_MON_CINTSTS0)
    {
        UTIL_Printf("[thermal_lisr] thermal sensor point 0 - cold interrupt trigger\n");
    }
    if (ret & THERMAL_MON_HINTSTS0)
    {
        UTIL_Printf("[thermal_lisr] thermal sensor point 0 - hot interrupt trigger\n");
    }
    if (ret & THERMAL_MON_LOINTSTS0)
        UTIL_Printf("[thermal_lisr] thermal sensor point 0 - low offset interrupt trigger\n");
    if (ret & THERMAL_MON_HOINTSTS0)
    {
        UTIL_Printf("[thermal_lisr] thermal sensor point 0 - high offset interrupt trigger\n");
    }
    if (ret & THERMAL_MON_NHINTSTS0)
        UTIL_Printf("[thermal_lisr] thermal sensor point 0 - hot to normal interrupt trigger\n");

    if (ret & THERMAL_MON_CINTSTS1)
        UTIL_Printf("[thermal_lisr] thermal sensor point 1 - cold interrupt trigger\n");
    if (ret & THERMAL_MON_HINTSTS1)
    {
        UTIL_Printf("[thermal_lisr] thermal sensor point 1 - hot interrupt trigger\n");
    }
    if (ret & THERMAL_MON_LOINTSTS1)
        UTIL_Printf("[thermal_lisr] thermal sensor point 1 - low offset interrupt trigger\n");
    if (ret & THERMAL_MON_HOINTSTS1)
    {
        UTIL_Printf("[thermal_lisr] thermal sensor point 1 - high offset interrupt trigger\n");
    }
    if (ret & THERMAL_MON_NHINTSTS1)
        UTIL_Printf("[thermal_lisr] thermal sensor point 1 - hot to normal interrupt trigger\n");

    if (ret & THERMAL_MON_CINTSTS2)
        UTIL_Printf("[thermal_lisr] thermal sensor point 2 - cold interrupt trigger\n");
    if (ret & THERMAL_MON_HINTSTS2)
        UTIL_Printf("[thermal_lisr] thermal sensor point 2 - hot interrupt trigger\n");
    if (ret & THERMAL_MON_LOINTSTS2)
        UTIL_Printf("[thermal_lisr] thermal sensor point 2 - low offset interrupt trigger\n");
    if (ret & THERMAL_MON_HOINTSTS2)
        UTIL_Printf("[thermal_lisr] thermal sensor point 2 - high offset interrupt trigger\n");
    if (ret & THERMAL_MON_NHINTSTS2)
        UTIL_Printf("[thermal_lisr] thermal sensor point 2 - hot to normal interrupt trigger\n");

    if (ret & THERMAL_MON_CINTSTS3)
        UTIL_Printf("[thermal_lisr] thermal sensor point 3 - cold interrupt trigger\n");
    if (ret & THERMAL_MON_HINTSTS3)
        UTIL_Printf("[thermal_lisr] thermal sensor point 3 - hot interrupt trigger\n");
    if (ret & THERMAL_MON_LOINTSTS3)
        UTIL_Printf("[thermal_lisr] thermal sensor point 3 - low offset interrupt trigger\n");
    if (ret & THERMAL_MON_HOINTSTS3)
        UTIL_Printf("[thermal_lisr] thermal sensor point 3 - high offset interrupt trigger\n");
    if (ret & THERMAL_MON_NHINTSTS3)
        UTIL_Printf("[thermal_lisr] thermal sensor point 3 - hot to normal interrupt trigger\n");

    if (ret & THERMAL_MON_TOINTSTS)
        UTIL_Printf("[thermal_lisr] poling timeout interrupt trigger\n");

    if (ret & THERMAL_MON_IMMDINTSTS0)
    {
        temp = DRV_Reg32(TEMPMONINT);
        temp &= 0xFFFEFFFF;
        DRV_WriteReg32(TEMPMONINT, temp); // disable immediate interrupt for sense point 0

        UTIL_Printf("[thermal_lisr] thermal sensor point 0 - immediate sense interrupt trigger\n");

        if (!thermal_real_test)
        {
            temp = DRV_Reg32(TEMPIMMD0);
            while ((temp & 0x8000) == 0)
                temp = DRV_Reg32(TEMPIMMD0);
            if ((temp & 0x0FFF) != 0x14) // first filter valid should be equal to 0x14
                UTIL_Printf("[thermal_lisr] sensor 0 immediate interrupt trigger fail, read TEMPIMMD0 = 0x%x\n", (temp & 0x0FFF));
            else
                UTIL_Printf("[thermal_lisr] sensor 0 immediate interrupt trigger pass, read TEMPIMMD0 = 0x%x\n", (temp & 0x0FFF));
        }

    }
    if (ret & THERMAL_MON_IMMDINTSTS1)
    {
        temp = DRV_Reg32(TEMPMONINT);
        temp &= 0xFFFDFFFF;
        DRV_WriteReg32(TEMPMONINT, temp); // disable immediate interrupt for sense point 1

        UTIL_Printf("[thermal_lisr] thermal sensor point 1 - immediate sense interrupt trigger\n");

        if (!thermal_real_test)
        {
            temp = DRV_Reg32(TEMPIMMD1);
            while ((temp & 0x8000) == 0)
                temp = DRV_Reg32(TEMPIMMD1);
            if ((temp & 0x0FFF) != 0x14) // first filter valid should be equal to 0x14
                UTIL_Printf("[thermal_lisr] sensor 1 immediate interrupt trigger fail, read TEMPIMMD1 = 0x%x\n", (temp & 0x0FFF));
            else
                UTIL_Printf("[thermal_lisr] sensor 1 immediate interrupt trigger pass, read TEMPIMMD1 = 0x%x\n", (temp & 0x0FFF));
        }
    }
    if (ret & THERMAL_MON_IMMDINTSTS2)
    {
        temp = DRV_Reg32(TEMPMONINT);
        temp &= 0xFFFBFFFF;
        DRV_WriteReg32(TEMPMONINT, temp); // disable immediate interrupt for sense point 2

        UTIL_Printf("[thermal_lisr] thermal sensor point 2 - immediate sense interrupt trigger\n");

        if (!thermal_real_test)
        {
            temp = DRV_Reg32(TEMPIMMD2);
            while ((temp & 0x8000) == 0)
                temp = DRV_Reg32(TEMPIMMD2);
            if ((temp & 0x0FFF) != 0x14) // first filter valid should be equal to 0x14
                UTIL_Printf("[thermal_lisr] sensor 2 immediate interrupt trigger fail, read TEMPIMMD2 = 0x%x\n", (temp & 0x0FFF));
            else
                UTIL_Printf("[thermal_lisr] sensor 2 immediate interrupt trigger pass, read TEMPIMMD2 = 0x%x\n", (temp & 0x0FFF));
        }
    }

	if (ret & THERMAL_MON_IMMDINTSTS3)
    {
        temp = DRV_Reg32(TEMPMONINT);
        temp &= 0xFFFBFFFF;
        DRV_WriteReg32(TEMPMONINT, temp); // disable immediate interrupt for sense point 2

        UTIL_Printf("[thermal_lisr] thermal sensor point 3 - immediate sense interrupt trigger\n");

        if (!thermal_real_test)
        {
            temp = DRV_Reg32(TEMPIMMD3);
            while ((temp & 0x8000) == 0)
                temp = DRV_Reg32(TEMPIMMD3);
            if ((temp & 0x0FFF) != 0x14) // first filter valid should be equal to 0x14
                UTIL_Printf("[thermal_lisr] sensor 3 immediate interrupt trigger fail, read TEMPIMMD3 = 0x%x\n", (temp & 0x0FFF));
            else
                UTIL_Printf("[thermal_lisr] sensor 3 immediate interrupt trigger pass, read TEMPIMMD3 = 0x%x\n", (temp & 0x0FFF));
        }
    }

    if (ret & THERMAL_MON_FILTINTSTS0) // just for debug
    {
        temp = DRV_Reg32(TEMPMONINTSTS);
        temp &= 0xFFF7FFFF;
        //DRV_WriteReg32(TEMPMONINT, temp);     // disable to filter sense point 0
        UTIL_Printf("[thermal_lisr] thermal sensor point 0 - THERMAL_MON_FILTINTSTS0 \n");
    }
    if (ret & THERMAL_MON_FILTINTSTS1) // just for debug
    {
        temp = DRV_Reg32(TEMPMONINTSTS); 
        temp &= 0xFFEFFFFF;
        //DRV_WriteReg32(TEMPMONINT, temp);     // disable to filter sense point 1
        UTIL_Printf("[thermal_lisr] thermal sensor point 1 - THERMAL_MON_FILTINTSTS1 \n");
    }
    if (ret & THERMAL_MON_FILTINTSTS2) // just for debug
    {
        temp = DRV_Reg32(TEMPMONINTSTS); 
        temp &= 0xFFDFFFFF;
        //DRV_WriteReg32(TEMPMONINT, temp);     // disable to filter sense point 2
        UTIL_Printf("[thermal_lisr] thermal sensor point 2 - THERMAL_MON_FILTINTSTS2 \n");
    }
    if (ret & THERMAL_MON_FILTINTSTS3) // just for debug
    {
        temp = DRV_Reg32(TEMPMONINTSTS); 
        temp &= 0xEFFFFFFF;
        //DRV_WriteReg32(TEMPMONINT, temp);     // disable to filter sense point 3
        UTIL_Printf("[thermal_lisr] thermal sensor point 3 - THERMAL_MON_FILTINTSTS3 \n");
    }

    /*
    temp = DRV_Reg32(TEMPMSR0);
    while ((temp & 0x8000) == 0)
        temp = DRV_Reg32(TEMPMSR0);
    UTIL_Printf("[thermal_lisr] read TEMPMSR0 = 0x%x\n", temp & 0x0FFF);

    temp = DRV_Reg32(TEMPMSR1);
    while ((temp & 0x8000) == 0)
        temp = DRV_Reg32(TEMPMSR1);
    UTIL_Printf("[thermal_lisr] read TEMPMSR1 = 0x%x\n", temp & 0x0FFF);
    */

    thermal_intr_flag = true;
}


#define HW_TS_CHANNEL_EN Fld(16,16,AC_FULLW32)//[31:16]

int thermal_interrupt_trigger_to_SPM_test(void)
{
	UINT32 temp = 0, count = 0, i = 0;

    thermal_real_test = false;

	// Thermal Controller 1
	//hywu: 330/32K * 190 = 10ms * 400 = 4s (sensing interval ??)
	DRV_WriteReg32(TEMPMONCTL1, 0x0000014A);                // counting unit is 330 / 66M = 5us
    DRV_WriteReg32(TEMPMONCTL2, 0x00000190);                // sensing interval is 400 * 5us = 2ms
	
	DRV_WriteReg32(TEMPAHBPOLL, 0x0000000F);                // polling interval to check if temperature sense is ready
    DRV_WriteReg32(TEMPAHBTO, 0x000000FF);                  // exceed this polling time, IRQ would be inserted
    
	DRV_WriteReg32(TEMPMONIDET0, 0x00000000);
	DRV_WriteReg32(TEMPMONIDET1, 0x00000000);
	DRV_WriteReg32(TEMPMONIDET2, 0x00000000);

	DRV_WriteReg32(TEMPH2NTHRE,  0x000000FF);	
	DRV_WriteReg32(TEMPHTHRE,    0x000000AA);	
	DRV_WriteReg32(TEMPCTHRE,    0x00000133);		

    DRV_WriteReg32(TEMPMSRCTL0, 0x0000000);                 // temperature measurement sampling control (one sampling)

    DRV_WriteReg32(TEMPADCPNP0, 0x1);                       // this value will be stored to TEMPPNPMUXADDR (TEMPSPARE0) automatically by hw
    DRV_WriteReg32(TEMPADCPNP1, 0x2);                       // this value will be stored to TEMPPNPMUXADDR (TEMPSPARE0) automatically by hw
	DRV_WriteReg32(TEMPADCPNP2, 0x3);

    DRV_WriteReg32(TEMPADCMUX, 0x800);
    DRV_WriteReg32(TEMPADCEN, 0x800);                         // AHB value for auxadc enable

	DRV_WriteReg32(TEMPPNPMUXADDR, (UINT32) TEMPSPARE0);    // AHB address for pnp sensor mux selection
    DRV_WriteReg32(TEMPADCMUXADDR, (UINT32) TEMPSPARE0);    // AHB address for auxadc mux selection
    DRV_WriteReg32(TEMPADCENADDR, (UINT32) TEMPSPARE1);     // AHB address for auxadc enable
    DRV_WriteReg32(TEMPADCVALIDADDR, (UINT32) TEMPSPARE2);  // AHB address for auxadc valid bit
    DRV_WriteReg32(TEMPADCVOLTADDR, (UINT32) TEMPSPARE2);   // AHB address for auxadc voltage output

	DRV_WriteReg32(TEMPRDCTRL, 0x0);                        // read valid & voltage are at the same register
    DRV_WriteReg32(TEMPADCVALIDMASK, 0x0000002C);           // indicate where the valid bit is (the 12th bit is valid bit and 1 is valid)
    DRV_WriteReg32(TEMPADCVOLTAGESHIFT, 0x0);               // do not need to shift
    DRV_WriteReg32(TEMPADCWRITECTRL, 0x3);                  // enable auxadc mux & pnp write transaction

    DRV_WriteReg32(TEMPPROTCTL, 0x20000);                   // !!! maximum of sensors
    DRV_WriteReg32(TEMPPROTTA, 0x133);                      // !!! COLD
    DRV_WriteReg32(TEMPPROTTB, 0xFF);                       // !!! NORMAL
    DRV_WriteReg32(TEMPPROTTC, 0xAA);                       // !!! HOT

	//hywu: enable thermal protection INT (1110).
 	DRV_WriteReg32(TEMPMONINT, 0xE0000000);                 // !!!0xE0000000 only enable interrupt for SPM thermal protection
    DRV_WriteReg32(TEMPMONCTL0, 0x00000007);                // enable all sensing point (sensing point 0,1,2)

    // Weiyi: 0x6810 is SPM wakeup event mask, 0 as all wakeup source could wake SPM up
    //DRV_WriteReg32(0x10006810,0);
       
	for(i=0;i<2;i++)
	{
	    //low temperature

	    UTIL_Printf("\n=================================================\n");
		UTIL_Printf("thermal_interrupt_trigger_to_SPM_test: low temp\n");
	    UTIL_Printf("=================================================\n");
	    temp = 0x234;
	    DRV_WriteReg32(TEMPSPARE2, (0x00001000 + temp));

        temp = DRV_Reg32(TEMPMSR0);
	    while ((temp & 0x0FFF) != 0x234)
	    {
	        kal_sleep_task(10000);
            UTIL_Printf(".");
	        temp = DRV_Reg32(TEMPMSR0);
	    }
		UTIL_Printf("\n");
		
	    
	    thermal_intr_flag = false;

	 	count=10;
	 	while (count--)
		{
	        kal_sleep_task(10000);
	    }

	 	//low offset temperature

	 	UTIL_Printf("\n=================================================\n");
	 	UTIL_Printf("thermal_interrupt_trigger_to_SPM_test: low offset\n");
        UTIL_Printf("=================================================\n");
	    temp = 0x111;
	    DRV_WriteReg32(TEMPSPARE2, (0x00001000 + temp));


        temp = DRV_Reg32(TEMPMSR0);
		while ((temp & 0x0FFF) != 0x111)
	    {
	        kal_sleep_task(10000);
	        temp = DRV_Reg32(TEMPMSR0);
	    }
		
	    thermal_intr_flag = false;

	 	count=10;
	 	while (count--)
		{
	        kal_sleep_task(10000);
	    }

	 	//high offset temperature
	 	UTIL_Printf("\n=================================================\n");
	 	UTIL_Printf("thermal_interrupt_trigger_to_SPM_test: high offset\n");
        UTIL_Printf("=================================================\n");
	    temp = 0xBB;
	    DRV_WriteReg32(TEMPSPARE2, (0x00001000 + temp));


        temp = DRV_Reg32(TEMPMSR0);
		while ((temp & 0x0FFF) != 0xBB)
	    {
	        kal_sleep_task(10000);
	        temp = DRV_Reg32(TEMPMSR0);
	    }
	
	    thermal_intr_flag = false;

		count=10;
	 	while (count--)
		{
	        kal_sleep_task(10000);
	    }


		//high temperature
		UTIL_Printf("\n=================================================\n");
		UTIL_Printf("thermal_interrupt_trigger_to_SPM_test: high temp\n");
        UTIL_Printf("=================================================\n");
	    temp = 0x14;                                            // set to very hot for pnp0
	    DRV_WriteReg32(TEMPSPARE2, (0x00001000 + temp));        // set sensor voltage and sensor valid


	    temp = DRV_Reg32(TEMPMSR0);
		while ((temp & 0x0FFF) != 0x14)
	    {
	        kal_sleep_task(10000);
	        temp = DRV_Reg32(TEMPMSR0);
	    }


	    temp = DRV_Reg32(TEMPMSR0);
	    if ((temp & 0x0FFF) != 0x14) // first filter valid should be equal to 0x14
	    {
	        UTIL_Printf("thermal_interrupt_trigger_to_SPM_test: fail, read TEMPMSR0 = 0x%x\n", (temp & 0x0FFF));
	        return -1;
	    }
	    else
	        UTIL_Printf("thermal_interrupt_trigger_to_SPM_test: pass, read TEMPMSR0 = 0x%x\n", (temp & 0x0FFF));

	    temp = DRV_Reg32(TEMPMSR1);
	    if ((temp & 0x0FFF) != 0x14) // first filter valid should be equal to 0x14
	    {
	        UTIL_Printf("thermal_interrupt_trigger_to_SPM_test: fail, read TEMPMSR1 = 0x%x\n", (temp & 0x0FFF));
	        return -1;
	    }
	    else
	        UTIL_Printf("thermal_interrupt_trigger_to_SPM_test: pass, read TEMPMSR1 = 0x%x\n", (temp & 0x0FFF));

	    temp = DRV_Reg32(TEMPMSR2);
	    if ((temp & 0x0FFF) != 0x14) // first filter valid should be equal to 0x14
	    {
	        UTIL_Printf("thermal_interrupt_trigger_to_SPM_test: fail, read TEMPMSR2 = 0x%x\n", (temp & 0x0FFF));
	        return -1;
	    }
	    else
	        UTIL_Printf("thermal_interrupt_trigger_to_SPM_test: pass, read TEMPMSR2 = 0x%x\n", (temp & 0x0FFF));

	    UTIL_Printf("\n\n\n");

	    thermal_intr_flag = false;
	}
	return 0;
}


#define PDWNC_SPARE 0xf0024170

int thermal_interrupt_trigger_test(void)
{
    UINT32 temp = 0, count = 0;

    thermal_real_test = false;

    //hwEnableClock(MT65XX_PDN_PERI_THERM, "Thermal");
    //hywu: 8581 period unit is 32k, but not bus clock like 8173/8135.
    //so period_unit should be 4 * 1/32K = 7.8ms.
    //sensing interval = 200(0xc8) * 7.8ms = 1560ms

    DRV_WriteReg32(TEMPMONCTL1, 0x00000004);                // counting unit is 320 * 31.25us = 10ms
    DRV_WriteReg32(TEMPMONCTL2, 0x000000C8);                // sensing interval is 200 * 10ms = 2000ms
    DRV_WriteReg32(TEMPAHBPOLL, 0x0000000F);                // polling interval to check if temperature sense is ready
    DRV_WriteReg32(TEMPAHBTO, 0x000000FF);                  // exceed this polling time, IRQ would be inserted
    DRV_WriteReg32(TEMPMONIDET0, 0x00000000);               // times for interrupt occurrance
    DRV_WriteReg32(TEMPMONIDET1, 0x00000000);               // times for interrupt occurrance

    DRV_WriteReg32(TEMPHTHRE,   0x000000AA);                // set hot threshold
    DRV_WriteReg32(TEMPOFFSETH, 0x000000BB);                // set high offset threshold
    DRV_WriteReg32(TEMPH2NTHRE, 0x000000FF);                // set hot to normal threshold
    DRV_WriteReg32(TEMPOFFSETL, 0x00000111);                // set low offset threshold
    DRV_WriteReg32(TEMPCTHRE,   0x00000133);                // set cold threshold

    DRV_WriteReg32(TEMPMSRCTL0, 0x0000000);                 // one sample

    //hywu: it may refer to TS_CON1 bit1/2/3 to allow three sensor to select 
    //different AUXADC clock mux.
    DRV_WriteReg32(TEMPADCPNP0, 0x1);                       // this value will be stored to TEMPPNPMUXADDR (TEMPSPARE0) automatically by hw
    DRV_WriteReg32(TEMPADCPNP1, 0x2);                       // this value will be stored to TEMPPNPMUXADDR (TEMPSPARE0) automatically by hw
    DRV_WriteReg32(TEMPADCPNP2, 0x3);

    //DRV_WriteReg32(TEMPADCMUX, 0x1014); // this value will be stored to TEMPPNPMUXADDR (TEMPSPARE0) automatically by hw
    DRV_WriteReg32(TEMPADCEN, 0x1014); // AHB value for auxadc enable
    DRV_WriteReg32(TEMPADCMUXADDR, PDWNC_SPARE); // AHB address for auxadc mux selection
    DRV_WriteReg32(TEMPADCENADDR, PDWNC_SPARE); // AHB address for auxadc enable
    DRV_WriteReg32(TEMPADCVALIDADDR, PDWNC_SPARE); // AHB address for auxadc valid bit
    DRV_WriteReg32(TEMPADCVOLTADDR, PDWNC_SPARE);

    DRV_WriteReg32(TEMPRDCTRL, 0x0); // read valid & voltage are at the same register
    DRV_WriteReg32(TEMPADCVALIDMASK, 0x0000002C);           // indicate where the valid bit is (the 12th bit is valid bit and 1 is valid)
    DRV_WriteReg32(TEMPADCVOLTAGESHIFT, 0x0);               // do not need to shift
    DRV_WriteReg32(TEMPADCWRITECTRL, 0x0); // enable auxadc mux & pnp write transaction 

    /**************hw reset********************/
    DRV_WriteReg32(TEMPPROTCTL, 0x20000);                   // !!! maximum of sensors
    DRV_WriteReg32(TEMPPROTTC, 0xAA);                       // !!! HOT
    /**************************************************/

  	DRV_WriteReg32(TEMPMONINT, 0xE000FFFF);                 // enable all interrupt except filter sense and immediate sense interrupt
	//	DRV_WriteReg32(TEMPMONINT, 0xFFFF);                 // enable all interrupt except filter sense and immediate sense interrupt
    DRV_WriteReg32(TEMPMONCTL0, 0x00000007);                // enable all sensing point (sensing point 2 is unused)

    count = 10;
    temp = DRV_Reg32(TEMPMSR0);
    while ((temp & 0x8000) == 0 && count--)
    {
        kal_sleep_task(1000);
        temp = DRV_Reg32(TEMPMSR0);
    }

    temp = DRV_Reg32(TEMPMSR0);
    UTIL_Printf("thermal_interrupt_trigger_test: (before testing), read TEMPMSR0 = 0x%x\n", (temp & 0x0FFF));
    temp = DRV_Reg32(TEMPMSR1);
    UTIL_Printf("thermal_interrupt_trigger_test: (before testing), read TEMPMSR1 = 0x%x\n", (temp & 0x0FFF));
    temp = DRV_Reg32(TEMPMSR2);
    UTIL_Printf("thermal_interrupt_trigger_test: (before testing), read TEMPMSR2 = 0x%x\n", (temp & 0x0FFF));

	UTIL_Printf("\n\n");
	//high temperature
	UTIL_Printf("===========================================\n");
	UTIL_Printf("thermal_interrupt_trigger_test: high temp=0x14\n");
	UTIL_Printf("===========================================\n");
    temp = 0x14;                                            // set to very hot for pnp0
    DRV_WriteReg32(TEMPADCEN, (0x00001000 + temp));        // set sensor voltage and sensor valid

    count = 20;
    while (thermal_intr_flag != true && count--)
    {
        kal_sleep_task(10000);
    }
    if (count == 0)
    {
        UTIL_Printf("thermal_interrupt_trigger_test: can not read temp.\n");
    }

    Print_Thermal_Reg();
    
    count = 10;
    temp = DRV_Reg32(TEMPMSR0);
    while ((temp & 0x8000) == 0 && count--)
        temp = DRV_Reg32(TEMPMSR0);

    if ((temp & 0x0FFF) != 0x14) // first filter valid should be equal to 0x14
    {
        UTIL_Printf("thermal_interrupt_trigger_test: fail, read TEMPMSR0 = 0x%x\n", (temp & 0x0FFF));
        return -1;
    }
    else
        UTIL_Printf("thermal_interrupt_trigger_test: pass, read TEMPMSR0 = 0x%x\n", (temp & 0x0FFF));



    count = 10;
    temp = DRV_Reg32(TEMPMSR1);
    while ((temp & 0x8000) == 0 && count--)
        temp = DRV_Reg32(TEMPMSR1);

    if ((temp & 0x0FFF) != 0x14) // first filter valid should be equal to 0x14
    {
        UTIL_Printf("thermal_interrupt_trigger_test: fail, read TEMPMSR1 = 0x%x\n", (temp & 0x0FFF));
        return -1;
    }
    else
        UTIL_Printf("thermal_interrupt_trigger_test: pass, read TEMPMSR1 = 0x%x\n", (temp & 0x0FFF));

    count = 10;
    temp = DRV_Reg32(TEMPMSR2);
    if ((temp & 0x0FFF) != 0x14) // first filter valid should be equal to 0x14
    {
        UTIL_Printf("thermal_interrupt_trigger_test: fail(0x14), read TEMPMSR2 = 0x%x\n", (temp & 0x0FFF));
        return -1;
    }
    else
        UTIL_Printf("thermal_interrupt_trigger_test: pass(0x14), read TEMPMSR2 = 0x%x\n", (temp & 0x0FFF));


    UTIL_Printf("\n");

    thermal_intr_flag = false;

	UTIL_Printf("\n\n");
	//Low temperature
	UTIL_Printf("===========================================\n");
	UTIL_Printf("thermal_interrupt_trigger_test: Low temp=0x234\n");
	UTIL_Printf("===========================================\n");

    temp = 0x234;                                           // set to very cold for pnp0
    DRV_WriteReg32(TEMPADCEN, (0x00001000 + temp));        // set sensor voltage and sensor valid
    //DRV_WriteReg32(TEMPSPARE3, temp);                       // sensor voltage

    UTIL_Printf("kal_sleep_task(10000)\n");
    count = 1;
    //while (thermal_intr_flag != true && count--) 
    while (count--)
    {
        UTIL_Printf("%d, 0x%8X\n", 100-count, DRV_Reg32(TEMPMSR0));
        kal_sleep_task(10000);
    }
    UTIL_Printf("kal_sleep_task(10000)\n");

    count = 10;
    temp = DRV_Reg32(TEMPMSR0);
    while ((temp & 0x8000) == 0 && count--)
        temp = DRV_Reg32(TEMPMSR0);
    if ((temp & 0x0FFF) != 0x234) // first filter valid should be equal to 0x234
    {
        UTIL_Printf("thermal_interrupt_trigger_test: fail, read TEMPMSR0 = 0x%x\n", (temp & 0x0FFF));
        return -1;
    }
    else
        UTIL_Printf("thermal_interrupt_trigger_test: pass, read TEMPMSR0 = 0x%x\n", (temp & 0x0FFF));

    count = 10;
    temp = DRV_Reg32(TEMPMSR1);
    while ((temp & 0x8000) == 0 && count--)
        temp = DRV_Reg32(TEMPMSR1);

    if ((temp & 0x0FFF) != 0x234) // first filter valid should be equal to 0x234
    {
        UTIL_Printf("thermal_interrupt_trigger_test: fail, read TEMPMSR1 = 0x%x\n", (temp & 0x0FFF));
        return -1;
    }
    else
        UTIL_Printf("thermal_interrupt_trigger_test: pass, read TEMPMSR1 = 0x%x\n", (temp & 0x0FFF));

    while ((temp & 0x8000) == 0 && count--)
        temp = DRV_Reg32(TEMPMSR2);
    temp = DRV_Reg32(TEMPMSR2);
    if ((temp & 0x0FFF) != 0x234) // first filter valid should be equal to 0x234
    {
        UTIL_Printf("thermal_interrupt_trigger_test: fail(0x234), read TEMPMSR2 = 0x%x\n", (temp & 0x0FFF));
        return -1;
    }
    else
        UTIL_Printf("thermal_interrupt_trigger_test: pass(0x234), read TEMPMSR2 = 0x%x\n", (temp & 0x0FFF));


    UTIL_Printf("\n");

    thermal_intr_flag = false;

	UTIL_Printf("\n\n");
	//High offset temperature
	UTIL_Printf("===========================================\n");
	UTIL_Printf("thermal_interrupt_trigger_test: High offset temp=0xB0\n");
	UTIL_Printf("===========================================\n");

    temp = 0xB0;                                           // set to very cold for pnp0
    DRV_WriteReg32(TEMPADCEN, (0x00001000 + temp));        // set sensor voltage and sensor valid
    //DRV_WriteReg32(TEMPSPARE3, temp);                       // sensor voltage

    count = 1;
    //while (thermal_intr_flag != true && count--)
    while (count--)
    {
        UTIL_Printf("%d, 0x%8X\n", 100-count, DRV_Reg32(TEMPMSR0));
        kal_sleep_task(10000);
    }

    count = 10;
    temp = DRV_Reg32(TEMPMSR0);
    while ((temp & 0x8000) == 0 && count--)
        temp = DRV_Reg32(TEMPMSR0);

    if ((temp & 0x0FFF) != 0xB0) // first filter valid should be equal to 0xB0
    {
        UTIL_Printf("thermal_interrupt_trigger_test: fail, read TEMPMSR0 = 0x%x\n", (temp & 0x0FFF));
        return -1;
    }
    else
        UTIL_Printf("thermal_interrupt_trigger_test: pass, read TEMPMSR0 = 0x%x\n", (temp & 0x0FFF));

    count = 10;
    temp = DRV_Reg32(TEMPMSR1);
    while ((temp & 0x8000) == 0 && count--)
        temp = DRV_Reg32(TEMPMSR1);

    if ((temp & 0x0FFF) != 0xB0) // first filter valid should be equal to 0xB0
    {
        UTIL_Printf("thermal_interrupt_trigger_test: fail, read TEMPMSR1 = 0x%x\n", (temp & 0x0FFF));
        return -1;
    }
    else
        UTIL_Printf("thermal_interrupt_trigger_test: pass, read TEMPMSR1 = 0x%x\n", (temp & 0x0FFF));

    count = 10;
    temp = DRV_Reg32(TEMPMSR2);
    while ((temp & 0x8000) == 0 && count--)
        temp = DRV_Reg32(TEMPMSR2);

    temp = DRV_Reg32(TEMPMSR2);
    if ((temp & 0x0FFF) != 0xB0) // first filter valid should be equal to 0xB0
    {
        UTIL_Printf("thermal_interrupt_trigger_test: fail(0xB0), read TEMPMSR2 = 0x%x\n", (temp & 0x0FFF));
        return -1;
    }
    else
        UTIL_Printf("thermal_interrupt_trigger_test: pass(0xB0), read TEMPMSR2 = 0x%x\n", (temp & 0x0FFF));


    UTIL_Printf("\n");

    thermal_intr_flag = false;

    return 0;
}

int thermal_interrupt_occurrance_test_ok(kal_uint32 times)
{
	UINT32 temp = 0, i = 0;
	
	 // enable all adc channel for HW mode 
	 DRV_WriteReg32(PDWNC_SRVCFG1, 0x8887FFFF); 			 // counting unit is 320 * 31.25us = 10ms
	 
	 // set TSADC hw mode
	 DRV_WriteReg32(PDWNC_SRVCFG0, 0x01);
	
	
	 DRV_WriteReg32(TEMPMONCTL1, 0x000000CC); // 3FF);	  // counting unit is 1024 / 66M = 15.5us
	 DRV_WriteReg32(TEMPMONCTL2, 0x02190219); // 219);	 // sensing interval is 537 * 15.5us = 8.3235ms
	 DRV_WriteReg32(TEMPAHBPOLL,  0x00000001);
	 DRV_WriteReg32(TEMPAHBTO,	  0xFFFFFFFF);
	
	 DRV_WriteReg32(TEMPMONIDET0, times);
	 DRV_WriteReg32(TEMPMONIDET1, times);
	 DRV_WriteReg32(TEMPMONIDET2, times);
	
 #if 1
	 // DRV_WriteReg32(TEMPMONIDET3, sensor);
 #endif
	
	 DRV_WriteReg32(TEMPH2NTHRE,  0x000000FF);	 // set hot to normal threshold
	 DRV_WriteReg32(TEMPHTHRE,	  0x000000AA);	 // set hot threshold
	 DRV_WriteReg32(TEMPCTHRE,	  0x00000133);	 // set cold threshold	 
	 DRV_WriteReg32(TEMPOFFSETH,  0x000000BB);	 // set high offset threshold	 
	 DRV_WriteReg32(TEMPOFFSETL,  0x00000111);	 // set low offset threshold	 
	
	 DRV_WriteReg32(TEMPMSRCTL0,  0x00000000);	 // DRV_WriteReg32(TEMPMSRCTL0,  0x000006DB);	 
	
	 DRV_WriteReg32(TEMPADCPNP0, 0x172);
	 DRV_WriteReg32(TEMPADCPNP1, 0x1B2);
	 DRV_WriteReg32(TEMPADCPNP2, 0x1F2);
	
	 DRV_WriteReg32(TEMPADCMUX, 0x800); 	 
	 DRV_WriteReg32(TEMPADCEXT, 0x800); 	  
	 DRV_WriteReg32(TEMPADCEN, 0x10FF);
	
	 DRV_WriteReg32(TEMPPNPMUXADDR, 0xf00243fc);
	 DRV_WriteReg32(TEMPADCENADDR, 0xf0024170);
	 DRV_WriteReg32(TEMPADCVALIDADDR,0xf0024170);
	 DRV_WriteReg32(TEMPADCVOLTADDR, 0xf0024170);
	
	 DRV_WriteReg32(TEMPRDCTRL, 0x1);			  
	 DRV_WriteReg32(TEMPADCVALIDMASK, 0x0000002C);
	 DRV_WriteReg32(TEMPADCVOLTAGESHIFT, 0x0);
	 DRV_WriteReg32(TEMPADCWRITECTRL, 0x00000000);
	
	 DRV_WriteReg32(TEMPMONINT, 	  0x0000FFFF); // 0x00000000); // 0xFFFFFFFF);
	 DRV_WriteReg32(TEMPMONCTL0,	  0x00000007); // 0x0000000F
	
	 temp = 0x14;											// set to very hot for pnp0
	 DRV_WriteReg32(TEMPADCEN, (0x00001000 + temp));		// set sensor voltage and sensor valid
	
	 while ((temp & 0x0FFF) != 0x14)
	 {
		 kal_sleep_task(10000);
		 temp = DRV_Reg32(TEMPMSR0);
	 }
	 
	 for (i = 0; i < 5; i++)
		 kal_sleep_task(10000);
	
	 thermal_intr_flag = false;
		 
	 while (thermal_intr_flag != true)
	 {
		 temp = 0x100;											  // set to normal for pnp0
		 DRV_WriteReg32(TEMPADCEN, (0x00001000 + temp));		// set sensor voltage and sensor valid		  
	
		 temp = DRV_Reg32(TEMPMSR0);		
		 while ((temp & 0x0FFF) != 0x100)
		 {
			 kal_sleep_task(10000);
			 temp = DRV_Reg32(TEMPMSR0);
		 }
		 
		 temp = 0x14;											// set to very hot for pnp0
		 DRV_WriteReg32(TEMPADCEN, (0x00001000 + temp));		// set sensor voltage and sensor valid
	
		 temp = DRV_Reg32(TEMPMSR0);		
		 while ((temp & 0x0FFF) != 0x14)
		 {
			 kal_sleep_task(10000);
			 temp = DRV_Reg32(TEMPMSR0);
		 }
	 }
	 
	 thermal_intr_flag = false;
	  
	 temp = DRV_Reg32(TEMPMSR0);
	 while ((temp & 0x0FFF) != 0x14)
	 {
		 kal_sleep_task(10000);
		 temp = DRV_Reg32(TEMPMSR0);
	 }
	 if ((temp & 0x0FFF) != 0x14) // first filter valid should be equal to 0x14
		 UTIL_Printf("thermal_interrupt_occurrance_test: fail, read TEMPMSR0(0x14) = 0x%x\n", (temp & 0x0FFF));
	 else
		 UTIL_Printf("thermal_interrupt_occurrance_test: pass, read TEMPMSR0(0x14) = 0x%x\n", (temp & 0x0FFF));
	 
	 temp = DRV_Reg32(TEMPMSR1);
	 if ((temp & 0x0FFF) != 0x14) // first filter valid should be equal to 0x14
		 UTIL_Printf("thermal_interrupt_occurrance_test: fail, read TEMPMSR1(0x14) = 0x%x\n", (temp & 0x0FFF));
	 else
		 UTIL_Printf("thermal_interrupt_occurrance_test: pass, read TEMPMSR1(0x14) = 0x%x\n", (temp & 0x0FFF));
	
	 temp = DRV_Reg32(TEMPMSR2);
	 if ((temp & 0x0FFF) != 0x14) // first filter valid should be equal to 0x14
		 UTIL_Printf("thermal_interrupt_occurrance_test: fail, read TEMPMSR2(0x14) = 0x%x\n", (temp & 0x0FFF));
	 else
		 UTIL_Printf("thermal_interrupt_occurrance_test: pass, read TEMPMSR2(0x14) = 0x%x\n", (temp & 0x0FFF));
	
	 UTIL_Printf("\n");
	
	 temp = 0x234;											 // set to very cold for pnp0
	 DRV_WriteReg32(TEMPADCEN, (0x00001000 + temp));		// set sensor voltage and sensor valid
	 
	 while ((temp & 0x0FFF) != 0x234)
	 {
		 kal_sleep_task(10000);
		 temp = DRV_Reg32(TEMPMSR0);
	 }
	 
	 for (i = 0; i < 5; i++)
		 kal_sleep_task(10000);
	
	 thermal_intr_flag = false;
		 
	 while (thermal_intr_flag != true)
	 {
		 temp = 0x100;											  // set to normal for pnp0
		 DRV_WriteReg32(TEMPADCEN, (0x00001000 + temp));		// set sensor voltage and sensor valid
		 
		 temp = DRV_Reg32(TEMPMSR0);		
		 while ((temp & 0x0FFF) != 0x100)
		 {
			 kal_sleep_task(10000);
			 temp = DRV_Reg32(TEMPMSR0);
		 }
		 
		 temp = 0x234;											 // set to very cold for pnp0
		 DRV_WriteReg32(TEMPADCEN, (0x00001000 + temp));		// set sensor voltage and sensor valid
	
		 temp = DRV_Reg32(TEMPMSR0);		
		 while ((temp & 0x0FFF) != 0x234)
		 {
			 kal_sleep_task(10000);
			 temp = DRV_Reg32(TEMPMSR0);
		 }
	 }
	 
	 thermal_intr_flag = false;
	  
	 temp = DRV_Reg32(TEMPMSR0);
	 while ((temp & 0x0FFF) != 0x234)
	 {
		 kal_sleep_task(10000);
		 temp = DRV_Reg32(TEMPMSR0);
	 }
	 if ((temp & 0x0FFF) != 0x234) // first filter valid should be equal to 0x234
		 UTIL_Printf("thermal_interrupt_occurrance_test: fail, read TEMPMSR0(0x234) = 0x%x\n", (temp & 0x0FFF));
	 else
		 UTIL_Printf("thermal_interrupt_occurrance_test: pass, read TEMPMSR0(0x234) = 0x%x\n", (temp & 0x0FFF));
	 
	 temp = DRV_Reg32(TEMPMSR1);
	 if ((temp & 0x0FFF) != 0x234) // first filter valid should be equal to 0x234
		 UTIL_Printf("thermal_interrupt_occurrance_test: fail, read TEMPMSR1(0x234) = 0x%x\n", (temp & 0x0FFF));
	 else
		 UTIL_Printf("thermal_interrupt_occurrance_test: pass, read TEMPMSR1(0x234) = 0x%x\n", (temp & 0x0FFF));
	
	 temp = DRV_Reg32(TEMPMSR2);
	 if ((temp & 0x0FFF) != 0x234) // first filter valid should be equal to 0x234
		 UTIL_Printf("thermal_interrupt_occurrance_test: fail, read TEMPMSR2(0x234) = 0x%x\n", (temp & 0x0FFF));
	 else
		 UTIL_Printf("thermal_interrupt_occurrance_test: pass, read TEMPMSR2(0x234) = 0x%x\n", (temp & 0x0FFF));
	
	 UTIL_Printf("thermal_interrupt_occurrance_test,  end \n");

    return 0;
}

int thermal_interrupt_occurrance_test(unsigned int sensor)
{
 	UINT32 temp = 0, i = 0;
	
	 // enable all adc channel for HW mode 
	 //DRV_WriteReg32(PDWNC_SRVCFG1, 0x8887FFFF); 			 // counting unit is 320 * 31.25us = 10ms
	 
	 // set TSADC hw mode
	 //DRV_WriteReg32(PDWNC_SRVCFG0, 0x01);
	
	 // 31.25us * 4 = 125 us
	 DRV_WriteReg32(TEMPMONCTL1, 0x00000004); // 3FF);	  // counting unit is 1024 / 66M = 15.5us
	 // 125 us * 80(0x50) = 10ms
	 DRV_WriteReg32(TEMPMONCTL2, 0x00500050); // 219);	 // sensing interval is 537 * 15.5us = 8.3235ms

	 // 
	 DRV_WriteReg32(TEMPAHBPOLL,  0x00000040);
	 DRV_WriteReg32(TEMPAHBTO,	  0x00000FFF);
	
	 DRV_WriteReg32(TEMPMONIDET0, sensor);
	 DRV_WriteReg32(TEMPMONIDET1, sensor);
	 DRV_WriteReg32(TEMPMONIDET2, sensor);
 	#if 0
	 DRV_WriteReg32(TEMPMONIDET3, sensor);
 	#endif
	
	 DRV_WriteReg32(TEMPHTHRE,	  0x000000AA);	 // set hot threshold
 	 DRV_WriteReg32(TEMPOFFSETH,  0x000000BB);	 // set high offset threshold
	 DRV_WriteReg32(TEMPH2NTHRE,  0x000000FF);	 // set hot to normal threshold
	 DRV_WriteReg32(TEMPOFFSETL,  0x00000111);	 // set low offset threshold	
	 DRV_WriteReg32(TEMPCTHRE,	  0x00000133);	 // set cold threshold	
	
	 DRV_WriteReg32(TEMPMSRCTL0,  0x00000000);	 // DRV_WriteReg32(TEMPMSRCTL0,  0x000006DB);	 
	
	 DRV_WriteReg32(TEMPADCPNP0, 0x172);
	 DRV_WriteReg32(TEMPADCPNP1, 0x1B2);
	 DRV_WriteReg32(TEMPADCPNP2, 0x1F2);
	
	 DRV_WriteReg32(TEMPADCMUX, 0x800); 	 
	 DRV_WriteReg32(TEMPADCEXT, 0x800); 	  
	 DRV_WriteReg32(TEMPADCEN, 0x10FF);
	
	 DRV_WriteReg32(TEMPPNPMUXADDR, 0xf00243fc);
	 DRV_WriteReg32(TEMPADCENADDR, 0xf0024170);
	 DRV_WriteReg32(TEMPADCVALIDADDR,0xf0024170);
	 DRV_WriteReg32(TEMPADCVOLTADDR, 0xf0024170);
	
	 DRV_WriteReg32(TEMPRDCTRL, 0x1);			  
	 DRV_WriteReg32(TEMPADCVALIDMASK, 0x0000002C);
	 DRV_WriteReg32(TEMPADCVOLTAGESHIFT, 0x0);
	 DRV_WriteReg32(TEMPADCWRITECTRL, 0x00000000);
	
	 DRV_WriteReg32(TEMPMONINT, 0x0000FFFF);
	 DRV_WriteReg32(TEMPMONCTL0, 0x00000007); // 0x0000000F
	
	 temp = 0x14;											// set to very hot for pnp0
	 DRV_WriteReg32(TEMPADCEN, (0x00001000 + temp));		// set sensor voltage and sensor valid

	 UTIL_Printf("thermal_interrupt_occurrance_test: 3 \n");
	
	 while ((temp & 0x0FFF) != 0x14)
	 {
		 kal_sleep_task(10000);
		 temp = DRV_Reg32(TEMPMSR0);
	 }
	 
	 for (i = 0; i < 5; i++)
		 kal_sleep_task(10000);
	
	 thermal_intr_flag = false;
		 
	 while (thermal_intr_flag != true)
	 {
		 temp = 0x100;											  // set to normal for pnp0
		 DRV_WriteReg32(TEMPADCEN, (0x00001000 + temp));		// set sensor voltage and sensor valid		  
	
		 temp = DRV_Reg32(TEMPMSR0);		
		 while ((temp & 0x0FFF) != 0x100)
		 {
			 kal_sleep_task(10000);
			 temp = DRV_Reg32(TEMPMSR0);
		 }
			 
		 temp = 0x14;											// set to very hot for pnp0
		 DRV_WriteReg32(TEMPADCEN, (0x00001000 + temp));		// set sensor voltage and sensor valid
	
		 temp = DRV_Reg32(TEMPMSR0);		
		 while ((temp & 0x0FFF) != 0x14)
		 {
			 kal_sleep_task(10000);
			 temp = DRV_Reg32(TEMPMSR0);
		 }

	 }
	 
	 thermal_intr_flag = false;
	  
	 temp = DRV_Reg32(TEMPMSR0);
	 while ((temp & 0x0FFF) != 0x14)
	 {
		 kal_sleep_task(10000);
		 temp = DRV_Reg32(TEMPMSR0);
	 }
	 if ((temp & 0x0FFF) != 0x14) // first filter valid should be equal to 0x14
		 UTIL_Printf("thermal_interrupt_occurrance_test: fail, read TEMPMSR0(0x14) = 0x%x\n", (temp & 0x0FFF));
	 else
		 UTIL_Printf("thermal_interrupt_occurrance_test: pass, read TEMPMSR0(0x14) = 0x%x\n", (temp & 0x0FFF));
	 
	 temp = DRV_Reg32(TEMPMSR1);
	 if ((temp & 0x0FFF) != 0x14) // first filter valid should be equal to 0x14
		 UTIL_Printf("thermal_interrupt_occurrance_test: fail, read TEMPMSR1(0x14) = 0x%x\n", (temp & 0x0FFF));
	 else
		 UTIL_Printf("thermal_interrupt_occurrance_test: pass, read TEMPMSR1(0x14) = 0x%x\n", (temp & 0x0FFF));
	
	 temp = DRV_Reg32(TEMPMSR2);
	 if ((temp & 0x0FFF) != 0x14) // first filter valid should be equal to 0x14
		 UTIL_Printf("thermal_interrupt_occurrance_test: fail, read TEMPMSR2(0x14) = 0x%x\n", (temp & 0x0FFF));
	 else
		 UTIL_Printf("thermal_interrupt_occurrance_test: pass, read TEMPMSR2(0x14) = 0x%x\n", (temp & 0x0FFF));
	
	 UTIL_Printf("\n");
	 for (i = 0; i < 10; i++)
		 kal_sleep_task(10000);
	 UTIL_Printf("\n"); 

#if 1	 
	 temp = 0x234;											 // set to very cold for pnp0
	 DRV_WriteReg32(TEMPADCEN, (0x00001000 + temp));		// set sensor voltage and sensor valid
	 
	 while ((temp & 0x0FFF) != 0x234)
	 {
		 kal_sleep_task(10000);
		 temp = DRV_Reg32(TEMPMSR0);
	 }
	 
	 for (i = 0; i < 5; i++)
		 kal_sleep_task(10000);
	
	 thermal_intr_flag = false;
		 
	 while (thermal_intr_flag != true)
	 {
		 temp = 0x100;											  // set to normal for pnp0
		 DRV_WriteReg32(TEMPADCEN, (0x00001000 + temp));		// set sensor voltage and sensor valid
		 
		 temp = DRV_Reg32(TEMPMSR0);		
		 while ((temp & 0x0FFF) != 0x100)
		 {
			 kal_sleep_task(10000);
			 temp = DRV_Reg32(TEMPMSR0);
		 }
		 
		 temp = 0x234;											 // set to very cold for pnp0
		 DRV_WriteReg32(TEMPADCEN, (0x00001000 + temp));		// set sensor voltage and sensor valid
	
		 temp = DRV_Reg32(TEMPMSR0);		
		 while ((temp & 0x0FFF) != 0x234)
		 {
			 kal_sleep_task(10000);
			 temp = DRV_Reg32(TEMPMSR0);
		 }
	 }
	 
	 thermal_intr_flag = false;
	  
	 temp = DRV_Reg32(TEMPMSR0);
	 while ((temp & 0x0FFF) != 0x234)
	 {
		 kal_sleep_task(10000);
		 temp = DRV_Reg32(TEMPMSR0);
	 }
	 if ((temp & 0x0FFF) != 0x234) // first filter valid should be equal to 0x234
		 UTIL_Printf("thermal_interrupt_occurrance_test: fail, read TEMPMSR0(0x234) = 0x%x\n", (temp & 0x0FFF));
	 else
		 UTIL_Printf("thermal_interrupt_occurrance_test: pass, read TEMPMSR0(0x234) = 0x%x\n", (temp & 0x0FFF));
	 
	 temp = DRV_Reg32(TEMPMSR1);
	 if ((temp & 0x0FFF) != 0x234) // first filter valid should be equal to 0x234
		 UTIL_Printf("thermal_interrupt_occurrance_test: fail, read TEMPMSR1(0x234) = 0x%x\n", (temp & 0x0FFF));
	 else
		 UTIL_Printf("thermal_interrupt_occurrance_test: pass, read TEMPMSR1(0x234) = 0x%x\n", (temp & 0x0FFF));
	
	 temp = DRV_Reg32(TEMPMSR2);
	 if ((temp & 0x0FFF) != 0x234) // first filter valid should be equal to 0x234
		 UTIL_Printf("thermal_interrupt_occurrance_test: fail, read TEMPMSR2(0x234) = 0x%x\n", (temp & 0x0FFF));
	 else
		 UTIL_Printf("thermal_interrupt_occurrance_test: pass, read TEMPMSR2(0x234) = 0x%x\n", (temp & 0x0FFF));
	#endif
	
	 UTIL_Printf("thermal_interrupt_occurrance_test,  end \n");

    return 0;
}
	

int thermal_sensing_filter_option_test(kal_uint16 option)
{
    UINT32 temp = 0, i = 0, count = 0;
	UINT8 avecnt=0;
    
    thermal_real_test = false;

	if((option&0x00000007)==0) avecnt = 1;
	else if((option&0x00000007)==1) avecnt = 2;	
	else if((option&0x00000007)==2) avecnt = 4;
	else if((option&0x00000007)==3) avecnt = 6;
	else if((option&0x00000007)==4) avecnt = 10;
	else if((option&0x00000007)==5) avecnt = 18;

	DRV_WriteReg32(TEMPMONCTL1, 0x00000004); // 3FF);    // counting unit is 1024 / 66M = 15.5us
	DRV_WriteReg32(TEMPMONCTL2, 0x00500050); // 219);	// sensing interval is 537 * 15.5us = 8.3235ms
	DRV_WriteReg32(TEMPAHBPOLL,  0x00000040);
	DRV_WriteReg32(TEMPAHBTO,    0x00000FFF);
	
    DRV_WriteReg32(TEMPMONIDET0, 0x00000000);               // times=1 for interrupt occurrance of sensing point 0
    DRV_WriteReg32(TEMPMONIDET1, 0x00000000);               // times=1 for interrupt occurrance of sensing point 1
    DRV_WriteReg32(TEMPMONIDET2, 0x00000000);               // times=1 for interrupt occurrance of sensing point 2

	DRV_WriteReg32(TEMPHTHRE, 0x000000AA);                  // set hot threshold
    DRV_WriteReg32(TEMPOFFSETH, 0x000000BB);                // set high offset threshold
    DRV_WriteReg32(TEMPH2NTHRE, 0x000000FF);                // set hot to normal threshold
    DRV_WriteReg32(TEMPOFFSETL, 0x00000111);                // set low offset threshold
    DRV_WriteReg32(TEMPCTHRE, 0x00000133);                  // set cold threshold

	DRV_WriteReg32(TEMPMSRCTL0, option);                    // !!! temperature measurement sampling control

	DRV_WriteReg32(TEMPADCPNP0, 0x172);
	DRV_WriteReg32(TEMPADCPNP1, 0x1B2);
	DRV_WriteReg32(TEMPADCPNP2, 0x1F2);

	DRV_WriteReg32(TEMPADCMUX, 0x11);		
	DRV_WriteReg32(TEMPADCEXT, 0x22);		 
	DRV_WriteReg32(TEMPADCEXT1, 0x33);				

	temp = 0x14;                                            // set to very hot for pnp0
    DRV_WriteReg32(TEMPADCEN, (0x00001000 + temp));        // set sensor voltage and sensor valid
    
	DRV_WriteReg32(TEMPPNPMUXADDR, 0xf00243fc);
	DRV_WriteReg32(TEMPADCENADDR, 0xf0024170);
	DRV_WriteReg32(TEMPADCVALIDADDR,0xf0024170);
	DRV_WriteReg32(TEMPADCVOLTADDR, 0xf0024170);
	
	DRV_WriteReg32(TEMPRDCTRL, 0x0); // 0x1);			 
	DRV_WriteReg32(TEMPADCVALIDMASK, 0x0000002C);
	DRV_WriteReg32(TEMPADCVOLTAGESHIFT, 0x0);
	DRV_WriteReg32(TEMPADCWRITECTRL, 0x00000000);

	//enable all filter interrupt 
	DRV_WriteReg32(TEMPMONINT,		 0x10380000);
	
	DRV_WriteReg32(TEMPMONCTL0, 	 0x00000007);

	// 219h x 0CCh x 100h x 16.6ns = 465,535,180.82 ns =0.4 sec

    for (i = 0; i < 5; i++)
        kal_sleep_task(10000);
    
    count = 10;
    while (thermal_intr_flag != true && count--)
    {
        kal_sleep_task(10000);
    }
    
    temp = DRV_Reg32(TEMPMSR0);
    while ((temp & 0x0FFF) != 0x14)
    {
        kal_sleep_task(10000);
        temp = DRV_Reg32(TEMPMSR0);
    }
		
    for (i = 0; i < (1 * 2 * avecnt * 2); i++)
		kal_sleep_task(10000);

    if ((temp & 0x0FFF) != 0x14) // first filter valid should be equal to 0x14
        UTIL_Printf("thermal_sensing_filter_option_test: fail, read TEMPMSR0(0x14) = 0x%x [%d]\n", (temp & 0x0FFF),(temp & 0x80000));
    else
        UTIL_Printf("thermal_sensing_filter_option_test: 1, pass, read TEMPMSR0(0x14) = 0x%x [%d]\n", (temp & 0x0FFF),(temp & 0x80000));
    
    temp = DRV_Reg32(TEMPMSR1);
    if ((temp & 0x0FFF) != 0x14) // first filter valid should be equal to 0x14
        UTIL_Printf("thermal_sensing_filter_option_test: fail, read TEMPMSR1(0x14) = 0x%x [%d]\n", (temp & 0x0FFF),(temp & 0x80000));
    else
        UTIL_Printf("thermal_sensing_filter_option_test: pass, read TEMPMSR1(0x14) = 0x%x [%d]\n", (temp & 0x0FFF),(temp & 0x80000));

    temp = DRV_Reg32(TEMPMSR2);
    if ((temp & 0x0FFF) != 0x14) // first filter valid should be equal to 0x14
        UTIL_Printf("thermal_sensing_filter_option_test: fail, read TEMPMSR2(0x14) = 0x%x [%d]\n", (temp & 0x0FFF),(temp & 0x80000));
    else
        UTIL_Printf("thermal_sensing_filter_option_test: pass, read TEMPMSR2(0x14) = 0x%x [%d]\n", (temp & 0x0FFF),(temp & 0x80000));
       
    UTIL_Printf("\n");
        
    thermal_intr_flag = false;
    

	DRV_WriteReg32(TEMPMONCTL0, 	 0x00000000);
    temp = 0x234;                                           // set to very cold for pnp0
    DRV_WriteReg32(TEMPADCEN, (0x00001000 + temp));        // set sensor voltage and sensor valid
	DRV_WriteReg32(TEMPMONCTL0, 	 0x00000007);
	
    for (i = 0; i < 5; i++)
        kal_sleep_task(10000);
    
    count = 10;
    while (thermal_intr_flag != true && count--)
    {
        kal_sleep_task(10000);
    }  
    
    temp = DRV_Reg32(TEMPMSR0);
    while ((temp & 0x0FFF) != 0x234)
    {
        kal_sleep_task(10000);
        temp = DRV_Reg32(TEMPMSR0);
    }

    for (i = 0; i < (1 * 2 * avecnt * 2); i++)
		kal_sleep_task(10000);

	
    if ((temp & 0x0FFF) != 0x234) // first filter valid should be equal to 0x234
        UTIL_Printf("thermal_sensing_filter_option_test: fail, read TEMPMSR0(0x234) = 0x%x [%d]\n", (temp & 0x0FFF),(temp & 0x80000));
    else
        UTIL_Printf("thermal_sensing_filter_option_test: pass, read TEMPMSR0(0x234) = 0x%x [%d]\n", (temp & 0x0FFF),(temp & 0x80000));
    
    temp = DRV_Reg32(TEMPMSR1);
    if ((temp & 0x0FFF) != 0x234) // first filter valid should be equal to 0x234
        UTIL_Printf("thermal_sensing_filter_option_test: fail, read TEMPMSR1(0x234) = 0x%x [%d]\n", (temp & 0x0FFF),(temp & 0x80000));
    else
        UTIL_Printf("thermal_sensing_filter_option_test: pass, read TEMPMSR1(0x234) = 0x%x [%d]\n", (temp & 0x0FFF),(temp & 0x80000));

    temp = DRV_Reg32(TEMPMSR2);
    if ((temp & 0x0FFF) != 0x234) // first filter valid should be equal to 0x234
        UTIL_Printf("thermal_sensing_filter_option_test: fail, read TEMPMSR2(0x234) = 0x%x [%d]\n", (temp & 0x0FFF),(temp & 0x80000));
    else
        UTIL_Printf("thermal_sensing_filter_option_test: pass, read TEMPMSR2(0x234) = 0x%x [%d]\n", (temp & 0x0FFF),(temp & 0x80000));
        
    UTIL_Printf("\n");
    
    thermal_intr_flag = false;

	DRV_WriteReg32(TEMPMONCTL0, 	 0x00000000);    
    temp = 0xB0;                                           // set to very cold for pnp0
    DRV_WriteReg32(TEMPADCEN, (0x00001000 + temp));        // set sensor voltage and sensor valid
	DRV_WriteReg32(TEMPMONCTL0, 	 0x00000007);    
    
    for (i = 0; i < 5; i++)
        kal_sleep_task(10000);
    
    count = 10;
    while (thermal_intr_flag != true && count--)
    {
        kal_sleep_task(10000);
    }  
    
    temp = DRV_Reg32(TEMPMSR0);
 
    while ((temp & 0x0FFF) != 0xB0)
    {
        kal_sleep_task(10000);
        temp = DRV_Reg32(TEMPMSR0);
    }

    for (i = 0; i < (1 * 2 * avecnt * 2); i++)
		kal_sleep_task(10000);

	
    if ((temp & 0x0FFF) != 0xB0) // first filter valid should be equal to 0xB0
        UTIL_Printf("thermal_sensing_filter_option_test: fail, read TEMPMSR0(0xB0) = 0x%x\n", (temp & 0x0FFF));
    else
        UTIL_Printf("thermal_sensing_filter_option_test: pass, read TEMPMSR0(0xB0) = 0x%x\n", (temp & 0x0FFF));
    
    temp = DRV_Reg32(TEMPMSR1);
    if ((temp & 0x0FFF) != 0xB0) // first filter valid should be equal to 0xB0
        UTIL_Printf("thermal_sensing_filter_option_test: fail, read TEMPMSR1(0xB0) = 0x%x\n", (temp & 0x0FFF));
    else
        UTIL_Printf("thermal_sensing_filter_option_test: pass, read TEMPMSR1(0xB0) = 0x%x\n", (temp & 0x0FFF));

    temp = DRV_Reg32(TEMPMSR2);
    if ((temp & 0x0FFF) != 0xB0) // first filter valid should be equal to 0xB0
        UTIL_Printf("thermal_sensing_filter_option_test: fail, read TEMPMSR2(0xB0) = 0x%x\n", (temp & 0x0FFF));
    else
        UTIL_Printf("thermal_sensing_filter_option_test: pass, read TEMPMSR2(0xB0) = 0x%x\n", (temp & 0x0FFF));
        
    UTIL_Printf("thermal_sensing_filter_option_test: end \n");
    
    thermal_intr_flag = false;
	
    return 0;
}

static void thermal_ahb_timeout_test_init(void);
int thermal_immediate_measurement_test(void)
{
	UINT32 temp = 0, times = 0, i = 0;

	UTIL_Printf("thermal_immediate_measurement_test: get real temp \n");
	
	thermal_real_test = true;

	// enable all adc channel for HW mode 
	DRV_WriteReg32(PDWNC_SRVCFG1, 0x8887FFFF);				// counting unit is 320 * 31.25us = 10ms
	
	// set TSADC hw mode
	DRV_WriteReg32(PDWNC_SRVCFG0, 0x01);

#if 1
	// set counting unit
	DRV_WriteReg32(TEMPMONCTL1, 0x00000004);				// counting unit is 320 * 31.25us = 10ms
	// set sensing interval
	DRV_WriteReg32(TEMPMONCTL2, 0x00500050);				// sensing interval is 200 * 10ms = 2000ms
	// polling interval to check if temperature sense is ready
	DRV_WriteReg32(TEMPAHBPOLL, 0x000000FF);				// polling interval to check if temperature sense is ready
	// polling time for exceed
	DRV_WriteReg32(TEMPAHBTO, 0xFFFFFFFF);					// exceed this polling time, IRQ would be inserted
	// all interrupt occurrance times is 1
	DRV_WriteReg32(TEMPMONIDET0, 0x00000000);				// times for interrupt occurrance
	DRV_WriteReg32(TEMPMONIDET1, 0x00000000);				// times for interrupt occurrance
	// set all threshold
	DRV_WriteReg32(TEMPHTHRE, 0x00000100);					// set hot threshold
	DRV_WriteReg32(TEMPOFFSETH, 0x00000200);				// set high offset threshold
	DRV_WriteReg32(TEMPH2NTHRE, 0x00000500);				// set hot to normal threshold
	DRV_WriteReg32(TEMPOFFSETL, 0x0000A00); 			   // set low offset threshold
	DRV_WriteReg32(TEMPCTHRE, 0x00000B00);					// set cold threshold
#endif

	// 
	DRV_WriteReg32(TEMPADCPNP0, 0x00);
	DRV_WriteReg32(TEMPADCPNP1, 0x01);
	DRV_WriteReg32(TEMPADCPNP2, 0x02);
	DRV_WriteReg32(TEMPADCPNP3, 0x03);
	
	DRV_WriteReg32(TEMPADCMUX, 0x11);
	DRV_WriteReg32(TEMPADCEXT, 0x22);
	DRV_WriteReg32(TEMPADCEXT1, 0x33);
	
	DRV_WriteReg32(TEMPADCEN, 0x01);

	DRV_WriteReg32(TEMPPNPMUXADDR, 0xf00243fc);
	DRV_WriteReg32(TEMPADCENADDR,  0xf0024620); 
	DRV_WriteReg32(TEMPADCVALIDADDR, 0xf0024634);
	DRV_WriteReg32(TEMPADCVOLTADDR, 0xf00243d4);

	DRV_WriteReg32(TEMPRDCTRL, 0x1);					  // read valid & voltage are at the same register
	DRV_WriteReg32(TEMPADCVALIDMASK, 0x00000022);			// indicate where the valid bit is (the 12th bit is valid bit and 1 is valid)
	DRV_WriteReg32(TEMPADCVOLTAGESHIFT, 0x0);				// do not need to shift
	DRV_WriteReg32(TEMPADCWRITECTRL, 0x1); 

	// set sample mode
	DRV_WriteReg32(TEMPMSRCTL0, 0x00000000);
	
	UTIL_Printf("read TEMPMONINTSTS (1) = 0x%x \n", DRV_Reg32(TEMPMONINTSTS));

	// enable perio mear on sensor 0,1,2,3
	//DRV_WriteReg32(TEMPMONCTL0, 0x0000000F);

	//for (j = 0; j <= 10; j++)
	{
		times = 4;
		while (times--)
		{
			// enable imme interrupt for sensing point 0,1,2,3
			DRV_WriteReg32(TEMPMONINT, 0x8070000); 

			// enable immediate mode for sensing point 0,1,2,3
			DRV_WriteReg32(TEMPMSRCTL1, 0x000000270);	 

			for (i=0;i<10;i++)
			{
				kal_sleep_task(10000);
			}
			
			UTIL_Printf("read TEMPMONINTSTS (2) = 0x%x \n", DRV_Reg32(TEMPMONINTSTS));

			// sensor point 0
			temp = DRV_Reg32(TEMPIMMD0); 
			while ((temp & 0x8000) == 0)
				temp = DRV_Reg32(TEMPIMMD0);
			UTIL_Printf("read TEMPIMMD0 = 0x%x \n", (temp));  

			#if 0
			temp = DRV_Reg32(TEMPMSR0); 
			while ((temp & 0x8000) == 0)
				temp = DRV_Reg32(TEMPMSR0);
			UTIL_Printf("read TEMPMSR0 = 0x%x \n", (temp));
			#endif
			
			// sensor point 1
			temp = DRV_Reg32(TEMPIMMD1); 
			while ((temp & 0x8000) == 0)
				temp = DRV_Reg32(TEMPIMMD1);

			UTIL_Printf("read TEMPIMMD1 = 0x%x \n", (temp));  

			#if 0
			temp = DRV_Reg32(TEMPMSR1); 
			while ((temp & 0x8000) == 0)
				temp = DRV_Reg32(TEMPMSR1);
			UTIL_Printf("read TEMPMSR1 = 0x%x \n", (temp));
			#endif

			// sensor point 2
			temp = DRV_Reg32(TEMPIMMD2); 
			while ((temp & 0x8000) == 0)
				temp = DRV_Reg32(TEMPIMMD2);

			UTIL_Printf("read TEMPIMMD2 = 0x%x \n", (temp));  

			#if 0
			temp = DRV_Reg32(TEMPMSR2); 
			while ((temp & 0x8000) == 0)
				temp = DRV_Reg32(TEMPMSR2);
			UTIL_Printf("read TEMPMSR2 = 0x%x \n", (temp));
			#endif

			// sensor point 3
			temp = DRV_Reg32(TEMPIMMD3); 
			while ((temp & 0x8000) == 0)
				temp = DRV_Reg32(TEMPIMMD3);

			UTIL_Printf("read TEMPIMMD3 = 0x%x \n", (temp));  

			#if 0
			temp = DRV_Reg32(TEMPMSR3); 
			while ((temp & 0x8000) == 0)
				temp = DRV_Reg32(TEMPMSR3);
			UTIL_Printf("read TEMPMSR3 = 0x%x \n", (temp));
			#endif
			
			DRV_WriteReg32(TEMPMSRCTL1, 0x00000000);				// disable immediate mode for sensing point 0

			for (i=0;i<10;i++)
			{
				kal_sleep_task(10000);
			}
			UTIL_Printf("read TEMPMONINTSTS (3) = 0x%x \n", DRV_Reg32(TEMPMONINTSTS));
		}
	}
	UTIL_Printf("thermal_immediate_measurement_test: end \n");

	return 0;

}

static void thermal_ahb_timeout_test_init(void)
{

	DRV_WriteReg32(TEMPMONCTL1, 0x00000004); 				// 3FF);	 // counting unit is 1024 / 66M = 15.5us
	DRV_WriteReg32(TEMPMONCTL2, 0x00500050); 				// 219);	// sensing interval is 537 * 15.5us = 8.3235ms
	DRV_WriteReg32(TEMPAHBPOLL,  0x00000040);
	DRV_WriteReg32(TEMPAHBTO,	 0x00000FFF);

	DRV_WriteReg32(TEMPMONIDET0, 0x00000000);				// times=1 for interrupt occurrance of sensing point 0
	DRV_WriteReg32(TEMPMONIDET1, 0x00000000);				// times=1 for interrupt occurrance of sensing point 1
	DRV_WriteReg32(TEMPMONIDET2, 0x00000000);				// times=1 for interrupt occurrance of sensing point 2

	DRV_WriteReg32(TEMPHTHRE, 0x000000AA);					// set hot threshold
	DRV_WriteReg32(TEMPOFFSETH, 0x000000BB);				// set high offset threshold
	DRV_WriteReg32(TEMPH2NTHRE, 0x000000FF);				// set hot to normal threshold
	DRV_WriteReg32(TEMPOFFSETL, 0x00000111);				// set low offset threshold
	DRV_WriteReg32(TEMPCTHRE, 0x00000133);					// set cold threshold
	DRV_WriteReg32(TEMPMSRCTL0, 0x0000000); 				// temperature measurement sampling control (one sampling)

	DRV_WriteReg32(TEMPADCPNP0, 0x172);
	DRV_WriteReg32(TEMPADCPNP1, 0x1B2);
	DRV_WriteReg32(TEMPADCPNP2, 0x1F2);

	DRV_WriteReg32(TEMPADCMUX, 0x11);		
	DRV_WriteReg32(TEMPADCEXT, 0x22);		 
	DRV_WriteReg32(TEMPADCEXT1, 0x33);				

	// set a fake temperature value 0x0FF
	DRV_WriteReg32(TEMPADCEN, 0x10FF);

	// for fake temperature
	DRV_WriteReg32(TEMPPNPMUXADDR, 0xf00243fc);
	DRV_WriteReg32(TEMPADCENADDR, 0xf0024170);
	DRV_WriteReg32(TEMPADCVALIDADDR,0xf0024170);
	DRV_WriteReg32(TEMPADCVOLTADDR, 0xf0024170);

	DRV_WriteReg32(TEMPRDCTRL, 0x1);
	DRV_WriteReg32(TEMPADCVALIDMASK, 0x0000002C);
	DRV_WriteReg32(TEMPADCVOLTAGESHIFT, 0x0);
	DRV_WriteReg32(TEMPADCWRITECTRL, 0x00000000);

	// just enable AHB timeout interrupt
	DRV_WriteReg32(TEMPMONINT, 0x00008000);

	return;

}


#define PTP_TEST_TEMP 5
UINT16 u2TestTemp1[PTP_TEST_TEMP]={0x14, 0x15, 0x13, 0xFF, 0xFF};

int thermal_ahb_timeout_test(kal_uint32 u4poling, kal_uint32 u4timeout)
{
    UINT32 temp = 0, count = 0, i = 0;
	UINT8 tempIdx=0;
	UINT16 tempIdx_temp=0;
	UINT32 poling, timeout;

	for(tempIdx=0; tempIdx<4; tempIdx++)
	{	
		if ((tempIdx == 0) || (tempIdx == 2)) // AHB Timeout, status machine hang up, need reset again
		{
			thermal_controller_Reset();
			thermal_ahb_timeout_test_init();
		}
		
		DRV_WriteReg32(TEMPMONCTL0, 	 0x00000000);
	
	    tempIdx_temp = u2TestTemp1[tempIdx];
	    DRV_WriteReg32(TEMPADCEN, (0x00001000 + tempIdx_temp));        // set sensor voltage and sensor valid	    

		if ((tempIdx == 0) || (tempIdx == 2))
		{
			poling = 0x00000001;
			timeout = 0x00000333;
		}
		else
		{
			poling = u4poling;
			timeout = u4timeout;
		}

		UTIL_Printf("poling=%d, timeout=%d\n", poling, timeout);
		DRV_WriteReg32(TEMPAHBPOLL, poling);					  // !!! polling interval to check if temperature sense is ready
		DRV_WriteReg32(TEMPAHBTO, timeout);					  // !!! exceed this polling time, IRQ would be inserted			

		DRV_WriteReg32(TEMPMONCTL0, 	 0x00000007);

	    for (i = 0; i < 10; i++)
	        kal_sleep_task(10000);    

      	
		count = 0;
		do
		{
		    count++;
			temp = DRV_Reg32(TEMPMSR0);
			UTIL_Printf("[Stage%d]Count=%d, thermal_intr_flag = %d temp = %x stemp = %x , status= %x \n",tempIdx, count, thermal_intr_flag, temp, tempIdx_temp,DRV_Reg32(TEMPMSRCTL1)&0xFFFFFFFE);
			kal_sleep_task(10000);
			if((temp & 0x0FFF) == tempIdx_temp)
			{
				UTIL_Printf("temp(%d) == tempIdx_temp(%d)\n",temp, tempIdx_temp);
				break;	
			}	
		}while((!thermal_intr_flag));
	    
	    temp = DRV_Reg32(TEMPMSR0);
		
	    if ((temp & 0x0FFF) != tempIdx_temp) // first filter valid should be equal to 0x14
	    {
	    	if(poling < timeout)
	        UTIL_Printf("[poling < timeout][%d]thermal_ahb_timeout_test: fail, read TEMPMSR0(0x%3x) = 0x%x\n", tempIdx,tempIdx_temp,(temp & 0x0FFF));
			else		
	        UTIL_Printf("[poling > timeout][%d]thermal_ahb_timeout_test: pass, read TEMPMSR0(0x%3x) = 0x%x\n", tempIdx,tempIdx_temp,(temp & 0x0FFF));		
	    }
	    else
	    {
	    	if(poling < timeout)    
	        UTIL_Printf("[poling < timeout][%d]thermal_ahb_timeout_test: pass, read TEMPMSR0(0x%3x) = 0x%x\n", tempIdx,tempIdx_temp,(temp & 0x0FFF));
			else
	        UTIL_Printf("[poling > timeout][%d]thermal_ahb_timeout_test: fail, read TEMPMSR0(0x%3x) = 0x%x\n", tempIdx,tempIdx_temp,(temp & 0x0FFF));			
	    }	

    	thermal_intr_flag = false;    
	}   
    
    return 0;
}


int thermal_first_hot_interrupt_test(kal_bool first_hot_en)
{
    UINT32 temp = 0, i = 0, count = 0;

    thermal_real_test = false;

    //hwEnableClock(MT65XX_PDN_PERI_THERM, "Thermal");

    DRV_WriteReg32(TEMPMONCTL1, 0x00000004);                // counting unit is 320 * 31.25us = 10ms

    if (first_hot_en)
    {
        temp = DRV_Reg32(TEMPMONCTL1) | 0x0000E000;             // only the first time when normal to hot threshold
        DRV_WriteReg32(TEMPMONCTL1, temp);
    }

    DRV_WriteReg32(TEMPMONCTL2, 0x000000C8);                // sensing interval is 200 * 10ms = 2000ms
    DRV_WriteReg32(TEMPAHBPOLL, 0x0000000F);                // polling interval to check if temperature sense is ready
    DRV_WriteReg32(TEMPAHBTO, 0x000000FF);                  // exceed this polling time, IRQ would be inserted
    DRV_WriteReg32(TEMPMONIDET0, 0x00000000);               // times for interrupt occurrance
    DRV_WriteReg32(TEMPMONIDET1, 0x00000000);               // times for interrupt occurrance
    DRV_WriteReg32(TEMPHTHRE, 0x000000AA);                  // set hot threshold
    DRV_WriteReg32(TEMPOFFSETH, 0x000000BB);                // set high offset threshold
    DRV_WriteReg32(TEMPH2NTHRE, 0x000000FF);                // set hot to normal threshold
    DRV_WriteReg32(TEMPOFFSETL, 0x00000111);                // set low offset threshold
    DRV_WriteReg32(TEMPCTHRE, 0x00000133);                  // set cold threshold
    DRV_WriteReg32(TEMPMSRCTL0, 0x0000000);                 // temperature measurement sampling control

    DRV_WriteReg32(TEMPADCPNP0, 0x0);                       // this value will be stored to TEMPPNPMUXADDR (TEMPSPARE0) automatically by hw
    DRV_WriteReg32(TEMPADCPNP1, 0x1);                       // this value will be stored to TEMPPNPMUXADDR (TEMPSPARE0) automatically by hw
    DRV_WriteReg32(TEMPADCMUX, 0x11);                       // this value will be stored to TEMPPNPMUXADDR (TEMPSPARE0) automatically by hw
    DRV_WriteReg32(TEMPADCEN, 0x1);                         // AHB value for auxadc enable
    DRV_WriteReg32(TEMPPNPMUXADDR, (UINT32) TEMPSPARE0);    // AHB address for pnp sensor mux selection
    DRV_WriteReg32(TEMPADCMUXADDR, (UINT32) TEMPSPARE0);    // AHB address for auxadc mux selection
    DRV_WriteReg32(TEMPADCENADDR, (UINT32) TEMPSPARE1);     // AHB address for auxadc enable
    DRV_WriteReg32(TEMPADCVALIDADDR, (UINT32) TEMPSPARE2);  // AHB address for auxadc valid bit
    DRV_WriteReg32(TEMPADCVOLTADDR, (UINT32) TEMPSPARE2);   // AHB address for auxadc voltage output
    DRV_WriteReg32(TEMPRDCTRL, 0x0);                        // read valid & voltage are at the same register
    DRV_WriteReg32(TEMPADCVALIDMASK, 0x0000002C);           // indicate where the valid bit is (the 12th bit is valid bit and 1 is valid)
    DRV_WriteReg32(TEMPADCVOLTAGESHIFT, 0x0);               // do not need to shift
    DRV_WriteReg32(TEMPADCWRITECTRL, 0x3);                  // enable auxadc mux & pnp write transaction

    DRV_WriteReg32(TEMPMONINT, 0x0000FFFF);                 // enable all interrupt
    DRV_WriteReg32(TEMPMONCTL0, 0x00000003);                // enable all sensing point (sensing point 2 is unused)


    temp = 0x14;                                            // set to very hot for pnp0
    DRV_WriteReg32(TEMPSPARE2, (0x00001000 + temp));        // set sensor voltage and sensor valid
    //DRV_WriteReg32(TEMPSPARE3, temp);                       // sensor voltage

    for (i = 0; i < 5; i++)
        kal_sleep_task(10000);

    count = 10;
    while (thermal_intr_flag != true && count--)
    {
        kal_sleep_task(10000);
    }

    temp = DRV_Reg32(TEMPMSR0);
    while ((temp & 0x8000) == 0)
        temp = DRV_Reg32(TEMPMSR0);
    if ((temp & 0x0FFF) != 0x14) // first filter valid should be equal to 0x14
        UTIL_Printf("thermal_first_hot_interrupt_test: fail, read TEMPMSR0 = 0x%x\n", (temp & 0x0FFF));
    else
        UTIL_Printf("thermal_first_hot_interrupt_test: pass, read TEMPMSR0 = 0x%x\n", (temp & 0x0FFF));

    temp = DRV_Reg32(TEMPMSR1);
    while ((temp & 0x8000) == 0)
        temp = DRV_Reg32(TEMPMSR1);
    if ((temp & 0x0FFF) != 0x14) // first filter valid should be equal to 0x14
        UTIL_Printf("thermal_first_hot_interrupt_test: fail, read TEMPMSR1 = 0x%x\n", (temp & 0x0FFF));
    else
        UTIL_Printf("thermal_first_hot_interrupt_test: pass, read TEMPMSR1 = 0x%x\n", (temp & 0x0FFF));

    UTIL_Printf("\n");

    thermal_intr_flag = false;


    temp = 0xCC;                                            // set to normal for pnp0
    DRV_WriteReg32(TEMPSPARE2, (0x00001000 + temp));        // set sensor voltage and sensor valid
    //DRV_WriteReg32(TEMPSPARE3, temp);                       // sensor voltage

    for (i = 0; i < 5; i++)
        kal_sleep_task(10000);

    count = 10;
    while (thermal_intr_flag != true && count--)
    {
        kal_sleep_task(10000);
    }

    temp = DRV_Reg32(TEMPMSR0);
    while ((temp & 0x8000) == 0)
        temp = DRV_Reg32(TEMPMSR0);
    if ((temp & 0x0FFF) != 0xCC) // first filter valid should be equal to 0xCC
        UTIL_Printf("thermal_first_hot_interrupt_test: fail, read TEMPMSR0 = 0x%x\n", (temp & 0x0FFF));
    else
        UTIL_Printf("thermal_first_hot_interrupt_test: pass, read TEMPMSR0 = 0x%x\n", (temp & 0x0FFF));

    temp = DRV_Reg32(TEMPMSR1);
    while ((temp & 0x8000) == 0)
        temp = DRV_Reg32(TEMPMSR1);
    if ((temp & 0x0FFF) != 0xCC) // first filter valid should be equal to 0xCC
        UTIL_Printf("thermal_first_hot_interrupt_test: fail, read TEMPMSR1 = 0x%x\n", (temp & 0x0FFF));
    else
        UTIL_Printf("thermal_first_hot_interrupt_test: pass, read TEMPMSR1 = 0x%x\n", (temp & 0x0FFF));

    UTIL_Printf("\n");

    thermal_intr_flag = false;


    temp = 0x14;                                            // set to very hot for pnp0
    DRV_WriteReg32(TEMPSPARE2, (0x00001000 + temp));        // set sensor voltage and sensor valid
    //DRV_WriteReg32(TEMPSPARE3, temp);                       // sensor voltage

    for (i = 0; i < 5; i++)
        kal_sleep_task(10000);

    count = 10;
    while (thermal_intr_flag != true && count--)
    {
        kal_sleep_task(10000);
    }

    temp = DRV_Reg32(TEMPMSR0);
    while ((temp & 0x8000) == 0)
        temp = DRV_Reg32(TEMPMSR0);
    if ((temp & 0x0FFF) != 0x14) // first filter valid should be equal to 0x14
        UTIL_Printf("thermal_first_hot_interrupt_test: fail, read TEMPMSR0 = 0x%x\n", (temp & 0x0FFF));
    else
        UTIL_Printf("thermal_first_hot_interrupt_test: pass, read TEMPMSR0 = 0x%x\n", (temp & 0x0FFF));

    temp = DRV_Reg32(TEMPMSR1);
    while ((temp & 0x8000) == 0)
        temp = DRV_Reg32(TEMPMSR1);
    if ((temp & 0x0FFF) != 0x14) // first filter valid should be equal to 0x14
        UTIL_Printf("thermal_first_hot_interrupt_test: fail, read TEMPMSR1 = 0x%x\n", (temp & 0x0FFF));
    else
        UTIL_Printf("thermal_first_hot_interrupt_test: pass, read TEMPMSR1 = 0x%x\n", (temp & 0x0FFF));

    UTIL_Printf("\n");

    thermal_intr_flag = false;


    temp = 0xCC;                                            // set to normal for pnp0
    DRV_WriteReg32(TEMPSPARE2, (0x00001000 + temp));        // set sensor voltage and sensor valid
    //DRV_WriteReg32(TEMPSPARE3, temp);                       // sensor voltage

    for (i = 0; i < 5; i++)
        kal_sleep_task(10000);

    count = 10;
    while (thermal_intr_flag != true && count--)
    {
        kal_sleep_task(10000);
    }

    temp = DRV_Reg32(TEMPMSR0);
    while ((temp & 0x8000) == 0)
        temp = DRV_Reg32(TEMPMSR0);
    if ((temp & 0x0FFF) != 0xCC) // first filter valid should be equal to 0xCC
        UTIL_Printf("thermal_first_hot_interrupt_test: fail, read TEMPMSR0 = 0x%x\n", (temp & 0x0FFF));
    else
        UTIL_Printf("thermal_first_hot_interrupt_test: pass, read TEMPMSR0 = 0x%x\n", (temp & 0x0FFF));

    temp = DRV_Reg32(TEMPMSR1);
    while ((temp & 0x8000) == 0)
        temp = DRV_Reg32(TEMPMSR1);
    if ((temp & 0x0FFF) != 0xCC) // first filter valid should be equal to 0xCC
        UTIL_Printf("thermal_first_hot_interrupt_test: fail, read TEMPMSR1 = 0x%x\n", (temp & 0x0FFF));
    else
        UTIL_Printf("thermal_first_hot_interrupt_test: pass, read TEMPMSR1 = 0x%x\n", (temp & 0x0FFF));

    UTIL_Printf("\n");

    thermal_intr_flag = false;


    temp = 0x14;                                            // set to very hot for pnp0
    DRV_WriteReg32(TEMPSPARE2, (0x00001000 + temp));        // set sensor voltage and sensor valid
    //DRV_WriteReg32(TEMPSPARE3, temp);                       // sensor voltage

    for (i = 0; i < 5; i++)
        kal_sleep_task(10000);

    count = 10;
    while (thermal_intr_flag != true && count--)
    {
        kal_sleep_task(10000);
    }

    temp = DRV_Reg32(TEMPMSR0);
    while ((temp & 0x8000) == 0)
        temp = DRV_Reg32(TEMPMSR0);
    if ((temp & 0x0FFF) != 0x14) // first filter valid should be equal to 0x14
        UTIL_Printf("thermal_first_hot_interrupt_test: fail, read TEMPMSR0 = 0x%x\n", (temp & 0x0FFF));
    else
        UTIL_Printf("thermal_first_hot_interrupt_test: pass, read TEMPMSR0 = 0x%x\n", (temp & 0x0FFF));

    temp = DRV_Reg32(TEMPMSR1);
    while ((temp & 0x8000) == 0)
        temp = DRV_Reg32(TEMPMSR1);
    if ((temp & 0x0FFF) != 0x14) // first filter valid should be equal to 0x14
        UTIL_Printf("thermal_first_hot_interrupt_test: fail, read TEMPMSR1 = 0x%x\n", (temp & 0x0FFF));
    else
        UTIL_Printf("thermal_first_hot_interrupt_test: pass, read TEMPMSR1 = 0x%x\n", (temp & 0x0FFF));

    UTIL_Printf("\n");

    thermal_intr_flag = false;

    return 0;
}

int thermal_filter_sample_interval_test(kal_uint32 filt_interval, kal_uint32 sen_interval)
{
    UINT32 temp = 0, i = 0, count = 0;
	UINT32 time_delay;
    
    thermal_real_test = false;

	// set counting unit
    DRV_WriteReg32(TEMPMONCTL1, 0x00000055);                // counting unit is 330 / 66M = 5us

	// set different interval will make different filter interrupt occerance interval
    temp = filt_interval << 16 | sen_interval;
    DRV_WriteReg32(TEMPMONCTL2, temp);                      // !!! sensing interval is sen_interval * 5us

	time_delay = (17 * 0x14A * sen_interval * 2 * 2)/1000;

	DRV_WriteReg32(TEMPAHBPOLL,  0x00000040);
	DRV_WriteReg32(TEMPAHBTO,    0x00000FFF);

    DRV_WriteReg32(TEMPMONIDET0, 0x00000000);               // times=1 for interrupt occurrance of sensing point 0
    DRV_WriteReg32(TEMPMONIDET1, 0x00000000);               // times=1 for interrupt occurrance of sensing point 1
    DRV_WriteReg32(TEMPMONIDET2, 0x00000000);               // times=1 for interrupt occurrance of sensing point 2

	DRV_WriteReg32(TEMPHTHRE, 0x000000AA);                  // set hot threshold
    DRV_WriteReg32(TEMPOFFSETH, 0x000000BB);                // set high offset threshold
    DRV_WriteReg32(TEMPH2NTHRE, 0x000000FF);                // set hot to normal threshold
    DRV_WriteReg32(TEMPOFFSETL, 0x00000111);                // set low offset threshold
    DRV_WriteReg32(TEMPCTHRE, 0x00000133);                  // set cold threshold

	DRV_WriteReg32(TEMPMSRCTL0, 0x00000092);                 // temperature measurement sampling control

	DRV_WriteReg32(TEMPADCPNP0, 0x172);
	DRV_WriteReg32(TEMPADCPNP1, 0x1B2);
	DRV_WriteReg32(TEMPADCPNP2, 0x1F2);

	DRV_WriteReg32(TEMPADCMUX, 0x11);		
	DRV_WriteReg32(TEMPADCEXT, 0x22);		 
	DRV_WriteReg32(TEMPADCEXT1, 0x33);				

	temp = 0x14;											// set to very hot for pnp0
	DRV_WriteReg32(TEMPADCEN, (0x00001000 + temp)); 	   // set sensor voltage and sensor valid
	
	DRV_WriteReg32(TEMPPNPMUXADDR, 0xf00243fc);
	DRV_WriteReg32(TEMPADCENADDR, 0xf0024170);
	DRV_WriteReg32(TEMPADCVALIDADDR,0xf0024170);
	DRV_WriteReg32(TEMPADCVOLTADDR, 0xf0024170);
	
	DRV_WriteReg32(TEMPRDCTRL, 0x1);			 
	DRV_WriteReg32(TEMPADCVALIDMASK, 0x0000002C);
	DRV_WriteReg32(TEMPADCVOLTAGESHIFT, 0x0);
	DRV_WriteReg32(TEMPADCWRITECTRL, 0x00000000);
	
    // open all filter interrupt for 0,1,2,3
	DRV_WriteReg32(TEMPMONINT,		 0x10380000);

	DRV_WriteReg32(TEMPMONCTL0, 	 0x00000007);
		
    for (i = 0; i < 5; i++)
        kal_sleep_task(10000);

	thermal_intr_flag = false;
	
    count = 10;
    while (thermal_intr_flag != true && count--)
    {
        kal_sleep_task(10000);
    }

    count = 0;    
    temp = DRV_Reg32(TEMPMSR0);
    while ((temp & 0x0FFF) != 0x14)
    {
        kal_sleep_task(10000);
        temp = DRV_Reg32(TEMPMSR0);
    }

	UTIL_Printf("thermal_filter_sample_interval_test: time_delay = %d \n", (time_delay));
    for (i = 0; i < time_delay; i++)
		kal_sleep_task(10000);
	
    if ((temp & 0x0FFF) != 0x14) // first filter valid should be equal to 0x14
        UTIL_Printf("thermal_filter_sample_interval_test: fail, read TEMPMSR0(0x14) = 0x%x\n", (temp & 0x0FFF));
    else
        UTIL_Printf("thermal_filter_sample_interval_test: pass, read TEMPMSR0(0x14) = 0x%x\n", (temp & 0x0FFF));
    
    count = 0;    
    temp = DRV_Reg32(TEMPMSR1);
    while ((temp & 0x0FFF) != 0x14)
    {
        kal_sleep_task(10000);
        temp = DRV_Reg32(TEMPMSR1);
    }
    if ((temp & 0x0FFF) != 0x14) // first filter valid should be equal to 0x14
        UTIL_Printf("thermal_filter_sample_interval_test: fail, read TEMPMSR1(0x14) = 0x%x\n", (temp & 0x0FFF));
    else
        UTIL_Printf("thermal_filter_sample_interval_test: pass, read TEMPMSR1(0x14) = 0x%x\n", (temp & 0x0FFF));

    count = 0;    
    temp = DRV_Reg32(TEMPMSR2);
    while ((temp & 0x0FFF) != 0x14)
    {
        kal_sleep_task(10000);
        temp = DRV_Reg32(TEMPMSR2);
    }
    if ((temp & 0x0FFF) != 0x14) // first filter valid should be equal to 0x14
        UTIL_Printf("thermal_filter_sample_interval_test: fail, read TEMPMSR2(0x14) = 0x%x\n", (temp & 0x0FFF));
    else
        UTIL_Printf("thermal_filter_sample_interval_test: pass, read TEMPMSR2(0x14) = 0x%x\n", (temp & 0x0FFF));
        
    UTIL_Printf("\n");
        
    thermal_intr_flag = false;
    
    DRV_WriteReg32(TEMPMONCTL0, 	 0x00000000);
    temp = 0x234;                                           // set to very cold for pnp0
    DRV_WriteReg32(TEMPADCEN, (0x00001000 + temp));        // set sensor voltage and sensor valid
    DRV_WriteReg32(TEMPMONCTL0, 	 0x00000007); 
    
    for (i = 0; i < 5; i++)
        kal_sleep_task(10000);
    
    count = 10;
    while (thermal_intr_flag != true && count--)
    {
        kal_sleep_task(10000);
    }  

    count = 0;    
    temp = DRV_Reg32(TEMPMSR0);
    while ((temp & 0x0FFF) != 0x234)
    {
        kal_sleep_task(10000);
        temp = DRV_Reg32(TEMPMSR0);
    }
    for (i = 0; i < time_delay; i++)
		kal_sleep_task(10000);
	
    if ((temp & 0x0FFF) != 0x234)
        UTIL_Printf("thermal_filter_sample_interval_test: fail, read TEMPMSR0(0x234) = 0x%x\n", (temp & 0x0FFF));
    else
        UTIL_Printf("thermal_filter_sample_interval_test: pass, read TEMPMSR0(0x234) = 0x%x\n", (temp & 0x0FFF));
    
    temp = DRV_Reg32(TEMPMSR1);
    if ((temp & 0x0FFF) != 0x234)
        UTIL_Printf("thermal_filter_sample_interval_test: fail, read TEMPMSR1(0x234) = 0x%x\n", (temp & 0x0FFF));
    else
        UTIL_Printf("thermal_filter_sample_interval_test: pass, read TEMPMSR1(0x234) = 0x%x\n", (temp & 0x0FFF));

    temp = DRV_Reg32(TEMPMSR2);
    if ((temp & 0x0FFF) != 0x234)
        UTIL_Printf("thermal_filter_sample_interval_test: fail, read TEMPMSR2(0x234) = 0x%x\n", (temp & 0x0FFF));
    else
        UTIL_Printf("thermal_filter_sample_interval_test: pass, read TEMPMSR2(0x234) = 0x%x\n", (temp & 0x0FFF));
        
    UTIL_Printf("\n");
    
    thermal_intr_flag = false;
    
    DRV_WriteReg32(TEMPMONCTL0, 	 0x00000000); 
    temp = 0xB0;                                           // set to very cold for pnp0
    DRV_WriteReg32(TEMPADCEN, (0x00001000 + temp));        // set sensor voltage and sensor valid
    DRV_WriteReg32(TEMPMONCTL0, 	 0x00000007); 
    
    for (i = 0; i < 5; i++)
        kal_sleep_task(10000);
    
    count = 10;
    while (thermal_intr_flag != true && count--)
    {
        kal_sleep_task(10000);
    }  

    count = 0;    
    temp = DRV_Reg32(TEMPMSR0);
    while ((temp & 0x0FFF) != 0xB0)
    {
        kal_sleep_task(10000);
        temp = DRV_Reg32(TEMPMSR0);
    }
	
    for (i = 0; i < time_delay; i++)
		kal_sleep_task(10000);
	
    if ((temp & 0x0FFF) != 0xB0) // first filter valid should be equal to 0xB0
        UTIL_Printf("thermal_filter_sample_interval_test: fail, read TEMPMSR0(0xB0) = 0x%x\n", (temp & 0x0FFF));
    else
        UTIL_Printf("thermal_filter_sample_interval_test: pass, read TEMPMSR0(0xB0) = 0x%x\n", (temp & 0x0FFF));
    
    temp = DRV_Reg32(TEMPMSR1);
    if ((temp & 0x0FFF) != 0xB0) // first filter valid should be equal to 0xB0
        UTIL_Printf("thermal_filter_sample_interval_test: fail, read TEMPMSR1(0xB0) = 0x%x\n", (temp & 0x0FFF));
    else
        UTIL_Printf("thermal_filter_sample_interval_test: pass, read TEMPMSR1(0xB0) = 0x%x\n", (temp & 0x0FFF));

    temp = DRV_Reg32(TEMPMSR2);
    if ((temp & 0x0FFF) != 0xB0) // first filter valid should be equal to 0xB0
        UTIL_Printf("thermal_filter_sample_interval_test: fail, read TEMPMSR2(0xB0) = 0x%x\n", (temp & 0x0FFF));
    else
        UTIL_Printf("thermal_filter_sample_interval_test: pass, read TEMPMSR2(0xB0) = 0x%x\n", (temp & 0x0FFF));
        
    UTIL_Printf("\n");
   
	DRV_WriteReg32(TEMPMONCTL0, 	 0x00000000); 

    thermal_intr_flag = false;
    
    return 0;
}


int thermal_different_threshold_test(kal_uint32 hot_threshold, kal_uint32 high_offset_threshold, kal_uint32 h2n_threshold, kal_uint32 low_offset_threshold, kal_uint32 cold_threshold)
{
    UINT32 temp = 0, i = 0, count = 0;
    
    thermal_real_test = false;
	   
	DRV_WriteReg32(TEMPMONCTL1, 0x00000004); // 3FF);    // counting unit is 1024 / 66M = 15.5us
	DRV_WriteReg32(TEMPMONCTL2, 0x00500050); // 219);	// sensing interval is 537 * 15.5us = 8.3235ms
	DRV_WriteReg32(TEMPAHBPOLL,  0x00000040);
	DRV_WriteReg32(TEMPAHBTO,    0x00000FFF);

    DRV_WriteReg32(TEMPMONIDET0, 0x00000000);               // times=1 for interrupt occurrance of sensing point 0
    DRV_WriteReg32(TEMPMONIDET1, 0x00000000);               // times=1 for interrupt occurrance of sensing point 1
    DRV_WriteReg32(TEMPMONIDET2, 0x00000000);               // times=1 for interrupt occurrance of sensing point 2

	DRV_WriteReg32(TEMPHTHRE, hot_threshold);               // !!! set hot threshold
    DRV_WriteReg32(TEMPOFFSETH, high_offset_threshold);     // !!! set high offset threshold
    DRV_WriteReg32(TEMPH2NTHRE, h2n_threshold);             // !!! set hot to normal threshold
    DRV_WriteReg32(TEMPOFFSETL, low_offset_threshold);      // !!! set low offset threshold
    DRV_WriteReg32(TEMPCTHRE, cold_threshold);              // !!! set cold threshold

	DRV_WriteReg32(TEMPMSRCTL0, 0x0000000);                 // temperature measurement sampling control (one sampling)

	DRV_WriteReg32(TEMPADCPNP0, 0x172);
	DRV_WriteReg32(TEMPADCPNP1, 0x1B2);
	DRV_WriteReg32(TEMPADCPNP2, 0x1F2);

	DRV_WriteReg32(TEMPADCMUX, 0x11);		
	DRV_WriteReg32(TEMPADCEXT, 0x22);		 
	DRV_WriteReg32(TEMPADCEXT1, 0x33);				

	temp = 0x14;                                            // set to very hot for pnp0
    DRV_WriteReg32(TEMPADCEN, (0x00001000 + temp));        // set sensor voltage and sensor valid
	
	DRV_WriteReg32(TEMPPNPMUXADDR, 0xf00243fc);
	DRV_WriteReg32(TEMPADCENADDR, 0xf0024170);
	DRV_WriteReg32(TEMPADCVALIDADDR,0xf0024170);
	DRV_WriteReg32(TEMPADCVOLTADDR, 0xf0024170);
	
	DRV_WriteReg32(TEMPRDCTRL, 0x1);			 
	DRV_WriteReg32(TEMPADCVALIDMASK, 0x0000002C);
	DRV_WriteReg32(TEMPADCVOLTAGESHIFT, 0x0);
	DRV_WriteReg32(TEMPADCWRITECTRL, 0x00000000);

	// enable hot/high/h2n/low/clod and AHB timeout interrupt 
	DRV_WriteReg32(TEMPMONINT, 0x0000FFFF);
	DRV_WriteReg32(TEMPMONCTL0, 0x00000007);

	thermal_intr_flag = false;
	for (i = 0; i < 5; i++)
        kal_sleep_task(10000);
    
    count = 10;
    while (thermal_intr_flag != true && count--)
    {
        kal_sleep_task(10000);
    }
	
    temp = DRV_Reg32(TEMPMSR0);
    while ((temp & 0x0FFF) != 0x14)
    {
        kal_sleep_task(10000);
        temp = DRV_Reg32(TEMPMSR0);
    }
    for (i = 0; i < (10 * 2 * 1 * 2); i++)
		kal_sleep_task(1000);
	
    if ((temp & 0x0FFF) != 0x14) // first filter valid should be equal to 0x14
        UTIL_Printf("thermal_different_threshold_test: fail, read TEMPMSR0(0x14) = 0x%x\n", (temp & 0x0FFF));
    else
        UTIL_Printf("thermal_different_threshold_test: pass, read TEMPMSR0(0x14) = 0x%x\n", (temp & 0x0FFF));
    
    temp = DRV_Reg32(TEMPMSR1);
    if ((temp & 0x0FFF) != 0x14) // first filter valid should be equal to 0x14
        UTIL_Printf("thermal_different_threshold_test: fail, read TEMPMSR1(0x14) = 0x%x\n", (temp & 0x0FFF));
    else
        UTIL_Printf("thermal_different_threshold_test: pass, read TEMPMSR1(0x14) = 0x%x\n", (temp & 0x0FFF));

    temp = DRV_Reg32(TEMPMSR2);
    if ((temp & 0x0FFF) != 0x14) // first filter valid should be equal to 0x14
        UTIL_Printf("thermal_different_threshold_test: fail, read TEMPMSR2(0x14) = 0x%x\n", (temp & 0x0FFF));
    else
        UTIL_Printf("thermal_different_threshold_test: pass, read TEMPMSR2(0x14) = 0x%x\n", (temp & 0x0FFF));

	for (i = 0; i < 50; i++)
        kal_sleep_task(10000);
	
    UTIL_Printf("\n");
        
    thermal_intr_flag = false;
    
    temp = 0x234;                                           // set to very cold for pnp0
    DRV_WriteReg32(TEMPADCEN, (0x00001000 + temp));        // set sensor voltage and sensor valid
    
    for (i = 0; i < 50; i++)
        kal_sleep_task(10000);
    
	count = 10;
    while (thermal_intr_flag != true && count--)
    {
        kal_sleep_task(10000);
    }

    temp = DRV_Reg32(TEMPMSR0);
    while ((temp & 0x0FFF) != 0x234)
    {
        kal_sleep_task(10000);
        temp = DRV_Reg32(TEMPMSR0);
    }
    for (i = 0; i < (10 * 2 * 1 * 2); i++)
		kal_sleep_task(1000);
	
    if ((temp & 0x0FFF) != 0x234)
        UTIL_Printf("thermal_different_threshold_test: fail, read TEMPMSR0(0x234) = 0x%x\n", (temp & 0x0FFF));
    else
        UTIL_Printf("thermal_different_threshold_test: pass, read TEMPMSR0(0x234) = 0x%x\n", (temp & 0x0FFF));
    
    temp = DRV_Reg32(TEMPMSR1);
    if ((temp & 0x0FFF) != 0x234)
        UTIL_Printf("thermal_different_threshold_test: fail, read TEMPMSR1(0x234) = 0x%x\n", (temp & 0x0FFF));
    else
        UTIL_Printf("thermal_different_threshold_test: pass, read TEMPMSR1(0x234) = 0x%x\n", (temp & 0x0FFF));

    temp = DRV_Reg32(TEMPMSR2);
    if ((temp & 0x0FFF) != 0x234)
        UTIL_Printf("thermal_different_threshold_test: fail, read TEMPMSR2(0x234) = 0x%x\n", (temp & 0x0FFF));
    else
        UTIL_Printf("thermal_different_threshold_test: pass, read TEMPMSR2(0x234) = 0x%x\n", (temp & 0x0FFF));

	for (i = 0; i < 50; i++)
        kal_sleep_task(10000);
	
    UTIL_Printf("\n");
    
    thermal_intr_flag = false;
	
    temp = 0xB0;                                           // set to very cold for pnp0
    DRV_WriteReg32(TEMPADCEN, (0x00001000 + temp));        // set sensor voltage and sensor valid
    
    for (i = 0; i < 50; i++)
        kal_sleep_task(10000);
    
    count = 10;
    while (thermal_intr_flag != true && count--)
    {
        kal_sleep_task(10000);
    }
	
    temp = DRV_Reg32(TEMPMSR0);
    while ((temp & 0x0FFF) != 0xB0)
    {
        kal_sleep_task(10000);
        temp = DRV_Reg32(TEMPMSR0);
    }
	
    for (i = 0; i < 10; i++)
        kal_sleep_task(10000);
	
    for (i = 0; i < (10 * 2 * 1 * 2); i++)
		kal_sleep_task(1000);
	
    if ((temp & 0x0FFF) != 0xB0) // first filter valid should be equal to 0xB0
        UTIL_Printf("thermal_different_threshold_test: fail, read TEMPMSR0(0xB0) = 0x%x\n", (temp & 0x0FFF));
    else
        UTIL_Printf("thermal_different_threshold_test: pass, read TEMPMSR0(0xB0) = 0x%x\n", (temp & 0x0FFF));
    
    temp = DRV_Reg32(TEMPMSR1);
    if ((temp & 0x0FFF) != 0xB0) // first filter valid should be equal to 0xB0
        UTIL_Printf("thermal_different_threshold_test: fail, read TEMPMSR1(0xB0) = 0x%x\n", (temp & 0x0FFF));
    else
        UTIL_Printf("thermal_different_threshold_test: pass, read TEMPMSR1(0xB0) = 0x%x\n", (temp & 0x0FFF));

    temp = DRV_Reg32(TEMPMSR2);
    if ((temp & 0x0FFF) != 0xB0) // first filter valid should be equal to 0xB0
        UTIL_Printf("thermal_different_threshold_test: fail, read TEMPMSR2(0xB0) = 0x%x\n", (temp & 0x0FFF));
    else
        UTIL_Printf("thermal_different_threshold_test: pass, read TEMPMSR2(0xB0) = 0x%x\n", (temp & 0x0FFF));
        
    UTIL_Printf("\n");

    thermal_intr_flag = false;
    
    return 0;
}


int thermal_trigger_wdt_reset(void)
{
    UINT32 temp = 0, i = 0, count = 0;
    
    thermal_real_test = false;
	   
	DRV_WriteReg32(TEMPMONCTL1, 0x00000004); // 3FF);    // counting unit is 1024 / 66M = 15.5us
	DRV_WriteReg32(TEMPMONCTL2, 0x00500050); // 219);	// sensing interval is 537 * 15.5us = 8.3235ms
	DRV_WriteReg32(TEMPAHBPOLL,  0x00000040);
	DRV_WriteReg32(TEMPAHBTO,    0x00000FFF);

    DRV_WriteReg32(TEMPMONIDET0, 0x00000000);               // times=1 for interrupt occurrance of sensing point 0
    DRV_WriteReg32(TEMPMONIDET1, 0x00000000);               // times=1 for interrupt occurrance of sensing point 1
    DRV_WriteReg32(TEMPMONIDET2, 0x00000000);               // times=1 for interrupt occurrance of sensing point 2

	DRV_WriteReg32(TEMPHTHRE, 0x000000AA);               // !!! set hot threshold
    DRV_WriteReg32(TEMPOFFSETH, 0x000000BB);     // !!! set high offset threshold
    DRV_WriteReg32(TEMPH2NTHRE, 0x000000FF);             // !!! set hot to normal threshold
    DRV_WriteReg32(TEMPOFFSETL, 0x00000111);      // !!! set low offset threshold
    DRV_WriteReg32(TEMPCTHRE, 0x00000133);              // !!! set cold threshold

	DRV_WriteReg32(TEMPMSRCTL0, 0x0000000);                 // temperature measurement sampling control (one sampling)

	DRV_WriteReg32(TEMPADCPNP0, 0x172);
	DRV_WriteReg32(TEMPADCPNP1, 0x1B2);
	DRV_WriteReg32(TEMPADCPNP2, 0x1F2);

	DRV_WriteReg32(TEMPADCMUX, 0x11);		
	DRV_WriteReg32(TEMPADCEXT, 0x22);		 
	DRV_WriteReg32(TEMPADCEXT1, 0x33);				

	temp = 0x234;                                            // set to very hot for pnp0
    DRV_WriteReg32(TEMPADCEN, (0x00001000 + temp));        // set sensor voltage and sensor valid
	
	DRV_WriteReg32(TEMPPNPMUXADDR, 0xf00243fc);
	DRV_WriteReg32(TEMPADCENADDR, 0xf0024170);
	DRV_WriteReg32(TEMPADCVALIDADDR,0xf0024170);
	DRV_WriteReg32(TEMPADCVOLTADDR, 0xf0024170);
	
	DRV_WriteReg32(TEMPRDCTRL, 0x1);			 
	DRV_WriteReg32(TEMPADCVALIDMASK, 0x0000002C);
	DRV_WriteReg32(TEMPADCVOLTAGESHIFT, 0x0);
	DRV_WriteReg32(TEMPADCWRITECTRL, 0x00000000);

	// set thermal protection 
    DRV_WriteReg32(TEMPPROTCTL, 0x20000);                   // !!! maximum of sensors
    DRV_WriteReg32(TEMPPROTTA, 0x133);                      // !!! COLD
    DRV_WriteReg32(TEMPPROTTB, 0xFF);                       // !!! NORMAL
    DRV_WriteReg32(TEMPPROTTC, 0xAA);                       // !!! HOT

	// enable thermal protection stage interrupt 
	DRV_WriteReg32(TEMPMONINT, 0xE0000000);

	UTIL_Printf("thermal_trigger_wdt_reset: read TEMPMONINTSTS = 0x%x\n", DRV_Reg32(TEMPMONINTSTS));
	// 
	DRV_WriteReg32(TEMPMONCTL0, 0x00000007);

	thermal_intr_flag = false;
    count = 10;
    while (thermal_intr_flag != true && count--)
    {
        kal_sleep_task(10000);
    }
	
    temp = DRV_Reg32(TEMPMSR0);
    while ((temp & 0x8000) == 0)
    {
        kal_sleep_task(10000);
        temp = DRV_Reg32(TEMPMSR0);
    }
	UTIL_Printf("thermal_trigger_wdt_reset: read TEMPMSR0 = 0x%x\n", (temp & 0x0FFF));

    temp = (TEMPMSR1);
    UTIL_Printf("thermal_trigger_wdt_reset: read TEMPMSR1 = 0x%x\n", (temp & 0x0FFF));

    temp = DRV_Reg32(TEMPMSR2);
    UTIL_Printf("thermal_trigger_wdt_reset: read TEMPMSR2 = 0x%x\n", (temp & 0x0FFF));

	for (i = 0; i < 50; i++)
        kal_sleep_task(10000);
	
    UTIL_Printf("\n");
        
    thermal_intr_flag = false;

	#if 0
    temp = 0xA9;                                           // set to very cold for pnp0
    DRV_WriteReg32(TEMPADCEN, (0x00001000 + temp));        // set sensor voltage and sensor valid
	#else
	DRV_WriteReg32(TEMPMONCTL0, 0x00000000);
    temp = 0xA9;                                           // set to very cold for pnp0
    DRV_WriteReg32(TEMPADCEN, (0x00001000 + temp));        // set sensor voltage and sensor valid
    DRV_WriteReg32(TEMPMONCTL0, 0x00000007); 
	#endif
	
    for (i = 0; i < 50; i++)
        kal_sleep_task(10000);
    
	count = 10;
    while (thermal_intr_flag != true && count--)
    {
        kal_sleep_task(10000);
    }

    temp = DRV_Reg32(TEMPMSR0);
    while ((temp & 0x8000) == 0)
    {
        kal_sleep_task(10000);
        temp = DRV_Reg32(TEMPMSR0);
    }
    UTIL_Printf("thermal_trigger_wdt_reset: read TEMPMSR0 = 0x%x\n", (temp & 0x0FFF));
    
    temp = DRV_Reg32(TEMPMSR1);
    UTIL_Printf("thermal_trigger_wdt_reset: read TEMPMSR1 = 0x%x\n", (temp & 0x0FFF));

    temp = DRV_Reg32(TEMPMSR2);
    UTIL_Printf("thermal_trigger_wdt_reset: read TEMPMSR2 = 0x%x\n", (temp & 0x0FFF));

    UTIL_Printf("thermal_trigger_wdt_reset, end \n");

    thermal_intr_flag = false;
    
    return 0;
}

int thermal_interrupt_mask_test(void)
{
 	UINT32 temp = 0, i = 0, count = 0;
    
    thermal_real_test = false;
	   
	DRV_WriteReg32(TEMPMONCTL1, 0x00000004); // 3FF);    // counting unit is 1024 / 66M = 15.5us
	DRV_WriteReg32(TEMPMONCTL2, 0x00500050); // 219);	// sensing interval is 537 * 15.5us = 8.3235ms
	DRV_WriteReg32(TEMPAHBPOLL,  0x00000040);
	DRV_WriteReg32(TEMPAHBTO,    0x00000FFF);

    DRV_WriteReg32(TEMPMONIDET0, 0x00000000);               // times=1 for interrupt occurrance of sensing point 0
    DRV_WriteReg32(TEMPMONIDET1, 0x00000000);               // times=1 for interrupt occurrance of sensing point 1
    DRV_WriteReg32(TEMPMONIDET2, 0x00000000);               // times=1 for interrupt occurrance of sensing point 2

	DRV_WriteReg32(TEMPHTHRE, 0x000000AA);               // !!! set hot threshold
    DRV_WriteReg32(TEMPOFFSETH, 0x000000BB);     		 // !!! set high offset threshold
    DRV_WriteReg32(TEMPH2NTHRE, 0x000000FF);             // !!! set hot to normal threshold
    DRV_WriteReg32(TEMPOFFSETL, 0x00000111);      		 // !!! set low offset threshold
    DRV_WriteReg32(TEMPCTHRE, 0x00000133);               // !!! set cold threshold

	DRV_WriteReg32(TEMPMSRCTL0, 0x0000000);                 // temperature measurement sampling control (one sampling)

	DRV_WriteReg32(TEMPADCPNP0, 0x172);
	DRV_WriteReg32(TEMPADCPNP1, 0x1B2);
	DRV_WriteReg32(TEMPADCPNP2, 0x1F2);

	DRV_WriteReg32(TEMPADCMUX, 0x11);		
	DRV_WriteReg32(TEMPADCEXT, 0x22);		 
	DRV_WriteReg32(TEMPADCEXT1, 0x33);				

	temp = 0x14;                                            // set to very hot for pnp0
    DRV_WriteReg32(TEMPADCEN, (0x00001000 + temp));        // set sensor voltage and sensor valid
	
	DRV_WriteReg32(TEMPPNPMUXADDR, 0xf00243fc);
	DRV_WriteReg32(TEMPADCENADDR, 0xf0024170);
	DRV_WriteReg32(TEMPADCVALIDADDR,0xf0024170);
	DRV_WriteReg32(TEMPADCVOLTADDR, 0xf0024170);
	
	DRV_WriteReg32(TEMPRDCTRL, 0x1);			 
	DRV_WriteReg32(TEMPADCVALIDMASK, 0x0000002C);
	DRV_WriteReg32(TEMPADCVOLTAGESHIFT, 0x0);
	DRV_WriteReg32(TEMPADCWRITECTRL, 0x00000000);

	// mask all interrupt (hot, high, h2n, low, clod)
	DRV_WriteReg32(TEMPMONINT, 0x00000000);
	
	DRV_WriteReg32(TEMPMONCTL0, 0x00000007);

	thermal_intr_flag = false;
	for (i = 0; i < 5; i++)
        kal_sleep_task(10000);
    
    count = 10;
    while (thermal_intr_flag != true && count--)
    {
        kal_sleep_task(10000);
    }
	
    temp = DRV_Reg32(TEMPMSR0);
    while ((temp & 0x0FFF) != 0x14)
    {
        kal_sleep_task(10000);
        temp = DRV_Reg32(TEMPMSR0);
    }
    for (i = 0; i < (10 * 2 * 1 * 2); i++)
		kal_sleep_task(1000);
	
    if ((temp & 0x0FFF) != 0x14) // first filter valid should be equal to 0x14
        UTIL_Printf("thermal_different_threshold_test: fail, read TEMPMSR0(0x14) = 0x%x\n", (temp & 0x0FFF));
    else
        UTIL_Printf("thermal_different_threshold_test: pass, read TEMPMSR0(0x14) = 0x%x\n", (temp & 0x0FFF));
    
    temp = DRV_Reg32(TEMPMSR1);
    if ((temp & 0x0FFF) != 0x14) // first filter valid should be equal to 0x14
        UTIL_Printf("thermal_different_threshold_test: fail, read TEMPMSR1(0x14) = 0x%x\n", (temp & 0x0FFF));
    else
        UTIL_Printf("thermal_different_threshold_test: pass, read TEMPMSR1(0x14) = 0x%x\n", (temp & 0x0FFF));

    temp = DRV_Reg32(TEMPMSR2);
    if ((temp & 0x0FFF) != 0x14) // first filter valid should be equal to 0x14
        UTIL_Printf("thermal_different_threshold_test: fail, read TEMPMSR2(0x14) = 0x%x\n", (temp & 0x0FFF));
    else
        UTIL_Printf("thermal_different_threshold_test: pass, read TEMPMSR2(0x14) = 0x%x\n", (temp & 0x0FFF));

	for (i = 0; i < 50; i++)
        kal_sleep_task(10000);
	
    UTIL_Printf("\n");
        
    thermal_intr_flag = false;
    
    temp = 0x234;                                           // set to very cold for pnp0
    DRV_WriteReg32(TEMPADCEN, (0x00001000 + temp));        // set sensor voltage and sensor valid
    
    for (i = 0; i < 50; i++)
        kal_sleep_task(10000);
    
	count = 10;
    while (thermal_intr_flag != true && count--)
    {
        kal_sleep_task(10000);
    }

    temp = DRV_Reg32(TEMPMSR0);
    while ((temp & 0x0FFF) != 0x234)
    {
        kal_sleep_task(10000);
        temp = DRV_Reg32(TEMPMSR0);
    }
    for (i = 0; i < (10 * 2 * 1 * 2); i++)
		kal_sleep_task(1000);
	
    if ((temp & 0x0FFF) != 0x234)
        UTIL_Printf("thermal_different_threshold_test: fail, read TEMPMSR0(0x234) = 0x%x\n", (temp & 0x0FFF));
    else
        UTIL_Printf("thermal_different_threshold_test: pass, read TEMPMSR0(0x234) = 0x%x\n", (temp & 0x0FFF));
    
    temp = DRV_Reg32(TEMPMSR1);
    if ((temp & 0x0FFF) != 0x234)
        UTIL_Printf("thermal_different_threshold_test: fail, read TEMPMSR1(0x234) = 0x%x\n", (temp & 0x0FFF));
    else
        UTIL_Printf("thermal_different_threshold_test: pass, read TEMPMSR1(0x234) = 0x%x\n", (temp & 0x0FFF));

    temp = DRV_Reg32(TEMPMSR2);
    if ((temp & 0x0FFF) != 0x234)
        UTIL_Printf("thermal_different_threshold_test: fail, read TEMPMSR2(0x234) = 0x%x\n", (temp & 0x0FFF));
    else
        UTIL_Printf("thermal_different_threshold_test: pass, read TEMPMSR2(0x234) = 0x%x\n", (temp & 0x0FFF));

	for (i = 0; i < 50; i++)
        kal_sleep_task(10000);
	
    UTIL_Printf("\n");
    
    thermal_intr_flag = false;
	
    temp = 0xB0;                                           // set to very cold for pnp0
    DRV_WriteReg32(TEMPADCEN, (0x00001000 + temp));        // set sensor voltage and sensor valid
    
    for (i = 0; i < 50; i++)
        kal_sleep_task(10000);
    
    count = 10;
    while (thermal_intr_flag != true && count--)
    {
        kal_sleep_task(10000);
    }
	
    temp = DRV_Reg32(TEMPMSR0);
    while ((temp & 0x0FFF) != 0xB0)
    {
        kal_sleep_task(10000);
        temp = DRV_Reg32(TEMPMSR0);
    }
	
    for (i = 0; i < 10; i++)
        kal_sleep_task(10000);
	
    for (i = 0; i < (10 * 2 * 1 * 2); i++)
		kal_sleep_task(1000);
	
    if ((temp & 0x0FFF) != 0xB0) // first filter valid should be equal to 0xB0
        UTIL_Printf("thermal_different_threshold_test: fail, read TEMPMSR0(0xB0) = 0x%x\n", (temp & 0x0FFF));
    else
        UTIL_Printf("thermal_different_threshold_test: pass, read TEMPMSR0(0xB0) = 0x%x\n", (temp & 0x0FFF));
    
    temp = DRV_Reg32(TEMPMSR1);
    if ((temp & 0x0FFF) != 0xB0) // first filter valid should be equal to 0xB0
        UTIL_Printf("thermal_different_threshold_test: fail, read TEMPMSR1(0xB0) = 0x%x\n", (temp & 0x0FFF));
    else
        UTIL_Printf("thermal_different_threshold_test: pass, read TEMPMSR1(0xB0) = 0x%x\n", (temp & 0x0FFF));

    temp = DRV_Reg32(TEMPMSR2);
    if ((temp & 0x0FFF) != 0xB0) // first filter valid should be equal to 0xB0
        UTIL_Printf("thermal_different_threshold_test: fail, read TEMPMSR2(0xB0) = 0x%x\n", (temp & 0x0FFF));
    else
        UTIL_Printf("thermal_different_threshold_test: pass, read TEMPMSR2(0xB0) = 0x%x\n", (temp & 0x0FFF));
        
    UTIL_Printf("\n");

    thermal_intr_flag = false;
    
    return 0;
}


#define TEST_CONFIG_1	0

int thermal_real_interrupt_test(unsigned int sensor)
{
    UINT32 temp = 0, times = 0, i = 0;

	// enable all adc channel for HW mode 
	DRV_WriteReg32(PDWNC_SRVCFG1, 0x8887FFFF);				// counting unit is 320 * 31.25us = 10ms
	
	// set TSADC hw mode
	DRV_WriteReg32(PDWNC_SRVCFG0, 0x01);

	// set counting unit
	DRV_WriteReg32(TEMPMONCTL1, 0x00000004);				// counting unit is 320 * 31.25us = 10ms
	// set sensing interval
	DRV_WriteReg32(TEMPMONCTL2, 0x000C8000);				// sensing interval is 200 * 10ms = 2000ms
	// polling interval to check if temperature sense is ready
	DRV_WriteReg32(TEMPAHBPOLL, 0x000000FF);				// polling interval to check if temperature sense is ready
	// polling time for exceed
	DRV_WriteReg32(TEMPAHBTO, 0xFFFFFFFF);					// exceed this polling time, IRQ would be inserted

	// all interrupt occurrance times is 1
	DRV_WriteReg32(TEMPMONIDET0, 0x00);				// times for interrupt occurrance
	DRV_WriteReg32(TEMPMONIDET1, 0x00);				// times for interrupt occurrance
	DRV_WriteReg32(TEMPMONIDET2, 0x00);

	// set all threshold
	DRV_WriteReg32(TEMPHTHRE, 0x00000100);					// set hot threshold
	DRV_WriteReg32(TEMPOFFSETH, 0x00000200);				// set high offset threshold
	DRV_WriteReg32(TEMPH2NTHRE, 0x00000500);				// set hot to normal threshold
	DRV_WriteReg32(TEMPOFFSETL, 0x0000A00); 			   // set low offset threshold
	DRV_WriteReg32(TEMPCTHRE, 0x00000B00);					// set cold threshold

	// 
	DRV_WriteReg32(TEMPADCPNP0, 0x00);
	DRV_WriteReg32(TEMPADCPNP1, 0x01);
	DRV_WriteReg32(TEMPADCPNP2, 0x02);
	DRV_WriteReg32(TEMPADCPNP3, 0x03);
	
	DRV_WriteReg32(TEMPADCMUX, 0x11);
	DRV_WriteReg32(TEMPADCEXT, 0x22);
	DRV_WriteReg32(TEMPADCEXT1, 0x33);

	// give a fake temp
	DRV_WriteReg32(TEMPADCEN, 0x010FE);

	// address for switch
	DRV_WriteReg32(TEMPPNPMUXADDR, 0xf00243fc);
	DRV_WriteReg32(TEMPADCENADDR,  0x24170); 
	DRV_WriteReg32(TEMPADCVALIDADDR, 0x24170);
	DRV_WriteReg32(TEMPADCVOLTADDR, 0x24170);

	DRV_WriteReg32(TEMPRDCTRL, 0x0);
	DRV_WriteReg32(TEMPADCVALIDMASK, 0x0000002C);           // indicate where the valid bit is (the 12th bit is valid bit and 1 is valid)
	
    DRV_WriteReg32(TEMPADCVOLTAGESHIFT, 0x0);               // do not need to shift

	DRV_WriteReg32(TEMPADCWRITECTRL, 0x0);

	UTIL_Printf("thermal_real_interrupt_test, 11 \n");  
	
	// set sample mode
	DRV_WriteReg32(TEMPMSRCTL0, 0x00000000);

	// enable periodic temperature meausrement on sense point 0,1,2,3
	DRV_WriteReg32(TEMPMONCTL0, 0x0000000F);

    // for (j = 0; j <= 10; j++)
    {
        times = 1;
        while (times--)
        {
			UTIL_Printf("++ times = %d \n", (times));
			
			// enable interrupts:
			// 1.  cold/hot/low offset/high offset/H2N for 0,1,2,3
			// 2. AHB polling temp timeout interrupt
			// 3. IMMD interrupt for 0,1,2,3
			// 4. filter sense interrupt for 0,1,2,3
			// 5. thermal protection interrupt for stage 1,2,3
			//DRV_WriteReg32(TEMPMONINT, 0xFFFFFFFF);

			#if 1
			// just HOT int for 0,1,2,3
			DRV_WriteReg32(TEMPMONINT, 0x00800842);
			#elif 0
			// just COLD int for 0,1,2,3
			DRV_WriteReg32(TEMPMONINT, 0x00400421);
			#elif 0
			// just HIGH Offset int for 0,1,2,3
			DRV_WriteReg32(TEMPMONINT, 0x02002108);
			#elif 0
			// HOT & LOW Offset int for 0,1,2,3
			DRV_WriteReg32(TEMPMONINT, 0x0300318C);
			//DRV_WriteReg32(TEMPMONINT, 0x02002108);
			//DRV_WriteReg32(TEMPMONINT, 0x01001084);
			#elif 0
			// just LOW Offset int for 0,1,2,3
			DRV_WriteReg32(TEMPMONINT, 0x01001084);
			#endif
	
			// enable imme measurement for sense point 0,1,2,3
            DRV_WriteReg32(TEMPMSRCTL1, 0x000000270);

			for (i=0; i<2; i++) {
                kal_sleep_task(10000);
            }

			// read TEMPIMMD0
			temp = DRV_Reg32(TEMPIMMD0); 
			while ((temp & 0x8000) == 0)
            	temp = DRV_Reg32(TEMPIMMD0);
            UTIL_Printf("read TEMPIMMD0 = 0x%x \n", (temp));  

			// disable immediate mode for sensing point 0,1,2,3
            DRV_WriteReg32(TEMPMSRCTL1, 0x00000000);                

		}
    }

	// wait interrupt to occarence and ready
	for (i=0; i<5; i++) {
		kal_sleep_task(10000);
	}
	
    return 0;
}

int thermal_real_interrupt_test_HCHL(unsigned int sensor)
{
    UINT32 temp = 0, times = 0, i = 0;

	// enable all adc channel for HW mode 
	DRV_WriteReg32(PDWNC_SRVCFG1, 0x8887FFFF);				// counting unit is 320 * 31.25us = 10ms
	
	// set TSADC hw mode
	DRV_WriteReg32(PDWNC_SRVCFG0, 0x01);

	// set counting unit
	DRV_WriteReg32(TEMPMONCTL1, 0x00000004);				// counting unit is 320 * 31.25us = 10ms
	// set sensing interval
	DRV_WriteReg32(TEMPMONCTL2, 0x000C8000);				// sensing interval is 200 * 10ms = 2000ms
	// polling interval to check if temperature sense is ready
	DRV_WriteReg32(TEMPAHBPOLL, 0x000000FF);				// polling interval to check if temperature sense is ready
	// polling time for exceed
	DRV_WriteReg32(TEMPAHBTO, 0xFFFFFFFF);					// exceed this polling time, IRQ would be inserted

	// all interrupt occurrance times is 1
	DRV_WriteReg32(TEMPMONIDET0, 0x00);				// times for interrupt occurrance
	DRV_WriteReg32(TEMPMONIDET1, 0x00);				// times for interrupt occurrance
	DRV_WriteReg32(TEMPMONIDET2, 0x00);

	// set all threshold
	DRV_WriteReg32(TEMPHTHRE, 0x00000100);					// set hot threshold
	DRV_WriteReg32(TEMPOFFSETH, 0x00000200);				// set high offset threshold
	DRV_WriteReg32(TEMPH2NTHRE, 0x00000500);				// set hot to normal threshold
	DRV_WriteReg32(TEMPOFFSETL, 0x0000A00); 			   // set low offset threshold
	DRV_WriteReg32(TEMPCTHRE, 0x00000B00);					// set cold threshold

	// 
	DRV_WriteReg32(TEMPADCPNP0, 0x00);
	DRV_WriteReg32(TEMPADCPNP1, 0x01);
	DRV_WriteReg32(TEMPADCPNP2, 0x02);
	DRV_WriteReg32(TEMPADCPNP3, 0x03);
	
	DRV_WriteReg32(TEMPADCMUX, 0x11);
	DRV_WriteReg32(TEMPADCEXT, 0x22);
	DRV_WriteReg32(TEMPADCEXT1, 0x33);

	// give a fake temp
	DRV_WriteReg32(TEMPADCEN, 0x010FE);

	// address for switch
	DRV_WriteReg32(TEMPPNPMUXADDR, 0xf00243fc);
	DRV_WriteReg32(TEMPADCENADDR,  0x24170); 
	DRV_WriteReg32(TEMPADCVALIDADDR, 0x24170);
	DRV_WriteReg32(TEMPADCVOLTADDR, 0x24170);

	DRV_WriteReg32(TEMPRDCTRL, 0x0);
	DRV_WriteReg32(TEMPADCVALIDMASK, 0x0000002C);           // indicate where the valid bit is (the 12th bit is valid bit and 1 is valid)
	
    DRV_WriteReg32(TEMPADCVOLTAGESHIFT, 0x0);               // do not need to shift

	DRV_WriteReg32(TEMPADCWRITECTRL, 0x0);

	UTIL_Printf("thermal_real_interrupt_test, 11 \n");  
	
	// set sample mode
	DRV_WriteReg32(TEMPMSRCTL0, 0x00000000);

	// enable periodic temperature meausrement on sense point 0,1,2,3
	DRV_WriteReg32(TEMPMONCTL0, 0x0000000F);

    // for (j = 0; j <= 10; j++)
    {
        times = 1;
        while (times--)
        {
			UTIL_Printf("++ times = %d \n", (times));
			
			// enable interrupts:
			// 1.  cold/hot/low offset/high offset/H2N for 0,1,2,3
			// 2. AHB polling temp timeout interrupt
			// 3. IMMD interrupt for 0,1,2,3
			// 4. filter sense interrupt for 0,1,2,3
			// 5. thermal protection interrupt for stage 1,2,3
			//DRV_WriteReg32(TEMPMONINT, 0xFFFFFFFF);

			#if 1
			// just HOT int for 0,1,2,3
			DRV_WriteReg32(TEMPMONINT, 0x00800842);
			#elif 0
			// just COLD int for 0,1,2,3
			DRV_WriteReg32(TEMPMONINT, 0x00400421);
			#elif 0
			// just HIGH Offset int for 0,1,2,3
			DRV_WriteReg32(TEMPMONINT, 0x02002108);
			#elif 0
			// HOT & LOW Offset int for 0,1,2,3
			DRV_WriteReg32(TEMPMONINT, 0x0300318C);
			//DRV_WriteReg32(TEMPMONINT, 0x02002108);
			//DRV_WriteReg32(TEMPMONINT, 0x01001084);
			#elif 0
			// just LOW Offset int for 0,1,2,3
			DRV_WriteReg32(TEMPMONINT, 0x01001084);
			#endif
	
			// enable imme measurement for sense point 0,1,2,3
            DRV_WriteReg32(TEMPMSRCTL1, 0x000000270);

			for (i=0; i<2; i++) {
                kal_sleep_task(10000);
            }

			// read TEMPIMMD0
			temp = DRV_Reg32(TEMPIMMD0); 
			while ((temp & 0x8000) == 0)
            	temp = DRV_Reg32(TEMPIMMD0);
            UTIL_Printf("read TEMPIMMD0 = 0x%x \n", (temp));  

			// disable immediate mode for sensing point 0,1,2,3
            DRV_WriteReg32(TEMPMSRCTL1, 0x00000000);                

		}
    }

	// wait interrupt to occarence and ready
	for (i=0; i<5; i++) {
		kal_sleep_task(10000);
	}

    return 0;
}

int thermal_real_interrupt_test_H2N(unsigned int sensor)
{
    UINT32 temp = 0, times = 0, i = 0;

	// enable all adc channel for HW mode 
	DRV_WriteReg32(PDWNC_SRVCFG1, 0x8887FFFF);				// counting unit is 320 * 31.25us = 10ms
	
	// set TSADC hw mode
	DRV_WriteReg32(PDWNC_SRVCFG0, 0x01);

	// set counting unit
	DRV_WriteReg32(TEMPMONCTL1, 0x00000004);				// counting unit is 320 * 31.25us = 10ms
	// set sensing interval
	DRV_WriteReg32(TEMPMONCTL2, 0x000C8000);				// sensing interval is 200 * 10ms = 2000ms
	// polling interval to check if temperature sense is ready
	DRV_WriteReg32(TEMPAHBPOLL, 0x000000FF);				// polling interval to check if temperature sense is ready
	// polling time for exceed
	DRV_WriteReg32(TEMPAHBTO, 0xFFFFFFFF);					// exceed this polling time, IRQ would be inserted

	// all interrupt occurrance times is 1
	DRV_WriteReg32(TEMPMONIDET0, 0x00);				// times for interrupt occurrance
	DRV_WriteReg32(TEMPMONIDET1, 0x00);				// times for interrupt occurrance
	DRV_WriteReg32(TEMPMONIDET2, 0x00);
	DRV_WriteReg32(TEMPMONIDET3, 0x00);

	// set all threshold
	DRV_WriteReg32(TEMPHTHRE, 0x00000100);					// set hot threshold
	DRV_WriteReg32(TEMPOFFSETH, 0x00000200);				// set high offset threshold
	DRV_WriteReg32(TEMPH2NTHRE, 0x00000500);				// set hot to normal threshold
	DRV_WriteReg32(TEMPOFFSETL, 0x0000A00); 			   // set low offset threshold
	DRV_WriteReg32(TEMPCTHRE, 0x00000B00);					// set cold threshold

	// 
	DRV_WriteReg32(TEMPADCPNP0, 0x00);
	DRV_WriteReg32(TEMPADCPNP1, 0x01);
	DRV_WriteReg32(TEMPADCPNP2, 0x02);
	DRV_WriteReg32(TEMPADCPNP3, 0x03);
	
	DRV_WriteReg32(TEMPADCMUX, 0x11);
	DRV_WriteReg32(TEMPADCEXT, 0x22);
	DRV_WriteReg32(TEMPADCEXT1, 0x33);

	// give a fake temp
	DRV_WriteReg32(TEMPADCEN, 0x010FE);

	// address for switch
	DRV_WriteReg32(TEMPPNPMUXADDR, 0xf00243fc);
	DRV_WriteReg32(TEMPADCENADDR,  0xf0024170); 
	DRV_WriteReg32(TEMPADCVALIDADDR, 0xf0024170);
	DRV_WriteReg32(TEMPADCVOLTADDR, 0xf0024170);

	DRV_WriteReg32(TEMPRDCTRL, 0x0);
	DRV_WriteReg32(TEMPADCVALIDMASK, 0x0000002C);           // indicate where the valid bit is (the 12th bit is valid bit and 1 is valid)
    DRV_WriteReg32(TEMPADCVOLTAGESHIFT, 0x0);               // do not need to shift
	DRV_WriteReg32(TEMPADCWRITECTRL, 0x0);

	// set sample mode
	DRV_WriteReg32(TEMPMSRCTL0, 0x00000000);

	// enable periodic temperature meausrement on sense point 0,1,2,3
	DRV_WriteReg32(TEMPMONCTL0, 0x0000000F);

    // for (j = 0; j <= 10; j++)
    {
        times = 2;
        while (times--)
        {
			UTIL_Printf("++ times = %d \n", (times));
			
			// enable interrupts:
			// 1.  cold/hot/low offset/high offset/H2N for 0,1,2,3
			// 2. AHB polling temp timeout interrupt
			// 3. IMMD interrupt for 0,1,2,3
			// 4. filter sense interrupt for 0,1,2,3
			// 5. thermal protection interrupt for stage 1,2,3

			// set H2N int for 0,1,2,3
			DRV_WriteReg32(TEMPMONINT, 0x04004210);
	
			// enable imme measurement for sense point 0,1,2,3
            DRV_WriteReg32(TEMPMSRCTL1, 0x000000270);

			for (i=0; i<2; i++) {
                kal_sleep_task(10000);
            }

			// read TEMPIMMD0
			temp = DRV_Reg32(TEMPIMMD0); 
			while ((temp & 0x8000) == 0)
            	temp = DRV_Reg32(TEMPIMMD0);
            UTIL_Printf("read TEMPIMMD0 = 0x%x \n", (temp));  

			// disable immediate mode for sensing point 0,1,2,3
            DRV_WriteReg32(TEMPMSRCTL1, 0x00000000);                

			for (i=0; i<3; i++) {
                kal_sleep_task(10000);
            }

			// give second fake temp
			DRV_WriteReg32(TEMPADCEN, 0x01600);

			for (i=0; i<3; i++) {
                kal_sleep_task(10000);
            }
		}
    }

	// wait interrupt to occarence and ready
	for (i=0; i<5; i++) {
		kal_sleep_task(10000);
	}
	
    return 0;
}

int thermal_slt_test(void)
{
    UINT32 temp = 0, i = 0, count = 0;
	//void (*pfnOldIsr) (UINT16); //for bdp isr register
	 
		DRV_WriteReg32(PTPSPARE0, 0x1D00);
		DRV_WriteReg32(PTPSPARE1, 0x1E00); //23100
		DRV_WriteReg32(PTPSPARE2, 0x1E10); //
		kal_sleep_task(2000);

    thermal_real_test = true;

#if 0 //hywu: TV auxadc seems default on.
    // AuxADC Initialization
    temp = DRV_Reg32(AUXADC_MISC);
    DRV_WriteReg32(AUXADC_MISC, (temp | (1<<14)));
    
    temp = DRV_Reg32(AUXADC_CON0);
    temp &= 0xFFFFFFDF;
    DRV_WriteReg32(AUXADC_CON0, temp);        // disable auxadc channel 5 synchronous mode

    DRV_WriteReg32(AUXADC_CON1_CLR, 0x20);    // disable auxadc channel 5 immediate mode
    
    DRV_WriteReg32(AUXADC_CON1, 0x00000020);
#endif 

    DRV_WriteReg32(TEMPMONCTL1, 0x00000004);                // counting unit is 320 * 31.25us = 10ms
    DRV_WriteReg32(TEMPMONCTL2, 0x000000C8);                // sensing interval is 200 * 10ms = 2000ms
#if 0
    DRV_WriteReg32(TEMPAHBPOLL, 0x00000300);
#else
    DRV_WriteReg32(TEMPAHBPOLL, 0x0000000F);                // polling interval to check if temperature sense is ready
#endif
    DRV_WriteReg32(TEMPAHBTO, 0xFFFFFFFF);                  // exceed this polling time, IRQ would be inserted
    DRV_WriteReg32(TEMPMONIDET0, 0x00000000);               // times for interrupt occurrance
    DRV_WriteReg32(TEMPMONIDET1, 0x00000000);               // times for interrupt occurrance
    DRV_WriteReg32(TEMPHTHRE, 0x00000300);                  // set hot threshold
    DRV_WriteReg32(TEMPOFFSETH, 0x00000500);                // set high offset threshold
    DRV_WriteReg32(TEMPH2NTHRE, 0x00000800);                // set hot to normal threshold
    DRV_WriteReg32(TEMPOFFSETL, 0x00000900);                // set low offset threshold
    DRV_WriteReg32(TEMPCTHRE, 0x00001000);                  // set cold threshold
    DRV_WriteReg32(TEMPMSRCTL0, 0x00000005);                // temperature measurement sampling control

    //DRV_WriteReg32(TEMPADCPNP0, 0x0);                       // this value will be stored to TEMPPNPMUXADDR (TEMPSPARE0) automatically by hw
    //DRV_WriteReg32(TEMPPNPMUXADDR, (UINT32) TEMPSPARE0);    // AHB address for pnp sensor mux selection

    DRV_WriteReg32(TEMPADCMUX, 0x20);                        // this value will be stored to TEMPPNPMUXADDR (TEMPSPARE0) automatically by hw
    DRV_WriteReg32(TEMPADCMUXADDR, PDWNC_SRVCFG0);   // AHB address for auxadc mux selection

    DRV_WriteReg32(TEMPADCEN, 0x20);                        // AHB value for auxadc enable
    DRV_WriteReg32(TEMPADCENADDR, (UINT32) PDWNC_SRVSWT);    // AHB address for auxadc enable (channel 0 immediate mode selected)

    DRV_WriteReg32(TEMPADCVALIDADDR, (UINT32) PDWNC_ADOUT5); // AHB address for auxadc valid bit
    DRV_WriteReg32(TEMPADCVOLTADDR, (UINT32) PDWNC_ADOUT5);  // AHB address for auxadc voltage output

	for(i=1;i<ROME_BANK_NUM;i++)
    {
        mtktscpu_switch_bank(i);        			
    		DRV_WriteReg32(TEMPADCVALIDADDR, (UINT32) (PTPSPARE0_P+4*i));
    		DRV_WriteReg32(TEMPADCVOLTADDR, (UINT32) (PTPSPARE0_P+4*i));	
    }

    DRV_WriteReg32(TEMPRDCTRL, 0x0);                        // read valid & voltage are at the same register
    DRV_WriteReg32(TEMPADCVALIDMASK, 0x0000002C);           // indicate where the valid bit is (the 12th bit is valid bit and 1 is valid)
    DRV_WriteReg32(TEMPADCVOLTAGESHIFT, 0x0);               // do not need to shift
    DRV_WriteReg32(TEMPADCWRITECTRL, 0x2);                  // enable auxadc mux write transaction

    DRV_WriteReg32(TEMPMONINT, 0x0007FFFF);     // enable all interrupt

    DRV_WriteReg32(TEMPMSRCTL1, 0x00000010);    // enable immediate mode for sensing point 0

    /* register thermal controller interrupt */	
	
#if 0 // zplee
	if (x_reg_isr(VECTOR_PTP_THERM, vDrvThermalISR, &pfnOldIsr))
    {
         UTIL_Printf("%s ISR regiser failure \n",__func__);
		 return -1;
    }

	BIM_EnableIrq(VECTOR_PTP_THERM);
#endif

	print_reg_setting();
				
    count = 1000;
    while (thermal_intr_flag != true && count--)
    {
        kal_sleep_task(100);
    }

    if (count <= 0)
        return -1;

    count = 1000;
    temp = DRV_Reg32(TEMPIMMD0);
    while ((temp & 0x8000) == 0 && count--)
    {
        UTIL_Printf("thermal_real_interrupt_test: read TEMPIMMD0 = 0x%x, data not ready, try again..\n", (temp & 0x0FFF));
        temp = DRV_Reg32(TEMPIMMD0);

        kal_sleep_task(100);
    }

    if (count <= 0)
        return -1;

    DRV_WriteReg32(TEMPMSRCTL1, 0x00000000);                // disable immediate mode for sensing point 0

    thermal_intr_flag = false;
    
	tscpu_config_all_tc_hw_protect(100000, 10000);
    
    return 0;
}


//-------------------------------------------------------
#if 0
//get real temp implemetation
static INT32 g_o_vtsmcu1 = 0;
static INT32 g_o_vtsmcu2 = 0;
static INT32 g_o_vtsmcu3 = 0;
static INT32 g_o_vtsabb = 0;
static INT32 g_degc_cali = 0;
static INT32 g_adc_cali_en = 0;
static INT32 g_o_slope = 0;
static INT32 g_o_slope_sign = 0;
static INT32 g_id = 0;

static INT32 g_ge = 0;
static INT32 g_oe = 0;
static INT32 g_gain = 0;


#define TS_NUM 5

static INT32 g_x_roomt[THERMAL_SENSOR_NUM] = {0};

static INT32 g_adc_cali_en_t= 0;
static INT32 g_adc_ge_t= 0;
static INT32 g_adc_oe_t= 0;
static INT32 g_o_vtsmcu4= 0;
#else
static INT32 g_o_vtsabb = 0;
static INT32 g_adc_cali_en_t= 0;
static INT32 g_adc_ge_t= 0;
static INT32 g_adc_oe_t= 0;
static INT32 g_o_vtsmcu4= 0;
#endif

static void thermal_cal_prepare(void)
{
	kal_uint32 temp0 = 0, temp1 = 0, temp2 = 0;

	temp0 = DRV_Reg32(0x10206184);//K2
    temp1 = DRV_Reg32(0x10206180);//K2
    temp2 = DRV_Reg32(0x10206188);//K2

	UTIL_Printf("[Power/CPU_Thermal] [Thermal calibration] temp0=0x%x, temp1=0x%x\n", temp0, temp1);
	//mtktscpu_dprintk("thermal_cal_prepare\n");

    g_adc_ge_t     = ((temp0 & 0xFFC00000)>>22);//ADC_GE_T    [9:0] *(0x10206184)[31:22]
	g_adc_oe_t     = ((temp0 & 0x003FF000)>>12);//ADC_OE_T    [9:0] *(0x10206184)[21:12]

	g_o_vtsmcu1    = (temp1 & 0x03FE0000)>>17;  //O_VTSMCU1    (9b) *(0x10206180)[25:17]
	g_o_vtsmcu2    = (temp1 & 0x0001FF00)>>8;   //O_VTSMCU2    (9b) *(0x10206180)[16:8]
	g_o_vtsmcu3    = (temp0 & 0x000001FF);      //O_VTSMCU3    (9b) *(0x10206184)[8:0]
	g_o_vtsmcu4    = (temp2 & 0xFF800000)>>23;  //O_VTSMCU4    (9b) *(0x10206188)[31:23]
 	g_o_vtsabb     = (temp2 & 0x007FC000)>>14;	//O_VTSABB     (9b) *(0x10206188)[22:14]

	g_degc_cali    = (temp1 & 0x0000007E)>>1;   //DEGC_cali    (6b) *(0x10206180)[6:1]
	g_adc_cali_en_t= (temp1 & 0x00000001);		//ADC_CALI_EN_T(1b) *(0x10206180)[0]
	g_o_slope_sign = (temp1 & 0x00000080)>>7;   //O_SLOPE_SIGN (1b) *(0x10206180)[7]
	g_o_slope      = (temp1 & 0xFC000000)>>26;  //O_SLOPE      (6b) *(0x10206180)[31:26]

	g_id           = (temp0 & 0x00000200)>>9;   //ID           (1b) *(0x10206184)[9]

	/*
	Check ID bit
	If ID=0 (TSMC sample)    , ignore O_SLOPE EFuse value and set O_SLOPE=0.
	If ID=1 (non-TSMC sample), read O_SLOPE EFuse value for following calculation.
    */
	if(g_id==0)
	{
		g_o_slope=0;
	}


	//FIX ME
	//g_adc_cali_en_t = 0;//Force this value to be "0" until chip is calibrated, need to mark after bring up.
	//FIX ME

	if(g_adc_cali_en_t == 1)
	{
		//thermal_enable = true;
	}
	else
	{
		UTIL_Printf("This sample is not Thermal calibrated\n");

		g_adc_ge_t     = 512;
		g_adc_oe_t     = 512;
		g_degc_cali    = 40;
		g_o_slope      = 0;
		g_o_slope_sign = 0;
        g_o_vtsmcu1    = 260;
		g_o_vtsmcu2    = 260;
		g_o_vtsmcu3    = 260;
		g_o_vtsmcu4    = 260;
		g_o_vtsabb     = 260;
	}
/*
	UTIL_Printf("[Power/CPU_Thermal] [calibration] g_adc_ge_t      = 0x%x\n",g_adc_ge_t);
	UTIL_Printf("[Power/CPU_Thermal] [calibration] g_adc_oe_t      = 0x%x\n",g_adc_oe_t);
	UTIL_Printf("[Power/CPU_Thermal] [calibration] g_degc_cali     = 0x%x\n",g_degc_cali);
	UTIL_Printf("[Power/CPU_Thermal] [calibration] g_adc_cali_en_t = 0x%x\n",g_adc_cali_en_t);
	UTIL_Printf("[Power/CPU_Thermal] [calibration] g_o_slope       = 0x%x\n",g_o_slope);
	UTIL_Printf("[Power/CPU_Thermal] [calibration] g_o_slope_sign  = 0x%x\n",g_o_slope_sign);
	UTIL_Printf("[Power/CPU_Thermal] [calibration] g_id            = 0x%x\n",g_id);


	UTIL_Printf("[Power/CPU_Thermal] [calibration] g_o_vtsmcu2     = 0x%x\n",g_o_vtsmcu2);
	UTIL_Printf("[Power/CPU_Thermal] [calibration] g_o_vtsmcu3     = 0x%x\n",g_o_vtsmcu3);
	UTIL_Printf("[Power/CPU_Thermal] [calibration] g_o_vtsmcu4     = 0x%x\n",g_o_vtsmcu4);
*/
}

static void thermal_cal_prepare_2(UINT32 ret)
{
	INT32 format_1= 0,format_2= 0, format_3= 0, format_4= 0, format_5= 0;

	UTIL_Printf("thermal_cal_prepare_2\n");

	g_ge = ((g_adc_ge_t - 512) * 10000 ) / 4096; // ge * 10000
	g_oe =  (g_adc_oe_t - 512);

	g_gain = (10000 + g_ge);

	format_1   = (g_o_vtsmcu1 + 3350 - g_oe);
	format_2   = (g_o_vtsmcu2 + 3350 - g_oe);
	format_3   = (g_o_vtsmcu3 + 3350 - g_oe);
	format_4   = (g_o_vtsmcu4 + 3350 - g_oe);
	format_5   = (g_o_vtsabb  + 3350 - g_oe);


	g_x_roomt[0]   = (((format_1   * 10000) / 4096) * 10000) / g_gain; // x_roomt * 10000
	g_x_roomt[1]   = (((format_2   * 10000) / 4096) * 10000) / g_gain; // x_roomt * 10000
	g_x_roomt[2]   = (((format_3   * 10000) / 4096) * 10000) / g_gain; // x_roomt * 10000
	g_x_roomt[3]   = (((format_4   * 10000) / 4096) * 10000) / g_gain; // x_roomt * 10000
	//g_x_roomt[4]   = (((format_5   * 10000) / 4096) * 10000) / g_gain; // x_roomt * 10000

/*
    UTIL_Printf("[calibration] g_ge         = 0x%x\n",g_ge);
    UTIL_Printf("[calibration] g_gain       = 0x%x\n",g_gain);
    UTIL_Printf("[calibration] g_x_roomt1   = 0x%x\n",g_x_roomt[0]);
    UTIL_Printf("[calibration] g_x_roomt2   = 0x%x\n",g_x_roomt[1]);
    UTIL_Printf("[calibration] g_x_roomt3   = 0x%x\n",g_x_roomt[2]);
    UTIL_Printf("[calibration] g_x_roomt4   = 0x%x\n",g_x_roomt[3]);
    UTIL_Printf("[calibration] g_x_roomtabb = 0x%x\n",g_x_roomt[4]);
*/
}

static INT32 temperature_to_raw_room(UINT32 ret)
{
	// Ycurr = [(Tcurr - DEGC_cali/2)*(165+O_slope)*(18/15)*(1/10000)+X_roomtabb]*Gain*4096 + OE

	INT32 t_curr = ret;
	INT32 format_1 = 0;
	INT32 format_2 = 0;
	INT32 format_3[THERMAL_SENSOR_NUM] = {0};
	INT32 format_4[THERMAL_SENSOR_NUM] = {0};
    INT32 i, index=0, temp = 0;


	//UTIL_Printf("temperature_to_raw_room\n");

	if(g_o_slope_sign==0)//O_SLOPE is Positive.
	{
		format_1 = t_curr-(g_degc_cali*1000/2);
		format_2 = format_1 * (165 + g_o_slope) * 18/15;
		format_2 = format_2 - 2*format_2;

		for (i=0; i<THERMAL_SENSOR_NUM; i++)
		{
			format_3[i] = format_2/1000 + g_x_roomt[i]*10;
			format_4[i] = (format_3[i]*4096/10000*g_gain)/100000 + g_oe;
			//UTIL_Printf("[Temperature_to_raw_roomt][roomt%d] format_1=%d, format_2=%d, format_3=%d, format_4=%d\n", i, format_1, format_2, format_3[i], format_4[i]);
		}

		//tscpu_dprintk("temperature_to_raw_abb format_1=%d, format_2=%d, format_3=%d, format_4=%d\n", format_1, format_2, format_3, format_4);
	}
	else //O_SLOPE is Negative.
	{
		format_1 = t_curr-(g_degc_cali*1000/2);
		format_2 = format_1 * (165 - g_o_slope) * 18/15;
		format_2 = format_2 - 2*format_2;

		for (i=0; i<THERMAL_SENSOR_NUM; i++)
		{
			format_3[i] = format_2/1000 + g_x_roomt[i]*10;
			format_4[i] = (format_3[i]*4096/10000*g_gain)/100000 + g_oe;
			//UTIL_Printf("[Temperature_to_raw_roomt][roomt%d] format_1=%d, format_2=%d, format_3=%d, format_4=%d\n", i, format_1, format_2, format_3[i], format_4[i]);
		}

		//tscpu_dprintk("temperature_to_raw_abb format_1=%d, format_2=%d, format_3=%d, format_4=%d\n", format_1, format_2, format_3, format_4);
	}


	temp = 0;
	for (i=0; i<THERMAL_SENSOR_NUM; i++)
	{
		if (temp < format_4[i])
		{
			temp = format_4[i];
			index = i;
		}
	}

	UTIL_Printf("[Temperature_to_raw_roomt] temperature=%d, raw[%d]=%d\n", ret, index, format_4[index]);
	return format_4[index];

}

static INT32 raw_to_temperature_roomt(UINT32 ret, thermal_sensor_name ts_name)
{
	INT32 t_current = 0;
	INT32 y_curr = ret;
	INT32 format_1 = 0;
	INT32 format_2 = 0;
	INT32 format_3 = 0;
	INT32 format_4 = 0;
	INT32 xtoomt=0;

    xtoomt = g_x_roomt[ts_name];

	UTIL_Printf("ts_name=%d, xtoomt=%d\n", ts_name, xtoomt);

	if(ret==0)
	{
		return 0;
	}

	format_1 = ((g_degc_cali*10) >> 1);
	format_2 = (y_curr - g_oe);
	UTIL_Printf("format_1 = %d, format_2 = %d\n", format_1, format_2);

	format_3 = (((((format_2) * 10000) >> 12 ) * 10000) / g_gain) - xtoomt;
	format_3 = format_3 * 15/18;
	UTIL_Printf("format_3 = %d \n", format_3);

	if(g_o_slope_sign==0)
	{
		format_4 = ((format_3 * 100) / (165+g_o_slope)); // uint = 0.1 deg
		UTIL_Printf("format_4 = %d, 1 \n", format_4);
	}
	else
	{
		format_4 = ((format_3 * 100) / (165-g_o_slope)); // uint = 0.1 deg
		UTIL_Printf("format_4 = %d, 2 \n", format_4);
	}
	format_4 = format_4 - (format_4 << 1);
	UTIL_Printf("format_4 = %d, 3 \n", format_4);
	
	t_current = format_1 + format_4; // uint = 0.1 deg

	UTIL_Printf("t_current=%d \n", t_current);
	
	return t_current;
}

static void thermal_calibration(void)
{
	if (g_adc_cali_en == 0)
		UTIL_Printf("###### Not Calibration ######\n");
	thermal_cal_prepare_2(0);
}

static int read_tc_raw_and_temp(unsigned long tempmsr_name, thermal_sensor_name ts_name, int *ts_raw)
{
	int temp=0, raw=0;
//	int curr_raw1,curr_raw2,curr_raw3, curr_temp1,curr_temp2, curr_temp3;

    UTIL_Printf("read_tc_raw_temp, tempmsr_name=0x%lx, ts_name=%d \n", tempmsr_name, ts_name);
	if (tempmsr_name != 0) {
		raw = DRV_Reg32(tempmsr_name) & 0x0fff;
	} else {
		raw = 0;
	}
	UTIL_Printf("adc raw = 0x%x \n", raw);

	if (tempmsr_name != 0) {
		temp = raw_to_temperature_roomt(raw, ts_name);
	} else {
		temp = 0;
	}
	
    *ts_raw = raw;
    UTIL_Printf("raw = %d, temp = %d \n", *ts_raw, temp*100);
	
	return (temp*100);
}

static int read_tc_raw_and_temp_check(void)
{
	int temp=0, raw=0;

	raw = DRV_Reg32(TEMPMSR0) & 0x0fff;
	UTIL_Printf("adc raw = 0x%x \n", raw);

	temp = raw_to_temperature_roomt(raw, 0);
    UTIL_Printf("temp = %d \n", temp);
	
	return (temp*100);
}

static int read_each_bank_TS_2(thermal_bank_name bank_num)
{
	int lv_CA7_TS1_T=0,lv_CA7_TS2_T=0;
	int lv_CA15_TS1_T=0,lv_CA15_TS3_T=0;
	int lv_GPU_TS3_T=0,lv_GPU_TS4_T=0;
	//int lv_CORE_TS2_T=0,lv_CORE_TS4_T=0,lv_CORE_TSABB_T=0;

	int lv_CA7_TS1_R=0,lv_CA7_TS2_R=0;
	int lv_CA15_TS1_R=0,lv_CA15_TS3_R=0;
	int lv_GPU_TS3_R=0,lv_GPU_TS4_R=0;
    int max_T = 0;

	//UTIL_Printf("read_each_bank_TS,bank_num=%d\n",bank_num);

	switch(bank_num){
	    case THERMAL_BANK0:
			//Bank 0 : CA7  (TS1 TS2)
			lv_CA7_TS1_T = read_tc_raw_and_temp(TEMPMSR0,THERMAL_SENSOR1,&lv_CA7_TS1_R);
			lv_CA7_TS2_T = read_tc_raw_and_temp(TEMPMSR1,THERMAL_SENSOR2,&lv_CA7_TS2_R);
            max_T = MAX(lv_CA7_TS1_T,lv_CA7_TS2_T);
	        break;
	    case THERMAL_BANK1:
			//Bank 1 : CA15 (TS1 TS3)
			lv_CA15_TS1_T = read_tc_raw_and_temp(TEMPMSR0,THERMAL_SENSOR1,&lv_CA15_TS1_R);
			lv_CA15_TS3_T = read_tc_raw_and_temp(TEMPMSR1,THERMAL_SENSOR3,&lv_CA15_TS3_R);
            max_T = MAX(lv_CA15_TS1_T,lv_CA15_TS3_T);
	        break;
	    case THERMAL_BANK2:
			//Bank 2 : GPU  (TS3 TS4)
			lv_GPU_TS3_T = read_tc_raw_and_temp(TEMPMSR0,THERMAL_SENSOR3,&lv_GPU_TS3_R);
			lv_GPU_TS4_T = read_tc_raw_and_temp(TEMPMSR1,THERMAL_SENSOR4,&lv_GPU_TS4_R);
            max_T = MAX(lv_GPU_TS3_T,lv_GPU_TS4_T);
	        break;
		default:
			lv_CA7_TS1_T = read_tc_raw_and_temp(TEMPMSR0,THERMAL_SENSOR1,&lv_CA7_TS1_R);
			lv_CA7_TS2_T = read_tc_raw_and_temp(TEMPMSR1,THERMAL_SENSOR2,&lv_CA7_TS2_R);
            max_T = MAX(lv_CA7_TS1_T,lv_CA7_TS2_T);
			break;
    }
   // UTIL_Printf("read_each_bank_TS,max_T=%d\n",max_T);
    return max_T;
}



static void read_each_bank_TS(thermal_bank_name bank_num)
{

	//UTIL_Printf("read_each_bank_TS,bank_num=%d\n",bank_num);

	switch(bank_num){
	    case THERMAL_BANK0:
			//Bank0 : CPU TS_MCU1 (TS_MCU1,TS_MCU2)
#if 1
			CPU_TS_MCU1_T = read_tc_raw_and_temp(TEMPMSR0,THERMAL_SENSOR1,&CPU_TS_MCU1_R);
#else			
			CPU_TS_MCU1_T = read_tc_raw_and_temp(TEMPMSR0,THERMAL_SENSOR1,&CPU_TS_MCU1_R);
			CPU_TS_MCU2_T = read_tc_raw_and_temp(TEMPMSR1,THERMAL_SENSOR2,&CPU_TS_MCU2_R);
#endif
	        break;
	    case THERMAL_BANK1:
			//Bank1 : GPU TS_MCU2 (TS_MCU3)
			//read extra ABB here
#if 1
			GPU_TS_MCU3_T = read_tc_raw_and_temp(TEMPMSR1,THERMAL_SENSOR2,&GPU_TS_MCU3_R);
#else
			GPU_TS_MCU3_T = read_tc_raw_and_temp(TEMPMSR0,THERMAL_SENSOR3,&GPU_TS_MCU3_R);
			ABB_TS_ABB_T  = read_tc_raw_and_temp(TEMPMSR1,THERMAL_SENSORABB,&ABB_TS_ABB_R);
#endif
	        break;
	    case THERMAL_BANK2:
      //Bank2 : SOC TS_MCU3 (TS_MCU4,TS_MCU2,TS_MCU3)
#if 1
			SOC_TS_MCU2_T = read_tc_raw_and_temp(TEMPMSR2,THERMAL_SENSOR3,&SOC_TS_MCU2_R);
			SOC_TS_MCU3_R = SOC_TS_MCU2_R;
			SOC_TS_MCU4_R = SOC_TS_MCU2_R;
			SOC_TS_MCU3_T = SOC_TS_MCU2_T;
			SOC_TS_MCU4_T = SOC_TS_MCU2_T;
#else
			SOC_TS_MCU4_T = read_tc_raw_and_temp(TEMPMSR0,THERMAL_SENSOR4,&SOC_TS_MCU4_R);
			SOC_TS_MCU2_T = read_tc_raw_and_temp(TEMPMSR1,THERMAL_SENSOR2,&SOC_TS_MCU2_R);
			SOC_TS_MCU3_T = read_tc_raw_and_temp(TEMPMSR2,THERMAL_SENSOR3,&SOC_TS_MCU3_R);
#endif
	        break;
		default:
            //Bank0 : CPU (TS_MCU1,TS_MCU2)
			CPU_TS_MCU1_T = read_tc_raw_and_temp(TEMPMSR0,THERMAL_SENSOR1,&CPU_TS_MCU1_R);
			CPU_TS_MCU2_T = read_tc_raw_and_temp(TEMPMSR1,THERMAL_SENSOR2,&CPU_TS_MCU2_R);
			break;
    }

}

static void read_all_bank_temperature(void)
{
	int i=0;

    /*config bank0,1,2,3*/
    for(i=0;i<ROME_BANK_NUM;i++){

		//UTIL_Printf("###########################\n");
        mtktscpu_switch_bank(i);
		read_each_bank_TS(i);
    }

 }

int mtktscpu_get_each_bank_temp(thermal_bank_name bank_num)
{
    int MAX_T;

	MAX_T = read_each_bank_TS_2(bank_num);

	UTIL_Printf("=======================\n");
	UTIL_Printf("MAX_T1=%d\n",MAX_T);
	UTIL_Printf("=======================\n");

	return MAX_T;

}

int thermal_get_all_TS_temp(void)
{

   	int bank0_T;
	int bank1_T;
	int bank2_T; 

    int MAX_T1,MAX_T2 ;


    UTIL_Printf("thermal_get_all_TS_temp\n");

   	thermal_cal_prepare();
	thermal_calibration();


    // turn off the sensor buffer to save power
    //DRV_WriteReg16(TS_CON0, DRV_Reg16(TS_CON0) | 0x00c0);


	//reset thremal ctrl
	tscpu_reset_thermal();

    thermal_initial_all_bank();


	/*
	Bank0 : CPU (TS_MCU1,TS_MCU2)        (TS3, TS4)
	Bank1 : GPU (TS_MCU3)                (TS5)
	Bank2 : SOC (TS_MCU4,TS_MCU2,TS_MCU3)(TS1, TS4, TS5)
	*/
	read_all_bank_temperature();
	bank0_T = MAX(CPU_TS_MCU1_T,CPU_TS_MCU2_T);
   	bank1_T = MAX(GPU_TS_MCU3_T,ABB_TS_ABB_T);
	bank2_T = MAX(SOC_TS_MCU4_T,SOC_TS_MCU2_T);
    bank2_T = MAX(SOC_TS_MCU3_T,bank2_T);



	MAX_T1 = MAX(bank0_T,bank1_T);
    MAX_T2 = MAX(bank2_T,MAX_T1);





	//UTIL_Printf("===============================================================\n");
	//UTIL_Printf("bank0_T =%d,bank1_T =%d,bank2_T =%d,bank3_T =%d,MAX_T3 =%d\n",bank0_T,bank1_T,bank2_T,bank3_T,MAX_T3);
	//UTIL_Printf("===============================================================\n");


	UTIL_Printf("\n\n");
	UTIL_Printf("Bank 0 : CPU  (TS_MCU1 = %d,TS_MCU2 = %d)             \n",CPU_TS_MCU1_T,CPU_TS_MCU2_T);
	UTIL_Printf("Bank 1 : GPU  (TS_MCU3 = %d)                          \n",GPU_TS_MCU3_T);
	UTIL_Printf("Bank 2 : SOC  (TS_MCU4 = %d,TS_MCU2 = %d,TS_MCU3 = %d)\n",SOC_TS_MCU4_T,SOC_TS_MCU2_T,SOC_TS_MCU3_T);


    //UTIL_Printf("CA7_TS1_R =%d,CA7_TS2_R =%d,CA15_TS3_R =%d,GPU_TS4_R =%d,,CORE_TSABB_R =%d\n",CA7_TS1_R,CA7_TS2_R,CA15_TS3_R,GPU_TS4_R,CORE_TSABB_R);

	return MAX_T2;

}

#if 1
void vDrvThermal_Cal_Prepare(void)
{
	kal_uint32 temp1;
	//int i = 0;

	#if 0
	for (i=0;i<30;i++) {
		temp0 = DRV_Reg32(EFUSE_CTRL_BASE + 0x660 + i*4);
		UTIL_Printf("=> efuse Reg[0x%x]=0x%x \n", (0x660 + i*4), temp0);
	}
	#endif

// 0x54690
//    8:0      -> efuse_thr_local_thermal
//    18:9    -> efuse_thr_adc_cali_oe
//    28:19  ->  efuse_thr_adc_cali_ge

// 0x546B8
//    8:0       ->  efuse_thr_thermal2
//	 17:9     ->  efuse_thr_thermal1
//    26:18    ->  efuse_thr_thermal0

// 0x546BC
//	  9:0		->	efuse_thr_reserved
//	  10	       ->   efuse_thr_chip_id
//	  16:11    ->   efuse_thr_o_slope
//	  17		 ->	 efuse_thr_o_slope_sign
//	  18         ->   efuse_thr_cali_en
//	  24:19     ->   efuse_thr_cur_temp (DEGC_Cali)

	temp1 = 0;
	temp1 = DRV_Reg32(EFUSE_CTRL_BASE + 0x690);
	UTIL_Printf("[thermal] => Reg[0x690]=0x%x \n", temp1);
	g_adc_ge = (temp1 & 0x1FF80000)>>19;
	g_adc_oe = (temp1 & 0x0007FE00)>>9;
	//local_thermal = temp1 & 0x000001FF;
	UTIL_Printf("[thermal]   g_adc_ge=0x%x (%d) \n", g_adc_ge, g_adc_ge);
	UTIL_Printf("[thermal]   g_adc_oe=0x%x (%d) \n", g_adc_oe, g_adc_oe);

	temp1 = 0;
	temp1 = DRV_Reg32(EFUSE_CTRL_BASE + 0x6B8);
	UTIL_Printf("=> Reg[0x6B8]=0x%x \n", temp1);
	g_o_vtsmcu1 = (temp1 & 0x07FC0000)>>18;
	g_o_vtsmcu2 = (temp1 & 0x0003FE00)>>9;
	g_o_vtsmcu3 = temp1 & 0x000001FF;
	UTIL_Printf("[thermal]   g_o_vtsmcu1=0x%x (%d) \n", g_o_vtsmcu1, g_o_vtsmcu1);
	UTIL_Printf("[thermal]   g_o_vtsmcu2=0x%x (%d) \n", g_o_vtsmcu2, g_o_vtsmcu2);
	UTIL_Printf("[thermal]   g_o_vtsmcu3=0x%x (%d) \n", g_o_vtsmcu3, g_o_vtsmcu3);


	temp1 = 0;
	temp1 = DRV_Reg32(EFUSE_CTRL_BASE + 0x6BC);
	UTIL_Printf("[thermal] => Reg[0x6BC]=0x%x \n", temp1);
	g_degc_cali = (temp1 & 0x01F80000)>>19;
	g_adc_cali_en = (temp1 & 0x00040000)>>18;
	g_o_slope_sign = (temp1 & 0x00020000)>>17;
	g_o_slope = (temp1 & 0x0001F800)>>11;
	g_id = (temp1 & 0x00000400)>>10;
	// efuse_thr_reserved = temp1 & 0x000003FF;
	UTIL_Printf("[thermal]   g_degc_cali=0x%x (%d) \n", g_degc_cali, g_degc_cali);
	UTIL_Printf("[thermal]   g_adc_cali_en=0x%x (%d) \n", g_adc_cali_en, g_adc_cali_en);
	UTIL_Printf("[thermal]   g_o_slope_sign=0x%x (%d) \n", g_o_slope_sign, g_o_slope_sign);
	UTIL_Printf("[thermal]   g_o_slope=0x%x (%d) \n", g_o_slope, g_o_slope);
	UTIL_Printf("[thermal]   g_id=0x%x (%d) \n", g_id, g_id);

	if(g_adc_cali_en == 1)
	{
		//thermal_enable = true;    
		UTIL_Printf("[thermal] got efuse value !!! \n");
	}
	else
	{
		UTIL_Printf("[thermal] Err: no thermal efuse value, usr default value \n");
		#if 0  //#ifdef CC_MT5891	
		g_adc_ge = 512;
		g_adc_oe = 512;
		g_degc_cali = 50;
		g_o_slope = 2;	// 165+2 = 167 1.67	
		g_o_slope_sign = 0;
		g_o_vtsmcu1 = 205;
		g_o_vtsmcu2 = 205;
		g_o_vtsmcu3 = 205;
		#else
		g_adc_ge = 327;
		g_adc_oe = 501;
		g_degc_cali = 44;
		g_o_slope = 6;
		g_o_slope_sign = 0;
		g_o_vtsmcu1 = 52;
		g_o_vtsmcu2 = 52;
		g_o_vtsmcu3 = 52;
		
		#if 0
		UTIL_Printf("   g_adc_ge = 0x%x \n   g_adc_oe = 0x%x \n   g_degc_cali = 0x%x \n   g_adc_cali_en = 0x%x \n", 
		g_adc_ge, g_adc_oe, g_degc_cali, g_adc_cali_en);
		UTIL_Printf("   g_o_slope = 0x%x \n   g_o_slope_sign = 0x%x \n   g_id = 0x%x \n", 
		g_o_slope, g_o_slope_sign, g_id);	
		UTIL_Printf("   g_o_vtsmcu1 = 0x%x \n   g_o_vtsmcu2 = 0x%x \n   g_o_vtsmcu3 = 0x%x \n",
		g_o_vtsmcu1, g_o_vtsmcu2, g_o_vtsmcu3);
		#endif
		#endif
	}

#if OUTPUT_LOG
	UTIL_Printf("=> [Thermal calibration] == \n g_adc_ge = 0x%x ,\n g_adc_oe = 0x%x,\n g_degc_cali = 0x%x,\n g_adc_cali_en = 0x%x,\n", 
		g_adc_ge, g_adc_oe, g_degc_cali, g_adc_cali_en);
	UTIL_Printf(" g_o_slope = 0x%x,\n g_o_slope_sign = 0x%x,\n g_id = 0x%x\n", 
		g_o_slope, g_o_slope_sign, g_id);	
	UTIL_Printf(" g_o_vtsmcu1 = 0x%x,\n g_o_vtsmcu2 = 0x%x,\n g_o_vtsmcu3 = 0x%x \n",
		g_o_vtsmcu1, g_o_vtsmcu2, g_o_vtsmcu3);
#endif

	return;
}

#if 0
void vDrvThermal_Cal_Prepare_2_test(void)
{
	kal_int32 format_1, format_2, format_3, format_4= 0;

	//  [FT] ADC_GE[9:0] = GE*4096 + 512 (round to integer)-(1)
	//  [FT] ADC_OE[9:0] = OE*4096 + 512 (round to integer)-(2)
	g_ge = ((g_adc_ge - 512) * 10000 ) / 4096; // ge * 10000
	g_oe = (g_adc_oe - 512);
	g_gain = (10000 + g_ge); // gain * 10000

	// [FT] O_VTSMCU1=Y_VTS1-3192
	// Ideal ADC Value * ADC_Gain + ADC offset  = ADC Value.... (3)
	// Ideal ADC Value * ADC_Gain = ADC Value - ADC offset
	// format_X = Ideal ADC Value * ADC_Gain = (g_o_vtsmcu1 + TS_LOW_CRITERIAL) - (g_oe)
	format_1 = (g_o_vtsmcu1 + TS_LOW_CRITERIAL - g_oe);
	format_2 = (g_o_vtsmcu2 + TS_LOW_CRITERIAL - g_oe);
	format_3 = (g_o_vtsmcu3 + TS_LOW_CRITERIAL - g_oe);
	format_4 = (g_o_vtsmcu3 + TS_LOW_CRITERIAL - g_oe);

	// TSUV_S1 / 2.8V = Ideal ADC Value / 4096 .... (2)
	// g_x_roomt[0]=TSUV_S1 / 2.8V = Ideal ADC Value / 4096 = format_X/ADC_Gain/4096
	g_x_roomt[0] = (((format_1 * 10000) / 4096) * 10000) / g_gain; // x_roomt * 10000
	g_x_roomt[1] = (((format_2 * 10000) / 4096) * 10000) / g_gain; // x_roomt * 10000
	g_x_roomt[2] = (((format_3 * 10000) / 4096) * 10000) / g_gain; // x_roomt * 10000
	g_x_roomt[3] = (((format_4 * 10000) / 4096) * 10000) / g_gain; // x_roomt * 10000

#if OUTPUT_LOG
	UTIL_Printf("=> g_ge = %d, g_oe = %d, g_gain = %d, g_x_roomt1 = %d, g_x_roomt2 = %d, g_x_roomt3 = %d, g_x_roomt4 = %d\n",
		g_ge, g_oe, g_gain, g_x_roomt[0], g_x_roomt[1], g_x_roomt[2], g_x_roomt[3]);
#endif

	// vDrvGetEFuse_ThermalSesnorData();
	return;
}

static void raw_to_temperature_roomt_test(UINT32 raw[], UINT32 temp[])
{
	#if 0
	int y_curr_1;
	int tmp = 0;
	int Vbe_curr_1 = 0;
	int Vbe_curr_2 = 0;
	int Vbe_curr_3 = 0;
	int Vbe_curr_4 = 0;
	int Vbe_curr_5 = 0;
	int Vbe_curr = 0;

	int sign = 0;

	int Y_vts_tmp_1 = 0;
	int Y_vts_tmp_2 = 0;
	int Y_vts_tmp_3 = 0;
	int Y_vts_tmp_4 = 0;
	int Y_vts_tmp_5 = 0;
	int Y_vts_tmp = 0;

	int T_Current_1 = 0;
	int T_Current = 0;
	#else
	long long y_curr_1;
	//long long tmp = 0;
	long long Vbe_curr_1 = 0;
	long long Vbe_curr_2 = 0;
	long long Vbe_curr_3 = 0;
	long long Vbe_curr_4 = 0;
	long long Vbe_curr_5 = 0;
	long long Vbe_curr = 0;

	long long sign = 0;

	long long Y_vts_tmp_1 = 0;
	long long Y_vts_tmp_2 = 0;
	long long Y_vts_tmp_3 = 0;
	long long Y_vts_tmp_4 = 0;
	long long Y_vts_tmp_5 = 0;
	long long Y_vts_tmp = 0;

	long long T_Current_1 = 0;
	long long T_Current = 0;

	#endif

	UTIL_Printf("=> 1 To Temp: raw[0]= [0x%lx] (%ld) \n", raw[0], raw[0]);

	y_curr_1 = (long long)raw[0];
	y_curr_1 = y_curr_1 * 10000;
	UTIL_Printf("  y_curr_1= 0x%lx (%ld) \n", y_curr_1, y_curr_1);

	Vbe_curr_5 = g_oe * 4096;
	Vbe_curr_1 = y_curr_1 - Vbe_curr_5;
	UTIL_Printf("  Vbe_curr_1= 0x%lx (%ld) \n", Vbe_curr_1, Vbe_curr_1);
	
	Vbe_curr_2 = (g_ge + 10000) * 4096;
	UTIL_Printf("  Vbe_curr_2= 0x%lx (%ld) \n", Vbe_curr_2, Vbe_curr_2);

	Vbe_curr_3 = Vbe_curr_1 * 28000;
	UTIL_Printf("  Vbe_curr_3= 0x%lx (%ld) \n", Vbe_curr_3, Vbe_curr_3);

	Vbe_curr_4 = Vbe_curr_2 * 32;
	UTIL_Printf("  Vbe_curr_4= 0x%lx (%ld) \n", Vbe_curr_4, Vbe_curr_4);
	Vbe_curr = Vbe_curr_3 / Vbe_curr_4;
	UTIL_Printf("  Vbe_curr= 0x%lx (%ld) \n", Vbe_curr, Vbe_curr);


	Y_vts_tmp_1 = Vbe_curr - g_x_roomt[0];
	UTIL_Printf("  Y_vts_tmp_1= 0x%lx (%ld) \n", Y_vts_tmp_1, Y_vts_tmp_1);
	Y_vts_tmp_2 = Y_vts_tmp_1 * 100000;
	UTIL_Printf("  Y_vts_tmp_2= 0x%lx (%ld) \n", Y_vts_tmp_2, Y_vts_tmp_2);

	if (g_o_slope_sign == 1) {
		sign = -1;
	} else if (g_o_slope_sign == 0) {
		sign = 1;
	}

	Y_vts_tmp_3 = sign * g_o_slope;
	UTIL_Printf("  Y_vts_tmp_3= 0x%lx (%ld) \n", Y_vts_tmp_3, Y_vts_tmp_3);
	Y_vts_tmp_4 = Y_vts_tmp_3 + 165;
	UTIL_Printf("  Y_vts_tmp_4= 0x%lx (%ld) \n", Y_vts_tmp_4, Y_vts_tmp_4);
	Y_vts_tmp_5 = 0-Y_vts_tmp_4;
	UTIL_Printf("  Y_vts_tmp_5= 0x%lx (%ld) \n", Y_vts_tmp_5, Y_vts_tmp_5);

	Y_vts_tmp = Y_vts_tmp_2 / Y_vts_tmp_5;
	UTIL_Printf("  Y_vts_tmp= 0x%lx (%ld) \n", Y_vts_tmp, Y_vts_tmp);

	T_Current_1 = g_degc_cali /2;
	UTIL_Printf("  T_Current_1= 0x%lx (%ld) \n", T_Current_1, T_Current_1);
	T_Current = T_Current_1 + Y_vts_tmp;
	UTIL_Printf("  T_Current= 0x%lx (%ld) \n", T_Current, T_Current);

	temp[0] = T_Current;
	
	return;
}

static void raw_to_temperature_roomt_test_1(UINT32 raw[], UINT32 temp[])
{
	#if 0
	int y_curr_1;
	int tmp = 0;
	int Vbe_curr_1 = 0;
	int Vbe_curr_2 = 0;
	int Vbe_curr_3 = 0;
	int Vbe_curr_4 = 0;
	int Vbe_curr_5 = 0;
	int Vbe_curr = 0;

	int sign = 0;

	int Y_vts_tmp_1 = 0;
	int Y_vts_tmp_2 = 0;
	int Y_vts_tmp_3 = 0;
	int Y_vts_tmp_4 = 0;
	int Y_vts_tmp_5 = 0;
	int Y_vts_tmp = 0;

	int T_Current_1 = 0;
	int T_Current = 0;
	#else
	long long y_curr_1;
	//long long tmp = 0;
	long long Vbe_curr_1 = 0;
	long long Vbe_curr_2 = 0;
	long long Vbe_curr_3 = 0;
	long long Vbe_curr_4 = 0;
	long long Vbe_curr_5 = 0;
	long long Vbe_curr = 0;

	long long sign = 0;

	long long Y_vts_tmp_1 = 0;
	long long Y_vts_tmp_2 = 0;
	long long Y_vts_tmp_3 = 0;
	long long Y_vts_tmp_4 = 0;
	long long Y_vts_tmp_5 = 0;
	long long Y_vts_tmp = 0;

	long long T_Current_1 = 0;
	long long T_Current = 0;

	#endif

	UTIL_Printf("=> 2 To Temp: raw[0]= [0x%lx] (%ld) \n", raw[0], raw[0]);

	y_curr_1 = (long long)raw[0];
	UTIL_Printf("  y_curr_1= 0x%lx (%ld) \n", y_curr_1, y_curr_1);

	Vbe_curr_1 = y_curr_1 - g_adc_oe + 512;
	UTIL_Printf("  Vbe_curr_1= 0x%lx (%ld) \n", Vbe_curr_1, Vbe_curr_1);

	Vbe_curr_2 = Vbe_curr_1 * 2800;
	UTIL_Printf("  Vbe_curr_2= 0x%lx (%ld) \n", Vbe_curr_2, Vbe_curr_2);

	Vbe_curr_3 = 4096 + g_adc_ge -512;
	UTIL_Printf("  Vbe_curr_3= 0x%lx (%ld) \n", Vbe_curr_3, Vbe_curr_3);

	Vbe_curr_4 = Vbe_curr_3 * 32;
	UTIL_Printf("  Vbe_curr_4= 0x%lx (%ld) \n", Vbe_curr_4, Vbe_curr_4);
	
	Vbe_curr = Vbe_curr_2 / Vbe_curr_4;
	UTIL_Printf("  Vbe_curr= 0x%lx (%ld) \n", Vbe_curr, Vbe_curr);



	Y_vts_tmp_1 = Vbe_curr - g_o_vtsmcu1 - 3192;
	UTIL_Printf("  Y_vts_tmp_1= 0x%lx (%ld) \n", Y_vts_tmp_1, Y_vts_tmp_1);

	Y_vts_tmp_2 = Y_vts_tmp_1 * 100000;
	UTIL_Printf("  Y_vts_tmp_2= 0x%lx (%ld) \n", Y_vts_tmp_2, Y_vts_tmp_2);

	if (g_o_slope_sign == 1) {
		sign = -1;
	} else if (g_o_slope_sign == 0) {
		sign = 1;
	}

	Y_vts_tmp_3 = sign * g_o_slope;
	UTIL_Printf("  Y_vts_tmp_3= 0x%lx (%ld) \n", Y_vts_tmp_3, Y_vts_tmp_3);

	Y_vts_tmp_4 = Y_vts_tmp_3 + 165;
	UTIL_Printf("  Y_vts_tmp_4= 0x%lx (%ld) \n", Y_vts_tmp_4, Y_vts_tmp_4);

	Y_vts_tmp_5 = 0-Y_vts_tmp_4;
	UTIL_Printf("  Y_vts_tmp_5= 0x%lx (%ld) \n", Y_vts_tmp_5, Y_vts_tmp_5);

	Y_vts_tmp = Y_vts_tmp_2 / Y_vts_tmp_5;
	UTIL_Printf("  Y_vts_tmp= 0x%lx (%ld) \n", Y_vts_tmp, Y_vts_tmp);

	T_Current_1 = g_degc_cali /2;
	UTIL_Printf("  T_Current_1= 0x%lx (%ld) \n", T_Current_1, T_Current_1);

	T_Current = T_Current_1 + Y_vts_tmp;
	UTIL_Printf("  T_Current= 0x%lx (%ld) \n", T_Current, T_Current);

	temp[0] = T_Current;
	
	return;
}

#endif

void vDrvThermal_Cal_Prepare_2(void)
{
	kal_int32 format_1, format_2, format_3, format_4= 0;

	//  [FT] ADC_GE[9:0] = GE*4096 + 512 (round to integer)-(1)
	//  [FT] ADC_OE[9:0] = OE*4096 + 512 (round to integer)-(2)
	g_ge = ((g_adc_ge - 512) * 10000 ) / 4096; // ge * 10000
	g_oe = (g_adc_oe - 512);
	g_gain = (10000 + g_ge); // gain * 10000

	// [FT] O_VTSMCU1=Y_VTS1-3192
	// Ideal ADC Value * ADC_Gain + ADC offset  = ADC Value.... (3)
	// Ideal ADC Value * ADC_Gain = ADC Value - ADC offset
	// format_X = Ideal ADC Value * ADC_Gain = (g_o_vtsmcu1 + TS_LOW_CRITERIAL) - (g_oe)
	format_1 = (g_o_vtsmcu1 + TS_LOW_CRITERIAL - g_oe);
	format_2 = (g_o_vtsmcu2 + TS_LOW_CRITERIAL - g_oe);
	format_3 = (g_o_vtsmcu3 + TS_LOW_CRITERIAL - g_oe);
	format_4 = (g_o_vtsmcu3 + TS_LOW_CRITERIAL - g_oe);

	// TSUV_S1 / 2.8V = Ideal ADC Value / 4096 .... (2)
	// g_x_roomt[0]=TSUV_S1 / 2.8V = Ideal ADC Value / 4096 = format_X/ADC_Gain/4096
	g_x_roomt[0] = (((format_1 * 10000) / 4096) * 10000) / g_gain; // x_roomt * 10000
	g_x_roomt[1] = (((format_2 * 10000) / 4096) * 10000) / g_gain; // x_roomt * 10000
	g_x_roomt[2] = (((format_3 * 10000) / 4096) * 10000) / g_gain; // x_roomt * 10000
	g_x_roomt[3] = (((format_4 * 10000) / 4096) * 10000) / g_gain; // x_roomt * 10000

#if OUTPUT_LOG
	UTIL_Printf("=> g_ge = %d, g_oe = %d, g_gain = %d, g_x_roomt1 = %d, g_x_roomt2 = %d, g_x_roomt3 = %d, g_x_roomt4 = %d\n",
		g_ge, g_oe, g_gain, g_x_roomt[0], g_x_roomt[1], g_x_roomt[2], g_x_roomt[3]);
#endif

	// vDrvGetEFuse_ThermalSesnorData();
	return;
}

static void raw_to_temperature_roomt_new(UINT32 raw[], UINT32 temp[])
{
	int i = 0;
	int y_curr[4];
	int format_1 = 0;
	int format_2[4];
	int format_3[4];
	int format_4[4];

	for (i=0; i<4; i++)
	{
		y_curr[i] = raw[i];
		format_2[i] = 0;
		format_3[i] = 0;
		format_4[i] = 0;
	}

	#if OUTPUT_LOG
	UTIL_Printf("[Power/CPU_Thermal]: g_ge = %d, g_oe = %d, g_gain = %d, g_x_roomt1 = %d, g_x_roomt2 = %d, g_x_roomt3 = %d, g_x_roomt4 = %d\n",
		g_ge, g_oe, g_gain, g_x_roomt[0], g_x_roomt[1], g_x_roomt[2], g_x_roomt[3]);
	#endif
	
	format_1 = (g_degc_cali / 2);
	
	#if OUTPUT_LOG
	UTIL_Printf("[Power/CPU_Thermal]: format_1=%d\n", format_1);
	#endif

	for (i=0; i<4; i++)
	{
		// Ideal ADC Value * ADC_Gain = ADC Value - ADC offset
		format_2[i] = (y_curr[i] - g_oe);
		// TSUV_S1 / 2.8V = Ideal ADC Value / 4096 = format_X/ADC_Gain/4096
		// Delta g_x_roomt[0]= Ideal ADC Value_T1 - Ideal ADC Value_T0
		format_3[i] = (((((format_2[i]) * 10000) / 4096) * 10000) / g_gain) - g_x_roomt[i]; // 10000 * format3
		// TSUV * 3.2X = TSUV_S1.... (1)
		// format_3[i] = Delta TSUV = delta Ideal ADC value /3.2*2.8
		format_3[i] = (format_3[i] * TS_ADC_INPUT_RANGE) / TS_BUFFER_GAIN;
		//T1=T0+(C1-C0)*G/S.... (0) Note: S=-1.65mV/degC 
		// Delta V=G*DeltaC
		// Delta T = Delta V /S
		if(g_o_slope_sign==0)
		{
			format_4[i] = ((format_3[i] * 100) / (165+g_o_slope)); // uint = 0.1 deg
		}
		else
		{
			format_4[i] = ((format_3[i] * 100) / (165-g_o_slope)); // uint = 0.1 deg
		}
		format_4[i] = format_4[i] - (2 * format_4[i]);

		// Current tempature = Environment tempature + Delta T
		if(y_curr[i] == 0)
		{
			temp[i] = 0;
		}
		else
		{
			temp[i] = (format_1 * 10) + format_4[i]; // uint = 0.1 deg
		}
		#if OUTPUT_LOG
		if(i==0)
			UTIL_Printf("=== [ADC Value] [Delta ADC Value] [Delta Tempauure(0.1 deg)] [Tempauure(0.1 deg)]\n");			
		UTIL_Printf("format_2[%d]=%d, format_3[%d]=%5d, format_4[%d]=%5d, temp[%d]=%5d\n", i, format_2[i], i, format_3[i],i, format_4[i], i, temp[i]);
		#endif
	}

	return;	
}

static UINT32 raw_to_temperature_roomt_ts(UINT32 raw, UINT32 ts_idx)
{
	int i = 0;
	UINT32 temp = 0;
	int format_1 = 0;
	int y_curr;
	int format_2;
	int format_3;
	int format_4;

	if (ts_idx > 3) {
		return 0;
	}

	y_curr = raw;
	format_2 = 0;
	format_3 = 0;
	format_4 = 0;

	#if OUTPUT_LOG
	UTIL_Printf("[Power/CPU_Thermal]: idx=%d, y_curr=0x%x \n", ts_idx, y_curr);
	#endif

	#if OUTPUT_LOG
	UTIL_Printf("[Power/CPU_Thermal]: g_ge = %d, g_oe = %d, g_gain = %d, g_x_roomt1 = %d, g_x_roomt2 = %d, g_x_roomt3 = %d, g_x_roomt4 = %d\n",
		g_ge, g_oe, g_gain, g_x_roomt[0], g_x_roomt[1], g_x_roomt[2], g_x_roomt[3]);
	#endif
	
	format_1 = (g_degc_cali / 2);
	
	#if OUTPUT_LOG
	UTIL_Printf("[Power/CPU_Thermal]: format_1=%d\n", format_1);
	#endif


	// Ideal ADC Value * ADC_Gain = ADC Value - ADC offset
	format_2 = (y_curr - g_oe);


	// TSUV_S1 / 2.8V = Ideal ADC Value / 4096 = format_X/ADC_Gain/4096
	// Delta g_x_roomt[0]= Ideal ADC Value_T1 - Ideal ADC Value_T0
	format_3 = (((((format_2) * 10000) / 4096) * 10000) / g_gain) - g_x_roomt[ts_idx]; // 10000 * format3


	// TSUV * 3.2X = TSUV_S1.... (1)
	// format_3[i] = Delta TSUV = delta Ideal ADC value /3.2*2.8
	format_3 = (format_3 * TS_ADC_INPUT_RANGE) / TS_BUFFER_GAIN;


	//T1=T0+(C1-C0)*G/S.... (0) Note: S=-1.65mV/degC 
	// Delta V=G*DeltaC
	// Delta T = Delta V /S
	if(g_o_slope_sign==0)
	{
		format_4 = ((format_3 * 100) / (165+g_o_slope)); // uint = 0.1 deg
	}
	else
	{
		format_4 = ((format_3 * 100) / (165-g_o_slope)); // uint = 0.1 deg
	}
	format_4 = format_4 - (2 * format_4);


	// Current tempature = Environment tempature + Delta T
	if(y_curr == 0)
	{
		temp = 0;
	}
	else
	{
		temp = (format_1 * 10) + format_4; // uint = 0.1 deg
	}
	

	#if OUTPUT_LOG
	UTIL_Printf("temp=%5d\n", temp);
	#endif

	return temp;	
}

// test read temp
#if 1
extern volatile int temp_flag;
extern volatile int temp_interval;

int init_thermal_hw(void)
{
	thermal_real_test = true;
		
	// enable all adc channel for HW mode 
	DRV_WriteReg32(PDWNC_SRVCFG1, 0x8887FFFF);				// counting unit is 320 * 31.25us = 10ms
	
	// set TSADC hw mode
	DRV_WriteReg32(PDWNC_SRVCFG0, 0x01);

	// set interval
	DRV_WriteReg32(TEMPMONCTL1, 0x00000004); // 3FF);	  // counting unit is 1024 / 66M = 15.5us
	DRV_WriteReg32(TEMPMONCTL2, 0x00500050); // 219);	 // sensing interval is 537 * 15.5us = 8.3235ms
	DRV_WriteReg32(TEMPAHBPOLL,  0x00000040);
	DRV_WriteReg32(TEMPAHBTO,	  0x00000FFF);

	// times for interrupt occurrance
	DRV_WriteReg32(TEMPMONIDET0, 0x00000000);				 // times=1 for interrupt occurrance of sensing point 0
	DRV_WriteReg32(TEMPMONIDET1, 0x00000000);				 // times=1 for interrupt occurrance of sensing point 1
	DRV_WriteReg32(TEMPMONIDET2, 0x00000000);				 // times=1 for interrupt occurrance of sensing point 2

	// set all threshold
	DRV_WriteReg32(TEMPHTHRE, 0x00000100);					// set hot threshold
	DRV_WriteReg32(TEMPOFFSETH, 0x00000200);				// set high offset threshold
	DRV_WriteReg32(TEMPH2NTHRE, 0x00000500);				// set hot to normal threshold
	DRV_WriteReg32(TEMPOFFSETL, 0x0000A00); 			   // set low offset threshold
	DRV_WriteReg32(TEMPCTHRE, 0x00000B00);					// set cold threshold

	/// set sample mode 
	// one sampling 		--> 0x00000000
	// average 2 sampling  --> 0x00000049
	// 4 sampling  		--> 0x000000A2
	// 6 sampling  		--> 0x000000DB
	// 10 sampling  		-->  0x0000124
	// 18 sampling  		--> 0x000016D
	DRV_WriteReg32(TEMPMSRCTL0, 0x000016D);				 // temperature measurement sampling control (one sampling)

	DRV_WriteReg32(TEMPADCPNP0, 0x172);
	DRV_WriteReg32(TEMPADCPNP1, 0x1B2);
	DRV_WriteReg32(TEMPADCPNP2, 0x1F2);

	DRV_WriteReg32(TEMPADCMUX, 0x11);		 
	DRV_WriteReg32(TEMPADCEXT, 0x22);		  
	DRV_WriteReg32(TEMPADCEXT1, 0x33); 			 

	DRV_WriteReg32(TEMPADCEN, 0x01);

	DRV_WriteReg32(TEMPPNPMUXADDR, 0xf00243fc);
	DRV_WriteReg32(TEMPADCENADDR,  0xf0024620); 
	DRV_WriteReg32(TEMPADCVALIDADDR, 0xf0024634);
	DRV_WriteReg32(TEMPADCVOLTADDR, 0xf00243d4);

	DRV_WriteReg32(TEMPRDCTRL, 0x1);					  // read valid & voltage are at the same register
	DRV_WriteReg32(TEMPADCVALIDMASK, 0x00000022);			// indicate where the valid bit is (the 12th bit is valid bit and 1 is valid)
	DRV_WriteReg32(TEMPADCVOLTAGESHIFT, 0x0);				// do not need to shift
	DRV_WriteReg32(TEMPADCWRITECTRL, 0x1); 

	//pr_info("read TEMPMONINTSTS (1) = 0x%x \n", DRV_Reg32(TEMPMONINTSTS));

	// enable all filter interrupt
	DRV_WriteReg32(TEMPMONINT, 0x10388000);

	// enable perio mear on sensor 0,1,2,3
	DRV_WriteReg32(TEMPMONCTL0, 0x0000000F);

	return 0;
}

int tz_get_temp(void)
{
	UINT32 i = 0, index = 0;
	UINT32 curr_raw[4] = {0}, curr_temp[4] = {0};
	int max_temp = 0;

	//msleep(200);

	// sensor point 0
	#if 1
	i = 0;
	curr_raw[0] = DRV_Reg32(TEMPMSR0); 
	while ((curr_raw[0] & 0x8000) == 0) {
		curr_raw[0] = DRV_Reg32(TEMPMSR0);
		i++;
		if (i > 100) {
			UTIL_Printf("ERROR: read TEMPMSR0 fail !!! \n");
			break;
		}
	}
	curr_raw[0] = curr_raw[0] & 0x0FFF;
	#endif
	
	// sensor point 1
	#if 1
	i = 0;
	curr_raw[1] = DRV_Reg32(TEMPMSR1); 
	while ((curr_raw[1] & 0x8000) == 0) {
		curr_raw[1] = DRV_Reg32(TEMPMSR1);
		i++;
		if (i > 100) {
			UTIL_Printf("ERROR: read TEMPMSR1 fail !!! \n");
			break;
		}
	}
	curr_raw[1] = curr_raw[1] & 0x0FFF;
	#endif

	// sensor point 2
	#if 1
	i = 0;
	curr_raw[2] = DRV_Reg32(TEMPMSR2); 
	while ((curr_raw[2] & 0x8000) == 0) {
		curr_raw[2] = DRV_Reg32(TEMPMSR2);
		i++;
		if (i > 100) {
			UTIL_Printf("ERROR: read TEMPMSR2 fail !!! \n");
			break;
		}
	}
	curr_raw[2] = curr_raw[2] & 0x0FFF;
	#endif

	// sensor point 3
	#if 1
	i = 0;
	curr_raw[3] = DRV_Reg32(TEMPMSR3); 
	while ((curr_raw[3] & 0x8000) == 0) {
		curr_raw[3] = DRV_Reg32(TEMPMSR3);
		i++;
		if (i > 100) {
			UTIL_Printf("ERROR: read TEMPMSR3 fail !!! \n");
			break;
		}
	}
	curr_raw[3] = curr_raw[3] & 0x0FFF;
	#endif

	raw_to_temperature_roomt_new(curr_raw, curr_temp);

	curr_temp[0] = curr_temp[0]*100;
	curr_temp[1] = curr_temp[1]*100;
	curr_temp[2] = curr_temp[2]*100;
	curr_temp[3] = curr_temp[3]*100;

	max_temp = 0;
	for (i=0; i<4; i++)
	{
		if (max_temp < curr_temp[i])
		{
			max_temp = curr_temp[i];
			index = i;
		}
	}

	#if 1
	pr_debug("[thermal] \ntemp[0]=%6d \ntemp[1]=%6d \ntemp[2]=%6d \ntemp[3]=%6d\n", 
		curr_temp[0], curr_temp[1], curr_temp[2], curr_temp[3]);

	pr_debug("[thermal] \nraw[0]=0x%x \nraw[1]=0x%x \nraw[2]=0x%x \nraw[3]=0x%x\n", 
		curr_raw[0], curr_raw[1], curr_raw[2], curr_raw[3]);

	///#else
	pr_debug("[thermal] max_temp= %d.%d (C), sensor [%d]\n", max_temp/1000, max_temp%1000, index);
	#endif

	return max_temp;
}

UINT32 tz_raw_to_temp(UINT32 raw, UINT32 ts_idx)
{
	UINT32 temp = 0;

    temp = raw_to_temperature_roomt_ts(raw, ts_idx);
	UTIL_Printf("[thermal] tz_raw_to_temp, ts [%d]: raw [0x%x], temp [%d] \n", ts_idx, raw, temp);

	return temp;
}

static int test_get_hw_temp(void * pvArg)
{
	UINT32 max_temp = 0, i = 0, index = 0;
	UINT32 curr_raw[4] = {0}, curr_temp[4] = {0};

	while (1)
	{
		if (!temp_flag) {
			msleep(1000);
			continue;
		}

		#if 0
		msleep(2000);
		#else
		UTIL_Printf("[thermal] sleep %d (s) \n", temp_interval);
		msleep(temp_interval * 1000);
		#endif

		// sensor point 0
		#if 1
		curr_raw[0] = DRV_Reg32(TEMPMSR0); 
		while ((curr_raw[0] & 0x8000) == 0)
			curr_raw[0] = DRV_Reg32(TEMPMSR0);
		//UTIL_Printf("==> read TEMPMSR0 = 0x%x  %d \n", (curr_raw[0]), (curr_raw[0]));
		curr_raw[0] = curr_raw[0] & 0x0FFF;
		#endif
		
		// sensor point 1
		#if 1
		curr_raw[1] = DRV_Reg32(TEMPMSR1); 
		while ((curr_raw[1] & 0x8000) == 0)
			curr_raw[1] = DRV_Reg32(TEMPMSR1);
		//UTIL_Printf("read TEMPMSR1 = 0x%x  %d \n", (curr_raw[1]), (curr_raw[1]));
		curr_raw[1] = curr_raw[1] & 0x0FFF;
		#endif

		// sensor point 2
		#if 1
		curr_raw[2] = DRV_Reg32(TEMPMSR2); 
		while ((curr_raw[2] & 0x8000) == 0)
			curr_raw[2] = DRV_Reg32(TEMPMSR2);
		//UTIL_Printf("read TEMPMSR2 = 0x%x  %d \n", (curr_raw[2]), (curr_raw[2]));
		curr_raw[2] = curr_raw[2] & 0x0FFF;
		#endif

		// sensor point 3
		#if 1
		curr_raw[3] = DRV_Reg32(TEMPMSR3); 
		while ((curr_raw[3] & 0x8000) == 0)
			curr_raw[3] = DRV_Reg32(TEMPMSR3);
		//UTIL_Printf("read TEMPMSR3 = 0x%x  %d \n", (curr_raw[3]), (curr_raw[3]));
		curr_raw[3] = curr_raw[3] & 0x0FFF;
		#endif

		raw_to_temperature_roomt_new(curr_raw, curr_temp);

		curr_temp[0] = curr_temp[0]*100;
		curr_temp[1] = curr_temp[1]*100;
		curr_temp[2] = curr_temp[2]*100;
		curr_temp[3] = curr_temp[3]*100;

		max_temp = 0;
		for (i=0; i<4; i++)
		{
			if (max_temp < curr_temp[i])
			{
				max_temp = curr_temp[i];
				index = i;
			}
		}

		UTIL_Printf("\n");

		#if 0
		UTIL_Printf("[CPU_Temp] \ntemp[0]=%6d, raw[0]=%d,\ntemp[1]=%6d, raw[1]=%d,\ntemp[2]=%6d, raw[2]=%d,\ntemp[3]=%6d, raw[3]=%d\n", 
			curr_temp[0], curr_raw[0], curr_temp[1], curr_raw[1], curr_temp[2], curr_raw[2], curr_temp[3], curr_raw[3]);
		#else
		UTIL_Printf("[thermal] \ntemp[0]=%6d \ntemp[1]=%6d \ntemp[2]=%6d \ntemp[3]=%6d\n", 
			curr_temp[0], curr_temp[1], curr_temp[2], curr_temp[3]);
		#endif

		UTIL_Printf("[thermal] max_temp= %d.%d (C), sensor [%d]\n", max_temp/1000, max_temp%1000, index);
	}

	UTIL_Printf("\n==== get tmep end ====\n\n");

	complete_and_exit(NULL, 0);

	return 0;
}

static int get_hw_temp(void * pvArg)
{
	UINT32 max_temp = 0, i = 0, index = 0;
	UINT32 curr_raw[4] = {0}, curr_temp[4] = {0};

	init_thermal_hw();

	while (1)
	{
		#if 0
		if (!temp_flag) {
			msleep(100);
			continue;
		}
		#endif

		#if 1
		msleep(2000);
		#else
		UTIL_Printf("[thermal] sleep %d (s) \n", temp_interval);
		msleep(temp_interval * 1000);
		#endif

		// sensor point 0
		#if 1
		curr_raw[0] = DRV_Reg32(TEMPMSR0); 
		while ((curr_raw[0] & 0x8000) == 0)
			curr_raw[0] = DRV_Reg32(TEMPMSR0);
		//UTIL_Printf("==> read TEMPMSR0 = 0x%x  %d \n", (curr_raw[0]), (curr_raw[0]));
		curr_raw[0] = curr_raw[0] & 0x0FFF;
		#endif
		
		// sensor point 1
		#if 1
		curr_raw[1] = DRV_Reg32(TEMPMSR1); 
		while ((curr_raw[1] & 0x8000) == 0)
			curr_raw[1] = DRV_Reg32(TEMPMSR1);
		//UTIL_Printf("read TEMPMSR1 = 0x%x  %d \n", (curr_raw[1]), (curr_raw[1]));
		curr_raw[1] = curr_raw[1] & 0x0FFF;
		#endif

		// sensor point 2
		#if 1
		curr_raw[2] = DRV_Reg32(TEMPMSR2); 
		while ((curr_raw[2] & 0x8000) == 0)
			curr_raw[2] = DRV_Reg32(TEMPMSR2);
		//UTIL_Printf("read TEMPMSR2 = 0x%x  %d \n", (curr_raw[2]), (curr_raw[2]));
		curr_raw[2] = curr_raw[2] & 0x0FFF;
		#endif

		// sensor point 3
		#if 1
		curr_raw[3] = DRV_Reg32(TEMPMSR3); 
		while ((curr_raw[3] & 0x8000) == 0)
			curr_raw[3] = DRV_Reg32(TEMPMSR3);
		//UTIL_Printf("read TEMPMSR3 = 0x%x  %d \n", (curr_raw[3]), (curr_raw[3]));
		curr_raw[3] = curr_raw[3] & 0x0FFF;
		#endif

		raw_to_temperature_roomt_new(curr_raw, curr_temp);

		curr_temp[0] = curr_temp[0]*100;
		curr_temp[1] = curr_temp[1]*100;
		curr_temp[2] = curr_temp[2]*100;
		curr_temp[3] = curr_temp[3]*100;

		max_temp = 0;
		for (i=0; i<4; i++)
		{
			if (max_temp < curr_temp[i])
			{
				max_temp = curr_temp[i];
				index = i;
			}
		}

		UTIL_Printf("\n");

		#if 0
		UTIL_Printf("[CPU_Temp] \ntemp[0]=%6d, raw[0]=%d,\ntemp[1]=%6d, raw[1]=%d,\ntemp[2]=%6d, raw[2]=%d,\ntemp[3]=%6d, raw[3]=%d\n", 
			curr_temp[0], curr_raw[0], curr_temp[1], curr_raw[1], curr_temp[2], curr_raw[2], curr_temp[3], curr_raw[3]);
		#else
		UTIL_Printf("[thermal] \ntemp[0]=%6d \ntemp[1]=%6d \ntemp[2]=%6d \ntemp[3]=%6d\n", 
			curr_temp[0], curr_temp[1], curr_temp[2], curr_temp[3]);
		#endif

		UTIL_Printf("[thermal] max_temp= %d.%d (C), sensor [%d]\n", max_temp/1000, max_temp%1000, index);
	}

	UTIL_Printf("\n==== get tmep end ====\n\n");

	complete_and_exit(NULL, 0);

	return 0;
}
//#else
int mtktscpu_get_hw_temp(void)
{
	UINT32 max_temp = 0, i = 0, times = 0, index = 0;
	UINT32 curr_raw[4] = {0}, curr_temp[4] = {0};
		
	thermal_real_test = true;

	//UTIL_Printf("mtktscpu_get_hw_temp: start \n");
		
	// enable all adc channel for HW mode 
	DRV_WriteReg32(PDWNC_SRVCFG1, 0x8887FFFF);				// counting unit is 320 * 31.25us = 10ms
	
	// set TSADC hw mode
	DRV_WriteReg32(PDWNC_SRVCFG0, 0x01);

	// set interval
	DRV_WriteReg32(TEMPMONCTL1, 0x00000004); // 3FF);	  // counting unit is 1024 / 66M = 15.5us
	DRV_WriteReg32(TEMPMONCTL2, 0x00500050); // 219);	 // sensing interval is 537 * 15.5us = 8.3235ms
	DRV_WriteReg32(TEMPAHBPOLL,  0x00000040);
	DRV_WriteReg32(TEMPAHBTO,	  0x00000FFF);

	// times for interrupt occurrance
	DRV_WriteReg32(TEMPMONIDET0, 0x00000000);				 // times=1 for interrupt occurrance of sensing point 0
	DRV_WriteReg32(TEMPMONIDET1, 0x00000000);				 // times=1 for interrupt occurrance of sensing point 1
	DRV_WriteReg32(TEMPMONIDET2, 0x00000000);				 // times=1 for interrupt occurrance of sensing point 2

	// set all threshold
	DRV_WriteReg32(TEMPHTHRE, 0x00000100);					// set hot threshold
	DRV_WriteReg32(TEMPOFFSETH, 0x00000200);				// set high offset threshold
	DRV_WriteReg32(TEMPH2NTHRE, 0x00000500);				// set hot to normal threshold
	DRV_WriteReg32(TEMPOFFSETL, 0x0000A00); 			   // set low offset threshold
	DRV_WriteReg32(TEMPCTHRE, 0x00000B00);					// set cold threshold

	/// set sample mode 
	// one sampling 		--> 0x00000000
	// average 2 sampling  --> 0x00000049
	// 4 sampling  		--> 0x000000A2
	// 6 sampling  		--> 0x000000DB
	// 10 sampling  		-->  0x0000124
	// 18 sampling  		--> 0x000016D
	DRV_WriteReg32(TEMPMSRCTL0, 0x000016D);				 // temperature measurement sampling control (one sampling)

	DRV_WriteReg32(TEMPADCPNP0, 0x172);
	DRV_WriteReg32(TEMPADCPNP1, 0x1B2);
	DRV_WriteReg32(TEMPADCPNP2, 0x1F2);

	DRV_WriteReg32(TEMPADCMUX, 0x11);		 
	DRV_WriteReg32(TEMPADCEXT, 0x22);		  
	DRV_WriteReg32(TEMPADCEXT1, 0x33); 			 

	DRV_WriteReg32(TEMPADCEN, 0x01);

	DRV_WriteReg32(TEMPPNPMUXADDR, 0xf00243fc);
	DRV_WriteReg32(TEMPADCENADDR,  0xf0024620); 
	DRV_WriteReg32(TEMPADCVALIDADDR, 0xf0024634);
	DRV_WriteReg32(TEMPADCVOLTADDR, 0xf00243d4);

	DRV_WriteReg32(TEMPRDCTRL, 0x1);					  // read valid & voltage are at the same register
	DRV_WriteReg32(TEMPADCVALIDMASK, 0x00000022);			// indicate where the valid bit is (the 12th bit is valid bit and 1 is valid)
	DRV_WriteReg32(TEMPADCVOLTAGESHIFT, 0x0);				// do not need to shift
	DRV_WriteReg32(TEMPADCWRITECTRL, 0x1); 

	//UTIL_Printf("read TEMPMONINTSTS (1) = 0x%x \n", DRV_Reg32(TEMPMONINTSTS));

	// enable all filter interrupt
	DRV_WriteReg32(TEMPMONINT, 0x10388000);

	// enable perio mear on sensor 0,1,2,3
	DRV_WriteReg32(TEMPMONCTL0, 0x0000000F);

	//for (j = 0; j <= 10; j++)
	{
		times = 1;
		while (times--)
		//while (1)
		{
			for (i=0;i<10;i++)
			{
				//// kal_sleep_task(10000);
				kal_sleep_task(1000);
			}

			//times++;
			UTIL_Printf("==> read temp, times = %d \n", (times));
			
			//UTIL_Printf("read TEMPMONINTSTS (2) = 0x%x \n", DRV_Reg32(TEMPMONINTSTS));

			// sensor point 0
			#if 1
			curr_raw[0] = DRV_Reg32(TEMPMSR0); 
			while ((curr_raw[0] & 0x8000) == 0)
				curr_raw[0] = DRV_Reg32(TEMPMSR0);
			//UTIL_Printf("==> read TEMPMSR0 = 0x%x  %d \n", (curr_raw[0]), (curr_raw[0]));
			curr_raw[0] = curr_raw[0] & 0x0FFF;
			#endif
			
			// sensor point 1
			#if 1
			curr_raw[1] = DRV_Reg32(TEMPMSR1); 
			while ((curr_raw[1] & 0x8000) == 0)
				curr_raw[1] = DRV_Reg32(TEMPMSR1);
			//UTIL_Printf("read TEMPMSR1 = 0x%x  %d \n", (curr_raw[1]), (curr_raw[1]));
			curr_raw[1] = curr_raw[1] & 0x0FFF;
			#endif

			// sensor point 2
			#if 1
			curr_raw[2] = DRV_Reg32(TEMPMSR2); 
			while ((curr_raw[2] & 0x8000) == 0)
				curr_raw[2] = DRV_Reg32(TEMPMSR2);
			//UTIL_Printf("read TEMPMSR2 = 0x%x  %d \n", (curr_raw[2]), (curr_raw[2]));
			curr_raw[2] = curr_raw[2] & 0x0FFF;
			#endif

			// sensor point 3
			#if 1
			curr_raw[3] = DRV_Reg32(TEMPMSR3); 
			while ((curr_raw[3] & 0x8000) == 0)
				curr_raw[3] = DRV_Reg32(TEMPMSR3);
			//UTIL_Printf("read TEMPMSR3 = 0x%x  %d \n", (curr_raw[3]), (curr_raw[3]));
			curr_raw[3] = curr_raw[3] & 0x0FFF;
			#endif
			
			//temp = (curr_raw[0]+curr_raw[1]+curr_raw[2]+curr_raw[3])/4;
			//UTIL_Printf("got raw MSR0 = 0x%x (%d), MSR1 = 0x%x (%d), MSR2 = 0x%x (%d), MSR3 = 0x%x (%d) \n", curr_raw[0], curr_raw[0], curr_raw[1], curr_raw[1], curr_raw[2], curr_raw[2], curr_raw[3], curr_raw[3]);
			//UTIL_Printf("average raw = 0x%x (%d) \n", temp, temp);

			//for (i=0;i<100;i++)
			{
				//kal_sleep_task(10000);
			}
		}
	}

	raw_to_temperature_roomt_new(curr_raw, curr_temp);

	curr_temp[0] = curr_temp[0]*100;
	curr_temp[1] = curr_temp[1]*100;
	curr_temp[2] = curr_temp[2]*100;
	curr_temp[3] = curr_temp[3]*100;

	max_temp = 0;
	for (i=0; i<4; i++)
	{
		if (max_temp < curr_temp[i])
		{
			max_temp = curr_temp[i];
			index = i;
		}
	}

	UTIL_Printf("\n");

	UTIL_Printf("[CPU_Temp] \ntemp[0]=%6d, raw[0]=%d,\ntemp[1]=%6d, raw[1]=%d,\ntemp[2]=%6d, raw[2]=%d,\ntemp[3]=%6d, raw[3]=%d\n", 
		curr_temp[0], curr_raw[0], curr_temp[1], curr_raw[1], curr_temp[2], curr_raw[2], curr_temp[3], curr_raw[3]);
	//UTIL_Printf("[CPU_Temp] max_temp(0.001 degree)=%d, ts_index=%d\n", max_temp, index);
	UTIL_Printf("[CPU_Temp] max_temp= %d.%d (C), ts_index=%d\n", max_temp/1000, max_temp%1000, index);

	return max_temp;
}
#endif
#endif

int thermal_adc_hw_auto_mode(unsigned int channel)
{
	UINT32 temp = 0, i = 0;

	// clear ADC output data and channel status
	temp = DRV_Reg32(PDWNC_SRVCLR);
	temp = temp | 0x02;
	DRV_WriteReg32(PDWNC_SRVCLR, temp);

	// polling clear to be done
	temp = DRV_Reg32(PDWNC_SRVCLR); 
	while ((temp & 0x0002) != 0){
			temp = DRV_Reg32(PDWNC_SRVCLR);
	}
	UTIL_Printf("set HW mode \n");

	#if 0
	// enable timeout
	temp = DRV_Reg32(PDWNC_SRVTOTEN);
	temp = temp | 0x01;
	DRV_WriteReg32(PDWNC_SRVTOTEN, temp);
	#endif

	// open channel x
	DRV_WriteReg32(PDWNC_SRVCFG1, channel);
	UTIL_Printf("select channel = 0x%x \n", (channel)); 
	temp = DRV_Reg32(PDWNC_SRVCFG1);
	//UTIL_Printf("PDWNC_SRVCFG1 = 0x%x \n", (temp)); 
	
	// set rate
	#if 0
	DRV_WriteReg32(PDWNC_SRVRAT, 0x000F0120);
	#elif 0
	DRV_WriteReg32(PDWNC_SRVRAT, 0x00070120);
	#elif 1
	//DRV_WriteReg32(PDWNC_SRVRAT, 0x00120120);
	DRV_WriteReg32(PDWNC_SRVRAT, 0x000F0120);
	#endif

	// enable HW auto mode
	temp = DRV_Reg32(PDWNC_SRVCFG0);
	temp = temp & 0xFFFFFFF0;
	temp = temp | 0x01;
	DRV_WriteReg32(PDWNC_SRVCFG0, temp);
	
	// read PDWNC_SRVTOTEN
	temp = DRV_Reg32(PDWNC_SRVTOTEN); 
	while ((temp & 0x0004) == 0)
			temp = DRV_Reg32(PDWNC_SRVTOTEN);
	UTIL_Printf("read PDWNC_SRVTOTEN = 0x%x \n", (temp));  
	
	// print reg
	UTIL_Printf("TSADC channel OUT x: \n"); 
	for (i=0;i<16;i++) {
		temp = DRV_Reg32(PDWNC_ADOUT0 + (i*4));
		UTIL_Printf("TSADC_OUT0 + %d = 0x%x \n", i, temp); 
	}

	for (i=0;i<100;i++)
	{
		kal_sleep_task(10000);
	}
	
	// print reg
	UTIL_Printf("TSADC channel OUT x: \n"); 
	for (i=0;i<16;i++) {
		temp = DRV_Reg32(PDWNC_ADOUT0 + (i*4));
		UTIL_Printf("TSADC_OUT0 + %d = 0x%x \n", i, temp); 
	}
	
	for (i=0;i<100;i++)
	{
		kal_sleep_task(10000);
	}
	
	// print reg
	UTIL_Printf("TSADC channel OUT x: \n"); 
	for (i=0;i<16;i++) {
		temp = DRV_Reg32(PDWNC_ADOUT0 + (i*4));
		UTIL_Printf("TSADC_OUT0 + %d = 0x%x \n", i, temp); 
	}

	UTIL_Printf("thermal_adc_hw_auto_mode: end \n"); 

	return 0;
}

int thermal_adc_sw_trigger_mode(unsigned int channel)
{
	unsigned int temp = 0, i = 0, j = 0;
	unsigned int cha = 0;

	// clear ADC output data and channel status
	temp = DRV_Reg32(PDWNC_SRVCLR);
	temp = temp | 0x02;
	DRV_WriteReg32(PDWNC_SRVCLR, temp);

	#if 0
	// enable timeout
	temp = DRV_Reg32(PDWNC_SRVTOTEN);
	temp = temp | 0x01;
	DRV_WriteReg32(PDWNC_SRVTOTEN, temp);
	#endif

	// select channel for SW mode
	temp = DRV_Reg32(PDWNC_SRVCFG0);
	temp = temp & 0xFFFFFE0F;
	cha = channel << 4;
	temp = temp | cha;
	UTIL_Printf("set SW mode, channel = 0x%x \n", (channel));
	DRV_WriteReg32(PDWNC_SRVCFG0, temp);
	
	// set rate
	#if 0
	DRV_WriteReg32(PDWNC_SRVRAT, 0x000F0120);
	#elif 0
	DRV_WriteReg32(PDWNC_SRVRAT, 0x00070120);
	#elif 1
	DRV_WriteReg32(PDWNC_SRVRAT, 0x000F0120);
	#endif

	// enable SW mode
	temp = DRV_Reg32(PDWNC_SRVCFG0);
	temp = temp & 0xFFFFFFF0;
	temp = temp | 0x02;
	// UTIL_Printf("SW mode, temp = 0x%x \n", (temp));
	DRV_WriteReg32(PDWNC_SRVCFG0, temp);

	// trigger
	UTIL_Printf("SW trigger \n");
	DRV_WriteReg32(PDWNC_SRVSWT, 0x01);

	for (j=0; j<3; j++) {		
		// read PDWNC_SRVTOTEN
		temp = DRV_Reg32(PDWNC_SRVTOTEN); 
		while ((temp & 0x0004) == 0)
				temp = DRV_Reg32(PDWNC_SRVTOTEN);
		//UTIL_Printf("read PDWNC_SRVTOTEN = 0x%x \n", (temp));  
		
		// print reg
		UTIL_Printf("TSADC channel OUT x: \n"); 
		for (i=0;i<16;i++) {
			temp = DRV_Reg32(PDWNC_ADOUT0 + (i*4));
			UTIL_Printf("TSADC_OUT0 + %d = 0x%x \n", i, temp); 
		}

		for (i=0;i<50;i++)
		{
			kal_sleep_task(10000);
		}

		// clear ADC output data and channel status
		temp = DRV_Reg32(PDWNC_SRVCLR);
		temp = temp | 0x02;
		DRV_WriteReg32(PDWNC_SRVCLR, temp);

		// clear status bit
		temp = DRV_Reg32(PDWNC_SRVTOTEN);
		temp = temp & 0xFFFFFFF9;
		DRV_WriteReg32(PDWNC_SRVTOTEN, temp);

		// trigger
		DRV_WriteReg32(PDWNC_SRVSWT, 0x01);
		
	}

	UTIL_Printf("thermal_adc_sw_trigger_mode: end \n"); 

	return 0;
}

unsigned int set_wakeup_sts_mode(unsigned int  type)  // 0: power control  1: gpio
{
	unsigned int  u4Tmp;
	
	if (type == 1) {
		u4Tmp = DRV_Reg32(PDWNC_PINMUX1);
		u4Tmp = u4Tmp | (1 << 0);
		UTIL_Printf("set wakeup_sts as gpio, u4Tmp 0x%x \n", u4Tmp);
		DRV_WriteReg32(PDWNC_PINMUX1, u4Tmp); 
	} else if (type == 0) {
		u4Tmp = DRV_Reg32(PDWNC_PINMUX1);
		u4Tmp = u4Tmp & ~(1 << 0);
		UTIL_Printf("set wakeup_sts as power control, u4Tmp 0x%x \n", u4Tmp);
		DRV_WriteReg32(PDWNC_PINMUX1, u4Tmp);
	} else {
		u4Tmp = DRV_Reg32(PDWNC_PINMUX1);
		u4Tmp = u4Tmp & ~(1 << 0);
		UTIL_Printf("default set wakeup_sts as power control, u4Tmp 0x%x \n", u4Tmp);
		DRV_WriteReg32(PDWNC_PINMUX1, u4Tmp);
	}

	return 0;
}

unsigned int set_wakeup_sts_function(unsigned int fun, unsigned int value)
{
	unsigned int  u4Tmp;

	if (fun == 1) { // output
		u4Tmp = DRV_Reg32(PDWNC_GPIOEN);
		u4Tmp = u4Tmp | (1 << 0);

		DRV_WriteReg32(PDWNC_GPIOEN, u4Tmp);

		if (value == 1) { // output H
			u4Tmp = DRV_Reg32(PDWNC_GPIOOUT);
			u4Tmp = u4Tmp | (1 << 0);
			UTIL_Printf("wakeup_sts output H, u4Tmp 0x%x \n", u4Tmp);
			DRV_WriteReg32(PDWNC_GPIOOUT, u4Tmp);
		} else if (value == 0) {  //  output L
			u4Tmp = DRV_Reg32(PDWNC_GPIOOUT);
			u4Tmp = u4Tmp & ~(1 << 0);
			UTIL_Printf("wakeup_sts output L, u4Tmp 0x%x \n", u4Tmp);
			DRV_WriteReg32(PDWNC_GPIOOUT, u4Tmp);
		} else {

		}
	} else if (fun == 0) { // input
		u4Tmp = DRV_Reg32(PDWNC_GPIOEN);
		u4Tmp = u4Tmp & ~(1 << 0);
		UTIL_Printf("wakeup_sts input, GPIOEN, u4Tmp 0x%x \n", u4Tmp);
		DRV_WriteReg32(PDWNC_GPIOEN, u4Tmp);
	} else {

	}
	
	return 0;
}

unsigned int set_wakeup_src_mode(unsigned int  type)  // 0: power control  1: gpio
{
	unsigned int  u4Tmp;
	
	if (type == 1) {
		u4Tmp = DRV_Reg32(PDWNC_PINMUX1);
		u4Tmp = u4Tmp | (1 << 1);
		UTIL_Printf("set wakeup_src as gpio, u4Tmp 0x%x \n", u4Tmp);
		DRV_WriteReg32(PDWNC_PINMUX1, u4Tmp); 
	} else if (type == 0) {
		u4Tmp = DRV_Reg32(PDWNC_PINMUX1);
		u4Tmp = u4Tmp & ~(1 << 1);
		UTIL_Printf("set wakeup_src as power control, u4Tmp 0x%x \n", u4Tmp);
		DRV_WriteReg32(PDWNC_PINMUX1, u4Tmp);
	} else {
		u4Tmp = DRV_Reg32(PDWNC_PINMUX1);
		u4Tmp = u4Tmp & ~(1 << 1);
		UTIL_Printf("default set wakeup_src as power control, u4Tmp 0x%x \n", u4Tmp);
		DRV_WriteReg32(PDWNC_PINMUX1, u4Tmp);
	}

	return 0;
}

unsigned int set_wakeup_src_function(unsigned int fun, unsigned int value)
{
	unsigned int  u4Tmp;

	if (fun == 1) { // output
		u4Tmp = DRV_Reg32(PDWNC_GPIOEN);
		u4Tmp = u4Tmp | (1 << 1);

		DRV_WriteReg32(PDWNC_GPIOEN, u4Tmp);

		if (value == 1) { // output H
			u4Tmp = DRV_Reg32(PDWNC_GPIOOUT);
			u4Tmp = u4Tmp | (1 << 1);
			UTIL_Printf("wakeup_src output H, u4Tmp 0x%x \n", u4Tmp);
			DRV_WriteReg32(PDWNC_GPIOOUT, u4Tmp);
		} else if (value == 0) {  //  output L
			u4Tmp = DRV_Reg32(PDWNC_GPIOOUT);
			u4Tmp = u4Tmp & ~(1 << 1);
			UTIL_Printf("wakeup_src output L, u4Tmp 0x%x \n", u4Tmp);
			DRV_WriteReg32(PDWNC_GPIOOUT, u4Tmp);
		} else {

		}
	} else if (fun == 0) { // input
		u4Tmp = DRV_Reg32(PDWNC_GPIOEN);
		u4Tmp = u4Tmp & ~(1 << 1);
		UTIL_Printf("wakeup_src input, GPIOEN, u4Tmp 0x%x \n", u4Tmp);
		DRV_WriteReg32(PDWNC_GPIOEN, u4Tmp);
	} else {

	}
	
	return 0;
}

int thermal_adc_io_trigger_mode(unsigned int channel)
{
	unsigned int temp = 0, i = 0, j = 0;
	unsigned int cha = 0;

	// set wakeup_sts as gpio input 
	set_wakeup_sts_mode(1);
	set_wakeup_sts_function(0, 0);

	// set wakeup_src as gpio input 
	//set_wakeup_src_mode(1);
	//set_wakeup_src_function(0, 0);
		
	// clear ADC output data and channel status
	temp = DRV_Reg32(PDWNC_SRVCLR);
	temp = temp | 0x02;
	DRV_WriteReg32(PDWNC_SRVCLR, temp);

	// polling clear to be done
	temp = DRV_Reg32(PDWNC_SRVCLR); 
	while ((temp & 0x0002) != 0){
			temp = DRV_Reg32(PDWNC_SRVCLR);
	}
	UTIL_Printf("set IO mode \n");

	// enable timeout
	temp = DRV_Reg32(PDWNC_SRVTOTEN);
	temp = temp | 0x02;
	temp = temp & 0xFFFFFFFB;
	DRV_WriteReg32(PDWNC_SRVTOTEN, temp);
	//UTIL_Printf("IO mode, PDWNC_SRVTOTEN = 0x%x \n", (temp));

	#if 1
	// open channel x
	DRV_WriteReg32(PDWNC_SRVCFG1, (1 << channel));
	temp = DRV_Reg32(PDWNC_SRVCFG1);
	UTIL_Printf("set channel = %d \n", (channel)); 
	#endif
		
	// set rate
	#if 0
	DRV_WriteReg32(PDWNC_SRVRAT, 0x000F0120);
	#elif 0
	DRV_WriteReg32(PDWNC_SRVRAT, 0x00070120);
	#elif 1
	DRV_WriteReg32(PDWNC_SRVRAT, 0x000F0120);
	#endif

	#if 1
	// select channel for IO mode
	temp = DRV_Reg32(PDWNC_SRVCFG0);
	temp = temp & 0xFFFFFE0F;
	cha = channel << 4;
	temp = temp | cha;
	UTIL_Printf("select channel = %d \n", (channel));
	DRV_WriteReg32(PDWNC_SRVCFG0, temp);
	#endif

	// enable IO mode
	temp = DRV_Reg32(PDWNC_SRVCFG0);
	temp = temp & 0xFFFFFFF0;
	temp = temp | 0x04;
	//UTIL_Printf("IO mode, temp = 0x%x \n", (temp));
	DRV_WriteReg32(PDWNC_SRVCFG0, temp);

	for (j=0; j<5; j++) {	

		temp = DRV_Reg32(PDWNC_SRVTOTEN); 
		while ((temp & 0x0004) == 0){
				UTIL_Printf("pls trigger PAD_WAKEUP_STS pin: \n"); 
				for (i=0;i<10;i++)
				{
					kal_sleep_task(10000);
				}
				temp = DRV_Reg32(PDWNC_GPIOIN);
				temp = temp & 0x01;
				UTIL_Printf("got wakeup_sts = [%d] \n", temp); 

				temp = 0;
				temp = DRV_Reg32(PDWNC_SRVTOTEN);
				//UTIL_Printf("read PDWNC_SRVTOTEN = 0x%x \n", (temp));
		}
		
		// print reg
		UTIL_Printf("TSADC channel OUT x: \n"); 
		for (i=0;i<16;i++) {
			temp = DRV_Reg32(PDWNC_ADOUT0 + (i*4));
			UTIL_Printf("TSADC_OUT0 + %d = 0x%x \n", i, temp); 
		}

		for (i=0;i<5;i++)
		{
			kal_sleep_task(10000);
		}

		// clear ADC output data and channel status
		temp = DRV_Reg32(PDWNC_SRVCLR);
		temp = temp | 0x02;
		DRV_WriteReg32(PDWNC_SRVCLR, temp);

		// clear status bit
		temp = DRV_Reg32(PDWNC_SRVTOTEN);
		temp = temp & 0xFFFFFFF1;
		DRV_WriteReg32(PDWNC_SRVTOTEN, temp);

	}

	UTIL_Printf("thermal_adc_io_trigger_mode: end \n"); 

	return 0;
}

int thermal_adc_change_sample_rate(unsigned int channel)
{

	return 0;
}

int thermal_adc_calibration_prepare_1(unsigned int channel)
{

	#if 0
	unsigned int temp0 = 0, temp1 = 0, temp2 = 0;

	temp0 = DRV_Reg32(0xF0054690);
	temp1 = DRV_Reg32(0xF00546B8);
	temp2 = DRV_Reg32(0xF00546BC);

	UTIL_Printf("==> calibration prepare 1, 0x690 = 0x%x, 0x6B8 = 0x%x, 0x6BC = 0x%x \n", temp0, temp1, temp2);
	
	g_adc_ge = (temp0 & 0x1FF80000) >> 19;
	g_adc_oe = (temp0 & 0x0007FE00) >> 9;

	UTIL_Printf("ge = %d, oe = %d \n", g_adc_ge, g_adc_oe);
	
	g_o_vtsmcu0 = (temp0 & 0x000001FF);
	g_o_vtsmcu1 = (temp1 & 0x07FC0000) >> 18;
	g_o_vtsmcu2 = (temp1 & 0x0003FE00) >> 9;
	g_o_vtsmcu3 = (temp1 & 0x000001FF);
	UTIL_Printf("local thermal = %d, thermal0 = %d, thermal1 = %d, thermal2 = %d \n", g_o_vtsmcu0, g_o_vtsmcu1, g_o_vtsmcu2, g_o_vtsmcu3);
	
	g_degc_cali = (temp2 & 0x01F80000) >> 19;
	g_adc_cali_en = (temp2 & 0x00040000) >> 18;
	g_o_slope_sign = (temp2 & 0x00020000) >> 17;
	g_o_slope = (temp2 & 0x0001F800) >> 11;
	
	g_id = (temp2 & 0x00000400) >> 10;

	UTIL_Printf("degc = %d, cali_en = %d, slope_sign = %d, slope = %d \n", g_degc_cali, g_adc_cali_en, g_o_slope_sign, g_o_slope);
	#else
	g_id = 0;
	g_adc_cali_en = 0;
	#endif

	if(g_id==0)
	{
		g_o_slope=0;
	}

	if(g_adc_cali_en == 1)
	{
		//thermal_enable = true;        
	}
	else
	{
	#if 0	
		g_adc_ge = 512;
		g_adc_oe = 512;
		g_degc_cali = 50;
		g_o_slope = 2;	// 165+2 = 167 1.67	
		g_o_slope_sign = 0;
		g_o_vtsmcu0 = 205;
		g_o_vtsmcu1 = 205;
		g_o_vtsmcu2 = 205;
		g_o_vtsmcu3 = 205;
	#else
		UTIL_Printf("==> g_id = %d, g_adc_cali_en = %d, g_degc_cali = %d \n", g_id, g_adc_cali_en, g_degc_cali);
		g_adc_ge = 512;
		g_adc_oe = 512;
		g_degc_cali = 40;
		g_o_slope = 0;
		g_o_slope_sign = 0;
		g_o_vtsmcu0 = 260;
		g_o_vtsmcu1 = 260;
		g_o_vtsmcu2 = 260;
		g_o_vtsmcu3 = 260;
		
		UTIL_Printf("g_adc_ge = %d, g_adc_oe = %d \n", g_adc_ge, g_adc_oe);	
		UTIL_Printf("g_o_slope = %d, g_o_slope_sign = %d \n", g_o_slope, g_o_slope_sign);
	#endif
	}

	return 0;
}

int thermal_adc_calibration_prepare_2(unsigned int channel)
{
	kal_int32 format_1, format_2, format_3, format_4= 0;

	//	[FT] ADC_GE[9:0] = GE*4096 + 512 (round to integer)-(1)
	//	[FT] ADC_OE[9:0] = OE*4096 + 512 (round to integer)-(2)
	g_ge = ((g_adc_ge - 512) * 10000 ) / 4096; // ge * 10000
	g_oe = (g_adc_oe - 512);
	g_gain = (10000 + g_ge); // gain * 10000

	UTIL_Printf("==> g_ge = %d, g_oe = %d, g_gain = %d \n", g_ge, g_oe, g_gain);

	// [FT] O_VTSMCU1=Y_VTS1-3192
	// Ideal ADC Value * ADC_Gain + ADC offset	= ADC Value.... (3)
	// Ideal ADC Value * ADC_Gain = ADC Value - ADC offset
	// format_X = Ideal ADC Value * ADC_Gain = (g_o_vtsmcu1 + TS_LOW_CRITERIAL) - (g_oe)
	format_1 = (g_o_vtsmcu1 + TS_LOW_CRITERIAL - g_oe);
	format_2 = (g_o_vtsmcu2 + TS_LOW_CRITERIAL - g_oe);
	format_3 = (g_o_vtsmcu3 + TS_LOW_CRITERIAL - g_oe);
	format_4 = (g_o_vtsmcu3 + TS_LOW_CRITERIAL - g_oe);

	UTIL_Printf("format_1 = %d, format_2 = %d, format_3 = %d, format_4 = %d\n", format_1, format_2, format_3, format_4);

	// TSUV_S1 / 2.8V = Ideal ADC Value / 4096 .... (2)
	// g_x_roomt[0]=TSUV_S1 / 2.8V = Ideal ADC Value / 4096 = format_X/ADC_Gain/4096
	g_x_roomt[0] = (((format_1 * 10000) / 4096) * 10000) / g_gain; // x_roomt * 10000
	g_x_roomt[1] = (((format_2 * 10000) / 4096) * 10000) / g_gain; // x_roomt * 10000
	g_x_roomt[2] = (((format_3 * 10000) / 4096) * 10000) / g_gain; // x_roomt * 10000
	g_x_roomt[3] = (((format_4 * 10000) / 4096) * 10000) / g_gain; // x_roomt * 10000

	UTIL_Printf("g_x_roomt1 = %d, g_x_roomt2 = %d, g_x_roomt3 = %d, g_x_roomt4 = %d\n",	g_x_roomt[0], g_x_roomt[1], g_x_roomt[2], g_x_roomt[3]);

	// vDrvGetEFuse_ThermalSesnorData();

	return 0;

}

int thermal_adc_calibration(unsigned int channel)
{
	int temp = 0;

	mtktscpu_get_hw_temp();

	thermal_adc_calibration_prepare_1(0);
	thermal_adc_calibration_prepare_2(0);

	temp = read_tc_raw_and_temp_check();

	UTIL_Printf("temperature = %d \n", temp);

	UTIL_Printf("end \n");

	return 0;
}

int thermal_adc_sndr(unsigned int channel)
{
	unsigned int temp = 0;
	unsigned int cha = 0;
	#if 0
	unsigned int i = 0, j = 0;
	#endif

	#if 0
	// set wakeup_sts as gpio input 
	UTIL_Printf("set wakeup_sts as gpio input \n");
	set_wakeup_sts_mode(1);
	set_wakeup_sts_function(0, 0);
	#endif
	
	// disable hw/sw/io tigger mode 
	temp = DRV_Reg32(PDWNC_SRVCFG0);
	temp = temp & 0xFFFFFFF8;
	UTIL_Printf("disable hw/sw/io tigger mode, PDWNC_SRVCFG0 = 0x%x \n", (temp));
	DRV_WriteReg32(PDWNC_SRVCFG0, temp);

	// clear ADC output data and channel status
	temp = DRV_Reg32(PDWNC_SRVCLR);
	temp = temp | 0x02;
	UTIL_Printf("clear ADC output data and channel status,  PDWNC_SRVCLR = 0x%x \n", temp);
	DRV_WriteReg32(PDWNC_SRVCLR, temp);

	// polling clear to be done
	temp = DRV_Reg32(PDWNC_SRVCLR); 
	while ((temp & 0x0002) != 0){
			temp = DRV_Reg32(PDWNC_SRVCLR);
	}

	// set rate
	UTIL_Printf("set rate, 0x000F0120 \n");
	DRV_WriteReg32(PDWNC_SRVRAT, 0x000F0120);

	// set monitor channel
	temp = DRV_Reg32(PDWNC_SRVCFG0);
	temp = temp & 0xFFFFFE0F;
	cha = channel << 4;
	temp = temp | cha;
	UTIL_Printf("set monitor channel = %d, PDWNC_SRVCFG0 = 0x%x \n", (channel), (temp));
	DRV_WriteReg32(PDWNC_SRVCFG0, temp);

	// open channel x
	DRV_WriteReg32(PDWNC_SRVCFG1, (1 << channel));
	temp = DRV_Reg32(PDWNC_SRVCFG1);
	UTIL_Printf("enable channel = %d, PDWNC_SRVCFG1 = 0x%x \n", (channel), temp); 

	#if 1
	// enable SW mode
	temp = DRV_Reg32(PDWNC_SRVCFG0);
	temp = temp & 0xFFFFFFF8;
	temp = temp | 0x02;
	UTIL_Printf("enable SW mode, temp = 0x%x \n", (temp));
	DRV_WriteReg32(PDWNC_SRVCFG0, temp);
	#else
	// enable IO mode
	temp = DRV_Reg32(PDWNC_SRVCFG0);
	temp = temp & 0xFFFFFFF8;
	temp = temp | 0x04;
	UTIL_Printf("enable IO mode, PDWNC_SRVCFG0 = 0x%x \n", (temp));
	DRV_WriteReg32(PDWNC_SRVCFG0, temp);
	#endif

	// enable SNDR
	temp = DRV_Reg32(PDWNC_SRVCFG0);
	temp = temp & 0xAFFFFFFF;
	temp = temp | 0x50000000;
	UTIL_Printf("enable SNDR, PDWNC_SRVCFG0 = 0x%x \n", (temp));
	DRV_WriteReg32(PDWNC_SRVCFG0, temp);

	//for (j=0; j<1000; j++) {
	while (1) {
		
		#if 1
		// trigger
		//UTIL_Printf("SW trigger \n");
		DRV_WriteReg32(PDWNC_SRVSWT, 0x01);

		temp = DRV_Reg32(PDWNC_SRVTOTEN); 
		while ((temp & 0x0004) == 0){
				#if 0
				for (i=0;i<5;i++)
				{
					kal_sleep_task(10000);
				}
				#endif
				temp = 0;
				temp = DRV_Reg32(PDWNC_SRVTOTEN);
				//UTIL_Printf("read PDWNC_SRVTOTEN = 0x%x \n", (temp));
		}
		#else
		temp = DRV_Reg32(PDWNC_SRVTOTEN); 
		while ((temp & 0x0004) == 0){
				UTIL_Printf("pls trigger PAD_WAKEUP_STS pin: \n"); 
				for (i=0;i<10;i++)
				{
					kal_sleep_task(10000);
				}
				temp = DRV_Reg32(PDWNC_GPIOIN);
				temp = temp & 0x01;
				UTIL_Printf("got wakeup_sts = [%d] \n", temp); 

				temp = 0;
				temp = DRV_Reg32(PDWNC_SRVTOTEN);
				//UTIL_Printf("read PDWNC_SRVTOTEN = 0x%x \n", (temp));
		}
		#endif

		#if 0
		// print reg
		UTIL_Printf("TSADC channel OUT x: \n"); 
		for (i=0;i<16;i++) {
			temp = DRV_Reg32(PDWNC_ADOUT0 + (i*4));
			UTIL_Printf("TSADC_OUT0 + %d = 0x%x \n", i, temp); 
		}

		for (i=0;i<10;i++)
		{
			kal_sleep_task(10000);
		}
		#endif

		// clear ADC output data and channel status
		temp = DRV_Reg32(PDWNC_SRVCLR);
		temp = temp | 0x02;
		DRV_WriteReg32(PDWNC_SRVCLR, temp);

		// polling clear to be done
		temp = DRV_Reg32(PDWNC_SRVCLR); 
		while ((temp & 0x0002) != 0){
				temp = DRV_Reg32(PDWNC_SRVCLR);
		}

		#if 1
		// clear status bit
		temp = DRV_Reg32(PDWNC_SRVTOTEN);
		temp = temp & 0xFFFFFFF9;
		DRV_WriteReg32(PDWNC_SRVTOTEN, temp);
		#endif

		#if 0
		for (i=0;i<5;i++)
		{
			kal_sleep_task(10000);
		}
		#endif
	}

	UTIL_Printf("thermal_adc_sndr: end \n"); 

	return 0;
}


void thermal_reset_and_initial(void)
{
	//t temp = 0;

	//UTIL_Printf("[Reset and init thermal controller]\n");


#if 0 //hywu: TV auxadc seems default on.
	// AuxADC Initialization,ref MT6592_AUXADC.doc // TODO: check this line
	temp = DRV_Reg32(AUXADC_CON0);//Auto set enable for CH11
	temp &= 0xFFFFF7FF;//0: Not AUTOSET mode
	THERMAL_WRAP_WR32(temp, AUXADC_CON0);        // disable auxadc channel 11 synchronous mode

	THERMAL_WRAP_WR32(0x800, AUXADC_CON1_CLR);    // disable auxadc channel 11 immediate mode
#endif 

    THERMAL_WRAP_WR32(0x00000004, TEMPMONCTL1);    // bus clock 66M counting unit is 4*15.15ns* 256 = 15513.6 ms=15.5us


    THERMAL_WRAP_WR32(0x000101AD, TEMPMONCTL2);	    // filter interval is 1023 * 15.5us ~ 15.86ms
    //THERMAL_WRAP_WR32(0x00FFFFFF, TEMPAHBPOLL);		// poll is set to 254.17ms
    THERMAL_WRAP_WR32(0x00000300, TEMPAHBPOLL);		// poll is set to 254.17ms
    THERMAL_WRAP_WR32(0x00000000, TEMPMSRCTL0);      // temperature sampling control, 1 sample

	THERMAL_WRAP_WR32(0xFFFFFFFF, TEMPAHBTO);      // exceed this polling time, IRQ would be inserted

	THERMAL_WRAP_WR32(0x00000000, TEMPMONIDET0);   // times for interrupt occurrance
	THERMAL_WRAP_WR32(0x00000000, TEMPMONIDET1);   // times for interrupt occurrance

	//THERMAL_WRAP_WR32(0x800, AUXADC_CON1_SET);    // enable auxadc channel 11 immediate mode


	THERMAL_WRAP_WR32(0x800, TEMPADCMUX);                         // this value will be stored to TEMPPNPMUXADDR (TEMPSPARE0) automatically by hw
	THERMAL_WRAP_WR32((int) PDWNC_SRVCFG0, TEMPADCMUXADDR);// AHB address for auxadc mux selection

	THERMAL_WRAP_WR32(0x800, TEMPADCEN);                          // AHB value for auxadc enable
	THERMAL_WRAP_WR32((int) PDWNC_SRVCFG0, TEMPADCENADDR); // AHB address for auxadc enable (channel 0 immediate mode selected)
																  // this value will be stored to TEMPADCENADDR automatically by hw

	THERMAL_WRAP_WR32((int) PDWNC_ADOUT11, TEMPADCVALIDADDR); // AHB address for auxadc valid bit
	THERMAL_WRAP_WR32((int) PDWNC_ADOUT11, TEMPADCVOLTADDR);  // AHB address for auxadc voltage output
	THERMAL_WRAP_WR32(0x0, TEMPRDCTRL);               			  // read valid & voltage are at the same register
	THERMAL_WRAP_WR32(0x0000002C, TEMPADCVALIDMASK);              // indicate where the valid bit is (the 12th bit is valid bit and 1 is valid)
	THERMAL_WRAP_WR32(0x0, TEMPADCVOLTAGESHIFT);                  // do not need to shift
	THERMAL_WRAP_WR32(0x2, TEMPADCWRITECTRL);                     // enable auxadc mux write transaction

#if 0 //hywu: TV no such TS control register.
	temp = DRV_Reg32(TS_CON0);
    temp &=~(0x000000C0);										  //TSCON0[7:6]=2'b00,   00: Buffer on, TSMCU to AUXADC
	THERMAL_WRAP_WR32(temp, TS_CON0);	                          //read abb need
#endif 
	//udelay(150);//RG_TS2AUXADC < set from 2'b11 to 2'b00 when resume.wait 100uS than turn on thermal controller.


}


/*
	Bank0 : CPU (TS_MCU1,TS_MCU2)        (TS3, TS4)
	Bank1 : GPU (TS_MCU3)                (TS5)
	Bank2 : SOC (TS_MCU4,TS_MCU2,TS_MCU3)(TS1, TS4, TS5)
*/
static void thermal_config_Bank0_TS(void)
{
	//UTIL_Printf( "thermal_config_Bank0_TS\n");

//    thermal_reset_and_initial();

	//Bank0:CPU(TS_MCU1 and TS_MCU2)
    //TSCON1[5:4]=2'b00
	//TSCON1[2:0]=3'b000
    THERMAL_WRAP_WR32(0x0,TEMPADCPNP0);                    // this value will be stored to TEMPPNPMUXADDR (TEMPSPARE0) automatically by hw

    //TSCON1[5:4]=2'b00
	//TSCON1[2:0]=3'b001
    THERMAL_WRAP_WR32(0x1,TEMPADCPNP1);                    // this value will be stored to TEMPPNPMUXADDR (TEMPSPARE0) automatically by hw


    THERMAL_WRAP_WR32((UINT32) PDWNC_SRVCFG0,TEMPPNPMUXADDR);  // AHB address for pnp sensor mux selection
   	THERMAL_WRAP_WR32(0x3, TEMPADCWRITECTRL);

    THERMAL_WRAP_WR32(0x00000003, TEMPMONCTL0);            // enable periodoc temperature sensing point 0, point 1
}

static void thermal_config_Bank1_TS(void)
{

	//UTIL_Printf( "thermal_config_Bank1_TS\n");

	//Bank1:CA15(TS1 and TS3)

	//Bank1:GPU(TS_MCU3)
    //TSCON1[5:4]=2'b00
	//TSCON1[2:0]=3'b010
    THERMAL_WRAP_WR32(0x2,TEMPADCPNP0);                    // this value will be stored to TEMPPNPMUXADDR (TEMPSPARE0) automatically by hw

	//Bank1:ABB(TS_ABB)
    //TSCON1[5:4]=2'b01
	//TSCON1[2:0]=3'b000
    THERMAL_WRAP_WR32(0x10,TEMPADCPNP1);                   // this value will be stored to TEMPPNPMUXADDR (TEMPSPARE0) automatically by hw

    THERMAL_WRAP_WR32((UINT32) PDWNC_SRVCFG0,TEMPPNPMUXADDR);  // AHB address for pnp sensor mux selection
   	THERMAL_WRAP_WR32(0x3, TEMPADCWRITECTRL);

    THERMAL_WRAP_WR32(0x00000003, TEMPMONCTL0);            // enable periodoc temperature sensing point 0, point 1


}

static void thermal_config_Bank2_TS(void)
{

	//UTIL_Printf( "thermal_config_Bank2_TS\n");

	//Bank1:SOC(TS_MCU4,TS_MCU2,TS_MCU3)

    //TSCON1[5:4]=2'b00
	//TSCON1[2:0]=3'b011
    THERMAL_WRAP_WR32(0x3,TEMPADCPNP0);                    // this value will be stored to TEMPPNPMUXADDR (TEMPSPARE0) automatically by hw

    //TSCON1[5:4]=2'b00
	//TSCON1[2:0]=3'b001
    THERMAL_WRAP_WR32(0x1,TEMPADCPNP1);                    // this value will be stored to TEMPPNPMUXADDR (TEMPSPARE0) automatically by hw

    //TSCON1[5:4]=2'b00
	//TSCON1[2:0]=3'b010
    THERMAL_WRAP_WR32(0x2,TEMPADCPNP2);                    // this value will be stored to TEMPPNPMUXADDR (TEMPSPARE0) automatically by hw


    THERMAL_WRAP_WR32((UINT32) PDWNC_SRVCFG0,TEMPPNPMUXADDR);  // AHB address for pnp sensor mux selection
   	THERMAL_WRAP_WR32(0x3, TEMPADCWRITECTRL);

    THERMAL_WRAP_WR32(0x00000007, TEMPMONCTL0);   		   // enable periodoc temperature sensing point 0, point 1, point 2

}




static void thermal_config_TS_in_banks(thermal_bank_name bank_num)
{
	//UTIL_Printf( "thermal_config_TS_in_banks bank_num=%d\n",bank_num);

	switch(bank_num){
        case THERMAL_BANK0://CPU(TS_MCU1 and TS_MCU2)
            thermal_config_Bank0_TS();
            break;
        case THERMAL_BANK1://GPU (TS_MCU3), ABB(TS_ABB)
            thermal_config_Bank1_TS();
            break;
        case THERMAL_BANK2://SOC(TS_MCU4,TS_MCU2,TS_MCU3)
            thermal_config_Bank2_TS();
            break;
        default:
            thermal_config_Bank0_TS();//CPU(TS_MCU1 and TS_MCU2)
            break;
    }
}

int mtktscpu_switch_bank(thermal_bank_name bank)
{
	#if 0  //  ac8237 just has one bank
	//UTIL_Printf( "mtktscpu_switch_bank =bank=%d\n",bank);

	switch(bank){
        case THERMAL_BANK0://bank0,A7 (TS1 TS2)
            thermal_clrl(PTPCORESEL, 0xF);//bank0
            break;
        case THERMAL_BANK1://bank1,CA15 (TS1 TS3)
        	thermal_clrl(PTPCORESEL, 0xF);
            thermal_setl(PTPCORESEL, 0x1);//bank1
            break;
        case THERMAL_BANK2://bank2,GPU (TS3 TS4)
        	thermal_clrl(PTPCORESEL, 0xF);
            thermal_setl(PTPCORESEL, 0x2);//bank2
            break;
        default:
            thermal_clrl(PTPCORESEL, 0xF);//bank0
            break;
    }
    UTIL_Printf( "mtktscpu_switch_bank %d. PTPCORESEL=0x%x\n",bank, DRV_Reg32(PTPCORESEL));
	#endif
	
	return 0;
}

static void thermal_initial_all_bank(void)
{
	int i=0; 
	//UTIL_Printf("thermal_initial_all_bank,ROME_BANK_NUM=%d\n",ROME_BANK_NUM);
#if 0
	// AuxADC Initialization,ref MT6592_AUXADC.doc // TODO: check this line
	temp = DRV_Reg32(AUXADC_CON0);//Auto set enable for CH11
	temp &= 0xFFFFF7FF;//0: Not AUTOSET mode
	THERMAL_WRAP_WR32(temp, AUXADC_CON0);        // disable auxadc channel 11 synchronous mode
	THERMAL_WRAP_WR32(0x800, AUXADC_CON1_CLR);    // disable auxadc channel 11 immediate mode
#endif

    /*config bank0,1,2*/
    for(i=0;i<ROME_BANK_NUM;i++){

		//UTIL_Printf("==============================\n");
		//mtktscpu_switch_bank(i);
        thermal_reset_and_initial();
        thermal_config_TS_in_banks(i);
        //UTIL_Printf("==============================\n");
    }

}

int thermal_fast_init(void)
{
	UINT32 temp = 0;
    UINT32 cunt = 0;
//    UINT32 temp1 = 0,temp2 = 0,temp3 = 0,count=0;

	//tscpu_printk( "thermal_fast_init\n");


    temp = 0xCE1; // 40 degree //DA1;
    //DRV_WriteReg32(PTPSPARE2, (0x00001000 + temp));//write temp to spare register
    DRV_WriteReg32(TEMPADCEN, (0x00001000 + temp));//write temp to spare register	

    DRV_WriteReg32(TEMPMONCTL1, 1);                // counting unit is 320 * 31.25us = 10ms
    DRV_WriteReg32(TEMPMONCTL2, 1);                // sensing interval is 200 * 10ms = 2000ms
    DRV_WriteReg32(TEMPAHBPOLL, 1);                // polling interval to check if temperature sense is ready

    DRV_WriteReg32(TEMPAHBTO,    0x000000FF);               // exceed this polling time, IRQ would be inserted
    DRV_WriteReg32(TEMPMONIDET0, 0x00000000);               // times for interrupt occurrance
    DRV_WriteReg32(TEMPMONIDET1, 0x00000000);               // times for interrupt occurrance

    DRV_WriteReg32(TEMPMSRCTL0, 0x0000000);                 // temperature measurement sampling control

    DRV_WriteReg32(TEMPADCPNP0, 0x1);                       // this value will be stored to TEMPPNPMUXADDR (TEMPSPARE0) automatically by hw
    DRV_WriteReg32(TEMPADCPNP1, 0x2);                       // this value will be stored to TEMPPNPMUXADDR (TEMPSPARE0) automatically by hw
    DRV_WriteReg32(TEMPADCPNP2, 0x3);
    DRV_WriteReg32(TEMPADCPNP3, 0x4);

#if 0
    DRV_WriteReg32(TEMPPNPMUXADDR, (UINT32)PTPSPARE0_P);    // AHB address for pnp sensor mux selection
    DRV_WriteReg32(TEMPADCMUXADDR, (UINT32) PTPSPARE0_P);    // AHB address for auxadc mux selection
    DRV_WriteReg32(TEMPADCENADDR,  (UINT32) PTPSPARE1_P);     // AHB address for auxadc enable
    DRV_WriteReg32(TEMPADCVALIDADDR,(UINT32) PTPSPARE2_P);  // AHB address for auxadc valid bit
    DRV_WriteReg32(TEMPADCVOLTADDR, (UINT32) PTPSPARE2_P);   // AHB address for auxadc voltage output
#else
    DRV_WriteReg32(TEMPPNPMUXADDR, PDWNC_SPARE);
    DRV_WriteReg32(TEMPADCMUXADDR, PDWNC_SPARE);    // AHB address for auxadc mux selection
    DRV_WriteReg32(TEMPADCENADDR, PDWNC_SPARE); // AHB address for auxadc enable
    DRV_WriteReg32(TEMPADCVALIDADDR, PDWNC_SPARE); // AHB address for auxadc valid bit
    DRV_WriteReg32(TEMPADCVOLTADDR, PDWNC_SPARE);
#endif

    DRV_WriteReg32(TEMPRDCTRL, 0x0);                        // read valid & voltage are at the same register
    DRV_WriteReg32(TEMPADCVALIDMASK, 0x0000002C);           // indicate where the valid bit is (the 12th bit is valid bit and 1 is valid)
    DRV_WriteReg32(TEMPADCVOLTAGESHIFT, 0x0);               // do not need to shift
#if 0
    DRV_WriteReg32(TEMPADCWRITECTRL, 0x3);                  // enable auxadc mux & pnp write transaction
#else
    DRV_WriteReg32(TEMPADCWRITECTRL, 0x0);                  // enable auxadc mux & pnp write transaction
#endif


	DRV_WriteReg32(TEMPMONINT, 0x00000000);                 // enable all interrupt except filter sense and immediate sense interrupt


    DRV_WriteReg32(TEMPMONCTL0, 0x0000000F);                // enable all sensing point (sensing point 2 is unused)


    for (cunt = 0; cunt < 0x100;)
    {
        temp = DRV_Reg32(THERM_CTRL_BASE + cunt);
        UTIL_Printf("0x%0X | %0X ", THERM_CTRL_BASE + cunt, temp);
        temp = DRV_Reg32(THERM_CTRL_BASE + cunt + 4);
        UTIL_Printf("%0X ", temp);
        temp = DRV_Reg32(THERM_CTRL_BASE + cunt + 8);
        UTIL_Printf("%0X ", temp);
        temp = DRV_Reg32(THERM_CTRL_BASE + cunt + 12);
        UTIL_Printf("%0X\n", temp);
        cunt += 0x10;
    }

	cunt=0;
	temp = DRV_Reg32(TEMPMSR0)& 0x0fff;
    while(temp!=0xDA1 && cunt <20){
		cunt++;
    	UTIL_Printf("[Power/CPU_Thermal]0 temp=%d,cunt=%d\n",temp,cunt);
        temp = DRV_Reg32(TEMPMSR0)& 0x0fff;
	}

    cunt=0;
	temp = DRV_Reg32(TEMPMSR1)& 0x0fff;
    while(temp!=0xDA1 &&  cunt <20){
		cunt++;
    	UTIL_Printf("[Power/CPU_Thermal]1 temp=%d,cunt=%d\n",temp,cunt);
        temp = DRV_Reg32(TEMPMSR1)& 0x0fff;
	}

	cunt=0;
	temp = DRV_Reg32(TEMPMSR2)& 0x0fff;
    while(temp!=0xDA1 &&  cunt <20){
		cunt++;
    	UTIL_Printf("[Power/CPU_Thermal]2 temp=%d,cunt=%d\n",temp,cunt);
        temp = DRV_Reg32(TEMPMSR2)& 0x0fff;
	}

	cunt=0;
	temp = DRV_Reg32(TEMPMSR3)& 0x0fff;
    while(temp!=0xDA1 &&  cunt <20){
		cunt++;
    	UTIL_Printf("[Power/CPU_Thermal]3 temp=%d,cunt=%d\n",temp,cunt);
        temp = DRV_Reg32(TEMPMSR3)& 0x0fff;
	}

	return 0;
}

static void tscpu_reset_thermal(void)
{
	
   //hywu: set ADC SW trigger mode as TV wukong setting 
    vIO32WriteFldAlign(PDWNC_SRVCFG0, 0, FLD_HWEN);
	vIO32WriteFldAlign(PDWNC_SRVCFG0, 1, FLD_SWEN);

}

#if 0
static void tscpu_fast_initial_sw_workaround(void)
{
	int i=0;

    /*config bank0,1,2,3*/
    for(i=0;i<ROME_BANK_NUM;i++){
	    mtktscpu_switch_bank(i);
	    thermal_fast_init();
    }
}

//disable ALL periodoc temperature sensing point
static void thermal_disable_all_periodoc_temp_sensing(void)
{
	int i=0; 

    UTIL_Printf("thermal_disable_all_periodoc_temp_sensing\n");


    /*config bank0,1,2,3*/
    for(i=0;i<ROME_BANK_NUM;i++){

		mtktscpu_switch_bank(i);
        //tscpu_printk("thermal_disable_all_periodoc_temp_sensing:Bank_%d\n",i);
	    THERMAL_WRAP_WR32(0x00000000, TEMPMONCTL0);
    }

}
//pause ALL periodoc temperature sensing point
static void thermal_pause_all_periodoc_temp_sensing(void)
{
	int i=0,temp;

    UTIL_Printf("thermal_pause_all_periodoc_temp_sensing\n");


    /*config bank0,1,2,3*/
    for(i=0;i<ROME_BANK_NUM;i++){

		mtktscpu_switch_bank(i);
		temp = DRV_Reg32(TEMPMSRCTL1);
		DRV_WriteReg32(TEMPMSRCTL1, (temp | 0x10E));//set /set bit8 =bit1=bit2=bit3=1 to pause sensing point 0,1,2,3
    }

}
static void thermal_release_all_periodoc_temp_sensing(void)
{
	int i=0; 
    int temp;

    UTIL_Printf("thermal_release_all_periodoc_temp_sensing\n");


    /*config bank0,1,2*/
    for(i=0;i<ROME_BANK_NUM;i++){

		mtktscpu_switch_bank(i);
		temp = DRV_Reg32(TEMPMSRCTL1);
		DRV_WriteReg32(TEMPMSRCTL1, ( (temp & (~0x0E)) ));//set bit8 = bit1=bit2=bit3=1 to pause sensing point 0,1,2
    }


}
#endif

static void set_tc_trigger_hw_protect(int temperature, int temperature2)
{

	int temp = 0;
	int raw_high, raw_middle, raw_low;

	//temperature to trigger SPM state2
	raw_high   = temperature_to_raw_room(temperature);
    if (temperature2 > -275000)
		raw_middle = temperature_to_raw_room(temperature2);
	//raw_low    = temperature_to_raw_room(5000);
	raw_low    = temperature_to_raw_room(100000);

	//temperature2=80000;  test only
	temp = DRV_Reg32(TEMPMONINT);
	//tscpu_printk("set_tc_trigger_hw_protect 1 TEMPMONINT:temp=0x%x\n",temp);
	//THERMAL_WRAP_WR32(temp & 0x1FFFFFFF, TEMPMONINT);	// disable trigger SPM interrupt
	THERMAL_WRAP_WR32(temp & 0x00000000, TEMPMONINT);	// disable trigger SPM interrupt


	THERMAL_WRAP_WR32(0x10000, TEMPPROTCTL);// set hot to wakeup event control

	THERMAL_WRAP_WR32(raw_low, TEMPPROTTA);
    if (temperature2 > -275000)
		THERMAL_WRAP_WR32(raw_middle, TEMPPROTTB); // register will remain unchanged if -275000...


	THERMAL_WRAP_WR32(raw_high, TEMPPROTTC);// set hot to HOT wakeup event


	/*trigger cold ,normal and hot interrupt*/
	//remove for temp	THERMAL_WRAP_WR32(temp | 0xE0000000, TEMPMONINT);	// enable trigger SPM interrupt
	/*Only trigger hot interrupt*/
#if 0
	if (temperature2 > -275000)
		THERMAL_WRAP_WR32(temp | 0xC0000000, TEMPMONINT);	// enable trigger middle & Hot SPM interrupt
	else
		THERMAL_WRAP_WR32(temp | 0x80000000, TEMPMONINT);	// enable trigger Hot SPM interrupt
#else
	THERMAL_WRAP_WR32(temp | 0x80000000, TEMPMONINT);	// enable trigger Hot SPM interrupt
#endif			
	temp = DRV_Reg32(TEMPMONINT);
  UTIL_Printf("====================set_tc_trigger_hw_protect=====================\n");
  UTIL_Printf("t1=%d t2=%d, raw_t1=%d(0x%x), raw_t2=%d(0x%x) \n", temperature, temperature2, raw_high, raw_high, raw_middle, raw_middle);
  UTIL_Printf("TEMPPROTCTL=0x%x\nTEMPMONCTL0=0x%x\nTEMPPROTTA=0x%x\nTEMPPROTTB=0x%x\nTEMPPROTTC=0x%x\n",
  					DRV_Reg32(TEMPPROTCTL), DRV_Reg32(TEMPMONCTL0), DRV_Reg32(TEMPPROTTA), DRV_Reg32(TEMPPROTTB), DRV_Reg32(TEMPPROTTC));	
  mtktscpu_get_hw_temp();
  UTIL_Printf("====================set_tc_trigger_hw_protect end==================\n");


#if 0 //debug
{
	int cunt=0,temp1=0;
	cunt=0;
	temp1 = DRV_Reg32(TEMPMSR0)& 0x0fff;
    if(cunt==0)  UTIL_Printf("[Power/CPU_Thermal]0 hw_protect temp=%d , temp = %d, line %d\n",temp1,
	  raw_to_temperature_roomt(temp1,THERMAL_SENSOR1), __LINE__);
    while(temp1 <= 3000 && cunt <20){
		cunt++;
    	UTIL_Printf("[Power/CPU_Thermal]0 hw_protect temp=%d, %d, %d\n",temp1,
    	raw_to_temperature_roomt(temp1,THERMAL_SENSOR1),__LINE__);
        temp1 = DRV_Reg32(TEMPMSR0)& 0x0fff;
        UTIL_Printf("TS_CON1=0x%x\n",DRV_Reg32(TS_CON1));
        udelay(2);
	}

    cunt=0;
	temp1 = DRV_Reg32(TEMPMSR1)& 0x0fff;
    if(cunt==0)  UTIL_Printf("[Power/CPU_Thermal]1 hw_protect temp=%d, temp = %d, line %d\n",temp1,
    	raw_to_temperature_roomt(temp1,THERMAL_SENSOR2),__LINE__);
    while(temp1 <= 3000 &&  cunt <20){
		cunt++;
    	UTIL_Printf("[Power/CPU_Thermal]1 hw_protect temp=%d,%d, %d\n",temp1,
    	raw_to_temperature_roomt(temp1,THERMAL_SENSOR2),__LINE__);
        temp1 = DRV_Reg32(TEMPMSR1)& 0x0fff;
        UTIL_Printf("TS_CON1=0x%x\n",DRV_Reg32(TS_CON1));
    	udelay(2);
	}

	cunt=0;
	temp1= DRV_Reg32(TEMPMSR2)& 0x0fff;
    if(cunt==0) UTIL_Printf("[Power/CPU_Thermal]2 hw_protect temp=%d,temp = %d,%d\n",temp1,
    	raw_to_temperature_roomt(temp1,THERMAL_SENSOR3),__LINE__);
    while(temp1 <= 3000 &&  cunt <20){
		cunt++;
    	UTIL_Printf("[Power/CPU_Thermal]2 hw_protect temp=%d, %d, %d\n",temp1,
    	raw_to_temperature_roomt(temp1,THERMAL_SENSOR3),__LINE__);
        temp1 = DRV_Reg32(TEMPMSR2)& 0x0fff;
        UTIL_Printf("TS_CON1=0x%x\n",DRV_Reg32(TS_CON1));
    	udelay(2);
	}

	cunt=0;
	temp1 = DRV_Reg32(TEMPMSR3)& 0x0fff;
    if(cunt==0)    UTIL_Printf("[Power/CPU_Thermal]3 hw_protect temp=%d %d ,%d\n",temp1,
    	raw_to_temperature_roomt(temp1,THERMAL_SENSOR4),__LINE__);
    while(temp1 <= 3000 &&  cunt <20){
		cunt++;
    	UTIL_Printf("[Power/CPU_Thermal]3 hw_protect temp=%d,%d, %d\n",temp1,
    	raw_to_temperature_roomt(temp1,THERMAL_SENSOR4),__LINE__);
        temp1 = DRV_Reg32(TEMPMSR3)& 0x0fff;
        UTIL_Printf("TS_CON1=0x%x\n",DRV_Reg32(TS_CON1));
    	udelay(2);
	}

}
#endif

	//tscpu_printk("set_tc_trigger_hw_protect 2 TEMPMONINT:temp=0x%x\n",temp);

}


static void tscpu_config_all_tc_hw_protect( int temperature, int temperature2)
{
	UTIL_Printf( "tscpu_config_all_tc_hw_protect,temperature=%d,temperature2=%d,\n",temperature,temperature2);
	set_tc_trigger_hw_protect(temperature,temperature2);
}

/*******************************************************************************
 * FUNCTION
 *	thermal_init
 *
 * DESCRIPTION
 *	This function gives a initial setting and enable irq for thermal controller
 *
 * PARAMETERS
 *  None
 *
 * RETURNS
 *	None
 ******************************************************************************/
static void vDrvThermalISR(UINT16 u2Vector)
{			
    if(u2Vector == VECTOR_PTP_THERM)  
    {
		thermal_lisr();
    }
    VERIFY(BIM_ClearIrq(VECTOR_PTP_THERM));
}

/* thermal export API for SLT */
int thermal_get_temp(void)
{
	unsigned int temp = 0;

	#if 1
    // 0xA0000
	thm_reg_base = (unsigned long)ioremap(0x100A0000, 0x1000);
	// 0x24000
	pdwnc_reg_base = (unsigned long)ioremap(0x10024000, 0x1000);
	// 0x00000
	ckgen_reg_base = (unsigned long)ioremap(0x10000000, 0x1000);
	// 0x54000
	efuse_reg_base = (unsigned long)ioremap(0x10054000, 0x1000);
	#endif

	//UTIL_Printf("open thermal clock start... \n");
	thermal_clk_select(1, 1);
	thermal_clk_enable(1, 1);
	thermal_clk_reset(0);
	thermal_clk_reset(1);
	//UTIL_Printf("open thermal clock end \n");

	vDrvThermal_Cal_Prepare();
	vDrvThermal_Cal_Prepare_2();

	fgStartGetTemp = 0;
	g_getTempThread = kthread_create(get_hw_temp, (void *)NULL, "ThermalThread");
	if (IS_ERR(g_getTempThread)) {
		printk("[thermal] creat get temp thread FAIL! \n");
		g_getTempThread = NULL;
		return -1;
	}
	else
	{
		#if 0
		struct sched_param param;
		int ret;

		param.sched_priority = change_to_sched_priority(60);
		ret = sched_setscheduler_nocheck(g_SLTThread, SCHED_RR, &param);
		if (ret != 0) {
			printk("set priority FAIL ! \n");
			return -1;
		}
		#endif

		printk("[thermal] creat get temp thread OK ! \n");
	}
	wake_up_process(g_getTempThread);

	return 0;
}

/* thermal export API for SLT */
int test_get_temp(void)
{
	//unsigned int temp = 0;

	fgStartGetTemp = 0;
	g_getTempThread = kthread_create(test_get_hw_temp, (void *)NULL, "ThermalThread");
	if (IS_ERR(g_getTempThread)) {
		printk("[thermal] creat get temp thread FAIL! \n");
		g_getTempThread = NULL;
		return -1;
	}
	else
	{
		#if 0
		struct sched_param param;
		int ret;

		param.sched_priority = change_to_sched_priority(60);
		ret = sched_setscheduler_nocheck(g_SLTThread, SCHED_RR, &param);
		if (ret != 0) {
			printk("set priority FAIL ! \n");
			return -1;
		}
		#endif

		printk("[thermal] creat get temp thread OK ! \n");
	}
	wake_up_process(g_getTempThread);

	return 0;
}

int thermal_init(void)
{
//    int temp;
//    int cnt=0;
	//void (*pfnOldIsr) (UINT16); //bdp isr

	UTIL_Printf("thermal_init, --------------- \n");
#if 0
   	thermal_cal_prepare();
	thermal_calibration();

	tscpu_reset_thermal();


	
	/*add this function to read all temp first to avoid
	 write TEMPPROTTC first will issue an fake signal to RGU*/
	tscpu_fast_initial_sw_workaround();    

    while(cnt < 50)
	{
		temp = (DRV_Reg32(THAHBST0) >> 16);
		if(cnt>10)
            UTIL_Printf("THAHBST0 = 0x%x,cnt=%d, %d\n", temp,cnt,__LINE__);
        if(temp == 0x0){
            // pause all periodoc temperature sensing point 0~2
			thermal_pause_all_periodoc_temp_sensing();//TEMPMSRCTL1
			break;
        }
        kal_sleep_task(2);
        cnt++;
	}
	thermal_disable_all_periodoc_temp_sensing();//TEMPMONCTL0

    thermal_initial_all_bank();


    UTIL_Printf("thermal_init: thermal initialized\n");
//    hwEnableClock(MT65XX_PDN_PERI_THERM, "Thermal");

//	PERI_disable_clock(MT65XX_PDN_PERI_THERM);
//	PERI_enable_clock(MT65XX_PDN_PERI_THERM);


	thermal_release_all_periodoc_temp_sensing();//must release before start

	read_all_bank_temperature();
#else
	//thermal_fast_init();
#endif

#if 0
TAG();
    ckgen_pll_all_init();
TAG();
    ckgen_mux_all_init();
    ckgen_cg_all_init();
TAG();
#endif
#if 0
UTIL_Printf("0x94=0x%x\n", DRV_Reg32(0x94));
    DRV_WriteReg32(0x94, ((DRV_Reg32(0x94) & 0xFFFFFC4F) | 0x210)); //write ptp_ck:60MHz,ptp_slow_ck:11kHz
TAG();
UTIL_Printf("0x94=0x%x\n", DRV_Reg32(0x94));
#endif    
    /* register thermal controller interrupt */

#if 0  //// zplee
  	if (x_reg_isr(VECTOR_PTP_THERM, vDrvThermalISR, &pfnOldIsr) != 0)
	{
		
		UTIL_Printf("thermal_init: IRQ register failure\n");
		return -1;
	}

    BIM_EnableIrq(VECTOR_PTP_THERM);
#endif

#if 0	
	tscpu_config_all_tc_hw_protect(90000, 60000);
#endif	
    return 0;
}

/*******************************************************************************
 * FUNCTION
 *	thermal_exit
 *
 * DESCRIPTION
 *	This function restore the default value for all registers
 *
 * PARAMETERS
 *  None
 *
 * RETURNS
 *	None
 ******************************************************************************/
void thermal_exit(void)
{
    UTIL_Printf("thermal_exit: thermal de-initialized, restore to the default value\n");

#if 0  // zplee
    BIM_DisableIrq(VECTOR_PTP_THERM);
#endif

    DRV_WriteReg32(TEMPMONCTL0,         0x00000000);
    DRV_WriteReg32(TEMPMONCTL1,         0x00000000);
    DRV_WriteReg32(TEMPMONCTL2,         0x00000000);
    DRV_WriteReg32(TEMPMONINT,          0x00000000);
    DRV_WriteReg32(TEMPMONIDET0,        0x00000000);
    DRV_WriteReg32(TEMPMONIDET1,        0x00000000);
    DRV_WriteReg32(TEMPMONIDET2,        0x00000000);
    DRV_WriteReg32(TEMPH2NTHRE,         0x00000000);
    DRV_WriteReg32(TEMPHTHRE,           0x00000000);
    DRV_WriteReg32(TEMPCTHRE,           0x00000000);
    DRV_WriteReg32(TEMPOFFSETH,         0x00000000);
    DRV_WriteReg32(TEMPOFFSETL,         0x00000000);
    DRV_WriteReg32(TEMPMSRCTL0,         0x00000000);
    DRV_WriteReg32(TEMPMSRCTL1,         0x00000000);
    DRV_WriteReg32(TEMPAHBPOLL,         0x00000000);
    DRV_WriteReg32(TEMPAHBTO,           0xFFFFFFFF);
    DRV_WriteReg32(TEMPADCPNP0,         0x00000000);
    DRV_WriteReg32(TEMPADCPNP1,         0x00000000);
    DRV_WriteReg32(TEMPADCPNP2,         0x00000000);
    DRV_WriteReg32(TEMPADCMUX,          0x00000000);
    DRV_WriteReg32(TEMPADCEXT,          0x00000000);
    DRV_WriteReg32(TEMPADCEXT1,         0x00000000);
    DRV_WriteReg32(TEMPADCEN,           0x00000000);
    DRV_WriteReg32(TEMPPNPMUXADDR,      0x00000000);
    DRV_WriteReg32(TEMPADCMUXADDR,      0x00000000);
    DRV_WriteReg32(TEMPADCEXTADDR,      0x00000000);
    DRV_WriteReg32(TEMPADCEXT1ADDR,     0x00000000);
    DRV_WriteReg32(TEMPADCENADDR,       0x00000000);
    DRV_WriteReg32(TEMPADCVALIDADDR,    0x00000000);
    DRV_WriteReg32(TEMPADCVOLTADDR,     0x00000000);
    DRV_WriteReg32(TEMPRDCTRL,          0x00000000);
    DRV_WriteReg32(TEMPADCVALIDMASK,    0x00000000);
    DRV_WriteReg32(TEMPADCVOLTAGESHIFT, 0x00000000);
    DRV_WriteReg32(TEMPADCWRITECTRL,    0x00000000);

    DRV_WriteReg32(TEMPSPARE0,          0x00000000);
    DRV_WriteReg32(TEMPSPARE1,          0x00000000);
    DRV_WriteReg32(TEMPSPARE2,          0x00000000);
    DRV_WriteReg32(TEMPSPARE3,          0x00000000);

#if 0 //for TV wukong PWDNC,it's always on.
    temp = DRV_Reg32(PERI_GLOBALCON_RST0);
    temp |= 0x00010000;
    DRV_WriteReg32(PERI_GLOBALCON_RST0, temp);

    temp = DRV_Reg32(PERI_GLOBALCON_RST0);
    temp &= 0xFFFEFFFF;
    DRV_WriteReg32(PERI_GLOBALCON_RST0, temp);
#endif 

}

