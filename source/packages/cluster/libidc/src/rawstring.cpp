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

#include "rawstring.h"
#include <iostream>
#include <string>
#include <cstring>

RawString::RawString(void)
{
    m_buf = NULL;
    m_bufLen = 0;
}

RawString::~RawString(void)
{
    clear();
}

void RawString::clear()
{
    SAFE_DELETE_ARRAY(m_buf);
    m_bufLen = 0;
}

const unsigned char *RawString::getStr() const
{
    return m_buf;
}

RawString::RawString(const RawString &rhs)
{
    m_buf = NULL;
    m_bufLen = 0;
    setStr(rhs.getStr(), rhs.getStrLen());
}

RawString::RawString(const char *str)
{
    m_buf = NULL;
    m_bufLen = 0;

    if (str) {
        setStr((const unsigned char *)str, strlen(str) * sizeof(char));
    }
}

RawString::RawString(const unsigned char *str, int len)
{
    m_buf = NULL;
    m_bufLen = 0;
    setStr(str, len);
}

int RawString::toInt() const
{
    int ret = 0;

    if (getStrLen() >= (int)sizeof(int))
    {
        ret = *(int*)getStr();
    }

    return ret;
}

RawString::RawString(const wchar_t *wstr)
{
    m_buf = NULL;
    m_bufLen = 0;
    setStr((const unsigned char *)wstr, wcslen(wstr) * sizeof(wchar_t));
}

RawString & RawString::operator=(const RawString &rhs)
{
    if (this != &rhs)
    {
        clear();
        setStr(rhs.getStr(), rhs.getStrLen());
    }
    return *this;
}

bool RawString::operator==(const RawString &rhs) const
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

bool RawString::operator!=(const RawString &rhs) const
{
    return !(*this == rhs);
}

bool RawString::deleteStr(int startPos, int len)
{
    bool ret = false;
    if (len > 0 && startPos < m_bufLen && startPos >= 0)
    {
        if (startPos + len > m_bufLen)
        {
            len = m_bufLen - startPos;
        }

        RawString strTemp = *this;
        clear();
        setStr(strTemp.getStr(), startPos);
        addTail(strTemp.getStr() + startPos + len, strTemp.getStrLen() - startPos - len);
        ret = true;
    }

    return ret;
}

bool RawString::deleteStr(const void *str, int len)
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

bool RawString::setStr(const void *str, int len, int start)
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
        RawString strTmp;
        if (start > 0)
        {
            strTmp = *this;
        }

        clear();
        m_buf = new (std::nothrow) unsigned char[len + BUF_EDGE_SIZE + start];
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

RawString RawString::operator+(const RawString &rhs)
{
    RawString strRet = *this;
    strRet += rhs;

    return strRet;
}

const RawString & RawString::operator+=(unsigned char ch)
{
    addTail((const unsigned char *)&ch, sizeof(ch));
    return *this;
}

const RawString & RawString::operator+=(const char *str)
{
    if ((const unsigned char *)str != getStr() && str)
    {
        addTail((const unsigned char *)str, strlen(str) * sizeof(char));
    }

    return *this;
}

const RawString & RawString::operator+=(const wchar_t *wstr)
{
    if ((const unsigned char *)wstr != getStr())
    {
        addTail((const unsigned char *)wstr, wcslen(wstr) * sizeof(wchar_t));
    }
    return *this;
}

const RawString & RawString::operator+=(const RawString &rhs)
{
    if (&rhs != this)
    {
        addTail(rhs.getStr(), rhs.getStrLen());
    }
    return *this;
}

unsigned char RawString::operator[](int index) const
{
    unsigned char ret = 0;

    if (index >= 0 && index < getStrLen())
    {
        ret = *(m_buf + index);
    }
    return ret;
}

unsigned char RawString::getAt(int index) const
{
    unsigned char ret = 0;

    if (index >= 0 && index < getStrLen())
    {
        ret = *(m_buf + index);
    }
    return ret;
}

bool RawString::getAt(void *buf, int len, int start)
{
    bool ret = false;

    if (start >= 0 && start + len <= getStrLen())
    {
        memcpy(buf, m_buf + start, len);
        ret = true;
    }

    return ret;
}

bool RawString::setAt(int index, unsigned char data)
{
    bool ret = false;

    if (index >= 0 && index < getStrLen())
    {
        ret = true;
        *(m_buf + index) = data;
    }
    return ret;
}

bool RawString::setAt(void *buf, int len, int start)
{
    bool ret = false;

    if (start >= 0 && start + len < getStrLen())
    {
        ret = true;
        memcpy(m_buf + start, buf, len);
    }
    return ret;
}

bool RawString::addTail(const void *str, int len)
{
    bool ret = false;
    if (str && len > 0)
    {
        RawString strTemp = *this;
        clear();

        m_buf = new (std::nothrow) unsigned char[strTemp.getStrLen() + len + BUF_EDGE_SIZE];
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

bool RawString::addHead(const void *str, int len)
{
    bool ret = false;
    if (str && len > 0)
    {
        RawString strTemp = *this;
        clear();
        m_buf = new (std::nothrow) unsigned char[strTemp.getStrLen() + len + BUF_EDGE_SIZE];
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

int RawString::getStrLen() const
{
    return m_bufLen;
}

int RawString::compare(const void *str, int len, int start) const
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

int RawString::findStr(const void *str, int len, int start) const
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

bool RawString::formulaDelete(FORMULAFUNC pFormulaFunc,
                               RawString &strDeleted,
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

