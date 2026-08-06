/* Copyright Statement:
 *
 * This software/firmware and related documentation ("AutoChips Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to AutoChips Inc. and/or its licensors. Without
 * the prior written permission of AutoChips inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of AutoChips Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 * 
 * AutoChips Inc. (C) 2016. All rights reserved.
 * 
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("AUTOCHIPS SOFTWARE")
 * RECEIVED FROM AUTOCHIPS AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. AUTOCHIPS EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES AUTOCHIPS PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE AUTOCHIPS SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN AUTOCHIPS
 * SOFTWARE. AUTOCHIPS SHALL ALSO NOT BE RESPONSIBLE FOR ANY AUTOCHIPS SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND AUTOCHIPS'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE AUTOCHIPS SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT AUTOCHIPS'S OPTION, TO REVISE OR REPLACE THE
 * AUTOCHIPS SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO AUTOCHIPS FOR SUCH AUTOCHIPS SOFTWARE AT ISSUE.
 */

#include "x_types.h"
#include "BCLib.h"
#include "custom_protocol.h"

/*============================================================
    local variables
==============================================================*/

#define IMAGE_COUNT                 7
#define MAX_ROTATION_ANGLE_ON_ONE_SIDE 90

#define ROTATON_ANGLE_GRANULARITY  ((MAX_ROTATION_ANGLE_ON_ONE_SIDE * 2) / (IMAGE_COUNT - 1))

// 0 -- uninited
// 1 -- inited
enum trackState{
    track_uninited,
    track_inited
};
static enum trackState eTrackState = track_uninited;

static int        g_curImageIndex = -1;
static HANDLE     g_hTrackMrf     = NULL;

extern UINT32 *g_pFrameBuf;
extern char    track_data[];

extern _u4LCDWidth;
extern _u4LCDHeight;


/*============================================================
    internal functions 
==============================================================*/

static BOOL memcpyImage(int nXDest, int nYDest, int nWidth, int nHeight, UINT32 *pSrcData){
    int i = 0, j = 0;
    for (i = 0; i < nHeight; i++)
    {
        #if USE_16BITS_TRACK_IMAGE
        unsigned short *dest = g_pFrameBuf;unsigned short *src = pSrcData;
        memcpy((dest + ((i + nYDest) * _u4LCDWidth + nXDest)),(src + i*nWidth), 2*nWidth);
        #else
        memcpy(&g_pFrameBuf[(i + nYDest) * _u4LCDWidth + nXDest],&pSrcData[i*nWidth], 4*nWidth);
        #endif
    }

    return TRUE;
}

/*==============================================
   show image according to g_curImageIndex
   return value : 0 -- success; -1 --- fail
*/
static int updateTrackImage(){
    BOOL bRet = FALSE;
    BITMAPOBJINFO BitmapInfo;

    //Printf("[BackcarTrack] info: image index: %d \r\n",g_curImageIndex );
    if(track_inited != eTrackState){
        Printf("[BackcarTrack] error: track module was not initialized!");
		return -1;
	}

    bRet = GetBitmapInfo(g_hTrackMrf, g_curImageIndex, &BitmapInfo);
    if (bRet)
    {
        int dest_x = (_u4LCDWidth  - BitmapInfo.u4Width) /2;
        int dest_y = (_u4LCDHeight - BitmapInfo.u4Height)/2;
        #if 0
        Printf("[BackcarTrack] info: bofore BCABitBlt() : %d \r\n",GetARM2TickCount());
        bRet = BCABitBlt(dest_x, dest_y, BitmapInfo.u4Width, BitmapInfo.u4Height, 
            (UINT32 *)GetRCObjectMemAddr(g_hTrackMrf, (RCOBJECT *)&BitmapInfo), 0, 0, RGB(0, 0, 0), 0xaa);
        Printf("[BackcarTrack] info: after BCABitBlt() : %d \r\n",GetARM2TickCount());
        #else
        Printf("[BackcarTrack] info: bofore memcpyImage() : %d \r\n",GetARM2TickCount());
        bRet = memcpyImage(dest_x, dest_y, BitmapInfo.u4Width, BitmapInfo.u4Height, (UINT32 *)GetRCObjectMemAddr(g_hTrackMrf, (RCOBJECT *)&BitmapInfo));
        Printf("[BackcarTrack] info: after memcpyImage() : %d \r\n",GetARM2TickCount());        
        #endif

        if(bRet){
            return 0;
        }else{
            Printf("[BackcarTrack] error:show image data failed\r\n");
            return -1;
        }
    }else{
        Printf("[BackcarTrack] error: GetBitmapInfo() return false \r\n");
        return -1;
    }
}

/*==============================================
   args: -1 or 1 ; 
           -1 trun left, show next track image of left turning
             1 trun right, show next track image of right turning
    return value: same as  updateTrackImage 
*/
static int incTrackImage(int inc){

    if (inc > 0)
        g_curImageIndex++;
    else
        g_curImageIndex--;

    if (g_curImageIndex > (IMAGE_COUNT -1))
        g_curImageIndex = (IMAGE_COUNT -1);
    else if (g_curImageIndex < 0)
        g_curImageIndex = 0;

    //Printf("[BackcarTrack] info: after image pick: %d \r\n",GetARM2TickCount());

    return updateTrackImage();
}

/*==============================================
   load track mrf file. (actually file is already loaded into memory, here we just get its address)
*/
static HANDLE loadTrackMRF(){
    void* lpBuf = track_data;
    return (HANDLE)lpBuf;   
}


/*==============================================
   calculate the track image index according to angle
*/
static int angleToIndex(int angle){
    /*angle has been checked. it locates in [0, 180] */
    int index = (IMAGE_COUNT - 1)/2 ;

    if(angle >= 0 && angle < 90){
        // 0, 1, 2
        index = angle/ROTATON_ANGLE_GRANULARITY;
    }else if(angle >90 && angle <= 180){
        // [90, 179], round up --> 3 4 5 --> 4 5 6
        index = ((angle -1) + ROTATON_ANGLE_GRANULARITY - 1)/ROTATON_ANGLE_GRANULARITY + 1;
    }

    return index;
}

static int setAngle(int angle){
    int ret = -2;
    if (angle >= 0 && angle <= 180){
        ret = 0;
    }
    if (0 == ret) {
        g_curImageIndex = angleToIndex(angle);
        return updateTrackImage();
    }else{
        return -2;
    }
}

static int parseAndHandleCmdPacket(CMDPacket* prFscPacket){

    switch(prFscPacket->uSubFunc)
    {
        case SUBFUNC_BAKCCAR_TURNLEFT:
            return incTrackImage(-1);
        case SUBFUNC_BAKCCAR_TURNRIGHT:
            return incTrackImage(1);
        default:
            Printf("[BackcarTrack] info: not track related subFunc \r\n");
            break;
    }
}

/*============================================================
    Backcar Track Interface functions
==============================================================*/

/*==============================================
   init backcar track module. should be called after CustomUIInit() which will init framebuffer.

   args : none
   return value:  0 - success; -1 fail
*/
int BCTrackInit(){

    BOOL bRet = FALSE;
    BITMAPOBJINFO BitmapInfo;

    g_hTrackMrf = loadTrackMRF();

    if(-1 == g_curImageIndex){
        g_curImageIndex = (IMAGE_COUNT - 1)/2 ; // (0 1 2) left trun  3   (4 5 6) right trun ; 7 images 
    }

    Printf("[BackcarTrack] info: loadTrackMRF success !! \r\n");

    if(NULL == g_pFrameBuf){
        Printf("[BackcarTrack] error: BCTrackInit:g_pFrameBuf is null \r\n");
        return -1;
    }
    eTrackState = track_inited;
    return updateTrackImage();
}
/*==============================================
   init backcar track module. should be called after CustomUIInit() which will init framebuffer.

   args : none
   return value:  0 - success; -1 fail
*/

int BCTrackUpdate(CMDPacket* prFscPacket){
    //char needACK = 0;
    if(eTrackState != track_inited){
        Printf("[BackcarTrack] error: track module has not been successfully initialized\n");
        return -1;
    }
    if(NULL == prFscPacket){
        // should never happen
        Printf("[BackcarTrack] error: null CMDPacket pointer \n");
        return -2;
    }
    return parseAndHandleCmdPacket(prFscPacket);
}

/*==============================================
   init backcar track module. should be called after CustomUIInit() which will init framebuffer.

   args : none
   return value:  0 - success; -1 fail
*/
int BCTrackUnInit(){
    eTrackState = track_uninited;
    return 0;
}

