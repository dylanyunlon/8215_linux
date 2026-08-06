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
** Id: //Department/DaVinci/BRANCHES/MT6620_WIFI_DRIVER_V2_3/include/nic/mt6630_reg.h#1
*/

/*
 * ! \file   "mt6630_reg.h"
 *   \brief  The common register definition of MT6630
 *
 *   N/A
 */

#ifndef _MT6630_REG_H
#define _MT6630_REG_H

/*******************************************************************************
*                         C O M P I L E R   F L A G S
********************************************************************************
*/

/*******************************************************************************
*                    E X T E R N A L   R E F E R E N C E S
********************************************************************************
*/

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

/* 1 MT6630 MCR Definition */

/* 2 Host Interface */

/* 4 CHIP ID Register */
#define MCR_WCIR                            0x0000U

/* 4 HIF Low Power Control  Register */
#define MCR_WHLPCR                          0x0004U

/* 4 Control  Status Register */
#define MCR_WSDIOCSR                        0x0008U

/* 4 HIF Control Register */
#define MCR_WHCR                            0x000CU

/* 4 HIF Interrupt Status  Register */
#define MCR_WHISR                           0x0010U

/* 4 HIF Interrupt Enable  Register */
#define MCR_WHIER                           0x0014U

/* 4 Abnormal Status Register */
#define MCR_WASR                            0x0020U

/* 4 WLAN Software Interrupt Control Register */
#define MCR_WSICR                           0x0024U

/* 4 WLAN TX Data Register 1 */
#define MCR_WTDR1                           0x0034U

/* 4 WLAN RX Data Register 0 */
#define MCR_WRDR0                           0x0050U

/* 4 WLAN RX Data Register 1 */
#define MCR_WRDR1                           0x0054U

/* 4 Host to Device Send Mailbox 0 Register */
#define MCR_H2DSM0R                         0x0070U

/* 4 Host to Device Send Mailbox 1 Register */
#define MCR_H2DSM1R                         0x0074U

/* 4 Device to Host Receive Mailbox 0 Register */
#define MCR_D2HRM0R                         0x0078U

/* 4 Device to Host Receive Mailbox 1 Register */
#define MCR_D2HRM1R                         0x007cU

/* 4 WLAN RX Packet Length Register */
#define MCR_WRPLR                           0x0090U

/* 4 Test Mode Data Port */
#define MCR_WTMDR                           0x00B0U

/* 4 Test Mode Control Register */
#define MCR_WTMCR                           0x00B4U

/* 4 Test Mode Data Pattern Control Register #0 */
#define MCR_WTMDPCR0                        0x00B8U

/* 4 Test Mode Data Pattern Control Register #1 */
#define MCR_WTMDPCR1                        0x00BcU

/* 4 WLAN Packet Length Report Control Register */
#define MCR_WPLRCR                          0x00D4U

/* 4 WLAN Snapshot Register */
#define MCR_WSR                             0x00D8U

#if defined(MT6631)
/* 4 Abnormal Status Register2 */
#define MCR_WASR2                           0x00E0U
#endif

/* 4 Clock Pad Macro IO Control Register */
#define MCR_CLKIOCR                         0x0100U

/* 4 Command Pad Macro IO Control Register */
#define MCR_CMDIOCR                         0x0104U

/* 4 Data 0 Pad Macro IO Control Register */
#define MCR_DAT0IOCR                        0x0108U

/* 4 Data 1 Pad Macro IO Control Register */
#define MCR_DAT1IOCR                        0x010CU

/* 4 Data 2 Pad Macro IO Control Register */
#define MCR_DAT2IOCR                        0x0110U

/* 4 Data 3 Pad Macro IO Control Register */
#define MCR_DAT3IOCR                        0x0114U

/* 4 Clock Pad Macro Delay Chain Control Register */
#define MCR_CLKDLYCR                        0x0118U

/* 4 Command Pad Macro Delay Chain Control Register */
#define MCR_CMDDLYCR                        0x011CU

/* 4 SDIO Output Data Delay Chain Control Register */
#define MCR_ODATDLYCR                       0x0120U

/* 4 SDIO Input Data Delay Chain Control Register 1 */
#define MCR_IDATDLYCR1                      0x0124U

/* 4 SDIO Input Data Delay Chain Control Register 2 */
#define MCR_IDATDLYCR2                      0x0128U

/* 4 SDIO Input Data Latch Time Control Register */
#define MCR_ILCHCR                          0x012CU

/* 4 WLAN TXQ Count Register 0 */
#define MCR_WTQCR0                          0x0130U

/* 4 WLAN TXQ Count Register 1 */
#define MCR_WTQCR1                          0x0134U

/* 4 WLAN TXQ Count Register 2 */
#define MCR_WTQCR2                          0x0138U

/* 4 WLAN TXQ Count Register 3 */
#define MCR_WTQCR3                          0x013CU

/* 4 WLAN TXQ Count Register 4 */
#define MCR_WTQCR4                          0x0140U

/* 4 WLAN TXQ Count Register 5 */
#define MCR_WTQCR5                          0x0144U

/* 4 WLAN TXQ Count Register 6 */
#define MCR_WTQCR6                          0x0148U

/* 4 WLAN TXQ Count Register 7 */
#define MCR_WTQCR7                          0x014CU

/* #if CFG_SDIO_INTR_ENHANCE */
typedef struct _ENHANCE_MODE_DATA_STRUCT_T {
	UINT_32 u4WHISR;
	union {
		struct {
			UINT_16 u2TQ0Cnt;
			UINT_16 u2TQ1Cnt;
			UINT_16 u2TQ2Cnt;
			UINT_16 u2TQ3Cnt;
			UINT_16 u2TQ4Cnt;
			UINT_16 u2TQ5Cnt;
			UINT_16 u2TQ6Cnt;
			UINT_16 u2TQ7Cnt;
			UINT_16 u2TQ8Cnt;
			UINT_16 u2TQ9Cnt;
			UINT_16 u2TQ10Cnt;
			UINT_16 u2TQ11Cnt;
			UINT_16 u2TQ12Cnt;
			UINT_16 u2TQ13Cnt;
			UINT_16 u2TQ14Cnt;
			UINT_16 u2TQ15Cnt;
		} u;
		UINT_32 au4WTSR[8];
	} rTxInfo;
	union {
		struct {
			UINT_16 u2NumValidRx0Len;
			UINT_16 u2NumValidRx1Len;
			UINT_16 au2Rx0Len[16];
			UINT_16 au2Rx1Len[16];
		} u;
		UINT_32 au4RxStatusRaw[17];
	} rRxInfo;
	UINT_32 u4RcvMailbox0;
	UINT_32 u4RcvMailbox1;
} ENHANCE_MODE_DATA_STRUCT_T, *P_ENHANCE_MODE_DATA_STRUCT_T;
			 /* #endif *//* ENHANCE_MODE_DATA_STRUCT_T */

/* 2 Definition in each register */
/* 3 WCIR 0x0000 */
#define WCIR_WLAN_READY                 BIT(21U)
#define WCIR_POR_INDICATOR              BIT(20U)
#define WCIR_REVISION_ID                BITS(16U, 19U)
#define WCIR_CHIP_ID                    BITS(0U, 15U)

#define MTK_CHIP_REV                    0x00006630U
#define MTK_CHIP_MP_REVERSION_ID        0x0U

/* 3 WHLPCR 0x0004 */
#define WHLPCR_FW_OWN_REQ_CLR           BIT(9U)
#define WHLPCR_FW_OWN_REQ_SET           BIT(8U)
#define WHLPCR_IS_DRIVER_OWN            BIT(8U)
#define WHLPCR_INT_EN_CLR               BIT(1U)
#define WHLPCR_INT_EN_SET               BIT(0U)

/* 3 WSDIOCSR 0x0008 */
#define WSDIOCSR_DB_CMD7_RESELECT_DIS   BIT(4U)
#define WSDIOCSR_DB_WR_BUSY_EN          BIT(3U)
#define WSDIOCSR_DB_RD_BUSY_EN          BIT(2U)
#define WSDIOCSR_SDIO_INT_CTL           BIT(1U)
#define WSDIOCSR_SDIO_RE_INIT_EN        BIT(0U)

/* 3 WHCR 0x000C */
#define WHCR_RX_ENHANCE_MODE_EN         BIT(16U)
#define WHCR_MAX_HIF_RX_LEN_NUM         BITS(8U, 13U)
#define WHCR_RPT_OWN_RX_PACKET_LEN      BIT(3U)
#define WHCR_RECV_MAILBOX_RD_CLR_EN     BIT(2U)
#define WHCR_W_INT_CLR_CTRL             BIT(1U)
#define WHCR_MCU_DBG_EN                 BIT(0U)
#define WHCR_OFFSET_MAX_HIF_RX_LEN_NUM  8U

/* 3 WHISR 0x0010 */
#define WHISR_D2H_SW_INT                BITS(8U, 31U)
#define WHISR_D2H_SW_ASSERT_INFO_INT    BIT(31U)
#define WHISR_FW_OWN_BACK_INT           BIT(7U)
#define WHISR_ABNORMAL_INT              BIT(6U)
#define WHISR_RX1_DONE_INT              BIT(2U)
#define WHISR_RX0_DONE_INT              BIT(1U)
#define WHISR_TX_DONE_INT               BIT(0U)

/* 3 WHIER 0x0014 */
#define WHIER_D2H_SW_INT                BITS(8U, 31U)
#define WHIER_FW_OWN_BACK_INT_EN        BIT(7U)
#define WHIER_ABNORMAL_INT_EN           BIT(6U)
#define WHIER_RX1_DONE_INT_EN           BIT(2U)
#define WHIER_RX0_DONE_INT_EN           BIT(1U)
#define WHIER_TX_DONE_INT_EN            BIT(0U)
#define WHIER_DEFAULT                   (WHIER_RX0_DONE_INT_EN    | \
					 WHIER_RX1_DONE_INT_EN    | \
					 WHIER_TX_DONE_INT_EN     | \
					 WHIER_ABNORMAL_INT_EN    | \
					 WHIER_D2H_SW_INT           \
					 )

/* 3 WASR 0x0020 */
#define WASR_FW_OWN_INVALID_ACCESS      BIT(16U)
#define WASR_RX1_UNDER_FLOW             BIT(9U)
#define WASR_RX0_UNDER_FLOW             BIT(8U)
#define WASR_TX1_OVER_FLOW              BIT(1U)

/* 3 WSICR 0x0024 */
#define WSICR_H2D_SW_INT_SET            BITS(16U, 31U)

/* 3 WRPLR 0x0090 */
#define WRPLR_RX1_PACKET_LENGTH         BITS(16U, 31U)
#define WRPLR_RX0_PACKET_LENGTH         BITS(0U, 15U)

/* 3 WTMCR 0x00b4 */
#define WMTCR_TEST_MODE_FW_OWN          BIT(24U)
#define WMTCR_PRBS_INIT_VAL             BITS(16U, 23U)
#define WMTCR_TEST_MODE_STATUS          BIT(8U)
#define WMTCR_TEST_MODE_SELECT          BITS(0U, 1U)

#endif /* _MT6628_REG_H */
