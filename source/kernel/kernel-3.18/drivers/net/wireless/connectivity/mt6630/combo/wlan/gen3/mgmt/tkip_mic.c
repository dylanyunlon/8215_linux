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
 * Id: /Department/DaVinci/TRUNK/MT6620_WiFi_Firmware/mcu/wifi/mgmt/tkip_mic.c#7
 */

/*
 * ! \file tkip_sw.c
 * \brief This file include the tkip encrypted / decrypted mic function.
 */

/*******************************************************************************
*                     C O M P I L E R   F L A G S
********************************************************************************
*/

/*******************************************************************************
*                E X T E R N A L   R E F E R E N C E S
********************************************************************************
*/
#include "precomp.h"

/*******************************************************************************
*                          C O N S T A N T S
********************************************************************************
*/

/*******************************************************************************
*                         D A T A   T Y P E S
********************************************************************************
*/

/*******************************************************************************
*                        P U B L I C   D A T A
********************************************************************************
*/

/*******************************************************************************
*                       P R I V A T E   D A T A
********************************************************************************
*/

/*******************************************************************************
*                             M A C R O S
********************************************************************************
*/

#define WLAN_MAC_MIC_LEN              8U

#define MK16_TKIP(x, y)       (((UINT_16) (x) << 8) | (UINT_16) (y))

#define LO_8BITS(x)     ((x) & 0x00ff)	/* obtain low 8-bit from 16-bit value, OK */
#define HI_8BITS(x)     ((x) >> 8)	/* obtain high 8-bit from 16-bit value, OK */

#define ROTR32(x, y)     (((x) >> (y)) | ((x) << (32U - (y))))
#define ROTL32(x, y)     (((x) << (y)) | ((x) >> (32U - (y))))
#define ROTR16(x, y)     (((x) >> (y)) | ((x) << (16U - (y))))
#define ROTL16(x, y)     (((x) << (y)) | ((x) >> (16U - (y))))

#define XSWAP32(x)      ((((x) & 0xFF00FF00U) >> 8U) |      \
						(((x) & 0x00FF00FFU) << 8U))

/*******************************************************************************
*                         D A T A   T Y P E S
********************************************************************************
*/

/*******************************************************************************
*                        P U B L I C   D A T A
********************************************************************************
*/

/*******************************************************************************
*                       P R I V A T E   D A T A
********************************************************************************
*/

/*******************************************************************************
*              F U N C T I O N   D E C L A R A T I O N S
********************************************************************************
*/

/*******************************************************************************
*                          F U N C T I O N S
********************************************************************************
*/

/*----------------------------------------------------------------------------*/
/*!
* \brief TKIP Michael block function
*
* \param[in][out] pu4L - pointer to left value
* \param[in][out] pu4PR - pointer to right value
*
* \return (none)
*/
/*----------------------------------------------------------------------------*/
static VOID tkipMicB(IN OUT PUINT_32 pu4L, IN OUT PUINT_32 pu4R)
{
	ASSERT(pu4L);
	ASSERT(pu4R);

	*pu4R = *pu4R ^ ROTL32(*pu4L, 17U);	/* r <- r ^ (l<<<17)    */
	*pu4L = (*pu4L + *pu4R);	/* l <- (l+r) mod 2^32  */
	*pu4R = *pu4R ^ XSWAP32(*pu4L);	/* r <- r ^ XSWAP(l)    */
	*pu4L = (*pu4L + *pu4R);	/* l <- (l+r) mod 2^32  */
	*pu4R = *pu4R ^ ROTL32(*pu4L, 3U);	/* r <- r ^ (l<<<3)     */
	*pu4L = (*pu4L + *pu4R);	/* l <- (l+r) mod 2^32  */
	*pu4R = *pu4R ^ ROTR32(*pu4L, 2U);	/* r <- r ^ (l>>>2)     */
	*pu4L = (*pu4L + *pu4R);	/* l <- (l+r) mod 2^32  */
}				/* tkipMicB */

/*----------------------------------------------------------------------------*/
/*!
* \brief TKIP Michael generation function
*
* \param[in]  pucMickey Pointer to MIC key
* \param[in]  pucData Pointer to message
* \param[in]  u4DataLen Message length, in byte(s)
* \param[in]  pucSa Pointer to source address SA
* \param[in]  pucDa Pointer to destination address DA
* \param[in]  ucPriority Priority of IEEE 802.11 traffic class
* \param[out] pucMic Pointer to 64-bit MIC
*
* \return (none)
*/
/*----------------------------------------------------------------------------*/
static VOID
tkipMicGen(IN PUCHAR pucMickey,
	   IN PUCHAR pucData, IN UINT_16 u4DataLen, IN PUCHAR pucSa,
	   IN PUCHAR pucDa, IN UCHAR ucPriority, OUT PUCHAR pucMic)
{

	UINT_16 i;
	UINT_32 l, r;
	UINT_32 au4Msg[3];

	ASSERT(pucMickey);
	ASSERT(pucData);
	ASSERT(pucSa);
	ASSERT(pucDa);
	ASSERT(pucMic);

	WLAN_GET_FIELD_32(pucMickey, &l);
	WLAN_GET_FIELD_32(pucMickey + 4, &r);

	/* Michael message processing for DA and SA. */
	WLAN_GET_FIELD_32(pucDa, &au4Msg[0]);
	au4Msg[1] = ((UINT_32) pucDa[4]) | ((UINT_32) pucDa[5] << 8U) |
	    ((UINT_32) pucSa[0] << 16U) | ((UINT_32) pucSa[1] << 24U);
	WLAN_GET_FIELD_32(pucSa + 2, &au4Msg[2]);

	for (i = 0U; i < 3U; i++) {
		l = l ^ au4Msg[i];
		tkipMicB(&l, &r);
	}

	/* Michael message processing for priority. */
	au4Msg[0] = (UINT_32) ucPriority;

	l = l ^ au4Msg[0];
	tkipMicB(&l, &r);

	/*
	 * Michael message processing for MSDU data playload except the last octets
	 * which cannot be partitioned into a 32-bit word.
	 */
	for (i = 0U; i < u4DataLen / 4U; i++) {
		WLAN_GET_FIELD_32(pucData + i * 4U, &au4Msg[0]);
		l = l ^ au4Msg[0];
		tkipMicB(&l, &r);
	}

	/*
	 * Michael message processing for the last uncomplete octets, if present,
	 * and the padding.
	 */
	switch (u4DataLen & 3U) {
	case 1:
		au4Msg[0] = ((UINT_32) pucData[u4DataLen - 1U]) | 0x00005A00U;
		break;

	case 2:
		au4Msg[0] = ((UINT_32) pucData[u4DataLen - 2U]) |
			((UINT_32) pucData[u4DataLen - 1U] << 8U) |
			0x005A0000U;
		break;

	case 3:
		au4Msg[0] = ((UINT_32) pucData[u4DataLen - 3U]) |
			((UINT_32) pucData[u4DataLen - 2U] << 8U) |
			((UINT_32) pucData[u4DataLen - 1U] << 16U) |
			0x5A000000U;
		break;

	default:
		au4Msg[0] = 0x0000005AU;
		break;
	}
	au4Msg[1] = 0;
	for (i = 0U; i < 2U; i++) {
		l = l ^ au4Msg[i];
		tkipMicB(&l, &r);
	}

	/* return ( l, r ), i.e. MIC */
	WLAN_SET_FIELD_32(pucMic, l);
	WLAN_SET_FIELD_32(pucMic + 4, r);

}				/* tkipMicGen */

/*----------------------------------------------------------------------------*/
/*!
* \brief This function decapsulate MSDU frame body (with MIC) according
*        to IEEE 802.11i TKIP sepcification.
*
* \param[in]  prAdapter Pointer to the adapter object data area.
* \param[in]  prMacHeader Pointer to frame MAC header
* \param[in]  pucFrameBody Pointer to frame body
* \param[in]  u4FrameBodyLen Length of frame body (in bytes), include
*                            length of ICV and MIC
* \param[in]  pucMickey Pointer to MIC key
* \param[out] pu4ResultFrameBodyLen Pointer to put the result frame body length.
*
* \retval FALSE(TKIP_MIC_ERR), if this MSDU is not decapsulatable, i.e. MIC
*         verification is failure.
*         TRUE(TKIP_DECAPSULATE_SUCCESS), if this TKIP MSDU is decapsulated
*         successfully, i.e. MIC verification is successful.
*
* \note 1  If return TRUE, result frame body length
*          is only equal to data payload legth, and the result frame
*          body's format is MSDU
*       2. If return FALSE, result frame body length is equal
*          to data payload legth plus MIC and MIC', and the result
*          frame body's format is: MSDU + MIC
*/
/*----------------------------------------------------------------------------*/
BOOLEAN tkipMicDecapsulate(IN P_SW_RFB_T prSwRfb, IN PUINT_8 pucMicKey)
{
	PUCHAR pucMic1;		/* MIC  */
	UCHAR aucMic2[8];	/* MIC' */
	UCHAR ucPriority;
	BOOLEAN fgStatus = FALSE;
	PUCHAR pucSa, pucDa;
	/* PUCHAR              pucMickey; */
	PUCHAR pucFrameBody;
	UINT_16 u2FrameBodyLen;
	P_WLAN_MAC_HEADER_T prMacHeader;

	DEBUGFUNC("tkipMicDecapsulate");

	ASSERT(prSwRfb);
	ASSERT(pucMicKey);

	/* prRxStatus = prSwRfb->prRxStatus; */
	pucFrameBody = prSwRfb->pucPayload;
	if (pucFrameBody == NULL) {
		DBGLOG(RSN, INFO, "pucPayload is NULL, drop this packet");
		return FALSE;
	}
	u2FrameBodyLen = prSwRfb->u2PayloadLength;

	/* if ((prRxStatus->ucKIdxSecMode & BITS(0,3)) != CIPHER_SUITE_TKIP_WO_MIC){ */
	/* return TRUE; */
	/* } */

	DBGLOG(RSN, LOUD, "Before TKIP MSDU Decapsulate:\n");
	DBGLOG(RSN, LOUD, "MIC key:\n");
	/* DBGLOG_MEM8(RSN, LOUD, pucMicKey, 8); */

	prMacHeader = (P_WLAN_MAC_HEADER_T) prSwRfb->pvHeader;
	ASSERT(prMacHeader);

	pucDa = prMacHeader->aucAddr1;
	pucSa = prMacHeader->aucAddr3;

	switch (prMacHeader->u2FrameCtrl & MASK_TO_DS_FROM_DS) {
	case 0:
		pucDa = prMacHeader->aucAddr1;
		pucSa = prMacHeader->aucAddr2;
		break;
	case MASK_FC_FROM_DS:
		pucDa = prMacHeader->aucAddr1;
		pucSa = prMacHeader->aucAddr3;
		break;
	default:
		ASSERT_BOOLEAN((prMacHeader->u2FrameCtrl &
			MASK_FC_TO_DS) == 0U);
		fgStatus = TRUE;
		break;
	}

	if (fgStatus == TRUE)
		return TRUE;

	if (RXM_IS_QOS_DATA_FRAME(prSwRfb->u2FrameCtrl) == TRUE)
		ucPriority = (UCHAR) ((((P_WLAN_MAC_HEADER_QOS_T) prSwRfb->pvHeader)->u2QosCtrl) & MASK_QC_TID);
	else
		ucPriority = 0;

	/* generate MIC' */
	tkipMicGen(pucMicKey, pucFrameBody, u2FrameBodyLen - WLAN_MAC_MIC_LEN, pucSa, pucDa, ucPriority, aucMic2);

	/* verify MIC and MIC' */
	pucMic1 = &pucFrameBody[u2FrameBodyLen - WLAN_MAC_MIC_LEN];
	if (pucMic1[0] == aucMic2[0] && pucMic1[1] == aucMic2[1] &&
	    pucMic1[2] == aucMic2[2] && pucMic1[3] == aucMic2[3] &&
	    pucMic1[4] == aucMic2[4] && pucMic1[5] == aucMic2[5] &&
	    pucMic1[6] == aucMic2[6] && pucMic1[7] == aucMic2[7]) {
		u2FrameBodyLen -= WLAN_MAC_MIC_LEN;
		fgStatus = TRUE;
	} else {
		fgStatus = FALSE;
	}

	/* DBGLOG(RSN, LOUD, ("TKIP MIC:\n")); */
	/* DBGLOG_MEM8(RSN, LOUD, pucMic1, 8); */
	/* DBGLOG(RSN, LOUD, ("TKIP MIC':\n")); */
	/* DBGLOG_MEM8(RSN, LOUD, aucMic2, 8); */

	prSwRfb->u2PayloadLength = u2FrameBodyLen;

	DBGLOG(RSN, LOUD, "After TKIP MSDU Decapsulate:\n");
	DBGLOG(RSN, LOUD, "Frame body: (length = %u)\n", u2FrameBodyLen);
	/* DBGLOG_MEM8(RSN, LOUD, pucFrameBody, u2FrameBodyLen); */

	return fgStatus;

}				/* tkipMicDecapsulate */
