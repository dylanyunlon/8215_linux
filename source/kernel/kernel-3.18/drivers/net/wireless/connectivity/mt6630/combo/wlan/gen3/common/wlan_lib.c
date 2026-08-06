//SPDX-License-Identifier: GPL-2.0-only
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
 * ! \file   wlan_lib.c
 * \brief  Internal driver stack will export the required procedures here for GLUE Layer.
 *
 * This file contains all routines which are exported from MediaTek 802.11 Wireless
 * LAN driver stack to GLUE Layer.
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
#if defined(MT6631) && 0
#include <mach/emi_mpu.h>
#endif

/*******************************************************************************
 *                              C O N S T A N T S
 ********************************************************************************
 */
/* 6.1.1.2 Interpretation of priority parameter in MAC service primitives */
/* Static convert the Priority Parameter/TID(User Priority/TS Identifier) to Traffic Class */
const UINT_8 aucPriorityParam2TC[] = {
	(UINT_8)TC1_INDEX,
	(UINT_8)TC0_INDEX,
	(UINT_8)TC0_INDEX,
	(UINT_8)TC1_INDEX,
	(UINT_8)TC2_INDEX,
	(UINT_8)TC2_INDEX,
	(UINT_8)TC3_INDEX,
	(UINT_8)TC3_INDEX
};

/*******************************************************************************
 *                             D A T A   T Y P E S
 ********************************************************************************
 */
typedef struct _CODE_MAPPING_T {
	UINT_32 u4RegisterValue;
	INT_32 u4TxpowerOffset;
} CODE_MAPPING_T, *P_CODE_MAPPING_T;

/*******************************************************************************
 *                            P U B L I C   D A T A
 ********************************************************************************
 */
BOOLEAN fgIsBusAccessFailed = FALSE;

#define CFG_POWER_CTRL_ACQUIRE_RES_TIMEOUT 3000

#define WLAN_WAIT_READY_BIT_TIMEOUT 5000

/*******************************************************************************
 *                           P R I V A T E   D A T A
 ********************************************************************************
 */

/*******************************************************************************
 *                                 M A C R O S
 ********************************************************************************
 */
#define SIGNED_EXTEND(n, _sValue) \
	(((_sValue) & BIT((n)-1)) ? ((_sValue) | BITS(n, 31U)) : \
	 ((_sValue) & ~BITS(n, 31U)))

/* TODO: Check */
/* OID set handlers without the need to access HW register */
PFN_OID_HANDLER_FUNC apfnOidSetHandlerWOHwAccess[] = {
	wlanoidSetChannel,
	wlanoidSetBeaconInterval,
	wlanoidSetAtimWindow,
	wlanoidSetFrequency,
};

/* TODO: Check */
/* OID query handlers without the need to access HW register */
PFN_OID_HANDLER_FUNC apfnOidQueryHandlerWOHwAccess[] = {
	wlanoidQueryBssid,
	wlanoidQuerySsid,
	wlanoidQueryInfrastructureMode,
	wlanoidQueryAuthMode,
	wlanoidQueryEncryptionStatus,
	wlanoidQueryNetworkTypeInUse,
	wlanoidQueryBssidList,
	wlanoidQueryAcpiDevicePowerState,
	wlanoidQuerySupportedRates,
	wlanoidQueryDesiredRates,
	wlanoidQuery802dot11PowerSaveProfile,
	wlanoidQueryBeaconInterval,
	wlanoidQueryAtimWindow,
	wlanoidQueryFrequency,
};

/* OID set handlers allowed in RF test mode */
PFN_OID_HANDLER_FUNC apfnOidSetHandlerAllowedInRFTest[] = {
	wlanoidRftestSetTestMode,
	wlanoidRftestSetAbortTestMode,
	wlanoidRftestSetAutoTest,
	wlanoidSetMcrWrite,
	wlanoidSetEepromWrite
};

/* OID query handlers allowed in RF test mode */
PFN_OID_HANDLER_FUNC apfnOidQueryHandlerAllowedInRFTest[] = {
	wlanoidRftestQueryAutoTest,
	wlanoidQueryMcrRead,
	wlanoidQueryEepromRead
};

PFN_OID_HANDLER_FUNC apfnOidWOTimeoutCheck[] = {
	wlanoidRftestSetTestMode,
	wlanoidRftestSetAbortTestMode,
	wlanoidSetAcpiDevicePowerState,
};

/*******************************************************************************
*                                 M A C R O S
********************************************************************************
*/

/*******************************************************************************
*                  F U N C T I O N   D E C L A R A T I O N S
********************************************************************************
*/
#if CFG_ENABLE_FW_DOWNLOAD
static WLAN_STATUS
wlanImageFullDownload(IN P_ADAPTER_T prAdapter,
		      IN PVOID pvFwImageMapFile,
		      IN UINT_32 u4FwLoadAddr,
		      IN UINT_32 u4FwImageFileLength);

#if CFG_ENABLE_FW_DIVIDED_DOWNLOAD
static WLAN_STATUS
wlanImageDividDownloadByEntry(IN P_ADAPTER_T prAdapter,
			      IN PVOID pvFwImageMapFile,
			      IN P_FIRMWARE_DIVIDED_DOWNLOAD_T prFwHead,
			      IN UINT_32 index);

static WLAN_STATUS
wlanImageDividDownload(IN P_ADAPTER_T prAdapter, IN PVOID pvFwImageMapFile);
#endif
#endif
/*******************************************************************************
*                              F U N C T I O N S
********************************************************************************
*/
/*----------------------------------------------------------------------------*/
/*!
* \brief This is a private routine, which is used to check if HW access is needed
*        for the OID query/ set handlers.
*
* \param[IN] pfnOidHandler Pointer to the OID handler.
* \param[IN] fgSetInfo     It is a Set information handler.
*
* \retval TRUE This function needs HW access
* \retval FALSE This function does not need HW access
*/
/*----------------------------------------------------------------------------*/
BOOLEAN wlanIsHandlerNeedHwAccess(IN PFN_OID_HANDLER_FUNC pfnOidHandler, IN BOOLEAN fgSetInfo)
{
	PFN_OID_HANDLER_FUNC *apfnOidHandlerWOHwAccess;
	UINT_32 i;
	UINT_32 u4NumOfElem;

	if (fgSetInfo == TRUE) {
		apfnOidHandlerWOHwAccess = apfnOidSetHandlerWOHwAccess;
		u4NumOfElem = (UINT_32)sizeof(apfnOidSetHandlerWOHwAccess) /
			(UINT_32)sizeof(PFN_OID_HANDLER_FUNC);
	} else {
		apfnOidHandlerWOHwAccess = apfnOidQueryHandlerWOHwAccess;
		u4NumOfElem = (UINT_32)sizeof(apfnOidQueryHandlerWOHwAccess) /
			(UINT_32)sizeof(PFN_OID_HANDLER_FUNC);
	}

	for (i = 0U; i < u4NumOfElem; i++) {
		if (apfnOidHandlerWOHwAccess[i] == pfnOidHandler)
			return FALSE;
	}

	return TRUE;
}				/* wlanIsHandlerNeedHwAccess */

/*----------------------------------------------------------------------------*/
/*!
* \brief This routine is called to set flag for later handling card
*        ejected event.
*
* \param[in] prAdapter Pointer to the Adapter structure.
*
* \return (none)
*
* \note When surprised removal happens, Glue layer should invoke this
*       function to notify WPDD not to do any hw access.
*/
/*----------------------------------------------------------------------------*/
VOID wlanCardEjected(IN P_ADAPTER_T prAdapter)
{
	DEBUGFUNC("wlanCardEjected");
	/* INITLOG(("\n")); */

	ASSERT(prAdapter);

	/* mark that the card is being ejected, NDIS will shut us down soon */
	nicTxRelease(prAdapter, FALSE);

}				/* wlanCardEjected */

/*----------------------------------------------------------------------------*/
/*!
* \brief Create adapter object
*
* \param prAdapter This routine is call to allocate the driver software objects.
*                  If fails, return NULL.
* \retval NULL If it fails, NULL is returned.
* \retval NOT NULL If the adapter was initialized successfully.
*/
/*----------------------------------------------------------------------------*/
P_ADAPTER_T wlanAdapterCreate(IN P_GLUE_INFO_T prGlueInfo)
{
	P_ADAPTER_T prAdpater = (P_ADAPTER_T) NULL;

	DEBUGFUNC("wlanAdapterCreate");

	do {
		prAdpater = (P_ADAPTER_T)
			kalMemAlloc((UINT_32)sizeof(ADAPTER_T), VIR_MEM_TYPE);

		if (prAdpater == NULL) {
			DBGLOG(INIT, ERROR, "Allocate ADAPTER memory ==> FAILED\n");
			break;
		}
#if QM_TEST_MODE
		g_rQM.prAdapter = prAdpater;
#endif
		kalMemZero(prAdpater, (UINT_32)sizeof(ADAPTER_T));
		prAdpater->prGlueInfo = prGlueInfo;

	} while (FALSE);

	return prAdpater;
}				/* wlanAdapterCreate */

/*----------------------------------------------------------------------------*/
/*!
* \brief Destroy adapter object
*
* \param prAdapter This routine is call to destroy the driver software objects.
*                  If fails, return NULL.
* \return (none)
*/
/*----------------------------------------------------------------------------*/
VOID wlanAdapterDestroy(IN P_ADAPTER_T prAdapter)
{

	if (prAdapter == NULL)
		return;

	kalMemFree(prAdapter, VIR_MEM_TYPE, (UINT_32)sizeof(ADAPTER_T));
}

/*----------------------------------------------------------------------------*/
/*!
* \brief Initialize the adapter. The sequence is
*        1. Disable interrupt
*        2. Read adapter configuration from EEPROM and registry, verify chip ID.
*        3. Create NIC Tx/Rx resource.
*        4. Initialize the chip
*        5. Initialize the protocol
*        6. Enable Interrupt
*
* \param prAdapter      Pointer of Adapter Data Structure
*
* \retval WLAN_STATUS_SUCCESS: Success
* \retval WLAN_STATUS_FAILURE: Failed
*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
wlanAdapterStart(IN P_ADAPTER_T prAdapter,
		 IN P_REG_INFO_T prRegInfo, IN PVOID pvFwImageMapFile, IN UINT_32 u4FwImageFileLength)
{
	WLAN_STATUS u4Status = WLAN_STATUS_SUCCESS;
	UINT_32 i, u4Value = 0U;
	UINT_32 u4WHISR = 0U;
	UINT_16 au2TxCount[16];
#if CFG_ENABLE_FW_DOWNLOAD
	UINT_32 u4FwLoadAddr;
#if CFG_ENABLE_FW_DIVIDED_DOWNLOAD
	P_FIRMWARE_DIVIDED_DOWNLOAD_T prFwHead;
	BOOLEAN fgValidHead;
	const UINT_32 u4CRCOffset = (UINT_32)
		offsetof(FIRMWARE_DIVIDED_DOWNLOAD_T, u4NumOfEntries);
#endif
#endif

	enum ENUM_ADAPTER_START_FAIL_REASON {
		ALLOC_ADAPTER_MEM_FAIL,
		DRIVER_OWN_FAIL,
		INIT_ADAPTER_FAIL,
		RAM_CODE_DOWNLOAD_FAIL,
		WAIT_FIRMWARE_READY_FAIL,
		FAIL_REASON_MAX
	} eFailReason;
	ASSERT(prAdapter);

	DEBUGFUNC("wlanAdapterStart");

	eFailReason = FAIL_REASON_MAX;
	/* 4 <0> Reset variables in ADAPTER_T */
	/* prAdapter->fgIsFwOwn = TRUE; */
	prAdapter->fgIsEnterD3ReqIssued = FALSE;

	prAdapter->u4OwnFailedCount = 0U;
	prAdapter->u4OwnFailedLogCount = 0U;

	QUEUE_INITIALIZE(&(prAdapter->rPendingCmdQueue));
#if CFG_SUPPORT_MULTITHREAD
	QUEUE_INITIALIZE(&prAdapter->rTxCmdQueue);
	QUEUE_INITIALIZE(&prAdapter->rTxCmdDoneQueue);
	QUEUE_INITIALIZE(&prAdapter->rTxP0Queue);
	QUEUE_INITIALIZE(&prAdapter->rTxP1Queue);
	QUEUE_INITIALIZE(&prAdapter->rRxQueue);
#endif

	/* Initialize rWlanInfo */
	kalMemSet(&(prAdapter->rWlanInfo), 0, (UINT_32)sizeof(WLAN_INFO_T));

	/* Initialize aprBssInfo[].
	 * Important: index shall be same when mapping between aprBssInfo[]
	 *            and arBssInfoPool[]. rP2pDevInfo is indexed to final one.
	 */
	for (i = 0; i < BSS_INFO_NUM; i++)
		prAdapter->aprBssInfo[i] = &prAdapter->rWifiVar.arBssInfoPool[i];
	prAdapter->aprBssInfo[P2P_DEV_BSS_INDEX] = &prAdapter->rWifiVar.rP2pDevInfo;

	/* 4 <0.1> reset fgIsBusAccessFailed */
	fgIsBusAccessFailed = FALSE;
	prAdapter->ulSuspendFlag = 0U;
	do {
		u4Status = nicAllocateAdapterMemory(prAdapter);
		if (u4Status != WLAN_STATUS_SUCCESS) {
			DBGLOG(INIT, ERROR, "nicAllocateAdapterMemory Error!\n");
			u4Status = WLAN_STATUS_FAILURE;
			eFailReason = ALLOC_ADAPTER_MEM_FAIL;
			break;
		}

		prAdapter->u4OsPacketFilter = PARAM_PACKET_FILTER_SUPPORTED;

		DBGLOG(INIT, TRACE, "wlanAdapterStart(): Acquiring LP-OWN\n");
		ACQUIRE_POWER_CONTROL_FROM_PM(prAdapter);
		DBGLOG(INIT, TRACE, "wlanAdapterStart(): Acquiring LP-OWN-end\n");

#if !CFG_ENABLE_FULL_PM
		nicpmSetDriverOwn(prAdapter);
#endif

		if (prAdapter->fgIsFwOwn == TRUE) {
			DBGLOG(INIT, ERROR, "nicpmSetDriverOwn() failed!\n");
			u4Status = WLAN_STATUS_FAILURE;
			eFailReason = DRIVER_OWN_FAIL;
#if CFG_CHIP_RESET_SUPPORT
			DBGLOG(INIT, WARN, "DRIVER_OWN_FAIL and trigger reset\n");
			//glResetTrigger(prAdapter);
			glResetWifiOnly();
#endif
			break;
		}
		/* 4 <1> Initialize the Adapter */
		u4Status = nicInitializeAdapter(prAdapter);
		if (u4Status != WLAN_STATUS_SUCCESS) {
			DBGLOG(INIT, ERROR, "nicInitializeAdapter failed!\n");
			u4Status = WLAN_STATUS_FAILURE;
			eFailReason = INIT_ADAPTER_FAIL;
			break;
		}

		/* 4 <2.1> Initialize System Service (MGMT Memory pool and STA_REC) */
		nicInitSystemService(prAdapter);

		/* 4 <2.2> Initialize Feature Options */
		wlanInitFeatureOption(prAdapter);
#if CFG_SUPPORT_MTK_SYNERGY
		if (kalIsConfigurationExist(prAdapter->prGlueInfo) == TRUE) {
			if ((prRegInfo->prNvramSettings->u2FeatureReserved &
				(UINT_16)BIT((UINT_32)
				MTK_FEATURE_2G_256QAM_DISABLED))
				!= 0U)
				prAdapter->rWifiVar.aucMtkFeature[0] &=
					~((UINT_8)
					MTK_SYNERGY_CAP_SUPPORT_24G_MCS89);
		}
#endif


		/* 4 <2.3> Overwrite debug level settings */
		wlanCfgSetDebugLevel(prAdapter);


		/* 4 <3> Initialize Tx */
		nicTxInitialize(prAdapter);
		wlanDefTxPowerCfg(prAdapter);

		/* 4 <4> Initialize Rx */
		nicRxInitialize(prAdapter);

#if CFG_ENABLE_FW_DOWNLOAD
		if (pvFwImageMapFile != NULL) {
			/* 1. disable interrupt, download is done by polling mode only */
			nicDisableInterrupt(prAdapter);

			/* 2. Initialize Tx Resource to fw download state */
			if (nicTxInitResetResource(prAdapter) ==
				WLAN_STATUS_SUCCESS)
				DBGLOG(INIT, TRACE, "Initialize Tx Resource\n");

			/* 3. FW download here */
			u4FwLoadAddr = prRegInfo->u4LoadAddress;

			DBGLOG(INIT, INFO, "FW download start...\n");

#if CFG_ENABLE_FW_DIVIDED_DOWNLOAD
			/* 3a. parse file header for decision of divided firmware download or not */
			prFwHead = (P_FIRMWARE_DIVIDED_DOWNLOAD_T) pvFwImageMapFile;

			if (prFwHead->u4Signature == MTK_WIFI_SIGNATURE &&
			    prFwHead->u4CRC == wlanCRC32((PUINT_8) pvFwImageMapFile + u4CRCOffset,
							 u4FwImageFileLength - u4CRCOffset)) {
				fgValidHead = TRUE;
			} else {
				fgValidHead = FALSE;
			}

			/* 3b. engage divided firmware downloading */
			if (fgValidHead == TRUE) {
				if (wlanImageDividDownload(prAdapter,
							   pvFwImageMapFile) != WLAN_STATUS_SUCCESS)
					u4Status = WLAN_STATUS_FAILURE;
			} else
#endif
			{
				if (wlanImageFullDownload(prAdapter,
							  pvFwImageMapFile,
							  u4FwLoadAddr,
							  u4FwImageFileLength) != WLAN_STATUS_FAILURE)
					u4Status = WLAN_STATUS_FAILURE;
			}

			/* escape to top */
			if (u4Status != WLAN_STATUS_SUCCESS) {
				DBGLOG(INIT, ERROR, "FW download failed!\n");
				eFailReason = RAM_CODE_DOWNLOAD_FAIL;
				break;
			}

#if !CFG_ENABLE_FW_DOWNLOAD_ACK
			/* Send INIT_CMD_ID_QUERY_PENDING_ERROR command and wait for response */
			if (wlanImageQueryStatus(prAdapter) != WLAN_STATUS_SUCCESS) {
				DBGLOG(INIT, ERROR, "FW download failed!\n");
				u4Status = WLAN_STATUS_FAILURE;
				eFailReason = RAM_CODE_DOWNLOAD_FAIL;
				break;
			}
#endif
		} else {
			DBGLOG(INIT, ERROR, "No valid RAM code found!\n");
			u4Status = WLAN_STATUS_FAILURE;
			eFailReason = RAM_CODE_DOWNLOAD_FAIL;
			break;
		}
		DBGLOG(INIT, INFO, "FW download end\n");
#endif

		/* 4. send Wi-Fi Start command */
#if CFG_OVERRIDE_FW_START_ADDRESS
		if (wlanConfigWifiFunc(prAdapter, TRUE,
			prRegInfo->u4StartAddress) == WLAN_STATUS_SUCCESS)
			DBGLOG(INIT, TRACE, "config wifi func\n");
#else
		if (wlanConfigWifiFunc(prAdapter, FALSE, 0) ==
			WLAN_STATUS_SUCCESS)
			DBGLOG(INIT, TRACE, "config wifi func\n");
#endif

		DBGLOG(INIT, TRACE, "Waiting for Ready bit..\n");
		/* 4 <5> check Wi-Fi FW asserts ready bit */
		i = 0;
		while (1U == 1U) {
			HAL_MCR_RD(prAdapter, MCR_WCIR, &u4Value);

			if ((u4Value & WCIR_WLAN_READY) != 0U) {
				DBGLOG(INIT, INFO, "Ready bit asserted\n");
				break;
			} else if (kalIsCardRemoved(prAdapter->prGlueInfo) == TRUE || fgIsBusAccessFailed == TRUE) {
				u4Status = WLAN_STATUS_FAILURE;
				eFailReason = WAIT_FIRMWARE_READY_FAIL;
				break;
			} else if (i >= (UINT_32)CFG_RESPONSE_POLLING_TIMEOUT) {
				UINT_32 u4MailBox0;

				nicGetMailbox(prAdapter, 0, &u4MailBox0);
				DBGLOG(INIT, ERROR, "Waiting for Ready bit timeout, Device to Host MailBox 0x%lx\n",
				       (u4MailBox0 & 0x0000FFFFU));
				u4Status = WLAN_STATUS_FAILURE;
				eFailReason = WAIT_FIRMWARE_READY_FAIL;
				break;
			}

			i++;
			kalMsleep(10);
		}

		if (u4Status == WLAN_STATUS_SUCCESS) {
			/* 1. reset interrupt status */
			HAL_READ_INTR_STATUS(prAdapter, 4, (PUINT_8)&u4WHISR);
			if (HAL_IS_TX_DONE_INTR(u4WHISR) == TRUE)
				HAL_READ_TX_RELEASED_COUNT(prAdapter, au2TxCount);

			/* 2. query & reset TX Resource for normal operation */
			wlanQueryNicResourceInformation(prAdapter);

#if (CFG_SUPPORT_NIC_CAPABILITY == 1)
			/* 3. query for NIC capability */
			wlanQueryNicCapability(prAdapter);
#endif

			/* 4. update basic configuration */
			if (wlanUpdateBasicConfig(prAdapter) ==
				WLAN_STATUS_SUCCESS)
				DBGLOG(INIT, TRACE, "update basic config\n");

			/* 5. Override network address */
			if (wlanUpdateNetworkAddress(prAdapter) ==
				WLAN_STATUS_SUCCESS)
				DBGLOG(INIT, TRACE, "update network address\n");

			/* 6. Apply Network Address */
			if (nicApplyNetworkAddress(prAdapter) ==
				WLAN_STATUS_SUCCESS)
				DBGLOG(INIT, TRACE, "apply network address\n");

			/* 7. indicate disconnection as default status */
			kalIndicateStatusAndComplete(prAdapter->prGlueInfo, WLAN_STATUS_MEDIA_DISCONNECT, NULL, 0);
		}

		RECLAIM_POWER_CONTROL_TO_PM(prAdapter, FALSE);

		if (u4Status != WLAN_STATUS_SUCCESS) {
			DBGLOG(INIT, ERROR, "Wait firmware ready fail\n");
			eFailReason = WAIT_FIRMWARE_READY_FAIL;
			break;
		}

		/* OID timeout timer initialize */
		cnmTimerInitTimer(prAdapter,
			&prAdapter->rOidTimeoutTimer,
			(PFN_MGMT_TIMEOUT_FUNC) wlanReleasePendingOid,
			(ULONG) 0);

		prAdapter->ucOidTimeoutCount = 0;

		prAdapter->fgIsChipNoAck = FALSE;

		/* Return Indicated Rfb list timer */
		cnmTimerInitTimer(prAdapter,
			&prAdapter->rPacketDelaySetupTimer,
			(PFN_MGMT_TIMEOUT_FUNC)
			wlanReturnPacketDelaySetupTimeout,
			(ULONG) 0);

		/* Power state initialization */
		prAdapter->fgWiFiInSleepyState = FALSE;
		prAdapter->rAcpiState = ACPI_STATE_D0;

		/* Online scan option */
		if (prRegInfo->fgDisOnlineScan == 0U)
			prAdapter->fgEnOnlineScan = TRUE;
		else
			prAdapter->fgEnOnlineScan = FALSE;

		/* Beacon lost detection option */
		if (prRegInfo->fgDisBcnLostDetection != 0U)
			prAdapter->fgDisBcnLostDetection = TRUE;

		/* Load compile time constant */
		prAdapter->rWlanInfo.u2BeaconPeriod = CFG_INIT_ADHOC_BEACON_INTERVAL;
		prAdapter->rWlanInfo.u2AtimWindow = CFG_INIT_ADHOC_ATIM_WINDOW;

#if 1				/* set PM parameters */
		prAdapter->fgEnArpFilter = prRegInfo->fgEnArpFilter;
		prAdapter->u4PsCurrentMeasureEn = prRegInfo->u4PsCurrentMeasureEn;

		prAdapter->u4UapsdAcBmp = prRegInfo->u4UapsdAcBmp;

		prAdapter->u4MaxSpLen = prRegInfo->u4MaxSpLen;

		DBGLOG(INIT, TRACE, "[1] fgEnArpFilter:0x%x, u4UapsdAcBmp:0x%x, u4MaxSpLen:0x%x",
				     prAdapter->fgEnArpFilter, prAdapter->u4UapsdAcBmp, prAdapter->u4MaxSpLen);

		prAdapter->fgEnCtiaPowerMode = FALSE;

#endif

		/* MGMT Initialization */
		nicInitMGMT(prAdapter, prRegInfo);

		/* Enable WZC Disassociation */
		prAdapter->rWifiVar.fgSupportWZCDisassociation = TRUE;

		/* Apply Rate Setting */
		if ((ENUM_REGISTRY_FIXED_RATE_T) (prRegInfo->u4FixedRate) < FIXED_RATE_NUM)
			prAdapter->rWifiVar.eRateSetting = (ENUM_REGISTRY_FIXED_RATE_T) (prRegInfo->u4FixedRate);
		else
			prAdapter->rWifiVar.eRateSetting = FIXED_RATE_NONE;

		if (prAdapter->rWifiVar.eRateSetting == FIXED_RATE_NONE) {
			/* Enable Auto (Long/Short) Preamble */
			prAdapter->rWifiVar.ePreambleType = PREAMBLE_TYPE_AUTO;
		} else if ((prAdapter->rWifiVar.eRateSetting >= FIXED_RATE_MCS0_20M_400NS &&
			    prAdapter->rWifiVar.eRateSetting <= FIXED_RATE_MCS7_20M_400NS)
			   || (prAdapter->rWifiVar.eRateSetting >= FIXED_RATE_MCS0_40M_400NS &&
			       prAdapter->rWifiVar.eRateSetting <= FIXED_RATE_MCS32_400NS)) {
			/* Force Short Preamble */
			prAdapter->rWifiVar.ePreambleType = PREAMBLE_TYPE_SHORT;
		} else {
			/* Force Long Preamble */
			prAdapter->rWifiVar.ePreambleType = PREAMBLE_TYPE_LONG;
		}

		/* Disable Hidden SSID Join */
		prAdapter->rWifiVar.fgEnableJoinToHiddenSSID = FALSE;

		/* Enable Short Slot Time */
		prAdapter->rWifiVar.fgIsShortSlotTimeOptionEnable = TRUE;

#if CFG_RX_BA_REORDERING_ENHANCEMENT
		prAdapter->rWifiVar.fgEnableReportIndependentPkt = TRUE;
#endif
		/* configure available PHY type set */
		nicSetAvailablePhyTypeSet(prAdapter);

#if 0				/* Marked for MT6630 */
#if 1				/* set PM parameters */
		{
#if CFG_SUPPORT_PWR_MGT
			prAdapter->u4PowerMode = prRegInfo->u4PowerMode;
#if CFG_ENABLE_WIFI_DIRECT
			prAdapter->rWlanInfo.arPowerSaveMode[NETWORK_TYPE_P2P_INDEX].ucNetTypeIndex =
			    NETWORK_TYPE_P2P_INDEX;
			prAdapter->rWlanInfo.arPowerSaveMode[NETWORK_TYPE_P2P_INDEX].ucPsProfile = ENUM_PSP_FAST_SWITCH;
#endif
#else
			prAdapter->u4PowerMode = ENUM_PSP_CONTINUOUS_ACTIVE;
#endif

			if (nicConfigPowerSaveProfile(prAdapter,
				prAdapter->prAisBssInfo->ucBssIndex,
				prAdapter->u4PowerMode, FALSE) ==
				WLAN_STATUS_SUCCESS)
				DBGLOG(INIT, TRACE, "config PS profile\n");
		}

#endif
#endif

#if CFG_SUPPORT_NVRAM
		/* load manufacture data */
		if (kalIsConfigurationExist(prAdapter->prGlueInfo) == TRUE) {
			if (wlanLoadManufactureData(prAdapter, prRegInfo) ==
				WLAN_STATUS_SUCCESS)
				DBGLOG(INIT, TRACE, "Load menufacture data\n");
		} else
			DBGLOG(INIT, WARN, "%s: load manufacture data fail\n", __func__);
#endif

#if 0
		/* Update Auto rate parameters in FW */
		if (nicRlmArUpdateParms(prAdapter,
				    prRegInfo->u4ArSysParam0,
				    prRegInfo->u4ArSysParam1,
				    prRegInfo->u4ArSysParam2,
				    prRegInfo->u4ArSysParam3)
			== WLAN_STATUS_SUCCESS)
			DBGLOG(INIT, TRACE, "Ar update parms\n");
#endif
	} while (0U != 0U);

	if (u4Status == WLAN_STATUS_SUCCESS) {
		/* restore to hardware default */
		HAL_SET_INTR_STATUS_READ_CLEAR(prAdapter);
		HAL_SET_MAILBOX_READ_CLEAR(prAdapter, FALSE);

		/* Enable interrupt */
		nicEnableInterrupt(prAdapter);

	} else {
		/* release allocated memory */
		switch (eFailReason) {
		case WAIT_FIRMWARE_READY_FAIL:
			nicRxUninitialize(prAdapter);
			nicTxRelease(prAdapter, FALSE);
			/* System Service Uninitialization */
			nicUninitSystemService(prAdapter);
			nicReleaseAdapterMemory(prAdapter);
			if (wlanPollingCpupcr(4U, 5U) == 0U)
				DBGLOG(INIT, TRACE, "Polling cpu pcr\n");
			/*g_IsNeedDoChipReset = 1U;*/
			break;
		case RAM_CODE_DOWNLOAD_FAIL:
			nicRxUninitialize(prAdapter);
			nicTxRelease(prAdapter, FALSE);
			/* System Service Uninitialization */
			nicUninitSystemService(prAdapter);
			nicReleaseAdapterMemory(prAdapter);
			if (wlanPollingCpupcr(4U, 5U) == 0U)
				DBGLOG(INIT, TRACE, "Polling cpu pcr\n");
			/*g_IsNeedDoChipReset = 1U;*/
			break;
		case INIT_ADAPTER_FAIL:
			nicReleaseAdapterMemory(prAdapter);
			break;
		case DRIVER_OWN_FAIL:
			nicReleaseAdapterMemory(prAdapter);
			// ATC6129 Modified
			// g_IsNeedDoChipReset = 1U;
			break;
		case ALLOC_ADAPTER_MEM_FAIL:
			break;
		default:
			break;
		}
	}

	return u4Status;
}				/* wlanAdapterStart */

#if CFG_ENABLE_FW_DOWNLOAD
static WLAN_STATUS
wlanImageFullDownload(IN P_ADAPTER_T prAdapter,
		      IN PVOID pvFwImageMapFile,
		      IN UINT_32 u4FwLoadAddr,
		      IN UINT_32 u4FwImageFileLength)
{
	UINT_32 u4ImgSecSize;
	UINT_32 j;
	WLAN_STATUS u4Status = WLAN_STATUS_SUCCESS;

	do {
		if (wlanImageSectionConfig(prAdapter,
					   u4FwLoadAddr,
					   u4FwImageFileLength,
					   TRUE,
					   TRUE,
					   0) != WLAN_STATUS_SUCCESS) {
			DBGLOG(INIT, ERROR, "Firmware download configuration failed!\n");

			u4Status = WLAN_STATUS_FAILURE;
			break;
		}

		for (j = 0; j < u4FwImageFileLength; j += CMD_PKT_SIZE_FOR_IMAGE) {
			if (j + CMD_PKT_SIZE_FOR_IMAGE < u4FwImageFileLength)
				u4ImgSecSize = CMD_PKT_SIZE_FOR_IMAGE;
			else
				u4ImgSecSize = u4FwImageFileLength - j;

			if (wlanImageSectionDownload(prAdapter,
						     u4ImgSecSize,
						     (PUINT_8) pvFwImageMapFile + j) !=
						     WLAN_STATUS_SUCCESS) {
				DBGLOG(INIT, ERROR, "Firmware scatter download failed!\n");

				u4Status = WLAN_STATUS_FAILURE;
				break;
			}
		}

	} while (0U != 0U);

	return u4Status;
}

#if CFG_ENABLE_FW_DIVIDED_DOWNLOAD
static WLAN_STATUS
wlanImageDividDownloadByEntry(IN P_ADAPTER_T prAdapter,
			      IN PVOID pvFwImageMapFile,
			      IN P_FIRMWARE_DIVIDED_DOWNLOAD_T prFwHead,
			      IN UINT_32 index)
{
	UINT_32 u4ImgSecSize;
	UINT_32 j;
	WLAN_STATUS u4Status = WLAN_STATUS_SUCCESS;

	do {
		if (wlanImageSectionConfig(prAdapter,
					   prFwHead->arSection[index].u4DestAddr,
					   prFwHead->arSection[index].u4Length,
					   index == 0U ? TRUE : FALSE,
#if defined(MT6631)
					   prFwHead->arSection[index].ucEnc ? TRUE : FALSE,
					   prFwHead->arSection[index].ucKIdx
#else
					   TRUE,
					   0
#endif
			) != WLAN_STATUS_SUCCESS) {

			DBGLOG(INIT, ERROR, "Firmware download configuration failed! idx %d, dst %p, len %d\n",
			       index,
			       prFwHead->arSection[index].u4DestAddr,
			       prFwHead->arSection[index].u4Length);

			u4Status = WLAN_STATUS_FAILURE;
			break;
		}

		for (j = 0U; j < prFwHead->arSection[index].u4Length;
			j += CMD_PKT_SIZE_FOR_IMAGE) {
			if (j + CMD_PKT_SIZE_FOR_IMAGE < prFwHead->arSection[index].u4Length)
				u4ImgSecSize = CMD_PKT_SIZE_FOR_IMAGE;
			else
				u4ImgSecSize = prFwHead->arSection[index].u4Length - j;

			if (wlanImageSectionDownload(prAdapter,
						     u4ImgSecSize,
						     (PUINT_8)pvFwImageMapFile +
						     prFwHead->arSection[index].u4Offset + j) !=
						     WLAN_STATUS_SUCCESS) {
				DBGLOG(INIT, ERROR,
				       "Firmware scatter download failed! idx %d, dst %p, len %d, offset %d\n",
				       index,
				       prFwHead->arSection[index].u4DestAddr,
				       prFwHead->arSection[index].u4Length,
				       j);

				u4Status = WLAN_STATUS_FAILURE;
				break;
			}
		}

	} while (0U != 0U);

	return u4Status;
}

static WLAN_STATUS
wlanImageDividDownload(IN P_ADAPTER_T prAdapter, IN PVOID pvFwImageMapFile)
{
	UINT_32 i;
	P_FIRMWARE_DIVIDED_DOWNLOAD_T prFwHead;
	WLAN_STATUS u4Status = WLAN_STATUS_SUCCESS;

	prFwHead = (P_FIRMWARE_DIVIDED_DOWNLOAD_T) pvFwImageMapFile;

	/* engage divided firmware downloading */
	for (i = 0U; i < prFwHead->u4NumOfEntries; i++) {
		if (i < 2U) { /* DLM + ILM */
			u4Status = wlanImageDividDownloadByEntry(prAdapter,
								 pvFwImageMapFile, prFwHead, i);
			if (u4Status == WLAN_STATUS_FAILURE)
				break;
		}
#if defined(MT6631)
		else { /* IEMI + DEMI */
#define WIFI_EMI_MEM_SIZE	(512 * 1024)

			if (gConEmiPhyBase) {
				UINT_8 __iomem *prWifiEmiBaseAddr;

				/* WIFI using TOP 512KB */
				if ((prFwHead->arSection[i].u4DestAddr
					& 0xfffffU) +
				     prFwHead->arSection[i].u4Length <= WIFI_EMI_MEM_SIZE) {
#if 0
					emi_mpu_set_region_protection(gConEmiPhyBase,
								      gConEmiPhyBase + WIFI_EMI_MEM_SIZE - 1,
								      18,
								      SET_ACCESS_PERMISSON(NO_PROTECTION, NO_PROTECTION,
								      NO_PROTECTION, NO_PROTECTION, NO_PROTECTION,
								      NO_PROTECTION, NO_PROTECTION, NO_PROTECTION));
#endif
					prWifiEmiBaseAddr = ioremap_nocache(gConEmiPhyBase, WIFI_EMI_MEM_SIZE);
					DBGLOG(INIT, INFO,
					       "ConsysEmiPhyBase %p, WifiEmiBaseAddr %p, idx %d, dst %p, len %d\n",
					       gConEmiPhyBase,
					       prWifiEmiBaseAddr,
					       i,
					       prFwHead->arSection[i].u4DestAddr & 0xfffff,
					       prFwHead->arSection[i].u4Length);

					/* TODO, EMI download only if reboot */
					kalMemCopy(prWifiEmiBaseAddr +
						(prFwHead->arSection[i].
						u4DestAddr
						& 0xfffffU),
						   (PUINT_8)pvFwImageMapFile + prFwHead->arSection[i].u4Offset,
						   prFwHead->arSection[i].u4Length);
#if 0
					emi_mpu_set_region_protection(gConEmiPhyBase,
								      gConEmiPhyBase + WIFI_EMI_MEM_SIZE - 1,
								      18,
								      SET_ACCESS_PERMISSON(FORBIDDEN, FORBIDDEN,
								      FORBIDDEN, FORBIDDEN, FORBIDDEN,
								      NO_PROTECTION, FORBIDDEN, FORBIDDEN));
#endif

					iounmap(prWifiEmiBaseAddr);
				} else {
					DBGLOG(INIT, ERROR, "FW section length out of bound! idx %d, dst %p, len %d\n",
						i,
						prFwHead->arSection[i].
						u4DestAddr
						& 0xfffffU,
						prFwHead->arSection[i].
						u4Length);
					u4Status = WLAN_STATUS_FAILURE;
					break;
				}
			} else {
				DBGLOG(INIT, ERROR, "Consys EMI phy address is invalid\n");
				u4Status = WLAN_STATUS_FAILURE;
				break;
			}
		}
#endif
	}
	return u4Status;
}
#endif
#endif

WLAN_STATUS wlanPowerOffInt(IN P_ADAPTER_T prAdapter)
{
	UINT_32 u4Value = 0;
	UINT_32 u4Feedback = 0;
	UINT_32 u4Loop = 0;


	if (prAdapter == NULL)
		return WLAN_STATUS_SUCCESS;

	DBGLOG(INIT, INFO, "Using INT for Power OFF\n");
	nicPutMailbox(prAdapter, CFG_MCU_POWER_OFF_MAILBOX_INDEX,
		CFG_MCU_POWER_OFF_MAGIC_CODE);

	HAL_MCR_WR(prAdapter, MCR_WSICR, BIT(CFG_MCU_POWER_OFF_SOFTINT_BIT));

	for (u4Loop = 0; u4Loop < CFG_MCU_POWER_OFF_POLLING_CNT; u4Loop++) {
		nicGetMailbox(prAdapter, CFG_MCU_POWER_OFF_MAILBOX_INDEX, &u4Feedback);
		DBGLOG(INIT, INFO, "INT FeedBack: 0x%x\n", u4Feedback);
		HAL_MCR_RD(prAdapter, MCR_WCIR, &u4Value);

		if ((u4Value & WCIR_WLAN_READY) == 0U) {
			/* Cleanup MailBox */
			nicPutMailbox(prAdapter, CFG_MCU_POWER_OFF_MAILBOX_INDEX, 0x0);
			DBGLOG(INIT, INFO, "Power OFF by INT successfully\n");
			return WLAN_STATUS_SUCCESS;
		}
		u4Feedback = 0;
		u4Value = 0;
		kalMsleep(1);
	}
	/* Cleanup MailBox */
	nicPutMailbox(prAdapter, CFG_MCU_POWER_OFF_MAILBOX_INDEX, 0x0);

	DBGLOG(INIT, INFO, "MCR_WCIR: 0x%x\n", u4Value);

	return WLAN_STATUS_FAILURE;
}

/*----------------------------------------------------------------------------*/
/*!
* \brief Uninitialize the adapter
*
* \param prAdapter      Pointer of Adapter Data Structure
*
* \retval WLAN_STATUS_SUCCESS: Success
* \retval WLAN_STATUS_FAILURE: Failed
*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS wlanAdapterStop(IN P_ADAPTER_T prAdapter)
{
	UINT_32 i = 0;
	UINT_32 u4Value = 0;
	UINT_32 u4CurrTick = 0;
	WLAN_STATUS u4Status = WLAN_STATUS_SUCCESS;

	ASSERT(prAdapter);

	/* Release all CMD/MGMT/SEC frame in command queue */
	kalClearCommandQueue(prAdapter->prGlueInfo);

#if CFG_SUPPORT_MULTITHREAD

	/* Flush all items in queues for multi-thread */
	wlanClearTxCommandQueue(prAdapter);

	wlanClearTxCommandDoneQueue(prAdapter);

	wlanClearDataQueue(prAdapter);

	wlanClearRxToOsQueue(prAdapter);

#endif

	if (prAdapter->rAcpiState == ACPI_STATE_D0 &&
	    wlanIsChipNoAck(prAdapter) == FALSE &&
	    kalIsCardRemoved(prAdapter->prGlueInfo) == FALSE) {

		/* 0. Disable interrupt, this can be done without Driver own */
		nicDisableInterrupt(prAdapter);

		ACQUIRE_POWER_CONTROL_FROM_PM(prAdapter);

		/* 1. Set CMD to FW to tell WIFI to stop (enter power off state) */
		if (prAdapter->fgIsFwOwn == FALSE && wlanSendNicPowerCtrlCmd(prAdapter, 1) == WLAN_STATUS_SUCCESS) {
			/* 2. Clear pending interrupt */
			i = 0U;
			while (i < CFG_IST_LOOP_COUNT && nicProcessIST(prAdapter) != WLAN_STATUS_NOT_INDICATING) {
				i++;
			};

			/* 3. Wait til RDY bit has been cleaerd */
			u4CurrTick = kalGetTimeTick();
			while (1) {
				HAL_MCR_RD(prAdapter, MCR_WCIR, &u4Value);

				if ((u4Value & WCIR_WLAN_READY) == 0U) {
					break;
				} else if (kalIsCardRemoved(prAdapter->prGlueInfo) || fgIsBusAccessFailed
						|| CHECK_FOR_TIMEOUT(kalGetTimeTick(), u4CurrTick, WLAN_WAIT_READY_BIT_TIMEOUT)) {
					DBGLOG(INIT, WARN, "%s: Failure to get RDY bit cleared! "
						"CardRemoved[%u] BusFailed[%u] Timeout[%u]",
						__func__, kalIsCardRemoved(prAdapter->prGlueInfo),
						fgIsBusAccessFailed, WLAN_WAIT_READY_BIT_TIMEOUT);
					DBGLOG(NIC, WARN, "wlanAdapterStop.u4Value=0x%x\n",u4Value);
					HAL_MCR_WR(prAdapter, MCR_WCIR, 0x116630);
					HAL_MCR_RD(prAdapter, MCR_WCIR, &u4Value);
					DBGLOG(NIC, WARN, "wlanAdapterStop.u4Value=0x%x\n",u4Value);
					break;
				}
			}
		}

		/* 4. Set Onwership to F/W */
		nicpmSetFWOwn(prAdapter, FALSE);

#if CFG_FORCE_RESET_UNDER_BUS_ERROR
		if (HAL_TEST_FLAG(prAdapter, ADAPTER_FLAG_HW_ERR) != 0U) {
			/* force acquire firmware own */
			if (kalDevRegWrite(prAdapter->prGlueInfo,
				MCR_WHLPCR, WHLPCR_FW_OWN_REQ_CLR) == TRUE)
				DBGLOG(INIT, TRACE, "write FW Own\n");

			/* delay for 10ms */
			kalMdelay(10);

			/* force firmware reset via software interrupt */
			if (kalDevRegWrite(prAdapter->prGlueInfo,
				MCR_WSICR, WSICR_H2D_SW_INT_SET) == TRUE)
				DBGLOG(INIT, TRACE, "write SW INT\n");

			/* force release firmware own */
			if (kalDevRegWrite(prAdapter->prGlueInfo,
				MCR_WHLPCR, WHLPCR_FW_OWN_REQ_SET) == TRUE)
				DBGLOG(INIT, TRACE, "write FW Own\n");
		}
#endif

		RECLAIM_POWER_CONTROL_TO_PM(prAdapter, FALSE);
	}

	nicRxUninitialize(prAdapter);

	nicTxRelease(prAdapter, FALSE);

	/* MGMT - unitialization */
	nicUninitMGMT(prAdapter);

	/* System Service Uninitialization */
	nicUninitSystemService(prAdapter);

	nicReleaseAdapterMemory(prAdapter);

#if defined(_HIF_SPI)
	/* Note: restore the SPI Mode Select from 32 bit to default */
	nicRestoreSpiDefMode(prAdapter);
#endif

	return u4Status;
}				/* wlanAdapterStop */

/*----------------------------------------------------------------------------*/
/*!
* \brief This function is called by ISR (interrupt).
*
* \param prAdapter      Pointer of Adapter Data Structure
*
* \retval TRUE: NIC's interrupt
* \retval FALSE: Not NIC's interrupt
*/
/*----------------------------------------------------------------------------*/
BOOL wlanISR(IN P_ADAPTER_T prAdapter, IN BOOLEAN fgGlobalIntrCtrl)
{
	ASSERT(prAdapter);

	if (fgGlobalIntrCtrl == TRUE) {
		nicDisableInterrupt(prAdapter);

		/* wlanIST(prAdapter); */
	}

	return TRUE;
}

/*----------------------------------------------------------------------------*/
/*!
* \brief This function is called by IST (task_let).
*
* \param prAdapter      Pointer of Adapter Data Structure
*
* \return (none)
*/
/*----------------------------------------------------------------------------*/
VOID wlanIST(IN P_ADAPTER_T prAdapter)
{
	ASSERT(prAdapter);

	ACQUIRE_POWER_CONTROL_FROM_PM(prAdapter);

	nicProcessIST(prAdapter);

	if (KAL_WAKE_LOCK_ACTIVE(prAdapter, &prAdapter->prGlueInfo->rIntrWakeLock))
		KAL_WAKE_UNLOCK(prAdapter, &prAdapter->prGlueInfo->rIntrWakeLock);

#if !defined(MT6631)
	nicEnableInterrupt(prAdapter);
#endif

	RECLAIM_POWER_CONTROL_TO_PM(prAdapter, FALSE);
}

/*----------------------------------------------------------------------------*/
/*!
* \brief This function will check command queue to find out if any could be dequeued
*        and/or send to HIF to MT6620
*
* \param prAdapter      Pointer of Adapter Data Structure
* \param prCmdQue       Pointer of Command Queue (in Glue Layer)
*
* \retval WLAN_STATUS_SUCCESS
*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS wlanProcessCommandQueue(IN P_ADAPTER_T prAdapter, IN P_QUE_T prCmdQue)
{
	WLAN_STATUS rStatus;
	QUE_T rTempCmdQue, rMergeCmdQue, rStandInCmdQue;
	P_QUE_T prTempCmdQue, prMergeCmdQue, prStandInCmdQue;
	P_QUE_ENTRY_T prQueueEntry;
	P_CMD_INFO_T prCmdInfo;
	P_MSDU_INFO_T prMsduInfo;
	ENUM_FRAME_ACTION_T eFrameAction = FRAME_ACTION_DROP_PKT;

	KAL_SPIN_LOCK_DECLARATION();

	ASSERT(prAdapter);
	ASSERT(prCmdQue);

	prTempCmdQue = &rTempCmdQue;
	prMergeCmdQue = &rMergeCmdQue;
	prStandInCmdQue = &rStandInCmdQue;

	QUEUE_INITIALIZE(prTempCmdQue);
	QUEUE_INITIALIZE(prMergeCmdQue);
	QUEUE_INITIALIZE(prStandInCmdQue);

	/* 4 <1> Move whole list of CMD_INFO to temp queue */
	KAL_ACQUIRE_SPIN_LOCK(prAdapter, SPIN_LOCK_CMD_QUE);
	QUEUE_MOVE_ALL(prTempCmdQue, prCmdQue);
	KAL_RELEASE_SPIN_LOCK(prAdapter, SPIN_LOCK_CMD_QUE);

	/* 4 <2> Dequeue from head and check it is able to be sent */
	QUEUE_REMOVE_HEAD(prTempCmdQue, prQueueEntry, P_QUE_ENTRY_T);
	while (prQueueEntry) {
		prCmdInfo = (P_CMD_INFO_T) prQueueEntry;

		switch (prCmdInfo->eCmdType) {
		case COMMAND_TYPE_NETWORK_IOCTL:
			if (prCmdInfo->ucCID == (UINT_8)CMD_ID_ADD_REMOVE_KEY) {
				P_BSS_INFO_T prBssInfo = prAdapter->aprBssInfo[prCmdInfo->ucBssIndex];
				P_WIFI_CMD_T prWifiCmd = (P_WIFI_CMD_T) (prCmdInfo->pucInfoBuffer);
				P_CMD_802_11_KEY prKey = (P_CMD_802_11_KEY)prWifiCmd->aucBuffer;

				if ((prBssInfo->eNetworkType == NETWORK_TYPE_AIS ||
					(prBssInfo->eNetworkType == NETWORK_TYPE_P2P &&
					prCmdInfo->ucBssIndex == P2P_DEV_BSS_INDEX)) &&
					prKey->ucAddRemove != 0U &&
					prKey->ucTxKey != 0U &&
					(prKey->ucAlgorithmId == CIPHER_SUITE_TKIP ||
					prKey->ucAlgorithmId == CIPHER_SUITE_CCMP)) {/* add key */
					switch (prBssInfo->ucKeyCmdAction) {
					case SEC_DROP_KEY_COMMAND:
						eFrameAction = FRAME_ACTION_DROP_PKT;
						break;
					case SEC_QUEUE_KEY_COMMAND:
						eFrameAction = FRAME_ACTION_QUEUE_PKT;
						break;
					case SEC_TX_KEY_COMMAND:
						eFrameAction = FRAME_ACTION_TX_PKT;
						break;
					default:
						break;
					}
					DBGLOG(TX, INFO, "Add Key Cmd Action %d\n", eFrameAction);
					break;
				}
			}
		case COMMAND_TYPE_GENERAL_IOCTL:
			/* command packet will be always sent */
			eFrameAction = FRAME_ACTION_TX_PKT;
			break;

		case COMMAND_TYPE_SECURITY_FRAME:
			/* inquire with QM */
			eFrameAction = qmGetFrameAction(prAdapter, prCmdInfo->ucBssIndex,
							prCmdInfo->ucStaRecIndex, NULL, FRAME_TYPE_802_1X,
							prCmdInfo->u2InfoBufLen);
			break;

		case COMMAND_TYPE_MANAGEMENT_FRAME:
			/* inquire with QM */
			prMsduInfo = prCmdInfo->prMsduInfo;

			eFrameAction = qmGetFrameAction(prAdapter, prMsduInfo->ucBssIndex,
							prMsduInfo->ucStaRecIndex, prMsduInfo, FRAME_TYPE_MMPDU,
							prMsduInfo->u2FrameLength);
			break;

		default:
			ASSERT(NULL);
			break;
		}

		/* 4 <3> handling upon dequeue result */
		if (eFrameAction == FRAME_ACTION_DROP_PKT) {
			DBGLOG(INIT, INFO, "DROP CMD TYPE[%u] ID[0x%02X] SEQ[%u]\n",
					    prCmdInfo->eCmdType, prCmdInfo->ucCID, prCmdInfo->ucCmdSeqNum);
			wlanReleaseCommand(prAdapter, prCmdInfo, TX_RESULT_DROPPED_IN_DRIVER);
			cmdBufFreeCmdInfo(prAdapter, prCmdInfo);
		} else if (eFrameAction == FRAME_ACTION_QUEUE_PKT) {
			DBGLOG(INIT, TRACE, "QUE back CMD TYPE[%u] ID[0x%02X] SEQ[%u]\n",
					     prCmdInfo->eCmdType, prCmdInfo->ucCID, prCmdInfo->ucCmdSeqNum);

			QUEUE_INSERT_TAIL(prMergeCmdQue, prQueueEntry);
		} else if (eFrameAction == FRAME_ACTION_TX_PKT) {
			/* 4 <4> Send the command */
#if CFG_SUPPORT_MULTITHREAD
			rStatus = wlanSendCommandMthread(prAdapter, prCmdInfo);

			if (rStatus == WLAN_STATUS_RESOURCES) {
				/* no more TC4 resource for further transmission */
				DBGLOG(INIT, EVENT, "NO Res CMD TYPE[%u] ID[0x%02X] SEQ[%u]\n",
						    prCmdInfo->eCmdType, prCmdInfo->ucCID, prCmdInfo->ucCmdSeqNum);

				QUEUE_INSERT_TAIL(prMergeCmdQue, prQueueEntry);
				break;
			} else if (rStatus != WLAN_STATUS_SUCCESS && rStatus != WLAN_STATUS_PENDING) {
				P_CMD_INFO_T prCmdInfo = (P_CMD_INFO_T) prQueueEntry;

				if (prCmdInfo->fgIsOid == TRUE)
					kalOidComplete(prAdapter->prGlueInfo, prCmdInfo->fgSetQuery,
						       prCmdInfo->u4SetInfoLen, rStatus);
				cmdBufFreeCmdInfo(prAdapter, prCmdInfo);
				DBGLOG(TX, WARN, "TX CMD Status[%u], TYPE[%u] ID[0x%02X] SEQ[%u]\n", rStatus,
						    prCmdInfo->eCmdType, prCmdInfo->ucCID, prCmdInfo->ucCmdSeqNum);
			}
#else
			rStatus = wlanSendCommand(prAdapter, prCmdInfo);

			if (rStatus == WLAN_STATUS_RESOURCES) {
				/* no more TC4 resource for further transmission */

				DBGLOG(TX, WARN, "NO Resource for CMD TYPE[%u] ID[0x%02X] SEQ[%u]\n",
					prCmdInfo->eCmdType, prCmdInfo->ucCID, prCmdInfo->ucCmdSeqNum);

				QUEUE_INSERT_TAIL(prMergeCmdQue, prQueueEntry);
				break;
			} else if (rStatus == WLAN_STATUS_PENDING) {
				/* command packet which needs further handling upon response */
				KAL_ACQUIRE_SPIN_LOCK(prAdapter, SPIN_LOCK_CMD_PENDING);
				QUEUE_INSERT_TAIL(&(prAdapter->rPendingCmdQueue), prQueueEntry);
				KAL_RELEASE_SPIN_LOCK(prAdapter, SPIN_LOCK_CMD_PENDING);
			} else {
				P_CMD_INFO_T prCmdInfo = (P_CMD_INFO_T) prQueueEntry;

				if (rStatus == WLAN_STATUS_SUCCESS) {
					if (prCmdInfo->pfCmdDoneHandler)
						prCmdInfo->pfCmdDoneHandler(prAdapter, prCmdInfo,
									    prCmdInfo->pucInfoBuffer);
				} else {
					if (prCmdInfo->fgIsOid)
						kalOidComplete(prAdapter->prGlueInfo,
							       prCmdInfo->fgSetQuery, prCmdInfo->u4SetInfoLen, rStatus);
				}

				cmdBufFreeCmdInfo(prAdapter, prCmdInfo);
			}
#endif
		} else {
			ASSERT(NULL);
		}

		QUEUE_REMOVE_HEAD(prTempCmdQue, prQueueEntry, P_QUE_ENTRY_T);
	}

	/* 4 <3> Merge back to original queue */
	/* 4 <3.1> Merge prMergeCmdQue & prTempCmdQue */
	QUEUE_CONCATENATE_QUEUES(prMergeCmdQue, prTempCmdQue);

	/* 4 <3.2> Move prCmdQue to prStandInQue, due to prCmdQue might differ due to incoming 802.1X frames */
	KAL_ACQUIRE_SPIN_LOCK(prAdapter, SPIN_LOCK_CMD_QUE);
	QUEUE_MOVE_ALL(prStandInCmdQue, prCmdQue);

	/* 4 <3.3> concatenate prStandInQue to prMergeCmdQue */
	QUEUE_CONCATENATE_QUEUES(prMergeCmdQue, prStandInCmdQue);

	/* 4 <3.4> then move prMergeCmdQue to prCmdQue */
	QUEUE_MOVE_ALL(prCmdQue, prMergeCmdQue);
	KAL_RELEASE_SPIN_LOCK(prAdapter, SPIN_LOCK_CMD_QUE);

#if CFG_SUPPORT_MULTITHREAD
	kalSetTxCmdEvent2Hif(prAdapter->prGlueInfo);
#endif

	return WLAN_STATUS_SUCCESS;
}				/* end of wlanProcessCommandQueue() */

/*----------------------------------------------------------------------------*/
/*!
* \brief This function will take CMD_INFO_T which carry some informantion of
*        incoming OID and notify the NIC_TX to send CMD.
*
* \param prAdapter      Pointer of Adapter Data Structure
* \param prCmdInfo      Pointer of P_CMD_INFO_T
*
* \retval WLAN_STATUS_SUCCESS   : CMD was written to HIF and be freed(CMD Done) immediately.
* \retval WLAN_STATUS_RESOURCE  : No resource for current command, need to wait for previous
*                                 frame finishing their transmission.
* \retval WLAN_STATUS_FAILURE   : Get failure while access HIF or been rejected.
*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS wlanSendCommand(IN P_ADAPTER_T prAdapter, IN P_CMD_INFO_T prCmdInfo)
{
	P_TX_CTRL_T prTxCtrl;
	UINT_8 ucTC;		/* "Traffic Class" SW(Driver) resource classification */
	WLAN_STATUS rStatus = WLAN_STATUS_SUCCESS;

	ASSERT(prAdapter);
	ASSERT(prCmdInfo);
	prTxCtrl = &prAdapter->rTxCtrl;

	do {
		/* <0> card removal check */
		if (kalIsCardRemoved(prAdapter->prGlueInfo) == TRUE || fgIsBusAccessFailed == TRUE) {
			rStatus = WLAN_STATUS_FAILURE;
			break;
		}
		/* <1> Normal case of sending CMD Packet */
		if (prCmdInfo->fgDriverDomainMCR == FALSE) {
			/* <1.1> Assign Traffic Class(TC) */
			ucTC = nicTxGetCmdResourceType(prCmdInfo);

			/* <1.2> Check if pending packet or resource was exhausted */
			rStatus = nicTxAcquireResource(prAdapter, ucTC, nicTxGetCmdPageCount(prCmdInfo));
			if (rStatus == WLAN_STATUS_RESOURCES) {
				DBGLOG(INIT, INFO, "NO Resource:%d\n", ucTC);
				break;
			}
			/* <1.3> Forward CMD_INFO_T to NIC Layer */
			rStatus = nicTxCmd(prAdapter, prCmdInfo, ucTC);

			/* <1.4> Set Pending in response to Query Command/Need Response */
			if (rStatus == WLAN_STATUS_SUCCESS) {
				if ((prCmdInfo->fgSetQuery == FALSE) ||
					(prCmdInfo->fgNeedResp == TRUE))
					rStatus = WLAN_STATUS_PENDING;
			}
		}
		/* <2> Special case for access Driver Domain MCR */
		else {
			P_CMD_ACCESS_REG prCmdAccessReg;

			prCmdAccessReg = (P_CMD_ACCESS_REG) (prCmdInfo->pucInfoBuffer + CMD_HDR_SIZE);

			if (prCmdInfo->fgSetQuery == TRUE) {
				/* address is in DWORD unit */
				HAL_MCR_WR(prAdapter,
					(UINT_32)(prCmdAccessReg->u4Address &
					BITS(2U, 31U)),
					prCmdAccessReg->u4Data);
			} else {
				P_CMD_ACCESS_REG prEventAccessReg;
				UINT_32 u4Address;

				u4Address = prCmdAccessReg->u4Address;
				prEventAccessReg = (P_CMD_ACCESS_REG) prCmdInfo->pucInfoBuffer;
				prEventAccessReg->u4Address = u4Address;
				/* address is in DWORD unit */
				HAL_MCR_RD(prAdapter,
					prEventAccessReg->u4Address &
					(UINT_32)BITS(2U, 31U),
					&prEventAccessReg->u4Data);
			}
		}

	} while (0U != 0U);

	return rStatus;
}				/* end of wlanSendCommand() */

#if CFG_SUPPORT_MULTITHREAD

/*----------------------------------------------------------------------------*/
/*!
* \brief This function will take CMD_INFO_T which carry some information of
*        incoming OID and notify the NIC_TX to send CMD.
*
* \param prAdapter      Pointer of Adapter Data Structure
* \param prCmdInfo      Pointer of P_CMD_INFO_T
*
* \retval WLAN_STATUS_SUCCESS   : CMD was written to HIF and be freed(CMD Done) immediately.
* \retval WLAN_STATUS_RESOURCE  : No resource for current command, need to wait for previous
*                                 frame finishing their transmission.
* \retval WLAN_STATUS_FAILURE   : Get failure while access HIF or been rejected.
*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS wlanSendCommandMthread(IN P_ADAPTER_T prAdapter, IN P_CMD_INFO_T prCmdInfo)
{
	P_TX_CTRL_T prTxCtrl;
	UINT_8 ucTC;		/* "Traffic Class" SW(Driver) resource classification */
	WLAN_STATUS rStatus = WLAN_STATUS_SUCCESS;

	QUE_T rTempCmdQue;
	P_QUE_T prTempCmdQue;

#if CFG_DBG_MGT_BUF
	struct MEM_TRACK *prMemTrack = NULL;
#endif

	KAL_SPIN_LOCK_DECLARATION();

	ASSERT(prAdapter);
	ASSERT(prCmdInfo);
	prTxCtrl = &prAdapter->rTxCtrl;

#if CFG_DBG_MGT_BUF
	if (prCmdInfo->pucInfoBuffer && !IS_FROM_BUF(prAdapter, prCmdInfo->pucInfoBuffer))
		prMemTrack = (struct MEM_TRACK *)((PUINT_8)prCmdInfo->pucInfoBuffer - sizeof(struct MEM_TRACK));
#endif

	prTempCmdQue = &rTempCmdQue;
	QUEUE_INITIALIZE(prTempCmdQue);

	do {
		/* <0> card removal check */
		if (kalIsCardRemoved(prAdapter->prGlueInfo) == TRUE || fgIsBusAccessFailed == TRUE) {
			rStatus = WLAN_STATUS_FAILURE;
			break;
		}
		/* <1> Normal case of sending CMD Packet */
		if (prCmdInfo->fgDriverDomainMCR != TRUE) {
			/* <1.1> Assign Traffic Class(TC) */
			ucTC = nicTxGetCmdResourceType(prCmdInfo);

			/* <1.2> Check if pending packet or resource was exhausted */
			rStatus = nicTxAcquireResource(prAdapter, ucTC, nicTxGetCmdPageCount(prCmdInfo));
			if (rStatus == WLAN_STATUS_RESOURCES) {
#if 0
				DBGLOG(INIT, WARN, "%s: NO Resource for CMD TYPE[%u] ID[0x%02X] SEQ[%u] TC[%u]\n",
						    __func__,
						    prCmdInfo->eCmdType,
						    prCmdInfo->ucCID, prCmdInfo->ucCmdSeqNum, ucTC);
#endif
				break;
			}

			/* Process to pending command queue firest */
			if ((prCmdInfo->fgSetQuery != TRUE) ||
				(prCmdInfo->fgNeedResp == TRUE)) {
				/* command packet which needs further handling upon response */
				/*
				 * KAL_ACQUIRE_SPIN_LOCK(prAdapter, SPIN_LOCK_CMD_PENDING);
				 * QUEUE_INSERT_TAIL(&(prAdapter->rPendingCmdQueue), (P_QUE_ENTRY_T)prCmdInfo);
				 * KAL_RELEASE_SPIN_LOCK(prAdapter, SPIN_LOCK_CMD_PENDING);
				 */
			}

#if CFG_DBG_MGT_BUF
			if (prMemTrack) {
				prMemTrack->u2CmdIdAndWhere &= 0x00FF;
				prMemTrack->u2CmdIdAndWhere |= 0x0100;
			}
#endif

			QUEUE_INSERT_TAIL(prTempCmdQue, (P_QUE_ENTRY_T) prCmdInfo);

			/* <1.4> Set Pending in response to Query Command/Need Response */
			if (rStatus == WLAN_STATUS_SUCCESS) {
				if ((prCmdInfo->fgSetQuery == FALSE) ||
				    (prCmdInfo->fgNeedResp == TRUE) ||
				    (prCmdInfo->eCmdType ==
				    COMMAND_TYPE_SECURITY_FRAME)) {
					rStatus = WLAN_STATUS_PENDING;
				}
			}
		}
		/* <2> Special case for access Driver Domain MCR */
		else {
#if CFG_DBG_MGT_BUF
			if (prMemTrack) {
				prMemTrack->u2CmdIdAndWhere &= 0x00FF;
				prMemTrack->u2CmdIdAndWhere |= 0x0100;
			}
#endif
			QUEUE_INSERT_TAIL(prTempCmdQue, (P_QUE_ENTRY_T) prCmdInfo);
			rStatus = WLAN_STATUS_PENDING;
		}
	} while (0 != 0);

	KAL_ACQUIRE_SPIN_LOCK(prAdapter, SPIN_LOCK_TX_CMD_QUE);
	QUEUE_CONCATENATE_QUEUES(&(prAdapter->rTxCmdQueue), prTempCmdQue);
	KAL_RELEASE_SPIN_LOCK(prAdapter, SPIN_LOCK_TX_CMD_QUE);

	return rStatus;
}				/* end of wlanSendCommandMthread() */

WLAN_STATUS wlanTxCmdMthread(IN P_ADAPTER_T prAdapter)
{
	QUE_T rTempCmdQue;
	P_QUE_T prTempCmdQue;
	QUE_T rTempCmdDoneQue;
	P_QUE_T prTempCmdDoneQue;
	P_QUE_ENTRY_T prQueueEntry;
	P_CMD_INFO_T prCmdInfo;
	P_CMD_ACCESS_REG prCmdAccessReg;
	P_CMD_ACCESS_REG prEventAccessReg;
	UINT_32 u4Address;
	UINT_32 u4TxDoneQueueSize;
#if CFG_DBG_MGT_BUF
	struct MEM_TRACK *prMemTrack = NULL;
#endif

	KAL_SPIN_LOCK_DECLARATION();

	ASSERT(prAdapter);

	prTempCmdQue = &rTempCmdQue;
	QUEUE_INITIALIZE(prTempCmdQue);

	prTempCmdDoneQue = &rTempCmdDoneQue;
	QUEUE_INITIALIZE(prTempCmdDoneQue);

	KAL_ACQUIRE_MUTEX(prAdapter, MUTEX_TX_CMD_CLEAR);

	/* TX Command Queue */
	/* 4 <1> Move whole list of CMD_INFO to temp queue */
	KAL_ACQUIRE_SPIN_LOCK(prAdapter, SPIN_LOCK_TX_CMD_QUE);
	QUEUE_MOVE_ALL(prTempCmdQue, &prAdapter->rTxCmdQueue);
	KAL_RELEASE_SPIN_LOCK(prAdapter, SPIN_LOCK_TX_CMD_QUE);

	/* 4 <2> Dequeue from head and check it is able to be sent */
	QUEUE_REMOVE_HEAD(prTempCmdQue, prQueueEntry, P_QUE_ENTRY_T);
	while (prQueueEntry != NULL) {
		prCmdInfo = (P_CMD_INFO_T) prQueueEntry;
#if CFG_DBG_MGT_BUF
		if (prCmdInfo->pucInfoBuffer && !IS_FROM_BUF(prAdapter, prCmdInfo->pucInfoBuffer))
			prMemTrack = (struct MEM_TRACK *)((PUINT_8)prCmdInfo->pucInfoBuffer - sizeof(struct MEM_TRACK));
#endif

		if (prCmdInfo->fgDriverDomainMCR != TRUE) {
			nicTxCmd(prAdapter, prCmdInfo, (UINT_8)TC4_INDEX);

			if ((prCmdInfo->fgSetQuery != TRUE) ||
				(prCmdInfo->fgNeedResp == TRUE)) {
				KAL_ACQUIRE_SPIN_LOCK(prAdapter, SPIN_LOCK_CMD_PENDING);
#if CFG_DBG_MGT_BUF
				if (prMemTrack) {
					prMemTrack->u2CmdIdAndWhere &= 0x00FF;
					prMemTrack->u2CmdIdAndWhere |= 0x0200;
				}
#endif
				QUEUE_INSERT_TAIL(&(prAdapter->rPendingCmdQueue), (P_QUE_ENTRY_T) prCmdInfo);
				KAL_RELEASE_SPIN_LOCK(prAdapter, SPIN_LOCK_CMD_PENDING);
			} else {
#if CFG_DBG_MGT_BUF
				if (prMemTrack) {
					prMemTrack->u2CmdIdAndWhere &= 0x00FF;
					prMemTrack->u2CmdIdAndWhere |= 0x0300;
				}
#endif

				QUEUE_INSERT_TAIL(prTempCmdDoneQue, prQueueEntry);
			}
			/*
			 * DBGLOG(INIT, INFO,
			 * ("==> TX CMD QID: %d (Q:%d)\n", prCmdInfo->ucCID, prTempCmdQue->u4NumElem));
			 */
		} else {
			prCmdAccessReg = (P_CMD_ACCESS_REG) (prCmdInfo->pucInfoBuffer + CMD_HDR_SIZE);

			if (prCmdInfo->fgSetQuery == TRUE) {
				/* address is in DWORD unit */
				HAL_MCR_WR(prAdapter,
					(prCmdAccessReg->u4Address
					& (UINT_32)BITS(2U, 31U)),
					   prCmdAccessReg->u4Data);
			} else {
				u4Address = prCmdAccessReg->u4Address;
				prEventAccessReg = (P_CMD_ACCESS_REG) prCmdInfo->pucInfoBuffer;
				prEventAccessReg->u4Address = u4Address;
				/* address is in DWORD unit */
				HAL_MCR_RD(prAdapter,
					(UINT_32)(prEventAccessReg->u4Address
					& BITS(2U, 31U)),
					   &prEventAccessReg->u4Data);
			}
#if CFG_DBG_MGT_BUF
			if (prMemTrack) {
				prMemTrack->u2CmdIdAndWhere &= 0x00FF;
				prMemTrack->u2CmdIdAndWhere |= 0x0300;
			}
#endif

			QUEUE_INSERT_TAIL(prTempCmdDoneQue, prQueueEntry);
		}
		QUEUE_REMOVE_HEAD(prTempCmdQue, prQueueEntry, P_QUE_ENTRY_T);
	}

	KAL_ACQUIRE_SPIN_LOCK(prAdapter, SPIN_LOCK_TX_CMD_DONE_QUE);
	QUEUE_CONCATENATE_QUEUES(&prAdapter->rTxCmdDoneQueue, prTempCmdDoneQue);
	u4TxDoneQueueSize = prAdapter->rTxCmdDoneQueue.u4NumElem;
	KAL_RELEASE_SPIN_LOCK(prAdapter, SPIN_LOCK_TX_CMD_DONE_QUE);

	KAL_RELEASE_MUTEX(prAdapter, MUTEX_TX_CMD_CLEAR);

	if (u4TxDoneQueueSize > 0U) {
		/* call tx thread to work */
		set_bit(GLUE_FLAG_TX_CMD_DONE_BIT, &prAdapter->prGlueInfo->ulFlag);
		wake_up_interruptible(&prAdapter->prGlueInfo->waitq);
	}

	return WLAN_STATUS_SUCCESS;
}

WLAN_STATUS wlanTxCmdDoneMthread(IN P_ADAPTER_T prAdapter)
{
	QUE_T rTempCmdQue;
	P_QUE_T prTempCmdQue;
	P_QUE_ENTRY_T prQueueEntry;
	P_CMD_INFO_T prCmdInfo;

	KAL_SPIN_LOCK_DECLARATION();

	ASSERT(prAdapter);

	prTempCmdQue = &rTempCmdQue;
	QUEUE_INITIALIZE(prTempCmdQue);

	/* 4 <1> Move whole list of CMD_INFO to temp queue */
	KAL_ACQUIRE_SPIN_LOCK(prAdapter, SPIN_LOCK_TX_CMD_DONE_QUE);
	QUEUE_MOVE_ALL(prTempCmdQue, &prAdapter->rTxCmdDoneQueue);
	KAL_RELEASE_SPIN_LOCK(prAdapter, SPIN_LOCK_TX_CMD_DONE_QUE);

	/* 4 <2> Dequeue from head and check it is able to be sent */
	QUEUE_REMOVE_HEAD(prTempCmdQue, prQueueEntry, P_QUE_ENTRY_T);
	while (prQueueEntry != NULL) {
		prCmdInfo = (P_CMD_INFO_T) prQueueEntry;

		if (prCmdInfo->pfCmdDoneHandler != NULL)
			prCmdInfo->pfCmdDoneHandler(prAdapter, prCmdInfo, prCmdInfo->pucInfoBuffer);
		/* Not pending cmd, free it after TX succeed! */
		cmdBufFreeCmdInfo(prAdapter, prCmdInfo);

		QUEUE_REMOVE_HEAD(prTempCmdQue, prQueueEntry, P_QUE_ENTRY_T);
	}

	return WLAN_STATUS_SUCCESS;
}

/*----------------------------------------------------------------------------*/
/*!
* \brief This routine is used to clear all commands in TX command queue
* \param prAdapter  Pointer of Adapter Data Structure
*
* \retval none
*/
/*----------------------------------------------------------------------------*/
VOID wlanClearTxCommandQueue(IN P_ADAPTER_T prAdapter)
{
	QUE_T rTempCmdQue;
	P_QUE_T prTempCmdQue = &rTempCmdQue;
	P_QUE_ENTRY_T prQueueEntry = (P_QUE_ENTRY_T) NULL;
	P_CMD_INFO_T prCmdInfo = (P_CMD_INFO_T) NULL;

	KAL_SPIN_LOCK_DECLARATION();
	QUEUE_INITIALIZE(prTempCmdQue);

	KAL_ACQUIRE_SPIN_LOCK(prAdapter, SPIN_LOCK_TX_CMD_QUE);
	QUEUE_MOVE_ALL(prTempCmdQue, &prAdapter->rTxCmdQueue);
	KAL_RELEASE_SPIN_LOCK(prAdapter, SPIN_LOCK_TX_CMD_QUE);

	QUEUE_REMOVE_HEAD(prTempCmdQue, prQueueEntry, P_QUE_ENTRY_T);
	while (prQueueEntry != NULL) {
		prCmdInfo = (P_CMD_INFO_T) prQueueEntry;

		if (prCmdInfo->pfCmdTimeoutHandler != NULL)
			prCmdInfo->pfCmdTimeoutHandler(prAdapter, prCmdInfo);
		else
			wlanReleaseCommand(prAdapter, prCmdInfo, TX_RESULT_QUEUE_CLEARANCE);

		cmdBufFreeCmdInfo(prAdapter, prCmdInfo);

		QUEUE_REMOVE_HEAD(prTempCmdQue, prQueueEntry, P_QUE_ENTRY_T);
	}
}

/*----------------------------------------------------------------------------*/
/*!
* \brief This routine is used to clear OID commands in TX command queue
* \param prAdapter  Pointer of Adapter Data Structure
*
* \retval none
*/
/*----------------------------------------------------------------------------*/
VOID wlanClearTxOidCommand(IN P_ADAPTER_T prAdapter)
{
	QUE_T rTempCmdQue;
	P_QUE_T prTempCmdQue = &rTempCmdQue;
	P_QUE_ENTRY_T prQueueEntry = (P_QUE_ENTRY_T) NULL;
	P_CMD_INFO_T prCmdInfo = (P_CMD_INFO_T) NULL;

	KAL_SPIN_LOCK_DECLARATION();
	QUEUE_INITIALIZE(prTempCmdQue);

	KAL_ACQUIRE_SPIN_LOCK(prAdapter, SPIN_LOCK_TX_CMD_QUE);

	QUEUE_MOVE_ALL(prTempCmdQue, &prAdapter->rTxCmdQueue);

	QUEUE_REMOVE_HEAD(prTempCmdQue, prQueueEntry, P_QUE_ENTRY_T);

	while (prQueueEntry != NULL) {
		prCmdInfo = (P_CMD_INFO_T) prQueueEntry;

		if (prCmdInfo->fgIsOid == TRUE) {

			if (prCmdInfo->pfCmdTimeoutHandler != NULL)
				prCmdInfo->pfCmdTimeoutHandler(prAdapter, prCmdInfo);
			else
				wlanReleaseCommand(prAdapter, prCmdInfo, TX_RESULT_QUEUE_CLEARANCE);

			cmdBufFreeCmdInfo(prAdapter, prCmdInfo);

		} else {
			QUEUE_INSERT_TAIL(&prAdapter->rTxCmdQueue, prQueueEntry);
		}

		QUEUE_REMOVE_HEAD(prTempCmdQue, prQueueEntry, P_QUE_ENTRY_T);
	}

	KAL_RELEASE_SPIN_LOCK(prAdapter, SPIN_LOCK_TX_CMD_QUE);
}

/*----------------------------------------------------------------------------*/
/*!
* \brief This routine is used to clear all commands in TX command done queue
* \param prAdapter  Pointer of Adapter Data Structure
*
* \retval none
*/
/*----------------------------------------------------------------------------*/
VOID wlanClearTxCommandDoneQueue(IN P_ADAPTER_T prAdapter)
{
	QUE_T rTempCmdDoneQue;
	P_QUE_T prTempCmdDoneQue = &rTempCmdDoneQue;
	P_QUE_ENTRY_T prQueueEntry = (P_QUE_ENTRY_T) NULL;
	P_CMD_INFO_T prCmdInfo = (P_CMD_INFO_T) NULL;

	KAL_SPIN_LOCK_DECLARATION();
	QUEUE_INITIALIZE(prTempCmdDoneQue);

	/* 4 <1> Move whole list of CMD_INFO to temp queue */
	KAL_ACQUIRE_SPIN_LOCK(prAdapter, SPIN_LOCK_TX_CMD_DONE_QUE);
	QUEUE_MOVE_ALL(prTempCmdDoneQue, &prAdapter->rTxCmdDoneQueue);
	KAL_RELEASE_SPIN_LOCK(prAdapter, SPIN_LOCK_TX_CMD_DONE_QUE);

	/* 4 <2> Dequeue from head and check it is able to be sent */
	QUEUE_REMOVE_HEAD(prTempCmdDoneQue, prQueueEntry, P_QUE_ENTRY_T);
	while (prQueueEntry != NULL) {
		prCmdInfo = (P_CMD_INFO_T) prQueueEntry;

		if (prCmdInfo->pfCmdDoneHandler != NULL)
			prCmdInfo->pfCmdDoneHandler(prAdapter, prCmdInfo, prCmdInfo->pucInfoBuffer);
		/* Not pending cmd, free it after TX succeed! */
		cmdBufFreeCmdInfo(prAdapter, prCmdInfo);

		QUEUE_REMOVE_HEAD(prTempCmdDoneQue, prQueueEntry, P_QUE_ENTRY_T);
	}
}

/*----------------------------------------------------------------------------*/
/*!
* \brief This routine is used to clear all buffer in port 0/1 queue
* \param prAdapter  Pointer of Adapter Data Structure
*
* \retval none
*/
/*----------------------------------------------------------------------------*/
VOID wlanClearDataQueue(IN P_ADAPTER_T prAdapter)
{
	QUE_T qDataPort0, qDataPort1;
	P_QUE_T prDataPort0, prDataPort1;

	KAL_SPIN_LOCK_DECLARATION();

	prDataPort0 = &qDataPort0;
	prDataPort1 = &qDataPort1;

	QUEUE_INITIALIZE(prDataPort0);
	QUEUE_INITIALIZE(prDataPort1);

	/* 4 <1> Move whole list of CMD_INFO to temp queue */
	KAL_ACQUIRE_SPIN_LOCK(prAdapter, SPIN_LOCK_TX_PORT_QUE);
	QUEUE_MOVE_ALL(prDataPort0, &prAdapter->rTxP0Queue);
	QUEUE_MOVE_ALL(prDataPort1, &prAdapter->rTxP1Queue);
	KAL_RELEASE_SPIN_LOCK(prAdapter, SPIN_LOCK_TX_PORT_QUE);

	/* 4 <2> Return sk buffer */
	nicTxReturnMsduInfo(prAdapter, (P_MSDU_INFO_T) QUEUE_GET_HEAD(prDataPort0));
	nicTxReturnMsduInfo(prAdapter, (P_MSDU_INFO_T) QUEUE_GET_HEAD(prDataPort1));

}

/*----------------------------------------------------------------------------*/
/*!
* \brief This routine is used to clear all buffer in port 0/1 queue
* \param prAdapter  Pointer of Adapter Data Structure
*
* \retval none
*/
/*----------------------------------------------------------------------------*/
VOID wlanClearRxToOsQueue(IN P_ADAPTER_T prAdapter)
{
	QUE_T rTempRxQue;
	P_QUE_T prTempRxQue = &rTempRxQue;
	P_QUE_ENTRY_T prQueueEntry = (P_QUE_ENTRY_T) NULL;

	KAL_SPIN_LOCK_DECLARATION();
	QUEUE_INITIALIZE(prTempRxQue);

	/* 4 <1> Move whole list of CMD_INFO to temp queue */
	KAL_ACQUIRE_SPIN_LOCK(prAdapter, SPIN_LOCK_RX_TO_OS_QUE);
	QUEUE_MOVE_ALL(prTempRxQue, &prAdapter->rRxQueue);
	KAL_RELEASE_SPIN_LOCK(prAdapter, SPIN_LOCK_RX_TO_OS_QUE);

	/* 4 <2> Remove all skbuf */
	QUEUE_REMOVE_HEAD(prTempRxQue, prQueueEntry, P_QUE_ENTRY_T);
	while (prQueueEntry != NULL) {
		if (kalRxIndicateOnePkt(prAdapter->prGlueInfo,
			(PVOID) GLUE_GET_PKT_DESCRIPTOR(prQueueEntry))
			== WLAN_STATUS_SUCCESS)
			DBGLOG(INIT, TRACE,
				"kalRxIndicateOnePkt\n");
		QUEUE_REMOVE_HEAD(prTempRxQue, prQueueEntry, P_QUE_ENTRY_T);
	}

}
#endif

/*----------------------------------------------------------------------------*/
/*!
 * \brief This function will release thd CMD_INFO upon its attribution
 *
 * \param prAdapter  Pointer of Adapter Data Structure
 * \param prCmdInfo  Pointer of CMD_INFO_T
 * \param rTxDoneStatus  Tx done status
 *
 * \return (none)
 */
/*----------------------------------------------------------------------------*/
VOID wlanReleaseCommand(IN P_ADAPTER_T prAdapter, IN P_CMD_INFO_T prCmdInfo, IN ENUM_TX_RESULT_CODE_T rTxDoneStatus)
{
	P_TX_CTRL_T prTxCtrl;
	P_MSDU_INFO_T prMsduInfo;

	ASSERT(prAdapter);
	ASSERT(prCmdInfo);

	prTxCtrl = &prAdapter->rTxCtrl;

	switch (prCmdInfo->eCmdType) {
	case COMMAND_TYPE_GENERAL_IOCTL:
	case COMMAND_TYPE_NETWORK_IOCTL:
		DBGLOG(INIT, INFO, "Free CMD: BSS[%u] ID[0x%x] SeqNum[%u] OID[%u]\n",
				    prCmdInfo->ucBssIndex,
				    prCmdInfo->ucCID, prCmdInfo->ucCmdSeqNum, prCmdInfo->fgIsOid);

		if (prCmdInfo->fgIsOid == TRUE) {
			kalOidComplete(prAdapter->prGlueInfo,
				       prCmdInfo->fgSetQuery, prCmdInfo->u4SetInfoLen, WLAN_STATUS_FAILURE);
		}
		break;

	case COMMAND_TYPE_SECURITY_FRAME:
	case COMMAND_TYPE_MANAGEMENT_FRAME:
		prMsduInfo = prCmdInfo->prMsduInfo;

		if (prCmdInfo->eCmdType == COMMAND_TYPE_SECURITY_FRAME) {
			kalSecurityFrameSendComplete(prAdapter->prGlueInfo, prCmdInfo->prPacket, WLAN_STATUS_FAILURE);
			/* Avoid skb multiple free */
			prMsduInfo->prPacket = NULL;
		}

		DBGLOG(INIT, INFO,
			"Free %s Frame: BSS[%u] WIDX:PID[%u:%u] SEQ[%u] STA[%u] RSP[%u] CMDSeq[%u]\n",
			prCmdInfo->eCmdType == COMMAND_TYPE_SECURITY_FRAME ? "SEC" : "MGMT",
			prMsduInfo->ucBssIndex,
			prMsduInfo->ucWlanIndex,
			prMsduInfo->ucPID,
			prMsduInfo->ucTxSeqNum,
			prMsduInfo->ucStaRecIndex,
			prMsduInfo->pfTxDoneHandler != NULL ? TRUE : FALSE,
			prCmdInfo->ucCmdSeqNum);

		/* invoke callbacks */
		if (prMsduInfo->pfTxDoneHandler != NULL)
			prMsduInfo->pfTxDoneHandler(prAdapter, prMsduInfo, rTxDoneStatus);

		if (prCmdInfo->eCmdType == COMMAND_TYPE_MANAGEMENT_FRAME)
			GLUE_DEC_REF_CNT(prTxCtrl->i4TxMgmtPendingNum);

		cnmMgtPktFree(prAdapter, prMsduInfo);
		break;

	default:
		ASSERT(NULL);
		break;
	}

}				/* end of wlanReleaseCommand() */

/*----------------------------------------------------------------------------*/
/*!
* \brief This function will search the CMD Queue to look for the pending OID and
*        compelete it immediately when system request a reset.
*
* \param prAdapter  ointer of Adapter Data Structure
*
* \return (none)
*/
/*----------------------------------------------------------------------------*/
VOID wlanReleasePendingOid(IN P_ADAPTER_T prAdapter, IN ULONG ulParamPtr)
{
	P_QUE_T prCmdQue;
	QUE_T rTempCmdQue;
	P_QUE_T prTempCmdQue = &rTempCmdQue;
	P_QUE_ENTRY_T prQueueEntry = (P_QUE_ENTRY_T) NULL;
	P_CMD_INFO_T prCmdInfo = (P_CMD_INFO_T) NULL;

	KAL_SPIN_LOCK_DECLARATION();

	DEBUGFUNC("wlanReleasePendingOid");

	ASSERT(prAdapter);

	do {
		if (ulParamPtr == 1U)
			break;

		if ((prAdapter->prGlueInfo->ulFlag & GLUE_FLAG_HALT) != 0U) {
			DBGLOG(INIT, INFO, "tx_thread stopped! Releasing pending OIDs ..\n");
		} else {
			DBGLOG(INIT, ERROR, "OID Timeout! Releasing pending OIDs ..\n");
			prAdapter->ucOidTimeoutCount++;

			if (prAdapter->ucOidTimeoutCount >= WLAN_OID_NO_ACK_THRESHOLD) {
				if (prAdapter->fgIsChipNoAck != TRUE) {
					DBGLOG(INIT, WARN,
					       "No response from chip for %u times, set NoAck flag!\n",
						prAdapter->ucOidTimeoutCount);
#if CFG_CHIP_RESET_SUPPORT
					if (glResetTrigger(prAdapter) == TRUE)
						DBGLOG(INIT, TRACE,
							"Reset trigger\n");
#endif
				}

				prAdapter->fgIsChipNoAck = TRUE;
			}
		}
	} while (0U != 0U);

	do {
#if CFG_SUPPORT_MULTITHREAD
		KAL_ACQUIRE_MUTEX(prAdapter, MUTEX_TX_CMD_CLEAR);
#endif

		/* 1: Clear pending OID in glue layer command queue */
		kalOidCmdClearance(prAdapter->prGlueInfo);

#if CFG_SUPPORT_MULTITHREAD
		/* Clear pending OID in main_thread to hif_thread command queue */
		wlanClearTxOidCommand(prAdapter);
#endif
		/* 2: Clear Pending OID in prAdapter->rPendingCmdQueue */
		KAL_ACQUIRE_SPIN_LOCK(prAdapter, SPIN_LOCK_CMD_PENDING);

		prCmdQue = &prAdapter->rPendingCmdQueue;
		QUEUE_MOVE_ALL(prTempCmdQue, prCmdQue);

		QUEUE_REMOVE_HEAD(prTempCmdQue, prQueueEntry, P_QUE_ENTRY_T);
		while (prQueueEntry != NULL) {
			prCmdInfo = (P_CMD_INFO_T) prQueueEntry;

			if (prCmdInfo->fgIsOid == TRUE) {
				if (prCmdInfo->pfCmdTimeoutHandler != NULL) {
					prCmdInfo->pfCmdTimeoutHandler(prAdapter, prCmdInfo);
				} else {
					kalOidComplete(prAdapter->prGlueInfo,
						       prCmdInfo->fgSetQuery, 0, WLAN_STATUS_FAILURE);
				}

				cmdBufFreeCmdInfo(prAdapter, prCmdInfo);
			} else {
				QUEUE_INSERT_TAIL(prCmdQue, prQueueEntry);
			}

			QUEUE_REMOVE_HEAD(prTempCmdQue, prQueueEntry, P_QUE_ENTRY_T);
		}

		KAL_RELEASE_SPIN_LOCK(prAdapter, SPIN_LOCK_CMD_PENDING);

		/* 3: Clear pending OID queued in pvOidEntry with REQ_FLAG_OID set */
		kalOidClearance(prAdapter->prGlueInfo);

#if CFG_SUPPORT_MULTITHREAD
		KAL_RELEASE_MUTEX(prAdapter, MUTEX_TX_CMD_CLEAR);
#endif
	} while (0 != 0);
}

/*----------------------------------------------------------------------------*/
/*!
* \brief This function will search the CMD Queue to look for the pending CMD/OID for specific
*        NETWORK TYPE and compelete it immediately when system request a reset.
*
* \param prAdapter  ointer of Adapter Data Structure
*
* \return (none)
*/
/*----------------------------------------------------------------------------*/
VOID wlanReleasePendingCMDbyBssIdx(IN P_ADAPTER_T prAdapter, IN UINT_8 ucBssIndex)
{
	P_QUE_T prCmdQue;
	QUE_T rTempCmdQue;
	P_QUE_T prTempCmdQue = &rTempCmdQue;
	P_QUE_ENTRY_T prQueueEntry = (P_QUE_ENTRY_T) NULL;
	P_CMD_INFO_T prCmdInfo = (P_CMD_INFO_T) NULL;

	KAL_SPIN_LOCK_DECLARATION();

	ASSERT(prAdapter);

	do {
		/* 1: Clear Pending OID in prAdapter->rPendingCmdQueue */
		KAL_ACQUIRE_SPIN_LOCK(prAdapter, SPIN_LOCK_CMD_PENDING);

		prCmdQue = &prAdapter->rPendingCmdQueue;
		QUEUE_MOVE_ALL(prTempCmdQue, prCmdQue);

		QUEUE_REMOVE_HEAD(prTempCmdQue, prQueueEntry, P_QUE_ENTRY_T);
		while (prQueueEntry != NULL) {
			prCmdInfo = (P_CMD_INFO_T) prQueueEntry;

			DBGLOG(P2P, TRACE, "Pending CMD for BSS:%d\n", prCmdInfo->ucBssIndex);

			if (prCmdInfo->ucBssIndex == ucBssIndex) {
				if (prCmdInfo->pfCmdTimeoutHandler != NULL) {
					prCmdInfo->pfCmdTimeoutHandler(prAdapter, prCmdInfo);
				} else if (prCmdInfo->fgIsOid == TRUE) {
					kalOidComplete(prAdapter->prGlueInfo,
						       prCmdInfo->fgSetQuery, 0, WLAN_STATUS_FAILURE);
				}

				cmdBufFreeCmdInfo(prAdapter, prCmdInfo);
			} else {
				QUEUE_INSERT_TAIL(prCmdQue, prQueueEntry);
			}

			QUEUE_REMOVE_HEAD(prTempCmdQue, prQueueEntry, P_QUE_ENTRY_T);
		}

		KAL_RELEASE_SPIN_LOCK(prAdapter, SPIN_LOCK_CMD_PENDING);

	} while (0U != 0U);
}				/* wlanReleasePendingCMDbyBssIdx */

/*----------------------------------------------------------------------------*/
/*!
* \brief Return the indicated packet buffer and reallocate one to the RFB
*
* \param prAdapter      Pointer of Adapter Data Structure
* \param pvPacket       Pointer of returned packet
*
* \retval WLAN_STATUS_SUCCESS: Success
* \retval WLAN_STATUS_FAILURE: Failed
*/
/*----------------------------------------------------------------------------*/
VOID wlanReturnPacketDelaySetupTimeout(IN P_ADAPTER_T prAdapter, IN ULONG ulParamPtr)
{
	P_RX_CTRL_T prRxCtrl;
	P_SW_RFB_T prSwRfb = NULL;
	WLAN_STATUS status = WLAN_STATUS_SUCCESS;
	P_QUE_T prQueList;

	KAL_SPIN_LOCK_DECLARATION();

	ASSERT(prAdapter);

	prRxCtrl = &prAdapter->rRxCtrl;
	ASSERT(prRxCtrl);

	prQueList = &prRxCtrl->rIndicatedRfbList;
	DBGLOG(RX, WARN, "%s: IndicatedRfbList num = %u\n", __func__, prQueList->u4NumElem);

	while (QUEUE_IS_NOT_EMPTY(&prRxCtrl->rIndicatedRfbList)) {
		KAL_ACQUIRE_SPIN_LOCK(prAdapter, SPIN_LOCK_RX_FREE_QUE);
		QUEUE_REMOVE_HEAD(&prRxCtrl->rIndicatedRfbList, prSwRfb, P_SW_RFB_T);
		KAL_RELEASE_SPIN_LOCK(prAdapter, SPIN_LOCK_RX_FREE_QUE);

		status = nicRxSetupRFB(prAdapter, prSwRfb);
		nicRxReturnRFB(prAdapter, prSwRfb);

		if (status != WLAN_STATUS_SUCCESS)
			break;
	}

	if (status != WLAN_STATUS_SUCCESS) {
		DBGLOG(RX, WARN, "Restart ReturnIndicatedRfb Timer (%u)\n", RX_RETURN_INDICATED_RFB_TIMEOUT_SEC);
		/* restart timer */
		cnmTimerStartTimer(prAdapter,
				   &prAdapter->rPacketDelaySetupTimer,
				   SEC_TO_MSEC(RX_RETURN_INDICATED_RFB_TIMEOUT_SEC));
	}
}

/*----------------------------------------------------------------------------*/
/*!
* \brief Return the packet buffer and reallocate one to the RFB
*
* \param prAdapter      Pointer of Adapter Data Structure
* \param pvPacket       Pointer of returned packet
*
* \retval WLAN_STATUS_SUCCESS: Success
* \retval WLAN_STATUS_FAILURE: Failed
*/
/*----------------------------------------------------------------------------*/
VOID wlanReturnPacket(IN P_ADAPTER_T prAdapter, IN PVOID pvPacket)
{
	P_RX_CTRL_T prRxCtrl;
	P_SW_RFB_T prSwRfb = NULL;

	KAL_SPIN_LOCK_DECLARATION();

	DEBUGFUNC("wlanReturnPacket");

	ASSERT(prAdapter);

	prRxCtrl = &prAdapter->rRxCtrl;
	ASSERT(prRxCtrl);

	if (pvPacket != NULL) {
		kalPacketFree(prAdapter->prGlueInfo, pvPacket);
		RX_ADD_CNT(prRxCtrl, RX_DATA_RETURNED_COUNT, 1);
#if CFG_NATIVE_802_11
		if (GLUE_TEST_FLAG(prAdapter->prGlueInfo, GLUE_FLAG_HALT)) {
			/*Todo:: nothing*/
			/*Todo:: nothing*/
		}
#endif
	}

	KAL_ACQUIRE_SPIN_LOCK(prAdapter, SPIN_LOCK_RX_FREE_QUE);
	QUEUE_REMOVE_HEAD(&prRxCtrl->rIndicatedRfbList, prSwRfb, P_SW_RFB_T);
	KAL_RELEASE_SPIN_LOCK(prAdapter, SPIN_LOCK_RX_FREE_QUE);
	if (prSwRfb == NULL) {
		DBGLOG(RX, WARN, "No free SwRfb!\n");
		return;
	}

	if (nicRxSetupRFB(prAdapter, prSwRfb) != WLAN_STATUS_SUCCESS) {
		DBGLOG(RX, WARN, "Cannot allocate packet buffer for SwRfb!\n");
		if (timerPendingTimer(
			&prAdapter->rPacketDelaySetupTimer) == 0) {
			DBGLOG(RX, WARN,
			       "Start ReturnIndicatedRfb Timer (%u)\n", RX_RETURN_INDICATED_RFB_TIMEOUT_SEC);
			cnmTimerStartTimer(prAdapter, &prAdapter->rPacketDelaySetupTimer,
					   SEC_TO_MSEC(RX_RETURN_INDICATED_RFB_TIMEOUT_SEC));
		}
	}
	nicRxReturnRFB(prAdapter, prSwRfb);
}

/*----------------------------------------------------------------------------*/
/*!
* \brief This function is a required function that returns information about
*        the capabilities and status of the driver and/or its network adapter.
*
* \param[IN] prAdapter        Pointer to the Adapter structure.
* \param[IN] pfnOidQryHandler Function pointer for the OID query handler.
* \param[IN] pvInfoBuf        Points to a buffer for return the query information.
* \param[IN] u4QueryBufferLen Specifies the number of bytes at pvInfoBuf.
* \param[OUT] pu4QueryInfoLen  Points to the number of bytes it written or is needed.
*
* \retval WLAN_STATUS_xxx Different WLAN_STATUS code returned by different handlers.
*
*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
wlanQueryInformation(IN P_ADAPTER_T prAdapter,
		     IN PFN_OID_HANDLER_FUNC pfnOidQryHandler,
		     IN PVOID pvInfoBuf, IN UINT_32 u4InfoBufLen, OUT PUINT_32 pu4QryInfoLen)
{
	WLAN_STATUS status;

	ASSERT(prAdapter);
	ASSERT(pu4QryInfoLen);

	/* ignore any OID request after connected, under PS current measurement mode */
	if (prAdapter->u4PsCurrentMeasureEn != 0U &&
		(prAdapter->prGlueInfo->eParamMediaStateIndicated == PARAM_MEDIA_STATE_CONNECTED)) {
		/*
		 * note: return WLAN_STATUS_FAILURE or
		 * WLAN_STATUS_SUCCESS for blocking OIDs during current measurement ??
		 */
		return WLAN_STATUS_SUCCESS;
	}
#if 1
	/* most OID handler will just queue a command packet */
	status = pfnOidQryHandler(prAdapter, pvInfoBuf, u4InfoBufLen, pu4QryInfoLen);
#else
	if (wlanIsHandlerNeedHwAccess(pfnOidQryHandler, FALSE) == TRUE) {
		ACQUIRE_POWER_CONTROL_FROM_PM(prAdapter);

		/* Reset sleepy state */
		if (prAdapter->fgWiFiInSleepyState == TRUE)
			prAdapter->fgWiFiInSleepyState = FALSE;

		status = pfnOidQryHandler(prAdapter, pvInfoBuf, u4InfoBufLen, pu4QryInfoLen);

		RECLAIM_POWER_CONTROL_TO_PM(prAdapter, FALSE);
	} else
		status = pfnOidQryHandler(prAdapter, pvInfoBuf, u4InfoBufLen, pu4QryInfoLen);
#endif

	return status;

}

/*----------------------------------------------------------------------------*/
/*!
* \brief This function is a required function that allows bound protocol drivers,
*        or NDIS, to request changes in the state information that the miniport
*        maintains for particular object identifiers, such as changes in multicast
*        addresses.
*
* \param[IN] prAdapter     Pointer to the Glue info structure.
* \param[IN] pfnOidSetHandler     Points to the OID set handlers.
* \param[IN] pvInfoBuf     Points to a buffer containing the OID-specific data for the set.
* \param[IN] u4InfoBufLen  Specifies the number of bytes at prSetBuffer.
* \param[OUT] pu4SetInfoLen Points to the number of bytes it read or is needed.
*
* \retval WLAN_STATUS_xxx Different WLAN_STATUS code returned by different handlers.
*
*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
wlanSetInformation(IN P_ADAPTER_T prAdapter,
		   IN PFN_OID_HANDLER_FUNC pfnOidSetHandler,
		   IN PVOID pvInfoBuf, IN UINT_32 u4InfoBufLen, OUT PUINT_32 pu4SetInfoLen)
{
	WLAN_STATUS status;

	ASSERT(prAdapter);
	ASSERT(pu4SetInfoLen);

	/* ignore any OID request after connected, under PS current measurement mode */
	if (prAdapter->u4PsCurrentMeasureEn != 0U &&
		(prAdapter->prGlueInfo->eParamMediaStateIndicated == PARAM_MEDIA_STATE_CONNECTED)) {
		/*
		 * note: return WLAN_STATUS_FAILURE or WLAN_STATUS_SUCCESS
		 * for blocking OIDs during current measurement ??
		 */
		return WLAN_STATUS_SUCCESS;
	}
#if 1
	/*
	 * most OID handler will just queue a command packet
	 * for power state transition OIDs, handler will acquire power control by itself
	 */
	status = pfnOidSetHandler(prAdapter, pvInfoBuf, u4InfoBufLen, pu4SetInfoLen);
#else
	if (wlanIsHandlerNeedHwAccess(pfnOidSetHandler, TRUE) == TRUE) {
		ACQUIRE_POWER_CONTROL_FROM_PM(prAdapter);

		/* Reset sleepy state */
		if (prAdapter->fgWiFiInSleepyState == TRUE)
			prAdapter->fgWiFiInSleepyState = FALSE;

		status = pfnOidSetHandler(prAdapter, pvInfoBuf, u4InfoBufLen, pu4SetInfoLen);

		RECLAIM_POWER_CONTROL_TO_PM(prAdapter, FALSE);
	} else {
		status = pfnOidSetHandler(prAdapter, pvInfoBuf, u4InfoBufLen, pu4SetInfoLen);
	}
#endif

	return status;
}

#if CFG_SUPPORT_WAPI
/*----------------------------------------------------------------------------*/
/*!
* \brief This function is a used to query driver's config wapi mode or not
*
* \param[IN] prAdapter     Pointer to the Glue info structure.
*
* \retval TRUE for use wapi mode
*
*/
/*----------------------------------------------------------------------------*/
BOOLEAN wlanQueryWapiMode(IN P_ADAPTER_T prAdapter)
{
	ASSERT(prAdapter);

	return prAdapter->rWifiVar.rConnSettings.fgWapiMode;
}
#endif

/*----------------------------------------------------------------------------*/
/*!
* \brief This function is called to set RX filter to Promiscuous Mode.
*
* \param[IN] prAdapter        Pointer to the Adapter structure.
* \param[IN] fgEnablePromiscuousMode Enable/ disable RX Promiscuous Mode.
*
* \return (none)
*/
/*----------------------------------------------------------------------------*/
VOID wlanSetPromiscuousMode(IN P_ADAPTER_T prAdapter, IN BOOLEAN fgEnablePromiscuousMode)
{
	ASSERT(prAdapter);

}

/*----------------------------------------------------------------------------*/
/*!
* \brief This function is called to set RX filter to allow to receive
*        broadcast address packets.
*
* \param[IN] prAdapter        Pointer to the Adapter structure.
* \param[IN] fgEnableBroadcast Enable/ disable broadcast packet to be received.
*
* \return (none)
*/
/*----------------------------------------------------------------------------*/
VOID wlanRxSetBroadcast(IN P_ADAPTER_T prAdapter, IN BOOLEAN fgEnableBroadcast)
{
	ASSERT(prAdapter);
}

/*----------------------------------------------------------------------------*/
/*!
* \brief This function is called to send out CMD_NIC_POWER_CTRL command packet
*
* \param[IN] prAdapter        Pointer to the Adapter structure.
* \param[IN] ucPowerMode      refer to CMD/EVENT document
*
* \return WLAN_STATUS_SUCCESS
* \return WLAN_STATUS_FAILURE
*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS wlanSendNicPowerCtrlCmd(IN P_ADAPTER_T prAdapter, IN UINT_8 ucPowerMode)
{
	WLAN_STATUS status = WLAN_STATUS_SUCCESS;
	P_GLUE_INFO_T prGlueInfo;
	P_CMD_INFO_T prCmdInfo;
	P_WIFI_CMD_T prWifiCmd;
	UINT_8 ucTC, ucCmdSeqNum;
	UINT_32 u4StartTime;

	ASSERT(prAdapter);

	prGlueInfo = prAdapter->prGlueInfo;

	/* 1. Prepare CMD */
	prCmdInfo = cmdBufAllocateCmdInfo(prAdapter,
		(UINT_32)(CMD_HDR_SIZE + sizeof(CMD_NIC_POWER_CTRL)));
	if (prCmdInfo == NULL) {
		DBGLOG(INIT, ERROR, "Allocate CMD_INFO_T ==> FAILED.\n");
		return WLAN_STATUS_FAILURE;
	}

	/* 2.1 increase command sequence number */
	ucCmdSeqNum = nicIncreaseCmdSeqNum(prAdapter);
	DBGLOG(REQ, TRACE, "ucCmdSeqNum =%d\n", ucCmdSeqNum);

	/* 2.2 Setup common CMD Info Packet */
	prCmdInfo->eCmdType = COMMAND_TYPE_GENERAL_IOCTL;
	prCmdInfo->u2InfoBufLen = (UINT_16) (CMD_HDR_SIZE + sizeof(CMD_NIC_POWER_CTRL));
	prCmdInfo->pfCmdDoneHandler = NULL;
	prCmdInfo->pfCmdTimeoutHandler = NULL;
	prCmdInfo->fgIsOid = TRUE;
	prCmdInfo->ucCID = (UINT_8)CMD_ID_NIC_POWER_CTRL;
	prCmdInfo->fgSetQuery = TRUE;
	prCmdInfo->fgNeedResp = FALSE;
	prCmdInfo->fgDriverDomainMCR = FALSE;
	prCmdInfo->ucCmdSeqNum = ucCmdSeqNum;
	prCmdInfo->u4SetInfoLen = (UINT_32)sizeof(CMD_NIC_POWER_CTRL);

	/* 2.3 Setup WIFI_CMD_T */
	prWifiCmd = (P_WIFI_CMD_T) (prCmdInfo->pucInfoBuffer);
	prWifiCmd->u2TxByteCount = prCmdInfo->u2InfoBufLen;
	prWifiCmd->u2PQ_ID = (UINT_16)CMD_PQ_ID;
	prWifiCmd->ucPktTypeID = (UINT_8)CMD_PACKET_TYPE_ID;
	prWifiCmd->ucCID = prCmdInfo->ucCID;
	prWifiCmd->ucSetQuery = prCmdInfo->fgSetQuery;
	prWifiCmd->ucSeqNum = prCmdInfo->ucCmdSeqNum;

	kalMemZero(prWifiCmd->aucBuffer, (UINT_32)sizeof(CMD_NIC_POWER_CTRL));
	((P_CMD_NIC_POWER_CTRL) (prWifiCmd->aucBuffer))->ucPowerMode = ucPowerMode;

	/* 3. Issue CMD for entering specific power mode */
	ucTC = (UINT_8)TC4_INDEX;

	u4StartTime = kalGetTimeTick();
	while (1 != 0) {
		/* 3.0 Removal check */
		if (kalIsCardRemoved(prAdapter->prGlueInfo) == TRUE || fgIsBusAccessFailed == TRUE) {
			status = WLAN_STATUS_FAILURE;
			break;
		}
		/* 3.1 Acquire TX Resource */
		if (nicTxAcquireResource(prAdapter, ucTC, nicTxGetCmdPageCount(prCmdInfo)) == WLAN_STATUS_RESOURCES) {
			if (nicTxPollingResource(prAdapter, ucTC) != WLAN_STATUS_SUCCESS) {
				DBGLOG(INIT, ERROR, "Fail to get TX resource return within timeout\n");
				status = WLAN_STATUS_FAILURE;
				prAdapter->fgIsChipNoAck = TRUE;
#if CFG_CHIP_RESET_SUPPORT
				DBGLOG(INIT, ERROR, "Fail to get TX resource and trigger assert!\n");
				//glResetTrigger(prAdapter);
				glResetWifiOnly();
#endif
				break;
			}
			if (CHECK_FOR_TIMEOUT(kalGetTimeTick(), u4StartTime,
					CFG_POWER_CTRL_ACQUIRE_RES_TIMEOUT)) {
				break;
			} else {
				continue;
			}
		}
		break;
	}

	/* 3.2 Send CMD Info Packet */
	if (nicTxCmd(prAdapter, prCmdInfo, ucTC) != WLAN_STATUS_SUCCESS) {
#if CFG_CHIP_RESET_SUPPORT
		DBGLOG(INIT, ERROR, "Fail to transmit CMD_NIC_POWER_CTRL command and trigger assert!\n");
		//glResetTrigger(prAdapter);
		glResetWifiOnly();
#endif
		status = WLAN_STATUS_FAILURE;
	}

	/* 4. Free CMD Info Packet. */
	cmdBufFreeCmdInfo(prAdapter, prCmdInfo);

	/* 5. Add flag */
	if (ucPowerMode == 1U)
		prAdapter->fgIsEnterD3ReqIssued = TRUE;

	return status;
}

/*----------------------------------------------------------------------------*/
/*!
* \brief This function is called to check if it is RF test mode and
*        the OID is allowed to be called or not
*
* \param[IN] prAdapter        Pointer to the Adapter structure.
* \param[IN] fgEnableBroadcast Enable/ disable broadcast packet to be received.
*
* \return (none)
*/
/*----------------------------------------------------------------------------*/
BOOLEAN wlanIsHandlerAllowedInRFTest(IN PFN_OID_HANDLER_FUNC pfnOidHandler, IN BOOLEAN fgSetInfo)
{
	PFN_OID_HANDLER_FUNC *apfnOidHandlerAllowedInRFTest;
	UINT_32 i;
	UINT_32 u4NumOfElem;

	if (fgSetInfo == TRUE) {
		apfnOidHandlerAllowedInRFTest = apfnOidSetHandlerAllowedInRFTest;
		u4NumOfElem = (UINT_32)
			sizeof(apfnOidSetHandlerAllowedInRFTest) /
			(UINT_32)sizeof(PFN_OID_HANDLER_FUNC);
	} else {
		apfnOidHandlerAllowedInRFTest = apfnOidQueryHandlerAllowedInRFTest;
		u4NumOfElem = (UINT_32)
			sizeof(apfnOidQueryHandlerAllowedInRFTest) /
			(UINT_32)sizeof(PFN_OID_HANDLER_FUNC);
	}

	for (i = 0U; i < u4NumOfElem; i++) {
		if (apfnOidHandlerAllowedInRFTest[i] == pfnOidHandler)
			return TRUE;
	}

	return FALSE;
}

#if CFG_ENABLE_FW_DOWNLOAD
/*----------------------------------------------------------------------------*/
/*!
* @brief This function is called to configure FWDL parameters
*
* @param prAdapter      Pointer to the Adapter structure.
*        u4DestAddr     Address of destination address
*        u4ImgSecSize   Length of the firmware block
*        fgReset        should be set to TRUE if this is the 1st configuration
*
* @return WLAN_STATUS_SUCCESS
*         WLAN_STATUS_FAILURE
*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
wlanImageSectionConfig(IN P_ADAPTER_T prAdapter, IN UINT_32 u4DestAddr, IN UINT_32 u4ImgSecSize, IN BOOLEAN fgReset,
		       IN UINT_8 ucEnc, IN UINT_8 ucKIdx)
{
	P_CMD_INFO_T prCmdInfo;
	P_INIT_HIF_TX_HEADER_T prInitHifTxHeader;
	P_INIT_CMD_DOWNLOAD_CONFIG prInitCmdDownloadConfig;
	UINT_8 ucTC, ucCmdSeqNum;
	WLAN_STATUS u4Status = WLAN_STATUS_SUCCESS;

	ASSERT(prAdapter);

	DEBUGFUNC("wlanImageSectionConfig");

	if (u4ImgSecSize == 0U)
		return WLAN_STATUS_SUCCESS;

	/* 1. Allocate CMD Info Packet and its Buffer. */
	prCmdInfo = cmdBufAllocateCmdInfo(prAdapter,
		(UINT_32)(sizeof(INIT_HIF_TX_HEADER_T) +
		sizeof(INIT_CMD_DOWNLOAD_CONFIG)));

	if (prCmdInfo == NULL) {
		DBGLOG(INIT, ERROR, "Allocate CMD_INFO_T ==> FAILED.\n");
		return WLAN_STATUS_FAILURE;
	}

	prCmdInfo->u2InfoBufLen =
		(UINT_16)(sizeof(INIT_HIF_TX_HEADER_T) +
		sizeof(INIT_CMD_DOWNLOAD_CONFIG));

	/* 2. Use TC4's resource to download image. (TC4 as CPU) */
	ucTC = (UINT_8)TC4_INDEX;

	/* 3. Increase command sequence number */
	ucCmdSeqNum = nicIncreaseCmdSeqNum(prAdapter);

	/* 4. Setup common CMD Info Packet */
	prInitHifTxHeader = (P_INIT_HIF_TX_HEADER_T) (prCmdInfo->pucInfoBuffer);
	prInitHifTxHeader->u2TxByteCount = prCmdInfo->u2InfoBufLen;
	prInitHifTxHeader->u2PQ_ID = (UINT_16)INIT_CMD_PQ_ID;

	prInitHifTxHeader->rInitWifiCmd.ucCID =
		(UINT_8)INIT_CMD_ID_DOWNLOAD_CONFIG;
	prInitHifTxHeader->rInitWifiCmd.ucPktTypeID =
		(UINT_8)INIT_CMD_PACKET_TYPE_ID;
	prInitHifTxHeader->rInitWifiCmd.ucReserved = 0U;
	prInitHifTxHeader->rInitWifiCmd.ucSeqNum = ucCmdSeqNum;

	/* 5. Setup Download config */
	prInitCmdDownloadConfig = (P_INIT_CMD_DOWNLOAD_CONFIG) (prInitHifTxHeader->rInitWifiCmd.aucBuffer);
	prInitCmdDownloadConfig->u4Address = u4DestAddr;
	prInitCmdDownloadConfig->u4Length = u4ImgSecSize;
	prInitCmdDownloadConfig->u4DataMode = 0U;

	/* ACK needed */
#if CFG_ENABLE_FW_DOWNLOAD_ACK
	prInitCmdDownloadConfig->u4DataMode |=
		(UINT_32)DOWNLOAD_CONFIG_ACK_OPTION;
#endif


#if CFG_ENABLE_FW_ENCRYPTION
#if defined(MT6631)
	if (ucEnc != 0U) {
		prInitCmdDownloadConfig->u4DataMode |=
			(UINT_32)DOWNLOAD_CONFIG_ENCRYPTION_MODE;
		prInitCmdDownloadConfig->u4DataMode |=
			((UINT_32)ucKIdx & (UINT_32)BITS(0U, 1U)) <<
			(UINT_32)DOWNLOAD_CONFIG_ENCRYPT_IDX_OFFSET;
	}
#else
	/* MT6630 don't check ucEnc and ucKIdx now, but forcibly enable download encryption */
	prInitCmdDownloadConfig->u4DataMode |=
		(UINT_32)DOWNLOAD_CONFIG_ENCRYPTION_MODE;
#endif
#endif

	if (fgReset == TRUE)
		prInitCmdDownloadConfig->u4DataMode |=
			(UINT_32)DOWNLOAD_CONFIG_RESET_OPTION;

	/* 6. Send Download config command */
	while (1U != 0U) {
		/* 6.1 Acquire TX Resource */
		if (nicTxAcquireResource
		    (prAdapter, ucTC, nicTxGetPageCount(prCmdInfo->u2InfoBufLen, TRUE)) == WLAN_STATUS_RESOURCES) {
			if (nicTxPollingResource(prAdapter, ucTC) != WLAN_STATUS_SUCCESS) {
				u4Status = WLAN_STATUS_FAILURE;
				DBGLOG(INIT, ERROR, "Fail to get TX resource return within timeout\n");
				break;
			}
			continue;
		}
		/* 6.2 Send CMD Info Packet */
		if (nicTxInitCmd(prAdapter, prCmdInfo) != WLAN_STATUS_SUCCESS) {
			u4Status = WLAN_STATUS_FAILURE;
			DBGLOG(INIT, ERROR, "Fail to transmit download config command\n");
		}

		break;
	};

	DBGLOG(INIT, TRACE, "u4Status = %d\n", u4Status);
#if CFG_ENABLE_FW_DOWNLOAD_ACK
	/* 7. Wait for INIT_EVENT_ID_CMD_RESULT */
	u4Status = wlanImageSectionDownloadStatus(prAdapter, ucCmdSeqNum);
#endif

	/* 8. Free CMD Info Packet. */
	cmdBufFreeCmdInfo(prAdapter, prCmdInfo);

	return u4Status;
}

/*----------------------------------------------------------------------------*/
/*!
* @brief This function is called to download FW image.
*
* @param prAdapter      Pointer to the Adapter structure.
*
* @return (none)
*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS wlanImageSectionDownload(IN P_ADAPTER_T prAdapter, IN UINT_32 u4ImgSecSize, IN PUINT_8 pucImgSecBuf)
{
	P_CMD_INFO_T prCmdInfo;
	P_INIT_HIF_TX_HEADER_T prInitHifTxHeader;
	WLAN_STATUS u4Status = WLAN_STATUS_SUCCESS;

	ASSERT(prAdapter);
	ASSERT(pucImgSecBuf);
	ASSERT_BOOLEAN(u4ImgSecSize <= CMD_PKT_SIZE_FOR_IMAGE);

	DEBUGFUNC("wlanImageSectionDownload");

	if (u4ImgSecSize == 0U)
		return WLAN_STATUS_SUCCESS;

	/* 1. Allocate CMD Info Packet and its Buffer. */
	prCmdInfo = cmdBufAllocateCmdInfo(prAdapter,
		(UINT_32)sizeof(INIT_HIF_TX_HEADER_T) + u4ImgSecSize);

	if (prCmdInfo == NULL) {
		DBGLOG(INIT, ERROR, "Allocate CMD_INFO_T ==> FAILED.\n");
		return WLAN_STATUS_FAILURE;
	}

	prCmdInfo->u2InfoBufLen = (UINT_16)sizeof(INIT_HIF_TX_HEADER_T)
		+ (UINT_16) u4ImgSecSize;

	/* 2. Setup common CMD Info Packet */
	prInitHifTxHeader = (P_INIT_HIF_TX_HEADER_T) (prCmdInfo->pucInfoBuffer);
	prInitHifTxHeader->u2TxByteCount = prCmdInfo->u2InfoBufLen;
	prInitHifTxHeader->u2PQ_ID = (UINT_16)INIT_CMD_PDA_PQ_ID;

	prInitHifTxHeader->rInitWifiCmd.ucCID = 0U;
	prInitHifTxHeader->rInitWifiCmd.ucPktTypeID =
		(UINT_8)INIT_CMD_PDA_PACKET_TYPE_ID;
	prInitHifTxHeader->rInitWifiCmd.ucSeqNum = 0U;

	/* 3. Copy FW image scatter section */
	kalMemCopy(prInitHifTxHeader->rInitWifiCmd.aucBuffer, pucImgSecBuf, u4ImgSecSize);

	/* 4. Send FW image scatter section */
	if (nicTxInitCmd(prAdapter, prCmdInfo) != WLAN_STATUS_SUCCESS) {
		u4Status = WLAN_STATUS_FAILURE;
		DBGLOG(INIT, ERROR, "Fail to transmit FW image scatter section\n");
	}
	/* 5. Free CMD Info Packet. */
	cmdBufFreeCmdInfo(prAdapter, prCmdInfo);

	return u4Status;
}

/* for AOSP */
/*----------------------------------------------------------------------------*/
/*!
* @brief This function is called to download FW by ilm and dlm section.
*
* @param prAdapter      Pointer to the Adapter structure.
*
* @return (none)
*/
/*----------------------------------------------------------------------------*/

VOID
wlanFwDvdDwnloadHandler(IN P_ADAPTER_T prAdapter,
			IN P_FIRMWARE_DIVIDED_DOWNLOAD_T prFwHead, IN PVOID pvFwImageMapFile,
			OUT WLAN_STATUS *u4Status)
{
	UINT_32 u4ImgSecSize, i, j;

	for (i = 0U; i < prFwHead->u4NumOfEntries; i++) {
		if (wlanImageSectionConfig(prAdapter,
					   prFwHead->arSection[i].u4DestAddr,
					   prFwHead->arSection[i].u4Length,
					   i == 0U ? TRUE : FALSE,
#if defined(MT6631)
					   prFwHead->arSection[i].ucEnc ? TRUE : FALSE,
					   prFwHead->arSection[i].ucKIdx
#else
					   TRUE,
					   0
#endif
			) != WLAN_STATUS_SUCCESS) {
			DBGLOG(INIT, ERROR, "Firmware download configuration failed!\n");
			*u4Status = WLAN_STATUS_FAILURE;
			break;
		}

		for (j = 0U; j < prFwHead->arSection[i].u4Length;
			j += CMD_PKT_SIZE_FOR_IMAGE) {
			if (j + CMD_PKT_SIZE_FOR_IMAGE < prFwHead->arSection[i].u4Length)
				u4ImgSecSize = CMD_PKT_SIZE_FOR_IMAGE;
			else
				u4ImgSecSize = prFwHead->arSection[i].u4Length - j;

			if (wlanImageSectionDownload(prAdapter,
						     u4ImgSecSize,
						     (PUINT_8) pvFwImageMapFile +
						     prFwHead->arSection[i].u4Offset + j) !=
			    WLAN_STATUS_SUCCESS) {
				DBGLOG(INIT, ERROR, "Firmware scatter download failed!\n");
				*u4Status = WLAN_STATUS_FAILURE;
				break;
			}
		}

		/* escape from loop if any pending error occurs */
		if (*u4Status == WLAN_STATUS_FAILURE)
			break;
	}
}

/* for AOSP */
/*----------------------------------------------------------------------------*/
/*!
* @brief This function is called to download FW
*
* @param prAdapter      Pointer to the Adapter structure.
*
* @return (none)
*/
/*----------------------------------------------------------------------------*/

VOID
wlanFwDwnloadHandler(IN P_ADAPTER_T prAdapter,
		     IN UINT_32 u4FwImgLength, IN PVOID pvFwImageMapFile, OUT WLAN_STATUS *u4Status)
{
	UINT_32 u4ImgSecSize, i;

	for (i = 0U; i < u4FwImgLength; i += CMD_PKT_SIZE_FOR_IMAGE) {
		if (i + CMD_PKT_SIZE_FOR_IMAGE < u4FwImgLength)
			u4ImgSecSize = CMD_PKT_SIZE_FOR_IMAGE;
		else
			u4ImgSecSize = u4FwImgLength - i;

		if (wlanImageSectionDownload(prAdapter,
					     u4ImgSecSize, (PUINT_8) pvFwImageMapFile + i) != WLAN_STATUS_SUCCESS) {
			DBGLOG(INIT, ERROR, "wlanImageSectionDownload failed!\n");
			*u4Status = WLAN_STATUS_FAILURE;
			break;
		}
	}
}

#if !CFG_ENABLE_FW_DOWNLOAD_ACK
/*----------------------------------------------------------------------------*/
/*!
* @brief This function is called to confirm previously firmware download is done without error
*
* @param prAdapter      Pointer to the Adapter structure.
*
* @return (none)
*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS wlanImageQueryStatus(IN P_ADAPTER_T prAdapter)
{
	P_CMD_INFO_T prCmdInfo;
	P_INIT_HIF_TX_HEADER_T prInitHifTxHeader;
	UINT_8 aucBuffer[sizeof(INIT_HIF_RX_HEADER_T) + sizeof(INIT_EVENT_PENDING_ERROR)];
	UINT_32 u4RxPktLength;
	P_INIT_HIF_RX_HEADER_T prInitHifRxHeader;
	P_INIT_EVENT_PENDING_ERROR prEventPendingError;
	WLAN_STATUS u4Status = WLAN_STATUS_SUCCESS;
	UINT_8 ucTC, ucCmdSeqNum;

	ASSERT(prAdapter);

	DEBUGFUNC("wlanImageQueryStatus");

	/* 1. Allocate CMD Info Packet and it Buffer. */
	prCmdInfo = cmdBufAllocateCmdInfo(prAdapter,
		(UINT_32)sizeof(INIT_HIF_TX_HEADER_T));

	if (prCmdInfo == NULL) {
		DBGLOG(INIT, ERROR, "Allocate CMD_INFO_T ==> FAILED.\n");
		return WLAN_STATUS_FAILURE;
	}

	kalMemZero(prCmdInfo->pucInfoBuffer,
		(UINT_32)sizeof(INIT_HIF_TX_HEADER_T));
	prCmdInfo->u2InfoBufLen = (UINT_16)sizeof(INIT_HIF_TX_HEADER_T);

	/* 2. Use TC0's resource to download image. (only TC0 is allowed) */
	ucTC = (UINT_8)TC0_INDEX;

	/* 3. Increase command sequence number */
	ucCmdSeqNum = nicIncreaseCmdSeqNum(prAdapter);

	/* 4. Setup common CMD Info Packet */
	prInitHifTxHeader = (P_INIT_HIF_TX_HEADER_T) (prCmdInfo->pucInfoBuffer);

	prInitHifTxHeader->u2TxByteCount = prCmdInfo->u2InfoBufLen;
	prInitHifTxHeader->u2PQ_ID = (UINT_16)INIT_CMD_PQ_ID;

	prInitHifTxHeader->rInitWifiCmd.ucCID =
		(UINT_8)INIT_CMD_ID_QUERY_PENDING_ERROR;
	prInitHifTxHeader->rInitWifiCmd.ucPktTypeID =
		(UINT_8)INIT_CMD_PACKET_TYPE_ID;
	prInitHifTxHeader->rInitWifiCmd.ucSeqNum = ucCmdSeqNum;

	/* 5. Send Query pending error command */
	while (1 != 0) {
		/* 5.1 Acquire TX Resource */
		if (nicTxAcquireResource
		    (prAdapter, ucTC, nicTxGetPageCount(prCmdInfo->u2InfoBufLen, TRUE)) == WLAN_STATUS_RESOURCES) {
			if (nicTxPollingResource(prAdapter, ucTC) != WLAN_STATUS_SUCCESS) {
				u4Status = WLAN_STATUS_FAILURE;
				DBGLOG(INIT, ERROR, "Fail to get TX resource return within timeout\n");
				break;
			}
			continue;
		}
		/* 5.2 Send CMD Info Packet */
		if (nicTxInitCmd(prAdapter, prCmdInfo) != WLAN_STATUS_SUCCESS) {
			u4Status = WLAN_STATUS_FAILURE;
			DBGLOG(INIT, ERROR, "Fail to transmit query pending error command\n");
		}

		break;
	};

	/* 6. Wait for INIT_EVENT_ID_PENDING_ERROR */
	do {
		if (kalIsCardRemoved(prAdapter->prGlueInfo) == TRUE || fgIsBusAccessFailed == TRUE) {
			u4Status = WLAN_STATUS_FAILURE;
		} else if (nicRxWaitResponse(prAdapter,
				0,
				aucBuffer,
				(UINT_32)(sizeof(INIT_HIF_RX_HEADER_T) +
				sizeof(INIT_EVENT_PENDING_ERROR)),
				&u4RxPktLength) != WLAN_STATUS_SUCCESS) {
			UINT_32 u4MailBox0;
			UINT_32 u4MailBox1;

			nicGetMailbox(prAdapter, 0, &u4MailBox0);
			nicGetMailbox(prAdapter, 1, &u4MailBox1);
			DBGLOG(INIT, WARN, "Device to Host Mailbox 0x%08x, 0x%08x\n", u4MailBox0, u4MailBox1);
			u4Status = WLAN_STATUS_FAILURE;
		} else {
			prInitHifRxHeader = (P_INIT_HIF_RX_HEADER_T) aucBuffer;

			/* EID / SeqNum check */
			if (prInitHifRxHeader->rInitWifiEvent.ucEID != INIT_EVENT_ID_PENDING_ERROR) {
				DBGLOG(INIT, ERROR, "Unexpected Event ID %d! expect %d\n",
				       prInitHifRxHeader->rInitWifiEvent.ucEID, INIT_EVENT_ID_PENDING_ERROR);
				u4Status = WLAN_STATUS_FAILURE;
			} else if (prInitHifRxHeader->rInitWifiEvent.ucSeqNum != ucCmdSeqNum) {
				DBGLOG(INIT, ERROR, "Unexpected SeqNum %d! expect %d\n",
				       prInitHifRxHeader->rInitWifiEvent.ucSeqNum, ucCmdSeqNum);
				u4Status = WLAN_STATUS_FAILURE;
			} else {
				prEventPendingError =
				    (P_INIT_EVENT_PENDING_ERROR) (prInitHifRxHeader->rInitWifiEvent.aucBuffer);
				if (prEventPendingError->ucStatus != 0U) {
					/* 0 for download success */
					DBGLOG(INIT, ERROR, "Event status error %d!\n", prEventPendingError->ucStatus);
					u4Status = WLAN_STATUS_FAILURE;
				} else {
					u4Status = WLAN_STATUS_SUCCESS;
				}
			}
		}
	} while (FALSE);

	/* 7. Free CMD Info Packet. */
	cmdBufFreeCmdInfo(prAdapter, prCmdInfo);

	return u4Status;
}

#else
/*----------------------------------------------------------------------------*/
/*!
* @brief This function is called to confirm the status of
*        previously downloaded firmware scatter
*
* @param prAdapter      Pointer to the Adapter structure.
*        ucCmdSeqNum    Sequence number of previous firmware scatter
*
* @return WLAN_STATUS_SUCCESS
*         WLAN_STATUS_FAILURE
*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS wlanImageSectionDownloadStatus(IN P_ADAPTER_T prAdapter, IN UINT_8 ucCmdSeqNum)
{
	UINT_8 aucBuffer[sizeof(INIT_HIF_RX_HEADER_T) + sizeof(INIT_EVENT_CMD_RESULT)];
	P_INIT_HIF_RX_HEADER_T prInitHifRxHeader;
	P_INIT_EVENT_CMD_RESULT prEventCmdResult;
	UINT_32 u4RxPktLength;
	WLAN_STATUS u4Status;

	ASSERT(prAdapter);

	do {
		if (kalIsCardRemoved(prAdapter->prGlueInfo) == TRUE || fgIsBusAccessFailed == TRUE) {
			u4Status = WLAN_STATUS_FAILURE;
		} else if (nicRxWaitResponse(prAdapter,
				0,
				aucBuffer,
				(UINT_32)(sizeof(INIT_HIF_RX_HEADER_T) +
				sizeof(INIT_EVENT_CMD_RESULT)),
				&u4RxPktLength) != WLAN_STATUS_SUCCESS) {
			UINT_32 u4MailBox0;
			UINT_32 u4MailBox1;

			nicGetMailbox(prAdapter, 0, &u4MailBox0);
			nicGetMailbox(prAdapter, 1, &u4MailBox1);
			DBGLOG(INIT, WARN, "Device to Host Mailbox 0x%08x, 0x%08x\n", u4MailBox0, u4MailBox1);
			u4Status = WLAN_STATUS_FAILURE;
		} else {
			prInitHifRxHeader = (P_INIT_HIF_RX_HEADER_T) aucBuffer;

			/* EID / SeqNum check */
			if (prInitHifRxHeader->rInitWifiEvent.ucEID !=
				(UINT_8)INIT_EVENT_ID_CMD_RESULT) {
				DBGLOG(INIT, ERROR, "Unexpected Event ID %d! expect %d\n",
				       prInitHifRxHeader->rInitWifiEvent.ucEID, INIT_EVENT_ID_CMD_RESULT);
				u4Status = WLAN_STATUS_FAILURE;
			} else if (prInitHifRxHeader->rInitWifiEvent.ucSeqNum != ucCmdSeqNum) {
				DBGLOG(INIT, ERROR, "Unexpected SeqNum %d! expect %d\n",
				       prInitHifRxHeader->rInitWifiEvent.ucSeqNum, ucCmdSeqNum);
				u4Status = WLAN_STATUS_FAILURE;
			} else {
				prEventCmdResult =
				    (P_INIT_EVENT_CMD_RESULT) (prInitHifRxHeader->rInitWifiEvent.aucBuffer);
				if (prEventCmdResult->ucStatus != 0U) {
					/* 0 for download success */
					DBGLOG(INIT, ERROR, "Event status error %d!\n", prEventCmdResult->ucStatus);
					u4Status = WLAN_STATUS_FAILURE;
				} else {
					u4Status = WLAN_STATUS_SUCCESS;
				}
			}
		}
	} while (0 != 0);

	return u4Status;
}

#endif
/*----------------------------------------------------------------------------*/
/*!
* @brief This function is called to start FW normal operation.
*
* @param prAdapter      Pointer to the Adapter structure.
*
* @return (none)
*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS wlanConfigWifiFunc(IN P_ADAPTER_T prAdapter, IN BOOLEAN fgEnable, IN UINT_32 u4StartAddress)
{
	P_CMD_INFO_T prCmdInfo;
	P_INIT_HIF_TX_HEADER_T prInitHifTxHeader;
	P_INIT_CMD_WIFI_START prInitCmdWifiStart;
	UINT_8 ucTC, ucCmdSeqNum;
	WLAN_STATUS u4Status = WLAN_STATUS_SUCCESS;

	ASSERT(prAdapter);

	DEBUGFUNC("wlanConfigWifiFunc");

	/* 1. Allocate CMD Info Packet and its Buffer. */
	prCmdInfo = cmdBufAllocateCmdInfo(prAdapter,
		(UINT_32)(sizeof(INIT_HIF_TX_HEADER_T) +
		sizeof(INIT_CMD_WIFI_START)));

	if (prCmdInfo == NULL) {
		DBGLOG(INIT, ERROR, "Allocate CMD_INFO_T ==> FAILED.\n");
		return WLAN_STATUS_FAILURE;
	}

	kalMemZero(prCmdInfo->pucInfoBuffer,
		(UINT_32)(sizeof(INIT_HIF_TX_HEADER_T) +
		sizeof(INIT_CMD_WIFI_START)));
	prCmdInfo->u2InfoBufLen = (UINT_16)(sizeof(INIT_HIF_TX_HEADER_T)
		+ sizeof(INIT_CMD_WIFI_START));

	/* 2. Always use TC0 */
	ucTC = (UINT_8)TC0_INDEX;

	/* 3. Increase command sequence number */
	ucCmdSeqNum = nicIncreaseCmdSeqNum(prAdapter);

	/* 4. Setup common CMD Info Packet */
	prInitHifTxHeader = (P_INIT_HIF_TX_HEADER_T) (prCmdInfo->pucInfoBuffer);
	prInitHifTxHeader->u2TxByteCount = prCmdInfo->u2InfoBufLen;
	prInitHifTxHeader->u2PQ_ID = (UINT_16)INIT_CMD_PQ_ID;

	prInitHifTxHeader->rInitWifiCmd.ucCID = (UINT_8)INIT_CMD_ID_WIFI_START;
	prInitHifTxHeader->rInitWifiCmd.ucPktTypeID =
		(UINT_8)INIT_CMD_PACKET_TYPE_ID;
	prInitHifTxHeader->rInitWifiCmd.ucSeqNum = ucCmdSeqNum;

	prInitCmdWifiStart = (P_INIT_CMD_WIFI_START) (prInitHifTxHeader->rInitWifiCmd.aucBuffer);
	prInitCmdWifiStart->u4Override = (UINT_32)(fgEnable == TRUE ? 1 : 0);
	prInitCmdWifiStart->u4Address = u4StartAddress;

	/* 5. Send WIFI start command */
	while (1U != 0U) {
		/* 5.1 Acquire TX Resource */
		if (nicTxAcquireResource
		    (prAdapter, ucTC, nicTxGetPageCount(prCmdInfo->u2InfoBufLen, TRUE)) == WLAN_STATUS_RESOURCES) {
			if (nicTxPollingResource(prAdapter, ucTC) != WLAN_STATUS_SUCCESS) {
				u4Status = WLAN_STATUS_FAILURE;
				DBGLOG(INIT, ERROR, "Fail to get TX resource return within timeout\n");
				break;
			}
			continue;
		}
		/* 5.2 Send CMD Info Packet */
		if (nicTxInitCmd(prAdapter, prCmdInfo) != WLAN_STATUS_SUCCESS) {
			u4Status = WLAN_STATUS_FAILURE;
			DBGLOG(INIT, ERROR, "Fail to transmit WIFI start command\n");
		}

		break;
	};

	/* 6. Free CMD Info Packet. */
	cmdBufFreeCmdInfo(prAdapter, prCmdInfo);

	return u4Status;
}


const UINT_32 crc32_ccitt_table[256] = {
	0x00000000U, 0x77073096U, 0xee0e612cU, 0x990951baU, 0x076dc419U,
	0x706af48fU, 0xe963a535U, 0x9e6495a3U, 0x0edb8832U, 0x79dcb8a4U,
	0xe0d5e91eU, 0x97d2d988U, 0x09b64c2bU, 0x7eb17cbdU, 0xe7b82d07U,
	0x90bf1d91U, 0x1db71064U, 0x6ab020f2U, 0xf3b97148U, 0x84be41deU,
	0x1adad47dU, 0x6ddde4ebU, 0xf4d4b551U, 0x83d385c7U, 0x136c9856U,
	0x646ba8c0U, 0xfd62f97aU, 0x8a65c9ecU, 0x14015c4fU, 0x63066cd9U,
	0xfa0f3d63U, 0x8d080df5U, 0x3b6e20c8U, 0x4c69105eU, 0xd56041e4U,
	0xa2677172U, 0x3c03e4d1U, 0x4b04d447U, 0xd20d85fdU, 0xa50ab56bU,
	0x35b5a8faU, 0x42b2986cU, 0xdbbbc9d6U, 0xacbcf940U, 0x32d86ce3U,
	0x45df5c75U, 0xdcd60dcfU, 0xabd13d59U, 0x26d930acU, 0x51de003aU,
	0xc8d75180U, 0xbfd06116U, 0x21b4f4b5U, 0x56b3c423U, 0xcfba9599U,
	0xb8bda50fU, 0x2802b89eU, 0x5f058808U, 0xc60cd9b2U, 0xb10be924U,
	0x2f6f7c87U, 0x58684c11U, 0xc1611dabU, 0xb6662d3dU, 0x76dc4190U,
	0x01db7106U, 0x98d220bcU, 0xefd5102aU, 0x71b18589U, 0x06b6b51fU,
	0x9fbfe4a5U, 0xe8b8d433U, 0x7807c9a2U, 0x0f00f934U, 0x9609a88eU,
	0xe10e9818U, 0x7f6a0dbbU, 0x086d3d2dU, 0x91646c97U, 0xe6635c01U,
	0x6b6b51f4U, 0x1c6c6162U, 0x856530d8U, 0xf262004eU, 0x6c0695edU,
	0x1b01a57bU, 0x8208f4c1U, 0xf50fc457U, 0x65b0d9c6U, 0x12b7e950U,
	0x8bbeb8eaU, 0xfcb9887cU, 0x62dd1ddfU, 0x15da2d49U, 0x8cd37cf3U,
	0xfbd44c65U, 0x4db26158U, 0x3ab551ceU, 0xa3bc0074U, 0xd4bb30e2U,
	0x4adfa541U, 0x3dd895d7U, 0xa4d1c46dU, 0xd3d6f4fbU, 0x4369e96aU,
	0x346ed9fcU, 0xad678846U, 0xda60b8d0U, 0x44042d73U, 0x33031de5U,
	0xaa0a4c5fU, 0xdd0d7cc9U, 0x5005713cU, 0x270241aaU, 0xbe0b1010U,
	0xc90c2086U, 0x5768b525U, 0x206f85b3U, 0xb966d409U, 0xce61e49fU,
	0x5edef90eU, 0x29d9c998U, 0xb0d09822U, 0xc7d7a8b4U, 0x59b33d17U,
	0x2eb40d81U, 0xb7bd5c3bU, 0xc0ba6cadU, 0xedb88320U, 0x9abfb3b6U,
	0x03b6e20cU, 0x74b1d29aU, 0xead54739U, 0x9dd277afU, 0x04db2615U,
	0x73dc1683U, 0xe3630b12U, 0x94643b84U, 0x0d6d6a3eU, 0x7a6a5aa8U,
	0xe40ecf0bU, 0x9309ff9dU, 0x0a00ae27U, 0x7d079eb1U, 0xf00f9344U,
	0x8708a3d2U, 0x1e01f268U, 0x6906c2feU, 0xf762575dU, 0x806567cbU,
	0x196c3671U, 0x6e6b06e7U, 0xfed41b76U, 0x89d32be0U, 0x10da7a5aU,
	0x67dd4accU, 0xf9b9df6fU, 0x8ebeeff9U, 0x17b7be43U, 0x60b08ed5U,
	0xd6d6a3e8U, 0xa1d1937eU, 0x38d8c2c4U, 0x4fdff252U, 0xd1bb67f1U,
	0xa6bc5767U, 0x3fb506ddU, 0x48b2364bU, 0xd80d2bdaU, 0xaf0a1b4cU,
	0x36034af6U, 0x41047a60U, 0xdf60efc3U, 0xa867df55U, 0x316e8eefU,
	0x4669be79U, 0xcb61b38cU, 0xbc66831aU, 0x256fd2a0U, 0x5268e236U,
	0xcc0c7795U, 0xbb0b4703U, 0x220216b9U, 0x5505262fU, 0xc5ba3bbeU,
	0xb2bd0b28U, 0x2bb45a92U, 0x5cb36a04U, 0xc2d7ffa7U, 0xb5d0cf31U,
	0x2cd99e8bU, 0x5bdeae1dU, 0x9b64c2b0U, 0xec63f226U, 0x756aa39cU,
	0x026d930aU, 0x9c0906a9U, 0xeb0e363fU, 0x72076785U, 0x05005713U,
	0x95bf4a82U, 0xe2b87a14U, 0x7bb12baeU, 0x0cb61b38U, 0x92d28e9bU,
	0xe5d5be0dU, 0x7cdcefb7U, 0x0bdbdf21U, 0x86d3d2d4U, 0xf1d4e242U,
	0x68ddb3f8U, 0x1fda836eU, 0x81be16cdU, 0xf6b9265bU, 0x6fb077e1U,
	0x18b74777U, 0x88085ae6U, 0xff0f6a70U, 0x66063bcaU, 0x11010b5cU,
	0x8f659effU, 0xf862ae69U, 0x616bffd3U, 0x166ccf45U, 0xa00ae278U,
	0xd70dd2eeU, 0x4e048354U, 0x3903b3c2U, 0xa7672661U, 0xd06016f7U,
	0x4969474dU, 0x3e6e77dbU, 0xaed16a4aU, 0xd9d65adcU, 0x40df0b66U,
	0x37d83bf0U, 0xa9bcae53U, 0xdebb9ec5U, 0x47b2cf7fU, 0x30b5ffe9U,
	0xbdbdf21cU, 0xcabac28aU, 0x53b39330U, 0x24b4a3a6U, 0xbad03605U,
	0xcdd70693U, 0x54de5729U, 0x23d967bfU, 0xb3667a2eU, 0xc4614ab8U,
	0x5d681b02U, 0x2a6f2b94U, 0xb40bbe37U, 0xc30c8ea1U, 0x5a05df1bU,
	0x2d02ef8dU
};

/*----------------------------------------------------------------------------*/
/*!
* @brief This function is used to generate CRC32 checksum
*
* @param buf Pointer to the data.
* @param len data length
*
* @return crc32 value
*/
/*----------------------------------------------------------------------------*/
UINT_32 wlanCRC32(PUINT_8 buf, UINT_32 len)
{
	UINT_32 i, crc32 = 0xFFFFFFFFU;

	for (i = 0U; i < len; i++)
		crc32 = crc32_ccitt_table[(crc32 ^ buf[i]) & 0xffU]
		^ (crc32 >> 8U);

	return ~crc32;
}
#endif

/*----------------------------------------------------------------------------*/
/*!
* @brief This function is called to process queued RX packets
*
* @param prAdapter          Pointer to the Adapter structure.
*        prSwRfbListHead    Pointer to head of RX packets link list
*
* @return (none)
*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS wlanProcessQueuedSwRfb(IN P_ADAPTER_T prAdapter, IN P_SW_RFB_T prSwRfbListHead)
{
	P_SW_RFB_T prSwRfb, prNextSwRfb;
	P_TX_CTRL_T prTxCtrl;
	P_RX_CTRL_T prRxCtrl;
	P_STA_RECORD_T prStaRec;

	ASSERT(prAdapter);
	ASSERT(prSwRfbListHead);

	prTxCtrl = &prAdapter->rTxCtrl;
	prRxCtrl = &prAdapter->rRxCtrl;

	prSwRfb = prSwRfbListHead;

	do {
		/* save next first */
		prNextSwRfb = (P_SW_RFB_T) QUEUE_GET_NEXT_ENTRY((P_QUE_ENTRY_T) prSwRfb);

		switch (prSwRfb->eDst) {
		case RX_PKT_DESTINATION_HOST:
			prStaRec = cnmGetStaRecByIndex(prAdapter, prSwRfb->ucStaRecIdx);
			if (prStaRec != NULL && IS_STA_IN_AIS(prStaRec)) {
#if ARP_MONITER_ENABLE
				qmHandleRxArpPackets(prAdapter, prSwRfb);
#endif
			}
			nicRxProcessPktWithoutReorder(prAdapter, prSwRfb);
			break;

		case RX_PKT_DESTINATION_FORWARD:
			nicRxProcessForwardPkt(prAdapter, prSwRfb);
			break;

		case RX_PKT_DESTINATION_HOST_WITH_FORWARD:
			nicRxProcessGOBroadcastPkt(prAdapter, prSwRfb);
			break;

		case RX_PKT_DESTINATION_NULL:
			nicRxReturnRFB(prAdapter, prSwRfb);
			break;

		default:
			break;
		}

#if CFG_HIF_RX_STARVATION_WARNING
		prRxCtrl->u4DequeuedCnt++;
#endif
		prSwRfb = prNextSwRfb;
	} while (prSwRfb != NULL);

	return WLAN_STATUS_SUCCESS;
}

/*----------------------------------------------------------------------------*/
/*!
* @brief This function is called to purge queued TX packets
*        by indicating failure to OS and returned to free list
*
* @param prAdapter          Pointer to the Adapter structure.
*        prMsduInfoListHead Pointer to head of TX packets link list
*
* @return (none)
*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS wlanProcessQueuedMsduInfo(IN P_ADAPTER_T prAdapter, IN P_MSDU_INFO_T prMsduInfoListHead)
{
	ASSERT(prAdapter);
	ASSERT(prMsduInfoListHead);

	nicTxFreeMsduInfoPacket(prAdapter, prMsduInfoListHead);
	nicTxReturnMsduInfo(prAdapter, prMsduInfoListHead);

	return WLAN_STATUS_SUCCESS;
}

/*----------------------------------------------------------------------------*/
/*!
* @brief This function is called to check if the OID handler needs timeout
*
* @param prAdapter          Pointer to the Adapter structure.
*        pfnOidHandler      Pointer to the OID handler
*
* @return TRUE
*         FALSE
*/
/*----------------------------------------------------------------------------*/
BOOLEAN wlanoidTimeoutCheck(IN P_ADAPTER_T prAdapter, IN PFN_OID_HANDLER_FUNC pfnOidHandler)
{
	PFN_OID_HANDLER_FUNC *apfnOidHandlerWOTimeoutCheck;
	UINT_32 i;
	UINT_32 u4NumOfElem;
	UINT_32 u4OidTimeout;

	apfnOidHandlerWOTimeoutCheck = apfnOidWOTimeoutCheck;
	u4NumOfElem = (UINT_32)sizeof(apfnOidWOTimeoutCheck) /
		(UINT_32)sizeof(PFN_OID_HANDLER_FUNC);

	for (i = 0U; i < u4NumOfElem; i++) {
		if (apfnOidHandlerWOTimeoutCheck[i] == pfnOidHandler)
			return FALSE;
	}

	/* Decrease OID timeout threshold if chip NoAck/resetting */
	if (wlanIsChipNoAck(prAdapter) == TRUE) {
		u4OidTimeout = WLAN_OID_TIMEOUT_THRESHOLD_IN_RESETTING;
		DBGLOG(INIT, INFO, "Decrease OID timeout to %ums due to NoACK/CHIP-RESET\n", u4OidTimeout);
	} else {
		u4OidTimeout = WLAN_OID_TIMEOUT_THRESHOLD;
	}

	/* Set OID timer for timeout check */
	cnmTimerStartTimer(prAdapter, &(prAdapter->rOidTimeoutTimer), u4OidTimeout);

	return TRUE;
}

/*----------------------------------------------------------------------------*/
/*!
* @brief This function is called to clear any pending OID timeout check
*
* @param prAdapter          Pointer to the Adapter structure.
*
* @return none
*/
/*----------------------------------------------------------------------------*/
VOID wlanoidClearTimeoutCheck(IN P_ADAPTER_T prAdapter)
{
	ASSERT(prAdapter);

	cnmTimerStopTimer(prAdapter, &(prAdapter->rOidTimeoutTimer));
}

/*----------------------------------------------------------------------------*/
/*!
* @brief This function is called to override network address
*        if NVRAM has a valid value
*
* @param prAdapter          Pointer to the Adapter structure.
*
* @return WLAN_STATUS_FAILURE   The request could not be processed
*         WLAN_STATUS_PENDING   The request has been queued for later processing
*         WLAN_STATUS_SUCCESS   The request has been processed
*/
/*----------------------------------------------------------------------------*/
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 9, 0))
WLAN_STATUS wlanUpdateNetworkAddress(IN P_ADAPTER_T prAdapter)
{

	extern UINT_32 MetaZone_ReadBinary(UINT_32 u4Idx, UCHAR *pbData, UINT_32 u4Sie);
	extern UINT_32 MetaZone_WriteBinary(UINT_32 u4Idx, UCHAR *pbData, UINT_32 u4Sie);
	const UINT_8 aucZeroMacAddr[] = NULL_MAC_ADDR;
	PARAM_MAC_ADDRESS rMacAddr = { 0 };
	UINT_32 u4SysTime;

	struct file *fileAddr = NULL;
	UINT_32   u4Size;
	DEBUGFUNC("wlanUpdateNetworkAddress");

	ASSERT(prAdapter);

	if (kalRetrieveNetworkAddress(prAdapter->prGlueInfo, &rMacAddr) == FALSE
		|| IS_BMCAST_MAC_ADDR(rMacAddr)
		|| EQUAL_MAC_ADDR(aucZeroMacAddr, rMacAddr)) {

		/* eFUSE has a valid address, don't do anything */
		if (prAdapter->fgIsEmbbededMacAddrValid == TRUE) {
#if CFG_SHOW_MACADDR_SOURCE
			DBGLOG(INIT, INFO, "Using embedded MAC address.\n");
#endif
			return WLAN_STATUS_SUCCESS;
		} else {
#if CFG_SHOW_MACADDR_SOURCE
			DBGLOG(INIT, INFO, "Using dynamically generated MAC address.\n");
#endif

			UINT_32 metaIndex = 0x10026;
			UINT_8 readData[6];
			UINT_32 metaSize = 6;
			readData[0] = 0x0;
			readData[1] = 0x0;
			readData[2] = 0x0;
			readData[3] = 0x0;
			readData[4] = 0x0;
			readData[5] = 0x0;


			MetaZone_ReadBinary(metaIndex, readData, metaSize);
			printk(KERN_ALERT"----metaZone read[0]:%x\n", readData[0]);
			printk(KERN_ALERT"----metaZone read[1]:%x\n", readData[1]);
			printk(KERN_ALERT"----metaZone read[2]:%x\n", readData[2]);
			printk(KERN_ALERT"----metaZone read[3]:%x\n", readData[3]);
			printk(KERN_ALERT"----metaZone read[4]:%x\n", readData[4]);
			printk(KERN_ALERT"----metaZone read[5]:%x\n", readData[5]);

			if(!((0==readData[0])&&(0==readData[1])&&(0==readData[2])&&(0==readData[3])&&(0==readData[4])&&(0==readData[5]))) {
				printk(KERN_ALERT"----metaZone read macAddress Success!\n");
				rMacAddr[0] = readData[0];
				rMacAddr[1] = readData[1];
				rMacAddr[2] = readData[2];
				rMacAddr[3] = readData[3];
				rMacAddr[4] = readData[4];
				rMacAddr[5] = readData[5];
			} else {
				fileAddr = filp_open("/data/misc/wifi/wifimacaddr", O_RDONLY, 0);
				if (IS_ERR(fileAddr)) {
					DBGLOG(INIT, INFO, "/data/misc/wifi/wifimacaddr is not exist\n");
					printk(KERN_ALERT"/data/misc/wifi/wifimacaddr is not exist\n");
					/* dynamic generate */
					u4SysTime = (UINT_32) kalGetTimeTick();

					rMacAddr[0] = 0x00;
					rMacAddr[1] = 0x08;
					rMacAddr[2] = 0x22;
					kalMemCopy(&rMacAddr[3], &u4SysTime, 3);
					fileAddr = filp_open("/data/misc/wifi/wifimacaddr", O_CREAT|O_RDWR, S_IRWXU);
					if (IS_ERR(fileAddr)) {
						DBGLOG(INIT, INFO, "create macaddr file fail\n");
						printk(KERN_ALERT"create macaddr file fail\n");
						//return WLAN_STATUS_FAILURE;
					} else {
						DBGLOG(INIT, INFO, "create macaddr file Success\n");
						printk(KERN_ALERT"create macaddr file Success\n");
						fileAddr->f_pos = 0;
						fileAddr->f_op->write(fileAddr, rMacAddr, sizeof(PARAM_MAC_ADDRESS), &fileAddr->f_pos);
					}
				} else {
					DBGLOG(INIT, INFO, "/data/misc/wifi/wifimacaddr is  exist.\n");
					printk(KERN_ALERT"/data/misc/wifi/wifimacaddr is  exist.\n");
					if ((fileAddr == NULL) || IS_ERR(fileAddr) || (fileAddr->f_op == NULL) || (fileAddr->f_op->read == NULL)) {
						DBGLOG(INIT, ERROR, " can't read MAC Addr from file.\n");
						//return WLAN_STATUS_FAILURE;
					} else {
						fileAddr->f_pos = 0;
						u4Size = fileAddr->f_op->read(fileAddr, rMacAddr, sizeof(PARAM_MAC_ADDRESS), &fileAddr->f_pos);
					}
					if (IS_BMCAST_MAC_ADDR(rMacAddr) || EQUAL_MAC_ADDR(aucZeroMacAddr,rMacAddr)) {
						/* dynamic generate */
						printk(KERN_ALERT"MAC Addr unuseful,Dynamic generate and write to file\n");
						u4SysTime = (UINT_32) kalGetTimeTick();
						rMacAddr[0] = 0x00;
						rMacAddr[1] = 0x08;
						rMacAddr[2] = 0x22;
						kalMemCopy(&rMacAddr[3], &u4SysTime, 3);
						fileAddr->f_pos = 0;
						fileAddr->f_op->write(fileAddr, rMacAddr, sizeof(PARAM_MAC_ADDRESS), &fileAddr->f_pos);
					}
				}

				if ((fileAddr != NULL) && !IS_ERR(fileAddr)) {
					filp_close(fileAddr, NULL);
					fileAddr = NULL;
				}
			}
		}
#if 0
	    rMacAddr[0] = 0x00;
        rMacAddr[1] = 0x08;
        rMacAddr[2] = 0x22;
	    rMacAddr[3] = 0x11;
        rMacAddr[4] = 0x11;
        rMacAddr[5] = 0x11;
#endif

            //kalMemCopy(&rMacAddr[3], &u4SysTime, 3);

	} else {
#if CFG_SHOW_MACADDR_SOURCE
        DBGLOG(INIT, INFO, "Using host-supplied MAC address.\n");
#endif
	}

	COPY_MAC_ADDR(prAdapter->rWifiVar.aucMacAddress, rMacAddr);

	return WLAN_STATUS_SUCCESS;
}
#else
WLAN_STATUS wlanUpdateNetworkAddress(IN P_ADAPTER_T prAdapter)
{
#define CFG_UNDYNAMIC_MAC_ADDRESS     1//ATC
#if CFG_UNDYNAMIC_MAC_ADDRESS//ATC
#define MTZ_WRITE_FLAG_NONE          (0)
#define MTZ_WRITE_FLAG_FACTORYREST   (1)
	extern UINT_32 MetaZone_ReadBinary(UINT_32 u4Idx, UCHAR *pbData, UINT_32 u4Sie);
	extern UINT_32 MetaZone_SpecWriteBinary(UINT_32 u4Idx, UCHAR *pbData, UINT_32 u4Sie, UINT_32 u4Flag);
	extern UINT_32 MetaZone_Flush(int fgblock);
#endif
	const UINT_8 aucZeroMacAddr[] = NULL_MAC_ADDR;
	PARAM_MAC_ADDRESS rMacAddr = {0};
	UINT_32 u4SysTime;

	DEBUGFUNC("wlanUpdateNetworkAddress");

	ASSERT(prAdapter);

	if (kalRetrieveNetworkAddress(prAdapter->prGlueInfo, &rMacAddr) == FALSE
	    || IS_BMCAST_MAC_ADDR(rMacAddr)
	    || EQUAL_MAC_ADDR(aucZeroMacAddr, rMacAddr)) {

		/* eFUSE has a valid address, don't do anything */
		if (prAdapter->fgIsEmbbededMacAddrValid == TRUE) {
#if CFG_SHOW_MACADDR_SOURCE
			DBGLOG(INIT, LOUD, "Using embedded MAC address.\n");
#endif
			return WLAN_STATUS_SUCCESS;
		} else {

#if CFG_UNDYNAMIC_MAC_ADDRESS//ATC

			UINT_32 metaIndex = 0x10001;
			UINT_8 readData[6];
			UINT_32 metaSize = 6;
			readData[0] = 0x0;
			readData[1] = 0x0;
			readData[2] = 0x0;
			readData[3] = 0x0;
			readData[4] = 0x0;
			readData[5] = 0x0;

			MetaZone_ReadBinary(metaIndex, readData, metaSize);
			printk(KERN_ALERT"----MetaZone read[0]:%02x\n", readData[0]);
			printk(KERN_ALERT"----MetaZone read[1]:%02x\n", readData[1]);
			printk(KERN_ALERT"----MetaZone read[2]:%02x\n", readData[2]);
			printk(KERN_ALERT"----MetaZone read[3]:%02x\n", readData[3]);
			printk(KERN_ALERT"----MetaZone read[4]:%02x\n", readData[4]);
			printk(KERN_ALERT"----MetaZone read[5]:%02x\n", readData[5]);

			if(IS_BMCAST_MAC_ADDR(readData) || EQUAL_MAC_ADDR(aucZeroMacAddr, readData)) {
				/* dynamic generate */
				printk(KERN_ALERT"Dynamic generate MAC Addr and save to MetaZone\n");
				u4SysTime = (UINT_32) kalGetTimeTick();
				rMacAddr[0] = 0x00;
				rMacAddr[1] = 0x08;
				rMacAddr[2] = 0x22;
				kalMemCopy(&rMacAddr[3], &u4SysTime, 3);
				MetaZone_SpecWriteBinary(metaIndex, rMacAddr, metaSize, MTZ_WRITE_FLAG_FACTORYREST);
				MetaZone_Flush(0);
			} else {
				printk(KERN_ALERT"MetaZone read MAC Addr Success\n");
				rMacAddr[0] = readData[0];
				rMacAddr[1] = readData[1];
				rMacAddr[2] = readData[2];
				rMacAddr[3] = readData[3];
				rMacAddr[4] = readData[4];
				rMacAddr[5] = readData[5];
			}
			
#else

			printk(KERN_ALERT"Using dynamic MAC address\n");
#if CFG_SHOW_MACADDR_SOURCE
			DBGLOG(INIT, LOUD, "Using dynamically generated MAC address.\n");
#endif
			u4SysTime = (UINT_32) kalGetTimeTick();
			rMacAddr[0] = 0x00;
			rMacAddr[1] = 0x08;
			rMacAddr[2] = 0x22;
			kalMemCopy(&rMacAddr[3], &u4SysTime, 3);
#endif
		}
	} else {
#if CFG_SHOW_MACADDR_SOURCE
        DBGLOG(INIT, LOUD, "Using host-supplied MAC address from NVRAM.\n");
#endif
	}

	COPY_MAC_ADDR(prAdapter->rWifiVar.aucMacAddress, rMacAddr);

	return WLAN_STATUS_SUCCESS;
}
#endif

/*----------------------------------------------------------------------------*/
/*!
* @brief This function is called to update basic configuration into firmware domain
*
* @param prAdapter          Pointer to the Adapter structure.
*
* @return WLAN_STATUS_FAILURE   The request could not be processed
*         WLAN_STATUS_PENDING   The request has been queued for later processing
*         WLAN_STATUS_SUCCESS   The request has been processed
*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS wlanUpdateBasicConfig(IN P_ADAPTER_T prAdapter)
{
	UINT_8 ucCmdSeqNum;
	P_CMD_INFO_T prCmdInfo;
	P_WIFI_CMD_T prWifiCmd;
	P_CMD_BASIC_CONFIG_T prCmdBasicConfig;

	DEBUGFUNC("wlanUpdateBasicConfig");

	ASSERT(prAdapter);

	prCmdInfo = cmdBufAllocateCmdInfo(prAdapter,
		(UINT_32)CMD_HDR_SIZE + (UINT_32)sizeof(CMD_BASIC_CONFIG_T));

	if (prCmdInfo == NULL) {
		DBGLOG(INIT, ERROR, "Allocate CMD_INFO_T ==> FAILED.\n");
		return WLAN_STATUS_FAILURE;
	}
	/* increase command sequence number */
	ucCmdSeqNum = nicIncreaseCmdSeqNum(prAdapter);

	/* compose CMD_BUILD_CONNECTION cmd pkt */
	prCmdInfo->eCmdType = COMMAND_TYPE_GENERAL_IOCTL;
	prCmdInfo->u2InfoBufLen =
		(UINT_16)(CMD_HDR_SIZE + sizeof(CMD_BASIC_CONFIG_T));
	prCmdInfo->pfCmdDoneHandler = NULL;
	prCmdInfo->pfCmdTimeoutHandler = NULL;
	prCmdInfo->fgIsOid = FALSE;
	prCmdInfo->ucCID = (UINT_8)CMD_ID_BASIC_CONFIG;
	prCmdInfo->fgSetQuery = TRUE;
	prCmdInfo->fgNeedResp = FALSE;
	prCmdInfo->fgDriverDomainMCR = FALSE;
	prCmdInfo->ucCmdSeqNum = ucCmdSeqNum;
	prCmdInfo->u4SetInfoLen = (UINT_32)sizeof(CMD_BASIC_CONFIG_T);

	/* Setup WIFI_CMD_T */
	prWifiCmd = (P_WIFI_CMD_T) (prCmdInfo->pucInfoBuffer);
	prWifiCmd->u2TxByteCount = prCmdInfo->u2InfoBufLen;
	prWifiCmd->u2PQ_ID = (UINT_16)CMD_PQ_ID;
	prWifiCmd->ucPktTypeID = (UINT_8)CMD_PACKET_TYPE_ID;
	prWifiCmd->ucCID = prCmdInfo->ucCID;
	prWifiCmd->ucSetQuery = prCmdInfo->fgSetQuery;
	prWifiCmd->ucSeqNum = prCmdInfo->ucCmdSeqNum;

	/* configure CMD_BASIC_CONFIG */
	prCmdBasicConfig = (P_CMD_BASIC_CONFIG_T) (prWifiCmd->aucBuffer);
	prCmdBasicConfig->ucNative80211 = 0U;
	prCmdBasicConfig->rCsumOffload.u2RxChecksum = 0U;
	prCmdBasicConfig->rCsumOffload.u2TxChecksum = 0U;

#if CFG_TCP_IP_CHKSUM_OFFLOAD
	if ((prAdapter->u4CSUMFlags & CSUM_OFFLOAD_EN_TX_TCP) != 0U)
		prCmdBasicConfig->rCsumOffload.u2TxChecksum |= (UINT_16)BIT(2U);

	if ((prAdapter->u4CSUMFlags & CSUM_OFFLOAD_EN_TX_UDP) != 0U)
		prCmdBasicConfig->rCsumOffload.u2TxChecksum |= (UINT_16)BIT(1U);

	if ((prAdapter->u4CSUMFlags & CSUM_OFFLOAD_EN_TX_IP) != 0U)
		prCmdBasicConfig->rCsumOffload.u2TxChecksum |= (UINT_16)BIT(0U);

	if ((prAdapter->u4CSUMFlags & CSUM_OFFLOAD_EN_RX_TCP) != 0U)
		prCmdBasicConfig->rCsumOffload.u2RxChecksum |= (UINT_16)BIT(2U);

	if ((prAdapter->u4CSUMFlags & CSUM_OFFLOAD_EN_RX_UDP) != 0U)
		prCmdBasicConfig->rCsumOffload.u2RxChecksum |= (UINT_16)BIT(1U);

	if ((prAdapter->u4CSUMFlags &
		(CSUM_OFFLOAD_EN_RX_IPv4 | CSUM_OFFLOAD_EN_RX_IPv6)) != 0U)
		prCmdBasicConfig->rCsumOffload.u2RxChecksum |= (UINT_16)BIT(0U);
#endif

	if (wlanSendCommand(prAdapter, prCmdInfo) == WLAN_STATUS_RESOURCES) {
		kalEnqueueCommand(prAdapter->prGlueInfo, (P_QUE_ENTRY_T) prCmdInfo);
		return WLAN_STATUS_PENDING;
	}

	cmdBufFreeCmdInfo(prAdapter, prCmdInfo);
		return WLAN_STATUS_SUCCESS;
}

/*----------------------------------------------------------------------------*/
/*!
* @brief This function is called to check if the device is in RF test mode
*
* @param pfnOidHandler      Pointer to the OID handler
*
* @return TRUE
*         FALSE
*/
/*----------------------------------------------------------------------------*/
BOOLEAN wlanQueryTestMode(IN P_ADAPTER_T prAdapter)
{
	ASSERT(prAdapter);

	return prAdapter->fgTestMode;
}

BOOLEAN wlanProcessTxFrame(IN P_ADAPTER_T prAdapter, IN P_NATIVE_PACKET prPacket)
{
	UINT_32 u4SysTime;
	UINT_8 ucMacHeaderLen;
	TX_PACKET_INFO rTxPacketInfo;
	P_BSS_INFO_T prBssInfo;

	ASSERT(prAdapter);
	ASSERT(prPacket);

	if (kalQoSFrameClassifierAndPacketInfo(prAdapter->prGlueInfo,
		prPacket, &rTxPacketInfo) == TRUE) {

		/* Save the value of Priority Parameter */
		GLUE_SET_PKT_TID(prPacket, rTxPacketInfo.ucPriorityParam);

#if 1
		if (rTxPacketInfo.u2Flag != 0U) {
			if ((rTxPacketInfo.u2Flag &
				BIT((UINT_8)ENUM_PKT_1X)) != 0U) {
				P_STA_RECORD_T prStaRec;

				DBGLOG(TX, TRACE, "T1X len=%d\n", rTxPacketInfo.u4PacketLen);

				prStaRec = cnmGetStaRecByAddress(prAdapter,
								 GLUE_GET_PKT_BSS_IDX(prPacket),
								 rTxPacketInfo.aucEthDestAddr);

				GLUE_SET_PKT_FLAG(prPacket,
					(UINT_8)ENUM_PKT_1X);
				if (prStaRec != NULL && prStaRec->ucBssIndex <= HW_BSSID_NUM) {
					prBssInfo = prAdapter->aprBssInfo[prStaRec->ucBssIndex];
				} else {
					prBssInfo = NULL;
					DBGLOG(TX, WARN, "Bss Index is invaild\n");
				}
				if (secIsProtected1xFrame(prAdapter,
					prStaRec) == TRUE) {
					/* 1st 4way-handshake don't encrpted it */
					if (prBssInfo == NULL ||
						(prBssInfo->fgUnencryptedEapol
						== FALSE)) {
						GLUE_SET_PKT_FLAG(prPacket,
						(UINT_8)ENUM_PKT_PROTECTED_1X);
						DBGLOG(RSN, INFO, "This EAP Frame will be encrypyed\n");
					}
				}

			}

			if ((rTxPacketInfo.u2Flag &
				BIT((UINT_8)ENUM_PKT_802_3)) != 0U)
				GLUE_SET_PKT_FLAG(prPacket,
					(UINT_8)ENUM_PKT_802_3);

			if ((rTxPacketInfo.u2Flag &
				BIT((UINT_8)ENUM_PKT_VLAN_EXIST)) != 0U)
				GLUE_SET_PKT_FLAG(prPacket,
					(UINT_8)ENUM_PKT_VLAN_EXIST);

			if ((rTxPacketInfo.u2Flag &
				BIT((UINT_8)ENUM_PKT_DHCP)) != 0U)
				GLUE_SET_PKT_FLAG(prPacket,
					(UINT_8)ENUM_PKT_DHCP);

			if ((rTxPacketInfo.u2Flag &
				BIT((UINT_8)ENUM_PKT_ARP)) != 0U)
				GLUE_SET_PKT_FLAG(prPacket,
					(UINT_8)ENUM_PKT_ARP);

			if ((rTxPacketInfo.u2Flag &
				BIT((UINT_8)ENUM_PKT_ICMP)) != 0U)
				GLUE_SET_PKT_FLAG(prPacket,
					(UINT_8)ENUM_PKT_ICMP);

			if ((rTxPacketInfo.u2Flag &
				BIT((UINT_8)ENUM_PKT_TDLS)) != 0U)
				GLUE_SET_PKT_FLAG(prPacket,
					(UINT_8)ENUM_PKT_TDLS);

			if ((rTxPacketInfo.u2Flag &
				BIT((UINT_8)ENUM_PKT_DNS)) != 0U)
				GLUE_SET_PKT_FLAG(prPacket,
					(UINT_8)ENUM_PKT_DNS);
		}
#else
		if (rTxPacketInfo.fgIs1X == TRUE) {
			P_STA_RECORD_T prStaRec;

			DBGLOG(RSN, INFO, "T1X len=%d\n", rTxPacketInfo.u4PacketLen);

			prStaRec = cnmGetStaRecByAddress(prAdapter,
							 GLUE_GET_PKT_BSS_IDX(prPacket), rTxPacketInfo.aucEthDestAddr);

			GLUE_SET_PKT_FLAG(prPacket, (UINT_8)ENUM_PKT_1X);

			if (secIsProtected1xFrame(prAdapter, prStaRec) == TRUE)
				GLUE_SET_PKT_FLAG(prPacket,
				(UINT_8)ENUM_PKT_PROTECTED_1X);
		}

		if (rTxPacketInfo.fgIs802_3 == TRUE)
			GLUE_SET_PKT_FLAG(prPacket, (UINT_8)ENUM_PKT_802_3);

		if (rTxPacketInfo.fgIsVlanExists == TRUE)
			GLUE_SET_PKT_FLAG(prPacket,
			(UINT_8)ENUM_PKT_VLAN_EXIST);

		if (rTxPacketInfo.fgIsDhcp == TRUE)
			GLUE_SET_PKT_FLAG(prPacket, (UINT_8)ENUM_PKT_DHCP);

		if (rTxPacketInfo.fgIsArp == TRUE)
			GLUE_SET_PKT_FLAG(prPacket, (UINT_8)ENUM_PKT_ARP);
#endif

		ucMacHeaderLen = ETHER_HEADER_LEN;

		/* Save the value of Header Length */
		GLUE_SET_PKT_HEADER_LEN(prPacket, ucMacHeaderLen);

		/* Save the value of Frame Length */
		GLUE_SET_PKT_FRAME_LEN(prPacket, (UINT_16) rTxPacketInfo.u4PacketLen);

		/* Save the value of Arrival Time */
		u4SysTime = (OS_SYSTIME) kalGetTimeTick();
		GLUE_SET_PKT_ARRIVAL_TIME(prPacket, u4SysTime);

		return TRUE;
	}

	return FALSE;
}


WLAN_STATUS
nicTxSecFrameTxDone(IN P_ADAPTER_T prAdapter, IN P_MSDU_INFO_T prMsduInfo,
		IN ENUM_TX_RESULT_CODE_T rTxDoneStatus)
{
	UINT_8 ucKeyCmdAction = SEC_TX_KEY_COMMAND;

	DBGLOG(TX, INFO, "SEC Msdu WIDX:PID[%u:%u] Status[%u], SeqNo[%u]\n",
			   prMsduInfo->ucWlanIndex, prMsduInfo->ucPID, rTxDoneStatus,
			   prMsduInfo->ucTxSeqNum);

	if (rTxDoneStatus != TX_RESULT_SUCCESS)
		ucKeyCmdAction = SEC_DROP_KEY_COMMAND;
	else
		ucKeyCmdAction = SEC_TX_KEY_COMMAND;
	secSetKeyCmdAction(prAdapter->aprBssInfo[prMsduInfo->ucBssIndex],
		prMsduInfo->ucEapolKeyType, ucKeyCmdAction);
	kalSetEvent(prAdapter->prGlueInfo);
	return WLAN_STATUS_SUCCESS;
}


/*----------------------------------------------------------------------------*/
/*!
* @brief This function is called to identify 802.1x and Bluetooth-over-Wi-Fi
*        security frames, and queued into command queue for strict ordering
*        due to 802.1x frames before add-key OIDs are not to be encrypted
*
* @param prAdapter      Pointer of Adapter Data Structure
* @param prPacket       Pointer of native packet
*
* @return TRUE
*         FALSE
*/
/*----------------------------------------------------------------------------*/
BOOLEAN wlanProcessSecurityFrame(IN P_ADAPTER_T prAdapter, IN P_NATIVE_PACKET prPacket)
{
	P_CMD_INFO_T prCmdInfo;
	P_STA_RECORD_T prStaRec;
	UINT_8 ucBssIndex;
	UINT_32 u4PacketLen;
	UINT_8 aucEthDestAddr[PARAM_MAC_ADDR_LEN];
	P_MSDU_INFO_T prMsduInfo;

	ASSERT(prAdapter);
	ASSERT(prPacket);

	prCmdInfo = cmdBufAllocateCmdInfo(prAdapter, 0);

	/* Get MSDU_INFO for TxDone */
	prMsduInfo = cnmPktAlloc(prAdapter, 0);

	u4PacketLen = (UINT_32) GLUE_GET_PKT_FRAME_LEN(prPacket);

	if (prCmdInfo != NULL && prMsduInfo != NULL) {
		ucBssIndex = GLUE_GET_PKT_BSS_IDX(prPacket);

		if (kalGetEthDestAddr(prAdapter->prGlueInfo,
			prPacket, aucEthDestAddr) == TRUE)
			DBGLOG(RSN, TRACE, "Get eth da\n");

		prStaRec = cnmGetStaRecByAddress(prAdapter, ucBssIndex, aucEthDestAddr);

		prCmdInfo->eCmdType = COMMAND_TYPE_SECURITY_FRAME;
		prCmdInfo->u2InfoBufLen = (UINT_16) u4PacketLen;
		prCmdInfo->prPacket = prPacket;
		prCmdInfo->prMsduInfo = prMsduInfo;
		if (prStaRec != NULL)
			prCmdInfo->ucStaRecIndex = prStaRec->ucIndex;
		else
			prCmdInfo->ucStaRecIndex = STA_REC_INDEX_NOT_FOUND;
		prCmdInfo->ucBssIndex = ucBssIndex;
		prCmdInfo->pfCmdDoneHandler = wlanSecurityFrameTxDone;
		prCmdInfo->pfCmdTimeoutHandler = wlanSecurityFrameTxTimeout;
		prCmdInfo->fgIsOid = FALSE;
		prCmdInfo->fgSetQuery = TRUE;
		prCmdInfo->fgNeedResp = FALSE;

		/* Fill-up MSDU_INFO */
		nicTxSetDataPacket(prAdapter, prMsduInfo, ucBssIndex,
			prCmdInfo->ucStaRecIndex, 0,
			(UINT_16)u4PacketLen, nicTxSecFrameTxDone,
			(UINT_8)MSDU_RATE_MODE_AUTO, TX_PACKET_OS,
			0, FALSE, TRUE);

		prMsduInfo->prPacket = prPacket;
		prMsduInfo->ucTxSeqNum = GLUE_GET_PKT_SEQ_NO(prPacket);
		/* No Tx descriptor template for MMPDU */
		prMsduInfo->fgIsTXDTemplateValid = FALSE;

		if (GLUE_TEST_PKT_FLAG(prPacket,
			(UINT_8)ENUM_PKT_PROTECTED_1X) != 0U)
			nicTxConfigPktOption(prMsduInfo,
				(UINT_32)MSDU_OPT_PROTECTED_FRAME, TRUE);
#if CFG_SUPPORT_MULTITHREAD
		nicTxComposeSecurityFrameDesc(prAdapter, prCmdInfo, prMsduInfo->aucTxDescBuffer, NULL);
#endif

		kalEnqueueCommand(prAdapter->prGlueInfo, (P_QUE_ENTRY_T) prCmdInfo);

		GLUE_SET_EVENT(prAdapter->prGlueInfo);

	} else {
		DBGLOG(RSN, INFO, "Failed to alloc CMD/MGMT INFO for 1X frame!!\n");
		cmdBufFreeCmdInfo(prAdapter, prCmdInfo);
		cnmPktFree(prAdapter, prMsduInfo);
		return FALSE;
	}

	return TRUE;
}

/*----------------------------------------------------------------------------*/
/*!
* @brief This function is called when 802.1x or Bluetooth-over-Wi-Fi
*        security frames has been sent to firmware
*
* @param prAdapter      Pointer of Adapter Data Structure
* @param prCmdInfo      Pointer of CMD_INFO_T
* @param pucEventBuf    meaningless, only for API compatibility
*
* @return none
*/
/*----------------------------------------------------------------------------*/
VOID wlanSecurityFrameTxDone(IN P_ADAPTER_T prAdapter, IN P_CMD_INFO_T prCmdInfo, IN PUINT_8 pucEventBuf)
{
	ASSERT(prAdapter);
	ASSERT(prCmdInfo);

	if (GET_BSS_INFO_BY_INDEX(prAdapter, prCmdInfo->ucBssIndex)->eNetworkType ==
		NETWORK_TYPE_AIS &&
		prAdapter->rWifiVar.rAisSpecificBssInfo.
		fgCounterMeasure == TRUE) {
		P_STA_RECORD_T prSta = cnmGetStaRecByIndex(prAdapter, prCmdInfo->ucBssIndex);

		if (prSta != NULL) {
			kalMsleep(10);
			if (authSendDeauthFrame(prAdapter,
						GET_BSS_INFO_BY_INDEX(prAdapter,
								      prCmdInfo->ucBssIndex), prSta,
						(P_SW_RFB_T) NULL, REASON_CODE_MIC_FAILURE, (PFN_TX_DONE_HANDLER) NULL
						/* secFsmEventDeauthTxDone left upper layer handle the 60 timer */
			    ) != WLAN_STATUS_SUCCESS) {
				ASSERT_BOOLEAN(FALSE);
			}
			/* secFsmEventEapolTxDone(prAdapter, prSta, TX_RESULT_SUCCESS); */
		}
	}
	DBGLOG(RSN, INFO, "SECURITY PKT HOST TO HIF TX DONE\n");
	/* Clear the flag when Eapol frame tx Done */
	GET_BSS_INFO_BY_INDEX(prAdapter, prCmdInfo->ucBssIndex)->fgUnencryptedEapol = FALSE;
	kalSecurityFrameSendComplete(prAdapter->prGlueInfo, prCmdInfo->prPacket, WLAN_STATUS_SUCCESS);
}

/*----------------------------------------------------------------------------*/
/*!
* @brief This function is called when 802.1x or Bluetooth-over-Wi-Fi
*        security frames has failed sending to firmware
*
* @param prAdapter      Pointer of Adapter Data Structure
* @param prCmdInfo      Pointer of CMD_INFO_T
*
* @return none
*/
/*----------------------------------------------------------------------------*/
VOID wlanSecurityFrameTxTimeout(IN P_ADAPTER_T prAdapter, IN P_CMD_INFO_T prCmdInfo)
{
	ASSERT(prAdapter);
	ASSERT(prCmdInfo);

	kalSecurityFrameSendComplete(prAdapter->prGlueInfo, prCmdInfo->prPacket, WLAN_STATUS_FAILURE);
}

/*----------------------------------------------------------------------------*/
/*!
* @brief This function is called before AIS is starting a new scan
*
* @param prAdapter      Pointer of Adapter Data Structure
*
* @return none
*/
/*----------------------------------------------------------------------------*/
VOID wlanClearScanningResult(IN P_ADAPTER_T prAdapter)
{
	BOOLEAN fgKeepCurrOne = FALSE;
	UINT_32 i;

	ASSERT(prAdapter);

	/* clear scanning result */
	if (kalGetMediaStateIndicated(prAdapter->prGlueInfo) == PARAM_MEDIA_STATE_CONNECTED) {
		for (i = 0U; i < prAdapter->rWlanInfo.u4ScanResultNum; i++) {
			if (EQUAL_MAC_ADDR(prAdapter->rWlanInfo.rCurrBssId.arMacAddress,
					   prAdapter->rWlanInfo.arScanResult[i].arMacAddress)) {
				fgKeepCurrOne = TRUE;

				if (i != 0U) {
					/* copy structure */
					kalMemCopy(&(prAdapter->rWlanInfo.arScanResult[0]),
						   &(prAdapter->rWlanInfo.arScanResult[i]),
						   OFFSET_OF(PARAM_BSSID_EX_T, aucIEs));
				}

				if (prAdapter->rWlanInfo.
					arScanResult[i].u4IELength > 0U) {
					if (prAdapter->rWlanInfo.apucScanResultIEs[i] !=
					    &(prAdapter->rWlanInfo.aucScanIEBuf[0])) {
						/* move IEs to head */
						kalMemCopy(prAdapter->rWlanInfo.aucScanIEBuf,
							   prAdapter->rWlanInfo.apucScanResultIEs[i],
							   prAdapter->rWlanInfo.arScanResult[i].u4IELength);
					}
					/* modify IE pointer */
					prAdapter->rWlanInfo.apucScanResultIEs[0] =
					    &(prAdapter->rWlanInfo.aucScanIEBuf[0]);
				} else {
					prAdapter->rWlanInfo.apucScanResultIEs[0] = NULL;
				}

				break;
			}
		}
	}

	if (fgKeepCurrOne == TRUE) {
		prAdapter->rWlanInfo.u4ScanResultNum = 1;
		prAdapter->rWlanInfo.u4ScanIEBufferUsage = ALIGN_4(prAdapter->rWlanInfo.arScanResult[0].u4IELength);
	} else {
		prAdapter->rWlanInfo.u4ScanResultNum = 0;
		prAdapter->rWlanInfo.u4ScanIEBufferUsage = 0;
	}
}

/*----------------------------------------------------------------------------*/
/*!
* @brief This function is called when AIS received a beacon timeout event
*
* @param prAdapter      Pointer of Adapter Data Structure
* @param arBSSID        MAC address of the specified BSS
*
* @return none
*/
/*----------------------------------------------------------------------------*/
VOID wlanClearBssInScanningResult(IN P_ADAPTER_T prAdapter, IN PUINT_8 arBSSID)
{
	UINT_32 i, j, u4IELength = 0, u4IEMoveLength;
	PUINT_8 pucIEPtr;

	ASSERT(prAdapter);

	/* clear scanning result */
	i = 0U;
	while (1 != 0) {
		if (i >= prAdapter->rWlanInfo.u4ScanResultNum)
			break;

		if (EQUAL_MAC_ADDR(arBSSID, prAdapter->rWlanInfo.arScanResult[i].arMacAddress)) {
			/* backup current IE length */
			u4IELength = ALIGN_4(prAdapter->rWlanInfo.arScanResult[i].u4IELength);
			pucIEPtr = prAdapter->rWlanInfo.apucScanResultIEs[i];

			/* removed from middle */
			for (j = i + 1U;
				j < prAdapter->rWlanInfo.u4ScanResultNum;
				j++) {
				kalMemCopy(
					&(prAdapter->rWlanInfo.
					arScanResult[j - 1U]),
					&(prAdapter->rWlanInfo.arScanResult[j]),
					OFFSET_OF(PARAM_BSSID_EX_T, aucIEs));

				prAdapter->rWlanInfo.apucScanResultIEs[j - 1U] =
				    prAdapter->rWlanInfo.apucScanResultIEs[j];
			}

			prAdapter->rWlanInfo.u4ScanResultNum--;

			/* remove IE buffer if needed := move rest of IE buffer */
			if (u4IELength > 0U) {
				u4IEMoveLength = prAdapter->rWlanInfo.u4ScanIEBufferUsage -
					(UINT_32)(((ULONG) pucIEPtr) +
					u4IELength -
					((ULONG) (&(prAdapter->rWlanInfo.
					aucScanIEBuf[0]))));

				kalMemCopy(pucIEPtr, (PUINT_8) (((ULONG) pucIEPtr) + u4IELength), u4IEMoveLength);

				prAdapter->rWlanInfo.u4ScanIEBufferUsage -= u4IELength;

				/* correction of pointers to IE buffer */
				for (j = 0U; j < prAdapter->rWlanInfo.
					u4ScanResultNum; j++) {
					if (prAdapter->rWlanInfo.apucScanResultIEs[j] > pucIEPtr) {
						prAdapter->rWlanInfo.apucScanResultIEs[j] =
							(PUINT_8) ((ULONG) (prAdapter->rWlanInfo.apucScanResultIEs[j]) -
							u4IELength);
					}
				}
			}
		}

		i++;
	}
}

#if CFG_TEST_WIFI_DIRECT_GO
VOID wlanEnableP2pFunction(IN P_ADAPTER_T prAdapter)
{
#if 0
	P_MSG_P2P_FUNCTION_SWITCH_T prMsgFuncSwitch = (P_MSG_P2P_FUNCTION_SWITCH_T) NULL;

	prMsgFuncSwitch =
	    (P_MSG_P2P_FUNCTION_SWITCH_T) cnmMemAlloc(prAdapter, RAM_TYPE_MSG, sizeof(MSG_P2P_FUNCTION_SWITCH_T));
	if (prMsgFuncSwitch == NULL) {
		ASSERT_BOOLEAN(FALSE);
		return;
	}

	prMsgFuncSwitch->rMsgHdr.eMsgId = MID_MNY_P2P_FUN_SWITCH;
	prMsgFuncSwitch->fgIsFuncOn = TRUE;

	mboxSendMsg(prAdapter, MBOX_ID_0, (P_MSG_HDR_T) prMsgFuncSwitch, MSG_SEND_METHOD_BUF);
#endif
}

VOID wlanEnableATGO(IN P_ADAPTER_T prAdapter)
{

	P_MSG_P2P_CONNECTION_REQUEST_T prMsgConnReq = (P_MSG_P2P_CONNECTION_REQUEST_T) NULL;
	UINT_8 aucTargetDeviceID[MAC_ADDR_LEN] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

	prMsgConnReq =
	    (P_MSG_P2P_CONNECTION_REQUEST_T) cnmMemAlloc(prAdapter,
	    RAM_TYPE_MSG, (UINT_32)sizeof(MSG_P2P_CONNECTION_REQUEST_T));
	if (prMsgConnReq == NULL) {
		ASSERT_BOOLEAN(FALSE);
		return;
	}

	prMsgConnReq->rMsgHdr.eMsgId = MID_MNY_P2P_CONNECTION_REQ;

	/*=====Param Modified for test=====*/
	COPY_MAC_ADDR(prMsgConnReq->aucDeviceID, aucTargetDeviceID);
	prMsgConnReq->fgIsTobeGO = TRUE;
	prMsgConnReq->fgIsPersistentGroup = FALSE;

	/*=====Param Modified for test=====*/

	mboxSendMsg(prAdapter, MBOX_ID_0, (P_MSG_HDR_T) prMsgConnReq, MSG_SEND_METHOD_BUF);
}
#endif

/*----------------------------------------------------------------------------*/
/*!
* @brief This function is called to retrieve NIC capability from firmware
*
* @param prAdapter      Pointer of Adapter Data Structure
*
* @return WLAN_STATUS_SUCCESS
*         WLAN_STATUS_FAILURE
*/
/*----------------------------------------------------------------------------*/
UINT_32 g_u2FwIDVersion;
WLAN_STATUS wlanQueryNicCapability(IN P_ADAPTER_T prAdapter)
{
	UINT_8 aucZeroMacAddr[] = NULL_MAC_ADDR;
	UINT_8 ucCmdSeqNum;
	P_CMD_INFO_T prCmdInfo;
	P_WIFI_CMD_T prWifiCmd;
	UINT_32 u4RxPktLength;
	UINT_8 aucBuffer[sizeof(WIFI_EVENT_T) + sizeof(EVENT_NIC_CAPABILITY_T)];
	P_HW_MAC_RX_DESC_T prRxStatus;
	P_WIFI_EVENT_T prEvent;
	P_EVENT_NIC_CAPABILITY_T prEventNicCapability;
	UINT_32 u4Tmp[4];

	ASSERT(prAdapter);

	DEBUGFUNC("wlanQueryNicCapability");

	/* 1. Allocate CMD Info Packet and its Buffer */
	prCmdInfo = cmdBufAllocateCmdInfo(prAdapter,
		(UINT_32)CMD_HDR_SIZE +
		(UINT_32)sizeof(EVENT_NIC_CAPABILITY_T));
	if (prCmdInfo == NULL) {
		DBGLOG(NIC, ERROR, "Allocate CMD_INFO_T ==> FAILED.\n");
		return WLAN_STATUS_FAILURE;
	}
	/* increase command sequence number */
	ucCmdSeqNum = nicIncreaseCmdSeqNum(prAdapter);

	/* compose CMD_BUILD_CONNECTION cmd pkt */
	prCmdInfo->eCmdType = COMMAND_TYPE_GENERAL_IOCTL;
	prCmdInfo->u2InfoBufLen =
		(UINT_16)(CMD_HDR_SIZE + sizeof(EVENT_NIC_CAPABILITY_T));
	prCmdInfo->pfCmdDoneHandler = NULL;
	prCmdInfo->fgIsOid = FALSE;
	prCmdInfo->ucCID = (UINT_8)CMD_ID_GET_NIC_CAPABILITY;
	prCmdInfo->fgSetQuery = FALSE;
	prCmdInfo->fgNeedResp = TRUE;
	prCmdInfo->fgDriverDomainMCR = FALSE;
	prCmdInfo->ucCmdSeqNum = ucCmdSeqNum;
	prCmdInfo->u4SetInfoLen = 0;

	/* Setup WIFI_CMD_T */
	prWifiCmd = (P_WIFI_CMD_T) (prCmdInfo->pucInfoBuffer);
	prWifiCmd->u2TxByteCount = prCmdInfo->u2InfoBufLen;
	prWifiCmd->u2PQ_ID = (UINT_16)CMD_PQ_ID;
	prWifiCmd->ucPktTypeID = (UINT_8)CMD_PACKET_TYPE_ID;
	prWifiCmd->ucCID = prCmdInfo->ucCID;
	prWifiCmd->ucSetQuery = prCmdInfo->fgSetQuery;
	prWifiCmd->ucSeqNum = prCmdInfo->ucCmdSeqNum;

	wlanSendCommand(prAdapter, prCmdInfo);
	cmdBufFreeCmdInfo(prAdapter, prCmdInfo);

	if (nicRxWaitResponse(prAdapter,
		1,
		aucBuffer,
		(UINT_32)(sizeof(WIFI_EVENT_T) +
		sizeof(EVENT_NIC_CAPABILITY_T)),
		&u4RxPktLength) != WLAN_STATUS_SUCCESS) {
		return WLAN_STATUS_FAILURE;
	}

	/* Header checking .. */
	prRxStatus = (P_HW_MAC_RX_DESC_T) aucBuffer;
	if (prRxStatus->u2PktTYpe != (UINT_16)RXM_RXD_PKT_TYPE_SW_EVENT) {
		DBGLOG(INIT, ERROR, "Unexpected packet type %d! not SW_EVENT\n", prRxStatus->u2PktTYpe);
		return WLAN_STATUS_FAILURE;
	}

	prEvent = (P_WIFI_EVENT_T) aucBuffer;
	if (prEvent->ucEID != (UINT_8)EVENT_ID_NIC_CAPABILITY) {
		DBGLOG(INIT, ERROR, "Unexpected Event ID %d! expect %d\n",
		       prEvent->ucEID, EVENT_ID_NIC_CAPABILITY);
		return WLAN_STATUS_FAILURE;
	}

	prEventNicCapability = (P_EVENT_NIC_CAPABILITY_T) (prEvent->aucBuffer);

	prAdapter->rVerInfo.u2FwProductID = prEventNicCapability->u2ProductID;
	prAdapter->rVerInfo.u2FwOwnVersion = prEventNicCapability->u2FwVersion;
	prAdapter->rVerInfo.u2FwPeerVersion = prEventNicCapability->u2DriverVersion;

	/* Support FW version extend */
	u4Tmp[0] = (UINT_32)prEventNicCapability->aucReserved0[0];
	u4Tmp[1] = (UINT_32)prEventNicCapability->aucReserved0[1];
	u4Tmp[2] = (UINT_32)prEventNicCapability->aucReserved0[2];
	u4Tmp[3] = (UINT_32)prEventNicCapability->aucReserved0[3];
	prAdapter->rVerInfo.u2FwOwnVersionExtend =
		(u4Tmp[0] << 24U)
		| (u4Tmp[1] << 16U)
		| (u4Tmp[2] << 8U)
		| (u4Tmp[3]);

	prAdapter->fgIsHw5GBandDisabled = (BOOLEAN) prEventNicCapability->ucHw5GBandDisabled;
	prAdapter->fgIsEepromUsed = (BOOLEAN) prEventNicCapability->ucEepromUsed;
	prAdapter->fgIsEmbbededMacAddrValid = (BOOLEAN)
	    (!IS_BMCAST_MAC_ADDR(prEventNicCapability->aucMacAddr) &&
	     !EQUAL_MAC_ADDR(aucZeroMacAddr, prEventNicCapability->aucMacAddr));

	COPY_MAC_ADDR(prAdapter->rWifiVar.aucPermanentAddress, prEventNicCapability->aucMacAddr);
	COPY_MAC_ADDR(prAdapter->rWifiVar.aucMacAddress, prEventNicCapability->aucMacAddr);

	prAdapter->u4FwCompileFlag0 = prEventNicCapability->u4CompileFlag0;
	prAdapter->u4FwCompileFlag1 = prEventNicCapability->u4CompileFlag1;
	prAdapter->u4FwFeatureFlag0 = prEventNicCapability->u4FeatureFlag0;
	prAdapter->u4FwFeatureFlag1 = prEventNicCapability->u4FeatureFlag1;

	u4Tmp[0] = (UINT_32)prAdapter->rVerInfo.u2FwProductID;
	u4Tmp[1] = (UINT_32)prAdapter->rVerInfo.u2FwOwnVersion;
	g_u2FwIDVersion = (u4Tmp[0] << 16U) | (u4Tmp[1]);
#if CFG_ENABLE_CAL_LOG
	DBGLOG(NIC, LOUD, " RF CAL FAIL  = (%d),BB CAL FAIL  = (%d)\n",
	       prEventNicCapability->ucRfCalFail, prEventNicCapability->ucBbCalFail);
#endif

	DBGLOG(NIC, INFO, "FW Ver DEC[%u.%u] HEX[%x.%x], Driver Ver[%u.%u]\n",
	       (prAdapter->rVerInfo.u2FwOwnVersion >> 8U),
	       (prAdapter->rVerInfo.u2FwOwnVersion & BITS(0U, 7U)),
	       (prAdapter->rVerInfo.u2FwOwnVersion >> 8U),
	       (prAdapter->rVerInfo.u2FwOwnVersion & BITS(0U, 7U)),
	       (prAdapter->rVerInfo.u2FwPeerVersion >> 8U),
	       (prAdapter->rVerInfo.u2FwPeerVersion & BITS(0U, 7U)));

	return WLAN_STATUS_SUCCESS;
}

UINT_32 wlanGetFwIDVersion(void)
{
	UINT_32 u4FwIDVersion = 0U;

	u4FwIDVersion = g_u2FwIDVersion;

	return u4FwIDVersion;
}
EXPORT_SYMBOL(wlanGetFwIDVersion);

#if TXPWR_USE_PDSLOPE

/*----------------------------------------------------------------------------*/
/*!
* @brief
*
* @param prAdapter      Pointer of Adapter Data Structure
*
* @return WLAN_STATUS_SUCCESS
*         WLAN_STATUS_FAILURE
*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS wlanQueryPdMcr(IN P_ADAPTER_T prAdapter, P_PARAM_MCR_RW_STRUCT_T prMcrRdInfo)
{
	UINT_8 ucCmdSeqNum;
	P_CMD_INFO_T prCmdInfo;
	P_WIFI_CMD_T prWifiCmd;
	UINT_32 u4RxPktLength;
	UINT_8 aucBuffer[sizeof(WIFI_EVENT_T) + sizeof(CMD_ACCESS_REG)];
	P_HW_MAC_RX_DESC_T prRxStatus;
	P_WIFI_EVENT_T prEvent;
	P_CMD_ACCESS_REG prCmdMcrQuery;

	ASSERT(prAdapter);

	/* 1. Allocate CMD Info Packet and its Buffer */
	prCmdInfo = cmdBufAllocateCmdInfo(prAdapter, CMD_HDR_SIZE + sizeof(CMD_ACCESS_REG));

	if (prCmdInfo == NULL) {
		DBGLOG(INIT, ERROR, "Allocate CMD_INFO_T ==> FAILED.\n");
		return WLAN_STATUS_FAILURE;
	}
	/* increase command sequence number */
	ucCmdSeqNum = nicIncreaseCmdSeqNum(prAdapter);

	/* compose CMD_BUILD_CONNECTION cmd pkt */
	prCmdInfo->eCmdType = COMMAND_TYPE_GENERAL_IOCTL;
	prCmdInfo->u2InfoBufLen = (UINT_16) (CMD_HDR_SIZE + sizeof(CMD_ACCESS_REG));
	prCmdInfo->pfCmdDoneHandler = NULL;
	prCmdInfo->pfCmdTimeoutHandler = nicOidCmdTimeoutCommon;
	prCmdInfo->fgIsOid = FALSE;
	prCmdInfo->ucCID = (UINT_8)CMD_ID_ACCESS_REG;
	prCmdInfo->fgSetQuery = FALSE;
	prCmdInfo->fgNeedResp = TRUE;
	prCmdInfo->fgDriverDomainMCR = FALSE;
	prCmdInfo->ucCmdSeqNum = ucCmdSeqNum;
	prCmdInfo->u4SetInfoLen = (UINT_32)sizeof(CMD_ACCESS_REG);

	/* Setup WIFI_CMD_T */
	prWifiCmd = (P_WIFI_CMD_T) (prCmdInfo->pucInfoBuffer);
	prWifiCmd->u2TxByteCount = prCmdInfo->u2InfoBufLen;
	prWifiCmd->u2PQ_ID = (UINT_16)CMD_PQ_ID;
	prWifiCmd->ucPktTypeID = (UINT_8)CMD_PACKET_TYPE_ID;
	prWifiCmd->ucCID = prCmdInfo->ucCID;
	prWifiCmd->ucSetQuery = prCmdInfo->fgSetQuery;
	prWifiCmd->ucSeqNum = prCmdInfo->ucCmdSeqNum;
	kalMemCopy(prWifiCmd->aucBuffer,
		prMcrRdInfo, (UINT_32)sizeof(CMD_ACCESS_REG));

	wlanSendCommand(prAdapter, prCmdInfo);
	cmdBufFreeCmdInfo(prAdapter, prCmdInfo);

	if (nicRxWaitResponse(prAdapter,
		1,
		aucBuffer,
		(UINT_32)(sizeof(WIFI_EVENT_T) + sizeof(CMD_ACCESS_REG)),
		&u4RxPktLength) != WLAN_STATUS_SUCCESS) {
		return WLAN_STATUS_FAILURE;
	}

	/* Header checking .. */
	prRxStatus = (P_HW_MAC_RX_DESC_T) aucBuffer;
	if (prRxStatus->u2PktTYpe != RXM_RXD_PKT_TYPE_SW_EVENT) {
		DBGLOG(INIT, ERROR, "Unexpected packet type %d! not SW_EVENT\n", prRxStatus->u2PktTYpe);
		return WLAN_STATUS_FAILURE;
	}

	prEvent = (P_WIFI_EVENT_T) aucBuffer;
	if (prEvent->ucEID != EVENT_ID_ACCESS_REG) {
		DBGLOG(INIT, ERROR, "Unexpected Event ID %d! expect %d\n",
		       prEvent->ucEID, EVENT_ID_ACCESS_REG);
		return WLAN_STATUS_FAILURE;
	}

	prCmdMcrQuery = (P_CMD_ACCESS_REG) (prEvent->aucBuffer);
	prMcrRdInfo->u4McrOffset = prCmdMcrQuery->u4Address;
	prMcrRdInfo->u4McrData = prCmdMcrQuery->u4Data;

	return WLAN_STATUS_SUCCESS;
}

static INT_32 wlanIntRound(INT_32 au4Input)
{

	if (au4Input >= 0) {
		if ((au4Input % 10) == 5) {
			au4Input = au4Input + 5;
			return au4Input;
		}
	}

	if (au4Input < 0) {
		if ((au4Input % 10) == -5) {
			au4Input = au4Input - 5;
			return au4Input;
		}
	}

	return au4Input;
}

static INT_32 wlanCal6628EfuseForm(IN P_ADAPTER_T prAdapter, INT_32 au4Input)
{

	PARAM_MCR_RW_STRUCT_T rMcrRdInfo;
	INT_32 au4PdSlope, au4TxPwrOffset, au4TxPwrOffset_Round;
	INT_8 auTxPwrOffset_Round;

	rMcrRdInfo.u4McrOffset = 0x60205c68U;
	rMcrRdInfo.u4McrData = 0U;
	au4TxPwrOffset = au4Input;
	wlanQueryPdMcr(prAdapter, &rMcrRdInfo);

	au4PdSlope = (rMcrRdInfo.u4McrData) & BITS(0U, 6U);
	au4TxPwrOffset_Round = wlanIntRound((au4TxPwrOffset * au4PdSlope)) / 10;

	au4TxPwrOffset_Round = -au4TxPwrOffset_Round;

	if (au4TxPwrOffset_Round < -128)
		au4TxPwrOffset_Round = 128;
	else if (au4TxPwrOffset_Round < 0)
		au4TxPwrOffset_Round += 256;
	else if (au4TxPwrOffset_Round > 127)
		au4TxPwrOffset_Round = 127;

	auTxPwrOffset_Round = (UINT_8) au4TxPwrOffset_Round;

	return au4TxPwrOffset_Round;
}

#endif

#if CFG_SUPPORT_NVRAM_5G
WLAN_STATUS wlanLoadManufactureData_5G(IN P_ADAPTER_T prAdapter, IN P_REG_INFO_T prRegInfo)
{

	P_BANDEDGE_5G_T pr5GBandEdge;

	ASSERT(prAdapter);

	pr5GBandEdge = &prRegInfo->prOldEfuseMapping->r5GBandEdgePwr;

	/* 1. set band edge tx power if available */
	if (pr5GBandEdge->uc5GBandEdgePwrUsed != 0U) {
		CMD_EDGE_TXPWR_LIMIT_T rCmdEdgeTxPwrLimit;

		rCmdEdgeTxPwrLimit.cBandEdgeMaxPwrCCK = 0;
		rCmdEdgeTxPwrLimit.cBandEdgeMaxPwrOFDM20 =
			(INT_8)pr5GBandEdge->c5GBandEdgeMaxPwrOFDM20;
		rCmdEdgeTxPwrLimit.cBandEdgeMaxPwrOFDM40 =
			(INT_8)pr5GBandEdge->c5GBandEdgeMaxPwrOFDM40;
		rCmdEdgeTxPwrLimit.cBandEdgeMaxPwrOFDM80 =
			(INT_8)pr5GBandEdge->c5GBandEdgeMaxPwrOFDM80;

		if (wlanSendSetQueryCmd(prAdapter,
				(UINT_8)CMD_ID_SET_EDGE_TXPWR_LIMIT_5G,
				TRUE,
				FALSE,
				FALSE,
				NULL,
				NULL,
				(UINT_32)sizeof(CMD_EDGE_TXPWR_LIMIT_T),
				(PUINT_8) &rCmdEdgeTxPwrLimit,
				NULL, 0) == TRUE)
			DBGLOG(INIT, TRACE, "Send TXPWR LIMIT 5G\n");

		/* dumpMemory8(&rCmdEdgeTxPwrLimit,4); */
	}

	/*2.set channel offset for 8 sub-band */
	if (prRegInfo->prOldEfuseMapping->uc5GChannelOffsetVaild != 0U) {
		CMD_POWER_OFFSET_T rCmdPowerOffset;
		UINT_8 i;

		rCmdPowerOffset.ucBand = (UINT_8)BAND_5G;
		for (i = (UINT_8)0; i < (UINT_8)MAX_SUBBAND_NUM_5G; i++)
			rCmdPowerOffset.ucSubBandOffset[i] = prRegInfo->prOldEfuseMapping->auc5GChOffset[i];

		if (wlanSendSetQueryCmd(prAdapter,
			(UINT_8)CMD_ID_SET_CHANNEL_PWR_OFFSET,
			TRUE,
			FALSE,
			FALSE, NULL, NULL,
			(UINT_32)sizeof(rCmdPowerOffset),
			(PUINT_8) &rCmdPowerOffset,
			NULL, 0) == TRUE)
			DBGLOG(INIT, TRACE, "Send CHANNEL PWR OFFSET\n");
		/* dumpMemory8(&rCmdPowerOffset,9); */
	}

	/*3.set 5G AC power */
	if (prRegInfo->prOldEfuseMapping->uc11AcTxPwrValid != 0U) {
		CMD_TX_AC_PWR_T rCmdAcPwr;

		kalMemCopy(&rCmdAcPwr.rAcPwr,
			&prRegInfo->prOldEfuseMapping->r11AcTxPwr,
			(UINT_32)sizeof(AC_PWR_SETTING_STRUCT));
		rCmdAcPwr.ucBand = (INT_8)BAND_5G;

		if (wlanSendSetQueryCmd(prAdapter,
				(UINT_8)CMD_ID_SET_80211AC_TX_PWR,
				TRUE,
				FALSE, FALSE, NULL, NULL,
				(UINT_32)sizeof(CMD_TX_AC_PWR_T),
				(PUINT_8) &rCmdAcPwr, NULL, 0) == TRUE)
			DBGLOG(INIT, TRACE, "Send 80211AC TX PWR\n");
		/* dumpMemory8(&rCmdAcPwr,9); */
	}

	return WLAN_STATUS_SUCCESS;
}
#endif
/*----------------------------------------------------------------------------*/
/*!
* @brief This function is called to load manufacture data from NVRAM
* if available and valid
*
* @param prAdapter      Pointer of Adapter Data Structure
* @param prRegInfo      Pointer of REG_INFO_T
*
* @return WLAN_STATUS_SUCCESS
*         WLAN_STATUS_FAILURE
*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS wlanLoadManufactureData(IN P_ADAPTER_T prAdapter, IN P_REG_INFO_T prRegInfo)
{
#if CFG_SUPPORT_RDD_TEST_MODE
	CMD_RDD_CH_T rRddParam;
#endif
#if CFG_SUPPORT_FCC_DYNAMIC_TX_PWR_ADJUST
	CMD_FCC_TX_PWR_ADJUST FccTxPwrAdjust = {0x00};
#endif

	CMD_NVRAM_SETTING_T rCmdNvramSettings;

	ASSERT(prAdapter);

	/* 1. Version Check */
	kalGetConfigurationVersion(prAdapter->prGlueInfo,
				   &(prAdapter->rVerInfo.u2Part1CfgOwnVersion),
				   &(prAdapter->rVerInfo.u2Part1CfgPeerVersion),
				   &(prAdapter->rVerInfo.u2Part2CfgOwnVersion),
				   &(prAdapter->rVerInfo.u2Part2CfgPeerVersion));

#if (CFG_SW_NVRAM_VERSION_CHECK == 1)
	if (prAdapter->rVerInfo.u2Part1CfgPeerVersion > CFG_DRV_OWN_VERSION
		|| prAdapter->rVerInfo.u2Part2CfgPeerVersion >
		CFG_DRV_OWN_VERSION) {
		return WLAN_STATUS_FAILURE;
	}
#endif

	/* MT6620 E1/E2 would be ignored directly */
	if (prAdapter->rVerInfo.u2Part1CfgOwnVersion == 0x0001U) {
		prRegInfo->ucTxPwrValid = 1U;
	} else {
		/* 2. Load TX power gain parameters if valid */
		if (prRegInfo->ucTxPwrValid != 0U) {
			/* send to F/W */

			if (nicUpdateTxPower(prAdapter, (P_CMD_TX_PWR_T)
				(&(prRegInfo->rTxPwr))) == WLAN_STATUS_SUCCESS)
				DBGLOG(INIT, TRACE, "Update TX PWR\n");
		}
	}

#if CFG_SUPPORT_FCC_DYNAMIC_TX_PWR_ADJUST
	/* Tx Power Adjust for FCC/CE Certification */
	FccTxPwrAdjust.fgFccTxPwrAdjust = 1;	/* 1:enable; 0:disable */
	FccTxPwrAdjust.Offset_CCK = 14;		/* drop 7dB */
	FccTxPwrAdjust.Offset_HT20 = 16;	/* drop 8dB */
	FccTxPwrAdjust.Offset_HT40 = 14;	/* drop 7dB*/
	FccTxPwrAdjust.Channel_CCK[0] = 11;	/* [0] for start channel */
	FccTxPwrAdjust.Channel_CCK[1] = 13;	/* [1] for ending channel */
	FccTxPwrAdjust.Channel_HT20[0] = 11;	/* [0] for start channel */
	FccTxPwrAdjust.Channel_HT20[1] = 13;	/* [1] for ending channel */
	FccTxPwrAdjust.Channel_HT40[0] = 7;	/* [0] for start channel,engineer mode ch9(2452) */
	FccTxPwrAdjust.Channel_HT40[1] = 9;	/* [1] for ending channel,engineer mode ch11(2462) */

	if (wlanSendSetQueryCmd(prAdapter,
			(UINT_8)CMD_ID_SET_FCC_TX_PWR_CERT,
			TRUE,
			FALSE,
			FALSE, NULL, NULL,
			(UINT_32)sizeof(CMD_FCC_TX_PWR_ADJUST),
			(PUINT_8) (&FccTxPwrAdjust), NULL, 0) ==
			WLAN_STATUS_SUCCESS)
		DBGLOG(INIT, TRACE, "Send FCC TX PWR\n");
#endif

	/* 3. Check if needs to support 5GHz */
	if (prRegInfo->ucEnable5GBand == TRUE) {
#if CFG_SUPPORT_NVRAM_5G
		wlanLoadManufactureData_5G(prAdapter, prRegInfo);
#endif
		/* check if it is disabled by hardware */
		if (prAdapter->fgIsHw5GBandDisabled == TRUE ||
			prRegInfo->ucSupport5GBand == 0U)
			prAdapter->fgEnable5GBand = FALSE;
		else
			prAdapter->fgEnable5GBand = TRUE;
	} else
		prAdapter->fgEnable5GBand = FALSE;


	/* 4. Send EFUSE data */
#if CFG_SUPPORT_NVRAM_5G
	/* If NvRAM read failed, this pointer will be NULL */
	if (prRegInfo->prOldEfuseMapping != NULL) {
		/*2.set channel offset for 3 sub-band */
		if (prRegInfo->prOldEfuseMapping->ucChannelOffsetVaild != 0U) {
			CMD_POWER_OFFSET_T rCmdPowerOffset;
			UINT_8 i;

			rCmdPowerOffset.ucBand = (UINT_8)BAND_2G4;
			for (i = 0U; i < 3U; i++)
				rCmdPowerOffset.ucSubBandOffset[i] = prRegInfo->prOldEfuseMapping->aucChOffset[i];
			rCmdPowerOffset.ucSubBandOffset[i] = prRegInfo->prOldEfuseMapping->acAllChannelOffset;

			if (wlanSendSetQueryCmd(prAdapter,
				(UINT_8)CMD_ID_SET_CHANNEL_PWR_OFFSET,
				TRUE,
				FALSE,
				FALSE,
				NULL, NULL,
				(UINT_32)sizeof(rCmdPowerOffset),
				(PUINT_8) &rCmdPowerOffset,
				NULL, 0) == WLAN_STATUS_SUCCESS)
				DBGLOG(INIT, TRACE, "Send PWR OFFSET\n");
			/* dumpMemory8(&rCmdPowerOffset,9); */
		}
	}
#else

	if (wlanSendSetQueryCmd(prAdapter,
		(UINT_8)CMD_ID_SET_PHY_PARAM,
		TRUE,
		FALSE,
		FALSE, NULL, NULL,
		(UINT_32)sizeof(CMD_PHY_PARAM_T),
		(PUINT_8) (prRegInfo->aucEFUSE),
		NULL, 0) == WLAN_STATUS_SUCCESS)
		DBGLOG(INIT, TRACE, "Send PHY PARAM\n");

#endif
	/*RSSI path compasation */
	if (prRegInfo->ucRssiPathCompasationUsed != 0U) {
		CMD_RSSI_PATH_COMPASATION_T rCmdRssiPathCompasation;

		rCmdRssiPathCompasation.c2GRssiCompensation = prRegInfo->rRssiPathCompasation.c2GRssiCompensation;
		rCmdRssiPathCompasation.c5GRssiCompensation = prRegInfo->rRssiPathCompasation.c5GRssiCompensation;

		if (wlanSendSetQueryCmd(prAdapter,
			(UINT_8)CMD_ID_SET_PATH_COMPASATION,
			TRUE,
			FALSE,
			FALSE,
			NULL,
			NULL,
			(UINT_32)sizeof(rCmdRssiPathCompasation),
			(PUINT_8) &rCmdRssiPathCompasation,
			NULL, 0) == WLAN_STATUS_SUCCESS)
			DBGLOG(INIT, TRACE, "Send PATH  COMPASATION\n");
	}
#if CFG_SUPPORT_RDD_TEST_MODE
	rRddParam.ucRddTestMode = (UINT_8) prRegInfo->u4RddTestMode;
	rRddParam.ucRddShutCh = (UINT_8) prRegInfo->u4RddShutFreq;
	rRddParam.ucRddStartCh = (UINT_8) nicFreq2ChannelNum(prRegInfo->u4RddStartFreq);
	rRddParam.ucRddStopCh = (UINT_8) nicFreq2ChannelNum(prRegInfo->u4RddStopFreq);
	rRddParam.ucRddDfs = (UINT_8) prRegInfo->u4RddDfs;
	prAdapter->ucRddStatus = 0U;
	nicUpdateRddTestMode(prAdapter, (P_CMD_RDD_CH_T) (&rRddParam));
#endif

	/* 5. Get 16-bits Country Code and Bandwidth */
	prAdapter->rWifiVar.rConnSettings.u2CountryCode =
		(((UINT_16) prRegInfo->au2CountryCode[0]) << 8U) |
		(((UINT_16) prRegInfo->au2CountryCode[1]) &
		(UINT_16)BITS(0U, 7U));

#if 0  /*
	* Bandwidth control will be controlled by GUI. 20110930
	* So ignore the setting from registry/NVRAM
	*/
	prAdapter->rWifiVar.rConnSettings.uc2G4BandwidthMode =
	    prRegInfo->uc2G4BwFixed20M ? CONFIG_BW_20M : CONFIG_BW_20_40M;
	prAdapter->rWifiVar.rConnSettings.uc5GBandwidthMode =
	    prRegInfo->uc5GBwFixed20M ? CONFIG_BW_20M : CONFIG_BW_20_40M;
#endif

	/* 6. Set domain and channel information to chip */
	rlmDomainSendCmd(prAdapter);
	/* Update supported channel list in channel table */
	wlanUpdateChannelTable(prAdapter->prGlueInfo);

	/* 7. set band edge tx power if available */
	if (prRegInfo->fg2G4BandEdgePwrUsed != 0U) {
		CMD_EDGE_TXPWR_LIMIT_T rCmdEdgeTxPwrLimit;

		rCmdEdgeTxPwrLimit.cBandEdgeMaxPwrCCK = prRegInfo->cBandEdgeMaxPwrCCK;
		rCmdEdgeTxPwrLimit.cBandEdgeMaxPwrOFDM20 = prRegInfo->cBandEdgeMaxPwrOFDM20;
		rCmdEdgeTxPwrLimit.cBandEdgeMaxPwrOFDM40 = prRegInfo->cBandEdgeMaxPwrOFDM40;

		if (wlanSendSetQueryCmd(prAdapter,
			(UINT_8)CMD_ID_SET_EDGE_TXPWR_LIMIT,
			TRUE,
			FALSE,
			FALSE,
			NULL,
			NULL,
			(UINT_32)sizeof(CMD_EDGE_TXPWR_LIMIT_T),
			(PUINT_8) &rCmdEdgeTxPwrLimit, NULL, 0)
			== WLAN_STATUS_SUCCESS)
			DBGLOG(INIT, TRACE, "Send EDGE TXPWR LIMIT\n");
	}
	/*8. Set 2.4G AC power */
	if (prRegInfo->prOldEfuseMapping != NULL) {
		if (prRegInfo->prOldEfuseMapping->uc11AcTxPwrValid2G != 0U) {
			CMD_TX_AC_PWR_T rCmdAcPwr;

			kalMemCopy(&rCmdAcPwr.rAcPwr, &prRegInfo->prOldEfuseMapping->r11AcTxPwr2G,
				   (UINT_32)sizeof(AC_PWR_SETTING_STRUCT));
			rCmdAcPwr.ucBand = (INT_8)BAND_2G4;

			if (wlanSendSetQueryCmd(prAdapter,
				(UINT_8)CMD_ID_SET_80211AC_TX_PWR,
				TRUE,
				FALSE, FALSE, NULL, NULL,
				(UINT_32)sizeof(CMD_TX_AC_PWR_T),
				(PUINT_8)&rCmdAcPwr, NULL, 0) ==
				WLAN_STATUS_SUCCESS)
				DBGLOG(INIT, TRACE, "Send 80211AC Tx PWR\n");
			/* dumpMemory8(&rCmdAcPwr,9); */
		}
	}
	/* 9. Send the full Parameters of NVRAM to FW */

	kalMemCopy(&rCmdNvramSettings.rNvramSettings,
		   &prRegInfo->prNvramSettings->u2Part1OwnVersion,
		   (UINT_32)sizeof(WIFI_CFG_PARAM_STRUCT));
	ASSERT_BOOLEAN(sizeof(WIFI_CFG_PARAM_STRUCT) == 512U);
	if (wlanSendSetQueryCmd(prAdapter,
		(UINT_8)CMD_ID_SET_NVRAM_SETTINGS,
		TRUE,
		FALSE,
		FALSE, NULL, NULL,
		(UINT_32)sizeof(rCmdNvramSettings),
		(PUINT_8) &rCmdNvramSettings,
		NULL, 0) == WLAN_STATUS_SUCCESS)
		DBGLOG(INIT, TRACE, "Send SET NVRAM SETTING\n");

	return WLAN_STATUS_SUCCESS;
}

/*----------------------------------------------------------------------------*/
/*!
* @brief This function is called to check
*        Media Stream Mode is set to non-default value or not,
*        and clear to default value if above criteria is met
*
* @param prAdapter      Pointer of Adapter Data Structure
*
* @return TRUE
*           The media stream mode was non-default value and has been reset
*         FALSE
*           The media stream mode is default value
*/
/*----------------------------------------------------------------------------*/
BOOLEAN wlanResetMediaStreamMode(IN P_ADAPTER_T prAdapter)
{
	ASSERT(prAdapter);

	if (prAdapter->rWlanInfo.eLinkAttr.ucMediaStreamMode != 0U) {
		prAdapter->rWlanInfo.eLinkAttr.ucMediaStreamMode = 0U;

		return TRUE;
	} else {
		return FALSE;
	}
}

/*----------------------------------------------------------------------------*/
/*!
* @brief This function is called to check if any pending timer has expired
*
* @param prAdapter      Pointer of Adapter Data Structure
*
* @return WLAN_STATUS_SUCCESS
*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS wlanTimerTimeoutCheck(IN P_ADAPTER_T prAdapter)
{
	ASSERT(prAdapter);

	cnmTimerDoTimeOutCheck(prAdapter);

	return WLAN_STATUS_SUCCESS;
}

/*----------------------------------------------------------------------------*/
/*!
* @brief This function is called to check if any pending mailbox message
*        to be handled
*
* @param prAdapter      Pointer of Adapter Data Structure
*
* @return WLAN_STATUS_SUCCESS
*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS wlanProcessMboxMessage(IN P_ADAPTER_T prAdapter)
{
	UINT_32 i;

	ASSERT(prAdapter);

	for (i = 0; i < (UINT_32)MBOX_ID_TOTAL_NUM; i++)
		mboxRcvAllMsg(prAdapter, (ENUM_MBOX_ID_T) i);

	return WLAN_STATUS_SUCCESS;
}

/*----------------------------------------------------------------------------*/
/*!
* @brief This function is called to enqueue a single TX packet into CORE
*
* @param prAdapter      Pointer of Adapter Data Structure
*        prNativePacket Pointer of Native Packet
*
* @return WLAN_STATUS_SUCCESS
*         WLAN_STATUS_RESOURCES
*         WLAN_STATUS_INVALID_PACKET
*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS wlanEnqueueTxPacket(IN P_ADAPTER_T prAdapter, IN P_NATIVE_PACKET prNativePacket)
{
	P_TX_CTRL_T prTxCtrl;
	P_MSDU_INFO_T prMsduInfo;

	ASSERT(prAdapter);

	prTxCtrl = &prAdapter->rTxCtrl;

	prMsduInfo = cnmPktAlloc(prAdapter, 0);

	if (prMsduInfo == NULL)
		return WLAN_STATUS_RESOURCES;

	if (nicTxFillMsduInfo(prAdapter, prMsduInfo, prNativePacket) == TRUE) {
		/* prMsduInfo->eSrc = TX_PACKET_OS; */

		/* Tx profiling */
		wlanTxProfilingTagMsdu(prAdapter, prMsduInfo, TX_PROF_TAG_DRV_ENQUE);

		/* enqueue to QM */
		nicTxEnqueueMsdu(prAdapter, prMsduInfo);
	} else {
		kalSendComplete(prAdapter->prGlueInfo, prNativePacket, WLAN_STATUS_INVALID_PACKET);

		nicTxReturnMsduInfo(prAdapter, prMsduInfo);

		return WLAN_STATUS_INVALID_PACKET;
	}

	return WLAN_STATUS_SUCCESS;
}

/*----------------------------------------------------------------------------*/
/*!
* @brief This function is called to flush pending TX packets in CORE
*
* @param prAdapter      Pointer of Adapter Data Structure
*
* @return WLAN_STATUS_SUCCESS
*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS wlanFlushTxPendingPackets(IN P_ADAPTER_T prAdapter)
{
	ASSERT(prAdapter);

	return nicTxFlush(prAdapter);
}

/*----------------------------------------------------------------------------*/
/*!
* \brief this function sends pending MSDU_INFO_T to MT6620
*
* @param prAdapter      Pointer to the Adapter structure.
* @param pfgHwAccess    Pointer for tracking LP-OWN status
*
* @retval WLAN_STATUS_SUCCESS   Reset is done successfully.
*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS wlanTxPendingPackets(IN P_ADAPTER_T prAdapter, IN OUT PBOOLEAN pfgHwAccess)
{
	P_TX_CTRL_T prTxCtrl;
	P_MSDU_INFO_T prMsduInfo;

	KAL_SPIN_LOCK_DECLARATION();

	ASSERT(prAdapter);
	prTxCtrl = &prAdapter->rTxCtrl;

#if !CFG_SUPPORT_MULTITHREAD
	ASSERT(pfgHwAccess);
#endif

	/* <1> dequeue packet by txDequeuTxPackets() */
#if CFG_SUPPORT_MULTITHREAD
	KAL_ACQUIRE_SPIN_LOCK(prAdapter, SPIN_LOCK_QM_TX_QUEUE);
	prMsduInfo = qmDequeueTxPacketsMthread(prAdapter, &prTxCtrl->rTc);
	KAL_RELEASE_SPIN_LOCK(prAdapter, SPIN_LOCK_QM_TX_QUEUE);
#else
	KAL_ACQUIRE_SPIN_LOCK(prAdapter, SPIN_LOCK_QM_TX_QUEUE);
	prMsduInfo = qmDequeueTxPackets(prAdapter, &prTxCtrl->rTc);
	KAL_RELEASE_SPIN_LOCK(prAdapter, SPIN_LOCK_QM_TX_QUEUE);
#endif
	if (prMsduInfo != NULL) {
		if (kalIsCardRemoved(prAdapter->prGlueInfo) == FALSE) {
#if !CFG_SUPPORT_MULTITHREAD
			/* <2> Acquire LP-OWN if necessary */
			if (*pfgHwAccess == FALSE) {
				*pfgHwAccess = TRUE;

				wlanAcquirePowerControl(prAdapter);
			}
#endif
			/* <3> send packets */
#if CFG_SUPPORT_MULTITHREAD
			if (nicTxMsduInfoListMthread(prAdapter, prMsduInfo)
				== WLAN_STATUS_SUCCESS)
				DBGLOG(INIT, TRACE, "TxMsduInfoList\n");
#else
			if (nicTxMsduInfoList(prAdapter, prMsduInfo)
				== WLAN_STATUS_SUCCESS)
				DBGLOG(INIT, TRACE, "nicTxMsduInfoList\n");
#endif
			/* <4> update TC by txAdjustTcQuotas() */
			if (nicTxAdjustTcq(prAdapter) == WLAN_STATUS_SUCCESS)
				DBGLOG(INIT, TRACE, "nicTxAdjustTcq\n");
		} else
			if (wlanProcessQueuedMsduInfo(prAdapter, prMsduInfo)
				== WLAN_STATUS_SUCCESS)
				DBGLOG(INIT, TRACE, "Process MsduInfo\n");
	}

	return WLAN_STATUS_SUCCESS;
}

/*----------------------------------------------------------------------------*/
/*!
* @brief This function is called to acquire power control from firmware
*
* @param prAdapter      Pointer of Adapter Data Structure
*
* @return WLAN_STATUS_SUCCESS
*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS wlanAcquirePowerControl(IN P_ADAPTER_T prAdapter)
{
	ASSERT(prAdapter);

	/* DBGLOG(INIT, INFO, ("Acquire Power Ctrl\n")); */

	ACQUIRE_POWER_CONTROL_FROM_PM(prAdapter);

	/* Reset sleepy state */
	if (prAdapter->fgWiFiInSleepyState == TRUE)
		prAdapter->fgWiFiInSleepyState = FALSE;

	return WLAN_STATUS_SUCCESS;
}

/*----------------------------------------------------------------------------*/
/*!
* @brief This function is called to release power control to firmware
*
* @param prAdapter      Pointer of Adapter Data Structure
*
* @return WLAN_STATUS_SUCCESS
*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS wlanReleasePowerControl(IN P_ADAPTER_T prAdapter)
{
	ASSERT(prAdapter);

	/* DBGLOG(INIT, INFO, ("Release Power Ctrl\n")); */

	RECLAIM_POWER_CONTROL_TO_PM(prAdapter, FALSE);

	return WLAN_STATUS_SUCCESS;
}

/*----------------------------------------------------------------------------*/
/*!
* @brief This function is called to report currently pending TX frames count
*        (command packets are not included)
*
* @param prAdapter      Pointer of Adapter Data Structure
*
* @return number of pending TX frames
*/
/*----------------------------------------------------------------------------*/
UINT_32 wlanGetTxPendingFrameCount(IN P_ADAPTER_T prAdapter)
{
	P_TX_CTRL_T prTxCtrl;
	UINT_32 u4Num;

	ASSERT(prAdapter);
	prTxCtrl = &prAdapter->rTxCtrl;

	u4Num = kalGetTxPendingFrameCount(prAdapter->prGlueInfo) + (UINT_32) (prTxCtrl->i4PendingFwdFrameCount);

	return u4Num;
}

/*----------------------------------------------------------------------------*/
/*!
* @brief This function is to report current ACPI state
*
* @param prAdapter      Pointer of Adapter Data Structure
*
* @return ACPI_STATE_D0 Normal Operation Mode
*         ACPI_STATE_D3 Suspend Mode
*/
/*----------------------------------------------------------------------------*/
ENUM_ACPI_STATE_T wlanGetAcpiState(IN P_ADAPTER_T prAdapter)
{
	ASSERT(prAdapter);

	return prAdapter->rAcpiState;
}

/*----------------------------------------------------------------------------*/
/*!
* @brief This function is to update current ACPI state only
*
* @param prAdapter      Pointer of Adapter Data Structure
* @param ePowerState    ACPI_STATE_D0 Normal Operation Mode
*                       ACPI_STATE_D3 Suspend Mode
*
* @return none
*/
/*----------------------------------------------------------------------------*/
VOID wlanSetAcpiState(IN P_ADAPTER_T prAdapter, IN ENUM_ACPI_STATE_T ePowerState)
{
	ASSERT(prAdapter);
	ASSERT_BOOLEAN(ePowerState <= ACPI_STATE_D3);

	prAdapter->rAcpiState = ePowerState;
}

/*----------------------------------------------------------------------------*/
/*!
* @brief This function is to query ECO version from HIFSYS CR
*
* @param prAdapter      Pointer of Adapter Data Structure
*
* @return zero      Unable to retrieve ECO version information
*         non-zero  ECO version (1-based)
*/
/*----------------------------------------------------------------------------*/
UINT_8 wlanGetEcoVersion(IN P_ADAPTER_T prAdapter)
{
	UINT_8 ucEcoVersion;

	ASSERT(prAdapter);

#if defined(MT6630)
#if CFG_MULTI_ECOVER_SUPPORT
	ucEcoVersion = nicGetChipEcoVer();
	DBGLOG(INIT, TRACE, "Chip ECO Ver: E%u\n", ucEcoVersion);
#else
	nicGetChipID(prAdapter);
	ucEcoVersion = prAdapter->ucRevID + 1;
#endif
#else /* MT6631 A-D die chip */
	ucEcoVersion = 1U;
#endif

	return ucEcoVersion;
}

/*----------------------------------------------------------------------------*/
/*!
* @brief This function is to setting the default Tx Power configuration
*
* @param prAdapter      Pointer of Adapter Data Structure
*
* @return zero      Unable to retrieve ECO version information
*         non-zero  ECO version (1-based)
*/
/*----------------------------------------------------------------------------*/
VOID wlanDefTxPowerCfg(IN P_ADAPTER_T prAdapter)
{
	UINT_8 i;
	P_GLUE_INFO_T prGlueInfo = prAdapter->prGlueInfo;
	P_SET_TXPWR_CTRL_T prTxpwr;

	ASSERT(prGlueInfo);

	prTxpwr = &prGlueInfo->rTxPwr;

	prTxpwr->c2GLegacyStaPwrOffset = 0;
	prTxpwr->c2GHotspotPwrOffset = 0;
	prTxpwr->c2GP2pPwrOffset = 0;
	prTxpwr->c2GBowPwrOffset = 0;
	prTxpwr->c5GLegacyStaPwrOffset = 0;
	prTxpwr->c5GHotspotPwrOffset = 0;
	prTxpwr->c5GP2pPwrOffset = 0;
	prTxpwr->c5GBowPwrOffset = 0;
	prTxpwr->ucConcurrencePolicy = 0U;
	for (i = 0U; i < 3U; i++)
		prTxpwr->acReserved1[i] = 0;

	for (i = 0U; i < 14U; i++)
		prTxpwr->acTxPwrLimit2G[i] = 63;

	for (i = 0U; i < 4U; i++)
		prTxpwr->acTxPwrLimit5G[i] = 63;

	for (i = 0U; i < 2U; i++)
		prTxpwr->acReserved2[i] = 0;

}

/*----------------------------------------------------------------------------*/
/*!
* @brief This function is to
*        set preferred band configuration corresponding to network type
*
* @param prAdapter      Pointer of Adapter Data Structure
* @param eBand          Given band
* @param ucBssIndex     BSS Info Index
*
* @return none
*/
/*----------------------------------------------------------------------------*/
VOID wlanSetPreferBandByNetwork(IN P_ADAPTER_T prAdapter, IN ENUM_BAND_T eBand, IN UINT_8 ucBssIndex)
{
	ASSERT(prAdapter);
	ASSERT_BOOLEAN(eBand <= BAND_NUM);
	ASSERT_BOOLEAN(ucBssIndex <= MAX_BSS_INDEX);
	ASSERT_BOOLEAN(ucBssIndex <= BSS_INFO_NUM);

	/* 1. set prefer band according to network type */
	prAdapter->aePreferBand[ucBssIndex] = eBand;

	/* 2. remove buffered BSS descriptors correspondingly */
	if (eBand == BAND_2G4)
		scanRemoveBssDescByBandAndNetwork(prAdapter, BAND_5G, ucBssIndex);
	else if (eBand == BAND_5G)
		scanRemoveBssDescByBandAndNetwork(prAdapter, BAND_2G4, ucBssIndex);
}

/*----------------------------------------------------------------------------*/
/*!
* @brief This function is to
*        get channel information corresponding to specified network type
*
* @param prAdapter      Pointer of Adapter Data Structure
* @param ucBssIndex     BSS Info Index
*
* @return channel number
*/
/*----------------------------------------------------------------------------*/
UINT_8 wlanGetChannelNumberByNetwork(IN P_ADAPTER_T prAdapter, IN UINT_8 ucBssIndex)
{
	P_BSS_INFO_T prBssInfo;

	ASSERT(prAdapter);
	ASSERT_BOOLEAN(ucBssIndex <= MAX_BSS_INDEX);

	prBssInfo = GET_BSS_INFO_BY_INDEX(prAdapter, ucBssIndex);

	return prBssInfo->ucPrimaryChannel;
}

/*----------------------------------------------------------------------------*/
/*!
* @brief This function is to
*        check unconfigured system properties and generate related message on
*        scan list to notify users
*
* @param prAdapter      Pointer of Adapter Data Structure
*
* @return WLAN_STATUS_SUCCESS
*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS wlanCheckSystemConfiguration(IN P_ADAPTER_T prAdapter)
{
#if (CFG_NVRAM_EXISTENCE_CHECK == 1) || (CFG_SW_NVRAM_VERSION_CHECK == 1)
	const UINT_8 aucZeroMacAddr[] = NULL_MAC_ADDR;
	const UINT_8 aucBCAddr[] = BC_MAC_ADDR;
	BOOLEAN fgIsConfExist = TRUE;
	BOOLEAN fgGenErrMsg = FALSE;
	P_REG_INFO_T prRegInfo = NULL;
	P_WLAN_BEACON_FRAME_T prBeacon = NULL;
	P_IE_SSID_T prSsid = NULL;
	UINT_32 u4ErrCode = 0;
	UINT_8 aucErrMsg[32];
	PARAM_SSID_T rSsid;
	PARAM_802_11_CONFIG_T rConfiguration;
	PARAM_RATES_EX rSupportedRates;
#endif

	DEBUGFUNC("wlanCheckSystemConfiguration");

	ASSERT(prAdapter);

#if (CFG_NVRAM_EXISTENCE_CHECK == 1)
	if (kalIsConfigurationExist(prAdapter->prGlueInfo) == FALSE) {
		fgIsConfExist = FALSE;
		fgGenErrMsg = TRUE;
	}
#endif

#if (CFG_SW_NVRAM_VERSION_CHECK == 1)
	prRegInfo = kalGetConfiguration(prAdapter->prGlueInfo);

#if (CFG_SUPPORT_PWR_LIMIT_COUNTRY == 1)
	if (fgIsConfExist == TRUE &&
		(prAdapter->rVerInfo.u2Part1CfgPeerVersion > CFG_DRV_OWN_VERSION
		|| prAdapter->rVerInfo.u2Part2CfgPeerVersion >
		CFG_DRV_OWN_VERSION
		|| prAdapter->rVerInfo.u2FwPeerVersion > CFG_DRV_OWN_VERSION
		|| (prAdapter->fgIsEmbbededMacAddrValid == FALSE &&
		(IS_BMCAST_MAC_ADDR(prRegInfo->aucMacAddr)
		|| EQUAL_MAC_ADDR(aucZeroMacAddr, prRegInfo->aucMacAddr)))
		|| prRegInfo->ucTxPwrValid == 0U
		|| prAdapter->fgIsPowerLimitTableValid == FALSE))
		fgGenErrMsg = TRUE;
#else
	if (fgIsConfExist == TRUE &&
		(prAdapter->rVerInfo.u2Part1CfgPeerVersion > CFG_DRV_OWN_VERSION
		|| prAdapter->rVerInfo.u2Part2CfgPeerVersion >
		CFG_DRV_OWN_VERSION
		|| prAdapter->rVerInfo.u2FwPeerVersion > CFG_DRV_OWN_VERSION
		|| (prAdapter->fgIsEmbbededMacAddrValid == FALSE &&
		(IS_BMCAST_MAC_ADDR(prRegInfo->aucMacAddr)
		|| EQUAL_MAC_ADDR(aucZeroMacAddr, prRegInfo->aucMacAddr)))
		|| prRegInfo->ucTxPwrValid == 0U))
		fgGenErrMsg = TRUE;
#endif
#endif

	if (fgGenErrMsg == TRUE) {
		prBeacon = cnmMemAlloc(prAdapter, RAM_TYPE_BUF,
			(UINT_32)(sizeof(WLAN_BEACON_FRAME_T)
			+ sizeof(IE_SSID_T)));

		/* initialization */
		kalMemZero(prBeacon,
			(UINT_32)(sizeof(WLAN_BEACON_FRAME_T)
			+ sizeof(IE_SSID_T)));

		/* prBeacon initialization */
		prBeacon->u2FrameCtrl = MAC_FRAME_BEACON;
		COPY_MAC_ADDR(prBeacon->aucDestAddr, aucBCAddr);
		COPY_MAC_ADDR(prBeacon->aucSrcAddr, aucZeroMacAddr);
		COPY_MAC_ADDR(prBeacon->aucBSSID, aucZeroMacAddr);
		prBeacon->u2BeaconInterval = 100U;
		prBeacon->u2CapInfo = (UINT_16)CAP_INFO_ESS;

		/* prSSID initialization */
		prSsid = (P_IE_SSID_T) (&prBeacon->aucInfoElem[0]);
		prSsid->ucId = ELEM_ID_SSID;

		/* rConfiguration initialization */
		rConfiguration.u4Length =
			(UINT_32)sizeof(PARAM_802_11_CONFIG_T);
		rConfiguration.u4BeaconPeriod = 100U;
		rConfiguration.u4ATIMWindow = 1U;
		rConfiguration.u4DSConfig = 2412U;
		rConfiguration.rFHConfig.u4Length =
			(UINT_32)sizeof(PARAM_802_11_CONFIG_FH_T);

		/* rSupportedRates initialization */
		kalMemZero(rSupportedRates, (UINT_32)sizeof(PARAM_RATES_EX));
	}
#if (CFG_NVRAM_EXISTENCE_CHECK == 1)
#define NVRAM_ERR_MSG "NVRAM WARNING: Err = 0x01"
	if ((kalIsConfigurationExist(prAdapter->prGlueInfo) == FALSE) &&
		(prSsid != NULL)) {
		COPY_SSID((UINT_8 *)&prSsid->aucSSID[0], prSsid->ucLength,
			(UINT_8 *)NVRAM_ERR_MSG,
			(UINT_8) (strlen(NVRAM_ERR_MSG)));

		kalIndicateBssInfo(prAdapter->prGlueInfo,
			(PUINT_8) prBeacon,
			(UINT_32)(OFFSET_OF(WLAN_BEACON_FRAME_T, aucInfoElem) +
			OFFSET_OF(IE_SSID_T, aucSSID) +
			prSsid->ucLength), 1U, 0);

		COPY_SSID(rSsid.aucSsid, rSsid.u4SsidLen, NVRAM_ERR_MSG,
			(UINT_32)strlen(NVRAM_ERR_MSG));
		nicAddScanResult(prAdapter,
			prBeacon->aucBSSID,
			&rSsid,
			0,
			0,
			PARAM_NETWORK_TYPE_FH,
			&rConfiguration,
			NET_TYPE_INFRA,
			rSupportedRates,
			(UINT_16)(OFFSET_OF(WLAN_BEACON_FRAME_T, aucInfoElem)
			+ OFFSET_OF(IE_SSID_T, aucSSID) +
			prSsid->ucLength - WLAN_MAC_MGMT_HEADER_LEN),
			(PUINT_8) ((ULONG) (prBeacon) +
			WLAN_MAC_MGMT_HEADER_LEN));
	}
#endif

#if (CFG_SW_NVRAM_VERSION_CHECK == 1)
#define VER_ERR_MSG     "NVRAM WARNING: Err = 0x%02X"
	if (fgIsConfExist == TRUE) {
		if ((prAdapter->rVerInfo.u2Part1CfgPeerVersion > CFG_DRV_OWN_VERSION
			|| prAdapter->rVerInfo.u2Part2CfgPeerVersion > CFG_DRV_OWN_VERSION
			|| prAdapter->rVerInfo.u2Part1CfgOwnVersion < CFG_DRV_PEER_VERSION
			|| prAdapter->rVerInfo.u2Part2CfgOwnVersion < CFG_DRV_PEER_VERSION	/* NVRAM */
			|| prAdapter->rVerInfo.u2FwPeerVersion > CFG_DRV_OWN_VERSION
			|| prAdapter->rVerInfo.u2FwOwnVersion < CFG_DRV_PEER_VERSION))
			u4ErrCode |= (UINT_32)NVRAM_ERROR_VERSION_MISMATCH;

		if (prRegInfo->ucTxPwrValid == 0U)
			u4ErrCode |= (UINT_32)NVRAM_ERROR_INVALID_TXPWR;

		if (prAdapter->fgIsEmbbededMacAddrValid == FALSE && (IS_BMCAST_MAC_ADDR(prRegInfo->aucMacAddr)
								     || EQUAL_MAC_ADDR(aucZeroMacAddr,
										       prRegInfo->aucMacAddr))) {
			u4ErrCode |= (UINT_32)NVRAM_ERROR_INVALID_MAC_ADDR;
		}
#if CFG_SUPPORT_PWR_LIMIT_COUNTRY
		if (prAdapter->fgIsPowerLimitTableValid == FALSE)
			u4ErrCode |= (UINT_32)NVRAM_POWER_LIMIT_TABLE_INVALID;
#endif
		if ((u4ErrCode != 0U) && (prSsid != NULL)) {
			sprintf(aucErrMsg, VER_ERR_MSG, (unsigned int)u4ErrCode);
			COPY_SSID(prSsid->aucSSID, prSsid->ucLength, aucErrMsg, (UINT_8) (strlen(aucErrMsg)));

			kalIndicateBssInfo(prAdapter->prGlueInfo,
				(PUINT_8) prBeacon,
				(UINT_32)(OFFSET_OF(WLAN_BEACON_FRAME_T,
				aucInfoElem) +
				OFFSET_OF(IE_SSID_T, aucSSID) +
				prSsid->ucLength), 1U, 0);

			COPY_SSID(rSsid.aucSsid, rSsid.u4SsidLen, NVRAM_ERR_MSG,
				(UINT_32)strlen(NVRAM_ERR_MSG));
			nicAddScanResult(prAdapter, prBeacon->aucBSSID, &rSsid, 0, 0,
				PARAM_NETWORK_TYPE_FH, &rConfiguration,
				NET_TYPE_INFRA,
				rSupportedRates,
				(UINT_16)(OFFSET_OF(WLAN_BEACON_FRAME_T,
				aucInfoElem) +
				OFFSET_OF(IE_SSID_T, aucSSID)
				+ prSsid->ucLength -
				WLAN_MAC_MGMT_HEADER_LEN),
				(PUINT_8) ((ULONG) (prBeacon) +
				WLAN_MAC_MGMT_HEADER_LEN));
		}
	}
#endif

	if (fgGenErrMsg == TRUE)
		cnmMemFree(prAdapter, prBeacon);

	return WLAN_STATUS_SUCCESS;
}

WLAN_STATUS
wlanoidQueryBssStatistics(IN P_ADAPTER_T prAdapter,
			  IN PVOID pvQueryBuffer, IN UINT_32 u4QueryBufferLen, OUT PUINT_32 pu4QueryInfoLen)
{
	P_PARAM_GET_BSS_STATISTICS prQueryBssStatistics;
	P_BSS_INFO_T prBssInfo;
	P_STA_RECORD_T prStaRec;
	WLAN_STATUS rResult = WLAN_STATUS_FAILURE;
	UINT_8 ucBssIndex;
	UINT_8 eAci;

	DEBUGFUNC("wlanoidQueryBssStatistics");

	do {
		ASSERT(pvQueryBuffer);

		/* 4 1. Sanity test */
		if ((prAdapter == NULL) || (pu4QueryInfoLen == NULL))
			break;

		if ((u4QueryBufferLen != 0U) && (pvQueryBuffer == NULL))
			break;

		if (u4QueryBufferLen <
			(UINT_32)sizeof(P_PARAM_GET_BSS_STATISTICS)) {
			*pu4QueryInfoLen =
				(UINT_32)sizeof(P_PARAM_GET_BSS_STATISTICS);
			rResult = WLAN_STATUS_BUFFER_TOO_SHORT;
			break;
		}

		prQueryBssStatistics = (P_PARAM_GET_BSS_STATISTICS) pvQueryBuffer;
		*pu4QueryInfoLen = (UINT_32)sizeof(PARAM_GET_BSS_STATISTICS);

		ucBssIndex = prQueryBssStatistics->ucBssIndex;
		prBssInfo = GET_BSS_INFO_BY_INDEX(prAdapter, ucBssIndex);

		if (prBssInfo != NULL) {
			/*AIS*/
			if (prBssInfo->eCurrentOPMode == OP_MODE_INFRASTRUCTURE) {
				prStaRec = prBssInfo->prStaRecOfAP;
				if (prStaRec != NULL) {
					for (eAci = 0U;
					eAci < (UINT_8)WMM_AC_INDEX_NUM;
					eAci++) {
						prQueryBssStatistics->arLinkStatistics[eAci].u4TxMsdu =
						    prStaRec->arLinkStatistics[eAci].u4TxMsdu;
						prQueryBssStatistics->arLinkStatistics[eAci].u4RxMsdu =
						    prStaRec->arLinkStatistics[eAci].u4RxMsdu;
						prQueryBssStatistics->arLinkStatistics[eAci].u4TxDropMsdu =
						    prStaRec->arLinkStatistics[eAci].u4TxDropMsdu +
						    prBssInfo->arLinkStatistics[eAci].u4TxDropMsdu;
						prQueryBssStatistics->arLinkStatistics[eAci].u4TxFailMsdu =
						    prStaRec->arLinkStatistics[eAci].u4TxFailMsdu;
						prQueryBssStatistics->arLinkStatistics[eAci].u4TxRetryMsdu =
						    prStaRec->arLinkStatistics[eAci].u4TxRetryMsdu;
					}
				}
			}
			rResult = WLAN_STATUS_SUCCESS;

			/*P2P */
			/* TODO */

			/*BOW*/
			/* TODO */
		}

	} while (0U != 0U);

	return rResult;

}

VOID wlanDumpBssStatistics(IN P_ADAPTER_T prAdapter, UINT_8 ucBssIdx)
{
	P_BSS_INFO_T prBssInfo;
	P_STA_RECORD_T prStaRec;
	UINT_8 eAci;
	WIFI_WMM_AC_STAT_T arLLStats[WMM_AC_INDEX_NUM];
	UINT_8 ucIdx;

	if (ucBssIdx > MAX_BSS_INDEX) {
		DBGLOG(SW4, WARN, "Invalid BssInfo index[%u], skip dump!\n", ucBssIdx);
		return;
	}

	prBssInfo = GET_BSS_INFO_BY_INDEX(prAdapter, ucBssIdx);
	if (prBssInfo == NULL) {
		DBGLOG(SW4, WARN, "Invalid BssInfo index[%u], skip dump!\n", ucBssIdx);
		return;
	}
	/* <1> fill per-BSS statistics */
#if 0
	 /*AIS*/ if (prBssInfo->eCurrentOPMode == OP_MODE_INFRASTRUCTURE) {
		prStaRec = prBssInfo->prStaRecOfAP;
		if (prStaRec) {
			for (eAci = 0; eAci < WMM_AC_INDEX_NUM; eAci++) {
				prBssInfo->arLinkStatistics[eAci].u4TxMsdu = prStaRec->arLinkStatistics[eAci].u4TxMsdu;
				prBssInfo->arLinkStatistics[eAci].u4RxMsdu = prStaRec->arLinkStatistics[eAci].u4RxMsdu;
				prBssInfo->arLinkStatistics[eAci].u4TxDropMsdu +=
				    prStaRec->arLinkStatistics[eAci].u4TxDropMsdu;
				prBssInfo->arLinkStatistics[eAci].u4TxFailMsdu =
				    prStaRec->arLinkStatistics[eAci].u4TxFailMsdu;
				prBssInfo->arLinkStatistics[eAci].u4TxRetryMsdu =
				    prStaRec->arLinkStatistics[eAci].u4TxRetryMsdu;
			}
		}
	}
#else
	for (eAci = 0U; eAci < (UINT_8)WMM_AC_INDEX_NUM; eAci++) {
		arLLStats[eAci].u4TxMsdu = prBssInfo->arLinkStatistics[eAci].u4TxMsdu;
		arLLStats[eAci].u4RxMsdu = prBssInfo->arLinkStatistics[eAci].u4RxMsdu;
		arLLStats[eAci].u4TxDropMsdu = prBssInfo->arLinkStatistics[eAci].u4TxDropMsdu;
		arLLStats[eAci].u4TxFailMsdu = prBssInfo->arLinkStatistics[eAci].u4TxFailMsdu;
		arLLStats[eAci].u4TxRetryMsdu = prBssInfo->arLinkStatistics[eAci].u4TxRetryMsdu;
	}

	for (ucIdx = 0; ucIdx < CFG_NUM_OF_STA_RECORD; ucIdx++) {
		prStaRec = cnmGetStaRecByIndex(prAdapter, ucIdx);
		if (prStaRec != NULL) {
			prBssInfo = GET_BSS_INFO_BY_INDEX(prAdapter, prStaRec->ucBssIndex);

			if (prBssInfo == NULL)
				continue;

			for (eAci = 0U; eAci <
				(UINT_8)WMM_AC_INDEX_NUM; eAci++) {
				arLLStats[eAci].u4TxMsdu += prStaRec->arLinkStatistics[eAci].u4TxMsdu;
				arLLStats[eAci].u4RxMsdu += prStaRec->arLinkStatistics[eAci].u4RxMsdu;
				arLLStats[eAci].u4TxDropMsdu += prStaRec->arLinkStatistics[eAci].u4TxDropMsdu;
				arLLStats[eAci].u4TxFailMsdu += prStaRec->arLinkStatistics[eAci].u4TxFailMsdu;
				arLLStats[eAci].u4TxRetryMsdu += prStaRec->arLinkStatistics[eAci].u4TxRetryMsdu;
			}
		}
	}
#endif

	/* <2>Dump BSS statistics */
	for (eAci = 0U; eAci < (UINT_8)WMM_AC_INDEX_NUM; eAci++) {
		DBGLOG(BSS, TRACE, "LLS BSS[%u] AC[%u]: T[%u] R[%u] T_D[%u] T_F[%u]\n",
				   prBssInfo->ucBssIndex, eAci, arLLStats[eAci].u4TxMsdu,
				   arLLStats[eAci].u4RxMsdu, arLLStats[eAci].u4TxDropMsdu,
				   arLLStats[eAci].u4TxFailMsdu);
	}
}

VOID wlanDumpAllBssStatistics(IN P_ADAPTER_T prAdapter)
{
	P_BSS_INFO_T prBssInfo;
	/* ENUM_WMM_ACI_T eAci; */
	UINT_32 ucIdx;

	/* wlanUpdateAllBssStatistics(prAdapter); */

	for (ucIdx = 0; ucIdx < BSS_INFO_NUM; ucIdx++) {
		prBssInfo = GET_BSS_INFO_BY_INDEX(prAdapter, ucIdx);
		if (!IS_BSS_ACTIVE(prBssInfo)) {
			DBGLOG(SW4, TRACE, "Invalid BssInfo index[%u], skip dump!\n", ucIdx);
			continue;
		}

		wlanDumpBssStatistics(prAdapter, (UINT_8)ucIdx);
	}
}

WLAN_STATUS
wlanoidQueryStaStatistics(IN P_ADAPTER_T prAdapter,
			  IN PVOID pvQueryBuffer, IN UINT_32 u4QueryBufferLen, OUT PUINT_32 pu4QueryInfoLen)
{
	WLAN_STATUS rResult = WLAN_STATUS_FAILURE;
	P_STA_RECORD_T prStaRec, prTempStaRec;
	P_PARAM_GET_STA_STATISTICS prQueryStaStatistics;
	UINT_8 ucStaRecIdx;
	P_QUE_MGT_T prQM = &prAdapter->rQM;
	CMD_GET_STA_STATISTICS_T rQueryCmdStaStatistics;
	UINT_8 ucIdx;
	UINT_8 eAci;

	DEBUGFUNC("wlanoidQueryStaStatistics");
	do {
		ASSERT(pvQueryBuffer);

		/* 4 1. Sanity test */
		if ((prAdapter == NULL) || (pu4QueryInfoLen == NULL))
			break;

		if ((u4QueryBufferLen != 0U) && (pvQueryBuffer == NULL))
			break;

		if (u4QueryBufferLen <
			(UINT_32)sizeof(PARAM_GET_STA_STA_STATISTICS)) {
			*pu4QueryInfoLen =
				(UINT_32)sizeof(PARAM_GET_STA_STA_STATISTICS);
			rResult = WLAN_STATUS_BUFFER_TOO_SHORT;
			break;
		}

		prQueryStaStatistics = (P_PARAM_GET_STA_STATISTICS) pvQueryBuffer;
		*pu4QueryInfoLen =
			(UINT_32)sizeof(PARAM_GET_STA_STA_STATISTICS);

		/* 4 5. Get driver global QM counter */
		for (ucIdx = (UINT_8)TC0_INDEX;
			ucIdx <= (UINT_8)TC3_INDEX; ucIdx++) {
			prQueryStaStatistics->au4TcAverageQueLen[ucIdx] = prQM->au4AverageQueLen[ucIdx];
			prQueryStaStatistics->au4TcCurrentQueLen[ucIdx] = prQM->au4CurrentTcResource[ucIdx];
		}

		/* 4 2. Get StaRec by MAC address */
		prStaRec = NULL;

		for (ucStaRecIdx = 0; ucStaRecIdx < CFG_NUM_OF_STA_RECORD; ucStaRecIdx++) {
			prTempStaRec = &(prAdapter->arStaRec[ucStaRecIdx]);
			if (prTempStaRec->fgIsValid == TRUE &&
				prTempStaRec->fgIsInUse == TRUE) {
				if (EQUAL_MAC_ADDR(prTempStaRec->aucMacAddr, prQueryStaStatistics->aucMacAddr)) {
					prStaRec = prTempStaRec;
					break;
				}
			}
		}

		if (prStaRec == NULL) {
			rResult = WLAN_STATUS_INVALID_DATA;
			break;
		}

		prQueryStaStatistics->u4Flag |= (UINT_32)BIT(0U);

#if CFG_ENABLE_PER_STA_STATISTICS

		DBGLOG(TX, INFO, "skbToDriver %lld, skbFreed: %lld\n",
			prAdapter->prGlueInfo->u8SkbToDriver,
			prAdapter->prGlueInfo->u8SkbFreed);
		prAdapter->prGlueInfo->u8SkbFreed = 0U;
		prAdapter->prGlueInfo->u8SkbToDriver = 0U;

		/* 4 3. Get driver statistics */
		prQueryStaStatistics->u4TxTotalCount = prStaRec->u4TotalTxPktsNumber;
		prQueryStaStatistics->u4RxTotalCount = prStaRec->u4TotalRxPktsNumber;
		prQueryStaStatistics->u4TxExceedThresholdCount = prStaRec->u4ThresholdCounter;
		prQueryStaStatistics->u4TxMaxTime = prStaRec->u4MaxTxPktsTime;
		prQueryStaStatistics->u4TxMaxHifTime = prStaRec->u4MaxTxPktsHifTime;

		if (prStaRec->u4TotalTxPktsNumber != 0U) {
			prQueryStaStatistics->u4TxAverageProcessTime =
			    (prStaRec->u4TotalTxPktsTime / prStaRec->u4TotalTxPktsNumber);
			prQueryStaStatistics->u4TxAverageHifTime =
				prStaRec->u4TotalTxPktsHifTxTime / prStaRec->u4TotalTxPktsNumber;
		} else
			prQueryStaStatistics->u4TxAverageProcessTime = 0U;

		/*link layer statistics */
		for (eAci = 0U; eAci < (UINT_8)WMM_AC_INDEX_NUM; eAci++) {
			prQueryStaStatistics->arLinkStatistics[eAci].u4TxMsdu =
			    prStaRec->arLinkStatistics[eAci].u4TxMsdu;
			prQueryStaStatistics->arLinkStatistics[eAci].u4RxMsdu =
			    prStaRec->arLinkStatistics[eAci].u4RxMsdu;
			prQueryStaStatistics->arLinkStatistics[eAci].u4TxDropMsdu =
			    prStaRec->arLinkStatistics[eAci].u4TxDropMsdu;
		}

		for (ucIdx = (UINT_8)TC0_INDEX;
			ucIdx <= (UINT_8)TC3_INDEX; ucIdx++) {
			prQueryStaStatistics->au4TcResourceEmptyCount[ucIdx] =
			    prQM->au4QmTcResourceEmptyCounter[prStaRec->ucBssIndex][ucIdx];
			/* Reset */
			prQM->au4QmTcResourceEmptyCounter
				[prStaRec->ucBssIndex][ucIdx] = 0U;
			prQueryStaStatistics->au4TcResourceBackCount[ucIdx] =
				prQM->au4QmTcResourceBackCounter[ucIdx];
			prQM->au4QmTcResourceBackCounter[ucIdx] = 0U;
			prQueryStaStatistics->au4DequeueNoTcResource[ucIdx] =
				prQM->au4DequeueNoTcResourceCounter[ucIdx];
			prQM->au4DequeueNoTcResourceCounter[ucIdx] = 0U;
			prQueryStaStatistics->au4TcResourceUsedPageCount[ucIdx] =
				prQM->au4QmTcUsedPageCounter[ucIdx];
			prQM->au4QmTcUsedPageCounter[ucIdx] = 0U;
			prQueryStaStatistics->au4TcResourceWantedPageCount[ucIdx] =
				prQM->au4QmTcWantedPageCounter[ucIdx];
			prQM->au4QmTcWantedPageCounter[ucIdx] = 0U;
		}

		prQueryStaStatistics->u4EnqueueCounter = prQM->u4EnqueueCounter;
		prQueryStaStatistics->u4EnqueueStaCounter = prStaRec->u4EnqueueCounter;

		prQueryStaStatistics->u4DequeueCounter = prQM->u4DequeueCounter;
		prQueryStaStatistics->u4DequeueStaCounter = prStaRec->u4DeqeueuCounter;

		prQueryStaStatistics->IsrCnt = prAdapter->prGlueInfo->IsrCnt;
		prQueryStaStatistics->IsrPassCnt = prAdapter->prGlueInfo->IsrPassCnt;
		prQueryStaStatistics->TaskIsrCnt = prAdapter->prGlueInfo->TaskIsrCnt;

		prQueryStaStatistics->IsrAbnormalCnt = prAdapter->prGlueInfo->IsrAbnormalCnt;
		prQueryStaStatistics->IsrSoftWareCnt = prAdapter->prGlueInfo->IsrSoftWareCnt;
		prQueryStaStatistics->IsrRxCnt = prAdapter->prGlueInfo->IsrRxCnt;
		prQueryStaStatistics->IsrTxCnt = prAdapter->prGlueInfo->IsrTxCnt;


		/* 4 4.1 Reset statistics */
		if (prQueryStaStatistics->ucReadClear != 0U) {
			prStaRec->u4ThresholdCounter = 0U;
			prStaRec->u4TotalTxPktsNumber = 0U;

			prStaRec->u4TotalTxPktsTime = 0U;
			prStaRec->u4TotalTxPktsHifTxTime = 0U;

			prStaRec->u4TotalRxPktsNumber = 0U;

			prStaRec->u4MaxTxPktsTime = 0U;
			prStaRec->u4MaxTxPktsHifTime = 0U;
			prQM->u4EnqueueCounter = 0U;
			prQM->u4DequeueCounter = 0U;
			prStaRec->u4EnqueueCounter = 0U;
			prStaRec->u4DeqeueuCounter = 0U;

			prAdapter->prGlueInfo->IsrCnt = 0U;
			prAdapter->prGlueInfo->IsrPassCnt = 0U;
			prAdapter->prGlueInfo->TaskIsrCnt = 0U;

			prAdapter->prGlueInfo->IsrAbnormalCnt = 0U;
			prAdapter->prGlueInfo->IsrSoftWareCnt = 0U;
			prAdapter->prGlueInfo->IsrRxCnt = 0U;
			prAdapter->prGlueInfo->IsrTxCnt = 0U;
		}
		/*link layer statistics */
		if (prQueryStaStatistics->ucLlsReadClear != 0U) {
			for (eAci = 0U;
				eAci < (UINT_8)WMM_AC_INDEX_NUM; eAci++) {
				prStaRec->arLinkStatistics[eAci].u4TxMsdu = 0U;
				prStaRec->arLinkStatistics[eAci].u4RxMsdu = 0U;
				prStaRec->arLinkStatistics[eAci].u4TxDropMsdu
					= 0U;
			}
		}
#endif

		for (ucIdx = (UINT_8)TC0_INDEX;
			ucIdx <= (UINT_8)TC3_INDEX; ucIdx++)
			prQueryStaStatistics->au4TcQueLen[ucIdx] = prStaRec->arTxQueue[ucIdx].u4NumElem;

		rResult = WLAN_STATUS_SUCCESS;

		/* 4 6. Ensure FW supports get station link status */
		if ((prAdapter->u4FwCompileFlag0 &
			(UINT_32)COMPILE_FLAG0_GET_STA_LINK_STATUS) != 0U) {

			rQueryCmdStaStatistics.ucIndex = prStaRec->ucIndex;
			COPY_MAC_ADDR(rQueryCmdStaStatistics.aucMacAddr, prQueryStaStatistics->aucMacAddr);
			rQueryCmdStaStatistics.ucReadClear = prQueryStaStatistics->ucReadClear;
			rQueryCmdStaStatistics.ucLlsReadClear = prQueryStaStatistics->ucLlsReadClear;

			rResult = wlanSendSetQueryCmd(prAdapter,
				(UINT_8)CMD_ID_GET_STA_STATISTICS,
				FALSE,
				TRUE,
				TRUE,
				nicCmdEventQueryStaStatistics,
				nicOidCmdTimeoutCommon,
				(UINT_32)sizeof(CMD_GET_STA_STATISTICS_T),
				(PUINT_8) &rQueryCmdStaStatistics,
				pvQueryBuffer, u4QueryBufferLen);

			prQueryStaStatistics->u4Flag |= (UINT_32)BIT(1U);
		} else {
			rResult = WLAN_STATUS_NOT_SUPPORTED;
		}

	} while (0 != 0);

	return rResult;
}				/* wlanoidQueryP2pVersion */

/*----------------------------------------------------------------------------*/
/*!
* @brief This function is to query Nic resource information
*
* @param prAdapter      Pointer of Adapter Data Structure
*
* @return WLAN_STATUS_SUCCESS
*/
/*----------------------------------------------------------------------------*/
VOID wlanQueryNicResourceInformation(IN P_ADAPTER_T prAdapter)
{
	/* 3 1. Get Nic resource information from FW */

	/* 3 2. Setup resource parameter */

	/* 3 3. Reset Tx resource */
	if (nicTxResetResource(prAdapter) == WLAN_STATUS_SUCCESS)
		DBGLOG(INIT, TRACE, "nicTxResetResource\n");
}

#if 0
/*----------------------------------------------------------------------------*/
/*!
* @brief This function is to SET network interface index for a network interface.
*           A network interface is a TX/RX data port hooked to OS.
*
* @param prGlueInfo                     Pointer of prGlueInfo Data Structure
* @param ucNetInterfaceIndex            Index of network interface
* @param ucBssIndex                     Index of BSS
*
* @return VOID
*/
/*----------------------------------------------------------------------------*/
VOID wlanBindNetInterface(IN P_GLUE_INFO_T prGlueInfo, IN UINT_8 ucNetInterfaceIndex, IN PVOID pvNetInterface)
{
	P_NET_INTERFACE_INFO_T prNetIfInfo;

	prNetIfInfo = &prGlueInfo->arNetInterfaceInfo[ucNetInterfaceIndex];

	prNetIfInfo->pvNetInterface = pvNetInterface;
}
#endif
/*----------------------------------------------------------------------------*/
/*!
* @brief This function is to SET BSS index for a network interface.
*           A network interface is a TX/RX data port hooked to OS.
*
* @param prGlueInfo                     Pointer of prGlueInfo Data Structure
* @param ucNetInterfaceIndex            Index of network interface
* @param ucBssIndex                     Index of BSS
*
* @return VOID
*/
/*----------------------------------------------------------------------------*/
VOID wlanBindBssIdxToNetInterface(IN P_GLUE_INFO_T prGlueInfo, IN UINT_8 ucBssIndex, IN PVOID pvNetInterface)
{
	P_NET_INTERFACE_INFO_T prNetIfInfo;

	if (ucBssIndex >= MAX_BSS_INDEX)
		return;

	prNetIfInfo = &prGlueInfo->arNetInterfaceInfo[ucBssIndex];

	prNetIfInfo->ucBssIndex = ucBssIndex;
	prNetIfInfo->pvNetInterface = pvNetInterface;
	/* prGlueInfo->aprBssIdxToNetInterfaceInfo[ucBssIndex] = prNetIfInfo; */
}

#if 0
/*----------------------------------------------------------------------------*/
/*!
* @brief This function is to GET BSS index for a network interface.
*           A network interface is a TX/RX data port hooked to OS.
*
* @param prGlueInfo                     Pointer of prGlueInfo Data Structure
* @param ucNetInterfaceIndex       Index of network interface
*
* @return UINT_8                         Index of BSS
*/
/*----------------------------------------------------------------------------*/
UINT_8 wlanGetBssIdxByNetInterface(IN P_GLUE_INFO_T prGlueInfo, IN PVOID pvNetInterface)
{
	UINT_8 ucIdx = 0;

	for (ucIdx = 0; ucIdx < HW_BSSID_NUM; ucIdx++) {
		if (prGlueInfo->arNetInterfaceInfo[ucIdx].pvNetInterface == pvNetInterface)
			break;
	}

	return ucIdx;
}
#endif
/*----------------------------------------------------------------------------*/
/*!
* @brief This function is to GET network interface for a BSS.
*           A network interface is a TX/RX data port hooked to OS.
*
* @param prGlueInfo                     Pointer of prGlueInfo Data Structure
* @param ucBssIndex                     Index of BSS
*
* @return PVOID                         pointer of network interface structure
*/
/*----------------------------------------------------------------------------*/
PVOID wlanGetNetInterfaceByBssIdx(IN P_GLUE_INFO_T prGlueInfo, IN UINT_8 ucBssIndex)
{
	if (ucBssIndex < HW_BSSID_NUM)
		return prGlueInfo->arNetInterfaceInfo[ucBssIndex].pvNetInterface;
	return NULL;
}

UINT_8 wlanGetBssIdx(struct net_device *ndev)
{
	if (ndev) {
		struct _NETDEV_PRIVATE_GLUE_INFO *prNetDevPrivate
			= (struct _NETDEV_PRIVATE_GLUE_INFO *)
			netdev_priv(ndev);

		DBGLOG(REQ, LOUD,
			"ucBssIndex = %d\n",
			prNetDevPrivate->ucBssIdx);

		return prNetDevPrivate->ucBssIdx;
	}

	DBGLOG(REQ, LOUD,
		"ucBssIndex = 0xff\n");

	return 0xff;
}

/*----------------------------------------------------------------------------*/
/*!
* @brief This function is to get BSS-INDEX for AIS network.
*
* @param prAdapter  Pointer of ADAPTER_T
*
* @return value, as corresponding index of BSS
*/
/*----------------------------------------------------------------------------*/
UINT_8 wlanGetAisBssIndex(IN P_ADAPTER_T prAdapter)
{
	ASSERT(prAdapter);

	return prAdapter->prAisBssInfo->ucBssIndex;
}

/*----------------------------------------------------------------------------*/
/*!
* @brief This function is to initialize WLAN feature options
*
* @param prAdapter  Pointer of ADAPTER_T
*
* @return none
*/
/*----------------------------------------------------------------------------*/
VOID wlanInitFeatureOption(IN P_ADAPTER_T prAdapter)
{
	P_WIFI_VAR_T prWifiVar = &prAdapter->rWifiVar;
	P_QUE_MGT_T prQM = &prAdapter->rQM;

	/* Feature options will be filled by config file */

	prWifiVar->ucQoS = (UINT_8) wlanCfgGetUint32(prAdapter, "Qos",
		(UINT_32)FEATURE_ENABLED);

	prWifiVar->ucStaHt = (UINT_8) wlanCfgGetUint32(prAdapter, "StaHT",
		(UINT_32)FEATURE_ENABLED);
	prWifiVar->ucStaVht = (UINT_8) wlanCfgGetUint32(prAdapter, "StaVHT",
		(UINT_32)FEATURE_ENABLED);

	prWifiVar->ucApHt = (UINT_8) wlanCfgGetUint32(prAdapter, "ApHT",
		(UINT_32)FEATURE_ENABLED);
	prWifiVar->ucApVht = (UINT_8) wlanCfgGetUint32(prAdapter, "ApVHT",
		(UINT_32)FEATURE_ENABLED);

	prWifiVar->ucP2pGoHt = (UINT_8) wlanCfgGetUint32(prAdapter, "P2pGoHT",
		(UINT_32)FEATURE_ENABLED);
	prWifiVar->ucP2pGoVht = (UINT_8) wlanCfgGetUint32(prAdapter, "P2pGoVHT",
		(UINT_32)FEATURE_ENABLED);

	prWifiVar->ucP2pGcHt = (UINT_8) wlanCfgGetUint32(prAdapter, "P2pGcHT",
		(UINT_32)FEATURE_ENABLED);
	prWifiVar->ucP2pGcVht = (UINT_8) wlanCfgGetUint32(prAdapter, "P2pGcVHT",
		(UINT_32)FEATURE_ENABLED);

	prWifiVar->ucAmpduRx = (UINT_8) wlanCfgGetUint32(prAdapter, "AmpduRx",
		(UINT_32)FEATURE_ENABLED);
	prWifiVar->ucAmpduTx = (UINT_8) wlanCfgGetUint32(prAdapter, "AmpduTx",
		(UINT_32)FEATURE_ENABLED);

	prWifiVar->ucAmsduInAmpduRx = (UINT_8) wlanCfgGetUint32(prAdapter,
		"AmsduInAmpduRx", (UINT_32)FEATURE_ENABLED);
	prWifiVar->ucHtAmsduInAmpduRx = (UINT_8) wlanCfgGetUint32(prAdapter,
		"HtAmsduInAmpduRx", (UINT_32)FEATURE_DISABLED);
	prWifiVar->ucVhtAmsduInAmpduRx = (UINT_8) wlanCfgGetUint32(prAdapter,
		"VhtAmsduInAmpduRx", (UINT_32)FEATURE_ENABLED);

	prWifiVar->ucTspec = (UINT_8) wlanCfgGetUint32(prAdapter, "Tspec",
		(UINT_32)FEATURE_DISABLED);

	prWifiVar->ucUapsd = (UINT_8) wlanCfgGetUint32(prAdapter, "Uapsd",
		(UINT_32)FEATURE_ENABLED);
	prWifiVar->ucStaUapsd = (UINT_8) wlanCfgGetUint32(prAdapter, "StaUapsd",
		(UINT_32)FEATURE_DISABLED);
	prWifiVar->ucApUapsd = (UINT_8) wlanCfgGetUint32(prAdapter, "ApUapsd",
		(UINT_32)FEATURE_DISABLED);
	prWifiVar->ucP2pUapsd = (UINT_8) wlanCfgGetUint32(prAdapter, "P2pUapsd",
		(UINT_32)FEATURE_ENABLED);

	prWifiVar->ucTxShortGI = (UINT_8) wlanCfgGetUint32(prAdapter, "SgiTx",
		(UINT_32)FEATURE_ENABLED);
	prWifiVar->ucRxShortGI = (UINT_8) wlanCfgGetUint32(prAdapter, "SgiRx",
		(UINT_32)FEATURE_ENABLED);

	prWifiVar->ucTxLdpc = (UINT_8) wlanCfgGetUint32(prAdapter, "LdpcTx",
		(UINT_32)FEATURE_ENABLED);
	prWifiVar->ucRxLdpc = (UINT_8) wlanCfgGetUint32(prAdapter, "LdpcRx",
		(UINT_32)FEATURE_ENABLED);

	prWifiVar->ucTxStbc = (UINT_8) wlanCfgGetUint32(prAdapter, "StbcTx",
		(UINT_32)FEATURE_DISABLED);
	prWifiVar->ucRxStbc = (UINT_8) wlanCfgGetUint32(prAdapter, "StbcRx",
		(UINT_32)FEATURE_ENABLED);

	prWifiVar->ucTxGf = (UINT_8) wlanCfgGetUint32(prAdapter, "GfTx",
		(UINT_32)FEATURE_ENABLED);
	prWifiVar->ucRxGf = (UINT_8) wlanCfgGetUint32(prAdapter, "GfRx",
		(UINT_32)FEATURE_ENABLED);

	prWifiVar->ucSigTaRts = (UINT_8) wlanCfgGetUint32(prAdapter, "SigTaRts",
		(UINT_32)FEATURE_DISABLED);
	prWifiVar->ucDynBwRts = (UINT_8) wlanCfgGetUint32(prAdapter, "DynBwRts",
		(UINT_32)FEATURE_DISABLED);
	prWifiVar->ucTxopPsTx = (UINT_8) wlanCfgGetUint32(prAdapter, "TxopPsTx",
		(UINT_32)FEATURE_DISABLED);

	prWifiVar->ucStaHtBfee = (UINT_8) wlanCfgGetUint32(prAdapter,
		"StaHTBfee", (UINT_32)FEATURE_DISABLED);
	prWifiVar->ucStaVhtBfee = (UINT_8) wlanCfgGetUint32(prAdapter,
		"StaVHTBfee", (UINT_32)FEATURE_ENABLED);
	prWifiVar->ucStaBfer = (UINT_8) wlanCfgGetUint32(prAdapter, "StaBfer",
		(UINT_32)FEATURE_DISABLED);
#ifdef MT6630
	prWifiVar->ucStaVhtMuBfee = (UINT_8) wlanCfgGetUint32(prAdapter,
		"StaVHTMuBfee", (UINT_32)FEATURE_DISABLED);
#else
	prWifiVar->ucStaVhtMuBfee = (UINT_8) wlanCfgGetUint32(prAdapter,
		"StaVHTMuBfee", (UINT_32)FEATURE_ENABLED);
#endif

	prWifiVar->ucApWpsMode =
		(UINT_8) wlanCfgGetUint32(prAdapter, "ApWpsMode", 0U);
	prWifiVar->ucCert11nMode =
		(UINT_8)wlanCfgGetUint32(prAdapter, "Cert11nMode", 0U);
	DBGLOG(INIT, LOUD, "CFG_FILE: ucApWpsMode = %u, ucCert11nMode = %u\n",
		prWifiVar->ucApWpsMode, prWifiVar->ucCert11nMode);

	prWifiVar->ucThreadScheduling =
		(UINT_8) wlanCfgGetUint32(prAdapter, "ThreadSched", 0U);
	prWifiVar->ucThreadPriority =
	    (UINT_8) wlanCfgGetUint32(prAdapter, "ThreadPriority",
	    (UINT_32)WLAN_TX_THREAD_TASK_PRIORITY);
	prWifiVar->cThreadNice = (INT_8) wlanCfgGetInt32(prAdapter,
		"ThreadNice", WLAN_TX_THREAD_TASK_NICE);

	//modify for TGn WMM
	prAdapter->rQM.u4MaxForwardBuffer =
		(UINT_32) wlanCfgGetUint32(prAdapter, "ApForwardBufferCnt",
		QM_FWD_PKT_QUE_HIGH_THRESHOLD);
	prAdapter->rQM.u4MaxForwardBufferForLowUP =
		(UINT_32) wlanCfgGetUint32(prAdapter,
		"ApForwardBufferCntForLowUP", QM_FWD_PKT_QUE_LOW_THRESHOLD);

	//prAdapter->rQM.u4MaxForwardBufferCount =
		//(UINT_32) wlanCfgGetUint32(prAdapter,
		//"ApForwardBufferCnt", QM_FWD_PKT_QUE_THRESHOLD);


	/*
	 * AP channel setting
	 * 0: auto
	 */
	prWifiVar->ucApChannel =
	(UINT_8) wlanCfgGetUint32(prAdapter, "ApChannel", 0U);

	/*
	 * 0: SCN
	 * 1: SCA
	 * 2: RES
	 * 3: SCB
	 */
	prWifiVar->ucApSco = (UINT_8) wlanCfgGetUint32(prAdapter, "ApSco", 0U);
	prWifiVar->ucP2pGoSco =
		(UINT_8) wlanCfgGetUint32(prAdapter, "P2pGoSco", 0U);

	/*
	 * Max bandwidth setting
	 * 0: 20Mhz
	 * 1: 40Mhz
	 * 2: 80Mhz
	 * 3: 80+80 or 160Mhz
	 * Note: For VHT STA, BW 80Mhz is a must!
	 */
	prWifiVar->ucStaBandwidth = (UINT_8) wlanCfgGetUint32(prAdapter,
		"StaBw", (UINT_32)MAX_BW_80MHZ);
	prWifiVar->ucSta2gBandwidth = (UINT_8) wlanCfgGetUint32(prAdapter,
		"Sta2gBw", (UINT_32)MAX_BW_20MHZ);
	prWifiVar->ucSta5gBandwidth = (UINT_8) wlanCfgGetUint32(prAdapter,
		"Sta5gBw", (UINT_32)MAX_BW_80MHZ);
	prWifiVar->ucAp2gBandwidth = (UINT_8) wlanCfgGetUint32(prAdapter,
		"Ap2gBw", (UINT_32)MAX_BW_20MHZ);
	prWifiVar->ucAp5gBandwidth = (UINT_8) wlanCfgGetUint32(prAdapter,
		"Ap5gBw", (UINT_32)MAX_BW_80MHZ);
	prWifiVar->ucP2p2gBandwidth = (UINT_8) wlanCfgGetUint32(prAdapter,
		"P2p2gBw", (UINT_32)MAX_BW_20MHZ);
	prWifiVar->ucP2p5gBandwidth = (UINT_8) wlanCfgGetUint32(prAdapter,
		"P2p5gBw", (UINT_32)MAX_BW_40MHZ);
	prWifiVar->ucNSS = (UINT_8) wlanCfgGetUint32(prAdapter, "Nss", 1);
	prWifiVar->ucStaDisconnectDetectTh =
		(UINT_8) wlanCfgGetUint32(prAdapter,
		"StaDisconnectDetectTh", 0U);
	prWifiVar->ucApDisconnectDetectTh =
		(UINT_8) wlanCfgGetUint32(prAdapter,
		"ApDisconnectDetectTh", 0U);
	prWifiVar->ucP2pDisconnectDetectTh =
		(UINT_8) wlanCfgGetUint32(prAdapter,
		"P2pDisconnectDetectTh", 0U);

	prWifiVar->ucTcRestrict =
		(UINT_8) wlanCfgGetUint32(prAdapter, "TcRestrict", 0xFFU);
	/* Max Tx dequeue limit: 0 => auto */
	prWifiVar->u4MaxTxDeQLimit =
	(UINT_32) wlanCfgGetUint32(prAdapter, "MaxTxDeQLimit", 0x0U);
	prWifiVar->ucAlwaysResetUsedRes =
		(UINT_8) wlanCfgGetUint32(prAdapter,
		"AlwaysResetUsedRes", 0x0U);

#if CFG_SUPPORT_MTK_SYNERGY
	prWifiVar->ucMtkOui = (UINT_8) wlanCfgGetUint32(prAdapter, "MtkOui",
		(UINT_32)FEATURE_ENABLED);
	prWifiVar->u4MtkOuiCap =
		(UINT_32) wlanCfgGetUint32(prAdapter, "MtkOuiCap", 0U);
	prWifiVar->aucMtkFeature[0] = 0xffU;
	prWifiVar->aucMtkFeature[1] = 0xffU;
	prWifiVar->aucMtkFeature[2] = 0xffU;
	prWifiVar->aucMtkFeature[3] = 0xffU;
#endif

	prWifiVar->ucCmdRsvResource = (UINT_8) wlanCfgGetUint32(prAdapter,
	"TxCmdRsv", (UINT_32)QM_CMD_RESERVED_THRESHOLD);
	prWifiVar->u4MgmtQueueDelayTimeout =
		(UINT_32) wlanCfgGetUint32(prAdapter, "TxMgmtQueTO",
		(UINT_32)QM_MGMT_QUEUED_TIMEOUT);	/* ms */

	/* Performance related */
	prWifiVar->u4HifIstLoopCount =
		(UINT_32) wlanCfgGetUint32(prAdapter,
		"IstLoop", (UINT_32)CFG_IST_LOOP_COUNT);
	prWifiVar->u4Rx2OsLoopCount =
		(UINT_32) wlanCfgGetUint32(prAdapter, "Rx2OsLoop", 4U);
	prWifiVar->u4HifTxloopCount =
		(UINT_32) wlanCfgGetUint32(prAdapter, "HifTxLoop", 1U);
	prWifiVar->u4TxFromOsLoopCount =
		(UINT_32) wlanCfgGetUint32(prAdapter, "OsTxLoop", 1U);
	prWifiVar->u4TxRxLoopCount =
		(UINT_32) wlanCfgGetUint32(prAdapter, "Rx2ReorderLoop", 1U);

	prWifiVar->u4NetifStopTh =
	    (UINT_32) wlanCfgGetUint32(prAdapter, "NetifStopTh",
	    (UINT_32)CFG_TX_STOP_NETIF_PER_QUEUE_THRESHOLD);
	prWifiVar->u4NetifStartTh =
	    (UINT_32) wlanCfgGetUint32(prAdapter, "NetifStartTh",
	    (UINT_32)CFG_TX_START_NETIF_PER_QUEUE_THRESHOLD);
	prWifiVar->ucTxBaSize =
		(UINT_8) wlanCfgGetUint32(prAdapter, "TxBaSize", 64U);
	prWifiVar->ucRxHtBaSize =
		(UINT_8) wlanCfgGetUint32(prAdapter, "RxHtBaSize", 64U);
#ifdef MT6630
	prWifiVar->ucRxVhtBaSize =
	(UINT_8) wlanCfgGetUint32(prAdapter, "RxVhtBaSize", 32U);
#else
	prWifiVar->ucRxVhtBaSize =
	(UINT_8) wlanCfgGetUint32(prAdapter, "RxVhtBaSize", 64U);
#endif
	/* Tx Buffer Management */
	prWifiVar->ucExtraTxDone =
	(UINT_8) wlanCfgGetUint32(prAdapter, "ExtraTxDone", 1U);
	prWifiVar->ucTxDbg = (UINT_8) wlanCfgGetUint32(prAdapter, "TxDbg", 0U);

	kalMemZero(prWifiVar->au4TcPageCount,
		(UINT_32)sizeof(prWifiVar->au4TcPageCount));

	prWifiVar->au4TcPageCount[TC0_INDEX] = (UINT_32) wlanCfgGetUint32(
		prAdapter, "Tc0Page", (UINT_32)NIC_TX_PAGE_COUNT_TC0);
	prWifiVar->au4TcPageCount[TC1_INDEX] = (UINT_32) wlanCfgGetUint32(
		prAdapter, "Tc1Page", (UINT_32)NIC_TX_PAGE_COUNT_TC1);
	prWifiVar->au4TcPageCount[TC2_INDEX] = (UINT_32) wlanCfgGetUint32(
		prAdapter, "Tc2Page", (UINT_32)NIC_TX_PAGE_COUNT_TC2);
	prWifiVar->au4TcPageCount[TC3_INDEX] = (UINT_32) wlanCfgGetUint32(
		prAdapter, "Tc3Page", (UINT_32)NIC_TX_PAGE_COUNT_TC3);
	prWifiVar->au4TcPageCount[TC4_INDEX] = (UINT_32) wlanCfgGetUint32(
		prAdapter, "Tc4Page", (UINT_32)NIC_TX_PAGE_COUNT_TC4);
	prWifiVar->au4TcPageCount[TC5_INDEX] = (UINT_32) wlanCfgGetUint32(
		prAdapter, "Tc5Page", (UINT_32)NIC_TX_PAGE_COUNT_TC5);

	prQM->au4MinReservedTcResource[TC0_INDEX] =
	    (UINT_32) wlanCfgGetUint32(prAdapter, "Tc0MinRsv",
	    (UINT_32)QM_MIN_RESERVED_TC0_RESOURCE);
	prQM->au4MinReservedTcResource[TC1_INDEX] =
	    (UINT_32) wlanCfgGetUint32(prAdapter, "Tc1MinRsv",
	    (UINT_32)QM_MIN_RESERVED_TC1_RESOURCE);
	prQM->au4MinReservedTcResource[TC2_INDEX] =
	    (UINT_32) wlanCfgGetUint32(prAdapter, "Tc2MinRsv",
	    (UINT_32)QM_MIN_RESERVED_TC2_RESOURCE);
	prQM->au4MinReservedTcResource[TC3_INDEX] =
	    (UINT_32) wlanCfgGetUint32(prAdapter, "Tc3MinRsv",
	    (UINT_32)QM_MIN_RESERVED_TC3_RESOURCE);
	prQM->au4MinReservedTcResource[TC4_INDEX] =
	    (UINT_32) wlanCfgGetUint32(prAdapter, "Tc4MinRsv",
	    (UINT_32)QM_MIN_RESERVED_TC4_RESOURCE);
	prQM->au4MinReservedTcResource[TC5_INDEX] =
	    (UINT_32) wlanCfgGetUint32(prAdapter, "Tc5MinRsv",
	    (UINT_32)QM_MIN_RESERVED_TC5_RESOURCE);

	prQM->au4GuaranteedTcResource[TC0_INDEX] =
	    (UINT_32) wlanCfgGetUint32(prAdapter, "Tc0Grt",
	    (UINT_32)QM_GUARANTEED_TC0_RESOURCE);
	prQM->au4GuaranteedTcResource[TC1_INDEX] =
	    (UINT_32) wlanCfgGetUint32(prAdapter, "Tc1Grt",
	    (UINT_32)QM_GUARANTEED_TC1_RESOURCE);
	prQM->au4GuaranteedTcResource[TC2_INDEX] =
	    (UINT_32) wlanCfgGetUint32(prAdapter, "Tc2Grt",
	    (UINT_32)QM_GUARANTEED_TC2_RESOURCE);
	prQM->au4GuaranteedTcResource[TC3_INDEX] =
	    (UINT_32) wlanCfgGetUint32(prAdapter, "Tc3Grt",
	    (UINT_32)QM_GUARANTEED_TC3_RESOURCE);
	prQM->au4GuaranteedTcResource[TC4_INDEX] =
	    (UINT_32) wlanCfgGetUint32(prAdapter, "Tc4Grt",
	    (UINT_32)QM_GUARANTEED_TC4_RESOURCE);
	prQM->au4GuaranteedTcResource[TC5_INDEX] =
	    (UINT_32) wlanCfgGetUint32(prAdapter, "Tc5Grt",
	    (UINT_32)QM_GUARANTEED_TC5_RESOURCE);

	prQM->u4TimeToAdjustTcResource =
	    (UINT_32) wlanCfgGetUint32(prAdapter, "TcAdjustTime",
	    (UINT_32)QM_INIT_TIME_TO_ADJUST_TC_RSC);
	prQM->u4TimeToUpdateQueLen =
	    (UINT_32) wlanCfgGetUint32(prAdapter, "QueLenUpdateTime",
	    (UINT_32)QM_INIT_TIME_TO_UPDATE_QUE_LEN);
	prQM->u4QueLenMovingAverage =
	    (UINT_32) wlanCfgGetUint32(prAdapter, "QueLenMovingAvg",
	    (UINT_32)QM_QUE_LEN_MOVING_AVE_FACTOR);
	prQM->u4ExtraReservedTcResource =
	    (UINT_32) wlanCfgGetUint32(prAdapter, "TcExtraRsv",
	    (UINT_32)QM_EXTRA_RESERVED_RESOURCE_WHEN_BUSY);

	/* Stats log */
	prWifiVar->u4StatsLogTimeout = (UINT_32) wlanCfgGetUint32(prAdapter,
		"StatsLogTO", (UINT_32)WLAN_TX_STATS_LOG_TIMEOUT);
	prWifiVar->u4StatsLogDuration =
	    (UINT_32) wlanCfgGetUint32(prAdapter, "StatsLogDur",
	    (UINT_32)WLAN_TX_STATS_LOG_DURATION);

	prWifiVar->ucDhcpTxDone =
		(UINT_8) wlanCfgGetUint32(prAdapter, "DhcpTxDone", 1U);
	prWifiVar->ucArpTxDone =
		(UINT_8) wlanCfgGetUint32(prAdapter, "ArpTxDone", 1U);
	prWifiVar->ucIcmpTxDone =
		(UINT_8) wlanCfgGetUint32(prAdapter, "IcmpTxDone", 1U);
	prWifiVar->ePowerMode =
		(PARAM_POWER_MODE) wlanCfgGetUint32(prAdapter,
		"PowerSave", (UINT32)Param_PowerModeMax);

	prWifiVar->fgSapCheckPmkidInDriver = (uint32_t) wlanCfgGetUint32(
		prAdapter, "SapCheckPmkidInDriver", FEATURE_ENABLED);

}

VOID wlanCfgSetSwCtrl(IN P_ADAPTER_T prAdapter)
{
	UINT_32 i = 0;
	CHAR aucKey[WLAN_CFG_VALUE_LEN_MAX];
	CHAR aucValue[WLAN_CFG_VALUE_LEN_MAX];
	CHAR *pcPtr = NULL;
	CHAR *pcDupValue = NULL;
	UINT_32 au4Values[2];
	UINT_32 u4TokenCount = 0;
	UINT_32 u4BufLen = 0;
	WLAN_STATUS rStatus = WLAN_STATUS_SUCCESS;
	P_GLUE_INFO_T prGlueInfo = prAdapter->prGlueInfo;
	PARAM_CUSTOM_SW_CTRL_STRUCT_T rSwCtrlInfo;
	INT_32 u4Ret = 0;
	const CHAR acDelim[] = " ";

	for (i = (UINT_32)0; i < (UINT_32)WLAN_CFG_SET_SW_CTRL_LEN_MAX; i++) {
		kalMemZero(aucValue, WLAN_CFG_VALUE_LEN_MAX);
		kalMemZero(aucKey, WLAN_CFG_VALUE_LEN_MAX);
		kalSprintf(aucKey, "SwCtrl%d", i);

		/* get nothing */
		if (wlanCfgGet(prAdapter, aucKey, aucValue, "", 0) != WLAN_STATUS_SUCCESS)
			continue;
		if (kalStrCmp(aucValue, "") == 0)
			continue;

		pcDupValue = aucValue;
		u4TokenCount = 0U;

		while ((pcPtr = kalStrSep((char **)(&pcDupValue), acDelim)) != NULL) {

			if (kalStrCmp(pcPtr, "") == 0)
				continue;

			/* au4Values[u4TokenCount] = kalStrtoul(pcPtr, NULL, 0); */
			u4Ret = kalkStrtou32(pcPtr, 0, &(au4Values[u4TokenCount]));
			if (u4Ret == 0)
				DBGLOG(INIT, LOUD, "parse au4Values error u4Ret=%d\n", u4Ret);
			u4TokenCount++;

			/* Only need 2 tokens */
			if (u4TokenCount >= 2U)
				break;
		}

		if (u4TokenCount != 2U)
			continue;

		rSwCtrlInfo.u4Id = au4Values[0];
		rSwCtrlInfo.u4Data = au4Values[1];

		rStatus = kalIoctl(prGlueInfo,
				wlanoidSetSwCtrlWrite,
				&rSwCtrlInfo,
				(UINT_32)sizeof(rSwCtrlInfo),
				FALSE, FALSE, TRUE, &u4BufLen);

	}
}

VOID wlanCfgSetChip(IN P_ADAPTER_T prAdapter)
{
	UINT_32 i = 0;
	CHAR aucKey[WLAN_CFG_VALUE_LEN_MAX];
	CHAR aucValue[WLAN_CFG_VALUE_LEN_MAX];

	UINT_32 u4BufLen = 0;
	WLAN_STATUS rStatus = WLAN_STATUS_SUCCESS;
	P_GLUE_INFO_T prGlueInfo = prAdapter->prGlueInfo;
	PARAM_CUSTOM_CHIP_CONFIG_STRUCT_T rChipConfigInfo;

	for (i = 0U; i < WLAN_CFG_SET_CHIP_LEN_MAX; i++) {
		kalMemZero(aucValue, WLAN_CFG_VALUE_LEN_MAX);
		kalMemZero(aucKey, WLAN_CFG_VALUE_LEN_MAX);
		kalSprintf(aucKey, "SetChip%d", i);

		/* get nothing */
		if (wlanCfgGet(prAdapter, aucKey, aucValue, "", 0) != WLAN_STATUS_SUCCESS)
			continue;
		if (kalStrCmp(aucValue, "") == 0)
			continue;

		kalMemZero(&rChipConfigInfo, (UINT_32)sizeof(rChipConfigInfo));

		rChipConfigInfo.ucType = (UINT_8)CHIP_CONFIG_TYPE_WO_RESPONSE;
		rChipConfigInfo.u2MsgSize =
			(UINT_16)kalStrnLen(aucValue, WLAN_CFG_VALUE_LEN_MAX);
		kalStrnCpy(rChipConfigInfo.aucCmd, aucValue,
			WLAN_CFG_VALUE_LEN_MAX);

		rStatus = kalIoctl(prGlueInfo,
				wlanoidSetChipConfig,
				&rChipConfigInfo,
				(UINT_32)sizeof(rChipConfigInfo),
				FALSE, FALSE, TRUE, &u4BufLen);
	}

}

VOID wlanGetFwInfo(IN P_ADAPTER_T prAdapter)
{
	CMD_GET_FW_INFO_T rCmdGetFwInfo;

	rCmdGetFwInfo.ucValue = 0x1U;
	if (wlanSendSetQueryCmd(prAdapter,
		(UINT_8)CMD_ID_GET_FW_INFO,
		TRUE,
		FALSE,
		FALSE, NULL, NULL,
		(UINT_32)sizeof(CMD_GET_FW_INFO_T),
		(PUINT_8)&rCmdGetFwInfo, NULL, 0) ==
		WLAN_STATUS_SUCCESS)
		DBGLOG(INIT, TRACE, "Get FW INFO\n");
}

VOID wlanCfgSetDebugLevel(IN P_ADAPTER_T prAdapter)
{
	UINT_32 i = 0U;
	CHAR aucKey[WLAN_CFG_VALUE_LEN_MAX];
	CHAR aucValue[WLAN_CFG_VALUE_LEN_MAX];
	const CHAR acDelim[] = " ";
	CHAR *pcDupValue;
	CHAR *pcPtr = NULL;

	UINT_32 au4Values[2];
	UINT_32 u4TokenCount = 0U;
	UINT_32 u4DbgIdx = 0U;
	UINT_32 u4DbgMask = 0U;
	INT_32 u4Ret = 0;

	for (i = 0U; i < (UINT_32)WLAN_CFG_SET_DEBUG_LEVEL_LEN_MAX; i++) {
		kalMemZero(aucValue, WLAN_CFG_VALUE_LEN_MAX);
		kalMemZero(aucKey, WLAN_CFG_VALUE_LEN_MAX);
		kalSprintf(aucKey, "DbgLevel%d", i);

		/* get nothing */
		if (wlanCfgGet(prAdapter, aucKey, aucValue, "", 0) != WLAN_STATUS_SUCCESS)
			continue;
		if (kalStrCmp(aucValue, "") == 0)
			continue;

		pcDupValue = aucValue;
		u4TokenCount = 0U;

		while ((pcPtr = kalStrSep((char **)(&pcDupValue), acDelim)) != NULL) {

			if (kalStrCmp(pcPtr, "") == 0)
				continue;

			/* au4Values[u4TokenCount] = kalStrtoul(pcPtr, NULL, 0); */
			u4Ret = kalkStrtou32(pcPtr, 0, &(au4Values[u4TokenCount]));
			if (u4Ret != 0)
				DBGLOG(INIT, LOUD, "parse au4Values error u4Ret=%d\n", u4Ret);
			u4TokenCount++;

			/* Only need 2 tokens */
			if (u4TokenCount >= 2U)
				break;
		}

		if (u4TokenCount != 2U)
			continue;

		u4DbgIdx = au4Values[0];
		u4DbgMask = au4Values[1];

		/* DBG level special control */
		if (u4DbgIdx == 0xFFFFFFFFU) {
			wlanSetDebugLevel(DBG_ALL_MODULE_IDX, u4DbgMask);
			DBGLOG(INIT, INFO, "Set ALL DBG module log level to [0x%02x]!", (UINT_8) u4DbgMask);
		} else if (u4DbgIdx == 0xFFFFFFFEU) {
			wlanDebugInit();
			DBGLOG(INIT, INFO, "Reset ALL DBG module log level to DEFAULT!");
		} else if (u4DbgIdx < (UINT_32)DBG_MODULE_NUM) {
			wlanSetDebugLevel(u4DbgIdx, u4DbgMask);
			DBGLOG(INIT, INFO,
			       "Set DBG module[%lu] log level to [0x%02x]!", u4DbgIdx, (UINT_8) u4DbgMask);
		}
	}

}

VOID wlanCfgSetCountryCode(IN P_ADAPTER_T prAdapter)
{
	CHAR aucValue[WLAN_CFG_VALUE_LEN_MAX];

	/* Apply COUNTRY Config */
	if (wlanCfgGet(prAdapter, "Country", aucValue, "", 0) == WLAN_STATUS_SUCCESS) {
		prAdapter->rWifiVar.rConnSettings.u2CountryCode =
		    (((UINT_16) aucValue[0]) << 8U) | ((UINT_16) aucValue[1]);

		/* Force to re-search country code in regulatory domains */
		prAdapter->prDomainInfo = NULL;
		rlmDomainSendCmd(prAdapter);

		/* Update supported channel list in channel table based on current country domain */
		wlanUpdateChannelTable(prAdapter->prGlueInfo);
	}
}

#if CFG_SUPPORT_CFG_FILE

P_WLAN_CFG_ENTRY_T wlanCfgGetEntry(IN P_ADAPTER_T prAdapter, const PCHAR pucKey)
{

	P_WLAN_CFG_ENTRY_T prWlanCfgEntry;
	P_WLAN_CFG_T prWlanCfg;
	UINT_32 i;

	prWlanCfg = prAdapter->prWlanCfg;

	ASSERT(prWlanCfg);
	ASSERT(pucKey);

	prWlanCfgEntry = NULL;

	for (i = 0; i < WLAN_CFG_ENTRY_NUM_MAX; i++) {
		prWlanCfgEntry = &prWlanCfg->arWlanCfgBuf[i];
		if (prWlanCfgEntry->aucKey[0] != (UINT_8)'\0') {
			if (kalStrnCmp(pucKey, prWlanCfgEntry->aucKey,
				WLAN_CFG_KEY_LEN_MAX - 1U) == 0) {
				DBGLOG(INIT, TRACE, "wifi config find key \'%s\'\n", pucKey);
				return prWlanCfgEntry;
			}
		}
	}

	DBGLOG(INIT, LOUD, "wifi config there is no entry \'%s\'\n", pucKey);
	return NULL;

}

WLAN_STATUS wlanCfgGet(IN P_ADAPTER_T prAdapter, const PCHAR pucKey, PCHAR pucValue, PCHAR pucValueDef, UINT_32 u4Flags)
{

	P_WLAN_CFG_ENTRY_T prWlanCfgEntry;
	P_WLAN_CFG_T prWlanCfg;

	prWlanCfg = prAdapter->prWlanCfg;

	ASSERT(prWlanCfg);
	ASSERT(pucValue);

	/* Find the exist */
	prWlanCfgEntry = wlanCfgGetEntry(prAdapter, pucKey);

	if (prWlanCfgEntry != NULL) {
		kalStrnCpy(pucValue, prWlanCfgEntry->aucValue,
			WLAN_CFG_VALUE_LEN_MAX - 1U);
	} else {
		if (pucValueDef != NULL)
			kalStrnCpy(pucValue, pucValueDef,
			WLAN_CFG_VALUE_LEN_MAX - 1U);
		return WLAN_STATUS_FAILURE;
	}

	return WLAN_STATUS_SUCCESS;
}

UINT_32 wlanCfgGetUint32(IN P_ADAPTER_T prAdapter, const PCHAR pucKey, UINT_32 u4ValueDef)
{
	P_WLAN_CFG_ENTRY_T prWlanCfgEntry;
	P_WLAN_CFG_T prWlanCfg;
	UINT_32 u4Value;
	INT_32 u4Ret;

	prWlanCfg = prAdapter->prWlanCfg;


	ASSERT(prWlanCfg);

	u4Value = u4ValueDef;
	/* Find the exist */
	prWlanCfgEntry = wlanCfgGetEntry(prAdapter, pucKey);

	if (prWlanCfgEntry != NULL) {
		/* u4Ret = kalStrtoul(prWlanCfgEntry->aucValue, NULL, 0); */
		u4Ret = kalkStrtou32(prWlanCfgEntry->aucValue, 0, &u4Value);
		if (u4Ret != 0)
			DBGLOG(INIT, LOUD, "parse aucValue error u4Ret=%d\n", u4Ret);
	}

	return u4Value;

}

INT_32 wlanCfgGetInt32(IN P_ADAPTER_T prAdapter, const PCHAR pucKey, INT_32 i4ValueDef)
{
	P_WLAN_CFG_ENTRY_T prWlanCfgEntry;
	P_WLAN_CFG_T prWlanCfg;
	INT_32 i4Value = 0;
	INT_32 i4Ret = 0;

	prWlanCfg = prAdapter->prWlanCfg;

	ASSERT(prWlanCfg);

	i4Value = i4ValueDef;
	/* Find the exist */
	prWlanCfgEntry = wlanCfgGetEntry(prAdapter, pucKey);

	if (prWlanCfgEntry != NULL) {
		/* i4Ret = kalStrtol(prWlanCfgEntry->aucValue, NULL, 0); */
		i4Ret = kalkStrtos32(prWlanCfgEntry->aucValue, 0, &i4Value);
		if (i4Ret != 0)
			DBGLOG(INIT, LOUD, "parse aucValue error i4Ret=%d\n", i4Ret);
	}

	return i4Value;

}

WLAN_STATUS wlanCfgSet(IN P_ADAPTER_T prAdapter, const PCHAR pucKey, PCHAR pucValue, UINT_32 u4Flags)
{

	P_WLAN_CFG_ENTRY_T prWlanCfgEntry;
	P_WLAN_CFG_T prWlanCfg;
	UINT_32 u4EntryIndex;
	UINT_32 i;
	UINT_8 ucExist;

	prWlanCfg = prAdapter->prWlanCfg;
	ASSERT(prWlanCfg);
	ASSERT(pucKey);

	/* Find the exist */
	ucExist = 0;
	prWlanCfgEntry = wlanCfgGetEntry(prAdapter, pucKey);

	if (prWlanCfgEntry == NULL) {
		/* Find the empty */
		for (i = 0U; i < WLAN_CFG_ENTRY_NUM_MAX; i++) {
			prWlanCfgEntry = &prWlanCfg->arWlanCfgBuf[i];
			if (prWlanCfgEntry->aucKey[0] == (UINT_8)'\0')
				break;
		}

		u4EntryIndex = i;
		if (u4EntryIndex < WLAN_CFG_ENTRY_NUM_MAX) {
			prWlanCfgEntry = &prWlanCfg->arWlanCfgBuf[u4EntryIndex];
			kalMemZero(prWlanCfgEntry,
				(UINT_32)sizeof(WLAN_CFG_ENTRY_T));
		} else {
			prWlanCfgEntry = NULL;
			DBGLOG(INIT, ERROR, "wifi config there is no empty entry\n");
		}
	} /* !prWlanCfgEntry */
	else
		ucExist = 1U;

	if (prWlanCfgEntry != NULL) {
		if (ucExist == 0U) {
			kalStrnCpy(prWlanCfgEntry->aucKey,
				pucKey, WLAN_CFG_KEY_LEN_MAX - 1U);
			prWlanCfgEntry->aucKey[WLAN_CFG_KEY_LEN_MAX - 1U]
				= (UINT_8)'\0';
		}

		if (pucValue != NULL && pucValue[0] != (CHAR)'\0') {
			kalStrnCpy(prWlanCfgEntry->aucValue,
				pucValue, WLAN_CFG_VALUE_LEN_MAX - 1U);
			prWlanCfgEntry->aucValue[WLAN_CFG_VALUE_LEN_MAX - 1U]
				= (UINT_8)'\0';

			if (ucExist != 0U) {
				if (prWlanCfgEntry->pfSetCb != NULL)
					prWlanCfgEntry->pfSetCb(prAdapter,
						prWlanCfgEntry->aucKey,
						prWlanCfgEntry->aucValue,
						prWlanCfgEntry->pPrivate,
						0U);
			}
		} else {
			/* Call the pfSetCb if value is empty ? */
			/* remove the entry if value is empty */
			kalMemZero(prWlanCfgEntry,
				(UINT_32)sizeof(WLAN_CFG_ENTRY_T));
		}

	}
	/* prWlanCfgEntry */
	if (prWlanCfgEntry != NULL) {
		DBGLOG(INIT, INFO, "Set wifi config exist %u \'%s\' \'%s\'\n",
				    ucExist, prWlanCfgEntry->aucKey, prWlanCfgEntry->aucValue);
	} else {
		if (pucKey != NULL) {
			DBGLOG(INIT, ERROR,
				"Set wifi config error key \'%s\'\n", pucKey);
		}
		if (pucValue != NULL) {
			DBGLOG(INIT, ERROR,
				"Set wifi config error value \'%s\'\n", pucValue);
		}
		return WLAN_STATUS_FAILURE;
	}

	return WLAN_STATUS_SUCCESS;
}

WLAN_STATUS
wlanCfgSetCb(IN P_ADAPTER_T prAdapter, const PCHAR pucKey, WLAN_CFG_SET_CB pfSetCb, void *pPrivate, UINT_32 u4Flags)
{

	P_WLAN_CFG_ENTRY_T prWlanCfgEntry;
	P_WLAN_CFG_T prWlanCfg;

	prWlanCfg = prAdapter->prWlanCfg;
	ASSERT(prWlanCfg);

	/* Find the exist */
	prWlanCfgEntry = wlanCfgGetEntry(prAdapter, pucKey);

	if (prWlanCfgEntry != NULL) {
		prWlanCfgEntry->pfSetCb = pfSetCb;
		prWlanCfgEntry->pPrivate = pPrivate;
	}

	if (prWlanCfgEntry != NULL)
		return WLAN_STATUS_SUCCESS;
	else
		return WLAN_STATUS_FAILURE;

}

WLAN_STATUS wlanCfgSetUint32(IN P_ADAPTER_T prAdapter, const PCHAR pucKey, UINT_32 u4Value)
{

	P_WLAN_CFG_T prWlanCfg;
	UINT_8 aucBuf[WLAN_CFG_VALUE_LEN_MAX];

	prWlanCfg = prAdapter->prWlanCfg;

	ASSERT(prWlanCfg);

	kalMemZero(aucBuf, (UINT_32)sizeof(aucBuf));

	kalSnprintf(aucBuf, WLAN_CFG_VALUE_LEN_MAX, "0x%x", (unsigned int)u4Value);

	return wlanCfgSet(prAdapter, pucKey, aucBuf, 0);
}

enum {
	STATE_EOF = 0,
	STATE_TEXT = 1,
	STATE_NEWLINE = 2
};

struct WLAN_CFG_PARSE_STATE_S {
	CHAR *ptr;
	CHAR *text;
	INT_32 nexttoken;
	UINT_32 maxSize;
};

INT_32 wlanCfgFindNextToken(struct WLAN_CFG_PARSE_STATE_S *state)
{
	CHAR *x = state->ptr;
	CHAR *s;

	if (state->nexttoken) {
		INT_32 t = state->nexttoken;

		state->nexttoken = 0;
		return t;
	}

	for (;;) {
		switch (*x) {
		case (CHAR)0:
			state->ptr = x;
			return STATE_EOF;
		case (CHAR)'\n':
			x++;
			state->ptr = x;
			return STATE_NEWLINE;
		case (CHAR)' ':
		case (CHAR)'\t':
		case (CHAR)'\r':
			x++;
			continue;
		case (CHAR)'#':
			while (*x != 0 && (*x != (CHAR)'\n'))
				x++;
			if (*x == (CHAR)'\n') {
				state->ptr = x + 1;
			} else {
				state->ptr = x;
				return STATE_EOF;
			}
			return STATE_NEWLINE;
		default:
			goto text;
		}
	}

textdone:
	state->ptr = x;
	*s = 0;
	return STATE_TEXT;
text:
	state->text = s = x;
textresume:
	for (;;) {
		switch (*x) {
		case (CHAR)0:
			goto textdone;
		case (CHAR)' ':
		case (CHAR)'\t':
		case (CHAR)'\r':
			x++;
			goto textdone;
		case (CHAR)'\n':
			state->nexttoken = STATE_NEWLINE;
			x++;
			goto textdone;
		case (CHAR)'"':
			x++;
			for (;;) {
				switch (*x) {
				case (CHAR)0:
					/* unterminated quoted thing */
					state->ptr = x;
					return STATE_EOF;
				case (CHAR)'"':
					x++;
					goto textresume;
				default:
					*s++ = *x++;
				}
			}

		case (CHAR)'\\':
			x++;
			switch (*x) {
			case (CHAR)0:
				goto textdone;
			case (CHAR)'n':
				*s++ = (CHAR)'\n';
				break;
			case (CHAR)'r':
				*s++ = (CHAR)'\r';
				break;
			case (CHAR)'t':
				*s++ = (CHAR)'\t';
				break;
			case (CHAR)'\\':
				*s++ = (CHAR)'\\';
				break;
			case (CHAR)'\r':
				/* \ <cr> <lf> -> line continuation */
				if (x[1] != (CHAR)'\n') {
					x++;
					continue;
				}
			case (CHAR)'\n':
				/* \ <lf> -> line continuation */
				x++;
				/* eat any extra whitespace */
				while ((*x == (CHAR)' ') || (*x == (CHAR)'\t'))
					x++;
				continue;
			default:
				/* unknown escape -- just copy */
				*s++ = *x++;
			}
			continue;
		default:
			*s++ = *x++;
		}
	}
}

WLAN_STATUS wlanCfgParseArgument(CHAR *cmdLine, INT_32 *argc, CHAR *argv[])
{
	struct WLAN_CFG_PARSE_STATE_S state;
	CHAR **args;
	INT_32 nargs;

	if (cmdLine == NULL || argc == NULL || argv == NULL) {
		ASSERT(NULL);
		return WLAN_STATUS_FAILURE;
	}
	args = argv;
	nargs = 0;
	state.ptr = cmdLine;
	state.nexttoken = 0;
	state.maxSize = 0;
	state.text = NULL;

	if (kalStrnLen(cmdLine, 512) >= 512U) {
		ASSERT(NULL);
		return WLAN_STATUS_FAILURE;
	}

	for (;;) {
		switch (wlanCfgFindNextToken(&state)) {
		case STATE_EOF:
			goto exit;
		case STATE_NEWLINE:
			goto exit;
		case STATE_TEXT:
			if (nargs < WLAN_CFG_ARGV_MAX)
				args[nargs++] = state.text;
			break;
		default:
			break;
		}
	}

exit:
	*argc = nargs;
	return WLAN_STATUS_SUCCESS;
}

WLAN_STATUS
wlanCfgParseAddEntry(IN P_ADAPTER_T prAdapter,
		     PUINT_8 pucKeyHead, PUINT_8 pucKeyTail, PUINT_8 pucValueHead, PUINT_8 pucValueTail)
{

	UINT_8 aucKey[WLAN_CFG_KEY_LEN_MAX];
	UINT_8 aucValue[WLAN_CFG_VALUE_LEN_MAX];
	UINT_32 u4Len;

	kalMemZero(aucKey, (UINT_32)sizeof(aucKey));
	kalMemZero(aucValue, (UINT_32)sizeof(aucValue));

	if ((pucKeyHead == NULL)
	    || (pucValueHead == NULL)
	    )
		return WLAN_STATUS_FAILURE;

	if (pucKeyTail != NULL) {
		if (pucKeyHead > pucKeyTail)
			return WLAN_STATUS_FAILURE;
		u4Len = (UINT_32)(pucKeyTail - pucKeyHead) + 1U;
	} else
		u4Len = (UINT_32)kalStrnLen(pucKeyHead,
			WLAN_CFG_KEY_LEN_MAX - 1U);

	if (u4Len >= WLAN_CFG_KEY_LEN_MAX)
		u4Len = WLAN_CFG_KEY_LEN_MAX - 1U;

	kalStrnCpy(aucKey, pucKeyHead, u4Len);

	if (pucValueTail != NULL) {
		if (pucValueHead > pucValueTail)
			return WLAN_STATUS_FAILURE;
		u4Len = (UINT_32)(pucValueTail - pucValueHead) + 1U;
	} else
		u4Len = (UINT_32)kalStrnLen(pucValueHead,
			WLAN_CFG_VALUE_LEN_MAX - 1U);

	if (u4Len >= WLAN_CFG_VALUE_LEN_MAX)
		u4Len = WLAN_CFG_VALUE_LEN_MAX - 1U;

	kalStrnCpy(aucValue, pucValueHead, u4Len);

	return wlanCfgSet(prAdapter, aucKey, aucValue, 0);
}

enum {
	WAIT_KEY_HEAD = 0,
	WAIT_KEY_TAIL,
	WAIT_VALUE_HEAD,
	WAIT_VALUE_TAIL,
	WAIT_COMMENT_TAIL
};

WLAN_STATUS wlanCfgParse(IN P_ADAPTER_T prAdapter, PUINT_8 pucConfigBuf, UINT_32 u4ConfigBufLen)
{

	struct WLAN_CFG_PARSE_STATE_S state;
	PCHAR apcArgv[WLAN_CFG_ARGV_MAX];
	CHAR **args;
	INT_32 nargs;

	if (pucConfigBuf == NULL) {
		ASSERT(NULL);
		return WLAN_STATUS_FAILURE;
	}
	if (kalStrnLen(pucConfigBuf, 4000U) >= 4000U) {
		ASSERT(NULL);
		return WLAN_STATUS_FAILURE;
	}
	if (u4ConfigBufLen == 0U)
		return WLAN_STATUS_FAILURE;
	args = apcArgv;
	nargs = 0;
	state.ptr = pucConfigBuf;
	state.nexttoken = 0;
	state.maxSize = u4ConfigBufLen;
	state.text = NULL;

	for (;;) {
		switch (wlanCfgFindNextToken(&state)) {
		case STATE_EOF:
			if (nargs > 1)
				if (wlanCfgParseAddEntry(prAdapter, args[0],
					NULL, args[1], NULL) ==
					WLAN_STATUS_SUCCESS)
					DBGLOG(INIT, TRACE, "parse entry\n");
			goto exit;
		case STATE_NEWLINE:
			if (nargs > 1)
				if (wlanCfgParseAddEntry(prAdapter, args[0],
					NULL, args[1], NULL) ==
					WLAN_STATUS_SUCCESS)
					DBGLOG(INIT, TRACE, "parse entry\n");
			nargs = 0;
			break;
		case STATE_TEXT:
			if (nargs < WLAN_CFG_ARGV_MAX)
				args[nargs++] = state.text;
			break;
		default:
			break;
		}
	}

exit:
	return WLAN_STATUS_SUCCESS;
}

WLAN_STATUS wlanCfgInit(IN P_ADAPTER_T prAdapter, PUINT_8 pucConfigBuf, UINT_32 u4ConfigBufLen, UINT_32 u4Flags)
{
	P_WLAN_CFG_T prWlanCfg;
	/* P_WLAN_CFG_ENTRY_T prWlanCfgEntry; */
	prAdapter->prWlanCfg = &prAdapter->rWlanCfg;
	prWlanCfg = prAdapter->prWlanCfg;

	kalMemZero(prWlanCfg, (UINT_32)sizeof(WLAN_CFG_T));
	ASSERT(prWlanCfg);
	prWlanCfg->u4WlanCfgEntryNumMax = WLAN_CFG_ENTRY_NUM_MAX;
	prWlanCfg->u4WlanCfgKeyLenMax = WLAN_CFG_KEY_LEN_MAX;
	prWlanCfg->u4WlanCfgValueLenMax = WLAN_CFG_VALUE_LEN_MAX;

	DBGLOG(INIT, LOUD, "Init wifi config len %u max entry %u\n", u4ConfigBufLen, prWlanCfg->u4WlanCfgEntryNumMax);
#if DBG
	/* self test */
	wlanCfgSet(prAdapter, "ConfigValid", "0x123", 0);
	if (wlanCfgGetUint32(prAdapter, "ConfigValid", 0) != 0x123U) {
		DBGLOG(INIT, ERROR,
			"wifi config error %u\n", __LINE__);
	}
	wlanCfgSet(prAdapter, "ConfigValid", "1", 0);
	if (wlanCfgGetUint32(prAdapter, "ConfigValid", 0) != 1U) {
		DBGLOG(INIT, ERROR,
			"wifi config error %u\n", __LINE__);
	}
#endif

	/* Parse the pucConfigBuff */

	if (pucConfigBuf != NULL && (u4ConfigBufLen > 0U))
		if (wlanCfgParse(prAdapter, pucConfigBuf,
			u4ConfigBufLen) == WLAN_STATUS_SUCCESS)
		DBGLOG(INIT, LOUD, "cfg parse\n");

	return WLAN_STATUS_SUCCESS;
}

#endif /* CFG_SUPPORT_CFG_FILE */

INT_32 wlanHexToNum(CHAR c)
{
	if (c >= (CHAR)'0' && c <= (CHAR)'9')
		return (INT_32)c - (INT_32)'0';
	if (c >= (CHAR)'a' && c <= (CHAR)'f')
		return (INT_32)c - (INT_32)'a' + 10;
	if (c >= (CHAR)'A' && c <= (CHAR)'F')
		return (INT_32)c - (INT_32)'A' + 10;
	return -1;
}

INT_32 wlanHexToByte(PCHAR hex)
{
	INT_32 a, b;

	a = wlanHexToNum(*hex++);
	if (a < 0)
		return -1;
	b = wlanHexToNum(*hex++);
	if (b < 0)
		return -1;
	return (INT_32)(((UINT_32)a << 4U) | (UINT_32)b);
}

INT_32 wlanHwAddrToBin(PCHAR txt, UINT_8 *addr)
{
	INT_32 i;
	PCHAR pos = txt;

	for (i = 0; i < 6; i++) {
		INT_32 a, b;

		while (*pos == (CHAR)':' || *pos == (CHAR)'.' ||
			*pos == (CHAR)'-')
			pos++;

		a = wlanHexToNum(*pos++);
		if (a < 0)
			return -1;
		b = wlanHexToNum(*pos++);
		if (b < 0)
			return -1;
		*addr++ = (UINT_8)((UINT_32)a << 4U) | (UINT_8)b;
	}

	return (INT_32)(pos - txt);
}

BOOLEAN wlanIsChipNoAck(IN P_ADAPTER_T prAdapter)
{
	BOOLEAN fgIsNoAck;

	fgIsNoAck = (BOOLEAN)((prAdapter->fgIsChipNoAck == TRUE) ||
		(kalIsResetting() == TRUE) ||
		(fgIsBusAccessFailed == TRUE));

	return fgIsNoAck;
}

#if CFG_ENABLE_PER_STA_STATISTICS
VOID wlanTxLifetimeUpdateStaStats(IN P_ADAPTER_T prAdapter, IN P_MSDU_INFO_T prMsduInfo)
{
	P_STA_RECORD_T prStaRec;
	UINT_32 u4DeltaTime;
	UINT_32 u4DeltaHifTxTime;
	P_QUE_MGT_T prQM = &prAdapter->rQM;
	P_PKT_PROFILE_T prPktProfile = &prMsduInfo->rPktProfile;
	UINT_32 u4PktPrintPeriod = 0U;

	prStaRec = cnmGetStaRecByIndex(prAdapter, prMsduInfo->ucStaRecIndex);

	if (prStaRec != NULL) {
		u4DeltaTime = (UINT_32) (prPktProfile->rHifTxDoneTimestamp - prPktProfile->rHardXmitArrivalTimestamp);
		u4DeltaHifTxTime = (UINT_32) (prPktProfile->rHifTxDoneTimestamp - prPktProfile->rDequeueTimestamp);


		/* Update StaRec statistics */
		prStaRec->u4TotalTxPktsNumber++;
		prStaRec->u4TotalTxPktsTime += u4DeltaTime;
		prStaRec->u4TotalTxPktsHifTxTime += u4DeltaHifTxTime;

		if (u4DeltaTime > prStaRec->u4MaxTxPktsTime)
			prStaRec->u4MaxTxPktsTime = u4DeltaTime;

		if (u4DeltaHifTxTime > prStaRec->u4MaxTxPktsHifTime)
			prStaRec->u4MaxTxPktsHifTime = u4DeltaHifTxTime;


		if (u4DeltaTime >= NIC_TX_TIME_THRESHOLD)
			prStaRec->u4ThresholdCounter++;

		if (u4PktPrintPeriod != 0U &&
			(prStaRec->u4TotalTxPktsNumber >= u4PktPrintPeriod)) {
			DBGLOG(TX, TRACE, "[%u]N[%4lu]A[%5lu]M[%4lu]T[%4lu]E[%4lu]\n",
					   prStaRec->ucIndex,
					   prStaRec->u4TotalTxPktsNumber,
					   (prStaRec->u4TotalTxPktsTime / prStaRec->u4TotalTxPktsNumber),
					   prStaRec->u4MaxTxPktsTime,
					   prStaRec->u4ThresholdCounter,
					   prQM->au4QmTcResourceEmptyCounter[prStaRec->ucBssIndex][TC2_INDEX]);

			prStaRec->u4TotalTxPktsNumber = 0U;
			prStaRec->u4TotalTxPktsTime = 0U;
			prStaRec->u4MaxTxPktsTime = 0U;
			prStaRec->u4ThresholdCounter = 0U;
			prQM->au4QmTcResourceEmptyCounter
				[prStaRec->ucBssIndex][TC2_INDEX] = 0U;
		}
	}
}
#endif

BOOLEAN wlanTxLifetimeIsProfilingEnabled(IN P_ADAPTER_T prAdapter)
{
	BOOLEAN fgEnabled = FALSE;
#if CFG_SUPPORT_WFD
	P_WFD_CFG_SETTINGS_T prWfdCfgSettings = (P_WFD_CFG_SETTINGS_T) NULL;

	prWfdCfgSettings = &prAdapter->rWifiVar.rWfdConfigureSettings;

	if (prWfdCfgSettings->ucWfdEnable > 0U)
		fgEnabled = TRUE;
#endif

	return fgEnabled;
}

BOOLEAN wlanTxLifetimeIsTargetMsdu(IN P_ADAPTER_T prAdapter, IN P_MSDU_INFO_T prMsduInfo)
{
	BOOLEAN fgResult = TRUE;

#if 0
	switch (prMsduInfo->ucTID) {
		/* BK */
	case 1:
	case 2:

		/* BE */
	case 0:
	case 3:
		fgResult = FALSE;
		break;
		/* VI */
	case 4:
	case 5:

		/* VO */
	case 6:
	case 7:
		fgResult = TRUE;
		break;
	default:
		break;
	}
#endif
	return fgResult;
}

VOID wlanTxLifetimeTagPacket(IN P_ADAPTER_T prAdapter, IN P_MSDU_INFO_T prMsduInfo, IN ENUM_TX_PROFILING_TAG_T eTag)
{
	P_PKT_PROFILE_T prPktProfile = &prMsduInfo->rPktProfile;

	if (wlanTxLifetimeIsProfilingEnabled(prAdapter) == FALSE)
		return;

	switch (eTag) {
	case TX_PROF_TAG_OS_TO_DRV:
		/* arrival time is tagged in wlanProcessTxFrame */
		break;

	case TX_PROF_TAG_DRV_ENQUE:
		/* Reset packet profile */
		prPktProfile->fgIsValid = FALSE;
		if (wlanTxLifetimeIsTargetMsdu(prAdapter, prMsduInfo) == TRUE) {
			/* Enable packet lifetime profiling */
			prPktProfile->fgIsValid = TRUE;

			/* Packet arrival time at kernel Hard Xmit */
			prPktProfile->rHardXmitArrivalTimestamp = GLUE_GET_PKT_ARRIVAL_TIME(prMsduInfo->prPacket);

			/* Packet enqueue time */
			prPktProfile->rEnqueueTimestamp = (OS_SYSTIME) kalGetTimeTick();
		}
		break;

	case TX_PROF_TAG_DRV_DEQUE:
		if (prPktProfile->fgIsValid == TRUE)
			prPktProfile->rDequeueTimestamp = (OS_SYSTIME) kalGetTimeTick();
		break;

	case TX_PROF_TAG_DRV_TX_DONE:
		if (prPktProfile->fgIsValid == TRUE) {

			prPktProfile->rHifTxDoneTimestamp = (OS_SYSTIME) kalGetTimeTick();

#if CFG_ENABLE_PER_STA_STATISTICS
			wlanTxLifetimeUpdateStaStats(prAdapter, prMsduInfo);
#endif
		}
		break;

	case TX_PROF_TAG_MAC_TX_DONE:
		break;

	default:
		break;
	}
}

VOID wlanTxLifetimeTagPacketQue(IN P_ADAPTER_T prAdapter, IN P_MSDU_INFO_T prMsduInfoListHead,
		IN ENUM_TX_PROFILING_TAG_T eTag)
{
	P_MSDU_INFO_T prMsduInfo = prMsduInfoListHead, prNextMsduInfo;
	P_PKT_PROFILE_T prPktProfile = NULL;

	if (wlanTxLifetimeIsProfilingEnabled(prAdapter) == FALSE)
		return;

	while (prMsduInfo != NULL) {
		prPktProfile = &prMsduInfo->rPktProfile;
		prNextMsduInfo = (P_MSDU_INFO_T) QUEUE_GET_NEXT_ENTRY((P_QUE_ENTRY_T) prMsduInfo);

		switch (eTag) {
		case TX_PROF_TAG_OS_TO_DRV:
			/* arrival time is tagged in wlanProcessTxFrame */
			break;

		case TX_PROF_TAG_DRV_ENQUE:
			/* Reset packet profile */
			prPktProfile->fgIsValid = FALSE;
			if (wlanTxLifetimeIsTargetMsdu(prAdapter,
				prMsduInfo) == TRUE) {
				/* Enable packet lifetime profiling */
				prPktProfile->fgIsValid = TRUE;

				/* Packet arrival time at kernel Hard Xmit */
				prPktProfile->rHardXmitArrivalTimestamp =
					GLUE_GET_PKT_ARRIVAL_TIME(prMsduInfo->prPacket);

				/* Packet enqueue time */
				prPktProfile->rEnqueueTimestamp = (OS_SYSTIME) kalGetTimeTick();
			}
			break;

		case TX_PROF_TAG_DRV_DEQUE:
			if (prPktProfile->fgIsValid == TRUE)
				prPktProfile->rDequeueTimestamp = (OS_SYSTIME) kalGetTimeTick();
			break;

		case TX_PROF_TAG_DRV_TX_DONE:
			if (prPktProfile->fgIsValid == TRUE) {
				prPktProfile->rHifTxDoneTimestamp = (OS_SYSTIME) kalGetTimeTick();

#if CFG_ENABLE_PER_STA_STATISTICS
				wlanTxLifetimeUpdateStaStats(prAdapter, prMsduInfo);
#endif
			}
			break;

		case TX_PROF_TAG_MAC_TX_DONE:
			break;

		default:
			break;
		}

		prMsduInfo = prNextMsduInfo;
	};
}

VOID wlanTxProfilingTagPacket(IN P_ADAPTER_T prAdapter, IN P_NATIVE_PACKET prPacket, IN ENUM_TX_PROFILING_TAG_T eTag)
{
#if CFG_MET_PACKET_TRACE_SUPPORT
	kalMetTagPacket(prAdapter->prGlueInfo, prPacket, eTag);
#endif
}

VOID wlanTxProfilingTagMsdu(IN P_ADAPTER_T prAdapter, IN P_MSDU_INFO_T prMsduInfo, IN ENUM_TX_PROFILING_TAG_T eTag)
{
	wlanTxLifetimeTagPacket(prAdapter, prMsduInfo, eTag);

	wlanTxProfilingTagPacket(prAdapter, prMsduInfo->prPacket, eTag);
}

VOID wlanUpdateTxStatistics(IN P_ADAPTER_T prAdapter, IN P_MSDU_INFO_T prMsduInfo, IN BOOLEAN fgTxDrop)
{
	P_STA_RECORD_T prStaRec;
	P_BSS_INFO_T prBssInfo;
	ENUM_WMM_ACI_T eAci = WMM_AC_BE_INDEX;
	P_QUE_MGT_T prQM = &prAdapter->rQM;
	OS_SYSTIME rCurTime;

	eAci = aucTid2ACI[prMsduInfo->ucUserPriority];

	prStaRec = cnmGetStaRecByIndex(prAdapter, prMsduInfo->ucStaRecIndex);

	if (prStaRec != NULL) {
		if (fgTxDrop == TRUE)
			prStaRec->arLinkStatistics[eAci].u4TxDropMsdu++;
		else
			prStaRec->arLinkStatistics[eAci].u4TxMsdu++;
	} else {
		prBssInfo = GET_BSS_INFO_BY_INDEX(prAdapter, prMsduInfo->ucBssIndex);

		if (fgTxDrop == TRUE)
			prBssInfo->arLinkStatistics[eAci].u4TxDropMsdu++;
		else
			prBssInfo->arLinkStatistics[eAci].u4TxMsdu++;
	}

	/* Trigger FW stats log every 20s */
	rCurTime = (OS_SYSTIME) kalGetTimeTick();

	DBGLOG(INIT, TRACE, "CUR[%u] LAST[%u] TO[%u]\n", rCurTime,
		prQM->rLastTxPktDumpTime, CHECK_FOR_TIMEOUT(rCurTime,
						prQM->rLastTxPktDumpTime,
						MSEC_TO_SYSTIME(prAdapter->
						rWifiVar.u4StatsLogTimeout)));

	if (CHECK_FOR_TIMEOUT(rCurTime, prQM->rLastTxPktDumpTime,
			      MSEC_TO_SYSTIME(prAdapter->rWifiVar.u4StatsLogTimeout))) {

		if (wlanTriggerStatsLog(prAdapter,
			prAdapter->rWifiVar.u4StatsLogDuration) == TRUE)
			DBGLOG(TX, TRACE, "wlanTriggerStatsLog TRUE\n");
		wlanDumpAllBssStatistics(prAdapter);

		prQM->rLastTxPktDumpTime = rCurTime;
	}
}

VOID wlanUpdateRxStatistics(IN P_ADAPTER_T prAdapter, IN P_SW_RFB_T prSwRfb)
{
	P_STA_RECORD_T prStaRec;
	ENUM_WMM_ACI_T eAci = WMM_AC_BE_INDEX;

	eAci = aucTid2ACI[prSwRfb->ucTid];

	prStaRec = cnmGetStaRecByIndex(prAdapter, prSwRfb->ucStaRecIdx);
	if (prStaRec != NULL)
		prStaRec->arLinkStatistics[eAci].u4RxMsdu++;
}

WLAN_STATUS wlanTriggerStatsLog(IN P_ADAPTER_T prAdapter, IN UINT_32 u4DurationInMs)
{
	CMD_STATS_LOG_T rStatsLogCmd;
	WLAN_STATUS rResult;

	kalMemZero(&rStatsLogCmd, (UINT_32)sizeof(CMD_STATS_LOG_T));

	rStatsLogCmd.u4DurationInMs = u4DurationInMs;

	rResult = wlanSendSetQueryCmd(prAdapter,
			(UINT_8)CMD_ID_STATS_LOG,
			TRUE,
			FALSE,
			FALSE,
			nicCmdEventSetCommon,
			nicOidCmdTimeoutCommon,
			(UINT_32)sizeof(CMD_STATS_LOG_T),
			(PUINT_8) &rStatsLogCmd, NULL, 0);

	return rResult;
}

WLAN_STATUS
wlanDhcpTxDone(IN P_ADAPTER_T prAdapter, IN P_MSDU_INFO_T prMsduInfo, IN ENUM_TX_RESULT_CODE_T rTxDoneStatus)
{
	DBGLOG(TX, INFO, "DHCP PKT TX DONE WIDX:PID[%u:%u] Status[%u], SeqNo: %d\n",
			prMsduInfo->ucWlanIndex, prMsduInfo->ucPID, rTxDoneStatus, prMsduInfo->ucTxSeqNum);

	return WLAN_STATUS_SUCCESS;
}

WLAN_STATUS
wlanArpTxDone(IN P_ADAPTER_T prAdapter, IN P_MSDU_INFO_T prMsduInfo, IN ENUM_TX_RESULT_CODE_T rTxDoneStatus)
{
	DBGLOG(TX, INFO, "ARP PKT TX DONE WIDX:PID[%u:%u] Status[%u], SeqNo: %d\n",
			prMsduInfo->ucWlanIndex, prMsduInfo->ucPID, rTxDoneStatus, prMsduInfo->ucTxSeqNum);

	return WLAN_STATUS_SUCCESS;
}

WLAN_STATUS
wlanIcmpTxDone(IN P_ADAPTER_T prAdapter, IN P_MSDU_INFO_T prMsduInfo, IN ENUM_TX_RESULT_CODE_T rTxDoneStatus)
{
	DBGLOG(TX, INFO, "ICMP PKT TX DONE WIDX:PID[%u:%u] Status[%u], SeqNo: %d\n",
			prMsduInfo->ucWlanIndex, prMsduInfo->ucPID, rTxDoneStatus, prMsduInfo->ucTxSeqNum);

	return WLAN_STATUS_SUCCESS;
}

WLAN_STATUS
wlanTdlsTxDone(IN P_ADAPTER_T prAdapter, IN P_MSDU_INFO_T prMsduInfo, IN ENUM_TX_RESULT_CODE_T rTxDoneStatus)
{
	DBGLOG(TX, INFO, "TDLS PKT TX DONE WIDX:PID[%u:%u] Status[%u], SeqNo: %d\n",
			prMsduInfo->ucWlanIndex, prMsduInfo->ucPID, rTxDoneStatus, prMsduInfo->ucTxSeqNum);

	return WLAN_STATUS_SUCCESS;
}

WLAN_STATUS
wlanDnsTxDone(IN P_ADAPTER_T prAdapter, IN P_MSDU_INFO_T prMsduInfo,
		IN ENUM_TX_RESULT_CODE_T rTxDoneStatus)
{
	DBGLOG(SW4, INFO, "DNS PKT TX DONE WIDX:PID[%u:%u] Status[%u], SeqNo: %d\n",
			prMsduInfo->ucWlanIndex, prMsduInfo->ucPID, rTxDoneStatus, prMsduInfo->ucTxSeqNum);

	return WLAN_STATUS_SUCCESS;
}

VOID wlanReleasePendingCmdById(P_ADAPTER_T prAdapter, UINT_8 ucCid)
{
	P_QUE_T prCmdQue;
	QUE_T rTempCmdQue;
	P_QUE_T prTempCmdQue = &rTempCmdQue;
	P_QUE_ENTRY_T prQueueEntry = (P_QUE_ENTRY_T) NULL;
	P_CMD_INFO_T prCmdInfo = (P_CMD_INFO_T) NULL;

	KAL_SPIN_LOCK_DECLARATION();

	ASSERT(prAdapter);
	DBGLOG(OID, INFO, "Remove pending Cmd: CID %d\n", ucCid);

	/* 1: Clear Pending OID in prAdapter->rPendingCmdQueue */
	KAL_ACQUIRE_SPIN_LOCK(prAdapter, SPIN_LOCK_CMD_PENDING);

	prCmdQue = &prAdapter->rPendingCmdQueue;
	QUEUE_MOVE_ALL(prTempCmdQue, prCmdQue);

	QUEUE_REMOVE_HEAD(prTempCmdQue, prQueueEntry, P_QUE_ENTRY_T);
	while (prQueueEntry != NULL) {
		prCmdInfo = (P_CMD_INFO_T) prQueueEntry;
		if (prCmdInfo->ucCID != ucCid) {
			QUEUE_INSERT_TAIL(prCmdQue, prQueueEntry);
			continue;
		}

		if (prCmdInfo->pfCmdTimeoutHandler != NULL) {
			prCmdInfo->pfCmdTimeoutHandler(prAdapter, prCmdInfo);
		} else if (prCmdInfo->fgIsOid == TRUE) {
			kalOidComplete(prAdapter->prGlueInfo,
					   prCmdInfo->fgSetQuery, 0, WLAN_STATUS_FAILURE);
		}

		cmdBufFreeCmdInfo(prAdapter, prCmdInfo);
		QUEUE_REMOVE_HEAD(prTempCmdQue, prQueueEntry, P_QUE_ENTRY_T);
	}

	KAL_RELEASE_SPIN_LOCK(prAdapter, SPIN_LOCK_CMD_PENDING);
}

UINT_8
wlanGetSupportNss(IN P_ADAPTER_T prAdapter, IN UINT_8 ucBssIndex)
{
	UINT_8 ucRetValNss = prAdapter->rWifiVar.ucNSS;

	return ucRetValNss;
}

