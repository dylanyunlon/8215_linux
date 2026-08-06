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

#include "tvd_drv_if.h"
#include "tvd_data_struct.h"
#include "tvd_internal_data_struct.h"
#include "tvd_hal_if.h"
#include "tvd_log.h"
#ifdef __ARM2__
#include "83xx_irqs_vector.h"
#else
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <linux/mutex.h>
#include <linux/sched.h>
#include <linux/kthread.h>
#include <linux/err.h>
#endif



u32 debug = TVD_LOG_LVL_INFO;
static TVD_DRV_CAMERA_PREVIEW_INIT_INFO_T tvd_init_info_backup;
static TVD_DRV_CAMERA_PREVIEW_CFG_T tvd_config_info_backup;

/*this parameter reserves the channel which we have used*/
static TVD_CHANNEL_T used_channel = TVD_CH_NONE;
static bool tvd_robbed;
static TVD_TYPE_STATE_TRANSITION play_src_type = NONE_IDLE;
static bool tvd_irq_register;
static bool bLock_inited = false;


#ifdef __ARM2__
#define Lock()
#define mutex_init(x)
#define Unlock()
PTVD_IRQ_PROCE tvd_isr_proc;
#else
static struct mutex tvd_lock;
#define Lock() mutex_lock(&tvd_lock)
#define Unlock() mutex_unlock(&tvd_lock)
static struct task_struct *tvd_task = NULL;
#endif


static bool tvd_irq_proc_register(PTVD_IRQ_PROCE tvd_irq_function)
{
#if defined(__ARM2__)
	tvd_isr_proc = tvd_irq_function;
	v_enable_bim_irq(VECTOR_VDOIN);
#else
	s32 ret = -1;

	if (false == tvd_irq_register) {
		ret = request_irq(tvd_irq, tvd_irq_function, 0, "tvd", NULL);

		if (0 != ret) {
			TVD_LOG(TVD_LOG_LVL_ERR, "TVD Register Interrupt failed\n");
			return false;
		}

		tvd_irq_register = true;
	}

#endif
	return true;
}

static void tvd_irq_proc_unregister(void)
{
#if defined(__ARM2__)
	tvd_isr_proc = (PTVD_IRQ_PROCE)NULL;
#else

	if (true == tvd_irq_register) {
		free_irq(tvd_irq, NULL);
		tvd_irq_register = false;
	}

#endif
}


#ifdef __ARM2__
static void tvd_interrupt_process(s32 irq)
{
	_tvd_interrupt_process(irq);
}
#else
int create_event(void)
{
	return _create_event();
}

int signal_process(void *arg)
{
	return _signal_process(arg);
}

void set_exit_event(void)
{
	return _set_exit_event();
}

static irqreturn_t tvd_interrupt_process(s32 irq, void *dev_id)
{
	return  _tvd_interrupt_process(irq, dev_id);
}
#endif

s8 tvd_register_notify(tvd_notify notify_fun, void *arg)
{
	return _tvd_register_notify(notify_fun, arg);
}
EXPORT_SYMBOL(tvd_register_notify);

void tvd_unregister_notify(void)
{
	_tvd_unregister_notify();
}
EXPORT_SYMBOL(tvd_unregister_notify);


static u32 cvbs_initiate(PTVD_DRV_CAMERA_PREVIEW_INIT_INFO_T pInputPara)
{
	u32 ret = ERROR_NONE;
	int func_ret = 0;

	PTVD_DRV_CAMERA_PREVIEW_INIT_INFO_T pInitInfo = pInputPara;

	TVD_LOG(TVD_LOG_LVL_INFO, "enter\n");

	if ((pInitInfo->u4CHACvbsInxP > CVBSIN_NONE) || (pInitInfo->u4CHBCvbsInxP > CVBSIN_NONE) ||
	    ((CVBSIN_NONE == pInitInfo->u4CHACvbsInxP) && (CVBSIN_NONE == pInitInfo->u4CHBCvbsInxP))) {
		TVD_LOG(TVD_LOG_LVL_ERR, "leave,CHA 0x%x or CHB_NUM 0x%x invalid,so failed\n",
			pInitInfo->u4CHACvbsInxP, pInitInfo->u4CHBCvbsInxP);
		return ERROR_INVALID_PARA;
	}

	if (((pInitInfo->u4CHAOutDest > TVD_CFG_OUT_BYPASS) || (pInitInfo->u4CHBOutDest > TVD_CFG_OUT_BYPASS)) ||
	    ((TVD_CFG_OUT_NONE == pInitInfo->u4CHAOutDest) && (TVD_CFG_OUT_NONE == pInitInfo->u4CHBOutDest))) {
		TVD_LOG(TVD_LOG_LVL_ERR, "leave,CHA_fmt 0x%x or CHB_fmt 0x%x invalid,so failed\n",
			pInitInfo->u4CHAOutDest, pInitInfo->u4CHBOutDest);
		return ERROR_INVALID_PARA;
	}

#ifndef __ARM2__
	/*create event*/
	func_ret = create_event();
	if(func_ret) {
		TVD_LOG(TVD_LOG_LVL_ERR, "create event failed\n");
		return ERROR_INVALID_PARA;
	}

	tvd_task = kthread_create(signal_process, NULL, "tvd_signal_process_thread" );
	if(IS_ERR(tvd_task)){
		TVD_LOG(TVD_LOG_LVL_ERR, "create tvd signal process thread failed\n");
		tvd_task = NULL;
		return ERROR_INVALID_PARA;
	}
	wake_up_process(tvd_task);
#endif

	tvd_irq_proc_register(&tvd_interrupt_process);
	tvd_hal_open(pInitInfo->u4UVSwap);

	if ((CVBSIN_NONE != pInitInfo->u4CHACvbsInxP) && (TVD_CFG_OUT_NONE != pInitInfo->u4CHAOutDest)) {
		used_channel = TVD_CHA;

		if (TVD_CFG_OUT_BYPASS == pInitInfo->u4CHAOutDest) {
			tvd_channel_port_config(TVD_CHA, pInitInfo->u4CHACvbsInxP, TVD_CHA_BYPASS, TVD_ANALOG_CFG_ALL);
		} else {
			tvd_channel_port_config(TVD_CHA, pInitInfo->u4CHACvbsInxP, TVD_CH0_BYPASS, TVD_ANALOG_CFG_ALL);
		}
	}

	if ((CVBSIN_NONE != pInitInfo->u4CHBCvbsInxP) && (TVD_CFG_OUT_NONE != pInitInfo->u4CHBOutDest)) {
		used_channel |= TVD_CHB;

		if (TVD_CFG_OUT_BYPASS == pInitInfo->u4CHBOutDest) {
			tvd_channel_port_config(TVD_CHB, pInitInfo->u4CHBCvbsInxP, TVD_CHB_BYPASS, TVD_ANALOG_CFG_ALL);
		} else {
			tvd_channel_port_config(TVD_CHB, pInitInfo->u4CHBCvbsInxP, TVD_CH0_BYPASS, TVD_ANALOG_CFG_ALL);
		}
	}

	TVD_LOG(TVD_LOG_LVL_INFO, "leave\n");
	return ret;
}


static u32 cvbs_configure(PTVD_DRV_CAMERA_PREVIEW_CFG_T pInputPara)
{
	u32 ret = ERROR_NONE;
	PTVD_DRV_CAMERA_PREVIEW_CFG_T pCameraPreviewCfg = pInputPara;
	TVD_CORE_PREVIEW_CFG_T       tvdHalCamPreviewInfo = {0};

	TVD_LOG(TVD_LOG_LVL_INFO, "enter\n");
	tvdHalCamPreviewInfo.u1HueLvl         = pCameraPreviewCfg->u1HueLvl;            /* hue gain level */
	tvdHalCamPreviewInfo.u1SaturationLvl = pCameraPreviewCfg->u1SaturationLvl;       /* saturation level */
	tvdHalCamPreviewInfo.u1BrightnessLvl = pCameraPreviewCfg->u1BrightnessLvl;       /* brightness gain level */

#if SUPPORT_CALIBRATE_BRIGHTNESS

	tvdHalCamPreviewInfo.u1Mask     = pCameraPreviewCfg->u1Mask;
	tvdHalCamPreviewInfo.u1YGain    = pCameraPreviewCfg->u1YGain;
	tvdHalCamPreviewInfo.u1YOffset    = pCameraPreviewCfg->u1YOffset;
	tvdHalCamPreviewInfo.u1UCosGain = pCameraPreviewCfg->u1UCosGain;
	tvdHalCamPreviewInfo.u1VCosGain = pCameraPreviewCfg->u1VCosGain;
	tvdHalCamPreviewInfo.u1USinGain = pCameraPreviewCfg->u1USinGain;
	tvdHalCamPreviewInfo.u1VSinGain = pCameraPreviewCfg->u1VSinGain;
	tvdHalCamPreviewInfo.u1UOffset    = pCameraPreviewCfg->u1UOffset;
	tvdHalCamPreviewInfo.u1VOffset    = pCameraPreviewCfg->u1VOffset;

	if (tvdHalCamPreviewInfo.u1Mask != 0) {
		/*Tvd_Core_Config(&tvdHalCamPreviewInfo);*/
	}

#endif
	TVD_LOG(TVD_LOG_LVL_INFO, "leave\n");
	return ret;
}

static u32 cvbs_start(void)
{
	u32 ret = ERROR_NONE;

	TVD_LOG(TVD_LOG_LVL_INFO, "enter\n");
	switch (used_channel) {
	case TVD_CHA:
		tvd_channel_on_off(TVD_CHA, true);
		break;

	case TVD_CHB:
		tvd_channel_on_off(TVD_CHB, true);
		break;

	case TVD_CHA_CHB:
		tvd_channel_on_off(TVD_CHA, true);
		tvd_channel_on_off(TVD_CHB, true);
		break;

	default:
		TVD_LOG(TVD_LOG_LVL_WARN, "the channel is invalid\n");
		break;
	}

	if (gfgNeedTurnClamp) {
		tvd_channel_port_config(TVD_CH_NONE, CVBSIN_NONE, TVD_CH0_BYPASS, TVD_ANALOG_CFG_CLAMP);
	}

	TVD_LOG(TVD_LOG_LVL_INFO, "leave\n");
	return ret;
}

static u32 cvbs_stop(TVD_CHANNEL_T stop_channel)
{
	u32 ret = ERROR_NONE;
	/*the parameter indicate the channel which we really can stop*/
	TVD_CHANNEL_T channel = used_channel & stop_channel;

	TVD_LOG(TVD_LOG_LVL_INFO, "enter\n");

	tvd_hal_notify_close_wch();
#ifndef __ARM2__
	set_exit_event();
	if (tvd_task) {
		kthread_stop(tvd_task);
		tvd_task = NULL;
	}
#endif

	switch (channel) {
	case TVD_CHA:
		tvd_channel_port_config(TVD_CHA, CVBSIN_NONE, TVD_CH0_BYPASS, TVD_ANALOG_CFG_ALL);
		tvd_channel_on_off(TVD_CHA, false);
		break;

	case TVD_CHB:
		tvd_channel_port_config(TVD_CHB, CVBSIN_NONE, TVD_CH0_BYPASS, TVD_ANALOG_CFG_ALL);
		tvd_channel_on_off(TVD_CHB, false);
		break;

	case TVD_CHA_CHB:
		tvd_channel_port_config(TVD_CHA, CVBSIN_NONE, TVD_CH0_BYPASS, TVD_ANALOG_CFG_ALL);
		tvd_channel_port_config(TVD_CHB, CVBSIN_NONE, TVD_CH0_BYPASS, TVD_ANALOG_CFG_ALL);
		/*Tvd_Ana_IO_PowerOn(false);*/
		break;

	default:
		ret = ERROR_INVALID_PARA;
		break;
	}

	if (ERROR_INVALID_PARA == ret) {
		TVD_LOG(TVD_LOG_LVL_WARN, "stop channel is not correct,and channel is %x,so fail\n", stop_channel);
		return ret;
	}

	tvd_hal_close();
	tvd_irq_proc_unregister();
	used_channel &= ~stop_channel;
	TVD_LOG(TVD_LOG_LVL_INFO, "leave\n");

	return ret;
}

u32 tvd_power_mgr(u32 u4PowerStateValue)
{
	switch (u4PowerStateValue)
	{
		case 0:
			TVD_LOG(TVD_LOG_LVL_INFO, "case 0 pw on!\n");
			break;

		case 4:
			TVD_LOG(TVD_LOG_LVL_INFO, "case 4 pw down!\n");
			tvd_clock_on_off(false);
			break;

		default:
			break;
	}
	
	return 0;
}

u32 tvd_internal_ioctl(u32 cmd, void *in_buf, void *out_buf)
{
	u32 ret = ERROR_NONE;

	TVD_LOG(TVD_LOG_LVL_DBG, "enter and cmd is 0x%x\n", cmd);
	if (!bLock_inited) {
		mutex_init(&tvd_lock);
		bLock_inited = true;
	}

	switch (cmd) {
	case TVD_CONTROL_CODE_INIT: {
		Lock();
		PTVD_DRV_CAMERA_PREVIEW_INIT_INFO_T tvd_init_info = (PTVD_DRV_CAMERA_PREVIEW_INIT_INFO_T)in_buf;
#ifndef __ARM2__
		/*this parameter indicate which source we are playing and which source we want to play*/
		TVD_TYPE_STATE_TRANSITION curr_will_status;
#endif

		TVD_LOG(TVD_LOG_LVL_INFO, "start executing INIT cmd\n");

		if (NULL == tvd_init_info) {
			TVD_LOG(TVD_LOG_LVL_ERR, "leave, the input arg invalid,so execute INIT cmd failed\n");
			Unlock();
			return ERROR_INVALID_PARA;
		}

#ifndef __ARM2__
		curr_will_status = play_src_type | tvd_init_info->source_type;
		TVD_LOG(TVD_LOG_LVL_INFO, "start init cmd with current_will status(0x%x)\n", curr_will_status);

		switch (curr_will_status) {
		case NONE_IDLE:
		case BACKCAR_NONE:
		case AVIN_NONE:
			TVD_LOG(TVD_LOG_LVL_ERR, "the app id is invalid\n");
			Unlock();
			return ERROR_INVALID_PARA;

		case BACKCAR_AVIN:
			memcpy(&tvd_init_info_backup, tvd_init_info, sizeof(TVD_DRV_CAMERA_PREVIEW_INIT_INFO_T));
			TVD_LOG(TVD_LOG_LVL_WARN, "just reserve avin init information when backcar is playing\n");
			tvd_robbed = true;
			Unlock();
			return ret;

		case NONE_BACKCAR:
			ret = cvbs_initiate(tvd_init_info);
			break;

		case NONE_AVIN:
			memcpy(&tvd_init_info_backup, tvd_init_info, sizeof(TVD_DRV_CAMERA_PREVIEW_INIT_INFO_T));
			ret = cvbs_initiate(tvd_init_info);
			break;

		case BACKCAR_BACKCAR:
			cvbs_stop(used_channel);
			break;

		case AVIN_AVIN:
			cvbs_stop(used_channel);
			memcpy(&tvd_init_info_backup, tvd_init_info, sizeof(TVD_DRV_CAMERA_PREVIEW_INIT_INFO_T));
			break;

		case AVIN_BACKCAR:
			cvbs_stop(used_channel);
			ret = cvbs_initiate(tvd_init_info);
			tvd_robbed = true;
			break;

		default:
			break;
		}

		play_src_type = (u8)(tvd_init_info->source_type << 4);
		TVD_LOG(TVD_LOG_LVL_INFO, "end init cmd with current_will status(0x%x)\n", play_src_type);
#else
		ret = cvbs_initiate(tvd_init_info);
#endif
		Unlock();
		TVD_LOG(TVD_LOG_LVL_INFO, "finish executing INIT cmd\n");
		break;
	}

	case TVD_CONTROL_CODE_CONFIG: {
		Lock();
		PTVD_DRV_CAMERA_PREVIEW_CFG_T tvd_config_info = (PTVD_DRV_CAMERA_PREVIEW_CFG_T) in_buf;
#ifndef __ARM2__
		/*this parameter indicate which source we are playing and which source we want to play*/
		TVD_TYPE_STATE_TRANSITION curr_will_status;
#endif

		TVD_LOG(TVD_LOG_LVL_INFO, "start execute CONFIG cmd\n");

		if (NULL == tvd_config_info) {
			TVD_LOG(TVD_LOG_LVL_ERR, "the input arg invalid,so execute CONFIG cmd failed\n");
			Unlock();
			return ERROR_INVALID_PARA;
		}

#ifndef __ARM2__
		curr_will_status = play_src_type | tvd_config_info->source_type;

		switch (curr_will_status) {
		case NONE_NONE:
		case BACKCAR_NONE:
		case AVIN_NONE:
			TVD_LOG(TVD_LOG_LVL_ERR, "the source id is invalid\n");
			Unlock();
			return ERROR_INVALID_PARA;

		case AVIN_BACKCAR:
		case NONE_BACKCAR:
			TVD_LOG(TVD_LOG_LVL_WARN, "discard the config infor from BC when BC is stopped\n");
			Unlock();
			return ret;

		case NONE_AVIN:
		case BACKCAR_AVIN:
			memcpy(&tvd_config_info_backup, tvd_config_info, sizeof(TVD_DRV_CAMERA_PREVIEW_CFG_T));
			Unlock();
			return ret;

		case BACKCAR_BACKCAR:
			cvbs_configure(tvd_config_info);
			break;

		case AVIN_AVIN:
			memcpy(&tvd_init_info_backup, tvd_config_info, sizeof(TVD_DRV_CAMERA_PREVIEW_CFG_T));
			cvbs_configure(tvd_config_info);
			break;

		default:
			break;
		}

#else
		cvbs_configure(tvd_config_info);
#endif
		Unlock();
		TVD_LOG(TVD_LOG_LVL_INFO, "finish executing CONFIG cmd\n");
		break;
	}

	case TVD_CONTROL_CODE_START: {
		TVD_LOG(TVD_LOG_LVL_INFO, "start to execute START cmd\n");
		Lock();

		/*app invoke the start command when CVBS was Robbed,return success and do nothing*/
		if (true == tvd_robbed) {
			Unlock();
			return ret;
		}

		ret = cvbs_start();
		Unlock();
		TVD_LOG(TVD_LOG_LVL_INFO, "finish executing START cmd\n");
		break;
	}

	case TVD_CONTROL_CODE_STOP: {
		Lock();
		PTVD_DRV_STOP_PARA_T tvd_stop_info = (PTVD_DRV_STOP_PARA_T) in_buf;
#ifndef __ARM2__
		/*this parameter indicate which source we are playing and which source we want to play*/
		TVD_TYPE_STATE_TRANSITION curr_will_status;
#endif

		TVD_LOG(TVD_LOG_LVL_INFO, "start to execute STOP cmd\n");

		if (NULL == tvd_stop_info) {
			TVD_LOG(TVD_LOG_LVL_ERR, "the input arg invalid,so execute STOP cmd failed\n");
			Unlock();
			return ERROR_INVALID_PARA;
		}

#ifndef __ARM2__
		curr_will_status = play_src_type | tvd_stop_info->source_type;
		TVD_LOG(TVD_LOG_LVL_INFO, "start stop cmd with current_will status(0x%x)\n", curr_will_status);

		switch (curr_will_status) {
		case NONE_BACKCAR:
		case NONE_AVIN:
			TVD_LOG(TVD_LOG_LVL_WARN, "nothing is play,so discard the stop information\n");
			Unlock();
			return ret;

		case BACKCAR_AVIN:
			if (true == tvd_robbed) {
				tvd_robbed = false;
			}
			break;

		case NONE_NONE:
		case BACKCAR_NONE:
		case AVIN_NONE:
		case AVIN_BACKCAR:
			TVD_LOG(TVD_LOG_LVL_WARN, "the source ipp is invalid\n");
			Unlock();
			return ret;

		case BACKCAR_BACKCAR:
			cvbs_stop(tvd_stop_info->stop_channel);
			play_src_type = NONE_IDLE;

			if (true == tvd_robbed) {
				cvbs_initiate(&tvd_init_info_backup);
				cvbs_configure(&tvd_config_info_backup);
				play_src_type = AVIN_IDLE;
				tvd_robbed = false;
			}
			Unlock();
			return ret;

		case AVIN_AVIN:
			cvbs_stop(tvd_stop_info->stop_channel);
			play_src_type = NONE_IDLE;
			break;

		default:
			break;
		}
		TVD_LOG(TVD_LOG_LVL_INFO, "end stop cmd with current_will status(0x%x)\n", play_src_type);
#else
		cvbs_stop(tvd_stop_info->stop_channel);
#endif
		Unlock();
		TVD_LOG(TVD_LOG_LVL_INFO, "finish executing STOP cmd\n");
		break;
	}

	case GET_SIGNAL_STATE: {
		TVD_LOG(TVD_LOG_LVL_DBG, "start to execute GET_SIGNAL_STATE cmd\n");

		if (NULL == out_buf) {
			TVD_LOG(TVD_LOG_LVL_ERR, "input arg invalid,so execute GET_SIGNAL_STATE cmd failed\n");
			return ERROR_INVALID_PARA;
		}

		*(TVD_SIG_STATE_T *)out_buf = tvd_hal_get_signal_status();
		TVD_LOG(TVD_LOG_LVL_DBG, "finish executing GET_SIGNAL_STATE cmd\n");
	}
	break;

	case GET_DI_FLAG: {
		TVD_LOG(TVD_LOG_LVL_DBG, "start to execute GET_DI_FLAG cmd\n");

		if (out_buf) {
			*(bool *)out_buf = tvd_get_di_flag();
		} else {
			TVD_LOG(TVD_LOG_LVL_ERR, "leave,input arg invalid,so execute GET_DI_FLAG cmd failed\n");
			return ERROR_INVALID_PARA;
		}

		TVD_LOG(TVD_LOG_LVL_DBG, "finish executing GET_DI_FLAG cmd\n");
		break;
	}
	#ifdef INIT_TVD_BEFORE_ARM2_START
	case OPTIMIZATION_TVD:
		TVD_LOG(TVD_LOG_LVL_INFO, "start to execute OPTIMIZATION_TVD cmd\n");
		tvd_irq_proc_register(&tvd_interrupt_process);
		TVD_LOG(TVD_LOG_LVL_INFO, "finish executing OPTIMIZATION_TVD cmd\n");
		break;
	#endif
	default:
		TVD_LOG(TVD_LOG_LVL_INFO, "the cmd 0x%x is not supported in current\n", cmd);
		break;
	}

	if (ret != ERROR_NONE) {
		TVD_LOG(TVD_LOG_LVL_ERR, "execute 0x%x cmd failed!\n", cmd);
	}

	TVD_LOG(TVD_LOG_LVL_DBG, "leave\n");
	return ret;
}
EXPORT_SYMBOL(tvd_internal_ioctl);














