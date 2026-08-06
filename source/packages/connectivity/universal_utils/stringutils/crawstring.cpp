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

#include <iostream>
#include <string>
#include <cstring>
#include "crawstring.h"

namespace universal_utils {
CRawString::CRawString(void)
{
    m_buf = NULL;
    m_bufLen = 0;
}

CRawString::~CRawString(void)
{
    clear();
}

void CRawString::clear()
{
    SAFE_DELETE_ARRAY(m_buf);
    m_bufLen = 0;
}

const unsigned char *CRawString::getStr() const
{
    return m_buf;
}

CRawString::CRawString(const CRawString &rhs)
{
    m_buf = NULL;
    m_bufLen = 0;
    setStr(rhs.getStr(), rhs.getStrLen());
}

CRawString::CRawString(const char *str)
{
    m_buf = NULL;
    m_bufLen = 0;

    if (str) {
        setStr((const unsigned char *)str, strlen(str) * sizeof(char));
    }
}

CRawString::CRawString(const unsigned char *str, int len)
{
    setStr(str, len);
}

int CRawString::toInt() const
{
    int ret = 0;

    if (getStrLen() >= (int)sizeof(int))
    {
        ret = *(int*)getStr();
    }

    return ret;
}

CRawString::CRawString(const wchar_t *wstr)
{
    m_buf = NULL;
    m_bufLen = 0;
    setStr((const unsigned char *)wstr, wcslen(wstr) * sizeof(wchar_t));
}

CRawString & CRawString::operator=(const CRawString &rhs)
{
    if (this != &rhs)
    {
        clear();
        setStr(rhs.getStr(), rhs.getStrLen());
    }
    return *this;
}

bool CRawString::operator==(const CRawString &rhs) const
{
    bool ret = false;

    if (&rhs == this)
    {
        ret = true;
    }
    else if (rhs.getStrLen() > 0 && getStrLen() > 0 && rhs.getStrLen() == getStrLen())
    {
        ret = (0 == memcmp(rhs.getStr(), getStr(), getStrLen()));
    }

    return ret;
}

bool CRawString::operator!=(const CRawString &rhs) const
{
    return !(*this == rhs);
}

bool CRawString::deleteStr(int startPos, int len)
{
    bool ret = false;
    if (len > 0 && startPos < m_bufLen && startPos >= 0)
    {
        if (startPos + len > m_bufLen)
        {
            len = m_bufLen - startPos;
        }

        CRawString strTemp = *this;
        clear();
        setStr(strTemp.getStr(), startPos);
        addTail(strTemp.getStr() + startPos + len, strTemp.getStrLen() - startPos - len);
        ret = true;
    }

    return ret;
}

bool CRawString::deleteStr(const void *str, int len)
{
    bool ret = false;
    int pos = findStr(str, len, 0);
    while (pos > 0)
    {
        deleteStr(pos, len);
        pos = findStr(str, len, 0);
        ret = true;
    }

    return ret;
}

bool CRawString::setStr(const void *str, int len, int start)
{
    bool ret = false;

    if (str == m_buf)
    {
        return false;
    }

    if (start > getStrLen())
    {
        start = getStrLen();
    }

    if (start < 0)
    {
        start = 0;
    }

    if (str && len >= 0)
    {
        CRawString strTmp;
        if (start > 0)
        {
            strTmp = *this;
        }

        clear();
        m_buf = new unsigned char[len + BUF_EDGE_SIZE + start];
        if (m_buf)
        {
            memset(m_buf, 0, sizeof(unsigned char) * (len + start + BUF_EDGE_SIZE));
            if (start > 0)
            {
                memcpy(m_buf, strTmp.getStr(), start);
            }
            memcpy(m_buf + start, str, len);
            m_bufLen = len + start;
            ret = true;
        }
    }else {
        m_buf = NULL;
        m_bufLen = 0;
    }

    return ret;
}

CRawString CRawString::operator+(const CRawString &rhs)
{
    CRawString strRet = *this;
    strRet += rhs;

    return strRet;
}

const CRawString & CRawString::operator+=(unsigned char ch)
{
    addTail((const unsigned char *)&ch, sizeof(ch));
    return *this;
}

const CRawString & CRawString::operator+=(const char *str)
{
    if ((const unsigned char *)str != getStr() && str)
    {
        addTail((const unsigned char *)str, strlen(str) * sizeof(char));
    }

    return *this;
}

const CRawString & CRawString::operator+=(const wchar_t *wstr)
{
    if ((const unsigned char *)wstr != getStr())
    {
        addTail((const unsigned char *)wstr, wcslen(wstr) * sizeof(wchar_t));
    }
    return *this;
}

const CRawString & CRawString::operator+=(const CRawString &rhs)
{
    if (&rhs != this)
    {
        addTail(rhs.getStr(), rhs.getStrLen());
    }
    return *this;
}

unsigned char CRawString::operator[](int index) const
{
    unsigned char ret = 0;

    if (index >= 0 && index < getStrLen())
    {
        ret = *(m_buf + index);
    }
    return ret;
}

unsigned char CRawString::getAt(int index) const
{
    unsigned char ret = 0;

    if (index >= 0 && index < getStrLen())
    {
        ret = *(m_buf + index);
    }
    return ret;
}

bool CRawString::getAt(void *buf, int len, int start)
{
    bool ret = false;

    if (start >= 0 && start + len <= getStrLen())
    {
        memcpy(buf, m_buf + start, len);
        ret = true;
    }

    return ret;
}

bool CRawString::setAt(int index, unsigned char data)
{
    bool ret = false;

    if (index >= 0 && index < getStrLen())
    {
        ret = true;
        *(m_buf + index) = data;
    }
    return ret;
}

bool CRawString::setAt(void *buf, int len, int start)
{
    bool ret = false;

    if (start >= 0 && start + len < getStrLen())
    {
        ret = true;
        memcpy(m_buf + start, buf, len);
    }
    return ret;
}

bool CRawString::addTail(const void *str, int len)
{
    bool ret = false;
    if (str && len > 0)
    {
        CRawString strTemp = *this;
        clear();

        m_buf = new unsigned char[strTemp.getStrLen() + len + BUF_EDGE_SIZE];
        if (m_buf)
        {
            memset(m_buf, 0, sizeof(unsigned char) * (strTemp.getStrLen() + len + BUF_EDGE_SIZE));
            memcpy(m_buf, strTemp.getStr(), strTemp.getStrLen());
            memcpy(m_buf + strTemp.getStrLen(), str, len);
            m_bufLen = strTemp.getStrLen() + len;
            ret = true;
        }
    }
    return ret;
}

bool CRawString::addHead(const void *str, int len)
{
    bool ret = false;
    if (str && len > 0)
    {
        CRawString strTemp = *this;
        clear();
        m_buf = new unsigned char[strTemp.getStrLen() + len + BUF_EDGE_SIZE];
        if (m_buf)
        {
            memset(m_buf, 0, sizeof(unsigned char) * (strTemp.getStrLen() + len + BUF_EDGE_SIZE));
            memcpy(m_buf, str, len);
            memcpy(m_buf + len, strTemp.getStr(), strTemp.getStrLen());
            m_bufLen = strTemp.getStrLen() + len;
            ret = true;
        }
    }
    return ret;
}

int CRawString::getStrLen() const
{
    return m_bufLen;
}

int CRawString::compare(const void *str, int len, int start) const
{
    int ret = -1;

    if (str
        && getStr()
        && getStrLen() > len
        && len > 0
        && start >= 0)
    {
        ret = memcmp(getStr() + start, str, len);
    }

    return ret;
}

int CRawString::findStr(const void *str, int len, int start) const
{
    int ret = -1;
    if (str && m_buf && len <= m_bufLen && len > 0 && start >= 0)
    {
        for (int i = start; i <= m_bufLen - len; i++)
        {
            if (!memcmp(m_buf + i, str, len))
            {
                ret = i;
                break;
            }
        }
    }

    return ret;
}

bool CRawString::formulaDelete(FORMULAFUNC pFormulaFunc,
                                CRawString &strDeleted,
                                bool bRemoveForwardRedundant)
{
    bool bRet = false;
    int nHead = -1;
    int nTail = -1;

    if (getStrLen() > 0 && pFormulaFunc) {
        bRet = pFormulaFunc(*this, nHead, nTail);
    }

    if (bRet
        && nTail >= 0
        && nTail >= nHead
        && nTail < getStrLen()) {
        bRet = strDeleted.setStr(getStr() + nHead, nTail - nHead + 1);
        if (bRemoveForwardRedundant) {
            nHead = 0;
        }
        if (bRet) {
            bRet = deleteStr(nHead, nTail - nHead + 1);
        }
    } else {
        bRet = false;
    }

    return bRet;
}

}
