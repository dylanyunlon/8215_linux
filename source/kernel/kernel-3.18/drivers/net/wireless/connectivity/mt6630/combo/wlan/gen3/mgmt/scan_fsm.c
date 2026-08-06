/*
* Copyright (C) 2016 MediaTek Inc.
*
* This program is free software: you can redistribute it and/or modify it under the terms of the
* GNU General Public License version 2 as published by the Free Software Foundation.
*
* This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
* without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
* See the GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License along with this program.
* If not, see <http://www.gnu.org/licenses/>.
*/

/*
 * Id: /Department/DaVinci/BRANCHES/MT6620_WIFI_DRIVER_V2_3/mgmt/scan_fsm.c#2
 */

/*
 * ! \file   "scan_fsm.c"
 *  \brief  This file defines the state transition function for SCAN FSM.
 *
 *   The SCAN FSM is part of SCAN MODULE and responsible for performing basic SCAN
 *   behavior as metioned in IEEE 802.11 2007 11.1.3.1 & 11.1.3.2 .
 */

/*******************************************************************************
*                         C O M P I L E R   F L A G S
********************************************************************************
*/

/*******************************************************************************
*                    E X T E R N A L   R E F E R E N C E S
********************************************************************************
*/
#include "precomp.h"

#define CURRENT_PSCN_VERSION 1U
#define RSSI_MARGIN_DEFAULT  5
#define MAX_PERIOD 200000U

/*******************************************************************************
*                              C O N S T A N T S
********************************************************************************
*/

/*******************************************************************************
*                             D A T A   T Y P E S
********************************************************************************
*/

/*******************************************************************************
*                            P U B L I C   D A T A
********************************************************************************
*/

/*******************************************************************************
*                           P R I V A T E   D A T A
********************************************************************************
*/
#if DBG
/*lint -save -e64 Type mismatch */
static PUINT_8 apucDebugScanState[SCAN_STATE_NUM] = {
	(PUINT_8) DISP_STRING("SCAN_STATE_IDLE"),
	(PUINT_8) DISP_STRING("SCAN_STATE_SCANNING"),
};

/*lint -restore */
#endif /* DBG */

/*******************************************************************************
*                                 M A C R O S
********************************************************************************
*/

/*******************************************************************************
*                   F U N C T I O N   D E C L A R A T I O N S
********************************************************************************
*/

/*******************************************************************************
*                              F U N C T I O N S
********************************************************************************
*/
/*----------------------------------------------------------------------------*/
/*!
* \brief
*
* \param[in]
*
* \return none
*/
/*----------------------------------------------------------------------------*/
VOID scnFsmSteps(IN P_ADAPTER_T prAdapter, IN ENUM_SCAN_STATE_T eNextState)
{
	P_SCAN_INFO_T prScanInfo;
	P_SCAN_PARAM_T prScanParam;
	P_MSG_HDR_T prMsgHdr;

	BOOLEAN fgIsTransition = (BOOLEAN) FALSE;

	prScanInfo = &(prAdapter->rWifiVar.rScanInfo);
	prScanParam = &prScanInfo->rScanParam;

	do {

#if DBG
		DBGLOG(SCN, STATE, "TRANSITION: [%s] -> [%s]\n",
		       apucDebugScanState[prScanInfo->eCurrentState], apucDebugScanState[eNextState]);
#else
		DBGLOG(SCN, STATE, "[%d] TRANSITION: [%d] -> [%d]\n",
		       DBG_SCN_IDX, prScanInfo->eCurrentState, eNextState);
#endif

		/* NOTE(Kevin): This is the only place to change the eCurrentState(except initial) */
		prScanInfo->eCurrentState = eNextState;

		fgIsTransition = FALSE;

		switch (prScanInfo->eCurrentState) {
		case SCAN_STATE_IDLE:
			/* check for pending scanning requests */
			if (cnmChUtilIsRunning(prAdapter) == FALSE &&
			!LINK_IS_EMPTY(&(prScanInfo->rPendingMsgList))) {
				/* load next message from pending list as scan parameters */
				LINK_REMOVE_HEAD(&(prScanInfo->rPendingMsgList), prMsgHdr, P_MSG_HDR_T);

				if ((prMsgHdr->eMsgId ==
					MID_AIS_SCN_SCAN_REQ) ||
					(prMsgHdr->eMsgId ==
					MID_BOW_SCN_SCAN_REQ) ||
					(prMsgHdr->eMsgId ==
					MID_P2P_SCN_SCAN_REQ) ||
					(prMsgHdr->eMsgId ==
					MID_RLM_SCN_SCAN_REQ)) {
					scnFsmHandleScanMsg(prAdapter, (P_MSG_SCN_SCAN_REQ) prMsgHdr);

					eNextState = SCAN_STATE_SCANNING;
					fgIsTransition = TRUE;
				} else if ((prMsgHdr->eMsgId ==
					MID_AIS_SCN_SCAN_REQ_V2) ||
					(prMsgHdr->eMsgId ==
					MID_BOW_SCN_SCAN_REQ_V2) ||
					(prMsgHdr->eMsgId ==
					MID_P2P_SCN_SCAN_REQ_V2) ||
					(prMsgHdr->eMsgId ==
					MID_RLM_SCN_SCAN_REQ_V2)) {
					scnFsmHandleScanMsgV2(prAdapter, (P_MSG_SCN_SCAN_REQ_V2) prMsgHdr);

					eNextState = SCAN_STATE_SCANNING;
					fgIsTransition = TRUE;
				} else {
					ASSERT(NULL);
				}

				/* switch to next state */
				cnmMemFree(prAdapter, prMsgHdr);
			}
			break;

		case SCAN_STATE_SCANNING:
			prScanInfo->u4ScanUpdateIdx++;
			if (prScanParam->fgIsScanV2 == FALSE)
				scnSendScanReq(prAdapter);
			else
				scnSendScanReqV2(prAdapter);
			cnmTimerStartTimer(prAdapter, &prScanInfo->rScanDoneTimer,
					   SEC_TO_MSEC(AIS_SCN_DONE_TIMEOUT_SEC));
			/* prScanInfo->ucScanDoneTimeoutCnt = 0; */
			break;

		default:
			ASSERT(NULL);
			break;

		}
	} while (fgIsTransition == TRUE);

}

/*----------------------------------------------------------------------------*/
/*!
* \brief        Generate CMD_ID_SCAN_REQ command
*
* \param[in]
*
* \return none
*/
/*----------------------------------------------------------------------------*/
VOID scnSendScanReq(IN P_ADAPTER_T prAdapter)
{
	P_SCAN_INFO_T prScanInfo;
	P_SCAN_PARAM_T prScanParam;
	/* CMD_SCAN_REQ rCmdScanReq; */
	P_CMD_SCAN_REQ prCmdScanReq;
	UINT_32 i;
	WLAN_STATUS rStatus;

	ASSERT(prAdapter);

	prScanInfo = &(prAdapter->rWifiVar.rScanInfo);
	prScanParam = &prScanInfo->rScanParam;

	prCmdScanReq = kalMemAlloc(sizeof(CMD_SCAN_REQ), VIR_MEM_TYPE);
	if (prCmdScanReq == NULL) {
		DBGLOG(SCN, ERROR, "alloc CmdScanReq V1 fail\n");
		return;
	}
	/* send command packet for scan */
	kalMemZero(prCmdScanReq, sizeof(CMD_SCAN_REQ));

	prCmdScanReq->ucSeqNum = prScanParam->ucSeqNum;
	prCmdScanReq->ucBssIndex = prScanParam->ucBssIndex;
	prCmdScanReq->ucScanType = (UINT_8) prScanParam->eScanType;
	prCmdScanReq->ucSSIDType = prScanParam->ucSSIDType;

	if (prScanParam->ucSSIDNum == 1U) {
		COPY_SSID(prCmdScanReq->aucSSID,
			  prCmdScanReq->ucSSIDLength,
			  prScanParam->aucSpecifiedSSID[0],
			  prScanParam->ucSpecifiedSSIDLen[0]);
	}

	prCmdScanReq->ucChannelType = (UINT_8) prScanParam->eScanChannel;

	if (prScanParam->eScanChannel == SCAN_CHANNEL_SPECIFIED) {
		/* P2P would use:
		 * 1. Specified Listen Channel of passive scan for LISTEN state.
		 * 2. Specified Listen Channel of Target Device of active scan for SEARCH state. (Target != NULL)
		 */
		prCmdScanReq->ucChannelListNum = prScanParam->ucChannelListNum;

		for (i = 0U; i < prCmdScanReq->ucChannelListNum; i++) {
			prCmdScanReq->arChannelList[i].ucBand =
				(UINT_8) prScanParam->arChnlInfoList[i].eBand;

			prCmdScanReq->arChannelList[i].ucChannelNum =
			    (UINT_8) prScanParam->arChnlInfoList[i].ucChannelNum;
		}
	}

	prCmdScanReq->u2ChannelDwellTime = prScanParam->u2ChannelDwellTime;
	prCmdScanReq->u2TimeoutValue = prScanParam->u2TimeoutValue;

	if (prScanParam->u2IELen <= MAX_IE_LENGTH)
		prCmdScanReq->u2IELen = prScanParam->u2IELen;
	else
		prCmdScanReq->u2IELen = MAX_IE_LENGTH;

	if (prScanParam->u2IELen != 0U)
		kalMemCopy(prCmdScanReq->aucIE, prScanParam->aucIE, sizeof(UINT_8) * prCmdScanReq->u2IELen);

	DBGLOG(SCN, INFO, "ScanReqV1: ScanType=%d, SSIDType=%d, Num=%d, ChannelType=%d, Num=%d",
			prCmdScanReq->ucScanType,
			prCmdScanReq->ucSSIDType,
			prScanParam->ucSSIDNum,
			prCmdScanReq->ucChannelType,
			prCmdScanReq->ucChannelListNum);

	rStatus = wlanSendSetQueryCmd(prAdapter,
			(UINT_8)CMD_ID_SCAN_REQ,
			TRUE,
			FALSE,
			FALSE,
			NULL,
			NULL,
			(UINT_32)OFFSET_OF(CMD_SCAN_REQ, aucIE) +
			(UINT_32)prCmdScanReq->u2IELen,
			(PUINT_8)prCmdScanReq, NULL, 0U);
	/* sanity check for some scan parameters */
	if (prCmdScanReq->ucScanType >= (UINT_8)SCAN_TYPE_NUM)
		kalSendAeeWarning("wlan", "wrong scan type %d", prCmdScanReq->ucScanType);
	else if (prCmdScanReq->ucChannelType >= (UINT_8)SCAN_CHANNEL_NUM)
		kalSendAeeWarning("wlan", "wrong channel type %d", prCmdScanReq->ucChannelType);
	else if (prCmdScanReq->ucChannelType !=
		(UINT_8)SCAN_CHANNEL_SPECIFIED &&
		prCmdScanReq->ucChannelListNum != 0U)
		kalSendAeeWarning("wlan",
			"channel list is not NULL but channel type is not specified");
	else if (prCmdScanReq->ucBssIndex >= MAX_BSS_INDEX)
		kalSendAeeWarning("wlan", "wrong bss index %d", prCmdScanReq->ucBssIndex);
	else if (prCmdScanReq->ucSSIDType >= BIT(4U)) /* ssid type is wrong */
		kalSendAeeWarning("wlan", "wrong ssid type %d", prCmdScanReq->ucSSIDType);
	else if (prCmdScanReq->ucSSIDLength > 32U)
		kalSendAeeWarning("wlan", "wrong ssid length %d", prCmdScanReq->ucSSIDLength);
	else
		DBGLOG(SCN, TRACE, "Unreachable Case!!!\n");

	kalMemFree(prCmdScanReq, VIR_MEM_TYPE, sizeof(CMD_SCAN_REQ));

}

/*----------------------------------------------------------------------------*/
/*!
* \brief        Generate CMD_ID_SCAN_REQ_V2 command
*
* \param[in]
*
* \return none
*/
/*----------------------------------------------------------------------------*/
VOID scnSendScanReqV2(IN P_ADAPTER_T prAdapter)
{
	P_SCAN_INFO_T prScanInfo;
	P_SCAN_PARAM_T prScanParam;
	/* CMD_SCAN_REQ_V2 rCmdScanReq; */
	P_CMD_SCAN_REQ_V2 prCmdScanReq;
	UINT_32 i;
	WLAN_STATUS rStatus;

	ASSERT(prAdapter);

	prScanInfo = &(prAdapter->rWifiVar.rScanInfo);
	prScanParam = &prScanInfo->rScanParam;

	prCmdScanReq = kalMemAlloc(sizeof(CMD_SCAN_REQ_V2), VIR_MEM_TYPE);
	if (prCmdScanReq == NULL) {
		DBGLOG(SCN, ERROR, "alloc CmdScanReq V2 fail\n");
		return;
	}
	/* send command packet for scan */
	kalMemZero(prCmdScanReq, sizeof(CMD_SCAN_REQ_V2));

	prCmdScanReq->ucSeqNum = prScanParam->ucSeqNum;
	prCmdScanReq->ucBssIndex = prScanParam->ucBssIndex;
	prCmdScanReq->ucScanType = (UINT_8) prScanParam->eScanType;
	prCmdScanReq->ucSSIDType = prScanParam->ucSSIDType;

	for (i = 0U; i < prScanParam->ucSSIDNum; i++) {
		COPY_SSID(prCmdScanReq->arSSID[i].aucSsid,
			  prCmdScanReq->arSSID[i].u4SsidLen,
			  prScanParam->aucSpecifiedSSID[i],
			  prScanParam->ucSpecifiedSSIDLen[i]);
	}

	prCmdScanReq->u2ProbeDelayTime = (UINT_8) prScanParam->u2ProbeDelayTime;
	prCmdScanReq->ucChannelType = (UINT_8) prScanParam->eScanChannel;

	if (prScanParam->eScanChannel == SCAN_CHANNEL_SPECIFIED) {
		/* P2P would use:
		 * 1. Specified Listen Channel of passive scan for LISTEN state.
		 * 2. Specified Listen Channel of Target Device of active scan for SEARCH state. (Target != NULL)
		 */
		prCmdScanReq->ucChannelListNum = prScanParam->ucChannelListNum;

		for (i = 0U; i < prCmdScanReq->ucChannelListNum; i++) {
			prCmdScanReq->arChannelList[i].ucBand =
				(UINT_8) prScanParam->arChnlInfoList[i].eBand;

			prCmdScanReq->arChannelList[i].ucChannelNum =
			    (UINT_8) prScanParam->arChnlInfoList[i].ucChannelNum;
		}
	}

	prCmdScanReq->u2ChannelDwellTime = prScanParam->u2ChannelDwellTime;
	prCmdScanReq->u2TimeoutValue = prScanParam->u2TimeoutValue;

	if (prScanParam->u2IELen <= MAX_IE_LENGTH)
		prCmdScanReq->u2IELen = prScanParam->u2IELen;
	else
		prCmdScanReq->u2IELen = MAX_IE_LENGTH;

	if (prScanParam->u2IELen != 0U)
		kalMemCopy(prCmdScanReq->aucIE, prScanParam->aucIE,
					sizeof(UINT_8) * prCmdScanReq->u2IELen);

	DBGLOG(SCN, INFO, "ScanReqV2: ScanType=%d, SSIDType=%d, Num=%d, ChannelType=%d, Num=%d",
		prCmdScanReq->ucScanType, prCmdScanReq->ucSSIDType, prScanParam->ucSSIDNum,
		prCmdScanReq->ucChannelType, prCmdScanReq->ucChannelListNum);

	rStatus = wlanSendSetQueryCmd(prAdapter,
			(UINT_8)CMD_ID_SCAN_REQ_V2,
			TRUE,
			FALSE,
			FALSE,
			NULL,
			NULL,
			(UINT_32)OFFSET_OF(CMD_SCAN_REQ_V2, aucIE) +
			(UINT_32)prCmdScanReq->u2IELen,
			(PUINT_8)prCmdScanReq, NULL, 0U);

	/* sanity check for some scan parameters */
	if (prCmdScanReq->ucScanType >= (UINT_8)SCAN_TYPE_NUM)
		kalSendAeeWarning("wlan", "wrong scan type %d", prCmdScanReq->ucScanType);
	else if (prCmdScanReq->ucChannelType >= (UINT_8)SCAN_CHANNEL_NUM)
		kalSendAeeWarning("wlan", "wrong channel type %d", prCmdScanReq->ucChannelType);
	else if (prCmdScanReq->ucChannelType !=
		(UINT_8)SCAN_CHANNEL_SPECIFIED &&
		prCmdScanReq->ucChannelListNum != 0U)
		kalSendAeeWarning("wlan",
			"channel list is not NULL but channel type is not specified");
	else if (prCmdScanReq->ucBssIndex > MAX_BSS_INDEX)
		kalSendAeeWarning("wlan", "wrong bss index %d", prCmdScanReq->ucBssIndex);
	else if (prCmdScanReq->ucSSIDType >= BIT(4U)) /* ssid type is wrong */
		kalSendAeeWarning("wlan", "wrong ssid type %d", prCmdScanReq->ucSSIDType);
	else
		DBGLOG(SCN, TRACE, "Unreachable Case!!!\n");

	kalMemFree(prCmdScanReq, VIR_MEM_TYPE, sizeof(CMD_SCAN_REQ_V2));

}

/*----------------------------------------------------------------------------*/
/*!
* \brief
*
* \param[in]
*
* \return none
*/
/*----------------------------------------------------------------------------*/
VOID scnFsmMsgStart(IN P_ADAPTER_T prAdapter, IN P_MSG_HDR_T prMsgHdr)
{
	P_SCAN_INFO_T prScanInfo;
	P_SCAN_PARAM_T prScanParam;

	ASSERT(prMsgHdr);

	prScanInfo = &(prAdapter->rWifiVar.rScanInfo);
	prScanParam = &prScanInfo->rScanParam;

	if (prMsgHdr->eMsgId == MID_MNY_CNM_SCAN_CONTINUE) {
		cnmMemFree(prAdapter, prMsgHdr);
		scnFsmSteps(prAdapter, SCAN_STATE_IDLE);
	} else if ((prScanInfo->eCurrentState == SCAN_STATE_IDLE) &&
			(cnmChUtilIsRunning(prAdapter) == FALSE)) {
		if ((prMsgHdr->eMsgId == MID_AIS_SCN_SCAN_REQ) ||
			(prMsgHdr->eMsgId == MID_BOW_SCN_SCAN_REQ) ||
			(prMsgHdr->eMsgId == MID_P2P_SCN_SCAN_REQ) ||
			(prMsgHdr->eMsgId == MID_RLM_SCN_SCAN_REQ)) {
			scnFsmHandleScanMsg(prAdapter, (P_MSG_SCN_SCAN_REQ) prMsgHdr);
		} else if ((prMsgHdr->eMsgId == MID_AIS_SCN_SCAN_REQ_V2) ||
		(prMsgHdr->eMsgId == MID_BOW_SCN_SCAN_REQ_V2) ||
		(prMsgHdr->eMsgId == MID_P2P_SCN_SCAN_REQ_V2) ||
		(prMsgHdr->eMsgId == MID_RLM_SCN_SCAN_REQ_V2)) {
			scnFsmHandleScanMsgV2(prAdapter, (P_MSG_SCN_SCAN_REQ_V2) prMsgHdr);
		} else {
			/* should not deliver to this function */
			ASSERT(NULL);
		}

		cnmMemFree(prAdapter, prMsgHdr);
		scnFsmSteps(prAdapter, SCAN_STATE_SCANNING);
	} else {
		LINK_INSERT_TAIL(&prScanInfo->rPendingMsgList, &prMsgHdr->rLinkEntry);
	}

}

/*----------------------------------------------------------------------------*/
/*!
* \brief
*
* \param[in]
*
* \return none
*/
/*----------------------------------------------------------------------------*/
VOID scnFsmMsgAbort(IN P_ADAPTER_T prAdapter, IN P_MSG_HDR_T prMsgHdr)
{
	P_MSG_SCN_SCAN_CANCEL prScanCancel;
	P_SCAN_INFO_T prScanInfo;
	P_SCAN_PARAM_T prScanParam;
	CMD_SCAN_CANCEL rCmdScanCancel;
	WLAN_STATUS rStatus = WLAN_STATUS_SUCCESS;

	ASSERT(prMsgHdr);

	prScanCancel = (P_MSG_SCN_SCAN_CANCEL) prMsgHdr;
	prScanInfo = &(prAdapter->rWifiVar.rScanInfo);
	prScanParam = &prScanInfo->rScanParam;

	if (prScanInfo->eCurrentState != SCAN_STATE_IDLE) {
		if (prScanCancel->ucSeqNum == prScanParam->ucSeqNum &&
		    prScanCancel->ucBssIndex == prScanParam->ucBssIndex) {
			/* send cancel message to firmware domain */
			rCmdScanCancel.ucSeqNum = prScanParam->ucSeqNum;
			rCmdScanCancel.ucIsExtChannel = (UINT_8) prScanCancel->fgIsChannelExt;

			rStatus = wlanSendSetQueryCmd(prAdapter,
					(UINT_8)CMD_ID_SCAN_CANCEL,
					TRUE,
					FALSE,
					FALSE,
					NULL, NULL,
					(UINT_32)sizeof(CMD_SCAN_CANCEL),
					(PUINT_8)&rCmdScanCancel, NULL, 0U);

			/* generate scan-done event for caller */
			scnFsmGenerateScanDoneMsg(prAdapter,
						  prScanParam->ucSeqNum,
						  prScanParam->ucBssIndex,
						  SCAN_STATUS_CANCELLED);

			/* switch to next pending scan */
			// Autochips delete it
			//scnFsmSteps(prAdapter, SCAN_STATE_IDLE);
		} else
			scnFsmRemovePendingMsg(prAdapter, prScanCancel->ucSeqNum, prScanCancel->ucBssIndex);
		/* switch to next pending scan */
		scnFsmSteps(prAdapter, SCAN_STATE_IDLE);
	}

	cnmMemFree(prAdapter, prMsgHdr);

}

/*----------------------------------------------------------------------------*/
/*!
* \brief            Scan Message Parsing (Legacy)
*
* \param[in]
*
* \return none
*/
/*----------------------------------------------------------------------------*/
VOID scnFsmHandleScanMsg(IN P_ADAPTER_T prAdapter, IN P_MSG_SCN_SCAN_REQ prScanReqMsg)
{
	P_SCAN_INFO_T prScanInfo;
	P_SCAN_PARAM_T prScanParam;
	UINT_32 i;

	ASSERT(prAdapter);
	ASSERT(prScanReqMsg);

	prScanInfo = &(prAdapter->rWifiVar.rScanInfo);
	prScanParam = &prScanInfo->rScanParam;

	prScanParam->eScanType = prScanReqMsg->eScanType;
	prScanParam->ucBssIndex = prScanReqMsg->ucBssIndex;
	prScanParam->ucSSIDType = prScanReqMsg->ucSSIDType;
	if ((prScanParam->ucSSIDType & (SCAN_REQ_SSID_SPECIFIED |
		SCAN_REQ_SSID_P2P_WILDCARD)) != 0U) {
		prScanParam->ucSSIDNum = 1U;

		COPY_SSID(prScanParam->aucSpecifiedSSID[0],
			  prScanParam->ucSpecifiedSSIDLen[0], prScanReqMsg->aucSSID, prScanReqMsg->ucSSIDLength);

		/* reset SSID length to zero for rest array entries */
		for (i = 1U; i < SCN_SSID_MAX_NUM; i++)
			prScanParam->ucSpecifiedSSIDLen[i] = 0U;
	} else {
		prScanParam->ucSSIDNum = 0;

		for (i = 0U; i < SCN_SSID_MAX_NUM; i++)
			prScanParam->ucSpecifiedSSIDLen[i] = 0U;
	}

	prScanParam->u2ProbeDelayTime = 0U;
	prScanParam->eScanChannel = prScanReqMsg->eScanChannel;
	if (prScanParam->eScanChannel == SCAN_CHANNEL_SPECIFIED) {
		if (prScanReqMsg->ucChannelListNum <= MAXIMUM_OPERATION_CHANNEL_LIST)
			prScanParam->ucChannelListNum = prScanReqMsg->ucChannelListNum;
		else
			prScanParam->ucChannelListNum = MAXIMUM_OPERATION_CHANNEL_LIST;

		kalMemCopy(prScanParam->arChnlInfoList,
					prScanReqMsg->arChnlInfoList,
					sizeof(RF_CHANNEL_INFO_T) *
					prScanParam->ucChannelListNum);
	}

	if (prScanReqMsg->u2IELen <= MAX_IE_LENGTH)
		prScanParam->u2IELen = prScanReqMsg->u2IELen;
	else
		prScanParam->u2IELen = MAX_IE_LENGTH;

	if (prScanParam->u2IELen != 0U)
		kalMemCopy(prScanParam->aucIE, prScanReqMsg->aucIE, prScanParam->u2IELen);

	prScanParam->u2ChannelDwellTime = prScanReqMsg->u2ChannelDwellTime;
	prScanParam->u2TimeoutValue = prScanReqMsg->u2TimeoutValue;
	prScanParam->ucSeqNum = prScanReqMsg->ucSeqNum;

	if (prScanReqMsg->rMsgHdr.eMsgId == MID_RLM_SCN_SCAN_REQ)
		prScanParam->fgIsObssScan = TRUE;
	else
		prScanParam->fgIsObssScan = FALSE;

	prScanParam->fgIsScanV2 = FALSE;

}

/*----------------------------------------------------------------------------*/
/*!
* \brief            Scan Message Parsing - V2 with multiple SSID support
*
* \param[in]
*
* \return none
*/
/*----------------------------------------------------------------------------*/
VOID scnFsmHandleScanMsgV2(IN P_ADAPTER_T prAdapter, IN P_MSG_SCN_SCAN_REQ_V2 prScanReqMsg)
{
	P_SCAN_INFO_T prScanInfo;
	P_SCAN_PARAM_T prScanParam;
	UINT_32 i;

	ASSERT(prAdapter);
	ASSERT(prScanReqMsg);
	ASSERT_BOOLEAN(prScanReqMsg->ucSSIDNum <= SCN_SSID_MAX_NUM);

	prScanInfo = &(prAdapter->rWifiVar.rScanInfo);
	prScanParam = &prScanInfo->rScanParam;

	prScanParam->eScanType = prScanReqMsg->eScanType;
	prScanParam->ucBssIndex = prScanReqMsg->ucBssIndex;
	prScanParam->ucSSIDType = prScanReqMsg->ucSSIDType;
	prScanParam->ucSSIDNum = prScanReqMsg->ucSSIDNum;

	for (i = 0U; i < prScanReqMsg->ucSSIDNum; i++) {
		COPY_SSID(prScanParam->aucSpecifiedSSID[i],
			  prScanParam->ucSpecifiedSSIDLen[i],
			  prScanReqMsg->prSsid[i].aucSsid,
			  (UINT_8)prScanReqMsg->prSsid[i].u4SsidLen);
	}

	prScanParam->u2ProbeDelayTime = prScanReqMsg->u2ProbeDelay;
	prScanParam->eScanChannel = prScanReqMsg->eScanChannel;
	if (prScanParam->eScanChannel == SCAN_CHANNEL_SPECIFIED) {
		if (prScanReqMsg->ucChannelListNum <= MAXIMUM_OPERATION_CHANNEL_LIST)
			prScanParam->ucChannelListNum = prScanReqMsg->ucChannelListNum;
		else
			prScanParam->ucChannelListNum = MAXIMUM_OPERATION_CHANNEL_LIST;

		kalMemCopy(prScanParam->arChnlInfoList,
					prScanReqMsg->arChnlInfoList,
					sizeof(RF_CHANNEL_INFO_T) *
					prScanParam->ucChannelListNum);
	}

	if (prScanReqMsg->u2IELen <= MAX_IE_LENGTH)
		prScanParam->u2IELen = prScanReqMsg->u2IELen;
	else
		prScanParam->u2IELen = MAX_IE_LENGTH;

	if (prScanParam->u2IELen != 0U)
		kalMemCopy(prScanParam->aucIE, prScanReqMsg->aucIE, prScanParam->u2IELen);

	prScanParam->u2ChannelDwellTime = prScanReqMsg->u2ChannelDwellTime;
	prScanParam->u2TimeoutValue = prScanReqMsg->u2TimeoutValue;
	prScanParam->ucSeqNum = prScanReqMsg->ucSeqNum;

	if (prScanReqMsg->rMsgHdr.eMsgId == MID_RLM_SCN_SCAN_REQ)
		prScanParam->fgIsObssScan = TRUE;
	else
		prScanParam->fgIsObssScan = FALSE;

	prScanParam->fgIsScanV2 = TRUE;

}

/*----------------------------------------------------------------------------*/
/*!
* \brief            Remove pending scan request
*
* \param[in]
*
* \return none
*/
/*----------------------------------------------------------------------------*/
VOID scnFsmRemovePendingMsg(IN P_ADAPTER_T prAdapter, IN UINT_8 ucSeqNum, IN UINT_8 ucBssIndex)
{
	P_SCAN_INFO_T prScanInfo;
	P_SCAN_PARAM_T prScanParam;
	P_MSG_HDR_T prPendingMsgHdr, prPendingMsgHdrNext, prRemoveMsgHdr = NULL;
	P_LINK_ENTRY_T prRemoveLinkEntry = NULL;
	BOOLEAN fgIsRemovingScan = FALSE;

	ASSERT(prAdapter);

	prScanInfo = &(prAdapter->rWifiVar.rScanInfo);
	prScanParam = &prScanInfo->rScanParam;

	/* traverse through rPendingMsgList for removal */
	LINK_FOR_EACH_ENTRY_SAFE(prPendingMsgHdr,
				 prPendingMsgHdrNext, &(prScanInfo->rPendingMsgList), rLinkEntry, MSG_HDR_T) {
		if ((prPendingMsgHdr->eMsgId == MID_AIS_SCN_SCAN_REQ) ||
			(prPendingMsgHdr->eMsgId == MID_BOW_SCN_SCAN_REQ) ||
			(prPendingMsgHdr->eMsgId == MID_P2P_SCN_SCAN_REQ) ||
			(prPendingMsgHdr->eMsgId == MID_RLM_SCN_SCAN_REQ)) {
			P_MSG_SCN_SCAN_REQ prScanReqMsg =
				(P_MSG_SCN_SCAN_REQ)prPendingMsgHdr;

			if ((ucSeqNum == prScanReqMsg->ucSeqNum) &&
				(ucBssIndex == prScanReqMsg->ucBssIndex)) {
				prRemoveLinkEntry = &(prScanReqMsg->rMsgHdr.rLinkEntry);
				prRemoveMsgHdr = prPendingMsgHdr;
				fgIsRemovingScan = TRUE;
			}
		} else if ((prPendingMsgHdr->eMsgId ==
			MID_AIS_SCN_SCAN_REQ_V2) ||
			(prPendingMsgHdr->eMsgId == MID_BOW_SCN_SCAN_REQ_V2) ||
			(prPendingMsgHdr->eMsgId == MID_P2P_SCN_SCAN_REQ_V2) ||
			(prPendingMsgHdr->eMsgId == MID_RLM_SCN_SCAN_REQ_V2)) {
			P_MSG_SCN_SCAN_REQ_V2 prScanReqMsgV2 =
				(P_MSG_SCN_SCAN_REQ_V2)prPendingMsgHdr;

			if ((ucSeqNum == prScanReqMsgV2->ucSeqNum) &&
				(ucBssIndex == prScanReqMsgV2->ucBssIndex)) {
				prRemoveLinkEntry = &(prScanReqMsgV2->rMsgHdr.rLinkEntry);
				prRemoveMsgHdr = prPendingMsgHdr;
				fgIsRemovingScan = TRUE;
			}
		} else
			fgIsRemovingScan = FALSE;

		if (prRemoveLinkEntry != NULL) {
			scnFsmGenerateScanDoneMsg(prAdapter, ucSeqNum,
				ucBssIndex, SCAN_STATUS_CANCELLED);

			/* remove from pending list */
			LINK_REMOVE_KNOWN_ENTRY(&(prScanInfo->rPendingMsgList), prRemoveLinkEntry);
			cnmMemFree(prAdapter, prRemoveMsgHdr);

			break;
		}
	}

}

/*----------------------------------------------------------------------------*/
/*!
* \brief
*
* \param[in]
*
* \return none
*/
/*----------------------------------------------------------------------------*/
VOID scnEventScanDone(IN P_ADAPTER_T prAdapter, IN P_EVENT_SCAN_DONE prScanDone, BOOLEAN fgIsNewVersion)
{
	P_SCAN_INFO_T prScanInfo;
	P_SCAN_PARAM_T prScanParam;

	prScanInfo = &(prAdapter->rWifiVar.rScanInfo);
	prScanParam = &prScanInfo->rScanParam;
	cnmTimerStopTimer(prAdapter, &prScanInfo->rScanDoneTimer);
	prScanInfo->ucScanDoneTimeoutCnt = 0U;
	if (fgIsNewVersion == TRUE) {
		DBGLOG(SCN, INFO,
		       "New scnEventScanDone Version%d! ChanCount=%d,CurState=%d, PNO=%d\n",
		       prScanDone->ucScanDoneVersion, prScanDone->ucCompleteChanCount,
		       prScanDone->ucCurrentState, prScanDone->fgIsPNOenabled);
		if (prScanDone->ucScanDoneVersion > 2U)
			DBGLOG(SCN, INFO, "New u4ScanDurBcnCnt[%u]!!! fgIsPNOenabled[%d]\n",
					prScanDone->u4ScanDurBcnCnt, prScanDone->fgIsPNOenabled);
	} else
		DBGLOG(SCN, INFO, "Old scnEventScanDone Version\n");

	/* buffer empty channel information */
	if ((prScanParam->eScanChannel == SCAN_CHANNEL_FULL) ||
		(prScanParam->eScanChannel == SCAN_CHANNEL_2G4)) {
		if (prScanDone->ucSparseChannelValid != 0U) {
			prScanInfo->fgIsSparseChannelValid = TRUE;
			prScanInfo->rSparseChannel.eBand =
				(ENUM_BAND_T)prScanDone->rSparseChannel.ucBand;
			prScanInfo->rSparseChannel.ucChannelNum = prScanDone->rSparseChannel.ucChannelNum;
		} else {
			prScanInfo->fgIsSparseChannelValid = FALSE;
		}
	}

	if ((prScanInfo->eCurrentState == SCAN_STATE_SCANNING) &&
		(prScanDone->ucSeqNum == prScanParam->ucSeqNum)) {
		/* generate scan-done event for caller */
		scnFsmGenerateScanDoneMsg(prAdapter, prScanParam->ucSeqNum,
			prScanParam->ucBssIndex, SCAN_STATUS_DONE);

		/* switch to next pending scan */
		scnFsmSteps(prAdapter, SCAN_STATE_IDLE);
	} else {
		DBGLOG(SCN, WARN, "Unexpected SCAN-DONE event: SeqNum = %d, Current State = %d\n",
		       prScanDone->ucSeqNum, prScanInfo->eCurrentState);
	}

}				/* end of scnEventScanDone */

/*----------------------------------------------------------------------------*/
/*!
* \brief
*
* \param[in]
*
* \return none
*/
/*----------------------------------------------------------------------------*/
VOID
scnFsmGenerateScanDoneMsg(IN P_ADAPTER_T prAdapter,
			  IN UINT_8 ucSeqNum, IN UINT_8 ucBssIndex, IN ENUM_SCAN_STATUS eScanStatus)
{
	P_SCAN_INFO_T prScanInfo;
	P_SCAN_PARAM_T prScanParam;
	P_MSG_SCN_SCAN_DONE prScanDoneMsg;

	ASSERT(prAdapter);

	prScanInfo = &(prAdapter->rWifiVar.rScanInfo);
	prScanParam = &prScanInfo->rScanParam;

	prScanDoneMsg = (P_MSG_SCN_SCAN_DONE)cnmMemAlloc(prAdapter,
			RAM_TYPE_MSG, (UINT_32)sizeof(MSG_SCN_SCAN_DONE));
	if (prScanDoneMsg == NULL) {
		ASSERT(NULL);	/* Can't indicate SCAN FSM Complete */
		return;
	}

	if (prScanParam->fgIsObssScan == TRUE) {
		prScanDoneMsg->rMsgHdr.eMsgId = MID_SCN_RLM_SCAN_DONE;
	} else {
		switch (GET_BSS_INFO_BY_INDEX(prAdapter, ucBssIndex)->eNetworkType) {
		case NETWORK_TYPE_AIS:
			prScanDoneMsg->rMsgHdr.eMsgId = MID_SCN_AIS_SCAN_DONE;
			break;

		case NETWORK_TYPE_P2P:
			prScanDoneMsg->rMsgHdr.eMsgId = MID_SCN_P2P_SCAN_DONE;
			break;

		case NETWORK_TYPE_BOW:
			prScanDoneMsg->rMsgHdr.eMsgId = MID_SCN_BOW_SCAN_DONE;
			break;

		default:
			DBGLOG(SCN, LOUD,
			       "Unexpected Network Type: %d\n",
			       GET_BSS_INFO_BY_INDEX(prAdapter, ucBssIndex)->eNetworkType);
			ASSERT(NULL);
			break;
		}
	}

	prScanDoneMsg->ucSeqNum = ucSeqNum;
	prScanDoneMsg->ucBssIndex = ucBssIndex;
	prScanDoneMsg->eScanStatus = eScanStatus;

	mboxSendMsg(prAdapter, MBOX_ID_0,
		(P_MSG_HDR_T)prScanDoneMsg, MSG_SEND_METHOD_BUF);

}				/* end of scnFsmGenerateScanDoneMsg() */

/*----------------------------------------------------------------------------*/
/*!
* \brief        Query for most sparse channel
*
* \param[in]
*
* \return none
*/
/*----------------------------------------------------------------------------*/
BOOLEAN scnQuerySparseChannel(IN P_ADAPTER_T prAdapter, P_ENUM_BAND_T prSparseBand, PUINT_8 pucSparseChannel)
{
	P_SCAN_INFO_T prScanInfo;

	ASSERT(prAdapter);

	prScanInfo = &(prAdapter->rWifiVar.rScanInfo);

	if (prScanInfo->fgIsSparseChannelValid == TRUE) {
		if (prSparseBand != NULL)
			*prSparseBand = prScanInfo->rSparseChannel.eBand;

		if (pucSparseChannel != NULL)
			*pucSparseChannel = prScanInfo->rSparseChannel.ucChannelNum;

		return TRUE;
	} else
		return FALSE;
}

/*----------------------------------------------------------------------------*/
/*!
* \brief        Event handler for NLO done event
*
* \param[in]
*
* \return none
*/
/*----------------------------------------------------------------------------*/
VOID scnEventNloDone(IN P_ADAPTER_T prAdapter, IN P_EVENT_NLO_DONE_T prNloDone)
{
	P_SCAN_INFO_T prScanInfo;
	P_NLO_PARAM_T prNloParam;

	prScanInfo = &(prAdapter->rWifiVar.rScanInfo);
	prNloParam = &prScanInfo->rNloParam;

	if ((prScanInfo->fgNloScanning == TRUE) &&
		(prNloDone->ucSeqNum == prNloParam->ucSeqNum)) {

		DBGLOG(SCN, INFO, "scnEventNloDone reporting to upper layer\n");

		kalSchedScanResults(prAdapter->prGlueInfo);

		if (prNloParam->fgStopAfterIndication == TRUE)
			prScanInfo->fgNloScanning = FALSE;

		kalMemZero(&prNloParam->aprPendingBssDescToInd[0],
			CFG_SCAN_SSID_MATCH_MAX_NUM *
			(UINT_32)sizeof(P_BSS_DESC_T));
	} else {
		DBGLOG(SCN, INFO, "Unexpected NLO-DONE event: SeqNum = %d, Current State = %d\n",
		       prNloDone->ucSeqNum, prScanInfo->eCurrentState);
	}

}

/*----------------------------------------------------------------------------*/
/*!
* \brief         handler for starting scheduled scan
*
* \param[in]
*
* \return none
*/
/*----------------------------------------------------------------------------*/
BOOLEAN
scnFsmSchedScanRequest(IN P_ADAPTER_T prAdapter,
		       IN P_PARAM_SCHED_SCAN_REQUEST prSchedScanRequest)
{
	P_SCAN_INFO_T prScanInfo;
	P_NLO_PARAM_T prNloParam;
	P_CMD_NLO_REQ prCmdNloReq;
	UINT_32 i;
	BOOLEAN ret = TRUE;
	struct NLO_SSID_MATCH_SETS *prNloSsidMatch = NULL;
	ENUM_BAND_T ePreferedChnl = BAND_NULL;
#if CFG_SUPPORT_SCHED_SCN_SSID_SETS
	UINT_32 j = 0U;
	UINT_8 ucNetworkIndex = 0U;
	BOOLEAN fgIsHiddenSSID = FALSE;
#endif
#if CFG_SUPPORT_SCN_PSCN
	BOOLEAN fgResult;
#endif

	ASSERT(prAdapter);

	prScanInfo = &(prAdapter->rWifiVar.rScanInfo);
	prNloParam = &prScanInfo->rNloParam;

	/* ASSERT(prScanInfo->fgNloScanning == FALSE); */
	if (prScanInfo->fgNloScanning == TRUE) {
		DBGLOG(SCN, WARN, "prScanInfo->fgNloScanning == FALSE  already scanning\n");
		return FALSE;
	}

	prScanInfo->fgNloScanning = TRUE;

	/* 1. load parameters */
	prNloParam->ucSeqNum++;
	prNloParam->ucBssIndex = prAdapter->prAisBssInfo->ucBssIndex;
	prNloParam->fgStopAfterIndication = FALSE;
	prNloParam->ucFastScanIteration = 1U;

	if (prSchedScanRequest->u2ScanInterval < SCAN_NLO_DEFAULT_INTERVAL) {
		prSchedScanRequest->u2ScanInterval = SCAN_NLO_DEFAULT_INTERVAL; /* millisecond */
		DBGLOG(SCN, TRACE, "force interval to SCAN_NLO_DEFAULT_INTERVAL\n");
	}
	prAdapter->prAisBssInfo->fgIsPNOEnable = TRUE;
#if !CFG_SUPPORT_SCN_PSCN
	if (!IS_NET_ACTIVE(prAdapter, prAdapter->prAisBssInfo->ucBssIndex)) {
		SET_NET_ACTIVE(prAdapter, prAdapter->prAisBssInfo->ucBssIndex);

		DBGLOG(SCN, INFO, "ACTIVATE AIS from INACTIVE to enable PNO\n");
		/* sync with firmware */
		nicActivateNetwork(prAdapter, prAdapter->prAisBssInfo->ucBssIndex);
	}
#endif
	prNloParam->u2FastScanPeriod = SCAN_NLO_MIN_INTERVAL; /* use second instead of millisecond for UINT_16*/
	prNloParam->u2SlowScanPeriod = SCAN_NLO_MAX_INTERVAL;

#if CFG_SUPPORT_SCHED_SCN_SSID_SETS
	if (prSchedScanRequest->u4MatchSsidNum > CFG_SCAN_SSID_MATCH_MAX_NUM)
		prNloParam->ucMatchSSIDNum = CFG_SCAN_SSID_MATCH_MAX_NUM;
	else
		prNloParam->ucMatchSSIDNum = (UINT_8)prSchedScanRequest->u4MatchSsidNum;
#else
	if (prSchedScanRequest->u4SsidNum > CFG_SCAN_SSID_MATCH_MAX_NUM)
		prNloParam->ucMatchSSIDNum = CFG_SCAN_SSID_MATCH_MAX_NUM;
	else
		prNloParam->ucMatchSSIDNum = (UINT_8)prSchedScanRequest->u4SsidNum;
#endif

	prNloSsidMatch = prNloParam->rNLONetwork.arMatchSets;
	kalMemZero(prNloSsidMatch, sizeof(prNloParam->rNLONetwork.arMatchSets));

#if CFG_SUPPORT_SCHED_SCN_SSID_SETS
	if (prSchedScanRequest->u4SsidNum > CFG_SCAN_HIDDEN_SSID_MAX_NUM)
		prNloParam->ucSSIDNum = CFG_SCAN_HIDDEN_SSID_MAX_NUM;
	else
		prNloParam->ucSSIDNum = (UINT_8)prSchedScanRequest->u4SsidNum;

	for (i = 0U; i < prNloParam->ucSSIDNum; i++) {
		COPY_SSID(prNloSsidMatch[ucNetworkIndex].aucSSID,
			  prNloSsidMatch[ucNetworkIndex].ucSSIDLength,
			  prSchedScanRequest->arSsid[i].aucSsid,
			  (UINT_8) prSchedScanRequest->arSsid[i].u4SsidLen);
		DBGLOG(SCN, TRACE, "ssid set(%d) %s\n",
			ucNetworkIndex, prNloSsidMatch[ucNetworkIndex].aucSSID);
		prNloSsidMatch[ucNetworkIndex].cRssiThresold = prSchedScanRequest->acRssiThresold[i];
		ucNetworkIndex++; /* ucNetworkIndex = prNloParam->ucSSIDNum */
	}
#endif

	for (i = 0U; i < prNloParam->ucMatchSSIDNum; i++) {
#if CFG_SUPPORT_SCHED_SCN_SSID_SETS
		fgIsHiddenSSID = FALSE;
		for (j = 0U; j < prNloParam->ucSSIDNum; j++) {
			if (EQUAL_SSID(prSchedScanRequest->arSsid[j].aucSsid,
			(UINT_8) prSchedScanRequest->arSsid[j].u4SsidLen,
			prSchedScanRequest->arMatchSsid[i].aucSsid,
			(UINT_8) prSchedScanRequest->arMatchSsid[i].u4SsidLen)) {
				fgIsHiddenSSID = TRUE;
				break;
			}
		}

		if (fgIsHiddenSSID == FALSE) {
			COPY_SSID(prNloSsidMatch[ucNetworkIndex].aucSSID,
				  prNloSsidMatch[ucNetworkIndex].ucSSIDLength,
				  prSchedScanRequest->arMatchSsid[i].aucSsid,
				  (UINT_8) prSchedScanRequest->arMatchSsid[i].u4SsidLen);
			DBGLOG(SCN, TRACE, "Match set(%d) hidden(%d) %s\n",
				i, fgIsHiddenSSID, prNloSsidMatch[ucNetworkIndex].aucSSID);
			prNloSsidMatch[ucNetworkIndex].cRssiThresold = prSchedScanRequest->acRssiThresold[i];
			ucNetworkIndex++;
		}
#else
		COPY_SSID(prNloSsidMatch[i].aucSSID, prNloSsidMatch[i].ucSSIDLength,
			prSchedScanRequest->arSsid[i].aucSsid,
			(UINT_8)prSchedScanRequest->arSsid[i].u4SsidLen);
		prNloSsidMatch[i].cRssiThresold = prSchedScanRequest->acRssiThresold[i];
#endif
	}
	if (prSchedScanRequest->ucChnlNum >
		(UINT_8)sizeof(prNloParam->rNLONetwork.aucChannel))
		prSchedScanRequest->ucChnlNum =
			(UINT_8)sizeof(prNloParam->rNLONetwork.aucChannel);

	ePreferedChnl = prAdapter->aePreferBand[prAdapter->prAisBssInfo->ucBssIndex];
	if (ePreferedChnl == BAND_2G4) {
		prNloParam->rNLONetwork.ucChannelType = NLO_CHANNEL_TYPE_2G4_ONLY;
		prNloParam->rNLONetwork.ucChnlNum = 0U;
	} else if (ePreferedChnl == BAND_5G) {
		prNloParam->rNLONetwork.ucChannelType = NLO_CHANNEL_TYPE_5G_ONLY;
		prNloParam->rNLONetwork.ucChnlNum = 0U;
	} else if (prSchedScanRequest->ucChnlNum > 0U) {
		prNloParam->rNLONetwork.ucChannelType = NLO_CHANNEL_TYPE_SPECIFIED;
		prNloParam->rNLONetwork.ucChnlNum = prSchedScanRequest->ucChnlNum;
		kalMemCopy(prNloParam->rNLONetwork.aucChannel, prSchedScanRequest->pucChannels,
			prSchedScanRequest->ucChnlNum);
	} else {
		prNloParam->rNLONetwork.ucChnlNum = 0U;
		prNloParam->rNLONetwork.ucChannelType = NLO_CHANNEL_TYPE_DUAL_BAND;
	}

	/* 2. prepare command for sending */
	prCmdNloReq = (P_CMD_NLO_REQ)cnmMemAlloc(prAdapter, RAM_TYPE_BUF,
			(UINT_32)sizeof(CMD_NLO_REQ) +
			prSchedScanRequest->u4IELength);
	if (prCmdNloReq == NULL) {
		ASSERT(NULL);	/* Can't initiate NLO operation */
		return FALSE;
	}

	/* 3. send command packet for NLO operation */
	kalMemZero(prCmdNloReq, sizeof(CMD_NLO_REQ));

	prCmdNloReq->ucSeqNum = prNloParam->ucSeqNum;
	prCmdNloReq->ucBssIndex = prNloParam->ucBssIndex;
	prCmdNloReq->fgStopAfterIndication = prNloParam->fgStopAfterIndication;
	prCmdNloReq->ucFastScanIteration = prNloParam->ucFastScanIteration;
	prCmdNloReq->u2FastScanPeriod = prNloParam->u2FastScanPeriod;
	prCmdNloReq->u2SlowScanPeriod = prNloParam->u2SlowScanPeriod;
	prCmdNloReq->ucEntryNum = prNloParam->ucMatchSSIDNum;

#ifdef LINUX
	prCmdNloReq->ucFlag = SCAN_NLO_CHECK_SSID_ONLY;
	DBGLOG(SCN, TRACE, "LINUX only check SSID for PNO SCAN\n");
#endif
	/* we set this bit to notify firmware that they should using the new NLO network design */
	prCmdNloReq->ucEntryNum |= (UINT_8)BIT(7U);

#if CFG_SUPPORT_SCHED_SCN_SSID_SETS
	/* ucFlag[7] enable FW's support, similar to gen2 ucReserved */
	prCmdNloReq->ucFlag |= 0x80U;
	/* ucFlag[4:6]: set SSID sets number */
	prCmdNloReq->ucFlag |= (prNloParam->ucSSIDNum & 0x07U) << 4U;
	DBGLOG(SCN, INFO,
	"SeqNum %d: chnlType %d, chnlNum %d, ssidNum %d, Flag 0x%x, MatchSSIDNum %d %d, Iteration=%d, Period=%d\n",
		prNloParam->ucSeqNum, prNloParam->rNLONetwork.ucChannelType,
		prNloParam->rNLONetwork.ucChnlNum, prNloParam->ucSSIDNum, prCmdNloReq->ucFlag,
		prNloParam->ucMatchSSIDNum, ucNetworkIndex,
		prNloParam->ucFastScanIteration, prNloParam->u2FastScanPeriod);
#else
	DBGLOG(SCN, INFO, "chnlType %d, chnlNum %d, ssidNum %d, Iteration=%d, ScanPeriod=%d\n",
		prNloParam->rNLONetwork.ucChannelType,
		prNloParam->rNLONetwork.ucChnlNum, prNloParam->ucMatchSSIDNum,
		prNloParam->ucFastScanIteration, prNloParam->u2FastScanPeriod);
#endif
	kalMemCopy(&prCmdNloReq->rNLONetwork, &prNloParam->rNLONetwork, sizeof(prNloParam->rNLONetwork));

	if (prSchedScanRequest->u4IELength <= MAX_IE_LENGTH)
		prCmdNloReq->u2IELen = (UINT_16)prSchedScanRequest->u4IELength;
	else
		prCmdNloReq->u2IELen = MAX_IE_LENGTH;

	if (prSchedScanRequest->u4IELength != 0U)
		kalMemCopy(prCmdNloReq->aucIE, prSchedScanRequest->pucIE, prCmdNloReq->u2IELen);

#if !CFG_SUPPORT_SCN_PSCN
	if (wlanSendSetQueryCmd(prAdapter,
				(UINT_8)CMD_ID_SET_NLO_REQ,
				TRUE,
				FALSE,
				FALSE,
				nicCmdEventSetCommon,
				nicOidCmdTimeoutCommon,
				(UINT_32)sizeof(CMD_NLO_REQ) +
				(UINT_32)prCmdNloReq->u2IELen,
				(PUINT_8) prCmdNloReq,
				NULL, 0U) == WLAN_STATUS_FAILURE)
		ret = FALSE;
#else
	fgResult = scnCombineParamsIntoPSCN(prAdapter, prCmdNloReq, NULL, NULL,
		NULL, FALSE, FALSE, FALSE);
	scnPSCNFsm(prAdapter, PSCN_RESET);
#endif
	cnmMemFree(prAdapter, (PVOID) prCmdNloReq);

	return ret;
}

/*----------------------------------------------------------------------------*/
/*!
* \brief         handler for stopping scheduled scan
*
* \param[in]
*
* \return none
*/
/*----------------------------------------------------------------------------*/
BOOLEAN scnFsmSchedScanStopRequest(IN P_ADAPTER_T prAdapter)
{
	P_SCAN_INFO_T prScanInfo;
	P_NLO_PARAM_T prNloParam;
	BOOLEAN fgRet = TRUE;
#if CFG_SUPPORT_SCN_PSCN
	BOOLEAN fgResult;
#else
	CMD_NLO_CANCEL rCmdNloCancel;
#endif

	ASSERT(prAdapter);

	prScanInfo = &(prAdapter->rWifiVar.rScanInfo);
	prNloParam = &prScanInfo->rNloParam;
	prAdapter->prAisBssInfo->fgIsPNOEnable = FALSE;

#if !CFG_SUPPORT_SCN_PSCN
	rCmdNloCancel.ucSeqNum = prNloParam->ucSeqNum;
	if (wlanSendSetQueryCmd(prAdapter,
				CMD_ID_SET_NLO_CANCEL,
				TRUE,
				FALSE,
				FALSE,
				nicCmdEventSetStopSchedScan,
				nicOidCmdTimeoutCommon,
				sizeof(CMD_NLO_CANCEL), (PUINT_8)&rCmdNloCancel, NULL, 0) == WLAN_STATUS_FAILURE)
		fgRet = FALSE;
#else
	fgResult = scnCombineParamsIntoPSCN(prAdapter, NULL, NULL, NULL,
		NULL, TRUE, FALSE, FALSE);
	if ((prScanInfo->prPscnParam->fgGScnEnable == TRUE) ||
		(prScanInfo->prPscnParam->fgBatchScnEnable == TRUE))
		scnPSCNFsm(prAdapter, PSCN_RESET); /* in case there is any PSCN */
	else
		scnPSCNFsm(prAdapter, PSCN_IDLE);
#endif

	prScanInfo->fgNloScanning = FALSE;

	return fgRet;
}

#if CFG_SUPPORT_SCN_PSCN
/*----------------------------------------------------------------------------*/
/*!
* \brief         handler for Set PSCN action
*
* \param[in]
*
* \return none
*/
/*----------------------------------------------------------------------------*/
BOOLEAN scnFsmPSCNAction(IN P_ADAPTER_T prAdapter, IN ENUM_PSCAN_ACT_T ucPscanAct)
{
	P_SCAN_INFO_T prScanInfo;
	CMD_SET_PSCAN_ENABLE rCmdPscnAction;
	WLAN_STATUS rStatus = WLAN_STATUS_SUCCESS;

	DBGLOG(SCN, INFO, "scnFsmPSCNAction Act = %d\n", ucPscanAct);

	kalMemZero(&rCmdPscnAction, sizeof(CMD_SET_PSCAN_ENABLE));

	ASSERT(prAdapter);
	prScanInfo = &(prAdapter->rWifiVar.rScanInfo);
	if (ucPscanAct == PSCAN_ACT_ENABLE) {
		prScanInfo->fgPscnOngoing = TRUE;
		rCmdPscnAction.ucPscanAct = 0U;
	} else {
		prScanInfo->fgPscnOngoing = FALSE;
		rCmdPscnAction.ucPscanAct = 1U;
	}

	rStatus = wlanSendSetQueryCmd(prAdapter,
				(UINT_8)CMD_ID_SET_PSCN_ENABLE,
				TRUE,
				FALSE,
				FALSE,
				nicCmdEventSetCommon,
				nicOidCmdTimeoutCommon,
				(UINT_32)sizeof(CMD_SET_PSCAN_ENABLE),
				(PUINT_8)&rCmdPscnAction, NULL, 0U);

	return TRUE;
}

/*----------------------------------------------------------------------------*/
/*!
* \brief        handler for Set PSCN param
* \param[in]
*
* \return none
*/
/*----------------------------------------------------------------------------*/
BOOLEAN scnFsmPSCNSetParam(IN P_ADAPTER_T prAdapter, IN P_CMD_SET_PSCAN_PARAM prCmdPscnParam)
{
	P_SCAN_INFO_T prScanInfo;
	WLAN_STATUS rStatus = WLAN_STATUS_SUCCESS;

	ASSERT(prAdapter);
	prScanInfo = &(prAdapter->rWifiVar.rScanInfo);

	DBGLOG(SCN, INFO, "fgNLOScnEnable=%d %d %d, basePeriod=%d\n",
	prCmdPscnParam->fgNLOScnEnable, prCmdPscnParam->fgBatchScnEnable,
	prCmdPscnParam->fgGScnEnable, prCmdPscnParam->u4BasePeriod);

	rStatus = wlanSendSetQueryCmd(prAdapter,
				(UINT_8)CMD_ID_SET_PSCAN_PARAM,
				TRUE,
				FALSE,
				FALSE,
				nicCmdEventSetCommon,
				nicOidCmdTimeoutCommon,
				(UINT_32)sizeof(CMD_SET_PSCAN_PARAM),
				(PUINT_8)prCmdPscnParam, NULL, 0U);

	return TRUE;
}

/*----------------------------------------------------------------------------*/
/*!
* \brief         handler for Set CMD_ID_SET_PSCN_ADD_SW_BSSID
*
* \param[in]
*
* \return none
*/
/*----------------------------------------------------------------------------*/
BOOLEAN scnFsmPSCNAddSWCBssId(IN P_ADAPTER_T prAdapter, IN P_CMD_SET_PSCAN_ADD_SWC_BSSID prCmdPscnAddSWCBssId)
{
	CMD_SET_PSCAN_ADD_SWC_BSSID rCmdPscnAddSWCBssId;
	P_SCAN_INFO_T prScanInfo;
	WLAN_STATUS rStatus = WLAN_STATUS_SUCCESS;

	ASSERT(prAdapter);
	prScanInfo = &(prAdapter->rWifiVar.rScanInfo);

	memcpy(&rCmdPscnAddSWCBssId.aucMacAddr, &(prCmdPscnAddSWCBssId->aucMacAddr), sizeof(MAC_ADDR_LEN));

	/* rCmdPscnAddSWCBssId.aucMacAddr = prCmdPscnAddSWCBssId->aucMacAddr; */
	rCmdPscnAddSWCBssId.i4RssiHighThreshold = prCmdPscnAddSWCBssId->i4RssiHighThreshold;
	rCmdPscnAddSWCBssId.i4RssiLowThreshold = prCmdPscnAddSWCBssId->i4RssiLowThreshold;

	if ((prScanInfo->fgPscnOngoing == TRUE) &&
		(prScanInfo->prPscnParam->fgGScnEnable == TRUE)) {
		rStatus = wlanSendSetQueryCmd(prAdapter,
				(UINT_8)CMD_ID_SET_PSCN_ADD_SW_BSSID,
				TRUE,
				FALSE,
				FALSE,
				NULL,
				nicOidCmdTimeoutCommon,
				(UINT_32)sizeof(CMD_SET_PSCAN_ADD_SWC_BSSID),
				(PUINT_8)&rCmdPscnAddSWCBssId, NULL, 0U);
		return TRUE;
	}
	/* debug msg, No PSCN, Sched SCAN no need to add the hotlist ??? */
	return FALSE;

}

/*----------------------------------------------------------------------------*/
/*!
* \brief         handler for Set CMD_ID_SET_PSCN_MAC_ADDR
*
* \param[in]
*
* \return none
*/
/*----------------------------------------------------------------------------*/
BOOLEAN scnFsmPSCNSetMacAddr(IN P_ADAPTER_T prAdapter, IN P_CMD_SET_PSCAN_MAC_ADDR prCmdPscnSetMacAddr)
{
	CMD_SET_PSCAN_MAC_ADDR rCmdPscnSetMacAddr;
	P_SCAN_INFO_T prScanInfo;
	WLAN_STATUS rStatus = WLAN_STATUS_SUCCESS;

	ASSERT(prAdapter);
	prScanInfo = &(prAdapter->rWifiVar.rScanInfo);

	/* rCmdPscnSetMacAddr.aucMacAddr = prCmdPscnSetMacAddr->aucMacAddr; */
	memcpy(&rCmdPscnSetMacAddr.aucMacAddr, &(prCmdPscnSetMacAddr->aucMacAddr), sizeof(MAC_ADDR_LEN));

	rCmdPscnSetMacAddr.ucFlags = prCmdPscnSetMacAddr->ucFlags;
	rCmdPscnSetMacAddr.ucVersion = prCmdPscnSetMacAddr->ucVersion;

	rStatus = wlanSendSetQueryCmd(prAdapter,
				(UINT_8)CMD_ID_SET_PSCN_MAC_ADDR,
				TRUE,
				FALSE,
				FALSE,
				NULL,
				nicOidCmdTimeoutCommon,
				(UINT_32)sizeof(CMD_SET_PSCAN_MAC_ADDR),
				(PUINT_8)&rCmdPscnSetMacAddr, NULL, 0U);
	return TRUE;

}

/*----------------------------------------------------------------------------*/
/*!
* \brief        handler for Combine PNO Scan params into PSCAN param
* \param[in]
*
* \return none
*/
/*----------------------------------------------------------------------------*/
static VOID
scnSubCombineNLOtoPSCN(IN P_ADAPTER_T prAdapter, IN P_CMD_NLO_REQ prNewCmdNloReq)
{
	P_SCAN_INFO_T prScanInfo;
	P_CMD_SET_PSCAN_PARAM prCmdPscnParam;

	ASSERT(prAdapter);
	prScanInfo = &(prAdapter->rWifiVar.rScanInfo);
	prCmdPscnParam = (P_CMD_SET_PSCAN_PARAM) prScanInfo->prPscnParam;

	if (prNewCmdNloReq != NULL) {
		prCmdPscnParam->fgNLOScnEnable = TRUE;
		kalMemCopy(&(prCmdPscnParam->rCmdNloReq), prNewCmdNloReq, sizeof(CMD_NLO_REQ));
	}

}

/*----------------------------------------------------------------------------*/
/*!
* \brief        handler for Combine Batch Scan params into PSCAN param
* \param[in]
*
* \return none
*/
/*----------------------------------------------------------------------------*/
static VOID
scnSubCombineBatchSCNtoPSCN(IN P_ADAPTER_T prAdapter, IN P_CMD_BATCH_REQ_T prNewCmdBatchReq)
{
	P_SCAN_INFO_T prScanInfo;
	P_CMD_SET_PSCAN_PARAM prCmdPscnParam;

	ASSERT(prAdapter);
	prScanInfo = &(prAdapter->rWifiVar.rScanInfo);
	prCmdPscnParam = (P_CMD_SET_PSCAN_PARAM) prScanInfo->prPscnParam;

	if (prNewCmdBatchReq != NULL) {
		prCmdPscnParam->fgBatchScnEnable = TRUE;
		kalMemCopy(&(prCmdPscnParam->rCmdBatchReq), prNewCmdBatchReq, sizeof(CMD_BATCH_REQ_T));
	}

}

/*----------------------------------------------------------------------------*/
/*!
* \brief        handler for Combine GSCN Scan params into PSCAN param
* \param[in]
*
* \return none
*/
/*----------------------------------------------------------------------------*/
static VOID
scnSubCombineGSCNtoPSCN(IN P_ADAPTER_T prAdapter,
			IN P_CMD_GSCN_REQ_T prNewCmdGscnReq, IN P_CMD_GSCN_SCN_COFIG_T prNewCmdGscnConfig)
{
	P_SCAN_INFO_T prScanInfo;
	P_CMD_SET_PSCAN_PARAM prCmdPscnParam;
	UINT_32 ucPeriodMin = MAX_PERIOD;

	ASSERT(prAdapter);
	prScanInfo = &(prAdapter->rWifiVar.rScanInfo);
	prCmdPscnParam = (P_CMD_SET_PSCAN_PARAM) prScanInfo->prPscnParam;
	prCmdPscnParam->fgGScnEnable = FALSE;

	DBGLOG(SCN, TRACE, "scnSubCombineGSCNtoPSCN fgGScnParamSet %d fgGScnConfigSet %d\n",
		prScanInfo->fgGScnParamSet, prScanInfo->fgGScnConfigSet);

	if (prNewCmdGscnReq != NULL) {
		DBGLOG(SCN, INFO, "setup prNewCmdGscnReq\n");
		prScanInfo->fgGScnParamSet = TRUE;
		kalMemCopy(&(prCmdPscnParam->rCmdGscnReq), prNewCmdGscnReq, sizeof(CMD_GSCN_REQ_T));
		if (prNewCmdGscnReq->u4BasePeriod < ucPeriodMin)
			prCmdPscnParam->u4BasePeriod = prNewCmdGscnReq->u4BasePeriod;
	}

	if (prNewCmdGscnConfig != NULL) {
		DBGLOG(SCN, INFO, "setup prNewCmdGscnConfig\n");
		prScanInfo->fgGScnConfigSet = TRUE;
		prCmdPscnParam->fgGScnEnable = TRUE;
		prCmdPscnParam->rCmdGscnReq.u4MaxApPerScan = prNewCmdGscnConfig->ucNumApPerScn;
		prCmdPscnParam->rCmdGscnReq.u4BufferThreshold = prNewCmdGscnConfig->u4BufferThreshold;
		prCmdPscnParam->rCmdGscnReq.ucNumScnToCache = (UINT_8) prNewCmdGscnConfig->u4NumScnToCache;
	}

}

/*----------------------------------------------------------------------------*/
/*!
* \brief        handler for Combine   GSCN Scan params into PSCAN param
* \param[in]
*
* \return none
*/
/*----------------------------------------------------------------------------*/
static VOID
scnRemoveFromPSCN(IN P_ADAPTER_T prAdapter,
		  IN BOOLEAN fgRemoveNLOfromPSCN,
		  IN BOOLEAN fgRemoveBatchSCNfromPSCN,
		  IN BOOLEAN fgRemoveGSCNfromPSCN)
{
	P_SCAN_INFO_T prScanInfo;
	P_CMD_SET_PSCAN_PARAM prCmdPscnParam;

	ASSERT(prAdapter);
	prScanInfo = &(prAdapter->rWifiVar.rScanInfo);
	prCmdPscnParam = (P_CMD_SET_PSCAN_PARAM) prScanInfo->prPscnParam;

	DBGLOG(SCN, INFO, "remove NLO or Batch or GSCN from PSCN--->NLO=%d, BSN=%d, GSN=%d\n",
		fgRemoveNLOfromPSCN, fgRemoveBatchSCNfromPSCN, fgRemoveGSCNfromPSCN);

	if (fgRemoveNLOfromPSCN == TRUE) {
		prCmdPscnParam->fgNLOScnEnable = FALSE;
		kalMemZero(&prCmdPscnParam->rCmdNloReq, sizeof(CMD_NLO_REQ));
	}
	if (fgRemoveBatchSCNfromPSCN == TRUE) {
		prCmdPscnParam->fgBatchScnEnable = FALSE;
		kalMemZero(&prCmdPscnParam->rCmdBatchReq, sizeof(CMD_BATCH_REQ_T));
	}
	if (fgRemoveGSCNfromPSCN == TRUE) {
		prCmdPscnParam->fgGScnEnable = FALSE;
		prScanInfo->fgGScnParamSet = FALSE;
		prScanInfo->fgGScnConfigSet = FALSE;
		prScanInfo->fgGScnAction = FALSE;
		kalMemZero(&prCmdPscnParam->rCmdGscnReq, sizeof(CMD_GSCN_REQ_T));
	}

}

/*----------------------------------------------------------------------------*/
/*!
* \brief        handler for Combine GSCN , Batch, PNO Scan params into PSCAN param
* \param[in]
*
* \return none
*/
/*----------------------------------------------------------------------------*/
BOOLEAN
scnCombineParamsIntoPSCN(IN P_ADAPTER_T prAdapter,
			 IN P_CMD_NLO_REQ prNewCmdNloReq,
			 IN P_CMD_BATCH_REQ_T prNewCmdBatchReq,
			 IN P_CMD_GSCN_REQ_T prNewCmdGscnReq,
			 IN P_CMD_GSCN_SCN_COFIG_T prNewCmdGscnConfig,
			 IN BOOLEAN fgRemoveNLOfromPSCN,
			 IN BOOLEAN fgRemoveBatchSCNfromPSCN, IN BOOLEAN fgRemoveGSCNfromPSCN)
{
	P_SCAN_INFO_T prScanInfo;
	P_CMD_SET_PSCAN_PARAM prCmdPscnParam;

	ASSERT(prAdapter);
	prScanInfo = &(prAdapter->rWifiVar.rScanInfo);
	prCmdPscnParam = (P_CMD_SET_PSCAN_PARAM) prScanInfo->prPscnParam;

	prCmdPscnParam->ucVersion = CURRENT_PSCN_VERSION;

	if ((fgRemoveNLOfromPSCN == TRUE) ||
		(fgRemoveBatchSCNfromPSCN == TRUE) ||
		(fgRemoveGSCNfromPSCN == TRUE)) {
		scnRemoveFromPSCN(prAdapter, fgRemoveNLOfromPSCN,
			fgRemoveBatchSCNfromPSCN, fgRemoveGSCNfromPSCN);
	} else {
		DBGLOG(SCN, TRACE, "combine GSCN or Batch or NLO to PSCN --->\n");

		scnSubCombineNLOtoPSCN(prAdapter, prNewCmdNloReq);
		scnSubCombineBatchSCNtoPSCN(prAdapter, prNewCmdBatchReq);
		scnSubCombineGSCNtoPSCN(prAdapter, prNewCmdGscnReq, prNewCmdGscnConfig);
	}

	return TRUE;
}

VOID scnPSCNFsm(IN P_ADAPTER_T prAdapter, IN ENUM_PSCAN_STATE_T eNextPSCNState)
{
	P_SCAN_INFO_T prScanInfo;
	BOOLEAN fgTransitionState = FALSE;
	BOOLEAN fgResult = FALSE;
	WLAN_STATUS rStatus = WLAN_STATUS_SUCCESS;

	ASSERT(prAdapter);
	prScanInfo = &(prAdapter->rWifiVar.rScanInfo);

	do {
		fgTransitionState = FALSE;

		DBGLOG(SCN, STATE, "eCurrentPSCNState=%d, eNextPSCNState=%d\n",
			prScanInfo->eCurrentPSCNState, eNextPSCNState);

		prScanInfo->eCurrentPSCNState = eNextPSCNState;

		switch (prScanInfo->eCurrentPSCNState) {
		case PSCN_IDLE:
			DBGLOG(SCN, TRACE, "PSCN_IDLE.... PSCAN_ACT_DISABLE\n");
			fgResult = scnFsmPSCNAction(prAdapter,
				PSCAN_ACT_DISABLE);
			eNextPSCNState = PSCN_IDLE;
			break;

		case PSCN_RESET:
			DBGLOG(SCN, TRACE, "PSCN_RESET.... PSCAN_ACT_DISABLE\n");
			if (!IS_NET_ACTIVE(prAdapter, prAdapter->prAisBssInfo->ucBssIndex)) {
				SET_NET_ACTIVE(prAdapter, prAdapter->prAisBssInfo->ucBssIndex);

				DBGLOG(SCN, TRACE, "ACTIVATE AIS from INACTIVE to enable PSCN\n");
				/* sync with firmware */
				rStatus = nicActivateNetwork(prAdapter,
					prAdapter->prAisBssInfo->ucBssIndex);
				prAdapter->prAisBssInfo->fgIsNetRequestInActive = FALSE;
			}
			fgResult =
				scnFsmPSCNAction(prAdapter, PSCAN_ACT_DISABLE);
			fgResult =
				scnFsmPSCNSetParam(prAdapter,
				prScanInfo->prPscnParam);

			if ((prScanInfo->prPscnParam->fgNLOScnEnable == TRUE) ||
			(prScanInfo->prPscnParam->fgBatchScnEnable == TRUE) ||
			((prScanInfo->prPscnParam->fgGScnEnable == TRUE) &&
				(prScanInfo->fgGScnAction == TRUE))) {
				eNextPSCNState = PSCN_SCANNING; /* keep original operation if there is any PSCN */
				DBGLOG(SCN, TRACE,
				       "PSCN_RESET->PSCN_SCANNING....fgNLOScnEnable/fgBatchScnEnable/fgGScnEnable ENABLE\n");
			} else {
				/* eNextPSCNState = PSCN_RESET; */
				DBGLOG(SCN, TRACE,
				       "PSCN_RESET->PSCN_RESET....fgNLOScnEnable/fgBatchScnEnable/fgGScnEnable DISABLE\n");
			}
			break;

		case PSCN_SCANNING:
			DBGLOG(SCN, TRACE, "PSCN_SCANNING.... PSCAN_ACT_ENABLE\n");
			if (prScanInfo->fgPscnOngoing == TRUE)
				break;
			fgResult =
				scnFsmPSCNAction(prAdapter, PSCAN_ACT_ENABLE);
			prScanInfo->fgPscnOngoing = TRUE;
			eNextPSCNState = PSCN_SCANNING;
			break;

		default:
			DBGLOG(SCN, WARN, "Unexpected state\n");
			ASSERT(NULL);
			break;
		}

		if (prScanInfo->eCurrentPSCNState != eNextPSCNState)
			fgTransitionState = TRUE;

	} while (fgTransitionState == TRUE);

}
#endif

#if CFG_SUPPORT_GSCN
/*----------------------------------------------------------------------------*/
/*!
* \brief        handler for Set GSCN param
* \param[in]
*
* \return none
*/
/*----------------------------------------------------------------------------*/
BOOLEAN scnSetGSCNParam(IN P_ADAPTER_T prAdapter, IN P_PARAM_WIFI_GSCAN_CMD_PARAMS prCmdGscnParam)
{
	P_CMD_GSCN_REQ_T prCmdGscnReq;
	UINT_8 i = 0, j = 0; /*ucBucketIndex, ucChIndex*/
	BOOLEAN fgResult = FALSE;

	ASSERT(prAdapter);
	prCmdGscnReq = kalMemAlloc(sizeof(CMD_GSCN_REQ_T), VIR_MEM_TYPE);
	if (prCmdGscnReq == NULL) {
		DBGLOG(SCN, ERROR, "alloc prCmdGscnReq fail\n");
		return FALSE;
	}
	kalMemZero(prCmdGscnReq, sizeof(CMD_GSCN_REQ_T));
	prCmdGscnReq->u4NumBuckets = prCmdGscnParam->num_buckets;
	prCmdGscnReq->u4BasePeriod = prCmdGscnParam->base_period;
	DBGLOG(SCN, TRACE, "u4BasePeriod[%d], u4NumBuckets[%d]\n",
		prCmdGscnReq->u4BasePeriod, prCmdGscnReq->u4NumBuckets);

	for (i = 0U; i < prCmdGscnReq->u4NumBuckets; i++) {
		prCmdGscnReq->arBucket[i].u2BucketIndex =
		    (UINT_16) prCmdGscnParam->buckets[i].bucket;
		prCmdGscnReq->arBucket[i].eBand = prCmdGscnParam->buckets[i].band;
		prCmdGscnReq->arBucket[i].ucBucketFreqMultiple =
		    (UINT_8)(prCmdGscnParam->buckets[i].period /
		    prCmdGscnParam->base_period);
		prCmdGscnReq->arBucket[i].ucReportFlag = prCmdGscnParam->buckets[i].report_events;
		prCmdGscnReq->arBucket[i].ucMaxBucketFreqMultiple =
		    (UINT_8)(prCmdGscnParam->buckets[i].max_period /
		    prCmdGscnParam->base_period);
		prCmdGscnReq->arBucket[i].ucStepCount = (UINT_8)prCmdGscnParam->buckets[i].step_count;

		prCmdGscnReq->arBucket[i].ucNumChannels =
		    (UINT_8)prCmdGscnParam->buckets[i].num_channels;
		DBGLOG(SCN, TRACE, "assign %d channels to bucket[%d]\n",
			prCmdGscnReq->arBucket[i].ucNumChannels, i);
		for (j = 0U; j < prCmdGscnParam->buckets[i].num_channels; j++) {
			prCmdGscnReq->arBucket[i].arChannelList[j].ucChannelNumber =
			(UINT_8)nicFreq2ChannelNum(1000U *
			prCmdGscnParam->buckets[i].channels[j].channel);
			prCmdGscnReq->arBucket[i].arChannelList[j].ucPassive =
			(UINT_8)prCmdGscnParam->buckets[i].channels[j].passive;
			prCmdGscnReq->arBucket[i].arChannelList[j].u4DwellTimeMs =
			    prCmdGscnParam->buckets[i].channels[j].dwellTimeMs;

			DBGLOG(SCN, TRACE, "[ucChannel %d, ucPassive %d, u4DwellTimeMs %d\n",
			       prCmdGscnReq->arBucket[i].arChannelList[j].ucChannelNumber,
			       prCmdGscnReq->arBucket[i].arChannelList[j].ucPassive,
			       prCmdGscnReq->arBucket[i].arChannelList[j].u4DwellTimeMs);
		}
	}

	fgResult = scnCombineParamsIntoPSCN(prAdapter, NULL, NULL, prCmdGscnReq,
		NULL, FALSE, FALSE, FALSE);
	scnPSCNFsm(prAdapter, PSCN_RESET);

	kalMemFree(prCmdGscnReq, VIR_MEM_TYPE, sizeof(CMD_GSCN_REQ_T));
	return TRUE;
}

/*----------------------------------------------------------------------------*/
/*!
* \brief         handler for scnSetGSCNConfig
*
* \param[in]
*
* \return none
*/
/*----------------------------------------------------------------------------*/
BOOLEAN scnSetGSCNConfig(IN P_ADAPTER_T prAdapter, IN P_CMD_GSCN_SCN_COFIG_T prCmdGscnScnConfig)
{
	BOOLEAN fgResult = FALSE;

	ASSERT(prAdapter);
	ASSERT(prCmdGscnScnConfig);

	fgResult = scnCombineParamsIntoPSCN(prAdapter, NULL, NULL, NULL,
		prCmdGscnScnConfig, FALSE, FALSE, FALSE);
	scnPSCNFsm(prAdapter, PSCN_RESET);

	return TRUE;
}

/*----------------------------------------------------------------------------*/
/*!
* \brief         handler for scnFsmGetGSCNResult
*
* \param[in]
*
* \return none
*/
/*----------------------------------------------------------------------------*/
BOOLEAN scnFsmGetGSCNResult(IN P_ADAPTER_T prAdapter, IN P_CMD_GET_GSCAN_RESULT_T prGetGscnResultCmd,
			    OUT PUINT_32 pu4SetInfoLen)
{
	CMD_GET_GSCAN_RESULT_T rGetGscnResultCmd;
	P_SCAN_INFO_T prScanInfo;
	P_PARAM_WIFI_GSCAN_RESULT_REPORT prGscnResult;
	struct wiphy *pWiphy;
	UINT_32 u4SizeofGScanResults = 0U;
	UINT_32 ucBkt;
	static UINT_8 scanId, numAp;
	P_PARAM_BSSID_EX_T prScanResults;
	UINT_8 i = 0, remainAp = 0;
	int iStatus;
	UINT_8 flag;

	ASSERT(prAdapter);
	pWiphy = priv_to_wiphy(prAdapter->prGlueInfo);
	prScanInfo = &(prAdapter->rWifiVar.rScanInfo);
	kalMemCopy(&rGetGscnResultCmd, prGetGscnResultCmd, sizeof(CMD_GET_GSCAN_RESULT_T));
	DBGLOG(SCN, TRACE, "rGetGscnResultCmd: ucGetNum[%d], fgFlush[%d]\n",
		rGetGscnResultCmd.u4Num, rGetGscnResultCmd.ucFlush);
	if ((rGetGscnResultCmd.u4Num == 0U) ||
		(rGetGscnResultCmd.u4Num > PSCAN_MAX_AP_CACHE_PER_SCAN))
		rGetGscnResultCmd.u4Num = PSCAN_MAX_AP_CACHE_PER_SCAN;

	scanId++;
	u4SizeofGScanResults =
		(UINT_32)(sizeof(PARAM_WIFI_GSCAN_RESULT_REPORT) +
		sizeof(PARAM_WIFI_GSCAN_RESULT) *
		((UINT_64)rGetGscnResultCmd.u4Num - 1U));
	prGscnResult = kalMemAlloc(u4SizeofGScanResults, VIR_MEM_TYPE);
	if (prGscnResult == NULL) {
		DBGLOG(SCN, ERROR, "Can not alloc memory for PARAM_WIFI_GSCAN_RESULT_REPORT\n");
		return FALSE;
	}
	kalMemZero(prGscnResult, u4SizeofGScanResults);

	prGscnResult->u4ScanId = (UINT_32)scanId;
	prGscnResult->ucScanFlag = 3U;
	for (ucBkt = 0U; ucBkt < GSCAN_MAX_BUCKETS; ucBkt++) {
		flag = prScanInfo->prPscnParam->rCmdGscnReq
				.arBucket[ucBkt].ucReportFlag;
		if ((flag == REPORT_EVENTS_EACH_SCAN) ||
			(flag == REPORT_EVENTS_FULL_RESULTS))
			prGscnResult->u4BucketMask |= ((UINT_32)(1U) << ucBkt);
	}

	if (numAp < prAdapter->rWlanInfo.u4ScanResultNum)
		remainAp = (UINT_8)prAdapter->rWlanInfo.u4ScanResultNum - numAp;
	else {
		kalMemFree(prGscnResult, VIR_MEM_TYPE, u4SizeofGScanResults);
		return FALSE;
	}
	rGetGscnResultCmd.u4Num = (rGetGscnResultCmd.u4Num <= remainAp) ?
		rGetGscnResultCmd.u4Num : remainAp;

	DBGLOG(SCN, TRACE, "u4Num=%d, numAp=%d\n",
		rGetGscnResultCmd.u4Num, numAp);

	for (i = 0U; i < rGetGscnResultCmd.u4Num; i++) {
		prScanResults = &(prAdapter->rWlanInfo.arScanResult[numAp]);
		prGscnResult->rResult[i].ts = kalGetBootTime();
		if (prScanResults->rSsid.u4SsidLen <= ELEM_MAX_LEN_SSID)
			kalMemCopy(prGscnResult->rResult[i].ssid,
				prScanResults->rSsid.aucSsid,
				prScanResults->rSsid.u4SsidLen);

		kalMemCopy(prGscnResult->rResult[i].bssid,
			prScanResults->arMacAddress, MAC_ADDR_LEN);
		prGscnResult->rResult[i].channel =
			prScanResults->rConfiguration.u4DSConfig / 1000U;
		prGscnResult->rResult[i].rssi = prScanResults->rRssi;
		prGscnResult->rResult[i].rtt = 0;
		prGscnResult->rResult[i].rtt_sd = 0;
		prGscnResult->rResult[i].beacon_period =
			(UINT_16)prScanResults->rConfiguration.u4BeaconPeriod;
		prGscnResult->rResult[i].capability = prScanResults->u2CapInfo;
		prGscnResult->rResult[i].ie_length = prScanResults->u4IELength;

		numAp++;
		DBGLOG(SCN, TRACE, "Report GScan SSID[%s][%d]",
				prGscnResult->rResult[i].ssid,
				prGscnResult->rResult[i].channel);
		DBGLOG(SCN, TRACE, "[" MACSTR "]u4IELength=%d u2CapInfo=0x%x\n",
				MAC2STR(prGscnResult->rResult[i].bssid),
				prScanResults->u4IELength,
				prScanResults->u2CapInfo);
	}
	if (numAp >= prAdapter->rWlanInfo.u4ScanResultNum)
		numAp = 0U;

	prGscnResult->u4NumOfResults = rGetGscnResultCmd.u4Num;
	u4SizeofGScanResults += (UINT_32)sizeof(struct nlattr) * 2U;
	DBGLOG(SCN, INFO, "scan_id=%d, scan_flag=0x%x, 0x%x, num=%d %d %d, u4SizeofGScanResults=%d\r\n",
		prGscnResult->u4ScanId,
		prGscnResult->ucScanFlag,
		prGscnResult->u4BucketMask, numAp,
		prGscnResult->u4NumOfResults, prAdapter->rWlanInfo.u4ScanResultNum,
		u4SizeofGScanResults);

	if (numAp == rGetGscnResultCmd.u4Num) /* start transfer*/
		iStatus = mtk_cfg80211_vendor_gscan_results(pWiphy,
			prAdapter->prGlueInfo->prDevHandler->ieee80211_ptr,
			prGscnResult, (int)u4SizeofGScanResults, TRUE, FALSE);
	iStatus = mtk_cfg80211_vendor_gscan_results(pWiphy,
			prAdapter->prGlueInfo->prDevHandler->ieee80211_ptr,
			prGscnResult, (int)u4SizeofGScanResults, FALSE, FALSE);
	if (numAp == 0U) /* end transfer */
		iStatus = mtk_cfg80211_vendor_gscan_results(pWiphy,
			prAdapter->prGlueInfo->prDevHandler->ieee80211_ptr,
			prGscnResult, (int)u4SizeofGScanResults, TRUE, TRUE);

	kalMemFree(prGscnResult, VIR_MEM_TYPE,
		u4SizeofGScanResults - sizeof(struct nlattr) * 2U);

	return TRUE;
}


BOOLEAN scnFsmGSCNResults(IN P_ADAPTER_T prAdapter, IN P_EVENT_GSCAN_RESULT_T prEventBuffer)
{
	P_PARAM_WIFI_GSCAN_RESULT_REPORT prGscnResult;
	struct wiphy *pWiphy;
	UINT_32 u4SizeofGScanResults = 0U;
	int uResult = 0;

	prGscnResult = kalMemAlloc(sizeof(PARAM_WIFI_GSCAN_RESULT_REPORT), VIR_MEM_TYPE);
	if (prGscnResult == NULL) {
		DBGLOG(SCN, ERROR, "Can not alloc memory for PARAM_WIFI_GSCAN_RESULT_REPORT\n");
		return FALSE;
	}

	prGscnResult->u4ScanId = (UINT_32)prEventBuffer->u2ScanId;
	prGscnResult->ucScanFlag = (UINT_8)prEventBuffer->u2ScanFlags;
	prGscnResult->u4BucketMask = 1U;
	prGscnResult->u4NumOfResults = (UINT_32)prEventBuffer->u2NumOfResults;

	/* PARAM_WIFI_GSCAN_RESULT similar to  WIFI_GSCAN_RESULT_T*/
	kalMemCopy(&prGscnResult->rResult[0], &prEventBuffer->rResult[0],
		sizeof(PARAM_WIFI_GSCAN_RESULT) * (prGscnResult->u4NumOfResults));

	u4SizeofGScanResults = (UINT_32)sizeof(PARAM_WIFI_GSCAN_RESULT_REPORT) +
				(UINT_32)sizeof(struct nlattr) * 2U +
				(UINT_32)sizeof(PARAM_WIFI_GSCAN_RESULT) *
				(prGscnResult->u4NumOfResults - 1U);
	DBGLOG(SCN, INFO, "scan_id=%d, scan_flag=0x%x, 0x%x, num=%d, u4SizeofGScanResults=%d\r\n",
		prGscnResult->u4ScanId,
		(UINT_32)prGscnResult->ucScanFlag,
		prGscnResult->u4BucketMask,
		prGscnResult->u4NumOfResults,
		u4SizeofGScanResults);

	pWiphy = priv_to_wiphy(prAdapter->prGlueInfo);
	uResult = mtk_cfg80211_vendor_gscan_results(pWiphy,
			prAdapter->prGlueInfo->prDevHandler->ieee80211_ptr,
			prGscnResult, (int)u4SizeofGScanResults, FALSE, FALSE);

	kalMemFree(prGscnResult, VIR_MEM_TYPE, sizeof(PARAM_WIFI_GSCAN_RESULT_REPORT));

	return TRUE;
}
#endif

VOID scnScanDoneTimeout(IN P_ADAPTER_T prAdapter, ULONG ulParamPtr)
{
	P_SCAN_INFO_T prScanInfo;
#if CFG_CHIP_RESET_SUPPORT
	// Autochips delete
	//BOOLEAN fgResult;
#endif

	prScanInfo = &(prAdapter->rWifiVar.rScanInfo);

	DBGLOG(SCN, WARN, "scnScanDoneTimeout %d \r\n", prScanInfo->ucScanDoneTimeoutCnt);

	prScanInfo->ucScanDoneTimeoutCnt++;
	/* whole chip reset check */
	if (prScanInfo->ucScanDoneTimeoutCnt > SCAN_DONE_TIMEOUT_THRESHOLD) {

		DBGLOG(SCN, ERROR,
		       " meet SCAN_DONE_TIMEOUT_THRESHOLD %d, trigger whole chip reset !! \r\n",
		       SCAN_DONE_TIMEOUT_THRESHOLD);
#if CFG_CHIP_RESET_SUPPORT
		// Autochips delete
		//fgResult = glResetTrigger(prAdapter);
#endif
	}
}
