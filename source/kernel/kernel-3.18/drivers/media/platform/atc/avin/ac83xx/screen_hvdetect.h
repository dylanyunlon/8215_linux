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


#ifndef __SCREEN_HVDETECT_H__
#define __SCREEN_HVDETECT_H__

//Identify wr_channel buffer rotate status algorithm
#define HALF_DIVISION (2)
#define VIDEO_RATIO_4_3  (3.0/4)
#define SCAN_INTERVAL_LINE_DFT_FOR_480P (4)
#define SCAN_INTERVAL_LINE_FOR_720P     (6) 
#define SCAN_INTERVAL_LINE_FOR_1080P    (9) 

typedef struct WFF_YC_VALUE_T{
    unsigned long u4YValue;
    unsigned long u4UValue;
    unsigned long u4VValue;
}WFF_YC_VALUE_T;

typedef enum{
    RECT_NULL,
    BUFFER_LRERR,       //Could not find left and right position!
    BUFFER_TBERR,        //Could not find top and bottom position!
    BUFFER_INACTIVE,
    BUFFER_ROTATE,
    BUFFER_UNROTATE
}RETNO;

typedef enum {
    MODE_NULL = 0x0,
    MODE_BLOCK = 0x1,
    MODE_LINE = 0x2
} VIDEO_MODE_INFO_E;

typedef struct VIDEO_INFO{
    unsigned long u4YVaAddr;
    unsigned long u4CVaAddr;
    unsigned long u4Height;
    unsigned long u4Width;
    VIDEO_MODE_INFO_E u4Mode;
}VIDEO_INFO_T;


typedef struct tagRECT_HV
{
	long left;
	long top;
	long right;
	long bottom;
}RECT_HV, *PRECT_HV;

RETNO GR_GetActiveRect(VIDEO_INFO_T InVdoInfo, PRECT_HV pRect);
    
#endif
