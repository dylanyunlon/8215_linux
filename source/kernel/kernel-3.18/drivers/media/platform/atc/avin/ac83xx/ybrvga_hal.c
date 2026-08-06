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

#include <ybr_vga_drv_if.h>
#include <media/atc/wch_if.h>
#include "ybrvga_hal.h"
#include "avin_common.h"

static WCH_SRC_APP_ID_E    mSrcAppId = SRC_APP_YPBPR;
//static int mWchId = 0;
static WCH_BUFF_INFO_T mWchBufferInfo;
static WCH_CTL_PARAM_T mWchCtl;
WCH_CFG_T mWchCfg;
static int mWidth;
static int mHeight;
static BOOL wchStop = TRUE;
static u8 sigStatus = 0;
YBR_VGA_CFG             mYbrVgaCfg;
YBR_VGA_VDO_INFO mYbrVgaVdoInfo;
static __u32 vdoinhandle;
bool need_di = false;

int ybrControl(int CtrlCode) {
    bool ErrorCode = 0;

#if 1 //mtk68031 to do
    switch (CtrlCode) {
        case IOCTL_YBR_VGA_INIT:
            {
                  
                YBR_IOControl(vdoinhandle, IOCTL_YBR_VGA_INIT, NULL, 0, NULL, 0, NULL);
                
            }
            break;
         
        case IOCTL_YBR_VGA_START: 
            {
               
                ErrorCode = YBR_IOControl(vdoinhandle, IOCTL_YBR_VGA_START , NULL, 0, NULL, 0, NULL);
               
            }
            break;

        case IOCTL_YBR_VGA_STOP:
            {
            
                ErrorCode = YBR_IOControl(vdoinhandle,IOCTL_YBR_VGA_STOP,NULL, 0, NULL, 0, NULL);
                
            }
            break;
         
        case IOCTL_YBR_VGA_CONFIG: 
            {
                
                ErrorCode = YBR_IOControl(vdoinhandle, IOCTL_YBR_VGA_CONFIG,(void*)&mYbrVgaCfg, sizeof(YBR_VGA_CFG), 
                                            NULL, 0, NULL);
                
            }
            break;
        
        case IOCTL_YBR_VGA_GET_VIDEO_INFO: 
            {
                YBR_IOControl(vdoinhandle, IOCTL_YBR_VGA_GET_VIDEO_INFO, NULL, 0, (void*)&mYbrVgaVdoInfo, sizeof(YBR_VGA_VDO_INFO),NULL);
                //return (ErrorCode);
            }
            break;
            
        case IOCTL_YBR_VGA_AUTO:    
            {
                ErrorCode = YBR_IOControl(vdoinhandle, IOCTL_YBR_VGA_AUTO, NULL, 0, NULL, 0, NULL);
            }
            break;    
             
        default:
            pr_info("ybrControl(): invalid control code!");
            return false;
    }
#endif

    pr_info("ybrControl(): Control Ybr Driver : return =%u", ErrorCode);
    return (ErrorCode > 0);
}   

extern int avin_buffer_complete(enum avin_device_type device_type, const struct capture_priv *data);

static void wch_buffer_get(u32 *bufindex) {
	struct capture_priv data;
	u32 buffer_id = *bufindex;
	bool need_hide = false;
	if (sigStatus == SV_VDO_NOSIGNAL) {
		pr_info("wch_buffer_get(): need hide vdp \r\n");
		need_hide = true;
	}

	memset(&data, 0, sizeof(struct capture_priv));
	data.ycaddr.y = mWchBufferInfo.u4YBuf[buffer_id];
	data.ycaddr.c = mWchBufferInfo.u4CBuf[buffer_id];
	data.buf_height = mHeight;
	data.buf_width = mWidth;
	data.signal_status = SIGNAL_NONE;
	data.need_hide = need_hide;
	data.di_flags = need_di;
	avin_buffer_complete(AVIN_TYPE_YPBPR, &data);
	
}

void onVdoSignal(int signalstatus) {
	struct capture_priv data;
    if (ybrControl(IOCTL_YBR_VGA_GET_VIDEO_INFO)) {
        pr_info("onVdoSignal(): IOCTL_YBR_VGA_GET_VIDEO_INFO fail!\n");
        
        return;
    }
    
    mWchCfg.fgVSyncPolarity = FALSE; // FALSE is LOW level present sync.
    mWchCfg.fgHSyncPolarity = TRUE; // TRUE is High. 

    //mWchCfg.eInputSrc = DATA_SRC_YPBPR;
    mWchCfg.eInputFmt = DATA_FMT_YUV444;

    
    mWchCfg.fgProgressive = ( mYbrVgaVdoInfo.u1Interlace ==1) ?0:1;
    mWchCfg.eOutputFmt = DATA_FMT_YUV420;

    mWchCfg.u1YSel = 1;//may change cause of different hw
    mWchCfg.u1USel = 5;//above
    mWchCfg.u1VSel = 5;//above
    
    mWchCtl.tWchCfg = mWchCfg;
    mWchCtl.eSrcId = mSrcAppId;
    if(mYbrVgaVdoInfo.u2Width == 720 && mYbrVgaVdoInfo.u1Interlace ==1)
    {
          need_di = true;
         mWchCtl.tWchCfg.fgBotFieldFirst = 0;
	mWchCtl.tWchCfg.u4SrcStartYTop = 3;
	mWchCtl.tWchCfg.u4SrcStartYBot = 3;
	mYbrVgaVdoInfo.u2Height = mYbrVgaVdoInfo.u2Height - 3;
          pr_info("video in need di\n");
    } else{
          need_di = false;
    }
    pr_info("video in onVdoSignal\n");
    pr_info("SrcWid=%d, SrcHei=%d, SrcProg=%d, InputSrc=%d\n",  
    mYbrVgaVdoInfo.u2Width, mYbrVgaVdoInfo.u2Height, 
    mYbrVgaVdoInfo.u1Interlace, mWchCfg.eInputSrc);
    
    mWchCtl.tWchCfg.u4SrcWidth = mYbrVgaVdoInfo.u2Width;
    mWchCtl.tWchCfg.u4SrcHeight = mYbrVgaVdoInfo.u2Height;
    mWchCtl.tWchCfg.u4DstWidth = mYbrVgaVdoInfo.u2Width;
    mWchCtl.tWchCfg.u4DstHeight = mYbrVgaVdoInfo.u2Height;
    mWidth = mYbrVgaVdoInfo.u2Width;
    mHeight = mYbrVgaVdoInfo.u2Height;
    mWchCtl.tWchCfg.GetWchBufIndx = wch_buffer_get;
    /*if(wchStop == FALSE){
          if (WchIoControl(1, IOCTL_WCH_STOP, (UCHAR *)&mSrcAppId, sizeof(WCH_SRC_APP_ID_E),NULL, 0, NULL)) {
        
                printk(KERN_ERR  "VideoIn::start() wch stop fail!\r\n");
                return;
          }
          printk("onVdoSignal(): stop wch!");
    }*/
    
    pr_info("onVdoSignal(): config wch!");
    if (WchIoControl(1, IOCTL_WCH_CONFIG, (UCHAR *)&mWchCtl, sizeof(WCH_CTL_PARAM_T),
        NULL, 0, NULL)) {
        pr_info("onVdoSignal(): wch config fail!");
        
        return;
    }
    pr_info("onVdoSignal(): wch config success!");
    
     if (WchIoControl(1, IOCTL_WCH_START, (UCHAR *)&mSrcAppId, sizeof(mSrcAppId),
        NULL, 0, NULL)) {
        pr_info("onVdoSignal(): wch start fail!");
        
        return;
    } else {
        wchStop = FALSE;
        pr_info("onVdoSignal(): wch start success!");
    }
    sigStatus = SV_VDO_STABLE;

	memset(&data, 0, sizeof(struct capture_priv));
	data.ycaddr.y = mWchBufferInfo.u4YBuf[0];
	data.ycaddr.c = mWchBufferInfo.u4CBuf[0];
	data.buf_height = mHeight;
	data.buf_width = mWidth;
	data.signal_status = SIGNAL_READY;
	data.need_hide = false;
	avin_buffer_complete(AVIN_TYPE_YPBPR, &data);
    pr_info("onVdoSignal(): Leave\r\n");
    
    return;
}

void onVdoNoSignal(void)
{
	struct capture_priv data;
	u32 buffer_id = 0;
	 if (WchIoControl(1, IOCTL_WCH_STOP, (UCHAR *)&mSrcAppId, sizeof(WCH_SRC_APP_ID_E),NULL, 0, NULL)) {
        
                pr_info(KERN_ERR  "VideoIn::start() wch stop fail!\r\n");
                return;
          }
          sigStatus = SV_VDO_NOSIGNAL;
          pr_info("onVdoSignal(): stop wch!");
		  memset(&data, 0, sizeof(struct capture_priv));
		  data.ycaddr.y = mWchBufferInfo.u4YBuf[buffer_id];
		  data.ycaddr.c = mWchBufferInfo.u4CBuf[buffer_id];
		  data.buf_height = mHeight;
		  data.buf_width = mWidth;
		  data.signal_status = SIGNAL_LOST;
		  data.need_hide = false;
		  avin_buffer_complete(AVIN_TYPE_YPBPR, &data);
}
void atc_ybrvga_isr(void *arg)
{
    u8 signalStatus = 0;
    signalStatus = *((u8 *)arg);
    pr_info("atc_videoin_isr(): enter %d\r\n", signalStatus);
    
    switch(signalStatus){
    	case SV_VDO_NOSIGNAL:
    		pr_info("atc_ybrvga vdo no signal\n");
    		onVdoNoSignal();
    		break;
    	case SV_VDO_STABLE:
    		pr_info("atc_ybrvga vdo signal stable\n");
    		onVdoSignal(signalStatus);
    		break;
    	default:
    		pr_info("atc_ybrvga vdo signalStatus invid\n");
    		break;
    }
    
}

int ybrvga_init_video(int index)
{
    // register tvd isr
    int ret = 0;

#if 1 //mtk68031 to do
    //vdoinhandle = YBR_Open(0,0,0);
    atc_ybr_register_isr(atc_ybrvga_isr, NULL);
#endif
    //when open wch ,wch  resource will be reserved, so when cocurrence happen, we release resource, after we get resource again
    
   
    return ret;
}

int ybrvga_set_auto(void )
{
    ybrControl(IOCTL_YBR_VGA_AUTO);
    pr_info(KERN_ERR  ":videoin_auto Leave\r\n");
    
    return 0;
}

int ybrvga_select_video(int index)
{
    pr_info("videoin: input index=%d\n", index);
    if(index == 0){
        mSrcAppId = SRC_APP_YPBPR;
        mWchCfg.eInputSrc = DATA_SRC_YPBPR;
        mYbrVgaCfg.source_type = SRC_YBR;
    } else if(index == 1){
        mSrcAppId = SRC_APP_VGA;
        mWchCfg.eInputSrc = DATA_SRC_VGA;
         mYbrVgaCfg.source_type = SRC_VGA;
    } else{
        pr_info("videoin: input index number is error\n");
    }
    return 0;
}

int ybrvga_start_video(int index) 
{
     int mWchId;
     WCH_BUFF_INFO_T bufferInfo;
    ybrvga_select_video(index);
    if (WchIoControl(1, IOCTL_WCH_OPEN, (UCHAR *)&mSrcAppId, sizeof(WCH_SRC_APP_ID_E),(UCHAR *)&mWchId, sizeof(int), NULL)) {
        
        pr_info(KERN_ERR  "VideoIn::start() wch open fail!\r\n");
        return -EINVAL;
    }
    pr_info(KERN_ERR  "VideoIn::start() wch open s!\r\n");
   
    memset(&bufferInfo,0,sizeof(WCH_BUFF_INFO_T));
    if (WchIoControl(1, IOCTL_WCH_GET_ADDR, (UCHAR *)&mSrcAppId, sizeof(WCH_SRC_APP_ID_E),
        (UCHAR *)&mWchBufferInfo, sizeof(WCH_BUFF_INFO_T), NULL)) {
        pr_info(KERN_ERR  "VideoIn::start() IOCTL_WCH_GET_ADDR L_FAILED!");
        
        return -EINVAL;
    }
    pr_info(KERN_ERR  ":videoin_start_video enter\r\n");
    ybrControl(IOCTL_YBR_VGA_INIT);
    ybrControl(IOCTL_YBR_VGA_CONFIG);
    ybrControl(IOCTL_YBR_VGA_START);
    pr_info(KERN_ERR  ":videoin_start_video Leave\r\n");
    
    return 0;
}

int ybrvga_stop_video(void)
{
     bool ret = false;
     if (WchIoControl(1, IOCTL_WCH_STOP, (UCHAR *)&mSrcAppId, sizeof(WCH_SRC_APP_ID_E),NULL, 0, NULL)) {
        
        pr_info(KERN_ERR  "VideoIn::start() wch stop fail!\r\n");
        return -1;
    }
    printk(KERN_ERR  "VideoIn::start() wch stop s!\r\n");
    
    if (WchIoControl(1, IOCTL_WCH_CLOSE, (UCHAR *)&mSrcAppId, sizeof(WCH_SRC_APP_ID_E), NULL, 0, NULL)) {
        
        pr_info(KERN_ERR  "VideoIn::start() wch close fail!\r\n");
        return -1;
    }
    pr_info(KERN_ERR  "VideoIn::start() wch close s!\r\n");
    
    ybrControl(IOCTL_YBR_VGA_STOP);
    pr_info(KERN_ERR  "VideoIn::start() ybr stop s!\r\n");
    
    return 0;
}

