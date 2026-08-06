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


#ifndef _DRV_AUDIN_H_
#define _DRV_AUDIN_H_

#include "x_typedef.h"
#include "x_os.h"
#include "x_audin.h"

#include "drv_config.h"
#include "chip_ver.h"
#include "drv_av_d.h"

// *********************************************************************
// Constant definitions
// *********************************************************************

#define AUDIN_OK	     (s32)(0)
#define AUDIN_FAIL     (s32)(-1)

   #if 1//KF lin permission (!CONFIG_DRV_LINUX_DATA_CONSISTENCY)
   #define AUDIN_SUPPORT_USB
   #endif
// *********************************************************************
// Variable define definitions
// *********************************************************************



// *********************************************************************
// Export API
// *********************************************************************

//extern INT32 MW_Audin_Init(void);
//extern INT32 MW_Audin_Unint(void);

//extern void vAudin_Init(const AUDIN_SET_T *prAudinSetInfo);
//extern void vAudin_UnInit(AUDIN_TYPE u1AudinType);

//extern void vAudinSwitchFunc(AUDIO_IN_TYPE_T u1Input, AUDIN_DIGITAL_DETECT u1Detect);
#if (CONFIG_DRV_HDMI_RX)
extern void vAudinExSPDIFSwitch(HDMI_SPDIF_IN_TYPE_T u1Input);
#endif
extern void vUSBSwitchFunc(u32 hInput);
extern void vAudinOnOffCtrl(bool fgOnOffCmd);
extern void vUSBAUDOnOffCtrl(bool fgOnOffCmd);
extern void SPDIF_InEnable(bool fgEnable);

extern void vScomGetAudInInfo(AUDIN_INFO_T *pv_get_info);
extern void vScomGetUSBAudChannelNum(AUDIN_INFO_T *pv_get_info);
extern void vScomGetUSBAudCodec(AUDIN_INFO_T *pv_get_info);
#if CONFIG_DRV_HDMI_RX
extern void vScomGetHDMIRXAUDINFO(HDMI_RX_IN_AUDIO_INFO_T *pv_get_info);
#endif
extern void vScomGetUSBSampleRate(AUDIN_INFO_T *pv_get_info);

#if (CONFIG_DRV_MIC_SUPPORT)
extern void vSetMICClk(AUDIO_SAMPLING_T u1SmpRate);
#endif

#if (!UNIFORM_DRV_CALLBACK)
extern void vScomSetAudInNfyFunc(x_audin_nfy_fct  prNfyInfo);
#else
extern void vScomSetAudInNfyFunc(DRV_CB_REG_INFO_T  *arCBInfo);
#endif

#endif /* _DRV_AUDIN_H_ */
