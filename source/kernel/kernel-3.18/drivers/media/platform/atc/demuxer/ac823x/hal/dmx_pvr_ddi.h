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
 * @file dmx_pvr_ddi.h
 *
 * @par Project
 *	  MT3360
 *
 * @par Description
 *	  Demuxer pvr ddi related structure, macro, interfaces declarations
 *
 * @par Author_Name
 *	  Shuhui Zhang
 *
 */

#ifndef DMX_PVR_DDI_H_FILE
#define DMX_PVR_DDI_H_FILE

/*-----------------------------------------------------------------------------*/
/* Include files*/
/*-----------------------------------------------------------------------------*/

#include "dmx_pvr_if.h"
#include "dmx_pvr.h"
#include "x_hal_ic.h"

/*-----------------------------------------------------------------------------*/
/* Configurations*/
/*-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------*/
/* Constant definitions*/
/*-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------*/
/* Macro definitions*/
/*-----------------------------------------------------------------------------*/

/* DDI regsiter access commands*/
extern struct dmx_dev_info *g_dmxdevinfo;

#define	DDI_READ32(off)					__raw_readl((volatile void *)(g_dmxdevinfo->dmx_ddi_regs + (off)))
#define	DDI_WRITE32(off, val)			do {\
	__raw_writel((val), (volatile void *)(g_dmxdevinfo->dmx_ddi_regs + (off))); mb();\
} while (0)

#define DDI_POINTER_ALIGNMENT						4
#define DDI_BUF_ALIGNMENT						16

#define DDI_PACKET_SIZE							188



/* DDI registers*/

#define	DDI_REG_GLOBAL_CTRL						0xc00

#define DDI_REG_PERIOD_M						0x410
#define DDI_REG_PERIOD_N						0x414
#define DDI_REG_PERIOD_K						0x418
#define DDI_REG_RATE_CMD						0x420
#define DDI_REG_LATCHED_PERIOD_M					0x440
#define DDI_REG_LATCHED_PERIOD_N					0x444
#define DDI_REG_PKT_QUADBYTE_LIMIT					0x450
#define	DDI_REG_DMX_RX_CTRL						0x460
#define	DDI_REG_DMA_REAL_RP						0x4D0

#define	DDI_REG_DCR_INT_SET						0x800
#define	DDI_REG_DCR_INT_CLR						0x804
#define DDI_REG_DCR_INT_MASK						0x808
#define DDI_REG_DMA_BUF_START						0x850
#define	DDI_REG_DMA_BUF_END						0x854
#define DDI_REG_DMA_RP							0x858
#define	DDI_REG_DMA_RP_INIT						0x85c
#define DDI_REG_DMA_WP							0x864
#define DDI_REG_DMA_AP							0x868
#define DDI_REG_DMA_CTRL						0x86c


/*-----------------------------------------------------------------------------*/
/* Type definitions*/
/*-----------------------------------------------------------------------------*/
#define PVR_DDI_EVENT_GROUP_NAME		("PVR_DDI_NOTIFY_EG")

typedef enum {
	PVR_DDI_NOTIFY_EV_BUF_EMPTY	= 1 << 0,
	PVR_DDI_NOTIFY_EV_BUF_ALERT	= 1 << 1,
	PVR_DDI_NOTIFY_EV_BUF_EXIT	= 1 << 2
} PVR_DDI_NOTIFY_EVENT_T;

typedef bool(*PFN_DDI_NOTIFY)(DDI_EVENT_CODE_T);

#define PVR_DDI_FLAG_NONE				0x00000000
#define PVR_DDI_FLAG_MODE				0x00000001		/* Set Mode*/
#define PVR_DDI_FLAG_ALLOCBUF				0x00000002		/* Allocate Buffer*/
#define PVR_DDI_FLAG_CALLBACK				0x00000004		/* Set/Get callback*/
#define PVR_DDI_FLAG_RATE				0x00000008		/* Set/Get rate (M/N)*/
#define PVR_DDI_FLAG_DATA_SIZE				0x00000010		/* Get data size in DDI*/
#define PVR_DDI_FLAG_SET_BUF				0x00001000
#define PVR_DDI_FLAG_ALL				0xFFFFFFFF

typedef enum {
	PVR_DDI_PORT_FRAMER0,		 /* to Demux framer 0*/
	PVR_DDI_PORT_FRAMER1,
	PVR_DDI_PORT_NOT_FRAMER,
	/*PVR_DDI_PORT_FRAMER2,*/
	/*PVR_DDI_PORT_FRAMER3,*/
	/*PVR_DDI_PORT_FRAMER2_BYPASS,*/
	/*PVR_DDI_PORT_FRAMER3_BYPASS,*/
	/*PVR_DDI_PORT_FRAMER4_BYPASS  // for new dbmport4.*/
} PVR_DDI_PORT_T;

typedef enum {
	PVR_DDI_MODE_SINGLE,		/* Move a single chunk of data*/
	PVR_DDI_MODE_STREAM,		/* Streaming data may have several chunks*/
	PVR_DDI_MODE_NONBLOCKING	/* Non-blocking DDI DMA operation*/
} PVR_DDI_MODE_T;

typedef enum {
	PVR_DDI_STOP,				/* DMA is de-activated (in the Stop state)*/
	PVR_DDI_PLAY				/* DMA is activated (in the Play state)*/
} PVR_DDI_STATE_T;

typedef struct {
	PVR_DDI_MODE_T	eMode;	/* Single mode or Stream mode*/
	bool	fgAllocBuf;
	uintptr_t ptrBufAddr;
	u32	u4BufSize;
	u32	u4Threshold;	/* s32 when buffer is decreased to this value*/
	u32	u4DataSize;	/* how much data is in the DDI buffer*/
	u32	u4RateN;	/* rate = (N / M)  Mbits/sec*/
	u32	u4RateM;	/* rate = (N / M)  Mbits/sec*/
	PFN_DDI_NOTIFY pfnDDINotify;
	/* The callback needs to return FALSE if error.*/
	/* Otherwise, return TRUE.*/
} PVR_DDI_T;

/* DDI*/
typedef struct {
	PVR_DDI_PORT_T	eDDIPortType;
	PVR_DDI_MODE_T	eMode;
	PVR_DDI_STATE_T	eState;
	bool	fgAllocBuf;			/* Indicate if DDI allocates buffer for users.*/
	u32	u4RateN;			/* rate = (N / M) Mbits/sec*/
	u32	u4RateM;			/* rate = (N / M) Mbits/sec*/
	uintptr_t ptrBufAddr;			/*the starting address of buffer*/
	u32	u4BufSize;			/* buffer size*/
	u32	u4Threshold;		/*copy data if the threshold of free space is met*/
	uintptr_t	ptrRp;				/* read pointer to buffer*/
	uintptr_t	ptrWp;				/* write pointer to buffer*/
	u8	u1PacketSize;
	u8	u1SyncOffset;
	bool	fgDDIISRInited;
	HANDLE	hDDINotifyEG;
	PFN_DDI_NOTIFY	pfnDDINotify;
} PVR_DDI_STRUCT_T;

/*-----------------------------------------------------------------------------*/
/* Prototype  of inter-file functions*/
/*-----------------------------------------------------------------------------*/


/* DDI*/

EXTERN bool _PVR_DDI_Init(void);
EXTERN bool _PVR_DDI_Set(u32 u4Flags, const PVR_DDI_T *prDDI);
EXTERN bool _PVR_DDI_Get(u32 u4Flags, PVR_DDI_T *prDDI);
EXTERN bool _PVR_DDI_DeInit(void);

EXTERN void _PVR_DDI_SetPort(PVR_DDI_PORT_T ePort);
EXTERN PVR_DDI_PORT_T _PVR_DDI_GetPort(void);

EXTERN bool _PVR_DDI_SetPacketSize(u8 u1PacketSize);
EXTERN u8 _PVR_DDI_GetPacketSize(void);
EXTERN bool _PVR_DDI_SetSyncOffset(u8 u1Offset);
EXTERN void _PVR_DDI_HWReset(void);

EXTERN bool _PVR_DDI_SingleMove(uintptr_t ptrBufferSa, uintptr_t ptrBufferEa,
				uintptr_t ptrSrcAddr, u32 u4Size);
EXTERN void _PVR_DDI_EndSingleMove(void);
EXTERN void _PVR_DDI_FixDMAEndAddr(u32 *pu4EA, u32 u4WPtr);
EXTERN bool _PVR_DDI_InitISR(void);
EXTERN bool _PVR_DDI_UninitISR(void);
EXTERN void _PVR_DDI_IrqHandler(u16 u2Vector);

EXTERN bool _PVR_DDI_SetISREventHandle(HANDLE hEvent);

EXTERN void _PVR_DDI_SetDMAInt(bool fgEmpty, bool fgAlert);

EXTERN bool _PVR_DDI_DumpInfo(void);

#endif	/* DMX_DDI_H*/

