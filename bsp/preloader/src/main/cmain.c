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
#include "../drv_cust/ac8317_m1v1_v00.h"
#include "../security/tz_init.h"

extern struct quickboot_param qb_param;
extern int loader(void);
extern void v_dram_bist(void);
void Menu_Config(void);
void SetStack2Mem(void);
extern void cpu_Resume();
extern void hardware_init(void);
extern UINT32 RunInNor(void);
#ifdef __AndroidM__
extern void GotoTZ();
#endif

extern UINT32 _dramk_start;
extern UINT32 _loader_start;
extern UINT32 _loader_end;


#define CPSR_MODE_IRQ  0x12 
#define	CPSR_MODE_SVC	0x13
#define CPSR_MODE_FIQ  0x11
#define CPSR_MODE_SYS  0x1F
#define CPSR_MODE_USR  0x10
#define CPSR_MODE_UND  0x1B
#define CPSR_MODE_ABT  0x17

#define	CPSR_BIT_I		0x80
#define	CPSR_BIT_F		0x40


//extern UINT32 _bss_start;
//extern UINT32 _end;

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

//=============================
// io macro
//=============================


//=============================
// globl variable
//=============================


//=============================
// functions
//=============================
unsigned int boot_time_ms(void)
{
	volatile unsigned int time = 0;
	
	/***
	* Register F000814C, which was triggered by BootROM, 
	* start with 0xFFFFFFFF, end with 0x00000000,
	* decrease with every 27M crystal oscillation.
	*/
	time = (0xFFFFFFFF - (*((volatile unsigned int*)(0xF000814C)))) / 27000;
	return time;
}

static inline void _imb(void)
{
	INT32 r = 0;
	
    asm volatile(
		"mcr 	p15, 0, %0, c7, c10, 5\n"
		"mcr	p15, 0, %0, c7, c5, 6\n"
		:
		: "r"(r)
		: "cc"
    );
}

void NFI_SetPadDriver(UINT32 u4drv)
{
	MASKMEM(REG_PAD_MSDC_CFG19,BIT_PIN_DRV(u4drv),BIT_PIN_DRV_MASK);	   // set MSDC0_8b_clk drv shared with NFI to 2ma
	MASKMEM(REG_PAD_MSDC_CFG20,BIT_PIN_DRV(u4drv),BIT_PIN_DRV_MASK);	   // set MSDC0_8b_cmd drv shared with NFI to 2ma
	MASKMEM(REG_PAD_MSDC_CFG21,BIT_PIN_DRV(u4drv),BIT_PIN_DRV_MASK);	   // set MSDC0_8b_dat0 drv shared with NFI to 2ma
	MASKMEM(REG_PAD_MSDC_CFG22,BIT_PIN_DRV(u4drv),BIT_PIN_DRV_MASK);	   // set MSDC0_8b_dat1 drv shared with NFI to 2ma
	MASKMEM(REG_PAD_MSDC_CFG23,BIT_PIN_DRV(u4drv),BIT_PIN_DRV_MASK);	   // set MSDC0_8b_dat2 drv shared with NFI to 2ma
	MASKMEM(REG_PAD_MSDC_CFG24,BIT_PIN_DRV(u4drv),BIT_PIN_DRV_MASK);	   // set MSDC0_8b_dat3 drv shared with NFI to 2ma
	MASKMEM(REG_PAD_MSDC_CFG25,BIT_PIN_DRV(u4drv),BIT_PIN_DRV_MASK);	   // set MSDC0_8b_dat4 drv shared with NFI to 2ma
	MASKMEM(REG_PAD_MSDC_CFG26,BIT_PIN_DRV(u4drv),BIT_PIN_DRV_MASK);	   // set MSDC0_8b_dat5 drv shared with NFI to 2ma
	MASKMEM(REG_PAD_MSDC_CFG27,BIT_PIN_DRV(u4drv),BIT_PIN_DRV_MASK);	   // set MSDC0_8b_dat6 drv shared with NFI to 2ma
	MASKMEM(REG_PAD_MSDC_CFG28,BIT_PIN_DRV(u4drv),BIT_PIN_DRV_MASK);	   // set MSDC0_8b_dat7 drv shared with NFI to 2ma    
}

#if defined(Config_WinCE)
static unsigned int _raw_readl(unsigned int ptr)
{
	return *((volatile unsigned int *)ptr);
}

static void _raw_writel(unsigned int value, unsigned int ptr)
{
	*((volatile unsigned int *)ptr) = value;
}

#define SPM_BASE_VA 0xF0048000 //SPM Virtual Address in Linux Kernel
#define SPM_READ32(REG)    _raw_readl(SPM_BASE_VA+REG)
#define SPM_WRITE32(VAL,REG)    _raw_writel(VAL,SPM_BASE_VA+REG)


void platform_cpu_kill(unsigned cpu)
{
    UINT32 rval = 0;

    SPM_WRITE32(0x02860001,0);   //Enable SPM clock

    
    //Printf("platform_cpu_kill; id = %d start\n", cpu);

    if (cpu == 1)
    {
        //power down core1 sram with hardware control
        //write core1_mem_pd_sel = 1 and core1_mem_pd_hw = 1
        rval = SPM_READ32(0x0c);
        rval &= 0x0000ffff;
        rval |= ((0x0286 << 16) | (1 << 6)|(1 << 5));
        SPM_WRITE32(rval,0x0c);

        //wait core1 sram power down ack == 1
        while(!(SPM_READ32(0x7c) & (1 << 12)));

        //power down core1
        //write core1_pwr_iso = 1
        rval = SPM_READ32(0x0c);
        rval &= 0x0000ffff;
        rval |= (1 << 1);
        rval |= (0x0286 << 16);
        SPM_WRITE32(rval,0x0c);

        //write core1_pwr_rst_ = 0 and core1_clock_dis = 1
        rval = SPM_READ32(0x0c);
        rval &= 0x0000ffff;
        rval &= (~0x01);
        rval |= ((0x0286 << 16) | (1 << 4));
        SPM_WRITE32(rval,0x0c);

        //write core1_pwr_on = 0 and core1_pwr_on_s = 0
        rval = SPM_READ32(0x0c);
        rval &= 0x0000ffff;
        rval &= (~0x0c);
        rval |= ((0x0286 << 16));
        SPM_WRITE32(rval,0x0c);

        //wait core1 power down ack = 0
        while((SPM_READ32(0x7c) & (1 << 14)));
        //wait core1 power down ack_s = 0
        while((SPM_READ32(0x7c) & (1 << 13)));
        
    }
    else if (cpu == 2)
    {
        //power down core2 sram with hardware control
        //write core2_mem_pd_sel = 1 and core2_mem_pd_hw = 1
        rval = SPM_READ32(0x0c);
        rval &= 0x0000ffff;
        rval |= ((0x0286 << 16) | (1 << 14)|(1 << 13));
        SPM_WRITE32(rval,0x0c);

        //wait core2 sram power down ack == 1
        while(!(SPM_READ32(0x7c) & (1 << 8)));

        //power down core2
        //write core2_pwr_iso = 1
        rval = SPM_READ32(0x0c);
        rval &= 0x0000ffff;
        rval |= (1 << 9);
        rval |= (0x0286 << 16);
        SPM_WRITE32(rval,0x0c);

        //write core2_pwr_rst_ = 0 and core2_clock_dis = 1
        rval = SPM_READ32(0x0c);
        rval &= 0x0000ffff;
        rval &= (~(0x01 << 8));
        rval |= ((0x0286 << 16) | (1 << 12));
        SPM_WRITE32(rval,0x0c);

        //write core2_pwr_on = 0 and core2_pwr_on_s = 0
        rval = SPM_READ32(0x0c);
        rval &= 0x0000ffff;
        rval &= (~(0x03 << 10));
        rval |= ((0x0286 << 16));
        SPM_WRITE32(rval,0x0c);

        //wait core2 power down ack = 0
        while((SPM_READ32(0x7c) & (1 << 10)));
        //wait core2 power down ack_s = 0
        while((SPM_READ32(0x7c) & (1 << 9)));
        
    }
    else if (cpu == 3)
    {
        //power down core3 sram with hardware control
        //write core3_mem_pd_sel = 1 and core3_mem_pd_hw = 1
        rval = SPM_READ32(0x10);
        rval &= 0x0000ffff;
        rval |= ((0x0286 << 16) | (1 << 6)|(1 << 5));
        SPM_WRITE32(rval,0x10);

        //wait core3 sram power down ack == 1
        while(!(SPM_READ32(0x7c) & (1 << 4)));

        //power down core3
        //write core3_pwr_iso = 1
        rval = SPM_READ32(0x10);
        rval &= 0x0000ffff;
        rval |= (1 << 1);
        rval |= (0x0286 << 16);
        SPM_WRITE32(rval,0x10);

        //write core3_pwr_rst_ = 0 and core3_clock_dis = 1
        rval = SPM_READ32(0x10);
        rval &= 0x0000ffff;
        rval &= (~0x01);
        rval |= ((0x0286 << 16) | (1 << 4));
        SPM_WRITE32(rval,0x10);

        //write core3_pwr_on = 0 and core3_pwr_on_s = 0
        rval = SPM_READ32(0x10);
        rval &= 0x0000ffff;
        rval &= (~0x0c);
        rval |= ((0x0286 << 16));
        SPM_WRITE32(rval,0x10);

        //wait core3 power down ack = 0
        while((SPM_READ32(0x7c) & (1 << 6)));
        //wait core3 power down ack_s = 0
        while((SPM_READ32(0x7c) & (1 << 5)));
        
    }
    else
    {
        //out of cpu id
    }
        
    SPM_WRITE32(0x02860000,0);

    //Printf("platform_cpu_kill; id = %d end\n", cpu);

}
#endif

void copy_boot_code()
{
   UINT32 *src = (UINT32 *)0xF4000000;
   volatile UINT32 *dst = 0x0;
   int i = 0;
   for(i = 0; i < 512;i+=4){

      *dst = *src;
      dst++;
      src++;
   }
   return ;

}

void print_preloader_info()
{
    Printf("+++++++++++++++++\n");
    Printf("[preloader] start boot at :%u\n", boot_time_ms());
    //Printf("++ Build time: " __DATE__ " " __TIME__"\n" );
    Printf("++ Commitid: " COMMITID  " \n");
    Printf("++ Last date: " LAST_CHANGE_TIME " \n");
    Printf("+++++++++++++++++\n");
}


int cmain(void)
{
	/* timer init */

	UINT32 testData;
	UINT32 n;


	//PrintEfuse();
	hardware_init();

	print_preloader_info();
  
#if defined(Config_WinCE)
  platform_cpu_kill(1);
  platform_cpu_kill(2);
  platform_cpu_kill(3);
#endif

#ifdef config_DVT

  #if (ICE_DEBUG_ENABLE)
  volatile UINT32 iceDebug = 1;
  while(iceDebug == 1);
  #endif


  Printf("[[[[[[cpu0 start]]]]]\n");
  //while(1);
  
  #if (DDR_SETTING_ENABLE)
  Printf("DDR Initialize start");
  DDR_Initialize();
  Printf("DDR Initialize finish");
  #endif  

  
  #if (CHIP_TEST_ENABLE)
  Printf(" Chip_Test start");
  Chip_Test(0);
  Printf(" Chip_Test finish");
  #endif
  
    
  #if(AP_INIT_TVE_ENABLE)
  Printf("AP Init TVE start");
   ApInitTVE();
  Printf("AP Init TVE finish");
  #endif
  
    
   IO_MASK(IO_BASE,0x831C,(0x3<<0),(0x3<<0));//[0]:DVP,[1]:Rambuf set 1 to bypass bus clock dynamic gating

  while(1);

  #if (AP_BOOTUP_DVD_ENABLE)
    Printf("AP boot up dvp start\n");
    ApBootupDvd();
  #endif

  
  

  
  
	
	#if (ICE_DEBUG_ENABLE)
	while(iceDebug == 1);
	#endif
	/* exec dramk */

	//move_loader_section();
	
	/* exec loader */
	_imb();

  //loader();
	//((FUNC_CALL)((UINT32)loader) - ((UINT32)&_loader_start - (UINT32)&_dramk_start))();

#else

  T64B_INIT();
  
#if defined(Config_WinCE)

#if defined(BOOTDEVICE_NAND)
	NFI_SetPadDriver(5);
#endif

#endif

	if(get_boot_type() == QUICK_BOOT)
	{
		Printf("save register address(V2):0x%x\n",(UINT32)&_loader_start);
                DDR_EnterResume(&_loader_start, qb_param.ddr_cal_addr >>4);
		Printf("resume finish\n");
		#ifndef Config_WinCE
		//copy_boot_code();
		#endif
		///
		set_opwrsb_mode(1);
		set_opwrsb_function(1, 0);
#ifdef __AndroidM__
        	GotoTZ();
#endif		
		cpu_Resume();
		while(1);


	}else{

#if defined(BOOTDEVICE_SD)
    if(get_boot_type() == EMMC_BOOT){
            set_boot_type(NORMAL_BOOT);
            sdmmc_loader(MSDC_CH1);

    }else {
		///
		set_opwrsb_mode(1);
		set_opwrsb_function(1, 0);

	     sdmmc_loader(MSDC_CH3);
         #if !FAST_BOOT_FOLLOW
         sdmmc_loader(MSDC_CH2);
         #endif
         sdmmc_loader(MSDC_CH1);
    }	
#elif defined(BOOTDEVICE_NAND)
  RunInNand();

#elif defined(BOOTDEVICE_EMMC)
	
	sdmmc_loader(MSDC_CH1);
	
#elif defined(BOOTDEVICE_NOR)

	RunInNor();

#endif
	}

#endif //config_DVT

}

void CPU1_Main(void)
{
	Printf("[[[[[[cpu1 start]]]]]\n");

#if (CHIP_TEST_ENABLE)
	Chip_Test(1);
#endif

	while(1);
}









