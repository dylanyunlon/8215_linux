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

#ifndef _MHL_RX_CBUS_HW_H_
#define _MHL_RX_CBUS_HW_H_

#define IO_BASE_MHL_SINK_CBUS (g_IO_VBASE_VA + 0x22900)
#define IO_BASE_MHL_SINK_CBUS_OFFSET (0x22900)


#define vSinkWriteCbus(dAddr, dVal)  (*((volatile unsigned int *)(IO_BASE_MHL_SINK_CBUS + dAddr)) = (dVal))
#define u4SinkReadCbus(dAddr)         (*((volatile unsigned int *)(IO_BASE_MHL_SINK_CBUS + dAddr)))
#define vSinkWriteCbusMsk(dAddr, dVal, dMsk) (vSinkWriteCbus((dAddr),\
	(u4SinkReadCbus(dAddr) & (~(dMsk))) | ((dVal) & (dMsk))))

#define REG_CBUS_WBUF0 (0x00)
#define SINK_CBUS_WBUF0 (0)/* [10:0] */
#define SINK_CBUS_WBUF1 (16)/* [26:16] */
#define SINK_CBUS_WBUF0_MASK (0x7FF << 0)/* [10:0] */
#define SINK_CBUS_WBUF1_MASK (0x7FF << 16)/* [26:16]   */
#define REG_CBUS_WBUF2 (0x04)
#define SINK_CBUS_WBUF2 (0)/* [10:0] */
#define SINK_CBUS_WBUF3 (16)/* [26:16] */
#define SINK_CBUS_WBUF2_MASK (0x7FF << 0)/* [10:0] */
#define SINK_CBUS_WBUF3_MASK (0x7FF << 16)/* [26:16] */
#define REG_CBUS_WBUF4 (0x08)
#define SINK_CBUS_WBUF4 (0)/* [10:0] */
#define SINK_CBUS_WBUF5 (16)/* [26:16] */
#define SINK_CBUS_WBUF4_MASK (0x7FF << 0)/* [10:0] */
#define SINK_CBUS_WBUF5_MASK (0x7FF << 16)/* [26:16] */
#define REG_CBUS_WBUF6 (0x0C)
#define SINK_CBUS_WBUF6 (0)/* [10:0] */
#define SINK_CBUS_WBUF7 (16)/* [26:16] */
#define SINK_CBUS_WBUF6_MASK (0x7FF << 0)/* [10:0] */
#define SINK_CBUS_WBUF7_MASK (0x7FF << 16)/* [26:16] */
#define REG_CBUS_WBUF8 (0x10)
#define SINK_CBUS_WBUF8 (0)/* [10:0] */
#define SINK_CBUS_WBUF9 (16)/* [26:16] */
#define SINK_CBUS_WBUF8_MASK (0x7FF << 0)/* [10:0] */
#define SINK_CBUS_WBUF9_MASK (0x7FF << 16)/* [26:16] */
#define REG_CBUS_WBUF10 (0x14)
#define SINK_CBUS_WBUF10 (0)/* [10:0] */
#define SINK_CBUS_WBUF11 (16)/* [26:16] */
#define SINK_CBUS_WBUF10_MASK (0x7FF << 0)/* [10:0] */
#define SINK_CBUS_WBUF11_MASK (0x7FF << 16)/* [26:16] */
#define REG_CBUS_WBUF12 (0x18)
#define SINK_CBUS_WBUF12 (0)/* [10:0] */
#define SINK_CBUS_WBUF13 (16)/* [26:16] */
#define SINK_CBUS_WBUF12_MASK (0x7FF << 0)/* [10:0] */
#define SINK_CBUS_WBUF13_MASK (0x7FF << 16)/* [26:16] */
#define REG_CBUS_WBUF14 (0x1C)
#define SINK_CBUS_WBUF14 (0)/* [10:0] */
#define SINK_CBUS_WBUF15 (16)/* [26:16] */
#define SINK_CBUS_WBUF14_MASK (0x7FF << 0)/* [10:0] */
#define SINK_CBUS_WBUF15_MASK (0x7FF << 16)/* [26:16] */
#define REG_CBUS_WBUF16 (0x20)
#define SINK_CBUS_WBUF16 (0)/* [10:0] */
#define SINK_CBUS_WBUF17 (16)/* [26:16] */
#define SINK_CBUS_WBUF16_MASK (0x7FF << 0)/* [10:0] */
#define SINK_CBUS_WBUF17_MASK (0x7FF << 16)/* [26:16] */
#define REG_CBUS_WBUF18 (0x24)
#define SINK_CBUS_WBUF18 (0)/* [10:0] */
#define SINK_CBUS_WBUF19 (16)/* [26:16] */
#define SINK_CBUS_WBUF18_MASK (0x7FF << 0)/* [10:0] */
#define SINK_CBUS_WBUF19_MASK (0x7FF << 16)/* [26:16] */
#define REG_CBUS_WBUF20 (0x28)
#define SINK_CBUS_WBUF20 (0)/* [10:0] */
#define SINK_CBUS_WBUF21 (16)/* [26:16] */
#define SINK_CBUS_WBUF20_MASK (0x7FF << 0)/* [10:0] */
#define SINK_CBUS_WBUF21_MASK (0x7FF << 16)/* [26:16] */
#define REG_CBUS_WBUF22 (0x2C)
#define SINK_CBUS_WBUF22 (0)/* [10:0] */
#define SINK_CBUS_WBUF23 (16)/* [26:16] */
#define SINK_CBUS_WBUF22_MASK (0x7FF << 0)/* [10:0] */
#define SINK_CBUS_WBUF23_MASK (0x7FF << 16)/* [26:16] */
#define REG_CBUS_RBUF (0x30)
#define SINK_CBUS_RBUF (0)/* [10:0] */
#define SINK_CBUS_RBUF_MASK (0x7FF << 0)/* [10:0] */
#define REG_CBUS_LINK_00 (0x34)
#define SINK_RBUF_LVL_THR (27)/* [31:27] */
#define SINK_IGNORE_PAR (25)/* [25:25] */
#define SINK_DUPLEX (24)/* [24:24] */
#define SINK_NRETRY (18)/* [23:18] */
#define SINK_TRESP_HOLD (14)/* [17:14] */
#define SINK_TWAIT (10)/* [13:10] */
#define SINK_TREQ_HOLD (6)/* [9:6] */
#define SINK_TX_NUM (1)/* [5:1] */
#define SINK_TX_TRIG (0)/* [0:0] */
#define SINK_RBUF_LVL_THR_MASK (0x1F << 27)/* [31:27] */
#define SINK_IGNORE_PAR_MASK (0x01 << 25)/* [25:25] */
#define SINK_DUPLEX_MASK (0x01 << 24)/* [24:24] */
#define SINK_NRETRY_MASK (0x3F << 18)/* [23:18] */
#define SINK_TRESP_HOLD_MASK (0x0F << 14)/* [17:14] */
#define SINK_TWAIT_MASK (0x0F << 10)/* [13:10] */
#define SINK_TREQ_HOLD_MASK (0x0F << 6)/* [9:6] */
#define SINK_TX_NUM_MASK (0x1F << 1)/* [5:1] */
#define SINK_TX_TRIG_MASK (0x01 << 0)/* [0:0] */
#define REG_CBUS_LINK_01 (0x38)
#define SINK_ADP_BITIME_EN (31)/* [31:31] */
#define SINK_ADP_BITIME_RST (30)/* [30:30] */
#define SINK_ADP_CONFIG1 (29)/* [29:29] */
#define SINK_ADP_CONFIG2 (28)/* [28:28] */
#define SINK_LINKTX_BITIME (21)/* [27:21] */
#define SINK_CBUS_ACK_0_MIN (14)/* [20:14] */
#define SINK_CBUS_ACK_0_MAX (7)/* [13:7] */
#define SINK_CBUS_ACK_FALL_MAX (0)/* [6:0] */
#define SINK_ADP_BITIME_EN_MASK (0x01 << 31)/* [31:31] */
#define SINK_ADP_BITIME_RST_MASK (0x01 << 30)/* [30:30] */
#define SINK_ADP_CONFIG1_MASK (0x01 << 29)/* [29:29] */
#define SINK_ADP_CONFIG2_MASK (0x01 << 28)/* [28:28] */
#define SINK_LINKTX_BITIME_MASK (0x7F << 21)/* [27:21] */
#define SINK_CBUS_ACK_0_MIN_MASK (0x7F << 14)/* [20:14] */
#define SINK_CBUS_ACK_0_MAX_MASK (0x7F << 7)/* [13:7] */
#define SINK_CBUS_ACK_FALL_MAX_MASK (0x7F << 0)/* [6:0] */
#define REG_CBUS_LINK_02 (0x3C)
#define SINK_LINKRX_EN (31)/* [31:31] */
#define SINK_FAKE_SOURCE (28)/* [28:28] */
#define SINK_TREQ_ACK_CONT (21)/* [27:21] */
#define SINK_LINK_TXDECISION (14)/* [20:14] */
#define SINK_LINK_HALFTRAN_MAX (7)/* [13:7] */
#define SINK_LINK_HALFTRAN_MIN (0)/* [6:0] */
#define SINK_LINKRX_EN_MASK (0x01 << 31)/* [31:31] */
#define SINK_FAKE_SOURCE_MASK (0x01 << 28)/* [28:28] */
#define SINK_TREQ_ACK_CONT_MASK (0x7F << 21)/* [27:21] */
#define SINK_LINK_TXDECISION_MASK (0x7F << 14)/* [20:14] */
#define SINK_LINK_HALFTRAN_MAX_MASK (0x7F << 7)/* [13:7] */
#define SINK_LINK_HALFTRAN_MIN_MASK (0x7F << 0)/* [6:0] */
#define REG_CBUS_LINK_03 (0x40)
#define SINK_SW_RESET_MISC (31)/* [31:31] */
#define SINK_SW_RESET_WAKE (30)/* [30:30] */
#define SINK_SW_RESET_RX (29)/* [29:29] */
#define SINK_SW_RESET_TX (28)/* [28:28] */
#define SINK_LINK_BITIME_MAX (21)/* [27:21] */
#define SINK_LINK_BITIME_MIN (14)/* [20:14] */
#define SINK_LINK_SYNCDUTY_MAX (7)/* [13:7] */
#define SINK_LINK_SYNCDUTY_MIN (0)/* [6:0] */
#define SINK_SW_RESET_MISC_MASK (0x01 << 31)/* [31:31] */
#define SINK_SW_RESET_WAKE_MASK (0x01 << 30)/* [30:30] */
#define SINK_SW_RESET_RX_MASK (0x01 << 29)/* [29:29] */
#define SINK_SW_RESET_TX_MASK (0x01 << 28)/* [28:28] */
#define SINK_LINK_BITIME_MAX_MASK (0x7F << 21)/* [27:21] */
#define SINK_LINK_BITIME_MIN_MASK (0x7F << 14)/* [20:14] */
#define SINK_LINK_SYNCDUTY_MAX_MASK (0x7F << 7)/* [13:7] */
#define SINK_LINK_SYNCDUTY_MIN_MASK (0x7F << 0)/* [6:0] */
#define REG_CBUS_LINK_04 (0x44)
#define SINK_LINK_ACK_MANU_EN (28)/* [28:28] */
#define SINK_LINK_ACK_WIDTH (21)/* [27:21] */
#define SINK_LINK_BITIME_MID (14)/* [20:14] */
#define SINK_LINK_RXDECISION (7)/* [13:7] */
#define SINK_RX_BT_TIMEOUT (0)/* [6:0] */
#define SINK_LINK_ACK_MANU_EN_MASK (0x01 << 28)/* [28:28] */
#define SINK_LINK_ACK_WIDTH_MASK (0x7F << 21)/* [27:21] */
#define SINK_LINK_BITIME_MID_MASK (0x7F << 14)/* [20:14] */
#define SINK_LINK_RXDECISION_MASK (0x7F << 7)/* [13:7] */
#define SINK_RX_BT_TIMEOUT_MASK (0x7F << 0)/* [6:0] */
#define REG_CBUS_LINK_05 (0x48)
#define SINK_DISCOVERY_EN (31)/* [31:31] */
#define SINK_WAKEUP_EN (30)/* [30:30] */
#define SINK_WAKE_PUL_WID2_MAX (20)/* [29:20] */
#define SINK_WAKE_PUL_WID1_MIN (10)/* [19:10] */
#define SINK_WAKE_PUL_WID1_MAX (0)/* [9:0] */
#define SINK_DISCOVERY_EN_MASK (0x01 << 31)/* [31:31] */
#define SINK_WAKEUP_EN_MASK (0x01 << 30)/* [30:30] */
#define SINK_WAKE_PUL_WID2_MAX_MASK (0x3FF << 20)/* [29:20] */
#define SINK_WAKE_PUL_WID1_MIN_MASK (0x3FF << 10)/* [19:10] */
#define SINK_WAKE_PUL_WID1_MAX_MASK (0x3FF << 0)/* [9:0] */
#define REG_CBUS_LINK_06 (0x4C)
#define SINK_WAKE_PUL_WID2_MIN (22)/* [31:22] */
#define SINK_WAKE_TO_DISC_MIN (11)/* [21:11] */
#define SINK_WAKE_TO_DISC_MAX (0)/* [10:0] */
#define SINK_WAKE_PUL_WID2_MIN_MASK (0x3FF << 22)/* [31:22] */
#define SINK_WAKE_TO_DISC_MIN_MASK (0x7FF << 11)/* [21:11] */
#define SINK_WAKE_TO_DISC_MAX_MASK (0x7FF << 0)/* [10:0] */
#define REG_CBUS_LINK_07 (0x50)
#define SINK_CBUS_OE (29)/* [29:29] */
#define SINK_CBUS_DO (28)/* [28:28] */
#define SINK_CBUS_DOE_SW (27)/* [27:27] */
#define SINK_ZCBUS_DISCOVER_EN (26)/* [26:26] */
#define SINK_ZCBUS_SINK_ON_CTRL (23)/* [25:23] */
#define SINK_ZCBUS_HW (22)/* [22:22] */
#define SINK_LDO_SWITCH (21)/* [21:21] */
#define SINK_LDO_SWITCH_HW (20)/* [20:20] */
#define SINK_DISC_PUL_WID_MIN (10)/* [19:10] */
#define SINK_DISC_PUL_WID_MAX (0)/* [9:0] */
#define SINK_CBUS_OE_MASK (0x01 << 29)/* [29:29] */
#define SINK_CBUS_DO_MASK (0x01 << 28)/* [28:28] */
#define SINK_CBUS_DOE_SW_MASK (0x01 << 27)/* [27:27] */
#define SINK_ZCBUS_DISCOVER_EN_MASK (0x01 << 26)/* [26:26] */
#define SINK_ZCBUS_SINK_ON_CTRL_MASK (0x07 << 23)/* [25:23] */
#define SINK_ZCBUS_HW_MASK (0x01 << 22)/* [22:22] */
#define SINK_LDO_SWITCH_MASK (0x01 << 21)/* [21:21] */
#define SINK_LDO_SWITCH_HW_MASK (0x01 << 20)/* [20:20] */
#define SINK_DISC_PUL_WID_MIN_MASK (0x3FF << 10)/* [19:10] */
#define SINK_DISC_PUL_WID_MAX_MASK (0x3FF << 0)/* [9:0] */
#define REG_CBUS_LINK_08 (0x54)
#define SINK_TX_TRUG_FAIL_INT_CLR (18)/* [18:18] */
#define SINK_LINKRX_TIMEOUT_INT_CLR (17)/* [17:17] */
#define SINK_WBUF_TRIG_INT_CLR (16)/* [16:16] */
#define SINK_RBUF_TRIG_INT_CLR (15)/* [15:15] */
#define SINK_CBUS_LOW_DISCONN_INT_CLR (14)/* [14:14] */
#define SINK_CBUS_NEG_INT_CLR (13)/* [13:13] */
#define SINK_CBUS_POS_INT_CLR (12)/* [12:12] */
#define SINK_MONITOR_CMP_INT_CLR (11)/* [11:11] */
#define SINK_CABLE_DISCONNECT_INT_CLR (10)/* [10:10] */
#define SINK_CABLE_DETECT_INT_CLR (9)/* [9:9] */
#define SINK_TX_ARB_FAIL_INT_CLR (8)/* [8:8] */
#define SINK_TX_OK_INT_CLR (7)/* [7:7] */
#define SINK_TX_RETRY_TO_INT_CLR (6)/* [6:6] */
#define SINK_RBUF_LVL_THR_INT_CLR (5)/* [5:5] */
#define SINK_WK_TIMEOUT_ST_INT_CLR (4)/* [4:4] */
#define SINK_WAKEUP_ILL_WID_INT_CLR (3)/* [3:3] */
#define SINK_ILL_WAKE2DISC_INT_CLR (2)/* [2:2] */
#define SINK_WAKEUP_DET_INT_CLR (1)/* [1:1] */
#define SINK_DISC_DET_INT_CLR (0)/* [0:0] */
#define SINK_TX_TRUG_FAIL_INT_CLR_MASK (0x01 << 18)/* [18:18] */
#define SINK_LINKRX_TIMEOUT_INT_CLR_MASK (0x01 << 17)/* [17:17] */
#define SINK_WBUF_TRIG_INT_CLR_MASK (0x01 << 16)/* [16:16] */
#define SINK_RBUF_TRIG_INT_CLR_MASK (0x01 << 15)/* [15:15] */
#define SINK_CBUS_LOW_DISCONN_INT_CLR_MASK (0x01 << 14)/* [14:14] */
#define SINK_CBUS_NEG_INT_CLR_MASK (0x01 << 13)/* [13:13] */
#define SINK_CBUS_POS_INT_CLR_MASK (0x01 << 12)/* [12:12] */
#define SINK_MONITOR_CMP_INT_CLR_MASK (0x01 << 11)/* [11:11] */
#define SINK_CABLE_DISCONNECT_INT_CLR_MASK (0x01 << 10)/* [10:10] */
#define SINK_CABLE_DETECT_INT_CLR_MASK (0x01 << 9)/* [9:9] */
#define SINK_TX_ARB_FAIL_INT_CLR_MASK (0x01 << 8)/* [8:8] */
#define SINK_TX_OK_INT_CLR_MASK (0x01 << 7)/* [7:7] */
#define SINK_TX_RETRY_TO_INT_CLR_MASK (0x01 << 6)/* [6:6] */
#define SINK_RBUF_LVL_THR_INT_CLR_MASK (0x01 << 5)/* [5:5] */
#define SINK_WK_TIMEOUT_ST_INT_CLR_MASK (0x01 << 4)/* [4:4] */
#define SINK_WAKEUP_ILL_WID_INT_CLR_MASK (0x01 << 3)/* [3:3] */
#define SINK_ILL_WAKE2DISC_INT_CLR_MASK (0x01 << 2)/* [2:2] */
#define SINK_WAKEUP_DET_INT_CLR_MASK (0x01 << 1)/* [1:1] */
#define SINK_DISC_DET_INT_CLR_MASK (0x01 << 0)/* [0:0] */
#define REG_CBUS_LINK_09 (0x58)
#define SINK_TREQ_ARB_CONT (25)/* [31:25] */
#define SINK_CBUS_DRV_H_PRD (20)/* [24:20] */
#define SINK_DISC_WID_CNT_TIMEOUT (10)/* [19:10] */
#define SINK_WAKE_CNT_TIMEOUT (0)/* [9:0] */
#define SINK_TREQ_ARB_CONT_MASK (0x7F << 25)/* [31:25] */
#define SINK_CBUS_DRV_H_PRD_MASK (0x1F << 20)/* [24:20] */
#define SINK_DISC_WID_CNT_TIMEOUT_MASK (0x3FF << 10)/* [19:10] */
#define SINK_WAKE_CNT_TIMEOUT_MASK (0x3FF << 0)/* [9:0] */
#define REG_CBUS_LINK_0A (0x5C)
#define SINK_LINKRX_ACK1_SYNC (25)/* [31:25] */
#define SINK_LINKRX_DIS_TO (19)/* [19:19] */
#define SINK_TX_ARB_BYPASS (18)/* [18:18] */
#define SINK_RX_ARB_BYPASS (17)/* [17:17] */
#define SINK_FORCE_NAK_EN (16)/* [16:16] */
#define SINK_FORCE_NAK_CNT (11)/* [15:11] */
#define SINK_MON_CMP_SEL (8)/* [10:8] */
#define SINK_MON_CMP_DATA (0)/* [7:0] */
#define SINK_LINKRX_ACK1_SYNC_MASK (0x7F << 25)/* [31:25] */
#define SINK_LINKRX_DIS_TO_MASK (0x01 << 19)/* [19:19] */
#define SINK_TX_ARB_BYPASS_MASK (0x01 << 18)/* [18:18] */
#define SINK_RX_ARB_BYPASS_MASK (0x01 << 17)/* [17:17] */
#define SINK_FORCE_NAK_EN_MASK (0x01 << 16)/* [16:16] */
#define SINK_FORCE_NAK_CNT_MASK (0x1F << 11)/* [15:11] */
#define SINK_MON_CMP_SEL_MASK (0x07 << 8)/* [10:8] */
#define SINK_MON_CMP_DATA_MASK (0xFF << 0)/* [7:0] */
#define REG_CBUS_LINK_0B (0x60)
#define SINK_INT_STA (13)/* [31:14] */
#define SINK_CBUS_DISCONN_CNT_EN (13)/* [13:13] */
#define SINK_RETRY_DISCONN_THR (7)/* [12:7] */
#define SINK_CBUS_DISCONN_THR (0)/* [6:0] */
#define SINK_INT_STA_MASK (0x7FFFF << 13)/* [31:14] */
#define SINK_CBUS_DISCONN_CNT_EN_MASK (0x01 << 13)/* [13:13] */
#define SINK_RETRY_DISCONN_THR_MASK (0x3F << 7)/* [12:7] */
#define SINK_CBUS_DISCONN_THR_MASK (0x7F << 0)/* [6:0] */
/*	#define SINK_CDSENSE_FORCE1_MASK (0x01 << 25) [25:25]
	#define SINK_CDSENSE_GATED_MASK (0x01 << 24)  [24:24]
	#define SINK_CDSENSE_OFF_DEGLITCH_MASK (0x01 << 23) [23:23]
	#define SINK_CDSENSE_ON_DEGLITCH_MASK (0x01 << 22) [22:22]
	#define SINK_CDSENSE_P_DEGLITCH_MASK (0x01 << 21)  [21:21]
	#define SINK_LINKRX_BITIME_MASK (0x7F << 14)  [20:14]
	#define SINK_ADP_BITIME_MIN_MASK (0x7F << 7)  [13:7]
	#define SINK_ADP_BITIME_MAX_MASK (0x7F << 0)  [6:0] */
#define REG_CBUS_LINK_0C (0x64)
#define SINK_CDSENSE_FORCE1 (25)/* [25:25] */
#define SINK_CDSENSE_GATED (24)/* [24:24] */
#define SINK_CDSENSE_OFF_DEGLITCH (23)/* [23:23] */
#define SINK_CDSENSE_ON_DEGLITCH (22)/* [22:22] */
#define SINK_CDSENSE_P_DEGLITCH (21)/* [21:21] */
#define SINK_LINKRX_BITIME (14)/* [20:14] */
#define SINK_ADP_BITIME_MIN (7)/* [13:7] */
#define SINK_ADP_BITIME_MAX (0)/* [6:0] */
#define REG_CBUS_LINK_0D (0x68)
#define SINK_INT_MASK (0)/* [16:0] */
#define SINK_INT_MASK_MASK (0x1FFFF << 0)/* [16:0] */
#define REG_CBUS_STA_00 (0x6C)
#define SINK_LINK_BITIME (25)/* [31:25] */
#define SINK_RBUF_LVL_LAT (20)/* [24:20] */
#define SINK_TX_TRIG_FAIL_INT (18)/* [18:18] */
#define SINK_LINKRX_TIMEOUT_INT (17)/* [17:17] */
#define SINK_WBUF_PULS1_INT (16)/* [16:16] */
#define SINK_RBUF_PULS1_INT (15)/* [15:15] */
#define SINK_CBUS_LOW_DISCONN_INT (14)/* [14:14] */
#define SINK_CBUS_NEG_INT (13)/* [13:13] */
#define SINK_CBUS_POS_INT (12)/* [12:12] */
#define SINK_MONITOR_CMP_INT (11)/* [11:11] */
#define SINK_CABLE_DISCONNECT_INT (10)/* [10:10] */
#define SINK_CABLE_DETECT_INT (9)/* [9:9] */
#define SINK_TX_ARB_FAIL_INT (8)/* [8:8] */
#define SINK_TX_OK_INT (7)/* [7:7] */
#define SINK_TX_RETRY_TO_INT (6)/* [6:6] */
#define SINK_RBUF_LVL_THR_INT (5)/* [5:5] */
#define SINK_WK_TIMEOUT_ST_INT (4)/* [4:4] */
#define SINK_WAKEUP_ILL_WID_INT (3)/* [3:3] */
#define SINK_ILL_WAKE2DISC_INT (2)/* [2:2] */
#define SINK_WAKEUP_DET_INT (1)/* [1:1] */
#define SINK_DISC_DET_INT (0)/* [0:0] */
#define SINK_LINK_BITIME_MASK (0x7F << 25)/* [31:25] */
#define SINK_RBUF_LVL_LAT_MASK (0x1F << 20)/* [24:20] */
#define SINK_TX_TRIG_FAIL_INT_MASK (0x01 << 18)/* [18:18] */
#define SINK_LINKRX_TIMEOUT_INT_MASK (0x01 << 17)/* [17:17] */
#define SINK_WBUF_PULS1_INT_MASK (0x01 << 16)/* [16:16] */
#define SINK_RBUF_PULS1_INT_MASK (0x01 << 15)/* [15:15] */
#define SINK_CBUS_LOW_DISCONN_INT_MASK (0x01 << 14)/* [14:14] */
#define SINK_CBUS_NEG_INT_MASK (0x01 << 13)/* [13:13] */
#define SINK_CBUS_POS_INT_MASK (0x01 << 12)/* [12:12] */
#define SINK_MONITOR_CMP_INT_MASK (0x01 << 11)/* [11:11] */
#define SINK_CABLE_DISCONNECT_INT_MASK (0x01 << 10)/* [10:10] */
#define SINK_CABLE_DETECT_INT_MASK (0x01 << 9)/* [9:9] */
#define SINK_TX_ARB_FAIL_INT_MASK (0x01 << 8)/* [8:8] */
#define SINK_TX_OK_INT_MASK (0x01 << 7)/* [7:7] */
#define SINK_TX_RETRY_TO_INT_MASK (0x01 << 6)/* [6:6] */
#define SINK_RBUF_LVL_THR_INT_MASK (0x01 << 5)/* [5:5] */
#define SINK_WK_TIMEOUT_ST_INT_MASK (0x01 << 4)/* [4:4] */
#define SINK_WAKEUP_ILL_WID_INT_MASK (0x01 << 3)/* [3:3] */
#define SINK_ILL_WAKE2DISC_INT_MASK (0x01 << 2)/* [2:2] */
#define SINK_WAKEUP_DET_INT_MASK (0x01 << 1)/* [1:1] */
#define SINK_DISC_DET_INT_MASK (0x01 << 0)/* [0:0] */
#define REG_CBUS_STA_01 (0x70)
#define SINK_CBUS_IN (31)/* [31:31] */
#define SINK_CBUS_CDSENSE (30)/* [30:30] */
#define SINK_LINKRX_FSM (12)/* [16:12] */
#define SINK_LINKTX_FSM (7)/* [11:7] */
#define SINK_DISC_FSM (4)/* [6:4] */
#define SINK_WAKEUP_FSM (0)/* [3:0] */
#define SINK_CBUS_IN_MASK (0x01 << 31)/* [31:31] */
#define SINK_CBUS_CDSENSE_MASK (0x01 << 30)/* [30:30] */
#define SINK_LINKRX_FSM_MASK (0x1F << 12)/* [16:12] */
#define SINK_LINKTX_FSM_MASK (0x1F << 7)/* [11:7] */
#define SINK_DISC_FSM_MASK (0x07 << 4)/* [6:4] */
#define SINK_WAKEUP_FSM_MASK (0x0F << 0)/* [3:0] */
#define REG_CBUS_LINK_BAK (0x74)
#define SINK_CBUS_DRV_H_PRD_ARB (16)/* [16:21] */
#define SINK_CBUS_DRV_H_SEL (15)/* [15:15] */
#define SINK_CBUS_OE_FAST1 (14)/* [14:14] */
#define SINK_ADP_SYNC (13)/* [13:13] */
#define SINK_CBUS_OE_FAST (12)/* [12:12] */
#define SINK_CBUS_DEGLITCH (11)/* [11:11]   */
#define SINK_LINKRX_DIS_TO_ARB23 (10)/* [10:10] */
#define SINK_LINK_ACK_WIDTH_UPPER (3)/* [3:10] */
#define SINK_ARB_DRIVEH (2)/* [2:2] */
#define SINK_TX_ARB2_DIS_CHK (1)/* [1:1] */
#define SINK_TX_ARB1_DIS_CHK (0)/* [0:0] */
#define SINK_CBUS_DRV_H_PRD_ARB_MASK (0x1F << 16)/* [16:21] */
#define SINK_CBUS_DRV_H_SEL_MASK (0x01 << 15)/* [15:15] */
#define SINK_CBUS_OE_FAST1_MASK (0x01 << 14)/* [14:14] */
#define SINK_ADP_SYNC_MASK (0x01 << 13)/* [13:13] */
#define SINK_CBUS_OE_FAST_MASK (0x01 << 12)/* [12:12] */
#define SINK_CBUS_DEGLITCH_MASK (0x01 << 11)/* [11:11] */
#define SINK_LINKRX_DIS_TO_ARB23_MASK (0x01 << 10)/* [10:10] */
#define SINK_LINK_ACK_WIDTH_UPPER_MASK (0x7F << 3)/* [3:10] */
#define SINK_ARB_DRIVEH_MASK (0x01 << 2)/* [2:2] */
#define SINK_TX_ARB2_DIS_CHK_MASK (0x01 << 1)/* [1:1] */
#define SINK_TX_ARB1_DIS_CHK_MASK (0x01 << 0)/* [0:0] */

/* #define MHLRX_DBG_BY_PIN_TRIGGER */

#endif
