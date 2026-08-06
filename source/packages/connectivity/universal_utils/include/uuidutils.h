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


#ifndef __UUIDUTILS_H
#define __UUIDUTILS_H

#include <string>
#include <vector>
#include <list>

namespace universal_utils {

#undef U8
#undef U16
#undef U32

#define U8  unsigned char
#define U16 unsigned short
#define U32 unsigned long

#define SDP_WRITE_8BIT(buf, idx, value)  {buf[idx++] = value;}
#define SDP_WRITE_16BIT(buf, idx, value) {buf[idx++] = (U8)((value & 0xff00) >> 8);       /* Bits[15:8] of size */ \
                                          buf[idx++] = (U8)(value & 0x00ff);              /* Bits[7:0] of size */}
#define SDP_WRITE_32BIT(buf, idx, value) {buf[idx++] = (U8)((value & 0xff000000) >> 24);  /* Bits[32:24] of size */\
                                          buf[idx++] = (U8)((value & 0x00ff0000) >> 16);  /* Bits[23:16] of size */\
                                          buf[idx++] = (U8)((value & 0x0000ff00) >> 8);   /* Bits[15:8] of size */\
                                          buf[idx++] = (U8)(value & 0x000000ff);          /* Bits[7:0] of size */}

#define SDP_DESC_UNSIGNED_INT 0x08  /* = 1 << 3 */
#define SDP_DESC_SIGNED_INT 0x10    /* = 2 << 3 */
#define SDP_DESC_UUID 0x18          /* = 3 << 3 */
#define SDP_DESC_TEXT 0x20          /* = 4 << 3 */
#define SDP_DESC_BOOL 0x28          /* = 5 << 3 */
#define SDP_DESC_SEQUENCE 0x30      /* = 6 << 3 */
#define SDP_DESC_ALTERNATIVE 0x38   /* = 7 << 3 */
#define SDP_DESC_URL 0x40           /* = 8 << 3 */

#define SDP_DESC_SIZE_1_B        0
#define SDP_DESC_SIZE_2_B        1
#define SDP_DESC_SIZE_4_B        2
#define SDP_DESC_SIZE_8_B        3
#define SDP_DESC_SIZE_16_B       4
#define SDP_DESC_SIZE_IN_NEXT_B  5
#define SDP_DESC_SIZE_IN_NEXT_2B 6
#define SDP_DESC_SIZE_IN_NEXT_4B 7

#define SDP_ATTR_SERVICE_CLASS_ID_LIST 0x0001
#define SDP_ATTR_PROTOCOL_DESC_LIST    0x0004
#define SDP_ATTR_SERVICE_NAME         (0x0000+0x0100)

#define SDP_PROT_L2CAP  0x0100
#define SDP_PROT_RFCOMM 0x0003

#define SDP_UUID_16_BIT_SIZE  2
#define SDP_UUID_32_BIT_SIZE  4
#define SDP_UUID_128_BIT_SIZE 16


typedef enum
{
    SDP_ELEM_UNSIGNED_INT,
    SDP_ELEM_SIGNED_INT,
    SDP_ELEM_UUID,
    SDP_ELEM_TEXT,
    SDP_ELEM_BOOL,
    SDP_ELEM_SEQUENCE,
    SDP_ELEM_ALTERNATIVE,
    SDP_ELEM_URL
} E_SDP_ELEMENT_TYPE;


class UuidUtils
{
public:
    UuidUtils();
    ~UuidUtils();

    const static int MAX_UUID_STR_SIZE = 37;
    const static int MAX_SDAP_UUID_NO = 30;

    static char BLUETOOTH_BASE_UUID[SDP_UUID_128_BIT_SIZE];

    static unsigned short pntohs(char *arr);
    static unsigned long pntohl(char *arr);
    static unsigned short phtons(char *arr);
    static unsigned long phtonl(char *arr);

    static void printUuid128(char *uuid128);
    static bool isAssignedUuid(char *uuid128);
    static void stringToUuid128 (char *uuid128, const char *buf);
    static void uuid128ToString(char *buf, char *uuid128);
    static void uuid16ToUuuid128(char *uuid128, unsigned short uuid16);
    static unsigned short uuid128ToUuid16(char *uuid128);
    static int uuidlistToUuid16(unsigned long service_list1,  /* 0x1100 ~ 0x111F */
                                    unsigned long  service_list2,  /* 0x1120 ~ 0x113F */
                                    unsigned long  service_list3,  /* 0x1200 ~ 0x121F */
                                    unsigned long  service_list4,  /* 0x1300~ */
                                    unsigned long  service_list5,
                                    unsigned long  service_list6,
                                    unsigned short uuid[]);

    static int getDataElementHeader(E_SDP_ELEMENT_TYPE type, int size);
    static int writeSizeBytes(char *buf, int size);
};

}

#endif // __UUIDUTILS_H
