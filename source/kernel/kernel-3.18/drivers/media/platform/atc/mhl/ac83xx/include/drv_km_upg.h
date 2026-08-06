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

#ifndef _DRV_KM_UPG_H_
#define _DRV_KM_UPG_H_

#include "x_typedef.h"
#include "u_km_ext.h"

#ifdef __cplusplus
extern "C" {
#endif


/*-----------------------------------------------------------------------------
 * Name: fgKmChkKbAva
 *
 * Description: Validate if key block is available
 *
 * Inputs:  -
 *
 * Outputs: -
 *
 * Returns: TRUE             success to process key block
 *          FALSE            fail to process key block
 *
 ----------------------------------------------------------------------------*/
extern BOOL fgKmChkKbAva(VOID);

/*-----------------------------------------------------------------------------
 * Name: fgKmDirUpgKb
 *
 * Description: Upgrade key block directly
 *
 * Inputs:  pui1_kb   upgrade key block data
 *          ui4_sz    upgrade key block size
 *
 * Outputs: -
 *
 * Returns: TRUE             success to upgrade key block
 *          FALSE            fail to upgrade key block
 *
 ----------------------------------------------------------------------------*/
extern BOOL fgKmDirUpgKb
(
	UINT8 * pui1_kb,
	UINT32  ui4_sz
);

/*-----------------------------------------------------------------------------
* Name: _km_smp_key_idx
*
* Description: calculate mp key index
*
* Inputs:  pui1_mp_key_flag    encrypted key block key (8 bytes)
*
* Outputs: -
*
* Returns: Valid Index   != 0xffffffff
*          Invalid Index == 0xffffffff
*
----------------------------------------------------------------------------*/
extern UINT8 _km_smp_key_idx
(
	UINT8 *pui1_key_flag
);

/*-----------------------------------------------------------------------------
* Name: _km_smp_key_install_start
*
* Description: Start mp key installing process
*
* Inputs:  -
*
* Outputs: -
*
* Returns: TRUE           success
*          FALSE          fail
*
----------------------------------------------------------------------------*/
extern BOOL _km_smp_key_install_start(VOID);

/*-----------------------------------------------------------------------------
* Name: _km_smp_key_install_reset
*
* Description: Reset mp key installing process
*
* Inputs:  -
*
* Outputs: -
*
* Returns: TRUE           success
*          FALSE          fail
*
----------------------------------------------------------------------------*/
extern BOOL _km_smp_key_install_reset(VOID);

/*-----------------------------------------------------------------------------
* Name: _km_smp_key_install_end
*
* Description: End mp key installing process
*
* Inputs:  -
*
* Outputs: -
*
* Returns: TRUE           success
*          FALSE          fail
*
----------------------------------------------------------------------------*/
extern BOOL _km_smp_key_install_end(VOID);

/*-----------------------------------------------------------------------------
* Name: _km_smp_key_install_enc_kb_key
*
* Description: Process encrypted key block key
*
* Inputs:  pui1_buf       encrypted key block key (12 + 16 bytes)
*
* Outputs: -
*
* Returns: TRUE           success
*          FALSE          fail
*
----------------------------------------------------------------------------*/
extern BOOL _km_smp_key_install_enc_kb_key
(
	UINT8 *pui1_buf
);

/*-----------------------------------------------------------------------------
* Name: _km_smp_key_install_aace_dev_key
*
* Description: Process aacs device key
*
* Inputs:  pui1_buf       aacs device key (12 + 8432 bytes)
*
* Outputs: -
*
* Returns: TRUE           success
*          FALSE          fail
*
----------------------------------------------------------------------------*/
extern BOOL _km_smp_key_install_aace_dev_key
(
	UINT8 *pui1_buf
);

/*-----------------------------------------------------------------------------
* Name: _km_smp_key_install_aace_host_cert
*
* Description: Process aacs host certificate
*
* Inputs:  pui1_buf       aacs host certificate key (12 + 160 bytes)
*
* Outputs: -
*
* Returns: TRUE           success
*          FALSE          fail
*
----------------------------------------------------------------------------*/
extern BOOL _km_smp_key_install_aace_host_cert
(
	UINT8 *pui1_buf
);

/*-----------------------------------------------------------------------------
* Name: _km_smp_key_install_aace_dev_nonce
*
* Description: Process aacs device nonce
*
* Inputs:  pui1_buf       aacs device nonce (12 + 16 bytes)
*
* Outputs: -
*
* Returns: TRUE           success
*          FALSE          fail
*
----------------------------------------------------------------------------*/
extern BOOL _km_smp_key_install_aace_dev_nonce
(
	UINT8 *pui1_buf
);

/*-----------------------------------------------------------------------------
* Name: _km_smp_key_install_bdplus_dev_key
*
* Description: Process bd+ device key
*
* Inputs:  pui1_buf       bd+ device key (12 + 400 bytes)
*
* Outputs: -
*
* Returns: TRUE           success
*          FALSE          fail
*
----------------------------------------------------------------------------*/
extern BOOL _km_smp_key_install_bdplus_dev_key
(
	UINT8 *pui1_buf
);

/*-----------------------------------------------------------------------------
* Name: _km_smp_key_install_bdplus_mv_key
*
* Description: Process bd+ model/version key
*
* Inputs:  pui1_buf       bd+ mv key (12 + 384 bytes)
*
* Outputs: -
*
* Returns: TRUE           success
*          FALSE          fail
*
----------------------------------------------------------------------------*/
extern BOOL _km_smp_key_install_bdplus_mv_key
(
	UINT8 *pui1_buf
);

/*-----------------------------------------------------------------------------
* Name: _km_smp_key_install_bdplus_opt_key
*
* Description: Process bd+ option key
*
* Inputs:  pui1_buf       bd+ option key (12 + 32 bytes)
*
* Outputs: -
*
* Returns: TRUE           success
*          FALSE          fail
*
----------------------------------------------------------------------------*/
extern BOOL _km_smp_key_install_bdplus_opt_key
(
	UINT8 *pui1_buf
);

/*-----------------------------------------------------------------------------
* Name: _km_smp_key_install_bdplus_nv_pub_key
*
* Description: Process bd+ native code public key
*
* Inputs:  pui1_buf       bd+ native code public key key (12 + 48 bytes)
*
* Outputs: -
*
* Returns: TRUE           success
*          FALSE          fail
*
----------------------------------------------------------------------------*/
extern BOOL _km_smp_key_install_bdplus_nv_pub_key
(
	UINT8 *pui1_buf
);

/*-----------------------------------------------------------------------------
* Name: _km_smp_key_install_cprm_dev_key
*
* Description: Process cprm device key
*
* Inputs:  pui1_buf       cprm device key (12 + 208 bytes)
*
* Outputs: -
*
* Returns: TRUE           success
*          FALSE          fail
*
----------------------------------------------------------------------------*/
extern BOOL _km_smp_key_install_cprm_dev_key
(
	UINT8 *pui1_buf
);

/*-----------------------------------------------------------------------------
* Name: _km_smp_key_install_cppm_dev_key
*
* Description: Process cppm device key
*
* Inputs:  pui1_buf       cppm device key (12 + 208 bytes)
*
* Outputs: -
*
* Returns: TRUE           success
*          FALSE          fail
*
----------------------------------------------------------------------------*/
extern BOOL _km_smp_key_install_cppm_dev_key
(
	UINT8 *pui1_buf
);

/*-----------------------------------------------------------------------------
* Name: _km_smp_key_install_hdcp_dev_key
*
* Description: Process hdcp device key
*
* Inputs:  pui1_buf       hdcp device key (12 + 320 bytes)
*
* Outputs: -
*
* Returns: TRUE           success
*          FALSE          fail
*
----------------------------------------------------------------------------*/
extern BOOL _km_smp_key_install_hdcp_dev_key
(
	UINT8 *pui1_buf
);

/*-----------------------------------------------------------------------------
* Name: _km_smp_key_install_cus_rnd_key
*
* Description: Process customer random key
*
* Inputs:  pui1_buf       customer random key (12 + 16 bytes)
*
* Outputs: -
*
* Returns: TRUE           success
*          FALSE          fail
*
----------------------------------------------------------------------------*/
extern BOOL _km_smp_key_install_cus_rnd_key
(
	UINT8 *pui1_buf
);

/*-----------------------------------------------------------------------------
* Name: _km_smp_key_install_cus_s_org_key
*
* Description: Process cusotmer s original key
*
* Inputs:  pui1_buf       customer s original key (12 + 32 bytes)
*
* Outputs: -
*
* Returns: TRUE           success
*          FALSE          fail
*
----------------------------------------------------------------------------*/
extern BOOL _km_smp_key_install_cus_s_org_key
(
	UINT8 *pui1_buf
);

/*-----------------------------------------------------------------------------
* Name: _km_smp_key_install_wmdrm_pd_pri_dat
*
* Description: Process wmdrm-pd pri.dat
*
* Inputs:  pui1_buf       wmdrm-pd pri.dat (12 + 48 bytes)
*
* Outputs: -
*
* Returns: TRUE           success
*          FALSE          fail
*
----------------------------------------------------------------------------*/
extern BOOL _km_smp_key_install_wmdrm_pd_pri_dat
(
	UINT8 *pui1_buf
);

/*-----------------------------------------------------------------------------
* Name: _km_smp_key_install_dtcp_kf5
*
* Description: Process dtcp key format 5
*
* Inputs:  pui1_buf       dtcp key format 5 (12 + 160 bytes)
*
* Outputs: -
*
* Returns: TRUE           success
*          FALSE          fail
*
----------------------------------------------------------------------------*/
extern BOOL _km_smp_key_install_dtcp_kf5
(
	UINT8 *pui1_buf
);

/*-----------------------------------------------------------------------------
* Name: _km_smp_key_install_marlin_dev_key
*
* Description: D-interface to process marlin device key
*
* Inputs:  pui1_buf       device key of marlin bb (12 + x), x<=(32KB+4)
*          ui4_sz         buffer size
*
* Outputs: -
*
* Returns: TRUE           success
*          FALSE          fail
*
----------------------------------------------------------------------------*/
extern BOOL _km_smp_key_install_marlin_dev_key
(
	UINT8 *pui1_buf,
	UINT32 ui4_sz
);

/*-----------------------------------------------------------------------------
* Name: _km_smp_key_install_get_err_msg
*
* Description: Get erro message
*
* Inputs:  pui1_last_cmd           last processed command
*          pui1_cmd_err_code       command error code
*          pui4_kb_gen_err_code    key block generation error code
*
* Outputs: -
*
* Returns: TRUE                    success
*          FALSE                   fail
*
----------------------------------------------------------------------------*/
extern void _km_smp_key_install_get_err_msg
(
	UINT8 *pui1_last_cmd,
	UINT8 *pui1_cmd_err_code,
	UINT32 * pui4_kb_gen_err_code
);

#ifdef __cplusplus
}
#endif

#endif /* #ifndef _DRV_KM_UPG_H_  */

