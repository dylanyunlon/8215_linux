//SPDX-License-Identifier: GPL-2.0
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
 * Id: /Department/DaVinci/BRANCHES/MT6620_WIFI_DRIVER_V2_3/mgmt/rlm.c#3
 */

/*
 *  ! \file   "rlm.c"
 *   \brief
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
BOOLEAN g_bCaptureDone = FALSE;
BOOLEAN g_bIcapEnable = FALSE;
UINT_16 g_u2DumpIndex;
static BOOLEAN g_fgHasChannelSwitchIE = FALSE;
#if CFG_SUPPORT_QA_TOOL
UINT_32 g_au4Offset[2][2];
UINT_32 g_au4IQData[20][1024];
UINT_32 g_au4I0Data[1][408000];
UINT_32 g_au4Q0Data[1][408000];
#endif
TIMER_T rBeaconReqTimer;
TIMER_T rTSMReqTimer;
/*******************************************************************************
*                                 M A C R O S
********************************************************************************
*/

/*******************************************************************************
*                   F U N C T I O N   D E C L A R A T I O N S
********************************************************************************
*/
static VOID rlmFillHtCapIE(P_ADAPTER_T prAdapter, P_BSS_INFO_T prBssInfo, P_MSDU_INFO_T prMsduInfo);

static VOID rlmFillExtCapIE(P_ADAPTER_T prAdapter, P_BSS_INFO_T prBssInfo, P_MSDU_INFO_T prMsduInfo);

static VOID rlmFillHtOpIE(P_ADAPTER_T prAdapter, P_BSS_INFO_T prBssInfo, P_MSDU_INFO_T prMsduInfo);

static UINT_8 rlmRecIeInfoForClient(P_ADAPTER_T prAdapter, P_BSS_INFO_T prBssInfo, PUINT_8 pucIE, UINT_16 u2IELength);

static BOOLEAN
rlmRecBcnFromNeighborForClient(P_ADAPTER_T prAdapter,
			       P_BSS_INFO_T prBssInfo, P_SW_RFB_T prSwRfb, PUINT_8 pucIE, UINT_16 u2IELength);

static BOOLEAN
rlmRecBcnInfoForClient(P_ADAPTER_T prAdapter,
		       P_BSS_INFO_T prBssInfo, P_SW_RFB_T prSwRfb, PUINT_8 pucIE, UINT_16 u2IELength);

static VOID rlmBssReset(P_ADAPTER_T prAdapter, P_BSS_INFO_T prBssInfo);

#if CFG_SUPPORT_802_11AC
static VOID rlmFillVhtCapIE(P_ADAPTER_T prAdapter, P_BSS_INFO_T prBssInfo, P_MSDU_INFO_T prMsduInfo);
static VOID rlmFillVhtOpNotificationIE(P_ADAPTER_T prAdapter, P_BSS_INFO_T prBssInfo,
		P_MSDU_INFO_T prMsduInfo, BOOLEAN fgIsMaxCap);

#endif

static UINT_8 rlmGetOpModeBwByVhtAndHtOpInfo(P_BSS_INFO_T prBssInfo);

static VOID rlmRecOpModeBwForClient(UINT_8 ucVhtOpModeChannelWidth,
				    P_BSS_INFO_T prBssInfo);
static VOID rlmCalibrateRepetions(struct RADIO_MEASUREMENT_REQ_PARAMS *prRmReq);


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
VOID rlmFsmEventInit(P_ADAPTER_T prAdapter)
{
	ASSERT(prAdapter);

	/* Note: assume TIMER_T structures are reset to zero or stopped
	 * before invoking this function.
	 */

	/* Initialize OBSS FSM */
	rlmObssInit(prAdapter);

#if CFG_SUPPORT_PWR_LIMIT_COUNTRY
	rlmDomainCheckCountryPowerLimitTable(prAdapter);
#endif

	g_fgHasChannelSwitchIE = FALSE;
	g_bCaptureDone = FALSE;
	g_bIcapEnable = FALSE;
	g_u2DumpIndex = 0;
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
VOID rlmFsmEventUninit(P_ADAPTER_T prAdapter)
{
	P_BSS_INFO_T prBssInfo;
	UINT_8 i;

	ASSERT(prAdapter);

	for (i = 0; i < BSS_INFO_NUM; i++) {
		prBssInfo = prAdapter->aprBssInfo[i];

		/* Note: all RLM timers will also be stopped.
		 *       Now only one OBSS scan timer.
		 */
		rlmBssReset(prAdapter, prBssInfo);
	}
}

/*----------------------------------------------------------------------------*/
/*!
* \brief For probe request, association request
*
* \param[in]
*
* \return none
*/
/*----------------------------------------------------------------------------*/
VOID rlmReqGenerateHtCapIE(P_ADAPTER_T prAdapter, P_MSDU_INFO_T prMsduInfo)
{
	P_BSS_INFO_T prBssInfo;
	P_STA_RECORD_T prStaRec;

	ASSERT(prAdapter);
	ASSERT(prMsduInfo);

	prBssInfo = prAdapter->aprBssInfo[prMsduInfo->ucBssIndex];
	if (prBssInfo == NULL)
		return;

	prStaRec = cnmGetStaRecByIndex(prAdapter, prMsduInfo->ucStaRecIndex);

	if ((prAdapter->rWifiVar.ucAvailablePhyTypeSet &
		PHY_TYPE_SET_802_11N) != 0U &&
	    (prStaRec == NULL ||
	    (prStaRec->ucPhyTypeSet & PHY_TYPE_SET_802_11N) != 0U))
		rlmFillHtCapIE(prAdapter, prBssInfo, prMsduInfo);
}

/*----------------------------------------------------------------------------*/
/*!
* \brief For probe request, association request
*
* \param[in]
*
* \return none
*/
/*----------------------------------------------------------------------------*/
VOID rlmReqGenerateExtCapIE(P_ADAPTER_T prAdapter, P_MSDU_INFO_T prMsduInfo)
{
	P_BSS_INFO_T prBssInfo;
	P_STA_RECORD_T prStaRec;

	ASSERT(prAdapter);
	ASSERT(prMsduInfo);

	prBssInfo = prAdapter->aprBssInfo[prMsduInfo->ucBssIndex];
	if (prBssInfo == NULL)
		return;

	prStaRec = cnmGetStaRecByIndex(prAdapter, prMsduInfo->ucStaRecIndex);

	if ((prAdapter->rWifiVar.ucAvailablePhyTypeSet &
		PHY_TYPE_SET_802_11N) != 0U &&
	    (prStaRec == NULL ||
	    (prStaRec->ucPhyTypeSet & PHY_TYPE_SET_802_11N) != 0U))
		rlmFillExtCapIE(prAdapter, prBssInfo, prMsduInfo);
#if CFG_SUPPORT_PASSPOINT
	else if (prAdapter->prGlueInfo->fgConnectHS20AP == TRUE)
		hs20FillExtCapIE(prAdapter, prBssInfo, prMsduInfo);
#endif /* CFG_SUPPORT_PASSPOINT */
}

/*----------------------------------------------------------------------------*/
/*!
* \brief For probe response (GO, IBSS) and association response
*
* \param[in]
*
* \return none
*/
/*----------------------------------------------------------------------------*/
VOID rlmRspGenerateHtCapIE(P_ADAPTER_T prAdapter, P_MSDU_INFO_T prMsduInfo)
{
	P_BSS_INFO_T prBssInfo;
	P_STA_RECORD_T prStaRec;
	UINT_8 ucPhyTypeSet;

	ASSERT(prAdapter);
	ASSERT(prMsduInfo);

	prBssInfo = prAdapter->aprBssInfo[prMsduInfo->ucBssIndex];
	if (prBssInfo == NULL)
		return;

	if (!IS_BSS_ACTIVE(prBssInfo))
		return;

	prStaRec = cnmGetStaRecByIndex(prAdapter, prMsduInfo->ucStaRecIndex);

	/* Decide PHY type set source */
	if (prStaRec != NULL) {
		/* Get PHY type set from target STA */
		ucPhyTypeSet = prStaRec->ucPhyTypeSet;
	} else {
		/* Get PHY type set from current BSS */
		ucPhyTypeSet = prBssInfo->ucPhyTypeSet;
	}

	if (RLM_NET_IS_11N(prBssInfo) &&
		((ucPhyTypeSet & PHY_TYPE_SET_802_11N) != 0U))
		rlmFillHtCapIE(prAdapter, prBssInfo, prMsduInfo);
}

/*----------------------------------------------------------------------------*/
/*!
* \brief For probe response (GO, IBSS) and association response
*
* \param[in]
*
* \return none
*/
/*----------------------------------------------------------------------------*/
VOID rlmRspGenerateExtCapIE(P_ADAPTER_T prAdapter, P_MSDU_INFO_T prMsduInfo)
{
	P_BSS_INFO_T prBssInfo;
	P_STA_RECORD_T prStaRec;
	UINT_8 ucPhyTypeSet;

	ASSERT(prAdapter);
	ASSERT(prMsduInfo);

	prBssInfo = prAdapter->aprBssInfo[prMsduInfo->ucBssIndex];
	if (prBssInfo == NULL)
		return;

	if (!IS_BSS_ACTIVE(prBssInfo))
		return;

	prStaRec = cnmGetStaRecByIndex(prAdapter, prMsduInfo->ucStaRecIndex);

	/* Decide PHY type set source */
	if (prStaRec != NULL) {
		/* Get PHY type set from target STA */
		ucPhyTypeSet = prStaRec->ucPhyTypeSet;
	} else {
		/* Get PHY type set from current BSS */
		ucPhyTypeSet = prBssInfo->ucPhyTypeSet;
	}

	if (RLM_NET_IS_11N(prBssInfo) &&
		((ucPhyTypeSet & PHY_TYPE_SET_802_11N) != 0U))
		rlmFillExtCapIE(prAdapter, prBssInfo, prMsduInfo);
}

void rlmRspGenerateInterworkingIE(P_ADAPTER_T prAdapter,
				P_MSDU_INFO_T prMsduInfo)
{
	P_BSS_INFO_T prBssInfo;
	P_IE_Interworking_T prInterworkingIe;
	ASSERT(prAdapter);
	ASSERT(prMsduInfo);

	prBssInfo = prAdapter->aprBssInfo[prMsduInfo->ucBssIndex];
	if (!prBssInfo)
		return;
	if (!IS_BSS_ACTIVE(prBssInfo))
		return;
	prInterworkingIe = (P_IE_Interworking_T)
					(((UINT_8 *)prMsduInfo->prPacket) +
					prMsduInfo->u2FrameLength);

	if (prBssInfo->ucLength == 0)
		return;

		/* Add Interworking IE */
	prInterworkingIe->ucId = ELEM_ID_INTERWORKING;
	prInterworkingIe->ucLength = prBssInfo->ucLength;
	prInterworkingIe->ucInDaConn = prBssInfo->ucInDaConn;
	prInterworkingIe->ucVenueGroup = prBssInfo->ucVenueGroup;
	prInterworkingIe->ucVenueType = prBssInfo->ucVenueType;
	prMsduInfo->u2FrameLength += IE_SIZE(prInterworkingIe);
}

/*----------------------------------------------------------------------------*/
/*!
* \brief For probe response (GO, IBSS) and association response
*
* \param[in]
*
* \return none
*/
/*----------------------------------------------------------------------------*/
VOID rlmRspGenerateHtOpIE(P_ADAPTER_T prAdapter, P_MSDU_INFO_T prMsduInfo)
{
	P_BSS_INFO_T prBssInfo;
	P_STA_RECORD_T prStaRec;
	UINT_8 ucPhyTypeSet;

	ASSERT(prAdapter);
	ASSERT(prMsduInfo);

	prStaRec = cnmGetStaRecByIndex(prAdapter, prMsduInfo->ucStaRecIndex);

	prBssInfo = prAdapter->aprBssInfo[prMsduInfo->ucBssIndex];
	if (prBssInfo == NULL)
		return;

	if (!IS_BSS_ACTIVE(prBssInfo))
		return;

	/* Decide PHY type set source */
	if (prStaRec != NULL) {
		/* Get PHY type set from target STA */
		ucPhyTypeSet = prStaRec->ucPhyTypeSet;
	} else {
		/* Get PHY type set from current BSS */
		ucPhyTypeSet = prBssInfo->ucPhyTypeSet;
	}

	if (RLM_NET_IS_11N(prBssInfo) &&
		((ucPhyTypeSet & PHY_TYPE_SET_802_11N) != 0U))
		rlmFillHtOpIE(prAdapter, prBssInfo, prMsduInfo);
}

/*----------------------------------------------------------------------------*/
/*!
* \brief For probe response (GO, IBSS) and association response
*
* \param[in]
*
* \return none
*/
/*----------------------------------------------------------------------------*/
VOID rlmRspGenerateErpIE(P_ADAPTER_T prAdapter, P_MSDU_INFO_T prMsduInfo)
{
	P_BSS_INFO_T prBssInfo;
	P_STA_RECORD_T prStaRec;
	P_IE_ERP_T prErpIe;
	UINT_8 ucPhyTypeSet;

	ASSERT(prAdapter);
	ASSERT(prMsduInfo);

	prStaRec = cnmGetStaRecByIndex(prAdapter, prMsduInfo->ucStaRecIndex);

	prBssInfo = prAdapter->aprBssInfo[prMsduInfo->ucBssIndex];
	if (prBssInfo == NULL)
		return;

	if (!IS_BSS_ACTIVE(prBssInfo))
		return;

	/* Decide PHY type set source */
	if (prStaRec != NULL) {
		/* Get PHY type set from target STA */
		ucPhyTypeSet = prStaRec->ucPhyTypeSet;
	} else {
		/* Get PHY type set from current BSS */
		ucPhyTypeSet = prBssInfo->ucPhyTypeSet;
	}

	if (RLM_NET_IS_11GN(prBssInfo) &&
		prBssInfo->eBand == BAND_2G4 &&
		((ucPhyTypeSet & PHY_TYPE_SET_802_11GN) != 0U)) {
		prErpIe = (P_IE_ERP_T)
		    (((PUINT_8) prMsduInfo->prPacket) + prMsduInfo->u2FrameLength);

		/* Add ERP IE */
		prErpIe->ucId = ELEM_ID_ERP_INFO;
		prErpIe->ucLength = 1;

		prErpIe->ucERP = prBssInfo->fgObssErpProtectMode == TRUE ?
			(UINT_8)ERP_INFO_USE_PROTECTION : 0U;

		if (prBssInfo->fgErpProtectMode == TRUE)
			prErpIe->ucERP |= (UINT_8)
			(ERP_INFO_NON_ERP_PRESENT | ERP_INFO_USE_PROTECTION);

		/* Handle barker preamble */
		if (prBssInfo->fgUseShortPreamble == FALSE)
			prErpIe->ucERP |= (UINT_8)ERP_INFO_BARKER_PREAMBLE_MODE;

		ASSERT_BOOLEAN(IE_SIZE(prErpIe)
			<= (ELEM_HDR_LEN + ELEM_MAX_LEN_ERP));

		prMsduInfo->u2FrameLength += (UINT_16)IE_SIZE(prErpIe);
	}
}

#if CFG_SUPPORT_MTK_SYNERGY
/*----------------------------------------------------------------------------*/
/*!
* \brief This function is used to generate MTK Vendor Specific OUI
*
* \param[in]
*
* \return none
*/
/*----------------------------------------------------------------------------*/
VOID rlmGenerateMTKOuiIE(P_ADAPTER_T prAdapter, P_MSDU_INFO_T prMsduInfo)
{
	P_BSS_INFO_T prBssInfo;
	PUINT_8 pucBuffer;
	UINT_8 aucMtkOui[] = VENDOR_OUI_MTK;

	ASSERT(prAdapter);
	ASSERT(prMsduInfo);

	if (prAdapter->rWifiVar.ucMtkOui == (UINT_8)FEATURE_DISABLED)
		return;

	prBssInfo = prAdapter->aprBssInfo[prMsduInfo->ucBssIndex];
	if (prBssInfo == NULL)
		return;

	pucBuffer = (PUINT_8) ((ULONG) prMsduInfo->prPacket + (ULONG) prMsduInfo->u2FrameLength);

	MTK_OUI_IE(pucBuffer)->ucId = ELEM_ID_VENDOR;
	MTK_OUI_IE(pucBuffer)->ucLength = ELEM_MIN_LEN_MTK_OUI;
	MTK_OUI_IE(pucBuffer)->aucOui[0] = aucMtkOui[0];
	MTK_OUI_IE(pucBuffer)->aucOui[1] = aucMtkOui[1];
	MTK_OUI_IE(pucBuffer)->aucOui[2] = aucMtkOui[2];
	MTK_OUI_IE(pucBuffer)->aucCapability[0] = (UINT_8)MTK_SYNERGY_CAP0 &
		(prAdapter->rWifiVar.aucMtkFeature[0]);
	MTK_OUI_IE(pucBuffer)->aucCapability[1] = MTK_SYNERGY_CAP1 & (prAdapter->rWifiVar.aucMtkFeature[1]);
	MTK_OUI_IE(pucBuffer)->aucCapability[2] = MTK_SYNERGY_CAP2 & (prAdapter->rWifiVar.aucMtkFeature[2]);
	MTK_OUI_IE(pucBuffer)->aucCapability[3] = MTK_SYNERGY_CAP3 & (prAdapter->rWifiVar.aucMtkFeature[3]);

	prMsduInfo->u2FrameLength += (UINT_16)IE_SIZE(pucBuffer);
	pucBuffer += IE_SIZE(pucBuffer);
}				/* rlmGenerateMTKOuiIE */

/*----------------------------------------------------------------------------*/
/*!
* @brief This function is used to check MTK Vendor Specific OUI
*
*
* @return true:  correct MTK OUI
*             false: incorrect MTK OUI
*/
/*----------------------------------------------------------------------------*/
BOOLEAN rlmParseCheckMTKOuiIE(IN P_ADAPTER_T prAdapter, IN PUINT_8 pucBuf, IN PUINT_32 pu4Cap)
{
	UINT_8 aucMtkOui[] = VENDOR_OUI_MTK;
	P_IE_MTK_OUI_T prMtkOuiIE = (P_IE_MTK_OUI_T) NULL;

	ASSERT_BREAK((prAdapter != NULL) && (pucBuf != NULL));

	prMtkOuiIE = (P_IE_MTK_OUI_T) pucBuf;

	if (prAdapter->rWifiVar.ucMtkOui == (UINT_8)FEATURE_DISABLED)
		return FALSE;
	if (IE_LEN(pucBuf) < ELEM_MIN_LEN_MTK_OUI ||
		prMtkOuiIE->aucOui[0] != aucMtkOui[0] ||
		prMtkOuiIE->aucOui[1] != aucMtkOui[1] ||
		prMtkOuiIE->aucOui[2] != aucMtkOui[2])
		return FALSE;

	/* apply NvRam setting */
	prMtkOuiIE->aucCapability[0] = prMtkOuiIE->aucCapability[0] &
		(prAdapter->rWifiVar.aucMtkFeature[0]);
	prMtkOuiIE->aucCapability[1] = prMtkOuiIE->aucCapability[1] &
		(prAdapter->rWifiVar.aucMtkFeature[1]);
	prMtkOuiIE->aucCapability[2] = prMtkOuiIE->aucCapability[2] &
		(prAdapter->rWifiVar.aucMtkFeature[2]);
	prMtkOuiIE->aucCapability[3] = prMtkOuiIE->aucCapability[3] &
		(prAdapter->rWifiVar.aucMtkFeature[3]);

	kalMemCopy(pu4Cap, prMtkOuiIE->aucCapability, sizeof(UINT_32));

	return TRUE;
}				/* rlmParseCheckMTKOuiIE */

#endif

/*----------------------------------------------------------------------------*/
/*!
* \brief
*
* \param[in]
*
* \return none
*/
/*----------------------------------------------------------------------------*/
static VOID rlmFillHtCapIE(P_ADAPTER_T prAdapter, P_BSS_INFO_T prBssInfo, P_MSDU_INFO_T prMsduInfo)
{
	P_IE_HT_CAP_T prHtCap;
	P_SUP_MCS_SET_FIELD prSupMcsSet;
	BOOLEAN fg40mAllowed;

	ASSERT(prAdapter);
	ASSERT(prBssInfo);
	ASSERT(prMsduInfo);

	fg40mAllowed = prBssInfo->fgAssoc40mBwAllowed;

	prHtCap = (P_IE_HT_CAP_T)
	    (((PUINT_8) prMsduInfo->prPacket) + prMsduInfo->u2FrameLength);

	/* Add HT capabilities IE */
	prHtCap->ucId = ELEM_ID_HT_CAP;
	prHtCap->ucLength = (UINT_8)sizeof(IE_HT_CAP_T) - ELEM_HDR_LEN;

	prHtCap->u2HtCapInfo = (UINT_16)HT_CAP_INFO_DEFAULT_VAL;

	if (IS_FEATURE_ENABLED(prAdapter->rWifiVar.ucRxShortGI))
		prHtCap->u2HtCapInfo |= (UINT_16)
		(HT_CAP_INFO_SHORT_GI_20M | HT_CAP_INFO_SHORT_GI_40M);

	if (IS_FEATURE_ENABLED(prAdapter->rWifiVar.ucRxLdpc))
		prHtCap->u2HtCapInfo |= (UINT_16)HT_CAP_INFO_LDPC_CAP;

	if (IS_FEATURE_ENABLED(prAdapter->rWifiVar.ucRxStbc))
		prHtCap->u2HtCapInfo |= (UINT_16)HT_CAP_INFO_RX_STBC_1_SS;

	if (IS_FEATURE_ENABLED(prAdapter->rWifiVar.ucRxGf))
		prHtCap->u2HtCapInfo |= (UINT_16)HT_CAP_INFO_HT_GF;

	if (fg40mAllowed == FALSE)
		prHtCap->u2HtCapInfo &= (UINT_16)~(HT_CAP_INFO_SUP_CHNL_WIDTH |
					  HT_CAP_INFO_SHORT_GI_40M | HT_CAP_INFO_DSSS_CCK_IN_40M);

	prHtCap->ucAmpduParam = (UINT_8)AMPDU_PARAM_DEFAULT_VAL;

	prSupMcsSet = &prHtCap->rSupMcsSet;
	kalMemZero((PVOID)&prSupMcsSet->aucRxMcsBitmask[0], SUP_MCS_RX_BITMASK_OCTET_NUM);

	prSupMcsSet->aucRxMcsBitmask[0] = (UINT_8)BITS(0U, 7U);

	if (fg40mAllowed == TRUE)
		prSupMcsSet->aucRxMcsBitmask[32 / 8] = (UINT_8)BIT(0U);
	prSupMcsSet->u2RxHighestSupportedRate = SUP_MCS_RX_DEFAULT_HIGHEST_RATE;
	prSupMcsSet->u4TxRateInfo = (UINT_32)SUP_MCS_TX_DEFAULT_VAL;

	prHtCap->u2HtExtendedCap = (UINT_16)HT_EXT_CAP_DEFAULT_VAL;
	if (fg40mAllowed == FALSE ||
		prBssInfo->eCurrentOPMode != OP_MODE_INFRASTRUCTURE)
		prHtCap->u2HtExtendedCap &= (UINT_16)
		~(HT_EXT_CAP_PCO | HT_EXT_CAP_PCO_TRANS_TIME_NONE);

	prHtCap->u4TxBeamformingCap = TX_BEAMFORMING_CAP_DEFAULT_VAL;
	if (IS_FEATURE_ENABLED(prAdapter->rWifiVar.ucStaHtBfee))
		prHtCap->u4TxBeamformingCap = (UINT_32)TX_BEAMFORMING_CAP_BFEE;

	prHtCap->ucAselCap = ASEL_CAP_DEFAULT_VAL;

	ASSERT_BOOLEAN(IE_SIZE(prHtCap)
		<= (ELEM_HDR_LEN + ELEM_MAX_LEN_HT_CAP));

	prMsduInfo->u2FrameLength += (UINT_16)IE_SIZE(prHtCap);
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
static VOID rlmFillExtCapIE(P_ADAPTER_T prAdapter, P_BSS_INFO_T prBssInfo, P_MSDU_INFO_T prMsduInfo)
{
	P_EXT_CAP_T prExtCap;
	BOOLEAN fg40mAllowed, fgAppendVhtCap;
	P_STA_RECORD_T prStaRec;

	ASSERT(prAdapter);
	ASSERT(prMsduInfo);

	fg40mAllowed = prBssInfo->fgAssoc40mBwAllowed;

	/* Add Extended Capabilities IE */
	prExtCap = (P_EXT_CAP_T)
	    (((PUINT_8) prMsduInfo->prPacket) + prMsduInfo->u2FrameLength);

	prExtCap->ucId = ELEM_ID_EXTENDED_CAP;
#if 0				/* CFG_SUPPORT_HOTSPOT_2_0 */
	if (prAdapter->prGlueInfo->fgConnectHS20AP == TRUE)
		prExtCap->ucLength = ELEM_MAX_LEN_EXT_CAP;
	else
#endif
		prExtCap->ucLength = 1;

	/* Reset memory */
	kalMemZero(prExtCap->aucCapabilities, ELEM_MAX_LEN_EXT_CAP);

	prExtCap->aucCapabilities[0] = (UINT_8)ELEM_EXT_CAP_DEFAULT_VAL;

	if (fg40mAllowed == FALSE)
		prExtCap->aucCapabilities[0] &= (UINT_8)
		~ELEM_EXT_CAP_20_40_COEXIST_SUPPORT;

	if (prBssInfo->eCurrentOPMode != OP_MODE_INFRASTRUCTURE)
		prExtCap->aucCapabilities[0] &= (UINT_8)~ELEM_EXT_CAP_PSMP_CAP;

	prStaRec = cnmGetStaRecByIndex(prAdapter, prMsduInfo->ucStaRecIndex);

#if CFG_SUPPORT_802_11AC
	fgAppendVhtCap = FALSE;

	/* Check append rule */
	if ((prAdapter->rWifiVar.ucAvailablePhyTypeSet
		& PHY_TYPE_SET_802_11AC) != 0U) {
		/* Note: For AIS connecting state, structure in BSS_INFO will not be inited */
		/*       So, we check StaRec instead of BssInfo */
		if (prStaRec != NULL) {
			if ((prStaRec->ucPhyTypeSet
				& PHY_TYPE_SET_802_11AC) != 0U)
				fgAppendVhtCap = TRUE;
		} else {
			if ((RLM_NET_IS_11AC(prBssInfo)) &&
				((prBssInfo->eCurrentOPMode
				== OP_MODE_INFRASTRUCTURE) ||
				(prBssInfo->eCurrentOPMode == OP_MODE_ACCESS_POINT)))
				fgAppendVhtCap = TRUE;
		}
	}

	if (fgAppendVhtCap == TRUE) {
		if (prExtCap->ucLength < ELEM_MAX_LEN_EXT_CAP)
			prExtCap->ucLength = ELEM_MAX_LEN_EXT_CAP;

		SET_EXT_CAP(prExtCap->aucCapabilities, ELEM_MAX_LEN_EXT_CAP, ELEM_EXT_CAP_OP_MODE_NOTIFICATION_BIT);

	}
#endif

#if CFG_SUPPORT_PASSPOINT
	if (prAdapter->prGlueInfo->fgConnectHS20AP == TRUE) {

		if (prExtCap->ucLength < ELEM_MAX_LEN_EXT_CAP)
			prExtCap->ucLength = ELEM_MAX_LEN_EXT_CAP;

		SET_EXT_CAP(prExtCap->aucCapabilities, ELEM_MAX_LEN_EXT_CAP, ELEM_EXT_CAP_INTERWORKING_BIT);

		/* For R2 WNM-Notification */
		SET_EXT_CAP(prExtCap->aucCapabilities, ELEM_MAX_LEN_EXT_CAP, ELEM_EXT_CAP_WNM_NOTIFICATION_BIT);
	}
#endif /* CFG_SUPPORT_PASSPOINT */

	ASSERT_BOOLEAN(IE_SIZE(prExtCap)
		<= (ELEM_HDR_LEN + ELEM_MAX_LEN_EXT_CAP));

	prMsduInfo->u2FrameLength += (UINT_16)IE_SIZE(prExtCap);
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
static VOID rlmFillHtOpIE(P_ADAPTER_T prAdapter, P_BSS_INFO_T prBssInfo, P_MSDU_INFO_T prMsduInfo)
{
	P_IE_HT_OP_T prHtOp;
	UINT_16 i;

	ASSERT(prAdapter);
	ASSERT(prBssInfo);
	ASSERT(prMsduInfo);

	prHtOp = (P_IE_HT_OP_T)
	    (((PUINT_8) prMsduInfo->prPacket) + prMsduInfo->u2FrameLength);

	/* Add HT operation IE */
	prHtOp->ucId = ELEM_ID_HT_OP;
	prHtOp->ucLength = (UINT_8)sizeof(IE_HT_OP_T) - ELEM_HDR_LEN;

	/* RIFS and 20/40 bandwidth operations are included */
	prHtOp->ucPrimaryChannel = prBssInfo->ucPrimaryChannel;
	prHtOp->ucInfo1 = prBssInfo->ucHtOpInfo1;

	/* Decide HT protection mode field */
	if (prBssInfo->eHtProtectMode == HT_PROTECT_MODE_NON_HT)
		prHtOp->u2Info2 = (UINT_16) HT_PROTECT_MODE_NON_HT;
	else if (prBssInfo->eObssHtProtectMode == HT_PROTECT_MODE_NON_MEMBER)
		prHtOp->u2Info2 = (UINT_16) HT_PROTECT_MODE_NON_MEMBER;
	else {
		/* It may be SYS_PROTECT_MODE_NONE or SYS_PROTECT_MODE_20M */
		prHtOp->u2Info2 = (UINT_16) prBssInfo->eHtProtectMode;
	}

	if (prBssInfo->eGfOperationMode != GF_MODE_NORMAL) {
		/* It may be GF_MODE_PROTECT or GF_MODE_DISALLOWED
		 * Note: it will also be set in ad-hoc network
		 */
		prHtOp->u2Info2 |= (UINT_16)HT_OP_INFO2_NON_GF_HT_STA_PRESENT;
	}

	prHtOp->u2Info3 = prBssInfo->u2HtOpInfo3;	/* To do: handle L-SIG TXOP */

	/* No basic MCSx are needed temporarily */
	for (i = 0; i < 16U; i++)
		prHtOp->aucBasicMcsSet[i] = 0;

	ASSERT_BOOLEAN(IE_SIZE(prHtOp) <= (ELEM_HDR_LEN + ELEM_MAX_LEN_HT_OP));

	prMsduInfo->u2FrameLength += (UINT_16)IE_SIZE(prHtOp);
}

#if CFG_SUPPORT_802_11AC

/*----------------------------------------------------------------------------*/
/*!
* \brief For probe request, association request
*
* \param[in]
*
* \return none
*/
/*----------------------------------------------------------------------------*/
VOID rlmReqGenerateVhtCapIE(P_ADAPTER_T prAdapter, P_MSDU_INFO_T prMsduInfo)
{
	P_BSS_INFO_T prBssInfo;
	P_STA_RECORD_T prStaRec;

	ASSERT(prAdapter);
	ASSERT(prMsduInfo);

	prBssInfo = prAdapter->aprBssInfo[prMsduInfo->ucBssIndex];
	if (prBssInfo == NULL)
		return;

	prStaRec = cnmGetStaRecByIndex(prAdapter, prMsduInfo->ucStaRecIndex);

	if (((prAdapter->rWifiVar.ucAvailablePhyTypeSet &
		PHY_TYPE_SET_802_11AC) != 0U) &&
	    (prStaRec == NULL ||
	    ((prStaRec->ucPhyTypeSet & PHY_TYPE_SET_802_11AC) != 0U)))
		rlmFillVhtCapIE(prAdapter, prBssInfo, prMsduInfo);
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief For probe response (GO, IBSS) and association response
 *
 * \param[in]
 *
 * \return none
 */
/*----------------------------------------------------------------------------*/
VOID rlmRspGenerateVhtCapIE(P_ADAPTER_T prAdapter, P_MSDU_INFO_T prMsduInfo)
{
	P_BSS_INFO_T prBssInfo;
	P_STA_RECORD_T prStaRec;
	UINT_8 ucPhyTypeSet;

	ASSERT(prAdapter);
	ASSERT(prMsduInfo);

	prBssInfo = prAdapter->aprBssInfo[prMsduInfo->ucBssIndex];
	if (prBssInfo == NULL)
		return;

	if (!IS_BSS_ACTIVE(prBssInfo))
		return;

	prStaRec = cnmGetStaRecByIndex(prAdapter, prMsduInfo->ucStaRecIndex);

	/* Decide PHY type set source */
	if (prStaRec != NULL) {
		/* Get PHY type set from target STA */
		ucPhyTypeSet = prStaRec->ucPhyTypeSet;
	} else {
		/* Get PHY type set from current BSS */
		ucPhyTypeSet = prBssInfo->ucPhyTypeSet;
	}

	if (RLM_NET_IS_11AC(prBssInfo) &&
		((ucPhyTypeSet & PHY_TYPE_SET_802_11AC) != 0U))
		rlmFillVhtCapIE(prAdapter, prBssInfo, prMsduInfo);

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
VOID rlmRspGenerateVhtOpIE(P_ADAPTER_T prAdapter, P_MSDU_INFO_T prMsduInfo)
{
	P_BSS_INFO_T prBssInfo;
	P_STA_RECORD_T prStaRec;
	UINT_8 ucPhyTypeSet;

	ASSERT(prAdapter);
	ASSERT(prMsduInfo);

	prStaRec = cnmGetStaRecByIndex(prAdapter, prMsduInfo->ucStaRecIndex);

	prBssInfo = prAdapter->aprBssInfo[prMsduInfo->ucBssIndex];
	if (prBssInfo == NULL)
		return;

	if (!IS_BSS_ACTIVE(prBssInfo))
		return;

	/* Decide PHY type set source */
	if (prStaRec != NULL) {
		/* Get PHY type set from target STA */
		ucPhyTypeSet = prStaRec->ucPhyTypeSet;
	} else {
		/* Get PHY type set from current BSS */
		ucPhyTypeSet = prBssInfo->ucPhyTypeSet;
	}

	if (RLM_NET_IS_11AC(prBssInfo) &&
		((ucPhyTypeSet & PHY_TYPE_SET_802_11AC) != 0U))
		rlmFillVhtOpIE(prAdapter, prBssInfo, prMsduInfo);
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
VOID rlmRspGenerateVhtOpNotificationIE(P_ADAPTER_T prAdapter, P_MSDU_INFO_T prMsduInfo)
{
	P_BSS_INFO_T prBssInfo;
	P_STA_RECORD_T prStaRec;
	UINT_8 ucPhyTypeSet;

	ASSERT(prAdapter);
	ASSERT(prMsduInfo);

	prStaRec = cnmGetStaRecByIndex(prAdapter, prMsduInfo->ucStaRecIndex);

	prBssInfo = prAdapter->aprBssInfo[prMsduInfo->ucBssIndex];
	if (!prBssInfo)
		return;

	if (!IS_BSS_ACTIVE(prBssInfo))
		return;

	/* Decide PHY type set source */
	if (prStaRec) {
		/* Get PHY type set from target STA */
		ucPhyTypeSet = prStaRec->ucPhyTypeSet;
	} else {
		/* Get PHY type set from current BSS */
		ucPhyTypeSet = prBssInfo->ucPhyTypeSet;
	}

	if (RLM_NET_IS_11AC(prBssInfo) && (ucPhyTypeSet & PHY_TYPE_SET_802_11AC))
		rlmFillVhtOpNotificationIE(prAdapter, prBssInfo, prMsduInfo, FALSE);
}

/*----------------------------------------------------------------------------*/
/*!
* \brief
* add VHT operation notification IE for VHT-BW40 case specific
* \param[in]
*
* \return none
*/
/*----------------------------------------------------------------------------*/
static VOID rlmFillVhtOpNotificationIE(P_ADAPTER_T prAdapter, P_BSS_INFO_T prBssInfo,
		P_MSDU_INFO_T prMsduInfo, BOOLEAN fgIsMaxCap)
{
	P_IE_VHT_OP_MODE_NOTIFICATION_T prVhtOpMode;
	UINT_8 ucMaxBw;

	ASSERT(prAdapter);
	ASSERT(prBssInfo);
	ASSERT(prMsduInfo);

	prVhtOpMode = (P_IE_VHT_OP_MODE_NOTIFICATION_T)
	    (((PUINT_8) prMsduInfo->prPacket) + prMsduInfo->u2FrameLength);

	kalMemZero((PVOID) prVhtOpMode, sizeof(IE_VHT_OP_MODE_NOTIFICATION_T));

	prVhtOpMode->ucId = ELEM_ID_OP_MODE;
	prVhtOpMode->ucLength = sizeof(IE_VHT_OP_MODE_NOTIFICATION_T) - ELEM_HDR_LEN;

	DBGLOG(RLM, INFO, "rlmFillVhtOpNotificationIE(%d) %u %u\n",
		prBssInfo->ucBssIndex, fgIsMaxCap, prBssInfo->ucNss);

	if (fgIsMaxCap) {

		ucMaxBw = cnmGetBssMaxBw(prAdapter, prBssInfo->ucBssIndex);

		/*handle 80P80 case*/
		if (ucMaxBw >= MAX_BW_160MHZ)
			ucMaxBw = MAX_BW_160MHZ;

		prVhtOpMode->ucOperatingMode |= ucMaxBw;

		prVhtOpMode->ucOperatingMode |=
			(((prBssInfo->ucNss-1) << VHT_OP_MODE_RX_NSS_OFFSET) & VHT_OP_MODE_RX_NSS);

	} else {

		switch (prBssInfo->ucVhtChannelWidth) {
		case VHT_OP_CHANNEL_WIDTH_80P80:
			ucMaxBw = MAX_BW_160MHZ;
			break;

		case VHT_OP_CHANNEL_WIDTH_160:
			ucMaxBw = MAX_BW_160MHZ;
			break;

		case VHT_OP_CHANNEL_WIDTH_80:
			ucMaxBw = MAX_BW_80MHZ;
			break;

		case VHT_OP_CHANNEL_WIDTH_20_40:
			{
				ucMaxBw = MAX_BW_20MHZ;

				if (prBssInfo->eBssSCO != CHNL_EXT_SCN)
					ucMaxBw = MAX_BW_40MHZ;
			}
			break;

		default:
			/*VHT default IE should support BW 80*/
			ucMaxBw = MAX_BW_80MHZ;
			break;
		}

		prVhtOpMode->ucOperatingMode |= ucMaxBw;

		prVhtOpMode->ucOperatingMode |= (((prBssInfo->ucNss-1)
			<< VHT_OP_MODE_RX_NSS_OFFSET) & VHT_OP_MODE_RX_NSS);
	}


	prMsduInfo->u2FrameLength += IE_SIZE(prVhtOpMode);

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
static VOID rlmFillVhtCapIE(P_ADAPTER_T prAdapter, P_BSS_INFO_T prBssInfo, P_MSDU_INFO_T prMsduInfo)
{
	P_IE_VHT_CAP_T prVhtCap;
	P_VHT_SUPPORTED_MCS_FIELD prVhtSupportedMcsSet;
	UINT_8 i;

	ASSERT(prAdapter);
	ASSERT(prBssInfo);
	ASSERT(prMsduInfo);

	prVhtCap = (P_IE_VHT_CAP_T)
	    (((PUINT_8) prMsduInfo->prPacket) + prMsduInfo->u2FrameLength);

	prVhtCap->ucId = ELEM_ID_VHT_CAP;
	prVhtCap->ucLength = (UINT_8)sizeof(IE_VHT_CAP_T) - ELEM_HDR_LEN;
	prVhtCap->u4VhtCapInfo = (UINT_32)VHT_CAP_INFO_DEFAULT_VAL;

	if (IS_FEATURE_ENABLED(prAdapter->rWifiVar.ucStaVhtBfee)) {
		prVhtCap->u4VhtCapInfo |= (UINT_32)FIELD_VHT_CAP_INFO_BF;
		if (IS_FEATURE_ENABLED(prAdapter->rWifiVar.ucStaVhtMuBfee))
			prVhtCap->u4VhtCapInfo |= (UINT_32)
			VHT_CAP_INFO_MU_BEAMFOMEE_CAPABLE;
	}

	if (IS_FEATURE_ENABLED(prAdapter->rWifiVar.ucRxShortGI))
		prVhtCap->u4VhtCapInfo |= (UINT_32)
		VHT_CAP_INFO_SHORT_GI_80;

	if (IS_FEATURE_ENABLED(prAdapter->rWifiVar.ucRxLdpc))
		prVhtCap->u4VhtCapInfo |= (UINT_32)
		VHT_CAP_INFO_RX_LDPC;

	if (IS_FEATURE_ENABLED(prAdapter->rWifiVar.ucRxStbc))
		prVhtCap->u4VhtCapInfo |= (UINT_32)
		VHT_CAP_INFO_RX_STBC_ONE_STREAM;

	if (IS_FEATURE_ENABLED(prAdapter->rWifiVar.ucTxStbc))
		prVhtCap->u4VhtCapInfo |= (UINT_32)
		VHT_CAP_INFO_TX_STBC;

	/*set MCS map */
	prVhtSupportedMcsSet = &prVhtCap->rVhtSupportedMcsSet;
	kalMemZero((PVOID) prVhtSupportedMcsSet, sizeof(VHT_SUPPORTED_MCS_FIELD));

	for (i = 0; i < 8U; i++) {
		prVhtSupportedMcsSet->u2RxMcsMap |= (UINT_16)
			BITS(2U * i, (2U * i + 1U));
		prVhtSupportedMcsSet->u2TxMcsMap |= (UINT_16)
			BITS(2U * i, (2U * i + 1U));
	}

	prVhtSupportedMcsSet->u2RxMcsMap &= (UINT_16)
		(VHT_CAP_INFO_MCS_MAP_MCS9 << VHT_CAP_INFO_MCS_1SS_OFFSET);
	prVhtSupportedMcsSet->u2TxMcsMap &= (UINT_16)
		(VHT_CAP_INFO_MCS_MAP_MCS9 << VHT_CAP_INFO_MCS_1SS_OFFSET);
	prVhtSupportedMcsSet->u2RxHighestSupportedDataRate = VHT_CAP_INFO_DEFAULT_HIGHEST_DATA_RATE;
	prVhtSupportedMcsSet->u2TxHighestSupportedDataRate = VHT_CAP_INFO_DEFAULT_HIGHEST_DATA_RATE;

	ASSERT_BOOLEAN(IE_SIZE(prVhtCap)
		<= (ELEM_HDR_LEN + ELEM_MAX_LEN_VHT_CAP));

	prMsduInfo->u2FrameLength += (UINT_16)IE_SIZE(prVhtCap);

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
VOID rlmFillVhtOpIE(P_ADAPTER_T prAdapter, P_BSS_INFO_T prBssInfo, P_MSDU_INFO_T prMsduInfo)
{
	P_IE_VHT_OP_T prVhtOp;

	ASSERT(prAdapter);
	ASSERT(prBssInfo);
	ASSERT(prMsduInfo);

	prVhtOp = (P_IE_VHT_OP_T)
	    (((PUINT_8) prMsduInfo->prPacket) + prMsduInfo->u2FrameLength);

	/* Add HT operation IE */
	prVhtOp->ucId = ELEM_ID_VHT_OP;
	prVhtOp->ucLength = (UINT_8)sizeof(IE_VHT_OP_T) - ELEM_HDR_LEN;

	ASSERT_BOOLEAN(IE_SIZE(prVhtOp)
		<= (ELEM_HDR_LEN + ELEM_MAX_LEN_VHT_OP));

	prVhtOp->ucVhtOperation[0] = prBssInfo->ucVhtChannelWidth;	/* (UINT8)VHT_OP_CHANNEL_WIDTH_80; */
	prVhtOp->ucVhtOperation[1] = prBssInfo->ucVhtChannelFrequencyS1;
	prVhtOp->ucVhtOperation[2] = prBssInfo->ucVhtChannelFrequencyS2;

	/*
	 * if(cnmGetBssMaxBw(prAdapter, prBssInfo->ucBssIndex) < MAX_BW_80MHZ) {
	 * prVhtOp->ucVhtOperation[0] = VHT_OP_CHANNEL_WIDTH_20_40;
	 * prVhtOp->ucVhtOperation[1] = 0;
	 * prVhtOp->ucVhtOperation[2] = 0;
	 * }
	 * else if(cnmGetBssMaxBw(prAdapter, prBssInfo->ucBssIndex) == MAX_BW_80MHZ) {
	 * prVhtOp->ucVhtOperation[0] = VHT_OP_CHANNEL_WIDTH_80;
	 * prVhtOp->ucVhtOperation[1] = nicGetVhtS1(prBssInfo->ucPrimaryChannel);
	 * prVhtOp->ucVhtOperation[2] = 0;
	 * }
	 * else {
	 * /4 TODO: BW80 + 80/160 support
	 * }
	 */

	prVhtOp->u2VhtBasicMcsSet = prBssInfo->u2VhtBasicMcsSet;

	prMsduInfo->u2FrameLength += (UINT_16)IE_SIZE(prVhtOp);
}

#endif

static VOID rlmReviseMaxBw(P_ADAPTER_T prAdapter, UINT_8 ucBssIndex, P_ENUM_CHNL_EXT_T peExtend,
			PUINT_8 peChannelWidth, PUINT_8 pucS1, UINT_8 ucPrimaryCh)
{
	UINT_8 ucMaxBandwidth;
	UINT_8 ucCurrentBandwidth = (UINT_8)MAX_BW_20MHZ;
	UINT_8 ucOffset = ((UINT_8)MAX_BW_80MHZ - (UINT_8)CW_80MHZ);

	ucMaxBandwidth = cnmGetBssMaxBw(prAdapter, ucBssIndex);

	if (*peChannelWidth > (UINT_8)CW_20_40MHZ) {
		/*case BW > 80 , 160 80P80 */
		ucCurrentBandwidth = (UINT_8)*peChannelWidth + ucOffset;
	} else {
		/*case BW20 BW40 */
		if (*peExtend != CHNL_EXT_SCN) {
			/*case BW40 */
			ucCurrentBandwidth = (UINT_8)MAX_BW_40MHZ;
		}
	}

	if (ucCurrentBandwidth > ucMaxBandwidth) {
		DBGLOG(RLM, INFO, "Decreasse the BW to (%d)\n", ucMaxBandwidth);

		if (ucMaxBandwidth <= (UINT_8)MAX_BW_40MHZ) {
			/*BW20 * BW40*/
			*peChannelWidth = (UINT_8)CW_20_40MHZ;

			if (ucMaxBandwidth == (UINT_8)MAX_BW_20MHZ)
				*peExtend = CHNL_EXT_SCN;
		} else {
			/*BW80, BW160, BW80P80*/
			/*ucMaxBandwidth Must be MAX_BW_80MHZ,MAX_BW_160MHZ,MAX_BW_80MHZ*/
			/*peExtend should not change*/
			*peChannelWidth = (ucMaxBandwidth - ucOffset);

			if (ucMaxBandwidth == (UINT_8)MAX_BW_80MHZ) {
				/*modify S1 for Bandwidth 160 downgrade 80 case*/
				if (ucCurrentBandwidth ==
					(UINT_8)MAX_BW_160MHZ) {

					if ((ucPrimaryCh >= 36U) &&
						(ucPrimaryCh <= 48U))
						*pucS1 = 42;
					else if ((ucPrimaryCh >= 52U) &&
						(ucPrimaryCh <= 64U))
						*pucS1 = 58;
					else if ((ucPrimaryCh >= 100U) &&
						(ucPrimaryCh <= 112U))
						*pucS1 = 106;
					else if ((ucPrimaryCh >= 116U) &&
						(ucPrimaryCh <= 128U))
						*pucS1 = 122;
					else if ((ucPrimaryCh >= 132U) &&
						(ucPrimaryCh <= 144U))
						*pucS1 = 138; /*160 downgrade should not in this case*/
					else if ((ucPrimaryCh >= 149U) &&
						(ucPrimaryCh <= 161U))
						*pucS1 = 155; /*160 downgrade should not in this case*/
					else
						DBGLOG(RLM, INFO,
							"Check connect 160 downgrde (%d) case\n", ucMaxBandwidth);

					DBGLOG(RLM, INFO, "Decreasse the BW160 to BW80, shift S1 to (%d)\n", *pucS1);
				}
			}
		}

		DBGLOG(RLM, INFO, "Modify ChannelWidth (%d) and Extend (%d)\n", *peChannelWidth, *peExtend);
	}
}

/*----------------------------------------------------------------------------*/
/*!
* \brief This function should be invoked to update parameters of associated AP.
*        (Association response and Beacon)
*
* \param[in]
*
* \return none
*/
/*----------------------------------------------------------------------------*/
static UINT_8 rlmRecIeInfoForClient(P_ADAPTER_T prAdapter, P_BSS_INFO_T prBssInfo, PUINT_8 pucIE, UINT_16 u2IELength)
{
	UINT_16 u2Offset;
	P_STA_RECORD_T prStaRec;
	P_IE_HT_CAP_T prHtCap;
	P_IE_HT_OP_T prHtOp;
	P_IE_OBSS_SCAN_PARAM_T prObssScnParam;
	UINT_8 ucERP, ucPrimaryChannel;
	P_WIFI_VAR_T prWifiVar = &prAdapter->rWifiVar;
#if CFG_SUPPORT_QUIET && 0
	BOOLEAN fgHasQuietIE = FALSE;
#endif

#if CFG_SUPPORT_802_11AC
	P_IE_VHT_OP_T prVhtOp;
	P_IE_VHT_CAP_T prVhtCap;
	P_IE_OP_MODE_NOTIFICATION_T prOPModeNotification;	/* Operation Mode Notification */
	BOOLEAN fgHasOPModeIE = FALSE;
	BOOLEAN fgHasNewOPModeIE = FALSE;
	UINT_8 ucVhtOpModeChannelWidth = 0;
	UINT_8 ucMaxBwAllowed;
	UINT_8 ucInitVhtOpMode = 0;
#endif

#if CFG_SUPPORT_DFS
	BOOLEAN fgHasWideBandIE = FALSE;
	BOOLEAN fgHasSCOIE = FALSE;
	BOOLEAN fgHasChannelSwitchIE = FALSE;
	UINT_8 ucChannelAnnouncePri;
	ENUM_CHNL_EXT_T eChannelAnnounceSco;
	UINT_8 ucChannelAnnounceChannelS1 = 0;
	UINT_8 ucChannelAnnounceChannelS2 = 0;
	UINT_8 ucChannelAnnounceVhtBw;
	P_IE_CHANNEL_SWITCH_T prChannelSwitchAnnounceIE;
	P_IE_SECONDARY_OFFSET_T prSecondaryOffsetIE;
	P_IE_WIDE_BAND_CHANNEL_T prWideBandChannelIE;
#endif

	UINT_8 ucSco = 0;
	UINT_16 ucProtectMode = 0;

	ASSERT(prAdapter);
	ASSERT(prBssInfo);
	ASSERT(pucIE);

	prStaRec = prBssInfo->prStaRecOfAP;
	ASSERT(prStaRec);

	prBssInfo->fgUseShortPreamble = prBssInfo->fgIsShortPreambleAllowed;
	ucPrimaryChannel = 0;
	prObssScnParam = NULL;
	ucMaxBwAllowed = cnmGetBssMaxBw(prAdapter, prBssInfo->ucBssIndex);

	/* Note: HT-related members in staRec may not be zero before, so
	 *       if following IE does not exist, they are still not zero.
	 *       These HT-related parameters are valid only when the corresponding
	 *       BssInfo supports 802.11n, i.e., RLM_NET_IS_11N()
	 */
	IE_FOR_EACH(pucIE, u2IELength, u2Offset) {
		switch (IE_ID(pucIE)) {
		case ELEM_ID_HT_CAP:
			if (!RLM_NET_IS_11N(prBssInfo) ||
				IE_LEN(pucIE) !=
				((UINT_8)sizeof(IE_HT_CAP_T) - 2U))
				break;
			prHtCap = (P_IE_HT_CAP_T) pucIE;
			prStaRec->ucMcsSet = prHtCap->rSupMcsSet.aucRxMcsBitmask[0];
			prStaRec->fgSupMcs32 = ((prHtCap->rSupMcsSet
				.aucRxMcsBitmask[32 / 8] & BIT(0U)) != 0U) ?
				TRUE : FALSE;

			kalMemCopy(prStaRec->aucRxMcsBitmask, prHtCap->rSupMcsSet.aucRxMcsBitmask,
				   sizeof(prStaRec->aucRxMcsBitmask) /*SUP_MCS_RX_BITMASK_OCTET_NUM */);
			prStaRec->u2RxHighestSupportedRate = prHtCap->rSupMcsSet.u2RxHighestSupportedRate;
			prStaRec->u4TxRateInfo = prHtCap->rSupMcsSet.u4TxRateInfo;

			prStaRec->u2HtCapInfo = prHtCap->u2HtCapInfo;
			/* Set LDPC Tx capability */
			if (IS_FEATURE_FORCE_ENABLED
				(prWifiVar->ucTxLdpc))
				prStaRec->u2HtCapInfo |= (UINT_16)
				HT_CAP_INFO_LDPC_CAP;
			else {
			if (IS_FEATURE_DISABLED(prWifiVar->ucTxLdpc))
				prStaRec->u2HtCapInfo &= (UINT_16)
				~HT_CAP_INFO_LDPC_CAP;
			}

			/* Set STBC Tx capability */
			if (IS_FEATURE_FORCE_ENABLED
				(prWifiVar->ucTxStbc))
				prStaRec->u2HtCapInfo |= (UINT_16)
				HT_CAP_INFO_TX_STBC;
			else {
			if (IS_FEATURE_DISABLED
				(prWifiVar->ucTxStbc))
				prStaRec->u2HtCapInfo &= (UINT_16)
				~HT_CAP_INFO_TX_STBC;
			}

			/* Set Short GI Tx capability */
			if (IS_FEATURE_FORCE_ENABLED
				(prWifiVar->ucTxShortGI)) {
				prStaRec->u2HtCapInfo |= (UINT_16)
					HT_CAP_INFO_SHORT_GI_20M;
				prStaRec->u2HtCapInfo |= (UINT_16)
					HT_CAP_INFO_SHORT_GI_40M;
			} else {
			if (IS_FEATURE_DISABLED
				(prWifiVar->ucTxShortGI)) {
				prStaRec->u2HtCapInfo &= (UINT_16)
					~HT_CAP_INFO_SHORT_GI_20M;
				prStaRec->u2HtCapInfo &= (UINT_16)
					~HT_CAP_INFO_SHORT_GI_40M;
			}
			}

			/* Set HT Greenfield Tx capability */
			if (IS_FEATURE_FORCE_ENABLED
				(prWifiVar->ucTxGf))
				prStaRec->u2HtCapInfo |= (UINT_16)
				HT_CAP_INFO_HT_GF;
			else {
			if (IS_FEATURE_DISABLED
				(prWifiVar->ucTxGf))
				prStaRec->u2HtCapInfo &= (UINT_16)
				~HT_CAP_INFO_HT_GF;
			}

			prStaRec->ucAmpduParam = prHtCap->ucAmpduParam;
			prStaRec->u2HtExtendedCap = prHtCap->u2HtExtendedCap;
			prStaRec->u4TxBeamformingCap = prHtCap->u4TxBeamformingCap;
			prStaRec->ucAselCap = prHtCap->ucAselCap;
			break;

		case ELEM_ID_HT_OP:
			if (!RLM_NET_IS_11N(prBssInfo) ||
				IE_LEN(pucIE) !=
				((UINT_8)sizeof(IE_HT_OP_T) - 2U))
				break;
			prHtOp = (P_IE_HT_OP_T) pucIE;
			/* Workaround that some APs fill primary channel field by its
			 * secondary channel, but its DS IE is correct 20110610
			 */
			if (ucPrimaryChannel == 0U)
				ucPrimaryChannel = prHtOp->ucPrimaryChannel;
			prBssInfo->ucHtOpInfo1 = prHtOp->ucInfo1;
			prBssInfo->u2HtOpInfo2 = prHtOp->u2Info2;
			prBssInfo->u2HtOpInfo3 = prHtOp->u2Info3;

			if (prBssInfo->fg40mBwAllowed == FALSE)
				prBssInfo->ucHtOpInfo1 &= (UINT_8)
				~(HT_OP_INFO1_SCO | HT_OP_INFO1_STA_CHNL_WIDTH);

			ucSco = prBssInfo->ucHtOpInfo1 & (UINT_8)
				HT_OP_INFO1_SCO;
			if (ucSco != (UINT_8)CHNL_EXT_RES) {
				if (ucSco == 0U)
					prBssInfo->eBssSCO = CHNL_EXT_SCN;
				else if (ucSco == 1U)
					prBssInfo->eBssSCO = CHNL_EXT_SCA;
				else if (ucSco == 3U)
					prBssInfo->eBssSCO = CHNL_EXT_SCB;
				else
					DBGLOG(RLM, TRACE,
					"ucSco = %d\n", ucSco);
			}

			ucProtectMode = prBssInfo->u2HtOpInfo2 &
				(UINT_16)HT_OP_INFO2_HT_PROTECTION;
			if (ucProtectMode == 0U)
				prBssInfo->eHtProtectMode =
				HT_PROTECT_MODE_NONE;
			else if (ucProtectMode == 1U)
				prBssInfo->eHtProtectMode =
				HT_PROTECT_MODE_NON_MEMBER;
			else if (ucProtectMode == 2U)
				prBssInfo->eHtProtectMode =
				HT_PROTECT_MODE_20M;
			else if (ucProtectMode == 3U)
				prBssInfo->eHtProtectMode =
				HT_PROTECT_MODE_NON_HT;
			else
				DBGLOG(RLM, TRACE,
				"ucProtectMode = %d\n", ucProtectMode);

			/* To do: process regulatory class 16 */
			if ((prBssInfo->u2HtOpInfo2 &
				HT_OP_INFO2_NON_GF_HT_STA_PRESENT) != 0U)
				prBssInfo->eGfOperationMode = GF_MODE_PROTECT;
			else
				prBssInfo->eGfOperationMode = GF_MODE_NORMAL;

			prBssInfo->eRifsOperationMode =
			    ((prBssInfo->ucHtOpInfo1 &
			    HT_OP_INFO1_RIFS_MODE) != 0U) ?
			    RIFS_MODE_NORMAL : RIFS_MODE_DISALLOWED;

			break;

#if CFG_SUPPORT_802_11AC
		case ELEM_ID_VHT_CAP:
			if (!RLM_NET_IS_11AC(prBssInfo) ||
				IE_LEN(pucIE) !=
				((UINT_8)sizeof(IE_VHT_CAP_T) - 2U))
				break;

			prVhtCap = (P_IE_VHT_CAP_T) pucIE;

			prStaRec->u4VhtCapInfo = prVhtCap->u4VhtCapInfo;
			/* Set Tx LDPC capability */
			if (IS_FEATURE_FORCE_ENABLED
				(prWifiVar->ucTxLdpc))
				prStaRec->u4VhtCapInfo |= (UINT_32)
				VHT_CAP_INFO_RX_LDPC;
			else {
			if (IS_FEATURE_DISABLED
				(prWifiVar->ucTxLdpc))
				prStaRec->u4VhtCapInfo &= (UINT_32)
				~VHT_CAP_INFO_RX_LDPC;
			}

			/* Set Tx STBC capability */
			if (IS_FEATURE_FORCE_ENABLED
				(prWifiVar->ucTxStbc))
				prStaRec->u4VhtCapInfo |= (UINT_32)
				VHT_CAP_INFO_TX_STBC;
			else {
			if (IS_FEATURE_DISABLED
				(prWifiVar->ucTxStbc))
				prStaRec->u4VhtCapInfo &= (UINT_32)
				~VHT_CAP_INFO_TX_STBC;
			}

			/* Set Tx TXOP PS capability */
			if (IS_FEATURE_FORCE_ENABLED
				(prWifiVar->ucTxopPsTx))
				prStaRec->u4VhtCapInfo |= (UINT_32)
				VHT_CAP_INFO_VHT_TXOP_PS;
			else {
			if (IS_FEATURE_DISABLED
				(prWifiVar->ucTxopPsTx))
				prStaRec->u4VhtCapInfo &= (UINT_32)
				~VHT_CAP_INFO_VHT_TXOP_PS;
			}

			/* Set Tx Short GI capability */
			if (IS_FEATURE_FORCE_ENABLED
				(prWifiVar->ucTxShortGI)) {
				prStaRec->u4VhtCapInfo |= (UINT_32)
					VHT_CAP_INFO_SHORT_GI_80;
				prStaRec->u4VhtCapInfo |= (UINT_32)
					VHT_CAP_INFO_SHORT_GI_160_80P80;
			} else {
			if (IS_FEATURE_DISABLED
				(prWifiVar->ucTxShortGI)) {
				prStaRec->u4VhtCapInfo &= (UINT_32)
					~VHT_CAP_INFO_SHORT_GI_80;
				prStaRec->u4VhtCapInfo &= (UINT_32)
					~VHT_CAP_INFO_SHORT_GI_160_80P80;
				}
			}

			prStaRec->u2VhtRxMcsMap = prVhtCap->rVhtSupportedMcsSet.u2RxMcsMap;
			prStaRec->u2VhtRxHighestSupportedDataRate =
			    prVhtCap->rVhtSupportedMcsSet.u2RxHighestSupportedDataRate;
			prStaRec->u2VhtTxMcsMap = prVhtCap->rVhtSupportedMcsSet.u2TxMcsMap;
			prStaRec->u2VhtTxHighestSupportedDataRate =
			    prVhtCap->rVhtSupportedMcsSet.u2TxHighestSupportedDataRate;

			break;

		case ELEM_ID_VHT_OP:
			if (!RLM_NET_IS_11AC(prBssInfo) ||
				IE_LEN(pucIE) !=
				((UINT_8)sizeof(IE_VHT_OP_T) - 2U))
				break;

			prVhtOp = (P_IE_VHT_OP_T) pucIE;

			prBssInfo->ucVhtChannelWidth = prVhtOp->ucVhtOperation[0];
			prBssInfo->ucVhtChannelFrequencyS1 = prVhtOp->ucVhtOperation[1];
			prBssInfo->ucVhtChannelFrequencyS2 = prVhtOp->ucVhtOperation[2];
			prBssInfo->u2VhtBasicMcsSet = prVhtOp->u2VhtBasicMcsSet;

			break;

		case ELEM_ID_OP_MODE:
			if (!RLM_NET_IS_11AC(prBssInfo) ||
				IE_LEN(pucIE) !=
				((UINT_8)sizeof(IE_OP_MODE_NOTIFICATION_T)
				- 2U))
				break;
			prOPModeNotification = (P_IE_OP_MODE_NOTIFICATION_T) pucIE;

			/* NOTE: An AP always sets this field to 0,
			 * so break it if this bit is set.
			 */
			if ((prOPModeNotification->ucOpMode & VHT_OP_MODE_RX_NSS_TYPE)
			    == VHT_OP_MODE_RX_NSS_TYPE) {
				break;
			}

			fgHasOPModeIE = TRUE;

			/* Same OP mode, no need to update.
			 * Let the further flow not to update VhtOpMode.
			 */
			if (prStaRec->ucVhtOpMode == prOPModeNotification->ucOpMode) {
				ucInitVhtOpMode = prStaRec->ucVhtOpMode;
				break;
			}

			fgHasNewOPModeIE = TRUE;
			prStaRec->ucVhtOpMode = prOPModeNotification->ucOpMode;
			ucVhtOpModeChannelWidth =
			    ((prOPModeNotification->ucOpMode) & VHT_OP_MODE_CHANNEL_WIDTH);

			break;
#if CFG_SUPPORT_DFS
		case ELEM_ID_WIDE_BAND_CHANNEL_SWITCH:
			if (!RLM_NET_IS_11AC(prBssInfo) ||
				IE_LEN(pucIE) !=
				((UINT_8)sizeof(IE_WIDE_BAND_CHANNEL_T) - 2U))
				break;
			DBGLOG(RLM, INFO, "[Channel Switch] ELEM_ID_WIDE_BAND_CHANNEL_SWITCH, 11AC\n");
			prWideBandChannelIE = (P_IE_WIDE_BAND_CHANNEL_T) pucIE;
			ucChannelAnnounceVhtBw = prWideBandChannelIE->ucNewChannelWidth;
			ucChannelAnnounceChannelS1 = prWideBandChannelIE->ucChannelS1;
			ucChannelAnnounceChannelS2 = prWideBandChannelIE->ucChannelS2;
			fgHasWideBandIE = TRUE;
			DBGLOG(RLM, INFO,
			       "[Ch] BW=%d, s1=%d, s2=%d\n", ucChannelAnnounceVhtBw, ucChannelAnnounceChannelS1,
				ucChannelAnnounceChannelS2);
			break;
#endif

#endif
		case ELEM_ID_20_40_BSS_COEXISTENCE:
			if (!RLM_NET_IS_11N(prBssInfo))
				break;
			/* To do: store if scanning exemption grant to BssInfo */
			break;

		case ELEM_ID_OBSS_SCAN_PARAMS:
			if (!RLM_NET_IS_11N(prBssInfo) ||
				IE_LEN(pucIE) !=
				((UINT_8)sizeof(IE_OBSS_SCAN_PARAM_T) - 2U))
				break;
			/* Store OBSS parameters to BssInfo */
			prObssScnParam = (P_IE_OBSS_SCAN_PARAM_T) pucIE;
			break;

		case ELEM_ID_EXTENDED_CAP:
			if (!RLM_NET_IS_11N(prBssInfo))
				break;
			/* To do: store extended capability (PSMP, coexist) to BssInfo */
			break;

		case ELEM_ID_ERP_INFO:
			if (IE_LEN(pucIE) != ((UINT_8)sizeof(IE_ERP_T) - 2U) ||
				prBssInfo->eBand != BAND_2G4)
				break;
			ucERP = ERP_INFO_IE(pucIE)->ucERP;
			prBssInfo->fgErpProtectMode =
				((ucERP & ERP_INFO_USE_PROTECTION) != 0U) ?
				TRUE : FALSE;

			if ((ucERP & ERP_INFO_BARKER_PREAMBLE_MODE) != 0U)
				prBssInfo->fgUseShortPreamble = FALSE;
			break;

		case ELEM_ID_DS_PARAM_SET:
			if (IE_LEN(pucIE) == ELEM_MAX_LEN_DS_PARAMETER_SET)
				ucPrimaryChannel = DS_PARAM_IE(pucIE)->ucCurrChnl;
			break;
#if CFG_SUPPORT_DFS
		case ELEM_ID_CH_SW_ANNOUNCEMENT:
			if (IE_LEN(pucIE) !=
				((UINT_8)sizeof(IE_CHANNEL_SWITCH_T) - 2U))
				break;

			prChannelSwitchAnnounceIE = (P_IE_CHANNEL_SWITCH_T) pucIE;

			DBGLOG(RLM, INFO, "[Ch] Count=%d\n", prChannelSwitchAnnounceIE->ucChannelSwitchCount);
#if 0
			qmSetStaRecTxAllowed(prAdapter, prStaRec, FALSE);
			DBGLOG(RLM, INFO, "[Ch] TxAllowed = %d\n", prStaRec->fgIsTxAllowed);
#endif
			if (prChannelSwitchAnnounceIE
				->ucChannelSwitchMode == 1U) {
				if (prChannelSwitchAnnounceIE
					->ucChannelSwitchCount <= 3U) {
					DBGLOG(RLM, INFO,
					"[Ch] switch channel [%d]->[%d]\n", prBssInfo->ucPrimaryChannel,
					prChannelSwitchAnnounceIE->ucNewChannelNum);
					ucChannelAnnouncePri = prChannelSwitchAnnounceIE->ucNewChannelNum;

					if (RLM_NET_IS_11AC(prBssInfo) &&
						(prBssInfo->ucVhtChannelWidth !=
						(UINT_8)CW_20_40MHZ))
						g_fgHasChannelSwitchIE = TRUE;
					fgHasChannelSwitchIE = TRUE;
#if 0
					qmSetStaRecTxAllowed(prAdapter, prStaRec, TRUE);
					DBGLOG(RLM, INFO, "[Ch] After switching , TxAllowed = %d\n",
									prStaRec->fgIsTxAllowed);
#endif
				}
				if (RLM_NET_IS_11AC(prBssInfo) &&
					(prBssInfo->ucVhtChannelWidth !=
					(UINT_8)CW_20_40MHZ)) {
					DBGLOG(RLM, INFO, "Send Operation Action Frame");
					rlmSendOpModeNotificationFrame(prAdapter, prStaRec,
					VHT_OP_MODE_CHANNEL_WIDTH_20, 1U);
				} else {
					DBGLOG(RLM, INFO, "Skip Send Operation Action Frame");
				}
			}

			break;
		case ELEM_ID_SCO:
			if (IE_LEN(pucIE) !=
				((UINT_8)sizeof(IE_SECONDARY_OFFSET_T) - 2U))
				break;

			prSecondaryOffsetIE = (P_IE_SECONDARY_OFFSET_T) pucIE;
			DBGLOG(RLM, INFO,
			       "[Channel Switch] SCO [%d]->[%d]\n", prBssInfo->eBssSCO,
				prSecondaryOffsetIE->ucSecondaryOffset);
			eChannelAnnounceSco = (ENUM_CHNL_EXT_T) prSecondaryOffsetIE->ucSecondaryOffset;
			fgHasSCOIE = TRUE;
			break;
#endif
		default:
		{
			DBGLOG(RLM, TRACE,
				"Default Case in rlmRecIeInfoForClient\n");
			break;
		}

		}		/* end of switch */
	}			/* end of IE_FOR_EACH */

	/* Some AP will have wrong channel number (255) when running time.
	 * Check if correct channel number information. 20110501
	 */
	if ((prBssInfo->eBand == BAND_2G4 && ucPrimaryChannel > 14U) ||
	    (prBssInfo->eBand != BAND_2G4 &&
	    (ucPrimaryChannel >= 200U || ucPrimaryChannel <= 14U)))
		ucPrimaryChannel = 0;
#if CFG_SUPPORT_802_11AC
	/* Check whether the Operation Mode IE is exist or not.
	 *  If exists, then the channel bandwidth of VHT operation field  is changed
	 *  with the channel bandwidth setting of Operation Mode field.
	 *  The channel bandwidth of OP Mode IE  is  0, represent as 20MHz.
	 *  The channel bandwidth of OP Mode IE  is  1, represent as 40MHz.
	 *  The channel bandwidth of OP Mode IE  is  2, represent as 80MHz.
	 *  The channel bandwidth of OP Mode IE  is  3, represent as 160/80+80MHz.
	 */
	if (fgHasNewOPModeIE == TRUE) {
		if (prStaRec->ucStaState == STA_STATE_3) {
			/* 1. Modify channel width parameters */
			rlmRecOpModeBwForClient(ucVhtOpModeChannelWidth,
						prBssInfo);

			/* 2. Update StaRec to FW (BssInfo will be updated after
			 * return from this function)
			 */
			DBGLOG(RLM, INFO,
			       "Update OpMode to 0x%x, to FW due to OpMode Notificaition",
			       prStaRec->ucVhtOpMode);
			cnmStaSendUpdateCmd(prAdapter, prStaRec, FALSE);
		}
	} else { /* Set Default if the VHT OP mode field is not present */
		if (!fgHasOPModeIE) {
			ucInitVhtOpMode |=
				rlmGetOpModeBwByVhtAndHtOpInfo(prBssInfo);
		}
		if ((prStaRec->ucVhtOpMode != ucInitVhtOpMode) &&
			(prStaRec->ucStaState == STA_STATE_3)) {
			prStaRec->ucVhtOpMode = ucInitVhtOpMode;
			DBGLOG(RLM, INFO, "Update OpMode to 0x%x",
			       prStaRec->ucVhtOpMode);
			DBGLOG(RLM, INFO,
			       "to FW due to NO OpMode Notificaition\n");
			cnmStaSendUpdateCmd(prAdapter, prStaRec, FALSE);
		} else
			prStaRec->ucVhtOpMode = ucInitVhtOpMode;
	}
#endif

#if CFG_SUPPORT_DFS
	/*Check whether Channel Announcement IE, Secondary Offset IE &
	 *  Wide Bandwidth Channel Switch IE exist or not. If exist, the priority is
	 the highest.
	 */

	if (fgHasChannelSwitchIE != FALSE) {
		P_BSS_DESC_T prBssDesc;

		prBssInfo->ucPrimaryChannel = ucChannelAnnouncePri;
		prBssDesc = scanSearchBssDescByBssid(prAdapter, prBssInfo->aucBSSID);

		if (prBssDesc != NULL) {
			DBGLOG(RLM, INFO, "DFS: BSS: " MACSTR " Desc found, channel from %u to %u\n ",
			MAC2STR(prBssInfo->aucBSSID),
			prBssDesc->ucChannelNum,
			ucChannelAnnouncePri);
			prBssDesc->ucChannelNum = ucChannelAnnouncePri;
		} else {
			DBGLOG(RLM, INFO, "DFS: BSS: " MACSTR " Desc is not found\n ", MAC2STR(prBssInfo->aucBSSID));
		}

		if (fgHasWideBandIE != FALSE) {
			prBssInfo->ucVhtChannelWidth = ucChannelAnnounceVhtBw;
			prBssInfo->ucVhtChannelFrequencyS1 = ucChannelAnnounceChannelS1;
			prBssInfo->ucVhtChannelFrequencyS2 = ucChannelAnnounceChannelS2;
		}
		if (fgHasSCOIE != FALSE)
			prBssInfo->eBssSCO = eChannelAnnounceSco;
	}


    /*DFS Certification for Channel Bandwidth 80Hz*/
	if (g_fgHasChannelSwitchIE == TRUE) {
		DBGLOG(RLM, INFO, "Ch : DFS 80M Flag= %d\n", g_fgHasChannelSwitchIE);
		prBssInfo->eBssSCO = CHNL_EXT_SCN;
		prBssInfo->ucVhtChannelWidth = (UINT_8)CW_20_40MHZ;
		prBssInfo->ucVhtChannelFrequencyS1 = 0;
		prBssInfo->ucVhtChannelFrequencyS2 = 255;
		prBssInfo->ucHtOpInfo1 &= (UINT_8)
			~(HT_OP_INFO1_SCO | HT_OP_INFO1_STA_CHNL_WIDTH);
		DBGLOG(RLM, INFO, "Ch : DFS has Appeared\n");
	}

	/*
	 * In STA+SAP concurrent mode, to ensure the two BSSes are working in SCC,
	 * we need to indicate Framework the channel switch event to let it restart SAP
	 * on the new channel.
	 */
	if (cnmSapIsActive(prAdapter))
		kalIndicateStatusAndComplete(prAdapter->prGlueInfo,
						 WLAN_STATUS_BSS_CH_SWITCH, NULL, 0);

	/*workaround sta dfs channel + sap turn on fail issue.*/
	wlanUpdateDfsChannelTable(prAdapter->prGlueInfo,
		prBssInfo->ucPrimaryChannel);

#endif
	rlmReviseMaxBw(prAdapter, prBssInfo->ucBssIndex, &prBssInfo->eBssSCO,
		&prBssInfo->ucVhtChannelWidth, &prBssInfo->ucVhtChannelFrequencyS1,
		prBssInfo->ucPrimaryChannel);

	if (rlmDomainIsValidRfSetting(prAdapter, prBssInfo->eBand,
			prBssInfo->ucPrimaryChannel, prBssInfo->eBssSCO,
			(ENUM_CHANNEL_WIDTH_T)prBssInfo->ucVhtChannelWidth,
			prBssInfo->ucVhtChannelFrequencyS1,
			prBssInfo->ucVhtChannelFrequencyS2) == FALSE) {

		/*Dump IE Inforamtion */
		PUINT_8 pucDumpIE;

		pucDumpIE = (PUINT_8)((ULONG)pucIE - u2IELength);
		DBGLOG(RLM, WARN, "rlmRecIeInfoForClient IE Information\n");
		DBGLOG(RLM, WARN, "IE Length = %d\n", u2IELength);
		DBGLOG_MEM8(RLM, WARN, pucDumpIE, u2IELength);

		/*Error Handling for Non-predicted IE - Fixed to set 20MHz */
		prBssInfo->ucVhtChannelWidth = (UINT_8)CW_20_40MHZ;
		prBssInfo->ucVhtChannelFrequencyS1 = 0;
		prBssInfo->ucVhtChannelFrequencyS2 = 0;
		prBssInfo->eBssSCO = CHNL_EXT_SCN;
		prBssInfo->ucHtOpInfo1 &= (UINT_8)
			~(HT_OP_INFO1_SCO | HT_OP_INFO1_STA_CHNL_WIDTH);
	}

	/* Check if OBSS scan process will launch */
	if (prAdapter->fgEnOnlineScan == FALSE || prObssScnParam == NULL ||
	    ((prStaRec->u2HtCapInfo & HT_CAP_INFO_SUP_CHNL_WIDTH) == 0U) ||
	    prBssInfo->eBand != BAND_2G4 ||
	    prBssInfo->fg40mBwAllowed == FALSE) {

		/* Note: it is ok not to stop rObssScanTimer() here */
		prBssInfo->u2ObssScanInterval = 0;
	} else {
		if (prObssScnParam->u2TriggerScanInterval < OBSS_SCAN_MIN_INTERVAL)
			prObssScnParam->u2TriggerScanInterval = OBSS_SCAN_MIN_INTERVAL;
		if (prBssInfo->u2ObssScanInterval != prObssScnParam->u2TriggerScanInterval) {

			prBssInfo->u2ObssScanInterval = prObssScnParam->u2TriggerScanInterval;

			/* Start timer to trigger OBSS scanning */
			cnmTimerStartTimer(prAdapter, &prBssInfo->rObssScanTimer,
			(UINT_32)prBssInfo->u2ObssScanInterval * MSEC_PER_SEC);
		}
	}

	return ucPrimaryChannel;
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief Update parameters from channel width field in OP Mode IE/action frame
 *
 * \param[in]
 *
 * \return none
 */
/*----------------------------------------------------------------------------*/
static void rlmRecOpModeBwForClient(UINT_8 ucVhtOpModeChannelWidth,
				    P_BSS_INFO_T prBssInfo)
{

	P_STA_RECORD_T prStaRec = NULL;

	if (!prBssInfo)
		return;

	prStaRec = prBssInfo->prStaRecOfAP;
	if (!prStaRec)
		return;

	switch (ucVhtOpModeChannelWidth) {
	case VHT_OP_MODE_CHANNEL_WIDTH_20:
		prBssInfo->ucVhtChannelWidth = VHT_OP_CHANNEL_WIDTH_20_40;
		prBssInfo->ucHtOpInfo1 &= ~HT_OP_INFO1_STA_CHNL_WIDTH;
		prStaRec->u2HtCapInfo &= ~HT_CAP_INFO_SUP_CHNL_WIDTH;

		break;
	case VHT_OP_MODE_CHANNEL_WIDTH_40:
		prBssInfo->ucVhtChannelWidth = VHT_OP_CHANNEL_WIDTH_20_40;
		prBssInfo->ucHtOpInfo1 |= HT_OP_INFO1_STA_CHNL_WIDTH;
		prStaRec->u2HtCapInfo |= HT_CAP_INFO_SUP_CHNL_WIDTH;

		break;
	case VHT_OP_MODE_CHANNEL_WIDTH_80:

		prBssInfo->ucVhtChannelWidth = VHT_OP_CHANNEL_WIDTH_80;
		prBssInfo->ucHtOpInfo1 |= HT_OP_INFO1_STA_CHNL_WIDTH;
		prStaRec->u2HtCapInfo |= HT_CAP_INFO_SUP_CHNL_WIDTH;

		break;
	default:
		break;
	}
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief Get operating notification channel width by VHT and HT operating Info
 *
 * \param[in]
 *
 * \return ucOpModeBw 0:20MHz, 1:40MHz, 2:80MHz, 3:160MHz/80+80MHz
 *
 */
/*----------------------------------------------------------------------------*/
static UINT_8 rlmGetOpModeBwByVhtAndHtOpInfo(P_BSS_INFO_T prBssInfo)
{
	UINT_8 ucOpModeBw = VHT_OP_MODE_CHANNEL_WIDTH_20;

	ASSERT(prBssInfo);

	switch (prBssInfo->ucVhtChannelWidth) {
	case VHT_OP_CHANNEL_WIDTH_20_40:
		if (prBssInfo->eBssSCO != CHNL_EXT_SCN)
			ucOpModeBw = VHT_OP_MODE_CHANNEL_WIDTH_40;
		break;
	case VHT_OP_CHANNEL_WIDTH_80:
		ucOpModeBw = VHT_OP_MODE_CHANNEL_WIDTH_80;
		break;
	case VHT_OP_CHANNEL_WIDTH_160:
	case VHT_OP_CHANNEL_WIDTH_80P80:
		ucOpModeBw = VHT_OP_MODE_CHANNEL_WIDTH_160_80P80;
		break;
	default:
		DBGLOG(RLM, WARN, "%s: unexpected VHT channel width: %d\n",
		       __func__, prBssInfo->ucVhtChannelWidth);
		/*VHT default IE should support BW 80*/
		ucOpModeBw = VHT_OP_MODE_CHANNEL_WIDTH_80;
		break;
	}

	return ucOpModeBw;
}

/*----------------------------------------------------------------------------*/
/*!
* \brief AIS or P2P GC.
*
* \param[in]
*
* \return none
*/
/*----------------------------------------------------------------------------*/
static BOOLEAN
rlmRecBcnFromNeighborForClient(P_ADAPTER_T prAdapter,
			       P_BSS_INFO_T prBssInfo, P_SW_RFB_T prSwRfb, PUINT_8 pucIE, UINT_16 u2IELength)
{
	UINT_16 u2Offset, i;
	UINT_8 ucPriChannel, ucSecChannel;
	UINT_8 eSCO = 0;
	BOOLEAN fgHtBss, fg20mReq;

	ASSERT(prAdapter);
	ASSERT(pucIE);
	if ((prBssInfo == NULL) || (prSwRfb == NULL))
		return FALSE;

	/* Record it to channel list to change 20/40 bandwidth */
	ucPriChannel = 0;

	fgHtBss = FALSE;
	fg20mReq = FALSE;

	IE_FOR_EACH(pucIE, u2IELength, u2Offset) {
		switch (IE_ID(pucIE)) {
		case ELEM_ID_HT_CAP:
			{
				P_IE_HT_CAP_T prHtCap;

				if (IE_LEN(pucIE) !=
					((UINT_8)sizeof(IE_HT_CAP_T) - 2U))
					break;

				prHtCap = (P_IE_HT_CAP_T) pucIE;
				if ((prHtCap->u2HtCapInfo &
					HT_CAP_INFO_40M_INTOLERANT) != 0U)
					fg20mReq = TRUE;
				fgHtBss = TRUE;
				break;
			}
		case ELEM_ID_HT_OP:
			{
				P_IE_HT_OP_T prHtOp;

				if (IE_LEN(pucIE) !=
					((UINT_8)sizeof(IE_HT_OP_T) - 2U))
					break;

				prHtOp = (P_IE_HT_OP_T) pucIE;
				/* Workaround that some APs fill primary channel field by its
				 * secondary channel, but its DS IE is correct 20110610
				 */
				if (ucPriChannel == 0U)
					ucPriChannel = prHtOp->ucPrimaryChannel;

				if ((prHtOp->ucInfo1 & HT_OP_INFO1_SCO) !=
					(UINT_8)CHNL_EXT_RES)
					eSCO = prHtOp->ucInfo1 &
					(UINT_8)HT_OP_INFO1_SCO;
				break;
			}
		case ELEM_ID_20_40_BSS_COEXISTENCE:
			{
				P_IE_20_40_COEXIST_T prCoexist;

				if (IE_LEN(pucIE) !=
					((UINT_8)sizeof(IE_20_40_COEXIST_T)
					- 2U))
					break;

				prCoexist = (P_IE_20_40_COEXIST_T) pucIE;
				if ((prCoexist->ucData &
					BSS_COEXIST_40M_INTOLERANT) != 0U)
					fg20mReq = TRUE;
				break;
			}
		case ELEM_ID_DS_PARAM_SET:
			if (IE_LEN(pucIE) !=
				((UINT_8)sizeof(IE_DS_PARAM_SET_T) - 2U))
				break;
			ucPriChannel = DS_PARAM_IE(pucIE)->ucCurrChnl;
			break;

		default:
		{
			DBGLOG(RLM, TRACE,
				"Default Case in rlmRecBcnFromNeighborForClient\n");
			break;
		}
		}
	}

	/* To do: Update channel list and 5G band. All channel lists have the same
	 * update procedure. We should give it the entry pointer of desired
	 * channel list.
	 */
	if (HAL_RX_STATUS_GET_RF_BAND(prSwRfb->prRxStatus) != BAND_2G4)
		return FALSE;

	if (ucPriChannel == 0U || ucPriChannel > 14U)
		ucPriChannel = HAL_RX_STATUS_GET_CHNL_NUM(prSwRfb->prRxStatus);

	if (fgHtBss == TRUE) {
		ASSERT_BOOLEAN(prBssInfo->auc2G_PriChnlList[0]
			<= CHNL_LIST_SZ_2G);
		for (i = 1; i <= prBssInfo->auc2G_PriChnlList[0] && i <= CHNL_LIST_SZ_2G; i++) {
			if (prBssInfo->auc2G_PriChnlList[i] == ucPriChannel)
				break;
		}
		if ((i > prBssInfo->auc2G_PriChnlList[0]) && (i <= CHNL_LIST_SZ_2G)) {
			prBssInfo->auc2G_PriChnlList[i] = ucPriChannel;
			prBssInfo->auc2G_PriChnlList[0]++;
		}

		/* Update secondary channel */
		if (eSCO != (UINT_8)CHNL_EXT_SCN) {
			ucSecChannel = (eSCO == (UINT_8)CHNL_EXT_SCA) ?
				(ucPriChannel + 4U) : (ucPriChannel - 4U);

			ASSERT_BOOLEAN(prBssInfo->auc2G_SecChnlList[0]
				<= CHNL_LIST_SZ_2G);
			for (i = 1; i <= prBssInfo->auc2G_SecChnlList[0] && i <= CHNL_LIST_SZ_2G; i++) {
				if (prBssInfo->auc2G_SecChnlList[i] == ucSecChannel)
					break;
			}
			if ((i > prBssInfo->auc2G_SecChnlList[0]) && (i <= CHNL_LIST_SZ_2G)) {
				prBssInfo->auc2G_SecChnlList[i] = ucSecChannel;
				prBssInfo->auc2G_SecChnlList[0]++;
			}
		}

		/* Update 20M bandwidth request channels */
		if (fg20mReq == TRUE) {
			ASSERT_BOOLEAN(prBssInfo->auc2G_20mReqChnlList[0]
				<= CHNL_LIST_SZ_2G);
			for (i = 1; i <= prBssInfo->auc2G_20mReqChnlList[0] && i <= CHNL_LIST_SZ_2G; i++) {
				if (prBssInfo->auc2G_20mReqChnlList[i] == ucPriChannel)
					break;
			}
			if ((i > prBssInfo->auc2G_20mReqChnlList[0]) && (i <= CHNL_LIST_SZ_2G)) {
				prBssInfo->auc2G_20mReqChnlList[i] = ucPriChannel;
				prBssInfo->auc2G_20mReqChnlList[0]++;
			}
		}
	} else {
		/* Update non-HT channel list */
		ASSERT_BOOLEAN(prBssInfo->auc2G_NonHtChnlList[0]
			<= CHNL_LIST_SZ_2G);
		for (i = 1; i <= prBssInfo->auc2G_NonHtChnlList[0] && i <= CHNL_LIST_SZ_2G; i++) {
			if (prBssInfo->auc2G_NonHtChnlList[i] == ucPriChannel)
				break;
		}
		if ((i > prBssInfo->auc2G_NonHtChnlList[0]) && (i <= CHNL_LIST_SZ_2G)) {
			prBssInfo->auc2G_NonHtChnlList[i] = ucPriChannel;
			prBssInfo->auc2G_NonHtChnlList[0]++;
		}
	}

	return FALSE;
}

/*----------------------------------------------------------------------------*/
/*!
* \brief AIS or P2P GC.
*
* \param[in]
*
* \return none
*/
/*----------------------------------------------------------------------------*/
static BOOLEAN
rlmRecBcnInfoForClient(P_ADAPTER_T prAdapter,
		       P_BSS_INFO_T prBssInfo, P_SW_RFB_T prSwRfb, PUINT_8 pucIE, UINT_16 u2IELength)
{
	UINT_8 rChannl;

	ASSERT(prAdapter);
	ASSERT(pucIE);
	if ((prBssInfo == NULL) || (prSwRfb == NULL))
		return FALSE;
#if 0				/* SW migration 2010/8/20 */
	/* Note: we shall not update parameters when scanning, otherwise
	 *       channel and bandwidth will not be correct or asserted failure
	 *       during scanning.
	 * Note: remove channel checking. All received Beacons should be processed
	 *       if measurement or other actions are executed in adjacent channels
	 *       and Beacon content checking mechanism is not disabled.
	 */
	if (IS_SCAN_ACTIVE()
	    /* || prBssInfo->ucPrimaryChannel != CHNL_NUM_BY_SWRFB(prSwRfb) */
	    ) {
		return FALSE;
	}
#endif

	/* Handle change of slot time */
	prBssInfo->u2CapInfo = ((P_WLAN_BEACON_FRAME_T) (prSwRfb->pvHeader))->u2CapInfo;
	prBssInfo->fgUseShortSlotTime =
		(((prBssInfo->u2CapInfo & CAP_INFO_SHORT_SLOT_TIME) != 0U)
		|| (prBssInfo->eBand != BAND_2G4)) ? TRUE : FALSE;

	rChannl = rlmRecIeInfoForClient(prAdapter,
		prBssInfo, pucIE, u2IELength);

	return TRUE;
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
VOID rlmProcessBcn(P_ADAPTER_T prAdapter, P_SW_RFB_T prSwRfb, PUINT_8 pucIE, UINT_16 u2IELength)
{
	P_BSS_INFO_T prBssInfo;
	BOOLEAN fgNewParameter;
	BOOLEAN fgResult;
	UINT_8 i;

	ASSERT(prAdapter);
	ASSERT(prSwRfb);
	ASSERT(pucIE);

	fgNewParameter = FALSE;

	/* When concurrent networks exist, GO shall have the same handle as
	 * the other BSS, so the Beacon shall be processed for bandwidth and
	 * protection mechanism.
	 * Note1: we do not have 2 AP (GO) cases simultaneously now.
	 * Note2: If we are GO, concurrent AIS AP should detect it and reflect
	 *        action in its Beacon, so AIS STA just follows Beacon from AP.
	 */
	for (i = 0; i < BSS_INFO_NUM; i++) {
		prBssInfo = prAdapter->aprBssInfo[i];

		if (IS_BSS_BOW(prBssInfo))
			continue;

		if (IS_BSS_ACTIVE(prBssInfo)) {
			if (prBssInfo->eCurrentOPMode == OP_MODE_INFRASTRUCTURE &&
			    prBssInfo->eConnectionState == PARAM_MEDIA_STATE_CONNECTED) {
				/* P2P client or AIS infra STA */
				if (EQUAL_MAC_ADDR(prBssInfo->aucBSSID, ((P_WLAN_MAC_MGMT_HEADER_T)
									 (prSwRfb->pvHeader))->aucBSSID)) {

					fgNewParameter = rlmRecBcnInfoForClient(prAdapter,
										prBssInfo, prSwRfb, pucIE, u2IELength);
				} else {
					fgNewParameter = rlmRecBcnFromNeighborForClient(prAdapter,
											prBssInfo,
											prSwRfb, pucIE, u2IELength);
				}
			}
#if CFG_ENABLE_WIFI_DIRECT
			else {
			if (prAdapter->fgIsP2PRegistered == TRUE &&
				(prBssInfo->eCurrentOPMode ==
				OP_MODE_ACCESS_POINT ||
				prBssInfo->eCurrentOPMode ==
				OP_MODE_P2P_DEVICE)) {
				/* AP scan to check if 20/40M bandwidth is permitted */
				fgResult =
				rlmRecBcnFromNeighborForClient(prAdapter,
				prBssInfo, prSwRfb, pucIE, u2IELength);
				}
			}
#endif

			/* Appy new parameters if necessary */
			if (fgNewParameter == TRUE) {
				rlmSyncOperationParams(prAdapter, prBssInfo);
				fgNewParameter = FALSE;
			}
		}		/* end of IS_BSS_ACTIVE() */
	}
}

static VOID rlmParseIeInfoForAssocRsp(P_ADAPTER_T prAdapter, P_BSS_INFO_T prBssInfo, PUINT_8 pucIE, UINT_16 u2IELength)
{
	UINT_16 u2Offset;
	P_STA_RECORD_T prStaRec;
	BOOLEAN fgWithHtCap = FALSE;
	BOOLEAN fgWithVhtCap = FALSE;

	prStaRec = prBssInfo->prStaRecOfAP;
	if (prStaRec == NULL)
		return;

	IE_FOR_EACH(pucIE, u2IELength, u2Offset) {
		switch (IE_ID(pucIE)) {
		case ELEM_ID_HT_CAP:
			if (!RLM_NET_IS_11N(prBssInfo) ||
				IE_LEN(pucIE) !=
				((UINT_8)sizeof(IE_HT_CAP_T) - 2U))
				break;
			fgWithHtCap = TRUE;
			break;

#if CFG_SUPPORT_802_11AC
		case ELEM_ID_VHT_CAP:
			if (!RLM_NET_IS_11AC(prBssInfo) ||
				IE_LEN(pucIE) !=
				((UINT_8)sizeof(IE_VHT_CAP_T) - 2U))
				break;
			fgWithVhtCap = TRUE;
			break;
#endif
		case ELEM_ID_BSS_MAX_IDLE_PERIOD:
		{
			struct IE_BSS_MAX_IDLE_PERIOD *prBssMaxIdle = (struct IE_BSS_MAX_IDLE_PERIOD *)pucIE;

			prStaRec->u2MaxIdlePeriod = prBssMaxIdle->u2MaxIdlePeriod;
			prStaRec->ucIdleOption = prBssMaxIdle->ucIdleOption;
			break;
		}
		default:
		{
			DBGLOG(RLM, TRACE,
				"Default Case in rlmParseIeInfoForAssocRsp\n");
			break;
		}
		}		/* end of switch */
	}			/* end of IE_FOR_EACH */

	if (fgWithHtCap == FALSE)
		prStaRec->ucDesiredPhyTypeSet &= (UINT_8)~PHY_TYPE_BIT_HT;

	if (fgWithVhtCap == FALSE)
		prStaRec->ucDesiredPhyTypeSet &= (UINT_8)~PHY_TYPE_BIT_VHT;
}

/*----------------------------------------------------------------------------*/
/*!
* \brief This function should be invoked after judging successful association.
*
* \param[in]
*
* \return none
*/
/*----------------------------------------------------------------------------*/
VOID rlmProcessAssocRsp(P_ADAPTER_T prAdapter, P_SW_RFB_T prSwRfb, PUINT_8 pucIE, UINT_16 u2IELength)
{
	P_BSS_INFO_T prBssInfo;
	P_STA_RECORD_T prStaRec;
	UINT_8 ucPriChannel;

	ASSERT(prAdapter);
	ASSERT(prSwRfb);
	ASSERT(pucIE);

	prStaRec = cnmGetStaRecByIndex(prAdapter, prSwRfb->ucStaRecIdx);
	if (prStaRec == NULL)
		return;

	prBssInfo = GET_BSS_INFO_BY_INDEX(prAdapter, prStaRec->ucBssIndex);
	if (prBssInfo == NULL)
		return;

	if (prStaRec != prBssInfo->prStaRecOfAP)
		return;

	/* To do: the invoked function is used to clear all members. It may be
	 *        done by center mechanism in invoker.
	 */
	rlmBssReset(prAdapter, prBssInfo);

	prBssInfo->fgUseShortSlotTime =
		(((prBssInfo->u2CapInfo & CAP_INFO_SHORT_SLOT_TIME) != 0U)
		|| (prBssInfo->eBand != BAND_2G4)) ? TRUE : FALSE;
	ucPriChannel = rlmRecIeInfoForClient(prAdapter, prBssInfo, pucIE, u2IELength);

	if (prBssInfo->ucPrimaryChannel != ucPriChannel) {
		DBGLOG(RLM, INFO,
		       "Use RF pri channel[%u].Pri channel in HT OP IE is :[%u]\n", prBssInfo->ucPrimaryChannel,
			ucPriChannel);
	}
	/*Avoid wrong primary channel info in HT operation IE info when accept association response */
#if 0
	if (ucPriChannel > 0)
		prBssInfo->ucPrimaryChannel = ucPriChannel;
#endif

	rlmParseIeInfoForAssocRsp(prAdapter, prBssInfo, pucIE, u2IELength);

	if (!RLM_NET_IS_11N(prBssInfo) ||
		((prStaRec->u2HtCapInfo & HT_CAP_INFO_SUP_CHNL_WIDTH) == 0U))
		prBssInfo->fg40mBwAllowed = FALSE;

	/* Note: Update its capabilities to WTBL by cnmStaRecChangeState(), which
	 *       shall be invoked afterwards.
	 *       Update channel, bandwidth and protection mode by nicUpdateBss()
	 */
}

#if CFG_SUPPORT_802_11AC
/*----------------------------------------------------------------------------*/
/*!
 * \brief
 *
 * \param[in]
 *
 * \return none
 */
/*----------------------------------------------------------------------------*/
VOID rlmProcessVhtAction(P_ADAPTER_T prAdapter, P_SW_RFB_T prSwRfb)
{
	P_ACTION_OP_MODE_NOTIFICATION_FRAME prRxFrame;
	P_STA_RECORD_T prStaRec;
	P_BSS_INFO_T prBssInfo;

	ASSERT(prAdapter);
	ASSERT(prSwRfb);

	prRxFrame = (P_ACTION_OP_MODE_NOTIFICATION_FRAME) prSwRfb->pvHeader;
	prStaRec = cnmGetStaRecByIndex(prAdapter, prSwRfb->ucStaRecIdx);

	if (!prStaRec)
		return;

	prBssInfo = GET_BSS_INFO_BY_INDEX(prAdapter, prStaRec->ucBssIndex);

	if (!prBssInfo)
		return;

	switch (prRxFrame->ucAction) {
	/* Support Operating mode notification action frame, TH3_Huang */
	case ACTION_OPERATING_MODE_NOTIFICATION:
		if (prStaRec->ucStaState != STA_STATE_3 ||
		    prSwRfb->u2PacketLen < sizeof(ACTION_OP_MODE_NOTIFICATION_FRAME)) {
			return;
		}

		if (((prRxFrame->ucOperatingMode & VHT_OP_MODE_RX_NSS_TYPE)
			!= VHT_OP_MODE_RX_NSS_TYPE) &&
			(prStaRec->ucVhtOpMode != prRxFrame->ucOperatingMode)) {
			prStaRec->ucVhtOpMode = prRxFrame->ucOperatingMode;
			DBGLOG(RLM, INFO,
				"rlmProcessVhtAction -- Update ucVhtOpMode to 0x%x\n", prStaRec->ucVhtOpMode);
			cnmStaSendUpdateCmd(prAdapter, prStaRec, FALSE);
		}
	break;
	default:
	break;
	}
}
#endif

/*----------------------------------------------------------------------------*/
/*!
* \brief This function should be invoked after judging successful association.
*
* \param[in]
*
* \return none
*/
/*----------------------------------------------------------------------------*/
VOID rlmFillSyncCmdParam(P_CMD_SET_BSS_RLM_PARAM_T prCmdBody, P_BSS_INFO_T prBssInfo)
{
	ASSERT_BOOLEAN(prCmdBody != NULL && prBssInfo != NULL);

	prCmdBody->ucBssIndex = prBssInfo->ucBssIndex;
	prCmdBody->ucRfBand = (UINT_8) prBssInfo->eBand;
	prCmdBody->ucPrimaryChannel = prBssInfo->ucPrimaryChannel;
	prCmdBody->ucRfSco = (UINT_8) prBssInfo->eBssSCO;
	prCmdBody->ucErpProtectMode = (UINT_8) prBssInfo->fgErpProtectMode;
	prCmdBody->ucHtProtectMode = (UINT_8) prBssInfo->eHtProtectMode;
	prCmdBody->ucGfOperationMode = (UINT_8) prBssInfo->eGfOperationMode;
	prCmdBody->ucTxRifsMode = (UINT_8) prBssInfo->eRifsOperationMode;
	prCmdBody->u2HtOpInfo3 = prBssInfo->u2HtOpInfo3;
	prCmdBody->u2HtOpInfo2 = prBssInfo->u2HtOpInfo2;
	prCmdBody->ucHtOpInfo1 = prBssInfo->ucHtOpInfo1;
	prCmdBody->ucUseShortPreamble = 0;
	prCmdBody->ucUseShortSlotTime = prBssInfo->fgUseShortSlotTime;
	prCmdBody->ucVhtChannelWidth = prBssInfo->ucVhtChannelWidth;
	prCmdBody->ucVhtChannelFrequencyS1 = prBssInfo->ucVhtChannelFrequencyS1;
	prCmdBody->ucVhtChannelFrequencyS2 = prBssInfo->ucVhtChannelFrequencyS2;
	prCmdBody->u2VhtBasicMcsSet = prBssInfo->u2BSSBasicRateSet;

	if (RLM_NET_PARAM_VALID(prBssInfo)) {
		DBGLOG(RLM, INFO, "N=%d b=%d c=%d s=%d e=%d h=%d I=0x%02x l=%d p=%d w=%d s1=%d s2=%d\n",
				   prCmdBody->ucBssIndex, prCmdBody->ucRfBand,
				   prCmdBody->ucPrimaryChannel, prCmdBody->ucRfSco,
				   prCmdBody->ucErpProtectMode, prCmdBody->ucHtProtectMode,
				   prCmdBody->ucHtOpInfo1, prCmdBody->ucUseShortSlotTime,
				   prCmdBody->ucUseShortPreamble,
				   prCmdBody->ucVhtChannelWidth,
				   prCmdBody->ucVhtChannelFrequencyS1, prCmdBody->ucVhtChannelFrequencyS2);
	} else {
		DBGLOG(RLM, INFO, "N=%d closed\n", prCmdBody->ucBssIndex);
	}
}

/*----------------------------------------------------------------------------*/
/*!
* \brief This function will operation parameters based on situations of
*        concurrent networks. Channel, bandwidth, protection mode, supported
*        rate will be modified.
*
* \param[in]
*
* \return none
*/
/*----------------------------------------------------------------------------*/
VOID rlmSyncOperationParams(P_ADAPTER_T prAdapter, P_BSS_INFO_T prBssInfo)
{
	P_CMD_SET_BSS_RLM_PARAM_T prCmdBody;
	WLAN_STATUS rStatus;

	ASSERT(prAdapter);
	ASSERT(prBssInfo);

	prCmdBody = (P_CMD_SET_BSS_RLM_PARAM_T)cnmMemAlloc(prAdapter,
		RAM_TYPE_BUF, (UINT_32)sizeof(CMD_SET_BSS_RLM_PARAM_T));

	/* ASSERT(prCmdBody); */
	/* To do: exception handle */
	if (prCmdBody == NULL) {
		DBGLOG(RLM, WARN, "No buf for sync RLM params (Net=%d)\n", prBssInfo->ucBssIndex);
		return;
	}

	rlmFillSyncCmdParam(prCmdBody, prBssInfo);

	rStatus = wlanSendSetQueryCmd(prAdapter,
			(UINT_8)CMD_ID_SET_BSS_RLM_PARAM,
			TRUE,
			FALSE,
			FALSE,
			NULL,
			NULL,
			(UINT_32)sizeof(CMD_SET_BSS_RLM_PARAM_T),
			(PUINT_8) prCmdBody,
			NULL,
			0
	    );

	/* ASSERT(rStatus == WLAN_STATUS_PENDING); */
	if (rStatus != WLAN_STATUS_PENDING)
		DBGLOG(RLM, WARN, "rlmSyncOperationParams set cmd fail\n");

	cnmMemFree(prAdapter, prCmdBody);
}

#if CFG_SUPPORT_AAA
/*----------------------------------------------------------------------------*/
/*!
* \brief This function should be invoked after judging successful association.
*
* \param[in]
*
* \return none
*/
/*----------------------------------------------------------------------------*/
VOID rlmProcessAssocReq(P_ADAPTER_T prAdapter, P_SW_RFB_T prSwRfb, PUINT_8 pucIE, UINT_16 u2IELength)
{
	P_BSS_INFO_T prBssInfo;
	P_STA_RECORD_T prStaRec;
	UINT_16 u2Offset;
	P_IE_HT_CAP_T prHtCap;
#if CFG_SUPPORT_802_11AC
	P_IE_VHT_CAP_T prVhtCap;
	P_IE_OP_MODE_NOTIFICATION_T prOPModeNotification;	/* Operation Mode Notification */
#endif

	ASSERT(prAdapter);
	ASSERT(prSwRfb);
	ASSERT(pucIE);

	prStaRec = cnmGetStaRecByIndex(prAdapter, prSwRfb->ucStaRecIdx);
	if (prStaRec == NULL)
		return;
	ASSERT_BOOLEAN(prStaRec->ucBssIndex <= MAX_BSS_INDEX);

	prBssInfo = prAdapter->aprBssInfo[prStaRec->ucBssIndex];

	IE_FOR_EACH(pucIE, u2IELength, u2Offset) {
		switch (IE_ID(pucIE)) {
		case ELEM_ID_HT_CAP:
			if (!RLM_NET_IS_11N(prBssInfo) ||
				IE_LEN(pucIE) != ((UINT_8)sizeof(IE_HT_CAP_T)
				- 2U))
				break;
			prHtCap = (P_IE_HT_CAP_T) pucIE;
			prStaRec->ucMcsSet = prHtCap->rSupMcsSet.aucRxMcsBitmask[0];
			prStaRec->fgSupMcs32 = ((prHtCap->rSupMcsSet
				.aucRxMcsBitmask[32 / 8] & BIT(0U)) != 0U) ?
				TRUE : FALSE;

			prStaRec->u2HtCapInfo = prHtCap->u2HtCapInfo;

			/* Set Short LDPC Tx capability */
			if (IS_FEATURE_FORCE_ENABLED
				(prAdapter->rWifiVar.ucTxLdpc))
				prStaRec->u2HtCapInfo |= (UINT_16)
				HT_CAP_INFO_LDPC_CAP;
			else {
			if (IS_FEATURE_DISABLED
				(prAdapter->rWifiVar.ucTxLdpc))
				prStaRec->u2HtCapInfo &= (UINT_16)
				~HT_CAP_INFO_LDPC_CAP;
			}

			/* Set STBC Tx capability */
			if (IS_FEATURE_FORCE_ENABLED
				(prAdapter->rWifiVar.ucTxStbc))
				prStaRec->u2HtCapInfo |= (UINT_16)
				HT_CAP_INFO_TX_STBC;
			else {
			if (IS_FEATURE_DISABLED(prAdapter->rWifiVar.ucTxStbc))
				prStaRec->u2HtCapInfo &= (UINT_16)
				~HT_CAP_INFO_TX_STBC;
			}
			/* Set Short GI Tx capability */
			if (IS_FEATURE_FORCE_ENABLED
				(prAdapter->rWifiVar.ucTxShortGI)) {
				prStaRec->u2HtCapInfo |= (UINT_16)
					HT_CAP_INFO_SHORT_GI_20M;
				prStaRec->u2HtCapInfo |= (UINT_16)
					HT_CAP_INFO_SHORT_GI_40M;
			} else {
			if (IS_FEATURE_DISABLED
				(prAdapter->rWifiVar.ucTxShortGI)) {
				prStaRec->u2HtCapInfo &= (UINT_16)
					~HT_CAP_INFO_SHORT_GI_20M;
				prStaRec->u2HtCapInfo &= (UINT_16)
					~HT_CAP_INFO_SHORT_GI_40M;
			}
			}

			/* Set HT Greenfield Tx capability */
			if (IS_FEATURE_FORCE_ENABLED
				(prAdapter->rWifiVar.ucTxGf))
				prStaRec->u2HtCapInfo |= (UINT_16)
				HT_CAP_INFO_HT_GF;
			else {
			if (IS_FEATURE_DISABLED
				(prAdapter->rWifiVar.ucTxGf))
				prStaRec->u2HtCapInfo &= (UINT_16)
				~HT_CAP_INFO_HT_GF;
			}

			prStaRec->ucAmpduParam = prHtCap->ucAmpduParam;
			prStaRec->u2HtExtendedCap = prHtCap->u2HtExtendedCap;
			prStaRec->u4TxBeamformingCap = prHtCap->u4TxBeamformingCap;
			prStaRec->ucAselCap = prHtCap->ucAselCap;
			break;

#if CFG_SUPPORT_802_11AC
		case ELEM_ID_VHT_CAP:
			if (!RLM_NET_IS_11AC(prBssInfo) ||
				IE_LEN(pucIE) !=
				((UINT_8)sizeof(IE_VHT_CAP_T) - 2U))
				break;

			prVhtCap = (P_IE_VHT_CAP_T) pucIE;
			prStaRec->u4VhtCapInfo = prVhtCap->u4VhtCapInfo;

			/* Set Tx LDPC capability */
			if (IS_FEATURE_FORCE_ENABLED
				(prAdapter->rWifiVar.ucTxLdpc))
				prStaRec->u4VhtCapInfo |= (UINT_32)
				VHT_CAP_INFO_RX_LDPC;
			else {
			if (IS_FEATURE_DISABLED
				(prAdapter->rWifiVar.ucTxLdpc))
				prStaRec->u4VhtCapInfo &= (UINT_32)
				~VHT_CAP_INFO_RX_LDPC;
			}

			/* Set Tx STBC capability */
			if (IS_FEATURE_FORCE_ENABLED
				(prAdapter->rWifiVar.ucTxStbc))
				prStaRec->u4VhtCapInfo |= (UINT_32)
				VHT_CAP_INFO_TX_STBC;
			else {
			if (IS_FEATURE_DISABLED
				(prAdapter->rWifiVar.ucTxStbc))
				prStaRec->u4VhtCapInfo &= (UINT_32)
				~VHT_CAP_INFO_TX_STBC;
			}

			/* Set Tx TXOP PS capability */
			if (IS_FEATURE_FORCE_ENABLED
				(prAdapter->rWifiVar.ucTxopPsTx))
				prStaRec->u4VhtCapInfo |= (UINT_32)
				VHT_CAP_INFO_VHT_TXOP_PS;
			else {
			if (IS_FEATURE_DISABLED
				(prAdapter->rWifiVar.ucTxopPsTx))
				prStaRec->u4VhtCapInfo &= (UINT_32)
				~VHT_CAP_INFO_VHT_TXOP_PS;
			}

			/* Set Tx Short GI capability */
			if (IS_FEATURE_FORCE_ENABLED
				(prAdapter->rWifiVar.ucTxShortGI)) {
				prStaRec->u4VhtCapInfo |= (UINT_32)
					VHT_CAP_INFO_SHORT_GI_80;
				prStaRec->u4VhtCapInfo |= (UINT_32)
					VHT_CAP_INFO_SHORT_GI_160_80P80;
			} else {
			if (IS_FEATURE_DISABLED
				(prAdapter->rWifiVar.ucTxShortGI)) {
				prStaRec->u4VhtCapInfo &= (UINT_32)
					~VHT_CAP_INFO_SHORT_GI_80;
				prStaRec->u4VhtCapInfo &= (UINT_32)
					~VHT_CAP_INFO_SHORT_GI_160_80P80;
			}
			}

			prStaRec->u2VhtRxMcsMap = prVhtCap->rVhtSupportedMcsSet.u2RxMcsMap;
			prStaRec->u2VhtRxHighestSupportedDataRate =
			    prVhtCap->rVhtSupportedMcsSet.u2RxHighestSupportedDataRate;
			prStaRec->u2VhtTxMcsMap = prVhtCap->rVhtSupportedMcsSet.u2TxMcsMap;
			prStaRec->u2VhtTxHighestSupportedDataRate =
			    prVhtCap->rVhtSupportedMcsSet.u2TxHighestSupportedDataRate;
			/* Set initial value of VHT OP mode */
			prStaRec->ucVhtOpMode = 0;
			switch (prBssInfo->ucVhtChannelWidth) {
			case VHT_OP_CHANNEL_WIDTH_20_40:
				prStaRec->ucVhtOpMode |= VHT_OP_MODE_CHANNEL_WIDTH_40;
				break;
			case VHT_OP_CHANNEL_WIDTH_80:
				prStaRec->ucVhtOpMode |= VHT_OP_MODE_CHANNEL_WIDTH_80;
				break;
			case VHT_OP_CHANNEL_WIDTH_160:
			case VHT_OP_CHANNEL_WIDTH_80P80:
				prStaRec->ucVhtOpMode |= VHT_OP_MODE_CHANNEL_WIDTH_160_80P80;
				break;
			default:
				prStaRec->ucVhtOpMode |= VHT_OP_MODE_CHANNEL_WIDTH_80;
				break;
			}
			prStaRec->ucVhtOpMode |= ((prBssInfo->ucNss-1) <<
				VHT_OP_MODE_RX_NSS_OFFSET) & VHT_OP_MODE_RX_NSS;

			break;
		case ELEM_ID_OP_MODE:
			if (!RLM_NET_IS_11AC(prBssInfo) || IE_LEN(pucIE) != (sizeof(IE_OP_MODE_NOTIFICATION_T) - 2))
				break;
			prOPModeNotification = (P_IE_OP_MODE_NOTIFICATION_T) pucIE;

			if ((prOPModeNotification->ucOpMode & VHT_OP_MODE_RX_NSS_TYPE)
			    != VHT_OP_MODE_RX_NSS_TYPE) {
				prStaRec->ucVhtOpMode = prOPModeNotification->ucOpMode;
			}

			break;
#endif

		default:
		{
			DBGLOG(RLM, TRACE,
				"Default Case in rlmProcessAssocReq\n");
			break;
		}
		}		/* end of switch */
	}			/* end of IE_FOR_EACH */
}
#endif /* CFG_SUPPORT_AAA */

/*----------------------------------------------------------------------------*/
/*!
* \brief It is for both STA and AP modes
*
* \param[in]
*
* \return none
*/
/*----------------------------------------------------------------------------*/
VOID rlmBssInitForAPandIbss(P_ADAPTER_T prAdapter, P_BSS_INFO_T prBssInfo)
{
	ASSERT(prAdapter);
	ASSERT(prBssInfo);

#if CFG_ENABLE_WIFI_DIRECT
	if (prAdapter->fgIsP2PRegistered == TRUE &&
		prBssInfo->eCurrentOPMode == OP_MODE_ACCESS_POINT)
		rlmBssInitForAP(prAdapter, prBssInfo);
#endif
}

/*----------------------------------------------------------------------------*/
/*!
* \brief It is for both STA and AP modes
*
* \param[in]
*
* \return none
*/
/*----------------------------------------------------------------------------*/
VOID rlmBssAborted(P_ADAPTER_T prAdapter, P_BSS_INFO_T prBssInfo)
{
	ASSERT(prAdapter);
	ASSERT(prBssInfo);

	rlmBssReset(prAdapter, prBssInfo);

	prBssInfo->fg40mBwAllowed = FALSE;
	prBssInfo->fgAssoc40mBwAllowed = FALSE;

	/* Assume FW state is updated by CMD_ID_SET_BSS_INFO, so
	 * the sync CMD is not needed here.
	 */
}

/*----------------------------------------------------------------------------*/
/*!
* \brief All RLM timers will also be stopped.
*
* \param[in]
*
* \return none
*/
/*----------------------------------------------------------------------------*/
static VOID rlmBssReset(P_ADAPTER_T prAdapter, P_BSS_INFO_T prBssInfo)
{
	ASSERT(prAdapter);
	ASSERT(prBssInfo);

	/* HT related parameters */
	prBssInfo->ucHtOpInfo1 = 0;	/* RIFS disabled. 20MHz */
	prBssInfo->u2HtOpInfo2 = 0;
	prBssInfo->u2HtOpInfo3 = 0;

#if CFG_SUPPORT_802_11AC
	prBssInfo->ucVhtChannelWidth = 0;	/* VHT_OP_CHANNEL_WIDTH_80; */
	prBssInfo->ucVhtChannelFrequencyS1 = 0;	/* 42; */
	prBssInfo->ucVhtChannelFrequencyS2 = 0;
	prBssInfo->u2VhtBasicMcsSet = 0;	/* 0xFFFF; */
#endif

	prBssInfo->eBssSCO = CHNL_EXT_SCN;
	prBssInfo->fgErpProtectMode = FALSE;
	prBssInfo->eHtProtectMode = HT_PROTECT_MODE_NONE;
	prBssInfo->eGfOperationMode = GF_MODE_NORMAL;
	prBssInfo->eRifsOperationMode = RIFS_MODE_NORMAL;

	/* OBSS related parameters */
	prBssInfo->auc2G_20mReqChnlList[0] = 0;
	prBssInfo->auc2G_NonHtChnlList[0] = 0;
	prBssInfo->auc2G_PriChnlList[0] = 0;
	prBssInfo->auc2G_SecChnlList[0] = 0;
	prBssInfo->auc5G_20mReqChnlList[0] = 0;
	prBssInfo->auc5G_NonHtChnlList[0] = 0;
	prBssInfo->auc5G_PriChnlList[0] = 0;
	prBssInfo->auc5G_SecChnlList[0] = 0;

	/* All RLM timers will also be stopped */
	cnmTimerStopTimer(prAdapter, &prBssInfo->rObssScanTimer);
	prBssInfo->u2ObssScanInterval = 0;

	prBssInfo->fgObssErpProtectMode = FALSE;	/* GO only */
	prBssInfo->eObssHtProtectMode = HT_PROTECT_MODE_NONE;	/* GO only */
	prBssInfo->eObssGfOperationMode = GF_MODE_NORMAL;	/* GO only */
	prBssInfo->fgObssRifsOperationMode = FALSE;	/* GO only */
	prBssInfo->fgObssActionForcedTo20M = FALSE;	/* GO only */
	prBssInfo->fgObssBeaconForcedTo20M = FALSE;	/* GO only */
}

#if CFG_SUPPORT_TDLS
/*----------------------------------------------------------------------------*/
/*!
* \brief
*
* \param[in]
*
* \return none
*/
/*----------------------------------------------------------------------------*/
UINT_32 rlmFillVhtCapIEByAdapter(P_ADAPTER_T prAdapter,
	P_BSS_INFO_T prBssInfo, UINT_8 *pOutBuf)
{
	P_IE_VHT_CAP_T prVhtCap;
	P_VHT_SUPPORTED_MCS_FIELD prVhtSupportedMcsSet;
	UINT_8 i;

	ASSERT(prAdapter);
	ASSERT(prBssInfo);
	/* ASSERT(prMsduInfo); */

	prVhtCap = (P_IE_VHT_CAP_T) pOutBuf;

	prVhtCap->ucId = ELEM_ID_VHT_CAP;
	prVhtCap->ucLength = (UINT_8)sizeof(IE_VHT_CAP_T) - ELEM_HDR_LEN;
	prVhtCap->u4VhtCapInfo = (UINT_32)VHT_CAP_INFO_DEFAULT_VAL;

	if (IS_FEATURE_ENABLED(prAdapter->rWifiVar.ucRxShortGI))
		prVhtCap->u4VhtCapInfo |= (UINT_32)
			VHT_CAP_INFO_SHORT_GI_80;

	if (IS_FEATURE_ENABLED(prAdapter->rWifiVar.ucRxLdpc))
		prVhtCap->u4VhtCapInfo |= (UINT_32)
			VHT_CAP_INFO_RX_LDPC;

	if (IS_FEATURE_ENABLED(prAdapter->rWifiVar.ucRxStbc))
		prVhtCap->u4VhtCapInfo |= (UINT_32)
			VHT_CAP_INFO_RX_STBC_ONE_STREAM;

	/*set MCS map */
	prVhtSupportedMcsSet = &prVhtCap->rVhtSupportedMcsSet;
	kalMemZero((PVOID) prVhtSupportedMcsSet, sizeof(VHT_SUPPORTED_MCS_FIELD));

	for (i = 0; i < 8U; i++) {
		prVhtSupportedMcsSet->u2RxMcsMap |= (UINT_16)
			BITS(2U * i, (2U * i + 1U));
		prVhtSupportedMcsSet->u2TxMcsMap |= (UINT_16)
			BITS(2U * i, (2U * i + 1U));
	}

	prVhtSupportedMcsSet->u2RxMcsMap &= (UINT_16)
		(VHT_CAP_INFO_MCS_MAP_MCS9 << VHT_CAP_INFO_MCS_1SS_OFFSET);
	prVhtSupportedMcsSet->u2TxMcsMap &= (UINT_16)
		(VHT_CAP_INFO_MCS_MAP_MCS9 << VHT_CAP_INFO_MCS_1SS_OFFSET);
	prVhtSupportedMcsSet->u2RxHighestSupportedDataRate = VHT_CAP_INFO_DEFAULT_HIGHEST_DATA_RATE;
	prVhtSupportedMcsSet->u2TxHighestSupportedDataRate = VHT_CAP_INFO_DEFAULT_HIGHEST_DATA_RATE;

	ASSERT_BOOLEAN(IE_SIZE(prVhtCap)
		<= (ELEM_HDR_LEN + ELEM_MAX_LEN_VHT_CAP));

	return IE_SIZE(prVhtCap);
}
#endif

#if CFG_SUPPORT_TDLS
/*----------------------------------------------------------------------------*/
/*!
* \brief
*
* \param[in]
*
* \return none
*/
/*----------------------------------------------------------------------------*/
UINT_32
rlmFillHtCapIEByParams(BOOLEAN fg40mAllowed,
		       BOOLEAN fgShortGIDisabled,
		       UINT_8 u8SupportRxSgi20,
		       UINT_8 u8SupportRxSgi40, UINT_8 u8SupportRxGf, ENUM_OP_MODE_T eCurrentOPMode, UINT_8 *pOutBuf)
{
	P_IE_HT_CAP_T prHtCap;
	P_SUP_MCS_SET_FIELD prSupMcsSet;

	ASSERT(pOutBuf);

	prHtCap = (P_IE_HT_CAP_T) pOutBuf;

	/* Add HT capabilities IE */
	prHtCap->ucId = ELEM_ID_HT_CAP;
	prHtCap->ucLength = (UINT_8)sizeof(IE_HT_CAP_T) - ELEM_HDR_LEN;

	prHtCap->u2HtCapInfo = (UINT_16)HT_CAP_INFO_DEFAULT_VAL;
	if (fg40mAllowed == FALSE) {
		prHtCap->u2HtCapInfo &= (UINT_16)~(HT_CAP_INFO_SUP_CHNL_WIDTH |
					  HT_CAP_INFO_SHORT_GI_40M | HT_CAP_INFO_DSSS_CCK_IN_40M);
	}
	if (fgShortGIDisabled == TRUE)
		prHtCap->u2HtCapInfo &= (UINT_16)
			~(HT_CAP_INFO_SHORT_GI_20M | HT_CAP_INFO_SHORT_GI_40M);

	if (u8SupportRxSgi20 == 2U)
		prHtCap->u2HtCapInfo &= (UINT_16)~(HT_CAP_INFO_SHORT_GI_20M);
	if (u8SupportRxSgi40 == 2U)
		prHtCap->u2HtCapInfo &= (UINT_16)~(HT_CAP_INFO_SHORT_GI_40M);
	if (u8SupportRxGf == 2U)
		prHtCap->u2HtCapInfo &= (UINT_16)~(HT_CAP_INFO_HT_GF);

	prHtCap->ucAmpduParam = (UINT_8)AMPDU_PARAM_DEFAULT_VAL;

	prSupMcsSet = &prHtCap->rSupMcsSet;
	kalMemZero((PVOID)&prSupMcsSet->aucRxMcsBitmask[0], SUP_MCS_RX_BITMASK_OCTET_NUM);

	prSupMcsSet->aucRxMcsBitmask[0] = (UINT_8)BITS(0U, 7U);

	if (fg40mAllowed == TRUE)
		prSupMcsSet->aucRxMcsBitmask[32 / 8] = (UINT_8)BIT(0U);
	prSupMcsSet->u2RxHighestSupportedRate = SUP_MCS_RX_DEFAULT_HIGHEST_RATE;
	prSupMcsSet->u4TxRateInfo = (UINT_32)SUP_MCS_TX_DEFAULT_VAL;

	prHtCap->u2HtExtendedCap = (UINT_16)HT_EXT_CAP_DEFAULT_VAL;
	if (fg40mAllowed == FALSE || eCurrentOPMode != OP_MODE_INFRASTRUCTURE)
		prHtCap->u2HtExtendedCap &= (UINT_16)
			~(HT_EXT_CAP_PCO | HT_EXT_CAP_PCO_TRANS_TIME_NONE);

	prHtCap->u4TxBeamformingCap = TX_BEAMFORMING_CAP_DEFAULT_VAL;

	prHtCap->ucAselCap = ASEL_CAP_DEFAULT_VAL;

	ASSERT_BOOLEAN(IE_SIZE(prHtCap)
		<= (ELEM_HDR_LEN + ELEM_MAX_LEN_HT_CAP));

	return IE_SIZE(prHtCap);
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
UINT_32 rlmFillHtCapIEByAdapter(P_ADAPTER_T prAdapter,
	P_BSS_INFO_T prBssInfo, UINT_8 *pOutBuf)
{
	P_IE_HT_CAP_T prHtCap;
	P_SUP_MCS_SET_FIELD prSupMcsSet;
	BOOLEAN fg40mAllowed;

	ASSERT(prAdapter);
	ASSERT(prBssInfo);
	ASSERT(pOutBuf);

	fg40mAllowed = prBssInfo->fgAssoc40mBwAllowed;

	prHtCap = (P_IE_HT_CAP_T) pOutBuf;

	/* Add HT capabilities IE */
	prHtCap->ucId = ELEM_ID_HT_CAP;
	prHtCap->ucLength = (UINT_8)sizeof(IE_HT_CAP_T) - ELEM_HDR_LEN;

	prHtCap->u2HtCapInfo = (UINT_16)HT_CAP_INFO_DEFAULT_VAL;
	if (fg40mAllowed == FALSE) {
		prHtCap->u2HtCapInfo &= (UINT_16)~(HT_CAP_INFO_SUP_CHNL_WIDTH |
					  HT_CAP_INFO_SHORT_GI_40M | HT_CAP_INFO_DSSS_CCK_IN_40M);
	}
	if (IS_FEATURE_ENABLED(prAdapter->rWifiVar.ucRxShortGI))
		prHtCap->u2HtCapInfo |= (UINT_16)
			(HT_CAP_INFO_SHORT_GI_20M | HT_CAP_INFO_SHORT_GI_40M);

	if (IS_FEATURE_ENABLED(prAdapter->rWifiVar.ucRxLdpc))
		prHtCap->u2HtCapInfo |= (UINT_16)HT_CAP_INFO_LDPC_CAP;

	if (IS_FEATURE_ENABLED(prAdapter->rWifiVar.ucRxStbc))
		prHtCap->u2HtCapInfo |= (UINT_16)HT_CAP_INFO_RX_STBC_1_SS;

	prHtCap->ucAmpduParam = (UINT_8)AMPDU_PARAM_DEFAULT_VAL;

	prSupMcsSet = &prHtCap->rSupMcsSet;
	kalMemZero((PVOID)&prSupMcsSet->aucRxMcsBitmask[0], SUP_MCS_RX_BITMASK_OCTET_NUM);

	prSupMcsSet->aucRxMcsBitmask[0] = (UINT_8)BITS(0U, 7U);

	if (fg40mAllowed == TRUE)
		prSupMcsSet->aucRxMcsBitmask[32 / 8] = (UINT_8)BIT(0U);
	prSupMcsSet->u2RxHighestSupportedRate = SUP_MCS_RX_DEFAULT_HIGHEST_RATE;
	prSupMcsSet->u4TxRateInfo = (UINT_32)SUP_MCS_TX_DEFAULT_VAL;

	prHtCap->u2HtExtendedCap = (UINT_16)HT_EXT_CAP_DEFAULT_VAL;
	if (fg40mAllowed == FALSE ||
		prBssInfo->eCurrentOPMode != OP_MODE_INFRASTRUCTURE)
		prHtCap->u2HtExtendedCap &=
		(UINT_16)~(HT_EXT_CAP_PCO | HT_EXT_CAP_PCO_TRANS_TIME_NONE);

	prHtCap->u4TxBeamformingCap = TX_BEAMFORMING_CAP_DEFAULT_VAL;

	prHtCap->ucAselCap = ASEL_CAP_DEFAULT_VAL;

	ASSERT_BOOLEAN(IE_SIZE(prHtCap)
		<= (ELEM_HDR_LEN + ELEM_MAX_LEN_HT_CAP));

	return IE_SIZE(prHtCap);

}

#endif

#if CFG_SUPPORT_DFS
/*----------------------------------------------------------------------------*/
/*!
* \brief This function handle spectrum management action frame
*
* \param[in]
*
* \return none
*/
/*----------------------------------------------------------------------------*/
VOID rlmProcessSpecMgtAction(P_ADAPTER_T prAdapter, P_SW_RFB_T prSwRfb)
{
	PUINT_8 pucIE;
	P_STA_RECORD_T prStaRec;
	P_BSS_INFO_T prBssInfo;
	UINT_16 u2IELength;
	UINT_16 u2Offset = 0;
	P_IE_CHANNEL_SWITCH_T prChannelSwitchAnnounceIE;
	P_IE_SECONDARY_OFFSET_T prSecondaryOffsetIE;
	P_IE_WIDE_BAND_CHANNEL_T prWideBandChannelIE;
	P_ACTION_CHANNEL_SWITCH_FRAME prRxFrame;
	BOOLEAN fgHasChannelSwitchIE = FALSE;
	WLAN_STATUS u4Status;

	DBGLOG(RLM, INFO, "[Mgt Action]rlmProcessSpecMgtAction\n");
	ASSERT(prAdapter);
	ASSERT(prSwRfb);

	u2IELength = prSwRfb->u2PacketLen -
	    (UINT_16) OFFSET_OF(ACTION_CHANNEL_SWITCH_FRAME, aucInfoElem[0]);

	prRxFrame = (P_ACTION_CHANNEL_SWITCH_FRAME) prSwRfb->pvHeader;
	pucIE = prRxFrame->aucInfoElem;

	prStaRec = cnmGetStaRecByIndex(prAdapter, prSwRfb->ucStaRecIdx);
	if (prStaRec == NULL)
		nicRxMgmtNoWTBLHandling(prAdapter, prSwRfb);
	if (prSwRfb->prStaRec == NULL)
		return;
	prStaRec = prSwRfb->prStaRec;
	prBssInfo = GET_BSS_INFO_BY_INDEX(prAdapter, prStaRec->ucBssIndex);

	DBGLOG_MEM8(RLM, INFO, pucIE, u2IELength);
	if (prRxFrame->ucAction == ACTION_CHNL_SWITCH) {
		IE_FOR_EACH(pucIE, u2IELength, u2Offset) {
			switch (IE_ID(pucIE)) {

			case ELEM_ID_WIDE_BAND_CHANNEL_SWITCH:
				if (!RLM_NET_IS_11AC(prBssInfo) ||
				    IE_LEN(pucIE) !=
				    ((UINT_8)sizeof(IE_WIDE_BAND_CHANNEL_T)
				    - 2U)) {
					DBGLOG(RLM, INFO, "[Mgt Action] ELEM_ID_WIDE_BAND_CHANNEL_SWITCH, Length\n");
					break;
				}
				DBGLOG(RLM, INFO, "[Mgt Action] ELEM_ID_WIDE_BAND_CHANNEL_SWITCH, 11AC\n");
				prWideBandChannelIE = (P_IE_WIDE_BAND_CHANNEL_T) pucIE;
				prBssInfo->ucVhtChannelWidth = prWideBandChannelIE->ucNewChannelWidth;
				prBssInfo->ucVhtChannelFrequencyS1 = prWideBandChannelIE->ucChannelS1;
				prBssInfo->ucVhtChannelFrequencyS2 = prWideBandChannelIE->ucChannelS2;
				break;

			case ELEM_ID_CH_SW_ANNOUNCEMENT:
				if (IE_LEN(pucIE) !=
					((UINT_8)sizeof(IE_CHANNEL_SWITCH_T)
					- 2U)) {
					DBGLOG(RLM, INFO, "[Mgt Action] ELEM_ID_CH_SW_ANNOUNCEMENT, Length\n");
					break;
				}

				prChannelSwitchAnnounceIE = (P_IE_CHANNEL_SWITCH_T) pucIE;

				if (prChannelSwitchAnnounceIE
					->ucChannelSwitchMode == 1U) {
					if (prChannelSwitchAnnounceIE
						->ucChannelSwitchCount <= 3U) {
						DBGLOG(RLM, INFO, "[Mgt Action] switch channel [%d]->[%d]",
							prBssInfo->ucPrimaryChannel,
							prChannelSwitchAnnounceIE->ucNewChannelNum);
						prBssInfo->ucPrimaryChannel =
							prChannelSwitchAnnounceIE->ucNewChannelNum;
						fgHasChannelSwitchIE = TRUE;
					}
					if (RLM_NET_IS_11AC(prBssInfo) &&
					(prBssInfo->ucVhtChannelWidth !=
					(UINT_8)CW_20_40MHZ)) {
						DBGLOG(RLM, INFO, "[Mgt Action] Send Operation Action Frame");
						rlmSendOpModeNotificationFrame(prAdapter,
							prStaRec, VHT_OP_MODE_CHANNEL_WIDTH_20, 1);
					} else {
						DBGLOG(RLM, INFO, "[Mgt Action] Skip Send Operation Action Frame");
					}
				} else {
					DBGLOG(RLM, INFO, "[Mgt Action] ucChannelSwitchMode = 0\n");
				}
				fgHasChannelSwitchIE = TRUE;
				break;
			case ELEM_ID_SCO:
				if (IE_LEN(pucIE) !=
					((UINT_8)sizeof(IE_SECONDARY_OFFSET_T)
					- 2U)) {
					DBGLOG(RLM, INFO, "[Mgt Action] ELEM_ID_SCO, Length\n");
					break;
				}
				prSecondaryOffsetIE = (P_IE_SECONDARY_OFFSET_T) pucIE;
				DBGLOG(RLM, INFO,
					"[Mgt Action] SCO [%d]->[%d]\n", prBssInfo->eBssSCO,
					prSecondaryOffsetIE->ucSecondaryOffset);
				prBssInfo->eBssSCO = (ENUM_CHNL_EXT_T)
					prSecondaryOffsetIE->ucSecondaryOffset;
				break;
			default:
			{
				DBGLOG(RLM, TRACE,
					"Default Case in rlmProcessSpecMgtAction\n");
				break;
			}
			}	/*end of switch IE_ID */
		}		/*end of IE_FOR_EACH */
		if (fgHasChannelSwitchIE == FALSE) {
			P_BSS_DESC_T prBssDesc;

			prBssDesc = scanSearchBssDescByBssid(prAdapter, prBssInfo->aucBSSID);
			if (RLM_NET_IS_11AC(prBssInfo) &&
				(prBssInfo->ucVhtChannelWidth !=
				(UINT_8)CW_20_40MHZ)) {
				/*Due to MT6630 BW80 sidelope issue*/
				DBGLOG(RLM, INFO,
					"[Mgt Action] AC Network and BW=%d\n",
					prBssInfo->ucVhtChannelWidth);
				/* Beacon and AssocRsp Process to fix 20M Case */
				g_fgHasChannelSwitchIE = TRUE;
				prBssInfo->ucVhtChannelWidth =
					(UINT_8)CW_20_40MHZ;
				prBssInfo->ucVhtChannelFrequencyS1 =  prBssInfo->ucPrimaryChannel;
				/* To Inform FW radar appear. */
				prBssInfo->ucVhtChannelFrequencyS2 = 255;
				prBssInfo->ucHtOpInfo1 &= (UINT_8)
					~(HT_OP_INFO1_SCO |
					HT_OP_INFO1_STA_CHNL_WIDTH);
				prBssInfo->eBssSCO = CHNL_EXT_SCN;
			}
			if (prBssDesc != NULL) {
				DBGLOG(RLM, INFO,
					"[Mgt Action]BSS: "MACSTR" Desc found, channel from %u to %u\n ",
					MAC2STR(prBssInfo->aucBSSID),
					prBssDesc->ucChannelNum,
					prBssInfo->ucPrimaryChannel);
				prBssDesc->ucChannelNum = prBssInfo->ucPrimaryChannel;
			} else {
				DBGLOG(RLM, INFO,
					"[Mgt Action]BSS: "MACSTR" Desc is not found\n ",
					MAC2STR(prBssInfo->aucBSSID));
			}
				/*
				 * In STA+SAP concurrent mode, to ensure the two BSSes are working in SCC,
				 * we need to indicate Framework the channel switch event to let it restart SAP
				 * on the new channel.
				 */
				if (cnmSapIsActive(prAdapter))
					kalIndicateStatusAndComplete(prAdapter->prGlueInfo,
									 WLAN_STATUS_BSS_CH_SWITCH, NULL, 0);

				/*workaround sta dfs channel + sap turn on fail issue.*/
				wlanUpdateDfsChannelTable(prAdapter->prGlueInfo,
					prBssInfo->ucPrimaryChannel);

			}
		u4Status = nicUpdateBss(prAdapter, prBssInfo->ucBssIndex);
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
VOID rlmSendOpModeNotificationFrame(P_ADAPTER_T prAdapter, P_STA_RECORD_T prStaRec, UINT_8 ucChannelWidth, UINT_8 ucNss)
{

	P_MSDU_INFO_T prMsduInfo;
	P_ACTION_OP_MODE_NOTIFICATION_FRAME prTxFrame;
	P_BSS_INFO_T prBssInfo;
	UINT_16 u2EstimatedFrameLen;
	WLAN_STATUS u4Status;

	/* Sanity Check */
	if (prStaRec == NULL)
		return;

	prBssInfo = GET_BSS_INFO_BY_INDEX(prAdapter, prStaRec->ucBssIndex);
	if (prBssInfo == NULL)
		return;

	/* Calculate MSDU buffer length */
	u2EstimatedFrameLen = MAC_TX_RESERVED_FIELD +
		(UINT_16)sizeof(ACTION_OP_MODE_NOTIFICATION_FRAME);

	/* Alloc MSDU_INFO */
	prMsduInfo = (P_MSDU_INFO_T) cnmMgtPktAlloc(prAdapter, u2EstimatedFrameLen);
	if (prMsduInfo == NULL)
		return;

	kalMemZero(prMsduInfo->prPacket, u2EstimatedFrameLen);

	prTxFrame = prMsduInfo->prPacket;

	/* Fill frame ctrl */
	prTxFrame->u2FrameCtrl = MAC_FRAME_ACTION;

	COPY_MAC_ADDR(prTxFrame->aucDestAddr, prStaRec->aucMacAddr);
	COPY_MAC_ADDR(prTxFrame->aucSrcAddr, prBssInfo->aucOwnMacAddr);
	COPY_MAC_ADDR(prTxFrame->aucBSSID, prBssInfo->aucBSSID);

	/* 3 Compose the frame body's frame */
	prTxFrame->ucCategory = CATEGORY_VHT_ACTION;
	prTxFrame->ucAction = ACTION_OPERATING_MODE_NOTIFICATION;

	prTxFrame->ucOperatingMode |= (ucChannelWidth &
		(UINT_8)VHT_OP_MODE_CHANNEL_WIDTH);

	if (ucNss == 0U)
		ucNss = 1;
	prTxFrame->ucOperatingMode |= (((ucNss - 1U) << 4U) &
		(UINT_8)VHT_OP_MODE_RX_NSS);
	prTxFrame->ucOperatingMode &= (UINT_8)~VHT_OP_MODE_RX_NSS_TYPE;

	/* 4 Update information of MSDU_INFO_T */
	TX_SET_MMPDU(prAdapter,
		prMsduInfo,
		prBssInfo->ucBssIndex,
		prStaRec->ucIndex,
		WLAN_MAC_MGMT_HEADER_LEN,
		(UINT_16)sizeof(ACTION_OP_MODE_NOTIFICATION_FRAME),
		NULL, (UINT_8)MSDU_RATE_MODE_AUTO);

	/* 4 Enqueue the frame to send this action frame. */
	u4Status = nicTxEnqueueMsdu(prAdapter, prMsduInfo);

}

#endif

#if CFG_SUPPORT_802_11K
VOID rlmProcessNeighborReportResonse(P_ADAPTER_T prAdapter, P_WLAN_ACTION_FRAME prAction, UINT_16 u2PacketLen)
{
	struct ACTION_NEIGHBOR_REPORT_FRAME *prNeighborResponse = (struct ACTION_NEIGHBOR_REPORT_FRAME *)prAction;

	ASSERT(prAdapter);
	ASSERT(prNeighborResponse);
	DBGLOG(RLM, INFO, "Neighbor Resp From %pM, DialogToken %d\n",
	       prNeighborResponse->aucSrcAddr, prNeighborResponse->ucDialogToken);
	aisCollectNeighborAPChannel(prAdapter, (struct IE_NEIGHBOR_REPORT_T *)&prNeighborResponse->aucInfoElem[0],
				    u2PacketLen - OFFSET_OF(struct ACTION_NEIGHBOR_REPORT_FRAME, aucInfoElem));
}

VOID rlmTxNeighborReportRequest(P_ADAPTER_T prAdapter, P_STA_RECORD_T prStaRec, struct SUB_ELEMENT_LIST *prSubIEs)
{
	static UINT_8 ucDialogToken = 1;
	P_MSDU_INFO_T prMsduInfo = NULL;
	P_BSS_INFO_T prBssInfo = NULL;
	PUINT_8 pucPayload = NULL;
	struct ACTION_NEIGHBOR_REPORT_FRAME *prTxFrame = NULL;
	UINT_16 u2TxFrameLen = 500;
	UINT_16 u2FrameLen = 0;

	if (!prStaRec)
		return;

	prBssInfo = GET_BSS_INFO_BY_INDEX(prAdapter, prStaRec->ucBssIndex);
	ASSERT(prBssInfo);
	/* 1 Allocate MSDU Info */
	prMsduInfo = (P_MSDU_INFO_T) cnmMgtPktAlloc(prAdapter, MAC_TX_RESERVED_FIELD + u2TxFrameLen);
	if (!prMsduInfo)
		return;
	prTxFrame = (struct ACTION_NEIGHBOR_REPORT_FRAME *)((ULONG) (prMsduInfo->prPacket) + MAC_TX_RESERVED_FIELD);

	/* 2 Compose The Mac Header. */
	prTxFrame->u2FrameCtrl = MAC_FRAME_ACTION;
	COPY_MAC_ADDR(prTxFrame->aucDestAddr, prStaRec->aucMacAddr);
	COPY_MAC_ADDR(prTxFrame->aucSrcAddr, prBssInfo->aucOwnMacAddr);
	COPY_MAC_ADDR(prTxFrame->aucBSSID, prBssInfo->aucBSSID);
	prTxFrame->ucCategory = CATEGORY_RM_ACTION;
	prTxFrame->ucAction = RM_ACTION_NEIGHBOR_REQUEST;
	u2FrameLen = OFFSET_OF(struct ACTION_NEIGHBOR_REPORT_FRAME, aucInfoElem);
	/* 3 Compose the frame body's frame. */
	prTxFrame->ucDialogToken = ucDialogToken++;
	u2TxFrameLen -= sizeof(*prTxFrame) - 1;
	pucPayload = &prTxFrame->aucInfoElem[0];
	while (prSubIEs && u2TxFrameLen >= (prSubIEs->rSubIE.ucLength + 2)) {
		kalMemCopy(pucPayload, &prSubIEs->rSubIE, prSubIEs->rSubIE.ucLength + 2);
		pucPayload += prSubIEs->rSubIE.ucLength + 2;
		u2FrameLen += prSubIEs->rSubIE.ucLength + 2;
		prSubIEs = prSubIEs->prNext;
	}
	nicTxSetMngPacket(prAdapter, prMsduInfo, prStaRec->ucBssIndex, prStaRec->ucIndex,
			  WLAN_MAC_MGMT_HEADER_LEN, u2FrameLen, NULL, MSDU_RATE_MODE_AUTO);

	/* 5 Enqueue the frame to send this action frame. */
	nicTxEnqueueMsdu(prAdapter, prMsduInfo);
}
#endif

VOID rlmComposeEmptyBeaconReport(P_ADAPTER_T prAdapter)
{
	struct RADIO_MEASUREMENT_REQ_PARAMS *prRmReq = &prAdapter->rWifiVar.rRmReqParams;
	struct RADIO_MEASUREMENT_REPORT_PARAMS *prRmRep = &prAdapter->rWifiVar.rRmRepParams;
	PUINT_8 pucReportFrame = prRmRep->pucReportFrameBuff + prRmRep->u2ReportFrameLen;
	P_IE_MEASUREMENT_REPORT_T prRepIE = (P_IE_MEASUREMENT_REPORT_T)pucReportFrame;
	struct RM_BCN_REPORT *prBcnReport = (struct RM_BCN_REPORT *)prRepIE->aucReportFields;

	/* fill in basic content of Measurement report IE */
	prRepIE->ucId = ELEM_ID_MEASUREMENT_REPORT;
	prRepIE->ucToken = prRmReq->prCurrMeasElem->ucToken;
	prRepIE->ucMeasurementType = prRmReq->prCurrMeasElem->ucMeasurementType;
	prRepIE->ucReportMode = 0;
	prRepIE->ucLength = 3 + OFFSET_OF(struct RM_BCN_REPORT, aucOptElem);
	kalMemZero(prBcnReport, OFFSET_OF(struct RM_BCN_REPORT, aucOptElem));
	prBcnReport->ucRegulatoryClass = 255; /* 255 means reglatory is not available */
	prBcnReport->ucChannel = 255; /* 255 means channel is not available */
	prBcnReport->ucReportInfo = 255; /* 255 means report frame info is not available */
	prBcnReport->ucRSNI = 255; /* 255 means RSNI is not available */
	prBcnReport->ucAntennaID = 1;

	prRmRep->u2ReportFrameLen += IE_SIZE(&prRepIE);
}

VOID rlmFreeMeasurementResources(P_ADAPTER_T prAdapter)
{
	struct RADIO_MEASUREMENT_REQ_PARAMS *prRmReq = &prAdapter->rWifiVar.rRmReqParams;
	struct RADIO_MEASUREMENT_REPORT_PARAMS *prRmRep = &prAdapter->rWifiVar.rRmRepParams;
	struct RM_MEASURE_REPORT_ENTRY *prReportEntry = NULL;
	P_LINK_T prReportLink = &prRmRep->rReportLink;
	P_LINK_T prFreeReportLink = &prRmRep->rFreeReportLink;

	kalMemFree(prRmReq->pucReqIeBuf, VIR_MEM_TYPE, prRmReq->u2ReqIeBufLen);
	kalMemFree(prRmRep->pucReportFrameBuff, VIR_MEM_TYPE, RM_REPORT_FRAME_MAX_LENGTH);
	while (!LINK_IS_EMPTY(prReportLink)) {
		LINK_REMOVE_HEAD(prReportLink, prReportEntry, struct RM_MEASURE_REPORT_ENTRY *);
		kalMemFree(prReportEntry, VIR_MEM_TYPE, sizeof(*prReportEntry));
	}
	while (!LINK_IS_EMPTY(prFreeReportLink)) {
		LINK_REMOVE_HEAD(prFreeReportLink, prReportEntry, struct RM_MEASURE_REPORT_ENTRY *);
		kalMemFree(prReportEntry, VIR_MEM_TYPE, sizeof(*prReportEntry));
	}
	kalMemZero(prRmReq, sizeof(*prRmReq));
	kalMemZero(prRmRep, sizeof(*prRmRep));
	prRmReq->rBcnRmParam.eState = RM_NO_REQUEST;
	prRmReq->fgRmIsOngoing = FALSE;
	LINK_INITIALIZE(&prRmRep->rFreeReportLink);
	LINK_INITIALIZE(&prRmRep->rReportLink);
}

/* purpose: check if Radio Measurement is done */
static BOOLEAN rlmAllMeasurementIssued(struct RADIO_MEASUREMENT_REQ_PARAMS *prReq)
{
	return prReq->u2RemainReqLen > IE_SIZE(prReq->prCurrMeasElem) ? FALSE : TRUE;
}

VOID rlmComposeIncapableRmRep(
	struct RADIO_MEASUREMENT_REPORT_PARAMS *prRep, UINT_8 ucToken, UINT_8 ucMeasType)
{
	P_IE_MEASUREMENT_REPORT_T prRepIE =
		(P_IE_MEASUREMENT_REPORT_T)(prRep->pucReportFrameBuff + prRep->u2ReportFrameLen);

	prRepIE->ucId = ELEM_ID_MEASUREMENT_REPORT;
	prRepIE->ucToken = ucToken;
	prRepIE->ucMeasurementType = ucMeasType;
	prRepIE->ucLength = 3;
	prRepIE->ucReportMode = RM_REP_MODE_INCAPABLE;
	prRep->u2ReportFrameLen += 5;
}
/* Purpose: Interative processing Measurement Request Element. If it is not the first element,
**	will copy all collected report element to the report frame buffer. and may tx the radio report frame.
**	prAdapter: pointer to the Adapter
**	fgNewStarted: if it is the first element in measurement request frame
*/
VOID rlmStartNextMeasurement(P_ADAPTER_T prAdapter, BOOLEAN fgNewStarted)
{
	struct RADIO_MEASUREMENT_REQ_PARAMS *prRmReq = &prAdapter->rWifiVar.rRmReqParams;
	struct RADIO_MEASUREMENT_REPORT_PARAMS *prRmRep = &prAdapter->rWifiVar.rRmRepParams;
	P_IE_MEASUREMENT_REQ_T prCurrReq = prRmReq->prCurrMeasElem;
	UINT_16 u2RandomTime = 0;

schedule_next:
	if (!prRmReq->fgRmIsOngoing) {
		DBGLOG(RLM, INFO, "Rm has been stopped\n");
		return;
	}
	/* we don't support parallel measurement now */
	if (prCurrReq->ucRequestMode & RM_REQ_MODE_PARALLEL_BIT) {
		DBGLOG(RLM, WARN, "Parallel request, compose incapable report\n");
		if (prRmRep->u2ReportFrameLen + 5 > RM_REPORT_FRAME_MAX_LENGTH)
			rlmTxRadioMeasurementReport(prAdapter);
		rlmComposeIncapableRmRep(prRmRep, prCurrReq->ucToken, prCurrReq->ucMeasurementType);
		if (rlmAllMeasurementIssued(prRmReq)) {
			if (prRmReq->rBcnRmParam.fgExistBcnReq && RM_EXIST_REPORT(prRmRep))
				rlmComposeEmptyBeaconReport(prAdapter);
			rlmTxRadioMeasurementReport(prAdapter);

			/* repeat measurement if repetitions is required and not only parallel measurements.
			** otherwise, no need to repeat, it is not make sense to do that.
			*/
			if (prRmReq->u2Repetitions > 0) {
				prRmReq->fgInitialLoop = FALSE;
				prRmReq->u2Repetitions--;
				prCurrReq = prRmReq->prCurrMeasElem = (P_IE_MEASUREMENT_REQ_T)prRmReq->pucReqIeBuf;
				prRmReq->u2RemainReqLen = prRmReq->u2ReqIeBufLen;
			} else {
				rlmFreeMeasurementResources(prAdapter);
				DBGLOG(RLM, INFO, "Radio Measurement done\n");
				return;
			}
		} else {
			UINT_16 u2IeSize = IE_SIZE(prRmReq->prCurrMeasElem);

			prCurrReq = prRmReq->prCurrMeasElem =
				(P_IE_MEASUREMENT_REQ_T)((PUINT_8)prRmReq->prCurrMeasElem + u2IeSize);
			prRmReq->u2RemainReqLen -= u2IeSize;
		}
		fgNewStarted = FALSE;
		goto schedule_next;
	}
	/* copy collected measurement report for specific measurement type */
	if (!fgNewStarted) {
		struct RM_MEASURE_REPORT_ENTRY *prReportEntry = NULL;
		P_LINK_T prReportLink = &prRmRep->rReportLink;
		P_LINK_T prFreeReportLink = &prRmRep->rFreeReportLink;
		PUINT_8 pucReportFrame = prRmRep->pucReportFrameBuff + prRmRep->u2ReportFrameLen;
		UINT_16 u2IeSize = 0;
		BOOLEAN fgNewLoop = FALSE;

		DBGLOG(RLM, INFO, "total %u report element for current request\n", prReportLink->u4NumElem);
		/* copy collected report into the Measurement Report Frame Buffer. */
		while (1) {
			LINK_REMOVE_HEAD(prReportLink, prReportEntry, struct RM_MEASURE_REPORT_ENTRY *);
			if (!prReportEntry)
				break;
			u2IeSize = IE_SIZE(prReportEntry->aucMeasReport);
			/* if reach the max length of a MMPDU size, send a Rm report first */
			if (u2IeSize + prRmRep->u2ReportFrameLen > RM_REPORT_FRAME_MAX_LENGTH) {
				rlmTxRadioMeasurementReport(prAdapter);
				pucReportFrame = prRmRep->pucReportFrameBuff + prRmRep->u2ReportFrameLen;
			}
			kalMemCopy(pucReportFrame, prReportEntry->aucMeasReport, u2IeSize);
			pucReportFrame += u2IeSize;
			prRmRep->u2ReportFrameLen += u2IeSize;
			LINK_INSERT_TAIL(prFreeReportLink, &prReportEntry->rLinkEntry);
		}
		/* if Measurement is done, free report element memory */
		if (rlmAllMeasurementIssued(prRmReq)) {
			if (prRmReq->rBcnRmParam.fgExistBcnReq && RM_EXIST_REPORT(prRmRep))
				rlmComposeEmptyBeaconReport(prAdapter);
			rlmTxRadioMeasurementReport(prAdapter);

			/* repeat measurement if repetitions is required */
			if (prRmReq->u2Repetitions > 0) {
				fgNewLoop = TRUE;
				prRmReq->fgInitialLoop = FALSE;
				prRmReq->u2Repetitions--;
				prRmReq->prCurrMeasElem = (P_IE_MEASUREMENT_REQ_T)prRmReq->pucReqIeBuf;
				prRmReq->u2RemainReqLen = prRmReq->u2ReqIeBufLen;
			} else {
				/* don't free radio measurement resource due to TSM is running */
				if (!wmmTsmIsOngoing(prAdapter)) {
					rlmFreeMeasurementResources(prAdapter);
					DBGLOG(RLM, INFO, "Radio Measurement done\n");
				}
				return;
			}
		}
		if (!fgNewLoop) {
			u2IeSize = IE_SIZE(prRmReq->prCurrMeasElem);
			prCurrReq = prRmReq->prCurrMeasElem =
				(P_IE_MEASUREMENT_REQ_T)((PUINT_8)prRmReq->prCurrMeasElem + u2IeSize);
			prRmReq->u2RemainReqLen -= u2IeSize;
		}
	}

	/* do specific measurement */
	switch (prCurrReq->ucMeasurementType) {
	case ELEM_RM_TYPE_BEACON_REQ:
	{
		P_RM_BCN_REQ_T prBeaconReq = (P_RM_BCN_REQ_T)&prCurrReq->aucRequestFields[0];

		if (!prRmReq->fgInitialLoop) {
			/* If this is the repeating measurement, then wait next scan done */
			prRmReq->rBcnRmParam.eState = RM_WAITING;
			break;
		}
		if (prBeaconReq->u2RandomInterval == 0)
			rlmDoBeaconMeasurement(prAdapter, 0);
		else {
			get_random_bytes(&u2RandomTime, 2);
			u2RandomTime = (u2RandomTime * prBeaconReq->u2RandomInterval) / 65535;
			u2RandomTime = TU_TO_MSEC(u2RandomTime);
			if (u2RandomTime > 0) {
				cnmTimerStopTimer(prAdapter, &rBeaconReqTimer);
				cnmTimerInitTimer(prAdapter, &rBeaconReqTimer, rlmDoBeaconMeasurement, 0);
				cnmTimerStartTimer(prAdapter, &rBeaconReqTimer, u2RandomTime);
			} else
				rlmDoBeaconMeasurement(prAdapter, 0);
		}
		break;
	}
#if 0
	case ELEM_RM_TYPE_TSM_REQ:
	{
		P_RM_TS_MEASURE_REQ_T prTsmReqIE = (P_RM_TS_MEASURE_REQ_T)&prCurrReq->aucRequestFields[0];
		struct RM_TSM_REQ *prTsmReq = NULL;
		UINT_16 u2OffSet = 0;
		PUINT_8 pucIE = prTsmReqIE->aucSubElements;
		P_ACTION_RM_REPORT_FRAME prReportFrame = NULL;

		/* In case of repeating measurement, no need to start triggered measurement again.
		** According to current specification of Radio Measurement, only TSM has the triggered
		** type of measurement.
		*/
		if ((prCurrReq->ucRequestMode & RM_REQ_MODE_ENABLE_BIT) && !prRmReq->fgInitialLoop)
			goto schedule_next;

		/* if enable bit is 1 and report bit is 0, need to stop all triggered TSM measurement */
		if ((prCurrReq->ucRequestMode & (RM_REQ_MODE_ENABLE_BIT|RM_REQ_MODE_REPORT_BIT)) ==
			RM_REQ_MODE_ENABLE_BIT) {
			wmmRemoveAllTsmMeasurement(prAdapter, TRUE);
			break;
		}
		prTsmReq = cnmMemAlloc(prAdapter, RAM_TYPE_BUF, sizeof(struct RM_TSM_REQ));
		if (!prTsmReq) {
			DBGLOG(RLM, ERROR, "No memory\n");
			break;
		}
		prTsmReq->ucToken = prCurrReq->ucToken;
		prTsmReq->u2Duration = prTsmReqIE->u2Duration;
		prTsmReq->ucTID = (prTsmReqIE->ucTrafficID & 0xf0) >> 4;
		prTsmReq->ucB0Range = prTsmReqIE->ucBin0Range;
		prReportFrame = (P_ACTION_RM_REPORT_FRAME)prRmRep->pucReportFrameBuff;
		COPY_MAC_ADDR(prTsmReq->aucPeerAddr, prReportFrame->aucDestAddr);
		IE_FOR_EACH(pucIE, prCurrReq->ucLength - 3, u2OffSet) {
			switch (IE_ID(pucIE)) {
			case 1: /* Triggered Reporting */
				kalMemCopy(&prTsmReq->rTriggerCond, pucIE+2, IE_LEN(pucIE));
				break;
			case 221: /* Vendor Specified */
				break; /* No vendor IE now */
			default:
				break;
			}
		}
		if (!prTsmReqIE->u2RandomInterval) {
			wmmStartTsmMeasurement(prAdapter, (ULONG)prTsmReq);
			break;
		}
		get_random_bytes(&u2RandomTime, 2);
		u2RandomTime = (u2RandomTime * prTsmReqIE->u2RandomInterval) / 65535;
		u2RandomTime = TU_TO_MSEC(u2RandomTime);
		cnmTimerStopTimer(prAdapter, &rTSMReqTimer);
		cnmTimerInitTimer(prAdapter, &rTSMReqTimer, wmmStartTsmMeasurement, (ULONG)prTsmReq);
		cnmTimerStartTimer(prAdapter, &rTSMReqTimer, u2RandomTime);
		break;
	}
#endif
	default:
	{
		if (prRmRep->u2ReportFrameLen + 5 > RM_REPORT_FRAME_MAX_LENGTH)
			rlmTxRadioMeasurementReport(prAdapter);
		rlmComposeIncapableRmRep(prRmRep, prCurrReq->ucToken, prCurrReq->ucMeasurementType);
		fgNewStarted = FALSE;
		DBGLOG(RLM, INFO, "RM type %d is not supported on this chip\n", prCurrReq->ucMeasurementType);
		goto schedule_next;
	}
	}
}

/* If disconnect with the target AP, radio measurement should be canceled. */
VOID rlmCancelRadioMeasurement(P_ADAPTER_T prAdapter)
{
	DBGLOG(RLM, INFO, "Cancel measurement, timer is running %d\n", timerPendingTimer(&rBeaconReqTimer));
	cnmTimerStopTimer(prAdapter, &rBeaconReqTimer);
	rlmFreeMeasurementResources(prAdapter);
}

BOOLEAN rlmBcnRmRunning(P_ADAPTER_T prAdapter)
{
	return prAdapter->rWifiVar.rRmReqParams.rBcnRmParam.eState == RM_ON_GOING;
}

#if defined(MT6631)
BOOLEAN rlmFillScanMsg(P_ADAPTER_T prAdapter, struct _MSG_SCN_SCAN_REQ_V3_T *prMsg)
#else
BOOLEAN rlmFillScanMsg(P_ADAPTER_T prAdapter, struct _MSG_SCN_SCAN_REQ_V2_T *prMsg)
#endif
{
	struct RADIO_MEASUREMENT_REQ_PARAMS *prRmReq = &prAdapter->rWifiVar.rRmReqParams;
	P_IE_MEASUREMENT_REQ_T prCurrReq = NULL;
	P_RM_BCN_REQ_T prBeaconReq = NULL;
	UINT_16 u2RemainLen = 0;
	PUINT_8 pucSubIE = NULL;

	static PARAM_SSID_T rBcnReqSsid;

	if (prRmReq->rBcnRmParam.eState != RM_ON_GOING || !prMsg)
		return FALSE;

	prCurrReq = prRmReq->prCurrMeasElem;
	prBeaconReq = (P_RM_BCN_REQ_T)&prCurrReq->aucRequestFields[0];
	prMsg->ucSSIDType = SCAN_REQ_SSID_WILDCARD;
	switch (prBeaconReq->ucMeasurementMode) {
	case RM_BCN_REQ_PASSIVE_MODE:
		prMsg->eScanType = SCAN_TYPE_PASSIVE_SCAN;
		break;
	case RM_BCN_REQ_ACTIVE_MODE:
		prMsg->eScanType = SCAN_TYPE_ACTIVE_SCAN;
		break;
	default:
		DBGLOG(RLM, WARN, "Unexpect measure mode %d, use active mode as default\n",
			   prBeaconReq->ucMeasurementMode);
		prMsg->eScanType = SCAN_TYPE_ACTIVE_SCAN;
		break;
	}

	WLAN_GET_FIELD_16(&prBeaconReq->u2Duration, &prMsg->u2ChannelDwellTime);

	COPY_MAC_ADDR(prMsg->aucBSSID, prBeaconReq->aucBssid);

	prMsg->u2ProbeDelay = 0;
	prMsg->u2TimeoutValue = 0;
	prMsg->ucSSIDNum = 0;
	prMsg->u2IELen = 0;
	/* if mandatory bit is set, we should do */
	if (prCurrReq->ucRequestMode & RM_REQ_MODE_DURATION_MANDATORY_BIT)
		prMsg->u2MinChannelDwellTime = prMsg->u2ChannelDwellTime;
	else
		prMsg->u2MinChannelDwellTime = (prMsg->u2ChannelDwellTime * 2) / 3;
	if (prBeaconReq->ucChannel == 0)
		prMsg->eScanChannel = SCAN_CHANNEL_FULL;
	else if (prBeaconReq->ucChannel == 255) { /* latest Ap Channel Report */
		P_BSS_DESC_T prBssDesc = prAdapter->rWifiVar.rAisFsmInfo.prTargetBssDesc;
		PUINT_8 pucChnl = NULL;
		UINT_8 ucChnlNum = 0;
		UINT_8 ucIndex = 0;
		P_RF_CHANNEL_INFO_T prChnlInfo = prMsg->arChnlInfoList;

		prMsg->eScanChannel = SCAN_CHANNEL_SPECIFIED;
		prMsg->ucChannelListNum = 0;
		if (prBssDesc) {
			PUINT_8 pucIE = NULL;
			UINT_16 u2IELength = 0;
			UINT_16 u2Offset = 0;

			pucIE = prBssDesc->aucIEBuf;
			u2IELength = prBssDesc->u2IELength;
			IE_FOR_EACH(pucIE, u2IELength, u2Offset) {
				if (IE_ID(pucIE) != ELEM_ID_AP_CHANNEL_REPORT)
					continue;
				pucChnl = ((struct IE_AP_CHNL_REPORT_T *)pucIE)->aucChnlList;
				ucChnlNum = pucIE[1] - 1;
				DBGLOG(RLM, INFO, "Channel number in latest AP channel report %d\n", ucChnlNum);
				while (ucIndex < ucChnlNum &&
					prMsg->ucChannelListNum < MAXIMUM_OPERATION_CHANNEL_LIST) {
					if (pucChnl[ucIndex] <= 14)
						prChnlInfo[prMsg->ucChannelListNum].eBand = BAND_2G4;
					else
						prChnlInfo[prMsg->ucChannelListNum].eBand = BAND_5G;
					prChnlInfo[prMsg->ucChannelListNum].ucChannelNum = pucChnl[ucIndex];
					prMsg->ucChannelListNum++;
					ucIndex++;
				}
			}
		}
	} else {
		prMsg->eScanChannel = SCAN_CHANNEL_SPECIFIED;
		prMsg->ucChannelListNum = 1;
		prMsg->arChnlInfoList[0].ucChannelNum = prBeaconReq->ucChannel;
		if (prBeaconReq->ucChannel <= 14)
			prMsg->arChnlInfoList[0].eBand = BAND_2G4;
		else
			prMsg->arChnlInfoList[0].eBand = BAND_5G;
	}
	u2RemainLen = prCurrReq->ucLength - 3 - OFFSET_OF(RM_BCN_REQ_T, aucSubElements);
	pucSubIE = &prBeaconReq->aucSubElements[0];
	while (u2RemainLen > 0) {
		if (IE_SIZE(pucSubIE) > u2RemainLen)
			break;
		switch (pucSubIE[0]) {
		case 0: /* SSID */
			/* length of sub-element ssid is 0 or first byte is 0, means wildcard ssid matching */
			if (!IE_LEN(pucSubIE) || !pucSubIE[2])
				break;
			prMsg->ucSSIDNum = 1;
			prMsg->prSsid = &rBcnReqSsid;
			COPY_SSID(&rBcnReqSsid.aucSsid[0], rBcnReqSsid.u4SsidLen, &pucSubIE[2], pucSubIE[1]);
			prMsg->ucSSIDType = SCAN_REQ_SSID_SPECIFIED_ONLY;
			break;
		case 51: /* AP channel report */
		{
			struct IE_AP_CHNL_REPORT_T *prApChnl = (struct IE_AP_CHNL_REPORT_T *)pucSubIE;
			UINT_8 ucChannelCnt = prApChnl->ucLength - 1;
			UINT_8 ucIndex = 0;

			if (prBeaconReq->ucChannel == 0)
				break;
			prMsg->eScanChannel = SCAN_CHANNEL_SPECIFIED;
			DBGLOG(RLM, INFO, "Channel number in measurement AP channel report %d\n", ucChannelCnt);
			while (ucIndex < ucChannelCnt &&
				prMsg->ucChannelListNum < MAXIMUM_OPERATION_CHANNEL_LIST) {
				if (prApChnl->aucChnlList[ucIndex] <= 14)
					prMsg->arChnlInfoList[prMsg->ucChannelListNum].eBand = BAND_2G4;
				else
					prMsg->arChnlInfoList[prMsg->ucChannelListNum].eBand = BAND_5G;
				prMsg->arChnlInfoList[prMsg->ucChannelListNum].ucChannelNum =
					prApChnl->aucChnlList[ucIndex];
				prMsg->ucChannelListNum++;
				ucIndex++;
			}
			break;
		}
		}
		u2RemainLen -= IE_SIZE(pucSubIE);
		pucSubIE += IE_SIZE(pucSubIE);
	}
	DBGLOG(RLM, INFO, "SSIDtype %d, ScanType %d, Dwell %d, MinDwell %d, ChnlType %d, ChnlNum %d\n",
		prMsg->ucSSIDType, prMsg->eScanType, prMsg->u2ChannelDwellTime,
		prMsg->u2MinChannelDwellTime, prMsg->eScanChannel, prMsg->ucChannelListNum);
	return TRUE;
}

VOID rlmDoBeaconMeasurement(P_ADAPTER_T prAdapter, ULONG ulParam)
{
	P_CONNECTION_SETTINGS_T prConnSettings = &(prAdapter->rWifiVar.rConnSettings);
	struct RADIO_MEASUREMENT_REQ_PARAMS *prRmReq = &prAdapter->rWifiVar.rRmReqParams;
	P_RM_BCN_REQ_T prBcnReq = (P_RM_BCN_REQ_T)&prRmReq->prCurrMeasElem->aucRequestFields[0];

	if (prBcnReq->ucMeasurementMode == RM_BCN_REQ_TABLE_MODE) {
		P_LINK_T prBSSDescList = &prAdapter->rWifiVar.rScanInfo.rBSSDescList;
		P_BSS_DESC_T prBssDesc = NULL;
		struct RM_BEACON_REPORT_PARAMS rRepParams;
		PUINT_16 pu2BcnInterval = (PUINT_16)&rRepParams.aucBcnFixedField[8];
		PUINT_16 pu2CapInfo = (PUINT_16)&rRepParams.aucBcnFixedField[10];

		kalMemZero(&rRepParams, sizeof(rRepParams));
		/* if this is a one antenna only device, the antenna id is always 1. 7.3.2.40 */
		rRepParams.ucAntennaID = 1;
		rRepParams.ucRSNI = 255; /* 255 means RSNI not available. see 7.3.2.41 */
		rRepParams.ucFrameInfo = 255;

		prRmReq->rBcnRmParam.eState = RM_ON_GOING;
		prBcnReq->ucChannel = 0;
		DBGLOG(RLM, INFO, "Beacon Table Mode, Beacon Table Num %u\n", prBSSDescList->u4NumElem);
		LINK_FOR_EACH_ENTRY(prBssDesc, prBSSDescList, rLinkEntry, BSS_DESC_T) {
			rRepParams.ucRCPI = prBssDesc->ucRCPI;
			rRepParams.ucChannel = prBssDesc->ucChannelNum;
			kalMemCopy(&rRepParams.aucBcnFixedField, &prBssDesc->u8TimeStamp, 8);
			*pu2BcnInterval = prBssDesc->u2BeaconInterval;
			*pu2CapInfo = prBssDesc->u2CapInfo;
			scanCollectBeaconReport(prAdapter, prBssDesc->aucIEBuf, prBssDesc->u2IELength,
				prBssDesc->aucBSSID, &rRepParams);
		}
		rlmStartNextMeasurement(prAdapter, FALSE);
		return;
	}
	if (prConnSettings->fgIsScanReqIssued) {
		prRmReq->rBcnRmParam.eState = RM_WAITING;
	} else {
		prRmReq->rBcnRmParam.eState = RM_ON_GOING;
		GET_CURRENT_SYSTIME(&prRmReq->rStartTime);
		aisFsmScanRequest(prAdapter, NULL, NULL, 0);
	}
}

/*
*/
static BOOLEAN rlmRmFrameIsValid(P_SW_RFB_T prSwRfb)
{
	UINT_16 u2ElemLen = 0;
	UINT_16 u2Offset = (UINT_16)OFFSET_OF(ACTION_RM_REQ_FRAME, aucInfoElem);
	PUINT_8 pucIE = (PUINT_8)prSwRfb->pvHeader;
	UINT_16 u2CalcIELen = 0;

	if (prSwRfb->u2PacketLen <= u2Offset) {
		DBGLOG(RLM, ERROR, "Rm Packet length %d is too short\n", prSwRfb->u2PacketLen);
		return FALSE;
	}
	pucIE += u2Offset;
	u2ElemLen = prSwRfb->u2PacketLen - u2Offset;
	IE_FOR_EACH(pucIE, u2ElemLen, u2Offset) {
		if (!IE_LEN(pucIE)) {
			DBGLOG(RLM, ERROR, "RM IE length is 0\n");
			return FALSE;
		}
		u2CalcIELen += IE_SIZE(pucIE);
	}
	if (u2CalcIELen != u2ElemLen) {
		DBGLOG(RLM, ERROR, "Calculated Total IE len is not equal to received length\n");
		return FALSE;
	}
	return TRUE;
}
/*
*/
VOID rlmProcessRadioMeasurementRequest(P_ADAPTER_T prAdapter, P_SW_RFB_T prSwRfb)
{
	P_ACTION_RM_REQ_FRAME prRmReqFrame = NULL;
	P_ACTION_RM_REPORT_FRAME prReportFrame = NULL;
	struct RADIO_MEASUREMENT_REQ_PARAMS *prRmReqParam = NULL;
	struct RADIO_MEASUREMENT_REPORT_PARAMS *prRmRepParam = NULL;
	enum RM_REQ_PRIORITY eNewPriority;

	ASSERT(prAdapter);
	ASSERT(prSwRfb);
	ASSERT(prAdapter->prAisBssInfo);
	prRmReqFrame = (P_ACTION_RM_REQ_FRAME)prSwRfb->pvHeader;
	prRmReqParam = &prAdapter->rWifiVar.rRmReqParams;
	prRmRepParam = &prAdapter->rWifiVar.rRmRepParams;

	if (!rlmRmFrameIsValid(prSwRfb))
		return;
	DBGLOG(RLM, INFO, "RM Request From %pM, DialogToken %d\n",
			prRmReqFrame->aucSrcAddr, prRmReqFrame->ucDialogToken);
	eNewPriority = rlmGetRmRequestPriority(prRmReqFrame->aucDestAddr);
	if (prRmReqParam->ePriority > eNewPriority) {
		DBGLOG(RLM, INFO, "ignore lower precedence rm request\n");
		return;
	}
	prRmReqParam->ePriority = eNewPriority;
	/* */
	if (prRmReqParam->fgRmIsOngoing) {
		DBGLOG(RLM, INFO, "Old RM is on-going, cancel it first\n");
		rlmTxRadioMeasurementReport(prAdapter);
		wmmRemoveAllTsmMeasurement(prAdapter, FALSE);
		rlmFreeMeasurementResources(prAdapter);
	}
	prRmReqParam->fgRmIsOngoing = TRUE;
	/* Step1: Save Measurement Request Params */
	prRmReqParam->u2ReqIeBufLen = prRmReqParam->u2RemainReqLen =
		prSwRfb->u2PacketLen - OFFSET_OF(ACTION_RM_REQ_FRAME, aucInfoElem);
	if (prRmReqParam->u2RemainReqLen <= sizeof(IE_MEASUREMENT_REQ_T)) {
		DBGLOG(RLM, ERROR, "empty Radio Measurement Request Frame, Elem Len %d\n",
			prRmReqParam->u2RemainReqLen);
		return;
	}
	WLAN_GET_FIELD_BE16(&prRmReqFrame->u2Repetitions, &prRmReqParam->u2Repetitions);
	prRmReqParam->pucReqIeBuf = kalMemAlloc(prRmReqParam->u2RemainReqLen, VIR_MEM_TYPE);
	if (!prRmReqParam->pucReqIeBuf) {
		DBGLOG(RLM, ERROR, "Alloc %d bytes Req IE Buffer failed, No Memory\n", prRmReqParam->u2RemainReqLen);
		return;
	}
	kalMemCopy(prRmReqParam->pucReqIeBuf, &prRmReqFrame->aucInfoElem[0], prRmReqParam->u2RemainReqLen);
	prRmReqParam->prCurrMeasElem = (P_IE_MEASUREMENT_REQ_T)prRmReqParam->pucReqIeBuf;
	prRmReqParam->fgInitialLoop = TRUE;

	/* Step2: Prepare Report Frame and fill in Frame Header */
	prRmRepParam->pucReportFrameBuff = kalMemAlloc(RM_REPORT_FRAME_MAX_LENGTH, VIR_MEM_TYPE);
	if (!prRmRepParam->pucReportFrameBuff) {
		DBGLOG(RLM, ERROR, "Alloc Memory for Measurement Report Frame buffer failed\n");
		return;
	}
	kalMemZero(prRmRepParam->pucReportFrameBuff, RM_REPORT_FRAME_MAX_LENGTH);
	prReportFrame = (P_ACTION_RM_REPORT_FRAME)prRmRepParam->pucReportFrameBuff;
	prReportFrame->u2FrameCtrl = MAC_FRAME_ACTION;
	COPY_MAC_ADDR(prReportFrame->aucDestAddr, prRmReqFrame->aucSrcAddr);
	COPY_MAC_ADDR(prReportFrame->aucSrcAddr, prAdapter->prAisBssInfo->aucOwnMacAddr);
	COPY_MAC_ADDR(prReportFrame->aucBSSID, prRmReqFrame->aucBSSID);
	prReportFrame->ucCategory = CATEGORY_RM_ACTION;
	prReportFrame->ucAction = RM_ACTION_RM_REPORT;
	prReportFrame->ucDialogToken = prRmReqFrame->ucDialogToken;
	prRmRepParam->u2ReportFrameLen = OFFSET_OF(ACTION_RM_REPORT_FRAME, aucInfoElem);
	rlmCalibrateRepetions(prRmReqParam);
	/* Step3: Start to process Measurement Request Element */
	rlmStartNextMeasurement(prAdapter, TRUE);
}

VOID rlmTxRadioMeasurementReport(P_ADAPTER_T prAdapter)
{
	P_MSDU_INFO_T prMsduInfo = NULL;
	struct RADIO_MEASUREMENT_REPORT_PARAMS *prRmRepParam = &prAdapter->rWifiVar.rRmRepParams;
	P_STA_RECORD_T prStaRec = NULL;

	if (prRmRepParam->u2ReportFrameLen <= OFFSET_OF(ACTION_RM_REPORT_FRAME, aucInfoElem)) {
		DBGLOG(RLM, INFO, "report frame length is too short, %d\n", prRmRepParam->u2ReportFrameLen);
		return;
	}
	if (!prAdapter->prAisBssInfo) {
		DBGLOG(RLM, INFO, "ais bss info is NULL\n");
		return;
	}
	prStaRec = prAdapter->prAisBssInfo->prStaRecOfAP;
	if (!prStaRec) {
		DBGLOG(RLM, INFO, "StaRec of Ais is NULL\n");
		return;
	}
	prMsduInfo = (P_MSDU_INFO_T) cnmMgtPktAlloc(prAdapter, prRmRepParam->u2ReportFrameLen);
	if (!prMsduInfo) {
		DBGLOG(RLM, INFO, "Alloc MSDU Info failed, frame length %d\n", prRmRepParam->u2ReportFrameLen);
		return;
	}
	DBGLOG(RLM, INFO, "frame length %d\n", prRmRepParam->u2ReportFrameLen);
	kalMemCopy(prMsduInfo->prPacket, prRmRepParam->pucReportFrameBuff, prRmRepParam->u2ReportFrameLen);

	/* 2 Update information of MSDU_INFO_T */
	TX_SET_MMPDU(prAdapter,
		     prMsduInfo,
		     prStaRec->ucBssIndex,
		     prStaRec->ucIndex,
		     WLAN_MAC_MGMT_HEADER_LEN,
		     prRmRepParam->u2ReportFrameLen, NULL, MSDU_RATE_MODE_AUTO);
	nicTxEnqueueMsdu(prAdapter, prMsduInfo);
	/* reset u2ReportFrameLen after tx frame */
	prRmRepParam->u2ReportFrameLen = OFFSET_OF(ACTION_RM_REPORT_FRAME, aucInfoElem);
}



VOID rlmGenerateRRMEnabledCapIE(IN P_ADAPTER_T prAdapter, IN P_MSDU_INFO_T prMsduInfo)
{
	P_IE_RRM_ENABLED_CAP_T prRrmEnabledCap = NULL;

	ASSERT(prAdapter);
	ASSERT(prMsduInfo);

	prRrmEnabledCap = (P_IE_RRM_ENABLED_CAP_T)
	    (((PUINT_8) prMsduInfo->prPacket) + prMsduInfo->u2FrameLength);
	prRrmEnabledCap->ucId = ELEM_ID_RRM_ENABLED_CAP;
	prRrmEnabledCap->ucLength = ELEM_MAX_LEN_RRM_CAP;
	kalMemZero(&prRrmEnabledCap->aucCap[0], ELEM_MAX_LEN_RRM_CAP);
	rlmFillRrmCapa(&prRrmEnabledCap->aucCap[0]);
	prMsduInfo->u2FrameLength += IE_SIZE(prRrmEnabledCap);
}

VOID rlmFillRrmCapa(PUINT_8 pucCapa)
{
	UINT_8 ucIndex = 0;
	UINT_8 aucEnabledBits[] = {RRM_CAP_INFO_LINK_MEASURE_BIT, RRM_CAP_INFO_NEIGHBOR_REPORT_BIT,
			RRM_CAP_INFO_REPEATED_MEASUREMENT, RRM_CAP_INFO_BEACON_PASSIVE_MEASURE_BIT,
			RRM_CAP_INFO_BEACON_ACTIVE_MEASURE_BIT, RRM_CAP_INFO_BEACON_TABLE_BIT, RRM_CAP_INFO_RRM_BIT};

	for (; ucIndex < sizeof(aucEnabledBits); ucIndex++)
		SET_EXT_CAP(pucCapa, ELEM_MAX_LEN_RRM_CAP, aucEnabledBits[ucIndex]);
}

VOID rlmGeneratePowerCapIE(IN P_ADAPTER_T prAdapter, IN P_MSDU_INFO_T prMsduInfo)
{
	P_IE_POWER_CAP_T prPwrCap = NULL;
	UINT_8 ucChannel = 0;

	ASSERT(prAdapter);
	ASSERT(prMsduInfo);

	ucChannel = prAdapter->rWifiVar.rAisFsmInfo.prTargetBssDesc->ucChannelNum;
	prPwrCap = (P_IE_POWER_CAP_T)
	    (((PUINT_8) prMsduInfo->prPacket) + prMsduInfo->u2FrameLength);
	prPwrCap->ucId = ELEM_ID_PWR_CAP;
	prPwrCap->ucLength = 2;
	prPwrCap->cMaxTxPowerCap = RLM_MAX_TX_PWR;
	prPwrCap->cMinTxPowerCap = RLM_MIN_TX_PWR;
	prMsduInfo->u2FrameLength += IE_SIZE(prPwrCap);
}

VOID rlmSetMaxTxPwrLimit(IN P_ADAPTER_T prAdapter, INT_8 cLimit, UINT_8 ucEnable)
{
	struct CMD_SET_MAX_TXPWR_LIMIT rTxPwrLimit;

	kalMemZero(&rTxPwrLimit, sizeof(rTxPwrLimit));
	rTxPwrLimit.ucMaxTxPwrLimitEnable =  ucEnable;
	if (ucEnable) {
		if (cLimit > RLM_MAX_TX_PWR) {
			DBGLOG(RLM, INFO, "Target MaxPwr %d Higher than Capability, reset to capability\n", cLimit);
			cLimit = RLM_MAX_TX_PWR;
		}
		if (cLimit < RLM_MIN_TX_PWR) {
			DBGLOG(RLM, INFO, "Target MinPwr %d Lower than Capability, reset to capability\n", cLimit);
			cLimit = RLM_MIN_TX_PWR;
		}
		DBGLOG(RLM, INFO, "Set Max Tx Power Limit %d, Min Limit %d\n", cLimit, RLM_MIN_TX_PWR);
		rTxPwrLimit.cMaxTxPwr = cLimit * 2; /* unit of cMaxTxPwr is 0.5 dBm */
		rTxPwrLimit.cMinTxPwr = RLM_MIN_TX_PWR * 2;
	} else
		DBGLOG(RLM, TRACE, "Disable Tx Power Limit\n");
	wlanSendSetQueryCmd(prAdapter,
					  CMD_ID_SET_MAX_TXPWR_LIMIT,
					  TRUE,
					  FALSE,
					  FALSE,
					  nicCmdEventSetCommon,
					  nicOidCmdTimeoutCommon,
					  sizeof(struct CMD_SET_MAX_TXPWR_LIMIT),
					  (PUINT_8) &rTxPwrLimit, NULL, 0);
}

enum RM_REQ_PRIORITY rlmGetRmRequestPriority(PUINT_8 pucDestAddr)
{
	if (IS_UCAST_MAC_ADDR(pucDestAddr))
		return RM_PRI_UNICAST;
	else if (EQUAL_MAC_ADDR(pucDestAddr, "\xff\xff\xff\xff\xff\xff"))
		return RM_PRI_BROADCAST;
	return RM_PRI_MULTICAST;
}

static VOID rlmCalibrateRepetions(struct RADIO_MEASUREMENT_REQ_PARAMS *prRmReq)
{
	UINT_16 u2IeSize = 0;
	UINT_16 u2RemainReqLen = prRmReq->u2ReqIeBufLen;
	P_IE_MEASUREMENT_REQ_T prCurrReq = (P_IE_MEASUREMENT_REQ_T)prRmReq->prCurrMeasElem;

	if (prRmReq->u2Repetitions == 0)
		return;

	u2IeSize = IE_SIZE(prCurrReq);
	while (u2RemainReqLen >= u2IeSize) {
		/* 1. If all measurement request has enable bit, no need to repeat
		** see 11.10.6 Measurement request elements with the enable bit set to 1 shall be processed once
		** regardless of the value in the number of repetitions in the measurement request.
		** 2. Due to we don't support parallel measurement, if all request has parallel bit, no need to repeat
		** measurement, to avoid frequent composing incapable response IE and exhauste CPU resource
		** and then cause watch dog timeout.
		** 3. if all measurements are not supported, no need to repeat. currently we only support Beacon request
		** on this chip.
		*/
		if (!(prCurrReq->ucRequestMode & (RM_REQ_MODE_ENABLE_BIT | RM_REQ_MODE_PARALLEL_BIT))) {
			if (prCurrReq->ucMeasurementType == ELEM_RM_TYPE_BEACON_REQ)
				return;
		}
		u2RemainReqLen -= u2IeSize;
		prCurrReq = (P_IE_MEASUREMENT_REQ_T)((PUINT_8)prCurrReq + u2IeSize);
		u2IeSize = IE_SIZE(prCurrReq);
	}
	DBGLOG(RLM, INFO,
		"All Measurement has set enable bit, or all are parallel or not supported, don't repeat\n");
	prRmReq->u2Repetitions = 0;
}

VOID rlmRunEventProcessNextRm(P_ADAPTER_T prAdapter, P_MSG_HDR_T prMsgHdr)
{
	cnmMemFree(prAdapter, prMsgHdr);
	rlmStartNextMeasurement(prAdapter, FALSE);
}

VOID rlmScheduleNextRm(P_ADAPTER_T prAdapter)
{
	P_MSG_HDR_T prMsg = NULL;

	prMsg = cnmMemAlloc(prAdapter, RAM_TYPE_MSG, sizeof(*prMsg));
	if (!prMsg) {
		DBGLOG(RLM, ERROR, "No memory\n");
		return;
	}
	prMsg->eMsgId = MID_RLM_RM_SCHEDULE;
	mboxSendMsg(prAdapter, MBOX_ID_0, prMsg, MSG_SEND_METHOD_BUF);
}

