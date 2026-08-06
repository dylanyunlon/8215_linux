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


#include "uuidutils.h"
#include "clog.h"
#include <string.h>
#include <arpa/inet.h>
#include <stdlib.h>

namespace universal_utils {

const static char *TAG = "UuidUtils";
const static bool DBG = false;

char UuidUtils::BLUETOOTH_BASE_UUID[SDP_UUID_128_BIT_SIZE] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB
};

UuidUtils::UuidUtils()
{

}

UuidUtils::~UuidUtils()
{

}

unsigned short UuidUtils::pntohs(char *arr)
{
    if( (((U32)arr) & 0x1) ) {
        unsigned short tmp;
        memcpy(&tmp, arr, sizeof(unsigned short));
        return ntohs(tmp);
    } else {
        return ntohs(*(unsigned short*)arr);
    }
}

unsigned long UuidUtils::pntohl(char *arr)
{
    if( (((U32)arr) & 0x3) ) {
        unsigned long tmp;
        memcpy(&tmp, arr, sizeof(unsigned long));
        return ntohl(tmp);
    } else {
        return ntohl(*(unsigned long*)arr);
    }
}

unsigned short UuidUtils::phtons(char *arr)
{
    if((((U32)arr) & 0x1) ) {
        unsigned short tmp;
        memcpy(&tmp, arr, sizeof(unsigned short));
        return htons(tmp);
    } else {
        return htons(*(unsigned short*)arr);
    }
}

unsigned long UuidUtils::phtonl(char *arr)
{
    if( (((U32)arr) & 0x3) ) {
        unsigned long tmp;
        memcpy(&tmp, arr, sizeof(unsigned long));
        return htonl(tmp);
    } else {
        return htonl(*(unsigned long*)arr);
    }
}

void UuidUtils::printUuid128(char *uuid128)
{
    if (DBG) UTILS_LOGI(TAG, "printUuid128 : %02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
            uuid128[0], uuid128[1], uuid128[2], uuid128[3], uuid128[4], uuid128[5], uuid128[6], uuid128[7],
            uuid128[8], uuid128[9], uuid128[10], uuid128[11], uuid128[12], uuid128[13], uuid128[14], uuid128[15]);
}

bool UuidUtils::isAssignedUuid(char *uuid128)
{
    return (memcmp(&BLUETOOTH_BASE_UUID[4], &uuid128[4], 12) == 0);
}

void UuidUtils::stringToUuid128 (char *uuid128, const char *buf)
{
    char *pch = (char *)buf, *endch = NULL, ch;
    unsigned long value_n, value_h, i = 0;
    char tmp[SDP_UUID_128_BIT_SIZE];

    if (DBG) UTILS_LOGI(TAG, "stringToUuid128 : %s", buf);
    memset(tmp, 0x0, SDP_UUID_128_BIT_SIZE);
    value_n = strtoul(pch, &endch, 16);
    value_h = pntohl((char *)&value_n);
    memcpy(tmp, &value_h, 4);

    pch = endch + 1;
    value_n = strtoul(pch, &endch, 16);
    value_h = pntohs((char *)&value_n);
    memcpy(tmp + 4, &value_h, 2);

    pch = endch + 1;
    value_n = strtoul(pch, &endch, 16);
    value_h = pntohs((char *)&value_n);
    memcpy(tmp + 6, &value_h, 2);

    pch = endch + 1;
    value_n = strtoul(pch, &endch, 16);
    value_h = pntohs((char *)&value_n);
    memcpy(tmp + 8, &value_h, 2);

    pch = endch + 1;
    ch = *(pch + 8);
    *(pch + 8) = '\0';
    value_n = strtoul(pch, &endch, 16);
    value_h = pntohl((char *)&value_n);
    memcpy(tmp + 10, &value_h, 4);
    *(pch + 8) = ch;

    pch = endch;
    value_n = strtoul(pch, &endch, 16);
    value_h = pntohs((char *)&value_n);
    memcpy(tmp + 14, &value_h, 2);

    while (i < SDP_UUID_128_BIT_SIZE) {
        uuid128[i] = tmp[i];
        i++;
    }

    printUuid128(uuid128);
}

void UuidUtils::uuid128ToString(char *buf, char *uuid128)
{
    snprintf(buf, MAX_UUID_STR_SIZE, "%.8lx-%.4x-%.4x-%.4x-%.8lx%.4x",
             pntohl(&uuid128[0]), pntohs(&uuid128[4]),
             pntohs(&uuid128[6]), pntohs(&uuid128[8]),
             pntohl(&uuid128[10]), pntohs(&uuid128[14]));
    if (DBG) UTILS_LOGI(TAG, "uuid128ToString : %s", buf);
}

void UuidUtils::uuid16ToUuuid128(char *uuid128, unsigned short uuid16)
{
    if (DBG) UTILS_LOGI(TAG, "uuid16ToUuuid128(0x%x)", uuid16);
    memcpy(uuid128, BLUETOOTH_BASE_UUID, SDP_UUID_128_BIT_SIZE);
    uuid16 += pntohs(&uuid128[2]);
    uuid16 = htons(uuid16);
    memcpy(&uuid128[2], &uuid16, 2);
}

unsigned short UuidUtils::uuid128ToUuid16(char *uuid128)
{
    unsigned short *data0 = (unsigned short *)&uuid128[2];
    unsigned short *data1 = (unsigned short *)&BLUETOOTH_BASE_UUID[2];
    if (DBG) UTILS_LOGI(TAG, "uuid128ToUuid16: 0x%X, 0x%X", *data0, *data1);

    return (pntohs(&uuid128[2]) - pntohs(&BLUETOOTH_BASE_UUID[2]));
}

int UuidUtils::uuidlistToUuid16(unsigned long service_list1,  /* 0x1100 ~ 0x111F */
                                    unsigned long  service_list2,  /* 0x1120 ~ 0x113F */
                                    unsigned long  service_list3,  /* 0x1200 ~ 0x121F */
                                    unsigned long  service_list4,  /* 0x1300~ */
                                    unsigned long  service_list5,
                                    unsigned long  service_list6,
                                    unsigned short uuid[])
{
    U8 i, idx = 0;
    if (uuid == NULL) {
        return 0;
    }

    for (i = 0; i < MAX_SDAP_UUID_NO && idx < 32 * 6; idx++) {
        if (idx < 32) {
            if ((service_list1 >> idx) & 0x1) {
                uuid[i] = 0x1100 | idx;
                if (DBG) UTILS_LOGI(TAG, "uuid[%d] = 0x%x", i, uuid[i]);
                i++;
            }
        } else if (idx < 64) {
            int bit = idx - 32;
            if ((service_list2 >> bit) & 0x1) {
                uuid[i] = 0x1120 | bit;
                if (DBG) UTILS_LOGI(TAG, "uuid[%d] = 0x%x", i, uuid[i]);
                i++;
            }
        } else if (idx < 96) {
            int bit = idx - 64;
            if ((service_list3 >> bit) & 0x1) {
                uuid[i] = 0x1200 | bit;
                if (DBG) UTILS_LOGI(TAG, "uuid[%d] = 0x%x", i, uuid[i]);
                i++;
            }
        } else if (idx < 128) {
            int bit = idx - 96;
            if ((service_list4 >> bit) & 0x1) {
                uuid[i] = 0x1300 | bit;
                if (DBG) UTILS_LOGI(TAG, "uuid[%d] = 0x%x", i, uuid[i]);
                i++;
            }
        } else if (idx < 160) {
            int bit = idx - 128;
            if ((service_list5 >> bit) & 0x1) {
                uuid[i] = 0x1400 | bit;
                if (DBG) UTILS_LOGI(TAG, "entry.uuid[%d] = 0x%x", i, uuid[i]);
                i++;
            }
        } else if (idx < 192) {
            int bit = idx - 160;
            if ((service_list6 >> bit) & 0x1) {
                uuid[i] = 0x1800 | bit;
                if (DBG) UTILS_LOGI(TAG, "[GAP] uuid[%d] = 0x%x", i, uuid[i]);
                i++;
            }
        }
    }

    return i;
}

int UuidUtils::getDataElementHeader(E_SDP_ELEMENT_TYPE type, int size)
{
    int size_desc;
    switch (type) {
    case SDP_ELEM_BOOL:
        return (SDP_DESC_BOOL | SDP_DESC_SIZE_1_B);
    case SDP_ELEM_SIGNED_INT:
        switch (size) {
        case 1:
            return (SDP_DESC_SIGNED_INT | SDP_DESC_SIZE_1_B);
        case 2:
            return (SDP_DESC_SIGNED_INT | SDP_DESC_SIZE_2_B);
        case 4:
            return (SDP_DESC_SIGNED_INT | SDP_DESC_SIZE_4_B);
        case 8:
            return (SDP_DESC_SIGNED_INT | SDP_DESC_SIZE_8_B);
        case 16:
            return (SDP_DESC_SIGNED_INT | SDP_DESC_SIZE_16_B);
        default:
            return 0;
        }
    case SDP_ELEM_UNSIGNED_INT:
        switch (size) {
        case 1:
            return (SDP_DESC_UNSIGNED_INT | SDP_DESC_SIZE_1_B);
        case 2:
            return (SDP_DESC_UNSIGNED_INT | SDP_DESC_SIZE_2_B);
        case 4:
            return (SDP_DESC_UNSIGNED_INT | SDP_DESC_SIZE_4_B);
        case 8:
            return (SDP_DESC_UNSIGNED_INT | SDP_DESC_SIZE_8_B);
        case 16:
            return (SDP_DESC_UNSIGNED_INT | SDP_DESC_SIZE_16_B);
        default:
            return 0;
        }
    case SDP_ELEM_UUID:
        switch (size) {
        case 2:
            return (SDP_DESC_UUID | SDP_DESC_SIZE_2_B);
        case 4:
            return (SDP_DESC_UUID | SDP_DESC_SIZE_4_B);
        case 16:
            return (SDP_DESC_UUID | SDP_DESC_SIZE_16_B);
        default:
            return 0;
        }
    default:
        if (size < 0xFF) {
            size_desc = SDP_DESC_SIZE_IN_NEXT_B;
        } else if (size < 0xFFFF) {
            size_desc = SDP_DESC_SIZE_IN_NEXT_2B;
        } else {
            size_desc = SDP_DESC_SIZE_IN_NEXT_4B;
        }
    }

    switch (type) {
    case SDP_ELEM_TEXT:
        return (SDP_DESC_TEXT | size_desc);
    case SDP_ELEM_URL:
        return (SDP_DESC_URL | size_desc);
    case SDP_ELEM_SEQUENCE:
        return (SDP_DESC_SEQUENCE | size_desc);
    case SDP_ELEM_ALTERNATIVE:
        return (SDP_DESC_ALTERNATIVE | size_desc);
    default:
        return 0;
    }

    return 0;
}

int UuidUtils::writeSizeBytes(char *buf, int size)
{
    int idx = 0;

    if (size < 0xFF) {
        SDP_WRITE_8BIT(buf, idx, size);
    } else if (size < 0xFFFF) {
        SDP_WRITE_16BIT(buf, idx, size);
    } else {
        SDP_WRITE_32BIT(buf, idx, size);
    }

    return idx;
}

}

