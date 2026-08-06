
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

#ifndef _DRV_VCP_H
#define _DRV_VCP_H

#include <linux/types.h>
#include <linux/miscdevice.h>
#include <linux/ioctl.h>

struct vcp_device{
	struct miscdevice cdev;   /* Char device structure */
	uint32_t dwVcpInst;
	#ifdef CONFIG_PM
	struct device * dev;
	#endif
};


//parameters for pattern generate
typedef struct {
	int32_t i4Channel;
	int32_t i4En;
	int32_t i4Step;
	int32_t i4Prec;
}vcp_pg_paras;


//parameters for GPP setting
typedef struct {
	int32_t i4Mode;
	int32_t i4DatY;
	int32_t i4DatU;
	int32_t i4DatV;
	int32_t i4EnU;
	int32_t i4EnV;
}vcp_gpp_paras;


//parameters for black/white level extend
typedef struct {
	int32_t i4Mode;
	int32_t i4Slope;
	int32_t i4Anchor;
}vcp_bwex_paras;


//parameters for contrast/brightness/saturation
typedef struct {
    int32_t i4Contr;
    int32_t i4Brit;
    int32_t i4Satr;
    int32_t srctype;
}vcp_cbs_paras;


//parameters for secondary color enhance(SCE)
typedef struct {
	int32_t i4Mode;
	int32_t i4Luma;
	int32_t i4Hue;
	int32_t i4Sat;
	int32_t i4Dat;
}vcp_sce_paras;


//parameters for color transient improve(CTI)
typedef struct {
	int32_t i4Mode;
	int32_t i4Value0;
	int32_t i4Value1;
}vcp_cti_paras;


//parameters for UV2CbCr
typedef struct {
	int32_t i4En;
	uint32_t u4GainU;
	uint32_t u4GainV;
	uint32_t u4Sign;
}vcp_uv2c_paras;


//parameters for color suppress
typedef struct {
	int32_t i4Mode;
	int32_t i4En;
	uint32_t u4GainU;
	uint32_t u4Offset;
	uint32_t u4SubDiv;
	uint32_t u4Spc;
	uint32_t u4Spcc;
}vcp_csp_paras;

//parameters for yuv gain
typedef struct {
    int32_t i4YGain;
    int32_t i4UGain;
    int32_t i4VGain;
    int32_t srctype;
}vcp_yuv_paras;

typedef struct {
    int32_t i4hue;
    int32_t srctype;
}vcp_hue_paras;


typedef enum
{	
	VCP_BYPASS = 0,
	VCP_PG,
	VCP_GPP,
	VCP_BWEX,
	VCP_SCE,
	VCP_CTI,
	VCP_UV2C,
	VCP_CSP,
	VCP_SET_CBS,//
	VCP_GET_CBS,//
	VCP_SET_HUE,//
	VCP_GET_HUE,//
	VCP_SET_YUV,//
	VCP_GET_YUV,//
	VCP_ON,//
	VCP_OFF//
}VCP_IOCTL_FUNCTIONS;

#define VCP_IOC_BASE 0x15
#define VCP_IOC_BYPASS			_IOR (VCP_IOC_BASE, VCP_BYPASS,  int*)
#define VCP_IOC_PG				_IOR (VCP_IOC_BASE, VCP_PG,  vcp_pg_paras *)
#define VCP_IOC_GPP				_IOR (VCP_IOC_BASE, VCP_GPP,  vcp_gpp_paras *)
#define VCP_IOC_BWEX			_IOR (VCP_IOC_BASE, VCP_BWEX,  vcp_bwex_paras *)
#define VCP_IOC_SCE				_IOR (VCP_IOC_BASE, VCP_SCE,  vcp_sce_paras *)
#define VCP_IOC_CTI				_IOR (VCP_IOC_BASE, VCP_CTI,  vcp_cti_paras *)
#define VCP_IOC_UV2C			_IOR (VCP_IOC_BASE, VCP_UV2C,  vcp_uv2c_paras *)
#define VCP_IOC_CSP				_IOR (VCP_IOC_BASE, VCP_CSP,  vcp_csp_paras *)
//new feature add
#define VCP_IOC_SET_CBS			_IOR (VCP_IOC_BASE, VCP_SET_CBS,  vcp_cbs_paras *)
#define VCP_IOC_GET_CBS			_IOR (VCP_IOC_BASE, VCP_GET_CBS,  vcp_cbs_paras *)
#define VCP_IOC_SET_HUE			_IOR (VCP_IOC_BASE, VCP_SET_HUE,  vcp_hue_paras *)
#define VCP_IOC_GET_HUE			_IOR (VCP_IOC_BASE, VCP_GET_HUE,  int *)
#define VCP_IOC_SET_YUV			_IOR (VCP_IOC_BASE, VCP_SET_YUV,  vcp_yuv_paras *)
#define VCP_IOC_GET_YUV			_IOR (VCP_IOC_BASE, VCP_GET_YUV,  vcp_yuv_paras *)
#define VCP_IOC_ON			    _IO (VCP_IOC_BASE, VCP_ON)
#define VCP_IOC_OFF			    _IO (VCP_IOC_BASE, VCP_OFF)









#endif
