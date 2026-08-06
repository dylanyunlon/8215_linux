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
#ifndef ___PERFSTAT_H___
#define ___PERFSTAT_H___
#if defined(__cplusplus)
extern "C" {
#endif   // __cplusplus

enum
{
    STAT_IDX_AEC_NDC = 0,
    STAT_IDX_UL,    
    STAT_IDX_DL,    
    STAT_IDX_UL_AEC,
    STAT_IDX_UL_NDC,
    STAT_IDX_DL_AEC,
    STAT_IDX_DL_NDC,
    STAT_IDX_DL_PLC,
    
    STAT_IDX_MAX,
};

#if (ENABLE_PERFORMANCE_STAT)

#define EXPORT_LOG_PER_MINUTES      10

#define AEC_NDC_LIMITATION      1080000//1000000 // 9ms
#define UL_LIMITATION           1552608//300000 // 6.0ms
#define DL_LIMITATION           599076//300000 // 6.0ms
#define UL_AEC_LIMITATION       971352//200000 // 2.0ms
#define UL_NDC_LIMITATION       572508//    200000 // 2.0ms
#define DL_AEC_LIMITATION       19764//   200000 // 2.0ms
#define DL_NDC_LIMITATION       575100//200000 // 2.0ms
#define DL_PLC_LIMITATION       200000 // 2.0ms; Performance test, need to modify by arm2 timer clock

BOOL TimeStatInit();
BOOL TimeStatUnInit();
BOOL TimeStatOutput(VOID);
VOID TimeStatEnter(UINT32 u4Index);
VOID TimeStatPause(UINT32 u4Index);
VOID TimeStatResume(UINT32 u4Index);
VOID TimeStatLeave(UINT32 u4Index);

#else 

#define TimeStatInit()
#define TimeStatUnInit()
#define TimeStatOutput()
#define TimeStatEnter(u4Index)
#define TimeStatPause(u4Index)
#define TimeStatResume(u4Index)
#define TimeStatLeave(u4Index)

#endif

#if defined(__cplusplus)
}
#endif   // __cplusplus

#endif /* ___PERFSTAT_H___ */
