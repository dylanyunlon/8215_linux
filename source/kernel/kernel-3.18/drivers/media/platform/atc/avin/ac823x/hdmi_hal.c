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

#include "hdmi_hal.h"
#include "mhl_drv_if.h"
#include "wch_drv.h"
#include "windev.h"
#include "aud_ioctrl.h"
#include "hdmi_dmx.h"
#include "x_audmhl_def.h"
#include "screen_hvdetect.h"
#include <asm/io.h>

#define AUD_DATA_RATE       (16000)
#define AUD_DATA_BLCOKALIGN (4)
#define AVIN_OVERLAY_FLAG_RESZ  (0x0002)
#define AVIN_VDO_SCAN_LINE      (0x1)

static u32 mWidth;
static u32 mHeight;
static WCH_BUF_T mWchBufferInfo;

static BOOL wchStop = TRUE;
/*struct file *hdmihandle = NULL;
struct file *hvsihandle = NULL;*/
static WCH_SRC_APP_ID_E mSrcAppId = SRC_APP_HDMI;
MHL_VIDEO_INFO_T mhdmiVdoInfo;
WCH_BUF_T mWchBufferInfo_orien;
static WCH_CFG_T mWchCtl;
static int mSigState = SIGNAL_IDLE;
int rcp_key;
int device_type;
int signalstatus;
BOOL phoneIsLandScape;
struct mutex audiolock;
struct mutex videolock;
u32 bufferId = 0;
struct task_struct *orientation_detect = NULL;


/****************************************************************************/

int hdmiControl(int CtrlCode)
{
	int ErrorCode = 0;

	switch (CtrlCode) {
	case IOCTL_MHL_INIT: {
		MHL_IOControl(IOCTL_MHL_INIT, NULL, 0, NULL, 0);
	}
	break;

	case IOCTL_MHL_START: {
		ErrorCode = MHL_IOControl(IOCTL_MHL_START, NULL, 0, NULL, 0);
	}
	break;

	case IOCTL_MHL_STOP: {
		ErrorCode = MHL_IOControl(IOCTL_MHL_STOP, NULL, 0, NULL, 0);
	}
	break;

	case IOCTL_MHL_CONFIG: {
		/*ErrorCode = MHL_IOControl(IOCTL_MHL_CONFIG, NULL, 0, NULL, 0);*/
	}
	break;

	case IOCTL_MHL_GET_VIDEO_INFO: {
		ErrorCode = MHL_IOControl(IOCTL_MHL_GET_VIDEO_INFO, NULL, 0, (UCHAR *)&mhdmiVdoInfo, sizeof(MHL_VIDEO_INFO_T));
	}
	break;

	case IOCTL_MHL_SEND_RCPKEY: {
		ErrorCode = MHL_IOControl(IOCTL_MHL_SEND_RCPKEY, (UCHAR *)&rcp_key, sizeof(rcp_key), NULL, 0);
	}
	break;
	case IOCTL_MHL_PROG_HDCP: {
	}
	break;
	case IOCTL_MHL_GET_DEVICE_TYPE: {
		ErrorCode = MHL_IOControl(IOCTL_MHL_GET_DEVICE_TYPE, NULL, 0, (UCHAR *)&device_type, sizeof(device_type));
	}
	break;
	case IOCTL_MHL_IS_SUPPORT:break;
	case IOCTL_MHL_GET_PHONE_ORIENTATION:break;
	case IOCTL_MHL_GET_PHONE_VIDEORECT:break;
	case IOCTL_MHL_GET_SINGAL_STATUS:
		ErrorCode = MHL_IOControl(IOCTL_MHL_GET_SINGAL_STATUS, NULL, 0, (UINT32 *)&signalstatus, sizeof(signalstatus));
	break;
	default:
		pr_info("[AVIN][hdmi_hal]%s invalid control code!\r\n", __func__);
	return false;
	}

	if (0 != ErrorCode){
		pr_info("[AVIN][hdmi_hal]%s fail\r\n", __func__);
	}		
	return (ErrorCode > 0);
}

/*bool hvsiControl(struct file *filp, unsigned long IoControlCOde, void* lpInBuf, unsigned long InBufSize ,
					void* lpOutBuf, unsigned long OutBufSize , unsigned long* lpBytesReturned)
{
	WIN32_IOCTL_DATA pData;
	int ret;

	pData.pInBuf = lpInBuf;
    pData.InSize = InBufSize;
	pData.pOutBuf = lpOutBuf;
    pData.OutSize = OutBufSize;
    pData.pBytesReturned = (unsigned int *)lpBytesReturned;

    ret = HVSI_DRV_IOcontrol((struct file *)filp, IoControlCOde, (unsigned long)&pData);

    if ( ret < 0 )
    {
        return false;
    }
    else
    {
        return true;
    }
}*/

int hdmi_getDeviceType(void)
{
	hdmiControl(IOCTL_MHL_GET_DEVICE_TYPE);
	return device_type;
}
int hdmi_getSignalStatus(void)
{
	hdmiControl(IOCTL_MHL_GET_SINGAL_STATUS);
	if(SIGNAL_CONNECTING == mSigState) {
		signalstatus = SIGNAL_CONNECTING;
	}
	return signalstatus;
}

void sendRcp(int rcpKey) {
	pr_info("[AVIN][hdmi_hal]%s enter!\r\n", __func__);
	//if(NULL != hdmihandle) {
		rcp_key = rcpKey;
		hdmiControl(IOCTL_MHL_SEND_RCPKEY);
	//}
	pr_info("[AVIN][hdmi_hal]%s leave!\r\n", __func__);
}

int getResolution(int freq, bool fgProgressive, int *ptiming)
{
	*ptiming = WCH_MODE_NUM;

	if (mWidth == 640 && mHeight == 480) {
		if (freq == 60) {
			if (fgProgressive) {
				*ptiming = WCH_640_480P_60HZ;
			} else {
				pr_info("[AVIN][hdmi_hal]%s 640_480, freq abnormal\r\n", __func__);
			}
		}
	} else if (mWidth == 720 && mHeight == 576) {

		if (freq == 50) {
			if (fgProgressive) {
				*ptiming = WCH_720_576P_50HZ;
			} else {
				*ptiming = WCH_720_576I_50HZ;
			}
		} else if(freq == 100) {
			if (fgProgressive) {
				*ptiming = WCH_720_576P_100HZ;
			} else {
				*ptiming = WCH_720_576I_100HZ;
			}
		} else if(freq == 200) {
			if (fgProgressive) {
				*ptiming = WCH_720_576P_200HZ;
			} else {
				*ptiming = WCH_720_576I_200HZ;
			}
		}
	} else if (mWidth == 720 && mHeight == 480) {

		if (freq == 60) {
			if (fgProgressive) {
				*ptiming = WCH_720_480P_60HZ;
			} else {
				*ptiming = WCH_720_480I_60HZ;
			}
		} else if(freq == 120) {
			if (fgProgressive) {
				*ptiming = WCH_720_480P_120HZ;
			} else {
				*ptiming = WCH_720_480I_120HZ;
			}
		} else if(freq == 240) {
			if (fgProgressive) {
				*ptiming = WCH_720_480P_240HZ;
			} else {
				*ptiming = WCH_720_480I_240HZ;
			}
		}
	} else if (mWidth == 1440 && mHeight == 480) {

		if (freq == 60) {
			if (fgProgressive) {
				*ptiming = WCH_1440_480P_60HZ;
			} else {
				*ptiming = WCH_720_480I_60HZ;
				/*pr_info("[AVIN][hdmi_hal]%s 1440_480, freq abnormal\r\n", __func__)*/;
			}
		}
	} else if (mWidth == 1440 && mHeight == 576) {

		if (freq == 50) {
			if (fgProgressive) {
				*ptiming = WCH_1440_576P_50HZ;
			} else {
				*ptiming = WCH_720_576I_50HZ;
				/*pr_info("[AVIN][hdmi_hal]%s 1440_480, freq abnormal\r\n", __func__);*/
			}
		} else if (freq == 199) {
			if (fgProgressive) {
				*ptiming = WCH_1440_576P_50HZ;
			} else {
				pr_info("[AVIN][hdmi_hal]%s 1280_720, freq abnormal\r\n", __func__);
			}
		}
	} else if (mWidth == 1280 && mHeight == 720) {

		if (freq == 50) {
			if (fgProgressive) {
				*ptiming = WCH_1280_720P_50HZ;
			} else {
				pr_info("[AVIN][hdmi_hal]%s 1280_720, freq abnormal\r\n", __func__);
			}
		} else if (freq == 60) {
			if (fgProgressive) {
				*ptiming = WCH_1280_720P_60HZ;
			} else {
				pr_info("[AVIN][hdmi_hal]%s 1280_720, freq abnormal\r\n", __func__);
			}
		}  else if (freq == 100) {
			if (fgProgressive) {
				*ptiming = WCH_1280_720P_100HZ;
			} else {
				pr_info("[AVIN][hdmi_hal]%s 1280_720, freq abnormal\r\n", __func__);
			}
		}  else if (freq == 120) {
			if (fgProgressive) {
				*ptiming = WCH_1280_720P_120HZ;
			} else {
				pr_info("[AVIN][hdmi_hal]%s 1280_720, freq abnormal\r\n", __func__);
			}
		}
	} else if (mWidth == 1920 && mHeight == 1080) {

		if (freq == 50) {
			if (fgProgressive) {
				*ptiming = WCH_1920_1080P_50HZ;
			} else {
				*ptiming = WCH_1920_1080I_50HZ;
			}
		} else if (freq == 60 || freq == 59) {
			if (fgProgressive) {
				*ptiming = WCH_1920_1080P_60HZ;
			} else {
				*ptiming = WCH_1920_1080I_60HZ;
			}
		} else if (freq == 100) {
			if (fgProgressive) {
				/**ptiming = WCH_1920_1080P_100HZ;*/
				pr_info("[AVIN][hdmi_hal]%s 1920_1080, freq abnormal\r\n", __func__);
			} else {
				*ptiming = WCH_1920_1080I_100HZ;
			}
		}  else if (freq == 120 || freq == 119) {
			if (fgProgressive) {
				/*ptiming = WCH_1920_1080P_120HZ;*/
				pr_info("[AVIN][hdmi_hal]%s 1920_1080, freq abnormal\r\n", __func__);
			} else {
				*ptiming = WCH_1920_1080I_120HZ;
			}
		}
	}

	return *ptiming;
}
#ifdef ANDROID
void getVideoRect(int cmd, int outRect[]){

	VIDEO_INFO_T mCurrentMhlWchVdoInfo;
	u32 buffer_id = 0;
	RECT_HV Rect;
	RETNO retno = RECT_NULL;

	pr_info("[AVIN][hdmi_hal]%s enter\r\n", __func__);

    /*if (0 != hdmihandle) {
         if (!hvsiControl((struct file*)hvsihandle, cmd, NULL, 0, outRect, sizeof(int), NULL)) {
             pr_info("[AVIN][hdmi_hal]%s Failed\r\n", __func__);
        }*/
		if ((DEVICE_MHL == device_type) && (SIGNAL_READY == mSigState)
			 /*&& (0 != hvsihandle)*/) {
			mCurrentMhlWchVdoInfo.u4YVaAddr = (unsigned long)phys_to_virt((unsigned long)mWchBufferInfo.tWchBuf.u4YBuf[buffer_id]);//mWchBufferInfo_orien.u4YBuf[buffer_id];
			mCurrentMhlWchVdoInfo.u4CVaAddr = (unsigned long)phys_to_virt((unsigned long)mWchBufferInfo.tWchBuf.u4CBuf[buffer_id]);//mWchBufferInfo_orien.u4CBuf[buffer_id];
			mCurrentMhlWchVdoInfo.u4Height = mHeight;
			mCurrentMhlWchVdoInfo.u4Width = mWidth;
			mCurrentMhlWchVdoInfo.u4Mode = MODE_LINE;
			pr_info("[AVIN][hdmi_hal]%s GR_GetActiveRect before\r\n", __func__);
			retno = GR_GetActiveRect(mCurrentMhlWchVdoInfo, &Rect);
			pr_info("[AVIN][hdmi_hal]%s GR_GetActiveRect after\r\n", __func__);
			if ((BUFFER_ROTATE == retno) || (BUFFER_UNROTATE == retno)) {
				switch (cmd){
					case V4L2_CID_GET_HDMI_MHL_VDORECT0:
						*outRect = Rect.top;
						break;
					case V4L2_CID_GET_HDMI_MHL_VDORECT1:
						*outRect = Rect.bottom;
						break;
					case V4L2_CID_GET_HDMI_MHL_VDORECT2:
						*outRect = Rect.left;
						break;
					case V4L2_CID_GET_HDMI_MHL_VDORECT3:
						*outRect = Rect.right;
						break;
					default:
						break;
				}
			} else {
				pr_info("[AVIN][hdmi_hal]%s fail!\r\n", __func__);
			}

			if (mCurrentMhlWchVdoInfo.u4YVaAddr == NULL) {
				printk("HDMI no need to unmap the address u4YVirtAddr! \r\n");
			} else {
				//iounmap((void __iomem *)mCurrentMhlWchVdoInfo.u4YVaAddr);
			}
	
			if (mCurrentMhlWchVdoInfo.u4CVaAddr == NULL) {
				printk("HDMI no need to unmap the address u4CVirtAddr! \r\n");
			} else {
				//iounmap((void __iomem *)mCurrentMhlWchVdoInfo.u4CVaAddr);
			}
		}
    	
    /*} else {
            pr_info("[AVIN][hdmi_hal]%s hdmihandle is NULL\r\n", __func__);
    }*/

	pr_info("[AVIN][hdmi_hal]%s leave\r\n", __func__);
}
#endif

#ifdef LINUX
void getVideoRect(int cmd, int outRect[]){

	VIDEO_INFO_T mCurrentMhlWchVdoInfo;
	u32 buffer_id = 0;
	RECT_HV Rect;
	RETNO retno = RECT_NULL;

	pr_info("[AVIN][hdmi_hal]%s enter\r\n", __func__);

    /*if (0 != hdmihandle) {
         if (!hvsiControl((struct file*)hvsihandle, cmd, NULL, 0, outRect, sizeof(int), NULL)) {
             pr_info("[AVIN][hdmi_hal]%s getVideoRect Failed", __func__);
        }*/
		if ((DEVICE_MHL == device_type) && (SIGNAL_READY == mSigState)
			 /*&& (0 != hvsihandle)*/) {
			mCurrentMhlWchVdoInfo.u4YVaAddr = mWchBufferInfo_orien.tWchBuf.u4YBuf[buffer_id];
			mCurrentMhlWchVdoInfo.u4CVaAddr = mWchBufferInfo_orien.tWchBuf.u4CBuf[buffer_id];
			mCurrentMhlWchVdoInfo.u4Height  = mHeight;
			mCurrentMhlWchVdoInfo.u4Width	= mWidth;
			mCurrentMhlWchVdoInfo.u4Mode = MODE_LINE;
			pr_info("[AVIN][hdmi_hal]%s GR_GetActiveRect before\r\n", __func__);
			//retno = GR_GetActiveRect(mCurrentMhlWchVdoInfo, &Rect);
			pr_info("[AVIN][hdmi_hal]%s GR_GetActiveRect after\r\n", __func__);
			if ((BUFFER_ROTATE == retno) || (BUFFER_UNROTATE == retno)) {
				switch (cmd){
					case V4L2_CID_GET_HDMI_MHL_VDORECT0:
						*outRect = Rect.top;
						break;
					case V4L2_CID_GET_HDMI_MHL_VDORECT1:
						*outRect = Rect.bottom;
						break;
					case V4L2_CID_GET_HDMI_MHL_VDORECT2:
						*outRect = Rect.left;
						break;
					case V4L2_CID_GET_HDMI_MHL_VDORECT3:
						*outRect = Rect.right;
						break;
					default:
						break;
				}
			} else {
				pr_info("[AVIN][hdmi_hal]%s fail!\r\n", __func__);
			}

			/*if (mCurrentMhlWchVdoInfo.u4YVaAddr == NULL) {
				printk("HDMI no need to unmap the address u4YVirtAddr! \r\n");
			} else {
				iounmap((void __iomem *)mCurrentMhlWchVdoInfo.u4YVaAddr);
			}
	
			if (mCurrentMhlWchVdoInfo.u4CVaAddr == NULL) {
				printk("HDMI no need to unmap the address u4CVirtAddr! \r\n");
			} else {
				iounmap((void __iomem *)mCurrentMhlWchVdoInfo.u4CVaAddr);		
			}*/
		}
    	
    /*} else {
            pr_info("[AVIN][hdmi_hal]%s hdmihandle is NULL\r\n", __func__);
    }*/

	pr_info("[AVIN][hdmi_hal]%s leave\r\n", __func__);
}
#endif

static long int buffer_count = 0;
static void wch_buffer_get(u32 *bufindex)
{
	bool need_hide = false;
	struct capture_priv data;
	bufferId = *bufindex;
	if ((mSigState == SIGNAL_CHANGE_START) || (mSigState == SIGNAL_LOST)) {
		pr_info("[AVIN][hdmi_hal]%s need hide hdmi\r\n", __func__);
		need_hide = true;
	}

	memset(&data, 0, sizeof(struct capture_priv));
	data.ycaddr.y = mWchBufferInfo.tWchBuf.u4YBuf[bufferId];
	data.ycaddr.c = mWchBufferInfo.tWchBuf.u4CBuf[bufferId];
	data.buf_height = mHeight;
	data.buf_width = mWidth;
	data.signal_status = SIGNAL_NONE;
	data.need_hide = false;
	avin_buffer_complete(AVIN_TYPE_HDMI, &data);

	buffer_count++;
	if(2 == buffer_count) {
		data.signal_status = SIGNAL_READY;
		data.need_hide = false;
		if(0x1 == device_type) {
			data.hdmi_dev_type= DEVICE_TYPE_HDMI;
		}else if(0x2 == device_type) {
			data.hdmi_dev_type = DEVICE_TYPE_MHL;
		}else {
			data.hdmi_dev_type = DEVICE_TYPE_NULL;
		}
		avin_buffer_complete(AVIN_TYPE_HDMI, &data);
	}

}
#ifdef ANDROID
void _hdmi_getOrientation(u32 buffer_id) {
	VIDEO_INFO_T mCurrentMhlWchVdoInfo;
	RECT_HV Rect;
	RETNO retno = RECT_NULL;

	if ((DEVICE_MHL == device_type) && (SIGNAL_READY == mSigState)) {
		mCurrentMhlWchVdoInfo.u4YVaAddr = (unsigned long)phys_to_virt((unsigned long)mWchBufferInfo.tWchBuf.u4YBuf[buffer_id]);//mWchBufferInfo_orien.u4YBuf[buffer_id];
		mCurrentMhlWchVdoInfo.u4CVaAddr = (unsigned long)phys_to_virt((unsigned long)mWchBufferInfo.tWchBuf.u4CBuf[buffer_id]);//mWchBufferInfo_orien.u4CBuf[buffer_id];
		mCurrentMhlWchVdoInfo.u4Height = mHeight;
		mCurrentMhlWchVdoInfo.u4Width  = mWidth;
		mCurrentMhlWchVdoInfo.u4Mode = MODE_LINE;

		retno = GR_GetActiveRect(mCurrentMhlWchVdoInfo, &Rect);

		if (BUFFER_ROTATE == retno) {
			phoneIsLandScape = true;
		} else if (BUFFER_UNROTATE == retno){
			phoneIsLandScape = false;
		} else {
			pr_debug("[AVIN][hdmi_hal]%s fail!\r\n", __func__);
		}

		if (mCurrentMhlWchVdoInfo.u4YVaAddr == NULL) {
			printk("HDMI no need to unmap the address u4YVirtAddr! \r\n");
		} else {
			//iounmap((void __iomem *)mCurrentMhlWchVdoInfo.u4YVaAddr);
		}
	
		if (mCurrentMhlWchVdoInfo.u4CVaAddr == NULL) {
			printk("HDMI no need to unmap the address u4CVirtAddr! \r\n");
		} else {
			//iounmap((void __iomem *)mCurrentMhlWchVdoInfo.u4CVaAddr);
		}
	}

}
#endif

#ifdef LINUX
void _hdmi_getOrientation(u32 buffer_id) {
	VIDEO_INFO_T mCurrentMhlWchVdoInfo;
	RECT_HV Rect;
	RETNO retno = RECT_NULL;

	if ((DEVICE_MHL == device_type) && (SIGNAL_READY == mSigState)) {
		mCurrentMhlWchVdoInfo.u4YVaAddr = mWchBufferInfo_orien.tWchBuf.u4YBuf[buffer_id];
		mCurrentMhlWchVdoInfo.u4CVaAddr = mWchBufferInfo_orien.tWchBuf.u4CBuf[buffer_id];
		mCurrentMhlWchVdoInfo.u4Height = mHeight;
		mCurrentMhlWchVdoInfo.u4Width  = mWidth;
		mCurrentMhlWchVdoInfo.u4Mode = MODE_LINE;
		//retno = GR_GetActiveRect(mCurrentMhlWchVdoInfo, &Rect);
		if (BUFFER_ROTATE == retno) {
			phoneIsLandScape = true;
		} else if (BUFFER_UNROTATE == retno){
			phoneIsLandScape = false;
		} else {
			pr_debug("[AVIN][hdmi_hal]%s fail!\r\n", __func__);
		}

		/*if (mCurrentMhlWchVdoInfo.u4YVaAddr == NULL) {
			printk("[AVIN][hdmi_hal]%s no need to unmap the address u4YVirtAddr!\r\n", __func__);
		} else {
			iounmap((void __iomem *)mCurrentMhlWchVdoInfo.u4YVaAddr);
		}
	
		if (mCurrentMhlWchVdoInfo.u4CVaAddr == NULL) {
			printk("[AVIN][hdmi_hal]%s no need to unmap the address u4CVirtAddr!\r\n", __func__);
		} else {
			iounmap((void __iomem *)mCurrentMhlWchVdoInfo.u4CVaAddr);		
		}*/
	}

}
#endif
BOOL hdmi_getOrientation(void)
{
	//pr_info("[AVIN][hdmi_hal]%s phoneIsLandScape = %d\r\n", __func__, phoneIsLandScape);
	return phoneIsLandScape;
}

void orientation_detect_thread(void *data){
	while(1){
			if (kthread_should_stop()) {
			pr_info("[AVIN][hdmi_hal]%s kthread_should_stop!\r\n", __func__);
			break;
		}
		msleep(20);
		_hdmi_getOrientation(bufferId);
	}
}

static void onVdoNoSignal(void)
{
	u32 buffer_id = 0;
	struct capture_priv data;

	pr_info("[AVIN][hdmi_hal]%s enter\r\n", __func__);

	if (wchStop == FALSE) {
		if (StopWch(WCH_7, mSrcAppId)) {
			pr_info("[AVIN][hdmi_hal]%s wch stop fail!\r\n", __func__);
			return;
		}
		wchStop = TRUE;
		pr_info("[AVIN][hdmi_hal]%s stop wch!\r\n", __func__);
	}

	mSigState = SIGNAL_LOST;

	memset(&data, 0, sizeof(struct capture_priv));
	data.ycaddr.y = mWchBufferInfo.tWchBuf.u4YBuf[buffer_id];
	data.ycaddr.c = mWchBufferInfo.tWchBuf.u4CBuf[buffer_id];
	data.buf_height = mHeight;
	data.buf_width = mWidth;
	data.signal_status = SIGNAL_LOST;
	data.need_hide = false;
	avin_buffer_complete(AVIN_TYPE_HDMI, &data);

	if(NULL != orientation_detect){
		kthread_stop(orientation_detect);
		orientation_detect = NULL;
		pr_info("[AVIN][hdmi_hal]%s kthread_stop(orientation_detect)\r\n", __func__);
	}

	pr_info("[AVIN][hdmi_hal]%s leave\r\n", __func__);
}

static void onVdoSignalChange(void)
{
	u32 buffer_id = 0;
	struct capture_priv data;

	pr_info("[AVIN][hdmi_hal]%s enter\r\n", __func__);

	if (wchStop == FALSE) {
		if (StopWch(WCH_7, mSrcAppId)) {
			pr_info("[AVIN][hdmi_hal]%s wch stop fail!\r\n", __func__);
			return;
		}
		wchStop = TRUE;
		pr_info("[AVIN][hdmi_hal]%s stop wch!\r\n", __func__);
	}

	mSigState = SIGNAL_CHANGE_START;
	memset(&data, 0, sizeof(struct capture_priv));
	data.ycaddr.y = mWchBufferInfo.tWchBuf.u4YBuf[buffer_id];
	data.ycaddr.c = mWchBufferInfo.tWchBuf.u4CBuf[buffer_id];
	data.buf_height = mHeight;
	data.buf_width = mWidth;
	data.signal_status = SIGNAL_CHANGE_START;
	data.need_hide = false;
	avin_buffer_complete(AVIN_TYPE_HDMI, &data);

	pr_info("[AVIN][hdmi_hal]%s leave\r\n", __func__);
}

static void onVdoSignal(void)
{
	int timing;
	u32 buffer_id = 0;
	struct capture_priv data;

	pr_info("[AVIN][hdmi_hal]%s enter\r\n", __func__);

	if (hdmiControl(IOCTL_MHL_GET_VIDEO_INFO)) {
		pr_info("[AVIN][hdmi_hal]%s IOCTL_MHL_GET_VIDEO_INFO fail!\r\n", __func__);
		return;
	}
	if (DEVICE_MHL == hdmi_getDeviceType()) {/*for phoneorientation detecting in wch_buffer_get*/
		pr_info("[AVIN][hdmi_hal]%s DeviceType is MHL\r\n", __func__);
	}
	memset(&mWchCtl, 0, sizeof(WCH_CFG_T));
	mWchCtl.eSrcId = SRC_APP_HDMI;
	mWchCtl.fgHSyncPolarity = 1;/*mhdmiVdoInfo.bHPol;*/
	mWchCtl.fgVSyncPolarity = 0;/*mhdmiVdoInfo.bVPol;*/

	mWchCtl.eInputSrc = DATA_SRC_HDMI;
	mWchCtl.eInputFmt = DATA_FMT_YUV422;
	mWchCtl.u4SrcWidth = mhdmiVdoInfo.u4Width;
	mWchCtl.u4SrcHeight = mhdmiVdoInfo.u4Height;

	mWidth = mhdmiVdoInfo.u4Width;
	mHeight = mhdmiVdoInfo.u4Height;

	pr_info("[AVIN][hdmi_hal]%s width = %d, height = %d\r\n", __func__, (int)(mhdmiVdoInfo.u4Width), (int)(mhdmiVdoInfo.u4Height));

	mWchCtl.u4DstWidth = mhdmiVdoInfo.u4Width;
	mWchCtl.u4DstHeight = mhdmiVdoInfo.u4Height;
	mWchCtl.eOutputFmt = DATA_FMT_YUV420;
	mWchCtl.u1WchId = WCH_7;
	mWchCtl.u4ScanLineMode = 1;

	mWchCtl.fgProgressive = !mhdmiVdoInfo.bInterlaced;
	pr_info("[hdmi_hal]%s bInterlaced = %d\r\n", __func__, (int)mhdmiVdoInfo.bInterlaced);

	mWchCtl.u1YSel = 0;/*may change cause of different hw*/
	mWchCtl.u1USel = 2;/*above*/
	mWchCtl.u1VSel = 1;/*above*/
	//mWchCtl.tWchCfg.u1UVSwap = mhdmiVdoInfo.bUVSwap;

	getResolution(mhdmiVdoInfo.u4VFreq, !mhdmiVdoInfo.bInterlaced, &timing);
	pr_info("[AVIN][hdmi_hal]%s timing = %d\r\n", __func__, timing);

	mWchCtl.eTiming = (WCH_TIMING_E)timing;
	mWidth = mhdmiVdoInfo.u4Width;
	mHeight = mhdmiVdoInfo.u4Height;
	mWchCtl.GetWchBufIndx = wch_buffer_get;/*wch_buffer_done;*/

	if (wchStop == FALSE) {
		if (StopWch(WCH_7, mSrcAppId)) {
			pr_info("[AVIN][hdmi_hal]%s wch stop fail!\r\n", __func__);
			return;
		}
		wchStop = TRUE;
		pr_info("[AVIN][hdmi_hal]%s stop wch!\r\n", __func__);
	}

	if (ConfigWch(&mWchCtl)) {
		pr_info("[AVIN][hdmi_hal]%s config wch fail!\r\n", __func__);
		return;
	}
	pr_info("[hdmi_hal]%s config success!\r\n", __func__);

	if (StartWch(WCH_7, mSrcAppId)) {
		pr_info("[AVIN][hdmi_hal]%s wch start fail!\r\n", __func__);
		return;
	}
	wchStop = FALSE;
	pr_info("[AVIN][hdmi_hal]%s wch start success!\r\n", __func__);

	mSigState = SIGNAL_READY;
	memset(&data, 0, sizeof(struct capture_priv));
	data.ycaddr.y = mWchBufferInfo.tWchBuf.u4YBuf[buffer_id];
	data.ycaddr.c = mWchBufferInfo.tWchBuf.u4CBuf[buffer_id];
	data.buf_height = mHeight;
	data.buf_width = mWidth;
	//data.signal_status = SIGNAL_READY;
	data.need_hide = false;
	buffer_count = 0;
	//avin_buffer_complete(AVIN_TYPE_HDMI, &data);
	if(0x1 == device_type) {
		pr_info("[AVIN][hdmi_hal]%s devicetype hdmi!\r\n", __func__);
		//data.signal_status = DEVICE_TYPE_HDMI;
		//avin_buffer_complete(AVIN_TYPE_HDMI, &data);
	}else if(0x2 == device_type) {
		pr_info("[AVIN][hdmi_hal]%s devicetype mhl!\r\n", __func__);
		//data.signal_status = DEVICE_TYPE_MHL;
		//avin_buffer_complete(AVIN_TYPE_HDMI, &data);

		orientation_detect =
		(struct task_struct *)kthread_create(orientation_detect_thread, NULL, "orientation_detect_thread");

		if (IS_ERR(orientation_detect)) {
			pr_info("[AVIN][hdmi_hal]%s create orientation_detect_thread fail\r\n", __func__);
			PTR_ERR(orientation_detect);
			orientation_detect = NULL;
		} else {
			pr_info("[AVIN][hdmi_hal]%s create orientation_detect_thread success\r\n", __func__);
			wake_up_process(orientation_detect);
		}

	}

	pr_info("[AVIN][hdmi_hal]%s leave\r\n", __func__);
}

void atc_hdmi_signal_status(void *signal_status)
{
	HDMI_SIG_INFORMATION *signal_info = NULL;
	u32 buffer_id = 0;
	struct capture_priv data;

	mutex_lock(&videolock);
	pr_info("[AVIN][hdmi_hal]%s enter\r\n", __func__);

	signal_info = (HDMI_SIG_INFORMATION *)signal_status;
	if (NULL == signal_info) {
		pr_info("[AVIN][hdmi_hal]%s leave failed, the input parameter is null\r\n", __func__);
		mutex_unlock(&videolock);
		return;
	}
	if (mSigState == SIGNAL_IDLE) {
		pr_info("[AVIN][hdmi_hal]%s mSigState is SIGNAL_IDLE, return\r\n", __func__);
		mutex_unlock(&videolock);
		return;
	}
	switch (signal_info->signal_state) {
	case HDMI_SIG_READY:
		onVdoSignal();
		break;

	case HDMI_SIG_LOST:
		onVdoNoSignal();
		break;

	case HDMI_SIG_CHANGE_START:
		onVdoSignalChange();
		break;

	case HDMI_SIG_CHANGE_DONE:
		/*vdoSignalChangeDone(signal_info->arg);*/
		break;
	case HDMI_SIG_CONNECTING:
		pr_info("[AVIN][hdmi_hal]%s HDMI_SIG_CONNECTING\r\n", __func__);
		memset(&data, 0, sizeof(struct capture_priv));
		data.ycaddr.y = mWchBufferInfo.tWchBuf.u4YBuf[buffer_id];
		data.ycaddr.c = mWchBufferInfo.tWchBuf.u4CBuf[buffer_id];
		data.buf_height = mHeight;
		data.buf_width = mWidth;
		data.signal_status = SIGNAL_CONNECTING;
		data.need_hide = false;
		avin_buffer_complete(AVIN_TYPE_HDMI, &data);
		hdmi_getDeviceType();
		mSigState = SIGNAL_CONNECTING;
		break;
	default:
		break;
	}

	mutex_unlock(&videolock);
	pr_info("[AVIN][hdmi_hal]%s leave\r\n", __func__);
}

void atc_hdmi_isr(void *arg)
{
	pr_info("[AVIN][hdmi_hal]%s enter arg = %p\r\n", __func__, *((int *)arg));
	atc_hdmi_signal_status(arg);
	pr_info("[AVIN][hdmi_hal]%s leave\r\n", __func__);
}
EXPORT_SYMBOL(atc_hdmi_isr);

int hdmi_init_video(int index)
{
	int ret = 0;
	int mWchId;
	WCH_BUFF_INFO_T bufferInfo;
	mutex_init(&videolock);

	pr_info("[AVIN][hdmi_hal]%s enter\r\n", __func__);

	/*pr_info("[AVIN][hdmi_hal]%s mhl_init start\r\n", __func__);
	mhl_init();
	mutex_lock(&videolock);
	pr_info("[AVIN][hdmi_hal]%s mhl_open start\r\n", __func__);
	hdmihandle = filp_open("/dev/mhldrv", O_RDWR, 0);
	if(IS_ERR(hdmihandle))
	{
	     pr_err("mhl_open fail\r\n");
	     hdmihandle = NULL;
	     return -1;
	}*/

	hdmiControl(IOCTL_MHL_START);
	atc_hdmi_register_isr(atc_hdmi_isr, NULL);
#if 0
	hvsihandle =  filp_open("/dev/hvsi_drv", O_RDWR, 0);
	if(IS_ERR(hvsihandle))
	{
	     pr_err("hvsi_drv open  fail\r\n");
	     hvsihandle = NULL;
	}
#endif
	/*when open wch ,wch  resource will be reserved,
	so when cocurrence happen, we release resource, after we get resource again*/
	if (WCH_SUCCESS != OpenWch(WCH_7, mSrcAppId)) {

		pr_info("[AVIN][hdmi_hal]%s wch open fail!\r\n", __func__);
		/*mutex_unlock(&videolock);*/
		return -EINVAL;
	}
	pr_info("[hdmi_hal]%s wch open success!\r\n", __func__);

	memset(&bufferInfo, 0, sizeof(WCH_BUFF_INFO_T));

	memset(&mWchBufferInfo, 0, sizeof(WCH_BUF_T));
	mWchBufferInfo.u1WchId= WCH_7;
	if (WchGetBufferAddress(&mWchBufferInfo)) {
		pr_info("[AVIN][hdmi_hal]%sIOCTL_WCH_GET_ADDR fail!\r\n", __func__);
		/*mutex_unlock(&videolock);*/
		return -EINVAL;
	}
#ifdef LINUX
	if (WchIoControl(1, IOCTL_WCH_GET_VIRTUAL_ADDR, (UCHAR *)&mSrcAppId, sizeof(WCH_SRC_APP_ID_E),
			(UCHAR *)&mWchBufferInfo_orien, sizeof(WCH_BUFF_INFO_T), NULL)) {
		pr_info("[AVIN][hdmi_hal]%s IOCTL_WCH_GET_VIRTUAL_ADDR fail!\r\n", __func__);
		return -EINVAL;
	}
#endif
	mSigState = SIGNAL_NONE;
	/*mutex_unlock(&videolock);*/

	pr_info("[AVIN][hdmi_hal]%s leave\r\n", __func__);
	return ret;
}

int hdmi_start_video(int index)
{
	u32 buffer_id = 0;
	/*struct capture_priv data;*/

	mutex_lock(&videolock);
	pr_info("[AVIN][hdmi_hal]%s enter\r\n", __func__);

	if (mSigState == SIGNAL_READY || mSigState == SIGNAL_LOST) {
		pr_info("[AVIN][hdmi_hal]%s already start\r\n", __func__);
		mutex_unlock(&videolock);
		return 1;
	}

	hdmiControl(IOCTL_MHL_INIT);
	hdmiControl(IOCTL_MHL_CONFIG);
	hdmiControl(IOCTL_MHL_START);
	mSigState = SIGNAL_LOST;/*when start, we set sigstate lost*/
	/*data.ycaddr.y = mWchBufferInfo.u4YBuf[buffer_id];
	data.ycaddr.c = mWchBufferInfo.u4CBuf[buffer_id];
	data.buf_height = mHeight;
	data.buf_width = mWidth;
	data.signal_status = SIGNAL_LOST;
	data.need_hide = false;
	avin_buffer_complete(AVIN_TYPE_HDMI, &data);*/

	mutex_unlock(&videolock);
	pr_info("[AVIN][hdmi_hal]%s leave\r\n", __func__);
	return 1;
}

int hdmi_stop_video(int index)
{
	mutex_lock(&videolock);
	pr_info("[AVIN][hdmi_hal]%s enter mSigState = %d\r\n", __func__, mSigState);

	if (mSigState == SIGNAL_NONE) {
		pr_info("[AVIN][hdmi_hal]%s already stop\r\n", __func__);
		mutex_unlock(&videolock);
		return 1;
	}

	if(NULL != orientation_detect){
		kthread_stop(orientation_detect);
		orientation_detect = NULL;
		pr_info("[AVIN][hdmi_hal]%s kthread_stop(orientation_detect)\r\n", __func__);
	}

	atc_hdmi_unregister_isr(atc_hdmi_isr, NULL);
	if (wchStop == FALSE) {
		if (StopWch(WCH_7, mSrcAppId)) {
			pr_info("[AVIN][hdmi_hal]%s wch stop fail!\r\n", __func__);
			mutex_unlock(&videolock);
			return -1;
		}
		wchStop = TRUE;
		pr_info("[AVIN][hdmi_hal]%s wch stop success!\r\n", __func__);
	}

	if (CloseWch(WCH_7, mSrcAppId)) {
		pr_info("[AVIN][hdmi_hal]%s wch close fail!\r\n", __func__);
		mutex_unlock(&videolock);
		return -1;
	}
	pr_info("[AVIN][hdmi_hal]%s hdmi_stop_video, wch close success!\r\n", __func__);

	hdmiControl(IOCTL_MHL_STOP);
	pr_info("[AVIN][hdmi_hal]%s hdmi driver stop\r\n", __func__);
#if 0

	if (NULL != hdmihandle)
	{
		filp_close((struct file*)hdmihandle, NULL);
		hdmihandle = NULL;
	}

	if (NULL != hvsihandle)
	{
		filp_close((struct file*)hvsihandle, NULL);
		hvsihandle = NULL;
	}
#endif
	mSigState = SIGNAL_IDLE;

	mutex_unlock(&videolock);
	pr_info("[AVIN][hdmi_hal]%s leave\r\n", __func__);
	return mSigState;
}

/*static __u32 HdmiAudioHandle;*/
static struct file *AudioFilp;
HAVDECINST          mAudInst;
HDMI_AUD_STATE_T    mAudState = HDMI_AUD_IDLE;
HDMI_AUD_E_STATE_T           mAState;

bool GetSampleRateByIndex(UINT8 uIndex, UINT32 *u4SampleRate)
{
	bool bRet = true;

	switch (uIndex) {
	case SPDIFIN_32K:
		*u4SampleRate = HDMI_SAMPLE_RATE_32K;
		break;

	case SPDIFIN_44K:
		*u4SampleRate = HDMI_SAMPLE_RATE_44K;
		break;

	case SPDIFIN_48K:
		*u4SampleRate = HDMI_SAMPLE_RATE_48K;
		break;

	case SPDIFIN_64K:
		*u4SampleRate = HDMI_SAMPLE_RATE_64K;
		break;

	case SPDIFIN_88K:
		*u4SampleRate = HDMI_SAMPLE_RATE_88K;
		break;

	case SPDIFIN_96K:
		*u4SampleRate = HDMI_SAMPLE_RATE_96K;
		break;

	case SPDIFIN_128K:
		*u4SampleRate = HDMI_SAMPLE_RATE_128K;
		break;

	case SPDIFIN_176K:
		*u4SampleRate = HDMI_SAMPLE_RATE_176K;
		break;

	case SPDIFIN_192K:
		*u4SampleRate = HDMI_SAMPLE_RATE_192K;
		break;

	case SPDIFIN_OUT_RANGE:
	default:
		bRet = FALSE;
		break;
	}

	return bRet;
}

bool GetSampleBitDepth(bool fgMaxBit, UINT8 uIndex, UINT8 *uBitDepth)
{
	bool bRet = TRUE;

	if (fgMaxBit) {
		switch (uIndex) {
		case IEC_BIT_DEPTH_INDEX_1:
			*uBitDepth = IEC_BIT_DEPTH_23;
			break;

		case IEC_BIT_DEPTH_INDEX_2:
			*uBitDepth = IEC_BIT_DEPTH_22;
			break;

		case IEC_BIT_DEPTH_INDEX_3:
			*uBitDepth = IEC_BIT_DEPTH_21;
			break;

		case IEC_BIT_DEPTH_INDEX_4:
			*uBitDepth = IEC_BIT_DEPTH_20;
			break;

		case IEC_BIT_DEPTH_INDEX_5:
			*uBitDepth = IEC_BIT_DEPTH_24;
			break;

		default:
			bRet = FALSE;
			break;
		}
	} else {
		switch (uIndex) {
		case IEC_BIT_DEPTH_INDEX_1:
			*uBitDepth = IEC_BIT_DEPTH_19;
			break;

		case IEC_BIT_DEPTH_INDEX_2:
			*uBitDepth = IEC_BIT_DEPTH_18;
			break;

		case IEC_BIT_DEPTH_INDEX_3:
			*uBitDepth = IEC_BIT_DEPTH_17;
			break;

		case IEC_BIT_DEPTH_INDEX_4:
			*uBitDepth = IEC_BIT_DEPTH_16;
			break;

		case IEC_BIT_DEPTH_INDEX_5:
			*uBitDepth = IEC_BIT_DEPTH_20;
			break;

		default:
			bRet = FALSE;
			break;
		}
	}

	return bRet;
}

bool GetSpeakerLayOut(UINT8 uSpeakerAlloc, AUD_DEC_AUD_TYPE_T *eAudDecType)
{
	bool bRet = TRUE;

	switch (uSpeakerAlloc) {
	case CA_FL_FR:
		*eAudDecType = AUD_DEC_TYPE_STEREO;          /*2.0*/
		break;

	case CA_LFE_FL_FR:
		*eAudDecType = AUD_DEC_TYPE_SURROUND_2CH;    /*2.1*/
		break;

	case CA_FC_FL_FR:
		*eAudDecType = AUD_DEC_TYPE_3_0;             /*3.0*/
		break;

	case CA_FC_LFE_FL_FR:
		*eAudDecType = AUD_DEC_TYPE_SURROUND;        /*3.1*/
		break;

	case CA_RR_RL_FL_FR:
		*eAudDecType = AUD_DEC_TYPE_4_0;             /*4.0*/
		break;

	case CA_RR_RL_LFE_FL_FR:
		*eAudDecType = AUD_DEC_TYPE_4_1;             /*4.1*/
		break;

	case CA_RR_RL_FC_FL_FR:
		*eAudDecType = AUD_DEC_TYPE_5_0;             /*5.0*/
		break;

	case CA_RR_RL_FC_LFE_FL_FR:
		*eAudDecType = AUD_DEC_TYPE_5_1;             /*5.1*/
		break;

	default:
		*eAudDecType = AUD_DEC_TYPE_STEREO;
		break;
	}

	return bRet;
}

bool openAOut(HDMI_AUD_E_DEST_TYPE_T eDestType)
{
	pr_info("[AVIN][hdmi_hal]%s enter eDestType=%d\r\n", __func__, eDestType);

	if (NULL == mAudInst) {
		pr_info("[AVIN][hdmi_hal]%s mAudInst is null\r\n", __func__);
		return false;
	}

	switch (eDestType) {
	case HDMI_AUD_DEST_TYPE_FRONT:
		ADec_OpenAOut(mAudInst, AUD_OUTPUT_FRONT);
		break;

	case HDMI_AUD_DEST_TYPE_REAR:
		ADec_OpenAOut(mAudInst, AUD_OUTPUT_REAR);
		break;

	case HDMI_AUD_DEST_TYPE_FRONT_REAR:
		ADec_OpenAOut(mAudInst, AUD_OUTPUT_FRONT_REAR);
		break;

	default:
		return false;
	}

	pr_info("[AVIN][hdmi_hal]%s leave!\r\n", __func__);
	return true;
}

bool closeAOut(HDMI_AUD_E_DEST_TYPE_T eDestType)
{
	pr_info("[AVIN][hdmi_hal]%s enter!\r\n", __func__);
	pr_info("[AVIN][hdmi_hal]%s eDestType=%d!\r\n", __func__, eDestType);

	if (NULL == mAudInst) {
		pr_info("[AVIN][hdmi_hal]%s mAudInst is null\r\n", __func__);
		return false;
	}

	switch (eDestType) {
	case HDMI_AUD_DEST_TYPE_FRONT:
		ADec_CloseAOut(mAudInst, AUD_OUTPUT_FRONT);
		break;

	case HDMI_AUD_DEST_TYPE_REAR:
		ADec_CloseAOut(mAudInst, AUD_OUTPUT_REAR);
		break;

	case HDMI_AUD_DEST_TYPE_FRONT_REAR:
		ADec_CloseAOut(mAudInst, AUD_OUTPUT_FRONT_REAR);
		break;

	default:
		return false;
	}

	pr_info("[AVIN][hdmi_hal]%s leave!\r\n", __func__);
	return true;
}

bool startAdec(void)
{

	AUDIN_INFO_T rAudinInfo;
	AVCODECID_T eAudCodec;

	pr_info("[AVIN][hdmi_hal]%s enter\r\n", __func__);
	memset(&rAudinInfo, 0, sizeof(AUDIN_INFO_T));

	if (NULL == AudioFilp)
	{
		pr_info("[AVIN][hdmi_hal]%s error, AudioFilp is NULL\r\n", __func__);
	}

	if (!AudioIoCtl(AudioFilp, IOCTL_AUDMHL_GET_INFO, NULL, 0, &rAudinInfo, sizeof(rAudinInfo), NULL)) {
		pr_info("[AVIN][hdmi_hal]%s IOCTL_AUDMHL_GET_INFO fail\r\n", __func__);
		return false;
	}

	pr_info("[AVIN][hdmi_hal]%s IOCTL_AUDMHL_GET_INFO: success\r\n", __func__);
	/* ADec*/
	pr_info("[AVIN][hdmi_hal]%s IOCTL_AUDMHL_GET_INFO: rAudinInfo.u1SpdifAudinType = %d\r\n", __func__, rAudinInfo.u1SpdifAudinType);
	pr_info("[AVIN][hdmi_hal]%s IOCTL_AUDMHL_GET_INFO: rAudinInfo.u1SpdifRawDataType = %d\r\n", __func__, rAudinInfo.u1SpdifRawDataType);

	eAudCodec = AVCODEC_ID_HDMI_PCM;
	if (rAudinInfo.u1SpdifAudinType == SPDIFIN_PCM) {
		eAudCodec = AVCODEC_ID_HDMI_PCM;
	} else {
		if (rAudinInfo.u1SpdifRawDataType == AUD_DRV_FMT_AC3) {
			eAudCodec = AVCODEC_ID_AC3;
		} else if (rAudinInfo.u1SpdifRawDataType == AUD_DRV_FMT_DTS) {
			eAudCodec = AVCODEC_ID_DTS;
		}
	}

	mAudInst = ADec_CreateInstance(eAudCodec, INS_FLAG_ISOURDMX);
	if (mAudInst == NULL) {
		pr_info("[AVIN][hdmi_hal]%s ADec_CreateInstance fail\r\n", __func__);
		return false;
	}
	pr_info("[AVIN][hdmi_hal]%s ADec_CreateInstance success\r\n", __func__);
	openAOut(AUD_OUTPUT_FRONT);/*need test,xiaochuan*/
	ADec_SetParam(mAudInst, APARAM_SET_FORMAT, NULL, false);

	if (eAudCodec == AVCODEC_ID_HDMI_PCM) {
		AUD_DEC_AUD_INFO_T *pAdecInfo = NULL;
		UINT8 uBitDepthIndex = 0;
		BOOL fgBitDepthMax = FALSE;

		pAdecInfo = (AUD_DEC_AUD_INFO_T *)kmalloc(sizeof(AUD_DEC_AUD_INFO_T), GFP_KERNEL);
		pAdecInfo->u_fmt_spec.pt_pcm_info =
			(AUD_DEC_PCM_INFO_T *)kmalloc(sizeof(AUD_DEC_PCM_INFO_T), GFP_KERNEL);

		GetSampleRateByIndex(rAudinInfo.u1AudinSampleRate, &pAdecInfo->ui4_sample_rate);

		uBitDepthIndex = rAudinInfo.u8HDMIRxAudCHSTS.WorldLen;
		fgBitDepthMax = (uBitDepthIndex & 0x1) ? TRUE : FALSE;
		uBitDepthIndex = (uBitDepthIndex >> 1) & 0X07;
		GetSampleBitDepth(fgBitDepthMax, uBitDepthIndex, &pAdecInfo->ui1_bit_depth);

		GetSpeakerLayOut(rAudinInfo.u4HDMIIRxPCMInfo.SpeakerPlacement, &pAdecInfo->e_aud_type);

		pAdecInfo->e_aud_fmt       = AUD_DEC_FMT_HDMI_IN_PCM;
		pAdecInfo->ui4_data_rate   = AUD_DATA_RATE;
		pAdecInfo->u_fmt_spec.pt_pcm_info->b_de_emphasis = FALSE;
		pAdecInfo->u_fmt_spec.pt_pcm_info->b_dlna_exist  = FALSE;
		pAdecInfo->u_fmt_spec.pt_pcm_info->ePCM_Format   = AUD_DEC_PCM_FMT_PCM_NORMAL;
		pAdecInfo->u_fmt_spec.pt_pcm_info->u2BlockAlign  = AUD_DATA_BLCOKALIGN;

		if (!ADec_SetParam(mAudInst, APARAM_PCM_T, (VOID *)pAdecInfo, false)) {
			pr_info("[AVIN][hdmi_hal]%s set adec pcm param fail\r\n", __func__);
		}

		if (pAdecInfo->u_fmt_spec.pt_pcm_info) {
			kfree(pAdecInfo->u_fmt_spec.pt_pcm_info);
			pAdecInfo->u_fmt_spec.pt_pcm_info = NULL;
		}

		if (pAdecInfo) {
			kfree(pAdecInfo);
			pAdecInfo = NULL;
		}
	}

	ADec_SetParam(mAudInst, APARAM_DISABLE_AVSYNC, NULL, true);
	ADec_Start(mAudInst, NULL);
	ADec_SetSpeed(mAudInst, 1);
	ADec_SetParam(mAudInst, APARAM_DISABLE_AVSYNC, NULL, false);

	DMXStart();
	mAudState = HDMI_AUD_STABLE;

	pr_info("[AVIN][hdmi_hal]%s leave\r\n", __func__);
	return true;
}

bool stopAdec(void)
{
	pr_info("[AVIN][hdmi_hal]%s enter\r\n", __func__);

	if (HDMI_AUD_IDLE == mAudState) {
		pr_info("[AVIN][hdmi_hal]%s already HAudioIn stop state!");
		return 0;
	}

	DMXStop();
	ADec_Stop(mAudInst);
	closeAOut(AUD_OUTPUT_FRONT);/*need test, xiaochuan*/
	mAudState = HDMI_AUD_IDLE;

	ADec_Release(mAudInst);
	mAudInst = NULL;

	pr_info("[AVIN][hdmi_hal]%s leave\r\n", __func__);
	return true;
}

bool HDMIRXParseOn(void)
{
	BOOL bRet = FALSE;
	AUDMHL_OPEN_CTRL eAudMhl = AUDMHL_START;

	pr_info("[AVIN][hdmi_hal]%s enter\r\n", __func__);

	if (AudioFilp != INVALID_HANDLE_VALUE) {
		bRet =  AudioIoCtl(AudioFilp, IOCTL_AUDMHL_CTL, &eAudMhl,
				   sizeof(eAudMhl), NULL, 0, NULL);

		if (!bRet) {
			pr_info("[AVIN][hdmi_hal]%s Faile to parse on HDMIRX!\r\n", __func__);
		} else {
			pr_info("[AVIN][hdmi_hal]%s Success parse on HDMIRX!\r\n", __func__);
		}
	}

	pr_info("[AVIN][hdmi_hal]%s leave\r\n", __func__);
	return bRet;
}

bool HDMIRXParseOff(void)
{
	BOOL bRet = FALSE;
	AUDMHL_OPEN_CTRL eAudMhl = AUDMHL_STOP;

	pr_info("[AVIN][hdmi_hal]%s enter\r\n", __func__);

	if (AudioFilp != INVALID_HANDLE_VALUE) {
		bRet =  AudioIoCtl(AudioFilp, IOCTL_AUDMHL_CTL, &eAudMhl,
				   sizeof(eAudMhl), NULL, 0, NULL);

		if (!bRet) {
			pr_info("[AVIN][hdmi_hal]%s Faile to parse off HDMIRX!\r\n", __func__);
		} else {
			pr_info("[AVIN][hdmi_hal]%s Success parse off HDMIRX!\r\n", __func__);
		}
	}

	pr_info("[AVIN][hdmi_hal]%s leave\r\n", __func__);
	return bRet;
}

static HDMI_AUD_SIGNAL_STATE_T Audio_signal_state = HDMI_AUD_FOR_NONE;

void onAudSignal(void)
{
	pr_info("[AVIN][hdmi_hal]%s enter\r\n", __func__);
	HDMIRXParseOn();
	startAdec();
	pr_info("[AVIN][hdmi_hal]%s leave\r\n", __func__);
}

void onAudNoSignal(void)
{

	pr_info("[AVIN][hdmi_hal]%s enter\r\n", __func__);
	HDMIRXParseOff();
	stopAdec();
	pr_info("[AVIN][hdmi_hal]%s leave\r\n", __func__);

}

void atc_hdmiaudio_signal_status(void *pdwMsg)
{
	uint  dwEvtData;
	UINT8 uCmdType;
	UINT8 uCmdVal;
	DWORD *pdwmsg = NULL;

	mutex_lock(&audiolock);
	pr_info("[AVIN][hdmi_hal]%s enter mAudState %d\r\n", __func__, mAudState);

	if(HDMI_AUD_STATE_STOP == mAState) {
		pr_info("[AVIN][hdmi_hal]%s mAState is STATE_STOP!\r\n", __func__);
		mutex_unlock(&audiolock);
		return;
	}

	pdwmsg = (DWORD *)pdwMsg;
	dwEvtData = (uint) *pdwmsg;
	uCmdType = dwEvtData & 0xFF;
	uCmdVal = (dwEvtData & 0xFF00) >> 8;
	if (mAudState == HDMI_AUD_IDLE) {
		if (uCmdType == AUDIN_CHG_SPDIFIN_INT_SIGNAL && uCmdVal == E_HDMIAUD_SIGNAL_LOCK) {
			if(HDMI_AUD_STATE_STOP != mAState)
			{
				/*onAudSignal();*/
				Audio_signal_state = HDMI_AUD_FOR_SIGNALON;
			}
		}
	} else {
		if (uCmdType == AUDIN_CHG_SPDIFIN_INT_SIGNAL && uCmdVal == E_HDMIAUD_SIGNAL_UNLOCK) {
			/* audio signal lost*/
			if (HDMI_AUD_FOR_SIGNALONOVER == Audio_signal_state) {
				/*onAudNoSignal();*/
				Audio_signal_state = HDMI_AUD_FOR_SIGNALOFF;
			}
		} else if (uCmdType == AUDIN_CHG_SPDIFIN_INT_FSCHG || uCmdType == AUDIN_CHG_HDMI_RX_PCM_CHLAYOUT) {
			/* audio format change*/
			if (HDMI_AUD_FOR_SIGNALONOVER == Audio_signal_state) {
				/*onAudNoSignal();*/
				Audio_signal_state = HDMI_AUD_FOR_SIGNALOFF;
			}
		} else if (uCmdType == AUDIN_CHG_SPDIFIN_INT_SIGNAL && uCmdVal == E_HDMIAUD_SIGNAL_LOCK) {
			pr_info("[AVIN][hdmi_hal]%s the second audio lock come!\r\n", __func__);
			Audio_signal_state = HDMI_AUD_FOR_SIGNALON;
		}
	}

	mutex_unlock(&audiolock);
	pr_info("[AVIN][hdmi_hal]%s leave\r\n", __func__);
}

void atc_hdmiaudio_isr(void *arg)
{
	pr_info("[AVIN][hdmi_hal]%s enter %p\r\n", __func__, *((int *)arg));
	atc_hdmiaudio_signal_status(arg);
	pr_info("[AVIN][hdmi_hal]%s leave %p\r\n", __func__, *((int *)arg));
}

EXPORT_SYMBOL(atc_hdmiaudio_isr);

int Audio_SetInputBuf_thread(void *data)
{
	while (1) {

		if (kthread_should_stop()) {
			pr_info("[AVIN][hdmi_hal]%s kthread_should_stop!\r\n", __func__);
			break;
		}

		mutex_lock(&audiolock);

		if (Audio_signal_state == HDMI_AUD_FOR_SIGNALON) {
			msleep(100);
			if(mAState != HDMI_AUD_STATE_PLAY) {
				pr_info("[AVIN][hdmi_hal]%s audio has not started, continue!\r\n", __func__);
				mutex_unlock(&audiolock);
				msleep(5);
				continue;				
			}
			if(mAudState == HDMI_AUD_STABLE) {
				pr_info("[AVIN][hdmi_hal]%s the second audio lock! before onAudNoSignal\r\n", __func__);

				onAudNoSignal();
				pr_info("[AVIN][hdmi_hal]%s the second audio lock! after onAudNoSignal\r\n", __func__);
			}
			onAudSignal();
			Audio_signal_state = HDMI_AUD_FOR_SIGNALONOVER;
		} else if (Audio_signal_state == HDMI_AUD_FOR_SIGNALOFF) {
			onAudNoSignal();
			Audio_signal_state = HDMI_AUD_FOR_NONE;
		}
		mutex_unlock(&audiolock);


		if (mAudState == HDMI_AUD_STABLE) {
			/* GetAU*/
			UINT32 u4Size = 0;
			ESM_IO_BUF_INFO *esm_info = (ESM_IO_BUF_INFO *)GetAudioOutputBuf((int*)&u4Size, 1);

			if (esm_info != NULL) {

				BOOL ret;

				ret = ADec_SetInputBuf(mAudInst, (void *)esm_info, sizeof(ESM_IO_BUF_INFO), NULL, 0);
				if (!ret) {
					;
				} else {
					ReleaseAudioOutputBuf();
				}
			}
			msleep(1);
			/*pr_info("[AVIN][hdmi_hal]%s HDMI_AUD_STABLE\r\n", __func__);*/
		} else {
			msleep(5);
			/*pr_info("[AVIN][hdmi_hal]%s HDMI_AUD_IDLE\r\n", __func__);*/
			/*schedule_timeout(20);*/
		}
	}
	return 0;
}

struct task_struct *audio_setinputbuf_task;

int hdmi_init_audio(int index)
{
	pr_info("[AVIN][hdmi_hal]%s enter\r\n", __func__);
	/*int err;*/
	mutex_init(&audiolock);
	/*mutex_lock(&audiolock);*/

	AudioFilp = filp_open("/dev/adec", O_RDWR, 0);

	if (IS_ERR(AudioFilp)) {
		pr_err("%s open mhlAudio device Fail\n", __func__);
		/*mutex_unlock(&audiolock);*/
		return -1;
	}

	atc_hdmiaudio_register_isr(atc_hdmiaudio_isr, NULL);

	audio_setinputbuf_task =
		(struct task_struct *)kthread_create(Audio_SetInputBuf_thread, NULL, "Audio_SetInputBuf_thread");
	if (IS_ERR(audio_setinputbuf_task)) {
		pr_info("[AVIN][hdmi_hal]%s create Audio_SetInputBuf_thread fail\r\n", __func__);
		PTR_ERR(audio_setinputbuf_task);
		audio_setinputbuf_task = NULL;
	} else {
		pr_info("[AVIN][hdmi_hal]%s create Audio_SetInputBuf_thread success\r\n", __func__);
	}
	wake_up_process(audio_setinputbuf_task);

	mAState = HDMI_AUD_STATE_IDLE;
	mAudState = HDMI_AUD_IDLE;
	/*mutex_unlock(&audiolock);*/
	pr_info("[AVIN][hdmi_hal]%s leave\r\n", __func__);
	return 0;
}

int hdmi_start_audio(int index)
{
	AUDMHL_OPEN_CTRL eAudMhl;

	mutex_lock(&audiolock);
	pr_info("[AVIN][hdmi_hal]%s enter\r\n", __func__);

	if (mAState == HDMI_AUD_STATE_PLAY) {
		pr_info("[AVIN][hdmi_hal]%s already play state!\r\n", __func__);
		mutex_unlock(&audiolock);
		return 0;
	}

	DMXInit();
	/*if (!AVDecoder_Init())//need change
	{
		pr_info("[AVIN][hdmi_hal]%s AVDecoder_Init failure!\r\n", __func__);
		return -1;
	}*/
	eAudMhl = AUDMHL_OPEN;

	if (!AudioIoCtl(AudioFilp, IOCTL_AUDMHL_CTL, &eAudMhl, sizeof(eAudMhl), NULL, 0, NULL)) {
		pr_info("[AVIN][hdmi_hal]%s AUDMHL_OPEN fail!\r\n", __func__);
		mutex_unlock(&audiolock);
		return -1;
	}

	mAState = HDMI_AUD_STATE_PLAY;
	mutex_unlock(&audiolock);
	pr_info("[AVIN][hdmi_hal]%s leave!\r\n", __func__);
	return 0;
}

int hdmi_stop_audio(int index)
{
	AUDMHL_OPEN_CTRL eAudMhl;
	kthread_stop(audio_setinputbuf_task);/*here, avoid of deadlock*/
	mutex_lock(&audiolock);
	pr_info("[AVIN][hdmi_hal]%s enter\r\n", __func__);

	if (mAState == HDMI_AUD_STATE_STOP || mAState == HDMI_AUD_STATE_IDLE) {
		pr_info("[AVIN][hdmi_hal]%s already stop state!\r\n", __func__);
		mutex_unlock(&audiolock);
		return 0;
	}

	atc_hdmiaudio_unregister_isr(atc_hdmiaudio_isr, NULL);
	/*kthread_stop(audio_setinputbuf_task);*/

	if (mAudState != HDMI_AUD_IDLE) {

		Audio_signal_state = HDMI_AUD_FOR_NONE;
		HDMIRXParseOff();/*need change*/
		stopAdec();
	}

	eAudMhl = AUDMHL_CLOSE;

	if (!AudioIoCtl(AudioFilp, IOCTL_AUDMHL_CTL, &eAudMhl, sizeof(eAudMhl), NULL, 0, NULL)) {
		pr_info("[AVIN][hdmi_hal]%s AUDMHL_CLOSE fail!\r\n", __func__);
		mutex_unlock(&audiolock);
		return -1;
	}
	filp_close(AudioFilp, NULL);
	AudioFilp = NULL;
	/*AVDecoder_Deinit();//need change*/
	mAudInst = NULL;/*need change*/
	DMXUninit();
	mAState = HDMI_AUD_STATE_STOP;
	mutex_unlock(&audiolock);

	if (audio_setinputbuf_task == NULL) {
		pr_info("[AVIN][hdmi_hal]%s audio_setinputbuf_task = NULL\r\n", __func__);
	}
	/*kthread_stop(audio_setinputbuf_task);*/
	audio_setinputbuf_task = NULL;
	pr_info("[AVIN][hdmi_hal]%s leave!\r\n", __func__);
	return 0;
}
