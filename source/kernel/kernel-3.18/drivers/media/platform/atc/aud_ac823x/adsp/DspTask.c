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

#define _DSP_TASK_C
/*-----------------------------------------------------------------------------
                    Include header files
-----------------------------------------------------------------------------*/
#include "chip_ver.h"
#include "DspTask.h"
#include "DspVar.h"
#include "AsvDspCtrl.h"
#include "aud_if.h"
#include "GpsMix_AsvTrigger.h"
#include "aud_power.h"
#include "aud_oal.h"
#include "aud_drv_config.h"
#include "aud_comm_os.h"
#include <linux/interrupt.h>

#if CONFIG_DRV_AUD_AC83XX
#include "x_bim.h"
#else
#include "bim.h"
#endif

/*******************************variable define **********************/
volatile static bool g_fgAdspTaskInited = FALSE;
uintptr_t g_hAdspCtrlEvent;

#if CONFIG_AUD_POWER_MANAGEMENT_SUPPORT
void* _hDspAStatusHandle = 0;
void* _hDspBStatusHandle = 0;
#endif


/*******************************function declarations *******************/
static void vADSPTaskUninit(void);
void vADSPTaskExit(void);


/*-----------------------------------------------------------------------------
                    Functions implementations
-----------------------------------------------------------------------------*/
/******************************************************************************
* Function      :vDspSetEvent
* Description   :set event to dsp
* Parameter     :u4Evt: event id
* Return        :
******************************************************************************/
void vDspSetEvent(u32 u4Evt)
{
    VERIFY(OSR_OK == x_ev_group_set_event(g_hAdspCtrlEvent, u4Evt, X_EV_OP_OR));
}

/******************************************************************************
* Function      :vDspWaitEvent
* Description   :wait for event
* Parameter     :
* Return        :
******************************************************************************/
void vDspWaitEvent(u32 u4Event)
{
    EV_GRP_EVENT_T  u8DspEvent;
    VERIFY(OSR_OK == x_ev_group_wait_event(g_hAdspCtrlEvent, u4Event, 
        &u8DspEvent, X_EV_OP_OR_CONSUME));
}

/******************************************************************************
* Function      :
* Description   :
* Parameter     :
* Return        :
******************************************************************************/
static void vADSPIRQEnable(bool fgEnable)
{
    // Enable ADSP interrupt
    if(fgEnable)
    {
    #if CONFIG_DRV_AUD_AC83XX
        BIM_EnableIrq(DSPA_INT_VECTOR);
        BIM_EnableIrq(DSPB_INT_VECTOR);
        #if AUD_DSPC_SUPPORT
        BIM_EnableIrq(DSPC_INT_VECTOR);
        #endif
     #else
	mt33xx_unmask_bim_irq(DSPA_INT_VECTOR);
        mt33xx_unmask_bim_irq(DSPB_INT_VECTOR);
        #if AUD_DSPC_SUPPORT
        mt33xx_unmask_bim_irq(DSPC_INT_VECTOR);
        #endif
     #endif
    }
    else
    {
    #if CONFIG_DRV_AUD_AC83XX
        BIM_DisableIrq(DSPA_INT_VECTOR);
        BIM_DisableIrq(DSPB_INT_VECTOR);
        #if AUD_DSPC_SUPPORT
        BIM_DisableIrq(DSPC_INT_VECTOR);
        #endif
	#else
	mt33xx_mask_bim_irq(DSPA_INT_VECTOR);
	mt33xx_mask_bim_irq(DSPB_INT_VECTOR);
        #if AUD_DSPC_SUPPORT
	mt33xx_mask_bim_irq(DSPC_INT_VECTOR);
        #endif
	#endif

    }
}

/******************************************************************************
* Function      :
* Description   :
* Parameter     :
* Return        :
******************************************************************************/
static void vADSPAIRQHandler(u16 u2Vector)
{
    vDspAIRQSvc();
#if CONFIG_DRV_AUD_AC83XX
    BIM_ClearIrq(DSPA_INT_VECTOR);
#else
    mt33xx_mask_ack_bim_irq(DSPA_INT_VECTOR);
#endif
    vAudHalClearInterupt(AUD_HAL_DSP_ID_A);
    return IRQ_HANDLED;  
}

/******************************************************************************
* Function      :
* Description   :
* Parameter     :
* Return        :
******************************************************************************/
static int vADSPBIRQHandler(u16 u2Vector)
{
    vDspBIRQSvc();
#if CONFIG_DRV_AUD_AC83XX
    BIM_ClearIrq(DSPB_INT_VECTOR);
#else
    mt33xx_mask_ack_bim_irq(DSPB_INT_VECTOR);
#endif
    vAudHalClearInterupt(AUD_HAL_DSP_ID_B);
    return IRQ_HANDLED;
}

#if AUD_DSPC_SUPPORT
static void vADSPCIRQHandler(u16 u2Vector)
{
    vDspCIRQSvc();
#if CONFIG_DRV_AUD_AC83XX
    BIM_ClearIrq(DSPC_INT_VECTOR);
#else
    mt33xx_mask_ack_bim_irq(DSPC_INT_VECTOR);
#endif
    vAudHalClearInterupt(AUD_HAL_DSP_ID_C);
    return IRQ_HANDLED;
}
#endif

void vADSPTaskExit(void)
{
    g_fgAdspTaskInited = FALSE;
    vDspSetEvent(EvDspUop);                     //waken vADSPTaskMain thread

    vDspCmdUnInit();
}

/******************************************************************************
* Function      : vAdspTaskMain
* Description   : main routine for ADSP Task
* Parameter     : None
* Return        : None
******************************************************************************/
static s32 vADSPTaskMain(void* pvArg)
{
    VERIFY(pvArg == NULL);
    // init DspCtrl task
    vDspInit();

    u1AsvDspReady();
    u1AsvDspAReady();
    u1AsvDspBReady();
    
    u1AsvDspAAoutOn();
    u1AsvDspAAout2On();

    u1AsvDspReencReady();

    while (g_fgAdspTaskInited == TRUE)
    {
        vDspState();
        if ((!g_fgDspASInt) && (!g_fgDspBSInt) && (!g_fgDspUop) && (!g_fgDspHUop))
        {
            vDspWaitEvent(EvDspUop | EvDspIsr);
        }
    }

    vADSPTaskUninit();
    
    complete_and_exit(NULL, 0);

	return 0;
}

/******************************************************************************
* Function      : vADSPTaskInit
* Description   : initialization routine for ADSP Task
* Parameter     : None
* Return        : None
******************************************************************************/
static struct task_struct *g_hAdspTaskInitThread = NULL;
void vADSPTaskInit(void)
{   
    x_os_isr_fct pfnOldIsr;

    // register ISR
    //VERIFY(x_reg_isr(DSPA_INT_VECTOR, vADSPAIRQHandler, &pfnOldIsr) == OSR_OK);
    VERIFY(request_irq(DSPA_INT_VECTOR, vADSPAIRQHandler, 0, "ISR_AudioDSPA", NULL) == OSR_OK);
    //VERIFY(x_reg_isr(DSPB_INT_VECTOR, vADSPBIRQHandler, &pfnOldIsr) == OSR_OK);
    VERIFY(request_irq(DSPB_INT_VECTOR, vADSPBIRQHandler, 0, "ISR_AudioDSPB", NULL) == OSR_OK);
    #if AUD_DSPC_SUPPORT
    //VERIFY(x_reg_isr(DSPC_INT_VECTOR, vADSPCIRQHandler, &pfnOldIsr) == OSR_OK);
    VERIFY(request_irq(DSPC_INT_VECTOR, vADSPCIRQHandler, 0, "ISR_AudioDSPC", NULL) == OSR_OK);
    #endif

    vADSPIRQEnable(TRUE);
    // create event
    VERIFY(AUD_OK == x_ev_group_create(&g_hAdspCtrlEvent, "DspCtrlEvent", 0));

  #if CONFIG_AUD_POWER_MANAGEMENT_SUPPORT
    _hDspAStatusHandle = x_event_create(NULL, FALSE, FALSE, TEXT("DspAStatusEvent"));
    _hDspBStatusHandle = x_event_create(NULL, FALSE, FALSE, TEXT("DspBStatusEvent"));
  #endif

    // init ASV state
    vAudStateInit();
    vAudGpsMixStateInit();
    // enter ASV initial state
    u1AsvDspInit();
    u1AsvDspAInit();
    u1AsvDspBInit();
    u1AsvDspReencInit();

    g_fgAdspTaskInited = TRUE;
    // Create ADSP task
    g_hAdspTaskInitThread = kthread_create(vADSPTaskMain, (void *)NULL, "ADSPTASK_NAME");
	if (IS_ERR(g_hAdspTaskInitThread)) {
		LOG(LOG_CTRLF, TEXT("[vADSPTaskInit]vADSPTaskMain thread create fail \r\n"));
		g_hAdspTaskInitThread = NULL;
		return;
	}
    else
    {
        struct sched_param param;
        s32 ret;

        param.sched_priority = to_sched_priority(ADSPTASK_THREAD_PRIORITY);
        ret = sched_setscheduler_nocheck(g_hAdspTaskInitThread, SCHED_RR, &param);
        ASSERT(ret == 0);
    }
	wake_up_process(g_hAdspTaskInitThread);

    g_fgAdspTaskInited = TRUE;
}



/******************************************************************************
* Function      : vADSPTaskUninit
* Description   : uninitialization routine for ADSP Task
* Parameter    : None
* Return         : None
******************************************************************************/
static void vADSPTaskUninit(void)
{
    x_os_isr_fct pfnOldIsr;

    vADSPIRQEnable(FALSE);
    
    #if 0
    VERIFY(x_reg_isr(DSPA_INT_VECTOR, NULL, &pfnOldIsr) == OSR_OK);
    VERIFY(x_reg_isr(DSPB_INT_VECTOR, NULL, &pfnOldIsr) == OSR_OK);
    VERIFY(x_reg_isr(DSPC_INT_VECTOR, NULL, &pfnOldIsr) == OSR_OK);    
    #else
    free_irq((unsigned int)DSPA_INT_VECTOR, NULL);
    free_irq((unsigned int)DSPB_INT_VECTOR, NULL);
    #if AUD_DSPC_SUPPORT
    free_irq((unsigned int)DSPC_INT_VECTOR, NULL);
    #endif
    #endif
    
    VERIFY(x_ev_group_delete(g_hAdspCtrlEvent) == OSR_OK);

  #if CONFIG_AUD_POWER_MANAGEMENT_SUPPORT
    if(_hDspAStatusHandle != NULL)
    {
        #ifndef __linux__
        CloseHandle(_hDspAStatusHandle);
        #else
        x_event_destroy(_hDspAStatusHandle);
        #endif
    }
    
    if(_hDspBStatusHandle != NULL) 
    {
        #ifndef __linux__
        CloseHandle(_hDspBStatusHandle);
        #else
        x_event_destroy(_hDspBStatusHandle);
        #endif
    }
  #endif
}
