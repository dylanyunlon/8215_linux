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
** Id: Department/DaVinci/TRUNK/WiFi_P2P_Driver/common/wlan_p2p.c#8
*/

/*
 * \
 * ! \file wlan_bow.c
 *    \brief This file contains the Wi-Fi Direct commands processing routines for
 *     MediaTek Inc. 802.11 Wireless LAN Adapters.
 */

/******************************************************************************
*                         C O M P I L E R   F L A G S
*******************************************************************************
*/

/******************************************************************************
*                    E X T E R N A L   R E F E R E N C E S
*******************************************************************************
*/
#include "precomp.h"
#include "gl_p2p_ioctl.h"

/******************************************************************************
*                              C O N S T A N T S
*******************************************************************************
*/

/******************************************************************************
*                             D A T A   T Y P E S
*******************************************************************************
*/

/******************************************************************************
*                            P U B L I C   D A T A
*******************************************************************************
*/

/******************************************************************************
*                           P R I V A T E   D A T A
*******************************************************************************
*/

/******************************************************************************
*                                 M A C R O S
*******************************************************************************
*/

/******************************************************************************
*                   F U N C T I O N   D E C L A R A T I O N S
*******************************************************************************
*/

/******************************************************************************
*                              F U N C T I O N S
*******************************************************************************
*/
/*----------------------------------------------------------------------------*/
/*!
* \brief command packet generation utility
*
* \param[in] prAdapter          Pointer to the Adapter structure.
* \param[in] ucCID              Command ID
* \param[in] fgSetQuery         Set or Query
* \param[in] fgNeedResp         Need for response
* \param[in] pfCmdDoneHandler   Function pointer when command is done
* \param[in] u4SetQueryInfoLen  The length of the set/query buffer
* \param[in] pucInfoBuffer      Pointer to set/query buffer
*
*
* \retval WLAN_STATUS_PENDING
* \retval WLAN_STATUS_FAILURE
*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
wlanoidSendSetQueryP2PCmd(IN P_ADAPTER_T prAdapter,
			  IN UINT_8 ucCID,
			  IN UINT_8 ucBssIdx,
			  IN BOOLEAN fgSetQuery,
			  IN BOOLEAN fgNeedResp,
			  IN BOOLEAN fgIsOid,
			  IN PFN_CMD_DONE_HANDLER pfCmdDoneHandler,
			  IN PFN_CMD_TIMEOUT_HANDLER pfCmdTimeoutHandler,
			  IN UINT_32 u4SetQueryInfoLen,
			  IN PUINT_8 pucInfoBuffer, OUT PVOID pvSetQueryBuffer, IN UINT_32 u4SetQueryBufferLen)
{
	P_GLUE_INFO_T prGlueInfo;
	P_CMD_INFO_T prCmdInfo;
	P_WIFI_CMD_T prWifiCmd;
	UINT_8 ucCmdSeqNum;

	ASSERT(prAdapter);

	prGlueInfo = prAdapter->prGlueInfo;
	ASSERT(prGlueInfo);

	DEBUGFUNC("wlanoidSendSetQueryP2PCmd");
	DBGLOG(REQ, TRACE, "Command ID = 0x%08X\n", ucCID);

	prCmdInfo = cmdBufAllocateCmdInfo(prAdapter,
		((UINT_32)CMD_HDR_SIZE + u4SetQueryInfoLen));

	if (prCmdInfo == NULL) {
		DBGLOG(INIT, ERROR, "Allocate CMD_INFO_T ==> FAILED.\n");
		return WLAN_STATUS_FAILURE;
	}
	/* increase command sequence number */
	ucCmdSeqNum = nicIncreaseCmdSeqNum(prAdapter);
	DBGLOG(REQ, TRACE, "ucCmdSeqNum =%d\n", ucCmdSeqNum);

	/* Setup common CMD Info Packet */
	prCmdInfo->eCmdType = COMMAND_TYPE_NETWORK_IOCTL;
	prCmdInfo->ucBssIndex = ucBssIdx;
	prCmdInfo->u2InfoBufLen = (UINT_16) (CMD_HDR_SIZE + u4SetQueryInfoLen);
	prCmdInfo->pfCmdDoneHandler = pfCmdDoneHandler;
	prCmdInfo->pfCmdTimeoutHandler = pfCmdTimeoutHandler;
	prCmdInfo->fgIsOid = fgIsOid;
	prCmdInfo->ucCID = ucCID;
	prCmdInfo->fgSetQuery = fgSetQuery;
	prCmdInfo->fgNeedResp = fgNeedResp;
	prCmdInfo->fgDriverDomainMCR = FALSE;
	prCmdInfo->ucCmdSeqNum = ucCmdSeqNum;
	prCmdInfo->u4SetInfoLen = u4SetQueryInfoLen;
	prCmdInfo->pvInformationBuffer = pvSetQueryBuffer;
	prCmdInfo->u4InformationBufferLength = u4SetQueryBufferLen;

	/* Setup WIFI_CMD_T (no payload) */
	prWifiCmd = (P_WIFI_CMD_T) (prCmdInfo->pucInfoBuffer);
	prWifiCmd->u2TxByteCount = prCmdInfo->u2InfoBufLen;
	prWifiCmd->ucCID = prCmdInfo->ucCID;
	prWifiCmd->ucSetQuery = prCmdInfo->fgSetQuery;
	prWifiCmd->ucSeqNum = prCmdInfo->ucCmdSeqNum;

	if (u4SetQueryInfoLen > 0U && pucInfoBuffer != NULL)
		kalMemCopy(prWifiCmd->aucBuffer, pucInfoBuffer, u4SetQueryInfoLen);
	/* insert into prCmdQueue */
	kalEnqueueCommand(prGlueInfo, &(prCmdInfo->rQueEntry));

	/* wakeup txServiceThread later */
	GLUE_SET_EVENT(prGlueInfo);
	return WLAN_STATUS_PENDING;
}

/*----------------------------------------------------------------------------*/
/*!
* \brief This routine is called to set a key to Wi-Fi Direct driver
*
* \param[in] prAdapter Pointer to the Adapter structure.
* \param[in] pvSetBuffer A pointer to the buffer that holds the data to be set.
* \param[in] u4SetBufferLen The length of the set buffer.
* \param[out] pu4SetInfoLen If the call is successful, returns the number of
*                          bytes read from the set buffer. If the call failed
*                          due to invalid length of the set buffer, returns
*                          the amount of storage needed.
*
* \retval WLAN_STATUS_SUCCESS
* \retval WLAN_STATUS_ADAPTER_NOT_READY
* \retval WLAN_STATUS_INVALID_LENGTH
* \retval WLAN_STATUS_INVALID_DATA
*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
wlanoidSetAddP2PKey(IN P_ADAPTER_T prAdapter,
		    IN PVOID pvSetBuffer, IN UINT_32 u4SetBufferLen, OUT PUINT_32 pu4SetInfoLen)
{
	CMD_802_11_KEY rCmdKey;
	P_PARAM_KEY_T prNewKey;
	P_BSS_INFO_T prBssInfo = (P_BSS_INFO_T) NULL;
	P_STA_RECORD_T prStaRec = (P_STA_RECORD_T) NULL;

	DEBUGFUNC("wlanoidSetAddP2PKey");
	DBGLOG(REQ, INFO, "\n");

	ASSERT(prAdapter);
	ASSERT(pvSetBuffer);
	ASSERT(pu4SetInfoLen);

	prNewKey = (P_PARAM_KEY_T) pvSetBuffer;

	/* Verify the key structure length. */
	if (prNewKey->u4Length > u4SetBufferLen) {
		DBGLOG(REQ, WARN,
		       "Invalid key structure length (%d) greater than total buffer length (%d)\n",
			(UINT_8) prNewKey->u4Length, (UINT_8) u4SetBufferLen);

		*pu4SetInfoLen = u4SetBufferLen;
		return WLAN_STATUS_INVALID_LENGTH;
	}

	/* Verify the key material length for key material buffer */
	if (prNewKey->u4KeyLength > prNewKey->u4Length -
		OFFSET_OF(PARAM_KEY_T, aucKeyMaterial)) {
		DBGLOG(REQ, WARN, "Invalid key material length (%d)\n", (UINT_8) prNewKey->u4KeyLength);
		*pu4SetInfoLen = u4SetBufferLen;
		return WLAN_STATUS_INVALID_DATA;
	}

	/* Exception check */
	if ((prNewKey->u4KeyIndex & 0x0fffff00U) != 0U)
		return WLAN_STATUS_INVALID_DATA;

	/* Exception check, pairwise key must with transmit bit enabled */
	if ((prNewKey->u4KeyIndex & BITS(30U, 31U)) == IS_UNICAST_KEY)
		return WLAN_STATUS_INVALID_DATA;

	if (!(prNewKey->u4KeyLength == CCMP_KEY_LEN)
		   && !(prNewKey->u4KeyLength == TKIP_KEY_LEN)) {
		return WLAN_STATUS_INVALID_DATA;
	}

	/* Exception check, pairwise key must with transmit bit enabled */
	if ((prNewKey->u4KeyIndex & BITS(30U, 31U)) == BITS(30U, 31U)) {
		if (((prNewKey->u4KeyIndex & 0xffU) != 0U) ||
		    ((prNewKey->arBSSID[0] == 0xffU) &&
		    (prNewKey->arBSSID[1] == 0xffU)
		     && (prNewKey->arBSSID[2] == 0xffU) &&
		     (prNewKey->arBSSID[3] == 0xffU)
		     && (prNewKey->arBSSID[4] == 0xffU) &&
		     (prNewKey->arBSSID[5] == 0xffU))) {
			return WLAN_STATUS_INVALID_DATA;
		}
	}

	*pu4SetInfoLen = u4SetBufferLen;

	prBssInfo = GET_BSS_INFO_BY_INDEX(prAdapter, prNewKey->ucBssIdx);
	ASSERT(prBssInfo);

	if (prBssInfo->ucBMCWlanIndex >= WTBL_SIZE) {
		prBssInfo->ucBMCWlanIndex =
		    secPrivacySeekForBcEntry(prAdapter, prBssInfo->ucBssIndex, prBssInfo->aucBSSID,
			0xff, CIPHER_SUITE_NONE, 0xff, 0x0, (UINT_8)BIT(0U));
	}
	/* fill CMD_802_11_KEY */
	kalMemZero(&rCmdKey, (UINT_32)sizeof(CMD_802_11_KEY));
	rCmdKey.ucAddRemove = 1U;	/* add */
	rCmdKey.ucTxKey = ((prNewKey->u4KeyIndex & IS_TRANSMIT_KEY)
		== IS_TRANSMIT_KEY) ? 1U : 0U;
	rCmdKey.ucKeyType = ((prNewKey->u4KeyIndex & IS_UNICAST_KEY)
		== IS_UNICAST_KEY) ? 1U : 0U;
#if 0
	if (prBssInfo->eCurrentOPMode == OP_MODE_INFRASTRUCTURE) {	/* group client */
#else
	if (kalP2PGetRole(prAdapter->prGlueInfo) == 1U) {
		/* group client */
#endif

		rCmdKey.ucIsAuthenticator = 0U;
	} else {		/* group owner */
		rCmdKey.ucIsAuthenticator = 1U;
		/* Force to set GO/AP Tx */
		rCmdKey.ucTxKey = 1U;
	}

	COPY_MAC_ADDR(rCmdKey.aucPeerAddr, prNewKey->arBSSID);
	rCmdKey.ucBssIdx = prNewKey->ucBssIdx;
#if (CFG_SUPPORT_SOFTAP_WPA3 == 0)
	if (prNewKey->u4KeyLength == CCMP_KEY_LEN)
		rCmdKey.ucAlgorithmId = CIPHER_SUITE_CCMP;	/* AES */
	else if (prNewKey->u4KeyLength == TKIP_KEY_LEN)
		rCmdKey.ucAlgorithmId = CIPHER_SUITE_TKIP;	/* TKIP */
	else if (prNewKey->u4KeyLength == WEP_40_LEN)
		rCmdKey.ucAlgorithmId = CIPHER_SUITE_WEP40;	/* WEP 40 */
	else if (prNewKey->u4KeyLength == WEP_104_LEN)
		rCmdKey.ucAlgorithmId = CIPHER_SUITE_WEP104;	/* WEP 104 */
	else
		ASSERT_BOOLEAN(FALSE);
#else
	if (prNewKey->ucCipher) {
		rCmdKey.ucAlgorithmId = prNewKey->ucCipher;
	} else {
		DBGLOG(REQ, WARN, "prNewKey->ucCipher == 0!\n");
#if 0
	if (prNewKey->u4KeyLength == CCMP_KEY_LEN)
		rCmdKey.ucAlgorithmId = CIPHER_SUITE_CCMP;	/* AES */
	else if (prNewKey->u4KeyLength == TKIP_KEY_LEN)
		rCmdKey.ucAlgorithmId = CIPHER_SUITE_TKIP;	/* TKIP */
	else if (prNewKey->u4KeyLength == WEP_40_LEN)
		rCmdKey.ucAlgorithmId = CIPHER_SUITE_WEP40;	/* WEP 40 */
	else if (prNewKey->u4KeyLength == WEP_104_LEN)
		rCmdKey.ucAlgorithmId = CIPHER_SUITE_WEP104;	/* WEP 104 */
	else
			ASSERT(FALSE);
#endif
	}
#endif
	rCmdKey.ucKeyId = (UINT_8) (prNewKey->u4KeyIndex & (UINT_8)0xffU);
	rCmdKey.ucKeyLen = (UINT_8) prNewKey->u4KeyLength;
	kalMemCopy(rCmdKey.aucKeyMaterial, (PUINT_8) prNewKey->aucKeyMaterial, rCmdKey.ucKeyLen);

	if ((rCmdKey.aucPeerAddr[0] & rCmdKey.aucPeerAddr[1] &
		rCmdKey.aucPeerAddr[2] & rCmdKey.aucPeerAddr[3] &
		rCmdKey.aucPeerAddr[4] & rCmdKey.aucPeerAddr[5]) == 0xFFU) {
		kalMemCopy(rCmdKey.aucPeerAddr, prBssInfo->aucBSSID, MAC_ADDR_LEN);
		if (rCmdKey.ucIsAuthenticator == 0U) {
			prStaRec = cnmGetStaRecByAddress(prAdapter, rCmdKey.ucBssIdx, rCmdKey.aucPeerAddr);
			if (prStaRec == NULL)
				ASSERT_BOOLEAN(FALSE);
		}
	} else {
		prStaRec = cnmGetStaRecByAddress(prAdapter, rCmdKey.ucBssIdx, rCmdKey.aucPeerAddr);
	}

#if CFG_SUPPORT_FRAG_AGG_ATTACK_DETECTION
	if (rCmdKey.ucKeyType && prStaRec) {
		/* clear fragment cache when p2p rekey. */
		nicRxClearFrag(prAdapter, prStaRec);
	}
#endif

#if (CFG_SUPPORT_SOFTAP_WPA3 == 0)
	if (rCmdKey.ucTxKey != 0U) {
		if (prStaRec != NULL) {
			if (rCmdKey.ucKeyType != 0U) {	/* RSN STA */
				ASSERT_BOOLEAN(prStaRec->ucWlanIndex
					< WTBL_SIZE);
				rCmdKey.ucWlanIndex = prStaRec->ucWlanIndex;
				prStaRec->fgTransmitKeyExist = TRUE;	/* wait for CMD Done ? */
#if CFG_SUPPORT_802_11W
				/* AP PMF */
				DBGLOG(RSN, INFO, "Assign client PMF flag = %d\n",
					prStaRec->rPmfCfg.fgApplyPmf);
				rCmdKey.ucMgmtProtection = prStaRec->rPmfCfg.fgApplyPmf;
#endif
			} else {
				ASSERT_BOOLEAN(FALSE);
				/* prCmdKey->ucWlanIndex = secPrivacySeekForBcEntry(prAdapter, prBssInfo->ucBssIndex, */
				/* NETWORK_TYPE_AIS, prCmdKey->aucPeerAddr,*/
				/* prCmdKey->ucAlgorithmId, prCmdKey->ucKeyId, */
				/* prStaRec->ucCurrentGtkId, BIT(1)); */
				/* Todo:: Check the prCmdKey->ucKeyType */
				/* for some case, like wep, add bc wep key before sta create,*/
				/* so use the rAisSpecificBssInfo to save key setting */
				/* fgAddTxBcKey = TRUE; */
			}
		} else {
			if (prBssInfo != NULL) {	/* GO/AP Tx BC */
				ASSERT_BOOLEAN(prBssInfo->ucBMCWlanIndex
					< WTBL_SIZE);
				rCmdKey.ucWlanIndex = prBssInfo->ucBMCWlanIndex;
				/* rCmdKey.ucWlanIndex =  secPrivacySeekForBcEntry(prAdapter, prBssInfo->ucBssIndex, */
				/* prBssInfo->aucBSSID, 0xff, rCmdKey.ucAlgorithmId, rCmdKey.ucKeyId, */
				/* prBssInfo->ucCurrentGtkId, BIT(1)); */
				prBssInfo->fgTxBcKeyExist = TRUE;
				prBssInfo->ucTxDefaultKeyID = rCmdKey.ucKeyId;
			} else {
				rCmdKey.ucWlanIndex = 255U;
				/* GC WEP Tx key ? */
				ASSERT_BOOLEAN(FALSE);
			}
		}
	} else {
		if (((rCmdKey.aucPeerAddr[0] & rCmdKey.aucPeerAddr[1] &
			rCmdKey.aucPeerAddr[2] & rCmdKey.aucPeerAddr[3] &
			rCmdKey.aucPeerAddr[4] & rCmdKey.aucPeerAddr[5])
			== 0xFFU)
			||
			((rCmdKey.aucPeerAddr[0] | rCmdKey.aucPeerAddr[1] |
			rCmdKey.aucPeerAddr[2] | rCmdKey.aucPeerAddr[3] |
			rCmdKey.aucPeerAddr[4] | rCmdKey.aucPeerAddr[5]) ==
			0x00U)) {
			rCmdKey.ucWlanIndex = 255U;	/* GC WEP ? */
			ASSERT_BOOLEAN(FALSE);
		} else {
			if (prStaRec != NULL) {	/* GC Rx RSN Group key */
				rCmdKey.ucWlanIndex =
				    secPrivacySeekForBcEntry(prAdapter, prStaRec->ucBssIndex,
						prStaRec->aucMacAddr,
						prStaRec->ucIndex,
						rCmdKey.ucAlgorithmId,
						rCmdKey.ucKeyId,
						prStaRec->ucCurrentGtkId,
						(UINT_8)BIT(0U));
				prStaRec->ucBMCWlanIndex = rCmdKey.ucWlanIndex;
				ASSERT_BOOLEAN(prStaRec->ucBMCWlanIndex
					< WTBL_SIZE);
			} else {	/* Exist this case ? */
				ASSERT_BOOLEAN(FALSE);
				/* prCmdKey->ucWlanIndex = */
				/*	secPrivacySeekForBcEntry(prAdapter, */
				/*	prBssInfo->ucBssIndex, */
				/*	NETWORK_TYPE_AIS, */
				/*	prCmdKey->aucPeerAddr, */
				/*	prCmdKey->ucAlgorithmId, */
				/*	prCmdKey->ucKeyId, */
				/*	prBssInfo->ucCurrentGtkId, */
				/*	BIT(0U)); */
			}
		}
	}

	/* Update Group Key Id after Seek Bc entry */
	if (rCmdKey.ucKeyType == 0U) {
		if (prStaRec != NULL)
			prStaRec->ucCurrentGtkId = rCmdKey.ucKeyId;
		else {
			/* GC WEP? */
			prBssInfo->ucCurrentGtkId = rCmdKey.ucKeyId;
		}
	}
#else
#if CFG_SUPPORT_802_11W
	/* AP PMF */
	if (rCmdKey.ucAlgorithmId == CIPHER_SUITE_BIP) {
		if (rCmdKey.ucIsAuthenticator != 0U) {
			DBGLOGLIMITED(RSN, INFO,
			"Authenticator BIP bssid:%d\n",
			prBssInfo->ucBssIndex);

			rCmdKey.ucWlanIndex =
				secPrivacySeekForBcEntry(prAdapter,
					prBssInfo->ucBssIndex,
					prBssInfo->aucOwnMacAddr,
					STA_REC_INDEX_NOT_FOUND,
					rCmdKey.ucAlgorithmId,
					rCmdKey.ucKeyId,
					0,
					0);
		} else {
			rCmdKey.ucWlanIndex =
				secPrivacySeekForBcEntry(prAdapter,
					prBssInfo->ucBssIndex,
					prBssInfo->prStaRecOfAP->aucMacAddr,
					prBssInfo->prStaRecOfAP->ucIndex,
					rCmdKey.ucAlgorithmId,
					rCmdKey.ucKeyId,
					0,
					0);
		}

		DBGLOGLIMITED(RSN, INFO, "BIP BC wtbl index:%d\n",
			rCmdKey.ucWlanIndex);
	} else
#endif
	if (1) {
		if (rCmdKey.ucTxKey != 0U) {
			if (prStaRec != NULL) {
				if (rCmdKey.ucKeyType != 0U) {	/* RSN STA */
					if (prStaRec->ucWlanIndex >= WTBL_SIZE)
						ASSERT(NULL);
					rCmdKey.ucWlanIndex = prStaRec->ucWlanIndex;
					prStaRec->fgTransmitKeyExist = TRUE;	/* wait for CMD Done ? */
#if CFG_SUPPORT_802_11W
					/* AP PMF */
					DBGLOGLIMITED(RSN, TRACE,
						"Assign client PMF flag = %d\n",
						prStaRec->rPmfCfg.fgApplyPmf);
					rCmdKey.ucMgmtProtection =
						prStaRec->rPmfCfg.fgApplyPmf;
#endif

				} else {
					ASSERT(FALSE);
					/* prCmdKey->ucWlanIndex = secPrivacySeekForBcEntry(prAdapter,*/
					/* prBssInfo->ucBssIndex, */
					/* NETWORK_TYPE_AIS, prCmdKey->aucPeerAddr,*/
					/* prCmdKey->ucAlgorithmId, prCmdKey->ucKeyId, */
					/* prStaRec->ucCurrentGtkId, BIT(1)); */
					/* Todo:: Check the prCmdKey->ucKeyType */
					/* for some case, like wep, add bc wep key before sta create,*/
					/* so use the rAisSpecificBssInfo to save key setting */
					/* fgAddTxBcKey = TRUE; */
				}
			} else {	/* GO/AP Tx BC */
				if (prBssInfo->ucBMCWlanIndex >= WTBL_SIZE)
					ASSERT(NULL);
				rCmdKey.ucWlanIndex = prBssInfo->ucBMCWlanIndex;
				/* rCmdKey.ucWlanIndex =  secPrivacySeekForBcEntry(prAdapter, prBssInfo->ucBssIndex, */
				/* prBssInfo->aucBSSID, 0xff, rCmdKey.ucAlgorithmId, rCmdKey.ucKeyId, */
				/* prBssInfo->ucCurrentGtkId, BIT(1)); */
				prBssInfo->fgTxBcKeyExist = TRUE;
				prBssInfo->ucTxDefaultKeyID = rCmdKey.ucKeyId;
			}
		} else {
			if (((rCmdKey.aucPeerAddr[0] & rCmdKey.aucPeerAddr[1] & rCmdKey.aucPeerAddr[2] &
			      rCmdKey.aucPeerAddr[3] & rCmdKey.aucPeerAddr[4] & rCmdKey.aucPeerAddr[5]) == 0xFFU)
			    ||
			    ((rCmdKey.aucPeerAddr[0] | rCmdKey.aucPeerAddr[1] | rCmdKey.
				aucPeerAddr[2] | rCmdKey.aucPeerAddr[3] | rCmdKey.aucPeerAddr[4]
				| rCmdKey.aucPeerAddr[5]) == 0x00U)) {
				rCmdKey.ucWlanIndex = 255U;	/* GC WEP ? */
				ASSERT(FALSE);
			} else {
				if (prStaRec != NULL) {	/* GC Rx RSN Group key */
					rCmdKey.ucWlanIndex =
					    secPrivacySeekForBcEntry(prAdapter, prStaRec->ucBssIndex,
								     prStaRec->aucMacAddr,
								     prStaRec->ucIndex,
								     rCmdKey.ucAlgorithmId, rCmdKey.ucKeyId,
								     prStaRec->ucCurrentGtkId, BIT(0));
					prStaRec->ucBMCWlanIndex = rCmdKey.ucWlanIndex;
					if (prStaRec->ucBMCWlanIndex >= WTBL_SIZE)
						ASSERT(NULL);
				} else {	/* Exist this case ? */
					ASSERT(FALSE);
					/* prCmdKey->ucWlanIndex = */
					/*	secPrivacySeekForBcEntry(prAdapter, */
					/*	prBssInfo->ucBssIndex, */
					/*	NETWORK_TYPE_AIS, */
					/*	prCmdKey->aucPeerAddr, */
					/*	prCmdKey->ucAlgorithmId, */
					/*	prCmdKey->ucKeyId, */
					/*	prBssInfo->ucCurrentGtkId, */
					/*	BIT(0)); */
				}
			}
		}

		/* Update Group Key Id after Seek Bc entry */
		if (rCmdKey.ucKeyType == 0U) {
			if (prStaRec != NULL)
				prStaRec->ucCurrentGtkId = rCmdKey.ucKeyId;
			else {
				/* GC WEP? */
				prBssInfo->ucCurrentGtkId = rCmdKey.ucKeyId;
			}
		}
	}

#endif
	return wlanoidSendSetQueryP2PCmd(prAdapter,
			 (UINT_8)CMD_ID_ADD_REMOVE_KEY,
			 prNewKey->ucBssIdx,
			 TRUE,
			 FALSE,
			 TRUE,
			 nicCmdEventSetCommon,
			 NULL,
			 (UINT_32)sizeof(CMD_802_11_KEY),
			 (PUINT_8)&rCmdKey,
			 pvSetBuffer,
			 u4SetBufferLen);
}

/*----------------------------------------------------------------------------*/
/*!
* \brief This routine is called to request Wi-Fi Direct driver to remove keys
*
* \param[in] prAdapter Pointer to the Adapter structure.
* \param[in] pvSetBuffer A pointer to the buffer that holds the data to be set.
* \param[in] u4SetBufferLen The length of the set buffer.
* \param[out] pu4SetInfoLen If the call is successful, returns the number of
*                          bytes read from the set buffer. If the call failed
*                          due to invalid length of the set buffer, returns
*                          the amount of storage needed.
*
* \retval WLAN_STATUS_SUCCESS
* \retval WLAN_STATUS_INVALID_DATA
* \retval WLAN_STATUS_INVALID_LENGTH
* \retval WLAN_STATUS_INVALID_DATA
*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
wlanoidSetRemoveP2PKey(IN P_ADAPTER_T prAdapter,
		       IN PVOID pvSetBuffer, IN UINT_32 u4SetBufferLen, OUT PUINT_32 pu4SetInfoLen)
{
	CMD_802_11_KEY rCmdKey;
	P_PARAM_REMOVE_KEY_T prRemovedKey;
	P_BSS_INFO_T prBssInfo = (P_BSS_INFO_T) NULL;
	P_STA_RECORD_T prStaRec = (P_STA_RECORD_T) NULL;
#if (CFG_SUPPORT_SOFTAP_WPA3 == 1)
	UINT_32 u4KeyIndex = 0U;
#endif
	DEBUGFUNC("wlanoidSetRemoveP2PKey");
	ASSERT(prAdapter);

	if (u4SetBufferLen < (UINT_32)sizeof(PARAM_REMOVE_KEY_T))
		return WLAN_STATUS_INVALID_LENGTH;

	ASSERT(pvSetBuffer);
	prRemovedKey = (P_PARAM_REMOVE_KEY_T) pvSetBuffer;

	/* Check bit 31: this bit should always 0 */
	if ((prRemovedKey->u4KeyIndex & IS_TRANSMIT_KEY) != 0U) {
		/* Bit 31 should not be set */
		DBGLOG(REQ, ERROR, "invalid key index: 0x%08lx\n", prRemovedKey->u4KeyIndex);
		return WLAN_STATUS_INVALID_DATA;
	}

	/* Check bits 8 ~ 29 should always be 0 */
	if ((prRemovedKey->u4KeyIndex & BITS(8U, 29U)) != 0U) {
		/* Bit 31 should not be set */
		DBGLOG(REQ, ERROR, "invalid key index: 0x%08lx\n", prRemovedKey->u4KeyIndex);
		return WLAN_STATUS_INVALID_DATA;
	}

#if (CFG_SUPPORT_SOFTAP_WPA3 == 1)
	u4KeyIndex = prRemovedKey->u4KeyIndex & 0x000000FF;

	if (u4KeyIndex >= 4) {
		DBGLOG(RSN, INFO, "Remove bip key Index : 0x%08x\n",
		       u4KeyIndex);
		return WLAN_STATUS_SUCCESS;
	}
#endif
	prBssInfo = GET_BSS_INFO_BY_INDEX(prAdapter, prRemovedKey->ucBssIdx);

	kalMemZero((PUINT_8)&rCmdKey, (UINT_32)sizeof(CMD_802_11_KEY));

	rCmdKey.ucAddRemove = 0U;	/* remove */
	if (prBssInfo->eCurrentOPMode == OP_MODE_INFRASTRUCTURE) {	/* group client */
		rCmdKey.ucIsAuthenticator = 0U;
	} else {		/* group owner */
		rCmdKey.ucIsAuthenticator = 1U;
	}
	kalMemCopy(rCmdKey.aucPeerAddr, (PUINT_8) prRemovedKey->arBSSID, MAC_ADDR_LEN);
	rCmdKey.ucBssIdx = prRemovedKey->ucBssIdx;
	rCmdKey.ucKeyId = (UINT_8) (prRemovedKey->u4KeyIndex & 0x000000ffU);

	/* Clean up the Tx key flag */
	prStaRec = cnmGetStaRecByAddress(prAdapter, prRemovedKey->ucBssIdx, prRemovedKey->arBSSID);

	/* mark for MR1 to avoid remove-key, but remove the wlan_tbl0 at the same time */
	if (1U == 1U/*prRemovedKey->u4KeyIndex & IS_UNICAST_KEY */) {
		if (prStaRec != NULL) {
			rCmdKey.ucKeyType = 1U;
			rCmdKey.ucWlanIndex = prStaRec->ucWlanIndex;
			prStaRec->fgTransmitKeyExist = FALSE;
		} else if (rCmdKey.ucIsAuthenticator != 0U)
			prBssInfo->fgTxBcKeyExist = FALSE;
		else
			DBGLOG(INIT, TRACE, "ucIsAuthenticator is 0\n");
	} else {
		if (rCmdKey.ucIsAuthenticator != 0U)
			prBssInfo->fgTxBcKeyExist = FALSE;
	}

	if (prStaRec == NULL) {
		if (prAdapter->rWifiVar.rConnSettings.eAuthMode < AUTH_MODE_WPA
		    && prAdapter->rWifiVar.rConnSettings.eEncStatus != ENUM_ENCRYPTION_DISABLED) {
			rCmdKey.ucWlanIndex = prBssInfo->ucBMCWlanIndex;
		} else {
			return WLAN_STATUS_SUCCESS;
		}
	}

	/* mark for MR1 to avoid remove-key, but remove the wlan_tbl0 at the same time */
	/* secPrivacyFreeForEntry(prAdapter, rCmdKey.ucWlanIndex); */

	return wlanoidSendSetQueryP2PCmd(prAdapter,
			 (UINT_8)CMD_ID_ADD_REMOVE_KEY,
			 prRemovedKey->ucBssIdx,
			 TRUE,
			 FALSE,
			 TRUE,
			 nicCmdEventSetCommon,
			 NULL,
			 (UINT_32)sizeof(CMD_802_11_KEY),
			 (PUINT_8)&rCmdKey,
			 pvSetBuffer,
			 u4SetBufferLen);
}

/*----------------------------------------------------------------------------*/
/*!
* \brief Setting the IP address for pattern search function.
*
* \param[in] prAdapter Pointer to the Adapter structure.
* \param[in] pvSetBuffer A pointer to the buffer that holds the data to be set.
* \param[in] u4SetBufferLen The length of the set buffer.
* \param[out] pu4SetInfoLen If the call is successful, returns the number of
*                           bytes read from the set buffer. If the call failed
*                           due to invalid length of the set buffer, returns
*                           the amount of storage needed.
*
* \return WLAN_STATUS_SUCCESS
* \return WLAN_STATUS_ADAPTER_NOT_READY
* \return WLAN_STATUS_INVALID_LENGTH
*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
wlanoidSetP2pNetworkAddress(IN P_ADAPTER_T prAdapter,
			    IN PVOID pvSetBuffer, IN UINT_32 u4SetBufferLen, OUT PUINT_32 pu4SetInfoLen)
{
	WLAN_STATUS rStatus = WLAN_STATUS_SUCCESS;
	UINT_32 i, j;
	P_CMD_SET_NETWORK_ADDRESS_LIST prCmdNetworkAddressList;
	P_PARAM_NETWORK_ADDRESS_LIST prNetworkAddressList = (P_PARAM_NETWORK_ADDRESS_LIST) pvSetBuffer;
	P_PARAM_NETWORK_ADDRESS prNetworkAddress;
	P_PARAM_NETWORK_ADDRESS_IP prNetAddrIp;
	UINT_32 u4IpAddressCount, u4CmdSize;
	PUINT_8 pucBuf;

	DEBUGFUNC("wlanoidSetP2pNetworkAddress");
	DBGLOG(INIT, TRACE, "\n");

	ASSERT(prAdapter);
	ASSERT(pu4SetInfoLen);

	*pu4SetInfoLen = 4U;

	if (u4SetBufferLen < (UINT_32)sizeof(PARAM_NETWORK_ADDRESS_LIST))
		return WLAN_STATUS_INVALID_DATA;

	*pu4SetInfoLen = 0U;
	u4IpAddressCount = 0U;

	prNetworkAddress = prNetworkAddressList->arAddress;
	for (i = 0U; i < prNetworkAddressList->u4AddressCount; i++) {
		if (prNetworkAddress->u2AddressType == PARAM_PROTOCOL_ID_TCP_IP &&
		    prNetworkAddress->u2AddressLength ==
		    (UINT_32)sizeof(PARAM_NETWORK_ADDRESS_IP)) {
			u4IpAddressCount++;
		}

		prNetworkAddress = (P_PARAM_NETWORK_ADDRESS) ((ULONG) prNetworkAddress +
							      (ULONG) (prNetworkAddress->u2AddressLength +
								       OFFSET_OF(PARAM_NETWORK_ADDRESS, aucAddress)));
	}

	/* construct payload of command packet */
	u4CmdSize = (UINT_32)OFFSET_OF(
		CMD_SET_NETWORK_ADDRESS_LIST, arNetAddress)
		+ (UINT_32)sizeof(IPV4_NETWORK_ADDRESS) * u4IpAddressCount;

	prCmdNetworkAddressList = (P_CMD_SET_NETWORK_ADDRESS_LIST) kalMemAlloc(u4CmdSize, VIR_MEM_TYPE);

	if (prCmdNetworkAddressList == NULL)
		return WLAN_STATUS_FAILURE;

	/* fill P_CMD_SET_NETWORK_ADDRESS_LIST */
	prCmdNetworkAddressList->ucBssIndex = prNetworkAddressList->ucBssIdx;
	prCmdNetworkAddressList->ucAddressCount = (UINT_8) u4IpAddressCount;
	prNetworkAddress = prNetworkAddressList->arAddress;
	j = 0U;
	for (i = 0U; i < prNetworkAddressList->u4AddressCount; i++) {
		if (prNetworkAddress->u2AddressType == PARAM_PROTOCOL_ID_TCP_IP &&
		    prNetworkAddress->u2AddressLength ==
		    (UINT_32)sizeof(PARAM_NETWORK_ADDRESS_IP)) {
			prNetAddrIp = (P_PARAM_NETWORK_ADDRESS_IP) prNetworkAddress->aucAddress;

			pucBuf = (PUINT_8) &prNetAddrIp->in_addr;
			kalMemCopy((PUINT_8)&(prCmdNetworkAddressList->
				arNetAddress[j].aucIpAddr[0]),
				pucBuf,
				(UINT_32)sizeof(UINT_32));

			j++;
		}

		prNetworkAddress = (P_PARAM_NETWORK_ADDRESS) ((ULONG) prNetworkAddress +
							      (ULONG) (prNetworkAddress->u2AddressLength +
								       OFFSET_OF(PARAM_NETWORK_ADDRESS, aucAddress)));
	}

	rStatus = wlanSendSetQueryCmd(prAdapter,
				      (UINT_8)CMD_ID_SET_IP_ADDRESS,
				      TRUE,
				      FALSE,
				      TRUE,
				      nicCmdEventSetIpAddress,
				      nicOidCmdTimeoutCommon,
				      u4CmdSize, (PUINT_8) prCmdNetworkAddressList, pvSetBuffer, u4SetBufferLen);

	kalMemFree(prCmdNetworkAddressList, VIR_MEM_TYPE, u4CmdSize);
	return rStatus;
}

/*----------------------------------------------------------------------------*/
/*!
* \brief This routine is used to query the power save profile.
*
* \param[in] prAdapter Pointer to the Adapter structure.
* \param[out] pvQueryBuf A pointer to the buffer that holds the result of
*                           the query.
* \param[in] u4QueryBufLen The length of the query buffer.
* \param[out] pu4QueryInfoLen If the call is successful, returns the number of
*                            bytes written into the query buffer. If the call
*                            failed due to invalid length of the query buffer,
*                            returns the amount of storage needed.
*
* \return WLAN_STATUS_SUCCESS
*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
wlanoidQueryP2pPowerSaveProfile(IN P_ADAPTER_T prAdapter,
				IN PVOID pvQueryBuffer, IN UINT_32 u4QueryBufferLen, OUT PUINT_32 pu4QueryInfoLen)
{
	DEBUGFUNC("wlanoidQueryP2pPowerSaveProfile");

	ASSERT(prAdapter);
	ASSERT(pu4QueryInfoLen);

	if (u4QueryBufferLen != 0U) {
		ASSERT(pvQueryBuffer);
		/* TODO: FIXME */
		/* *(PPARAM_POWER_MODE) pvQueryBuffer =
		 * (PARAM_POWER_MODE)(prAdapter->rWlanInfo.arPowerSaveMode[P2P_DEV_BSS_INDEX].ucPsProfile);
		 */
		/* *pu4QueryInfoLen = (UINT_32)sizeof(PARAM_POWER_MODE); */
	}

	return WLAN_STATUS_SUCCESS;
}

/*----------------------------------------------------------------------------*/
/*!
* \brief This routine is used to set the power save profile.
*
* \param[in] pvAdapter Pointer to the Adapter structure.
* \param[in] pvSetBuffer A pointer to the buffer that holds the data to be set.
* \param[in] u4SetBufferLen The length of the set buffer.
* \param[out] pu4SetInfoLen If the call is successful, returns the number of
*                          bytes read from the set buffer. If the call failed
*                          due to invalid length of the set buffer, returns
*                          the amount of storage needed.
*
* \retval WLAN_STATUS_SUCCESS
* \retval WLAN_STATUS_INVALID_LENGTH
*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
wlanoidSetP2pPowerSaveProfile(IN P_ADAPTER_T prAdapter,
			      IN PVOID pvSetBuffer, IN UINT_32 u4SetBufferLen, OUT PUINT_32 pu4SetInfoLen)
{
	WLAN_STATUS status;
	PARAM_POWER_MODE ePowerMode;

	DEBUGFUNC("wlanoidSetP2pPowerSaveProfile");

	ASSERT(prAdapter);
	ASSERT(pu4SetInfoLen);

	*pu4SetInfoLen = (UINT_32)sizeof(PARAM_POWER_MODE);
	if (u4SetBufferLen < (UINT_32)sizeof(PARAM_POWER_MODE)) {
		DBGLOG(REQ, WARN, "Invalid length %u\n", u4SetBufferLen);
		return WLAN_STATUS_INVALID_LENGTH;
	}

	if (*(PPARAM_POWER_MODE) pvSetBuffer >= Param_PowerModeMax) {
		DBGLOG(REQ, WARN, "Invalid power mode %d\n", *(PPARAM_POWER_MODE) pvSetBuffer);
		return WLAN_STATUS_INVALID_DATA;
	}

	ePowerMode = *(PPARAM_POWER_MODE) pvSetBuffer;

	if (prAdapter->fgEnCtiaPowerMode == TRUE) {
		if (ePowerMode == Param_PowerModeCAM) {
			/*Todo::  Nothing*/
			/*Todo::  Nothing*/
		} else {
			/* User setting to PS mode (Param_PowerModeMAX_PSP or Param_PowerModeFast_PSP) */

			if (prAdapter->u4CtiaPowerMode == 0U) {
				/* force to keep in CAM mode */
				ePowerMode = Param_PowerModeCAM;
			} else if (prAdapter->u4CtiaPowerMode == 1U) {
				ePowerMode = Param_PowerModeMAX_PSP;
			} else if (prAdapter->u4CtiaPowerMode == 2U) {
				ePowerMode = Param_PowerModeFast_PSP;
			} else
				DBGLOG(INIT, TRACE, "u4CtiaPowerMode is %d\n",
					prAdapter->u4CtiaPowerMode);
		}
	}

	status = nicConfigPowerSaveProfile(prAdapter, P2P_DEV_BSS_INDEX,	/* TODO: FIXME */
					   ePowerMode, TRUE);
	return status;
}				/* end of wlanoidSetP2pPowerSaveProfile() */

/*----------------------------------------------------------------------------*/
/*!
* \brief This routine is used to set the power save profile.
*
* \param[in] pvAdapter Pointer to the Adapter structure.
* \param[in] pvSetBuffer A pointer to the buffer that holds the data to be set.
* \param[in] u4SetBufferLen The length of the set buffer.
* \param[out] pu4SetInfoLen If the call is successful, returns the number of
*                          bytes read from the set buffer. If the call failed
*                          due to invalid length of the set buffer, returns
*                          the amount of storage needed.
*
* \retval WLAN_STATUS_SUCCESS
* \retval WLAN_STATUS_INVALID_LENGTH
*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
wlanoidSetP2pSetNetworkAddress(IN P_ADAPTER_T prAdapter,
			       IN PVOID pvSetBuffer, IN UINT_32 u4SetBufferLen, OUT PUINT_32 pu4SetInfoLen)
{
	WLAN_STATUS rStatus = WLAN_STATUS_SUCCESS;
	UINT_32 i, j;
	P_CMD_SET_NETWORK_ADDRESS_LIST prCmdNetworkAddressList;
	P_PARAM_NETWORK_ADDRESS_LIST prNetworkAddressList = (P_PARAM_NETWORK_ADDRESS_LIST) pvSetBuffer;
	P_PARAM_NETWORK_ADDRESS prNetworkAddress;
	P_PARAM_NETWORK_ADDRESS_IP prNetAddrIp;
	UINT_32 u4IpAddressCount, u4CmdSize;
	PUINT_8 pucBuf = (PUINT_8) pvSetBuffer;

	DEBUGFUNC("wlanoidSetP2pSetNetworkAddress");
	DBGLOG(INIT, TRACE, "\n");
	DBGLOG(INIT, INFO, "wlanoidSetP2pSetNetworkAddress (%d)\n", (INT_16) u4SetBufferLen);

	ASSERT(prAdapter);
	ASSERT(pu4SetInfoLen);

	*pu4SetInfoLen = 4U;

	if (u4SetBufferLen < (UINT_32)sizeof(PARAM_NETWORK_ADDRESS_LIST))
		return WLAN_STATUS_INVALID_DATA;

	*pu4SetInfoLen = 0U;
	u4IpAddressCount = 0U;

	prNetworkAddress = prNetworkAddressList->arAddress;
	for (i = 0U; i < prNetworkAddressList->u4AddressCount; i++) {
		if (prNetworkAddress->u2AddressType == PARAM_PROTOCOL_ID_TCP_IP &&
		    prNetworkAddress->u2AddressLength ==
		    (UINT_32)sizeof(PARAM_NETWORK_ADDRESS_IP)) {
			u4IpAddressCount++;
		}

		prNetworkAddress = (P_PARAM_NETWORK_ADDRESS) ((ULONG) prNetworkAddress +
							      (ULONG) (prNetworkAddress->u2AddressLength +
								       OFFSET_OF(PARAM_NETWORK_ADDRESS, aucAddress)));
	}

	/* construct payload of command packet */
	u4CmdSize = (UINT_32)OFFSET_OF(CMD_SET_NETWORK_ADDRESS_LIST,
		arNetAddress) +
		(UINT_32)sizeof(IPV4_NETWORK_ADDRESS) * u4IpAddressCount;

	if (u4IpAddressCount == 0U)
		u4CmdSize = (UINT_32)sizeof(CMD_SET_NETWORK_ADDRESS_LIST);

	prCmdNetworkAddressList = (P_CMD_SET_NETWORK_ADDRESS_LIST) kalMemAlloc(u4CmdSize, VIR_MEM_TYPE);

	if (prCmdNetworkAddressList == NULL)
		return WLAN_STATUS_FAILURE;

	/* fill P_CMD_SET_NETWORK_ADDRESS_LIST */
	prCmdNetworkAddressList->ucBssIndex = prNetworkAddressList->ucBssIdx;

	/* only to set IP address to FW once ARP filter is enabled */
	if (prAdapter->fgEnArpFilter > 0U) {
		prCmdNetworkAddressList->ucAddressCount = (UINT_8) u4IpAddressCount;
		prNetworkAddress = prNetworkAddressList->arAddress;

		DBGLOG(INIT, INFO, "u4IpAddressCount (%u)\n", u4IpAddressCount);
		j = 0U;
		for (i = 0U; i < prNetworkAddressList->u4AddressCount; i++) {
			if (prNetworkAddress->u2AddressType == PARAM_PROTOCOL_ID_TCP_IP &&
			    prNetworkAddress->u2AddressLength ==
			    (UINT_32)sizeof(PARAM_NETWORK_ADDRESS_IP)) {
				prNetAddrIp = (P_PARAM_NETWORK_ADDRESS_IP) prNetworkAddress->aucAddress;

				pucBuf = (PUINT_8) &prNetAddrIp->in_addr;
				kalMemCopy((PUINT_8)&(prCmdNetworkAddressList->
					arNetAddress[j].aucIpAddr[0]),
					   pucBuf,
					   (UINT_32)sizeof(UINT_32));

				j++;

				DBGLOG(INIT, INFO, "prNetAddrIp->in_addr:%d:%d:%d:%d\n",
					(UINT_8) pucBuf[0], (UINT_8) pucBuf[1],
					(UINT_8) pucBuf[2], (UINT_8) pucBuf[3]);
			}

			prNetworkAddress = (P_PARAM_NETWORK_ADDRESS) ((ULONG) prNetworkAddress +
								      (ULONG) (prNetworkAddress->u2AddressLength +
									       OFFSET_OF
									       (PARAM_NETWORK_ADDRESS, aucAddress)));
		}

	} else {
		prCmdNetworkAddressList->ucAddressCount = 0U;
	}

	rStatus = wlanSendSetQueryCmd(prAdapter,
				      (UINT_8)CMD_ID_SET_IP_ADDRESS,
				      TRUE,
				      FALSE,
				      TRUE,
				      nicCmdEventSetIpAddress,
				      nicOidCmdTimeoutCommon,
				      u4CmdSize, (PUINT_8) prCmdNetworkAddressList, pvSetBuffer, u4SetBufferLen);

	kalMemFree(prCmdNetworkAddressList, VIR_MEM_TYPE, u4CmdSize);
	return rStatus;
}				/* end of wlanoidSetP2pSetNetworkAddress() */

/*----------------------------------------------------------------------------*/
/*!
* \brief This routine is called to set Multicast Address List.
*
* \param[in] prAdapter      Pointer to the Adapter structure.
* \param[in] pvSetBuffer    Pointer to the buffer that holds the data to be set.
* \param[in] u4SetBufferLen The length of the set buffer.
* \param[out] pu4SetInfoLen If the call is successful, returns the number of
*                           bytes read from the set buffer. If the call failed
*                           due to invalid length of the set buffer, returns
*                           the amount of storage needed.
*
* \retval WLAN_STATUS_SUCCESS
* \retval WLAN_STATUS_INVALID_LENGTH
* \retval WLAN_STATUS_ADAPTER_NOT_READY
* \retval WLAN_STATUS_MULTICAST_FULL
*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
wlanoidSetP2PMulticastList(IN P_ADAPTER_T prAdapter,
			   IN PVOID pvSetBuffer, IN UINT_32 u4SetBufferLen, OUT PUINT_32 pu4SetInfoLen)
{
	CMD_MAC_MCAST_ADDR rCmdMacMcastAddr;

	ASSERT(prAdapter);
	ASSERT(pu4SetInfoLen);

	/* The data must be a multiple of the Ethernet address size. */
	if ((u4SetBufferLen % MAC_ADDR_LEN) != 0U) {
		DBGLOG(REQ, WARN, "Invalid MC list length %u\n",
			u4SetBufferLen);

		*pu4SetInfoLen =
			(((u4SetBufferLen + MAC_ADDR_LEN) - 1U) /
			MAC_ADDR_LEN) * MAC_ADDR_LEN;

		return WLAN_STATUS_INVALID_LENGTH;
	}

	*pu4SetInfoLen = u4SetBufferLen;

	/* Verify if we can support so many multicast addresses. */
	if ((u4SetBufferLen / MAC_ADDR_LEN) > MAX_NUM_GROUP_ADDR) {
		DBGLOG(REQ, WARN, "Too many MC addresses\n");

		return WLAN_STATUS_MULTICAST_FULL;
	}

	/* NOTE(Kevin): Windows may set u4SetBufferLen == 0 &&
	 * pvSetBuffer == NULL to clear exist Multicast List.
	 */
	if (u4SetBufferLen != 0U)
		ASSERT(pvSetBuffer);

	if (prAdapter->rAcpiState == ACPI_STATE_D3) {
		DBGLOG(REQ, WARN,
		       "Fail in set multicast list! (Adapter not ready). ACPI=D%d, Radio=%d\n",
			prAdapter->rAcpiState, prAdapter->fgIsRadioOff);
		return WLAN_STATUS_ADAPTER_NOT_READY;
	}

	rCmdMacMcastAddr.u4NumOfGroupAddr = u4SetBufferLen / MAC_ADDR_LEN;
	rCmdMacMcastAddr.ucBssIndex = P2P_DEV_BSS_INDEX;	/* TODO: */
	if (u4SetBufferLen > 0U)
		kalMemCopy(rCmdMacMcastAddr.arAddress,
			pvSetBuffer, u4SetBufferLen);

	return wlanoidSendSetQueryP2PCmd(prAdapter,
					(UINT_8)CMD_ID_MAC_MCAST_ADDR,
					P2P_DEV_BSS_INDEX,	/* TODO: */
	/* This CMD response is no need to complete the OID. Or the event would unsync. */
					TRUE, FALSE, FALSE,
					nicCmdEventSetCommon,
					nicOidCmdTimeoutCommon,
					(UINT_32)sizeof(CMD_MAC_MCAST_ADDR),
					(PUINT_8) &rCmdMacMcastAddr, pvSetBuffer, u4SetBufferLen);

}				/* end of wlanoidSetP2PMulticastList() */

/*----------------------------------------------------------------------------*/
/*!
* \brief This routine is called to send GAS frame for P2P Service Discovery Request
*
* \param[in] prAdapter      Pointer to the Adapter structure.
* \param[in] pvSetBuffer    Pointer to the buffer that holds the data to be set.
* \param[in] u4SetBufferLen The length of the set buffer.
* \param[out] pu4SetInfoLen If the call is successful, returns the number of
*                           bytes read from the set buffer. If the call failed
*                           due to invalid length of the set buffer, returns
*                           the amount of storage needed.
*
* \retval WLAN_STATUS_SUCCESS
* \retval WLAN_STATUS_INVALID_LENGTH
* \retval WLAN_STATUS_ADAPTER_NOT_READY
* \retval WLAN_STATUS_MULTICAST_FULL
*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
wlanoidSendP2PSDRequest(IN P_ADAPTER_T prAdapter,
			IN PVOID pvSetBuffer, IN UINT_32 u4SetBufferLen, OUT PUINT_32 pu4SetInfoLen)
{
	WLAN_STATUS rWlanStatus = WLAN_STATUS_SUCCESS;

	ASSERT(prAdapter);
	ASSERT(pu4SetInfoLen);

	if (u4SetBufferLen != 0U)
		ASSERT(pvSetBuffer);

	if (u4SetBufferLen < (UINT_32)sizeof(PARAM_P2P_SEND_SD_REQUEST)) {
		*pu4SetInfoLen = (UINT_32)sizeof(PARAM_P2P_SEND_SD_REQUEST);
		return WLAN_STATUS_BUFFER_TOO_SHORT;
	}
/* rWlanStatus = p2pFsmRunEventSDRequest(prAdapter, (P_PARAM_P2P_SEND_SD_REQUEST)pvSetBuffer); */

	return rWlanStatus;
}				/* end of wlanoidSendP2PSDRequest() */

/*----------------------------------------------------------------------------*/
/*!
* \brief This routine is called to send GAS frame for P2P Service Discovery Response
*
* \param[in] prAdapter      Pointer to the Adapter structure.
* \param[in] pvSetBuffer    Pointer to the buffer that holds the data to be set.
* \param[in] u4SetBufferLen The length of the set buffer.
* \param[out] pu4SetInfoLen If the call is successful, returns the number of
*                           bytes read from the set buffer. If the call failed
*                           due to invalid length of the set buffer, returns
*                           the amount of storage needed.
*
* \retval WLAN_STATUS_SUCCESS
* \retval WLAN_STATUS_INVALID_LENGTH
* \retval WLAN_STATUS_ADAPTER_NOT_READY
* \retval WLAN_STATUS_MULTICAST_FULL
*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
wlanoidSendP2PSDResponse(IN P_ADAPTER_T prAdapter,
			 IN PVOID pvSetBuffer, IN UINT_32 u4SetBufferLen, OUT PUINT_32 pu4SetInfoLen)
{
	WLAN_STATUS rWlanStatus = WLAN_STATUS_SUCCESS;

	ASSERT(prAdapter);
	ASSERT(pu4SetInfoLen);

	if (u4SetBufferLen != 0U)
		ASSERT(pvSetBuffer);

	if (u4SetBufferLen < (UINT_32)sizeof(PARAM_P2P_SEND_SD_RESPONSE)) {
		*pu4SetInfoLen = (UINT_32)sizeof(PARAM_P2P_SEND_SD_RESPONSE);
		return WLAN_STATUS_BUFFER_TOO_SHORT;
	}
/* rWlanStatus = p2pFsmRunEventSDResponse(prAdapter, (P_PARAM_P2P_SEND_SD_RESPONSE)pvSetBuffer); */

	return rWlanStatus;
}				/* end of wlanoidGetP2PSDRequest() */

/*----------------------------------------------------------------------------*/
/*!
* \brief This routine is called to get GAS frame for P2P Service Discovery Request
*
* \param[in]  prAdapter        Pointer to the Adapter structure.
* \param[out] pvQueryBuffer    A pointer to the buffer that holds the result of
*                              the query.
* \param[in]  u4QueryBufferLen The length of the query buffer.
* \param[out] pu4QueryInfoLen  If the call is successful, returns the number of
*                              bytes written into the query buffer. If the call
*                              failed due to invalid length of the query buffer,
*                              returns the amount of storage needed.
*
* \retval WLAN_STATUS_SUCCESS
* \retval WLAN_STATUS_INVALID_LENGTH
* \retval WLAN_STATUS_ADAPTER_NOT_READY
* \retval WLAN_STATUS_MULTICAST_FULL
*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
wlanoidGetP2PSDRequest(IN P_ADAPTER_T prAdapter,
		       IN PVOID pvQueryBuffer, IN UINT_32 u4QueryBufferLen, OUT PUINT_32 pu4QueryInfoLen)
{
	WLAN_STATUS rWlanStatus = WLAN_STATUS_SUCCESS;
	/*PUINT_8 pucPacketBuffer = NULL, pucTA = NULL;*/
/* PUINT_8 pucChannelNum = NULL; */
	/*PUINT_16 pu2PacketLength = NULL;*/
	/*P_WLAN_MAC_HEADER_T prWlanHdr = (P_WLAN_MAC_HEADER_T) NULL;*/
	/*UINT_8 ucVersionNum = 0;*/
/* UINT_8 ucChannelNum = 0, ucSeqNum = 0; */

	ASSERT(prAdapter);
	ASSERT(pu4QueryInfoLen);

	if (u4QueryBufferLen != 0U)
		ASSERT(pvQueryBuffer);

	if (u4QueryBufferLen < (UINT_32)sizeof(PARAM_P2P_GET_SD_REQUEST)) {
		*pu4QueryInfoLen = (UINT_32)sizeof(PARAM_P2P_GET_SD_REQUEST);
		return WLAN_STATUS_BUFFER_TOO_SHORT;
	}

	DBGLOG(P2P, TRACE, "Get Service Discovery Request\n");
#if 0
	ucVersionNum = p2pFuncGetVersionNumOfSD(prAdapter);
	if (ucVersionNum == 0) {
		P_PARAM_P2P_GET_SD_REQUEST prP2pGetSdReq = (P_PARAM_P2P_GET_SD_REQUEST) pvQueryBuffer;

		pucPacketBuffer = prP2pGetSdReq->aucPacketContent;
		pu2PacketLength = &prP2pGetSdReq->u2PacketLength;
		pucTA = &prP2pGetSdReq->rTransmitterAddr;
	} else {
		P_PARAM_P2P_GET_SD_REQUEST_EX prP2pGetSdReqEx = (P_PARAM_P2P_GET_SD_REQUEST_EX) NULL;

		prP2pGetSdReqEx = (P_PARAM_P2P_GET_SD_REQUEST) pvQueryBuffer;
		pucPacketBuffer = prP2pGetSdReqEx->aucPacketContent;
		pu2PacketLength = &prP2pGetSdReqEx->u2PacketLength;
		pucTA = &prP2pGetSdReqEx->rTransmitterAddr;
		pucChannelNum = &prP2pGetSdReqEx->ucChannelNum;
		ucSeqNum = prP2pGetSdReqEx->ucSeqNum;
	}

	rWlanStatus = p2pFuncGetServiceDiscoveryFrame(prAdapter,
		      pucPacketBuffer,
		      (u4QueryBufferLen -
		      (UINT_32)sizeof(PARAM_P2P_GET_SD_REQUEST)),
		      (PUINT_32) pu2PacketLength, pucChannelNum, ucSeqNum);
#else
	*pu4QueryInfoLen = 0U;
	return rWlanStatus;
#endif
#if 0
	prWlanHdr = (P_WLAN_MAC_HEADER_T) pucPacketBuffer;

	kalMemCopy(pucTA, prWlanHdr->aucAddr2, MAC_ADDR_LEN);

	if (pu4QueryInfoLen > 0U) {
		if (ucVersionNum == 0U)
			*pu4QueryInfoLen = (UINT_32) (sizeof(PARAM_P2P_GET_SD_REQUEST) + (*pu2PacketLength));
		else
			*pu4QueryInfoLen = (UINT_32) (sizeof(PARAM_P2P_GET_SD_REQUEST_EX) + (*pu2PacketLength));

	}

	return rWlanStatus;
#endif
}				/* end of wlanoidGetP2PSDRequest() */

/*----------------------------------------------------------------------------*/
/*!
* \brief This routine is called to get GAS frame for P2P Service Discovery Response
*
* \param[in]  prAdapter        Pointer to the Adapter structure.
* \param[out] pvQueryBuffer    A pointer to the buffer that holds the result of
*                              the query.
* \param[in]  u4QueryBufferLen The length of the query buffer.
* \param[out] pu4QueryInfoLen  If the call is successful, returns the number of
*                              bytes written into the query buffer. If the call
*                              failed due to invalid length of the query buffer,
*                              returns the amount of storage needed.
*
* \retval WLAN_STATUS_SUCCESS
* \retval WLAN_STATUS_INVALID_LENGTH
* \retval WLAN_STATUS_ADAPTER_NOT_READY
* \retval WLAN_STATUS_MULTICAST_FULL
*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
wlanoidGetP2PSDResponse(IN P_ADAPTER_T prAdapter,
			IN PVOID pvQueryBuffer, IN UINT_32 u4QueryBufferLen, OUT PUINT_32 pu4QueryInfoLen)
{
	WLAN_STATUS rWlanStatus = WLAN_STATUS_SUCCESS;
	/*P_WLAN_MAC_HEADER_T prWlanHdr = (P_WLAN_MAC_HEADER_T) NULL;*/
	/* UINT_8 ucSeqNum = 0, */
	/*UINT_8 ucVersionNum = 0;*/
	/*PUINT_8 pucPacketContent = (PUINT_8) NULL, pucTA = (PUINT_8) NULL;*/
	/*PUINT_16 pu2PacketLength = (PUINT_16) NULL;*/

	ASSERT(prAdapter);
	ASSERT(pu4QueryInfoLen);

	if (u4QueryBufferLen != 0U)
		ASSERT(pvQueryBuffer);

	if (u4QueryBufferLen < (UINT_32)sizeof(PARAM_P2P_GET_SD_RESPONSE)) {
		*pu4QueryInfoLen = (UINT_32)sizeof(PARAM_P2P_GET_SD_RESPONSE);
		return WLAN_STATUS_BUFFER_TOO_SHORT;
	}

	DBGLOG(P2P, TRACE, "Get Service Discovery Response\n");

#if 0
	ucVersionNum = p2pFuncGetVersionNumOfSD(prAdapter);
	if (ucVersionNum == 0U) {
		P_PARAM_P2P_GET_SD_RESPONSE prP2pGetSdRsp = (P_PARAM_P2P_GET_SD_RESPONSE) NULL;

		prP2pGetSdRsp = (P_PARAM_P2P_GET_SD_REQUEST) pvQueryBuffer;
		pucPacketContent = prP2pGetSdRsp->aucPacketContent;
		pucTA = &prP2pGetSdRsp->rTransmitterAddr;
		pu2PacketLength = &prP2pGetSdRsp->u2PacketLength;
	} else {
		P_PARAM_P2P_GET_SD_RESPONSE_EX prP2pGetSdRspEx = (P_PARAM_P2P_GET_SD_RESPONSE_EX) NULL;

		prP2pGetSdRspEx = (P_PARAM_P2P_GET_SD_RESPONSE_EX) pvQueryBuffer;
		pucPacketContent = prP2pGetSdRspEx->aucPacketContent;
		pucTA = &prP2pGetSdRspEx->rTransmitterAddr;
		pu2PacketLength = &prP2pGetSdRspEx->u2PacketLength;
		ucSeqNum = prP2pGetSdRspEx->ucSeqNum;
	}

/* rWlanStatus = p2pFuncGetServiceDiscoveryFrame(prAdapter, */
/* pucPacketContent, */
/* (u4QueryBufferLen - sizeof(PARAM_P2P_GET_SD_RESPONSE)), */
/* (PUINT_32)pu2PacketLength, */
/* NULL, */
/* ucSeqNum); */
#else
	*pu4QueryInfoLen = 0U;
	return rWlanStatus;
#endif
#if 0
	prWlanHdr = (P_WLAN_MAC_HEADER_T) pucPacketContent;

	kalMemCopy(pucTA, prWlanHdr->aucAddr2, MAC_ADDR_LEN);

	if (pu4QueryInfoLen) {
		if (ucVersionNum == 0)
			*pu4QueryInfoLen = (UINT_32) (sizeof(PARAM_P2P_GET_SD_RESPONSE) + *pu2PacketLength);
		else
			*pu4QueryInfoLen = (UINT_32) (sizeof(PARAM_P2P_GET_SD_RESPONSE_EX) + *pu2PacketLength);
	}

	return rWlanStatus;
#endif
}				/* end of wlanoidGetP2PSDResponse() */

/*----------------------------------------------------------------------------*/
/*!
* \brief This routine is called to terminate P2P Service Discovery Phase
*
* \param[in] prAdapter      Pointer to the Adapter structure.
* \param[in] pvSetBuffer    Pointer to the buffer that holds the data to be set.
* \param[in] u4SetBufferLen The length of the set buffer.
* \param[out] pu4SetInfoLen If the call is successful, returns the number of
*                           bytes read from the set buffer. If the call failed
*                           due to invalid length of the set buffer, returns
*                           the amount of storage needed.
*
* \retval WLAN_STATUS_SUCCESS
* \retval WLAN_STATUS_INVALID_LENGTH
* \retval WLAN_STATUS_ADAPTER_NOT_READY
* \retval WLAN_STATUS_MULTICAST_FULL
*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
wlanoidSetP2PTerminateSDPhase(IN P_ADAPTER_T prAdapter,
			      IN PVOID pvSetBuffer, IN UINT_32 u4SetBufferLen, OUT PUINT_32 pu4SetInfoLen)
{
	WLAN_STATUS rWlanStatus = WLAN_STATUS_SUCCESS;
	P_PARAM_P2P_TERMINATE_SD_PHASE prP2pTerminateSD = (P_PARAM_P2P_TERMINATE_SD_PHASE) NULL;
	UINT_8 aucNullAddr[] = NULL_MAC_ADDR;

	do {
		if ((prAdapter == NULL) || (pu4SetInfoLen == NULL))
			break;

		if ((u4SetBufferLen > 0U) && (pvSetBuffer == NULL))
			break;

		if (u4SetBufferLen <
			(UINT_32)sizeof(PARAM_P2P_TERMINATE_SD_PHASE)) {
			*pu4SetInfoLen =
				(UINT_32)sizeof(PARAM_P2P_TERMINATE_SD_PHASE);
			rWlanStatus = WLAN_STATUS_BUFFER_TOO_SHORT;
			break;
		}

		prP2pTerminateSD = (P_PARAM_P2P_TERMINATE_SD_PHASE) pvSetBuffer;

		if (EQUAL_MAC_ADDR(prP2pTerminateSD->rPeerAddr, aucNullAddr)) {
			DBGLOG(P2P, TRACE, "Service Discovery Version 2.0\n");
/* p2pFuncSetVersionNumOfSD(prAdapter, 2); */
		}
		/* rWlanStatus = p2pFsmRunEventSDAbort(prAdapter); */

	} while (0U != 0U);

	return rWlanStatus;
}				/* end of wlanoidSetP2PTerminateSDPhase() */

#if CFG_SUPPORT_ANTI_PIRACY
/*----------------------------------------------------------------------------*/
/*!
* \brief This routine is called to
*
* \param[in] prAdapter      Pointer to the Adapter structure.
* \param[in] pvSetBuffer    Pointer to the buffer that holds the data to be set.
* \param[in] u4SetBufferLen The length of the set buffer.
* \param[out] pu4SetInfoLen If the call is successful, returns the number of
*                           bytes read from the set buffer. If the call failed
*                           due to invalid length of the set buffer, returns
*                           the amount of storage needed.
*
* \retval WLAN_STATUS_SUCCESS
* \retval WLAN_STATUS_INVALID_LENGTH
* \retval WLAN_STATUS_ADAPTER_NOT_READY
* \retval WLAN_STATUS_MULTICAST_FULL
*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
wlanoidSetSecCheckRequest(IN P_ADAPTER_T prAdapter,
			  IN PVOID pvSetBuffer, IN UINT_32 u4SetBufferLen, OUT PUINT_32 pu4SetInfoLen)
{
	ASSERT(prAdapter);
	ASSERT(pu4SetInfoLen);

	if (u4SetBufferLen != 0U)
		ASSERT(pvSetBuffer);

	return wlanoidSendSetQueryP2PCmd(prAdapter,
					 (UINT_8)CMD_ID_SEC_CHECK,
					 P2P_DEV_BSS_INDEX,
					 FALSE,
					 TRUE,
					 TRUE,
					 NULL,
					 nicOidCmdTimeoutCommon,
					 u4SetBufferLen, (PUINT_8) pvSetBuffer, pvSetBuffer, u4SetBufferLen);

}				/* end of wlanoidSetSecCheckRequest() */

/*----------------------------------------------------------------------------*/
/*!
* \brief This routine is called to
*
* \param[in]  prAdapter        Pointer to the Adapter structure.
* \param[out] pvQueryBuffer    A pointer to the buffer that holds the result of
*                              the query.
* \param[in]  u4QueryBufferLen The length of the query buffer.
* \param[out] pu4QueryInfoLen  If the call is successful, returns the number of
*                              bytes written into the query buffer. If the call
*                              failed due to invalid length of the query buffer,
*                              returns the amount of storage needed.
*
* \retval WLAN_STATUS_SUCCESS
* \retval WLAN_STATUS_INVALID_LENGTH
* \retval WLAN_STATUS_ADAPTER_NOT_READY
* \retval WLAN_STATUS_MULTICAST_FULL
*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
wlanoidGetSecCheckResponse(IN P_ADAPTER_T prAdapter,
			   IN PVOID pvQueryBuffer, IN UINT_32 u4QueryBufferLen, OUT PUINT_32 pu4QueryInfoLen)
{
	WLAN_STATUS rWlanStatus = WLAN_STATUS_SUCCESS;
	/* P_WLAN_MAC_HEADER_T prWlanHdr = (P_WLAN_MAC_HEADER_T)NULL; */
	P_GLUE_INFO_T prGlueInfo;

	prGlueInfo = prAdapter->prGlueInfo;

	ASSERT(prAdapter);
	ASSERT(pu4QueryInfoLen);

	if (u4QueryBufferLen != 0U)
		ASSERT(pvQueryBuffer);

	if (u4QueryBufferLen > 256U)
		u4QueryBufferLen = 256U;

	*pu4QueryInfoLen = u4QueryBufferLen;

#if DBG
	DBGLOG_MEM8(SEC, LOUD, prGlueInfo->prP2PInfo->aucSecCheckRsp, u4QueryBufferLen);
#endif
	if (u4QueryBufferLen > 0U)
		kalMemCopy((PUINT_8) (pvQueryBuffer +
			OFFSET_OF(IW_P2P_TRANSPORT_STRUCT, aucBuffer)),
			prGlueInfo->prP2PInfo->aucSecCheckRsp,
			u4QueryBufferLen);

	return rWlanStatus;
}				/* end of wlanoidGetSecCheckResponse() */
#endif

WLAN_STATUS
wlanoidSetNoaParam(IN P_ADAPTER_T prAdapter,
		   IN PVOID pvSetBuffer, IN UINT_32 u4SetBufferLen, OUT PUINT_32 pu4SetInfoLen)
{
	P_PARAM_CUSTOM_NOA_PARAM_STRUCT_T prNoaParam;
	CMD_CUSTOM_NOA_PARAM_STRUCT_T rCmdNoaParam;

	DEBUGFUNC("wlanoidSetNoaParam");
	DBGLOG(INIT, TRACE, "\n");

	ASSERT(prAdapter);
	ASSERT(pu4SetInfoLen);

	*pu4SetInfoLen = (UINT_32)sizeof(PARAM_CUSTOM_NOA_PARAM_STRUCT_T);

	if (u4SetBufferLen < (UINT_32)sizeof(PARAM_CUSTOM_NOA_PARAM_STRUCT_T))
		return WLAN_STATUS_INVALID_LENGTH;

	ASSERT(pvSetBuffer);

	prNoaParam = (P_PARAM_CUSTOM_NOA_PARAM_STRUCT_T) pvSetBuffer;

	kalMemZero(&rCmdNoaParam,
		(UINT_32)sizeof(CMD_CUSTOM_NOA_PARAM_STRUCT_T));
	rCmdNoaParam.u4NoaDurationMs = prNoaParam->u4NoaDurationMs;
	rCmdNoaParam.u4NoaIntervalMs = prNoaParam->u4NoaIntervalMs;
	rCmdNoaParam.u4NoaCount = prNoaParam->u4NoaCount;

#if 0
	return wlanSendSetQueryCmd(prAdapter,
			(UINT_8)CMD_ID_SET_NOA_PARAM,
			TRUE,
			FALSE,
			TRUE,
			nicCmdEventSetCommon,
			nicOidCmdTimeoutCommon,
			(UINT_32)sizeof(CMD_CUSTOM_NOA_PARAM_STRUCT_T),
			(PUINT_8) &rCmdNoaParam, pvSetBuffer, u4SetBufferLen);
#else
	return wlanoidSendSetQueryP2PCmd(prAdapter,
			(UINT_8)CMD_ID_SET_NOA_PARAM,
			prNoaParam->ucBssIdx,
			TRUE,
			FALSE,
			TRUE,
			NULL,
			nicOidCmdTimeoutCommon,
			(UINT_32)sizeof(CMD_CUSTOM_NOA_PARAM_STRUCT_T),
			(PUINT_8) &rCmdNoaParam, pvSetBuffer, u4SetBufferLen);

#endif

}

WLAN_STATUS
wlanoidSetOppPsParam(IN P_ADAPTER_T prAdapter,
		     IN PVOID pvSetBuffer, IN UINT_32 u4SetBufferLen, OUT PUINT_32 pu4SetInfoLen)
{
	P_PARAM_CUSTOM_OPPPS_PARAM_STRUCT_T prOppPsParam;
	CMD_CUSTOM_OPPPS_PARAM_STRUCT_T rCmdOppPsParam;

	DEBUGFUNC("wlanoidSetOppPsParam");
	DBGLOG(INIT, TRACE, "\n");

	ASSERT(prAdapter);
	ASSERT(pu4SetInfoLen);

	*pu4SetInfoLen = (UINT_32)sizeof(PARAM_CUSTOM_OPPPS_PARAM_STRUCT_T);

	if (u4SetBufferLen < (UINT_32)sizeof(PARAM_CUSTOM_OPPPS_PARAM_STRUCT_T))
		return WLAN_STATUS_INVALID_LENGTH;

	ASSERT(pvSetBuffer);

	prOppPsParam = (P_PARAM_CUSTOM_OPPPS_PARAM_STRUCT_T) pvSetBuffer;

	kalMemZero(&rCmdOppPsParam,
		(UINT_32)sizeof(CMD_CUSTOM_OPPPS_PARAM_STRUCT_T));
	rCmdOppPsParam.u4CTwindowMs = prOppPsParam->u4CTwindowMs;

#if 0
	return wlanSendSetQueryCmd(prAdapter,
		(UINT_8)CMD_ID_SET_OPPPS_PARAM,
		TRUE,
		FALSE,
		TRUE,
		nicCmdEventSetCommon,
		nicOidCmdTimeoutCommon,
		(UINT_32)sizeof(CMD_CUSTOM_OPPPS_PARAM_STRUCT_T),
		(PUINT_8) &rCmdOppPsParam, pvSetBuffer, u4SetBufferLen);
#else
	return wlanoidSendSetQueryP2PCmd(prAdapter,
			(UINT_8)CMD_ID_SET_NOA_PARAM,
			prOppPsParam->ucBssIdx,
			TRUE,
			FALSE,
			TRUE,
			NULL,
			nicOidCmdTimeoutCommon,
			(UINT_32)sizeof(CMD_CUSTOM_OPPPS_PARAM_STRUCT_T),
			(PUINT_8) &rCmdOppPsParam, pvSetBuffer, u4SetBufferLen);

#endif

}

WLAN_STATUS
wlanoidSetUApsdParam(IN P_ADAPTER_T prAdapter,
		     IN PVOID pvSetBuffer, IN UINT_32 u4SetBufferLen, OUT PUINT_32 pu4SetInfoLen)
{
	P_PARAM_CUSTOM_UAPSD_PARAM_STRUCT_T prUapsdParam;
	CMD_CUSTOM_UAPSD_PARAM_STRUCT_T rCmdUapsdParam;
	P_PM_PROFILE_SETUP_INFO_T prPmProfSetupInfo;
	P_BSS_INFO_T prBssInfo;

	DEBUGFUNC("wlanoidSetUApsdParam");
	DBGLOG(INIT, TRACE, "\n");

	ASSERT(prAdapter);
	ASSERT(pu4SetInfoLen);

	*pu4SetInfoLen = (UINT_32)sizeof(PARAM_CUSTOM_UAPSD_PARAM_STRUCT_T);

	if (u4SetBufferLen < (UINT_32)sizeof(PARAM_CUSTOM_UAPSD_PARAM_STRUCT_T))
		return WLAN_STATUS_INVALID_LENGTH;

	ASSERT(pvSetBuffer);

	prUapsdParam = (P_PARAM_CUSTOM_UAPSD_PARAM_STRUCT_T) pvSetBuffer;

	prBssInfo = GET_BSS_INFO_BY_INDEX(prAdapter, prUapsdParam->ucBssIdx);
	prPmProfSetupInfo = &prBssInfo->rPmProfSetupInfo;

	kalMemZero(&rCmdUapsdParam,
		(UINT_32)sizeof(CMD_CUSTOM_UAPSD_PARAM_STRUCT_T));
	rCmdUapsdParam.fgEnAPSD = prUapsdParam->fgEnAPSD;

	rCmdUapsdParam.fgEnAPSD_AcBe = prUapsdParam->fgEnAPSD_AcBe;
	rCmdUapsdParam.fgEnAPSD_AcBk = prUapsdParam->fgEnAPSD_AcBk;
	rCmdUapsdParam.fgEnAPSD_AcVo = prUapsdParam->fgEnAPSD_AcVo;
	rCmdUapsdParam.fgEnAPSD_AcVi = prUapsdParam->fgEnAPSD_AcVi;
	prPmProfSetupInfo->ucBmpDeliveryAC =
	    ((prUapsdParam->fgEnAPSD_AcBe << 0U) |
	     (prUapsdParam->fgEnAPSD_AcBk << 1U) |
	     (prUapsdParam->fgEnAPSD_AcVi << 2U) |
	     (prUapsdParam->fgEnAPSD_AcVo << 3U));
	prPmProfSetupInfo->ucBmpTriggerAC =
	    ((prUapsdParam->fgEnAPSD_AcBe << 0U) |
	     (prUapsdParam->fgEnAPSD_AcBk << 1U) |
	     (prUapsdParam->fgEnAPSD_AcVi << 2U) |
	     (prUapsdParam->fgEnAPSD_AcVo << 3U));

	rCmdUapsdParam.ucMaxSpLen = prUapsdParam->ucMaxSpLen;
	prPmProfSetupInfo->ucUapsdSp = prUapsdParam->ucMaxSpLen;

	return wlanoidSendSetQueryP2PCmd(prAdapter,
		 (UINT_8)CMD_ID_SET_UAPSD_PARAM,
		 prBssInfo->ucBssIndex,
		 TRUE,
		 FALSE,
		 TRUE,
		 NULL,
		 nicOidCmdTimeoutCommon,
		 (UINT_32)sizeof(CMD_CUSTOM_UAPSD_PARAM_STRUCT_T),
		 (PUINT_8) &rCmdUapsdParam, pvSetBuffer, u4SetBufferLen);
}

WLAN_STATUS
wlanoidQueryP2pOpChannel(IN P_ADAPTER_T prAdapter,
			 IN PVOID pvQueryBuffer, IN UINT_32 u4QueryBufferLen, OUT PUINT_32 pu4QueryInfoLen)
{

	WLAN_STATUS rResult = WLAN_STATUS_FAILURE;
/* PUINT_8 pucOpChnl = (PUINT_8)pvQueryBuffer; */

	do {
		if ((prAdapter == NULL) || (pu4QueryInfoLen == NULL))
			break;

		if ((u4QueryBufferLen != 0U) && (pvQueryBuffer == NULL))
			break;

		if (u4QueryBufferLen < (UINT_32)sizeof(UINT_8)) {
			*pu4QueryInfoLen = (UINT_32)sizeof(UINT_8);
			rResult = WLAN_STATUS_BUFFER_TOO_SHORT;
			break;
		}
#if 0
		if (!p2pFuncGetCurrentOpChnl(prAdapter, pucOpChnl)) {
			rResult = WLAN_STATUS_INVALID_DATA;
			break;
		}
#else
		rResult = WLAN_STATUS_INVALID_DATA;
		break;
#endif
/*
		*pu4QueryInfoLen = (UINT_32)sizeof(UINT_8);
		rResult = WLAN_STATUS_SUCCESS;
*/
	} while (0U != 0U);

	return rResult;
}				/* wlanoidQueryP2pOpChannel */

WLAN_STATUS
wlanoidQueryP2pVersion(IN P_ADAPTER_T prAdapter,
		       IN PVOID pvQueryBuffer, IN UINT_32 u4QueryBufferLen, OUT PUINT_32 pu4QueryInfoLen)
{
	WLAN_STATUS rResult = WLAN_STATUS_FAILURE;
/* PUINT_8 pucVersionNum = (PUINT_8)pvQueryBuffer; */

	do {
		if ((prAdapter == NULL) || (pu4QueryInfoLen == NULL))
			break;

		if ((u4QueryBufferLen != 0U) && (pvQueryBuffer == NULL))
			break;

		if (u4QueryBufferLen < (UINT_32)sizeof(UINT_8)) {
			*pu4QueryInfoLen = (UINT_32)sizeof(UINT_8);
			rResult = WLAN_STATUS_BUFFER_TOO_SHORT;
			break;
		}

	} while (0U != 0U);

	return rResult;
}				/* wlanoidQueryP2pVersion */

#if CFG_SUPPORT_HOTSPOT_WPS_MANAGER

/*----------------------------------------------------------------------------*/
/*!
* \brief This routine is used to set the WPS mode.
*
* \param[in] pvAdapter Pointer to the Adapter structure.
* \param[in] pvSetBuffer A pointer to the buffer that holds the data to be set.
* \param[in] u4SetBufferLen The length of the set buffer.
* \param[out] pu4SetInfoLen If the call is successful, returns the number of
*                          bytes read from the set buffer. If the call failed
*                          due to invalid length of the set buffer, returns
*                          the amount of storage needed.
*
* \retval WLAN_STATUS_SUCCESS
* \retval WLAN_STATUS_INVALID_LENGTH
*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
wlanoidSetP2pWPSmode(IN P_ADAPTER_T prAdapter,
		     IN PVOID pvSetBuffer, IN UINT_32 u4SetBufferLen, OUT PUINT_32 pu4SetInfoLen)
{
	WLAN_STATUS status;
	UINT_32 u4IsWPSmode = 0;

	DEBUGFUNC("wlanoidSetP2pWPSmode");

	ASSERT(prAdapter);
	ASSERT(pu4SetInfoLen);

	if (pvSetBuffer != NULL)
		u4IsWPSmode = *(PUINT_32) pvSetBuffer;
	else
		u4IsWPSmode = 0U;

	if (u4IsWPSmode != 0U)
		prAdapter->rWifiVar.prP2PConnSettings->fgIsWPSMode = 1U;
	else
		prAdapter->rWifiVar.prP2PConnSettings->fgIsWPSMode = 0U;

	status = nicUpdateBss(prAdapter, P2P_DEV_BSS_INDEX);

	return status;
}				/* end of wlanoidSetP2pWPSmode() */

#endif

WLAN_STATUS
wlanoidSetP2pSupplicantVersion(IN P_ADAPTER_T prAdapter,
			       IN PVOID pvSetBuffer, IN UINT_32 u4SetBufferLen, OUT PUINT_32 pu4SetInfoLen)
{
	WLAN_STATUS rResult;
	UINT_8 ucVersionNum;

	do {
		if ((prAdapter == NULL) || (pu4SetInfoLen == NULL)) {

			rResult = WLAN_STATUS_INVALID_DATA;
			break;
		}

		if ((u4SetBufferLen != 0U) && (pvSetBuffer == NULL)) {
			rResult = WLAN_STATUS_INVALID_DATA;
			break;
		}

		*pu4SetInfoLen = (UINT_32)sizeof(UINT_8);

		if (u4SetBufferLen < (UINT_32)sizeof(UINT_8)) {
			rResult = WLAN_STATUS_INVALID_LENGTH;
			break;
		}

		ucVersionNum = *((PUINT_8) pvSetBuffer);
		DBGLOG(REQ, TRACE, "ucVersionNum %d\n", ucVersionNum);

		rResult = WLAN_STATUS_SUCCESS;
	} while (0U != 0U);

	return rResult;
}				/* wlanoidSetP2pSupplicantVersion */

#if CFG_SUPPORT_P2P_RSSI_QUERY
WLAN_STATUS
wlanoidQueryP2pRssi(IN P_ADAPTER_T prAdapter,
		    IN PVOID pvQueryBuffer, IN UINT_32 u4QueryBufferLen, OUT PUINT_32 pu4QueryInfoLen)
{
	DEBUGFUNC("wlanoidQueryP2pRssi");

	ASSERT(prAdapter);
	ASSERT(pu4QueryInfoLen);
	if (u4QueryBufferLen != 0U)
		ASSERT(pvQueryBuffer);

	*pu4QueryInfoLen = (UINT_32)sizeof(PARAM_RSSI);

	/* Check for query buffer length */
	if (u4QueryBufferLen < *pu4QueryInfoLen) {
		DBGLOG(REQ, WARN, "Too short length %ld\n", u4QueryBufferLen);
		return WLAN_STATUS_BUFFER_TOO_SHORT;
	}

	if (prAdapter->fgIsP2pLinkQualityValid == TRUE &&
	    (kalGetTimeTick() - prAdapter->rP2pLinkQualityUpdateTime) <= CFG_LINK_QUALITY_VALID_PERIOD) {
		PARAM_RSSI rRssi;

		rRssi = (PARAM_RSSI) prAdapter->rP2pLinkQuality.cRssi;	/* ranged from (-128 ~ 30) in unit of dBm */

		if (rRssi > PARAM_WHQL_RSSI_MAX_DBM)
			rRssi = PARAM_WHQL_RSSI_MAX_DBM;
		else if (rRssi < PARAM_WHQL_RSSI_MIN_DBM)
			rRssi = PARAM_WHQL_RSSI_MIN_DBM;

		kalMemCopy(pvQueryBuffer, &rRssi, (UINT_32)sizeof(PARAM_RSSI));
		return WLAN_STATUS_SUCCESS;
	}
#ifdef LINUX
	return wlanSendSetQueryCmd(prAdapter,
				   (UINT_8)CMD_ID_GET_LINK_QUALITY,
				   FALSE,
				   TRUE,
				   TRUE,
				   nicCmdEventQueryLinkQuality,
				   nicOidCmdTimeoutCommon,
				   *pu4QueryInfoLen, pvQueryBuffer, pvQueryBuffer, u4QueryBufferLen);
#else
	return wlanSendSetQueryCmd(prAdapter,
				   (UINT_8)CMD_ID_GET_LINK_QUALITY,
				   FALSE,
				   TRUE,
				   TRUE,
				   nicCmdEventQueryLinkQuality,
				   nicOidCmdTimeoutCommon, 0, NULL, pvQueryBuffer, u4QueryBufferLen);

#endif
}				/* wlanoidQueryP2pRssi */
#endif
