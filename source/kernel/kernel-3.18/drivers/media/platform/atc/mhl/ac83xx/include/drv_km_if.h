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

#ifndef _DRV_KM_H_
#define _DRV_KM_H_

#include "x_typedef.h"
#include "u_km_ext.h"
#include "x_km.h"

#ifdef __cplusplus
extern "C" {
#endif

/* key block size for different version */
#define KEY_BLK_32KB_SZ    (32*1024)
#define KEY_BLK_64KB_SZ    (64*1024)

/*
    export functions for KM
*/
extern INT32 i4KmDrvInitExt(void);
extern INT32 i4KmDrvUninitExt(void);
extern INT32 i4KmDrvInit(KM_EXT_INF_T *pt_km_ext_inf);
extern INT32 i4KmDrvUninit(void);

extern INT32 i4KmCreateInst
(
	UINT32 * pui4_handle           /* [OUT]instance handle */
);

extern INT32 i4KmDestroyInst
(
	UINT32 ui4_handle             /* [IN] instance handle */
);

extern INT32 i4KmExeCmd
(
	UINT32 ui4_handle,
					/* [IN] instance handle */
	UINT32 u4_cmd,
					/* [IN] command */
	void  *pv_param
					/* [IN/OUT] command parameter structure pointer according to command type */
);

extern VOID vKmInitCommon(VOID);

extern VOID vKmUninitCommon(VOID);

extern INT32 i4KmProcSetMlRbpInfo
(
	KM_SET_TYPE_CMD_SET_ML_RBP_INFO_T * pt_param /* [IN] information for setting rollback protection data */
);

extern INT32 i4KmGetAacsDevNodeNum
(
	UINT32  ui4_handle,
					/* [IN] instance handle */
	UINT32 * pui4_dev_node_num
					/* [OUT] device node number */
);

extern INT32 i4KmGetAacsPubKey
(
	UINT32 ui4_handle,
					/* [IN] instance handle */
	UINT8 * pui1_aacs_pub_key
					/* [OUT] 320 bits */
);

extern INT32 i4KmGetAacsCcPubKey
(
	UINT32 ui4_handle,
					/* [IN] instance handle */
	UINT8 * pui1_aacs_cc_pub_key
					/* [OUT] 320 bits */
);

extern INT32 i4KmGetAacsDevKeyInfo
(
	UINT32  ui4_handle,
					/* [IN] instance handle */
	UINT8   ui1_dk_idx,
					/* [IN] device key index */
	UINT16  ui2_dk_slot_handle,
					/* [IN/OUT] key slot handle  */
	UINT32 * pui4_u_mask,
					/* [OUT] u mask */
	UINT32 * pui4_uv_num
					/* [OUT] uv number */
);

extern INT32 i4KmGetAacsSeqKeyInfo
(
	UINT32  ui4_handle,
					/* [IN] instance handle */
	UINT16  ui2_col_num,
					/* [IN] column number */
	UINT16  ui2_sk_ssl,
					/* [IN/OUT] key slot handle */
	UINT16 * pui2_row_num
					/* [OUT] row number */
);

extern INT32 i4KmGetAacsHostCert
(
	UINT32 ui4_handle,
					/* [IN] instance handle */
	UINT8 * pui1_host_cert
					/* [OUT] aacs host certificate, 92 bytes */
);

extern INT32 i4KmGetAacsDevNonce
(
	UINT32 ui4_handle,
					/* [IN] instance handle */
	UINT8 * pui1_dev_nonce
					/* [OUT] aacs device nonce, 16 bytes */
);

extern INT32 i4KmGetBdplusCcPubKey
(
	UINT32 ui4_handle,
					/* [IN] instance handle */
	UINT8 * pui1_cc_pub_key
					/* [OUT] 40 bytes */
);

extern INT32 i4KmGetBdplusNvPubKey
(
	UINT32 ui4_handle,
					/* [IN] instance handle */
	UINT8 * pui1_nv_pub_key
					/* [OUT] 320 buts */
);

extern INT32 i4KmGetBdplusAesKey
(
	UINT32 ui4_handle,
					/* [IN] instance handle */
	UINT32 ui4_idx,
					/* [IN] key index, 0~8 */
	UINT16 ui2_ak_slot_handle
					/* [IN/OUT] key slot handle */
);

extern INT32 i4KmGetBdplusDevCert
(
	UINT32 ui4_handle,
					/* [IN] instance handle */
	UINT8 * pui1_dev_cert
					/* [OUT] 284 bytes */
);

extern INT32 i4KmGetBdplusMvCert
(
	UINT32 ui4_handle,
					/* [IN] instance handle */
	UINT8 * pui1_mv_cert
					/* [OUT] 292 bytes */
);

extern INT32 i4KmGetDivxDrmKeyTbl
(
	UINT32 ui4_handle,
					/* [IN] instance handle */
	UINT8 * pui1_key_tbl
					/* [OUT] 5280 bytes */
);

extern INT32 i4KmGetDivxDrmHdKeyTbl
(
	UINT32 ui4_handle,
					/* [IN] instance handle */
	UINT8 * pui1_key_tbl
					/* [OUT] 1920 bytes */
);

extern INT32 i4KmGetDivxDrmHt30KeyTbl
(
	UINT32 ui4_handle,
					/* [IN] instance handle */
	UINT8 * pui1_key_tbl
					/* [OUT] 1920 bytes */
);

extern INT32 i4KmGetDivxDrmPlusBaseKeyTbl
(
	UINT32 ui4_handle,
					/* [IN] instance handle */
	UINT8 * pui1_key_tbl
					/* [OUT] 5280 bytes */
);

extern INT32 i4KmGetDivxDrmPlusBundingKeyTbl
(
	UINT32 ui4_handle,
					/* [IN] instance handle */
	UINT8 * pui1_key_tbl
					/* [OUT] 5280 bytes */
);

extern INT32 i4KmGetSacdDevId
(
	UINT32 ui4_handle,
					/* [IN] instance handle */
	UINT8 * pui1_dev_id
					/* [OUT] 5 bytes */
);

extern INT32 i4KmGetSacdDevKey
(
	UINT32 ui4_handle,
					/* [IN] instance handle */
	UINT8  ui1_key_idx,
					/* [IN] key index [0, 39] */
	UINT8 * pui1_dev_id
					/* [OUT] 16 bytes */
);

extern INT32 i4KmGetHdcpKey
(
	UINT32 ui4_handle,
					/* [IN] instance handle */
	UINT32 ui4_key_idx,
					/* [IN] key index 0~39 */
	UINT16 ui2_key_slot_handle
					/* [IN/OUT] key slot handle  */
);

extern INT32 i4KmGetHdcpKsv
(
	UINT32 ui4_handle,
					/* [IN] instance handle */
	UINT8 * pui1_ksv
					/* [IN/OUT] key selection vector, 8 bytes */
);

extern INT32 i4KmGetCprmKey
(
	UINT32  ui4_handle,
					/* [IN] instance handle */
	UINT8   ui1_col_num,
					/* [IN] column number */
	UINT16  ui2_key_ssl
					/* [IN/OUT] key slot handle ( 16 bytes, key is in [1,7] ) */
);

extern INT32 i4KmGetCprmKeyInfo
(
	UINT32  ui4_handle,
					/* [IN] instance handle */
	UINT8   ui1_col_num,
					/* [IN] column number */
	UINT32 * pui4_row_num
					/* [OUT] row number */
);

extern INT32 i4KmGetCppmKey
(
	UINT32  ui4_handle,
					/* [IN] instance handle */
	UINT8   ui1_col_num,
					/* [IN] column number */
	UINT16  ui2_key_ssl
					/* [IN/OUT] key slot handle ( 16 bytes, key is in [1,7] ) */
);

extern INT32 i4KmGetCppmKeyInfo
(
	UINT32  ui4_handle,
					/* [IN] instance handle */
	UINT8   ui1_col_num,
					/* [IN] column number */
	UINT32 * pui4_row_num
					/* [OUT] row number */
);

extern INT32 i4KmGetNfKpeB64
(
	UINT32  ui4_handle,
					/* [IN]  instance handle */
	UINT8 * pui1_nf_kpe_b64
					/* [OUT] kpe base64 format, 24 bytes */
);

extern INT32 i4KmGetNfKphB64
(
	UINT32  ui4_handle,
					/* [IN]  instance handle */
	UINT8 * pui1_nf_kph_b64
					/* [OUT] kph base64 format, 44 bytes */
);

extern INT32 i4KmGetNfEsn
(
	UINT32  ui4_handle,
					/* [IN]  instance handle */
	UINT8 * pui1_nf_esn
					/* [OUT] esn, 48 bytes */
);

extern INT32 i4KmGetWmDrmPdUid
(
	UINT32  ui4_handle,
					/* [IN]  instance handle */
	UINT8 * pui1_wmdrmpd_uid
					/* [OUT] unique id, 16 bytes */
);

extern INT32 i4KmGetWmDrmPdPriData
(
	UINT32  ui4_handle,
					/* [IN]  instance handle */
	UINT8 * pui1_wmdrmpd_pri_data
					/* [OUT] private data, 40 bytes */
);

extern INT32 i4KmGetFwVfyInfoSz
(
	UINT32   ui4_handle,
					/* [IN]  instance handle */
	UINT32 * pui4_fw_vfy_info_sz
					/* [OUT] size of firmware verification information size */
);

extern INT32 i4KmGetFwVfyInfo
(
	UINT32  ui4_handle,
					/* [IN]  instance handle */
	UINT32  ui4_buffer_sz,
					/* [IN]  size of buffer */
	UINT8 * pui1_buffer
					/* [OUT] buffer for firmware verification information */
);

extern INT32 i4KmGetCusInfo1
(
	UINT32  ui4_handle,
					/* [IN]  instance handle */
	UINT8 * pui1_cus_info1
					/* [OUT] customer information 1, 32 bytes */
);

extern INT32 i4KmGetWmdrmPdDevCertTemplateSz
(
	UINT32   ui4_handle,
					/* [IN]  instance handle */
	UINT32 * pui4_sz
					/* [OUT] size of wmdrm-pd device certificate template size */
);

extern INT32 i4KmGetWmdrmPdDevCertTemplate
(
	UINT32  ui4_handle,
					/* [IN]     instance handle */
	UINT32  ui4_ofst,
					/* [IN]     byte offset in device certificate template */
	UINT32 * pui4_buffer_sz,
					/* [IN/OUT] size of buffer and read size */
	UINT8 * pui1_buffer
					/* [OUT]    buffer for wmdrm-pd device certificate template */
);

extern INT32 i4KmGetKeySt
(
	UINT32 ui4_handle,
					/* [IN]     instance handle */
	UINT32 ui4_key_type,
					/* [IN]     key type */
	BOOL * pfg_exist
					/* [IN/OUT] exist or not */
);

extern INT32 i4KmGetDtcpKf5FullCert
(
	UINT32 ui4_handle,
								/* [IN]  instance handle */
	UINT8 * pui1_full_cert
								/* [OUT] full certificate, 88 bytes */
);

extern INT32 i4KmGetDtcpKf5FullCertPrivKey
(
	UINT32 ui4_handle,
								/* [IN]  instance handle */
	UINT8 * pui1_full_cert_priv_key
								/* [OUT] full certificate private key, 20 bytes */
);

extern INT32 i4KmGetEcdKey
(
	UINT32  ui4_handle,
								/* [IN] instance handle */
	UINT32  ui4_key_type,
	UINT32  ui4_key_sz,
	UINT8  * pui1_key_data
);

extern INT32 i4KmGetCNWKey
(
	UINT32  ui4_handle,           /* [IN] instance handle */
	UINT32  ui4_key_type,
	UINT8 * pui1_key_data
);

extern INT32 i4KmGetICEKey
(
	UINT32  ui4_handle,           /* [IN] instance handle */
	UINT32  ui4_key_sz,
	UINT8 * pui1_key_data
);

extern INT32 i4KmGetKeyStatus
(
	UINT32  ui4_handle,           /* [IN] instance handle */
	UINT8 * pui1_fw_op_mode,
	UINT8 * pui1_key_status
);

extern INT32 i4KmChangeKeyBlock
(
	UINT32  ui4_handle,
								/* [IN] instance handle */
	UINT8 * pui1_key_status
);

extern INT32 i4KmGetDtcpDevPara
(
	UINT32  ui4_handle,           /* [IN] instance handle */
	UINT32  ui4_key_mode,
	UINT32  ui4_buf_sz,
	UINT8 * pui1_buf
);

extern INT32 i4KmGetWVKey
(
	UINT32  ui4_handle,           /* [IN] instance handle */
	UINT32  ui4_key_type,
	UINT32  ui4_key_sz,
	UINT8 * pui1_key_data
);

extern INT32 i4KmGetWVAssetKey
(
	UINT32  ui4_handle,           /* [IN] instance handle */
	UINT32  ui4_key_type,
	UINT16  ui2_dk_slot_handle,
	UINT8 * pui1_key_data
);

extern INT32 i4KmGetCusInfoE
(
	UINT32  ui4_handle,           /* [IN] instance handle */
	UINT32  ui4_key_type,
	UINT32  ui4_key_sz,
	UINT8 * pui1_key_data
);

extern INT32 i4KmGetHdcpRxKey
(
	UINT32  ui4_handle,           /* [IN] instance handle */
	UINT32  ui4_key_type,
	UINT32  ui4_key_sz,
	UINT8 * pui1_key_data
);

BOOL fgKmGetHdcpRxKey
(
	UINT8 * pui1_key_data         /* from BDP pool, 320Byte */
);

extern INT32 i4KmGetMarlin
(
	UINT32  ui4_handle,           /* [IN] instance handle */
	UINT32 *pui4_marlin_sz,
	UINT8 * pui1_marlin_key
);

extern INT32 i4KmGetUserKey
(
	UINT32  ui4_handle,           /* [IN] instance handle */
	UINT32  ui4_key_type,
	UINT32  ui4_ssl_sz,
	UINT16  ui2_slot_handle
);

extern INT32 i4KmGetVuduKey
(
	UINT32  ui4_handle,           /* [IN] instance handle */
	UINT32  ui4_key_sz,
	UINT8 * pui1_key_data
);

extern INT32 i4KmGetHDCP2Lc
(
	UINT32  ui4_handle,           /* [IN] instance handle */
	UINT32  ui4_key_sz,
	UINT8 * pui1_key_data
);

extern INT32 i4KmGetEncHDCP2Lc
(
	UINT32  ui4_handle,           /* [IN] instance handle */
	UINT32  ui4_key_sz,
	UINT8 * pui1_key_data
);

extern INT32 i4KmGetHDCP2Cert
(
	UINT32  ui4_handle,           /* [IN] instance handle */
	UINT32  ui4_key_sz,
	UINT8 * pui1_key_data
);

extern INT32 i4KmGetHDCP2Pri
(
	UINT32  ui4_handle,           /* [IN] instance handle */
	UINT32  ui4_key_sz,
	UINT8 * pui1_key_data
);

extern INT32 i4KmGetEncHDCP2Pri
(
	UINT32  ui4_handle,           /* [IN] instance handle */
	UINT32  ui4_key_sz,
	UINT8 * pui1_key_data
);

extern INT32 i4KmGetPlayReady
(
	UINT32  ui4_handle,           /* [IN] instance handle */
	UINT32  ui4_key_type,
	UINT32  ui4_key_sz,
	UINT8 * pui1_key_data
);

extern INT32 i4KmGetCusHDD
(
	UINT32  ui4_handle,           /* [IN] instance handle */
	UINT32  ui4_key_type,
	UINT32  ui4_key_sz,
	UINT8 * pui1_key_data
);

extern INT32 i4KmGetMCKey
(
	UINT32  ui4_handle,           /* [IN] instance handle */
	UINT32  ui4_key_type,
	UINT16  ui2_dk_slot_handle,
	UINT32  ui4_key_sz,
	UINT8  * pui1_key_data
);

extern void vKmDrvUseAacsKCD
(
	BOOL fg_use                  /* [IN] flag to use aacd kcd or not */
);

extern void vKmDrvSwSacdTxMd
(
	BOOL fg_use                  /* [IN] flag to switch sacd test mode */
);

extern void vKmDrvBurnEfuse(void);

extern UINT32 ui4KmDrvBurnCusInfo1
(
	UINT8 * pui1_cus_info_1       /* [IN] customer information 1, 32 bytes */
);

extern INT32 i4KmGetFEInfo
(
	UINT32  ui4_handle,
								/* [IN] instance handle */
	UINT8   ui1_fe_info_type,
								/* [IN] fe information type */
	UINT8   ui1_fe_info_idx,
								/* [IN] fe information index */
	UINT8 * pui1_fe_info,
		/* [OUT] fe information, depend on fe information type and index */
	UINT16  ui2_fe_info
		/* [IN/OUT] slot handle for fe information, depend on fe information type and index */
);

extern INT32 i4KmUpgKbProc
(
	UINT8 * pui1_upg_kb,
	UINT32 ui4_upg_kb_sz,
	UINT8 * pui1_ins_ck_st
);

extern INT32 i4KmEncFEInfo
(
	UINT8 * pui1_fe_info,
				/* [IN]  fe information */
	UINT8 * pui1_enc_fe_info
				/* [OUT] enc fe information */
);

extern INT32 i4KmReadChipID
(
	UINT8 * dev_nonce                    /* 16 bytes */
);

extern BOOL _km_smp_key_install_start(VOID);
extern BOOL _km_smp_key_install_reset(VOID);
extern BOOL _km_smp_key_install_end(VOID);
extern BOOL _km_smp_key_install_enc_kb_key
(
	UINT8 * pui1_buf
);
extern BOOL _km_smp_key_install_aace_dev_key
(
	UINT8 * pui1_buf
);
extern BOOL _km_smp_key_install_aace_host_cert
(
	UINT8 * pui1_buf
);
extern BOOL _km_smp_key_install_aace_dev_nonce
(
	UINT8 * pui1_buf
);
extern BOOL _km_smp_key_install_bdplus_dev_key
(
	UINT8 * pui1_buf
);
extern BOOL _km_smp_key_install_bdplus_mv_key
(
	UINT8 * pui1_buf
);
extern BOOL _km_smp_key_install_bdplus_opt_key
(
	UINT8 * pui1_buf
);
extern BOOL _km_smp_key_install_bdplus_nv_pub_key
(
	UINT8 * pui1_buf
);
extern BOOL _km_smp_key_install_cprm_dev_key
(
	UINT8 * pui1_buf
);
extern BOOL _km_smp_key_install_cppm_dev_key
(
	UINT8 * pui1_buf
);
extern BOOL _km_smp_key_install_hdcp_dev_key
(
	UINT8 * pui1_buf
);
extern BOOL _km_smp_key_install_cus_rnd_key
(
	UINT8 * pui1_buf
);
extern BOOL _km_smp_key_install_cus_s_org_key
(
	UINT8 * pui1_buf
);
extern BOOL _km_smp_key_install_wmdrm_pd_pri_dat
(
	UINT8 * pui1_buf
);
extern BOOL _km_smp_key_install_dtcp_kf5
(
	UINT8 * pui1_buf
);
extern BOOL _km_smp_key_install_marlin_dev_key
(
	UINT8 * pui1_buf,
	UINT32 ui4_sz
);
extern BOOL _km_smp_key_install_hdcp_rx_dev_key
(
	UINT8 * pui1_buf
);
extern BOOL _km_smp_key_install_spd_key
(
	UINT8 * pui1_buf
);
extern BOOL _km_smp_key_install_tpd_key
(
	UINT8 * pui1_buf
);
extern BOOL _km_smp_key_install_play_ready_zgpriv
(
	UINT8 * pui1_buf
);
extern BOOL _km_smp_key_install_play_ready_priv
(
	UINT8 * pui1_buf
);
extern BOOL _km_smp_key_install_widevine
(
	UINT8 * pui1_buf
);
extern BOOL _km_smp_key_install_cinemanow
(
	UINT8 * pui1_buf
);
extern BOOL _km_smp_key_install_nf_key
(
	UINT8 * pui1_buf
);
extern BOOL _km_smp_key_install_hdcp2rx_key
(
	UINT8 * pui1_buf
);
extern BOOL _km_smp_key_install_vup_key
(
	UINT8 * pui1_buf
);

extern void _km_smp_key_install_get_err_msg
(
	UINT8 *pui1_last_cmd,
	UINT8 *pui1_cmd_err_code,
	UINT32 *pui4_kb_gen_err_code
);

extern void vKmEncByDefNonce
(
	UINT8 *pui1_src,
	UINT8 *pui1_des,
	UINT32 ui4_sz
);

extern BOOL fgKmTestAES
(
	UINT32  ui4_key_type,
	UINT32  ui4_key_sz,
	UINT8  *pui1_key_data
);

extern BOOL fgSetRPDKey(VOID);
extern BOOL fgFreeRPDKey(VOID);

#ifdef __cplusplus
}
#endif

#endif /* #ifndef _DRV_KM_H_ */

