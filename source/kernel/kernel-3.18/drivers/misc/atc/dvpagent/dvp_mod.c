/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of AutoChips Inc. (C) 2008 AutoChips Inc.
*
*  BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
*  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("AUTOCHIPS SOFTWARE")
*  RECEIVED FROM AUTOCHIPS AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON
*  AN "AS-IS" BASIS ONLY. AUTOCHIPS EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
*  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
*  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
*  NEITHER DOES AUTOCHIPS PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
*  SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
*  SUPPLIED WITH THE AUTOCHIPS SOFTWARE, AND BUYER AGREES TO LOOK ONLY TO SUCH
*  THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. AUTOCHIPS SHALL ALSO
*  NOT BE RESPONSIBLE FOR ANY AUTOCHIPS SOFTWARE RELEASES MADE TO BUYER'S
*  SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*
*  BUYER'S SOLE AND EXCLUSIVE REMEDY AND AUTOCHIPS'S ENTIRE AND CUMULATIVE
*  LIABILITY WITH RESPECT TO THE AUTOCHIPS SOFTWARE RELEASED HEREUNDER WILL BE,
*  AT AUTOCHIPS'S OPTION, TO REVISE OR REPLACE THE AUTOCHIPS SOFTWARE AT ISSUE,
*  OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY BUYER TO
*  AUTOCHIPS FOR SUCH AUTOCHIPS SOFTWARE AT ISSUE.
*
*  THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE
*  WITH THE LAWS OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF
*  LAWS PRINCIPLES.  ANY DISPUTES, CONTROVERSIES OR CLAIMS ARISING THEREOF AND
*  RELATED THERETO SHALL BE SETTLED BY ARBITRATION IN SAN FRANCISCO, CA, UNDER
*  THE RULES OF THE INTERNATIONAL CHAMBER OF COMMERCE (ICC).
*
*****************************************************************************/

/*****************************************************************************
 *
 * Filename:
 * ---------
 *   $Workfile:$
 *
 * Project:
 * --------
 * AC8015 Project
 *
 * Description:
 * ------------
 * video decode driver kernel module
 *
 * Author:
 * -------
 * mtk40505 : 2011-04, draft
 *
 * $Modtime:  2015-10
 *
 ****************************************************************************/

#include "agent.h"
#include <linux/cdev.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/ioctl.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <asm/uaccess.h>
#include <linux/miscdevice.h>
#include <linux/printk.h>
#include <asm/page.h>
#include "agent_drv.h"
#include "mm_debug.h"
#include "dvp_mod.h"
#include "dvp_hal.h"
#include <asm/io.h>

#include "x_ver.h" /*add_version_info*/

MODULE_LICENSE("GPL");

#define BTDRV_MODE_NAME                   "DVPDRV"
#define BTDRV_VER_MAJOR                   01
#define BTDRV_VER_MINOR                   01
#define BTDRV_VER_REV                     00

MMLOG_DECLARATION(TEXT("DVPK"));
#define ATC_KERNEL_LINUX_LICENSE     "Proprietary"

#define DLOG(x...)

struct dvp_dev {
    struct miscdevice cdev;   /* Char device structure */
    u32 dwDVPContext;         /*Char device handle*/
    s32 u4UserCount;         /*Char device' user counter*/

    struct fasync_struct *async_quene;

    #ifdef DVP_PM_SUPPORT
        struct device *dev;
    #endif
};

struct resvd_mem_info dvp_share_rsv_mem;
struct resvd_mem_info dsp_share_rsv_mem;
struct dvp_dev g_dvpDevices;
bool g_isOpenWch = false;

#ifdef DVP_PM_SUPPORT
#include "oal.h"
#include <linux/module.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/pm.h>
#include "x_ckgen.h"

enum DVP_PM_STATE {
    DVP_POWER_STATE_INVALID,
    DVP_POWER_STATE_ON,
    DVP_POWER_STATE_OFF
};

#define DVD_START_FLAG_ADDR         (0x00800000)
#define DVD_START_FLAG_MEM_LEN      (16)

enum DVP_PM_STATE g_DvpPowerState = DVP_POWER_STATE_ON;

s32 dvp_start_ok_check(void)
{
    s8 flagStr[DVD_START_FLAG_MEM_LEN];
    void *dvpStartFlagStr = (void *) changePhyToVirtualAddress(GetDvpMemBaseAddr() + DVD_START_FLAG_ADDR);

    memcpy(flagStr, dvpStartFlagStr, DVD_START_FLAG_MEM_LEN);
    flagStr[DVD_START_FLAG_MEM_LEN - 1] = 0;
    pr_debug("[dvp][drv][mod][flag] dvp_start_ok_check -- now start flag:%s\r\n",
        flagStr);

    return strcmp(flagStr, "DVP Startup OK!");
}

void erase_start_ok_flag(void)
{
    s8 flagStr[DVD_START_FLAG_MEM_LEN];
    void *dvpStartFlagStr = (void *) changePhyToVirtualAddress(GetDvpMemBaseAddr()
        + DVD_START_FLAG_ADDR);

    memcpy(flagStr, dvpStartFlagStr, DVD_START_FLAG_MEM_LEN);
    flagStr[DVD_START_FLAG_MEM_LEN - 1] = 0;
    pr_debug("[dvp][drv][mod][flag] erase_start_ok_flag -- now start flag:%s  -- erase it\r\n",
        flagStr);

    memset(dvpStartFlagStr, 0, DVD_START_FLAG_MEM_LEN);
}

s32 dvp_suspend(struct device *dev)
{
    pr_info("[dvp][drv][mod] dvp_suspend -- g_DvpPowerState: %d!\r\n",
        g_DvpPowerState);

    if (DVP_POWER_STATE_OFF == g_DvpPowerState) {
        pr_debug("[dvp][drv][mod] dvp_suspend exit, g_DvpPowerState == DVP_POWER_STATE_OFF\r\n");
        return 0;
    }
    DVPAgent_PowerOff();
    g_DvpPowerState = DVP_POWER_STATE_OFF;
    pr_debug("[dvp][drv][mod] dvp_suspend exit\r\n");
    erase_start_ok_flag();
    return 0;
}

s32 dvp_resume(struct device *dev)
{
    pr_info("[dvp][drv][mod] dvp_resume -- g_DvpPowerState: %d!\r\n",
        g_DvpPowerState);

    if (DVP_POWER_STATE_ON == g_DvpPowerState) {
        pr_debug("[dvp][drv][mod] dvp_resume exit, g_DvpPowerState == DVP_POWER_STATE_ON!\r\n");
        return 0;
    }

    DVPAgent_PowerOn();

    g_DvpPowerState = DVP_POWER_STATE_ON;
    pr_info("[dvp][drv][mod] dvp_resume exit, success -- g_DmxPmState: %d!\r\n",
        g_DvpPowerState);

    return 0;
}

static const struct dev_pm_ops dvp_pm_ops = {
    SET_SYSTEM_SLEEP_PM_OPS(dvp_suspend, dvp_resume)
};

/*-------------------------------*/
#endif /* DVP_PM_SUPPORT*/

static s32 mt_dvp_ioctl(struct file *file, u32 cmd, u32 arg)
{
    WIN32_IOCTL_DATA *pData = (WIN32_IOCTL_DATA *)arg;
    if (0 == g_dvpDevices.dwDVPContext) {
        pr_err("[dvp][drv][mod] mt_dvp_ioctl dwDVPContext error:[file = %s function = %s lineNo = %d]\r\n", FILE_ONLY, __func__, __LINE__);
        return -1;
    }

    pr_debug("[dvp][driver][mt_dvp_ioct1] cmd = %d \r\n", cmd);
    if (DVP_IOControl(g_dvpDevices.dwDVPContext, cmd, pData->pInBuf,
                pData->InSize, /*nInBufferSize*/
                pData->pOutBuf, /*lpOutBuffer*/
                pData->OutSize, /*nOutBufferSize*/
                pData->pBytesReturned /*lpBytesReturned*/
        )) {
        return 0;
    } else {
        return -1;
    }
}


static s32 mt_dvp_open(struct inode *inode, struct file *file)
{
    s32 ret = 0;
    pr_info("[dvp][drv][mod]    mt_dvp_open\r\n");
    g_dvpDevices.u4UserCount++;
    if (g_dvpDevices.u4UserCount >= 2) {
        pr_debug("[dvp][drv][mod]   dvp driver has been open no need close again\r\n");
        return 0;
    }

    if (dvp_start_ok_check() != 0) {
        g_dvpDevices.u4UserCount--;
        return -1;
    }

    g_dvpDevices.dwDVPContext = DVP_Open(0, 0, 0);
    if (g_dvpDevices.dwDVPContext == 0) {
    pr_debug("[dvp][drv][mod]  mt_dvp_open failed!\r\n");
        ret = -1;
    } else {
        pr_debug("[dvp][drv][mod]   mt_dvp_open success!\r\n");
        ret = 0 ;
    }
    return ret;
}

static s32 mt_dvp_release(struct inode *inode, struct file *file)
{
    pr_info("[dvp][drv][mod]  mt_dvp_release\r\n");

    if ((g_dvpDevices.u4UserCount <= 0) ||
        (g_dvpDevices.dwDVPContext == 0)) {
        pr_debug("[dvp][drv][mod]  dvp driver has been release no need close again\r\n");
        return 0;
    }

    g_dvpDevices.u4UserCount--;
    if (g_dvpDevices.u4UserCount == 0) {
        pr_debug("[dvp][drv][mod] mt_dvp_release ++++++++++++++ DVP_Close\r\n");
        DVP_Close(g_dvpDevices.dwDVPContext);
        g_dvpDevices.dwDVPContext = 0;
    }

    return 0;
}

const struct file_operations dvp_fops = {
    .release = mt_dvp_release,
    .open = mt_dvp_open,
    .unlocked_ioctl = mt_dvp_ioctl,
};

#ifdef DVP_PM_SUPPORT
static int dvp_probe(struct platform_device *pdev)
{
    s32 result = 0;
    pr_info("[dvp][drv][mod] dvp_probe enter!!\r\n");

    memset(&g_dvpDevices, 0, sizeof(struct dvp_dev));

    g_dvpDevices.cdev.name  = "atc_dvp";
    g_dvpDevices.cdev.minor = MISC_DYNAMIC_MINOR;
    g_dvpDevices.cdev.fops  = &dvp_fops;
    g_dvpDevices.dev = &(pdev->dev);

    result = misc_register(&(g_dvpDevices.cdev));

    if (result != 0) {
        pr_debug("[dvp][drv][mod] dvp_probe fail in misc_register,");
        pr_debug(" error = %d\r\n", result);
        return -1;
    }

    LOG_ModInit();
    DVP_Init();

    pr_info("[dvp][drv][mod] dvp_probe success!!\r\n");
    return 0;
}

static int dvp_remove(struct platform_device *pdev)
{
    pr_info("[dvp][drv][mod] dvp_remove\r\n");

    misc_deregister(&(g_dvpDevices.cdev));
    DVP_Deinit();
    LOG_ModDeinit();
    return 0;
}

static struct platform_driver dvp_driver = {
    .probe   = dvp_probe,
    .remove  = dvp_remove,
    .driver = { .name = "atc_dvp",
                .owner = THIS_MODULE,
                .pm   = &dvp_pm_ops,}
};

static struct platform_device dvp_device = {
    .name = "atc_dvp",
    .id = -1,
};

#endif


static int __init mt_dvp_init(void)
{
    s32 result;
    s32 ret = 0;
    /*add_version_info*/
    MOD_VERSION_INFO(BTDRV_MODE_NAME, BTDRV_VER_MAJOR, BTDRV_VER_MINOR, BTDRV_VER_REV);

#ifdef DVP_PM_SUPPORT

    pr_info("[dvp][drv][mod] mt_dvp_init enter!!\r\n");

    result = os_device_register(&dvp_device);
    if (result) {
        pr_err("[dvp][drv][mod] mt_dvp_init fail in os_device_register, error = %d:[file = %s function = %s lineNo = %d]\r\n", result, FILE_ONLY, __func__, __LINE__);
        goto err_platform_device_register;
    }

    result = os_driver_register(&dvp_driver);
    if (result) {
        pr_err("[dvp][drv][mod] mt_dvp_init fail in os_driver_register, error = %d, [file = %s function = %s lineNo = %d]\r\n", result, FILE_ONLY, __func__, __LINE__);
        goto err_platform_driver_register;
    }

    memset((void *)(&dvp_share_rsv_mem), 0, sizeof(dvp_share_rsv_mem));
    ret = get_static_reserved_memory("dvp", &(dvp_share_rsv_mem.base), &(dvp_share_rsv_mem.size));
    if (ret) {
        pr_err("[dvp][drv][mod]can not findnode reserve memory![file = %s function = %s lineNo = %d]\r\n", FILE_ONLY, __func__, __LINE__);
        return -1;
    }

    dvp_share_rsv_mem.virt_addr = ioremap(dvp_share_rsv_mem.base, dvp_share_rsv_mem.size);
    if (NULL == dvp_share_rsv_mem.virt_addr) {
        pr_err("[DVP]can not ioremap dvp reserve mem[file = %s function = %s lineNo = %d]\r\n", FILE_ONLY, __func__, __LINE__);
        return -2;
    }

    memset((void *)(&dsp_share_rsv_mem), 0, sizeof(dsp_share_rsv_mem));
    ret = get_static_reserved_memory("dsp", &(dsp_share_rsv_mem.base), &(dsp_share_rsv_mem.size));
    if (ret) {
        pr_err("[DVP]can not find [dsp] node reserve memory![file = %s function = %s lineNo = %d]\r\n", FILE_ONLY, __func__, __LINE__);
        return -1;
    }

    dsp_share_rsv_mem.virt_addr = ioremap(dsp_share_rsv_mem.base, dsp_share_rsv_mem.size);
    if (NULL == dsp_share_rsv_mem.virt_addr)
    {
        pr_err("[DVP]can not ioremap dsp reserve mem[file = %s function = %s lineNo = %d]\r\n", FILE_ONLY, __func__, __LINE__);
        return -2;
    }

    pr_debug("[dvp][drv][mod] mt_dvp_init success\r\n");

    return result;
    pr_debug("[dvp][drv][mod] mt_dvp_init fail !!!\r\n");

err_platform_driver_register:
    os_driver_unregister(&dvp_driver);
err_platform_device_register:
    os_device_unregister(&dvp_device);
    return -1;

#else /* DVP_PM_SUPPORT*/
    LOG_ModInit();
    pr_info("[dvp][drv][mod] ++++++++++++++++++++++++++++ mt_dvp_init ++++++++++++++++++++++++\r\n");

    memset(&g_dvpDevices, 0, sizeof(struct dvp_dev));

    g_dvpDevices.cdev.name  = "dvp";
    g_dvpDevices.cdev.minor = MISC_DYNAMIC_MINOR;
    g_dvpDevices.cdev.fops  = &dvp_fops;
    pr_debug("[dvp][drv][mod] misc_register\r\n");
    result = misc_register(&(g_dvpDevices.cdev));

    if (result != 0)
        pr_err("[dvp][drv][mod] dvp init error=%d, [file = %s function = %s lineNo = %d]\r\n", result, FILE_ONLY, __func__, __LINE__);

    DVP_Init();
    g_isOpenWch = false;
    return result;
#endif
}

static void __exit mt_dvp_exit(void)
{
    pr_debug("[dvp][drv][mod] mt_dvp_exit\r\n");

    if (dvp_share_rsv_mem.virt_addr) {
        iounmap(dvp_share_rsv_mem.virt_addr);
   }
   if (dsp_share_rsv_mem.virt_addr) {
        iounmap(dsp_share_rsv_mem.virt_addr);
   }
#ifdef DVP_PM_SUPPORT
    os_driver_unregister(&dvp_driver);
    os_device_unregister(&dvp_device);
#else
    misc_deregister(&(g_dvpDevices.cdev));
    g_isOpenWch = false;
    DVP_Deinit();
    LOG_ModDeinit();
#endif
}

module_init(mt_dvp_init);
module_exit(mt_dvp_exit);

//MODULE_AUTHOR("mtk68570");
//MODULE_LICENSE(ATC_KERNEL_LINUX_LICENSE);




