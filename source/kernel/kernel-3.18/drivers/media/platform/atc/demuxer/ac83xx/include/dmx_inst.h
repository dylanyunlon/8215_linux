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
 * @file dmx_inst.h
 *
 * @par Project
 *
 * @par Description
 *    Demuxer Support Multiple Instances
 *
 * @par Author_Name
 *    Shuhui Zhang
 *
 */

#ifndef DMX_INST_H_FILE
#define DMX_INST_H_FILE


#ifdef __cplusplus
extern "C" {
#endif

#include "x_typedef.h"
#include "chip_ver.h"
#include "drv_config.h"
#include "drv_common.h"
#ifdef __linux__
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/mm.h>
#include <linux/miscdevice.h>
#include "types.h"
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/init.h>
#include "windows.h"
#include "winutil.h"
#include <media/atc/dmx_define.h>
#include <media/atc/dmx_splitter.h>
#include <media/atc/dmx_cfa_def.h>
#include <media/atc/mm_errcode.h>
#include <media/atc/memchk_cfg.h>
#include <media/atc/memdbg_c.h>
#include <media/atc/mm_common.h>
/* #include <media/atc/mm_debug.h> */
#else
#include "dmx_define.h"
#include "dmx_splitter.h"
#include "dmx_cfa_def.h"
#include "mm_errcode.h"
#include "memchk_cfg.h"
#include "memdbg_c.h"
#include "mm_common.h"
#include "mm_debug.h"
#endif				/* __linux__ */

#include "dmx_errcode.h"
#include "dmx_spt.h"
#include "dmx_stream.h"
#include "dmx_gau.h"

typedef struct _dmxInst_t {
	u8 u1DevID;
	bool fgPsrOff;
	DMX_SPT_INST_T * parSpt[MAX_SPT_INST_CNT_PER_DMX];
	DMX_STM_INST_T * parStm[MAX_STM_INST_CNT_PER_DMX];
	u32 u4SptCnt;
	u32 u4StmCnt;
	HANDLE hLock;
	struct _dmxInst_t *prNext;
	struct _dmxInst_t *prPrev;
} DMX_INST_T;

typedef struct _dmxInstlist_t {
	u32 u4Cnt;
	struct _dmxInst_t *prHead;
	struct _dmxInst_t *prTail;
	void   *hLock;
} DMX_INST_LIST_T;

#define DmxInstGetStreamCnt(prDmxInst)  (((prDmxInst) == NULL) ? \
					0 : (((DMX_INST_T *)(prDmxInst))->u4StmCnt))

MRESULT	DmxInstListCreateSema(void);
MRESULT DmxInstListDeleteSema(void);
MRESULT DmxInstListLockSema(void);
MRESULT DmxInstListReleaseSema(void);

MRESULT DmxInstCreateSema(DMX_INST_T *prDmxInst);
MRESULT DmxInstDeleteSema(DMX_INST_T *prDmxInst);
MRESULT DmxInstLockSema(DMX_INST_T *prDmxInst);
MRESULT DmxInstReleaseSema(DMX_INST_T *prDmxInst);

void	DmxGlobalParamsInit(void);

MRESULT DmxInit(void);
MRESULT DmxUninit(void);

void *DmxInstGet(void);
MRESULT DmxInstReset(void *dwContext);

u32	DmxGetInstCount(void);
u32 dmx_inst_create(void);
bool	dmx_inst_release(void *dwContext);
MRESULT dmx_sptinst_create(void *dwContext, void **ppvSptHdl);
MRESULT dmx_sptinst_destroy(void *dwContext, void *pvSptHdl);
MRESULT dmx_sptinst_enable(
	void	*dwContext,
	void 	*pvSptHdl,
	DMX_PBBUF_CONFIG_INFO_T	*prPbbufCfgInfo);
MRESULT dmx_sptinst_disable(void *dwContext, void *pvSptHdl);
bool dmx_spt_isenable(void *dwContext, void *pvSptHdl);
MRESULT DmxSetRate(void *dwContext, SPT_PARAM_SET_RATE *prSetRate);

MRESULT dmx_stminst_create(void *dwContext, STM_PARAM_CREATE *prParam, void **ppvHandle);
MRESULT dmx_stminst_destroy(void *dwContext, void *pvSptHdl, void *pvStm);
MRESULT dmx_stminst_getfifofullness(void *dwContext, void *pvSptHdl,
	u32 u4SptDataType, u32 *pu4FullNess);
MRESULT dmx_stminst_enable(void *dwContext, void *pvStm);
MRESULT dmx_stminst_disable(void *dwContext, void *pvStm);
MRESULT dmx_stminst_setfifoinfo(void *dwContext, void *pvStm, u32 u4FifoSz);
MRESULT dmx_stminst_setfifoflush(void *dwContext, void *pvStm);

MRESULT dmx_parse_on(void *dwContext, DMX_PSR_ON_PARAM_T *prParam);
MRESULT dmx_parse_pause(void *dwContext, void *pvSptHdl);
MRESULT dmx_parse_off(void *dwContext, void *pvSptHdl);

MRESULT dmx_sptinst_getpsrfileofst(void *dwContext, void *pvSptHdl, u64 *pu8FileOfst);

MRESULT dmx_rsp_off(void *dwContext, SPLITTER_PTX_RSP_OFF_INFO_T *prParam);
MRESULT dmx_rsp_rebuf(void *dwContext,
	SPLITTER_PTX_REBUFFER_RANGE_INFO_T *prParam);
MRESULT dmx_rsp_on(void *dwContext, SPLITTER_PTX_RSP_ON_INFO_T *prParam);

MRESULT dmx_cfa_settype(void *dwContext, CFA_PARAM_SET_TYPE *prParam);
MRESULT dmx_cfa_config(void *dwContext, void *pvSptHdl, void *pvConfigInfo, bool fgIsUserMem);
MRESULT dmx_cfa_setrange(void *dwContext, void *pvSptHdl, void *pvRangeInfo, bool fgIsUserMem);
MRESULT dmx_cfa_setinquiretype(void *dwContext, void *pvSptHdl, u32 u4CfaQID);
MRESULT dmx_cfa_setgeneral(void *dwContext,
	void *pvSptHdl, u32 u4CfaFID, void *pvCfaParameter,
	u32 u4CfaParameterSize);
MRESULT dmx_cfa_getgeneral(void *dwContext,
	void *pvSptHdl, u32 u4CfaFID, void *pvCfaParameter,
	u32 u4CfaParameterSize);
MRESULT dmx_cfa_getpsrpos(void *dwContext, void *pvSptHdl, u64 *pu8FileOfst);

MRESULT dmx_pbbuf_allocbuf(void *dwContext, void *pvSptHdl, SEND_BUFFER *prSdBuf);
MRESULT dmx_pbbuf_cancelalloc(void *dwContext, void *pvSptHdl);
MRESULT dmx_pbbuf_sendbuf(void *dwContext, void *pvSptHdl,
	SEND_BUFFER *prSdBuf, bool *pfgExitSent);
MRESULT dmx_pbbuf_releasebuf(void *dwContext, void *pvSptHdl, SEND_BUFFER *prSdBuf);
MRESULT dmx_pbbuf_infonodata(void *dwContext, DMX_PBBUF_NODATA_PARAM_T *prParam);
bool dmx_stm_getau(void *pvStmHdl, void *pvIOBuf);
bool dmx_stm_releaseau(void *pvStmHdl, void *pvIOBuf);


#ifdef __cplusplus
}
#endif
#endif	/* #ifndef DMX_INST_H_FILE */
