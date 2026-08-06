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
*/

/*!
 * @file dmx_stream.h
 *
 *
 * @par Project
 *
 * @par Description
 *
 *
 * @par Author_Name
 *	  Shuhui Zhang
 *
 */

#ifndef DMX_STREAM_H
#define DMX_STREAM_H

#include "x_typedef.h"
#ifdef __linux__
#include <media/atc/dmx_define.h>
#include <media/atc/dmx_splitter.h>
#include <media/atc/ioctl_dmx.h>
#else
#include "dmx_define.h"
#include "dmx_splitter.h"
#include "ioctl_dmx.h"
#endif /*__linux__*/

#include "dmx_def.h"

#ifdef __cplusplus
extern "C" {
#endif

/*Enumerate Filter Stream Type*/
typedef enum {
	STREAM_NONE     = SPT_DATA_UNDEFINE,	/*< None stream type*/
	STREAM_VIDEO	  = SPT_DATA_V,			/*< Video stream type*/
	STREAM_AUDIO	  = SPT_DATA_A,			/*< Audio stream type*/
	STREAM_SUBTITLE = SPT_DATA_SP,		/*< Sub Picture type stream type*/
	STREAM_SECTION	= SPT_DATA_SECTION,	/*< Section stream type*/
	STREAM_DMA		  = SPT_DATA_BUF,		/*< DMA (Buffer stream) type(Used For Cfa Sync PbBuf)*/
	STREAM_INITIAL	= SPT_DATA_GRD,		/*< Initial ground stream type(Used For Reset Parser */
/*Offset and Block transferring data into FIFO)*/
} FilterStreamType;

/*! @name Stream Structures (6.2) */
/*! @{ */
/* Filter private instance data*/
/*! @name Stream Related Information */ /*! @{ */
typedef struct {
	bool   fgInUsing;          /*< this instance is in use*/
	bool   fgEnable;            /*< this instance is enable*/
	u32 u4CompId;            /*< filter component ID, it is equal to the index in the strream*/
								/*instanse array -- g_paStmInst*/
	u32 u4StmType;          /*< stream type of this filter manager handle, use FilterStreamType*/
	u32 u4StmUID;            /*< stream id and channel, default is 0, (optional)*/
	u32 u4FifoSize;          /*< stream fifo size*/
	u32 u4VideoCodec;        /*< Stream video codec type, @see FtrVideoCodec*/
	u32 u4FifoThreshold;      /*< Stream fifo threshold (byte) for High bitrate stream*/

/*! @} */
/*! @name Splitter Related Information */
/*! @{ */
	void *pvSptHdl;
	void *pvPsrCC;				/*< Parser handle of this instance*/
	void *pvPsrFtr;		        /*< Parser Filter handle of this instance*/
	u32 u4GAUHandle;			    /*< dmydec handle of this instance*/

	void *pvDmxInst;
} DMX_STM_INST_T;

typedef struct {
	bool	fgStmInitial;
	DMX_STM_INST_T arStmInst[(u32)MAX_STREAM_INSTANCE_CNT];
} DMX_STM_MAN_INFO_T;

typedef struct _STM_ENABLE_INPUTBUF_T {
	void *pvStm;
	bool   fgEnable;
} DMX_STM_ENABLE_INPUTBUF_T;

#define  StreamGetStreamUID(pvStm)                                               \
    ((pvStm) ? ((DMX_STM_INST_T *)(pvStm))->u4StmUID : DMX_INVALID_UINT32)

#define  StreamGetStreamType(pvStm)		                                        \
    ((pvStm) ? ((DMX_STM_INST_T *)(pvStm))->u4StmType : STREAM_NONE)

#define  StreamGetStreamSize(pvStm)		                                        \
    ((pvStm) ? ((DMX_STM_INST_T *)(pvStm))->u4FifoSize : 0)

#define  StreamGetPtxHandle(pvStm)		                                        \
    ((pvStm) ? ((DMX_STM_INST_T *)(pvStm))->pvPsrCC : NULL)

#define  StreamGetPfrHandle(pvStm)		                                        \
    ((pvStm) ? ((DMX_STM_INST_T *)(pvStm))->pvPsrFtr : NULL)

#define  StreamGetSptHandle(pvStm)		                                        \
    ((pvStm) ? ((DMX_STM_INST_T *)(pvStm))->pvSptHdl : NULL)

#define  StreamGetGAUHandle(pvStm)		                                        \
    ((pvStm) ? ((DMX_STM_INST_T *)(pvStm))->ptrGAU : NULL)

#define  StreamGetFifoThreshold(pvStm)	                                        \
    ((pvStm) ? ((DMX_STM_INST_T *)(pvStm))->u4FifoThreshold : DMX_INVALID_UINT32)

#define StreamIsEnabled(pvStm)			                                        \
    ((pvStm) ? ((DMX_STM_INST_T *)(pvStm))->fgEnable : FALSE)

#define StreamSetEnable(pvStm)			                                        \
    ((pvStm) ? ((DMX_STM_INST_T *)(pvStm))->fgEnable  = TRUE : 0)

#define StreamSetDisable(pvStm)			                                        \
    ((pvStm) ? ((DMX_STM_INST_T *)(pvStm))->fgEnable  = FALSE : 0)

#define StreamGetCompId(pvStm)			                                        \
    ((pvStm) ? ((DMX_STM_INST_T *)(pvStm))->u4CompId : 0)

/* Get the Spt Handle of the Stream*/
#define StreamGetSptHandle(pvStm)		                                        \
    ((pvStm) ? ((DMX_STM_INST_T *)(pvStm))->pvSptHdl : NULL)

/* Get the Parser Filter of the Stream*/
#define GetPsrFtrFromStm(pvStm)			                                        \
    ((pvStm) ? ((DMX_STM_INST_T *)(pvStm))->pvPsrFtr : NULL)


MRESULT StreamInit(void);

MRESULT StreamUninit(void);

MRESULT CreateDmaStm(void *pvSptHdl);

MRESULT CreateGrdStm(void *pvSptHdl);

MRESULT DeleteDmaStm(void *pvSptHdl);

MRESULT DeleteGrdStm(void *pvSptHdl);

MRESULT StmDisconnectPsr(void *pvSptHdl);

MRESULT StreamDisconnectSptByHandle(void *pvStm, void *pvSptHdl);

void	*GetStreamByTypes(void *pvDmxInst, u32 u4StreamType);

void	*GetStreamByType(void *pvSptHdl, u32 u4StreamType);
u32	GetStmUIDByType(void *pvSptHdl, u32 u4StreamType);

void	*GetStreamPsrFtrByTypes(void *pvDmxInst, u32 u4StreamType);

void	*GetStreamPsrCCByTypes(void *pvDmxInst, u32 u4StreamType);

u32	GetStreamCntFromType(void *pvDmxInst, void *pvStm);

MRESULT StreamGetCodec(void *pvStm, u32 *pu4Codec);

#ifdef __cplusplus
}
#endif
#endif /* #ifndef DMX_STREAM_H*/
