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

#ifndef _DRV_VDOIN_H_
#define _DRV_VDOIN_H_


#include "x_typedef.h"
/* #include "x_vid_vdoin.h" */
#include "x_drv_cb.h"

#include "dram_model.h"

#include "x_hal_ic.h"
#include "x_hal_1176.h"
#include "drv_av_d.h"
#include "drv_common.h"
#include <media/atc/x_vid_dec.h>

#define VDOIN_DRV_OK                     ((INT32) 0)
#define VDOIN_DRV_INV_GET_INFO       ((INT32) - 258)
#define VDOIN_DRV_INV_SET_INFO       ((INT32) - 259)


#define  VIN_GET_INFO_TYPE_RES        (0x1<<1)
#define  VIN_GET_INFO_TYPE_ASR        (0x1<<2)
#define  VIN_GET_INFO_TYPE_DATA      (0x1<<3)
#define  VIN_GET_INFO_TYPE_SIGNAL    (0x1<<4)
#define  VIN_GET_INFO_TYPE_ALL         \
	(VIN_GET_INFO_TYPE_RES | VIN_GET_INFO_TYPE_ASR | VIN_GET_INFO_TYPE_DATA|VIN_GET_INFO_TYPE_SIGNAL)

typedef enum _VIN_DRAM_ADDRESS_MODE_T {
	VIN_BLOCK = 0,
	VIN_LINEAR = 1

} VIN_DRAM_ADDRESS_MODE_T;

#if CONFIG_DRV_CUSTOM_JSN
typedef struct {
	UINT8 u1Calc; /* 0: No video displayed and no value calculated.
					1: Video displayed and RGB value is calculated. */
	UINT8 u1R;
	UINT8 u1G;
	UINT8 u1B;

	UINT32  u4FlagExt;
} VIN_FRAME_RGB_INFO_T;
#endif

typedef struct {
	UINT32 u4Flag; /* combination value of VID_PLA_CALC_RGB_PARAM_FLAG_T. */
	/* If any one of the following parameter is ZERO, it means don't calculate average rgb. */
	UINT32 u4FrameSampleRate;
	UINT32 u4PixelSampleRateH; /* horizontal pixel */
	UINT32 u4PixelSampleRateV; /* vertical line */
} VIN_PLA_CALC_RGB_PARAM_T;

typedef enum _VIN_DRAM_SWAP_MODE_T {
	VIN_SWAP_MODE_0 = 0,
	VIN_SWAP_MODE_1 = 1,
	VIN_SWAP_MODE_2 = 2,
	VIN_SWAP_MODE_3 = 3

} VIN_DRAM_SWAP_MODE_T;

typedef enum _VIN_DRAM_FORMAT_T {
	VIN_420 = 0,
	VIN_422 = 1

} VIN_DRAM_FORMAT_T;

typedef struct _VDOIN_CONFIG_WRITE_DRAM_TYPE_T {
	VIN_DRAM_ADDRESS_MODE_T eVdoInAddrMode;
	VIN_DRAM_SWAP_MODE_T eVdoInSwapMode;
	VIN_DRAM_FORMAT_T eVdoInDramFmt;

} VDOIN_CONFIG_WRITE_DRAM_TYPE_T;

typedef struct _VDOIN_CONFIG_INFO_T {
	/* Input */
	PMX_RESOLUTION_MODE_T          eVdoInResMode;  /* Resolution */
	CCIR_IN_OUT_MODE_T             eVdoInMode;     /* 601 or 656 */
	CCIR_IN_OUT_FORMAT_T           eVdoInPinType;  /* 444 or 422, Pin_mux */
	BOOL                           fgVgaIsCeaType;
	/* Output */
	VDOIN_CONFIG_WRITE_DRAM_TYPE_T rVdoInWDramType;
	SOURCE_ASPECT_RATIO_T          eVdoInAR;
	BOOL fgNTSC60;

} VDOIN_CONFIG_INFO_T;

#if UNIFORM_DRV_CALLBACK
typedef struct _VDOIN_INFO_T {
	VID_VDOIN_CB_DATA rVdoInCbData;
	DRV_CB_REG_INFO_T rVdoInCbInfo;

	void (*pfVdoIn_Switch2VideoIn)(void);
	void (*pfVdoIn_Switch2CcirIn)(void);
	void (*pfVdoIn_HDMIRx2Switch)(void);
	void (*pfVdoIn_Tvd2Switch)(void);
} VDOIN_INFO_T;
#else
typedef void (*x_vid_vdoin_nfy_fct)(void);
typedef struct _VDOIN_INFO_T {
	void *pvVdoInNfyTag;
	x_vid_vdoin_nfy_fct pfVdoInNfyFct;

	void (*pfVdoIn_Switch2VideoIn)(void);
	void (*pfVdoIn_Switch2CcirIn)(void);
	void (*pfVdoIn_HDMIRx2Switch)(void);
	void (*pfVdoIn_Tvd2Switch)(void);
} VDOIN_INFO_T;
#endif

typedef enum _VIN_DEVICE_ID_T {
	VIN_DEVICE_UNKNOW = 0,
	VIN_EXTERNAL_TVD  = 1,
	VIN_HDMI_1        = 2,
	VIN_HDMI_2        = 3,
	VIN_DEVICE_MAX,
} VIN_DEVICE_ID_T;

typedef struct _INPUT_DEVICE_INFO_T {
	BOOL                   fgIsTimingOk;
	VIN_DEVICE_ID_T        eDeviceId;
	PMX_RESOLUTION_MODE_T  eInputRes;     /* use EDID_VIDEO_RES_T, there are many resolution */
	CCIR_IN_OUT_FORMAT_T   ePinType;
	CCIR_IN_OUT_MODE_T     eInputMode;
	SOURCE_ASPECT_RATIO_T  eAspectRatio;
	BOOL                   fgIsJpeg;
	BOOL                   fgIsCinema;
	BOOL                   fgVgaIsCeaType;
	BOOL                   fgNTSC60;/* different from 59.94 or 23.976 */
} INPUT_DEVICE_INFO_T;

extern void VdoIn_Init(void);

extern INT32 i4VdoIn_Uninit(void);

extern void VdoIn_Drv_Init(void);

extern void VDec_Termint(void);

extern INT32 i4VdoIn_Simp_Connect(
	UINT16 u2CompType,
					/* < [IN] component type of VDec Mw_If */
	UINT16 u2CompId,
					/* < [IN] component Id of VDec Mw_If */
	UINT16 u2DmxCompType,
					/* < [IN] component type of Demuxer */
	UINT16 u2DmxCompId
					/* < [IN] component Id of Demuxer */
);

extern INT32 i4VdoIn_Simp_Disconnect(
	UINT16 u2CompType,
					/* < [IN] component type of VDec Mw_If */
	UINT16 u2CompId,
					/* < [IN] component Id of VDec Mw_If */
	UINT16 u2DmxCompType,
					/* < [IN] component type of Demuxer */
	UINT16 u2DmxCompId
					/* < [IN] component Id of Demuxer */
);

#if CONFIG_DRV_CUSTOM_JSN
#if CONFIG_DRV_HDMI_RX
void _u4VinGetRGBParam(VIN_FRAME_RGB_INFO_T *prParam);
void _u4VinSetCalcRGBParam(VIN_PLA_CALC_RGB_PARAM_T *prParam);
#endif
#endif

#endif /* _DRV_VDOIN_H_ */
