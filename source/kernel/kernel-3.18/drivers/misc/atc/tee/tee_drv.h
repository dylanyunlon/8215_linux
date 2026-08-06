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

#include <linux/types.h>

struct smc_param {
	unsigned int cmd_id;
	unsigned int ret;
	unsigned int data[0];
};

struct resvd_mem_info {
	phys_addr_t  base;
	void        *virt_addr;
	phys_addr_t  size;
	
};

extern struct resvd_mem_info tz_share_rsv_mem;

typedef enum TEE_DRV_SMC_CALL_TYPE
{    
    TEE_SMC_CALL_Efuse_GetChipFeature,
    TEE_SMC_CALL_Efuse_FeatureInit,
    TEE_SMC_CALL_HDCP_DECRYPT_KEY,
    TEE_SMC_CALL_HDCP2_SetEncDcp2Key,
    TEE_SMC_CALL_HDCP2_GetCertInfo,
    TEE_SMC_CALL_HDCP2_GetKsXorLc128,
    TEE_SMC_CALL_HDCP2_DecryptRSAESOAEP,
    TEE_SMC_CALL_HDCP2_2_KdKeyDerivation,
    TEE_SMC_CALL_HDCP2_KdKeyDerivation,
    TEE_SMC_CALL_HDCP2_ComputeHprime,
    TEE_SMC_CALL_HDCP2_2_ComputeHprime,
    TEE_SMC_CALL_HDCP2_2_ComputeLprime,
    TEE_SMC_CALL_HDCP2_ComputeLprime,
    TEE_SMC_CALL_HDCP2_ComputeKh,
    TEE_SMC_CALL_HDCP2_EncryptKmUsingKh,
    TEE_SMC_CALL_HDCP2_DecryptKmUsingKh,
    TEE_SMC_CALL_HDCP2_DecryptEKs,
    TEE_SMC_CALL_HDCP2_2_DecryptEKs,
    TEE_SMC_CALL_HDCP2_DataDecrypt,
    TEE_SMC_CALL_Efuse_UNKNOWN,
}TEE_DRV_SMC_CALL_TYPE;

int tee_smc_call(unsigned long u4physAddr);

extern int LoadHDCPKeyToSRAM(unsigned char* data, unsigned int len);