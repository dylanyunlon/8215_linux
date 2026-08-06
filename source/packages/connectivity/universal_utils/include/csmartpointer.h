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


#ifndef __CSMARTPOINTER_H
#define __CSMARTPOINTER_H

#include "csync.h"

namespace universal_utils {
static const char* const smartpointertag = "CSmartPointer";

template <class T>
class CSmartPointer: public CMutexObject
{
public:
    CSmartPointer(T *pointer = NULL, bool showLog = false)
        : CMutexObject()
        , m_pointer(pointer)
        , m_referenceCount(NULL)
        , m_showLog(showLog)
    {
        if (m_pointer) {
            m_referenceCount = new long(1);
        }
    }

    ~CSmartPointer() {
        deletePointer();
    }

    CSmartPointer(const CSmartPointer &rhs)
        : m_pointer(rhs.getPointer())
        , m_referenceCount(rhs.m_referenceCount) {
        addReference();
    }

    CSmartPointer& operator=(const CSmartPointer &rhs) {
        if (this != &rhs) {
            deletePointer();
            m_pointer = rhs.getPointer();
            m_referenceCount = rhs.m_referenceCount;
            addReference();
        }

        return *this;
    }

    T* operator->(){
        return m_pointer;
    }

    const T* operator->() const{
        return m_pointer;
    }

    bool operator==(const T *rhs) const {
        return rhs == m_pointer;
    }

    bool operator==(const CSmartPointer &rhs) const{
        return rhs.m_pointer == m_pointer;
    }

    bool operator!=(const T *rhs) const{
        return rhs != m_pointer;
    }
    bool operator!() const{
        return m_pointer != NULL;
    }
    bool operator!=(const CSmartPointer &rhs) const{
        return rhs.m_pointer != m_pointer;
    }

    T* getPointer() const{
        return m_pointer;
    }

    bool setSmartPointer(CSmartPointer &des, CSmartPointer &src) {
        if (src != 0) {
            src.addReference();
        }

        if (des && (des.getReferenceCount() > 0)) {
            des.removeReference();
            if (0 == des.getReferenceCount()) {
                delete des.getPointer();
                delete des.getReferenceCountPointer();
            }
        }

        des.getPointer() = src.getPointer();
        des.getReferenceCountPointer() = src.getReferenceCountPointer();

        return true;
    }

    void deletePointer() {
        if (getReferenceCount() > 0) {
            removeReference();
            if (0 == getReferenceCount()) {
                delete m_pointer;
                m_pointer = NULL;
                delete m_referenceCount;
                m_referenceCount = NULL;
            }
        }
    }

    void setPointer(T *pointer) {
        doSetPointer(&m_pointer, &pointer);
    }

    bool doSetPointer(T **pDst, T **pSrc) {
        if (!pDst || !pSrc) {
            return false;
        }

        if (*pSrc != 0) {
            (*pSrc)->addReference();
        }

        if (*pDst && (*pDst)->getReferenceCount() > 0)
        {
            (*pDst)->removeReference();
            if (0 == (*pDst)->getReferenceCount())
            {
                delete *pDst;
                *pDst = 0;
            }
        }

        *pDst = *pSrc;

        return true;
    }

    long *getReferenceCountPointer() const {
        return m_referenceCount;
    }

    long getReferenceCount() {
        long count = 0;

        if (m_referenceCount) {
            lock();
            count = *m_referenceCount;
            unlock();
        }
        return count;
    }

    void addReference() {
        if (m_referenceCount) {
            lock();
            ++*m_referenceCount;
            unlock();
        }
    }

    void removeReference() {
        if (m_referenceCount) {
            lock();
            --*m_referenceCount;
            unlock();
        }
    }

protected:

private:
    T *m_pointer;
    long *m_referenceCount;
    bool m_showLog;

};


template <class T>
static bool _SET_POINTER(T **pDst, T **pSrc)
{
    if (!pDst || !pSrc)
    {
        return false;
    }

    if (*pSrc != 0)
    {
        (*pSrc)->addReference();
    }

    if (*pDst && (*pDst)->getReferenceCount() > 0)
    {
        (*pDst)->removeReference();
        if (0 == (*pDst)->getReferenceCount())
        {
            delete *pDst;
            *pDst = 0;
        }
    }

    *pDst = *pSrc;

    return true;
}



#define DEFINITION_POINTER(MCLASS) \
    template <> void CSmartPointer<MCLASS>::setPointer(MCLASS *pointer);\
    template <> CSmartPointer<MCLASS> CSmartPointer<MCLASS>::NEW(); \
    template <> void CSmartPointer<MCLASS>::setPointer(MCLASS *pointer) \
{\
    _SET_POINTER(&m_pointer, &pointer);\
}\
    template <> CSmartPointer<MCLASS> CSmartPointer<MCLASS>::NEW()\
{\
    CSmartPointer<MCLASS> ret = new MCLASS();\
    return ret;\
}\

}
#endif //__CSMARTPOINTER_H

