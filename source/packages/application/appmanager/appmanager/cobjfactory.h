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
 
#ifndef COBJFACTORY_H
#define COBJFACTORY_H

#include <string>
#include <iostream>
#include <stddef.h>
#include <list>
#include <stdlib.h>
#include <dlfcn.h>
#include "appobj.h"
#include "apptype.h"

class LibObj;

class APPConfig
{
public:
    APPConfig()
        : m_minFreeRamKB(0)
        , m_minFreeRamPCT(0)
        , m_maxAPPCount(0)
    {
    }

    unsigned long m_minFreeRamKB;
    unsigned char m_minFreeRamPCT;
    int m_maxAPPCount;
};

class CObjFactory
{
public:
    CObjFactory();
    virtual ~CObjFactory();

    bool init(const std::string &appXMLFile);
    CAPPControllerObj *createObj(unsigned char appID);
    bool destroyObj(CAPPControllerObj *obj);
    const APPConfig *getConf();

protected:
    class ObjProperty
    {
    public:
        ObjProperty()
            : appID(0)
            , priority(10)
            , isAudio(false)
            , isVideo(false)
            , needResume(false)
            , audioState(CAPPBaseObj::LEVEL_NORMAL)
        {
        }

        unsigned char appID;
        int priority;
        std::string  name;
        std::string filePath;
        std::string libPath;
        bool isAudio;
        bool isVideo;
        bool needResume;
        std::string shareProcess;
        CAPPBaseObj::E_PARAMETER audioState;
    };

private:
    ObjProperty *findProperty(unsigned char appID) const;
    LibObj *loadLib(std::string &libPath);
    bool parserXml(const std::string &appXMLFile,
                    std::list<ObjProperty *> &proList);

    CObjFactory(const CObjFactory &rhs);
    const CObjFactory& operator = (const CObjFactory &rhs);

    std::list<CAPPControllerObj *> m_objList;
    std::list<ObjProperty *> m_propertyList;
    std::list<LibObj *> m_libList;
    APPConfig *m_config;
};

#endif // COBJFACTORY_H
