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
#include "aud_debug.h"



#define AUDIODECODER_DEVNAME "adec"


struct adec_dev_info *adec_dev;

extern u32 ADE_Init(s8 *pszContext);
extern bool ADE_Deinit(u32 dwContext);
extern u32 ADE_Open(u32 dwContext, u32 dwAccessMode, u32 dwShareMode);
extern bool ADE_Close(u32 dwContext);
extern u32 ADE_Read(u32 context, void *pBuffer, u32 dwCount);
extern u32 ADE_Write(u32 context, void *pBuffer, u32 dwCount);
extern bool ADE_IOControl(u32 context, u32 code, u8 *pInBuffer, u32 inSize,
                  u8 *pOutBuffer, u32 outSize, u32 *pOutSize);
extern s32 ADE_Mmap(u32 dwContext, struct vm_area_struct *vma);
extern s32 get_static_reserved_memory(const char *uname, phys_addr_t *base, phys_addr_t *size);


static s32 adec_ioctl(struct file *filp, u32 cmd, u32 arg)
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
         bRet = ADE_IOControl((unsigned int)private_data, cmd, (u8 *)win_ioctl.pInBuf, win_ioctl.InSize,
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

    bRet = ADE_IOControl((unsigned int)private_data, cmd, (u8 *)pInBuf, win_ioctl.InSize, (u8 *)pOutBuf, win_ioctl.OutSize, &dwReturn);

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
            return -10;
        }
    }

    return (bRet ? 0 : -1);
}

static int32_t adec_read(struct file *filp, char __user *buf, u32 count, loff_t *f_pos)
{
    int32_t i4ret = 0;
    void *private_data;

    private_data = filp->private_data;

    if (!private_data)
        return -1;

    i4ret = ADE_Read((u32)private_data, (void*)buf, count);

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

    ADE_Close((u32)private_data);

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
    .mmap = adec_mmap,
};

static s32 audiodecoder_probe(struct platform_device *pdev)
{
	struct device_node *node;
    struct reserved_mem *adec_mem;
    phys_addr_t adspm_base, adspm_size;
    
	s32 result = -1;

    LOG(LOG_INFO, "enter audio decoder probe\r\n");
   
	of_reserved_mem_device_init(&(pdev->dev));
	adec_mem = (struct reserved_mem *)(pdev->dev.cma_area);
	if (!adec_mem) {
		LOG(LOG_FAIL, "audio decoder get reserved memory failed!\n");
		result = -1;
		goto err_probe;
	}
	LOG(LOG_INFO, "get afifo reserve memory name %s, base 0x%x, size 0x%x\r\n", adec_mem->name, adec_mem->base, adec_mem->size);

	result = get_static_reserved_memory("dsp", &adspm_base, &adspm_size);
	if (0 != result)
		LOG(LOG_FAIL, "node not exist or not a static reserved memory node\n");
	
	LOG(LOG_INFO, "get adsp reserve memory : base 0x%x, size 0x%x\r\n", adspm_base, adspm_size);

    node = pdev->dev.of_node;
	if (!node) {
		LOG(LOG_FAIL, "audio decoder probe fail because of no adec device compatible dts node!\r\n");
		return -1;
	}

	adec_dev = kzalloc(sizeof(struct adec_dev_info), GFP_KERNEL);
	if (!adec_dev) {
		result = -ENOMEM;
		LOG(LOG_FAIL, "audio decoder probe fail, alloc memory fail!\n");
		goto err_free_mem;
	}

	memset(adec_dev, 0x0, sizeof(struct adec_dev_info));

	adec_dev->adspm_base = adspm_base;
	adec_dev->adspm_size = adspm_size;
	adec_dev->afifom_base = adec_mem->base;
	adec_dev->afifom_size = adec_mem->size;
	adec_dev->adspm_base_va = (phys_addr_t)(ioremap(adec_dev->adspm_base, MT3360_ADSP_BUF_SZ));
	adec_dev->afifom_base_va = (phys_addr_t)(ioremap(adec_dev->afifom_base, SYS_AFIFO_MAX_SIZE));

	adec_dev->dev = &(pdev->dev);
	adec_dev->cdev.name = AUDIODECODER_DEVNAME;
	adec_dev->cdev.minor = MISC_DYNAMIC_MINOR;
	adec_dev->cdev.fops = &adec_fops;

	LOG(LOG_INFO, "adsp / afifo reserve memory VA:(0x%x) / (0x%x)\r\n", adec_dev->adspm_base_va, adec_dev->afifom_base_va);

	platform_set_drvdata(pdev, adec_dev);

	result = misc_register(&(adec_dev->cdev));
	if (result != 0) {
	    pr_err("[aud] audio decoder probe fail because of misc_register, error = %d\r\n\n", result);
		goto err_unset_drvdata;
	}

	ADE_Init(NULL);

	LOG(LOG_INFO, "audio decoder probe ok\r\n");
	return 0;

err_unset_drvdata:
	platform_set_drvdata(pdev, NULL);
err_free_mem:
	kfree(adec_dev);
err_probe:
	LOG(LOG_FAIL, "audio decoder probe fail \r\n");
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

    LOG(LOG_INFO, "enter audio decoder init\r\n");

    node = of_find_compatible_node(NULL, NULL, "atc,adec");
    if (!node) 
    {
        LOG(LOG_FAIL, "audio decoder init fail in find dts compatible node!!\r\r\n");
        result = -ENOMEM;
        goto err_node;
    }

    result = platform_driver_register(&adec_of_driver);
    if (result) 
    {
        LOG(LOG_FAIL, "audio decoder init fail in platform_driver_register, error = %d\r\n",result);
        goto err_node;
    }

    LOG(LOG_INFO, "audio decoder init ok!.\r\n");
    return 0;

err_node:
    return result;

}

static void __exit adec_exit(void)
{   
    card_audio_exit();

    ADE_Deinit(0);
    
    LOG(LOG_INFO, "enter audio decoder exit!\r\n");

	platform_driver_unregister(&adec_of_driver);

	LOG(LOG_INFO, "audio decoder exit ok!\r\n");
    
    return;
}

module_init(adec_init);
module_exit(adec_exit);


MODULE_AUTHOR("Autochips Inc");
MODULE_DESCRIPTION("ATC AC83xx audio Decode Driver");
MODULE_LICENSE("GPL");
