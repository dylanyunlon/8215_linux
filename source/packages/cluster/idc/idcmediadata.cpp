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
 * AutoChips Inc. (C) 2023. All rights reserved.
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

#include "idcmediadata.h"

#define TAG_ATC_CLUSTER "AtcClusterMediaMetadata"

namespace clusteridcdata {

AtcClusterMediaMetadata::AtcClusterMediaMetadata()
{

}

AtcClusterMediaMetadata::~AtcClusterMediaMetadata()
{
}

void AtcClusterMediaMetadata::setPlaybackState(int state)
{
    mPlaybackState = state;
}

int AtcClusterMediaMetadata::getPlaybackState()
{
    return mPlaybackState;
}

void AtcClusterMediaMetadata::setMediaId(std::string mediaId)
{
    if (mediaId.empty()) {
        mMediaId = MEDIA_DATA_UNKNOWN;
    } else {
        mMediaId = mediaId;
    }
}

std::string AtcClusterMediaMetadata::getMediaId()
{
    return mMediaId;
}

void AtcClusterMediaMetadata::setTitle(std::string title)
{
    if (title.empty()) {
        mTitle = MEDIA_DATA_UNKNOWN;
    } else {
        mTitle = title;
    }
}

std::string AtcClusterMediaMetadata::getTitle()
{
    return mTitle;
}

void AtcClusterMediaMetadata::setArtist(std::string artist)
{
    if (artist.empty()) {
        mArtist = MEDIA_DATA_UNKNOWN;
    } else {
        mArtist = artist;
    }
}

std::string AtcClusterMediaMetadata::getArtist()
{
    return mArtist;
}

void AtcClusterMediaMetadata::setAction(int action)
{
    mAction = action;
}

int AtcClusterMediaMetadata::getAction()
{
    return mAction;
}

void AtcClusterMediaMetadata::setBitmap(std::vector<uint8_t> data)
{
    mBitmap = data;
}

std::vector<uint8_t> AtcClusterMediaMetadata::getBitmap()
{
    return mBitmap;
}

void AtcClusterMediaMetadata::setMetadata(std::string mediaId, std::string title, std::string artist, int action, std::vector<uint8_t> data)
{
    setMediaId(mediaId);
    setTitle(title);
    setArtist(artist);
    mAction = action;
    mBitmap = data;
}

}  // namespace clusteridcdata

