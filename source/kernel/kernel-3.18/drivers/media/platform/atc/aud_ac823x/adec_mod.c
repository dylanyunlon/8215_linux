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
 
#include <linux/module.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include "x_typedef.h"
#include "windev.h"
#include <linux/vmalloc.h>
#include <linux/platform_device.h>
#include <linux/miscdevice.h>
#include <linux/irq.h>
#include <linux/slab.h>
#include <linux/of_reserved_mem.h>
#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/of_address.h>
#include <linux/errno.h>
#include <linux/types.h>
#include "aud_drv.h"
#include <linux/slab.h>
#include "aud_ioctrl.h"
#include <linux/compat.h>
#include "x_audmhl_def.h"
#include "x_audin.h"
#include "aud_drv.h"
#include <linux/of_address.h>
#include "x_ioopt.h"
unsigned long io_v_base;
unsigned long io_base_aud;
unsigned long io_base_ckgen;


#ifdef CONFIG_COMPAT


#define AUD_ALIGN_SZ(u4Sz, u4Aligned) (((u4Sz) + (u4Aligned)) / (u4Aligned) * (u4Aligned))

typedef struct {
    compat_uptr_t ptrBufAddr;
    __u32   u4BufLen;
    __u64   u8Pts;
} AUD_SEND_BUF_INFO32;

typedef struct {    
    __u32   u4WritePointer; 
    __u32   u4Len;  
    compat_uptr_t   ptrFifoSA;  
    compat_uptr_t   ptrFifoEA;
} AUDIO_BUF_INFO32;

typedef struct {    
    compat_uptr_t ptrAfifoRPtr;    
    compat_uptr_t ptrAfifoWPtr;    
    compat_uptr_t ptrAfifoSA;    
    compat_uptr_t ptrAfifoEA;    
    compat_uptr_t ptrAfifoVirSA;    
    compat_uptr_t ptrAfifoVirEA;} 
AUD_POSINFO_T32;


typedef struct {
    compat_uptr_t pInBuf;   
    int InSize ;    
    compat_uptr_t pOutBuf;  
    int OutSize ;   
    unsigned int *pBytesReturned;
} WIN32_IOCTL_DATA32;

typedef struct _tagAudSeOpCmd32{
    __u32 u4OpCode;                        // Operation Command Code
    compat_uptr_t pvData;                         // Command related information data
    __u32 u4DataSize;                      // related information data size
    AUD_SE_TYPE_T  u1Type;                  // target post process type
} AUD_SE_OPCMD_T32;


typedef struct{    
    AUD_DEC_FMT_T       eAudDecFmt;   
    compat_uptr_t       prInfo;    
    AUD_DEC_PB_SPEED_TYPE_T eSpeed;
}AUD_DEC_AUDIO_PB_INFO_T32;


typedef struct _AUD_SPECTRUM_BUF_INFO_T32
{
    compat_uptr_t    u4buf;
    __u32    u4size;
    __u32    u4scalingMode;
} AUD_SPECTRUM_BUF_INFO_T32;


typedef struct _AUD_DECONLY_GET_BUF32
{
    AUD_DECONLY_DATA_ENDIAN eDataEndian;
    AUD_DECONLY_BIT_DEPTH eBitDepth;
    AUD_DECONLY_CH_CFG eChCfg;
    __u32 u4SampleRate;
    __u32 u4BufLen;
    compat_uptr_t u4BufAddr;
    __u64 u8BufPts;
    
}AUD_DECONLY_GET_BUF32;


/// Audio information of a Access unit
typedef struct
{
  __u64 u8Pts;                ///< Pts
  AUD_TYPE eAudType;          ///< 0: AC3, 1:MLP
} __attribute__ ((packed)) AudInfo32;


#define AUDIODECODER_DEVNAME "adec"
typedef struct
{
    bool fgSkipData;               ///< start skip audio data flag

    compat_uptr_t ptrSAddr;                ///< start address in fifo
    compat_uptr_t ptrEAddr;              ///< end address in fifo
    AU_TYPE eAuType;                     ///< Access unit type
  
    union
    {
        AudInfo32 rInfo;                          ///< audio data information
    } rAUInfo;
} __attribute__ ((packed)) AU_AUDIO32;


typedef struct {

    AUD_DEC_CLI_TYPE eAudCliType;

    __u32    u4arg1;//u4InputID;
    __u32    u4arg2;//u4Len;
    __u32    u4arg3;//u4Size;
    __u32    u4arg4;//u4Value;
    compat_uptr_t ptParam;//filename

}AUD_DEC_CLI_CFG32;

typedef struct _AUD_USER_INFO32
{
    compat_uptr_t puser;
    __u32 buf_size;
}AUD_USER_INFO32;
#endif

struct adec_dev_info *adec_dev;

extern u32 ADE_Init(s8 *pszContext);
extern bool ADE_Deinit(uintptr_t dwContext);
extern uintptr_t ADE_Open(uintptr_t dwContext, u32 dwAccessMode, u32 dwShareMode);
extern bool ADE_Close(uintptr_t dwContext);
extern u32 ADE_Read(uintptr_t context, void *pBuffer, u32 dwCount);
extern u32 ADE_Write(uintptr_t context, void *pBuffer, u32 dwCount);
extern bool ADE_IOControl(uintptr_t context, u32 code, u8 *pInBuffer, u32 inSize,
                  u8 *pOutBuffer, u32 outSize, u32 *pOutSize);
extern s32 ADE_Mmap(u32 dwContext, struct vm_area_struct *vma);
extern s32 get_static_reserved_memory(const char *uname, phys_addr_t *base, phys_addr_t *size);


static s32 adec_ioctl(struct file *filp, u32 cmd, uintptr_t arg)
{
    void *private_data;
    WIN32_IOCTL_DATA win_ioctl;
    bool bRet;
    void* pInBuf = NULL;
    void* pOutBuf = NULL;
    u32 dwReturn = 0;

    private_data = filp->private_data;
 
    if (!private_data)
        return -2;

    if (!access_ok(VERIFY_READ, (void __user *)arg, sizeof(win_ioctl)))
        return -3;

    if (copy_from_user((void *)&win_ioctl, (void *)arg, sizeof(win_ioctl)))
        return -4;

    //copy data path
    if(IOCTL_AUDIN_COPY_FROM_USER==cmd){
         bRet = ADE_IOControl((uintptr_t)private_data, cmd, (u8 *)win_ioctl.pInBuf, win_ioctl.InSize,
                              (u8 *)win_ioctl.pOutBuf, win_ioctl.OutSize, win_ioctl.pBytesReturned);
         return (bRet ? 0 : -1);
    }

    //user sapce pointer copy
    if(NULL != win_ioctl.pInBuf&& 0 != win_ioctl.InSize){
        if(NULL == (pInBuf = kzalloc(win_ioctl.InSize, GFP_KERNEL))){
            return -5;
        }
        if(copy_from_user(pInBuf, win_ioctl.pInBuf, win_ioctl.InSize)){
            return -6;
        }
    }

    if(NULL != win_ioctl.pOutBuf&& 0 != win_ioctl.OutSize){
        if(NULL == (pOutBuf = kzalloc(win_ioctl.OutSize, GFP_KERNEL))){
            return -7;
        }
        if(copy_from_user(pOutBuf, win_ioctl.pOutBuf, win_ioctl.OutSize)){
         return -8;
        }
    }

    bRet = ADE_IOControl((uintptr_t)private_data, cmd, (u8 *)pInBuf, win_ioctl.InSize, (u8 *)pOutBuf, win_ioctl.OutSize, &dwReturn);

    if(NULL != pInBuf){
        kfree(pInBuf);
        pInBuf = NULL;
    }
    if(NULL != pOutBuf){
        if(copy_to_user(win_ioctl.pOutBuf, pOutBuf, win_ioctl.OutSize)){
            return -9;
        }        
        kfree(pOutBuf);
        pOutBuf = NULL;
    }

    if(NULL != win_ioctl.pBytesReturned){
        if(copy_to_user(win_ioctl.pBytesReturned, &dwReturn, sizeof(dwReturn))){
            pr_info("[aud] win_ioctl.pBytesReturned != NULL \n");
            return -10;
        }
    }

    return (bRet ? 0 : -1);
}

#ifdef CONFIG_COMPAT
static s32 do_adec_ioctrl(struct file *filp, u32 cmd, WIN32_IOCTL_DATA *kp, WIN32_IOCTL_DATA32 *up )
{
    void __user *inputBuf;
    void __user *inputBuf32;
    void __user *inputBuf32_2;
    void __user *outputBuf;
    void __user *outputBuf32;
    void __user *u4BufAddr32;
    compat_uptr_t temp;
    int32_t i4ret = 0;
    u32 __user *pBytesReturned;
    u32 __user *pBytesReturned32;
    int i = 0;
    u32 sizeToAlloc = 0;

    kp->pInBuf = NULL;
    kp->pOutBuf = NULL;
    kp->pBytesReturned = NULL;
    mm_segment_t old_fs = get_fs();
    if (get_user(kp->InSize, &up->InSize) || get_user(kp->OutSize, &up->OutSize) )
    {
        return (-EPERM);
    }

    switch (cmd)
    {
    case IOCTL_AUDIO_SET_SE:
        pr_info("[aud] do_adec_ioctrl IOCTL_AUDIO_SET_SE\n");
        if(NULL != up->pInBuf) {
            if(get_user(temp, &up->pInBuf))
            {
                pr_err("get user from input buffer error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }
            inputBuf32 = compat_ptr(temp);
            if (((AUD_SE_OPCMD_T32*)inputBuf32)->pvData != NULL && ((AUD_SE_OPCMD_T32 *)inputBuf32)->u4DataSize > 0)
            {
                sizeToAlloc =  AUD_ALIGN_SZ(sizeof(AUD_SE_OPCMD_T), sizeof(uintptr_t)) + AUD_ALIGN_SZ(((AUD_SE_OPCMD_T32 *)inputBuf32)->u4DataSize, sizeof(uintptr_t));
                if(get_user(temp, &((AUD_SE_OPCMD_T32*)inputBuf32)->pvData))
                {
                    pr_err("get user from AUD_SE_OPCMD_T buffer error err(%i)!\r\n", -EPERM);
                    return (-EPERM);
                }
                ((AUD_SE_OPCMD_T32*)inputBuf32)->pvData = compat_ptr(temp);

            } else{
                sizeToAlloc =  AUD_ALIGN_SZ(sizeof(AUD_SE_OPCMD_T), sizeof(uintptr_t));
            }
            if (NULL == (inputBuf = (AUD_SE_OPCMD_T __user*)compat_alloc_user_space(sizeToAlloc)))
            {
                pr_err("compat_alloc_user_space error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }
            if(get_user(((AUD_SE_OPCMD_T *)inputBuf)->u4OpCode, &((AUD_SE_OPCMD_T32*)inputBuf32)->u4OpCode) ||
                get_user(((AUD_SE_OPCMD_T *)inputBuf)->u4DataSize, &((AUD_SE_OPCMD_T32*)inputBuf32)->u4DataSize) ||
                get_user(((AUD_SE_OPCMD_T *)inputBuf)->u1Type, &((AUD_SE_OPCMD_T32*)inputBuf32)->u1Type))
            {
                pr_err("get_user error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }
            if (((AUD_SE_OPCMD_T32*)inputBuf32)->pvData != NULL && ((AUD_SE_OPCMD_T32 *)inputBuf32)->u4DataSize > 0)
            {
                pr_info("[aud] fdfs IOCTL_AUDIO_SET_SExx \n");
                ((AUD_SE_OPCMD_T *)inputBuf)->pvData = inputBuf +  AUD_ALIGN_SZ(sizeof(AUD_SE_OPCMD_T), sizeof(uintptr_t));
                if(copy_in_user(((AUD_SE_OPCMD_T *)inputBuf)->pvData, ((AUD_SE_OPCMD_T32*)inputBuf32)->pvData, ((AUD_SE_OPCMD_T *)inputBuf)->u4DataSize))
                {
                    pr_err("copy_in_user error err(%i)!\r\n", -EPERM);
                    return ( -EPERM);
                }
            }

            kp->InSize = sizeof(AUD_SE_OPCMD_T);
            kp->pInBuf= inputBuf;
            
            set_fs(KERNEL_DS);
            i4ret = adec_ioctl(filp, cmd, kp);
            set_fs(old_fs);
        }else {
            pr_err("in buffer is null!\r\n");
            return ( -EPERM);
        }
        break;
        
    case IOCTL_AUDIO_GET_SPECTRUM:
        pr_info("[aud] do_adec_ioctrl IOCTL_AUDIO_GET_SPECTRUM \n");
        if(NULL != up->pOutBuf && up->pBytesReturned != NULL) {
            if (get_user(temp, &up->pOutBuf))
            {
                pr_err("get user from out buffer error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }
            outputBuf32 = compat_ptr(temp);
            
            if (get_user(temp, &up->pBytesReturned))
            {
                pr_err("get user from pBytesReturned error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }
            pBytesReturned32 = compat_ptr(temp);
            sizeToAlloc =  AUD_ALIGN_SZ(sizeof(AUD_DEC_SPECTRUM_INFO_T), sizeof(uintptr_t)) + AUD_ALIGN_SZ(sizeof(u32), sizeof(uintptr_t));
            if (NULL == (outputBuf = (AUD_DEC_SPECTRUM_INFO_T __user*)compat_alloc_user_space(sizeToAlloc)))
            {
                pr_err("compat_alloc_user_space error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }
            #if 0
            if (NULL == (pBytesReturned = (u32 __user*)compat_alloc_user_space(sizeof(u32))))
            {
                pr_err("compat_alloc_user_space error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }
            #endif
            pBytesReturned = outputBuf +  AUD_ALIGN_SZ(sizeof(AUD_DEC_SPECTRUM_INFO_T), sizeof(uintptr_t));         
            kp->pOutBuf = outputBuf;
            kp->pBytesReturned = pBytesReturned;

            set_fs(KERNEL_DS);
            i4ret = adec_ioctl(filp, cmd, kp);
            set_fs(old_fs);
            for (i = 0; i< 16; i++){
                if(put_user(((AUD_DEC_SPECTRUM_INFO_T *)outputBuf)->u4_aud_spectrum[i], &((AUD_DEC_SPECTRUM_INFO_T*)outputBuf32)->u4_aud_spectrum[i]) || 
                    put_user(((AUD_DEC_SPECTRUM_INFO_T *)outputBuf)->u4_aud_spectrum_bar[i], &((AUD_DEC_SPECTRUM_INFO_T*)outputBuf32)->u4_aud_spectrum_bar[i]))
                {
                    pr_err("put_user error err(%i)!\r\n", -EPERM);
                    return (-EPERM);
                }
            }
            if (pBytesReturned32 != NULL)
            {
                if(copy_in_user(pBytesReturned32, pBytesReturned, sizeof(u32)))
                {
                    pr_err("put_user error err(%i)!\r\n", -EPERM);
                    return (-EPERM);
                }
            }
        }else {
            pr_err("out buffer is null!\r\n");
            return (-EPERM);
        }
        break;
        
    case IOCTL_AUDIO_GET_SPECTRUM_SOURCE:
        if(NULL != up->pOutBuf && up->pBytesReturned != NULL) {
            if (get_user(temp, &up->pOutBuf))
            {
                pr_err("get user from out buffer error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }
            outputBuf32 = compat_ptr(temp);
            if (get_user(temp, &up->pBytesReturned))
            {
                pr_err("get user from pBytesReturned error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }
            pBytesReturned32 = compat_ptr(temp);
            
            if (get_user(temp, &((AUD_SPECTRUM_BUF_INFO_T32*)outputBuf32)->u4buf))
            {
                pr_err("get user from outputBuf error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }
            ((AUD_SPECTRUM_BUF_INFO_T32*)outputBuf32)->u4buf = compat_ptr(temp);
            sizeToAlloc =  AUD_ALIGN_SZ(sizeof(AUD_SPECTRUM_BUF_INFO_T), sizeof(uintptr_t)) + 
                           AUD_ALIGN_SZ(((AUD_SPECTRUM_BUF_INFO_T32*)outputBuf32)->u4size, sizeof(uintptr_t)) + AUD_ALIGN_SZ(sizeof(u32), sizeof(uintptr_t));
           
            if (NULL == (outputBuf = (AUD_SPECTRUM_BUF_INFO_T __user*)compat_alloc_user_space(sizeToAlloc)))
            {
                pr_err("compat_alloc_user_space error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }
            #if 0
            if (NULL == (((AUD_SPECTRUM_BUF_INFO_T *)outputBuf)->u4buf = (u8 __user*)compat_alloc_user_space(sizeof(u8))))
            {
                pr_err("compat_alloc_user_space error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }
            if (NULL == (pBytesReturned = (u32 __user*)compat_alloc_user_space(sizeof(u32))))
            {
                pr_err("compat_alloc_user_space error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }
            #endif

             if(get_user(((AUD_SPECTRUM_BUF_INFO_T *)outputBuf)->u4size, &((AUD_SPECTRUM_BUF_INFO_T32*)outputBuf32)->u4size) ||
                get_user(((AUD_SPECTRUM_BUF_INFO_T *)outputBuf)->u4scalingMode, &((AUD_SPECTRUM_BUF_INFO_T32*)outputBuf32)->u4scalingMode))
            {
                pr_err("get_user error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }
            ((AUD_SPECTRUM_BUF_INFO_T *)outputBuf)->u4buf = outputBuf + AUD_ALIGN_SZ(sizeof(AUD_SPECTRUM_BUF_INFO_T), sizeof(uintptr_t));
            pBytesReturned = ((AUD_SPECTRUM_BUF_INFO_T *)outputBuf)->u4buf + AUD_ALIGN_SZ(((AUD_SPECTRUM_BUF_INFO_T32*)outputBuf32)->u4size, sizeof(uintptr_t));
            kp->OutSize = sizeof(AUD_SPECTRUM_BUF_INFO_T);
            kp->pOutBuf = outputBuf;
            kp->pBytesReturned = pBytesReturned;
            
            set_fs(KERNEL_DS);
            i4ret = adec_ioctl(filp, cmd, kp);
            set_fs(old_fs);

            if(copy_in_user(((AUD_SPECTRUM_BUF_INFO_T32*)outputBuf32)->u4buf, ((AUD_SPECTRUM_BUF_INFO_T *)outputBuf)->u4buf, ((AUD_SPECTRUM_BUF_INFO_T32*)outputBuf32)->u4size))
            {
                pr_err("put_user error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }
            if (pBytesReturned32 != NULL)
            {
                if(copy_in_user(pBytesReturned32, pBytesReturned, sizeof(u32)))
                {
                    pr_err("pBytesReturned32 put_user error err(%i)!\r\n", -EPERM);
                    return (-EPERM);
                }
            }
        }else {
            pr_err("out buffer is null!\r\n");
            return (-EPERM);
        }
        break;
        
    case IOCTL_AUDIO_SET_FEATURE:
        pr_info("[aud] do_adec_ioctrl IOCTL_AUDIO_SET_FEATURE \n");
        if(NULL != up->pInBuf) {
            if(get_user(temp, &up->pInBuf))
            {
                pr_err("get user from input buffer error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }
            inputBuf32 = compat_ptr(temp);
            
            if (NULL == (inputBuf = (AUD_DEC_FEATURE_INFO_T __user*)compat_alloc_user_space(sizeof(AUD_DEC_FEATURE_INFO_T))))
            {
                pr_err("compat_alloc_user_space error err(%i)!\r\n", -EPERM);
                return ( -EPERM);
            }
            if(copy_in_user(inputBuf, inputBuf32, sizeof(AUD_DEC_FEATURE_INFO_T)))
            {
                pr_err("copy_in_user error err(%i)!\r\n", -EPERM);
                return ( -EPERM);
            }
            kp->pInBuf = inputBuf;
            
            set_fs(KERNEL_DS);
            i4ret = adec_ioctl(filp, cmd, kp);
            set_fs(old_fs);
        }else {
            pr_err("in buffer is null!\r\n");
            return ( -EPERM);
        }
        break;
        
    case IOCTL_AUDIO_SET_BMANAGEMENT_MODE:
        pr_info("[aud] do_adec_ioctrl IOCTL_AUDIO_SET_BMANAGEMENT_MODE \n");
        if(NULL != up->pInBuf) {
            if(get_user(temp, &up->pInBuf))
            {
                pr_err("get user from input buffer error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }
            inputBuf32 = compat_ptr(temp);
            
            if (NULL == (inputBuf = (AUD_DEC_MODULE_BMANAGEMENT_CHANNEL_INFO_T __user*)compat_alloc_user_space(sizeof(AUD_DEC_MODULE_BMANAGEMENT_CHANNEL_INFO_T))))
            {
                pr_err("compat_alloc_user_space error err(%i)!\r\n", -EPERM);
                return ( -EPERM);
            }
            if(copy_in_user(inputBuf, &inputBuf32, sizeof(AUD_DEC_MODULE_BMANAGEMENT_CHANNEL_INFO_T)))
            {
                pr_err("copy_in_user error err(%i)!\r\n", -EPERM);
                return ( -EPERM);
            }
            kp->pInBuf = inputBuf;
            
            set_fs(KERNEL_DS);
            i4ret = adec_ioctl(filp, cmd, kp);
            set_fs(old_fs);
        }else {
            pr_err("in buffer is null!\r\n");
            return ( -EPERM);
        }
        break;
        
    case IOCTL_AUDIO_SET_BASS_MANAGEMENT_MODE:
        pr_info("[aud] do_adec_ioctrl IOCTL_AUDIO_SET_BASS_MANAGEMENT_MODE \n");
        if(NULL != up->pInBuf) {
            if(get_user(temp, &up->pInBuf))
            {
                pr_err("get user from input buffer error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }
            inputBuf32 = compat_ptr(temp);
            
            if (NULL == (inputBuf = (AUD_DEC_BASS_MANAGEMENT_MODE_T __user*)compat_alloc_user_space(sizeof(AUD_DEC_BASS_MANAGEMENT_MODE_T))))
            {
                pr_err("compat_alloc_user_space error err(%i)!\r\n", -EPERM);
                return ( -EPERM);
            }
            if(copy_in_user(inputBuf, &inputBuf32, sizeof(AUD_DEC_BASS_MANAGEMENT_MODE_T)))
            {
                pr_err("copy_in_user error err(%i)!\r\n", -EPERM);
                return ( -EPERM);
            }
            kp->pInBuf = inputBuf;
            
            set_fs(KERNEL_DS);
            i4ret = adec_ioctl(filp, cmd, kp);
            set_fs(old_fs);
        }else {
            pr_err("in buffer is null!\r\n");
            return ( -EPERM);
        }
        break;
        
    case IOCTL_AUDIO_SET_LRMIX:
        pr_info("[aud] do_adec_ioctrl IOCTL_AUDIO_SET_LRMIX \n");
        if(NULL != up->pInBuf) {
            if(get_user(temp, &up->pInBuf))
            {
                pr_err("get user from input buffer error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }
            inputBuf32 = compat_ptr(temp);
            
            if (NULL == (inputBuf = (AUD_DEC_LRMIX_OUTPUT_T __user*)compat_alloc_user_space(sizeof(AUD_DEC_LRMIX_OUTPUT_T))))
            {
                pr_err("compat_alloc_user_space error err(%i)!\r\n", -EPERM);
                return ( -EPERM);
            }
            if(copy_in_user(inputBuf, &inputBuf32, sizeof(AUD_DEC_LRMIX_OUTPUT_T)))
            {
                pr_err("copy_in_user error err(%i)!\r\n", -EPERM);
                return ( -EPERM);
            }
            kp->pInBuf = inputBuf;
            
            set_fs(KERNEL_DS);
            i4ret = adec_ioctl(filp, cmd, kp);
            set_fs(old_fs);
        }else {
            pr_err("in buffer is null!\r\n");
            return ( -EPERM);
        }
        break;

    case IOCTL_AUDIO_GET_VOLUME:
    case IOCTL_AUDIO_GET_REAR_VOLUME:
    case IOCTL_AUDIN_GET_DEC_DATALEN:
    case IOCTL_AUDIO_GET_READ_DATA_SUM:
        if(NULL != up->pOutBuf && up->pBytesReturned != NULL) {
            if (get_user(temp, &up->pOutBuf))
            {
                pr_err("get user from out buffer error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }
            outputBuf32 = compat_ptr(temp);
            get_user(temp, &up->pBytesReturned);
            pBytesReturned32 = compat_ptr(temp);
            sizeToAlloc =  AUD_ALIGN_SZ(sizeof(u32), sizeof(uintptr_t)) +  AUD_ALIGN_SZ(sizeof(u32), sizeof(uintptr_t));
            if (NULL == (outputBuf = (u32 __user*)compat_alloc_user_space(sizeToAlloc)))
            {
                pr_err("compat_alloc_user_space error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }
            #if 0
            if (NULL == (pBytesReturned = (u32 __user*)compat_alloc_user_space(sizeof(u32))))
            {
                pr_err("compat_alloc_user_space error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }
            #endif
            pBytesReturned = outputBuf + AUD_ALIGN_SZ(sizeof(u32), sizeof(uintptr_t));
            kp->pOutBuf = outputBuf;
            kp->pBytesReturned = pBytesReturned;
            
            set_fs(KERNEL_DS);
            i4ret = adec_ioctl(filp, cmd, kp);
            set_fs(old_fs);
            if(copy_in_user(outputBuf32, outputBuf, sizeof(u32)) || copy_in_user(pBytesReturned32, pBytesReturned, sizeof(u32)))
            {
                pr_err("copy_in_user error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }
        }else {
            pr_err("out buffer is null!\r\n");
            return (-EPERM);
        }
        break;
        
    case IOCTL_AUDIO_SET_VOLUME:
        pr_info("[aud] do_adec_ioctrl IOCTL_AUDIO_SET_VOLUME \n");
        if(NULL != up->pInBuf) {
            if(get_user(temp, &up->pInBuf))
            {
                pr_err("get user from input buffer error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }
            inputBuf32 = compat_ptr(temp);
            
            if (NULL == (inputBuf = (AUD_DEC_VOLUME_GAIN_INFO_T __user*)compat_alloc_user_space(sizeof(AUD_DEC_VOLUME_GAIN_INFO_T))))
            {
                pr_err("compat_alloc_user_space error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }
            if(get_user(((AUD_DEC_VOLUME_GAIN_INFO_T *)inputBuf)->e_vol_type, &((AUD_DEC_VOLUME_GAIN_INFO_T*)inputBuf32)->e_vol_type))
            {
                pr_err("get_user error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }
            if (((AUD_DEC_VOLUME_GAIN_INFO_T *)inputBuf)->e_vol_type != AUD_DEC_ALL_CH)
            {
                if(get_user(((AUD_DEC_VOLUME_GAIN_INFO_T *)inputBuf)->u.t_ch_gain_vol.e_out_port, &((AUD_DEC_VOLUME_GAIN_INFO_T*)inputBuf32)->u.t_ch_gain_vol.e_out_port) ||
                    get_user(((AUD_DEC_VOLUME_GAIN_INFO_T *)inputBuf)->u.t_ch_gain_vol.e_ls, &((AUD_DEC_VOLUME_GAIN_INFO_T*)inputBuf32)->u.t_ch_gain_vol.e_ls) ||
                    get_user(((AUD_DEC_VOLUME_GAIN_INFO_T *)inputBuf)->u.t_ch_gain_vol.u4FrontChVolGain, &((AUD_DEC_VOLUME_GAIN_INFO_T*)inputBuf32)->u.t_ch_gain_vol.u4FrontChVolGain))
                {
                    pr_err("get_user error err(%i)!\r\n", -EPERM);
                    return (-EPERM);
                }
            
            }else {
                if(get_user(((AUD_DEC_VOLUME_GAIN_INFO_T *)inputBuf)->u.u4FrontMasterVolGain, &((AUD_DEC_VOLUME_GAIN_INFO_T*)inputBuf32)->u.u4FrontMasterVolGain))
                {
                    pr_err("get_user error err(%i)!\r\n", -EPERM);
                    return (-EPERM);
                }
            }
            kp->InSize = sizeof(AUD_DEC_VOLUME_GAIN_INFO_T);
            kp->pInBuf = inputBuf;
            
            set_fs(KERNEL_DS);
            i4ret = adec_ioctl(filp, cmd, kp);
            set_fs(old_fs);
        }else {
            pr_err("in buffer is null!\r\n");
            return (-EPERM);
        }
        break;
    case IOCTL_AUDIO_SET_VOL_POLICY:
        pr_info("[aud] do_adec_ioctrl IOCTL_AUDIO_SET_VOL_POLICY \n");
        if(NULL != up->pInBuf) {
            if(get_user(temp, &up->pInBuf))
            {
                pr_err("get user from input buffer error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }
            inputBuf32 = compat_ptr(temp);
            
            if (NULL == (inputBuf = (AUD_VOLUME_POLICY_INFO __user*)compat_alloc_user_space(sizeof(AUD_VOLUME_POLICY_INFO))))
            {
                pr_err("compat_alloc_user_space error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }
            if(get_user(((AUD_VOLUME_POLICY_INFO *)inputBuf)->eType, &((AUD_VOLUME_POLICY_INFO*)inputBuf32)->eType)||
                get_user(((AUD_VOLUME_POLICY_INFO *)inputBuf)->rVolGainInfo.e_vol_type, &((AUD_VOLUME_POLICY_INFO*)inputBuf32)->rVolGainInfo.e_vol_type))
            {
                pr_err("get_user error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }
            if (((AUD_DEC_VOLUME_GAIN_INFO_T *)inputBuf)->e_vol_type == AUD_DEC_ALL_CH)
            {
                if(get_user(((AUD_VOLUME_POLICY_INFO *)inputBuf)->rVolGainInfo.u.t_ch_gain_vol.e_out_port, &((AUD_VOLUME_POLICY_INFO*)inputBuf32)->rVolGainInfo.u.t_ch_gain_vol.e_out_port) ||
                    get_user(((AUD_VOLUME_POLICY_INFO *)inputBuf)->rVolGainInfo.u.t_ch_gain_vol.e_ls, &((AUD_VOLUME_POLICY_INFO*)inputBuf32)->rVolGainInfo.u.t_ch_gain_vol.e_ls) ||
                    get_user(((AUD_VOLUME_POLICY_INFO *)inputBuf)->rVolGainInfo.u.t_ch_gain_vol.u4FrontChVolGain, &((AUD_VOLUME_POLICY_INFO*)inputBuf32)->rVolGainInfo.u.t_ch_gain_vol.u4FrontChVolGain))
                {
                    pr_err("get_user error err(%i)!\r\n", -EPERM);
                    return (-EPERM);
                }
            
            }else {
                if(get_user(((AUD_DEC_VOLUME_GAIN_INFO_T *)inputBuf)->u.u4FrontMasterVolGain, &((AUD_DEC_VOLUME_GAIN_INFO_T*)inputBuf32)->u.u4FrontMasterVolGain))
                {
                    pr_err("get_user error err(%i)!\r\n", -EPERM);
                    return (-EPERM);
                }
            }
            kp->InSize = sizeof(AUD_VOLUME_POLICY_INFO);
            kp->pInBuf = inputBuf;
            
            set_fs(KERNEL_DS);
            i4ret = adec_ioctl(filp, cmd, kp);
            set_fs(old_fs);
        }else {
            pr_err("in buffer is null!\r\n");
            return (-EPERM);
        }
        break;

    case IOCTL_AUDIO_SET_REAR_VOLUME:
        pr_info("[aud] do_adec_ioctrl IOCTL_AUDIO_SET_REAR_VOLUME \n");
        if(NULL != up->pInBuf) {
            if(get_user(temp, &up->pInBuf))
            {
                pr_err("get user from input buffer error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }
            inputBuf32 = compat_ptr(temp);
            
            if (NULL == (inputBuf = (AUD_DEC_REAR_VOLUME_GAIN_INFO_T __user*)compat_alloc_user_space(sizeof(AUD_DEC_REAR_VOLUME_GAIN_INFO_T))))
            {
                pr_err("compat_alloc_user_space error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }
            if(get_user(((AUD_DEC_REAR_VOLUME_GAIN_INFO_T *)inputBuf)->u4RearVolGain, &((AUD_DEC_REAR_VOLUME_GAIN_INFO_T*)inputBuf32)->u4RearVolGain))
            {
                pr_err("get_user error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }
            kp->pInBuf = inputBuf;
            
            set_fs(KERNEL_DS);
            i4ret = adec_ioctl(filp, cmd, kp);
            set_fs(old_fs);
        }else {
            pr_err("in buffer is null!\r\n");
            return (-EPERM);
        }
        break;
    case IOCTL_AUDIO_SET_MUTE_TYPE:
        pr_info("[aud] do_adec_ioctrl IOCTL_AUDIO_SET_MUTE_TYPE \n");
        if(NULL != up->pInBuf) {
            if(get_user(temp, &up->pInBuf))
            {
                pr_err("get user from input buffer error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }
            inputBuf32 = compat_ptr(temp);
            
            if (NULL == (inputBuf = (AUD_DEC1_MUTE_CTRL_T __user*)compat_alloc_user_space(sizeof(AUD_DEC1_MUTE_CTRL_T))))
            {
                pr_err("compat_alloc_user_space error err(%i)!\r\n", -EPERM);
                return ( -EPERM);
            }
            if(copy_in_user(inputBuf, inputBuf32, sizeof(AUD_DEC_FEATURE_INFO_T)))
            {
                pr_err("copy_in_user error err(%i)!\r\n", -EPERM);
                return ( -EPERM);
            }
            kp->pInBuf = inputBuf;
            
            set_fs(KERNEL_DS);
            i4ret = adec_ioctl(filp, cmd, kp);
            set_fs(old_fs);
        }else {
            pr_err("in buffer is null!\r\n");
            return ( -EPERM);
        }
        break;
        
    case IOCTL_AUDIO_SET_SPEAKER_LAYOUT:
        pr_info("[aud] do_adec_ioctrl IOCTL_AUDIO_SET_SPEAKER_LAYOUT \n");
        if(NULL != up->pInBuf) {
            if(get_user(temp, &up->pInBuf))
            {
                pr_err("get user from input buffer error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }
            inputBuf32 = compat_ptr(temp);
            
            if (NULL == (inputBuf = (AUD_DEC_SPEAKER_LAYOUT_T __user*)compat_alloc_user_space(sizeof(AUD_DEC_SPEAKER_LAYOUT_T))))
            {
                pr_err("compat_alloc_user_space error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }
            if(get_user(((AUD_DEC_SPEAKER_LAYOUT_T *)inputBuf)->ui1_total_spk_num, &((AUD_DEC_SPEAKER_LAYOUT_T*)inputBuf32)->ui1_total_spk_num) ||
                get_user(((AUD_DEC_SPEAKER_LAYOUT_T *)inputBuf)->ui8_spk_layout, &((AUD_DEC_SPEAKER_LAYOUT_T*)inputBuf32)->ui8_spk_layout) ||
                get_user(((AUD_DEC_SPEAKER_LAYOUT_T *)inputBuf)->ui2_front_size, &((AUD_DEC_SPEAKER_LAYOUT_T*)inputBuf32)->ui2_front_size) ||
                get_user(((AUD_DEC_SPEAKER_LAYOUT_T *)inputBuf)->ui2_center_size, &((AUD_DEC_SPEAKER_LAYOUT_T*)inputBuf32)->ui2_center_size) ||
                get_user(((AUD_DEC_SPEAKER_LAYOUT_T *)inputBuf)->ui2_rear_size, &((AUD_DEC_SPEAKER_LAYOUT_T*)inputBuf32)->ui2_rear_size) ||
                get_user(((AUD_DEC_SPEAKER_LAYOUT_T *)inputBuf)->ui2_sub_size, &((AUD_DEC_SPEAKER_LAYOUT_T*)inputBuf32)->ui2_sub_size) ||
                get_user(((AUD_DEC_SPEAKER_LAYOUT_T *)inputBuf)->ui4_sub_force_out, &((AUD_DEC_SPEAKER_LAYOUT_T*)inputBuf32)->ui4_sub_force_out))
            {
                pr_err("get_user error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }
            kp->InSize = sizeof(AUD_DEC_SPEAKER_LAYOUT_T);
            kp->pInBuf = inputBuf;
            
            set_fs(KERNEL_DS);
            i4ret = adec_ioctl(filp, cmd, kp);
            set_fs(old_fs);
        }else {
            pr_err("in buffer is null!\r\n");
            return (-EPERM);
        }
        break;
        
    case IOCTL_AUDIO_SET_TEST_TONE_TYPE:
        pr_info("[aud] do_adec_ioctrl IOCTL_AUDIO_SET_TEST_TONE_TYPE \n");
        if(NULL != up->pInBuf) {
            if(get_user(temp, &up->pInBuf))
            {
                pr_err("get user from input buffer error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }
            inputBuf32 = compat_ptr(temp);
            
            if (NULL == (inputBuf = (AUD_TESTTONE_SET_TYPE __user*)compat_alloc_user_space(sizeof(AUD_TESTTONE_SET_TYPE))))
            {
                pr_err("compat_alloc_user_space error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }
            if(get_user(((AUD_TESTTONE_SET_TYPE *)inputBuf)->eTTType, &((AUD_TESTTONE_SET_TYPE*)inputBuf32)->eTTType) ||
                get_user(((AUD_TESTTONE_SET_TYPE *)inputBuf)->eTTOut, &((AUD_TESTTONE_SET_TYPE*)inputBuf32)->eTTOut))
            {
                pr_err("get_user error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }
            kp->pInBuf = inputBuf;
            
            set_fs(KERNEL_DS);
            i4ret = adec_ioctl(filp, cmd, kp);
            set_fs(old_fs);
        }else {
            pr_err("in buffer is null!\r\n");
            return (-EPERM);
        }
        break;
        
    case IOCTL_AUDIO_SET_TEST_TONE_CHANNEL:
        pr_info("[aud] do_adec_ioctrl IOCTL_AUDIO_SET_TEST_TONE_CHANNEL \n");
        if(NULL != up->pInBuf) {
            if(get_user(temp, &up->pInBuf))
            {
                pr_err("get user from input buffer error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }
            inputBuf32 = compat_ptr(temp);
            
            if (NULL == (inputBuf = (AUD_TESTTONE_SET_CHANNEL __user*)compat_alloc_user_space(sizeof(AUD_TESTTONE_SET_CHANNEL))))
            {
                pr_err("compat_alloc_user_space error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }
            if(get_user(((AUD_TESTTONE_SET_CHANNEL *)inputBuf)->eTTLs, &((AUD_TESTTONE_SET_CHANNEL*)inputBuf32)->eTTLs) ||
                get_user(((AUD_TESTTONE_SET_CHANNEL *)inputBuf)->eTTOut, &((AUD_TESTTONE_SET_CHANNEL*)inputBuf32)->eTTOut))
            {
                pr_err("get_user error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }
            kp->pInBuf = inputBuf;
            
            set_fs(KERNEL_DS);
            i4ret = adec_ioctl(filp, cmd, kp);
            set_fs(old_fs);
        }else {
            pr_err("in buffer is null!\r\n");
            return (-EPERM);
        }
        break;
        
    case IOCTL_AUDIO_SET_TEST_TONE_ONOFF:
        pr_info("[aud] do_adec_ioctrl IOCTL_AUDIO_SET_TEST_TONE_ONOFF \n");
        if(NULL != up->pInBuf) {
            if(get_user(temp, &up->pInBuf))
            {
                pr_err("get user from input buffer error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }
            inputBuf32 = compat_ptr(temp);
            
            if (NULL == (inputBuf = (AUD_TESTTONE_SWITCH_T __user*)compat_alloc_user_space(sizeof(AUD_TESTTONE_SWITCH_T))))
            {
                pr_err("compat_alloc_user_space error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }
            if(get_user(((AUD_TESTTONE_SWITCH_T *)inputBuf)->eTTSwitch, &((AUD_TESTTONE_SWITCH_T*)inputBuf32)->eTTSwitch) ||
                       get_user(((AUD_TESTTONE_SWITCH_T *)inputBuf)->eTTOut, &((AUD_TESTTONE_SWITCH_T*)inputBuf32)->eTTOut))
            {
                pr_err("get_user error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }
            kp->pInBuf = inputBuf;
            
            set_fs(KERNEL_DS);
            i4ret = adec_ioctl(filp, cmd, kp);
            set_fs(old_fs);
        }else {
            pr_err("in buffer is null!\r\n");
            return (-EPERM);
        }
        break;
        
    case IOCTL_AUDIO_GET_OUTPUT_VOL:
        pr_info("[aud] do_adec_ioctrl IOCTL_AUDIO_GET_OUTPUT_VOL \n");
        if(NULL != up->pOutBuf && up->pBytesReturned != NULL) {
            if (get_user(temp, &up->pOutBuf))
            {
                pr_err("get user from out buffer error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }
            outputBuf32 = compat_ptr(temp);
            get_user(temp, &up->pBytesReturned);
            pBytesReturned32 = compat_ptr(temp);
            sizeToAlloc =  AUD_ALIGN_SZ(sizeof(AUD_OUTPUT_VOL), sizeof(uintptr_t)) +  AUD_ALIGN_SZ(sizeof(u32), sizeof(uintptr_t));
            if (NULL == (outputBuf = (AUD_OUTPUT_VOL __user*)compat_alloc_user_space(sizeToAlloc)))
            {
                pr_err("compat_alloc_user_space error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }
            #if 0
            if (NULL == (pBytesReturned = (u32 *)compat_alloc_user_space(sizeof(u32))))
            {
                pr_err("compat_alloc_user_space error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }
            #endif
            pBytesReturned = outputBuf + AUD_ALIGN_SZ(sizeof(AUD_OUTPUT_VOL), sizeof(uintptr_t));
            kp->pOutBuf = outputBuf;
            kp->pBytesReturned = pBytesReturned;
            
            set_fs(KERNEL_DS);
            i4ret = adec_ioctl(filp, cmd, kp);
            set_fs(old_fs);
            if(put_user(((AUD_OUTPUT_VOL *)outputBuf)->u4ChVolFrontL, &((AUD_OUTPUT_VOL*)outputBuf32)->u4ChVolFrontL) || 
                put_user(((AUD_OUTPUT_VOL *)outputBuf)->u4ChVolFrontR, &((AUD_OUTPUT_VOL*)outputBuf32)->u4ChVolFrontR) ||
                put_user(((AUD_OUTPUT_VOL *)outputBuf)->u4ChVolFrontLs, &((AUD_OUTPUT_VOL*)outputBuf32)->u4ChVolFrontLs) ||
                put_user(((AUD_OUTPUT_VOL *)outputBuf)->u4ChVolFrontRs, &((AUD_OUTPUT_VOL*)outputBuf32)->u4ChVolFrontRs) ||
                put_user(((AUD_OUTPUT_VOL *)outputBuf)->u4ChVolFrontC, &((AUD_OUTPUT_VOL*)outputBuf32)->u4ChVolFrontC) ||
                put_user(((AUD_OUTPUT_VOL *)outputBuf)->u4ChVolFrontSub, &((AUD_OUTPUT_VOL*)outputBuf32)->u4ChVolFrontSub) ||
                put_user(((AUD_OUTPUT_VOL *)outputBuf)->u4ChVolGpsMix, &((AUD_OUTPUT_VOL*)outputBuf32)->u4ChVolGpsMix) ||
                put_user(((AUD_OUTPUT_VOL *)outputBuf)->u4ChVolRearL, &((AUD_OUTPUT_VOL*)outputBuf32)->u4ChVolRearL) ||
                put_user(((AUD_OUTPUT_VOL *)outputBuf)->u4ChVolRearR, &((AUD_OUTPUT_VOL*)outputBuf32)->u4ChVolRearR) ||
                put_user(((AUD_OUTPUT_VOL *)outputBuf)->u4ChVolBypassL, &((AUD_OUTPUT_VOL*)outputBuf32)->u4ChVolBypassL) ||
                put_user(((AUD_OUTPUT_VOL *)outputBuf)->u4ChVolBypassR, &((AUD_OUTPUT_VOL*)outputBuf32)->u4ChVolBypassR))
            {
                pr_err("put_user error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }
            if (pBytesReturned32 != NULL)
            {
                if(copy_in_user(pBytesReturned32, pBytesReturned, sizeof(u32)))
                {
                    pr_err("put_user error err(%i)!\r\n", -EPERM);
                    return (-EPERM);
                }
            }
        }else {
            pr_err("out buffer is null!\r\n");
            return (-EPERM);
        }
        break;

    case IOCTL_AUDIO_SET_THRESHOLD:
        pr_info("[aud] do_adec_ioctrl IOCTL_AUDIO_SET_THRESHOLD \n");
        if(NULL != up->pInBuf) {
            if(get_user(temp, &up->pInBuf))
            {
                pr_err("get user from input buffer error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }
            inputBuf32 = compat_ptr(temp);
            
            if (NULL == (inputBuf = (AUD_THRESHOLD_T __user*)compat_alloc_user_space(sizeof(AUD_THRESHOLD_T))))
            {
                pr_err("compat_alloc_user_space error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }
            if(get_user(((AUD_THRESHOLD_T *)inputBuf)->u4FrontThrshld, &((AUD_THRESHOLD_T*)inputBuf32)->u4FrontThrshld) ||
                  get_user(((AUD_THRESHOLD_T *)inputBuf)->u4RearThrshld, &((AUD_THRESHOLD_T*)inputBuf32)->u4RearThrshld) ||
                get_user(((AUD_THRESHOLD_T *)inputBuf)->u4WaveFormThrshld, &((AUD_THRESHOLD_T*)inputBuf32)->u4WaveFormThrshld))
            {
                pr_err("get_user error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }
            kp->pInBuf = inputBuf;
            
            set_fs(KERNEL_DS);
            i4ret = adec_ioctl(filp, cmd, kp);
            set_fs(old_fs);
        }else {
            pr_err("in buffer is null!\r\n");
            return (-EPERM);
        }
        break;
        
    case IOCTL_AUDIO_SET_MUTE_DEC1:
        pr_info("[aud] do_adec_ioctrl IOCTL_AUDIO_SET_MUTE_DEC1 \n");
        if(NULL != up->pInBuf) {
            if(get_user(temp, &up->pInBuf))
            {
                pr_err("get user from input buffer error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }
            inputBuf32 = compat_ptr(temp);
            
            if (NULL == (inputBuf = (AUD_DEC1_MUTE_CTRL_T __user*)compat_alloc_user_space(sizeof(AUD_DEC1_MUTE_CTRL_T))))
            {
                pr_err("compat_alloc_user_space error err(%i)!\r\n", -EPERM);
                return ( -EPERM);
            }
            if(copy_in_user(inputBuf, inputBuf32, sizeof(AUD_DEC_FEATURE_INFO_T)))
            {
                pr_err("copy_in_user error err(%i)!\r\n", -EPERM);
                return ( -EPERM);
            }
            kp->pInBuf = inputBuf;
            
            set_fs(KERNEL_DS);
            i4ret = adec_ioctl(filp, cmd, kp);
            set_fs(old_fs);
        }else {
            pr_err("in buffer is null!\r\n");
            return ( -EPERM);
        }
        break;

    case IOCTL_AUDIO_SET_SRC_VOLUME:
        pr_info("[aud] do_adec_ioctrl IOCTL_AUDIO_SET_SRC_VOLUME \n");
        if(NULL != up->pInBuf) {
            if(get_user(temp, &up->pInBuf))
            {
                pr_err("get user from input buffer error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }
            inputBuf32 = compat_ptr(temp);
            
            if (NULL == (inputBuf = (AUD_SRC_VOL_CTL __user*)compat_alloc_user_space(sizeof(AUD_SRC_VOL_CTL))))
            {
                pr_err("compat_alloc_user_space error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }
            if(get_user(((AUD_SRC_VOL_CTL *)inputBuf)->eMediaSrc, &((AUD_SRC_VOL_CTL*)inputBuf32)->eMediaSrc) ||
                get_user(((AUD_SRC_VOL_CTL *)inputBuf)->eMediaOut, &((AUD_SRC_VOL_CTL*)inputBuf32)->eMediaOut) ||
                get_user(((AUD_SRC_VOL_CTL *)inputBuf)->u4Vol, &((AUD_SRC_VOL_CTL*)inputBuf32)->u4Vol))
            {
                pr_err("get_user error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }
            
            kp->InSize = sizeof(AUD_SRC_VOL_CTL);
            kp->pInBuf = inputBuf;
            
            set_fs(KERNEL_DS);
            i4ret = adec_ioctl(filp, cmd, kp);
            set_fs(old_fs);
        }else {
            pr_err("in buffer is null!\r\n");
            return (-EPERM);
        }
        break;

    case IOCTL_AUDIO_CTL:
        pr_info("[aud] do_adec_ioctrl IOCTL_AUDIO_CTL \n");
        if(NULL != up->pInBuf) {
            if(get_user(temp, &up->pInBuf))
            {
                pr_err("get user from input buffer error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }
            inputBuf32 = compat_ptr(temp);
            
            sizeToAlloc = AUD_ALIGN_SZ(sizeof(AUD_DEC_CTRL_T), sizeof(uintptr_t)) + AUD_ALIGN_SZ(sizeof(u8), sizeof(uintptr_t));
            if (NULL == (inputBuf = (AUD_DEC_CTRL_T __user*)compat_alloc_user_space(sizeToAlloc)))
            {
                pr_err("compat_alloc_user_space error err(%i)!\r\n", -EPERM);
                return ( -EPERM);
            }
            if(copy_in_user(inputBuf, inputBuf32, sizeof(AUD_DEC_CTRL_T)))
            {
                pr_err("copy_in_user error err(%i)!\r\n", -EPERM);
                return ( -EPERM);
            }
            if ( NULL != up->pOutBuf) {
                if (get_user(temp, &up->pOutBuf))
                {
                    pr_err("get user from out buffer error err(%i)!\r\n", -EPERM);
                    return (-EPERM);
                }
                outputBuf32 = compat_ptr(temp);
                outputBuf = inputBuf + AUD_ALIGN_SZ(sizeof(AUD_DEC_CTRL_T), sizeof(uintptr_t));
                *((u8*)outputBuf) = AUD_ADSP_NORMAL;
                kp->pOutBuf = outputBuf;
            }
            kp->pInBuf = inputBuf;
            set_fs(KERNEL_DS);
            i4ret = adec_ioctl(filp, cmd, kp);
            set_fs(old_fs);
            if ( NULL != up->pOutBuf) {
                if(copy_in_user(outputBuf32, outputBuf, sizeof(u8)))
                {
                   pr_err("get_user error err(%i)!\r\n", -EPERM);
                   return (-EPERM);
                }
            }
        }else {
            pr_err("in buffer is null!\r\n");
            return ( -EPERM);
        }
        break;
        
    case IOCTL_AUDIO_SET_FORMAT:    // Set Format
        pr_info("[aud] do_adec_ioctrl IOCTL_AUDIO_SET_FORMAT \n");
        if(NULL != up->pInBuf) {
            if(get_user(temp, &up->pInBuf))
            {
                pr_err("get user from input buffer error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }
            inputBuf32 = compat_ptr(temp);
            sizeToAlloc = AUD_ALIGN_SZ(sizeof(AUD_DEC_FMT_T), sizeof(uintptr_t)) + AUD_ALIGN_SZ(sizeof(u8), sizeof(uintptr_t));
            if (NULL == (inputBuf = (AUD_DEC_FMT_T __user*)compat_alloc_user_space(sizeToAlloc)))
            {
                pr_err("compat_alloc_user_space error err(%i)!\r\n", -EPERM);
                return ( -EPERM);
            }
            if(copy_in_user(inputBuf, inputBuf32, sizeof(AUD_DRV_FMT_INFO_T)))
            {
                pr_err("copy_in_user error err(%i)!\r\n", -EPERM);
                return ( -EPERM);
            }

            if ( NULL != up->pOutBuf) {
                if (get_user(temp, &up->pOutBuf))
                {
                    pr_err("get user from out buffer error err(%i)!\r\n", -EPERM);
                    return (-EPERM);
                }
                outputBuf32 = compat_ptr(temp);
                outputBuf = inputBuf + AUD_ALIGN_SZ(sizeof(AUD_DEC_FMT_T), sizeof(uintptr_t));
                *((u8*)outputBuf) = AUD_ADSP_NORMAL;
                kp->pOutBuf = outputBuf;
            }

            kp->pInBuf = inputBuf;
            
            set_fs(KERNEL_DS);
            i4ret = adec_ioctl(filp, cmd, kp);
            set_fs(old_fs);
            if ( NULL != up->pOutBuf) {
                if(copy_in_user(outputBuf32, outputBuf, sizeof(u8)))
                {
                   pr_err("get_user error err(%i)!\r\n", -EPERM);
                   return (-EPERM);
                }
            }
        }else {
            pr_err("in buffer is null!\r\n");
            return ( -EPERM);
        }
        break;
        
    case IOCTL_AUDIO_SET_ORIG_SAMPRATE:    // Set SampleRate
        pr_info("[aud] do_adec_ioctrl IOCTL_AUDIO_SET_ORIG_SAMPRATE \n");
        i4ret = adec_ioctl(filp, cmd, kp);
        break;
        
    case IOCTL_AUDIO_SET_PLAY_SPEED:
        pr_info("[aud] do_adec_ioctrl IOCTL_AUDIO_SET_PLAY_SPEED \n");
        if(NULL != up->pInBuf) {
            if(get_user(temp, &up->pInBuf))
            {
                pr_err("get user from input buffer error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }
            inputBuf32 = compat_ptr(temp);
            sizeToAlloc = AUD_ALIGN_SZ(sizeof(AUD_DEC_PB_SPEED_TYPE_T), sizeof(uintptr_t)) + AUD_ALIGN_SZ(sizeof(u8), sizeof(uintptr_t));
            if (NULL == (inputBuf = (AUD_DEC_PB_SPEED_TYPE_T __user*)compat_alloc_user_space(sizeToAlloc)))
            {
                pr_err("compat_alloc_user_space error err(%i)!\r\n", -EPERM);
                return ( -EPERM);
            }
            if(copy_in_user(inputBuf, inputBuf32, sizeof(AUD_DEC_PB_SPEED_TYPE_T)))
            {
                pr_err("copy_in_user error err(%i)!\r\n", -EPERM);
                return ( -EPERM);
            }
            if (NULL != up->pOutBuf) {
                if (get_user(temp, &up->pOutBuf))
                {
                    pr_err("get user from out buffer error err(%i)!\r\n", -EPERM);
                    return (-EPERM);
                }
                outputBuf32 = compat_ptr(temp);
                outputBuf = inputBuf + AUD_ALIGN_SZ(sizeof(AUD_DEC_PB_SPEED_TYPE_T), sizeof(uintptr_t));
                *((u8*)outputBuf) = AUD_ADSP_NORMAL;
                kp->pOutBuf = outputBuf;
            }
            kp->pInBuf = inputBuf;
            
            set_fs(KERNEL_DS);
            i4ret = adec_ioctl(filp, cmd, kp);
            set_fs(old_fs);
            if ( NULL != up->pOutBuf) {
                pr_err("error reover return %d!\r\n", *((u8*)outputBuf));
                if(copy_in_user(outputBuf32, outputBuf, sizeof(u8)))
                {
                   pr_err("get_user error err(%i)!\r\n", -EPERM);
                   return (-EPERM);
                }
            }
        }else {
            pr_err("in buffer is null!\r\n");
            return ( -EPERM);
        }
        break;
        
    case IOCTL_AUDIO_SET_AUD_INFO: // Set Audio Info
        pr_info("[aud] do_adec_ioctrl IOCTL_AUDIO_SET_AUD_INFO \n");
        if(NULL != up->pInBuf) {
            if(get_user(temp, &up->pInBuf))
            {
                pr_err("get user from input buffer error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }
            inputBuf32 = compat_ptr(temp);
            sizeToAlloc = AUD_ALIGN_SZ(sizeof(AUD_INFO_T), sizeof(uintptr_t)) + AUD_ALIGN_SZ(sizeof(u8), sizeof(uintptr_t));
            if (NULL == (inputBuf = (AUD_INFO_T __user*)compat_alloc_user_space(sizeToAlloc)))
            {
                pr_err("compat_alloc_user_space error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }
            if(get_user(((AUD_INFO_T *)inputBuf)->e_aud_fmt, &((AUD_INFO_T*)inputBuf32)->e_aud_fmt) ||
                  get_user(((AUD_INFO_T *)inputBuf)->e_aud_type, &((AUD_INFO_T*)inputBuf32)->e_aud_type) ||
                get_user(((AUD_INFO_T *)inputBuf)->ui4_sample_rate, &((AUD_INFO_T*)inputBuf32)->ui4_sample_rate) ||
                get_user(((AUD_INFO_T *)inputBuf)->ui4_data_rate, &((AUD_INFO_T*)inputBuf32)->ui4_data_rate) ||
                get_user(((AUD_INFO_T *)inputBuf)->ui1_bit_depth, &((AUD_INFO_T*)inputBuf32)->ui1_bit_depth) ||
                get_user(((AUD_INFO_T *)inputBuf)->ui2_pid, &((AUD_INFO_T*)inputBuf32)->ui2_pid))
            {
                pr_err("get_user error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }

            if(get_user(((AUD_INFO_T *)inputBuf)->pt_ra_cook_info.u4_sample_rate, &((AUD_INFO_T*)inputBuf32)->pt_ra_cook_info.u4_sample_rate) ||
                  get_user(((AUD_INFO_T *)inputBuf)->pt_ra_cook_info.ui2_sample_per_frame, &((AUD_INFO_T*)inputBuf32)->pt_ra_cook_info.ui2_sample_per_frame) ||
                get_user(((AUD_INFO_T *)inputBuf)->pt_ra_cook_info.ui4_frame_size_byte, &((AUD_INFO_T*)inputBuf32)->pt_ra_cook_info.ui4_frame_size_byte) ||
                get_user(((AUD_INFO_T *)inputBuf)->pt_ra_cook_info.ui2_region_num, &((AUD_INFO_T*)inputBuf32)->pt_ra_cook_info.ui2_region_num) ||
                get_user(((AUD_INFO_T *)inputBuf)->pt_ra_cook_info.ui4_cpl_region_start, &((AUD_INFO_T*)inputBuf32)->pt_ra_cook_info.ui4_cpl_region_start) ||
                get_user(((AUD_INFO_T *)inputBuf)->pt_ra_cook_info.ui2_Q_bits_num, &((AUD_INFO_T*)inputBuf32)->pt_ra_cook_info.ui2_Q_bits_num))
            {
                pr_err("get_user error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }

            if(get_user(((AUD_INFO_T *)inputBuf)->pcm_info.ePCM_Format, &((AUD_INFO_T*)inputBuf32)->pcm_info.ePCM_Format) ||
                  get_user(((AUD_INFO_T *)inputBuf)->pcm_info.u2BlockAlign, &((AUD_INFO_T*)inputBuf32)->pcm_info.u2BlockAlign) ||
                get_user(((AUD_INFO_T *)inputBuf)->pcm_info.b_de_emphasis, &((AUD_INFO_T*)inputBuf32)->pcm_info.b_de_emphasis) ||
                get_user(((AUD_INFO_T *)inputBuf)->pcm_info.b_dlna_exist, &((AUD_INFO_T*)inputBuf32)->pcm_info.b_dlna_exist))
            {
                pr_err("get_user error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }

            if(get_user(((AUD_INFO_T *)inputBuf)->wma_info.ui2_version, &((AUD_INFO_T*)inputBuf32)->wma_info.ui2_version) ||
                  get_user(((AUD_INFO_T *)inputBuf)->wma_info.ui4_packet_count, &((AUD_INFO_T*)inputBuf32)->wma_info.ui4_packet_count) ||
                get_user(((AUD_INFO_T *)inputBuf)->wma_info.ui4_packet_size, &((AUD_INFO_T*)inputBuf32)->wma_info.ui4_packet_size) ||
                get_user(((AUD_INFO_T *)inputBuf)->wma_info.ui2_enc_option, &((AUD_INFO_T*)inputBuf32)->wma_info.ui2_enc_option) ||
                get_user(((AUD_INFO_T *)inputBuf)->wma_info.ui2_blockalign, &((AUD_INFO_T*)inputBuf32)->wma_info.ui2_blockalign) ||
                get_user(((AUD_INFO_T *)inputBuf)->wma_info.ui4_bytes_per_sec, &((AUD_INFO_T*)inputBuf32)->wma_info.ui4_bytes_per_sec) ||
                get_user(((AUD_INFO_T *)inputBuf)->wma_info.ui2_bit_depth, &((AUD_INFO_T*)inputBuf32)->wma_info.ui2_bit_depth) ||
                get_user(((AUD_INFO_T *)inputBuf)->wma_info.u2CodecSpecDataSize, &((AUD_INFO_T*)inputBuf32)->wma_info.u2CodecSpecDataSize))
            {
                pr_err("get_user error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }
            for (i= 0; i < WMA_SPEC_DATA_MAX_SIZE; i++)
            {
                if (get_user(((AUD_INFO_T*)inputBuf)->wma_info.au1CodecSpecData[i], &((AUD_INFO_T*)inputBuf32)->wma_info.au1CodecSpecData[i])) 
                {
                    pr_err("get_user error err(%i)!\r\n", -EPERM);
                    return (-EPERM);
                }
            }

            if(get_user(((AUD_INFO_T *)inputBuf)->ape_info.ui4_file_versoin, &((AUD_INFO_T*)inputBuf32)->ape_info.ui4_file_versoin) ||
                  get_user(((AUD_INFO_T *)inputBuf)->ape_info.ui4_compress_level, &((AUD_INFO_T*)inputBuf32)->ape_info.ui4_compress_level) ||
                get_user(((AUD_INFO_T *)inputBuf)->ape_info.ui4_block_per_frame, &((AUD_INFO_T*)inputBuf32)->ape_info.ui4_block_per_frame) ||
                get_user(((AUD_INFO_T *)inputBuf)->ape_info.ui4_final_frame_block, &((AUD_INFO_T*)inputBuf32)->ape_info.ui4_final_frame_block) ||
                get_user(((AUD_INFO_T *)inputBuf)->ape_info.ui4_total_frame_num, &((AUD_INFO_T*)inputBuf32)->ape_info.ui4_total_frame_num) ||
                get_user(((AUD_INFO_T *)inputBuf)->ape_info.ui4_bits_per_sample, &((AUD_INFO_T*)inputBuf32)->ape_info.ui4_bits_per_sample) ||
                get_user(((AUD_INFO_T *)inputBuf)->ape_info.ui4_channel_num_1, &((AUD_INFO_T*)inputBuf32)->ape_info.ui4_channel_num_1) ||
                get_user(((AUD_INFO_T *)inputBuf)->ape_info.ui4_input_sampling_rate, &((AUD_INFO_T*)inputBuf32)->ape_info.ui4_input_sampling_rate) ||
                get_user(((AUD_INFO_T *)inputBuf)->ape_info.ui4_mute_bank_numbers, &((AUD_INFO_T*)inputBuf32)->ape_info.ui4_mute_bank_numbers) ||
                get_user(((AUD_INFO_T *)inputBuf)->ape_info.ui4_invalid_bytes, &((AUD_INFO_T*)inputBuf32)->ape_info.ui4_invalid_bytes))
            {
                pr_err("get_user error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }

            if(get_user(((AUD_INFO_T *)inputBuf)->flac_info.ui4_min_block_size, &((AUD_INFO_T*)inputBuf32)->flac_info.ui4_min_block_size) ||
                  get_user(((AUD_INFO_T *)inputBuf)->flac_info.ui4_max_block_size, &((AUD_INFO_T*)inputBuf32)->flac_info.ui4_max_block_size) ||
                get_user(((AUD_INFO_T *)inputBuf)->flac_info.ui4_min_frame_size, &((AUD_INFO_T*)inputBuf32)->flac_info.ui4_min_frame_size) ||
                get_user(((AUD_INFO_T *)inputBuf)->flac_info.ui4_max_frame_size, &((AUD_INFO_T*)inputBuf32)->flac_info.ui4_max_frame_size) ||
                get_user(((AUD_INFO_T *)inputBuf)->flac_info.ui4_sampling_rate, &((AUD_INFO_T*)inputBuf32)->flac_info.ui4_sampling_rate) ||
                get_user(((AUD_INFO_T *)inputBuf)->flac_info.ui2_channel_num_1, &((AUD_INFO_T*)inputBuf32)->flac_info.ui2_channel_num_1) ||
                get_user(((AUD_INFO_T *)inputBuf)->flac_info.ui2_bits_per_sample_1, &((AUD_INFO_T*)inputBuf32)->flac_info.ui2_bits_per_sample_1) ||
                get_user(((AUD_INFO_T *)inputBuf)->flac_info.ui2_sample_num_high12, &((AUD_INFO_T*)inputBuf32)->flac_info.ui2_sample_num_high12) ||
                get_user(((AUD_INFO_T *)inputBuf)->flac_info.ui4_sample_num_low24, &((AUD_INFO_T*)inputBuf32)->flac_info.ui4_sample_num_low24) ||
                get_user(((AUD_INFO_T *)inputBuf)->flac_info.ui2_frame_num_high12, &((AUD_INFO_T*)inputBuf32)->flac_info.ui2_frame_num_high12) ||
                get_user(((AUD_INFO_T *)inputBuf)->flac_info.ui4_frame_num_low24, &((AUD_INFO_T*)inputBuf32)->flac_info.ui4_frame_num_low24) ||
                get_user(((AUD_INFO_T *)inputBuf)->flac_info.ui2_stream_end, &((AUD_INFO_T*)inputBuf32)->flac_info.ui2_stream_end))
            {
                pr_err("get_user error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }

            if (NULL != up->pOutBuf) {
                if (get_user(temp, &up->pOutBuf))
                {
                    pr_err("get user from out buffer error err(%i)!\r\n", -EPERM);
                    return (-EPERM);
                }
                outputBuf32 = compat_ptr(temp);
                outputBuf = inputBuf + AUD_ALIGN_SZ(sizeof(AUD_INFO_T), sizeof(uintptr_t));
                *((u8*)outputBuf) = AUD_ADSP_NORMAL;
                kp->pOutBuf = outputBuf;
            }
            kp->pInBuf = inputBuf;
            kp->InSize = sizeof(AUD_INFO_T);
            
            set_fs(KERNEL_DS);
            i4ret = adec_ioctl(filp, cmd, kp);
            set_fs(old_fs);
            if ( NULL != up->pOutBuf) {
                if(copy_in_user(outputBuf32, outputBuf, sizeof(u8)))
                {
                   pr_err("get_user error err(%i)!\r\n", -EPERM);
                   return (-EPERM);
                }
            }
        }else {
            pr_err("in buffer is null!\r\n");
            return (-EPERM);
        }
        break;
        
    case IOCTL_AUDIO_GET_PLAYBACK_INFO:// Get playback time
        pr_info("[aud] do_adec_ioctrl IOCTL_AUDIO_GET_PLAYBACK_INFO \n");
        if(NULL != up->pOutBuf  && up->pBytesReturned != NULL) {
            if (get_user(temp, &up->pOutBuf))
            {
                pr_err("get user from out buffer error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }
            outputBuf32 = compat_ptr(temp);
            get_user(temp, &up->pBytesReturned);
            pBytesReturned32 = compat_ptr(temp);
            sizeToAlloc =  AUD_ALIGN_SZ(sizeof(PBINF_A), sizeof(uintptr_t)) +  AUD_ALIGN_SZ(sizeof(u32), sizeof(uintptr_t));
            if (NULL == (outputBuf = (PBINF_A __user*)compat_alloc_user_space(sizeToAlloc)))
            {
                pr_err("compat_alloc_user_space error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }
            #if 0
            if (NULL == (pBytesReturned = (u32 __user*)compat_alloc_user_space(sizeof(u32))))
            {
                pr_err("compat_alloc_user_space error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }
            #endif

            pBytesReturned = outputBuf + AUD_ALIGN_SZ(sizeof(PBINF_A), sizeof(uintptr_t));
            kp->OutSize = sizeof(PBINF_A);
            kp->pOutBuf= outputBuf;
            kp->pBytesReturned = pBytesReturned;
            
            set_fs(KERNEL_DS);
            i4ret = adec_ioctl(filp, cmd, kp);
            set_fs(old_fs);
            if(put_user(((PBINF_A *)outputBuf)->u8DspPlayBackTime, &((PBINF_A*)outputBuf32)->u8DspPlayBackTime))
            {
                pr_err("put_user error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }
            if (pBytesReturned32 != NULL)
            {
                if(copy_in_user(pBytesReturned32, pBytesReturned, sizeof(u32)))
                {
                    pr_err("put_user error err(%i)!\r\n", -EPERM);
                    return (-EPERM);
                }
            }
            
        }else {
            pr_err("out buffer is null!\r\n");
            return (-EPERM);
        }
        break;
        
    case IOCTL_AUDIO_SET_ASRC_SWITCH:
        if(NULL != up->pInBuf) {
            if(get_user(temp, &up->pInBuf))
            {
                pr_err("get user from input buffer error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }
            inputBuf32 = compat_ptr(temp);
            
            if (NULL == (inputBuf = (bool __user*)compat_alloc_user_space(sizeof(bool))))
            {
                pr_err("compat_alloc_user_space error err(%i)!\r\n", -EPERM);
                return ( -EPERM);
            }
            if(copy_in_user(inputBuf, inputBuf32, sizeof(bool)))
            {
                pr_err("copy_in_user error err(%i)!\r\n", -EPERM);
                return ( -EPERM);
            }
            kp->pInBuf = inputBuf;
            
            set_fs(KERNEL_DS);
            i4ret = adec_ioctl(filp, cmd, kp);
            set_fs(old_fs);
        }else {
            pr_err("in buffer is null!\r\n");
            return ( -EPERM);
        }
        break;
        
    case IOCTL_AUDIO_SET_TARGETPTS:
        pr_info("[aud] do_adec_ioctrl IOCTL_AUDIO_SET_TARGETPTS \n");
        if(NULL != up->pInBuf) {
            if(get_user(temp, &up->pInBuf))
            {
                pr_err("get user from input buffer error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }
            inputBuf32 = compat_ptr(temp);
            sizeToAlloc =  AUD_ALIGN_SZ(sizeof(AUD_SYNC_CONTROL_INFO), sizeof(uintptr_t)) +  AUD_ALIGN_SZ(sizeof(u8), sizeof(uintptr_t));
            if (NULL == (inputBuf = (AUD_SYNC_CONTROL_INFO __user*)compat_alloc_user_space(sizeToAlloc)))
            {
                pr_err("compat_alloc_user_space error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }
            if(get_user(((AUD_SYNC_CONTROL_INFO *)inputBuf)->u8DecReadyPTS, &((AUD_SYNC_CONTROL_INFO*)inputBuf32)->u8DecReadyPTS) ||
                  get_user(((AUD_SYNC_CONTROL_INFO *)inputBuf)->u8TargetPTS, &((AUD_SYNC_CONTROL_INFO*)inputBuf32)->u8TargetPTS) ||
                get_user(((AUD_SYNC_CONTROL_INFO *)inputBuf)->u1DecId, &((AUD_SYNC_CONTROL_INFO*)inputBuf32)->u1DecId))
            {
                pr_err("get_user error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }
            if (NULL != up->pOutBuf) {
                if (get_user(temp, &up->pOutBuf))
                {
                    pr_err("get user from out buffer error err(%i)!\r\n", -EPERM);
                    return (-EPERM);
                }
                outputBuf32 = compat_ptr(temp);
                outputBuf = inputBuf + AUD_ALIGN_SZ(sizeof(AUD_SYNC_CONTROL_INFO), sizeof(uintptr_t));
                *((u8*)outputBuf) = AUD_ADSP_NORMAL;
                kp->pOutBuf = outputBuf;
            }
            kp->InSize = sizeof(AUD_SYNC_CONTROL_INFO);
            kp->pInBuf = inputBuf;
            
            set_fs(KERNEL_DS);
            i4ret = adec_ioctl(filp, cmd, kp);
            set_fs(old_fs);

            if ( NULL != up->pOutBuf) {
                if(copy_in_user(outputBuf32, outputBuf, sizeof(u8)))
                {
                   pr_err("get_user error err(%i)!\r\n", -EPERM);
                   return (-EPERM);
                }
            }
        }else {
            pr_err("in buffer is null!\r\n");
            return (-EPERM);
        }
        break;
        
    case IOCTL_AUDIO_GET_CURRENTPTS:
        pr_info("[aud] do_adec_ioctrl IOCTL_AUDIO_GET_CURRENTPTS \n");
        if(NULL != up->pOutBuf) {
            if (get_user(temp, &up->pOutBuf))
            {
                pr_err("get user from out buffer error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }
            outputBuf32 = compat_ptr(temp);
            
            if (NULL == (outputBuf = (AUD_SYNC_CONTROL_INFO __user*)compat_alloc_user_space(sizeof(AUD_SYNC_CONTROL_INFO))))
            {
                pr_err("compat_alloc_user_space error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }
            if( get_user(((AUD_SYNC_CONTROL_INFO *)outputBuf)->u1DecId, &((AUD_SYNC_CONTROL_INFO*)outputBuf32)->u1DecId))
            {
                pr_err("get_user error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }
            kp->pOutBuf= outputBuf;
            
            set_fs(KERNEL_DS);
            i4ret = adec_ioctl(filp, cmd, kp);
            set_fs(old_fs);
            if(put_user(((AUD_SYNC_CONTROL_INFO *)outputBuf)->u8DecReadyPTS, &((AUD_SYNC_CONTROL_INFO*)outputBuf32)->u8DecReadyPTS) || 
                put_user(((AUD_SYNC_CONTROL_INFO *)outputBuf)->u8TargetPTS, &((AUD_SYNC_CONTROL_INFO*)outputBuf32)->u8TargetPTS))
            {
                pr_err("put_user error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }
        }else {
            pr_err("out buffer is null!\r\n");
            return (-EPERM);
        }
        break;
        
    case IOCTL_AUDIO_SET_AVSYNC_DISABLE:
        pr_info("[aud] do_adec_ioctrl IOCTL_AUDIO_SET_AVSYNC_DISABLE \n");
                set_fs(KERNEL_DS);
        i4ret = adec_ioctl(filp, cmd, kp);
        set_fs(old_fs);
        break;
        
    case IOCTL_AUDIO_GET_LATEST_PTS:
        pr_info("[aud] do_adec_ioctrl IOCTL_AUDIO_GET_LATEST_PTS \n");
        if(NULL != up->pOutBuf) {
            if (get_user(temp, &up->pOutBuf))
            {
                pr_err("get user from out buffer error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }
            outputBuf32 = compat_ptr(temp);
            
            if (NULL == (outputBuf = (AUD_PTS_CONTEXT __user*)compat_alloc_user_space(sizeof(AUD_PTS_CONTEXT))))
            {
                pr_err("compat_alloc_user_space error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }
            kp->pOutBuf= outputBuf;
                        set_fs(KERNEL_DS);
            i4ret = adec_ioctl(filp, cmd, kp);
            set_fs(old_fs);
            if(put_user(((AUD_PTS_CONTEXT *)outputBuf)->u4AudioPTSHi, &((AUD_PTS_CONTEXT*)outputBuf32)->u4AudioPTSHi) || 
                put_user(((AUD_PTS_CONTEXT *)outputBuf)->u4AudioPTSLo, &((AUD_PTS_CONTEXT*)outputBuf32)->u4AudioPTSLo) ||
                put_user(((AUD_PTS_CONTEXT *)outputBuf)->u1DecId, &((AUD_PTS_CONTEXT*)outputBuf32)->u1DecId))
            {
                pr_err("put_user error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }
        }else {
            pr_err("out buffer is null!\r\n");
            return (-EPERM);
        }
        break;

    case IOCTL_AUDIO_SET_ASRC_BYPASS:
        if(NULL != up->pInBuf) {
            if(get_user(temp, &up->pInBuf))
            {
                pr_err("get user from input buffer error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }
            inputBuf32 = compat_ptr(temp);
            sizeToAlloc = AUD_ALIGN_SZ(sizeof(bool), sizeof(uintptr_t)) + AUD_ALIGN_SZ(sizeof(u8), sizeof(uintptr_t));
            if (NULL == (inputBuf = (bool __user*)compat_alloc_user_space(sizeToAlloc)))
            {
                pr_err("compat_alloc_user_space error err(%i)!\r\n", -EPERM);
                return ( -EPERM);
            }
            if(copy_in_user(inputBuf, inputBuf32, sizeof(bool)))
            {
                pr_err("copy_in_user error err(%i)!\r\n", -EPERM);
                return ( -EPERM);
            }
            if (NULL != up->pOutBuf) 
            {
                if (get_user(temp, &up->pOutBuf))
                {
                    pr_err("get user from out buffer error err(%i)!\r\n", -EPERM);
                    return (-EPERM);
                }
                outputBuf32 = compat_ptr(temp);
                outputBuf = inputBuf + AUD_ALIGN_SZ(sizeof(bool), sizeof(uintptr_t));
                *((u8*)outputBuf) = AUD_ADSP_NORMAL;
                kp->pOutBuf= outputBuf;
            }
            kp->pInBuf = inputBuf;
            
            set_fs(KERNEL_DS);
            i4ret = adec_ioctl(filp, cmd, kp);
            set_fs(old_fs);
            if (NULL != up->pOutBuf) {
                if(copy_in_user(outputBuf32, outputBuf, sizeof(u8)))
                {
                    pr_err("get_user error err(%i)!\r\n", -EPERM);
                    return (-EPERM);
                }
            }
        }else {
            pr_err("in buffer is null!\r\n");
            return ( -EPERM);
        }
        break;

    case IOCTL_AUDIO_SET_AUDIO_DEC_INFO:
        pr_info("[aud] do_adec_ioctrl IOCTL_AUDIO_SET_AUDIO_DEC_INFO \n");
        if(NULL != up->pInBuf) {
           if (get_user(temp, &up->pInBuf))
           {
               pr_err("get the point fail");
               return -1;
           }
           inputBuf32 = compat_ptr(temp);
           if (NULL != ((AUD_DEC_AUDIO_PB_INFO_T32*)inputBuf32)->prInfo)
           {
           sizeToAlloc = AUD_ALIGN_SZ(sizeof(AUD_DEC_AUDIO_PB_INFO_T), sizeof(uintptr_t)) + AUD_ALIGN_SZ(sizeof(AUD_INFO_T), sizeof(uintptr_t)) +
            AUD_ALIGN_SZ(sizeof(u8), sizeof(uintptr_t));
           if (NULL == (inputBuf = (AUD_DEC_AUDIO_PB_INFO_T __user*)compat_alloc_user_space(sizeToAlloc)))
           {
               pr_err("compat_alloc_user_space error err(%i)!\r\n", -EPERM);
               return (-EPERM);
           }
               ((AUD_DEC_AUDIO_PB_INFO_T *)inputBuf)->prInfo = inputBuf + AUD_ALIGN_SZ(sizeof(AUD_DEC_AUDIO_PB_INFO_T), sizeof(uintptr_t));
           } else{
               sizeToAlloc = AUD_ALIGN_SZ(sizeof(AUD_DEC_AUDIO_PB_INFO_T), sizeof(uintptr_t)) + AUD_ALIGN_SZ(sizeof(u8), sizeof(uintptr_t));
               if (NULL == (inputBuf = (AUD_DEC_AUDIO_PB_INFO_T __user*)compat_alloc_user_space(sizeToAlloc)))
           {
               pr_err("compat_alloc_user_space error err(%i)!\r\n", -EPERM);
               return (-EPERM);
           }
               ((AUD_DEC_AUDIO_PB_INFO_T *)inputBuf)->prInfo = NULL;

           }
          
           if (!access_ok(VERIFY_READ, inputBuf32, sizeof(AUD_DEC_AUDIO_PB_INFO_T32)) )
           {
               pr_err("get_user error err(%i)!\r\n", -EPERM);
                              return (-EPERM);
           }        
           if(get_user(((AUD_DEC_AUDIO_PB_INFO_T *)inputBuf)->eAudDecFmt, &((AUD_DEC_AUDIO_PB_INFO_T32*)inputBuf32)->eAudDecFmt) ||
                get_user(((AUD_DEC_AUDIO_PB_INFO_T *)inputBuf)->eSpeed, &((AUD_DEC_AUDIO_PB_INFO_T32*)inputBuf32)->eSpeed))
           {
               pr_err("get_user error err(%i)!\r\n", -EPERM);
               return (-EPERM);
           }

           if (NULL != ((AUD_DEC_AUDIO_PB_INFO_T32*)inputBuf32)->prInfo)
           {        
               get_user(temp, &(((AUD_DEC_AUDIO_PB_INFO_T32*)inputBuf32)->prInfo));
               inputBuf32_2 = compat_ptr(temp);
               if (NULL == inputBuf32_2)
               {
                   pr_err("inputBuf32_2 == NULL ");
                   return -1;              
               }

               pr_err("inputBuf32_2(0x%lx)!\r\n", inputBuf32_2);
               pr_err("((AUD_INFO_T*)inputBuf32_2)->e_aud_fmt(0x%d)!\r\n", ((AUD_INFO_T*)inputBuf32_2)->e_aud_fmt);
               if(get_user(((AUD_INFO_T*)((AUD_DEC_AUDIO_PB_INFO_T *)inputBuf)->prInfo)->e_aud_fmt, &((AUD_INFO_T*)inputBuf32_2)->e_aud_fmt) ||
                     get_user(((AUD_INFO_T*)((AUD_DEC_AUDIO_PB_INFO_T *)inputBuf)->prInfo)->e_aud_type, &((AUD_INFO_T*)inputBuf32_2)->e_aud_type) ||
                   get_user(((AUD_INFO_T*)((AUD_DEC_AUDIO_PB_INFO_T *)inputBuf)->prInfo)->ui4_sample_rate, &((AUD_INFO_T*)inputBuf32_2)->ui4_sample_rate) ||
                   get_user(((AUD_INFO_T*)((AUD_DEC_AUDIO_PB_INFO_T *)inputBuf)->prInfo)->ui4_data_rate, &((AUD_INFO_T*)inputBuf32_2)->ui4_data_rate) ||
                   get_user(((AUD_INFO_T*)((AUD_DEC_AUDIO_PB_INFO_T *)inputBuf)->prInfo)->ui1_bit_depth, &((AUD_INFO_T*)inputBuf32_2)->ui1_bit_depth) ||
                   get_user(((AUD_INFO_T*)((AUD_DEC_AUDIO_PB_INFO_T *)inputBuf)->prInfo)->ui2_pid, &((AUD_INFO_T*)inputBuf32_2)->ui2_pid))
               {
                   pr_err("get_user error err(%i)!\r\n", -EPERM);
                   return (-EPERM);
               }

               if(get_user(((AUD_INFO_T*)((AUD_DEC_AUDIO_PB_INFO_T *)inputBuf)->prInfo)->pt_ra_cook_info.u4_sample_rate, &((AUD_INFO_T*)inputBuf32_2)->pt_ra_cook_info.u4_sample_rate) ||
                     get_user(((AUD_INFO_T*)((AUD_DEC_AUDIO_PB_INFO_T *)inputBuf)->prInfo)->pt_ra_cook_info.ui2_sample_per_frame, &((AUD_INFO_T*)inputBuf32_2)->pt_ra_cook_info.ui2_sample_per_frame) ||
                   get_user(((AUD_INFO_T*)((AUD_DEC_AUDIO_PB_INFO_T *)inputBuf)->prInfo)->pt_ra_cook_info.ui4_frame_size_byte, &((AUD_INFO_T*)inputBuf32_2)->pt_ra_cook_info.ui4_frame_size_byte) ||
                   get_user(((AUD_INFO_T*)((AUD_DEC_AUDIO_PB_INFO_T *)inputBuf)->prInfo)->pt_ra_cook_info.ui2_region_num, &((AUD_INFO_T*)inputBuf32_2)->pt_ra_cook_info.ui2_region_num) ||
                   get_user(((AUD_INFO_T*)((AUD_DEC_AUDIO_PB_INFO_T *)inputBuf)->prInfo)->pt_ra_cook_info.ui4_cpl_region_start, &((AUD_INFO_T*)inputBuf32_2)->pt_ra_cook_info.ui4_cpl_region_start) ||
                   get_user(((AUD_INFO_T*)((AUD_DEC_AUDIO_PB_INFO_T *)inputBuf)->prInfo)->pt_ra_cook_info.ui2_Q_bits_num, &((AUD_INFO_T*)inputBuf32_2)->pt_ra_cook_info.ui2_Q_bits_num))
               {
                   pr_err("get_user error err(%i)!\r\n", -EPERM);
                   return (-EPERM);
               }

               if(get_user(((AUD_INFO_T*)((AUD_DEC_AUDIO_PB_INFO_T *)inputBuf)->prInfo)->pcm_info.ePCM_Format, &((AUD_INFO_T*)inputBuf32_2)->pcm_info.ePCM_Format) ||
                     get_user(((AUD_INFO_T*)((AUD_DEC_AUDIO_PB_INFO_T *)inputBuf)->prInfo)->pcm_info.u2BlockAlign, &((AUD_INFO_T*)inputBuf32_2)->pcm_info.u2BlockAlign) ||
                   get_user(((AUD_INFO_T*)((AUD_DEC_AUDIO_PB_INFO_T *)inputBuf)->prInfo)->pcm_info.b_de_emphasis, &((AUD_INFO_T*)inputBuf32_2)->pcm_info.b_de_emphasis)||
                   get_user(((AUD_INFO_T*)((AUD_DEC_AUDIO_PB_INFO_T *)inputBuf)->prInfo)->pcm_info.b_dlna_exist, &((AUD_INFO_T*)inputBuf32_2)->pcm_info.b_dlna_exist))
               {
                   pr_err("get_user error err(%i)!\r\n", -EPERM);
                   return (-EPERM);
               }

               if(get_user(((AUD_INFO_T*)((AUD_DEC_AUDIO_PB_INFO_T *)inputBuf)->prInfo)->wma_info.ui2_version, &((AUD_INFO_T*)inputBuf32_2)->wma_info.ui2_version) ||
                     get_user(((AUD_INFO_T*)((AUD_DEC_AUDIO_PB_INFO_T *)inputBuf)->prInfo)->wma_info.ui4_packet_count, &((AUD_INFO_T*)inputBuf32_2)->wma_info.ui4_packet_count) ||
                   get_user(((AUD_INFO_T*)((AUD_DEC_AUDIO_PB_INFO_T *)inputBuf)->prInfo)->wma_info.ui4_packet_size, &((AUD_INFO_T*)inputBuf32_2)->wma_info.ui4_packet_size) ||
                   get_user(((AUD_INFO_T*)((AUD_DEC_AUDIO_PB_INFO_T *)inputBuf)->prInfo)->wma_info.ui2_enc_option, &((AUD_INFO_T*)inputBuf32_2)->wma_info.ui2_enc_option) ||
                   get_user(((AUD_INFO_T*)((AUD_DEC_AUDIO_PB_INFO_T *)inputBuf)->prInfo)->wma_info.ui2_blockalign, &((AUD_INFO_T*)inputBuf32_2)->wma_info.ui2_blockalign) ||
                   get_user(((AUD_INFO_T*)((AUD_DEC_AUDIO_PB_INFO_T *)inputBuf)->prInfo)->wma_info.ui4_bytes_per_sec, &((AUD_INFO_T*)inputBuf32_2)->wma_info.ui4_bytes_per_sec) ||
                   get_user(((AUD_INFO_T*)((AUD_DEC_AUDIO_PB_INFO_T *)inputBuf)->prInfo)->wma_info.ui2_bit_depth, &((AUD_INFO_T*)inputBuf32_2)->wma_info.ui2_bit_depth) ||
                   get_user(((AUD_INFO_T*)((AUD_DEC_AUDIO_PB_INFO_T *)inputBuf)->prInfo)->wma_info.u2CodecSpecDataSize, &((AUD_INFO_T*)inputBuf32_2)->wma_info.u2CodecSpecDataSize) )
               {
                   pr_err("get_user error err(%i)!\r\n", -EPERM);
                   return (-EPERM);
               }
               for (i= 0; i < WMA_SPEC_DATA_MAX_SIZE; i++)
               {
                    if (get_user(((AUD_INFO_T*)((AUD_DEC_AUDIO_PB_INFO_T *)inputBuf)->prInfo)->wma_info.au1CodecSpecData[i], &((AUD_INFO_T*)inputBuf32_2)->wma_info.au1CodecSpecData[i])) 
                    {
                       pr_err("get_user error err(%i)!\r\n", -EPERM);
                       return (-EPERM);
                   }
               }

               if(get_user(((AUD_INFO_T*)((AUD_DEC_AUDIO_PB_INFO_T *)inputBuf)->prInfo)->ape_info.ui4_file_versoin, &((AUD_INFO_T*)inputBuf32_2)->ape_info.ui4_file_versoin) ||
                     get_user(((AUD_INFO_T*)((AUD_DEC_AUDIO_PB_INFO_T *)inputBuf)->prInfo)->ape_info.ui4_compress_level, &((AUD_INFO_T*)inputBuf32_2)->ape_info.ui4_compress_level) ||
                   get_user(((AUD_INFO_T*)((AUD_DEC_AUDIO_PB_INFO_T *)inputBuf)->prInfo)->ape_info.ui4_block_per_frame, &((AUD_INFO_T*)inputBuf32_2)->ape_info.ui4_block_per_frame) ||
                   get_user(((AUD_INFO_T*)((AUD_DEC_AUDIO_PB_INFO_T *)inputBuf)->prInfo)->ape_info.ui4_final_frame_block, &((AUD_INFO_T*)inputBuf32_2)->ape_info.ui4_final_frame_block) ||
                   get_user(((AUD_INFO_T*)((AUD_DEC_AUDIO_PB_INFO_T *)inputBuf)->prInfo)->ape_info.ui4_total_frame_num, &((AUD_INFO_T*)inputBuf32_2)->ape_info.ui4_total_frame_num) ||
                   get_user(((AUD_INFO_T*)((AUD_DEC_AUDIO_PB_INFO_T *)inputBuf)->prInfo)->ape_info.ui4_bits_per_sample, &((AUD_INFO_T*)inputBuf32_2)->ape_info.ui4_bits_per_sample) ||
                   get_user(((AUD_INFO_T*)((AUD_DEC_AUDIO_PB_INFO_T *)inputBuf)->prInfo)->ape_info.ui4_channel_num_1, &((AUD_INFO_T*)inputBuf32_2)->ape_info.ui4_channel_num_1) ||
                   get_user(((AUD_INFO_T*)((AUD_DEC_AUDIO_PB_INFO_T *)inputBuf)->prInfo)->ape_info.ui4_input_sampling_rate, &((AUD_INFO_T*)inputBuf32_2)->ape_info.ui4_input_sampling_rate) ||
                   get_user(((AUD_INFO_T*)((AUD_DEC_AUDIO_PB_INFO_T *)inputBuf)->prInfo)->ape_info.ui4_mute_bank_numbers, &((AUD_INFO_T*)inputBuf32_2)->ape_info.ui4_mute_bank_numbers) ||
                   get_user(((AUD_INFO_T*)((AUD_DEC_AUDIO_PB_INFO_T *)inputBuf)->prInfo)->ape_info.ui4_invalid_bytes, &((AUD_INFO_T*)inputBuf32_2)->ape_info.ui4_invalid_bytes))
               {
                   pr_err("get_user error err(%i)!\r\n", -EPERM);
                   return (-EPERM);
               }

               if(get_user(((AUD_INFO_T*)((AUD_DEC_AUDIO_PB_INFO_T *)inputBuf)->prInfo)->flac_info.ui4_min_block_size, &((AUD_INFO_T*)inputBuf32_2)->flac_info.ui4_min_block_size) ||
                     get_user(((AUD_INFO_T*)((AUD_DEC_AUDIO_PB_INFO_T *)inputBuf)->prInfo)->flac_info.ui4_max_block_size, &((AUD_INFO_T*)inputBuf32_2)->flac_info.ui4_max_block_size) ||
                   get_user(((AUD_INFO_T*)((AUD_DEC_AUDIO_PB_INFO_T *)inputBuf)->prInfo)->flac_info.ui4_min_frame_size, &((AUD_INFO_T*)inputBuf32_2)->flac_info.ui4_min_frame_size)||
                   get_user(((AUD_INFO_T*)((AUD_DEC_AUDIO_PB_INFO_T *)inputBuf)->prInfo)->flac_info.ui4_max_frame_size, &((AUD_INFO_T*)inputBuf32_2)->flac_info.ui4_max_frame_size) ||
                   get_user(((AUD_INFO_T*)((AUD_DEC_AUDIO_PB_INFO_T *)inputBuf)->prInfo)->flac_info.ui4_sampling_rate, &((AUD_INFO_T*)inputBuf32_2)->flac_info.ui4_sampling_rate) ||
                   get_user(((AUD_INFO_T*)((AUD_DEC_AUDIO_PB_INFO_T *)inputBuf)->prInfo)->flac_info.ui2_channel_num_1, &((AUD_INFO_T*)inputBuf32_2)->flac_info.ui2_channel_num_1) ||
                   get_user(((AUD_INFO_T*)((AUD_DEC_AUDIO_PB_INFO_T *)inputBuf)->prInfo)->flac_info.ui2_bits_per_sample_1, &((AUD_INFO_T*)inputBuf32_2)->flac_info.ui2_bits_per_sample_1) ||
                   get_user(((AUD_INFO_T*)((AUD_DEC_AUDIO_PB_INFO_T *)inputBuf)->prInfo)->flac_info.ui2_sample_num_high12, &((AUD_INFO_T*)inputBuf32_2)->flac_info.ui2_sample_num_high12) ||
                   get_user(((AUD_INFO_T*)((AUD_DEC_AUDIO_PB_INFO_T *)inputBuf)->prInfo)->flac_info.ui4_sample_num_low24, &((AUD_INFO_T*)inputBuf32_2)->flac_info.ui4_sample_num_low24) ||
                   get_user(((AUD_INFO_T*)((AUD_DEC_AUDIO_PB_INFO_T *)inputBuf)->prInfo)->flac_info.ui2_frame_num_high12, &((AUD_INFO_T*)inputBuf32_2)->flac_info.ui2_frame_num_high12) ||
                   get_user(((AUD_INFO_T*)((AUD_DEC_AUDIO_PB_INFO_T *)inputBuf)->prInfo)->flac_info.ui4_frame_num_low24, &((AUD_INFO_T*)inputBuf32_2)->flac_info.ui4_frame_num_low24) ||
                   get_user(((AUD_INFO_T*)((AUD_DEC_AUDIO_PB_INFO_T *)inputBuf)->prInfo)->flac_info.ui2_stream_end, &((AUD_INFO_T*)inputBuf32_2)->flac_info.ui2_stream_end))
               {
                   pr_err("get_user error err(%i)!\r\n", -EPERM);
                   return (-EPERM);
               }
           }
           if (NULL != up->pOutBuf) 
           {
               if (get_user(temp, &up->pOutBuf))
               {
                   pr_err("get user from out buffer error err(%i)!\r\n", -EPERM);
                   return (-EPERM);
               }
               outputBuf32 = compat_ptr(temp);
               outputBuf = inputBuf + AUD_ALIGN_SZ(sizeof(AUD_DEC_AUDIO_PB_INFO_T), sizeof(uintptr_t)) + AUD_ALIGN_SZ(sizeof(AUD_INFO_T), sizeof(uintptr_t));
               *((u8*)outputBuf) = AUD_ADSP_NORMAL;
               kp->pOutBuf= outputBuf;
           }
           kp->InSize = sizeof(AUD_DEC_AUDIO_PB_INFO_T);
           kp->pInBuf= inputBuf;
           set_fs(KERNEL_DS);
           i4ret = adec_ioctl(filp, cmd, kp);
           set_fs(old_fs);
           if (NULL != up->pOutBuf) {
               if(copy_in_user(outputBuf32, outputBuf, sizeof(u8)))
               {
                   pr_err("get_user error err(%i)!\r\n", -EPERM);
                   return (-EPERM);
               }
           }
       }else {
           pr_err("in buffer is null!\r\n");
           return (-EPERM);
       }
       break;


    case IOCTL_AUDIO_SET_AC3DRC:
    case IOCTL_AUDIO_SET_DTSDRC:
    case IOCTL_AUDIO_SWITCH_AOUT:
    case IOCTL_AUDIO_SET_REAR_OUT_MODE:
    case IOCTL_AUDMHL_MHL_SEND_INFO:
    case IOCTL_AUDIN_INPUT_TYPE:
        if(NULL != up->pInBuf) {
           if(get_user(temp, &up->pInBuf))
            {
                pr_err("get user from input buffer error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }
            inputBuf32 = compat_ptr(temp);
            
            if (NULL == (inputBuf = (u32 __user*)compat_alloc_user_space(sizeof(u32))))
            {
                pr_err("compat_alloc_user_space error err(%i)!\r\n", -EPERM);
                return ( -EPERM);
            }
            if(copy_in_user(inputBuf, inputBuf32, sizeof(u32)))
            {
                pr_err("copy_in_user error err(%i)!\r\n", -EPERM);
                return ( -EPERM);
            }
            kp->pInBuf= inputBuf;
            
            set_fs(KERNEL_DS);
            i4ret = adec_ioctl(filp, cmd, kp);
            set_fs(old_fs);
        }else {
            pr_err("in buffer is null!\r\n");
            return ( -EPERM);
        }
        break;
        
    case IOCTL_AUDIO_SET_DIVERSITY_INFO:
        pr_info("[aud] do_adec_ioctrl IOCTL_AUDIO_SET_DIVERSITY_INFO \n");
        i4ret = adec_ioctl(filp, cmd, kp);
        break;
        
    case IOCTL_AUDIO_SET_APE_SEEKINFO:
        pr_info("[aud] do_adec_ioctrl IOCTL_AUDIO_SET_APE_SEEKINFO \n");
        if(NULL != up->pInBuf) {
            if(get_user(temp, &up->pInBuf))
            {
                pr_err("get user from input buffer error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }
            inputBuf32 = compat_ptr(temp);
            
            if (NULL == (inputBuf = (APE_SEEKINFO_INFO_T __user*)compat_alloc_user_space(sizeof(APE_SEEKINFO_INFO_T))))
            {
                pr_err("compat_alloc_user_space error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }
            if(get_user(((APE_SEEKINFO_INFO_T *)inputBuf)->ui4_mute_bank_numbers, &((APE_SEEKINFO_INFO_T*)inputBuf32)->ui4_mute_bank_numbers) ||
                  get_user(((APE_SEEKINFO_INFO_T *)inputBuf)->ui4_invalid_bytes, &((APE_SEEKINFO_INFO_T*)inputBuf32)->ui4_invalid_bytes))
            {
                pr_err("get_user error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }
            kp->pInBuf= inputBuf;
            
            set_fs(KERNEL_DS);
            i4ret = adec_ioctl(filp, cmd, kp);
            set_fs(old_fs);
        }else {
            pr_err("in buffer is null!\r\n");
            return (-EPERM);
        }
        break;
    case IOCTL_AUDIO_SET_DEC4_INFO:
        pr_info("[aud] do_adec_ioctrl IOCTL_AUDIO_SET_DEC4_INFO \n");
        if(NULL != up->pInBuf) {
            if(get_user(temp, &up->pInBuf))
            {
                pr_err("get user from input buffer error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }
            inputBuf32 = compat_ptr(temp);
            
            if (NULL == (inputBuf = (AUD_DEC4_INFO_T __user*)compat_alloc_user_space(sizeof(AUD_DEC4_INFO_T))))
            {
                pr_err("compat_alloc_user_space error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }
            if(get_user(((AUD_DEC4_INFO_T *)inputBuf)->e_aud_fmt, &((AUD_DEC4_INFO_T*)inputBuf32)->e_aud_fmt) ||
                  get_user(((AUD_DEC4_INFO_T *)inputBuf)->t_aud_a2dp_info.eBitDepth, &((AUD_DEC4_INFO_T*)inputBuf32)->t_aud_a2dp_info.eBitDepth) ||
                  get_user(((AUD_DEC4_INFO_T *)inputBuf)->t_aud_a2dp_info.eDataEndian, &((AUD_DEC4_INFO_T*)inputBuf32)->t_aud_a2dp_info.eDataEndian) ||
                  get_user(((AUD_DEC4_INFO_T *)inputBuf)->t_aud_a2dp_info.u4SmpRate, &((AUD_DEC4_INFO_T*)inputBuf32)->t_aud_a2dp_info.u4SmpRate) ||
                   get_user(((AUD_DEC4_INFO_T *)inputBuf)->t_aud_a2dp_info.u4channel_cnt, &((AUD_DEC4_INFO_T*)inputBuf32)->t_aud_a2dp_info.u4channel_cnt))
            {
                pr_err("get_user error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }
            kp->pInBuf= inputBuf;
            kp->InSize = sizeof(AUD_DEC4_INFO_T);
            
            set_fs(KERNEL_DS);
            i4ret = adec_ioctl(filp, cmd, kp);
            set_fs(old_fs);
        }else {
            pr_err("in buffer is null!\r\n");
            return (-EPERM);
        }
        break;
        
    case IOCTL_AUDIO_FEATURE_SUPPORT:
        pr_info("[aud] do_adec_ioctrl IOCTL_AUDIO_FEATURE_SUPPORT \n");
        if(NULL != up->pInBuf && NULL != up->pOutBuf) {
            if(get_user(temp, &up->pInBuf))
            {
                pr_err("get user from input buffer error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }
            inputBuf32 = compat_ptr(temp);
            if (get_user(temp, &up->pOutBuf))
            {
                pr_err("get user from out buffer error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }
            outputBuf32 = compat_ptr(temp);
            sizeToAlloc =  AUD_ALIGN_SZ(sizeof(u32), sizeof(uintptr_t)) +  AUD_ALIGN_SZ(sizeof(u8), sizeof(uintptr_t));
            if (NULL == (inputBuf = (u32 __user*)compat_alloc_user_space(sizeToAlloc)))
            {
                pr_err("compat_alloc_user_space error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }
            #if 0
            if (NULL == (outputBuf = (u8 __user*)compat_alloc_user_space(sizeof(u8))))
            {
                pr_err("compat_alloc_user_space error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }
            #endif
            outputBuf = inputBuf + AUD_ALIGN_SZ(sizeof(u32), sizeof(uintptr_t));

            if(copy_in_user(inputBuf, inputBuf32, sizeof(u32)))
            {
                pr_err("get_user error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }
            kp->pInBuf= inputBuf;
            kp->pOutBuf= outputBuf;
            
            set_fs(KERNEL_DS);
            i4ret = adec_ioctl(filp, cmd, kp);
            set_fs(old_fs);
            if(copy_in_user(outputBuf32, outputBuf, sizeof(u8)))
            {
                pr_err("get_user error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }
        }else {
            pr_err("in buffer is null!\r\n");
            return (-EPERM);
        }
        break;
    case IOCTL_AUDIO_GET_CODEC_STATUS:
        pr_info("[aud] do_adec_ioctrl IOCTL_AUDIO_GET_CODEC_STATUS \n");
        if(NULL != up->pInBuf && NULL != up->pOutBuf) {
            if(get_user(temp, &up->pInBuf))
            {
                pr_err("get user from input buffer error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }
            inputBuf32 = compat_ptr(temp);
            if (get_user(temp, &up->pOutBuf))
            {
                pr_err("get user from out buffer error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }
            outputBuf32 = compat_ptr(temp);
            sizeToAlloc =  AUD_ALIGN_SZ(sizeof(AUD_DEC_ID_T), sizeof(uintptr_t)) +  AUD_ALIGN_SZ(sizeof(u8), sizeof(uintptr_t));
            if (NULL == (inputBuf = (AUD_DEC_ID_T __user*)compat_alloc_user_space(sizeToAlloc)))
            {
                pr_err("compat_alloc_user_space error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }
            #if 0
            if (NULL == (outputBuf = (u8 __user*)compat_alloc_user_space(sizeof(u8))))
            {
                pr_err("compat_alloc_user_space error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }
            #endif
            outputBuf = inputBuf + AUD_ALIGN_SZ(sizeof(AUD_DEC_ID_T), sizeof(uintptr_t));

            
            if(copy_in_user(inputBuf, inputBuf32, sizeof(AUD_DEC_ID_T)) )
            {
                pr_err("get_user error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }
            kp->pInBuf= inputBuf;
            kp->pOutBuf= outputBuf;
            kp->OutSize = sizeof(AUD_DRV_CONTEXT);
            
            set_fs(KERNEL_DS);
            i4ret = adec_ioctl(filp, cmd, kp);
            set_fs(old_fs);
            if(put_user(((AUD_DRV_CONTEXT *)outputBuf)->u1DecId, &((AUD_DRV_CONTEXT*)outputBuf32)->u1DecId) || 
                put_user(((AUD_DRV_CONTEXT *)outputBuf)->u1Output, &((AUD_DRV_CONTEXT*)outputBuf32)->u1Output) ||
                put_user(((AUD_DRV_CONTEXT *)outputBuf)->fgPlaying, &((AUD_DRV_CONTEXT*)outputBuf32)->fgPlaying) ||
                put_user(((AUD_DRV_CONTEXT *)outputBuf)->fgEnPlay, &((AUD_DRV_CONTEXT*)outputBuf32)->fgEnPlay) ||
                put_user(((AUD_DRV_CONTEXT *)outputBuf)->ePlayType, &((AUD_DRV_CONTEXT*)outputBuf32)->ePlayType))
            {
                pr_err("put_user error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }
        
        }else {
            pr_err("in buffer is null!\r\n");
            return (-EPERM);
        }
        break;
    case IOCTL_AUDIO_CODEC_RESET:
    case IOCTL_AUDIO_SET_DEC_CONTEXT:
        if(NULL != up->pInBuf) {
            if(get_user(temp, &up->pInBuf))
            {
                pr_err("get user from input buffer error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }
            inputBuf32 = compat_ptr(temp);
            
            if (NULL == (inputBuf = (AUD_DRV_CONTEXT __user*)compat_alloc_user_space(sizeof(AUD_DRV_CONTEXT))))
            {
                pr_err("compat_alloc_user_space error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }

            if(get_user(((AUD_DRV_CONTEXT *)inputBuf)->u1DecId, &((AUD_DRV_CONTEXT*)inputBuf32)->u1DecId) || 
                get_user(((AUD_DRV_CONTEXT *)inputBuf)->u1Output, &((AUD_DRV_CONTEXT*)inputBuf32)->u1Output) ||
                get_user(((AUD_DRV_CONTEXT *)inputBuf)->fgPlaying, &((AUD_DRV_CONTEXT*)inputBuf32)->fgPlaying) ||
                get_user(((AUD_DRV_CONTEXT *)inputBuf)->fgEnPlay, &((AUD_DRV_CONTEXT*)inputBuf32)->fgEnPlay) ||
                get_user(((AUD_DRV_CONTEXT *)inputBuf)->ePlayType, &((AUD_DRV_CONTEXT*)inputBuf32)->ePlayType))
            {
                pr_err("put_user error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }   

            kp->pInBuf= inputBuf;
            kp->InSize = sizeof(AUD_DRV_CONTEXT);
            
            set_fs(KERNEL_DS);
            i4ret = adec_ioctl(filp, cmd, kp);
            set_fs(old_fs);

        }else {
            pr_err("in buffer is null!\r\n");
            return (-EPERM);
        }
        break;

    case IOCTL_AUDIO_CONNECT_ESM: // ESM Connect
    case IOCTL_AUDIO_DISCONNECT_ESM:
        pr_info("[aud] do_adec_ioctrl IOCTL_AUDIO_CONNECT/DISCONNECT_ESM\n");
        if(NULL != up->pOutBuf) {
            if(get_user(temp, &up->pOutBuf))
            {
                pr_err("get user from input buffer error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }
            outputBuf32 = compat_ptr(temp);
            
            if (NULL == (outputBuf = (u8 __user*)compat_alloc_user_space(sizeof(u8))))
            {
                pr_err("compat_alloc_user_space error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }
            *((u8*)outputBuf) = AUD_ADSP_NORMAL;
            kp->pOutBuf= outputBuf;
        }
        set_fs(KERNEL_DS);
        i4ret = adec_ioctl(filp, cmd, kp);
        set_fs(old_fs);
        if(NULL != up->pOutBuf) {
            if(copy_in_user(outputBuf32, outputBuf, sizeof(u8)))
            {
               pr_err("copy_in_user error err(%i)!\r\n", -EPERM);
               return (-EPERM);
            }
        }
        break;

    case IOCTL_AUDIO_SEND_AU:
        //pr_info("[aud] do_adec_ioctrl IOCTL_AUDIO_SEND_AU\n");
        if(NULL != up->pInBuf) {
            if(get_user(temp, &up->pInBuf))
            {
                pr_err("get user from input buffer error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }
            inputBuf32 = compat_ptr(temp);
            sizeToAlloc =  AUD_ALIGN_SZ(sizeof(AU_AUDIO), sizeof(uintptr_t)) +  AUD_ALIGN_SZ(sizeof(u8), sizeof(uintptr_t));
            if (NULL == (inputBuf = (AU_AUDIO __user*)compat_alloc_user_space(sizeToAlloc)))
            {
                pr_err("compat_alloc_user_space error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }

            if(get_user(((AU_AUDIO *)inputBuf)->fgSkipData, &((AU_AUDIO32*)inputBuf32)->fgSkipData) || 
                get_user(((AU_AUDIO *)inputBuf)->ptrSAddr, &((AU_AUDIO32*)inputBuf32)->ptrSAddr) ||
                get_user(((AU_AUDIO *)inputBuf)->ptrEAddr, &((AU_AUDIO32*)inputBuf32)->ptrEAddr) ||
                get_user(((AU_AUDIO *)inputBuf)->eAuType, &((AU_AUDIO32*)inputBuf32)->eAuType) ||
                get_user(((AU_AUDIO *)inputBuf)->rAUInfo.rInfo.u8Pts, &((AU_AUDIO32*)inputBuf32)->rAUInfo.rInfo.u8Pts) ||
                get_user(((AU_AUDIO *)inputBuf)->rAUInfo.rInfo.eAudType, &((AU_AUDIO32*)inputBuf32)->rAUInfo.rInfo.eAudType))
            {
                pr_err("put_user error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }  
            if(NULL != up->pOutBuf) {
                if (get_user(temp, &up->pOutBuf))
                {
                    pr_err("get user from out buffer error err(%i)!\r\n", -EPERM);
                    return (-EPERM);
                }
                outputBuf32 = compat_ptr(temp);
                
                if (NULL == (outputBuf = (u8 __user*)compat_alloc_user_space(sizeof(u8))))
                {
                    pr_err("compat_alloc_user_space error err(%i)!\r\n", -EPERM);
                    return (-EPERM);
                }
                *((u8*)outputBuf) = AUD_ADSP_NORMAL;
                kp->pOutBuf= outputBuf;
            }
            kp->InSize = sizeof(AU_AUDIO);
            kp->pInBuf= inputBuf;
            
            set_fs(KERNEL_DS);
            i4ret = adec_ioctl(filp, cmd, kp);
            set_fs(old_fs);
            if(NULL != up->pOutBuf) {
                if(copy_in_user(outputBuf32, outputBuf, sizeof(u8)))
                {
                   pr_err("get_user error err(%i)!\r\n", -EPERM);
                   return (-EPERM);
                }
            }

        }else {
            pr_err("in buffer is null!\r\n");
            return (-EPERM);
        }
        break;
        
    case IOCTL_AUDIO_SEND_BUFFER:
        if(NULL != up->pInBuf) {
            if(get_user(temp, &up->pInBuf))
            {
                pr_err("get user from input buffer error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }
            inputBuf32 = compat_ptr(temp);

            sizeToAlloc =  AUD_ALIGN_SZ(sizeof(AUD_SEND_BUF_INFO), sizeof(uintptr_t)) +  AUD_ALIGN_SZ(sizeof(u8), sizeof(uintptr_t));
            if (NULL == inputBuf32)
            {
                 pr_err("input buffer from userspace is null error err(%i)!\r\n", -EPERM);
                 return -EPERM;
            }
            get_user(temp, &((AUD_SEND_BUF_INFO32*)inputBuf32)->ptrBufAddr);

            if (NULL == (inputBuf = (AUD_SEND_BUF_INFO __user*)compat_alloc_user_space(sizeToAlloc)))
            {
                pr_err("compat_alloc_user_space error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }

            ((AUD_SEND_BUF_INFO *)inputBuf)->ptrBufAddr = compat_ptr(temp);

            if(get_user(((AUD_SEND_BUF_INFO *)inputBuf)->u4BufLen, &((AUD_SEND_BUF_INFO32*)inputBuf32)->u4BufLen) || 
                get_user(((AUD_SEND_BUF_INFO *)inputBuf)->u8Pts, &((AUD_SEND_BUF_INFO32*)inputBuf32)->u8Pts) ||
                get_user(((AUD_SEND_BUF_INFO *)inputBuf)->ptrBufAddr, &((AUD_SEND_BUF_INFO32*)inputBuf32)->ptrBufAddr))
            {
                pr_err("put_user error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }
            if ( NULL != up->pOutBuf) 
            {
                if (get_user(temp, &up->pOutBuf))
                {
                    pr_err("get user from out buffer error err(%i)!\r\n", -EPERM);
                    return (-EPERM);
                }
                outputBuf32 = compat_ptr(temp);
                outputBuf = inputBuf + AUD_ALIGN_SZ(sizeof(AUD_SEND_BUF_INFO), sizeof(uintptr_t));
                *((u8*)outputBuf) = AUD_ADSP_NORMAL;
                kp->pOutBuf = outputBuf;
            }
            kp->InSize = sizeof(AUD_SEND_BUF_INFO);
            kp->pInBuf= inputBuf;
              
            set_fs(KERNEL_DS);
            i4ret = adec_ioctl(filp, cmd, kp);
            set_fs(old_fs);
            if ( NULL != up->pOutBuf) 
            {
                if(copy_in_user(outputBuf32, outputBuf, sizeof(u8)))
                {
                    pr_err("get_user error err(%i)!\r\n", -EPERM);
                    return (-EPERM);
                }
            }
        }else {
            pr_err(" IOCTL_AUDIO_SEND_BUFFER in buffer is null!\r\n");
            return (-EPERM);
        }
        break;

    case IOCTL_AUD_SMIX_SEND_BUFFER:
        if(NULL != up->pInBuf) {
            if(get_user(temp, &up->pInBuf))
            {
                pr_err("get user from input buffer error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }
            inputBuf32 = compat_ptr(temp);

            sizeToAlloc =  AUD_ALIGN_SZ(sizeof(AUD_SEND_BUF_INFO), sizeof(uintptr_t));
            if (NULL == inputBuf32)
            {
                 pr_err("input buffer from userspace is null error err(%i)!\r\n", -EPERM);
                 return -EPERM;
            }
            get_user(temp, &((AUD_SEND_BUF_INFO32*)inputBuf32)->ptrBufAddr);

            if (NULL == (inputBuf = (AUD_SEND_BUF_INFO __user*)compat_alloc_user_space(sizeToAlloc)))
            {
                pr_err("compat_alloc_user_space error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }

            ((AUD_SEND_BUF_INFO *)inputBuf)->ptrBufAddr = compat_ptr(temp);

            if(get_user(((AUD_SEND_BUF_INFO *)inputBuf)->u4BufLen, &((AUD_SEND_BUF_INFO32*)inputBuf32)->u4BufLen) || 
                get_user(((AUD_SEND_BUF_INFO *)inputBuf)->u8Pts, &((AUD_SEND_BUF_INFO32*)inputBuf32)->u8Pts) ||
                get_user(((AUD_SEND_BUF_INFO *)inputBuf)->ptrBufAddr, &((AUD_SEND_BUF_INFO32*)inputBuf32)->ptrBufAddr))
            {
                pr_err("put_user error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }
            kp->InSize = sizeof(AUD_SEND_BUF_INFO);
            kp->pInBuf= inputBuf;
            set_fs(KERNEL_DS);
            i4ret = adec_ioctl(filp, cmd, kp);
            set_fs(old_fs);
        }else {
            pr_err(" IOCTL_AUD_SMIX_SEND_BUFFER in buffer is null!\r\n");
            return (-EPERM);
        }
        break;
        
    case IOCTL_AUDIO_SEND_BUFFER_KERNEL:
        //pr_info("[aud] do_adec_ioctrl IOCTL_AUDIO_SEND_BUFFER_KERNEL \n");
        i4ret = adec_ioctl(filp, cmd, kp);
        break;
    case IOCTL_AUDIO_SEND_ESM_INFO:
        //pr_info("[aud] do_adec_ioctrl IOCTL_AUDIO_SEND_ESM_INFO \n");
        if(NULL != up->pInBuf) {
            if(get_user(temp, &up->pInBuf))
            {
                pr_err("get user from input buffer error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }
            inputBuf32 = compat_ptr(temp);
            sizeToAlloc =  AUD_ALIGN_SZ(sizeof(ESM_IO_BUF_INFO), sizeof(uintptr_t)) +  AUD_ALIGN_SZ(sizeof(u8), sizeof(uintptr_t));
            if (NULL == (inputBuf = (ESM_IO_BUF_INFO __user*)compat_alloc_user_space(sizeToAlloc)))
            {
                pr_err("compat_alloc_user_space error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }

            if(get_user(((ESM_IO_BUF_INFO *)inputBuf)->rAU.rAudioAU.fgSkipData, &((ESM_IO_BUF_INFO*)inputBuf32)->rAU.rAudioAU.fgSkipData) || 
                get_user(((ESM_IO_BUF_INFO *)inputBuf)->rAU.rAudioAU.ptrSAddr, &((ESM_IO_BUF_INFO*)inputBuf32)->rAU.rAudioAU.ptrSAddr) ||
                get_user(((ESM_IO_BUF_INFO *)inputBuf)->rAU.rAudioAU.ptrEAddr, &((ESM_IO_BUF_INFO*)inputBuf32)->rAU.rAudioAU.ptrEAddr) ||
                get_user(((ESM_IO_BUF_INFO *)inputBuf)->rAU.rAudioAU.eAuType, &((ESM_IO_BUF_INFO*)inputBuf32)->rAU.rAudioAU.eAuType) ||
                get_user(((ESM_IO_BUF_INFO *)inputBuf)->rAU.rAudioAU.rAUInfo.rInfo.u8Pts, &((ESM_IO_BUF_INFO*)inputBuf32)->rAU.rAudioAU.rAUInfo.rInfo.u8Pts) ||
                get_user(((ESM_IO_BUF_INFO *)inputBuf)->rAU.rAudioAU.rAUInfo.rInfo.eAudType, &((ESM_IO_BUF_INFO*)inputBuf32)->rAU.rAudioAU.rAUInfo.rInfo.eAudType))
            {
                pr_err("put_user error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }   
            if ( NULL != up->pOutBuf) 
            {
                if (get_user(temp, &up->pOutBuf))
                {
                    pr_err("get user from out buffer error err(%i)!\r\n", -EPERM);
                    return (-EPERM);
                }
                outputBuf32 = compat_ptr(temp);
                outputBuf = inputBuf + AUD_ALIGN_SZ(sizeof(ESM_IO_BUF_INFO), sizeof(uintptr_t));
                *((u8*)outputBuf) = AUD_ADSP_NORMAL;
                kp->pOutBuf = outputBuf;
            }
            kp->pInBuf= inputBuf;
            
            set_fs(KERNEL_DS);
            i4ret = adec_ioctl(filp, cmd, kp);
            set_fs(old_fs);
            if ( NULL != up->pOutBuf) 
            {
                if(copy_in_user(outputBuf32, outputBuf, sizeof(u8)))
                {
                    pr_err("get_user error err(%i)!\r\n", -EPERM);
                    return (-EPERM);
                }
            }
        }else {
            pr_err("in buffer is null!\r\n");
            return (-EPERM);
        }
        break;
        
    case IOCTL_AUDIO_SEND_END_OF_STREAM:
        pr_info("[aud] do_adec_ioctrl IOCTL_AUDIO_SEND_END_OF_STREAM \n");
        if(NULL != up->pOutBuf) {
            if (get_user(temp, &up->pOutBuf))
            {
                pr_err("get user from out buffer error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }
            outputBuf32 = compat_ptr(temp);
            
            if (NULL == (outputBuf = (u8 __user*)compat_alloc_user_space(sizeof(u8))))
            {
                pr_err("compat_alloc_user_space error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }
            *((u8*)outputBuf) = AUD_ADSP_NORMAL;
            kp->pOutBuf= outputBuf;
        }
        set_fs(KERNEL_DS);
        i4ret = adec_ioctl(filp, cmd, kp);
        set_fs(old_fs);
        if ( NULL != up->pOutBuf) 
        {
            if(copy_in_user(outputBuf32, outputBuf, sizeof(u8)))
            {
                pr_err("get_user error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }
        }
        break;
        
    case IOCTL_AUDIO_GET_AFIFO_INFO_VIRTUAL:
    case IOCTL_AUDIO_GET_AFIFO_WRITE_POINTER:
        if(NULL != up->pOutBuf && up->pBytesReturned != NULL) {
            if(get_user(temp, &up->pOutBuf))
            {
                pr_err("get user from output buffer error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }
            outputBuf32 = compat_ptr(temp);

            if (get_user(temp, &up->pBytesReturned))
            {
                pr_err("get user from pBytesReturned buffer error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }
            pBytesReturned32 = compat_ptr(temp);
            sizeToAlloc =  AUD_ALIGN_SZ(sizeof(AUDIO_BUF_INFO), sizeof(uintptr_t)) +  AUD_ALIGN_SZ(sizeof(u32), sizeof(uintptr_t));
            if (NULL == (outputBuf = (AUDIO_BUF_INFO __user*)compat_alloc_user_space(sizeToAlloc)))
            {
               pr_err("compat_alloc_user_space error err(%i)!\r\n", -EPERM);
               return (-EPERM);
            }
#if 0
            if (NULL == (pBytesReturned = (u32 __user*)compat_alloc_user_space(sizeof(u32))))
            {
               pr_err("compat_alloc_user_space error err(%i)!\r\n", -EPERM);
               return (-EPERM);
            }
#endif

            pBytesReturned = outputBuf +  AUD_ALIGN_SZ(sizeof(AUDIO_BUF_INFO), sizeof(uintptr_t));
            kp->OutSize = sizeof(AUDIO_BUF_INFO);
            kp->pOutBuf= outputBuf;
            kp->pBytesReturned = pBytesReturned;

            set_fs(KERNEL_DS);
            i4ret = adec_ioctl(filp, cmd, kp);
            set_fs(old_fs);

            if(put_user(((AUDIO_BUF_INFO *)outputBuf)->u4WritePointer, &((AUDIO_BUF_INFO32*)outputBuf32)->u4WritePointer) || 
               put_user(((AUDIO_BUF_INFO *)outputBuf)->u4Len, &((AUDIO_BUF_INFO32*)outputBuf32)->u4Len)||
              put_user(ptr_to_compat(((AUDIO_BUF_INFO *)outputBuf)->ptrFifoSA), &((AUDIO_BUF_INFO32*)outputBuf32)->ptrFifoSA) ||
              put_user(ptr_to_compat(((AUDIO_BUF_INFO *)outputBuf)->ptrFifoEA), &((AUDIO_BUF_INFO32*)outputBuf32)->ptrFifoEA))
            {
                pr_err("put_user error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }
            if (pBytesReturned32 != NULL)
            {
                if(copy_in_user(pBytesReturned32, pBytesReturned, sizeof(u32)))
                {
                   pr_err("copy_in_user error err(%i)!\r\n", -EPERM);
                   return (-EPERM);
                }
            }

        }else {
               pr_err("in buffer is null!\r\n");
               return (-EPERM);
            }
            break;

    case IOCTL_AUDIO_GET_CODEC_FIFO_INFO:
        pr_info("[aud] do_adec_ioctrl IOCTL_AUDIO_GET_CODEC_FIFO_INFO \n");
        if(NULL != up->pOutBuf) {
            if(get_user(temp, &up->pOutBuf))
            {
                pr_err("get user from input buffer error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }
            outputBuf32 = compat_ptr(temp);

            if (NULL == (outputBuf = (AUDIO_BUF_INFO __user*)compat_alloc_user_space(sizeof(AUDIO_BUF_INFO))))
            {
                pr_err("compat_alloc_user_space error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }
            kp->pOutBuf= outputBuf;
            
            set_fs(KERNEL_DS);
            i4ret = adec_ioctl(filp, cmd, kp);
            set_fs(old_fs);

            if(put_user(ptr_to_compat(((AUD_POSINFO_T *)outputBuf)->ptrAfifoSA), &((AUD_POSINFO_T32*)outputBuf32)->ptrAfifoSA) || 
               put_user(ptr_to_compat(((AUD_POSINFO_T *)outputBuf)->ptrAfifoEA), &((AUD_POSINFO_T32*)outputBuf32)->ptrAfifoEA) ||
               put_user(ptr_to_compat(((AUD_POSINFO_T *)outputBuf)->ptrAfifoVirSA), &((AUD_POSINFO_T32*)outputBuf32)->ptrAfifoVirSA) ||
               put_user(ptr_to_compat(((AUD_POSINFO_T *)outputBuf)->ptrAfifoVirEA), &((AUD_POSINFO_T32*)outputBuf32)->ptrAfifoVirEA) ||
               put_user(((AUD_POSINFO_T *)outputBuf)->ptrAfifoRPtr, &((AUD_POSINFO_T32*)outputBuf32)->ptrAfifoRPtr)||
               put_user(((AUD_POSINFO_T *)outputBuf)->ptrAfifoWPtr, &((AUD_POSINFO_T32*)outputBuf32)->ptrAfifoWPtr))
            {
               pr_err("put_user error err(%i)!\r\n", -EPERM);
               return (-EPERM);
            }   

        }else {
           pr_err("in buffer is null!\r\n");
           return (-EPERM);
        }
        break;

    case IOCTL_AUDIO_AOUT_CONFIG:
        pr_info("[aud] do_adec_ioctrl IOCTL_AUDIO_AOUT_CONFIG \n");
        if(NULL != up->pInBuf) {
            if(get_user(temp, &up->pInBuf))
            {
                pr_err("get user from input buffer error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }
            inputBuf32 = compat_ptr(temp);

            if (NULL == (inputBuf = (AUD_OUTPUT_PATH_T __user*)compat_alloc_user_space(sizeof(AUD_OUTPUT_PATH_T))))
            {
                pr_err("compat_alloc_user_space error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }

            if(get_user(((AUD_OUTPUT_PATH_T *)inputBuf)->eOut, &((AUD_OUTPUT_PATH_T*)inputBuf32)->eOut) || 
                get_user(((AUD_OUTPUT_PATH_T *)inputBuf)->eSrc, &((AUD_OUTPUT_PATH_T*)inputBuf32)->eSrc))
            {
                pr_err("put_user error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }   
            kp->pInBuf= inputBuf;
            
            set_fs(KERNEL_DS);
            i4ret = adec_ioctl(filp, cmd, kp);
            set_fs(old_fs);

        }else {
            pr_err("in buffer is null!\r\n");
            return (-EPERM);
        }
        break;

    case IOCTL_AUDIO_SET_DAC_TYPE:
        pr_info("[aud] do_adec_ioctrl IOCTL_AUDIO_SET_DAC_TYPE \n");
        if(NULL != up->pInBuf) {
            if(get_user(temp, &up->pInBuf))
            {
                pr_err("get user from input buffer error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }
            inputBuf32 = compat_ptr(temp);

            
            if (NULL == (inputBuf = (AUD_DAC_TYPE_SEL_T __user*)compat_alloc_user_space(sizeof(AUD_DAC_TYPE_SEL_T))))
            {
                pr_err("compat_alloc_user_space error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }

            if(get_user(((AUD_DAC_TYPE_SEL_T *)inputBuf)->eOut, &((AUD_DAC_TYPE_SEL_T*)inputBuf32)->eOut) || 
                get_user(((AUD_DAC_TYPE_SEL_T *)inputBuf)->eDacType, &((AUD_DAC_TYPE_SEL_T*)inputBuf32)->eDacType))
            {
                pr_err("put_user error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }   
            kp->pInBuf= inputBuf;
            
            set_fs(KERNEL_DS);
            i4ret = adec_ioctl(filp, cmd, kp);
            set_fs(old_fs);

        }else {
            pr_err("in buffer is null!\r\n");
            return (-EPERM);
        }
        break;
        
    case IOCTL_AUDIO_SPDIF_ENABLE:
        pr_info("[aud] do_adec_ioctrl IOCTL_AUDIO_SPDIF_ENABLE \n");
        i4ret = adec_ioctl(filp, cmd, kp);
        break;
        
    case IOCTL_AUDIO_SET_TYPE_SPDIF:
        pr_info("[aud] do_adec_ioctrl IOCTL_AUDIO_SET_TYPE_SPDIF \n");
        if(NULL != up->pInBuf) {
            if(get_user(temp, &up->pInBuf))
            {
                pr_err("get user from input buffer error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }
            inputBuf32 = compat_ptr(temp);
            
            if (NULL == (inputBuf = (AUD_DEC_SPDIF_TYPE_T __user*)compat_alloc_user_space(sizeof(AUD_DEC_SPDIF_TYPE_T))))
            {
                pr_err("compat_alloc_user_space error err(%i)!\r\n", -EPERM);
                return ( -EPERM);
            }
            if(copy_in_user(inputBuf, inputBuf32, sizeof(AUD_DEC_SPDIF_TYPE_T)))
            {
                pr_err("copy_in_user error err(%i)!\r\n", -EPERM);
                return ( -EPERM);
            }
            kp->pInBuf= inputBuf;
            
            set_fs(KERNEL_DS);
            i4ret = adec_ioctl(filp, cmd, kp);
            set_fs(old_fs);
        }else {
            pr_err("in buffer is null!\r\n");
            return ( -EPERM);
        }
        break;
        
    case IOCTL_AUDIO_INFO_FROM_DVP:
        pr_info("[aud] do_adec_ioctrl IOCTL_AUDIO_INFO_FROM_DVP \n");
        if(NULL != up->pInBuf) {
            if(get_user(temp, &up->pInBuf))
            {
                pr_err("get user from input buffer error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }
            inputBuf32 = compat_ptr(temp);
            
            if (NULL == (inputBuf = (u8 __user*)compat_alloc_user_space(sizeof(u8))))
            {
                pr_err("compat_alloc_user_space error err(%i)!\r\n", -EPERM);
                return ( -EPERM);
            }
            if(copy_in_user(inputBuf, inputBuf32, sizeof(u8)))
            {
                pr_err("copy_in_user error err(%i)!\r\n", -EPERM);
                return ( -EPERM);
            }
            kp->pInBuf= inputBuf;
            
            set_fs(KERNEL_DS);
            i4ret = adec_ioctl(filp, cmd, kp);
            set_fs(old_fs);
        }else {
            pr_err("in buffer is null!\r\n");
            return ( -EPERM);
        }
        break;
        
    case IOCTL_AUDIO_SET_REAR_I2S_GROUP:
        pr_info("[aud] do_adec_ioctrl IOCTL_AUDIO_SET_REAR_I2S_GROUP \n");
        if(NULL != up->pInBuf) {
            if(get_user(temp, &up->pInBuf))
            {
                pr_err("get user from input buffer error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }
            inputBuf32 = compat_ptr(temp);
            
            if (NULL == (inputBuf = (u32 __user*)compat_alloc_user_space(sizeof(u32))))
            {
                pr_err("compat_alloc_user_space error err(%i)!\r\n", -EPERM);
                return ( -EPERM);
            }
            if(copy_in_user(inputBuf, inputBuf32, sizeof(u32)))
            {
                pr_err("copy_in_user error err(%i)!\r\n", -EPERM);
                return ( -EPERM);
            }
            kp->pInBuf= inputBuf;
            
            set_fs(KERNEL_DS);
            i4ret = adec_ioctl(filp, cmd, kp);
            set_fs(old_fs);
        }else {
            pr_err("in buffer is null!\r\n");
            return ( -EPERM);
        }
        break;
        
    case IOCTL_AUDIO_SET_MIRACAST_ONOFF:
        pr_info("[aud] do_adec_ioctrl IOCTL_AUDIO_SET_MIRACAST_ONOFF \n");
        if(NULL != up->pInBuf) {
            if(get_user(temp, &up->pInBuf))
            {
                pr_err("get user from input buffer error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }
            inputBuf32 = compat_ptr(temp);
            
            if (NULL == (inputBuf = (AUD_MIRACAST_CTRL_T __user*)compat_alloc_user_space(sizeof(AUD_MIRACAST_CTRL_T))))
            {
                pr_err("compat_alloc_user_space error err(%i)!\r\n", -EPERM);
                return ( -EPERM);
            }
            if(copy_in_user(inputBuf, inputBuf32, sizeof(AUD_MIRACAST_CTRL_T)))
            {
                pr_err("copy_in_user error err(%i)!\r\n", -EPERM);
                return ( -EPERM);
            }
            kp->pInBuf= inputBuf;
            
            set_fs(KERNEL_DS);
            i4ret = adec_ioctl(filp, cmd, kp);
            set_fs(old_fs);
        }else {
            pr_err("in buffer is null!\r\n");
            return ( -EPERM);
        }
        break;
        
    case IOCTL_AUDIO_SET_MIRACAST_PARAM:
        pr_info("[aud] do_adec_ioctrl IOCTL_AUDIO_SET_MIRACAST_PARAM \n");
        if(NULL != up->pInBuf) {
            if(get_user(temp, &up->pInBuf))
            {
                pr_err("get user from input buffer error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }
            inputBuf32 = compat_ptr(temp);
        
            if (NULL == (inputBuf = (AUD_MIRACAST_PARAM_T __user*)compat_alloc_user_space(sizeof(AUD_MIRACAST_PARAM_T))))
            {
                pr_err("compat_alloc_user_space error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }

            if(get_user(((AUD_MIRACAST_PARAM_T *)inputBuf)->e_ParamID, &((AUD_MIRACAST_PARAM_T*)inputBuf32)->e_ParamID))
            {
                pr_err("put_user error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }

            switch(((AUD_MIRACAST_PARAM_T *)inputBuf)->e_ParamID)
            {
            case AUD_MIRA_LOWTH:
                if(get_user(((AUD_MIRACAST_PARAM_T *)inputBuf)->uVal.i8LowThVal, &((AUD_MIRACAST_PARAM_T*)inputBuf32)->uVal.i8LowThVal))
                {
                    pr_err("get_user error err(%i)!\r\n", -EPERM);
                    return (-EPERM);
                } 
                break;
            case AUD_MIRA_LIGHTH:
                if(get_user(((AUD_MIRACAST_PARAM_T *)inputBuf)->uVal.i8HighThVal, &((AUD_MIRACAST_PARAM_T*)inputBuf32)->uVal.i8HighThVal))
                {
                    pr_err("get_user error err(%i)!\r\n", -EPERM);
                    return (-EPERM);
                } 
                break;
            case AUD_MIRA_ADJSIZE:
                if(get_user(((AUD_MIRACAST_PARAM_T *)inputBuf)->uVal.u4AdjustSize, &((AUD_MIRACAST_PARAM_T*)inputBuf32)->uVal.u4AdjustSize))
                {
                    pr_err("get_user error err(%i)!\r\n", -EPERM);
                    return (-EPERM);
                } 
                break;
            case AUD_MIRA_SLEEPTIME:
                if(get_user(((AUD_MIRACAST_PARAM_T *)inputBuf)->uVal.u2SleepTime, &((AUD_MIRACAST_PARAM_T*)inputBuf32)->uVal.u2SleepTime))
                {
                    pr_err("get_user error err(%i)!\r\n", -EPERM);
                    return (-EPERM);
                } 
                break;
            default:
                break;
            }
            kp->InSize = sizeof(AUD_MIRACAST_PARAM_T);
            kp->pInBuf= inputBuf;
            
            set_fs(KERNEL_DS);
            i4ret = adec_ioctl(filp, cmd, kp);
            set_fs(old_fs);

        }else {
            pr_err("in buffer is null!\r\n");
            return (-EPERM);
        }
        break;
    
    case IOCTL_AUDIO_GET_FRONT_STATUS:
    case IOCTL_AUDIO_GET_REAR_STATUS:
        pr_info("[aud] do_adec_ioctrl IOCTL_AUDIO_GET_FRONT/REAR_STATUS \n");
        if(NULL != up->pOutBuf) {
            if (get_user(temp, &up->pOutBuf))
            {
                pr_err("get user from out buffer error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }
            outputBuf32 = compat_ptr(temp);
            
            if (NULL == (outputBuf = (bool __user*)compat_alloc_user_space(sizeof(bool))))
            {
                pr_err("compat_alloc_user_space error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }
            kp->pInBuf= inputBuf;
            
            set_fs(KERNEL_DS);
            i4ret = adec_ioctl(filp, cmd, kp);
            set_fs(old_fs);
            if(copy_in_user(outputBuf32, outputBuf, sizeof(bool)))
            {
                pr_err("copy_in_user error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }
        }else {
            pr_err("out buffer is null!\r\n");
            return (-EPERM);
        }
        break;
    case IOCTL_AUDIO_GET_FRONT_TYPE:
    case IOCTL_AUDIO_GET_REAR_TYPE:
        pr_info("[aud] do_adec_ioctrl IOCTL_AUDIO_GET_FRONT/REAR_TYPE \n");
        if(NULL != up->pOutBuf) {
            if (get_user(temp, &up->pOutBuf))
            {
                pr_err("get user from out buffer error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }
            outputBuf32 = compat_ptr(temp);
            
            if (NULL == (outputBuf = (u8 __user*)compat_alloc_user_space(sizeof(u8))))
            {
                pr_err("compat_alloc_user_space error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }
            kp->pOutBuf= outputBuf;
            set_fs(KERNEL_DS);
            i4ret = adec_ioctl(filp, cmd, kp);
            set_fs(old_fs);
            if(copy_in_user(outputBuf32, outputBuf, sizeof(u8)))
            {
                pr_err("copy_in_user error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }
        }else {
            pr_err("out buffer is null!\r\n");
            return (-EPERM);
        }
        break;

    case IOCTL_AUDIO_GET_DEC_CONTEXT:
        pr_info("[aud] do_adec_ioctrl IOCTL_AUDIO_GET_DEC_CONTEXT \n");
        if(NULL != up->pOutBuf) {
            if(get_user(temp, &up->pOutBuf))
            {
                pr_err("get user from input buffer error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }
            outputBuf32 = compat_ptr(temp);

            if (NULL == (outputBuf = (AUD_DRV_CONTEXT __user*)compat_alloc_user_space(sizeof(AUD_DRV_CONTEXT))))
            {
                pr_err("compat_alloc_user_space error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }
            kp->pOutBuf= outputBuf;

            set_fs(KERNEL_DS);
            i4ret = adec_ioctl(filp, cmd, kp);
            set_fs(old_fs);
            if(put_user(((AUD_DRV_CONTEXT *)outputBuf)->u1DecId, &((AUD_DRV_CONTEXT*)outputBuf32)->u1DecId) || 
                put_user(((AUD_DRV_CONTEXT *)outputBuf)->u1Output, &((AUD_DRV_CONTEXT*)outputBuf32)->u1Output) ||
                put_user(((AUD_DRV_CONTEXT *)outputBuf)->fgPlaying, &((AUD_DRV_CONTEXT*)outputBuf32)->fgPlaying) ||
                put_user(((AUD_DRV_CONTEXT *)outputBuf)->fgEnPlay, &((AUD_DRV_CONTEXT*)outputBuf32)->fgEnPlay) ||
                put_user(((AUD_DRV_CONTEXT *)outputBuf)->ePlayType, &((AUD_DRV_CONTEXT*)outputBuf32)->ePlayType))
            {
                pr_err("put_user error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }

        }else {
            pr_err("in buffer is null!\r\n");
            return (-EPERM);
        }
        break;

    case IOCTL_AUDIO_SET_FRONT_MEDIA_TYPE:
    case IOCTL_AUDIO_SET_REAR_MEDIA_TYPE:
        pr_info("[aud] do_adec_ioctrl IOCTL_AUDIO_SET_FRONT/REAR_MEDIA_TYPE \n");
        if(NULL != up->pInBuf) {
            if(get_user(temp, &up->pInBuf))
            {
                pr_err("get user from input buffer error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }
            inputBuf32 = compat_ptr(temp);

            if (NULL == (inputBuf = (AUD_OUT_MEDIA_TYPE_T __user*)compat_alloc_user_space(sizeof(AUD_OUT_MEDIA_TYPE_T))))
            {
                pr_err("compat_alloc_user_space error err(%i)!\r\n", -EPERM);
                return ( -EPERM);
            }
            if(copy_in_user(inputBuf, inputBuf32, sizeof(AUD_OUT_MEDIA_TYPE_T)))
            {
                pr_err("copy_in_user error err(%i)!\r\n", -EPERM);
                return ( -EPERM);
            }

            kp->pInBuf= inputBuf;            
            set_fs(KERNEL_DS);
            i4ret = adec_ioctl(filp, cmd, kp);
            set_fs(old_fs);
        
            pr_info("IOCTL_AUDIO_SET_FRONT_MEDIA_TYPE i4ret(%i)!\r\n", i4ret);
        }else {
            pr_err("in buffer is null!\r\n");
            return ( -EPERM);
        }
        break;
    case IOCTL_AUDIO_GET_MEDIA_TYPE_STATUS:
        if(NULL != up->pOutBuf) {
            if (get_user(temp, &up->pOutBuf))
            {
                pr_err("get user from out buffer error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }
            outputBuf32 = compat_ptr(temp);

            if (NULL == (outputBuf = (AUD_MEDIA_TYPE __user*)compat_alloc_user_space(sizeof(AUD_MEDIA_TYPE))))
            {
                pr_err("compat_alloc_user_space error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }

            if(get_user(((AUD_MEDIA_TYPE *)outputBuf)->eMediaSrc, &((AUD_MEDIA_TYPE*)outputBuf32)->eMediaSrc) || 
                get_user(((AUD_MEDIA_TYPE *)outputBuf)->eMediaOut, &((AUD_MEDIA_TYPE*)outputBuf32)->eMediaOut))
            {
                pr_err("get_user error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }  
            kp->pOutBuf= outputBuf;
            set_fs(KERNEL_DS);
            i4ret = adec_ioctl(filp, cmd, kp);
            set_fs(old_fs);
            if(put_user(((AUD_MEDIA_TYPE *)outputBuf)->eMediaCtrl, &((AUD_MEDIA_TYPE*)outputBuf32)->eMediaCtrl))
            {
                pr_err("put_user error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }
        }else {
            pr_err("out buffer is null!\r\n");
            return (-EPERM);
        }
        break;
    case IOCTL_AUDIO_SET_MEDIA_TYPE:
    case IOCTL_AUD_SMIX_CTL:
        pr_info("[aud] do_adec_ioctrl IOCTL_AUDIO_SET_MEDIA_TYPE\n");
        if(NULL != up->pInBuf) {
            if(get_user(temp, &up->pInBuf))
            {
                pr_err("get user from input buffer error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }
            inputBuf32 = compat_ptr(temp);

            if (NULL == (inputBuf = (AUD_MEDIA_TYPE __user*)compat_alloc_user_space(sizeof(AUD_MEDIA_TYPE))))
            {
                pr_err("compat_alloc_user_space error err(%i)!\r\n", -EPERM);
                return ( -EPERM);
            }
             if(get_user(((AUD_MEDIA_TYPE *)inputBuf)->eMediaSrc, &((AUD_MEDIA_TYPE*)inputBuf32)->eMediaSrc) || 
                get_user(((AUD_MEDIA_TYPE *)inputBuf)->eMediaOut, &((AUD_MEDIA_TYPE*)inputBuf32)->eMediaOut) ||
                get_user(((AUD_MEDIA_TYPE *)inputBuf)->eMediaCtrl, &((AUD_MEDIA_TYPE*)inputBuf32)->eMediaCtrl))
            {
                pr_err("get_user error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }

            kp->pInBuf= inputBuf;
            set_fs(KERNEL_DS);
            i4ret = adec_ioctl(filp, cmd, kp);
            set_fs(old_fs);
            pr_info("IOCTL_AUDIO_SET_FRONT_MEDIA_TYPE i4ret(%i)!\r\n", i4ret);
        }else {
            pr_err("in buffer is null!\r\n");
            return ( -EPERM);
        }
        break;

    case IOCTL_AUDIO_DECONLY_ONOFF:
        pr_info("[aud] do_adec_ioctrl IOCTL_AUDIO_DECONLY_ONOFF \n");
        if(NULL != up->pInBuf) {
            if(get_user(temp, &up->pInBuf))
            {
                pr_err("get user from input buffer error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }
            inputBuf32 = compat_ptr(temp);

            if (NULL == (inputBuf = (AUD_DECONLY_CTRL_T __user*)compat_alloc_user_space(sizeof(AUD_DECONLY_CTRL_T))))
            {
                pr_err("compat_alloc_user_space error err(%i)!\r\n", -EPERM);
                return ( -EPERM);
            }
            if(copy_in_user(inputBuf, inputBuf32, sizeof(AUD_DECONLY_CTRL_T)))
            {
                pr_err("copy_in_user error err(%i)!\r\n", -EPERM);
                return ( -EPERM);
            }
            kp->pInBuf= inputBuf;
            
            set_fs(KERNEL_DS);
            i4ret = adec_ioctl(filp, cmd, kp);
            set_fs(old_fs);
        }else {
            pr_err("in buffer is null!\r\n");
            return ( -EPERM);
        }
        break;
        
    case IOCTL_AUDIO_DECONLY_GET_BUF:
        if(NULL != up->pOutBuf) {
            if(get_user(temp, &up->pOutBuf))
            {
                pr_err("get user from input buffer error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }
            outputBuf32 = compat_ptr(temp);

            sizeToAlloc =  AUD_ALIGN_SZ(sizeof(AUD_DECONLY_GET_BUF), sizeof(uintptr_t));
            if (NULL == (outputBuf = (AUD_DECONLY_GET_BUF __user*)compat_alloc_user_space(sizeToAlloc)))
            {
                pr_err("compat_alloc_user_space error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }

            if(get_user(temp, &((AUD_DECONLY_GET_BUF32*)outputBuf32)->u4BufAddr))
            {
                pr_err("get_user error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }
            u4BufAddr32 = compat_ptr(temp);

            ((AUD_DECONLY_GET_BUF *)outputBuf)->u4BufAddr = u4BufAddr32;//outputBuf + AUD_ALIGN_SZ(sizeof(AUD_DECONLY_GET_BUF), sizeof(uintptr_t));
            kp->pOutBuf= outputBuf;
            kp->OutSize= sizeof(AUD_DECONLY_GET_BUF);
            set_fs(KERNEL_DS);
            i4ret = adec_ioctl(filp, cmd, kp);
            set_fs(old_fs);

            if(put_user(((AUD_DECONLY_GET_BUF *)outputBuf)->eDataEndian, &((AUD_DECONLY_GET_BUF32*)outputBuf32)->eDataEndian) || 
                put_user(((AUD_DECONLY_GET_BUF *)outputBuf)->eBitDepth, &((AUD_DECONLY_GET_BUF32*)outputBuf32)->eBitDepth) ||
                put_user(((AUD_DECONLY_GET_BUF *)outputBuf)->u4SampleRate, &((AUD_DECONLY_GET_BUF32*)outputBuf32)->u4SampleRate) ||
                put_user(((AUD_DECONLY_GET_BUF *)outputBuf)->u4BufLen, &((AUD_DECONLY_GET_BUF32*)outputBuf32)->u4BufLen) ||
                put_user(((AUD_DECONLY_GET_BUF *)outputBuf)->u8BufPts, &((AUD_DECONLY_GET_BUF32*)outputBuf32)->u8BufPts))
            {
                pr_err("put_user error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }   

            if(put_user(((AUD_DECONLY_GET_BUF *)outputBuf)->eChCfg.u2ChNum, &((AUD_DECONLY_GET_BUF32*)outputBuf32)->eChCfg.u2ChNum) || 
                put_user(((AUD_DECONLY_GET_BUF *)outputBuf)->eChCfg.u1LayoutC, &((AUD_DECONLY_GET_BUF32*)outputBuf32)->eChCfg.u1LayoutC) ||
                put_user(((AUD_DECONLY_GET_BUF *)outputBuf)->eChCfg.u1LayoutL, &((AUD_DECONLY_GET_BUF32*)outputBuf32)->eChCfg.u1LayoutL) ||
                put_user(((AUD_DECONLY_GET_BUF *)outputBuf)->eChCfg.u1LayoutR, &((AUD_DECONLY_GET_BUF32*)outputBuf32)->eChCfg.u1LayoutR) ||
                put_user(((AUD_DECONLY_GET_BUF *)outputBuf)->eChCfg.u1LayoutLs, &((AUD_DECONLY_GET_BUF32*)outputBuf32)->eChCfg.u1LayoutLs) ||
                put_user(((AUD_DECONLY_GET_BUF *)outputBuf)->eChCfg.u1LayoutRs, &((AUD_DECONLY_GET_BUF32*)outputBuf32)->eChCfg.u1LayoutRs) ||
                put_user(((AUD_DECONLY_GET_BUF *)outputBuf)->eChCfg.u1LayoutSub, &((AUD_DECONLY_GET_BUF32*)outputBuf32)->eChCfg.u1LayoutSub))
            {
                pr_err("put_user error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }   

        }else {
            pr_err("in buffer is null!\r\n");
            return (-EPERM);
        }
        break;

    case IOCTL_AUDIN_SET_ONOFF:
    case IOCTL_AUDIN_SET_ADCIN_CTRL:
        if(NULL != up->pInBuf) {
            if(get_user(temp, &up->pInBuf))
            {
                pr_err("get user from input buffer error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }
            inputBuf32 = compat_ptr(temp);
            
            if (NULL == (inputBuf = (AUDIN_SET_ONOFF __user*)compat_alloc_user_space(sizeof(AUDIN_SET_ONOFF))))
            {
                pr_err("compat_alloc_user_space error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }

            if(get_user(((AUDIN_SET_ONOFF *)inputBuf)->lMode, &((AUDIN_SET_ONOFF*)inputBuf32)->lMode) || 
                get_user(((AUDIN_SET_ONOFF *)inputBuf)->fgAudInOnOff, &((AUDIN_SET_ONOFF*)inputBuf32)->fgAudInOnOff) ||
                get_user(((AUDIN_SET_ONOFF *)inputBuf)->eLineINGroupSel, &((AUDIN_SET_ONOFF*)inputBuf32)->eLineINGroupSel))
            {
                pr_err("get_user error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }  
            kp->pInBuf= inputBuf;
            
            set_fs(KERNEL_DS);
            i4ret = adec_ioctl(filp, cmd, kp);
            set_fs(old_fs);

        }else {
            pr_err("in buffer is null!\r\n");
            return (-EPERM);
        }
        break;
        
    case IOCTL_AUDIN_SET_ADDR:
        pr_info("[aud] do_adec_ioctrl IOCTL_AUDIN_SET_ADDR \n");
        i4ret = adec_ioctl(filp, cmd, kp);
        break;
    
    case IOCTL_AUDIN_GET_DEC_CFG_INFO:
        pr_info("[aud] do_adec_ioctrl IOCTL_AUDIN_GET_DEC_CFG_INFO \n");
        if(NULL != up->pOutBuf && up->pBytesReturned != NULL) {
            if(get_user(temp, &up->pOutBuf))
            {
                pr_err("get user from input buffer error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }
            outputBuf32 = compat_ptr(temp);
            get_user(temp, &up->pBytesReturned);
            pBytesReturned32 = compat_ptr(temp);
            sizeToAlloc =  AUD_ALIGN_SZ(sizeof(AUDIN_INFO), sizeof(uintptr_t)) +  AUD_ALIGN_SZ(sizeof(u32), sizeof(uintptr_t));
            if (NULL == (outputBuf = (AUDIN_INFO __user*)compat_alloc_user_space(sizeToAlloc)))
            {
                pr_err("compat_alloc_user_space error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }
            #if 0
            if (NULL == (pBytesReturned = (u32 __user*)compat_alloc_user_space(sizeof(u32))));
            {
                pr_err("compat_alloc_user_space error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }
            #endif
            pBytesReturned = outputBuf + AUD_ALIGN_SZ(sizeof(AUDIN_INFO), sizeof(uintptr_t));
            kp->pOutBuf= outputBuf;
            kp->OutSize = sizeof(AUDIN_INFO);
            kp->pBytesReturned= pBytesReturned;
                        set_fs(KERNEL_DS);
            i4ret = adec_ioctl(filp, cmd, kp);
            set_fs(old_fs);

            if(put_user(((AUDIN_INFO *)outputBuf)->e_audin_fmt, &((AUDIN_INFO*)outputBuf32)->e_audin_fmt) || 
                put_user(((AUDIN_INFO *)outputBuf)->e_audin_type, &((AUDIN_INFO*)outputBuf32)->e_audin_type) ||
                put_user(((AUDIN_INFO *)outputBuf)->ui4_sample_rate, &((AUDIN_INFO*)outputBuf32)->ui4_sample_rate) ||
                put_user(((AUDIN_INFO *)outputBuf)->ui4_data_rate, &((AUDIN_INFO*)outputBuf32)->ui4_data_rate) ||
                put_user(((AUDIN_INFO *)outputBuf)->ui1_bit_depth, &((AUDIN_INFO*)outputBuf32)->ui1_bit_depth) ||
                put_user(((AUDIN_INFO *)outputBuf)->ui2_pid, &((AUDIN_INFO*)outputBuf32)->ui2_pid) ||
                put_user(((AUDIN_INFO *)outputBuf)->e_linendian, &((AUDIN_INFO*)outputBuf32)->e_linendian))
            {
                pr_err("put_user error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }   
            
            if(put_user(((AUDIN_INFO *)outputBuf)->pcm_info.ePCM_Format, &((AUDIN_INFO*)outputBuf32)->pcm_info.ePCM_Format) ||
                 put_user(((AUDIN_INFO *)outputBuf)->pcm_info.b_de_emphasis, &((AUDIN_INFO*)outputBuf32)->pcm_info.b_de_emphasis) ||
               put_user(((AUDIN_INFO *)outputBuf)->pcm_info.b_dlna_exist, &((AUDIN_INFO*)outputBuf32)->pcm_info.b_dlna_exist))
            {
               pr_err("get_user error err(%i)!\r\n", -EPERM);
               return (-EPERM);
            }
            if (pBytesReturned32 != NULL)
            {
                if(copy_in_user(pBytesReturned32, pBytesReturned, sizeof(u32)))
                {
                    pr_err("put_user error err(%i)!\r\n", -EPERM);
                    return (-EPERM);
                }
            }

        }else {
            pr_err("in buffer is null!\r\n");
            return (-EPERM);
        }
        break;
        
    case IOCTL_AUDIN_REAR_VOL_GAIN_INFO:
    case IOCTL_AUDIN_FRONT_VOL_GAIN_INFO:
        pr_info("[aud] do_adec_ioctrl IOCTL_AUDIN_FRONT/REAR_VOL_GAIN_INFO \n");
        i4ret = adec_ioctl(filp, cmd, kp);
        break;
        
    case IOCTL_AUDIN_IIS_CTRL:
    case IOCTL_AUDIN_SET_IISIN_CTRL:
        if(NULL != up->pInBuf) {
            if(get_user(temp, &up->pInBuf))
            {
                pr_err("get user from input buffer error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }
            inputBuf32 = compat_ptr(temp);
            
            if (NULL == (inputBuf = (AUD_IIS_CTRL_INFO __user*)compat_alloc_user_space(sizeof(AUD_IIS_CTRL_INFO))))
            {
                pr_err("compat_alloc_user_space error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }

            if(get_user(((AUD_IIS_CTRL_INFO *)inputBuf)->lMode, &((AUD_IIS_CTRL_INFO*)outputBuf32)->lMode) || 
                get_user(((AUD_IIS_CTRL_INFO *)inputBuf)->fgAudInOnOff, &((AUD_IIS_CTRL_INFO*)outputBuf32)->fgAudInOnOff) ||
                get_user(((AUD_IIS_CTRL_INFO *)inputBuf)->rI2sInfo.eMode, &((AUD_IIS_CTRL_INFO*)outputBuf32)->rI2sInfo.eMode) ||
                get_user(((AUD_IIS_CTRL_INFO *)inputBuf)->rI2sInfo.ePinGrp, &((AUD_IIS_CTRL_INFO*)outputBuf32)->rI2sInfo.ePinGrp) ||
                get_user(((AUD_IIS_CTRL_INFO *)inputBuf)->rI2sInfo.rFmt.eMclkType, &((AUD_IIS_CTRL_INFO*)outputBuf32)->rI2sInfo.rFmt.eMclkType) ||
                get_user(((AUD_IIS_CTRL_INFO *)inputBuf)->rI2sInfo.rFmt.eFs, &((AUD_IIS_CTRL_INFO*)outputBuf32)->rI2sInfo.rFmt.eFs) ||
                get_user(((AUD_IIS_CTRL_INFO *)inputBuf)->rI2sInfo.rFmt.eCycle, &((AUD_IIS_CTRL_INFO*)outputBuf32)->rI2sInfo.rFmt.eCycle) ||
                get_user(((AUD_IIS_CTRL_INFO *)inputBuf)->rI2sInfo.rFmt.eDataFmt, &((AUD_IIS_CTRL_INFO*)outputBuf32)->rI2sInfo.rFmt.eDataFmt))
            {
                pr_err("put_user error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }   
            kp->pInBuf= inputBuf;
            
            set_fs(KERNEL_DS);
            i4ret = adec_ioctl(filp, cmd, kp);
            set_fs(old_fs);

        }else {
            pr_err("in buffer is null!\r\n");
            return (-EPERM);
        }
        break;


#if CONFIG_DRV_HDMI_RX
    case IOCTL_AUDMHL_CTL:
        pr_info("[aud] do_adec_ioctrl IOCTL_AUDMHL_CTL \n");
        if(NULL != up->pInBuf) {
            if(get_user(temp, &up->pInBuf))
            {
                pr_err("get user from input buffer error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }
            inputBuf32 = compat_ptr(temp);
            
            if (NULL == (inputBuf = (AUDMHL_OPEN_CTRL __user*)compat_alloc_user_space(sizeof(AUDMHL_OPEN_CTRL))))
            {
                pr_err("compat_alloc_user_space error err(%i)!\r\n", -EPERM);
                return ( -EPERM);
            }
            if(copy_in_user(inputBuf, inputBuf32, sizeof(AUDMHL_OPEN_CTRL)))
            {
                pr_err("copy_in_user error err(%i)!\r\n", -EPERM);
                return ( -EPERM);
            }
            kp->pInBuf= inputBuf;
            
            set_fs(KERNEL_DS);
            i4ret = adec_ioctl(filp, cmd, kp);
            set_fs(old_fs);
        }else {
            pr_err("in buffer is null!\r\n");
            return ( -EPERM);
        }
        break;
        
    case IOCTL_AUDMHL_GET_INFO:
        pr_info("[aud] do_adec_ioctrl IOCTL_AUDMHL_GET_INFO \n");
        if(NULL != up->pOutBuf) {
            if(get_user(temp, &up->pOutBuf))
            {
                pr_err("get user from input buffer error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }
            outputBuf32 = compat_ptr(temp);

            if (NULL == (outputBuf = (AUDIN_INFO_T __user*)compat_alloc_user_space(sizeof(AUDIN_INFO_T))))
            {
                pr_err("compat_alloc_user_space error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }
            kp->pOutBuf= outputBuf;
            
            set_fs(KERNEL_DS);
            i4ret = adec_ioctl(filp, cmd, kp);
            set_fs(old_fs);

            if(put_user(((AUDIN_INFO_T *)outputBuf)->u1AudinPauseStatus, &((AUDIN_INFO_T*)outputBuf32)->u1AudinPauseStatus) || 
                put_user(((AUDIN_INFO_T *)outputBuf)->u1AudinLockStatus, &((AUDIN_INFO_T*)outputBuf32)->u1AudinLockStatus) ||
                put_user(((AUDIN_INFO_T *)outputBuf)->u1AudinChStatus, &((AUDIN_INFO_T*)outputBuf32)->u1AudinChStatus) ||
                put_user(((AUDIN_INFO_T *)outputBuf)->u1AudinSampleRate, &((AUDIN_INFO_T*)outputBuf32)->u1AudinSampleRate) ||
                put_user(((AUDIN_INFO_T *)outputBuf)->u1AudinSwitchOK, &((AUDIN_INFO_T*)outputBuf32)->u1AudinSwitchOK) ||
                put_user(((AUDIN_INFO_T *)outputBuf)->u1AudinOnOffOK, &((AUDIN_INFO_T*)outputBuf32)->u1AudinOnOffOK) ||
                put_user(((AUDIN_INFO_T *)outputBuf)->u1HdmiRxINT, &((AUDIN_INFO_T*)outputBuf32)->u1HdmiRxINT) ||
                put_user(((AUDIN_INFO_T *)outputBuf)->u1SpdifAudinType, &((AUDIN_INFO_T*)outputBuf32)->u1SpdifAudinType) ||
                put_user(((AUDIN_INFO_T *)outputBuf)->u1SpdifRawDataType, &((AUDIN_INFO_T*)outputBuf32)->u1SpdifRawDataType) ||
                put_user(((AUDIN_INFO_T *)outputBuf)->u1AudinUSBNo, &((AUDIN_INFO_T*)outputBuf32)->u1AudinUSBNo) ||
                put_user(((AUDIN_INFO_T *)outputBuf)->u1HDMIRxAudFmt, &((AUDIN_INFO_T*)outputBuf32)->u1HDMIRxAudFmt))
            {
                pr_err("put_user error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }   

            copy_in_user(&((AUDIN_INFO_T *)outputBuf32)->u4HDMIIRxPCMInfo, &((AUDIN_INFO_T *)outputBuf)->u4HDMIIRxPCMInfo, sizeof(HDMI_RX_PCM_INFO_T));
            copy_in_user(&((AUDIN_INFO_T *)outputBuf32)->u8HDMIRxAudCHSTS, &((AUDIN_INFO_T *)outputBuf)->u8HDMIRxAudCHSTS, sizeof(HDMI_RX_AUDIO_CHSTS));

        }else {
            pr_err("in buffer is null!\r\n");
            return (-EPERM);
        }
        break;
    case IOCTL_AUDMHL_PARSING_INFO:
        pr_info("[aud] do_adec_ioctrl IOCTL_AUDMHL_PARSING_INFO \n");
        if(NULL != up->pOutBuf) {
            if(get_user(temp, &up->pOutBuf))
            {
                pr_err("get user from input buffer error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }
            outputBuf32 = compat_ptr(temp);

            if (NULL == (outputBuf = (AUDIN_PARSING_INFO_T __user*)compat_alloc_user_space(sizeof(AUDIN_PARSING_INFO_T))))
            {
                pr_err("compat_alloc_user_space error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }
            kp->pOutBuf= outputBuf;
            
            set_fs(KERNEL_DS);
            i4ret = adec_ioctl(filp, cmd, kp);
            set_fs(old_fs);

            if(put_user(((AUDIN_PARSING_INFO_T *)outputBuf)->fgAudinDSD, &((AUDIN_PARSING_INFO_T*)outputBuf32)->fgAudinDSD) || 
                put_user(((AUDIN_PARSING_INFO_T *)outputBuf)->u1DSDChNum, &((AUDIN_PARSING_INFO_T*)outputBuf32)->u1DSDChNum) ||
                put_user(((AUDIN_PARSING_INFO_T *)outputBuf)->fgAudinRAW, &((AUDIN_PARSING_INFO_T*)outputBuf32)->fgAudinRAW) ||
                put_user(((AUDIN_PARSING_INFO_T *)outputBuf)->fgAudinReOrder, &((AUDIN_PARSING_INFO_T*)outputBuf32)->fgAudinReOrder) ||
                put_user(((AUDIN_PARSING_INFO_T *)outputBuf)->AudinHBRAudioType, &((AUDIN_PARSING_INFO_T*)outputBuf32)->AudinHBRAudioType) ||
                put_user(((AUDIN_PARSING_INFO_T *)outputBuf)->u4PsrPcmUintSize, &((AUDIN_PARSING_INFO_T*)outputBuf32)->u4PsrPcmUintSize))
            {
                pr_err("put_user error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }   
        }else {
            pr_err("in buffer is null!\r\n");
            return (-EPERM);
        }
        break;
        
    case IOCTL_AUDMHL_RAW_INFO:
        pr_info("[aud] do_adec_ioctrl IOCTL_AUDMHL_RAW_INFO \n");
        if(NULL != up->pOutBuf) {
            if (get_user(temp, &up->pOutBuf))
            {
                pr_err("get user from out buffer error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }
            outputBuf32 = compat_ptr(temp);
            
            if (NULL == (outputBuf = (u32 __user*)compat_alloc_user_space(sizeof(u32))))
            {
                pr_err("compat_alloc_user_space error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }
            kp->pOutBuf= outputBuf;
            
            set_fs(KERNEL_DS);
            i4ret = adec_ioctl(filp, cmd, kp);
            set_fs(old_fs);
            if(copy_in_user(outputBuf32, outputBuf, sizeof(u32)))
            {
                pr_err("put_user error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }
        }else {
            pr_err("out buffer is null!\r\n");
            return (-EPERM);
        }
        break;
        
    case IOCTL_AUDMHL_GET_AUDMHLBUFFER_INFO:
        pr_info("[aud] do_adec_ioctrl IOCTL_AUDMHL_GET_AUDMHLBUFFER_INFO \n");
        i4ret = adec_ioctl(filp, cmd, kp);
        break;
    
        
#endif

    case IOCTL_AUDIO_SET_CLI_CMD_INFO:
        pr_info("[aud] do_adec_ioctrl IOCTL_AUDIO_SET_CLI_CMD_INFO \n");
        if(NULL != up->pInBuf) {
            if(get_user(temp, &up->pInBuf))
            {
                pr_err("get user from input buffer error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }
            inputBuf32 = compat_ptr(temp);
            sizeToAlloc =  AUD_ALIGN_SZ(sizeof(AUD_DEC_CLI_CFG), sizeof(uintptr_t)) +  AUD_ALIGN_SZ(sizeof(AUD_DEC_CLI_CFG), sizeof(uintptr_t));
            if (NULL == (inputBuf = (AUD_DEC_CLI_CFG __user*)compat_alloc_user_space(sizeToAlloc)))
            {
                pr_err("compat_alloc_user_space error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }
            #if 0
            if (NULL == (((AUD_DEC_CLI_CFG *)inputBuf)->ptParam = (char  __user**)compat_alloc_user_space(sizeof(char*))))
            {
                pr_err("compat_alloc_user_space error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }
            #endif
            ((AUD_DEC_CLI_CFG *)inputBuf)->ptParam = inputBuf + AUD_ALIGN_SZ(sizeof(AUD_DEC_CLI_CFG), sizeof(uintptr_t));
            get_user(temp, &((AUD_DEC_CLI_CFG32 *)inputBuf32)->ptParam);
            ((AUD_DEC_CLI_CFG *)inputBuf)->ptParam = compat_ptr(temp);
            *(((AUD_DEC_CLI_CFG *)inputBuf)->ptParam) = (char*)compat_ptr(*((char*)((AUD_DEC_CLI_CFG32 *)inputBuf32)->ptParam));

            if(get_user(((AUD_DEC_CLI_CFG *)inputBuf)->eAudCliType, &((AUD_DEC_CLI_CFG32*)inputBuf32)->eAudCliType) || 
                get_user(((AUD_DEC_CLI_CFG *)inputBuf)->u4arg1, &((AUD_DEC_CLI_CFG32*)inputBuf32)->u4arg1) ||
                get_user(((AUD_DEC_CLI_CFG *)inputBuf)->u4arg2, &((AUD_DEC_CLI_CFG32*)inputBuf32)->u4arg2) ||
                get_user(((AUD_DEC_CLI_CFG *)inputBuf)->u4arg3, &((AUD_DEC_CLI_CFG32*)inputBuf32)->u4arg3) ||
                get_user(((AUD_DEC_CLI_CFG *)inputBuf)->u4arg4, &((AUD_DEC_CLI_CFG32*)inputBuf32)->u4arg4))
            {
                pr_err("put_user error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }   
            kp->pInBuf= inputBuf;
            
            set_fs(KERNEL_DS);
            i4ret = adec_ioctl(filp, cmd, kp);
            set_fs(old_fs);

        }else {
            pr_err("in buffer is null!\r\n");
            return (-EPERM);
        }
        break;

    case IOCTL_AUDIO_SET_BT_SCO:
        pr_info("[aud] do_adec_ioctrl IOCTL_AUDIO_SET_BT_SCO \n");
        i4ret = adec_ioctl(filp, cmd, kp);
        break;
    case IOCTL_AUDIO_FUNC_OPTION_SET:
        pr_info("[aud] do_adec_ioctrl IOCTL_AUDIO_FUNC_OPTION_SET \n");
        if(NULL != up->pInBuf) {
            if(get_user(temp, &up->pInBuf))
            {
                pr_err("get user from input buffer error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }
            inputBuf32 = compat_ptr(temp);

            if (NULL == (inputBuf = (AUD_FUNC_OPTION_T __user*)compat_alloc_user_space(sizeof(AUD_FUNC_OPTION_T))))
            {
                pr_err("compat_alloc_user_space error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }

            if(get_user(((AUD_FUNC_OPTION_T *)inputBuf)->u4FuncOption0, &((AUD_FUNC_OPTION_T*)inputBuf32)->u4FuncOption0) || 
                get_user(((AUD_FUNC_OPTION_T *)inputBuf)->u4FuncOption1, &((AUD_FUNC_OPTION_T*)inputBuf32)->u4FuncOption1) ||
                get_user(((AUD_FUNC_OPTION_T *)inputBuf)->u4FuncOption2, &((AUD_FUNC_OPTION_T*)inputBuf32)->u4FuncOption2) ||
                get_user(((AUD_FUNC_OPTION_T *)inputBuf)->u4BassCutOffFreq, &((AUD_FUNC_OPTION_T*)inputBuf32)->u4BassCutOffFreq) ||
                get_user(((AUD_FUNC_OPTION_T *)inputBuf)->u4GainAvIn, &((AUD_FUNC_OPTION_T*)inputBuf32)->u4GainAvIn) ||
                get_user(((AUD_FUNC_OPTION_T *)inputBuf)->u4GainUSB, &((AUD_FUNC_OPTION_T*)inputBuf32)->u4GainUSB) ||
                get_user(((AUD_FUNC_OPTION_T *)inputBuf)->u4GainDVD, &((AUD_FUNC_OPTION_T*)inputBuf32)->u4GainDVD) ||
                get_user(((AUD_FUNC_OPTION_T *)inputBuf)->u4Reserve1, &((AUD_FUNC_OPTION_T*)inputBuf32)->u4Reserve1) ||
                get_user(((AUD_FUNC_OPTION_T *)inputBuf)->u4Reserve2, &((AUD_FUNC_OPTION_T*)inputBuf32)->u4Reserve2) ||
                get_user(((AUD_FUNC_OPTION_T *)inputBuf)->u4Reserve3, &((AUD_FUNC_OPTION_T*)inputBuf32)->u4Reserve3) ||
                get_user(((AUD_FUNC_OPTION_T *)inputBuf)->u4Reserve4, &((AUD_FUNC_OPTION_T*)inputBuf32)->u4Reserve4) ||
                get_user(((AUD_FUNC_OPTION_T *)inputBuf)->u4Reserve5, &((AUD_FUNC_OPTION_T*)inputBuf32)->u4Reserve5) ||
                get_user(((AUD_FUNC_OPTION_T *)inputBuf)->u4Reserve6, &((AUD_FUNC_OPTION_T*)inputBuf32)->u4Reserve6))
            {
                pr_err("put_user error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }   
            kp->pInBuf= inputBuf;
            
            set_fs(KERNEL_DS);
            i4ret = adec_ioctl(filp, cmd, kp);
            set_fs(old_fs);

        }else {
            pr_err("in buffer is null!\r\n");
            return (-EPERM);
        }
        break;

    case IOCTL_AUDIN_COPY_FROM_USER:
        pr_info("[aud] do_adec_ioctrl IOCTL_AUDIN_COPY_FROM_USER \n");
        if(NULL != up->pInBuf) {
            if(get_user(temp, &up->pInBuf))
            {
                pr_err("get user from input buffer error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }
            inputBuf32 = compat_ptr(temp);
            sizeToAlloc =  AUD_ALIGN_SZ(sizeof(AUD_USER_INFO), sizeof(uintptr_t)) +  AUD_ALIGN_SZ(((AUD_USER_INFO *)inputBuf)->buf_size, sizeof(uintptr_t));
            if (NULL == (inputBuf = (AUD_USER_INFO __user*)compat_alloc_user_space(sizeToAlloc)))
            {
                pr_err("compat_alloc_user_space error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }
            #if 0
            if (NULL == (((AUD_USER_INFO *)inputBuf)->puser = (char __user*)compat_alloc_user_space(((AUD_USER_INFO *)inputBuf)->buf_size)))
            {
                pr_err("compat_alloc_user_space error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }
            #endif
            ((AUD_USER_INFO *)inputBuf)->puser = inputBuf + AUD_ALIGN_SZ(sizeof(AUD_USER_INFO), sizeof(uintptr_t));
            if(get_user(((AUD_USER_INFO *)inputBuf)->buf_size, &((AUD_USER_INFO32*)inputBuf32)->buf_size))
            {
                pr_err("put_user error err(%i)!\r\n", -EPERM);
                return (-EPERM);
            }   

            if(copy_in_user(((AUD_USER_INFO *)inputBuf)->puser, ((AUD_USER_INFO32 *)inputBuf)->puser, ((AUD_USER_INFO *)inputBuf)->buf_size))
            {
                pr_err("copy_in_user error err(%i)!\r\n", -EPERM);
                return ( -EPERM);
            }
            kp->pInBuf= inputBuf;
            kp->InSize = sizeof(AUD_USER_INFO);
            
            set_fs(KERNEL_DS);
            i4ret = adec_ioctl(filp, cmd, kp);
            set_fs(old_fs);

        }else {
            pr_err("in buffer is null!\r\n");
            return (-EPERM);
        }
        break;

    default:
        pr_err("ioctl cmd is err!\r\n");
        i4ret = FALSE;
        break;
        }

    return i4ret;
 }



static s32 adec_compat_ioctl(struct file *filp, u32 cmd, u32 arg)
{
    int32_t i4ret = 0;

    WIN32_IOCTL_DATA  kp;
    WIN32_IOCTL_DATA32 __user *up = NULL;

    if (NULL == filp) {
        pr_err("[AUD] fail for file is NULL, adec_compat_ioctl\n");
        return -EINVAL;
    }

    if (0 == arg) {
        pr_err("[AUD] fail for no arg, ioctl adec_compat_ioctl\n");
        return -EINVAL;
    }
    
    if (!filp->f_op->unlocked_ioctl) {
        pr_err("[AUD] fail for no unlocked_ioctl, adec_compat_ioctl\n");
        return -ENOIOCTLCMD;
    }
    #if 0
    if (NULL == (kp = compat_alloc_user_space(sizeof(WIN32_IOCTL_DATA))))
    {
        pr_err("compat_alloc_user_space error err(%i)!\r\n", -EPERM);
        return (-EPERM);
    }
    #endif
    up = compat_ptr(arg);
    kp.pInBuf = NULL;
    kp.pOutBuf = NULL;
    kp.pBytesReturned = NULL;
    kp.InSize = 0;
    kp.OutSize = 0;
    i4ret = do_adec_ioctrl(filp, cmd, &kp, up);
    //pr_err("[AUD] do_adec_ioctrl ret = %d\n", i4ret);
    
    return i4ret;
    
}

#endif
static int32_t adec_read(struct file *filp, char __user *buf, u32 count, loff_t *f_pos)
{
    int32_t i4ret = 0;
    void *private_data;

    private_data = filp->private_data;

    if (!private_data)
        return -1;

    i4ret = ADE_Read((uintptr_t)private_data, (void*)buf, count);

    return i4ret;
}

static int32_t adec_write(struct file *filp, const char __user *buf, u32 count, loff_t *f_pos)
{
    int32_t i4ret = 0;
    void *private_data;

    private_data = filp->private_data;

    if (!private_data)
        return -1;

    i4ret = (int32_t)ADE_Write((u32)private_data, (void*)buf, count);

    return i4ret;
}

static s32 adec_open(struct inode *inode, struct file *file)
{
    s32 ret = 0;
    void *private_data;

    /* you can only open a adec device one time */
#if 0
    if (file->private_data != NULL) {
        printk(KERN_ALERT "%s:%d\n", __FUNCTION__, __LINE__);
        return -1;
    }
#endif
    private_data = (void *)ADE_Open(0, 0, 0);
  
    /* FIXME */
    file->private_data = private_data;
    return ret;
}

static s32 adec_release(struct inode *inode, struct file *file)
{
    s32 ret = 0;
    void *private_data = file->private_data;

    if (!private_data)
        return -1;

    ADE_Close((uintptr_t)private_data);

    file->private_data = NULL;

    return ret;
}

static s32 adec_mmap(struct file *filp, struct vm_area_struct *vma)
{
    void *private_data = filp->private_data;
    if (!private_data)
        return -1;
    
    return ADE_Mmap((u32)private_data, vma);    
}

struct file_operations adec_fops = {
    .open = adec_open,
    .release = adec_release,
    .read = adec_read,
    .write = adec_write,
    .unlocked_ioctl = adec_ioctl,
#ifdef CONFIG_COMPAT
    .compat_ioctl=adec_compat_ioctl,
#endif
    .mmap = adec_mmap,
};

static s32 audiodecoder_probe(struct platform_device *pdev)
{
    struct device_node *node;
    struct reserved_mem *adec_mem;
    phys_addr_t adspm_base, adspm_size;
    s32 result = -1;

    pr_info("[aud] enter audio decoder probe\r\n");
 #if CONFIG_DRV_AUD_AC83XX  
    of_reserved_mem_device_init(&(pdev->dev));
    adec_mem = (struct reserved_mem *)(pdev->dev.cma_area);
    if (!adec_mem) {
        pr_err("[aud] audio decoder get reserved memory failed!\n");
        result = -1;
        goto err_probe;
    }
    pr_info("[aud] get afifo reserve memory name %s, base 0x%x, size 0x%x\r\n", adec_mem->name, adec_mem->base, adec_mem->size);

    result = get_static_reserved_memory("dsp", &adspm_base, &adspm_size);
    pr_info("[aud] get adsp reserve memory : base 0x%lx, size 0x%lx\r\n", adspm_base, adspm_size);
        if (0 != result)
    {
    pr_err("[aud] node not exist or not a static reserved memory node\n");
    }
#endif


    node = pdev->dev.of_node;
    if (!node) {
        pr_err("[aud] audio decoder probe fail because of no adec device compatible dts node!\r\n");
        return -1;
    }

    adec_dev = kzalloc(sizeof(struct adec_dev_info), GFP_KERNEL);
    if (!adec_dev) {
        result = -ENOMEM;
        pr_err("[aud] audio decoder probe fail, alloc memory fail!\n");
        goto err_free_mem;
    }

    memset(adec_dev, 0x0, sizeof(struct adec_dev_info));

     #if CONFIG_DRV_AUD_AC83XX  
    adec_dev->adspm_base = adspm_base;
    adec_dev->adspm_size = adspm_size;
    adec_dev->afifom_base = adec_mem->base;
    adec_dev->afifom_size = adec_mem->size;
    adec_dev->adspm_base_va = (phys_addr_t)(ioremap(adec_dev->adspm_base, MT3360_ADSP_BUF_SZ));
    adec_dev->afifom_base_va = (phys_addr_t)(ioremap(adec_dev->afifom_base, SYS_AFIFO_MAX_SIZE));
     #else
     node = of_find_compatible_node(NULL, NULL, "atc-audio-dsp");
     if(node == NULL)
     {
        pr_info("[aud] node is null\r\n");
        goto err_probe;
     }
     adec_dev->adspm_base = 0x118b00000;
     adec_dev->adspm_size = 0x600000;
     adec_dev->adspm_base_va = phys_to_virt(0x118b00000);
     node = of_find_compatible_node(NULL, NULL, "atc,adec");
     io_v_base = of_iomap(node, 0); 
     io_base_ckgen = io_v_base;
     io_base_aud = io_v_base + 0x5000;
     node = of_find_compatible_node(NULL, NULL, "atc-audio-fifo");
     if(node == NULL)
     {
        pr_info("[aud] afifo node is null\r\n");
        goto err_probe;
     }
     adec_dev->afifom_base = 0x11e300000;
     adec_dev->afifom_size = 0x400000;
     adec_dev->afifom_base_va = of_iomap(node, 0);
     #endif

    adec_dev->dev = &(pdev->dev);
    adec_dev->cdev.name = AUDIODECODER_DEVNAME;
    adec_dev->cdev.minor = MISC_DYNAMIC_MINOR;
    adec_dev->cdev.fops = &adec_fops;

    pr_info("[aud] adsp / afifo reserve memory VA:(0x%lx) / (0x%lx), io_base: 0x%lx)\r\n", adec_dev->adspm_base_va, adec_dev->afifom_base_va, io_v_base);

    platform_set_drvdata(pdev, adec_dev);
    result = misc_register(&(adec_dev->cdev));
    if (result != 0) {
        pr_err("[aud] audio decoder probe fail because of misc_register, error = %d\r\n\n", result);
        goto err_unset_drvdata;
    }

    ADE_Init(NULL);

    pr_info("[aud] audio decoder probe ok\r\n");
    return 0;

err_unset_drvdata:
    platform_set_drvdata(pdev, NULL);
err_free_mem:
    kfree(adec_dev);
err_probe:
    pr_err("[aud] audio decoder probe fail \r\n");
    return result;
}

static s32 audiodecoder_remove(struct platform_device *pdev)
{
    struct adec_dev_info *adec_dev = platform_get_drvdata(pdev);

    misc_deregister(&(adec_dev->cdev));

    platform_set_drvdata(pdev, NULL);

    kfree(adec_dev);

    return 0;
}

static const struct of_device_id adec_of_ids[] = {
    {.compatible = "atc,adec",},
    {}
};

static struct platform_driver adec_of_driver = {
    .driver = {.name = "ac83xx_audiodecoder",
           .owner = THIS_MODULE,
           .of_match_table = adec_of_ids,
           },
    .probe = audiodecoder_probe,
    .remove = audiodecoder_remove,
};

//extern int32_t __init mt33xx_card_audio_init(void);
extern void __exit card_audio_exit(void);
static s32 __init adec_init(void)
{

    struct device_node *node = NULL;
    s32 result = 0;

    pr_info("[aud] enter audio decoder init\r\n");

    node = of_find_compatible_node(NULL, NULL, "atc,adec");
    if (!node) 
    {
        pr_info("[aud] audio decoder init fail in find dts compatible node!!\r\r\n");
        result = -ENOMEM;
        goto err_node;
    }
    result = platform_driver_register(&adec_of_driver);
    if (result) 
    {
        pr_err("[aud] audio decoder init fail in platform_driver_register, error = %d\r\n",result);
        goto err_node;
    }

    pr_info("[aud] audio decoder init ok!.\r\n");
    return 0;

err_node:
    return result;

}

static void __exit adec_exit(void)
{   
    card_audio_exit();

    ADE_Deinit(0);
    
    pr_info("[aud] enter audio decoder exit!\r\n");

    platform_driver_unregister(&adec_of_driver);

    pr_info("[aud] audio decoder exit ok!\r\n");
    
    return;
}

module_init(adec_init);
module_exit(adec_exit);


MODULE_AUTHOR("Autochips Inc");
MODULE_DESCRIPTION("ATC AC83xx audio Decode Driver");
MODULE_LICENSE("GPL");
