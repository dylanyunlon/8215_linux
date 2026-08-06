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

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/input.h>
#include <linux/io.h>
#include <linux/delay.h>
#include <linux/kthread.h>
#include <linux/timer.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/interrupt.h>
#include <linux/jiffies.h>
#include <generated/atc_project.h>
#include <linux/uaccess.h>

#include <linux/gpio.h>

#include "mhl_private.h"
#include "x_typedef.h"
#include "x_os.h"
#include "x_printf.h"
#include "x_stl_lib.h"
#include "x_assert.h"

#include "x_bim.h"
#include "drv_thread.h"
#include "drv_av_d.h"
#include "x_timer.h"
#include "hdmi_rx_task.h"
#include "drv_hdmi.h"
#include "drv_hdmi_rx.h"
/* #include "x_gpio.h" */
/* #if (DRV_SUPPORT_HDMI_RX) */
#include "drv_hdmi_rx.h"
#include "rx_io.h"
#include "drv_vdout.h"
#include "edid_eeprom.h"
#include "edid_data.h"
#include "spdif_if.h"
#include "x_audin.h"
#include "mhl_drv_if.h"

#include "hdmi_rx_ctrl.h"
#include "video_timing.h"
#include "hdmi_rx_dvi.h"
#include "hdmi_rx_hal.h"
#include "vsw_drv_if.h"
#include "hdmi_rx_aud_task.h"
#include "x_pdwnc.h"
#include "mhl_rx_cbus_ctrl.h"
#include "mhl_rx_cbus.h"

#include "drv_gcpu_if.h"
#include "drv_gcpu_errcode.h"
#include "mhl_drv.h"
#include "mhl_rx_cbus_hw.h"
#include "hdmi_debug.h"
#include <generated/atc_project.h>



unsigned int hdmi_rx_log_on;

/* HDMI Timer */

#define HDMI_RX_TMR_ISR_TICKS   (jiffies+((20*HZ)/1000))/* now 20ms for MT8530  milliseconds. */
#define MHL_RX_CBUS_TMR_ISR_TICKS   (jiffies+((10*HZ)/1000))




#define MAX_HDMI_RX_TMR_NUMBER  4

#define HDMI_UNUSED_RX_TIMER    (-1)
#define HDMI_RX_CTRL_TMR_INX  0
#define HDCP_RX_CTRL_TMR_INX  1
#define HDMI_RX_DELAY_TMR_INX  2
#define HDMI_RX_SRV_TMR_INX  3



/*Hdmi Rx Thread*/
#ifndef HDMIRXTASK_NAME
#define HDMIRXTASK_NAME                      "HDMIRxTask Thread"
#define HDMIRXTASK_STACK_SIZE                2048
#define HDMIRXTASK_THREAD_PRIORITY           PRIORITY(PRIORITY_CLASS_NORMAL, PRIORITY_LAYER_DRIVER, 0)
#endif

#define MHLRXCBUSLINKTASK_NAME               "HDMIRxCbusLink Thread"
#define MHLRXCBUSLINKTASK_STACK_SIZE         2048
#define MHLRXCBUSLINKTASK_THREAD_PRIORITY    PRIORITY(PRIORITY_CLASS_NORMAL, PRIORITY_LAYER_DRIVER, 0)

#define MHLRXCBUSCMDTASK_NAME                "HDMIRxCbusCmd Thread"
#define MHLRXCBUSCMDTASK_STACK_SIZE          2048
#define MHLRXCBUSCMDTASK_THREAD_PRIORITY     PRIORITY(PRIORITY_CLASS_NORMAL, PRIORITY_LAYER_DRIVER, 0)



HANDLE_T  _hHdmiRxEvent;
HANDLE_T _hHdmiRxThread1;

HANDLE_T _hMhlRxCbusLinkThread;
HANDLE_T _hMhlRxCbusCmdThread;

static BOOL _fgHdmiRxDrvInitiated = FALSE;

/* static UINT8 _u1RxSignalTiming= MODE_NOSIGNAL; */

#if 0
static HANDLE_T _hHdmiRxTimer;
static HANDLE_T _hMhlRxCbusTimer;
#else
struct timer_list _hMhlRxCbusTimer;
struct timer_list _hHdmiRxTimer;
#endif

static volatile INT32 _HdmiRxTmrValue[MAX_HDMI_RX_TMR_NUMBER];
UINT8 _u1RxSysState = RX_IDLE_STATE;/* RX_CHANGE_EDID_STATE; */
UINT8 _u1TxEdidReady = HDMI_PLUG_OUT;
extern unsigned long  g_IO_VBASE_VA;
extern unsigned int  g_VECTOR_HDMI;
extern unsigned int  g_VECTOR_CBUSINT;


HAL_TIME_T _rHdmiRxTmrStart[MAX_HDMI_RX_TMR_NUMBER];
HAL_TIME_T _rHdmiRxTmrStop[MAX_HDMI_RX_TMR_NUMBER];
HAL_TIME_T _rHdmiRxTmrDelta[MAX_HDMI_RX_TMR_NUMBER];


BOOL _fgHDMI_Rx_wait_finish = FALSE;

HDMI_REG_AUD_F hdmi_aud_fun = {
	vEnableHdmiRxAudTask,
	vAudInGetRxInAudInfo,
	GetHdmiRxAudioChannelStatus,
	GetHdmiRxAudioInfoFrame,
	u1GetHDMIRxACPType,
	FALSE,
};




/****************************************************************************
** Local function prototype
****************************************************************************/

static INT32 HDMI_RX_TmrInit(void);
static void HDMI_RX_TmrReset(void);
static INT32 vHDMIRxTaskInit(void);
static void vHDMIRxTaskMain(void *pvArg);
/* static INT32 vGetHdmiRxCmd(EV_GRP_EVENT_T *pr_u8HdmiRxEvent); */
static INT32 HDMI_RX_DRVInit(void);
static INT32 HDMI_RX_DRVUnInit(void);
static void vSetHdmiRxSrvTimeOut(INT32 i4_count);
static void vSendHdmiRxCmd(EV_GRP_EVENT_T u8Event);
/* static void HDMI_Rx_HwInit(void); */
static void vHdmiRxDetectService(void);






BOOL _fgDisableHdmiRx = TRUE;


UINT32 (*_u4pfHDMIRX_SetNfy)(void *pt_nfy_info);


void HDMI_Rx_Reg_u4HDMIRX_SetNfy(UINT32(*u4pfHDMIRX_SetNfy)(void *pt_nfy_info))
{
	_u4pfHDMIRX_SetNfy = u4pfHDMIRX_SetNfy;
}
EXPORT_SYMBOL(HDMI_Rx_Reg_u4HDMIRX_SetNfy);




void HAL_GetTime(HAL_TIME_T *pTime)
{
}


void HAL_Delay_us(UINT32 u4Micros)
{
	udelay(u4Micros);
}



/************************************************************************


some debug function, pls exclude from driver


*************************************************************************/

#if 0



static INT32 initWChForHDMI(void)
{
	/* PmxVerifyDrvInit(); */

	/* PmxVerifySetMode(0, 480, 480, 0, 9, 0);  //480p */
	UINT32 u4SrcFmt = 480;

	UINT32 u4Data = 0;
	UINT32 u4Width = 0;
	UINT32 u4Height = 0;
	UINT32 fgInterlace = 0;

	/* bApSclerMasterModeEn = 0; */

	/* get hdmi info */
	u4Data = vRxReadReg(0x22c4c);
	u4Width = (UINT16)((u4Data >> 16) & 0xfff);

	u4Data = vRxReadReg(0x22c50);
	u4Height = (UINT16)(u4Data & 0xfff);

	fgInterlace = HDMIinterlaced();

	if ((u4Height == 480) || (u4Height == 240))
		u4SrcFmt = 480;

	if ((u4Height == 720))
		u4SrcFmt = 720;

	if ((u4Height == 1080) || (u4Height == 540))
		u4SrcFmt = 1080;

	Printf("[HDMI] pmx u4SrcFmt set %d \r\n", u4SrcFmt);
	PmxVerifySetMode(0, u4SrcFmt, 480, 0, 9, 0);
	PmxVerifySetVdoSrcFmt(0, 2);

	return 0;
}


static INT32 conWchForHDMI(void)
{
	UCHAR ucVdoId;
	UCHAR ucOn;
	UINT32 u4PmxFmt, u4TvType, u4DataType, u4Interlace, u4MuxSelect, u4WrchIndex;
	static BOOL fgConfig = FALSE;

	UINT32 u4Data = 0;
	UINT32 u4Width = 0;
	UINT32 u4Height = 0;
	UINT32 fgInterlace = 0;

	ucVdoId = 0;
	ucOn = 1;
	u4PmxFmt = 480;
	u4TvType = 0;
	u4DataType = 420;
	u4Interlace = 0;
	u4MuxSelect = 0;
	u4WrchIndex = 0;

	/* get hdmi info */
	u4Data = vRxReadReg(0x22c4c);
	u4Width = (UINT16)((u4Data >> 16) & 0xfff);

	u4Data = vRxReadReg(0x22c50);
	u4Height = (UINT16)(u4Data & 0xfff);

	fgInterlace = HDMIinterlaced();

	if ((u4Height == 480) || (u4Height == 240))
		u4PmxFmt = 480;

	if ((u4Height == 720))
		u4PmxFmt = 720;

	if ((u4Height == 1080) || (u4Height == 540))
		u4PmxFmt = 1080;

	u4Interlace = (UINT32)fgInterlace;



	if (u4Interlace)
		u4Height *= 2;


	if (!fgConfig) {
		/* PmxVerifyWriteChnEntry(ucVdoId, ucOn, u4PmxFmt, u4TvType,
		u4DataType, u4Interlace, u4MuxSelect, u4WrchIndex, vHDMI_WCH_IRQ); */
		PmxVerifyWriteChnEntry_Hdmi(ucVdoId, ucOn, u4Width, u4Height,
		u4DataType, u4Interlace, u4MuxSelect, u4WrchIndex, NULL);
		fgConfig = TRUE;
		Printf("[HDMI]wch config first time \r\n");
	} else {
		/* PmxVerifyWriteChnEntry(ucVdoId, ucOn, u4PmxFmt, u4TvType,
		u4DataType, u4Interlace, u4MuxSelect, u4WrchIndex, NULL); */
		PmxVerifyWriteChnEntry_Hdmi(ucVdoId, ucOn, u4Width, u4Height,
		u4DataType, u4Interlace, u4MuxSelect, u4WrchIndex, NULL);
		Printf("[HDMI]wch config n time \r\n");
	}

	if ((u4PmxFmt == 720) || (u4PmxFmt == 1080)) {
		BASE_WRITE32(0x42134, 0x40484040);
		Printf("[wch config fifo...0x40. \r\n");
	}


	return 0;
}

#endif
/************************************************************************
end
*************************************************************************/


irqreturn_t mhlrx_cbus_irq(UINT32 u2Vector, void *dev_id)
{
	vMhlIntProcess();
	/* PDWNC_WRITE32(REG_RW_INTCLR, RW_INTEN_CBUS_SINK_INTCLR); */
	#ifdef CONFIG_ATC_PLATFORM_ac83xx
	ac83xx_mask_ack_bim_irq(g_VECTOR_CBUSINT);
	#elif defined CONFIG_ATC_PLATFORM_ac823x
	mt33xx_mask_ack_bim_irq(g_VECTOR_CBUSINT);
	#endif
	
	return IRQ_HANDLED;
}
void mhlrx_cbus_irq_enable(BOOL en)
{
	/*
	PDWNC_WRITE32(REG_RW_INTCLR, RW_INTEN_CBUS_SINK_INTCLR);
	if(en)
	PDWNC_WRITE32(REG_RW_INTEN, (PDWNC_READ32(REG_RW_INTEN ) | RW_INTEN_CBUS_SINK_EN));
	else
	PDWNC_WRITE32(REG_RW_INTEN, (PDWNC_READ32(REG_RW_INTEN ) & ~RW_INTEN_CBUS_SINK_EN));
	*/
}
/******************************************************************************
* Function      : UINT8 uMHL_GetConfigSet(UINT32 u4Addr)
* Description   : Get MHL cbus or RX config setting
* Parameter     : u4Addr, Only 20 or 21 can be used
* Return        : Byte value
******************************************************************************/
unsigned char sink_efuse_config = 0;

void vMHLRxCbusLinkTask(void *pvArg)
{
	/* get sink cbus efuse */
	vMHLCbusHwInit();


/* init cbus hd end */
	//VERIFY(x_reg_isr(VECTOR_CBUSINT/*VECTOR_INT_P_CBUS_SRCEN*/, mhlrx_cbus_irq, &mhl_cbus_isr) == OSR_OK);
	VERIFY(request_irq(g_VECTOR_CBUSINT, mhlrx_cbus_irq, 0, "ISR_MHLRxCbus", NULL) == OSR_OK);
	mhlrx_cbus_irq_enable(TRUE);/* xiaochuan, code in this fun has been annotated */

	while (_fgHdmiRxDrvInitiated == TRUE) {
		vSinkLinkState();
		msleep(10);
	}

	mhlrx_cbus_irq_enable(FALSE);
	/* VERIFY(x_reg_isr(VECTOR_CBUS_INT, NULL, &mhl_cbus_isr) == OSR_OK); */

	/*x_timer_delete(_hMhlRxCbusTimer); */
	/* del_timer(&_hMhlRxCbusTimer); */
	del_timer_sync(&_hMhlRxCbusTimer);

	sink_reset_attach();
	return;
}

void vMHLRxCbusCmdTask(void *pvArg)
{
	while (_fgHdmiRxDrvInitiated == TRUE) {
		vSinkCommandState();
		msleep(10);
	}
	return;
}


struct gpio_desc * hdmi_hpd_desc;

static INT32 vHDMIRxTaskInit(void)
{

	EV_GRP_EVENT_T u8HdmiRxEvent;
	static struct task_struct *hdmiRxTask = NULL;
	static struct task_struct *mhlRxCbusLinkTask = NULL;
	static struct task_struct *mhlRxCbusCmdTask = NULL;
	int i4Ret = 0;
	/* hdmi_aud_fun */

	u8HdmiRxEvent = 0;

	if (!hdmi_aud_fun.fgReg) {
		if (RegHDMIAudFunc(&hdmi_aud_fun)) {
			HDMI_LOG(HDMI_LOG_DEBUG, "reg to aud success\r\n");
			hdmi_aud_fun.fgReg = TRUE;
	    }
	}
	/* just for build err quzhi */

	/*  Register timer ISR */
	if (HDMI_RX_TmrInit() == SRV_FAIL)
		return SRV_FAIL;


	_u1RxSysState = RX_DETECT_STATE;
	_fgHdmiRxDrvInitiated = TRUE;

	BIT_SET(0x22c6c, 23);

	pinctrl_hdmi = devm_pinctrl_get(hdmi_dev);
	if(IS_ERR(pinctrl_hdmi)){
		HDMI_LOG(HDMI_LOG_INFO, "vHDMIRxTaskInit: devm_pinctrl_get fail \r\n");
	} else {
		HDMI_LOG(HDMI_LOG_INFO, "vHDMIRxTaskInit: devm_pinctrl_get succuess \r\n");
	}
	
	/*gpio_request(PIN_40_HDMI_HPD_RX, "HDMI_HPD");*/
	hdmi_hpd_desc = __gpiod_get(hdmi_dev, "hdmi_hpd", GPIOD_ASIS);
	if(IS_ERR(hdmi_hpd_desc))
		HDMI_LOG(HDMI_LOG_INFO, "vHDMIRxTaskInit:__gpiod_get fail\n");
	__gpiod_get(hdmi_dev, "mhl_sense", GPIOD_ASIS);
	/* HDMI_LOG(HDMI_LOG_INFO, "create main thread... \r\n"); */

	/*  Create HDMI task */
	
	hdmiRxTask = kthread_create(vHDMIRxTaskMain, NULL, HDMIRXTASK_NAME);
	if (IS_ERR(hdmiRxTask))
	{
		i4Ret = PTR_ERR(hdmiRxTask);
		HDMI_LOG(HDMI_LOG_ERROR, "vHDMIRxTaskInit:hdmiRxTask thread create fail \r\n");
		hdmiRxTask = NULL;
		return SRV_FAIL;
	}
	wake_up_process(hdmiRxTask);
#if 1
	mhlRxCbusLinkTask= kthread_create(vMHLRxCbusLinkTask, NULL, MHLRXCBUSLINKTASK_NAME);
	if (IS_ERR(mhlRxCbusLinkTask))
	{
		i4Ret = PTR_ERR(mhlRxCbusLinkTask);
		HDMI_LOG(HDMI_LOG_ERROR, "vHDMIRxTaskInit:mhlRxCbusLinkTask thread create fail \r\n");
		mhlRxCbusLinkTask = NULL;
		return SRV_FAIL;
	}
	wake_up_process(mhlRxCbusLinkTask);

	mhlRxCbusCmdTask= kthread_create(vMHLRxCbusCmdTask, NULL, MHLRXCBUSCMDTASK_NAME);
	if (IS_ERR(mhlRxCbusCmdTask))
	{
		i4Ret = PTR_ERR(mhlRxCbusCmdTask);
		HDMI_LOG(HDMI_LOG_ERROR, "vHDMIRxTaskInit:mhlRxCbusCmdTask thread create fail \r\n");
		mhlRxCbusCmdTask = NULL;
		return SRV_FAIL;
	}
	wake_up_process(mhlRxCbusCmdTask);
#endif

	vSetHdmiRxSrvTimeOut(HDMI_RX_TIMER_100MS);/* temply initial */
	return SRV_OK;
}

/******************************************************************************
* Function      : vHDMITaskMain
* Description   : main routine for HDMI Task
* Parameter     : None
* Return        : None
******************************************************************************/
void vChageRxSysState(UINT8 u1State)
{
	_u1RxSysState = u1State;
}



/*  MT3363_hdmi_emulation */

#define MT3363_HDMI_EMU  (0)
#ifdef MT3363_HDMI_EMU

UINT32 gHdmiReg_emu[0x300] = {
	0,
};


#endif




/******************************************************************************
* Function      : vHDMITaskMain
* Description   : main routine for HDMI Task
* Parameter     : None
* Return        : None
******************************************************************************/


static void vHDMIRxTaskMain(void *pvArg)
{

	VERIFY(pvArg == NULL);

	/* HDMI_LOG(HDMI_LOG_INFO, "Request ISR... \r\n"); */
	//VERIFY(x_reg_isr(VECTOR_HDMI, HdmiRxIrqHandler, &pfnOldIsr) == OSR_OK);
	VERIFY(request_irq(g_VECTOR_HDMI, HdmiRxIrqHandler, 0, "ISR_HDMIRxTask", NULL) == OSR_OK);
	vEnableHdmiRxTask(TRUE);

	/* HDMI_LOG(HDMI_LOG_INFO, "Main Loop... \r\n"); */
	while (_fgHdmiRxDrvInitiated == TRUE) {
		msleep(20);

		switch (_u1RxSysState) {
		case RX_CHANGE_EDID_STATE:
			break;


		case RX_DETECT_STATE:
			vHdmiRxDetectService();
			vSetHdmiRxSrvTimeOut(HDMI_RX_TIMER_20MS);

			break;

		case RX_IDLE_STATE:
			HDMIHPDHigh(0);
			break;


		default:
			break;

		}


	}

	/* release all OS resource */
	/* Disable Interrupt */
	/*  vHDMIIRQEnable(FALSE); */
	//VERIFY(x_reg_isr(0x22/*VECTOR_HDMIRXINT*/, NULL, &pfnOldIsr) == OSR_OK);
	free_irq(0x22/*VECTOR_HDMIRXINT*/, NULL);
	/* del message Queue */
	VERIFY(x_ev_group_delete(_hHdmiRxEvent) == OSR_OK);

	/* del timer */
	/* x_timer_delete(_hHdmiRxTimer); */

	/* Disable HDMI hardware */
	/*  vHdmiHwUnInit(); */
	_fgHDMI_Rx_wait_finish = TRUE;
	return;
}

/************************************************************************
  Function    : static INT32 vGetHdmiCmd(UINT32 *pr_u4Msg)
  Description : This function will get hdmi command from command queue
  Parameter   : None
  Return      : None
********************************************************************/
/*
static INT32 vGetHdmiRxCmd(EV_GRP_EVENT_T *pr_u8HdmiRxEvent)
{
	INT32 i4Ret;

	i4Ret = x_ev_group_wait_event(_hHdmiRxEvent,
					HDMI_RX_EG_WAKEUP_THREAD,
					pr_u8HdmiRxEvent,
					X_EV_OP_OR_CONSUME);
	ASSERT(i4Ret == OSR_OK);

	if (i4Ret == OSR_OK)
		return OSR_OK;
	return OSR_NO_MSG;

}
*/




void mhl_rx_cbus_timer(void)
{
	vSinkCbusTimer();

	_hMhlRxCbusTimer.expires = MHL_RX_CBUS_TMR_ISR_TICKS;
	_hMhlRxCbusTimer.function = (void *)mhl_rx_cbus_timer;
	_hMhlRxCbusTimer.data = 0;
	add_timer(&_hMhlRxCbusTimer);
}


/************************************************************************
  Function    : INT32 IDEUT_TmrInit()
  Description : This function will init hdmi timer Isr
  Parameter   : None
  Return      : None
********************************************************************/
/* void SinkCbus_HwInit(void); */

#if 0
#include "cbus_hw_reg.h"

void SinkCbus_HwInit(void)
{
	/*vIO32Write4B(0xA00000b4, 0xffffffff);
	vIO32Write4B(0xA00000D0, 0xffffffff);
	vIO32Write4B(0xA00229fc, 0x0000000f);
	vIO32Write1B(0xA0000070, 0x28);
	BIT_SET(0x368, 13); */


	*(volatile unsigned int *)0xA00000b4 = 0xffffffff;
	*(volatile unsigned int *)0xA00000D0 = 0xffffffff;
	*(volatile unsigned int *)0xA00229fc = 0x0000000f;
	*(volatile unsigned int *)0xA0000070 = 0x29;

	*(volatile unsigned int *)0xA0000368 |= (0x1 << 13);

	/* vIO32Write4B(0xA0022968, 0xc603); */
	*(volatile unsigned int *)0xA0022968 = 0x0001; /* 0xc603; discovery enable */
	/* vSinkWriteCbus(REG_CBUS_LINK_0B, 0x); */


	SinkWriteCbus(REG_CBUS_LINK_00, 0x08812e02);
	SinkWriteCbus(REG_CBUS_LINK_01, 0xb3024818);
	SinkWriteCbus(REG_CBUS_LINK_02, 0x87e50905);
	SinkWriteCbus(REG_CBUS_LINK_03, 0x04240789);
	SinkWriteCbus(REG_CBUS_LINK_04, 0x10830946);
	SinkWriteCbus(REG_CBUS_LINK_05, 0x1f6190a2);
	/* vSinkWriteCbus(REG_CBUS_LINK_06, 0x4e4495b2); */
	SinkWriteCbus(REG_CBUS_LINK_06, 0x4ec495b2);
	/* vIO32Write4B(PDWNC_CBUS_LINK_07, 0x00f138f9); //for hw mode */
	SinkWriteCbus(REG_CBUS_LINK_07, 0x003138f9); /* Look out */
	SinkWriteCbus(REG_CBUS_LINK_08, 0x00000000);
	SinkWriteCbus(REG_CBUS_LINK_09, 0x74771e47);
	SinkWriteCbus(REG_CBUS_LINK_0A, 0x2000000f);
	SinkWriteCbus(REG_CBUS_LINK_0B, 0xfffff02a);
	SinkWriteCbus(REG_CBUS_LINK_0C, 0xbce6081e);
	msleep(1);
	SinkWriteCbusMsk(REG_CBUS_LINK_03, 0, 0xf0000000);

	/* vIO32Write4B(0xA000007c, (u4IO32Read4B(0xA000007c)|(1<<14))); */

	*(volatile unsigned int *)0xA000007C |= (0x1 << 14);
	/* vCbus_HwInit(); */
	/* vIO32Write4B(0xA0022974, 0x7c7c); */
	*(volatile unsigned int *)0xA0022974 = 0x7c7c;
	/* cbus pwr */




}
#endif
static INT32 HDMI_RX_TmrInit(void)
{
	/* INT32 i4Ret; */

	HDMI_RX_TmrReset();

	/* RETAILMSG(1,(TEXT("CreateTimer:{_HDMI_RX_TmrIsr} \r\n"))); */
	HDMI_RX_TmrReset();

	init_timer(&_hHdmiRxTimer);
	_hHdmiRxTimer.expires = HDMI_RX_TMR_ISR_TICKS;
	_hHdmiRxTimer.function = (void *)_HDMI_RX_TmrIsr;
	_hHdmiRxTimer.data = 0;
	add_timer(&_hHdmiRxTimer);

	init_timer(&_hMhlRxCbusTimer);
	_hMhlRxCbusTimer.expires = MHL_RX_CBUS_TMR_ISR_TICKS;
	_hMhlRxCbusTimer.function = (void *)mhl_rx_cbus_timer;
	_hMhlRxCbusTimer.data = 0;
	add_timer(&_hMhlRxCbusTimer);

	return SRV_OK;
}


/************************************************************************
  Function    : static void HDMI_RX_TmrReset()
  Description : This function will reset Hdmi timer counter
  Parameter   : None
  Return      : None
********************************************************************/
static void HDMI_RX_TmrReset(void)
{
	INT32 i;

	/* clear used flag array and value. */
	for (i = 0; i < MAX_HDMI_RX_TMR_NUMBER; i++)
		_HdmiRxTmrValue[i] = HDMI_UNUSED_RX_TIMER;


}


/************************************************************************
  Function    : void _HDMI_RX_TmrIsr(HANDLE_T h, void *pvArg)
  Description : This function will reset Hdmi timer counter, 5ms
  Parameter   : None
  Return      : None
********************************************************************/

void _HDMI_RX_TmrIsr(void)
{
	INT32 i;
	UINT32 DeltaMs;


	/* decrease used timer */
	for (i = 0; i < MAX_HDMI_RX_TMR_NUMBER; i++) {
		if (_HdmiRxTmrValue[i] > 0) {


			Linux_HAL_GetTime((unsigned long *)&_rHdmiRxTmrStop[i]);
			DeltaMs = (_rHdmiRxTmrDelta[i].u4Seconds * 1000) + (_rHdmiRxTmrDelta[i].u4Micros / 1000);

			if (i == HDMI_RX_SRV_TMR_INX) {
				/* UTIL_Printf("[HDMI RX]_rHdmiRxTmrStop =%d\n", _rHdmiRxTmrStop[i]); */
				/* UTIL_Printf("[HDMI RX]_rHdmiRxTmrStart =%d\n", _rHdmiRxTmrStart[i]); */
				/* UTIL_Printf("[HDMI RX]_rHdmiRxTmrDelta =%d\n", _rHdmiRxTmrDelta[i]); */
				/* UTIL_Printf("[HDMI RX]DeltaMs =%d\n", DeltaMs); */
			}

			if (((INT32)DeltaMs >= _HdmiRxTmrValue[i]) && (DeltaMs < 40000)) {
				/* UTIL_Printf("[HDMI RX]DeltaMs = %d\n", DeltaMs); */
				_HdmiRxTmrValue[i] = 0;

			}

			if ((i == HDMI_RX_SRV_TMR_INX) && (_HdmiRxTmrValue[HDMI_RX_SRV_TMR_INX] == 0)) {
				vSendHdmiRxCmd(HDMI_RX_SERVICE_CMD);/* temply */
			}

		}
	}

}


/************************************************************************
  Function    : void vSendHdmiRxCmd(UINT32 u4Cmd)
  Description : This function will send command to HDMI queue
  Parameter   : None
  Return      : None
********************************************************************/

static void vSendHdmiRxCmd(EV_GRP_EVENT_T u8Event)
{
	INT32 i4Ret;


	i4Ret = x_ev_group_set_event(_hHdmiRxEvent,
				     u8Event,
				     X_EV_OP_OR);

	if (i4Ret != OSR_OK)
		HDMI_LOG(HDMI_LOG_INFO, "[HDMI RX] !!!HDMI set event error=%d\n", (int)i4Ret);

	/* UTIL_Printf("[HDMI RX]!!!HDMI set event error=%d\n", i4Ret); */

	ASSERT(i4Ret == OSR_OK);

}


/************************************************************************
  Function    : static INT32 HDMI_RX_DRVInit(void)
  Description : This function will init hdmi driver
  Parameter   : None
  Return      : None
********************************************************************/
static INT32 HDMI_RX_DRVInit(void)
{
	if (!_fgHdmiRxDrvInitiated) {
		if (vHDMIRxTaskInit() == SRV_FAIL)
			return SRV_FAIL;
	}

	return SRV_OK;
}


/************************************************************************
  Function    : void HDMI_DRVUnInit();
  Description : This function will disable hdmi driver
  Parameter   : None
  Return      : None
********************************************************************/
static INT32 HDMI_RX_DRVUnInit(void)
{

	if (_fgHdmiRxDrvInitiated) {

		vSendHdmiRxCmd(HDMI_DISABLE_HDMI_RX_TASK_CMD);

		while (!_fgHDMI_Rx_wait_finish)
			msleep(5);

		msleep(50);
		HDMI_LOG(HDMI_LOG_DEBUG, "[HDMI RX] HDMI_RX_DRVUnInit  Done");
	}
	return SRV_OK;
}

/************************************************************************
  Function    : INT32 IDEUT_TmrInit()
  Description : This function will set HDCP control timer
  Parameter   : i4_count: ? ms
  Return      : None
********************************************************************/
static void vSetHdmiRxSrvTimeOut(INT32 i4_count)
{

	_HdmiRxTmrValue[HDMI_RX_SRV_TMR_INX] = i4_count;

	Linux_HAL_GetTime((unsigned long *)&_rHdmiRxTmrStart[HDMI_RX_SRV_TMR_INX]);


	/* UTIL_Printf("ST =%d, _rHdmiRxTmrStart =%d\n",_HdmiRxTmrValue[HDMI_RX_SRV_TMR_INX],
	_rHdmiRxTmrStart[HDMI_RX_SRV_TMR_INX]); */
}


void Set640x480PEnable(BYTE bType)
{

}

/*
static void vNotifyTxResChg(BYTE bResMode)
{
	int i8Res = -1;
	BYTE bVFrontPorch = HDMI_HalGetVFrontPorch();

	Set640x480PEnable(FALSE);

#if 0
	switch(bResMode)
	{
	case MODE_HDMI_640_480P:
		if(fgIsHdmiRxDebug(HDMI_RX_DEBUG_HOT_PLUG))
		UTIL_Printf("[HDMI RX]Rx detect 640x480P");
		if(bVFrontPorch == 0x12)
		{
		Set640x480PEnable(2);
		}
		else
		{
		Set640x480PEnable(1);
		}
		break;
	case MODE_480P:
		if(fgIsHdmiRxDebug(HDMI_RX_DEBUG_HOT_PLUG))
		UTIL_Printf("[HDMI RX]Rx detect 720x480P");
		break;
	case MODE_720p_60:
		if(fgIsHdmiRxDebug(HDMI_RX_DEBUG_HOT_PLUG))
		UTIL_Printf("[HDMI RX]Rx detect 1280x720P60HZ");
		i8Res = RES_720P60HZ;
		break;
	case MODE_1080i:
		if(fgIsHdmiRxDebug(HDMI_RX_DEBUG_HOT_PLUG))
		UTIL_Printf("[HDMI RX]Rx detect 1920x1080(I)@60Hz");
		i8Res = RES_1080I60HZ;
		break;
	case MODE_525I:
		if(fgIsHdmiRxDebug(HDMI_RX_DEBUG_HOT_PLUG))
		UTIL_Printf("[HDMI RX]Rx detect 720x480(I)@60Hz");
		i8Res = RES_480I;
		break;
	case MODE_525I_OVERSAMPLE:
		if(fgIsHdmiRxDebug(HDMI_RX_DEBUG_HOT_PLUG))
		UTIL_Printf("[HDMI RX]Rx detect 1440x480(I)@60Hz");
		i8Res = RES_480I_2880;
		break;
	case MODE_480P_OVERSAMPLE:
		if(fgIsHdmiRxDebug(HDMI_RX_DEBUG_HOT_PLUG))
		UTIL_Printf("[HDMI RX]Rx detect 1440x480(P)@60Hz");
		i8Res = RES_480P_1440;
		break;
	case MODE_1080p_60:
		if(fgIsHdmiRxDebug(HDMI_RX_DEBUG_HOT_PLUG))
		UTIL_Printf("[HDMI RX]Rx detect 1920x1080@60Hz");
		i8Res = RES_1080P60HZ;
		break;
	case MODE_576P:
		if(fgIsHdmiRxDebug(HDMI_RX_DEBUG_HOT_PLUG))
		UTIL_Printf("[HDMI RX]Rx detect 720x576@50Hz");
		i8Res = RES_576P;
		break;
	case MODE_720p_50:
		if(fgIsHdmiRxDebug(HDMI_RX_DEBUG_HOT_PLUG))
		UTIL_Printf("[HDMI RX]Rx detect 1280x720@50Hz");
		i8Res = RES_720P50HZ;
		break;
	case MODE_1080i_50:
		if(fgIsHdmiRxDebug(HDMI_RX_DEBUG_HOT_PLUG))
		UTIL_Printf("[HDMI RX]Rx detect 1920x1080(I)@50Hz");
		i8Res = RES_1080I50HZ;
		break;
	case MODE_625I:
		if(fgIsHdmiRxDebug(HDMI_RX_DEBUG_HOT_PLUG))
		UTIL_Printf("[HDMI RX]Rx detect 720x576(I)@50Hz");
		i8Res = RES_576I;
		break;
	case MODE_625I_OVERSAMPLE:
		if(fgIsHdmiRxDebug(HDMI_RX_DEBUG_HOT_PLUG))
		UTIL_Printf("[HDMI RX]Rx detect 1440x576(I)@50Hz");
		i8Res = RES_576I_2880;
      break;
	case MODE_576P_OVERSAMPLE:
		if(fgIsHdmiRxDebug(HDMI_RX_DEBUG_HOT_PLUG))
		UTIL_Printf("[HDMI RX]Rx detect 1440x576(P)@50Hz");
		i8Res = RES_576P_1440;
		break;
	case MODE_1080p_50:
		if(fgIsHdmiRxDebug(HDMI_RX_DEBUG_HOT_PLUG))
		UTIL_Printf("[HDMI RX]Rx detect 1920x1080P@50Hz");
		i8Res = RES_1080P50HZ;
		break;
	case MODE_1080p_24:
		if(fgIsHdmiRxDebug(HDMI_RX_DEBUG_HOT_PLUG))
		UTIL_Printf("[HDMI RX]Rx detect 1920x1080@24Hz");
		i8Res = RES_1080P24HZ;
		break;
	case MODE_1080p_25:
		if(fgIsHdmiRxDebug(HDMI_RX_DEBUG_HOT_PLUG))
		UTIL_Printf("[HDMI RX]Rx detect 1920x1080@25Hz");
		i8Res = RES_1080P25HZ;
		break;
	case MODE_1080p_30:
		if(fgIsHdmiRxDebug(HDMI_RX_DEBUG_HOT_PLUG))
		UTIL_Printf("[HDMI RX]Rx detect 1920x1080@30Hz);
		i8Res = RES_1080P30HZ;
		break;
	default:
		if(fgIsHdmiRxDebug(HDMI_RX_DEBUG_HOT_PLUG))
		UTIL_Printf("[HDMI RX]Rx detect bResMode =%d\n", bResMode);
		break;
    }
#endif
  if(i8Res != -1)
  {

  }

}*/

/************************************************************************
  Function    : void vHdmiRxDetectService(void)
  Description : This function is a service for CAT6023
  Parameter   : i4_count: ? ms
  Return      : None
********************************************************************/
static void vHdmiRxDetectService(void)
{

	/* UINT8 bResMode; */

	if (_fgDisableHdmiRx)
		return;

	HDMIHpdLoop();



#if 1
	HDMIPowerDetextExt();
	vDviModeDetectExt();

#endif

	vHdmiRxAudLoop();

#if 0

	if (!CheckRxDetectDone()) {
		DviModeDetect();

		bResMode = GetRxCapturedTiming();

		if ((bResMode > MODE_NOSIGNAL) && (bResMode < MODE_MAX)) {
			if (_u1RxSignalTiming != bResMode) {
				_u1RxSignalTiming = bResMode;
				/* vNotifyTxResChg(bResMode); */
				HDMI_LOG(HDMI_LOG_DEBUG, "Notify mode change! \r\n"); */
			}
		}
	} else
		DviChkModeChange();

#endif

}

void vHDMIRxPowerOnOff(BOOL fgEnable)
{

}




/*************************************************************************************
Global Function
*************************************************************************************/

INT32 HDMI_RX_Init(void)
{
	UINT32 u4DebugLevel = 0;

	/* Mask debug log */
	u4DebugLevel = HDMI_LOG_ERROR |
		       HDMI_LOG_WARN |
		       HDMI_LOG_DEBUG |
		       HDMI_LOG_INFO |
		       HDMI_LOG_USER1 |
		       HDMI_LOG_USER2;
	vHdmiSetDebugMask(u4DebugLevel);


	/*HDMI_LOG(HDMI_LOG_INFO, "debug level is 0x%x \r\n", u4DebugLevel); */

	/* HDMI_Rx_HwInit(); */
	HDMIInterRxInit();
	#ifdef CONFIG_ATC_PLATFORM_ac823x
	HDMI_HwInit();//add
	#endif

	if (HDMI_RX_DRVInit() == SRV_FAIL)
		return 0;/* E_HDMI_TASK_INIT_FAIL; */

	return 1;/* HDMI_OK;//S_HDMI_OK; */

}



UINT32 HDMI_RX_Config(MHL_DRV_CONFIG_T rMhlConfig)
{
	HDMI_DrvConfigEvent(rMhlConfig);

	return 1;
}

UINT32 HDMI_RX_Start(void)
{
	HDMI_DrvSetStart(TRUE);
	return 1;
}


UINT32 HDMI_RX_Stop(void)
{
	HDMI_DrvSetStart(FALSE);
	return 1;
}




MHL_DEVICE_TYPE_T HDMI_RX_GetDeviceType(void)
{
	if (is_sink_attached) {
		HDMI_LOG(HDMI_LOG_INFO, "Device is MHL \r\n");
		return DEVICE_MHL;
	}
	HDMI_LOG(HDMI_LOG_INFO, "Device is HDMI \r\n");
	return DEVICE_HDMI;
}

/*
0, signal none
1, signal on but not stable
2, signal stable but hdcp not pass
3, signal stable and hdcp auth
*/
UINT32 HDMI_RX_GetSignalStatus(void)
{
	UINT32 signal_status = 0;
	UINT8 ckdt = HDMI_HalGetCKDT();
	UINT8 scdt = HDMI_HalGetSCDT();

	//if (ckdt & scdt)
	//	signal_status = 1;

	/* HDMI_LOG(HDMI_LOG_INFO, "***************signal status is %d ckdt %d scdt %d\r\n",
	signal_status, ckdt, scdt); */

	return signal_status;
}

UINT32 HDMI_RX_GetClockStable(BOOL *pStable)
{
	if (NULL == pStable)
		return 0;
	*pStable = HdmiIsPclkStable();
	return 1;
}
UINT32 HDMI_RX_GetHsyncStable(BOOL *pStable)
{
	if (NULL == pStable)
		return 0;
	*pStable = HdmiIsHVStable();
	return 1;
}



VOID HDMI_PrintInfo(void)
{
	/* HDMI_LOG(HDMI_LOG_DEBUG, "widht : %d \r\n", g_rVideoInfo.u4Width); */
	/* HDMI_LOG(HDMI_LOG_DEBUG, "height: %d \r\n", g_rVideoInfo.u4Height); */
	/* HDMI_LOG(HDMI_LOG_DEBUG, "interlace : %d \r\n", g_rVideoInfo.bInterlaced); */

}

UINT32 HDMI_RX_GetVideoInfo(MHL_VIDEO_INFO_T *pVideoInfo)
{
	if (NULL == pVideoInfo)
		return 0;
	HDMI_DrvGetVideoInfo(pVideoInfo);
	return 1;
}

UINT32 HDMI_RX_SendRcpKey(UINT32 u4Key)
{
	unsigned char u1Key = 0;

	u1Key = (unsigned char)u4Key;
	/* RETAILMSG(1,(TEXT("Send Rcp Key1(0x%x) \r\n"), u4Key)); */
	sink_get_keycode_from_ir(u4Key);

	/* RETAILMSG(1,(TEXT("Send Rcp Key2(0x%x) \r\n"), u4Key)); */
	return 1;



}



/* INT32 i4HdmiRxUnInit(void) */
INT32 i4HdmiRx_Uninit(UINT32 u4Case)
{

	HDMI_LOG(HDMI_LOG_DEBUG, "[HDMI RX] HDMI_RX_DRVUnInit  Start");

	if (fgIsHdmiRxBoardExist() == TRUE) {

		if (HDMI_RX_DRVUnInit() == SRV_FAIL)
			return 0;/* E_HDMI_TASK_INIT_FAIL; */
	}


	return 1;/* HDMI_OK;//S_HDMI_OK; */
}


#if 1
void vAudInGetRxInAudInfo(HDMI_RX_IN_AUDIO_INFO_T *pv_get_info)
{
	HDMI_LOG(HDMI_LOG_DEBUG, "get rx aud info \r\n");
	vGetHdmiRxAudioParameter(pv_get_info);
}
#endif

void vSetAvdHdmiRxInMode(BOOL fgRxInOn)
{
}


void vEnableHdmiRxTask(UINT8 u1Enable)
{
	if (u1Enable) {
		if (_fgDisableHdmiRx == TRUE) { /* temply add */
			vSetAvdHdmiRxInMode(TRUE);
			Delay5MS(2);
			_fgDisableHdmiRx = FALSE;
		}
	} else {
		if (_fgDisableHdmiRx == FALSE) { /* temply add */
			vSetAvdHdmiRxInMode(FALSE);
			_fgDisableHdmiRx = TRUE;
			Set640x480PEnable(FALSE);
			vSetHdmiRxSrvTimeOut(HDMI_RX_TIMER_5MS);
			vChageRxSysState(RX_DETECT_STATE);
			/* vSetHdmiDisableCcirIn(); */
		}

	}

}
EXPORT_SYMBOL(vEnableHdmiRxTask);





/**************************
  For Repeater HDCP
**************************/
void vTxSetRxReceiverMode(void)
{

	RxHDCPSetReceiver();

}


void vTxSetRxRepeaterMode(void)
{
}

void vTxSetKsvReady(BYTE bTxDownStream, UINT16 u2TxBStatus, BYTE *prbTxBksv, BYTE *prbTxKsvlist, BOOL fgTxVMatch)
{
}

BOOL fgIs640x480PEnable(void)
{
	return  0;
}


void vSetHdmiRxSpdifOn(BOOL fgOn)
{

	/* SetMUTE(~(B_TRI_SPDIF),(fgOn)? 0:(1<<O_TRI_SPDIF)); //kenny mark */
	/* You need to add HDMI RX audio spdif out On/off function */

}
EXPORT_SYMBOL(vSetHdmiRxSpdifOn);

/**********************************************************
void vSetHdmiRxI2sOn(UINT8 u1OnBit)
u1OnBit: see x_audin.h HDMI_I2S_CH_SEL_T
***********************************************************/
void vSetHdmiRxI2sOn(UINT8 u1OnBit)
{

	/* SetMUTE(~(B_TRI_I2S), (~u1OnBit)&0x0f); */
	/* You need to add HDMI RX audio I2s out On/off function */

}
EXPORT_SYMBOL(vSetHdmiRxI2sOn);

/**********************************************************
int GetHdmiRxAudioInfoFrame(HDMI_RX_Audio_InfoFrame *pAudioInfoFrame)
u1OnBit: see x_audin.h HDMI_I2S_CH_SEL_T
********************************************************** */
#if 0
int GetHdmiRxAudioInfoFrame(/*HDMI_RX_Audio_InfoFrame*/void *pAudioInfoFrame)
{
	/*  can not find STRUCT:pAudioInfoFrame, */
	/* need to overwrite. */

	/*
	Audio_InfoFrame AudioInfoFrame;
	int i, result;

	if(pAudioInfoFrame==NULL)
	return 1;

	result= ExportAudioInfoFrame(&AudioInfoFrame);



	pAudioInfoFrame->info.Type= AudioInfoFrame.info.Type;
	pAudioInfoFrame->info.Ver= AudioInfoFrame.info.Ver;
	pAudioInfoFrame->info.Len= AudioInfoFrame.info.Len;
	pAudioInfoFrame->info.AudioChannelCount= AudioInfoFrame.info.AudioChannelCount;
	pAudioInfoFrame->info.RSVD1= AudioInfoFrame.info.RSVD1;
	pAudioInfoFrame->info.AudioCodingType= AudioInfoFrame.info.AudioCodingType;
	pAudioInfoFrame->info.SampleSize= AudioInfoFrame.info.SampleSize;
	pAudioInfoFrame->info.SampleFreq= AudioInfoFrame.info.SampleFreq;
	pAudioInfoFrame->info.Rsvd2= AudioInfoFrame.info.Rsvd2;
	pAudioInfoFrame->info.FmtCoding= AudioInfoFrame.info.FmtCoding;
	pAudioInfoFrame->info.SpeakerPlacement= AudioInfoFrame.info.SpeakerPlacement;
	pAudioInfoFrame->info.Rsvd3= AudioInfoFrame.info.Rsvd3;
	pAudioInfoFrame->info.LevelShiftValue= AudioInfoFrame.info.LevelShiftValue;
	pAudioInfoFrame->info.DM_INH= AudioInfoFrame.info.DM_INH;

	for(i=0; i<3; i++)
	{
	  pAudioInfoFrame->pktbyte.AUD_HB[i]= AudioInfoFrame.pktbyte.AUD_HB[i];
	}
	for(i=0; i< 10; i++)
	{
	  pAudioInfoFrame->pktbyte.AUD_DB[i]= AudioInfoFrame.pktbyte.AUD_DB[i];
	}

	return result;
	*/

	return 0;

}
EXPORT_SYMBOL(GetHdmiRxAudioInfoFrame);
#endif
#if 0
int GetHdmiRxAudioChannelStatus(/*HDMI_RX_AUDIO_CHSTS*/void *pHdmiRxChStat)
{
	/* can not find STRUCT */
	/* need to overwrite */

	/*
	RX_REG_AUDIO_CHSTS HdmiRxChStat;

	ExportAudioChannelStatus(&HdmiRxChStat);

	pHdmiRxChStat->ISLPCM = HdmiRxChStat.ISLPCM ;
	pHdmiRxChStat->CopyRight = HdmiRxChStat.CopyRight;
	pHdmiRxChStat->AdditionFormatInfo = HdmiRxChStat.AdditionFormatInfo;
	pHdmiRxChStat->ChannelStatusMode = HdmiRxChStat.ChannelStatusMode;
	pHdmiRxChStat->CategoryCode = HdmiRxChStat.CategoryCode;
	pHdmiRxChStat->SourceNumber = HdmiRxChStat.SourceNumber;
	pHdmiRxChStat->ChannelNumber = HdmiRxChStat.ChannelNumber;
	pHdmiRxChStat->SamplingFreq = HdmiRxChStat.SamplingFreq;
	pHdmiRxChStat->ClockAccuary = HdmiRxChStat.ClockAccuary;
	pHdmiRxChStat->WorldLen = HdmiRxChStat.WorldLen;
	pHdmiRxChStat->OriginalSamplingFreq = HdmiRxChStat.OriginalSamplingFreq;
	*/

	return 0;
}
EXPORT_SYMBOL(GetHdmiRxAudioChannelStatus);
#endif

/************************************************************************
  Function    : BOOL fgIsHdmiRxBoardExist(void)
  Description : This function will init hdmi task
  Parameter   : None
  Return      : None
********************************************************************/
BOOL fgIsHdmiRxBoardExist(void)
{

	return TRUE;
}


BOOL GetSourceVidInfo(MHL_VIDEO_INFO_T *pmhlvidInfo)
{
	if (HDMI_HalIsVResStable()) {
		pmhlvidInfo->bInterlaced = HDMIinterlaced();
		pmhlvidInfo->u4Height    = HDMIResoHeight();
		pmhlvidInfo->u4Width     =  HDMIResoWidth();
		/* HDMI_LOG(HDMI_LOG_WARN, "Height:%d, Width:%d, Interlace:%d \r\n",
		pmhlvidInfo->u4Height, pmhlvidInfo->u4Width, pmhlvidInfo->bInterlaced); */
		return TRUE;
	}
	HDMI_LOG(HDMI_LOG_WARN, "HV Not Stable, meanless \r\n");
	return FALSE;
}



UINT8 hdcp_key_to_sram[292] = {0};
UINT8 hdcp_key_from_mt[320] = {0};

VOID HDMI_RX_LoadHDCPKeyToSRAM(HDCP_KEY_ST *hk)
{
	unsigned int  size = hk->size;
	/* char *key = hk->buf; */

	memcpy(hdcp_key_from_mt, hk->buf, sizeof(hdcp_key_from_mt));

/* 0x0 , BKSV[5], 0xff, 0xff */
	hdcp_key_to_sram[0] = 0x0;
	memcpy(&hdcp_key_to_sram[1], &hdcp_key_from_mt[0], 5);
	hdcp_key_to_sram[6] = 0xFF;
	hdcp_key_to_sram[7] = 0xFF;

/* 0x0, key[280], 0x0, 0x0, 0x0 */
	hdcp_key_to_sram[8] = 0x0;
	memcpy(&hdcp_key_to_sram[9], &hdcp_key_from_mt[8], 280);
	hdcp_key_to_sram[289] = 0x0;
	hdcp_key_to_sram[290] = 0x0;
	hdcp_key_to_sram[291] = 0x0;


	HDMI_LOG(HDMI_LOG_DEBUG, "HDCP Key size : %d \r\n", size);

	HDMI_HalLoadHdcp2Sram(hdcp_key_to_sram);
#if 0

	for (i = 0; i < size;) {
		HDMI_LOG(HDMI_LOG_DEBUG, "0x%08x: 0x%02x  0x%02x  0x%02x  0x%02x \r\n",
			i, key[i], key[i + 1], key[i + 2], key[i + 3]);
		i = i + 4;
	}

#endif

}

VOID HDMI_RX_LoadHDCPKeyToSRAM2(HDCP_KEY_ST *hk)
{
	unsigned int  size = hk->size;
	/* char *key = hk->buf; */

	if(copy_from_user(hdcp_key_from_mt, hk->buf, sizeof(hdcp_key_from_mt))) {
        HDMI_LOG(HDMI_LOG_INFO, "HHDMI_RX_LoadHDCPKeyToSRAM2  copy_from_user fail\r\n");
		return;
	}


/* 0x0 , BKSV[5], 0xff, 0xff */
	hdcp_key_to_sram[0] = 0x0;
	memcpy(&hdcp_key_to_sram[1], &hdcp_key_from_mt[0], 5);
	hdcp_key_to_sram[6] = 0xFF;
	hdcp_key_to_sram[7] = 0xFF;

/* 0x0, key[280], 0x0, 0x0, 0x0 */
	hdcp_key_to_sram[8] = 0x0;
	memcpy(&hdcp_key_to_sram[9], &hdcp_key_from_mt[8], 280);
	hdcp_key_to_sram[289] = 0x0;
	hdcp_key_to_sram[290] = 0x0;
	hdcp_key_to_sram[291] = 0x0;


	HDMI_LOG(HDMI_LOG_DEBUG, "HDCP Key size : %d \r\n", size);

	HDMI_HalLoadHdcp2Sram(hdcp_key_to_sram);
#if 0

	for (i = 0; i < size;) {
		HDMI_LOG(HDMI_LOG_DEBUG, "0x%08x: 0x%02x  0x%02x  0x%02x  0x%02x \r\n",
			i, key[i], key[i + 1], key[i + 2], key[i + 3]);
		i = i + 4;
	}

#endif

}

