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

/*-----------------------------------------------------------------------------
                    include files
 ----------------------------------------------------------------------------*/
 
 
#include <linux/init.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/miscdevice.h>
 
#include <linux/err.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/input.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <linux/clk.h>
#include <linux/io.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/delay.h>
#include <asm/pgtable.h>
#include <linux/vmalloc.h>
#include <linux/mm.h>
#include <linux/sched.h>
 
#include <linux/semaphore.h>
#include <linux/platform_device.h>
 
#include <linux/of.h>
#include <linux/of_address.h>
 
#include "x_typedef.h"
#include "oal.h"
#include "windev.h"
#include "hdcp2x_drv.h"
#include "hdcp2x_drv_if.h"
#include <x_ver.h>
#include "metazone.h"

#if 0
#define CIPHSV_HDCP_SW_ALIGN_SZ(sz, align) (((sz + align - 1) / align ) * align)
#define CIPHSV_HDCP_SEC_SW_ALIGN_32(sz)        CIPHSV_HDCP_SW_ALIGN_SZ(sz, 32)

/* Determine to use memory phase 3 related functions or not */
#if CONFIG_SYS_MEM_PHASE3
#define ciphsv_hdcp_sw_aligned_mem_alloc(sz, align) x_mem_aligned_alloc(sz, align)
#define ciphsv_hdcp_sw_aligned_mem_free(ptr)        x_mem_aligned_free(ptr)
#else
#define ciphsv_hdcp_sw_aligned_mem_alloc(sz, align) x_alloc_aligned_mem(sz, align)
#define ciphsv_hdcp_sw_aligned_mem_free(ptr)        x_free_aligned_mem(ptr)
#endif
#endif

#define ENCRYPT_KEY_2_X_LENGTH      (912)
#define ENCRYPT_KEY_1_4_LENGTH      (320)
#define COMBINATION_KEY_2_X         (0x2)

#if 0
extern int TZ_HDCP2_SetEncDcp2Key(unsigned char *penckey, unsigned int len);

extern int TZ_HDCP2_GetCertInfo(unsigned char *pdata, unsigned int len);

extern int TZ_HDCP2_GetKsXorLc128(unsigned char *pKsXorLc128, unsigned int len);

extern int TZ_HDCP2_DecryptRSAESOAEP (unsigned char *pEkpub_km, unsigned int len);

extern int TZ_HDCP2_2_KdKeyDerivation (unsigned char *pRtx, unsigned int Rtx_len, unsigned char *pRrx, unsigned int Rrx_len);

extern int TZ_HDCP2_KdKeyDerivation (unsigned char *pRtx, unsigned int len);

extern int TZ_HDCP2_ComputeHprime (unsigned char* data, unsigned int len);

extern int TZ_HDCP2_2_ComputeHprime (unsigned char* data, unsigned int len);

extern int TZ_HDCP2_2_ComputeLprime (unsigned char* data, unsigned int len);

extern int TZ_HDCP2_ComputeLprime(unsigned char* data, unsigned int len);
extern int TZ_HDCP2_ComputeKh (void);

extern int TZ_HDCP2_EncryptKmUsingKh (unsigned char *pEkh_km, 
                                      unsigned int pEkh_km_len,
                                      unsigned char *pM,
                                      unsigned int pM_len);

extern int TZ_HDCP2_DecryptKmUsingKh (unsigned char *pM, 
                                      unsigned int pM_len,
                                      unsigned char *ekh_km,
                                      unsigned int ekh_km_len);

extern int TZ_HDCP2_DecryptEKs(unsigned char* data, unsigned int len);

extern int TZ_HDCP2_2_DecryptEKs (unsigned char* data, unsigned int len);
#endif
static BOOL  isKey2xExisted(UINT32 u4Idx)
{
    UINT32 u4Data = 0;
    UINT32 ret = sizeof(UINT32);
    //MTZ_Init();
    if (ret != MetaZone_ReadBinary(u4Idx, (BYTE *)&u4Data,sizeof(UINT32)))
    {
        return FALSE;
    }    
    if (u4Data & COMBINATION_KEY_2_X)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

static BOOL ReadKeyBlockFromMZ(UINT32 u4Idx, BYTE* pbData, UINT32 u4Size)
{
	/*UINT32 u4Len = 0;*/
	UINT32 ret = 0;
	UINT32 loop = 0;
	UINT32 u4LoopCount = u4Size / MZ_FS_BINARY_SIZE;
	UINT32 u4ResidueLen = u4Size - (u4LoopCount * MZ_FS_BINARY_SIZE); 
	if(u4Size != (ENCRYPT_KEY_2_X_LENGTH + ENCRYPT_KEY_1_4_LENGTH)) {
		return FALSE;
	}
	if(!pbData) {
		return FALSE;
	}
    
	while(loop < u4LoopCount) {
		ret = MetaZone_ReadBinary(u4Idx + loop, pbData + (loop * MZ_FS_BINARY_SIZE), MZ_FS_BINARY_SIZE);
		if(ret == 0) {
		    return FALSE;

		}
		loop++;
	}

	if (u4ResidueLen > 0) {
		ret = MetaZone_ReadBinary(u4Idx + u4LoopCount, pbData + (u4LoopCount * MZ_FS_BINARY_SIZE), u4ResidueLen);
		if(ret == 0) {
		    return FALSE;
		}
	}

	return TRUE;
}


INT32 _i4DRV_HDCP2X_SetEncKey (unsigned long arg)
{
    int i4Ret = 0;
    HDCP2X_IOCTL_2ARG_T* rArg=NULL;

    UINT8 bData[ENCRYPT_KEY_1_4_LENGTH + ENCRYPT_KEY_2_X_LENGTH];
    
	if (!isKey2xExisted(MZ_HDCPKEY_BIN_IDX_START)) {
		printk("[HDCP_DRV]HDCP key not existed\r\n");
        return -EFAULT;
	}

    if (!ReadKeyBlockFromMZ(MZ_HDCPKEY_BIN_IDX_START, bData, ENCRYPT_KEY_1_4_LENGTH + ENCRYPT_KEY_2_X_LENGTH)){
        return -EFAULT;
    }
    UINT8 *pData = &bData[ENCRYPT_KEY_1_4_LENGTH];

   // i4Ret = TZ_HDCP2_SetEncDcp2Key(pData, ENCRYPT_KEY_2_X_LENGTH);

    if (copy_to_user((void*)&(rArg->i4Ret), (const void*)&(i4Ret), sizeof(int)))
    {
        return -EFAULT;
    }
    return 0;
}

INT32 _i4DRV_HDCP2X_GetCertInfo (unsigned long arg)
{
    int i4Ret = 0;
    HDCP2X_KEYCERT_T pDcp2Cert;
    HDCP2X_IOCTL_2ARG_T* rArg;
    
    rArg = (HDCP2X_IOCTL_2ARG_T __user *)arg;
    if (NULL == rArg)
    {
        ASSERT(0);
        return -EFAULT;
    }
                
    // TODO: 
    //i4Ret = TZ_HDCP2_GetCertInfo((unsigned char*)pDcp2Cert.au1RxID, sizeof(HDCP2X_KEYCERT_T));
    
    if (copy_to_user((void*)&(rArg->i4Ret), (const void*)&(i4Ret), sizeof(int)))
    {
        ASSERT(0);
        return -EFAULT;
    }

    if (copy_to_user((void*)(rArg->ai4Arg[0]), pDcp2Cert.au1RxID, 522))
    {
        ASSERT(0);
        return -EFAULT;
    }
    return 0;
}


INT32 _i4DRV_HDCP2X_DecryptRSAESOAEP (unsigned long arg)
{
    int i4Ret = 0;
    HDCP2X_IOCTL_2ARG_T* rArg;
    HDCP2X_EKPUBKM_T pEkpub_km;

    rArg = (HDCP2X_IOCTL_2ARG_T __user *)arg;
    if (NULL == rArg)
    {
        ASSERT(0);
        return -EFAULT;
    }
                
    if (copy_from_user(&pEkpub_km, (void*)(rArg->ai4Arg[0]), sizeof(HDCP2X_EKPUBKM_T)))
    {
        ASSERT(0);
        return -EFAULT;
    }
	
    // TODO: 
	
	//i4Ret = TZ_HDCP2_DecryptRSAESOAEP(pEkpub_km.au1Ekpubkm, HDCP2X_KEYLEN_EKPUB_KM);

    if (copy_to_user((void*)&(rArg->i4Ret), (const void*)&(i4Ret), sizeof(int)))
    {
        ASSERT(0);
        return -EFAULT;
    }
    return 0;
}


INT32 _i4DRV_HDCP2X_KdKeyDerivation (unsigned long arg)
{
    int i4Ret = 0;
    HDCP2X_IOCTL_2ARG_T* rArg;
    HDCP2X_RTX_T pRtx;
    
    rArg = (HDCP2X_IOCTL_2ARG_T __user *)arg;
    if (NULL == rArg)
    {
        ASSERT(0);
        return -EFAULT;
    }
                
    if (copy_from_user(&pRtx, (void*)(rArg->ai4Arg[0]), sizeof(HDCP2X_RTX_T)))
    {
        ASSERT(0);
        return -EFAULT;
    }

    // TODO: 
    //i4Ret = TZ_HDCP2_KdKeyDerivation((unsigned char*)&pRtx, HDCP2X_KEYLEN_RTX);

    if (copy_to_user((void*)&(rArg->i4Ret), (const void*)&(i4Ret), sizeof(int)))
    {
        ASSERT(0);
        return -EFAULT;
    }
    return 0;
}


INT32 _i4DRV_HDCP2X_ComputeHprime (unsigned long arg)
{
    int i4Ret = 0;
    HDCP2X_IOCTL_2ARG_T* rArg;
    HDCP2X_CALHPRIME_T pCalHprime;
    
    rArg = (HDCP2X_IOCTL_2ARG_T __user *)arg;
    if (NULL == rArg)
    {
        ASSERT(0);
        return -EFAULT;
    }
                
    if (copy_from_user(&pCalHprime, (void*)(rArg->ai4Arg[0]), sizeof(HDCP2X_CALHPRIME_T)))
    {
        ASSERT(0);
        return -EFAULT;
    }

    // TODO: 
    //i4Ret = TZ_HDCP2_ComputeHprime((unsigned char*)&pCalHprime, sizeof(HDCP2X_CALHPRIME_T));
    if (copy_to_user((void*)&(rArg->i4Ret), (const void*)&(i4Ret), sizeof(int)))
    {
        ASSERT(0);
        return -EFAULT;
    }

    if (copy_to_user((void*)(rArg->ai4Arg[0]), (void*)&pCalHprime, sizeof(HDCP2X_CALHPRIME_T)))
    {
        ASSERT(0);
        return -EFAULT;
    }
    return 0;
}


INT32 _i4DRV_HDCP2X_ComputeLprime (unsigned long arg)
{
    int i4Ret = 0;
    HDCP2X_IOCTL_2ARG_T* rArg;
    HDCP2X_CALLPRIME_T pCalLprime;
        
    rArg = (HDCP2X_IOCTL_2ARG_T __user *)arg;
    if (NULL == rArg)
    {
        ASSERT(0);
        return -EFAULT;
    }
                
    if (copy_from_user(&pCalLprime, (void*)(rArg->ai4Arg[0]), sizeof(HDCP2X_CALLPRIME_T)))
    {
        ASSERT(0);
        return -EFAULT;
    }

    // TODO: 
    //i4Ret = TZ_HDCP2_ComputeLprime((unsigned char *)&pCalLprime, sizeof(HDCP2X_CALLPRIME_T));

    if (copy_to_user((void*)&(rArg->i4Ret), (const void*)&(i4Ret), sizeof(int)))
    {
        ASSERT(0);
        return -EFAULT;
    }

    if (copy_to_user((void*)(rArg->ai4Arg[0]), (void*)&pCalLprime, sizeof(HDCP2X_CALLPRIME_T)))
    {
        ASSERT(0);
        return -EFAULT;
    }
    return 0;
}


INT32 _i4DRV_HDCP2X_ComputeKh (unsigned long arg)
{
    //TZ_HDCP2_ComputeKh();
    return 0;
}


INT32 _i4DRV_HDCP2X_EncryptKmUsingKh (unsigned long arg)
{
    int i4Ret = 0;
    HDCP2X_IOCTL_2ARG_T* rArg;
    UINT8 pEkh_km[16];
    UINT8 pM[16];
    
    rArg = (HDCP2X_IOCTL_2ARG_T __user *)arg;
    if (NULL == rArg)
    {
        ASSERT(0);
        return -EFAULT;
    }
                
    if (copy_from_user(&pEkh_km, (void*)(rArg->ai4Arg[0]), 16))
    {
        ASSERT(0);
        return -EFAULT;
    }

    if (copy_from_user(&pM, (void*)(rArg->ai4Arg[1]), 16))
    {
        ASSERT(0);
        return -EFAULT;
    }
    
    // TODO: 
    //i4Ret = DRV_HDCP2_EncryptKmUsingKh(pEkh_km,pM);

    //i4Ret = TZ_HDCP2_EncryptKmUsingKh(pEkh_km, 16, pM, 16);

    if (copy_to_user((void*)&(rArg->i4Ret), (const void*)&(i4Ret), sizeof(int)))
    {
        ASSERT(0);
        return -EFAULT;
    }
    return 0;
}


INT32 _i4DRV_HDCP2X_DecryptKmUsingKh (unsigned long arg)
{
    int i4Ret = 0;
    HDCP2X_IOCTL_2ARG_T* rArg;
    UINT8 ekh_km[16];
    UINT8 pM[16];
        
    rArg = (HDCP2X_IOCTL_2ARG_T __user *)arg;
    if (NULL == rArg)
    {
        ASSERT(0);
        return -EFAULT;
    }
                
    if (copy_from_user(&pM, (void*)(rArg->ai4Arg[0]), 16))
    {
        ASSERT(0);
        return -EFAULT;
    }

    if (copy_from_user(&ekh_km, (void*)(rArg->ai4Arg[1]), 16))
    {
        ASSERT(0);
        return -EFAULT;
    }
    
    // TODO: 
    //i4Ret = TZ_HDCP2_DecryptKmUsingKh(pM, 16, ekh_km, 16);

    if (copy_to_user((void*)&(rArg->i4Ret), (const void*)&(i4Ret), sizeof(int)))
    {
        ASSERT(0);
        return -EFAULT;
    }
    return 0;
}


INT32 _i4DRV_HDCP2X_DecryptEKs (unsigned long arg)
{
    int i4Ret = 0;
    HDCP2X_IOCTL_4ARG_T* rArg;

    HDCP2X_EKS_T pstEks;        
       
    rArg = (HDCP2X_IOCTL_4ARG_T __user *)arg;
    if (NULL == rArg)
    {
        ASSERT(0);
        return -EFAULT;
    }
                
    if (copy_from_user(&(pstEks.au1Eks), (void*)(rArg->ai4Arg[0]), HDCP2X_KEYLEN_EKS))
    {
        ASSERT(0);
        return -EFAULT;
    }

    if (copy_from_user(&(pstEks.au1Rtx), (void*)(rArg->ai4Arg[1]), HDCP2X_KEYLEN_RTX))
    {
        ASSERT(0);
        return -EFAULT;
    }

    if (copy_from_user(&(pstEks.au1Rrx), (void*)(rArg->ai4Arg[2]), HDCP2X_KEYLEN_RRX))
    {
        ASSERT(0);
        return -EFAULT;
    }

    if (copy_from_user(&(pstEks.au1Rn), (void*)(rArg->ai4Arg[3]), HDCP2X_KEYLEN_RRN))
    {
        ASSERT(0);
        return -EFAULT;
    }
    // TODO: 
    //i4Ret = TZ_HDCP2_DecryptEKs((unsigned char *)&pstEks, sizeof(HDCP2X_EKS_T));
    if (copy_to_user((void*)&(rArg->i4Ret), (const void*)&(i4Ret), sizeof(int)))
    {
        ASSERT(0);
        return -EFAULT;
    }
    return 0;
}



INT32 _i4DRV_HDCP2X_DataDecrypt (unsigned long arg)
{
#if 0
    int i4Ret = CIPHSVR_OK;
    HDCP2X_IOCTL_6ARG_T* rArg;

    //rArg.ai4Arg[0] = (INT32)pulStreamCounter;
    //rArg.ai4Arg[1] = (INT32)pullInputCounter;
    //rArg.ai4Arg[2] = (INT32)ucSrcFrame;
    //rArg.ai4Arg[3] = (INT32)ulCount;
    //rArg.ai4Arg[4] = (INT32)ucDstFrame;
    //rArg.ai4Arg[5] = (INT32)0;
    
    unsigned long pulStreamCounter;
    unsigned long long pullInputCounter;
    unsigned long ulCount;
    char* pcKrnSegBuf = NULL;




					
    rArg = (HDCP2X_IOCTL_6ARG_T __user *)arg;
    if (NULL == rArg)
    {
        ASSERT(0);
        return -EFAULT;
    }
                
    if (copy_from_user(&pulStreamCounter, (void*)(rArg->ai6Arg[0]), sizeof(unsigned long)))
    {
        ASSERT(0);
        return -EFAULT;
    }

    if (copy_from_user(&pullInputCounter, (void*)(rArg->ai6Arg[1]), sizeof(unsigned long long)))
    {
        ASSERT(0);
        return -EFAULT;
    }

    if (copy_from_user(&ulCount, (void*)&(rArg->ai6Arg[3]), sizeof(unsigned long)))
    {
        ASSERT(0);
        return -EFAULT;
    }

    if (CIPHSV_AVAIL_CONT_MEM_SZ <= ulCount)
        pcKrnSegBuf = ciphsv_contmem_alloc(CIPHSV_AVAIL_CONT_MEM_SZ);
    else
        pcKrnSegBuf = ciphsv_contmem_alloc(ulCount); 
    if (NULL == pcKrnSegBuf)
    {
        ASSERT(0);
        return -EFAULT;
    }

    if (copy_from_user(pcKrnSegBuf, (void*)(rArg->ai6Arg[2]), ulCount))
    {
        ASSERT(0);
        ciphsv_contmem_free(pcKrnSegBuf);
        pcKrnSegBuf = NULL;
        return -EFAULT;
    }
    // TODO: 

	

	//SYS_Printf("do _i4CIPHSV_HDCP2_DataDecrypt\n");

/*
	_i4CIPHSV_HDCP2_TZ_DataDecrypt(pulStreamCounter,	//		UINT32 pulStreamCounter,
										pullInputCounter0,
										pullInputCounter1,
										pcKrnSegBuf,
										(UINT8 *)&ulCount,
										pcKrnSegBuf);
*/


   i4Ret = DRV_HDCP2_DataDecrypt(&pulStreamCounter, &pullInputCounter, pcKrnSegBuf, ulCount, pcKrnSegBuf);

    if (copy_to_user((void*)&(rArg->i4Ret), (const void*)&(i4Ret), sizeof(int)))
    {
        ASSERT(0);
        return -EFAULT;
    }
    
    if (copy_to_user((void*)(rArg->ai6Arg[4]), pcKrnSegBuf, ulCount))
    {
        ASSERT(0);
        ciphsv_contmem_free(pcKrnSegBuf);
        pcKrnSegBuf = NULL;
        return -EFAULT;
    }

    ciphsv_contmem_free(pcKrnSegBuf);
#endif    
    return 0;
}


INT32 _i4DRV_HDCP2X_GetKsXorLc128 (unsigned long arg)
{
    int i4Ret = 0;
    HDCP2X_IOCTL_2ARG_T* rArg;
    UINT8 pKsXorLc128[16];
        
    rArg = (HDCP2X_IOCTL_2ARG_T __user *)arg;
    if (NULL == rArg)
    {
        ASSERT(0);
        return -EFAULT;
    }
                
    // TODO: 
    //_i4CIPHSV_HDCP2_TZ_SetRiv(_riv);
	//i4Ret = TZ_HDCP2_GetKsXorLc128((unsigned char *)&pKsXorLc128, 16);

    if (copy_to_user((void*)&(rArg->i4Ret), (const void*)&(i4Ret), sizeof(int)))
    {
        ASSERT(0);
        return -EFAULT;
    }    
    if (copy_to_user((void*)(rArg->ai4Arg[0]), pKsXorLc128, 16))
    {
        ASSERT(0);
        return -EFAULT;
    }
    return 0;
}






INT32 _i4DRV_HDCP2_2_DecryptEKs (unsigned long arg)
{
    int i4Ret = 0;
    HDCP2X_IOCTL_4ARG_T* rArg;

    HDCP2X_EKS_T pstEks;        
    rArg = (HDCP2X_IOCTL_4ARG_T __user *)arg;
    if (NULL == rArg)
    {
        ASSERT(0);
        return -EFAULT;
    }
                
    if (copy_from_user(&(pstEks.au1Eks), (void*)(rArg->ai4Arg[0]), HDCP2X_KEYLEN_EKS))
    {
        ASSERT(0);
        return -EFAULT;
    }

    if (copy_from_user(&(pstEks.au1Rtx), (void*)(rArg->ai4Arg[1]), HDCP2X_KEYLEN_RTX))
    {
        ASSERT(0);
        return -EFAULT;
    }

    if (copy_from_user(&(pstEks.au1Rrx), (void*)(rArg->ai4Arg[2]), HDCP2X_KEYLEN_RRX))
    {
        ASSERT(0);
        return -EFAULT;
    }

    if (copy_from_user(&(pstEks.au1Rn), (void*)(rArg->ai4Arg[3]), HDCP2X_KEYLEN_RRN))
    {
        ASSERT(0);
        return -EFAULT;
    }
    // TODO: 
    //i4Ret = DRV_HDCP2_DecryptEKs(pEks, pRtx, pRrx, pRn);
    
    //i4Ret = TZ_HDCP2_2_DecryptEKs((unsigned char *)&pstEks, sizeof(HDCP2X_EKS_T));
    if (copy_to_user((void*)&(rArg->i4Ret), (const void*)&(i4Ret), sizeof(int)))
    {
        ASSERT(0);
        return -EFAULT;
    }
    return 0;
}




INT32 _i4DRV_HDCP2_2_KdKeyDerivation (unsigned long arg)
{
    int i4Ret = 0;
    HDCP2X_IOCTL_2ARG_T* rArg;
    HDCP2X_RTX_T pRtx;

	HDCP2X_RRX_T pRrx;
    
    rArg = (HDCP2X_IOCTL_2ARG_T __user *)arg;
    if (NULL == rArg)
    {
        ASSERT(0);
        return -EFAULT;
    }
                
    if (copy_from_user(&pRtx, (void*)(rArg->ai4Arg[0]), sizeof(HDCP2X_RTX_T)))
    {
        ASSERT(0);
        return -EFAULT;
    }

    if (copy_from_user(&pRrx, (void*)(rArg->ai4Arg[1]), sizeof(HDCP2X_RRX_T)))
    {
        ASSERT(0);
        return -EFAULT;
    }

    // TODO: 
    //i4Ret = DRV_HDCP2_KdKeyDerivation((unsigned char*)&pRtx,sizeof(CIPHSV_HDCP2_RTX_T));

    //i4Ret = TZ_HDCP2_2_KdKeyDerivation((unsigned char *)&pRtx, sizeof(HDCP2X_RTX_T), (unsigned char *)&pRrx, sizeof(HDCP2X_RRX_T));

    if (copy_to_user((void*)&(rArg->i4Ret), (const void*)&(i4Ret), sizeof(int)))
    {
        ASSERT(0);
        return -EFAULT;
    }
    return 0;
}


INT32 _i4DRV_HDCP2_2_ComputeHprime (unsigned long arg)
{
    int i4Ret = 0;
    HDCP2X_IOCTL_2ARG_T* rArg;
    HDCP2X_CALHPRIME_T pCalHprime;
    
    rArg = (HDCP2X_IOCTL_2ARG_T __user *)arg;
    if (NULL == rArg)
    {
        ASSERT(0);
        return -EFAULT;
    }
                
    if (copy_from_user(&pCalHprime, (void*)(rArg->ai4Arg[0]), sizeof(HDCP2X_CALHPRIME_T)))
    {
        ASSERT(0);
        return -EFAULT;
    }

    // TODO: 
    //i4Ret = DRV_HDCP2_ComputeHprime((unsigned char*)pCalHprime.au1rHprime, pCalHprime.u1Repeater, (unsigned char*)pCalHprime.au1rTx);

    //i4Ret = TZ_HDCP2_2_ComputeHprime((unsigned char *)&pCalHprime, sizeof(HDCP2X_CALHPRIME_T));
	
    if (copy_to_user((void*)&(rArg->i4Ret), (const void*)&(i4Ret), sizeof(int)))
    {
        ASSERT(0);
        return -EFAULT;
    }

    if (copy_to_user((void*)(rArg->ai4Arg[0]), (void*)&pCalHprime, sizeof(HDCP2X_CALHPRIME_T)))
    {
        ASSERT(0);
        return -EFAULT;
    }
    return 0;
}


INT32 _i4DRV_HDCP2_2_ComputeLprime (unsigned long arg)
{
    int i4Ret = 0;
    HDCP2X_IOCTL_2ARG_T* rArg;
    HDCP2X_CALLPRIME_T pCalLprime;
    
    rArg = (HDCP2X_IOCTL_2ARG_T __user *)arg;
    if (NULL == rArg)
    {
        ASSERT(0);
        return -EFAULT;
    }
                
    if (copy_from_user(&pCalLprime, (void*)(rArg->ai4Arg[0]), sizeof(HDCP2X_CALLPRIME_T)))
    {
        ASSERT(0);
        return -EFAULT;
    }

    // TODO: 
    //i4Ret = TZ_HDCP2_2_ComputeLprime((unsigned char *)&pCalLprime, sizeof(HDCP2X_CALLPRIME_T));

    if (copy_to_user((void*)&(rArg->i4Ret), (const void*)&(i4Ret), sizeof(int)))
    {
        ASSERT(0);
        return -EFAULT;
    }

    if (copy_to_user((void*)(rArg->ai4Arg[0]), (void*)&pCalLprime, sizeof(HDCP2X_CALLPRIME_T)))
    {
        ASSERT(0);
        return -EFAULT;
    }
    return 0;
}



INT32 _i4DRV_HDCP2X_CmdHandle(unsigned int u4Cmd, unsigned long arg)
{
  switch(u4Cmd)
  {
    case IOCTL_HDCP2X_CMD_SET_ENC_KEY:
        return _i4DRV_HDCP2X_SetEncKey(arg);
    case IOCTL_HDCP2X_CMD_GETCERTINFO:
        return _i4DRV_HDCP2X_GetCertInfo(arg);
    case IOCTL_HDCP2X_CMD_DECRYPT_RSAESOAEP:
        return _i4DRV_HDCP2X_DecryptRSAESOAEP(arg);
    case IOCTL_HDCP2X_CMD_KDKEYDERIVATION:
        return _i4DRV_HDCP2X_KdKeyDerivation(arg);
    case IOCTL_HDCP2X_CMD_COMPUTE_HPRIME:
        return _i4DRV_HDCP2X_ComputeHprime(arg);
    case IOCTL_HDCP2X_CMD_COMPUTE_LPRIME:
        return _i4DRV_HDCP2X_ComputeLprime(arg);
    case IOCTL_HDCP2X_CMD_COMPUTE_KH:
        return _i4DRV_HDCP2X_ComputeKh(arg);
    case IOCTL_HDCP2X_CMD_ENCRYPT_KM:
        return _i4DRV_HDCP2X_EncryptKmUsingKh(arg);
    case IOCTL_HDCP2X_CMD_DECRYPT_KM:
        return _i4DRV_HDCP2X_DecryptKmUsingKh(arg);
    case IOCTL_HDCP2X_CMD_DECRYPT_EKS:
        return _i4DRV_HDCP2X_DecryptEKs(arg);
    case IOCTL_HDCP2X_CMD_DECRYPT_PES:
        return _i4DRV_HDCP2X_DataDecrypt(arg);
    case IOCTL_HDCP2X_CMD_GET_KSXORLC128:
        return _i4DRV_HDCP2X_GetKsXorLc128(arg);
    case IOCTL_HDCP2X_CMD_HDCP2_2_DECRYPT_EKS:
        return _i4DRV_HDCP2_2_DecryptEKs(arg);
	case IOCTL_HDCP2X_CMD_HDCP2_2_KDKEYDERIVATION:
		return _i4DRV_HDCP2_2_KdKeyDerivation(arg);
	case IOCTL_HDCP2X_CMD_HDCP2_2_COMPUTE_HPRIME:
		return _i4DRV_HDCP2_2_ComputeHprime(arg);
	case IOCTL_HDCP2X_CMD_HDCP2_2_COMPUTE_LPRIME:	 
		return _i4DRV_HDCP2_2_ComputeLprime(arg);
	
    default:
        return 0;
      //ASSERT(0);
  }

  return 0;
}
