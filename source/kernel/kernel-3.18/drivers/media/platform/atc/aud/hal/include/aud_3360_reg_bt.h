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

#ifndef _AUDIO_3360_REG_BT_H_
#define _AUDIO_3360_REG_BT_H_


/////////////////////////////////////////////////////////////////////////////
//                   Follow Register access by RISC
/////////////////////////////////////////////////////////////////////////////
#define AUD_REG_PCM_BASE                       (0xA8400)
#define AUD_REG_PCM_BUF_BANK            (AUD_REG_RGBK2_BASE + 0xcc)
#define AUD_REG_PCM_BUF_BLK             (AUD_REG_RGBK2_BASE + 0xe0)
#define AUD_REG_PCM_RXBLK_START         (0)
#define AUD_REG_PCM_TXBLK_START         (16)
#define AUD_REG_PCM_BLK_NUM             (16)

#define AUD_REG_PCM_CTRL                (AUD_REG_PCM_BASE + 0xc4)
#define AUD_REG_PCM_RX_SADR             (AUD_REG_PCM_BASE + 0xcc)
#define AUD_REG_PCM_RX_EADR             (AUD_REG_PCM_BASE + 0xd0)
#define AUD_REG_PCM_TX_SADR             (AUD_REG_PCM_BASE + 0xd8)
#define AUD_REG_PCM_TX_EADR             (AUD_REG_PCM_BASE + 0xdc)
#define AUD_REG_PCM_TX_NSADR            (AUD_REG_PCM_BASE + 0xe0)
#define AUD_REG_PCM_TX_SAMPLE_NUM       (AUD_REG_PCM_BASE + 0xe8)
#define AUD_REG_PCM_TX_REMAIN_NUM       (AUD_REG_PCM_BASE + 0xe8)

#define AUD_REG_PCM_RX_WP               (AUD_REG_PCM_BASE + 0xec)
#define AUD_REG_PCM_TX_RP               (AUD_REG_PCM_BASE + 0x00)


#define AUD_PCM_DATA_ORDER_BIT_START    (14)
#define AUD_PCM_DATA_ORDER_BIT_NUM      (1)

#define AUD_PCM_SYNC_CYCLE_BIT_START    (6)
#define AUD_PCM_SYNC_CYCLE_BIT_NUM      (1)

#define AUD_PCM_SYNC_MOD_SEL_BIT_START   (7)
#define AUD_PCM_SYNC_MOD_SEL_BIT_NUM     (1)

#define AUD_PCM_SYNC_LEN_BIT_START   (8)
#define AUD_PCM_SYNC_LEN_BIT_NUM     (2)

#define AUD_PCM_MODE_SEL_BIT_START       (3)
#define AUD_PCM_MODE_SEL_BIT_NUM         (1)

#define AUD_PCM_TX_EN_BIT_START          (2)
#define AUD_PCM_TX_EN_BIT_NUM            (1)

#define AUD_PCM_RX_EN_BIT_START          (1)
#define AUD_PCM_RX_EN_BIT_NUM            (1)

#define AUD_PCM_ENABLE_BIT_START         (0)
#define AUD_PCM_ENABLE_BIT_NUM           (1)


#define AUD_PCM_TX_DRAM_S_BIT_START       (0)
#define AUD_PCM_TX_DRAM_S_BIT_NUM         (20)


#define AUD_PCM_TX_DRAM_E_BIT_START     (0)
#define AUD_PCM_TX_DRAM_E_BIT_NUM       (20)
#define AUD_PCM_RX_DRAM_S_BIT_START      (0)
#define AUD_PCM_RX_DRAM_S_BIT_NUM        (20)

#define AUD_PCM_RX_DRAM_E_BIT_START      (0)
#define AUD_PCM_RX_DRAM_E_BIT_NUM        (20)

/////////////////////////////////////////////////////////////////////////////
//                   Follow Register access by DSPC
/////////////////////////////////////////////////////////////////////////////
#define AUD_REG_DSPC_PCM_BASE                  (0x340)

#define AUD_REG_DSPC_PCM_CTRL                  (AUD_REG_DSPC_PCM_BASE + 0x00)
#define AUD_REG_DSPC_PCM_RX_BLK                (AUD_REG_DSPC_PCM_BASE + 0x01)
#define AUD_REG_DSPC_PCM_RX_SADR               (AUD_REG_DSPC_PCM_BASE + 0x02)
#define AUD_REG_DSPC_PCM_RX_EADR               (AUD_REG_DSPC_PCM_BASE + 0x03)
#define AUD_REG_DSPC_PCM_TX_BLK                (AUD_REG_DSPC_PCM_BASE + 0x04)
#define AUD_REG_DSPC_PCM_TX_SADR               (AUD_REG_DSPC_PCM_BASE + 0x05)
#define AUD_REG_DSPC_PCM_TX_EADR               (AUD_REG_DSPC_PCM_BASE + 0x06)
#define AUD_REG_DSPC_PCM_TX_NSADR              (AUD_REG_DSPC_PCM_BASE + 0x07)
#define AUD_REG_DSPC_PCM_TX_NEADR              (AUD_REG_DSPC_PCM_BASE + 0x08)
#define AUD_REG_DSPC_PCM_TX_INTR               (AUD_REG_DSPC_PCM_BASE + 0x09)
#define AUD_REG_DSPC_PCM_RX_WR_ADR             (AUD_REG_DSPC_PCM_BASE + 0x0A)


#endif //#ifndef _AUDIO_3360_REG_BT_H_
