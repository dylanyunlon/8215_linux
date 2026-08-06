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

#ifndef __ATC_VSINKBUFFERPOOL_H__
#define __ATC_VSINKBUFFERPOOL_H__

#include "atcsurface.h"
#include <atcsema.h>
#include "atcvsinkbuffer.h"

#ifdef __cplusplus
extern "C" {
#endif

void atc_vsink_buffer_pool_destroy (void * pool);

bool atc_vsink_buffer_pool_init_buffers (void *pool,
    __s32 num_buffers,  ATC_VSINK_FMT_INFO_T *prFormat);

bool atc_vsink_buffer_pool_reset(void * pool,
    ATC_VSINK_FMT_INFO_T *prFormat);

void *atc_vsink_buffer_pool_new (void *parent);
bool atc_vsink_buffer_pool_free (void *atcpool);

bool atc_vsink_buffer_pool_get (void * pool, bool blocking,
    void **ppBuffer, __u32 *pBuflen);
bool atc_vsink_buffer_pool_qbuf (void * pool, void *pBuffer, __u32 buflen);
bool atc_vsink_buffer_pool_dqbuf (void * pool);

bool atc_vsink_buffer_pool_unshow (void * pool, void *pBuffer, __u32 buflen);

__s32 atc_vsink_buffer_pool_available_buffers (void *pool);

bool atc_vsink_buffer_pool_recycle_buffers (void * pool);

void *atc_vsink_buffer_pool_get_surface (void * pool);
void  atc_vsink_buffer_pool_set_surface (void *pool, void *surface);

#ifdef __cplusplus
}
#endif

#endif /* __GSTATCSINKBUFFER_H__ */
