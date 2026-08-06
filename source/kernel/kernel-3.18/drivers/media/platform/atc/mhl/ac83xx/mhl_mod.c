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
#include <linux/of_irq.h>

#include "x_typedef.h"
#include "oal.h"
#include "windev.h"
#include "mhl_drv.h"
#include "drv_hdmi.h"
#include "hdmi_debug.h"
#include "mhl_drv_if.h"
#include "soc_cfg.h"
#include "mhl_rx_cbus_ctrl.h"
#include "mhl_mod.h"
#include <x_ver.h>
#include "metazone.h"
#include "tee_drv.h"
#include <generated/atc_project.h>

#define ATC_KERNEL_LINUX_LICENSE     "Proprietary"
#define IO_PHYS_ADDR 0x10000000
#define IO_PHYS_SIZE 0x00400000

bool forceRxsence = FALSE;
bool mhlrx_Single = FALSE;
struct semaphore mhlrx_sem;
int  HVdoRect[4] = {0};


bool  isHDMIstop = false;

unsigned long  g_IO_VBASE_VA = 0;
unsigned int  g_VECTOR_HDMI = 0;
unsigned int  g_VECTOR_CBUSINT = 0;


#ifdef SUPPORT_KEY_IN_TRUST_ZONE
#define ENCRYPT_KEY_1_4_LENGTH      (320)
#define COMBINATION_KEY_1_4          0x1
#endif

#ifdef HDMI_BURN_IN
static int thr_fun(void *unused)
{
	static unsigned int errcnt;
	BOOL bCRCPass = FALSE;

	HDMI_LOG(HDMI_LOG_INFO, "Start check CRC");

	for (;;) {
		msleep(1000);

		if (HdmiIsHVStable() && HdmiIsPclkStable()) {
			bCRCPass = HDMICRC(50);

			if (!bCRCPass) {
				HDMI_LOG(HDMI_LOG_DEBUG, "****************************************");
				HDMI_LOG(HDMI_LOG_INFO, "*********  CRC Fail ********************");
				HDMI_LOG(HDMI_LOG_DEBUG, "****************************************");

				HDMI_LOG(HDMI_LOG_DEBUG, "thr_fun exit");
				errcnt++;
				/* break; */
			} else {
				HDMI_LOG(HDMI_LOG_DEBUG, "errcnt = %d", errcnt);
			}

		} else {
			HDMI_LOG(HDMI_LOG_INFO, "SYNC Not stable");
		}
	}
}
#endif
static int mhl_mmap(struct file *fp, struct vm_area_struct *vma)
{
    int pag_size;
    long pfn;

	vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);
	pfn = (0xF0022000) + ((0xE80) >> PAGE_SHIFT);
    pag_size = vma->vm_end - vma->vm_start;
    /*printk("pag_size =0x%x\n",pag_size);
    printk("vma->vm_start=0x%x", vma->vm_start);
    printk("vma->vm_page_prot=0x%x", vma->vm_page_prot);*/
    /* Remap-pfn-range will mark the range VM_IO and VM_RESERVED */
    vma->vm_flags |= VM_IO;
    /*vma->vm_flags |= VM_RESERVED;*/
    if (remap_pfn_range(vma,
                vma->vm_start,
                pfn,
                vma->vm_end - vma->vm_start,
                vma->vm_page_prot))
        return -EAGAIN;

    return 0;
}

atc_hdmi_isr_data isr_data = {NULL, NULL};
bool register_isr = FALSE;

int atc_hdmi_register_isr(atc_hdmi_isr_t isr, void *arg)
{
	if (isr == NULL)
		{
		HDMI_LOG(HDMI_LOG_INFO, "atc_hdmi_register_isr return -EINVAL\n");
		return -EINVAL;
		}

	if ((isr_data.isr != NULL) && (isr_data.isr != isr)
	    && (isr_data.arg != arg))
		{
		HDMI_LOG(HDMI_LOG_INFO, "atc_hdmi_register_isr return -EBUSY\n");
		return -EBUSY;
		}

	isr_data.isr = isr;
	isr_data.arg = arg;
	register_isr = TRUE;
	HDMI_LOG(HDMI_LOG_INFO, "atc_hdmi_register_isr return TRUE\n");

	return 0;
}
EXPORT_SYMBOL(atc_hdmi_register_isr);
int atc_hdmi_unregister_isr(atc_hdmi_isr_t isr, void *arg)
{
	if (isr == NULL)
		return -EINVAL;

	if ((isr_data.isr == isr) && (isr_data.arg == arg)) {
		isr_data.isr = isr;
		isr_data.arg = arg;
		register_isr = FALSE;

		return 0;
	}

	return -EBUSY;
}
EXPORT_SYMBOL(atc_hdmi_unregister_isr);


int mhl_open(struct inode *inode, struct file *file)
{
	down(&mhlrx_sem);

	if (mhlrx_Single) {
		HDMI_LOG(HDMI_LOG_DEBUG, "mhlrx has opened");
		up(&mhlrx_sem);
		return -1;
	}

	mhlrx_Single = TRUE;
	up(&mhlrx_sem);

	HDMI_LOG(HDMI_LOG_INFO, "MHL open success");
	return 0;
}
EXPORT_SYMBOL(mhl_open);
int mhl_release(struct inode *inode, struct file *filp)
{
	down(&mhlrx_sem);
	mhlrx_Single = FALSE;
	up(&mhlrx_sem);

	HDMI_LOG(HDMI_LOG_INFO, "MHL release success");
	return 0;
}
EXPORT_SYMBOL(mhl_release);
BOOL fgMHLSupport = FALSE;
BOOL fgResume = FALSE;

long MHL_IOControl(DWORD cmd, UCHAR *pInBuffer, DWORD inSize, UCHAR *pOutBuffer, DWORD outSize){
    unsigned int bret = 0;
    unsigned int keyValue = 0;
    MHL_WCH_BUF_ADDR tMhlBufAddr;
    memset(&tMhlBufAddr,0, sizeof(MHL_WCH_BUF_ADDR));

    switch(cmd)
    {
    case IOCTL_MHL_INIT:
        HDMI_LOG(HDMI_LOG_INFO, "MHL RX INIT \r\n");
        break;

    case IOCTL_MHL_CONFIG:
        HDMI_LOG(HDMI_LOG_INFO, "MHL RX CONFIG \r\n");
        /*MHL_DRV_CONFIG_T *pConfig = pInBuffer;*/
        /*HDMI_RX_Config(*pConfig);*/
        break;

    case IOCTL_MHL_START:
        HDMI_LOG(HDMI_LOG_INFO, "MHL start \r\n");
        isHDMIstop = false;
        HDMI_RX_Start();
        break;

    case IOCTL_MHL_STOP:
        HDMI_LOG(HDMI_LOG_INFO, "MHL stop \r\n");
        isHDMIstop = true;
        HDMI_RX_Stop();
        break;

    case IOCTL_MHL_GET_VIDEO_INFO:
        HDMI_LOG(HDMI_LOG_INFO, "MHL RX GET VIDEO INFO \r\n");
        HDMI_RX_GetVideoInfo((MHL_VIDEO_INFO_T *)pOutBuffer);
        break;

    case IOCTL_MHL_GET_DEVICE_TYPE:
        HDMI_LOG(HDMI_LOG_INFO, "MHL RX GET DEVICE TYPE \r\n");
        *((MHL_DEVICE_TYPE_T *)pOutBuffer) = HDMI_RX_GetDeviceType();
        break;

    case IOCTL_MHL_SEND_RCPKEY:
#ifdef HDMI_BURN_IN
        enable_output = 1;
#endif
        /*memcpy(&keyValue, pbuf->pInBuf,pbuf->InSize);*/
        keyValue = *((unsigned int *)pInBuffer);
        HDMI_LOG(HDMI_LOG_INFO, "Send Rcp Key:%d \r\n", keyValue);

        if (keyValue > RCPKEY_MAX) {
            HDMI_LOG(HDMI_LOG_ERROR, "Send Rcp Key:%d is out of range,\r\n", keyValue); 
            bret = 1;
        } else {
            vTrigRCPMsg(keyValue);
            bret = 0;
        }
        break;

	case IOCTL_MHL_PROG_HDCP:
        HDMI_LOG(HDMI_LOG_WARN, "Program HDCP Key to SRAM \r\n");
        {
			WIN32_IOCTL_DATA pData;
			HDCP_KEY_ST *hk;

			pData.pInBuf = pInBuffer;
			pData.InSize = inSize;
			pData.pOutBuf = pOutBuffer;
			pData.OutSize = outSize;
			pData.pBytesReturned = NULL;

            hk = (HDCP_KEY_ST*)(&pData);
            if(!fgMHLSupport) {
                HDMI_LOG(HDMI_LOG_DEBUG, "No support MHL \r\n");
                break;
            }
            HDMI_RX_LoadHDCPKeyToSRAM(hk);
        }
        break;

    case IOCTL_MHL_IS_SUPPORT:
        HDMI_LOG(HDMI_LOG_INFO, "Get type of HDMI \r\n");
        {
            BOOL *pfgSupport = (BOOL *)pOutBuffer;
            *pfgSupport = fgMHLSupport;
            if(*pfgSupport) {
                HDMI_LOG(HDMI_LOG_INFO, "supported \r\n");
                bret = 0;
            }
            else {
                HDMI_LOG(HDMI_LOG_INFO, "unsupported \r\n");
                bret = 1;
            }
        }
        break;
        
    case IOCTL_MHL_GET_SINGAL_STATUS:
        {
            UINT32 *signal_status;
            UINT32 hdmi_signal;

			signal_status = (UINT32 *)pOutBuffer;
			hdmi_signal = 0;
            HDMI_LOG(HDMI_LOG_INFO, "Get HDMI Signal Status \r\n");
            hdmi_signal = HDMI_RX_GetSignalStatus();
            memcpy(signal_status, &hdmi_signal, sizeof(UINT32));
        }
        break;

    default:
        break;
    }

    return bret;

}
EXPORT_SYMBOL(MHL_IOControl);


static long mhl_ioctl(struct file *filp, unsigned int cmd, unsigned long arg){
    WIN32_IOCTL_DATA win32_ioctl;
    HDCP_KEY_ST KeyData;
    UCHAR *pIn = NULL;
    UCHAR *pOut = NULL;
    void __user *param = (void __user *)arg;

    if (IOCTL_MHL_PROG_HDCP == cmd) {
        HDMI_LOG(HDMI_LOG_WARN, "Program HDCP Key to SRAM \r\n");
        if (!fgMHLSupport) {
            HDMI_LOG(HDMI_LOG_DEBUG, "No support MHL \r\n");
            return 0;
        }
        if (0 != copy_from_user(&KeyData, param, sizeof(KeyData)))
        {
            HDMI_LOG(HDMI_LOG_INFO, "copy_from_user failed in mhl_ioctl() when copy param!\r\n");
            return -1;
        }

        HDMI_RX_LoadHDCPKeyToSRAM2(&KeyData);
        return 0;
    }

    if (0 != copy_from_user((void *)&win32_ioctl, param, sizeof(win32_ioctl)))
    {
        HDMI_LOG(HDMI_LOG_INFO, "copy_from_user failed in mhl_ioctl() when copy param!\r\n");
        return -1;
    }

    if (win32_ioctl.InSize > 0) {
        pIn = (UCHAR *)kmalloc(win32_ioctl.InSize, GFP_KERNEL);
        if (NULL == pIn) {
            HDMI_LOG(HDMI_LOG_INFO, "kmalloc input_buf memory fail\n");
            return -1;
        } else {
            if(copy_from_user(pIn, (void __user *)win32_ioctl.pInBuf, win32_ioctl.InSize)) {
                HDMI_LOG(HDMI_LOG_INFO, "copy_from_user failed in mhl_ioctl() when copy pInBuf!\r\n");
				kfree(pIn);
                return -1;
            }
        }
    }

    if (win32_ioctl.OutSize> 0) {
        pOut= (UCHAR *)kmalloc(win32_ioctl.OutSize, GFP_KERNEL);
        if (NULL == pOut) {
            HDMI_LOG(HDMI_LOG_INFO, "kmalloc out_buf memory fail\n");
			if (NULL != pIn) {
        		kfree(pIn);
			}
            return -1;
        }
    }

    if (MHL_IOControl(cmd, pIn, win32_ioctl.InSize, pOut, win32_ioctl.OutSize))
    {
        HDMI_LOG(HDMI_LOG_INFO, "MHL_IOControl cmd = %d fail\n", cmd);
        if (NULL != pIn) {
            kfree(pIn);
    	}
        if (NULL != pOut) {
            kfree(pOut);
        }
        return -1;
    }

    if (win32_ioctl.OutSize > 0 && NULL != pOut) {
        if (copy_to_user((void __user *)win32_ioctl.pOutBuf, pOut, win32_ioctl.OutSize)) {
            HDMI_LOG(HDMI_LOG_INFO, "copy_to_user() failed in mhl_ioctl()when copy pOutBuf!\r\n");
			if (NULL != pIn) {
				kfree(pIn);
			}
			if (NULL != pOut) {
				kfree(pOut);
			}
            return -1;
        }
    }

    if (NULL != pIn) {
        kfree(pIn);
    }

    if (NULL != pOut) {
        kfree(pOut);
    }

    return 0;

}

const struct file_operations mhl_fops = {
	.open           = mhl_open,
	.release        = mhl_release,
	.mmap           = mhl_mmap,
	.unlocked_ioctl = mhl_ioctl,
};

static struct miscdevice mhl_dev = {
	MISC_DYNAMIC_MINOR,
	"mhldrv",
	&mhl_fops
};

static ssize_t mhlrx_cmd_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	ssize_t size;
	unsigned int sub_cmd = 0;

	size = sprintf(buf, "loglevel = %u\r\n", sub_cmd);
	HDMI_LOG(HDMI_LOG_DEBUG, "**********************************************\r\n");
	HDMI_LOG(HDMI_LOG_DEBUG, "0 -> crc 300");
	HDMI_LOG(HDMI_LOG_DEBUG, "1 -> gtimenr");
	HDMI_LOG(HDMI_LOG_DEBUG, "2 -> force pp mode");
	HDMI_LOG(HDMI_LOG_DEBUG, "3 -> no pp mode");
	HDMI_LOG(HDMI_LOG_DEBUG, "4 -> force rxsense");
	HDMI_LOG(HDMI_LOG_DEBUG, "4 -> no force rxsense");
	HDMI_LOG(HDMI_LOG_DEBUG, "**********************************************\r\n");
	return size;
}
static ssize_t mhlrx_cmd_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	/* char name_buf[128]; */
	unsigned int sub_cmd = 0;
	int sscanfrt = 0;

	sscanfrt = sscanf(buf, "%d", &sub_cmd);

	switch (sub_cmd) {
	case 0:
		HDMI_LOG(HDMI_LOG_INFO, "CRC 300");
		HDMICRC(300);
		break;

	case 1:
		HDMI_LOG(HDMI_LOG_INFO, "PRINT GTIME INFO:");
		HDMI_PrintInfo();
		break;

	case 2:
		HDMI_LOG(HDMI_LOG_INFO, "Burn HDCP-KEY to SRAM");
		/* HDMI_HalLoadHdcp2Sram(); */
		HdmiRxLoadHdcpKey();
		break;

	case 3:
		/* HDMI_LOG(HDMI_LOG_DEBUG, "NO Pp mode"); */
		/* _bPPMode = FALSE; */
		break;

	case 4:
		/* HDMI_LOG(HDMI_LOG_DEBUG, "Force Rxsense ON"); */
		/* forceRxsence = TRUE; */
		break;

	case 5:
		/* HDMI_LOG(HDMI_LOG_DEBUG, "No Force Rxsense ON"); */
		/* forceRxsence = FALSE; */
		break;

	case 6:
		break;

	case 7:
		break;

	case 8:
		break;

	default:
		HDMI_LOG(HDMI_LOG_INFO, "unknow cmd");
		break;
	}

	return count;

}
#ifdef SUPPORT_KEY_IN_TRUST_ZONE
static BOOL  isKey14Existed(UINT32 u4Idx)
{
    UINT32 u4Data = 0;
    UINT32 ret = sizeof(UINT32);
    //MTZ_Init();
    if (ret != MetaZone_ReadBinary(u4Idx, (BYTE *)&u4Data,sizeof(UINT32)))
    {
        return FALSE;
    }    
    if (u4Data & COMBINATION_KEY_1_4)
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
	if(u4Size != ENCRYPT_KEY_1_4_LENGTH) {
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
#endif
static DEVICE_ATTR(cmd, S_IWUSR | S_IRUGO, mhlrx_cmd_show, mhlrx_cmd_store);

struct clk *clk_ac8317_mhl = NULL;
struct device *hdmi_dev;
struct pinctrl *pinctrl_hdmi;

//extern int LoadHDCPKeyToSRAM(unsigned char* data);

static int  mhlrx_probe(struct platform_device *pdev)
{
	struct device_node *dn = pdev->dev.of_node;
	long bret = 0;
	MHL_DRV_CONFIG_T config = {{0}, {0} };;
	void __iomem *reg_addr = NULL;
#ifdef SUPPORT_KEY_IN_TRUST_ZONE
	unsigned char bData[ENCRYPT_KEY_1_4_LENGTH];
#endif
	HDMI_LOG(HDMI_LOG_INFO, "*****mhl prob start*****");

	hdmi_dev = &(pdev->dev);
	
	clk_ac8317_mhl = devm_clk_get(&pdev->dev, "mhl-device");

	if (clk_ac8317_mhl == NULL) {
		HDMI_LOG(HDMI_LOG_INFO, "get mhl clk error!");
		return bret;
	}

	/* get mhl irq from device tree*/
	g_VECTOR_HDMI = irq_of_parse_and_map(dn, 0);
	g_VECTOR_CBUSINT = irq_of_parse_and_map(dn, 1);

	if (g_VECTOR_HDMI == IRQ_TYPE_NONE || g_VECTOR_CBUSINT == IRQ_TYPE_NONE) {
		HDMI_LOG(HDMI_LOG_INFO, "get mhl irq error!");
		return bret;
	}

	HDMI_LOG(HDMI_LOG_INFO, "VECTOR_HDMI=%u, VECTOR_CBUSINT=%u\n", g_VECTOR_HDMI, g_VECTOR_CBUSINT);
	reg_addr = of_iomap(dn, 0);

	if (reg_addr == NULL) {
		HDMI_LOG(HDMI_LOG_ERROR, "get rtc reg base addr error!");
		return bret;
	}

	#ifdef CONFIG_ATC_PLATFORM_ac83xx
	g_IO_VBASE_VA = 0xFD000000;
	#elif defined CONFIG_ATC_PLATFORM_ac823x
	g_IO_VBASE_VA = ioremap(IO_PHYS_ADDR, IO_PHYS_SIZE);
	#endif
	HDMI_LOG(HDMI_LOG_INFO, "mhl_mod IO_VBASE_VA is: 0x%lx\r\n", g_IO_VBASE_VA);

	/* printk("reg_addr=0x%x\n", reg_addr); */
	bret = misc_register(&(mhl_dev));

	if (bret) {
		HDMI_LOG(HDMI_LOG_INFO, "mhl register fail\n");
		return bret;
	}

	bret = os_create_file(&(pdev->dev), &dev_attr_cmd);

	if (bret) {
		os_remove_file(&(pdev->dev), &dev_attr_cmd);
		misc_deregister(&(mhl_dev));
		return bret;
	}

#ifdef CONFIG_ATC_PLATFORM_ac83xx
    HDMI_LOG(HDMI_LOG_INFO, "\nMHL Get Chip Feature\n");
	fgMHLSupport = fgGetChipFeature(FEATURE_MHL);
	if(fgMHLSupport) {
		HDMI_LOG(HDMI_LOG_INFO, "Support MHL \r\n");
		featureInit(FEATURE_MHL, 0, 0, 0);
		HDMI_RX_Init();
	}
	else {
		HDMI_LOG(HDMI_LOG_INFO, "No Support MHL \r\n");
		return bret;
	}
#elif defined CONFIG_ATC_PLATFORM_ac823x
	fgMHLSupport = true;
	HDMI_RX_Init();//ac823x not ready for fgGetChipFeature
#endif

	/*if (bret) {
		os_remove_file(&(pdev->dev), &dev_attr_cmd);
		misc_deregister(&(mhl_dev));
		return bret;
	}*/

#ifdef SUPPORT_KEY_IN_TRUST_ZONE
//********* Get encrept key from trust zone srt**********
	if (!isKey14Existed(MZ_HDCPKEY_BIN_IDX_START)) {
		HDMI_LOG(HDMI_LOG_INFO, "HDCP key not existed\r\n");
	} else if (!ReadKeyBlockFromMZ(MZ_HDCPKEY_BIN_IDX_START, bData, ENCRYPT_KEY_1_4_LENGTH)) {
	//*************** Set key to trust zone****************
		HDMI_LOG(HDMI_LOG_INFO, "Get HDCP key failed\r\n");
	} else if(LoadHDCPKeyToSRAM(bData, ENCRYPT_KEY_1_4_LENGTH)) {
		HDMI_LOG(HDMI_LOG_INFO, "Load Key to hw failed\r\n");
	}
//**********************end************************
#endif
	HDMI_RX_Config(config);
	HDMI_LOG(HDMI_LOG_INFO, "*****mhl prob sucess*****\n\n");
	return bret;
}

static int  mhlrx_remove(struct platform_device *pdev)
{
	misc_deregister(&(mhl_dev));
	return 0;
}

static int mhlrx_resume(struct platform_device *device)
{
	return 0;
}

static int mhlrx_suspend(struct platform_device *device, pm_message_t state)
{
	return 0;
}

/* extern int i4HDMIRx_Suspend(void *param); */
static int mhlrx_pm_ops_suspend(struct device *dev)
{
	clk_disable_unprepare(clk_ac8317_mhl);
	return 0;/* i4HDMIRx_Suspend(dev); */
}

static  int mhlrx_pm_ops_resume(struct device *dev)
{
	HDMI_LOG(HDMI_LOG_INFO, "HDMI RESUME#########################################\r\n");

	if (!fgMHLSupport) {
		return 0;
	}
	clk_prepare_enable(clk_ac8317_mhl);
	HDMIInterRxInit();
	HDMI_HwInit();
	HDMI_HalLoadHdcp2Sram(hdcp_key_to_sram);
	HdmiRxLoadEdidTable();
	vMHLCbusHwInit();

	fgResume = TRUE;
	return 0;/* i4HDMIRx_Resume(dev); */
}

struct platform_device p_dev_mhlrx = {
	.name = "mhldrv",
	.id   = -1 ,
};

static const struct dev_pm_ops p_mhl_pm = {
	.suspend    = mhlrx_pm_ops_suspend,
	.resume     = mhlrx_pm_ops_resume,
};

static const struct of_device_id mhl_of_ids[] = {
	{ .compatible = "atc,mhl", },
	{}
};

static struct platform_driver p_drv_mhlrx = {
	.probe      = mhlrx_probe,
	.remove     = mhlrx_remove,
	.resume     = mhlrx_resume,
	.suspend    = mhlrx_suspend,
	.driver     = {
		.name   = "mhldrv",
		.owner  = THIS_MODULE,
		.pm     = &p_mhl_pm,
		.of_match_table = mhl_of_ids,
	},
};

int __init mhl_init(void)
{
	long bret = 0;

	HDMI_LOG(HDMI_LOG_INFO, "*****mhl_init for change*****\n\n");
	MOD_VERSION_INFO("MHL", 1, 1, 1);
	init_MUTEX(&mhlrx_sem);
	/* HDMI_LOG(HDMI_LOG_DEBUG, "register mhl to platform device & driver"); */
	/*bret = os_device_register(&p_dev_mhlrx);
	if(bret) {
	    HDMI_LOG(HDMI_LOG_DEBUG, "register mhl device err : %u", bret);
	    return bret;
	}
	*/
	bret = os_driver_register(&p_drv_mhlrx);

	if (bret) {
		/* HDMI_LOG(HDMI_LOG_DEBUG, "register mhl driver err : %d ", bret); */
		HDMI_LOG(HDMI_LOG_INFO, "*****register mhl driver err*****");
		return bret;
	}

#ifdef HDMI_BURN_IN
	kernel_thread(thr_fun, NULL, CLONE_FS | CLONE_FILES | CLONE_SIGHAND | SIGCHLD);
#endif

	return bret;
}
EXPORT_SYMBOL(mhl_init);

static void __exit mhl_exit(void)
{
	os_device_unregister(&p_dev_mhlrx);
	os_driver_unregister(&p_drv_mhlrx);
}

module_init(mhl_init);
module_exit(mhl_exit);

MODULE_AUTHOR("ATC Inc");
MODULE_DESCRIPTION("MHL Driver of AC8317");
MODULE_LICENSE("GPL");


