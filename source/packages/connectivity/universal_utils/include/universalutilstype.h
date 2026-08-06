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

#ifndef __UNIVERSALUTILSTYPE__
#define __UNIVERSALUTILSTYPE__
#include <stddef.h>

namespace universal_utils {
#ifdef MIN
#undef MIN
#endif
#ifndef MIN
#define MIN(a,b) ((a < b) ? a : b)
#endif

#ifndef SAFE_DELETE
#define SAFE_DELETE(a) {if (a) {delete a; a = NULL;}}
#endif

#ifndef SAFE_DELETE_ARRAY
#define SAFE_DELETE_ARRAY(a) {if (a) {delete [] a; a = NULL;}}
#endif

#define CHECKNULL_RETURNBOOL(ptr) if(NULL == ptr)           \
            {                                               \
                UTILS_LOGE(TAG, "ERROR! %s is null!", (#ptr));  \
                return false;                               \
            }                                               \

#define CHECKNULL_RETURNINT(ptr) if(NULL == ptr)            \
            {                                               \
                UTILS_LOGE(TAG, "ERROR! %s is null!", (#ptr));  \
                return BLUETOOTH_ERROR_NULL_POINT;          \
            }                                               \

#define CHECKNULL_RETURNVOID(ptr) if(NULL == ptr)           \
            {                                               \
                UTILS_LOGE(TAG, "ERROR! %s is null!", (#ptr));  \
                return;                                     \
            }                                               \

}
#endif // UNIVERSAL_TYPE

