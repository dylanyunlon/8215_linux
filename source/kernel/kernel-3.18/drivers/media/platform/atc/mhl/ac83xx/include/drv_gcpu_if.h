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

#ifndef _DRV_GCPU_IF_H_
#define _DRV_GCPU_IF_H_

#include "x_typedef.h"
#include "drv_config.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Invalid handle in GCPU */
#define GCPU_INV_INST_HANDLE 0xFF
#define GCPU_INV_SLOT_HANDLE 0xFFFF

typedef struct _GCPUMW_CURINST_INFO {
	UINT16 u2CompId;
	UINT32 u4Cmd;
} GCPUMW_CURINST_INFO;

/* GCPU supported encrypt and decrypt algorithm */
#define GCPU_CMD_USE_SEC_CODE   (0x1 << 16)  /* < run command in secure code */
#define GCPU_CMD_IN_SEC_WD      (0x2 << 16)  /* < command used in secure world */

#define CSS_CMD_SID      (0x00 << 8)
#define CPPM_CMD_SID    (0x01 << 8)
#define AES_CMD_SID      (0x02 << 8)
#define VCPS_CMD_SID    (0x03 << 8)
#define AACS_CMD_SID    (0x04 << 8)
#define TDES_CMD_SID    (0x05 << 8)
#define BDRE_CMD_SID    (0x06 << 8)
#define EFUSE_CMD_SID  (0x07 << 8)
#define MISC_CMD_SID    (0x08 << 8)
#define SW_CMD_SID       (0x09 << 8)
/*
#define CONFIG_GCPU_DEBUG_EN
#if CONFIG_GCPU_DEBUG_EN
#define GCPU_DEBUG_LEVEL_FLOW     0
#define GCPU_DEBUG_LEVEL_PARAM    1
#define GCPU_DEBUG_LEVEL_SLOT     2
#define GCPU_DEBUG_LEVEL_BDRE     11
#define GCPU_DEBUG_LEVEL_LOOP     3
#define GCPU_DEBUG_LEVEL_BDROM    4
#define GCPU_DEBUG_LEVEL_CPPM     5
#define GCPU_DEBUG_LEVEL_CPRM     6
#define GCPU_DEBUG_LEVEL_CSS      7
#define GCPU_DEBUG_LEVEL_DIVX     8
#define GCPU_DEBUG_LEVEL_MAIN     9
#define GCPU_DEBUG_LEVEL_MARLIN   10
#define GCPU_DEBUG_LEVEL_MAX      32
#endif*/
/*! \name CSS command group
* @{
*/
/*!
 * @brief Cmd parameter structure for CSS_DEC_DK
 */
typedef struct _CSS_PARAM_DK {
	BYTE *pbRDK;                 /*!< [IN] reference disc key DRAM address. Len: 40 bits */
	BYTE *pbEDK;                 /*!< [IN] encrypted disc key DRAM address. Len: 40 bits */
} CSS_Param_DK;

/*!
 * @brief Cmd parameter structure for CSS_DEC_TK
 */
typedef struct _CSS_PARAM_TK {
	BYTE *pbETK;                 /*!< [IN] encrypted disc key DRAM address. Len: 40 bits */
} CSS_Param_TK;

/*!
 * @brief Cmd parameter structure for CSS_DSC_AV
 */
typedef struct _CSS_PARAM_AV {
	UINT32 u4SrcSa;                 /*!< [IN] Source DRAM Address */
	UINT32 u4DstSa;                 /*!< [IN] Destination DRAM Address */
	UINT32 u4DatLen;               /*!< [IN] Data Length (in unit of byte) */
	UINT8 uMode;                     /*!< [IN] Mode 0 / 1 / 2
						0: off: do not descramble any packet; only doing data moving
						1: on: descramble every packet
						2: auto: depending on PES_SCRAMBLE field of the packet */
} CSS_Param_AV;

/*!
 * @brief Cmd parameter structure for CSS_AUTH_DRV, CSS_AUTH_DEC, CSS_AUTH_BK
 */
typedef struct _CSS_PARAM_AUTH {
	BYTE *pbCD;       /*!< [IN] Challenge data / Concatenated response data DRAM address. Len: 80 bits */
	UINT32 u4ACC;            /*!< [IN] Authentication control code */
	BYTE *pbRD;               /*!< [OUT] Response data / bus key DRAM address. Len: 40 bits */
} CSS_Param_Auth;

#define GCPU_CSS_DEC_DK        \
	((UINT32)(CSS_CMD_SID + 0x00))  /* < CSS Disk Key Decryption, parameter struct is CSS_Param_DK */
#define GCPU_CSS_DEC_TK        \
	((UINT32)(CSS_CMD_SID + 0x01))  /* < CSS Title Key Decryption, parameter struct is CSS_Param_TK */
#define GCPU_CSS_DSC_AV        \
	((UINT32)(CSS_CMD_SID + 0x02))  /* < CSS AV Descramble, parameter struct is CSS_Param_AV */
#define GCPU_CSS_AUTH_DRV      \
	((UINT32)(CSS_CMD_SID + 0x03))  /* < CSS Drive Authentication, parameter struct is CSS_Param_Auth */
#define GCPU_CSS_AUTH_DEC      \
	((UINT32)(CSS_CMD_SID + 0x04))  /* < CSS Decoder Authentication, parameter struct is CSS_Param_Auth */
#define GCPU_CSS_AUTH_BK       \
	((UINT32)(CSS_CMD_SID + 0x05))  /* < CSS Key Share Authentication, parameter struct is CSS_Param_Auth */
/*! @} */

/*! \name CPPM_CPRM command group
* @{
*/

/*!
 * @brief Cmd parameter structure for C2_D
 *
 * Key value can use pbKey or u2KeySlotHandle according to uFlag
 * Data value can use pbData or u2DataSlotHandle according to uFlag
 * Result value can use pbResult or u2ResSlotHandle according to uFlag
 *
 * for C2_D and C2_G, if u2KeySlotHandle or u2DataSlotHandle is a secure slot,
 * u2ResSlotHandle must be a secure slot
 * for C2_E, if u2DataSlotHandle is a secure slot, u2ResSlotHandle must be a secure slot
 */
typedef struct _C2_PARAM_D {
	BYTE *pbKey;              /*!< [IN] Key value DRAM address. Len: 56 bits */
	BYTE *pbData;             /*!< [IN] Data value DRAM address. Len: 64 bits */
	BYTE *pbResult;           /*!< [OUT] Result DRAM address. Len: 64 bits */
	BYTE *pbCmpRes;           /*!< [OUT] Compare result. Len: 8 bits */
	BYTE *pbValue;            /*!< [OUT] value . Len: 32 bits */
	UINT16 u2KeySlotHandle;   /*!< [IN] Key Slot Handle. Slot size should be 64 bits. 0:0x00[55:32], 1:[31:0] */
	UINT16 u2DataSlotHandle;  /*!< [IN] Data Slot Handle. Slot size should be 64 bits. 0:[63:32], 1:[31:0] */
	UINT16 u2ResSlotHandle;   /*!< [OUT] Result Slot Handle. Slot size should be 64 bits. 0:[63:32], 1:[31:0] */
	UINT8 uFlag;    /*!< [IN] Slot handle flag.
			1: using slot handle, 0: using DRAM address. bit0: key, bit1: data, bit2: result */
} C2_Param_D;

/*!
 * @brief Cmd parameter structure for C2_E, C2_G
 *
 * Key value can use pbKey or u2KeySlotHandle according to uFlag
 * Data value can use pbData or u2DataSlotHandle according to uFlag
 * Result value can use pbResult or u2ResSlotHandle according to uFlag
 *
 * for C2_D and C2_G, if u2KeySlotHandle or u2DataSlotHandle is a secure slot,
 * u2ResSlotHandle must be a secure slot
 * for C2_E, if u2DataSlotHandle is a secure slot, u2ResSlotHandle must be a secure slot
 */
typedef struct _C2_PARAM_EG {
	BYTE *pbKey;             /*!< [IN] Key value DRAM address. Len: 56 bits */
	BYTE *pbData;            /*!< [IN] Data value DRAM address. Len: 64 bits */
	BYTE *pbResult;          /*!< [OUT] Result DRAM address. Len: 64 bits */
	UINT16 u2KeySlotHandle;  /*!< [IN] Key Slot Handle.
						Slot size should be 64 bits. 0:0x00[55:32], 1:[31:0] */
	UINT16 u2DataSlotHandle; /*!< [IN] Data Slot Handle.
						Slot size should be 64 bits. 0:[63:32], 1:[31:0] */
	UINT16 u2ResSlotHandle;  /*!< [OUT] Result Slot Handle.
						Slot size should be 64 bits. 0:[63:32], 1:[31:0] */
	UINT8 uFlag;   /*!< [IN] Slot handle flag.
		1: using slot handle, 0: using DRAM address. bit0: key, bit1: data, bit2: result */
} C2_Param_EG;

#if 0
/*!
 * @brief Cmd parameter structure for C2_D, C2_E, C2_G
 *
 * Key value can use pbKey or u2KeySlotHandle according to uFlag
 * Data value can use pbData or u2DataSlotHandle according to uFlag
 * Result value can use pbResult or u2ResSlotHandle according to uFlag
 *
 * for C2_D and C2_G, if u2KeySlotHandle or u2DataSlotHandle is a secure slot,
 * u2ResSlotHandle must be a secure slot
 * for C2_E, if u2DataSlotHandle is a secure slot, u2ResSlotHandle must be a secure slot
 */
typedef struct _C2_PARAM_DEG {
	BYTE *pbKey;             /*!< [IN] Key value DRAM address. Len: 56 bits */
	BYTE *pbData;            /*!< [IN] Data value DRAM address. Len: 64 bits */
	BYTE *pbResult;          /*!< [OUT] Result DRAM address. Len: 64 bits */
	UINT16 u2KeySlotHandle;  /*!< [IN] Key Slot Handle.
							Slot size should be 64 bits. 0:0x00[55:32], 1:[31:0] */
	UINT16 u2DataSlotHandle; /*!< [IN] Data Slot Handle.
							Slot size should be 64 bits. 0:[63:32], 1:[31:0] */
	UINT16 u2ResSlotHandle;  /*!< [OUT] Result Slot Handle.
							Slot size should be 64 bits. 0:[63:32], 1:[31:0] */
	UINT8 uFlag;    /*!< [IN] Slot handle flag.
				1: using slot handle, 0: using DRAM address. bit0: key, bit1: data, bit2: result */
} C2_Param_DEG;
#endif /* 0 */

/*!
 * @brief Cmd parameter structure for C2_H
 */
typedef struct _C2_PARAM_H {
	UINT32 u4SrcSa;       /*!< [IN] Source DRAM Address */
	UINT32 u4DatLen;      /*!< [IN] Data transfer length (in unit of byte) */
	UINT64 u8BitCnt;      /*!< [IN] Bit Count for previouse data,
						for first packet, it should be zero */
	BOOL   fgFirstPacket;  /*!< [IN] TRUE: The content contains the first packet.
						FALSE: The content does not contain the first packet. */
	BOOL   fgLastPacket; /*!< [IN] TRUE: The content contains the last packet.
						FALSE: The content does not contain the last packet. */
	BYTE  *pbIniHash;   /*!< [IN] Initial Hash value DRAM address. Len: 32 bits.
						It should not be set for first packet */
	BYTE  *pbResHash;   /*!< [OUT] Result Hash value DRAM address. Len: 32 bits */
	/*!< [OUT] Result Hash value DRAM address. Len: 160 bits */
} C2_Param_H;
#if 0
typedef struct _C2_PARAM_H {
	BYTE *pbIniHash;               /*!< [IN] Initial Hash value DRAM address. Len: 64 bits */
	BYTE *pbResHash;              /*!< [OUT] Result Hash value DRAM address. Len: 64 bits */
	UINT32 u4SrcSa;                 /*!< [IN] Source DRAM Address */
	UINT32 u4Len;                    /*!< [IN] Length (in unit of byte) */
	UINT8 uMode; /*!< [IN] Mode. Bit0: 1 (first packet)
				0 (not first packet, successive packet), Bit1: 1 (last packet) / 0 (not last packet)
				first packet: the content contains the first packet
				successive packet: the contents does not contain the first packet.
				The initial hash value has to be set */
} C2_Param_H;
#endif /* 0 */

/*!
 * @brief Cmd parameter structure for CPPM_DPAK, CPRM_DPAK, CPRM_EPAK
 *
 * Key value can use pbKey or u2KeySlotHandle according to uFlag
 */
typedef struct _CPPM_CPRM_PARAM {
	UINT32 u4SrcSa;     /*!< [IN] Source DRAM Address */
	UINT32 u4DstSa;     /*!< [IN] Destination DRAM Address */
	UINT32 u4Len;       /*!< [IN] Length (in unit of byte) */
	UINT32 u4APSTB;     /*!< [IN] APSTB value for the packet. Only for CPRM_DPAK, CPRM_EPAK */
	BYTE *pbKey;            /*!< [IN] Key value. Len: 56 bits */
	UINT16 u2KeySlotHandle; /*!< [IN] Key Slot Handle.
				Kau value [55:0] for packet with 0x00 padding. Slot size should be 64 bits */
	UINT8 uFlag;  /*!< [IN] Slot handle flag.
				1: using slot handle, 0: using DRAM address. bit0: key*/
	UINT8 uMode;  /*!< [IN] Mode 0 / 1 / 2
				0: off: do not descramble any packet; only doing data moving
				1: on: descramble every packet
				2: auto: depending on PES_SCRAMBLE field of the packet */
} CPPM_CPRM_Param;


/*!
 * @brief Cmd parameter structure for CPRM_DCICCI_VERIFY
 *
 * Key value can use pbKey or u2KeySlotHandle according to uFlag
 * Data value can use pbData or u2DataSlotHandle according to uFlag
 * DCICCI value can use pbDCICCI or u2DataSlotHandle according to uFlag
 * Result value can use pbResult or u2ResSlotHandle according to uFlag
 *
 */
typedef struct _CPRM_DCICCI_VERIFY_PARAM {
	BYTE *pbKey;                  /*!< [IN] Key value DRAM address. Len: 32 bits */
	BYTE *pbDCICCI;               /*!< [IN] DCI/CCI value DRAM address. Len: 32 bits */
	BYTE *pbData;                 /*!< [IN] Data value DRAM address. Len: 32 bits */
	BYTE *pbResult;               /*!< [OUT] Result DRAM address. Len: 32 bits */
	UINT16 u2KeySlotHandle;       /*!< [IN] Key Slot Handle. Slot size should be 32 bits.*/
	UINT16 u2DCICCISlotHandle;    /*!< [IN] DCI/CCI Slot Handle. Slot size should be 32 bits.*/
	UINT16 u2DataSlotHandle;      /*!< [IN] Data Slot Handle. Slot size should be 32 bits.*/
	UINT16 u2ResSlotHandle;       /*!< [OUT] Result Slot Handle. Slot size should be 32 bits.*/
	UINT8 uFlag;        /*!< [IN] Slot handle flag. 1: using slot handle, 0: using DRAM address.
						bit0: key, bit1: DCICCI, bit2: data, bit3: result*/
} CPRM_DCICCI_Verify_Param;

#define GCPU_C2_D          \
	((UINT32)(CPPM_CMD_SID + 0x00)) /* < CPPM/CPRM C2 Decryption, parameter struct is C2_Param_DEG */
#define GCPU_C2_E          \
	((UINT32)(CPPM_CMD_SID + 0x01)) /* < CPPM/CPRM C2 Encryption, parameter struct is C2_Param_DEG */
#define GCPU_C2_G          \
	((UINT32)(CPPM_CMD_SID + 0x02)) /* < CPPM/CPRM C2 Generating Function, parameter struct is C2_Param_DEG */
#define GCPU_C2_H          \
	((UINT32)(CPPM_CMD_SID + 0x03)) /* < CPPM/CPRM C2 Hash Function, parameter struct is C2_Param_H */
#define GCPU_CPPM_DPAK     \
	((UINT32)(CPPM_CMD_SID + 0x04)) /* < CPPM Packet Decryption, parameter struct is CPPM_CPRM_Param */
#define GCPU_CPRM_DPAK     \
	((UINT32)(CPPM_CMD_SID + 0x05)) /* < CPRM Packet Decryption, parameter struct is CPPM_CPRM_Param */
#define GCPU_CPRM_EPAK     \
	((UINT32)(CPPM_CMD_SID + 0x06)) /* < CPRM Packet Encryption, parameter struct is CPPM_CPRM_Param */
#define GCPU_CPRM_DCI_VFY  \
	((UINT32)(CPPM_CMD_SID + 0x07)) /* < CPRM DCI/CCI Verify, parameter struct is CPRM_DCICCI_Verify_Param */

/*! @} */

/*! \name AES command group
* @{
*/
/*!
 * @brief Cmd parameter structure for AES_D, AES_E, AES_G, AES_CMAC
 *
 * for AES_CMAC, key length must be 128-bit
 * for AES_D and AES_G, if u2KeySlotHandle or u2DataSlotHandle is a secure slot, u2ResSlotHandle must be a secure slot
 * for AES_E, if u2DataSlotHandle is a secure slot, u2ResSlotHandle must be a secure slot
 *
 * Key value can use pbKey or u2KeySlotHandle according to uFlag
 * Data value can use pbData or u2DataSlotHandle according to uFlag
 * Result value can use pbResult or u2ResSlotHandle according to uFlag
 */
typedef struct _AES_PARAM_DEG {
	BYTE *pbKey;            /*!< [IN] Key value DRAM address. Len: should be same with key length */
	BYTE *pbData;           /*!< [IN] Data value DRAM address. Len: 128 bits or 160 bits (CMAC 20 bytes Only) */
	BYTE *pbResult;         /*!< [OUT] Result DRAM address. Len: 128 bits */
	UINT16 u2KeySlotHandle; /*!< [IN] Key Slot Handle. Slot size should be same with key length */
	UINT16 u2DataSlotHandle;/*!< [IN] Data Slot Handle.
		Slot size should be 128 bits or 256 bits (Only for CMAC 20 Bytes Only) */
	UINT16 u2ResSlotHandle; /*!< [IN] Result Slot Handle.
		Slot size should be 128 bits or 256 bits (Only for CMAC 20 Bytes Only) */
	UINT8 uFlag;      /*!< [IN] Slot handle flag.
		1: using slot handle, 0: using DRAM address. bit0: key, bit1: data, bit2: result */
	UINT8 uKeyLen;    /*!< [IN] Key Length. 0: 128bit, 1: 192bit, 2: 256bit */
	UINT32 u4DatLen;  /*!< [IN] Data transfer length
		(in unit of byte, Only for CMAC, 128 bits or 160 bits) */
} AES_Param_DEG;


/*!
 * @brief Cmd parameter structure for AES_H
 *
 *  Key Length must be 0. 128 bit
 *
 * Key value can use pbKey or u2KeySlotHandle according to uFlag
 * Data value can use pbData or u2DataSlotHandle according to uFlag
 * Result value can use pbResult or u2ResSlotHandle according to uFlag
 */
typedef struct _AES_PARAM_H {
	BYTE *pbKey;            /*!< [IN] Key value DRAM address. Len: should be same with key length */
	BYTE *pbData;           /*!< [IN] Data value DRAM address. Len: 128 bits */
	BYTE *pbResult;         /*!< [OUT] Result DRAM address. Len: 128 bits */
	UINT16 u2KeySlotHandle; /*!< [IN] Key Slot Handle. Slot size should be same with key length */
	UINT16 u2DataSlotHandle;/*!< [IN] Data Slot Handle. Slot size should be 128 bits */
	UINT16 u2ResSlotHandle; /*!< [IN] Result Slot Handle. Slot size should be 128 bits */
	UINT8 uFlag;         /*!< [IN] Slot handle flag. 1: using slot handle,
						0: using DRAM address. bit0: key, bit1: data, bit2: result */
	UINT8 uKeyLen;       /*!< [IN] Key Length must be 0. 0: 128bit */
} AES_Param_H;



/*!
 * @brief Cmd parameter structure for AES_D_CMP
 *
 * AES key length must be 128-bit
 * Key value can use pbKey or u2KeySlotHandle according to uFlag
 * Data value can use pbData or u2DataSlotHandle according to uFlag
 * Result value can use pbResult or u2ResSlotHandle according to uFlag
 * if u2KeySlotHandle or u2DataSlotHandle is a secure slot, u2ResSlotHandle must be a secure slot
 */
typedef struct _AES_PARAM_D_CMP {
	BYTE *pbKey;            /*!< [IN] Key value DRAM address. Len: should be same with key length */
	BYTE *pbData;           /*!< [IN] Data value DRAM address. Len: 128 bits */
	BYTE *pbResult;         /*!< [OUT] Result DRAM address. Len: 128 bits */
	BYTE *pbCmpRes;         /*!< [OUT] Compare result. Len: 1 byte */
	BYTE *pbValue;          /*!< [OUT] value . Len: 64 bits */
	UINT16 u2KeySlotHandle; /*!< [IN] Key Slot Handle. Slot size should be same with key length */
	UINT16 u2DataSlotHandle;/*!< [IN] Data Slot Handle. Slot size should be 128 bits */
	UINT16 u2ResSlotHandle; /*!< [IN] Result Slot Handle. Slot size should be 128 bits */
	UINT8 uFlag;         /*!< [IN] Slot handle flag.
		1: using slot handle, 0: using DRAM address. bit0: key, bit1: data, bit2: result */
	UINT8 uKeyLen;       /*!< [IN] Key Length. 0: 128bit, 1: 192bit, 2: 256bit */
} AES_Param_D_CMP;

/*!
 * @brief Cmd parameter structure for AES_DPAK, AES_EPAK
 *
 * Key value can use pbKey or u2KeySlotHandle according to uFlag
 * for AES_DPAK, u2KeySlotHandle must not be a secure slot
 */
typedef struct _AES_PARAM_PAK {
	UINT32 u4SrcSa;                          /*!< [IN] Source DRAM Address */
	UINT32 u4DstSa;                          /*!< [IN] Destination DRAM Address */
	UINT32 u4DatLen;                        /*!< [IN] Data transfer length (in unit of byte) */
	BYTE *pbKey;          /*!< [IN] Key value DRAM address. Len: should be same with key length */
	UINT16 u2KeySlotHandle; /*!< [IN] Key Slot Handle. Slot size should be same with key length */
	UINT8 uFlag;     /*!< [IN] Slot handle flag. 1: using slot handle, 0: using DRAM address. bit0: key */
	UINT8 uKeyLen;   /*!< [IN] Key Length. 0: 128bit, 1: 192bit, 2: 256bit */
} AES_Param_PAK;

/*!
 * @brief Cmd parameter structure for AES_DCBC, AES_ECBC
 *
 * Key value can use pbKey or u2KeySlotHandle according to uFlag
 * IV value can use pbIV or u2IVSlotHandle according to uFlag
 * XOR value can use pbXOR or u2XORSlotHandle according to uFlag
 * u2IVSlotHandle and u2XORSlotHandle must not be a secure slot
 * for AES_DCBC, u2KeySlotHandle must not be a secure slot
 */
typedef struct _AES_PARAM_CBC {
	UINT32 u4SrcSa;         /*!< [IN] Source DRAM Address */
	UINT32 u4DstSa;         /*!< [IN] Destination DRAM Address */
	UINT32 u4DatLen;        /*!< [IN] Data transfer length (in unit of byte) */
	BYTE *pbKey;            /*!< [IN] Key value DRAM address. Len: should be same with key length */
	BYTE *pbIV;             /*!< [IN] Initial vector value DRAM address. Len: 128bits */
	BYTE *pbXOR;      /*!< [OUT] XOR data value DRAM address. Len: 128bits (Not support CBC CTS mode)*/
	UINT16 u2KeySlotHandle; /*!< [IN] Key Slot Handle. Slot size should be same with key length. */
	UINT16 u2IVSlotHandle;  /*!< [IN] Initial vector slot handle. Slot size should be 128-bit. */
	UINT16 u2XORSlotHandle; /*!< [OUT] XOR data slot handle. Feedback value for the next block.
		This can be set as the initial value of the next consecutive block. Slot size should be 128-bit. */
	UINT8 uFlag;     /*!< [IN] Slot handle flag. 1: using slot handle,
					0: using DRAM address. bit0: key, bit1: IV, bit2: XOR */
	UINT8 uKeyLen;   /*!< [IN] Key Length. 0: 128bit, 1: 192bit, 2: 256bit */
} AES_Param_CBC;

/*!
 * @brief Cmd parameter structure for AESPK_D, AESPK_E
 *
 * Key value can use pbKey or u2KeySlotHandle according to uFlag
 * Data value can use pbData or u2DataSlotHandle according to uFlag
 * Result value can use pbResult or u2ResSlotHandle according to uFlag
 * u2DataSlotHandle must not be a secure slot
 */
typedef struct _AESPK_PARAM_DE {
	BYTE *pbKey;             /*!< [IN] Key value DRAM address. Len: 128bits.
							Valid only when mode is 1. */
	BYTE *pbData;            /*!< [IN] Data value DRAM address. Len: 128bits */
	BYTE *pbResult;          /*!< [OUT] Result value DRAM address. Len: 128bits */
	UINT16 u2KeySlotHandle;  /*!< [IN] Key Slot Handle. Slot size should be 128 bits.
							Valid only when mode is 1 */
	UINT16 u2DataSlotHandle; /*!< [IN] Data Slot Handle. Slot size should be 128 bits */
	UINT16 u2ResSlotHandle;  /*!< [OUT] Result Slot Handle. Slot size should be 128 bits */
	UINT8 uFlag;    /*!< [IN] Slot handle flag.
		1: using slot handle, 0: using DRAM address. bit0: key, bit1: Data, bit2: Result */
	UINT8 uMode;    /*!< [IN] Key source selection
			0: Predetermined secret number
			1: Predetermined secret number and key
			2: Predetermined secret number and key*/
} AESPK_Param_DE;

/*!
 * @brief Cmd parameter structure for AESPK_DPAK, AESPK_EPAK
 *
 * Key value can use pbKey or u2KeySlotHandle according to uFlag
 */
typedef struct _AESPK_PARAM_PAK {
	UINT32 u4SrcSa;               /*!< [IN] Source DRAM Address */
	UINT32 u4DstSa;              /*!< [IN] Destination DRAM Address */
	UINT32 u4DatLen;            /*!< [IN] Data transfer length (in unit of byte) */
	BYTE *pbKey;               /*!< [IN] Key value DRAM address. Len: 128bits */
	UINT16 u2KeySlotHandle;   /*!< [IN] Key Slot Handle. Slot size should be 128 bits */
	UINT8 uFlag;        /*!< [IN] Slot handle flag.
						1: using slot handle, 0: using DRAM address. bit0: key */
	UINT8 uMode;       /*!< [IN] Key source selection
						0: Predetermined secret number
						1: Predetermined secret number and key */
} AESPK_Param_PAK;

/*!
 * @brief Cmd parameter structure for AESPK_DCBC, AESPK_ECBC
 *
 * Key value can use pbKey or u2KeySlotHandle according to uFlag
 * IV value can use pbIV or u2IVSlotHandle according to uFlag
 * XOR value can use pbXOR or u2XORSlotHandle according to uFlag
 * u2IVSlotHandle and u2XORSlotHandle must not be a secure slot
 */
typedef struct _AESPK_PARAM_CBC {
	UINT32 u4SrcSa;                          /*!< [IN] Source DRAM Address */
	UINT32 u4DstSa;                          /*!< [IN] Destination DRAM Address */
	UINT32 u4DatLen;        /*!< [IN] Data transfer length (in unit of byte) */
	BYTE *pbKey;            /*!< [IN] Key value DRAM address. Len: 128bits */
	BYTE *pbIV;             /*!< [IN] Initial vector value DRAM address. Len: 128bits */
	BYTE *pbXOR;            /*!< [OUT] XOR data value DRAM address. Len: 128bits */
	UINT16 u2KeySlotHandle; /*!< [IN] Key Slot Handle. Slot size should be 128 bits. */
	UINT16 u2IVSlotHandle;  /*!< [IN] Initial vector slot handle. Slot size should be 128-bit. */
	UINT16 u2XORSlotHandle; /*!< [OUT] XOR data slot handle. Feedback value for the next block.
			This can be set as the initial value of the next consecutive block.
			Slot size should be 128-bit. */
	UINT8 uFlag;   /*!< [IN] Slot handle flag.
			1: using slot handle, 0: using DRAM address. bit0: key, bit1: IV, bit2: XOR */
	UINT8 uMode;   /*!< [IN] Key source selection
				0: Predetermined secret number
				1: Predetermined secret number and key
				Note: Reserved for AESPK_EK_CBC*/
} AESPK_Param_CBC;

/*!
 * @brief Cmd parameter structure for AES_EKD, AES_EDE
 *
 * for AES_CMAC, key length must be 128-bit
 * for AES_D and AES_G, if u2KeySlotHandle or u2DataSlotHandle is a secure slot,
 * u2ResSlotHandle must be a secure slot
 * for AES_E, if u2DataSlotHandle is a secure slot, u2ResSlotHandle must be a secure slot
 *
 * Key value can use pbKey or u2KeySlotHandle according to uFlag
 * Data value can use pbData or u2DataSlotHandle according to uFlag
 * Result value can use pbResult or u2ResSlotHandle according to uFlag
 */
typedef struct _AES_PARAM_EK {
	BYTE *pbKey;             /*!< [IN] Key value DRAM address. Len: should be same with key length */
	BYTE *pbEncKey;          /*!< [IN] Enc Key value DRAM address. Len: should be same with key length */
	BYTE *pbData;            /*!< [IN] Data value DRAM address. Len: 128 bits */
	BYTE *pbResult;          /*!< [OUT] Result DRAM address. Len: 128 bits */
	UINT16 u2KeySlotHandle;  /*!< [IN] Key Slot Handle. Slot size should be same with key length
						 Note: For AESPK_EK_D, AESPK_EK_E, this should be sslot */
	UINT16 u2EncKeySlotHandle;  /*!< [IN] Encryption Key Slot Handle. Slot size should be same with key length */
	UINT16 u2DataSlotHandle;  /*!< [IN] Data Slot Handle. Slot size should be 128 bits */
	UINT16 u2ResSlotHandle;   /*!< [IN] Result Slot Handle. Slot size should be 128 bits */
	UINT8 uFlag;    /*!< [IN] Slot handle flag.
	1: using slot handle, 0: using DRAM address. bit0: key, bit1: Enc key, bit2: data, bit3: result */
} AES_Param_EK;


/*!
 * @brief Cmd parameter structure for AES_CTR
 *
 * Key value can use pbKey or u2KeySlotHandle according to uFlag
 * Data value can use pbData or u2DataSlotHandle according to uFlag
 * Result value can use pbResult or u2ResSlotHandle according to uFlag
 */
typedef struct _AES_PARAM_CTR {
	UINT32 u4SrcSa;                          /*!< [IN] Source DRAM Address */
	UINT32 u4DstSa;                          /*!< [IN] Destination DRAM Address */
	UINT32 u4DatLen;                        /*!< [IN] Data transfer length (in unit of byte) */
	BYTE *pbKey;            /*!< [IN] Key value DRAM address. Len: should be same with key length */
	BYTE *pbCtr;            /*!< [IN] Enc Key value DRAM address. Len: should be same with key length */
	BYTE *pbCtrResult;       /*!< [OUT] Ctr Output DRAM address */
	UINT16 u2KeySlotHandle; /*!< [IN] Key Slot Handle. Slot size should be same with key length
							Note: For AESPK_EK_D, AESPK_EK_E, this should be sslot */
	UINT16 u2CtrSlotHandle;  /*!< [IN] Encryption Key Slot Handle. Slot size should be same with key length */
	UINT16 u2CtrResultSlotHandle;   /*!< [IN] Data Slot Handle. Slot size should be 128 bits */
	UINT8 uFlag;     /*!< [IN] Slot handle flag.
			1: using slot handle, 0: using DRAM address. bit0: key, bit1: Ctr, bit2: Ctr Result */
	UINT8 uKeyLen;   /*!< [IN] Key Length. 0: 128bit, 1: 192bit, 2: 256bit */
	UINT64 u8Offset; /*!< [IN] Decrypted Data Length. */
} AES_Param_CTR;


/*!
 * @brief Cmd parameter structure for AES_CTR64
 *
 * Key value can use pbKey or u2KeySlotHandle according to uFlag
 * Data value can use pbData or u2DataSlotHandle according to uFlag
 * Result value can use pbResult or u2ResSlotHandle according to uFlag
 */
typedef struct _AES_PARAM_CTR64 {
	UINT32 u4SrcSa;                          /*!< [IN] Source DRAM Address */
	UINT32 u4DstSa;                          /*!< [IN] Destination DRAM Address */
	UINT32 u4DatLen;     /*!< [IN] Data transfer length (in unit of byte) */
	BYTE *pbKey;        /*!< [IN] Key value DRAM address. Len: should be same with key length */
	BYTE *pbCtr;        /*!< [IN] Enc Key value DRAM address. Len: should be same with key length */
	BYTE *pbCtrResult;  /*!< [OUT] Ctr Output DRAM address */
	UINT16 u2KeySlotHandle;  /*!< [IN] Key Slot Handle. Slot size should be same with key length
		Note: For AESPK_EK_D, AESPK_EK_E, this should be sslot */
	UINT16 u2CtrSlotHandle;   /*!< [IN] Encryption Key Slot Handle. Slot size should be same with key length */
	UINT16 u2CtrResultSlotHandle;   /*!< [IN] Data Slot Handle. Slot size should be 128 bits */
	UINT8 uFlag;     /*!< [IN] Slot handle flag.
			1: using slot handle, 0: using DRAM address. bit0: key, bit1: Ctr, bit2: Ctr Result */
	UINT8 uKeyLen;    /*!< [IN] Key Length. 0: 128bit, 1: 192bit, 2: 256bit */
	UINT64 u8Offset;	/*!< [IN] Decrypted Data Length. */
} AES_Param_CTR64;

/*!
 * @brief Cmd parameter structure for AES_WRAP
 *
 * Key value can use pbKey or u2KeySlotHandle according to uFlag
 * Data value can use pbData or u2DataSlotHandle according to uFlag
 * Result value can use pbResult or u2ResSlotHandle according to uFlag
 */
typedef struct _AES_PARAM_WRAP {
	BYTE *pbKey;             /*!< [IN] Key value DRAM address. Len: should be same with key length */
	BYTE *pbData;            /*!< [IN] Data value DRAM address. Noly support Data Len: 128 bits */
	UINT32 u4DatLen;         /*!< [IN] Data transfer length (in unit of byte) */
	BYTE *pbResult;          /*!< [OUT] Result DRAM address. Len: 128 bits */
	UINT16 u2KeySlotHandle;  /*!< [IN] Key Slot Handle. Slot size should be same with key length */
	UINT16 u2DataSlotHandle; /*!< [IN] Data Slot Handle. Slot size should be 128 bits */
	UINT16 u2ResSlotHandle;  /*!< [IN] Result Slot Handle. Slot size should be 128 bits */
	UINT8 uFlag;           /*!< [IN] Slot handle flag.
				1: using slot handle, 0: using DRAM address. bit0: key, bit1: data, bit2: result */
	UINT8 uKeyLen;         /*!< [IN] Key Length. 0: 128bit, 1: 192bit, 2: 256bit */
} AES_Param_WRAP;


#define GCPU_AES_D                  \
	((UINT32)(AES_CMD_SID + 0x00))    /* < AES Decryption Function, parameter struct is AES_Param_DEG */
#define GCPU_AES_E                  \
	((UINT32)(AES_CMD_SID + 0x01))    /* < AES Encryption Function, parameter struct is AES_Param_DEG */
#define GCPU_AES_G                  \
	((UINT32)(AES_CMD_SID + 0x02))    /* < AES Generating Function, parameter struct is AES_Param_DEG */
#define GCPU_AES_DPAK               \
	((UINT32)(AES_CMD_SID + 0x03))    /* < AES Packet Decryption, parameter struct is AES_PARAM_PAK */
#define GCPU_AES_EPAK               \
	((UINT32)(AES_CMD_SID + 0x04))    /* < AES Packet Encryption, parameter struct is AES_PARAM_PAK */
#define GCPU_AES_CMAC               \
	((UINT32)(AES_CMD_SID + 0x05))    /* < AES CMAC Algorithm, parameter struct is AES_Param_DEG */
#define GCPU_AES_DCBC               \
	((UINT32)(AES_CMD_SID + 0x06))    /* < AES Cipher Block Chaining Decryption, parameter struct
										is AES_Param_CBC */
#define GCPU_AES_ECBC               \
	((UINT32)(AES_CMD_SID + 0x07))    /* < AES Cipher Block Chaining Encryption, parameter struct
										is AES_Param_CBC */
#define GCPU_AES_D_CMP              \
	((UINT32)(AES_CMD_SID + 0x08))    /* < AES Decryption Function with Result Compare,
										parameter struct is AES_Param_DEG */
#define GCPU_AESPK_D                \
	((UINT32)(AES_CMD_SID + 0x09))    /* < AES Decryption with Predetermined Key,
										parameter struct is AESPK_Param_DE */
#define GCPU_AESPK_E                \
	((UINT32)(AES_CMD_SID + 0x0A))    /* < AES Encryption with Predetermined Key,
										parameter struct is AESPK_Param_DE */
#define GCPU_AESPK_DPAK             \
	((UINT32)(AES_CMD_SID + 0x0B))    /* < AES Packet Decryption with Predetermined Key,
										parameter struct is AESPK_PARAM_PAK */
#define GCPU_AESPK_EPAK             \
	((UINT32)(AES_CMD_SID + 0x0C))    /* < AES Packet Encryption with Predetermined Key,
										parameter struct is AESPK_PARAM_PAK */
#define GCPU_AESPK_DCBC             \
	((UINT32)(AES_CMD_SID + 0x0D))    /* < AES Cipher Block Chaining Decryption with Predetermined Key,
										parameter struct is AESPK_Param_CBC */
#define GCPU_AESPK_ECBC             \
	((UINT32)(AES_CMD_SID + 0x0E))    /* < AES Cipher Block Chaining Encryption with Predetermined Key,
										parameter struct is AESPK_Param_CBC */
#define GCPU_AESEK_D                \
	((UINT32)(AES_CMD_SID + 0x0F))    /* < AES Decryption with Encrypted Key, parameter struct is AESPK_Param_EK */
#define GCPU_AESEK_E                \
	((UINT32)(AES_CMD_SID + 0x10))    /* < AES Encryption with Encrypted Key, parameter struct is AESPK_Param_EK */
#define GCPU_AESPK_EK_D             \
	((UINT32)(AES_CMD_SID + 0x11))    /* < AES Decryption with Predetermined/Encrypted Key,
										parameter struct is AESPK_Param_PK_EK */
#define GCPU_AESPK_EK_E             \
	((UINT32)(AES_CMD_SID + 0x12))    /* < AES Encryption with Predetermined/Encrypted Key,
										parameter struct is AESPK_Param_PK_EK */
#define GCPU_AESPK_EK_DCBC          \
	((UINT32)(AES_CMD_SID + 0x13))    /* < AES Cipher Block Chaining Decryption
									with Predetermined/Encryption Key, */
/* < parameter struct is AESPK_Param_PKEK_CBC */
#define GCPU_AES_H                  \
	((UINT32)(AES_CMD_SID + 0x14))    /* < AES Hash Function, parameter struct is AES_Param_H */
#define GCPU_AES_CTR                \
	((UINT32)(AES_CMD_SID + 0x15))    /* < AES Counter Mode Decryption Function,
										parameter struct is AES_Param_CTR */
#define GCPU_AES_OFB                \
	((UINT32)(AES_CMD_SID + 0x16))    /* < AES Output Feedback Mode Decryption Function,
										parameter struct is AES_Param_OFB */

#define GCPU_AES_WRAPD              \
	((UINT32)(AES_CMD_SID + 0x17))    /* < AES WRAP Mode Decryption Function, parameter struct is ?? */
#define GCPU_AES_WRAPE              \
	((UINT32)(AES_CMD_SID + 0x18))    /* < AES WRAP Mode Decryption Function, parameter struct is ?? */

#define GCPU_AES_DCBC_CTS           \
	((UINT32)(AES_CMD_SID + 0x19))    /* < AES Cipher Block Chaining Decryption,
							parameter struct is AES_Param_CBC */
#define GCPU_AES_ECBC_CTS           \
	((UINT32)(AES_CMD_SID + 0x1a))    /* < AES Cipher Block Chaining Decryption,
							parameter struct is AES_Param_CBC */

#define GCPU_AES_CTR64              \
	((UINT32)(AES_CMD_SID + 0x1b))    /* < AES 64 bits Counter Mode Decryption Function,
									parameter struct is AES_Param_CTR64 */
#define GCPU_AES_CTR128             \
	((UINT32)(AES_CMD_SID + 0x1c))    /* < AES Counter Mode Decryption Function,
									parameter struct is AES_Param_CTR */

#define GCPU_AES_DPAK_ONETZ         \
	((UINT32)(AES_CMD_SID + 0x21))    /* < AES Packet Decryption, parameter struct is AES_PARAM_PAK,
							use one tz service call, performance better for small dec unit,
							around 1024 bytes */
#define GCPU_AES_EPAK_ONETZ         \
	((UINT32)(AES_CMD_SID + 0x22))    /* < AES Packet Encryption, parameter struct is AES_PARAM_PAK,
							use one tz service call, performance better for small dec unit,
							around 1024 bytes */

#define GCPU_AES_DCBC_ONETZ         \
	((UINT32)(AES_CMD_SID + 0x23))    /* < AES Cipher Block Chaining Decryption,
							parameter struct is AES_Param_CBC, use one tz service call,
							performance better for small dec unit, around 1024 bytes */
#define GCPU_AES_ECBC_ONETZ         \
	((UINT32)(AES_CMD_SID + 0x24))    /* < AES Cipher Block Chaining Encryption,
							parameter struct is AES_Param_CBC, use one tz service call,
							performance better for small dec unit, around 1024 bytes */

/*! @} */

/*! \name VCPS command group
* @{
*/
/*!
 * @brief Cmd parameter structure for VCPS_H
 *
 * Key value can use pbKey or u2KeySlotHandle according to uFlag
 * Data value can use pbData or u2DataSlotHandle according to uFlag
 * Result value can use pbResult or u2ResSlotHandle according to uFlag
 */
typedef struct _VCPS_PARAM_H {
	BYTE *pbKey;                     /*!< [IN] Key value DRAM address. Len: 128bits. */
	BYTE *pbData;                    /*!< [IN] Data value DRAM address. Len: 128bits */
	BYTE *pbResult;                 /*!< [OUT] Result value DRAM address. Len: 128bits */
	UINT16 u2KeySlotHandle;      /*!< [IN] Key Slot Handle. Slot size shoule be 128-bits. */
	UINT16 u2DatSlotHandle;      /*!< [IN] Data Slot Handle. Slot size shoule be 128-bits. */
	UINT16 u2ResSlotHandle;      /*!< [OUT] Result Slot Handle. Slot size shoule be 128-bits. */
	UINT8 uFlag;   /*!< [IN] Slot handle flag.
				1: using slot handle, 0: using DRAM address. bit0: key, bit1: Data, bit2: Result */
} VCPS_Param_H;

/*!
 * @brief Cmd parameter structure for VCPS_DPAK, VCPS_EPAK
 *
 * Kp value can use pbKP or u2KPSlotHandle according to uFlag
 */
typedef struct _VCPS_PARAM_PAK {
	UINT32 u4SrcSa;                 /*!< [IN] Source DRAM Address */
	UINT32 u4DstSa;                 /*!< [IN] Destination DRAM Address */
	UINT32 u4DatLen;              /*!< [IN] Data Length (in unit of byte) */
	BYTE *pbKP;                     /*!< [IN] Kp value DRAM address. Len: 128bits. */
	UINT16 u2KPSlotHandle;        /*!< [IN] Kp Slot Handle. Slot size should be 128 bits. */
	UINT8 uFlag;   /*!< [IN] Slot handle flag. 1: using slot handle, 0: using DRAM address. bit0: kp */
	UINT8 uMode;   /*!< [IN] Mode 0 / 1 / 2
0: off: do not descramble any packet; only doing data moving
1: on: descramble every packet
2: auto: depending on PES_SCRAMBLE field of the packet */
} VCPS_Param_PAK;

/*!
 * @brief Cmd parameter structure for VCPS_DKBH
 */
typedef struct _VCPS_PARAM_DKBH {
	UINT32 u4SrcSa;                 /*!< [IN] Source DRAM Address */
	UINT32 u4Len;                    /*!< [IN] Length (in unit of byte) */
	BYTE *pbIniHash;               /*!< [IN] Initial Hash value DRAM address. Len: 128 bits */
	BYTE *pbResHash;              /*!< [OUT] Result Hash value DRAM address. Len: 128 bits */
	UINT8 uMode;            /*!< [IN] Mode. Bit0: 1 (first packet)
/0 (not first packet, successive packet), Bit1: 1 (last packet) / 0 (not last packet)
first packet: the content contains the first packet
successive packet: the contents does not contain the first packet. The initial hash value has to be set */
} VCPS_Param_DKBH;

/*!
 * @brief Cmd parameter structure for VCPS_DHDK
 *
 * Data value can use pbData or u2DataSlotHandle according to uFlag
 * Result value can use pbResult or u2ResSlotHandle according to uFlag
 * u2ResSlotHandle must be a secure slot
 */
typedef struct _VCPS_PARAM_DHDK {
	BYTE *pbData;                    /*!< [IN] Data value DRAM address. Len: 128bits */
	BYTE *pbResult;                 /*!< [OUT] Result value DRAM address. Len: 128bits */
	UINT16 u2DatSlotHandle;      /*!< [IN] Data Slot Handle. Slot size shoule be 128-bits. */
	UINT16 u2ResSlotHandle;      /*!< [OUT] Result Slot Handle. Slot size shoule be 128-bits. */
	UINT8 uFlag;       /*!< [IN] Slot handle flag.
					1: using slot handle, 0: using DRAM address. bit0: data, bit1: result */
} VCPS_Param_DHDK;

/*!
 * @brief Cmd parameter structure for VCPS_DCBC, VCPS_ECBC
 */
typedef struct _VCPS_PARAM_CBC {
	BYTE *pbData;          /*!< [IN] Data value DRAM address. Len: 256bits */
	BYTE *pbKey;            /*!< [IN] Key value DRAM address. Len: 128bits */
	BYTE *pbResult;        /*!< [OUT] Result value DRAM address. Len: 256bits */
} VCPS_Param_CBC;

#define GCPU_VCPS_H                 \
	((UINT32)(VCPS_CMD_SID + 0x00))   /* < VCPS Hash Function, parameter struct is VCPS_Param_H */
#define GCPU_VCPS_DPAK              \
	((UINT32)(VCPS_CMD_SID + 0x01))   /* < VCPS Packet Decryption, parameter struct is VCPS_Param_PAK */
#define GCPU_VCPS_EPAK              \
	((UINT32)(VCPS_CMD_SID + 0x02))   /* < VCPS Packet Encryption, parameter struct is VCPS_Param_PAK */
#define GCPU_VCPS_DKBH              \
	((UINT32)(VCPS_CMD_SID + 0x03))   /* < VCPS DKB Hash Function, parameter struct is VCPS_Param_DKBH */
#define GCPU_VCPS_DHDK              \
	((UINT32)(VCPS_CMD_SID + 0x04))   /* < VCPS Hardware Device Key Decryption,
										parameter struct is VCPS_Param_DHDK */
#define GCPU_VCPS_DCBC              \
	((UINT32)(VCPS_CMD_SID + 0x05))   /* < VCPS AES Cipher Block Chaining Decryption,
									parameter struct is VCPS_Param_CBC */
#define GCPU_VCPS_ECBC              \
	((UINT32)(VCPS_CMD_SID + 0x06))   /* < VCPS AES Cipher Block Chaining Encryption,
									parameter struct is VCPS_Param_CBC */
/*! @} */

/*! \name AACS command group
* @{
*/
/*!
 * @brief Cmd parameter structure for AACS_DBD, AACS_EBD, AACS_DTN, AACS_ETN
 *
 * Uk value can use pbUK or u2UKSlotHandle according to uFlag
 */
typedef struct _AACS_PARAM_BLURAY {
	UINT32 u4SrcSa;                 /*!< [IN] Source DRAM Address */
	UINT32 u4DstSa;                 /*!< [IN] Destination DRAM Address */
	UINT32 u4DatLen;               /*!< [IN] Data Length (in unit of byte) */
	BYTE *pbUK;                       /*!< [IN] Uk value DRAM address. Len: 128bits */
	UINT16 u2UKSlotHandle;        /*!< [IN] Uk Slot Handle, Uk value for packet. Key size should be 128 bits */
	UINT8 uFlag;           /*!< [IN] Slot handle flag. 1: using slot handle, 0: using DRAM address. bit0: Uk */
} AACS_Param_Bluray;

/*!
 * @brief Cmd parameter structure for AACS_DHD, AACS_EHD
 *
 * KT value can use pbKT or u2KTSlotHandle according to uFlag
 */
typedef struct _AACS_PARAM_HDDVD {
	UINT32 u4SrcSa;                 /*!< [IN] Source DRAM Address */
	UINT32 u4DstSa;                 /*!< [IN] Destination DRAM Address */
	UINT32 u4DatLen;               /*!< [IN] Data Length (in unit of byte) */
	BYTE *pbCPI;                 /*!< [IN] CPI value DRAM address. CPI value for packet. Len: 96bits */
	BYTE *pbKT;                  /*!< [IN] KT value DRAM address. Len: 128bits */
	UINT16 u2KTSlotHandle; /*!< [IN] KT Slot Handle, KT value for packet. Key size should be 128 bits */
	UINT8 uFlag;       /*!< [IN] Slot handle flag. 1: using slot handle, 0: using DRAM address. bit0: KT */
	UINT8 uMode;       /*!< [IN] Mode 0 / 1 / 2
						0: off: do not descramble any packet; only doing data moving
						1: on: descramble every packet
						2: auto: depending on PES_SCRAMBLE field of the packet */
} AACS_Param_HDDVD;

/*!
 * @brief Cmd parameter structure for AACS_DV_CALC
 *
 * XFCR value can use pbXFCR or u2XFCRSlotHandle according to uFlag
 * DKDER value can use pbDKDER or u2DKDERSlotHandle according to uFlag
 * u2XFCRSlotHandle and u2DKDERSlotHandle must not be a secure slot
 * u2KMSISlotHandle and u2DVSlotHandle must be secure slot
 */
typedef struct _AACS_PARAM_DVCALC {
	BYTE *pbXFCR;                    /*!< [IN] XFCR value DRAM address. Len: 128bits */
	BYTE *pbDKDER;                 /*!< [IN] DKDER value DRAM address. Len: 80bits */
	UINT32 u4DVLSB10;            /*!< [OUT] The least significant 10 bits of DV */
	BOOL fgCMP;         /*!< [OUT] TRUE: DVold equal to 0x00000000000000000000.
					FALSE: DV do not equal to 0x00000000000000000000 */
	UINT16 u2KMSISlotHandle;      /*!< [IN] KMSI Secure Slot Handle. Slot size should be 128 bits */
	UINT16 u2XFCRSlotHandle;      /*!< [IN] XFCR Slot Handle. Slot size should be 128 bits */
	UINT16 u2DKDERSlotHandle; /*!< [IN] DKDER Slot Handle. Slot size should be 96 bits.
							0: [79:48], 1:[47:16], 2:[15:0],0x0000 */
	UINT16 u2DVSlotHandle; /*!< [IN] DV Secure Slot Handle.
						Slot size should be 96 bits. 0: [79:48], 1:[47:16], 2:[15:0],0x0000 */
	UINT8 uFlag;           /*!< [IN] Slot handle flag. 1: using slot handle,
							0: using DRAM address. bit0: XFCR, bit1: DKDER */
	UINT8 uMode;   /*!< [IN] Mode
			Bit0: 0: DVold not exist. 1: DVold exists
			Bit1: 0: Do not concatenate the result with 0x041826fa7749.
			1: Concatenate the result with 0x041826fa7749 */
} AACS_Param_DVCALC;

#define GCPU_AACS_DBD               \
	((UINT32)(AACS_CMD_SID + 0x00))   /* < AACS Blu-ray AV Packet Decryption,
										parameter struct is AACS_Param_Bluray */
#define GCPU_AACS_EBD               \
	((UINT32)(AACS_CMD_SID + 0x01))   /* < AACS Blu-ray AV Packet Encryption,
										parameter struct is AACS_Param_Bluray */
#define GCPU_AACS_DTN               \
	((UINT32)(AACS_CMD_SID + 0x02))	/* < AACS Blu-ray Thumbnail Packet Decryption,
										parameter struct is AACS_Param_Bluray */
#define GCPU_AACS_ETN               \
	((UINT32)(AACS_CMD_SID + 0x03))	/* < AACS Blu-ray Thumbnail Packet Encryption,
										parameter struct is AACS_Param_Bluray */
#define GCPU_AACS_DHD               \
	((UINT32)(AACS_CMD_SID + 0x04))   /* < AACS HD-DVD Packet Decryption, parameter struct is AACS_Param_HDDVD */
#define GCPU_AACS_EHD               \
	((UINT32)(AACS_CMD_SID + 0x05))   /* < AACS HD-DVD Packet Encryption, parameter struct is AACS_Param_HDDVD */
#define GCPU_AACS_DV_CALC           \
	((UINT32)(AACS_CMD_SID + 0x06))   /* < AACS DV Calculation, parameter struct is AACS_Param_DVCALC */
/*! @} */

/*! \name TDES command group
* @{
*/
/*!
 * @brief Cmd parameter structure for TDES_D, TDES_E
 *
 * Key value can use pbKey or u2KeySlotHandle according to uFlag
 * Data value can use pbData or u2DataSlotHandle according to uFlag
 * Result value can use pbResult or u2ResSlotHandle according to uFlag
 * for TDES_D, if u2KeySlotHandle or u2DataSlotHandle is a secure slot, u2ResSlotHandle must be a secure slot
 * for TDES_E, if u2DataSlotHandle is a secure slot, u2ResSlotHandle must be a secure slot
 */
typedef struct _TDES_PARAM_DE {
	BYTE *pbKey;                     /*!< [IN] Key value DRAM address. Len: same with key length */
	BYTE *pbData;                   /*!< [IN] Data value DRAM address. Len: 64bits */
	BYTE *pbResult;                 /*!< [OUT] Result value DRAM address. Len: 64bits */
	UINT16 u2KeySlotHandle;      /*!< [IN] Key Slot Handle. Slot size should be same with key length*/
	UINT16 u2DatSlotHandle;      /*!< [IN] Data Slot Handle. Slot size should be 64 bits. */
	UINT16 u2ResSlotHandle;      /*!< [OUT] Result Slot Handle. Slot size should be 64 bits. */
	UINT8 uFlag;              /*!< [IN] Slot handle flag.
	1: using slot handle, 0: using DRAM address. bit0: key, bit1: data, bit2: result */
	UINT8 uKeyLen;                  /*!< [IN] Key length. 0:64bits, 1:128bits, 2:192bits, 3:40bits */
} TDES_Param_DE;

/*!
 * @brief Cmd parameter structure for TDES_DMA_D, TDES_DMA_E
 *
 * Key value can use pbKey or u2KeySlotHandle according to uFlag
 * for TDES_DMA_D, u2KeySlotHandle must not be a secure slot
 */
typedef struct _TDES_PARAM_DMA_DE {
	UINT32 u4SrcSa;                 /*!< [IN] Source DRAM Address */
	UINT32 u4DstSa;                 /*!< [IN] Destination DRAM Address */
	UINT32 u4Len;                     /*!< [IN] data length (in unit of byte) */
	BYTE *pbKey;                     /*!< [IN] Key value DRAM address. Len: same with key length */
	UINT16 u2KeySlotHandle;      /*!< [IN] Key Slot Handle. Slot size should be same with key length*/
	UINT8 uKeyLen;                  /*!< [IN] Key length. 0:64bits, 1:128bits, 2:192bits, 3:40bits */
	UINT8 uFlag;       /*!< [IN] Slot handle flag. 1: using slot handle, 0: using DRAM address. bit0: key */
} TDES_Param_DMA_DE;

/*!
 * @brief Cmd parameter structure for TDES_CBC_D, TDES_CBC_E
 *
 * Key value can use pbKey or u2KeySlotHandle according to uFlag
 * for TDES_CBC_D, u2KeySlotHandle must not be a secure slot
 */
typedef struct _TDES_PARAM_CBC_DE {
	UINT32 u4SrcSa;       /*!< [IN] Source DRAM Address */
	UINT32 u4DstSa;       /*!< [IN] Destination DRAM Address */
	UINT32 u4Len;         /*!< [IN] data length (in unit of byte) */
	BYTE *pbKey;          /*!< [IN] Key value DRAM address. Len: same with key length */
	BYTE *pbIV;           /*!< [IN] initial vector for CBC. Len: 64bits */
	BYTE *pbFB;           /*!< [OUT] Feedback value for the next block.
	This can be set as the initial value of the next consecutive block. Len: 64bits*/
	UINT16 u2KeySlotHandle;  /*!< [IN] Key Slot Handle. Slot size should be same with key length*/
	UINT8 uKeyLen;        /*!< [IN] Key length. 0:64bits, 1:128bits, 2:192bits, 3:40bits */
	UINT8 uFlag;          /*!< [IN] Slot handle flag. 1: using slot handle, 0: using DRAM address. bit0: key */
} TDES_Param_CBC_DE;

#define GCPU_TDES_D                   \
	((UINT32)(TDES_CMD_SID + 0x00))         /* < T-DES Decryption, parameter struct is TDES_Param_DE */
#define GCPU_TDES_E                   \
	((UINT32)(TDES_CMD_SID + 0x01))         /* < T-DES Encryption, parameter struct is TDES_Param_DE */
#define GCPU_TDES_DMA_D               \
	((UINT32)(TDES_CMD_SID + 0x02))         /* < T-DES DMA Decryption, parameter struct is TDES_Param_DMA_DE */
#define GCPU_TDES_DMA_E               \
	((UINT32)(TDES_CMD_SID + 0x03))         /* < T-DES DMA Encryption, parameter struct is TDES_Param_DMA_DE */
#define GCPU_TDES_CBC_D               \
	((UINT32)(TDES_CMD_SID + 0x04)) /* < T-DES Cipher Block Chaining Decryption,
									parameter struct is TDES_Param_CBC_DE */
#define GCPU_TDES_CBC_E               \
	((UINT32)(TDES_CMD_SID + 0x05)) /* < T-DES Cipher Block Chaining Encryption,
									parameter struct is TDES_Param_CBC_DE */
/*! @} */

/*! \name BDRE command group
* @{
*/
/*!
 * @brief Cmd parameter structure for BDRE_DBD, BDRE_EBD, BDRE_DTN, BDRE_ETN
 *
 * KREC value can use pbKREC or u2KRECSlotHandle according to uFlag
 */
typedef struct _BDRE_PARAM {
	UINT32 u4SrcSa;                 /*!< [IN] Source DRAM Address */
	UINT32 u4DstSa;                 /*!< [IN] Destination DRAM Address */
	UINT32 u4DatLen;               /*!< [IN] Data length (in unit of byte) */
	UINT32 u4VBKG;                   /*!< [IN] VBKG value */
	BYTE *pbKREC;                     /*!< [IN] KREC value DRAM address. Len: 128bits */
	UINT16 u2KRECSlotHandle;      /*!< [IN] KREC Slot Handle. Slot size should be 128bits. */
	UINT8 uFlag;           /*!< [IN] Slot handle flag. 1: using slot handle, 0: using DRAM address. bit0: krec */
} BDRE_Param;

/*!
 * @brief Cmd parameter structure for BDRE_BE
 *
 * Data value can use pbData or u2DatSlotHandle according to uFlag
 */
typedef struct _BDRE_PARAM_BE {
	BYTE *pbData;                    /*!< [IN] Data value DRAM address. Len: 64bits */
	UINT16 u2DatSlotHandle;       /*!< [IN] Data Slot Handle. Slot size should be 64bits. */
	UINT8 uFlag;              /*!< [IN] Slot handle flag. 1: using slot handle, 0: using DRAM address. bit0: data */
} BDRE_Param_BE;

#define GCPU_BDRE_DBD                    \
	((UINT32)(BDRE_CMD_SID + 0x00))         /* < BDRE AV Packet Decryption, parameter struct is BDRE_Param */
#define GCPU_BDRE_EBD                    \
	((UINT32)(BDRE_CMD_SID + 0x01))         /* < BDRE AV Packet Encryption, parameter struct is BDRE_Param */
#define GCPU_BDRE_DTN                    \
	((UINT32)(BDRE_CMD_SID + 0x02))         /* < BDRE Thumbnail Packet Decryption, parameter struct is BDRE_Param */
#define GCPU_BDRE_ETN                    \
	((UINT32)(BDRE_CMD_SID + 0x03))         /* < BDRE Thumbnail Packet Encryption, parameter struct is BDRE_Param */
#define GCPU_BDRE_BE                      \
	((UINT32)(BDRE_CMD_SID + 0x04))         /* < BDRE BytePerm and ExtendKey, parameter struct is BDRE_Param_BE */
/*! @} */

/*! \name E_FUSE command group
* @{
*/
/*!
 * @brief Cmd parameter structure for EF_PGM_BT_WR
 */
typedef struct _EF_PARAM_PGM_BT {
	UINT32 u4ADR;               /*!< [IN] E-Fuse address */
	UINT8 uData;                  /*!< [IN] Data to be write. */
	UINT8 uResult;                /*!< [OUT] Result */
} EF_Param_PGM_BT;


/*!
 * @brief Cmd parameter structure for EF_PGM_DW_WR
 */
typedef struct _EF_PARAM_PGM_DWEX {
	UINT32 u4ADR;                          /*!< [IN] E-Fuse address (Unit: 4 Byte) */
	UINT16 u2SrcSlotHandle;            /*!< [IN] Source slot handle. Slot size should be same with length */
	UINT16 u2SrcLen;                        /*! < [IN] Source Data Length (Unit: Byte)*/
} EF_Param_PGM_DWEX;

/*!
 * @brief Cmd parameter structure for EF_PGM_DW_WR
 */
typedef struct _EF_PARAM_PGM_DW {
	UINT32 u4ADR;                          /*!< [IN] E-Fuse address */
	UINT32 u4Data;                         /*!< [IN] Data to be write. */
} EF_Param_PGM_DW;

/*!
 * @brief Cmd parameter structure for EF_RD
 */
typedef struct _EF_PARAM_RD {
	UINT32 u4ADR;                /*!< [IN] E-Fuse address */
	UINT8 uResult;                /*!< [OUT] Result */
} EF_Param_RD;

/*!
 * @brief Cmd parameter structure for LD_EF_KEY
 */
typedef struct _EF_PARAM_LDKEY {
	UINT32 u4ADR;                          /*!< [IN] E-Fuse address (in unit of double word) */
	UINT32 u4Len;                           /*!< [IN] Length (in unit of double word) */
	UINT16 u2DstSlotHandle;            /*!< [IN] Target slot handle. Slot size should be same with length */
	UINT32 u4Result;
} EF_Param_LDKEY;

#define GCPU_EF_INI_VFY                 \
	((UINT32)(EFUSE_CMD_SID + 0x00))       /* < E_Fuse Initial Verify, parameter struct is NULL */
#define GCPU_EF_PGM_INI                \
	((UINT32)(EFUSE_CMD_SID + 0x01))       /* < E_Fuse Programming Initialization, parameter struct is NULL */
#define GCPU_EF_PGM_BT_WR          \
	((UINT32)(EFUSE_CMD_SID + 0x02))   /* < E_Fuse Programming Byte Write,
										parameter struct is EF_Param_PGM_BT */
#define GCPU_EF_PGM_DW_WR         \
	((UINT32)(EFUSE_CMD_SID + 0x03))    /* < E_Fuse Programming Double Word Write,
parameter struct is EF_Param_PGM_DW */
#define GCPU_EF_PGM_VFY               \
	((UINT32)(EFUSE_CMD_SID + 0x04))       /* < E_Fuse Programming Verify, parameter struct is NULL */
#define GCPU_EF_RD                    \
	((UINT32)(EFUSE_CMD_SID + 0x05))       /* < E_Fuse Read, parameter struct is EF_Param_RD */
#define GCPU_EF_EO_INI                 \
	((UINT32)(EFUSE_CMD_SID + 0x06))       /* < E_Fuse Option Initialization, parameter struct is NULL */
#define GCPU_LD_EF_KEY                  \
	((UINT32)(EFUSE_CMD_SID + 0x07))       /* < Load E_Fuse Key, parameter struct is EF_Param_LDKEY */
#define GCPU_EF_PGM_DW_WREX          ((UINT32)(EFUSE_CMD_SID + 0x08))
#define GCPU_EF_ZTE_IN_KEY          ((UINT32)(EFUSE_CMD_SID + 0x20))
#define GCPU_EF_ZTE_CMAC_KEY          ((UINT32)(EFUSE_CMD_SID + 0x21))
/*! @} */

/*! \name Misc command group
* @{
*/

/*!
* @brief Cmd parameter structure for TSDESC_VUDU
*/
#define GCPU_SUPPORT_TSDESC_VUDU 1
#define GCPU_SUPPORT_TSDESC_VUDU_TEST 0 /* Read Vudu Golden Sample for test hw support ts decrypt or not */
#if GCPU_SUPPORT_TSDESC_VUDU
typedef struct _TSDESC_PARAM {
	UINT32 u4DatLen;                        /*!< [IN] Data transfer length (in unit of byte) */
	UINT32 u4SrcSa;                         /*!< [IN] Source DRAM Start Address */
	UINT32 u4SrcEa;                         /*!< [IN] Source DRAM End Address */
	UINT32 u4DstSa;                         /*!< [OUT] Destination DRAM Start Address */
	UINT32 u4DstEa;                         /*!< [OUT] Destination DRAM End Address */
	UINT32 u4HWWorkingBufferAddr;           /*!< [NONE] GCPU Hardware working buffer Address */
	UINT32 u4CmdPtr[10];                    /* < Cmd Ptr (0: Key0 Ptr, 1: Key1 Ptr, 2: Result Ptr) */
	UINT32 u4Param0[64];                    /* < Opt Key0 Data */
	UINT32 u4Param1[64];                    /* < Opt Key1 Data */
	UINT32 u4Param2[64];                    /* < Opt Key2 Data */
	UINT32 u4Param3[64];                    /* < Opt Key3 Data */

	UINT8  u1EvenKey[16];
	UINT8  u1OddKey[16];
	UINT8  u1IV[16];
} TSDESC_Param;
#endif
/*!
 * @brief Cmd parameter structure for Efuse
 */
#ifdef GCPU_SUPPORT_EFUSE_WRITE
typedef struct _EFUSE_PARAM {
	UINT32 u4EfuseAddr;                     /*!< [IN] Efuse DRAM Address */
	UINT32 u4EfuseValue;                    /*!< [IN] Efuse Value*/

} EFUSE_Param;
#endif
/*!
 * @brief Cmd parameter structure for SHA-1
 */
typedef struct _SHA1_PARAM {
	UINT32 u4SrcSa;           /*!< [IN] Source DRAM Address */
	UINT32 u4DatLen;          /*!< [IN] Data transfer length (in unit of byte) */
	UINT64 u8BitCnt;          /*!< [IN] Bit Count for previouse data, for first packet, it should be zero */
	BOOL   fgFirstPacket;     /*!< [IN] TRUE: The content contains the first packet.
FALSE: The content does not contain the first packet. */
	BOOL   fgLastPacket;      /*!< [IN] TRUE: The content contains the last packet.
FALSE: The content does not contain the last packet. */
	BYTE  *pbIniHash;         /*!< [IN] Initial Hash value DRAM address. Len: 160 bits.
It should not be set for first packet */
	BYTE  *pbResHash;         /*!< [OUT] Result Hash value DRAM address. Len: 160 bits */
	/*!< [OUT] Result Hash value DRAM address. Len: 160 bits */
} SHA1_Param;

/*!
 * @brief Cmd parameter structure for MD5
 */
typedef struct _MD5_PARAM {
	UINT32 u4SrcSa;             /*!< [IN] Source DRAM Address */
	UINT32 u4DatLen;            /*!< [IN] Data transfer length (in unit of byte) */
	UINT64 u8BitCnt;            /*!< [IN] Bit Count for previouse data, for first packet, it should be zero */
	BOOL fgFirstPacket;         /*!< [IN] TRUE: The content contains the first packet.
	FALSE: The content does not contain the first packet. The initial value has to be set. */
	BOOL   fgLastPacket;        /*!< [IN] TRUE: The content contains the last packet.
	FALSE: The content does not contain the last packet. */
	BYTE *pbIniHash;            /*!< [IN] Initial Hash value DRAM address. Len: 128 bits */
	BYTE *pbResHash;             /*!< [OUT] Result Hash value DRAM address. Len: 128 bits */
} MD5_Param;

/*!
 * @brief Cmd parameter structure for MEMCPY
 */
typedef struct _MEM_PARAM_CPY {
	UINT32 u4SrcSa;                          /*!< [IN] Source DRAM Address */
	UINT32 u4DstSa;                          /*!< [IN] Destination DRAM Address */
	UINT32 u4DatLen;                        /*!< [IN] Data transfer length (in unit of byte) */
} MEM_Param_CPY;

/*!
 * @brief Cmd parameter structure for DMA
 */
typedef struct _DMA_PARAM {
	UINT32 u4DramSa;                      /*!< [IN] DRAM Address */
	UINT32 u4DatLen;                        /*!< [IN] Data transfer length (in unit of byte) */
	UINT16 u2SlotHandle;                   /*!< [IN] Slot handle. Slot size shoule be same with Data length. */
	UINT8 uMode;                /*!< [IN] DMA mode. 0 /1
								0: Copying data from DRAM to slot
								1: Copying data from non secure slot to DRAM */
} DMA_Param;

/*!
 * @brief Cmd parameter structure for MEM_XOR
 */
typedef struct _MEM_PARAM_XOR {
	UINT32 u4DataSz;                         /*!< [IN] Data size. */
	UINT8 *pbData;                           /*!< [IN] Data value DRAM address. */
	UINT8 *pbXORValue;                       /*!< [IN] XOR value DRAM address. */
	UINT16  u2DataSlotHandle;                  /*!< [IN/OUT] Data slot handle. Slot size should be 128bits. */
	UINT8  uFlag;        /*!< [IN] Slot handle flag. 1: using slot handle, 0: using DRAM address. */
} MEM_Param_XOR;

/*!
 * @brief Cmd parameter structure for KP_SET
 *
 * Key value can use pbKey or uKeySlotHandle according to uFlag
 */
typedef struct _KP_PARAM {
	BYTE *pbKey;                      /*!< [IN] Key value DRAM address. Len: 128bits */
	UINT16 u2KeySlotHandle;       /*!< [IN] Key slot handle. Slot size should be 128bits. */
	UINT8 uFlag;  /*!< [IN] Slot handle flag. 1: using slot handle, 0: using DRAM address. bit0: key */
} KP_Param;


/*!
 * @brief Cmd parameter structure for SACD Dec Command
 *
 */
typedef struct _SACDDec_PARAM {
	UINT32 u4SrcSa;  /*!< [IN] Source DRAM Address */
	UINT32 u4DstSa;  /*!< [IN] Destination DRAM Address */
	UINT32 u4DatLen; /*!< [IN] Data transfer length (in unit of byte, Should be 2064 bytes align) */
} SACDDec_Param;


/*!
 * @brief Cmd parameter structure for RC4 KSA Command
 *
 */
typedef struct _RC4KSA_PARAM {
	BYTE *pbKey;                     /*!< [IN] Key value DRAM address. Len: same with key length */
	UINT8 uKeyLen;                  /*!< [IN] Key length. 0: 8 byte, 1: 12byte */
	UINT32 u4WorkBufSa;        /*!< [IN] Working Buffer DRAM Address (Size Must > 1024 Byte) */
	UINT8 uFlag;  /*!< [IN] Slot handle flag. 1: using slot handle, 0: using DRAM address. bit0: key */
	UINT16 u2KeySlotHandle;    /*!< [IN] Key Slot Handle. Slot size should be same with key length*/
} RC4KSA_Param;


/*!
 * @brief Cmd parameter structure for RC4 64B0 Command
 *
 */
typedef struct _RC464B0_PARAM {
	UINT32 u4WorkBufSa;        /*!< [IN] Working Buffer DRAM Address (Size Must > 1024 Byte) */
	BYTE *pbRC464B0Result;   /*!< [OUT] Result RC464B0 Result DRAM address. Len: 64 bytes */
	UINT16 u2ResSlotHandle;  /*!< [OUT] Result Slot Handle. Slot size should be 64 bits. */
	UINT8 uFlag; /*!< [IN] Slot handle flag.
				1: using slot handle, 0: using DRAM address. bit0: result */
} RC464B0_Param;


/*!
 * @brief Cmd parameter structure for RC4 64B0 Command
 *
 */
typedef struct _WMDRMPDDEC_PARAM {
	UINT32 u4SrcSa;                          /*!< [IN] Source DRAM Address */
	UINT32 u4DatLen;                        /*!< [IN] Data transfer length (in unit of byte) */
	UINT32 u4WorkBufSa;        /*!< [IN] Working Buffer DRAM Address (Size Must > 1024 Byte) */
	UINT32 u4FifoSa;                                /*!< [IN] FIFO Start Address */
	UINT32 u4FifoEa;                                  /*!< [IN] FIFO End Address */
	UINT8 uMode;                      /*!< [IN] Mode, 0: One-Pass Mode */
	UINT8 *pbMACKey;                           /*!< [IN] MacKey DRAM address. */
	UINT8 *pbRC4PayloadKey;               /*!< [IN] RC4PayloadKey DRAM address. */
	UINT16 u2MacKeySlotHandle;  /*!< [OUT] MacKey Slot Handle. Slot size should be 48 Bytes */
	UINT16 u2RC4KeySlotHandle;  /*!< [OUT] RC4PayloadKey Slot Handle. Slot size should be 8 Bytes */
	UINT8 uFlag; /*!< [IN] Slot handle flag.
			1: using slot handle, 0: using DRAM address. bit0: MacKey, 1: RC4 Key */
} WMDRMPDDEC_Param;


/*!
 * @brief Cmd parameter structure for Set Key to Secure Slot
 *
 */
typedef struct _SETKEY2SLOT_PARAM {
	UINT16 u2SSlotPtr;
	UINT8 *pui1_content_key;
	UINT32 *pui4_content_key_sz;
} SETKEY2SLOT_Param;


/*!
 * @brief Cmd parameter structure for Marlin Dec Command
 *
 */
typedef struct _MARLINDec_PARAM {
	UINT32 u4SrcSa;                          /*!< [IN] Source DRAM Address */
	UINT32 u4DstSa;                          /*!< [IN] Destination DRAM Address */
	UINT32 u4DatLen;                        /*!< [IN] Data transfer length (in unit of byte,
	Should be 2064 bytes align) */
	UINT8 *pbIVData;                        /*!< [IN] IV Data Ptr (in unit of byte, Should be 16 bytes) */
} MARLINDec_Param;


#define GCPU_MEMCPY                       \
	((UINT32)(MISC_CMD_SID + 0x00))         /* < DRAM Data Moving, parameter struct is MEM_Param_CPY */
#define GCPU_DMA                             \
	((UINT32)(MISC_CMD_SID + 0x01))         /* < DRAM Direct Memory Access, parameter struct is DMA_Param */
#define GCPU_SHA_1                          \
	((UINT32)(MISC_CMD_SID + 0x02))         /* < SHA-1 Algorithm, parameter struct is SHA1_Param */
#define GCPU_MD5                              \
	((UINT32)(MISC_CMD_SID + 0x03))         /* < MD5 Algorithm, parameter struct is MD5_Param */
#define GCPU_KP_SET                        \
	((UINT32)(MISC_CMD_SID + 0x04))          /* < Hardware Key Bus Setting, The bus is currently used by
	HDMI key protection. Parameter struct is KP_Param */
#define GCPU_MEM_XOR                     \
	((UINT32)(MISC_CMD_SID + 0x05))         /* < Memory XOR, parameter struct is MEM_Param_XOR */
#define GCPU_SACD_DEC                    \
	((UINT32)(MISC_CMD_SID + 0x06))         /* < SACD Decryption, parameter struct is SACDDec_Param */
#define GCPU_RC4_KSA                       \
	((UINT32)(MISC_CMD_SID + 0x07))         /* < WMDRMPD-RC4 Opt , parameter struct is RC4KSA_Param */
#define GCPU_RC4_64B0                     \
	((UINT32)(MISC_CMD_SID + 0x08))         /* < WMDRMPD-RC4 Opt , parameter struct is RC464B0_Param */
#define GCPU_WMDRMPD_DEC             \
	((UINT32)(MISC_CMD_SID + 0x09))         /* < WMDRMPD-PD Dec , parameter struct is WMDRMPDDEC_Param */
#define GCPU_SETKEY2SLOT                \
	((UINT32)(MISC_CMD_SID + 0x0A))         /* < Set Key to Secure Slot */
#if GCPU_SUPPORT_TSDESC_VUDU
#define GCPU_TSDESC_VUDU                \
	((UINT32)(MISC_CMD_SID + 0x0B))         /* < Set Key to Secure Slot */
#endif
#define GCPU_EFUSE_WR                   ((UINT32)(MISC_CMD_SID + 0X0C))
/*! @} */

/*! \name SEC_SW command group
* @{
*/

/*!
 * @brief Cmd parameter structure for SEC_SW_ECDSA_SIGN
 */
typedef struct _SEC_SW_ECDSA_SIGN_PARAM {
	UINT8 *pui1_ecdsa_param;          /*!< [IN]  ecdsa parameter 5*20 bytes, memory allocation way
shall be continuous, it also shall be 32 bytes alignment if SMP support */
	UINT32 ui4_pri_key_enc_mode;      /*!< [IN]  private key encryption mode (0 or 1) */
	UINT8 *pui1_enc_pri_key;          /*!< [IN]  encrypted private key ( 32 bytes, 20 bytes original key size ) */
	UINT8 *pui1_enc_cus_key;          /*!< [IN]  encrypted customer key ( 16 bytes ) */
	UINT8 *pui1_data;                 /*!< [IN]  data, memory allocation way shall be continuous,
it also shall be 32 bytes alignment if SMP support */
	UINT32 ui4_data_sz;               /*!< [IN]  data size */
	BOOL   fg_sha1_value;             /*!< [IN]  flag indicates if data is sha1 hash value */
	UINT8 *pui1_sig;                  /*!< [OUT] signature 320 bits */
} SEC_SW_ECDSA_SIGN_param;

/*!
 * @brief Cmd parameter structure for SEC_INIT_RNG
 */
typedef struct _SEC_SW_INIT_RNG_PARAM {
	UINT32 ui4_sec_ef_dw_adr;           /*!< [IN] secure data DWORD address in efuse */
	UINT32 ui4_sec_ef_dw_sz;            /*!< [IN] secure data DWORD size in efuse */
	UINT32 ui4_nonsec_ef_dw_adr;        /*!< [IN] non-secure DWORD address in efuse */
	UINT32 ui4_nonsec_ef_dw_sz;         /*!< [IN] non-secure DWORD address in efuse */
} SEC_SW_INIT_RNG_param;

/*!
 * @brief Cmd parameter structure for SEC_RNG
 */
typedef struct _SEC_SW_RNG_PARAM {
	UINT32 ui4_rn_sz;                  /*!< [IN] random number size */
	UINT8 *pui1_rn;                    /*!< [IN/OUT] random number */
} SEC_SW_RNG_param;


/*!
 * @brief Cmd parameter structure for X_SEC_SW_HASH_RNS
 */
typedef struct _SEC_SW_HASH_RNS_PARAM {
	UINT8 *pui1_rns;        /*!< [IN] sower which scramble seed of random number generation. Len: 16 bytes */
} SEC_SW_HASH_RNS_param;



/*!
 * @brief Cmd parameter structure for SW_ECDSA_CAL_PUB_KEY
 */
typedef struct _SW_ECDSA_CAL_PUB_KEY_PARAM {
	UINT8 *pui1_ecdsa_param;    /*!< [IN]  ecdsa parameter 5*20 bytes */
	UINT8 *pui1_pri_key;        /*!< [IN]  private key 160 bits */
	UINT8 *pui1_pub_key;        /*!< [OUT] public key 320 bits */
} SW_ECDSA_CAL_PUB_KEY_param;

/*!
 * @brief Cmd parameter structure for SEC_SW_SFE_AUTH_SIGN
 */
typedef struct _SEC_SW_SFE_AUTH_SIGN_PARAM {
	UINT8 *pui1_rd;         /*!< [IN]  rd, 16 bytes */
	UINT8 *pui1_iv_nonce;   /*!< [IN]  iv nonce, 16 bytes */
	UINT8 *pui1_eiv;        /*!< [IN]  eiv, 16 bytes */
	UINT8 *pui1_ka_nonce;   /*!< [IN]  ka nonce, 16 bytes */
	UINT8 *pui1_eka;        /*!< [IN]  eka, 16 bytes */
	UINT8 *pui1_host_sig;   /*!< [OUT] host signature, 48 bytes */
} SEC_SW_SFE_AUTH_SIGN_param;

/*!
 * @brief Cmd parameter structure for SEC_SW_SFE_AUTH_VERIFY
 */
typedef struct _SEC_SW_SFE_AUTH_VERIFY_PARAM {
	UINT8 *pui1_host_sig;   /*!< [IN]     host signature, 48 bytes */
	UINT8 *pui1_drv_sig;    /*!< [IN]     drive signature, 48 bytes */
	UINT8 *pui1_iv_nonce;   /*!< [IN]     iv nonce, 16 bytes */
	UINT8 *pui1_eiv;        /*!< [IN]     eiv, 16 bytes */
	UINT8 *pui1_ka_nonce;   /*!< [IN]     ka nonce, 16 bytes */
	UINT8 *pui1_eka;        /*!< [IN]     eka, 16 bytes */
	UINT8 *pui1_c_nonce;    /*!< [IN]     c nonce, 16 bytes */
	UINT8 *pui1_ec;         /*!< [IN]     ec, 16 bytes */
	UINT16 ui2_kse_ssl;     /*!< [IN/OUT] secure slot of kse, 16 bytes */
	UINT16 ui2_ksm_ssl;     /*!< [IN/OUT] secure slot of ksm, 16 bytes */
} SEC_SW_SFE_AUTH_VERIFY_param;

/*!
 * @brief Cmd parameter structure for SEC_SW_SMP_GEN_KB
 */
typedef struct _SEC_SW_SMP_GEN_KB_PARAM {
	UINT8 **ppui1_key_set;      /*!< [IN] key set array, every key elements's memory allocation way
shall be continuous, it also shall be 32 bytes alignment if SMP support*/
	UINT32  *pui4_key_set_sz;   /*!< [IN] key set sz array, memory allocation way shall be continuous,
								it also shall be 32 bytes alignment if SMP support */
	UINT32   ui4_kb_sz;         /*!< [IN] key block size */
	UINT8  *pui1_kb;            /*!< [IN/OUT] kb, 32K bytes, memory allocation way shall be continuous,
								it also shall be 32 bytes alignment if SMP support */
	UINT32  *pui4_st;           /*!< [IN/OUT] process status, 4 byte */
} SEC_SW_SMP_GEN_KB_param;

/*!
 * @brief Cmd parameter structure for SEC_SW_VEN_HMAC_SHA256
 */
typedef struct _SEC_SW_VEN_HMAC_SHA256_PARAM {
	BOOL   fg_b64_enc_key;   /*!< [IN]  flag indicates if pui1_hmac_key is base64 encoded or not */
	BOOL   fg_use_enc_key;   /*!< [IN]  flag indicates if pui1_hmac_key is the encrypted */
	UINT8  ui1_dec_key_flag; /*!< [IN]  slot handle flag. 1: using slot handle, 0: using DRAM address. bit0: key */
	UINT8 *pui1_dec_key;     /*!< [IN]  key for decrypting hmac key, 128 bits */
	UINT16 ui2_dec_key_ssl;  /*!< [IN]  key slot handle for decrypting hmac key, slot size should be 128 bits. */
	UINT8 *pui1_hmac_key;    /*!< [IN]  memory allocation way shall be continuous
									plain key,     when fg_use_enc_key == FALSE
									32 bytes, when fg_b64_enc_key == FALSE
									44 bytes, when fg_b64_enc_key == TRUE
									encrypted key, when fg_use_enc_key == TRUE
									32 bytes, when fg_b64_enc_key == FALSE
									48 bytes, when fg_b64_enc_key == TRUE */
	UINT8 *pui1_data;        /*!< [IN]  data, memory allocation way shall be continuous,
							it also shall be 32 bytes alignment if SMP support */
	UINT32 ui4_data_sz;      /*!< [IN]  data size */
	UINT8 *pui1_hv;          /*!< [OUT] hash value, 32 bytes */
} SEC_SW_VEN_HMAC_SHA256_PARAM;

/*!
 * @brief Cmd parameter structure for SEC_SW_VEN_AES128
 */
typedef struct _SEC_SW_VEN_AES128_PARAM {
	BOOL   fg_enc;                   /*!< [IN]  TRUE -> encryption, FALSE -> decryption */
	UINT32 ui4_ven_nf_cipher_mode;   /*!< [IN]  vendor nf cipher mode,
									0 -> ecb mode
									1 -> cbc mode */
	UINT8 *pui1_iv;                  /*!< [IN]  memory allocation way shall be continuous
									initialization vector, 16 bytes,
									only for cbc mode
									NULL for ecb mode */
	BOOL   fg_b64_enc_key;           /*!< [IN]  flag indicates if pui1_key is base64 encoded or not */
	BOOL   fg_use_enc_key;           /*!< [IN]  flag indicates if pui1_key is the encrypted */
	UINT8  ui1_dec_key_flag;         /*!< [IN]  slot handle flag. 1: using slot handle,
									0: using DRAM address. bit0: key */
	UINT8 *pui1_dec_key;             /*!< [IN]  key for decrypting hmac key, 128 bits */
	UINT16 ui2_dec_key_ssl;          /*!< [IN]  key slot handle for decrypting hmac key,
									slot size should be 128 bits. */
	UINT8 *pui1_key;                 /*!< [IN]  memory allocation way shall be continuous
									plain key,     when fg_use_enc_key == FALSE
									16 bytes, when fg_b64_enc_key == FALSE
									24 bytes, when fg_b64_enc_key == TRUE
									encrypted key, when fg_use_enc_key == TRUE
									16 bytes, when fg_b64_enc_key == FALSE
									32 bytes, when fg_b64_enc_key == TRUE */

	UINT8 *pui1_data;                /*!< [IN]  data, memory allocation way shall be continuous,
	it also shall be 32 bytes alignment if SMP support */
	UINT32 ui4_sz;                   /*!< [IN]  data/result size, 16 byte alignment */
	UINT8 *pui1_res;                 /*!< [OUT] result, memory allocation way
	shall be continuous,it also shall be 32 bytes alignment if SMP support */
} SEC_SW_VEN_AES128_PARAM;

/*!
 * @brief Cmd parameter structure for EC_SW_INIT_MARLIN_INFO
 */
typedef struct _SEC_SW_INIT_MARLIN_INFO_PARAM {
	UINT16 ui2_prot_ssl;             /*!< [IN]  key slot handle for protecting device key,
									slot size should be 128 bits. */
	UINT8 *pui1_enc_org_dev_key_sz;  /*!< [IN]  encrypted original device key size, 16 bytes */
	UINT8 *pui1_enc_dev_key;         /*!< [IN]  encrypted device key, memory allocation
	way shall be continuous,it also shall be 32 bytes alignment if SMP support */
	UINT32 ui4_enc_dev_key_sz;       /*!< [IN]  size for encrypted device key */
} SEC_SW_INIT_MARLIN_INFO_param;



/*!
 * @brief Cmd parameter structure for SEC_SW_DERIVECW_PROCESS
 */
typedef struct _SEC_SW_DERIVECW_PROCESS_PARAM {
	UINT16 ui2_Key_ssl;             /*!< [IN]  key slot handle for protecting device key,
									slot size should be 128 bits. */
	UINT8 *pui1_Ecm;             /*!< [IN]  Ecm Data, data size should be 256 bits. */
	UINT16 ui2_OutputKey_ssl;
	/*!< [OUT]  Output key slot handle for protecting device key, slot size should be 128 bits. */
	UINT8 *pui1_Result_Flag;  /*!< [OUT]  encrypted original device key size, 32 bits */
} SEC_SW_DERIVECW_PROCESS_PARAM;



/*!
 * @brief Cmd parameter structure for EC_SW_UNINIT_MARLIN_INFO
 */
/* NULL */

/*!
 * @brief Cmd parameter structure for SW_AES128_CMAC
 */
typedef struct _SW_AES128_CMAC_PARAM {
	UINT8 *pui1_key;            /*!< [IN]  key 128 bits */
	UINT8 *pui1_res;            /*!< [OUT] result 128 bits */
	UINT8 *pui1_data;           /*!< [IN]  data */
	UINT32 ui4_data_sz;         /*!< [IN]  data size */
} SW_AES128_CMAC_param;

/*!
 * @brief Cmd parameter structure for SW_ECDSA_SIGN
 */
typedef struct _SW_ECDSA_SIGN_PARAM {
	UINT8 *pui1_ecdsa_param;   /*!< [IN]  ecdsa parameter 5*20 bytes */
	UINT8 *pui1_priv_key;      /*!< [IN]  private key 160 bits */
	UINT8 *pui1_data;          /*!< [IN]  data */
	UINT32 ui4_data_sz;        /*!< [IN]  data size */
	BOOL   fg_sha1_value;      /*!< [IN]  flag indicates if data is sha1 hash value */
	UINT8 *pui1_signature;     /*!< [OUT] signature 320 bits */
} SW_ECDSA_SIGN_param;

/*!
 * @brief Cmd parameter structure for SW_ECDSA_VERIFY
 */
typedef struct _SW_ECDSA_VERIFY_PARAM {
	UINT8 *pui1_ecdsa_param;    /*!< [IN] ecdsa parameter 5*20 bytes */
	UINT8 *pui1_pub_key;        /*!< [IN] public key 320 bits */
	UINT8 *pui1_signature;      /*!< [IN] signature 320 bits */
	UINT8 *pui1_data;           /*!< [IN] data */
	UINT32 ui4_data_sz;         /*!< [IN] data suze */
	BOOL   fg_sha1_value;       /*!< [IN] indicate if the data is sha1 hash value or normal data */
} SW_ECDSA_VERIFY_param;

/*!
 * @brief Cmd parameter structure for SW_ECDSA_MUL_ECP_WITH_SCALAR
 */
typedef struct _SW_ECDSA_MUL_ECP_WITH_SCALAR_PARAM {
	UINT8 *pui1_ecdsa_param;    /*!< [IN]  ecdsa parameter 5*20 bytes */
	UINT8 *pui1_scalar;         /*!< [IN]  scalar 160 bits */
	UINT8 *pui1_ecp;            /*!< [IN]  ec point 320 bits */
	UINT8 *pui1_res;            /*!< [OUT] result 320 bits */
} SW_ECDSA_MUL_ECP_WITH_SCALAR_param;

/*!
 * @brief Cmd parameter structure for SW_AES128E_CMAC
 */
typedef struct _SW_AES128E_CMAC_PARAM {
	UINT8  ui1_flag;
	/*!< [IN]  slot handle flag.1: using slot handle, 0: using DRAM address. bit0: key */
	UINT16 ui2_key_ssl;         /*!< [IN]  key Slot Handle. Slot size should be same with key length */
	UINT8 *pui1_key;            /*!< [IN]  key 128 bits */
	UINT8 *pui1_data;           /*!< [IN]  data */
	UINT32 ui4_data_sz;         /*!< [IN]  data size */
	UINT8 *pui1_res;            /*!< [OUT] result 128 bits */
} SW_AES128E_CMAC_param;

/*!
 * @brief Cmd parameter structure for SEC_SW_IN_WD_SEC_PD_VEN_CIPHER
 */
typedef struct _SEC_SW_IN_WD_SEC_TZ_PD_VEN_CIPHER_PARAM {
	BOOL   fg_enc;              /*!< [IN]  encryption (TRUE) or decryption (FALSE) */
	UINT8 *pui1_data;           /*!< [IN]  data */
	UINT32 ui4_sz;              /*!< [IN]  data/result size, 16 byte alignment */
	UINT8 *pui1_res;            /*!< [IN/OUT] result */
} SEC_SW_IN_WD_SEC_TZ_PD_VEN_CIPHER_param;

/*!
 * @brief Cmd parameter structure for SEC_SW_IN_WD_SEC_RNG
 */
typedef struct _SEC_SW_IN_WD_SEC_RNG_PARAM {
	UINT32 ui4_rn_sz;           /*!< [IN] random number size */
	UINT8 *pui1_rn;             /*!< [IN/OUT] random number */
} SEC_SW_IN_WD_SEC_RNG_param;

/*!
 * @brief Cmd parameter structure for SEC_SW_IN_WD_SEC_GET_MARLIN_DEV_KEY_SZ
 */
typedef struct _SEC_SW_IN_WD_SEC_GET_MARLIN_DEV_KEY_SZ_PARAM {
	UINT32 *pui4_sz;                /*!< [IN/OUT] marlin device key size */
} SEC_SW_IN_WD_SEC_GET_MARLIN_DEV_KEY_SZ_param;

/*!
 * @brief Cmd parameter structure for SEC_SW_IN_WD_SEC_GET_MARLIN_DEV_KEY
 */
typedef struct _SEC_SW_IN_WD_SEC_GET_MARLIN_DEV_KEY_PARAM {
	UINT8 *pui1_dev_key;           /*!< [IN/OUT] marlin device key */
} SEC_SW_IN_WD_SEC_GET_MARLIN_DEV_KEY_param;

/*!
 * @brief Cmd parameter structure for SEC_SW_IN_WD_SEC_UT_AESPK_D
 */
typedef struct _SEC_SW_IN_WD_SEC_UT_DTCP_AESPK_D_PARAM {
	UINT8 *pui1_key;           /*!< [IN] common data public key */
	UINT8 *pui1_data;           /*!< [IN/OUT] in buffer to decrypt and out buffer*/
	UINT32 ui4_sz;           /*!< [IN/OUT]  data/result size */
} SEC_SW_IN_WD_SEC_UT_DTCP_AESPK_D_param;

/*!
 * @brief Cmd parameter structure for secure code
 */
/* command in normal world and uses secure code */
#define SEC_SW_ECDSA_SIGN                   \
	((UINT32)(GCPU_CMD_USE_SEC_CODE + (SW_CMD_SID + 0x00))) /*!< ecdsa sign */
#define SEC_SW_INIT_RNG                     \
	((UINT32)(GCPU_CMD_USE_SEC_CODE + (SW_CMD_SID + 0x01))) /*!< aacs initialize generate random number */
#define SEC_SW_RNG                          \
	((UINT32)(GCPU_CMD_USE_SEC_CODE + (SW_CMD_SID + 0x02))) /*!< aacs generate random number */
#define SEC_SW_SFE_AUTH_SIGN                \
	((UINT32)(GCPU_CMD_USE_SEC_CODE + (SW_CMD_SID + 0x03))) /*!< sfe sign */
#define SEC_SW_SFE_AUTH_VERIFY              \
	((UINT32)(GCPU_CMD_USE_SEC_CODE + (SW_CMD_SID + 0x04))) /*!< sfe sign */
#define SEC_SW_SMP_GEN_KB                   \
	((UINT32)(GCPU_CMD_USE_SEC_CODE + (SW_CMD_SID + 0x05))) /*!< smp generate kb */
#define SEC_SW_VEN_HMAC_SHA256              \
	((UINT32)(GCPU_CMD_USE_SEC_CODE + (SW_CMD_SID + 0x06))) /*!< vendor hmac sha256 */
#define SEC_SW_VEN_AES128                   \
	((UINT32)(GCPU_CMD_USE_SEC_CODE + (SW_CMD_SID + 0x07))) /*!< vendor aes 128 */
#define SEC_SW_INIT_MARLIN_INFO             \
	((UINT32)(GCPU_CMD_USE_SEC_CODE + (SW_CMD_SID + 0x08))) /*!< initialize marlin device key information */
#define SEC_SW_UNINIT_MARLIN_INFO           \
((UINT32)(GCPU_CMD_USE_SEC_CODE + (SW_CMD_SID + 0x09))) /*!< un-initialize marlin device key information */
#define SEC_SW_HASH_RNS                     \
	((UINT32)(GCPU_CMD_USE_SEC_CODE + (SW_CMD_SID + 0x0A))) /*!< un-initialize marlin device key information */
#define SEC_SW_DERIVECW_PROCESS             \
	((UINT32)(GCPU_CMD_USE_SEC_CODE + (SW_CMD_SID + 0x0B))) /*!< Widevine Key Process */

/* command in normal world and uses normal code */
#define SW_AES128_CMAC               \
	((UINT32)(SW_CMD_SID + 0x10))      /*!< aes128 cmac by software */
#define SW_ECDSA_SIGN                \
	((UINT32)(SW_CMD_SID + 0x11))      /*!< ecdsa sign by software */
#define SW_ECDSA_VERIFY              \
	((UINT32)(SW_CMD_SID + 0x12))      /*!< ecdsa verify by software */
#define SW_ECDSA_CAL_PUB_KEY         \
	((UINT32)(SW_CMD_SID + 0x13))      /*!< calculate ecdsa public key by software */
#define SW_ECDSA_MUL_ECP_WITH_SCALAR \
	((UINT32)(SW_CMD_SID + 0x14))      /*!< calculate ecdsa ecp point multiplication with scalar by software */
#define SW_AES128E_CMAC              \
	((UINT32)(SW_CMD_SID + 0x15))      /*!< aes128e cmac */

/* command in secure world and uses secure code */
#define SEC_SW_IN_WD_SEC_PD_VEN_CIPHER \
	((UINT32)(GCPU_CMD_USE_SEC_CODE + GCPU_CMD_IN_SEC_WD + (SW_CMD_SID + 0x00)))
	/*!< platformdepend vendor cipher */
#define SEC_SW_IN_WD_SEC_RNG \
	((UINT32)(GCPU_CMD_USE_SEC_CODE + GCPU_CMD_IN_SEC_WD + (SW_CMD_SID + 0x01))) /*!< random number generation */
#define SEC_SW_IN_WD_SEC_GET_MARLIN_DEV_KEY_SZ \
	((UINT32)(GCPU_CMD_USE_SEC_CODE + GCPU_CMD_IN_SEC_WD + (SW_CMD_SID + 0x02))) /*!< get marlin device key size */
#define SEC_SW_IN_WD_SEC_GET_MARLIN_DEV_KEY \
	((UINT32)(GCPU_CMD_USE_SEC_CODE + GCPU_CMD_IN_SEC_WD + (SW_CMD_SID + 0x03))) /*!< get marlin device key */
#define SEC_SW_IN_WD_SEC_UT_DTCP_AESPK_D \
	((UINT32)(GCPU_CMD_USE_SEC_CODE + GCPU_CMD_IN_SEC_WD + (SW_CMD_SID + 0x04)))
	/*!< for dtcp usage,decrypt with common data public key */

/*! @} */


/*  GCPU supported encrypt and decrypt algorithm command ground */
typedef enum {
	None = 0,  /* < Don't use it, only for debug using */
	CSS,          /* < CSS_DEC_DK, CSS_DEC_TK, CSS_DSC_AV, CSS_AUTH_DRV, CSS_AUTH_DEC, CSS_AUTH_BK */
	Others      /* < other command */
} GCPU_CMDGROUP;


INT32 i4GCPU_CheckSslot(
	UINT16 u2KeySlot    /* < [IN] Key Slot */
);

/*  Allocate slot */
/*  \return If return value < 0, it's failed. Please reference drv_gcpu_errcode.h. */
INT32 i4GCPU_SlotAlloc(
	BOOL fgSecure,
	/* < [IN] TRUE: secure slot, FALSE: nonsecure slot */
	UINT16 u2SlotSize,
	/* < [IN] Slot size, uint: bits */
	UINT16 *pu2SlotHandle
	/* < [OUT] Slot handle */
);

/*  Free slot */
/*  \return None. */
void vGCPU_SlotFree(
	UINT16 u2SlotHandle    /* < [IN] Slot handle */
);

/*  GCPU callback function prototype */
/*  - Used for Async command execute */
/*  . */
typedef void (*GCPU_FUNC_CB)(
	BOOL fgResult,
	/* < [IN] TRUE: Success. FALSE: Fail */
	UINT32 u4Cmd,
	/* < [IN] Execute command */
	void *pvUserPrivate
	/* < [IN] user private data */
);

/*  Create GCPU instance */
/*  \return If return value < 0, it's failed. Please reference drv_gcpu_errcode.h. */
INT32 i4GCPU_InstanceCreate(
	GCPU_FUNC_CB pfnCB,
	/* < [IN] Callback function */
	void *pvUserPrivate,
	/* < [IN] User private data for callback function */
	UINT8 *puHandle
	/* < [OUT] GCPU instance handle */
);

/*  Destroy GCPU instance */
/*  \return None. */
void vGCPU_InstanceDestroy(
	UINT8 uHandle        /* < [IN] GCPU instance handle */
);

/*  Switch command group service
- Before use cmd to encrypt or decrypt data, caller should turn on command group service
- If caller turn on a command group, GCPU will auto turn off old command group service
.
 \return If return value < 0, it's failed. Please reference drv_gcpu_errcode.h. */
INT32 i4GCPU_CmdGroupSwitch(
	UINT8 uHandle,
	/* < [IN] GCPU instance handle */
	GCPU_CMDGROUP eCmdGroup,
	/* < [IN] Command group */
	BOOL fgOn
	/* < [IN] TRUE: On, FALSE: Off */
);

/*  Exec command
/// - If you want to run command in secure code, u4Cmd = (command priority)<<24 + command
/// .
/// \return If return value < 0, it's failed. Please reference drv_gcpu_errcode.h. */
INT32 i4GCPU_CmdExec(
	UINT8 uHandle,
	/* < [IN] GCPU instance handle */
	UINT32 u4Cmd,
	/* < [IN] Command */
	void *pvParam,
	/* < [IN] Command parameter structure pointer according to command type */
	BOOL fgSync
	/* < [IN] TRUE: sync, FALSE: async */
);

/*  Copy from user for Linux */
/*  \return None. */
void _GCPU_Copy_From_User(
	void *kernelbuf,
	/* < [IN] Kernel Buffer Address */
	void *userbuf,
	/* < [IN] User Buffer Address */
	UINT32 u4length
	/* < [IN] Copy Length */
);

/*  Copy from user for Linux */
/*  \return None. */
void _GCPU_Copy_to_User(
	void *userbuf,
	/* < [IN] Kernel Buffer Address */
	void *kernelbuf,
	/* < [IN] User Buffer Address */
	UINT32 u4length
	/* < [IN] Copy Length */
);




/*  Exec command (TZ Flow)
- If you want to run command in secure code, u4Cmd = (command priority)<<24 + command
.
\return If return value < 0, it's failed. Please reference drv_gcpu_errcode.h. */
INT32 i4TZ_GCPU_CmdExec(
	UINT8 uHandle,
	/* < [IN] GCPU instance handle */
	UINT32 u4Cmd,
	/*  [IN] Command */
	void *pvParam,
	/* < [IN] Command parameter structure pointer according to command type */
	BOOL fgSync
	/* < [IN] TRUE: sync, FALSE: async */
);

/*  SHA1 (TZ Flow) */
/*  \return If return value < 0, it's failed. Please reference drv_gcpu_errcode.h. */
INT32 i4TZ_GCPU_SW_SHA1(
	UINT32 u4Length,
	/* < [IN] Sha1 Data Length */
	BYTE *pbData,
	/* < [IN] Src Data */
	BYTE *pbOutHash
	/* < [IN] Hash Output Data */
);

/*  SHA1 Init (TZ Flow) */
/*  \return If return value < 0, it's failed. Please reference drv_gcpu_errcode.h. */
INT32 i4TZ_GCPU_SW_SHA1Init(
	UINT32 *prSha1Ctx                         /* < [IN/OUT] SHA1 Ctx Handle */
);

/*  SHA1 Update (TZ Flow) */
/*  \return If return value < 0, it's failed. Please reference drv_gcpu_errcode.h. */
INT32 i4TZ_GCPU_SW_SHA1Update(
	UINT32 *prSha1Ctx,
	/* < [IN] SHA1 Ctx Handle */
	int len,
	/* < [IN] Message Data Length */
	BYTE *dataIn
	/* < [IN] Message Data Buffer Address */
);

/*  SHA1 Final (TZ Flow) */
/*  \return If return value < 0, it's failed. Please reference drv_gcpu_errcode.h. */
INT32 i4TZ_GCPU_SW_SHA1Final(
	UINT32 *prSha1Ctx,
	/* < [IN] SHA1 Ctx Handle */
	BYTE *hashout
	/* < [OUT] Hash Result Buffer Address */
);

/*  SHA256 (TZ Flow) */
/*  \return If return value < 0, it's failed. Please reference drv_gcpu_errcode.h. */
INT32 i4TZ_GCPU_SW_SHA256(
	UINT8 *pui1_data,
	INT32 ui4_data_sz,
	UINT8 *pui1_hv
);

/*  SHA256 Init (TZ Flow) */
/*  \return If return value < 0, it's failed. Please reference drv_gcpu_errcode.h. */
INT32 i4TZ_GCPU_SW_SHA256Init(
	UINT32 *prSha1Ctx                         /* < [IN/OUT] SHA256 Ctx Handle */
);

/*  SHA256 Update (TZ Flow) */
/*  \return If return value < 0, it's failed. Please reference drv_gcpu_errcode.h. */
INT32 i4TZ_GCPU_SW_SHA256Update(
	UINT32 *prSha1Ctx,
	/* < [IN] SHA256 Ctx Handle */
	int len,
	/* < [IN] Message Data Length */
	BYTE *dataIn
	/* < [IN] Message Data Buffer Address */
);

/*  SHA256 Final (TZ Flow) */
/*  \return If return value < 0, it's failed. Please reference drv_gcpu_errcode.h. */
INT32 i4TZ_GCPU_SW_SHA256Final(
	UINT32 *prSha1Ctx,
	/* < [IN] SHA256 Ctx Handle */
	BYTE *hashout
	/* < [OUT] Hash Result Buffer Address */
);

/*  Random Number Generator Function (TZ Flow) */
/*  \return If return value < 0, it's failed. Please reference drv_gcpu_errcode.h. */
INT32 i4TZ_GCPU_Rng(
	BYTE *pbRandomDataBuf,
	/* < [OUT] Random Number */
	UINT32 u4DataLength
	/* < [IN] Random Number Length (Unit: BYTE) */
);


/* Get GCPU Instance Status */
/* return Instance Status */
UINT32 u4GCPU_GetInstance_Status(
	UINT32 u4GCPUHandle
);

/* Set GCPU Instance Status */
/* return 0 if ok; */
UINT32 u4GCPU_SetInstance_Status(
	UINT32 u4GCPUHandle,
	UINT32 u4Status
);

/*! \name GCPU driver init/uninit
* @{
*/
/* This function turns on HW GCPU driver
- This API can only be called when system power on.
.
return If return value < 0, it's failed. Please reference drv_gcpu_errcode.h. */
INT32 i4GCPU_Init(void);

/* This function turns off HW GCPU driver
- This API can only be called when system power off.
.
return None. */
INT32 i4GCPU_Uninit(UINT32 u4Case);
/* INT32 i4GCPU_Uninit(void); */
/*! @} */
/*
#if CONFIG_GCPU_DEBUG_EN
void vGCPU_DebugMode(UINT32 u4Enable, UINT32 u4Level);
void vGCPU_ShowCallStack(UINT32 uHandle, UINT32 u4Cmd, UINT32 u4DbgLoop);
void vGCPU_Printf(UINT32 u4Line, UINT32 u4Level, CHAR *format, ...);
#endif

#if CONFIG_GCPU_DEBUG_EN
#define GCPU_Printf(x...) vGCPU_Printf(__LINE__, x)
#else
#define GCPU_Printf(x...)
#endif
*/
#ifdef __cplusplus
}
#endif

#endif /* #ifndef _DRV_GCPU_IF_H_ */

