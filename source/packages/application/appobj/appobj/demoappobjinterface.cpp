/*
* Copyright (c) 2016 AutoChips Inc.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*    http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/
 
#include <iostream>
#include <stddef.h>
#include "apptype.h"
#include "cappobject.h"
#include "demoappobjinterface.h"
#include "applog.h"

class CObjManager
{
public:
    CObjManager(){}
    ~CObjManager()
    {
        for (std::list<CAPPBaseObj *>::iterator iter = m_objList.begin();
                iter != m_objList.end();
                iter++) {
            CAPPBaseObj *obj = (*iter);
            SAFE_DELETE(obj);
        }
    }

    bool addObj(CAPPBaseObj *obj)
    {
        m_objList.push_back(obj);
        return true;
    }

    bool deleteObj(CAPPBaseObj *obj)
    {
        bool ret = false;

        for (std::list<CAPPBaseObj *>::iterator iter = m_objList.begin();
                iter != m_objList.end();
                iter++) {
            CAPPBaseObj *findObj = (*iter);
            if (findObj == obj) {
                SAFE_DELETE(findObj);
                m_objList.erase(iter);
                ret = true;
                break;
            }
        }

        return ret;
    }

private:
    std::list<CAPPBaseObj *> m_objList;
};

static CObjManager g_objManager;

CAPPControllerObj *createAPPControllerObj(unsigned char appID)
{
    CAPPControllerObj *objCtl = new CAPPObject();
    if (NULL == objCtl) {
        LOGE("", "new CAPPObject fail\n");
    } else {
        objCtl->setAppID(appID);
        g_objManager.addObj(objCtl);
    }

    return objCtl;
}

CAPPTargetObj *createAPPTargetObj(unsigned char appID)
{
    CAPPTargetObj *objTag = new CAPPObject();
    if (NULL == objTag) {
        LOGE("", "new CAPPObject fail\n");
    } else {
        objTag->setAppID(appID);
        g_objManager.addObj(objTag);
    }

    return objTag;
}

bool releaseAppObj(CAPPBaseObj *obj)
{
    return g_objManager.deleteObj(obj);
}

