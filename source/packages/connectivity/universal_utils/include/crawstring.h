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

#ifndef CRAMSTRING_H
#define CRAMSTRING_H
#include "universalutilstype.h"

namespace universal_utils {
class CRawString;
typedef bool (*FORMULAFUNC)(const CRawString &strSrc, int &nHead, int &nTail);

class CRawString
{
public:
    CRawString(void);
    ~CRawString(void);

    CRawString(const CRawString &rhs);
    CRawString(const char *str);
    CRawString(const wchar_t *wstr);
    CRawString(const unsigned char *str, int strLen);
    CRawString & operator=(const CRawString &rhs);
    bool operator==(const CRawString &rhs) const;
    bool operator!=(const CRawString &rhs) const;
    CRawString operator+(const CRawString &rhs);
    const CRawString & operator+=(const CRawString &rhs);
    const CRawString & operator+=(const char *str);
    const CRawString & operator+=(unsigned char ch);
    const CRawString & operator+=(const wchar_t *wstr);
    unsigned char operator[](int index) const;

    int toInt() const;
    unsigned char getAt(int index) const;
    bool getAt(void *buf, int len, int start = 0);
    bool setAt(int index, unsigned char data);
    bool setAt(void *buf, int len, int start = 0);
    int compare(const void *str, int len, int start = 0) const;

    bool setStr(const void *str, int len, int start = 0);
    bool addTail(const void *str, int len);
    bool addHead(const void *str, int len);
    int findStr(const void *str, int len, int start) const;
    bool deleteStr(int startPos, int len);
    bool deleteStr(const void *str, int len);
    int getStrLen() const;
    const unsigned char *getStr() const;
    void clear();
     bool formulaDelete(FORMULAFUNC pFormulaFunc,
                        CRawString &strDeleted,
                        bool bRemoveForwardRedundant = true);

private:
    static const int BUF_EDGE_SIZE = 2; //for string buffer, like char or wchar_t.

    unsigned char *m_buf;
    int m_bufLen;
};

}
#endif // CRAMSTRING_H
