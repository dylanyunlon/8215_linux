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
//#include <windows.h>
//#include "blcommon.h"
//#include <pehdr.h>
//#include <romldr.h>
#ifndef __KERNEL__
#define __KERNEL__
#endif


#include "auto_version.h" // BDP_Generic/src/system/prop/version/auto_version.h
#include "x_types.h"
#include "mem_operation.h"
#include "x_bim.h"
#include "x_ckgen.h"
#include "dual_task.h"
#include "x_pdwnc.h"
#include "x_ver.h"
#include "backcar_cfg.h"
#include <generated/atc_project.h>

#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>


/*
*  Comment by Ke Xu ? must be fix it~
*/
//#include "tvd_drv_if.h"
#include "metazone_inter.h"

#define ARM2_TVD_DRV
#define TAGS "MAIN"

#include "u_os.h"
#include "printf.h"
#define ARM2_IO_TEST  1
#ifdef ARM2_IO_TEST
#include "ac823x_gpio_pinmux.h"
#include "ac823x_pinmux_table.h"
void arm2_gpio_test(void);

#include "ac823x_uart.h"
void arm2_uart_test(void);
#endif

extern unsigned int logo_base;
BOOL fgIRTDone = FALSE;
extern void ResizeAndUpdateVDOHw(UINT32 u4VdpIdx);


extern x_os_isr_fct tvd_isr_proc;

extern void YbrIsr(UINT16 u2Vector);


//=============================
// define
//=============================
//version info
#define ARM2_VER_NAME    ("ARM2")
#define ARM2_VER_MAIN     01
#define ARM2_VER_MINOR  00
#define ARM2_VER_REV       00

/* timer tick */
#define REG_RW_T64b_LO_0   0x0728
#define REG_RW_T64b_HI_0   0x072C
#define REG_RW_T64b_EN_0   0x0730
#define T64B_INIT()	\
	BIM_WRITE32(REG_RW_T64b_EN_0, 0); \
	BIM_WRITE32(REG_RW_T64b_LO_0, 0); \
	BIM_WRITE32(REG_RW_T64b_HI_0, 0); \
	BIM_WRITE32(REG_RW_T64b_EN_0, 1)
#define BASE_ADDR 0x00020000//128k       //ARM2_OFFSET_BASE + 128K
#define END_ADDR  0x00120000//1M size    //ARM2_OFFSET_BASE + 1M
#define WRITEMEM(Address, Value) *(unsigned int *)(Address) = Value
#define READMEM(Address)         *(unsigned int *)(Address)
#define PSR_IRQ_DISABLE             (1 << 7)
#define PSR_FIQ_DISABLE             (1 << 6)

//=============================
// io macro
//=============================


//=============================
// globl variable
//=============================
//ROMHDR * volatile const pTOC = (ROMHDR *)-1;
//ROMHDR * volatile pTOC = (ROMHDR *)-1;


//=============================
// functions
//=============================
//extern void uartTestTaskMain(void);
extern UINT32 fgDualHALGetOffset(void);
extern int interrupt_init (void);
extern void v_enable_bim_irq(UINT32 u4Id);
extern void v_timer_interrupt_init(void);
extern void Enable_IRQ(void);
extern void Disable_IRQ(void);
extern UINT32 u4_get_irq_vector_id(void);
extern void v_clear_bim_irq(UINT32 u4Id);
extern void vdecISR();
extern void DisplayVsyncInit(void);
extern void SendVsync(int event);
extern void platform_early_init(void);

//
// KernelRelocate: move global variables to RAM
//
#if 0
static BOOL KernelRelocate (ROMHDR *const pTOC)
{
    ULONG loop;
    COPYentry *cptr;
    if (pTOC == (ROMHDR *const) -1)
    {
        return (FALSE); // spin forever!
    }
    // This is where the data sections become valid... don't read globals until after this
    for (loop = 0; loop < pTOC->ulCopyEntries; loop++)
    {
        cptr = (COPYentry *)(pTOC->ulCopyOffset + loop*sizeof(COPYentry));
        if (cptr->ulCopyLen)
            memcpy((LPVOID)cptr->ulDest,(LPVOID)cptr->ulSource,cptr->ulCopyLen);
        if (cptr->ulCopyLen != cptr->ulDestLen)
            memset((LPVOID)(cptr->ulDest+cptr->ulCopyLen),0,cptr->ulDestLen-cptr->ulCopyLen);
    }
    return (TRUE);
}
#endif


BOOL BL_DisplayInit();

#if defined(CONFIG_ATC_PLATFORM_ac823x)
#define PDWNC_BASE 0x10024000
#endif


/*-----------------------------------------------------------*/
static void exampleTask1( void * parameters )
{
	/* Unused parameters. */
	( void ) parameters;

	Printf("example task1 \n");
	for( ; ; )
	{
		/* Example Task Code */
		vTaskDelay( 5000 ); /* delay 100 ticks */
		Printf("task1 delay 5s\n");
	}
}

static void exampleTask2( void * parameters )
{
	/* Unused parameters. */
	( void ) parameters;

	Printf("example task2 for mainloop \n");
#if 0
	for( ; ; )
	{
		/* Example Task Code */
		vTaskDelay( 10000 ); /* delay 100 ticks */
		Printf("task2 delay 10s\n");
	}
#else
	Printf("[Arm2] arm2 MainLoop start time %d\n", GetBootTime());
	Printf("[Arm2] arm2 MainLoop start time %d, GetUpgradeMode: %d\n", GetBootTime(), fgDualHALGetUpgradeMode());
	MainLoop();
#endif
}

int cmain(void)
//int main(void)
{
	static StaticTask_t exampleTaskTCB;
	static StackType_t exampleTaskStack[ configMINIMAL_STACK_SIZE ];
	BaseType_t xReturn = pdFAIL;

	platform_early_init();
#if 0
    // relocate globals to RAM
    //if (pTOC == (ROMHDR *const) -1)
    {
       //image start (in arm2.bib) + offset
       //some reason, ce romimage.exe  don't fix pTOC :(
       pTOC =  (ROMHDR *) (*((UINT32 *)(0x1000 + ROM_TOC_POINTER_OFFSET)));
    }
    if (!KernelRelocate (pTOC))
    {
        // spin forever
    }
#endif
	Printf("[Arm2] arm2 start time %d\n", GetBootTime());
	  /* timer init */
	  //T64B_INIT();  //remove, init at preloader
   	Printf("Arm2 Build Time %s %s \n", __DATE__, __TIME__);
	Printf("[VER][ARM2] V%02d.%02d_%02d_%06d [%s] [Time] %s %s %s\r\n", ARM2_VER_MAIN, ARM2_VER_MINOR, ARM2_VER_REV, P4_CHANGELIST, BRANCH_NAME, (__DATE__),(__TIME__), AUTO_BUILD);

  	//Printf("Arm2 Main Enter.. \n");
  	fgDualHALGetOffset();
  	interrupt_init();
  	v_enable_bim_irq(VECTOR_TOCORISC);
  	v_timer_interrupt_init();
  //	Enable_IRQ();

	Metazone_Init();

	BootAnimation_Init();
	AwtkTaskInit();
	arm2system_service_init();
	DisplayVsyncInit();

	Printf( "Task start FreeRTOS scheduler\n" );

	//uartTestTaskMain();//Demo code for uart interrupt mode and polling mode.


	/* Start the scheduler. */
	vTaskStartScheduler();
	//arm2_gpio_test();  //for gpio test
	//arm2_uart_test();  //for uart test
	Printf("[Arm2] arm2 MainLoop start time %d\n", GetBootTime());
	Printf("[Arm2] arm2 MainLoop start time %d, GetUpgradeMode: %d\n", GetBootTime(), fgDualHALGetUpgradeMode());


	return(0);
}
void vApplicationMallocFailedHook(void)
{
    /* Called if a call to pvPortMalloc() fails because there is insufficient
    free memory available in the FreeRTOS heap.  pvPortMalloc() is called
    internally by FreeRTOS API functions that create tasks, queues, software
    timers, and semaphores.  The size of the FreeRTOS heap is set by the
    configTOTAL_HEAP_SIZE configuration constant in FreeRTOSConfig.h. */
    int count = 10;
    // 修改汇编指令格式以提高兼容性
    Disable_IRQ();
    dump_callback();
    for(;;) {
        if(count-- > 0)
            pr_err("vApplication Malloc Failed!\n");
        else
            while(1);
    }
}
void vApplicationStackOverflowHook( TaskHandle_t xTask, char * pcTaskName )
{
	( void ) xTask;

    /* Run time stack overflow checking is performed if
    configCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2.  This hook
    function is called if a stack overflow is detected. */
    int count = 10;
    // 修改汇编指令格式以提高兼容性
    Disable_IRQ();
    dump_callback();
    for(;;) {
        if(count-- > 0)
            pr_err("Application Stack Overflow! pcTaskName:%s\n", pcTaskName);
        else
            while(1);
    }
}


void AudHalGPSAoutIsr(UINT16 u2Vector);
#ifndef IRQ_BIM_GIC_OFFSET
#define IRQ_BIM_GIC_OFFSET 64
#endif
#ifdef CONFIG_ATC_PLATFORM_ac823x
extern void PmxVerifyMainIsr(UINT16 u2Vector);
#endif
void arm2_ISR(void)
{
    UINT32 u4VectorId;

    u4VectorId = u4_get_irq_vector_id();
    switch(u4VectorId)
    {
#ifdef CONFIG_ATC_PLATFORM_ac823x
    case VECTOR_T0:
        TimerLoop();
        break;

    case VECTOR_TOCORISC:
        vDualIsr();
        BIM2_WRITE32(REG_RW_IRQCLR, TOCORISC_INTR_CLR);
        break;

#ifdef ARM2_TVD_DRV
    case VECTOR_VDOIN:
		if(NULL != tvd_isr_proc)
		tvd_isr_proc(VECTOR_VDOIN);
        break;
#endif

#if (BACK_CAR_SRC_VGA ||BACK_CAR_SRC_YPBPR)
    case  VECTOR_YPBPRINT:
        YbrIsr(VECTOR_YPBPRINT);
        break;
#endif
    case 45://149-104
    	vPmxHalMainIsr(45,NULL);
		break;
    case VECTOR_AOUT_2ND_RC:
        AudHal_ISR(VECTOR_AOUT_2ND_RC);
        break;
    case 59:
        Wch_Isr(59, 0);
        break;
#else
    case VECTOR_T0:
        TimerLoop();
        break;
    case VECTOR_T1:
        vTickISR();
        break;
    case VECTOR_TOCORISC:
        vDualIsr();
        BIM2_WRITE32(REG_RW_IRQCLR, TOCORISC_INTR_CLR);
        break;

    case VECTOR_UART_1:
        //Printf("uart1_int_trigger\n");
        uart1_irq_handler();
        break;

    case VECTOR_UART_2:
        //Printf("uart2_int_trigger\n");
        uart2_irq_handler();
        break;

    case VECTOR_UART_3:
        //Printf("uart3_int_trigger\n");
        uart3_irq_handler();
        break;

    case VECTOR_UART_4:
        //Printf("uart4_int_trigger\n");
        uart4_irq_handler();
        break;

    case VECTOR_UART_5:
        //Printf("uart5_int_trigger\n");
        uart5_irq_handler();
        break;

#ifdef ARM2_TVD_DRV
    case VECTOR_VDOIN:
		if(NULL != tvd_isr_proc)
		tvd_isr_proc(VECTOR_VDOIN);
        break;
#endif

#if (BACK_CAR_SRC_VGA ||BACK_CAR_SRC_YPBPR)
    case  VECTOR_YPBPRINT:
        YbrIsr(VECTOR_YPBPRINT);
        break;
#endif
	case VECTOR_DDMANEW:
			fgIRTDone = TRUE;
			//Printf("IRT Rotate done!\r\n");
			ResizeAndUpdateVDOHw(0); //0, VDP1, front; 1, VDP2, rear
			break;

    case VECTOR_VSYNC:
		SendVsync(0);
        vPmxHalMainIsr(VECTOR_VSYNC, 0);
		break;
    case VECTOR_PANEL_SCALER:
        vSclHalIsr(VECTOR_PANEL_SCALER, 0);
        break;
    case VECTOR_AOUT_2ND_RC:
        AudHal_ISR(VECTOR_AOUT_2ND_RC);
        break;
    case VECTOR_WCHNL:
        WchIsr(VECTOR_WCHNL+IRQ_BIM_GIC_OFFSET);
        break;

    case VECTOR_WCHNL2:
        WchIsr(VECTOR_WCHNL2+IRQ_BIM_GIC_OFFSET);
        break;
#endif
    case VECTOR_VDFUL:
        vdecISR();
        break;
		default:
			break;
    }
    v_clear_bim_irq(u4VectorId);
}

#if  0 //ARM2_IO_TEST
void arm2_gpio_test(void)
{
	UINT32 i,j=0;

	GPIO_MultiFun_Set(PIN_143_UTXD0, PINMUX_LEVEL_GPIO_END_FLAG); //set pinmux

	ac823x_gpio_set_value_reg(PIN_143_UTXD0, 1);  //set gpio output value
	ac823x_gpio_inout_sel_reg(PIN_143_UTXD0, OUTPUT);  //set gpio output direction
	i=9;
	while(i--)
	{
		ac823x_gpio_set_value_reg(PIN_143_UTXD0, 1);  //set gpio output 1
		for(j=10000; j>0; j--);

		ac823x_gpio_set_value_reg(PIN_143_UTXD0, 0);  //set gpio output 0
		for(j=10000; j>0; j--);
	}
}

void arm2_uart_test(void)
{
	int i;
	BYTE bData = 0x0;
	UartInit(115200);  //default use uart2, select uart in HWinit function
	for(i=0; i<100; i++)
	{
		WriteByte(bData);
		bData ++;
	}
	for(i=0; i<100; i++)
		Printf("data=%x\n", ReadByte());
}

#endif


#if 0

#define PDWNC_READ32(offset)            IO_READ32(PDWNC_BASE, offset)
#define PDWNC_WRITE32(offset, value)    IO_WRITE32(PDWNC_BASE, offset, (value))



#define REG_RW_PAD_PINMUX1				0x0F4
#define REG_RW_GPIOIN                 0x0D0
#define REG_RW_GPIOEN                 0x0D4
#define REG_RW_GPIOOUT                0x0D8
#define REG_RW_EXINTCFG			  0xc0    //External interrupt configuration register
#define REG_RW_PDIO			  0xc4

void  Set_PDWN_GPIO_value(UINT32 u4Pin,UINT32 u4value)
{

	UINT32 u4Mode =0;
	UINT32 u4Tmp;

//set pin to GPIO
	u4Tmp = (1<<  u4Pin);
	PDWNC_WRITE32(REG_RW_PAD_PINMUX1, u4Tmp);


//GPIO direct
        u4Tmp = PDWNC_READ32(REG_RW_GPIOEN);

        if(u4Mode == 1)
	   u4Tmp = u4Tmp & (~(1U << u4Pin));
	else
	   u4Tmp = u4Tmp | ((1U << u4Pin));

        PDWNC_WRITE32(REG_RW_GPIOEN, u4Tmp);

//set output value
        u4Tmp = PDWNC_READ32(REG_RW_GPIOOUT);
	u4Tmp = u4Tmp & ( ~(1 << u4Pin));
	u4Tmp = u4Tmp | (u4value << u4Pin);
        if(u4Mode == 0)
	   PDWNC_WRITE32(REG_RW_GPIOOUT,u4Tmp);



}
#endif

void vMainAssertCalled(const char *pcFileName, uint32_t ulLineNumber)
{
    Disable_IRQ();
    Printf("[E][ARM2-PANIC] ASSERT! %s : %d \n", pcFileName, ulLineNumber);
    dump_callback();

    for(;;);
}
