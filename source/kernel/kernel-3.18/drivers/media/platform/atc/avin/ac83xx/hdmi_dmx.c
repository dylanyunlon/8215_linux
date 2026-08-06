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

#include "windev.h"
/*#include "../../../../../include/media/atc/ose_mem.h"*/
/*#include "mm_debug.h"*/
#include "dmx_splitter.h"
#include "dmx_cfa_audin.h"
#include "dmx_event.h"
#include "dmx_def.h"
#include "dmx_inst.h"
#include "aud_esm.h"
#include "ioctl_dmx.h"
/*#include "ac83xx_memory.h"*/
#include "hdmi_dmx.h"
#include <linux/platform_device.h>
#include <linux/slab.h>
/*#include <linux/videodev2.h>*/
/*#include <media/v4l2-ioctl.h>*/
#include "winutil.h"


#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG				"HDMIDmx"
#define INVALID_HANDLE_VALUE (-1)
#define DMX_DRIVER_NAME		_T("/dev/demuxer")
int  m_u4State;
u32 m_hDmxDrv = 0;
HANDLE m_hSpt;
HANDLE m_hStream;
HANDLE m_hPsrOffEvt;
ESM_IO_BUF_INFO mAudioEsmBufInfo;

/*extern long demuxer_ioctl(struct file *file, unsigned int cmd, unsigned long arg);

BOOL DmxIoCtl(struct filp *file, DWORD IoControlCOde, VOID *lpInBuf, DWORD InBufSize ,
	      VOID *lpOutBuf, DWORD OutBufSize , DWORD *lpBytesReturned, VOID *reserver3)
{
	WIN32_IOCTL_DATA pData;

	pData.pInBuf = lpInBuf;
	pData.InSize = InBufSize;
	pData.pOutBuf = lpOutBuf;
	pData.OutSize = OutBufSize;
	pData.pBytesReturned = lpBytesReturned;

	int ret = demuxer_ioctl(file, IoControlCOde, &pData);

	if (ret < 0) {
		return FALSE;
	} else {
		return TRUE;
	}
}*/
/*
extern u32	dmx_inst_create(void);
extern bool	dmx_inst_release(u32 dwContext);
extern MRESULT dmx_sptinst_create(u32 dwContext, HANDLE *phSpt);
extern MRESULT dmx_sptinst_destroy(u32 dwContext, HANDLE hSpt);
extern MRESULT dmx_sptinst_enable(
	u32	dwContext,
	HANDLE	hSpt,
	DMX_PBBUF_CONFIG_INFO_T	*prPbbufCfgInfo);
extern MRESULT dmx_sptinst_disable(u32 dwContext, HANDLE hSpt);
extern bool dmx_spt_isenable(u32 dwContext, HANDLE hSpt);
extern MRESULT DmxSetRate(u32 dwContext, SPT_PARAM_SET_RATE *prSetRate);

extern MRESULT dmx_stminst_create(u32 dwContext, STM_PARAM_CREATE *prParam, HANDLE *pHandle);
extern MRESULT dmx_stminst_destroy(u32 dwContext, HANDLE hSpt, HANDLE hStm);
extern MRESULT dmx_stminst_getfifofullness(u32 dwContext, HANDLE hSpt,
	u32 u4SptDataType, u32 *pu4FullNess);
extern MRESULT dmx_stminst_enable(u32 dwContext, HANDLE hStm);
extern MRESULT dmx_stminst_disable(u32 dwContext, HANDLE hStm);
extern MRESULT dmx_stminst_setfifoinfo(u32 dwContext, HANDLE hStm, u32 u4FifoSz);

extern MRESULT dmx_parse_on(u32 dwContext, DMX_PSR_ON_PARAM_T *prParam);
extern MRESULT dmx_parse_pause(u32 dwContext, HANDLE hSpt);
extern MRESULT dmx_parser_off(u32 dwContext, HANDLE hSpt);

extern MRESULT dmx_sptinst_getpsrfileofst(u32 dwContext, HANDLE hSpt, u64 *pu8FileOfst);

extern MRESULT dmx_rsp_off(u32 dwContext, SPLITTER_PTX_RSP_OFF_INFO_T *prParam);
extern MRESULT dmx_rsp_rebuf(u32 dwContext,
	SPLITTER_PTX_REBUFFER_RANGE_INFO_T *prParam);
extern MRESULT dmx_rsp_on(u32 dwContext, SPLITTER_PTX_RSP_ON_INFO_T *prParam);

extern MRESULT dmx_cfa_settype(u32 dwContext, CFA_PARAM_SET_TYPE *prParam);
extern MRESULT dmx_cfa_config(u32 dwContext, HANDLE hSpt, void *pvConfigInfo);
extern MRESULT dmx_cfa_setrange(u32 dwContext, HANDLE hSpt, void *pvRangeInfo);
extern MRESULT dmx_cfa_setinquiretype(u32 dwContext, HANDLE hSpt, u32 u4CfaQID);
extern MRESULT dmx_cfa_setgeneral(u32 dwContext,
	HANDLE hSpt, u32 u4CfaFID, void *pvCfaParameter,
	u32 u4CfaParameterSize);
extern MRESULT dmx_cfa_getgeneral(u32 dwContext,
	HANDLE hSpt, u32 u4CfaFID, void *pvCfaParameter,
	u32 u4CfaParameterSize);
extern MRESULT dmx_cfa_getpsrpos(u32 dwContext, HANDLE hSpt, u64 *pu8FileOfst);

extern MRESULT dmx_pbbuf_allocbuf(u32 dwContext, HANDLE hSpt, SEND_BUFFER *prSdBuf);
extern MRESULT dmx_pbbuf_cancelalloc(u32 dwContext, HANDLE hSpt);
extern MRESULT dmx_pbbuf_sendbuf(u32 dwContext, HANDLE hSpt,
	SEND_BUFFER *prSdBuf, bool *pfgExitSent);
extern MRESULT dmx_pbbuf_releasebuf(u32 dwContext, HANDLE hSpt, SEND_BUFFER *prSdBuf);
extern MRESULT dmx_pbbuf_infonodata(u32 dwContext, DMX_PBBUF_NODATA_PARAM_T *prParam);
extern bool dmx_stm_getau(u32 u4StmHdl, void *pvIOBuf);
extern bool dmx_stm_releaseau(u32 u4StmHdl, void *pvIOBuf);
*/

int DMXInit(void)
{
	CFA_PARAM_SET_TYPE rCfaParam;
	STM_PARAM_CREATE rStmParam;
	SPT_PARAM_ENABLE rSptEnableParam;
	AUD_POSINFO_T rAudPos;
	MRESULT mrRet = RET_DMX_OK;

	m_u4State = ST_DMX_UNKNOWN;
	m_hDmxDrv = 0;
	m_hSpt = NULL;
	m_hStream = NULL;

	memset(&rCfaParam, 0, sizeof(rCfaParam));
	memset(&rStmParam, 0, sizeof(rStmParam));
	memset(&rSptEnableParam, 0, sizeof(rSptEnableParam));
	memset(&rAudPos, 0, sizeof(rAudPos));

	pr_info("[AVIN][hdmi_dmx]%s enter\r\n", __func__);

	if (m_u4State != ST_DMX_UNKNOWN && m_u4State != ST_DMX_UNINIT) {
		return 0;
	}

	m_hPsrOffEvt = x_event_create(NULL, TRUE, FALSE, SPT_EVT_NAME_PSROFF_0); /* manual-reset*/

	if (NULL == m_hPsrOffEvt) {
		return 0;
	}

	m_hDmxDrv = dmx_inst_create();

	if (0 == m_hDmxDrv) {
		pr_info("[AVIN][hdmi_dmx]%s dmx_inst_create fail\r\n", __func__);
		goto ERRINIT;
	} else {
		pr_info("[AVIN][hdmi_dmx]%s dmx_inst_create success\r\n", __func__);
	}

	mrRet = dmx_sptinst_create(m_hDmxDrv, &m_hSpt);

	if (DMX_FAILED(mrRet)) {
		pr_info("[AVIN][hdmi_dmx]%s DMX_IOCTL_SPT_CREATE fail\r\n", __func__);
		goto ERRINIT;
	} else {
		pr_info("[AVIN][hdmi_dmx]%s DMX_IOCTL_SPT_CREATE success\r\n", __func__);
	}

	if (NULL == m_hSpt) {
		pr_info("[AVIN][hdmi_dmx]%s m_hSpt is null, fail\r\n", __func__);
		goto ERRINIT;
	}

	rCfaParam.pvSptHdl = m_hSpt;
	rCfaParam.u4Type = CFA_TYPE_AUDIN;
	mrRet = dmx_cfa_settype(m_hDmxDrv, &rCfaParam);

	if (DMX_FAILED(mrRet)) {
		pr_info("[AVIN][hdmi_dmx]%s dmx_cfa_settype fail\r\n", __func__);
		goto ERRINIT;
	} else {
		pr_info("[AVIN][hdmi_dmx]%s dmx_cfa_settype success\r\n", __func__);
	}

	pr_info("[AVIN][hdmi_dmx]%s after dmx_cfa_settype\r\n", __func__);

	rStmParam.pvSptHdl = m_hSpt;
	rStmParam.u4StmType = SPT_DATA_A;
	rStmParam.u4StmUID = 5;
	rStmParam.u8DecSendBufMask = 0;
	mrRet = dmx_stminst_create(m_hDmxDrv, &rStmParam, &m_hStream);

	if (DMX_FAILED(mrRet)) {
		pr_info("[AVIN][hdmi_dmx]%s dmx_stminst_create fail\r\n", __func__);
		goto ERRINIT;
	} else {
		pr_info("[AVIN][hdmi_dmx]%s dmx_stminst_create success\r\n", __func__);
	}

	pr_info("[AVIN][hdmi_dmx]%s after dmx_stminst_create\r\n", __func__);
	
	mm_memset(&rAudPos, 0, sizeof(rAudPos));
	
	if (0 != i4AudEsm_GetAudioCodecFifoInfo(AUD_FIFO_HDMI_RX, &rAudPos)) {
		pr_info("[AVIN][hdmi_dmx]%s fail in i4AudEsm_GetAudioCodecFifoInfo\r\n", __func__);
		goto ERRINIT;
	}

	mrRet = dmx_stminst_setfifoinfo(m_hDmxDrv, m_hStream,
		(rAudPos.ptrAfifoEA - rAudPos.ptrAfifoSA));

	if (DMX_FAILED(mrRet)) {
		pr_info("[AVIN][hdmi_dmx]%s dmx_stminst_setfifoinfo fail\r\n", __func__);
		goto ERRINIT;
	} else {
		pr_info("[AVIN][hdmi_dmx]%s dmx_stminst_setfifoinfo success\r\n", __func__);
	}

	pr_info("[AVIN][hdmi_dmx]%s after dmx_stminst_setfifoinfo\r\n", __func__);

	rSptEnableParam.fgBadInterLeave = FALSE;
	rSptEnableParam.pvSptHdl = m_hSpt;
	rSptEnableParam.rPbbufCfgInfo.eSptPbuffType = SPT_PBUFF_AUDIN;
	rSptEnableParam.rPbbufCfgInfo.u4PBBufTotalSz = 1024*1024*2ul;//AUDIO_IN_MEM_SIZE;
	rSptEnableParam.rPbbufCfgInfo.u4PBBufSlotSz = 4 * 1024;
	mrRet = dmx_sptinst_enable(m_hDmxDrv, m_hSpt, &(rSptEnableParam.rPbbufCfgInfo));

	if (DMX_FAILED(mrRet)) {
		pr_info("[AVIN][hdmi_dmx]%s dmx_sptinst_enable fail\r\n", __func__);
		goto ERRINIT;
	} else {
		pr_info("[AVIN][hdmi_dmx]%s dmx_sptinst_enable success\r\n", __func__);
	}

	pr_info("[AVIN][hdmi_dmx]%s after dmx_sptinst_enable\r\n", __func__);

	m_u4State = ST_DMX_INIT;
	pr_info("[AVIN][hdmi_dmx]%s leave\r\n", __func__);
	return 1;

ERRINIT:

	if (NULL != m_hStream) {
		mrRet = dmx_stminst_disable(m_hDmxDrv, m_hStream);

		if (DMX_FAILED(mrRet)) {
			pr_info("[AVIN][hdmi_dmx]%s dmx_stminst_disable fail\r\n", __func__);
			return 0;
		}

		mrRet = dmx_stminst_destroy(m_hDmxDrv, m_hSpt, m_hStream);

		if (DMX_FAILED(mrRet)) {
			pr_info("[AVIN][hdmi_dmx]%s dmx_stminst_destroy fail\r\n", __func__);
			return 0;
		}

		m_hStream = NULL;
	}

	if (NULL != m_hSpt) {
		mrRet = dmx_sptinst_disable(m_hDmxDrv, m_hSpt);

		if (DMX_FAILED(mrRet)) {
			pr_info("[AVIN][hdmi_dmx]%s dmx_sptinst_disable fail\r\n", __func__);
			return 0;
		}

		mrRet = dmx_sptinst_destroy(m_hDmxDrv, m_hSpt);

		if (DMX_FAILED(mrRet)) {
			pr_info("[AVIN][hdmi_dmx]%s dmx_sptinst_destroy fail\r\n", __func__);
			return 0;
		}

		m_hSpt = NULL;
	}

	if (NULL != m_hPsrOffEvt) {
		x_event_destroy(m_hPsrOffEvt);
		m_hPsrOffEvt = NULL;
	}

	if (0 != m_hDmxDrv) {
		dmx_inst_release(m_hDmxDrv);
		m_hDmxDrv = 0;
	}
	pr_info("[AVIN][hdmi_dmx]%s leave fail\r\n", __func__);
	return 0;
}

int DMXUninit(void)
{
	MRESULT mrRet = RET_DMX_OK;
	/*int sResult = RET_IS_OK;*/

	pr_info("[AVIN][hdmi_dmx]%s enter\r\n", __func__);

	mrRet = dmx_stminst_destroy(m_hDmxDrv, m_hSpt, m_hStream);

	if (DMX_FAILED(mrRet)) {
		pr_info("[AVIN][hdmi_dmx]%s dmx_stminst_destroy fail\r\n", __func__);
		return 0;
	}

	m_hStream = NULL;

	mrRet = dmx_sptinst_disable(m_hDmxDrv, m_hSpt);

	if (DMX_FAILED(mrRet)) {
		pr_info("[AVIN][hdmi_dmx]%s dmx_sptinst_disable fail\r\n", __func__);
		return 0;
	}

	mrRet = dmx_sptinst_destroy(m_hDmxDrv, m_hSpt);

	if (DMX_FAILED(mrRet)) {
		pr_info("[AVIN][hdmi_dmx]%s dmx_sptinst_destroy fail\r\n", __func__);
		return 0;
	}

	m_hSpt = NULL;

	dmx_inst_release(m_hDmxDrv);

	if (NULL != m_hPsrOffEvt) {
		x_event_destroy(m_hPsrOffEvt);/*need change*/
		m_hPsrOffEvt = NULL;
	}

	m_hDmxDrv = 0;
	m_u4State = ST_DMX_UNINIT;
	pr_info("[AVIN][hdmi_dmx]%s leave\r\n", __func__);

	return 1;
}

int DMXStart(void)
{
	CfaAudInPR rCfaInfo;
	DMX_PSR_ON_PARAM_T rDmxParseOnParam;
	CfaAudInCfgInf rAudInCfg;
	bool fgIsUserMem = FALSE;

	MRESULT mrRet = RET_DMX_OK;
	/*int sResult = RET_IS_OK;*/

	pr_info("[AVIN][hdmi_dmx]%s enter\r\n", __func__);

	if (ST_DMX_START == m_u4State) {
		return 0;
	}

	memset(&rCfaInfo, 0, sizeof(rCfaInfo));
	memset(&rDmxParseOnParam, 0, sizeof(rDmxParseOnParam));
	memset(&rAudInCfg, 0, sizeof(rAudInCfg));

	/*\\LJAN*/

	mrRet = dmx_cfa_config(m_hDmxDrv, m_hSpt, &rAudInCfg, fgIsUserMem);

	if (DMX_FAILED(mrRet)) {
		pr_info("[AVIN][hdmi_dmx]%s dmx_cfa_config fail\r\n", __func__);
		return 0;
	}

	pr_info("[AVIN][hdmi_dmx]%s dmx_cfa_config success\r\n", __func__);


	mrRet = dmx_stminst_enable(m_hDmxDrv, m_hStream);

	if (DMX_FAILED(mrRet)) {
		pr_info("[AVIN][hdmi_dmx]%s dmx_stminst_enable fail\r\n", __func__);
		return 0;
	}

	pr_info("[AVIN][hdmi_dmx]%s dmx_stminst_enable success\r\n", __func__);

	rCfaInfo.u8Sa = 0;
	rCfaInfo.u8Ea = (UINT64) - 1;
	mrRet = dmx_cfa_setrange(m_hDmxDrv, m_hSpt, &rCfaInfo, fgIsUserMem);

	if (DMX_FAILED(mrRet)) {
		pr_info("[AVIN][hdmi_dmx]%s dmx_cfa_setrange fail\r\n", __func__);
		return 0;
	}

	pr_info("[AVIN][hdmi_dmx]%s dmx_cfa_setrange success\r\n", __func__);


	rDmxParseOnParam.pvSptHdl = m_hSpt;
	rDmxParseOnParam.rStmsCnt.u4VidStmCnt = 0;
	rDmxParseOnParam.rStmsCnt.u4AudStmCnt = 1;
	rDmxParseOnParam.rStmsCnt.u4SPStmCnt = 0;
	mrRet = dmx_parse_on(m_hDmxDrv, &rDmxParseOnParam);

	if (DMX_FAILED(mrRet)) {
		pr_info("[AVIN][hdmi_dmx]%s dmx_parse_on fail\r\n", __func__);
		return 0;
	}

	pr_info("[AVIN][hdmi_dmx]%s dmx_parse_on success\r\n", __func__);

	m_u4State = ST_DMX_START;

	pr_info("[AVIN][hdmi_dmx]%s leave\r\n", __func__);
	return 1;
}

int DMXStop(void)
{
	MRESULT mrRet = RET_DMX_OK;
	/*int sResult = RET_IS_OK;*/

	pr_info("[AVIN][hdmi_dmx]%s enter\r\n", __func__);

	if (m_u4State == ST_DMX_STOP) {
		pr_info("[AVIN][hdmi_dmx]%s already stop state\r\n", __func__);
		return 0;
	}

	mrRet = dmx_parse_pause(m_hDmxDrv, m_hSpt);

	if (DMX_FAILED(mrRet)) {
		pr_info("[AVIN][hdmi_dmx]%s dmx_parse_pause fail\r\n", __func__);
		return 0;
	}

	mrRet = dmx_stminst_disable(m_hDmxDrv, m_hStream);

	if (DMX_FAILED(mrRet)) {
		pr_info("[AVIN][hdmi_dmx]%s dmx_stminst_disable fail\r\n", __func__);
		return 0;
	}

	mrRet = dmx_parse_off(m_hDmxDrv, m_hSpt);

	if (DMX_FAILED(mrRet)) {
		pr_info("[AVIN][hdmi_dmx]%s dmx_parse_off fail\r\n", __func__);
		return 0;
	}

	/*WaitForSingleObject(m_hPsrOffEvt, INFINITE);*/
	x_event_reset(m_hPsrOffEvt);/*?*/

	mrRet = dmx_stminst_setfifoflush(m_hDmxDrv, m_hStream);

	if (DMX_FAILED(mrRet)) {
		pr_info("[AVIN][hdmi_dmx]%s dmx_parse_off fail\r\n", __func__);
		return 0;
	}

	m_u4State = ST_DMX_STOP;

	pr_info("[AVIN][hdmi_dmx]%s leave\r\n", __func__);
	return 0;
}

void *GetAudioOutputBuf(int *pu4BufSz, int u4TimeWait)
{
	ESM_IO_BUF_INFO *pEsmBufInfo = NULL;

	if (NULL == pu4BufSz) {
		return NULL;
	}

	pEsmBufInfo = &mAudioEsmBufInfo;
	memset(pEsmBufInfo, 0 , sizeof(ESM_IO_BUF_INFO));

	pEsmBufInfo->u4TimeWait = u4TimeWait;

	if (!dmx_stm_getau((u32)m_hStream, (void *)pEsmBufInfo)) {
		*pu4BufSz = 0;
		/*pr_info("[AVIN][hdmi_dmx]%s hdmi GetAudioOutputBufleave fail\n", __func__);*/
		return NULL;
	}

	*pu4BufSz = sizeof(ESM_IO_BUF_INFO);
	return pEsmBufInfo;
}

BOOL ReleaseAudioOutputBuf(void *pvIOBuf)
{
	BOOL bRet = false;

	bRet = dmx_stm_releaseau((u32)m_hStream, pvIOBuf);

	return bRet;
}





