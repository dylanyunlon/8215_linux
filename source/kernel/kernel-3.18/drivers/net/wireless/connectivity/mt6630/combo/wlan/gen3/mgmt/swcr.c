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
** Id: /Department/DaVinci/BRANCHES/MT6620_WIFI_DRIVER_V2_3/mgmt/swcr.c#1
*/

/*
 * ! \file   "swcr.c"
 *  \brief
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
#include "swcr.h"

#if CFG_SUPPORT_SWCR

/*******************************************************************************
*                             D A T A   T Y P E S
********************************************************************************
*/

/*******************************************************************************
*                            P U B L I C   D A T A
********************************************************************************
*/
static UINT_32 g_au4SwCR[SWCR_CR_NUM];	/*: 0: command other: data */

/* JB mDNS Filter*/
static UINT_32 g_u4RxFilter;	/* [31] 0: stop 1: start, [3] IPv6 [2] IPv4 */

static TIMER_T g_rSwcrDebugTimer;
static BOOLEAN g_fgSwcrDebugTimer = FALSE;
static UINT_32 g_u4SwcrDebugCheckTimeout;
static ENUM_SWCR_DBG_TYPE_T g_ucSwcrDebugCheckType;
static UINT_32 g_u4SwcrDebugFrameDumpType;

/*******************************************************************************
*                           P R I V A T E   D A T A
********************************************************************************
*/
static const PFN_CMD_RW_T g_arSwCtrlCmd[] = {
	swCtrlCmdCategory0,
	swCtrlCmdCategory1
#if TEST_PS
	    , testPsCmdCategory0, testPsCmdCategory1
#endif
#if CFG_SUPPORT_802_11V
#if (CFG_SUPPORT_802_11V_TIMING_MEASUREMENT == 1) && (WNM_UNIT_TEST == 1)
	    , testWNMCmdCategory0
#endif
#endif
};

static const PFN_SWCR_RW_T g_arSwCrModHander[] = {
	swCtrlSwCr,
	NULL
};

/*******************************************************************************
*                                 M A C R O S
********************************************************************************
*/

enum {
	SWCTRL_MAGIC,
	SWCTRL_DEBUG,
	SWCTRL_WIFI_VAR,
	SWCTRL_ENABLE_INT,
	SWCTRL_DISABLE_INT,
	SWCTRL_TXM_INFO,
	SWCTRL_RXM_INFO,
	SWCTRL_DUMP_BSS,
	SWCTRL_QM_INFO,
	SWCTRL_DUMP_ALL_QUEUE_LEN,
	SWCTRL_DUMP_MEM,
	SWCTRL_TX_CTRL_INFO,
	SWCTRL_DUMP_QUEUE,
	SWCTRL_DUMP_QM_DBG_CNT,
	SWCTRL_QM_DBG_CNT,
	SWCTRL_RX_PKTS_DUMP,
	SWCTRL_RX_FILTER,
#if CFG_INIT_ENABLE_PATTERN_FILTER_ARP
	SWCTRL_RX_ARP_OFFLOAD,
#endif
	SWCTRL_PS_DTIM_SKIP,
	SWCTRL_ROAMING,
	SWCTRL_CATA0_INDEX_NUM
};

enum {
	SWCTRL_STA_INFO,
	SWCTRL_DUMP_STA,
	SWCTRL_STA_QUE_INFO,
	SWCTRL_CATA1_INDEX_NUM
};

/* JB mDNS Filter*/
#define RX_FILTER_START ((UINT_32)1U<<31U)
#define RX_FILTER_IPV4  (1U<<2U)
#define RX_FILTER_IPV6  (1U<<3U)
typedef enum _ENUM_SWCR_RX_FILTER_CMD_T {
	SWCR_RX_FILTER_CMD_STOP = 0,
	SWCR_RX_FILTER_CMD_START,
	SWCR_RX_FILTER_CMD_ADD,
	SWCR_RX_FILTER_CMD_REMOVE,
	SWCR_RX_FILTER_NUM
} ENUM_SWCR_RX_FILTER_CMD_T;

#if TEST_PS
enum {
	TEST_PS_MAGIC,
	TEST_PS_SETUP_BSS,
	TEST_PS_ENABLE_BEACON,
	TEST_PS_TRIGGER_BMC,
	TEST_PS_SEND_NULL,
	TEST_PS_BUFFER_BMC,
	TEST_PS_UPDATE_BEACON,
	TEST_PS_CATA0_INDEX_NUM
};

enum {
	TEST_PS_STA_PS,
	TEST_PS_STA_ENTER_PS,
	TEST_PS_STA_EXIT_PS,
	TEST_PS_STA_TRIGGER_PSPOLL,
	TEST_PS_STA_TRIGGER_FRAME,
	TEST_PS_CATA1_INDEX_NUM
};
#endif

#if CFG_SUPPORT_802_11V
#if WNM_UNIT_TEST
enum {
	TEST_WNM_TIMING_MEAS,
	TEST_WNM_CATA0_INDEX_NUM
};
#endif
#endif

#define g_SWCTRL_MAGIC 0x66201642U

/*******************************************************************************
*                   F U N C T I O N   D E C L A R A T I O N S
********************************************************************************
*/

/*******************************************************************************
*                              F U N C T I O N S
********************************************************************************
*/

static void dumpQueue(P_ADAPTER_T prAdapter)
{

	P_TX_CTRL_T prTxCtrl;
	P_QUE_MGT_T prQM;
	P_GLUE_INFO_T prGlueInfo;
	UINT_32 i, j;

	DEBUGFUNC("dumpQueue");

	prTxCtrl = &prAdapter->rTxCtrl;
	prQM = &prAdapter->rQM;
	prGlueInfo = prAdapter->prGlueInfo;
#if QM_ADAPTIVE_TC_RESOURCE_CTRL
	for (i = (UINT_32)TC0_INDEX; i <= (UINT_32)TC5_INDEX; i++) {
		DBGLOG(SW4, INFO, "TC %u\n", i);
		DBGLOG(SW4, INFO, "Max %u Free %u\n",
				   prTxCtrl->rTc.au2MaxNumOfBuffer[i], prTxCtrl->rTc.au2FreeBufferCount[i]);

		DBGLOG(SW4, INFO,
		       "Average %u minReserved %u CurrentTcResource %u GuaranteedTcResource %u\n",
			QM_GET_TX_QUEUE_LEN(prAdapter, i), prQM->au4MinReservedTcResource[i],
			prQM->au4CurrentTcResource[i], prQM->au4GuaranteedTcResource[i]);

	}
#endif

#if QM_FORWARDING_FAIRNESS
	for (i = 0U; i < NUM_OF_PER_STA_TX_QUEUES; i++) {
		DBGLOG(SW4, INFO,
		       "TC %u HeadStaIdx %u ForwardCount %u\n", i, prQM->au4HeadStaRecIndex[i],
			prQM->au4ResourceUsedCount[i]);
	}
#endif

	DBGLOG(SW4, INFO, "BMC or unknown TxQueue Len %u\n", prQM->arTxQueue[0].u4NumElem);
	DBGLOG(SW4, INFO, "Pending %d\n", prGlueInfo->i4TxPendingFrameNum);
	DBGLOG(SW4, INFO, "Pending Security %d\n", prGlueInfo->i4TxPendingSecurityFrameNum);
#if defined(LINUX)
	for (i = 0U; i < 4U; i++) {
		for (j = 0U; j < CFG_MAX_TXQ_NUM; j++) {
			DBGLOG(SW4, INFO,
			       "Pending Q[%u][%u] %d\n", i, j, prGlueInfo->ai4TxPendingFrameNumPerQueue[i][j]);
		}
	}
#endif

	DBGLOG(SW4, INFO, " rFreeSwRfbList %u\n", prAdapter->rRxCtrl.rFreeSwRfbList.u4NumElem);
	DBGLOG(SW4, INFO, " rReceivedRfbList %u\n", prAdapter->rRxCtrl.rReceivedRfbList.u4NumElem);
	DBGLOG(SW4, INFO, " rIndicatedRfbList %u\n", prAdapter->rRxCtrl.rIndicatedRfbList.u4NumElem);
	DBGLOG(SW4, INFO, " ucNumIndPacket %u\n", prAdapter->rRxCtrl.ucNumIndPacket);
	DBGLOG(SW4, INFO, " ucNumRetainedPacket %u\n", prAdapter->rRxCtrl.ucNumRetainedPacket);

}

static void dumpSTA(P_ADAPTER_T prAdapter, P_STA_RECORD_T prStaRec)
{
	UINT_8 ucWTEntry;
	UINT_32 i;
	P_BSS_INFO_T prBssInfo;

	DEBUGFUNC("dumpSTA");

	ASSERT(prStaRec);
	ucWTEntry = prStaRec->ucWlanIndex;

	prBssInfo = GET_BSS_INFO_BY_INDEX(prAdapter, prStaRec->ucBssIndex);
	ASSERT(prBssInfo);

	DBGLOG(SW4, INFO, "Mac address: " MACSTR " Rcpi %u\n", MAC2STR(prStaRec->aucMacAddr),
			prStaRec->ucRCPI);

	DBGLOG(SW4, INFO, "Idx %u Wtbl %u Used %u State %u Bss Phy 0x%x Sta DesiredPhy 0x%x\n",
			   prStaRec->ucIndex, ucWTEntry,
			   prStaRec->fgIsInUse, prStaRec->ucStaState,
			   prBssInfo->ucPhyTypeSet, prStaRec->ucDesiredPhyTypeSet);

	DBGLOG(SW4, INFO,
	       "Sta Operation 0x%x  DesiredNontHtRateSet  0x%x Mcs 0x%x u2HtCapInfo 0x%x\n",
		prStaRec->u2OperationalRateSet, prStaRec->u2DesiredNonHTRateSet, prStaRec->ucMcsSet,
		prStaRec->u2HtCapInfo);

	for (i = 0U; i < NUM_OF_PER_STA_TX_QUEUES; i++)
		DBGLOG(SW4, INFO, "TC %u Queue Len %u\n", i, prStaRec->arTxQueue[i].u4NumElem);

	DBGLOG(SW4, INFO, "BmpDeliveryAC %x\n", prStaRec->ucBmpDeliveryAC);
	DBGLOG(SW4, INFO, "BmpTriggerAC  %x\n", prStaRec->ucBmpTriggerAC);
	DBGLOG(SW4, INFO, "UapsdSpSupproted  %u\n", prStaRec->fgIsUapsdSupported);
	DBGLOG(SW4, INFO, "IsQoS  %u\n", prStaRec->fgIsQoS);
	DBGLOG(SW4, INFO, "AssocId %u\n", prStaRec->u2AssocId);

	DBGLOG(SW4, INFO, "fgIsInPS %u\n", prStaRec->fgIsInPS);
	DBGLOG(SW4, INFO, "ucFreeQuota %u\n", prStaRec->ucFreeQuota);
	DBGLOG(SW4, INFO, "ucFreeQuotaForDelivery %u\n", prStaRec->ucFreeQuotaForDelivery);
	DBGLOG(SW4, INFO, "ucFreeQuotaForNonDelivery %u\n", prStaRec->ucFreeQuotaForNonDelivery);

	for (i = 0U; i < CFG_RX_MAX_BA_TID_NUM; i++) {
		if (prStaRec->aprRxReorderParamRefTbl[i] != NULL) {
			DBGLOG(SW4, INFO,
			       "RxReorder fgIsValid: %u\n", prStaRec->aprRxReorderParamRefTbl[i]->fgIsValid);
			DBGLOG(SW4, INFO, "RxReorder Tid: %u\n", prStaRec->aprRxReorderParamRefTbl[i]->ucTid);
			DBGLOG(SW4, INFO,
			       "RxReorder rReOrderQue Len: %u\n",
				prStaRec->aprRxReorderParamRefTbl[i]->rReOrderQue.u4NumElem);
			DBGLOG(SW4, INFO,
			       "RxReorder WinStart: %u\n", prStaRec->aprRxReorderParamRefTbl[i]->u2WinStart);
			DBGLOG(SW4, INFO, "RxReorder WinEnd: %u\n", prStaRec->aprRxReorderParamRefTbl[i]->u2WinEnd);
			DBGLOG(SW4, INFO, "RxReorder WinSize: %u\n", prStaRec->aprRxReorderParamRefTbl[i]->u2WinSize);
		}
	}

}

static VOID dumpBss(P_ADAPTER_T prAdapter, P_BSS_INFO_T prBssInfo)
{

	DBGLOG(SW4, INFO, "SSID %s\n", prBssInfo->aucSSID);
	DBGLOG(SW4, INFO, "OWN " MACSTR "\n", MAC2STR(prBssInfo->aucOwnMacAddr));
	DBGLOG(SW4, INFO, "BSSID " MACSTR "\n", MAC2STR(prBssInfo->aucBSSID));
	DBGLOG(SW4, INFO, "eNetworkType %u\n", prBssInfo->eNetworkType);
	DBGLOG(SW4, INFO, "ucBssIndex %u\n", prBssInfo->ucBssIndex);
	DBGLOG(SW4, INFO, "eConnectionState %u\n", prBssInfo->eConnectionState);
	DBGLOG(SW4, INFO, "eCurrentOPMode %u\n", prBssInfo->eCurrentOPMode);
	DBGLOG(SW4, INFO, "fgIsQBSS %u\n", prBssInfo->fgIsQBSS);
	DBGLOG(SW4, INFO, "fgIsShortPreambleAllowed %u\n", prBssInfo->fgIsShortPreambleAllowed);
	DBGLOG(SW4, INFO, "fgUseShortPreamble %u\n", prBssInfo->fgUseShortPreamble);
	DBGLOG(SW4, INFO, "fgUseShortSlotTime %u\n", prBssInfo->fgUseShortSlotTime);
	DBGLOG(SW4, INFO, "ucNonHTBasicPhyType %x\n", prBssInfo->ucNonHTBasicPhyType);
	DBGLOG(SW4, INFO, "u2OperationalRateSet %x\n", prBssInfo->u2OperationalRateSet);
	DBGLOG(SW4, INFO, "u2BSSBasicRateSet %x\n", prBssInfo->u2BSSBasicRateSet);
	DBGLOG(SW4, INFO, "ucPhyTypeSet %x\n", prBssInfo->ucPhyTypeSet);
	DBGLOG(SW4, INFO, "rStaRecOfClientList %d\n", prBssInfo->rStaRecOfClientList.u4NumElem);
	DBGLOG(SW4, INFO, "u2CapInfo %x\n", prBssInfo->u2CapInfo);
	DBGLOG(SW4, INFO, "u2ATIMWindow %x\n", prBssInfo->u2ATIMWindow);
	DBGLOG(SW4, INFO, "u2AssocId %x\n", prBssInfo->u2AssocId);
	DBGLOG(SW4, INFO, "ucDTIMPeriod %x\n", prBssInfo->ucDTIMPeriod);
	DBGLOG(SW4, INFO, "ucDTIMCount %x\n", prBssInfo->ucDTIMCount);
	DBGLOG(SW4, INFO, "fgIsNetAbsent %x\n", prBssInfo->fgIsNetAbsent);
	DBGLOG(SW4, INFO, "eBand %d\n", prBssInfo->eBand);
	DBGLOG(SW4, INFO, "ucPrimaryChannel %d\n", prBssInfo->ucPrimaryChannel);
	DBGLOG(SW4, INFO, "ucHtOpInfo1 %d\n", prBssInfo->ucHtOpInfo1);
	DBGLOG(SW4, INFO, "ucHtOpInfo2 %d\n", prBssInfo->u2HtOpInfo2);
	DBGLOG(SW4, INFO, "ucHtOpInfo3 %d\n", prBssInfo->u2HtOpInfo3);
	DBGLOG(SW4, INFO, "fgErpProtectMode %d\n", prBssInfo->fgErpProtectMode);
	DBGLOG(SW4, INFO, "eHtProtectMode %d\n", prBssInfo->eHtProtectMode);
	DBGLOG(SW4, INFO, "eGfOperationMode %d\n", prBssInfo->eGfOperationMode);
	DBGLOG(SW4, INFO, "eRifsOperationMode %d\n", prBssInfo->eRifsOperationMode);
	DBGLOG(SW4, INFO, "fgObssErpProtectMode %d\n", prBssInfo->fgObssErpProtectMode);
	DBGLOG(SW4, INFO, "eObssHtProtectMode %d\n", prBssInfo->eObssHtProtectMode);
	DBGLOG(SW4, INFO, "eObssGfProtectMode %d\n", prBssInfo->eObssGfOperationMode);
	DBGLOG(SW4, INFO, "fgObssRifsOperationMode %d\n", prBssInfo->fgObssRifsOperationMode);
	DBGLOG(SW4, INFO, "fgAssoc40mBwAllowed %d\n", prBssInfo->fgAssoc40mBwAllowed);
	DBGLOG(SW4, INFO, "fg40mBwAllowed %d\n", prBssInfo->fg40mBwAllowed);
	DBGLOG(SW4, INFO, "eBssSCO %d\n", prBssInfo->eBssSCO);

}

VOID swCtrlCmdCategory0(P_ADAPTER_T prAdapter, UINT_8 ucCate, UINT_8 ucAction, UINT_8 ucOpt0, UINT_8 ucOpt1)
{
	UINT_8 ucIndex, ucRead;

	DEBUGFUNC("swCtrlCmdCategory0");

	SWCR_GET_RW_INDEX(ucAction, ucRead, ucIndex);

	if (ucIndex >= (UINT_8)SWCTRL_CATA0_INDEX_NUM)
		return;

	if (ucRead == SWCR_WRITE) {
		switch (ucIndex) {
		case (UINT_8)SWCTRL_DEBUG:
			break;
		case (UINT_8)SWCTRL_WIFI_VAR:
			break;

#if QM_DEBUG_COUNTER
		case (UINT_8)SWCTRL_DUMP_QM_DBG_CNT:
			UINT_32 i = 0U;

			for (i = 0U; i < (UINT_32)QM_DBG_CNT_NUM; i++)
				prAdapter->rQM.au4QmDebugCounters[i] = 0U;
			break;
		case (UINT_8)SWCTRL_QM_DBG_CNT:
			prAdapter->rQM.au4QmDebugCounters[ucOpt0] =
				g_au4SwCR[1];

			break;
#endif
#if CFG_RX_PKTS_DUMP
		case (UINT_8)SWCTRL_RX_PKTS_DUMP:
			/* DBGLOG(SW4, INFO,("SWCTRL_RX_PKTS_DUMP: mask %x\n", g_au4SwCr[1])); */
			prAdapter->rRxCtrl.u4RxPktsDumpTypeMask = g_au4SwCR[1];
			break;
#endif
		case (UINT_8)SWCTRL_RX_FILTER:
		{
			UINT_32 u4rxfilter;
			BOOLEAN fgUpdate = FALSE;
			WLAN_STATUS rStatus = WLAN_STATUS_SUCCESS;

			if (ucOpt0 == (UINT_8)SWCR_RX_FILTER_CMD_STOP) {
				g_u4RxFilter &= ~(RX_FILTER_START);
				prAdapter->u4OsPacketFilter &=
					~PARAM_PACKET_FILTER_MULTICAST;
				u4rxfilter = prAdapter->u4OsPacketFilter;
				fgUpdate = TRUE;
			} else if (ucOpt0 == (UINT_8)SWCR_RX_FILTER_CMD_START) {
				g_u4RxFilter |= (RX_FILTER_START);

			if (((g_u4RxFilter & RX_FILTER_IPV4) != 0U) ||
				((g_u4RxFilter & RX_FILTER_IPV6) != 0U)) {
				prAdapter->u4OsPacketFilter |=
					PARAM_PACKET_FILTER_MULTICAST;
			}
				u4rxfilter = prAdapter->u4OsPacketFilter;
				fgUpdate = TRUE;
			} else if (ucOpt0 == (UINT_8)SWCR_RX_FILTER_CMD_ADD) {
				if (ucOpt1 < 31U)
					g_u4RxFilter |=
					(UINT_32)1U << (UINT_32)ucOpt1;
			} else if (ucOpt0 ==
			(UINT_8)SWCR_RX_FILTER_CMD_REMOVE) {
				if (ucOpt1 < 31U)
					g_u4RxFilter &=
					~((UINT_32)1U << (UINT_32)ucOpt1);
			} else
				DBGLOG(INIT, TRACE,
				"Not handled case\n");

			if (fgUpdate == TRUE)
				rStatus = wlanoidSetPacketFilter(prAdapter,
					u4rxfilter, FALSE, NULL, 0);
			break;
		}

#if CFG_INIT_ENABLE_PATTERN_FILTER_ARP
		case (UINT_8)SWCTRL_RX_ARP_OFFLOAD:
			{
				WLAN_STATUS rStatus = WLAN_STATUS_SUCCESS;
				UINT_32 u4SetInfoLen = 0U;
				UINT_32 u4Len = OFFSET_OF(PARAM_NETWORK_ADDRESS_LIST, arAddress);
				UINT_32 u4NumIPv4 = 0U, u4NumIPv6 = 0U;
				UINT_32 i = 0U;
				PUINT_8 pucBufIpAddr = NULL;
				P_PARAM_NETWORK_ADDRESS_LIST prParamNetAddrList = NULL;
				P_PARAM_NETWORK_ADDRESS_IP prParamIpAddr = NULL;
				PUINT_8 pucIp = NULL;
				/* PUINT_8                         pucIpv6 = NULL; */
				UINT_32 bufSize = u4Len +
				(UINT_32)(OFFSET_OF(PARAM_NETWORK_ADDRESS,
				 aucAddress) + 3U *
				 (UINT_32)sizeof(PARAM_NETWORK_ADDRESS_IP));

				P_PARAM_NETWORK_ADDRESS prParamNetAddr = NULL;

				/* <1> allocate IP address buffer */
				pucBufIpAddr = kalMemAlloc(bufSize, VIR_MEM_TYPE);
				pucIp = kalMemAlloc(12U, VIR_MEM_TYPE);

				prParamNetAddrList = (P_PARAM_NETWORK_ADDRESS_LIST) pucBufIpAddr;
				prParamNetAddr = prParamNetAddrList->arAddress;
				/* <2> clear IP address buffer */
				kalMemZero(pucBufIpAddr, bufSize);
				kalMemZero(pucIp, 12U);

				/* <3> setup the number of IP address */
				if (ucOpt1 == 1U) {
					if ((wlanGetIPV4Address
						(prAdapter->prGlueInfo,
						pucIp, &u4NumIPv4) ==
						WLAN_STATUS_SUCCESS) &&
						(u4NumIPv4 > 3U))
						u4NumIPv4 = 3U;
				} else if (ucOpt1 == 0U) {
					u4NumIPv4 = u4NumIPv6 = 0U;
				}
				DBGLOG(INIT, INFO, "u4Len:%d bufSize:%d u4NumIPv4:%d\n", u4Len, bufSize, u4NumIPv4);

				prParamNetAddrList->u4AddressCount = u4NumIPv6 + u4NumIPv4;
				prParamNetAddrList->u2AddressType = PARAM_PROTOCOL_ID_TCP_IP;

				for (i = 0; i < u4NumIPv4; i++) {
					prParamNetAddr->u2AddressLength = sizeof(PARAM_NETWORK_ADDRESS_IP);
					prParamNetAddr->u2AddressType = PARAM_PROTOCOL_ID_TCP_IP;
					prParamIpAddr = (P_PARAM_NETWORK_ADDRESS_IP) prParamNetAddr->aucAddress;
					kalMemCopy(&prParamIpAddr->in_addr, pucIp + (i * 4), 4);
					prParamNetAddr =
					    (P_PARAM_NETWORK_ADDRESS) ((UINT_32) prParamNetAddr +
								       OFFSET_OF
								       (PARAM_NETWORK_ADDRESS,
									aucAddress) + sizeof(PARAM_NETWORK_ADDRESS_IP));
					u4Len +=
					    OFFSET_OF(PARAM_NETWORK_ADDRESS,
						      aucAddress) + sizeof(PARAM_NETWORK_ADDRESS_IP);
				}

				ASSERT(u4Len <= bufSize);

				rStatus = wlanoidSetNetworkAddress(prAdapter,
								   (PVOID) prParamNetAddrList, u4Len, &u4SetInfoLen);

				if (rStatus != WLAN_STATUS_SUCCESS)
					DBGLOG(INIT, INFO, "set HW packet filter fail 0x%1x\n", rStatus);

				if (pucIp != NULL)
					kalMemFree(pucIp, VIR_MEM_TYPE, 12U);
				if (pucBufIpAddr != NULL)
					kalMemFree(pucBufIpAddr, VIR_MEM_TYPE, bufSize);

			}
			break;
#endif
		case (UINT_8)SWCTRL_PS_DTIM_SKIP:
			break;
		case (UINT_8)SWCTRL_ROAMING:
			break;
		default:
			DBGLOG(SW4, LOUD, "Unreachable case!!\n");
			break;
		}
	} else {
		switch (ucIndex) {
		case (UINT_8)SWCTRL_DEBUG:
			break;
		case (UINT_8)SWCTRL_MAGIC:
			g_au4SwCR[1] = g_SWCTRL_MAGIC;
			break;
		case (UINT_8)SWCTRL_QM_INFO:
		{
			P_QUE_MGT_T prQM = &prAdapter->rQM;

			switch (ucOpt0) {
			case 0U:
#if QM_ADAPTIVE_TC_RESOURCE_CTRL
				g_au4SwCR[1] =
				(QM_GET_TX_QUEUE_LEN(prAdapter, ucOpt1));
				g_au4SwCR[2] =
					prQM->au4MinReservedTcResource[ucOpt1];
				g_au4SwCR[3] =
					prQM->au4CurrentTcResource[ucOpt1];
				g_au4SwCR[4] =
					prQM->au4GuaranteedTcResource[ucOpt1];
#endif
				break;

			case 1U:
#if QM_FORWARDING_FAIRNESS
				g_au4SwCR[1] =
					prQM->au4ResourceUsedCount[ucOpt1];
				g_au4SwCR[2] =
					prQM->au4HeadStaRecIndex[ucOpt1];
#endif
				break;

			case 2U:
				g_au4SwCR[1] =
				prQM->arTxQueue[ucOpt1].u4NumElem;

				break;
			default:
				DBGLOG(SW4, LOUD,
				"Unreachable case!!\n");
				break;
			}
			break;
		}
		case (UINT_8)SWCTRL_TX_CTRL_INFO:
		{
			P_TX_CTRL_T prTxCtrl;

			prTxCtrl = &prAdapter->rTxCtrl;

			if (ucOpt0 == 0U) {
			g_au4SwCR[1] =
			prAdapter->rTxCtrl.rTc.au2FreeBufferCount[ucOpt1];
			g_au4SwCR[2] =
			prAdapter->rTxCtrl.rTc.au2MaxNumOfBuffer[ucOpt1];
			}
			break;
		}
		case (UINT_8)SWCTRL_DUMP_QUEUE:
			dumpQueue(prAdapter);

			break;
#if QM_DEBUG_COUNTER
		case (UINT_8)SWCTRL_DUMP_QM_DBG_CNT:
			for (i = 0U; i < (UINT_32)QM_DBG_CNT_NUM; i++)
				DBGLOG(SW4, INFO, "QM:DBG %u %u\n", i, prAdapter->rQM.au4QmDebugCounters[i]);
			break;

		case (UINT_8)SWCTRL_QM_DBG_CNT:
			g_au4SwCR[1] =
				prAdapter->rQM.au4QmDebugCounters[ucOpt0];
			break;
#endif
		case (UINT_8)SWCTRL_DUMP_BSS:
		{
			dumpBss(prAdapter,
				GET_BSS_INFO_BY_INDEX(prAdapter, ucOpt0));
			break;
		}
		default:
			DBGLOG(SW4, LOUD, "Unreachable case!!\n");
			break;
		}

	}
}

VOID swCtrlCmdCategory1(P_ADAPTER_T prAdapter, UINT_8 ucCate, UINT_8 ucAction, UINT_8 ucOpt0, UINT_8 ucOpt1)
{
	UINT_8 ucIndex, ucRead;
	P_STA_RECORD_T prStaRec;

	DEBUGFUNC("swCtrlCmdCategory1");

	SWCR_GET_RW_INDEX(ucAction, ucRead, ucIndex);

	if (ucOpt0 >= CFG_STA_REC_NUM)
		return;

	/* prStaRec = cnmGetStaRecByIndex (prAdapter, ucOpt0); */
	prStaRec = &prAdapter->arStaRec[ucOpt0];
	if (ucRead == SWCR_WRITE) {
		/* ToDo:: Nothing */
	} else {
		/* Read */
		switch (ucIndex) {
		case (UINT_8)SWCTRL_STA_QUE_INFO:
		{
			g_au4SwCR[1] = prStaRec->arTxQueue[ucOpt1].u4NumElem;
			break;
		}
		case (UINT_8)SWCTRL_STA_INFO:
		{
			if (ucOpt1 == 0U)
				g_au4SwCR[1] = prStaRec->fgIsInPS;

			break;
		}
		case (UINT_8)SWCTRL_DUMP_STA:
		{
			dumpSTA(prAdapter, prStaRec);
			break;
		}
		default:
			DBGLOG(SW4, LOUD, "Unreachable case!!\n");
			break;
		}
	}
}

#if TEST_PS
static VOID
testPsSendQoSNullFrame(IN P_ADAPTER_T prAdapter,
		       IN P_STA_RECORD_T prStaRec,
		       IN UINT_8 ucUP,
		       IN UINT_8 ucBssIndex,
		       IN BOOLEAN fgBMC,
		       IN BOOLEAN fgIsBurstEnd, IN BOOLEAN ucPacketType, IN BOOLEAN ucPsSessionID, IN BOOLEAN fgSetEOSP)
{
	P_MSDU_INFO_T prMsduInfo;
	UINT_16 u2EstimatedFrameLen;
	P_WLAN_MAC_HEADER_QOS_T prQoSNullFrame;
	WLAN_STATUS rStatus = WLAN_STATUS_SUCCESS;

	DEBUGFUNC("testPsSendQoSNullFrame");
	DBGLOG(SW4, LOUD, "\n");

	/* 4 <1> Allocate a PKT_INFO_T for Null Frame */
	/* Init with MGMT Header Length */
	u2EstimatedFrameLen = MAC_TX_RESERVED_FIELD + WLAN_MAC_HEADER_QOS_LEN;

	/* Allocate a MSDU_INFO_T */

	prMsduInfo = cnmMgtPktAlloc(prAdapter, u2EstimatedFrameLen);
	if (prMsduInfo == NULL) {
		DBGLOG(SW4, WARN, "No PKT_INFO_T for sending Null Frame.\n");
		return;
	}
	/* 4 <2> Compose Null frame in MSDU_INfO_T. */
	bssComposeQoSNullFrame(prAdapter,
		(PUINT_8)(prMsduInfo->prPacket + MAC_TX_RESERVED_FIELD),
		prStaRec, ucUP, fgSetEOSP);

	TX_SET_MMPDU(prAdapter,
		     prMsduInfo,
		     ucBssIndex,
		     prStaRec->ucIndex,
		     WLAN_MAC_HEADER_QOS_LEN,
		     WLAN_MAC_HEADER_QOS_LEN,
		     NULL,
		     0U);

	prMsduInfo->ucUserPriority = ucUP;
	prMsduInfo->ucPacketType = ucPacketType;

	prQoSNullFrame = (P_WLAN_MAC_HEADER_QOS_T)
		(prMsduInfo->prPacket + MAC_TX_RESERVED_FIELD);

	if (fgBMC == TRUE)
		prQoSNullFrame->aucAddr1[0] = 0xfdU;
	else
		prQoSNullFrame->aucAddr1[5] = 0xddU;

	/* 4 <4> Inform TXM  to send this Null frame. */
	rStatus = nicTxEnqueueMsdu(prAdapter, prMsduInfo);

}

static VOID testPsSetupBss(IN P_ADAPTER_T prAdapter, IN UINT_8 ucBssIndex)
{
	P_BSS_INFO_T prBssInfo;
	UINT_8 aucZeroMacAddr[] = NULL_MAC_ADDR;

	DEBUGFUNC("testPsSetupBss()");
	DBGLOG(SW4, INFO, "index %d\n", ucBssIndex);

	prBssInfo = GET_BSS_INFO_BY_INDEX(prAdapter, ucBssIndex);

	/* 4 <1.2> Initiate PWR STATE */
	/* SET_NET_PWR_STATE_IDLE(prAdapter, ucNetworkTypeIndex); */

	/* 4 <2> Initiate BSS_INFO_T - common part */
	BSS_INFO_INIT(prAdapter, prBssInfo);

	prBssInfo->eConnectionState = PARAM_MEDIA_STATE_DISCONNECTED;
	prBssInfo->eConnectionStateIndicated = PARAM_MEDIA_STATE_DISCONNECTED;
	prBssInfo->eCurrentOPMode = OP_MODE_ACCESS_POINT;
	prBssInfo->fgIsNetActive = TRUE;
	prBssInfo->ucBssIndex = ucBssIndex;
	prBssInfo->ucReasonOfDisconnect = DISCONNECT_REASON_CODE_RESERVED;

	prBssInfo->ucPhyTypeSet = (UINT_8)PHY_TYPE_SET_802_11BG;
	prBssInfo->ucConfigAdHocAPMode = (UINT_8)AP_MODE_MIXED_11BG;
	prBssInfo->u2BSSBasicRateSet = (UINT_16)RATE_SET_ERP;
	prBssInfo->u2OperationalRateSet = (UINT_16)RATE_SET_OFDM;
	prBssInfo->fgErpProtectMode = FALSE;
	prBssInfo->fgIsQBSS = TRUE;

	/* 4 <1.5> Setup MIB for current BSS */
	prBssInfo->u2BeaconInterval = 100U;
	prBssInfo->ucDTIMPeriod = DOT11_DTIM_PERIOD_DEFAULT;
	prBssInfo->u2ATIMWindow = 0U;

	prBssInfo->ucBeaconTimeoutCount = 0U;

	bssInitForAP(prAdapter, prBssInfo, TRUE);

	COPY_MAC_ADDR(prBssInfo->aucBSSID, &aucZeroMacAddr[0]);
	LINK_INITIALIZE(&prBssInfo->rStaRecOfClientList);
	prBssInfo->fgIsBeaconActivated = TRUE;
	prBssInfo->u2HwDefaultFixedRateCode = RATE_CCK_1M_LONG;

	COPY_MAC_ADDR(prBssInfo->aucOwnMacAddr, prAdapter->rWifiVar.aucMacAddress);

	/* 4 <3> Initiate BSS_INFO_T - private part */
	/* TODO */
	prBssInfo->eBand = BAND_2G4;
	prBssInfo->ucPrimaryChannel = 1U;
	prBssInfo->prStaRecOfAP = NULL;

	/* prBssInfo->fgErpProtectMode =  eErpProectMode; */
	/* prBssInfo->eHtProtectMode = eHtProtectMode; */
	/* prBssInfo->eGfOperationMode = eGfOperationMode; */

	/* 4 <4> Allocate MSDU_INFO_T for Beacon */
	prBssInfo->prBeacon = cnmMgtPktAlloc(prAdapter,
		(UINT_32)OFFSET_OF(WLAN_BEACON_FRAME_T,
		aucInfoElem[0]) + MAX_IE_LENGTH);

	if (prBssInfo->prBeacon != NULL) {
		prBssInfo->prBeacon->eSrc = TX_PACKET_MGMT;
		prBssInfo->prBeacon->ucBssIndex = ucBssIndex;
	} else {
		DBGLOG(SW4, INFO, "prBeacon allocation fail\n");
	}

	prBssInfo->rPmProfSetupInfo.ucBmpDeliveryAC =
		(UINT_8)prAdapter->u4UapsdAcBmp;
	prBssInfo->rPmProfSetupInfo.ucBmpTriggerAC =
		(UINT_8)prAdapter->u4UapsdAcBmp;
	prBssInfo->rPmProfSetupInfo.ucUapsdSp =
		(UINT_8)prAdapter->u4MaxSpLen;

	DBGLOG(SW4, INFO, "[2] ucBmpDeliveryAC:0x%x, ucBmpTriggerAC:0x%x, ucUapsdSp:0x%x",
			   prBssInfo->rPmProfSetupInfo.ucBmpDeliveryAC,
			   prBssInfo->rPmProfSetupInfo.ucBmpTriggerAC, prBssInfo->rPmProfSetupInfo.ucUapsdSp);

}

VOID testPsCmdCategory0(P_ADAPTER_T prAdapter, UINT_8 ucCate, UINT_8 ucAction, UINT_8 ucOpt0, UINT_8 ucOpt1)
{
	UINT_8 ucIndex, ucRead;
	P_STA_RECORD_T prStaRec;
	WLAN_STATUS rStatus = WLAN_STATUS_SUCCESS;

	DEBUGFUNC("testPsCmdCategory0");
	SWCR_GET_RW_INDEX(ucAction, ucRead, ucIndex);

	DBGLOG(SW4, LOUD, "Read %u Index %u\n", ucRead, ucIndex);

	prStaRec = cnmGetStaRecByIndex(prAdapter, 0U);

	if (ucIndex >= (UINT_8)TEST_PS_CATA0_INDEX_NUM)
		return;

	if (ucRead == SWCR_WRITE) {
		switch (ucIndex) {
		case (UINT_8)TEST_PS_SETUP_BSS:
			testPsSetupBss(prAdapter, ucOpt0);
			break;

		case (UINT_8)TEST_PS_ENABLE_BEACON:
			break;

		case (UINT_8)TEST_PS_TRIGGER_BMC:
			/* txmForwardQueuedBmcPkts (ucOpt0); */
			break;
		case (UINT_8)TEST_PS_SEND_NULL:
			{
				testPsSendQoSNullFrame(prAdapter, prStaRec,
				(UINT_8)(g_au4SwCR[1] & 0xFFU), ucOpt0,
				(BOOLEAN)((g_au4SwCR[1] >> 8U) & 0xFFU),
				(BOOLEAN)((g_au4SwCR[1] >> 16U) & 0xFFU),
				(BOOLEAN)((g_au4SwCR[1] >> 24U) & 0xFFU),
				(UINT_8)((g_au4SwCR[2]) & 0xFFU), FALSE);
			}
			break;
		case (UINT_8)TEST_PS_BUFFER_BMC:
			/* g_aprBssInfo[ucOpt0]->fgApToBufferBMC = (g_au4SwCr[1] & 0xFF); */
			break;
		case (UINT_8)TEST_PS_UPDATE_BEACON:
			rStatus = bssUpdateBeaconContent(prAdapter, ucOpt0);
			break;
		default:
			DBGLOG(SW4, LOUD, "Unreachable case!!\n");
			break;
		}
	} else {
		if (ucIndex == (UINT_8)TEST_PS_MAGIC)
			g_au4SwCR[1] = 0x88660011U;
	}
}

#endif /* TEST_PS */

#if TEST_PS

VOID testPsCmdCategory1(P_ADAPTER_T prAdapter, UINT_8 ucCate, UINT_8 ucAction, UINT_8 ucOpt0, UINT_8 ucOpt1)
{
	UINT_8 ucIndex, ucRead;
	P_STA_RECORD_T prStaRec;

	DEBUGFUNC("testPsCmdCategory1");

	SWCR_GET_RW_INDEX(ucAction, ucRead, ucIndex);

	if (ucOpt0 >= CFG_STA_REC_NUM)
		return;

	prStaRec = cnmGetStaRecByIndex(prAdapter, ucOpt0);
	if (prStaRec == NULL)
		return;

	if (ucRead == SWCR_WRITE && ucIndex == (UINT_8)TEST_PS_STA_PS) {
		prStaRec->fgIsInPS =
			(BOOLEAN)(g_au4SwCR[1] & 0x1U);
		prStaRec->fgIsQoS =
			(BOOLEAN)(g_au4SwCR[1] >> 8U & 0xFFU);
		prStaRec->fgIsUapsdSupported =
			(BOOLEAN)(g_au4SwCR[1] >> 16U & 0xFFU);
		prStaRec->ucBmpDeliveryAC =
			(BOOLEAN)(g_au4SwCR[1] >> 24U & 0xFFU);
	}
}

#endif /* TEST_PS */

#if CFG_SUPPORT_802_11V
#if (CFG_SUPPORT_802_11V_TIMING_MEASUREMENT == 1) && (WNM_UNIT_TEST == 1)
VOID testWNMCmdCategory0(P_ADAPTER_T prAdapter, UINT_8 ucCate, UINT_8 ucAction, UINT_8 ucOpt0, UINT_8 ucOpt1)
{
	UINT_8 ucIndex, ucRead;
	P_STA_RECORD_T prStaRec;

	DEBUGFUNC("testWNMCmdCategory0");
	SWCR_GET_RW_INDEX(ucAction, ucRead, ucIndex);

	DBGLOG(SW4, INFO, "Read %u Index %u\n", ucRead, ucIndex);

	if (ucIndex >= (UINT_8)TEST_WNM_CATA0_INDEX_NUM)
		return;

	if (ucRead == SWCR_WRITE && ucIndex == (UINT_8)TEST_WNM_TIMING_MEAS)
		wnmTimingMeasUnitTest1(prAdapter, ucOpt0);
}
#endif /* TEST_WNM */
#endif /* CFG_SUPPORT_802_11V */

VOID swCtrlSwCr(P_ADAPTER_T prAdapter, UINT_8 ucRead, UINT_16 u2Addr, UINT_32 *pu4Data)
{
	/* According other register STAIDX */
	UINT_8 ucOffset;

	ucOffset = (UINT_8)(u2Addr >> 2U) & 0x3FU;

	if (ucOffset >= SWCR_CR_NUM)
		return;

	if (ucRead == SWCR_WRITE) {
		g_au4SwCR[ucOffset] = *pu4Data;
		if (ucOffset == 0x0U) {
			/* Commmand   [31:24]: Category */
			/* Commmand   [23:23]: 1(W) 0(R) */
			/* Commmand   [22:16]: Index */
			/* Commmand   [15:08]: Option0  */
			/* Commmand   [07:00]: Option1   */
			UINT_8 ucCate;
			UINT_32 u4Cmd;

			u4Cmd = g_au4SwCR[0];
			ucCate = (UINT_8) (u4Cmd >> 24U);
			if (ucCate < ARRAY_SIZE(g_arSwCtrlCmd)) {
				if (g_arSwCtrlCmd[ucCate] != NULL) {
					g_arSwCtrlCmd[ucCate] (prAdapter, ucCate,
						(UINT_8)(u4Cmd >> 16U & 0xFFU),
						(UINT_8)((u4Cmd >> 8U) & 0xFFU),
						(UINT_8)(u4Cmd & 0xFFU));
				}
			}
		}
	} else {
		*pu4Data = g_au4SwCR[ucOffset];
	}
}

VOID swCrReadWriteCmd(P_ADAPTER_T prAdapter, UINT_8 ucRead, UINT_16 u2Addr, UINT_32 *pu4Data)
{
	UINT_8 ucMod;

	ucMod = (UINT_8)(u2Addr >> 8U);
	/* Address [15:8] MOD ID */
	/* Address [7:0] OFFSET */

	DEBUGFUNC("swCrReadWriteCmd");
	DBGLOG(SW4, TRACE, "%u addr 0x%x data 0x%x\n", ucRead, u2Addr, *pu4Data);

	if (ucMod < ARRAY_SIZE(g_arSwCrModHander)) {
		if (g_arSwCrModHander[ucMod] != NULL)
			g_arSwCrModHander[ucMod](prAdapter,
			ucRead, u2Addr, pu4Data);
	}			/* ucMod */
}

/* Debug Support */
VOID swCrFrameCheckEnable(P_ADAPTER_T prAdapter, UINT_32 u4DumpType)
{
	g_u4SwcrDebugFrameDumpType = u4DumpType;
#if CFG_RX_PKTS_DUMP
	prAdapter->rRxCtrl.u4RxPktsDumpTypeMask = u4DumpType;
#endif
}

VOID swCrDebugInit(P_ADAPTER_T prAdapter)
{
	/* frame dump */
	if (g_u4SwcrDebugFrameDumpType != 0U)
		swCrFrameCheckEnable(prAdapter, g_u4SwcrDebugFrameDumpType);
	/* debug counter */
	g_fgSwcrDebugTimer = FALSE;

	cnmTimerInitTimer(prAdapter, &g_rSwcrDebugTimer,
		(PFN_MGMT_TIMEOUT_FUNC)swCrDebugCheckTimeout, 0x00000000UL);

	if (g_u4SwcrDebugCheckTimeout != 0U)
		swCrDebugCheckEnable(prAdapter, TRUE,
		(UINT_8)g_ucSwcrDebugCheckType, g_u4SwcrDebugCheckTimeout);
}

VOID swCrDebugUninit(P_ADAPTER_T prAdapter)
{
	cnmTimerStopTimer(prAdapter, &g_rSwcrDebugTimer);

	g_fgSwcrDebugTimer = FALSE;
}

VOID swCrDebugCheckEnable(P_ADAPTER_T prAdapter, BOOLEAN fgIsEnable, UINT_8 ucType, UINT_32 u4Timeout)
{
	if (fgIsEnable == TRUE) {
		g_ucSwcrDebugCheckType = (ENUM_SWCR_DBG_TYPE_T)ucType;
		g_u4SwcrDebugCheckTimeout = u4Timeout;
		if (g_fgSwcrDebugTimer == FALSE)
			swCrDebugCheckTimeout(prAdapter, 0UL);
	} else {
		cnmTimerStopTimer(prAdapter, &g_rSwcrDebugTimer);
		g_u4SwcrDebugCheckTimeout = 0U;
	}

	g_fgSwcrDebugTimer = fgIsEnable;
}

VOID swCrDebugCheck(P_ADAPTER_T prAdapter, P_CMD_SW_DBG_CTRL_T prCmdSwCtrl)
{
	P_RX_CTRL_T prRxCtrl;
	P_TX_CTRL_T prTxCtrl;

	ASSERT(prAdapter);

	prTxCtrl = &prAdapter->rTxCtrl;
	prRxCtrl = &prAdapter->rRxCtrl;

	/* dump counters */
	if (prCmdSwCtrl != NULL) {
		if (prCmdSwCtrl->u4Data == (UINT_32)SWCR_DBG_TYPE_ALL) {

			/* TX Counter from fw */
			DBGLOG(SW4, INFO, "TX0\n"
					   "%08x %08x %08x %08x\n"
					   "%08x %08x %08x %08x\n",
			prCmdSwCtrl->u4DebugCnt
				[(UINT_32)SWCR_DBG_ALL_TX_CNT],
			prCmdSwCtrl->u4DebugCnt
				[(UINT_32)SWCR_DBG_ALL_TX_BCN_CNT],
			prCmdSwCtrl->u4DebugCnt
				[(UINT_32)SWCR_DBG_ALL_TX_FAILED_CNT],
			prCmdSwCtrl->u4DebugCnt
				[(UINT_32)SWCR_DBG_ALL_TX_RETRY_CNT],
			prCmdSwCtrl->u4DebugCnt
				[(UINT_32)SWCR_DBG_ALL_TX_AGING_TIMEOUT_CNT],
			prCmdSwCtrl->u4DebugCnt
				[(UINT_32)SWCR_DBG_ALL_TX_PS_OVERFLOW_CNT],
			prCmdSwCtrl->u4DebugCnt
				[(UINT_32)SWCR_DBG_ALL_TX_MGNT_DROP_CNT],
			prCmdSwCtrl->u4DebugCnt
				[(UINT_32)SWCR_DBG_ALL_TX_ERROR_CNT]);
#if 1
			/* TX Counter from drv */
			DBGLOG(SW4, INFO, "TX1\n"
					   "%08x %08x %08x %08x\n",
			(UINT_32) TX_GET_CNT(prTxCtrl,
				(UINT_32)TX_INACTIVE_BSS_DROP),
			(UINT_32) TX_GET_CNT(prTxCtrl,
				(UINT_32)TX_INACTIVE_STA_DROP),
			(UINT_32) TX_GET_CNT(prTxCtrl,
				(UINT_32)TX_FORWARD_OVERFLOW_DROP),
			(UINT_32) TX_GET_CNT(prTxCtrl,
				(UINT_32)TX_AP_BORADCAST_DROP));
#endif

			/* RX Counter */
			DBGLOG(SW4, INFO, "RX0\n"
					   "%08x %08x %08x %08x\n"
					   "%08x %08x %08x %08x\n"
					   "%08x %08x %08x %08x\n"
					   "%08x %08x %08x %08x\n",
			prCmdSwCtrl->u4DebugCnt
				[(UINT_32)SWCR_DBG_ALL_RX_CNT],
			prCmdSwCtrl->u4DebugCnt
				[(UINT_32)SWCR_DBG_ALL_RX_DROP_CNT],
			prCmdSwCtrl->u4DebugCnt
				[(UINT_32)SWCR_DBG_ALL_RX_DUP_DROP_CNT],
			prCmdSwCtrl->u4DebugCnt
				[(UINT_32)SWCR_DBG_ALL_RX_TYPE_ERROR_DROP_CNT],
			prCmdSwCtrl->u4DebugCnt
				[(UINT_32)SWCR_DBG_ALL_RX_CLASS_ERROR_DROP_CNT],
			prCmdSwCtrl->u4DebugCnt
				[(UINT_32)SWCR_DBG_ALL_RX_AMPDU_ERROR_DROP_CNT],
			prCmdSwCtrl->u4DebugCnt
			[(UINT_32)SWCR_DBG_ALL_RX_STATUS_ERROR_DROP_CNT],
			prCmdSwCtrl->u4DebugCnt
			[(UINT_32)SWCR_DBG_ALL_RX_FORMAT_ERROR_DROP_CNT],
			prCmdSwCtrl->u4DebugCnt
				[(UINT_32)SWCR_DBG_ALL_RX_ICV_ERROR_DROP_CNT],
			prCmdSwCtrl->u4DebugCnt
				[(UINT_32)SWCR_DBG_ALL_RX_KEY_ERROR_DROP_CNT],
			prCmdSwCtrl->u4DebugCnt
				[(UINT_32)SWCR_DBG_ALL_RX_TKIP_ERROR_DROP_CNT],
			prCmdSwCtrl->u4DebugCnt
				[(UINT_32)SWCR_DBG_ALL_RX_MIC_ERROR_DROP_CNT],
			prCmdSwCtrl->u4DebugCnt
				[(UINT_32)SWCR_DBG_ALL_RX_BIP_ERROR_DROP_CNT],
			prCmdSwCtrl->u4DebugCnt
				[(UINT_32)SWCR_DBG_ALL_RX_FCSERR_CNT],
			prCmdSwCtrl->u4DebugCnt
				[(UINT_32)SWCR_DBG_ALL_RX_FIFOFULL_CNT],
			prCmdSwCtrl->u4DebugCnt
				[(UINT_32)SWCR_DBG_ALL_RX_PFDROP_CNT]);

			DBGLOG(SW4, INFO, "RX1\n"
					   "%08x %08x %08x %08x\n"
					   "%08x %08x %08x %08x\n",
			(UINT_32) RX_GET_CNT(prRxCtrl,
				(UINT_32)RX_MPDU_TOTAL_COUNT),
			(UINT_32) RX_GET_CNT(prRxCtrl,
				(UINT_32)RX_DATA_INDICATION_COUNT),
			(UINT_32) RX_GET_CNT(prRxCtrl,
				(UINT_32)RX_DATA_RETURNED_COUNT),
			(UINT_32) RX_GET_CNT(prRxCtrl,
				(UINT_32)RX_DATA_RETAINED_COUNT),
			(UINT_32) RX_GET_CNT(prRxCtrl,
				(UINT_32)RX_DROP_TOTAL_COUNT),
			(UINT_32) RX_GET_CNT(prRxCtrl,
				(UINT_32)RX_TYPE_ERR_DROP_COUNT),
			(UINT_32) RX_GET_CNT(prRxCtrl,
				(UINT_32)RX_CLASS_ERR_DROP_COUNT),
			(UINT_32) RX_GET_CNT(prRxCtrl,
				(UINT_32)RX_DST_NULL_DROP_COUNT));

			DBGLOG(SW4, INFO, "PWR\n"
					   "%08x %08x %08x %08x\n"
					   "%08x %08x %08x %08x\n",
			prCmdSwCtrl->u4DebugCnt
				[(UINT_32)SWCR_DBG_ALL_PWR_PS_POLL_CNT],
			prCmdSwCtrl->u4DebugCnt
				[(UINT_32)SWCR_DBG_ALL_PWR_TRIGGER_NULL_CNT],
			prCmdSwCtrl->u4DebugCnt
				[(UINT_32)SWCR_DBG_ALL_PWR_BCN_IND_CNT],
			prCmdSwCtrl->u4DebugCnt
				[(UINT_32)SWCR_DBG_ALL_PWR_BCN_TIMEOUT_CNT],
			prCmdSwCtrl->u4DebugCnt
				[(UINT_32)SWCR_DBG_ALL_PWR_PM_STATE0],
			prCmdSwCtrl->u4DebugCnt
				[(UINT_32)SWCR_DBG_ALL_PWR_PM_STATE1],
			prCmdSwCtrl->u4DebugCnt
				[(UINT_32)SWCR_DBG_ALL_PWR_CUR_PS_PROF0],
			prCmdSwCtrl->u4DebugCnt
				[(UINT_32)SWCR_DBG_ALL_PWR_CUR_PS_PROF1]);

			DBGLOG(SW4, INFO, "ARM\n"
					   "%08x %08x %08x %08x\n"
					   "%08x %08x\n",
			prCmdSwCtrl->u4DebugCnt
				[(UINT_32)SWCR_DBG_ALL_AR_STA0_RATE],
			prCmdSwCtrl->u4DebugCnt
				[(UINT_32)SWCR_DBG_ALL_AR_STA0_BWGI],
			prCmdSwCtrl->u4DebugCnt
				[(UINT_32)SWCR_DBG_ALL_AR_STA0_RX_RATE_RCPI],
			prCmdSwCtrl->u4DebugCnt
				[(UINT_32)SWCR_DBG_ALL_ROAMING_ENABLE],
			prCmdSwCtrl->u4DebugCnt
				[(UINT_32)SWCR_DBG_ALL_ROAMING_ROAM_CNT],
			prCmdSwCtrl->u4DebugCnt
				[(UINT_32)SWCR_DBG_ALL_ROAMING_INT_CNT]);

			DBGLOG(SW4, INFO, "BB\n"
					   "%08x %08x %08x %08x\n"
					   "%08x %08x %08x %08x\n",
			prCmdSwCtrl->u4DebugCnt
				[(UINT_32)SWCR_DBG_ALL_BB_RX_MDRDY_CNT],
			prCmdSwCtrl->u4DebugCnt
				[(UINT_32)SWCR_DBG_ALL_BB_RX_FCSERR_CNT],
			prCmdSwCtrl->u4DebugCnt
				[(UINT_32)SWCR_DBG_ALL_BB_CCK_PD_CNT],
			prCmdSwCtrl->u4DebugCnt
				[(UINT_32)SWCR_DBG_ALL_BB_OFDM_PD_CNT],
			prCmdSwCtrl->u4DebugCnt
				[(UINT_32)SWCR_DBG_ALL_BB_CCK_SFDERR_CNT],
			prCmdSwCtrl->u4DebugCnt
				[(UINT_32)SWCR_DBG_ALL_BB_CCK_SIGERR_CNT],
			prCmdSwCtrl->u4DebugCnt
				[(UINT_32)SWCR_DBG_ALL_BB_OFDM_TAGERR_CNT],
			prCmdSwCtrl->u4DebugCnt
				[(UINT_32)SWCR_DBG_ALL_BB_OFDM_SIGERR_CNT]);

		}
	}
	/* start the next check */
	if (g_u4SwcrDebugCheckTimeout != 0U)
		cnmTimerStartTimer(prAdapter, &g_rSwcrDebugTimer, g_u4SwcrDebugCheckTimeout * MSEC_PER_SEC);
}

VOID swCrDebugCheckTimeout(IN P_ADAPTER_T prAdapter, ULONG ulParamPtr)
{
	CMD_SW_DBG_CTRL_T rCmdSwCtrl;
	WLAN_STATUS rStatus;

	rCmdSwCtrl.u4Id = ((UINT_32)0xb000U << 16U) +
		(UINT_32)g_ucSwcrDebugCheckType;
	rCmdSwCtrl.u4Data = 0U;
	rStatus = wlanSendSetQueryCmd(prAdapter,	/* prAdapter */
				      (UINT_8)CMD_ID_SW_DBG_CTRL,
				      FALSE,	/* fgSetQuery */
				      TRUE,	/* fgNeedResp */
				      FALSE,	/* fgIsOid */
				      swCrDebugQuery,	/* pfCmdDoneHandler */
				      swCrDebugQueryTimeout,	/* pfCmdTimeoutHandler */
				      (UINT_32)sizeof(CMD_SW_DBG_CTRL_T),
				      (PUINT_8) &rCmdSwCtrl,	/* pucInfoBuffer */
				      NULL,	/* pvSetQueryBuffer */
				      0U	/* u4SetQueryBufferLen */
	    );

	ASSERT_BOOLEAN(rStatus == WLAN_STATUS_PENDING);

}

VOID swCrDebugQuery(IN P_ADAPTER_T prAdapter, IN P_CMD_INFO_T prCmdInfo, IN PUINT_8 pucEventBuf)
{
	ASSERT(prAdapter);

	swCrDebugCheck(prAdapter, (P_CMD_SW_DBG_CTRL_T)(pucEventBuf));
}

VOID swCrDebugQueryTimeout(IN P_ADAPTER_T prAdapter, IN P_CMD_INFO_T prCmdInfo)
{
	ASSERT(prAdapter);

	swCrDebugCheck(prAdapter, NULL);
}

#endif /* CFG_SUPPORT_SWCR */
