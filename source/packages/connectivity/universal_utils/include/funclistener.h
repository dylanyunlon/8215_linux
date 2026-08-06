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

#ifndef FUNCLISTENER_H
#define FUNCLISTENER_H

namespace universal_utils {

#ifndef SAFE_DELETE
#define SAFE_DELETE(a)  {if (a) {delete a; a = nullptr;}}
#endif

class CListenerObj
{
public:
    ~CListenerObj() {}
};

}
namespace universal_utils {
typedef bool (CListenerObj::*LISTENER_FUNC)();
typedef bool (CListenerObj::*LISTENER_FUNC_WITH_PARA)(unsigned int,
    unsigned int, unsigned int);
typedef bool (CListenerObj::*LISTENER_FUNC_WITH_PARA2)(unsigned char*,
    unsigned int);
typedef bool (CListenerObj::*LISTENER_FUNC_WITH_PARA3)(unsigned char,
    unsigned char*, unsigned long long);
typedef bool (CListenerObj::*LISTENER_FUNC_WITH_PARA4)(unsigned char*,
    unsigned int, bool);
}

namespace universal_utils {
class CFuncListener
{
public:
    CFuncListener()
        : m_pListenerObj(nullptr)
        , m_pFunc(nullptr)
        , m_pFuncWithParam(nullptr)
        , m_pFuncWithParam2(nullptr)
        , m_pFuncWithParam3(nullptr)
        , m_pFuncWithParam4(nullptr)
    {
    }

    CFuncListener(CListenerObj *pObj, LISTENER_FUNC pFunc)
        : m_pListenerObj(pObj)
        , m_pFunc(pFunc)
        , m_pFuncWithParam(nullptr)
        , m_pFuncWithParam2(nullptr)
        , m_pFuncWithParam3(nullptr)
        , m_pFuncWithParam4(nullptr)
    {
    }

    CFuncListener(CListenerObj *pObj, LISTENER_FUNC_WITH_PARA pFunc)
        : m_pListenerObj(pObj)
        , m_pFunc(nullptr)
        , m_pFuncWithParam(pFunc)
        , m_pFuncWithParam2(nullptr)
        , m_pFuncWithParam3(nullptr)
        , m_pFuncWithParam4(nullptr)
    {
    }

    ~CFuncListener() {}

    void setListenerObj(CListenerObj *pObj, LISTENER_FUNC_WITH_PARA2 pFunc)
    {
        m_pListenerObj = pObj;
        m_pFuncWithParam2 = pFunc;
    }

    void setListenerObj(CListenerObj *pObj, LISTENER_FUNC_WITH_PARA3 pFunc)
    {
        m_pListenerObj = pObj;
        m_pFuncWithParam3 = pFunc;
    }

    void setListenerObj(CListenerObj *pObj, LISTENER_FUNC_WITH_PARA4 pFunc)
    {
        m_pListenerObj = pObj;
        m_pFuncWithParam4 = pFunc;
    }


    bool doFunc()
    {
        bool ret = false;

        if (m_pListenerObj && m_pFunc)
        {
            ret = (m_pListenerObj->*m_pFunc)();
        }

        return ret;
    }

    bool doFunc(unsigned int param1, unsigned int param2, unsigned int param3)
    {
        bool ret = false;

        if (m_pListenerObj && m_pFuncWithParam)
        {
            ret = (m_pListenerObj->*m_pFuncWithParam)(param1, param2, param3);
        }

        return ret;
    }

    bool doFunc(unsigned char* wParam, unsigned int lParam)
    {
        bool ret = false;

        if (m_pListenerObj && m_pFuncWithParam2)
        {
            ret = (m_pListenerObj->*m_pFuncWithParam2)(wParam, lParam);
        }

        return ret;
    }

    bool doFunc(unsigned char param1, unsigned char* param2, unsigned long long param3)
    {
        bool ret = false;

        if (m_pListenerObj && m_pFuncWithParam3)
        {
            ret = (m_pListenerObj->*m_pFuncWithParam3)(param1, param2, param3);
        }

        return ret;
    }

    bool doFunc(unsigned char* wParam, unsigned int lParam, bool success = true)
    {
        bool ret = false;

        if (m_pListenerObj && m_pFuncWithParam4)
        {
            ret = (m_pListenerObj->*m_pFuncWithParam4)(wParam, lParam, success);
        }

        return ret;
    }


private:
    CListenerObj* m_pListenerObj;
    LISTENER_FUNC m_pFunc;
    LISTENER_FUNC_WITH_PARA m_pFuncWithParam;
    LISTENER_FUNC_WITH_PARA2 m_pFuncWithParam2;
    LISTENER_FUNC_WITH_PARA3 m_pFuncWithParam3;
    LISTENER_FUNC_WITH_PARA4 m_pFuncWithParam4;
};

}

#endif /*FUNCLISTENER_H*/