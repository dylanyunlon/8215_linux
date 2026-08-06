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

/*!
 * @file dmx_pvr_if.h
 *
 * @par Project
 *	  MT3360
 *
 * @par Description
 *
 *
 * @par Author_Name
 *	  Shuhui Zhang
 *
 */

#ifndef DMX_PVR_IF_H_FILE
#define DMX_PVR_IF_H_FILE

/*-----------------------------------------------------------------------------*/
/* Include files*/
/*-----------------------------------------------------------------------------*/

#include "drv_config.h"
#include "x_typedef.h"
#ifdef __linux__
#include <media/atc/dmx_define.h>
#else
#include "dmx_define.h"
#endif /* __linux__*/

/*-----------------------------------------------------------------------------*/
/* Constant definitions*/
/*-----------------------------------------------------------------------------*/
#define DMX_MAX_DBM_WAIT_COUNT				10

/*====================================*/
#define PVR_NUM_PID_INDEX				(64)	  /*/< Number of PIDs*/
#define PVR_NUM_FILTER_INDEX				(80)	  /*/<Number of filters per tsidx*/
#define PVR_NUM_ONEBYTE_FILTER_INDEX			(64)	  /*/<Number of One byte filters*/
#define FVR_NUM_PID_INDEX				(64)	  /*/<Number of PVR filters*/

#define PVR_PLAY_START_PID				(0)
#define PVR_FVR_START_PID				(64)

/*/ Demux resource limitations*/
#define PVR_FRAMER_COUNT				2
#define PVR_FRAMER_TOTAL_INDEX				2

#define PVR_DBM_PATH_COUNT				4
#define PVR_MIN_TUNER_INDEX				0		/*/< Minimum tuner index*/
#define PVR_MAX_TUNER_INDEX				3		/*/< Maximum tuner index*/

/*/< framer0, framer1, DDI, PVR*/
#define PVR_MAX_CA_MODE					8		/*/< Maximum CA mode*/
#define PVR_MIN_FILTER_OFFSET				1		/*/< Minimum filter offset*/
#define PVR_MAX_FILTER_OFFSET				15		/*/< Maximum filter offset*/
#define PVR_NUM_FILTER_BANK				1		/*/< Number of filter bank*/

#define	PVR_DMEM_CA_KEY_NUM				(16)	/*/MT3363	have 16 keys*/

#define	PVR_NUM_STC_NUM					(2)

#define DMX_DDI_MM_MOVE_TSIDX           (3)

#define DMX_FVR_MM_MOVE_TSIDX           (2)


/* PID Data Structure Info*/
#define	PID_DATA_STRUCT_FIFO_SA					5	/* Start Address of Output FIFO*/
#define	PID_DATA_STRUCT_FIFO_EA					6	/* End Address of Output FIFO*/
#define	PID_DATA_STRUCT_FIFO_PAYLOAD_SA				7	/* Payload Start Address of Output FIFO*/
#define	PID_DATA_STRUCT_FIFO_WP					8	/* Write pointer of Output FIFO*/
#define	PID_DATA_STRUCT_FIFO_RP					9	/* Read pointer of Output FIFO*/
#define PID_DATA_STRUCT_HEADER_SA				10  /* Start Address of Header detect buffer*/
#define PID_DATA_STRUCT_HEADER_EA				11  /* End Address of Header detect buffer*/
#define PID_DATA_STRUCT_HEADER_CURRENT_SA			12  /* Current Start Address of Header detect buffer*/
#define PID_DATA_STRUCT_HEADER_WP				13  /* Write Pointer of Header detect buffer*/
#define PID_DATA_STRUCT_HEADER_RP				14  /* Read Pointer of Header dectect buffer*/

/*/ Function bitmap of PID operations*/
#define PVR_PID_FLAG_VALID				0x00000001	/*/< Enabled or not*/
#define PVR_PID_FLAG_PID				0x00000002	/*/< PID value*/
#define	PVR_PID_FLAG_BUFFER				0x00000004	/*/< Buffer control*/
#define PVR_PID_FLAG_CALLBACK				0x00000008	/*/< Callback handler*/
#define	PVR_PID_FLAG_SCRAMBLE_STATE			0x00000010	/*/< Scrambling state*/
#define PVR_PID_FLAG_TS_INDEX				0x00000020	/*/< TS index*/
#define PVR_PID_FLAG_PCR				0x00000040	/*/< PCR mode*/
#define PVR_PID_FLAG_STEER				0x00000080	/*/< Steering*/
#define PVR_PID_FLAG_DESC_MODE				0x00000100	/*/< Descrambling mode, not use now*/
#define PVR_PID_FLAG_DEVICE_ID				0x00000200	/*/< Device (decoder) ID*/
#define PVR_PID_FLAG_KEY_INDEX				0x00000400	/*/< Key index*/
#define PVR_PID_FLAG_DATA_POINTERS			0x00000800	/*/< Data pointers*/
#define PVR_PID_FLAG_PRIMARY				0x00001000	/*/< Primary PID*/
#define PVR_PID_FLAG_STREAM_ID				0x00002000	/*/< Stream ID*/
#define PVR_PID_FLAG_SUBSTREAM_ID			0x00004000	/*/< Substream ID (SID)*/
#define PVR_PID_FLAG_NONE				0			/*/< None*/
#define PVR_PID_FLAG_ALL				(0xffffffff & ~PVR_PID_FLAG_SCRAMBLE_STATE)

/*/ Function bitmap of filter operations*/
#define PVR_FILTER_FLAG_VALID				0x00000001	/*/< Enabled or not*/
#define PVR_FILTER_FLAG_PIDX				0x00000002	/*/< PID index*/
#define PVR_FILTER_FLAG_OFFSET				0x00000004	/*/< Offset*/
#define	PVR_FILTER_FLAG_CRC				0x00000008	/*/< Check CRC or not*/
#define	PVR_FILTER_FLAG_PATTERN				0x00000010	/*/< Pattern and mask*/
#define PVR_FILTER_FLAG_MODE				0x00000020	/*/< Filter mode*/
#define PVR_FILTER_FLAG_NONE				0			/*/< None*/
#define	PVR_FILTER_FLAG_ALL				0xffffffff	/*/< All*/

/*/ Steering bitmap*/
#define PVR_STEER_TO_PVR				0x02		/*/< Steering to PVR*/
#define PVR_STEER_TO_FTUP				0x04		/*/< Steering to uP*/

/*/ Alignments*/
#define PVR_CA_BUFFER_ALIGNMENT				32			/*/< Alignment of CA buffers*/
/* Misc*/

#define PVR_STARTCODE_NONINTR_NUM			5
#define PVR_STARTCODE_INTR_NUM				5

#define PVR_PICINFO_SIZE				(6)
#define PVR_PICINFO_OFFSET				(3)

/* Multiple instance*/
#define PVR_ALL_INSTINDEX				(0xF0)
#define PVR_NULL_INSTINDEX				(0xFF)

#define	PVR_PVRPLAY_TS_INDEX				2
#define	PVR_DBM_TESTPID_IDX				31

#define PVR_MAX_INST_BYTES_CNT				7

/*-----------------------------------------------------------------------------*/
/* Configurations*/
/*-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------*/
/* Constant definitions*/
/*-----------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------------*/
/* Type definitions*/
/*-----------------------------------------------------------------------------*/

/*---------------------------------------------------------------------*/
/*/ DDI*/
/*---------------------------------------------------------------------*/
typedef enum {
	DDI_EVENT_TRANSFER_OK,
	DDI_EVENT_TRANSFER_FAILED
} DDI_EVENT_CODE_T;

typedef struct {
	u32 u4Size;

	u32 u4SrcBufStart;
	u32 u4SrcBufEnd;
	u32 u4SrcStart;

	u32 u4DstBufStart;
	u32 u4DstBufEnd;
	u32 u4DstStart;
} PVR_AES_DATA_T;

/*-----------------------------------------------------------------------------*/
/* Type definitions*/
/*-----------------------------------------------------------------------------*/
typedef enum {
	PVR_SCRAMBLE_STATE_CLEAR,			/*/< Clear data*/
	PVR_SCRAMBLE_STATE_SCRAMBLED,		/*/< Scrambled data*/
	PVR_SCRAMBLE_STATE_UNKNOWN			/*/< Unknown data*/
} PVR_SCRAMBLE_STATE_T;

typedef enum {
	PVR_SCRAMBLE_TSFLAG_ONLY,			/* check flag only*/
	PVR_SCRAMBLE_PESSTART_ONLY,			/* check PES start code only*/
	PVR_SCRAMBLE_BOTH_TSFLAG_PESSTART
} PVR_SCRAMBLE_TYPE_T;

typedef struct {
	PVR_SCRAMBLE_TYPE_T eType;
	u8 u1Flag;
} DMX_SCRAMBLE_CONFIG_T;

#define PVR_IN_BROADCAST_MASK		0xF0
#define PVR_IN_PLAYBACK_MASK		0xF00

typedef enum {
	PVR_IN_NONE			= 0,
	PVR_IN_DIAG			= 1,
	PVR_IN_BROADCAST_TS		= 0x10,
	PVR_IN_PLAYBACK_TS		= 0x100,
	PVR_IN_PLAYBACK_MM		= 0x200,
	PVR_IN_PVR_TS			= 0x400,
} PVR_INPUT_TYPE_T;

typedef enum {
	PVR_EDGE_POSITIVE,
	PVR_EDGE_NEGATIVE,
} PVR_EDGE_T;

typedef enum {
	PVR_SYNC_INTERNAL,
	PVR_SYNC_EXTERNAL,
} PVR_SYNC_T;

typedef enum {
	PVR_FRAMER_SERIAL = 0,
	PVR_FRAMER_PARALLEL = 1,
	PVR_FRAMER_TWOBIT = 2,
	PVR_FRAMER_UNKNOWN = 3
} PVR_FRAMER_MODE_T;

/***********************************************************************************************/
/* TODO: This should be modify (MTK40144) ---- BEGIN*/
/***********************************************************************************************/
/*/ pin set*/
typedef enum {
	PVR_PINSET_INTERNAL = 0x0,
	PVR_PINSET_EXT1   = 0x10,
	PVR_PINSET_EXT2_0 = 0x20,
	PVR_PINSET_EXT2_1 = 0x30,
	PVR_PINSET_EXT2_2 = 0x40,
	PVR_PINSET_EXT2_3 = 0x50,
	PVR_PINSET_EXT2_4 = 0x60,
	PVR_PINSET_EXT3_1 = 0x70,
	PVR_PINSET_EXT3_2 = 0x80,
	PVR_PINSET_EXT4 = 0x90,
	PVR_PINSET_EXT5 = 0xA0,
} PVR_PINSET_T;
/***********************************************************************************************/
/*TODO: This should be modify (MTK40144) ---- END*/
/***********************************************************************************************/


/*/ Front-end config struct*/
typedef struct {
	bool							fgEnable;
	PVR_PINSET_T			ePinSet;
	PVR_FRAMER_MODE_T	eMode;
	PVR_EDGE_T				eEdge;
	PVR_SYNC_T				eSync;
} PVR_FRONTEND_CONFIG_T;

typedef enum {
	PVR_TS_SEL_NOTHING = 0,
	PVR_TS_SEL_EXT_TS,
	PVR_TS_SEL_DEMUX,
} E_PVR_TS_IN_SEL_T;

/* Rule*/
/*	 above 0x10 is external TS*/
/*	 when above 0x10, 0=serial, 1=parallel, 2:2bit*/
/* Set by TS index*/
typedef enum {
	PVR_FE_NO_TSVALID = 0,
	PVR_FE_DDI = 2,
	PVR_FE_EXT_S = 0x10,
	PVR_FE_EXT_P = 0x11,
	PVR_FE_EXT_2BIT = 0x12,
	PVR_FE_NULL = 0xFF
} PVR_FRONTEND_T;

typedef enum {
	PVR_SOURCE_DTV = 0,			/* Demod*/
	PVR_SOURCE_TSFILE,			/* TS file*/
	PVR_SOURCE_DRAM,			/* AVI, MKV, ES...*/
	PVR_SOURCE_NUM
} PVR_SOURCE_T;

typedef enum {
	PVR_CONN_TYPE_TUNER_0 = 0,
	PVR_CONN_TYPE_TUNER_1,
	PVR_CONN_TYPE_TUNER_MAX,
	PVR_CONN_TYPE_BUFAGENT_0,
	PVR_CONN_TYPE_BUFAGENT_1,
	PVR_CONN_TYPE_BUFAGENT_2,
	PVR_CONN_TYPE_BUFAGENT_3,
	PVR_CONN_TYPE_BUFAGENT_4,
	PVR_CONN_TYPE_DIRECT_PLAY_1,
	PVR_CONN_TYPE_DIRECT_PLAY_2,
	PVR_CONN_TYPE_BUFAGENT_MAX,
	PVR_CONN_TYPE_NONE,
} PVR_CONN_TYPE_T;

typedef enum {
	PVR_DBM_INPUT_PB = 0,
	PVR_DBM_INPUT_DDI = 1,
	PVR_DBM_INPUT_FRAMER = 2,
} PVR_DBM_INPUT_SOURCE_T;

typedef enum {
	PVR_TSFMT_NONE,
	PVR_TSFMT_188,
	PVR_TSFMT_192,
	PVR_TSFMT_192_ENCRYPT,
	PVR_TSFMT_204,
	PVR_TSFMT_TIMESHIFT,
} PVR_TSFMT_T;

/*/ PMT data structure for PID swap*/
typedef struct {
	u16 u2Pid;				/*/< PID of this PMT*/
	u16 u2ProgramNumber;		/*/< Program number*/
	u16 u2PcrPid;			/*/< PCR PID*/
	u16 u2VideoPid;			/*/< Video PID*/
	u16 u2AudioPid;			/*/< Audio PID*/
	u8  u1AudioType;			/*/< Type of audio stream*/
} PVR_PMT_T;

/*/ PID types*/
typedef enum {
	PVR_PID_TYPE_NONE = 0,		/*/< None*/
	PVR_PID_TYPE_PSI,			/*/< PSI*/
	PVR_PID_TYPE_PES,			/*/< PES (NON-AV)*/
	PVR_PID_TYPE_ES_VIDEO,		/*/< Video ES*/
	PVR_PID_TYPE_ES_AUDIO,		/*/< Audio ES*/
	PVR_PID_TYPE_ES_VIDEOCLIP,	/*/< Video clip ES*/
	PVR_PID_TYPE_ES_OTHER,		/*/< Other ES*/
} PVR_PID_TYPE_T;

typedef enum {
	PVR_VIDEO_UNKNOWN,
	PVR_VIDEO_MPEG,
	PVR_VIDEO_H263,
	PVR_VIDEO_H264,
	PVR_VIDEO_VC1,
	PVR_VIDEO_MPEG4,
	PVR_VIDEO_WMV7,
	PVR_VIDEO_WMV8,
	PVR_VIDEO_WMV9,
	PVR_VIDEO_MP4_IN_WMV,
	PVR_VIDEO_RV,
	PVR_VIDEO_VP6,
	PVR_VIDEO_VP8,
	PVR_VIDEO_H265
} PVR_VIDEO_TYPE_T;

typedef struct _PVR_INST_BYTES_INFO_T_ {
	bool	fgInsertBytes;
	u32	u4InsertLen;
	u32	u4Inserttimes;
	u8	*pu1InsertBuf;
} PVR_INST_BYTES_INFO_T;

typedef struct {
	u8				u1TsIdx;	/* Ts index combine with ext ckgen*/
	PVR_FRONTEND_T		eFrontEnd;
	PVR_TSFMT_T			eTSFmt;
	PVR_VIDEO_TYPE_T	eVideoType;
	u8				u1Instance;	/* Multiple instance*/
} PVR_SOURCE_INFO_T;

/*/ PCR mode*/
typedef enum {
	PVR_PCR_MODE_NONE = 0,				/*/< None*/
	PVR_PCR_MODE_OLD,					/*/< Old mode (5371 type)*/
	PVR_PCR_MODE_NEW					/*/< New mode*/
} PVR_PCR_MODE_T;

/*/ Descrambling schemes*/
/*/ Keep this just for backward compatible, not use after 5368*/
typedef enum {
	PVR_DESC_MODE_NONE = 0,				/*/< None*/
	PVR_DESC_MODE_DES_ECB,				/*/< DES ECB*/
	PVR_DESC_MODE_DES_CBC,				/*/< DES CBC*/
	PVR_DESC_MODE_3DES_ECB,				/*/< 3DES ECB*/
	PVR_DESC_MODE_3DES_CBC,				/*/< 3DES CBC*/
	PVR_DESC_MODE_DVB,					/*/< DVB*/
	PVR_DESC_MODE_DVB_CONF,				/*/< DVB conformance*/
	PVR_DESC_MODE_MULTI2_BIG,			/*/< Multi-2 big-endian*/
	PVR_DESC_MODE_MULTI2_LITTLE,		/*/< Multi-2 little-endian*/
	PVR_DESC_MODE_AES_ECB,				/*/< AES ECB*/
	PVR_DESC_MODE_AES_CBC,				/*/< AES CBC*/
} PVR_DESC_MODE_T;

/*/ Descrambling residual termination block(RTB)*/
typedef enum {
	PVR_DESC_RTB_MODE_CLEAR = 0,		/*/< leave clear*/
	PVR_DESC_RTB_MODE_CTS,				/*/< CTS*/
	PVR_DESC_RTB_MODE_SCTE52,			/*/< SCTE-52*/
	PVR_DESC_RTB_MODE_MAX			   /*/< END*/
} PVR_DESC_RTB_MODE_T;

/*/ Descrambling schemes*/
typedef enum {
	PVR_CA_MODE_NONE = 0x80,		/*/< None, start fomr 0x80 is due to set difference to PVR_DESC_MODE_T*/
	PVR_CA_MODE_DES,
	PVR_CA_MODE_3DES,
	PVR_CA_MODE_DVB,				/*/< DVB*/
	PVR_CA_MODE_DVB_CONF,			/*/< DVB conformance*/
	PVR_CA_MODE_MULTI2_BIG,			/*/< Multi-2 big-endian*/
	PVR_CA_MODE_MULTI2_LITTLE,		/*/< Multi-2 little-endian*/
	PVR_CA_MODE_AES,
	PVR_CA_MODE_SMS4
} PVR_CA_MODE_T;

typedef enum {
	PVR_CA_FB_ECB = 0,				  /* No feedback*/
	PVR_CA_FB_CBC,
	PVR_CA_FB_CFB,
	PVR_CA_FB_OFB,
	PVR_CA_FB_CTR					  /*< Counter mode*/
} PVR_CA_FEEDBACK_MODE_T;

typedef enum {
	PVR_DRM_MODE_BYPASS,
	PVR_DRM_MODE_AES,
	PVR_DRM_MODE_ND,
	PVR_DRM_MODE_PD,
	PVR_DRM_MODE_UNKNOWN
} PVR_DRM_MODE_T;

typedef struct {
	u16 u2KeyLen;
	u8  au1Key[32];
	u8  au1InitVector[16];
	bool   fgDoEncrypt;
	bool   fgCbc;
	PVR_DRM_MODE_T eMode;
	u32 u4DecryptOfst;
	u32 u4DecryptLen;
} PVR_DRM_PARAM_T;

/* Notification code*/
typedef enum {
	PVR_NOTIFY_CODE_PSI,				/*/< PSI notification*/
	PVR_NOTIFY_CODE_ES,					/*/< ES notification*/
	PVR_NOTIFY_CODE_PES,				/*/< PES notification*/
	PVR_NOTIFY_CODE_PES_TIME,			/*/< PES notification with time information*/
	PVR_NOTIFY_CODE_SCRAMBLE_STATE,		/*/< Scramble state change notification*/
	PVR_NOTIFY_CODE_OVERFLOW,			/*/< Overflow notification*/
	PVR_NOTIFY_CODE_STREAM_ID			/*/< Report pre-parsed Stream IDs*/
} PVR_NOTIFY_CODE_T;

/* Demux notification callback function*/
typedef MRESULT(*PFN_PVR_NOTIFY)(u8 u1Pidx,
	PVR_NOTIFY_CODE_T eCode,
	u32 u4Data,
	const void *pvNotifyTag);

/* PID data structure*/
typedef struct {
	bool	fgEnable;				/*/< Enable or disable*/
	bool	fgAllocateBuffer;		/*/< Allocate buffer*/
	bool	fgPrimary;				/*/< Primary PID*/
	u8	u1TsIndex;				/*/< TS index*/
	u8	u1DeviceId;				/*/< Device ID*/
	u8	u1KeyIndex;				/*/< Descramble key index*/
	u8	u1SteerMode;			/*/< Steering mode*/
	u16 u2Pid;					/*/< PID*/
	u32 u4BufAddr;				/*/< Buffer address*/
	u32 u4BufSize;				/*/< Buffer size*/
	u32 u4Rp;					/*/< Read pointer*/
	u32 u4Wp;					/*/< Write pointer*/
	u32 u4PeakBufFull;			/*/< Peak buffer fullness, to estimate*/
									/*/< required ES FIFO size*/
	PVR_PCR_MODE_T	ePcrMode;		/*/< PCR mode*/
	PVR_PID_TYPE_T	ePidType;		/*/< PID type*/
	PVR_DESC_MODE_T	eDescMode;		/*/< Descramble mode*/
	PFN_PVR_NOTIFY	pfnNotify;		/*/< Callback function*/
	void		*pvNotifyTag;		/*/< Tag value of callback function*/
	PFN_PVR_NOTIFY	pfnScramble;	/*/< Callback function of scramble state*/
	void		*pvScrambleTag;		/*/< Tag value of scramble callback function*/
} PVR_PID_T;

/* --- End of "CI SLT from TS-output" ---*/
/*-----------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------------*/
/* Macro definitions*/
/*-----------------------------------------------------------------------------*/

/* Data size calculation*/
#define DMX_DATASIZE(rp, wp, size)		\
	(((rp) <= (wp)) ? ((wp) - (rp)) : (((wp) + (size)) - (rp)))


#define DMX_EMPTYSIZE(rp, wp, size)		\
	(((wp) < (rp)) ? ((rp) - (wp)) : (((rp) + (size)) - (wp)))

/*-----------------------------------------------------------------------------*/
/* Prototype  of inter-file functions*/
/*-----------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------------*/
/* Prototype  of public functions*/
/*-----------------------------------------------------------------------------*/

#endif	/* _PVR_PVR_IF_H_*/

