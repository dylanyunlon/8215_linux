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

#include <stdint.h>
#include "atcmediaclock.h"
#include <glib.h>
#define TAG_TIME "SystemTimeSource"
SystemTimeSource::SystemTimeSource() {
  pthread_mutex_init(&mPositionLock, NULL);
  mStartTimeUs = -1;
  mpbRate = 1;
  mCurrentTime = -1;
  mPauseTime = -1;
  mStarted = false;
  mMediaTime = 0;
}

SystemTimeSource::~SystemTimeSource() {
  pthread_mutex_destroy(&mPositionLock);
}
__s64 SystemTimeSource::getRealTimeUs() {
  __s64 CurTimeUs = 0;

  pthread_mutex_lock(&mPositionLock);
  CurTimeUs = (__s64)g_get_monotonic_time();

  if (CurTimeUs > mStartTimeUs) {
    mCurrentTime = (mMediaTime + (CurTimeUs - mStartTimeUs) * mpbRate);
    //PRINT_KEY("[SystemTimeSource] %s mMediaTime:%lldus, CurTimeus:%lldus, mStartTimeUs:%lldus, mpbRate: %d\r\n",
    //  __FUNCTION__, mMediaTime, CurTimeUs, mStartTimeUs, mpbRate);
    pthread_mutex_unlock(&mPositionLock);
    return mCurrentTime;
  }

  pthread_mutex_unlock(&mPositionLock);
  return mMediaTime;
}

void SystemTimeSource::setTimeUs(__s64 time) {
  pthread_mutex_lock(&mPositionLock);
  mMediaTime = time;
  //PRINT_KEY(TAG_TIME," %s mMediaTime = %lld us\r\n",
    //__FUNCTION__, mMediaTime);
  pthread_mutex_unlock(&mPositionLock);
}

void SystemTimeSource::setRate(__s32 rate) {
  pthread_mutex_lock(&mPositionLock);
  if (mStarted) {
    __s64 curtime = g_get_monotonic_time();
    if (curtime > mStartTimeUs) {
      mMediaTime += (curtime - mStartTimeUs) * mpbRate;
      mStartTimeUs = curtime;
    }
  }
  mpbRate = rate;
  pthread_mutex_unlock(&mPositionLock);
}

void SystemTimeSource::pause() {
  pthread_mutex_lock(&mPositionLock);
  if (mStarted) {
    mMediaTime += (g_get_monotonic_time() - mStartTimeUs) * mpbRate;
  }
  mStarted = false;
  pthread_mutex_unlock(&mPositionLock);
}

void SystemTimeSource::start() {
  pthread_mutex_lock(&mPositionLock);
  mStartTimeUs = g_get_monotonic_time();
  mStarted = true;
  //PRINT_KEY(TAG_TIME," %s mStartTimeUs = %lld us\r\n",
    //__FUNCTION__, mStartTimeUs);
  pthread_mutex_unlock(&mPositionLock);
}


