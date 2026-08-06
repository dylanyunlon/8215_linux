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
#if ! defined(DRV_COMP_ID_H)
#define DRV_COMP_ID_H

#include "x_typedef.h"

//#include "drv_mpv.h"
//#include "drv_vdp.h"
//#include "drv_vdec.h"
//#include "drv_if_pmx.h"
//#include "drv_tve.h"
//#include "drv_aud.h"
//#include "drv_fbm.h"

/* Tuner Component ID */
typedef enum
{
	TNR_1_COMP_TER,
	TNR_1_COMP_CAB,
	/*
	TNR_2_COMP_TER,
	TNR_2_COMP_CAB,
	*/
	TNR_COMP_NS,
} TNR_COMP_ID_T;


/* DCC Component ID */
#define DCC_COMP_NUM                        (4)  /**/
#define DCC_COMP_ID_START                   (0)  /**/

/* DMX Component ID */
// PCR: 0 - 3
#define DMX_PCR_COMP_NUM                    (4)  /**/
#define DMX_PCR_COMP_ID_START               (0)  /**/

// PES PACKET: 4 - 13
#define DMX_PES_COMP_NUM                    (12) /**/
#define DMX_PES_COMP_ID_START               (DMX_PCR_COMP_ID_START + DMX_PCR_COMP_NUM)  /* 4 */
#define DMX_PES_COMP_ID_END                 (DMX_PES_COMP_ID_START + DMX_PES_COMP_NUM - 1) /**/

// Section: 14 - 83
#define DMX_SECTION_MEM_COMP_NUM            (4) /**/
#define DMX_SECTION_MEM_COMP_ID_START       (DMX_PES_COMP_ID_END + 1)  /* 14 */
#define DMX_SECTION_MEM_COMP_ID_END         (DMX_SECTION_MEM_COMP_ID_START + DMX_SECTION_MEM_COMP_NUM - 1)

// PES MEMORY: 84 - 95
#define DMX_PES_MEM_COMP_NUM                12
#define DMX_PES_MEM_COMP_ID_START           (DMX_SECTION_MEM_COMP_ID_END + 1)
#define DMX_PES_MEM_COPY_ID_END             (DMX_PES_MEM_COMP_ID_START + DMX_PES_MEM_COMP_NUM - 1)

#define DMX_SECTION_FILTER_COMP_NUM         (64) /**/
#define DMX_SECTION_FILTER_COMP_ID_START    (0)  /**/
#define DMX_SECTION_FILTER_COMP_ID_END      (DMX_SECTION_FILTER_COMP_ID_START + DMX_SECTION_FILTER_COMP_NUM - 1)

/* DMX PCR ID mapping to H/W ID function */
extern UCHAR MW_PcrCompIdtoStcId(UINT16 u2CompId);

/* MPV Component ID */
#define MPV_COMP_SD_NS  4

typedef enum
{
	MPV_COMP_1,
	MPV_COMP_2,
  #ifndef VDEC_OPTIMIZE_RESOURCE
	MPV_COMP_3,
	MPV_COMP_4,
	MPV_COMP_5,
	MPV_COMP_6,
	MPV_COMP_7,
	MPV_COMP_8,
	MPV_COMP_9,
	MPV_COMP_10,
  #endif
	MPV_COMP_NS,

} MPV_COMP_ID_T;

extern UCHAR MW_MpvCompIdtoEsId(UINT16 u2CompId);
extern UCHAR MW_MpvCompIdtoVldId(UINT16 u2CompId, UINT32 u4ChannelId);

/* VDP Component ID */
#define VDP_COMP_NS	VDP_NS
#define VDP_COMP_1	VDP_1
#define VDP_COMP_2	VDP_2
#define VDP_COMP_3	VDP_3		// in the future, virtual vdp with osd
#define VDP_COMP_4	VDP_4		// in the future, virtual vdp with osd
#define VDP_COMP_5	VDP_5		// in the future, virtual vdp with osd
#define VDP_COMP_6  VDP_6
#define VDP_COMP_7  VDP_7
#define VDP_COMP_8  VDP_8


/* PBBUF Component ID */
typedef enum _PBBUF_COMP_ID_T
{
  PBBUF_COMP_1,
  PBBUF_COMP_2,
  PBBUF_COMP_3,
  PBBUF_COMP_4,
  PBBUF_COMP_NS
} PBBUF_COMP_ID_T;

/* SYNCCTRL Component ID */
typedef enum _SYNCCTRL_COMP_ID_T
{
  SYNCCTRL_COMP_1,
//  SYNCCTRL_COMP_2,
//  SYNCCTRL_COMP_3,
  SYNCCTRL_COMP_NS
} SYNCCTRL_COMP_ID_T;

typedef enum _AUDIN_COMP_ID_T
{
  AUDIN_COMP_1,
  AUDIN_COMP_NS
} AUDIN_COMP_ID_T;


typedef enum _PARTY_COMP_ID_T
{
  PARTY_COMP_1,
  PARTY_COMP_NS
} PARTY_COMP_ID_T;

/*
typedef enum _IPODIN_COMP_ID_T
{
  IPODIN_COMP_1,
  IPODIN_COMP_NS
} IPODIN_COMP_ID_T;
*/
extern void MW_VDP_SetConnect(UCHAR ucVdpId, UCHAR ucConnect);
extern void MW_VDP_Comp2EsInd(UCHAR ucCompId, UCHAR ucEsId);

/* PMX Component ID */
#define PMX_COMP_NS	PMX_MAX_NS
#define PMX_COMP_1	PMX_1
#define PMX_COMP_2	PMX_1

/* TVE Component ID */
#define TVE_COMP_NS	TVE_MAX_NS
#define TVE_COMP_1	TVE_1
#define TVE_COMP_2	TVE_2

/* AVD Component ID */
// Maximum number of plane mixer
#define AVD_MAX_NS				1
#define AVD_1					0	//
#define AVD_COMP_NS	AVD_MAX_NS
#define AVD_COMP_1	AVD_1

/* CEC Component ID */
#define CEC_COMP_NS	1
#define CEC_COMP_1	0

/* OSD Component ID */
#define OSD_COMP_NS	5
typedef enum _OSD_COMP_ID_T
{
	OSD_COMP_1,
	OSD_COMP_2,
	OSD_COMP_3,
	OSD_COMP_4,					// in the future, virtual osd with vdp
	OSD_COMP_5,					// in the future, virtual osd with vdp
	OSD_COMP_V_NS
} OSD_COMP_ID_T;

/* GFX Component ID */
#define GFX_COMP_NS 1
#define GFX_COMP_1	0

/* AUD Component ID */
#define AUD_MAX_NS    (MAX_AUDDRV_NUM)
#define AUD_COMP_NS	  (MAX_AUDDRV_NUM)
#define AUD_COMP_1ST_ID PRI_DEC
#define AUD_COMP_1    AUD_COMP_1ST_ID  //FIXME: for compatibility


/* JPG Component ID */
#define JPG_COMP_NS 1
#define JPG_COMP_1  0

/* NET Component ID */
#define NET_MAX_NS  1
#define NET_COMP_NS	1
#define NET_COMP_1	0

/* Splitter Component ID */
typedef enum _SPT_COMP_ID_T
{
	SPT_COMP_1,
	SPT_COMP_2,
	SPT_COMP_NS
} SPT_COMP_ID_T;

/* Filter Component ID */
typedef enum _FTR_COMP_ID_T
{
	FTR_COMP_1,
	FTR_COMP_2,
	FTR_COMP_3,
	FTR_COMP_4,
	FTR_COMP_5,
	FTR_COMP_6,
	FTR_COMP_7,
	FTR_COMP_8,
	FTR_COMP_9,
	FTR_COMP_10,
	FTR_COMP_11,
	FTR_COMP_12,
	FTR_COMP_NS
} FTR_COMP_ID_T;

/* GCPU Component ID */
typedef enum _GCPU_COMP_ID_T
{
  GCPU_COMP_1,
  GCPU_COMP_2,
  GCPU_COMP_NS
} GCPU_COMP_ID_T;

/* CPSA Component ID */
typedef enum _CPSA_COMP_ID_T
{
  CPSA_COMP_1,
  CPSA_COMP_2,
  CPSA_COMP_3,
  CPSA_COMP_4,
  CPSA_COMP_NS
} CPSA_COMP_ID_T;

/* KM Component ID */
typedef enum _KM_COMP_ID_T
{
  KM_COMP_1,
  KM_COMP_2,
  KM_COMP_3,
  KM_COMP_4,
  KM_COMP_5,
  KM_COMP_6,
  KM_COMP_NS
} KM_COMP_ID_T;

/* KM Component ID */
typedef enum _AM_COMP_ID_T
{
  AM_COMP_1,
  AM_COMP_2,
  AM_COMP_3,
  AM_COMP_4,
  AM_COMP_5,
  AM_COMP_6,
  AM_COMP_7,
  AM_COMP_8,
  AM_COMP_9,
  AM_COMP_10,
  AM_COMP_NS
} AM_COMP_ID_T;

/* General Purpose Register ID */
typedef enum _GPR_B_COMP_ID_T
{
  GPR_B_COMP_1,
  GPR_B_COMP_2,
  GPR_B_COMP_3,
  GPR_B_COMP_4,
  GPR_B_COMP_5,
  GPR_B_COMP_6,
  GPR_B_COMP_7,
  GPR_B_COMP_NS
} GPR_B_COMP_ID_T;

typedef enum _GPR_DW_COMP_ID_T
{
  GPR_DW_COMP_1,
  GPR_DW_COMP_2,
  GPR_DW_COMP_3,
  GPR_DW_COMP_4,
  GPR_DW_COMP_5,
  GPR_DW_COMP_6,
  GPR_DW_COMP_NS
} GPR_DW_COMP_ID_T;

typedef enum _SYS_COND_STATUS_COMP_ID_T
{
  SYS_COND_STATUS_COMP_1,
  SYS_COND_STATUS_COMP_NS
} SYS_COND_STATUS_COMP_ID_T;

typedef enum _USBHW_COMP_ID_T
{
  USBHW_COMP_1,
  USBHW_COMP_2, 
  USBHW_COMP_NS
} USBHW_COMP_ID_T;

typedef enum _USBHUB_COMP_ID_T
{
  USBHUB_COMP_1,
  USBHUB_COMP_2,
  USBHUB_COMP_3,
  USBHUB_COMP_4,
  USBHUB_COMP_5,
  USBHUB_COMP_6,
  USBHUB_COMP_7,
  USBHUB_COMP_8,
  USBHUB_COMP_9,
  USBHUB_COMP_10,
  USBHUB_COMP_NS
} USBHUB_COMP_ID_T;

typedef enum _USBCARDREADER_COMP_ID_T
{
  USBCARDREADER_COMP_1,
  USBCARDREADER_COMP_2,
  USBCARDREADER_COMP_3,
  USBCARDREADER_COMP_4,
  USBCARDREADER_COMP_5,
  USBCARDREADER_COMP_6,
  USBCARDREADER_COMP_7,
  USBCARDREADER_COMP_8,
  USBCARDREADER_COMP_NS
} USBCARDREADER_COMP_ID_T;

typedef enum _USBBLOCKDEVICE_COMP_ID_T
{
  USBBLOCKDEVICE_COMP_1,
  USBBLOCKDEVICE_COMP_2,
  USBBLOCKDEVICE_COMP_3,
  USBBLOCKDEVICE_COMP_4,
  USBBLOCKDEVICE_COMP_5,
  USBBLOCKDEVICE_COMP_6,
  USBBLOCKDEVICE_COMP_7,
  USBBLOCKDEVICE_COMP_8,
  USBBLOCKDEVICE_COMP_9,
  USBBLOCKDEVICE_COMP_10,
  USBBLOCKDEVICE_COMP_11,
  USBBLOCKDEVICE_COMP_12,
  USBBLOCKDEVICE_COMP_13,
  USBBLOCKDEVICE_COMP_14,
  USBBLOCKDEVICE_COMP_15,
  USBBLOCKDEVICE_COMP_16,
  USBBLOCKDEVICE_COMP_17,
  USBBLOCKDEVICE_COMP_18,
  USBBLOCKDEVICE_COMP_19,
  USBBLOCKDEVICE_COMP_20,
  USBBLOCKDEVICE_COMP_NS
} USBBLOCKDEVICE_COMP_ID_T;

typedef enum _USBSICD_COMP_ID_T
{
  USBSICD_COMP_1,
  USBSICD_COMP_2,
  USBSICD_COMP_3,
  USBSICD_COMP_4,
  USBSICD_COMP_5,
  USBSICD_COMP_6,
  USBSICD_COMP_7,
  USBSICD_COMP_8,
  USBSICD_COMP_NS
} USBSICD_COMP_ID_T;

typedef enum _USBHID_COMP_ID_T
{
  USBHID_COMP_1,
  USBHID_COMP_2,
  USBHID_COMP_3,
  USBHID_COMP_NS
} USBHID_COMP_ID_T;

typedef enum _USBIPOD_COMP_ID_T
{
  USBIPOD_COMP_1,
  USBIPOD_COMP_2,
  USBIPOD_COMP_3,
  USBIPOD_COMP_4,
  USBIPOD_COMP_5,
  USBIPOD_COMP_6,
  USBIPOD_COMP_7,
  USBIPOD_COMP_NS
} USBIPOD_COMP_ID_T;

typedef enum _USBAUD_COMP_ID_T
{
  USBAUD_COMP_1,
  USBAUD_COMP_2,
  USBAUD_COMP_3,
  USBAUD_COMP_4,
  USBAUD_COMP_5,
  USBAUD_COMP_6,
  USBAUD_COMP_7,
  USBAUD_COMP_NS
} USBAUD_COMP_ID_T;



typedef enum _EADEV_COMP_ID_T
{
  EADEV_COMP_1 = 0,
  EADEV_COMP_NS
} EADEV_COMP_ID_T;


#endif //DRV_COMP_ID_H
