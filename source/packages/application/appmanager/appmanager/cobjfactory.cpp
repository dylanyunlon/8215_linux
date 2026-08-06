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

#include "cobjfactory.h"
#include "applog.h"
#include "tinyxml2.h"
#include "stringutils.h"
#include <QFile>

using namespace universal_utils;

static const char TAG[] = "CObjFactory";

typedef CAPPControllerObj* (*CREATOBJ)(unsigned char appID);
typedef bool (*RELEASEOBJ)(CAPPBaseObj *obj);

class LibObj{
public:
    LibObj();
    ~LibObj();
    bool isLoaded (std::string &libPath) const;
    bool load (std::string &libPath);
    CREATOBJ getCreateInterface ();
    RELEASEOBJ getReleaseInterface ();

private:
    void *libHandle;
    CREATOBJ createAPPControllerObj;
    RELEASEOBJ releaseAppObj;
    std::string libName;
};

LibObj::LibObj()
    : libHandle(NULL)
    , createAPPControllerObj(NULL)
    , releaseAppObj(NULL)
{
}

LibObj::~LibObj()
{
    if (NULL != libHandle) {
        dlclose(libHandle);
        libHandle = NULL;
        createAPPControllerObj = NULL;
        releaseAppObj = NULL;
    }
}

bool LibObj::isLoaded(std::string &libPath) const
{
    bool ret = false;

    if (libPath == this->libName
        && NULL !=createAPPControllerObj
        && NULL != releaseAppObj) {
        ret = true;
    }

    return ret;
}

bool LibObj::load(std::string &libPath)
{
    bool ret = false;

    if (NULL == libHandle) {
        libHandle = dlopen(libPath.c_str(), RTLD_LAZY);
    }

    if (NULL == libHandle) {
        LOGE(TAG, "load lib (%s) error: %s\n", libPath.c_str(), dlerror());
    } else {
        createAPPControllerObj = (CREATOBJ)dlsym(libHandle, "createAPPControllerObj");
        releaseAppObj = (RELEASEOBJ)dlsym(libHandle, "releaseAppObj");
    }

    if (NULL == createAPPControllerObj || NULL == releaseAppObj) {
        LOGE(TAG, "lib (%s) can not find interface! \n", libPath.c_str());
    } else {
        this->libName = libPath;
        ret = true;
    }

    return ret;
}

CREATOBJ LibObj::getCreateInterface()
{
    return createAPPControllerObj;
}

RELEASEOBJ LibObj::getReleaseInterface()
{
    return releaseAppObj;
}

//====================================================================

CObjFactory::CObjFactory()
    : m_config(NULL)
{
}

CObjFactory::~CObjFactory()
{
    while (m_objList.size() != 0) {
        destroyObj(*(m_objList.begin()));
    }

    for(std::list<ObjProperty*>::iterator iter = m_propertyList.begin();
            iter != m_propertyList.end();
            iter++) {
        SAFE_DELETE(*(iter));
    }

    for(std::list<LibObj*>::iterator iter = m_libList.begin();
            iter != m_libList.end();
            iter++) {
        SAFE_DELETE(*(iter));
    }

    SAFE_DELETE(m_config);
}

bool CObjFactory::init(const std::string &appXMLFile)
{
    LOGD(TAG, "CObjFactory::init enter\n");
   parserXml(appXMLFile, m_propertyList);
/*
   for(std::list<ObjProperty*>::iterator iter = m_propertyList.begin(); iter != m_propertyList.end(); iter++) {
       LOGD(TAG, "appobj===>\r\n");
       LOGD(TAG, "appID:%d\r\n", (*iter)->appID);
       LOGD(TAG, "priority:%d\r\n", (*iter)->priority);
       LOGD(TAG, "name:%s\r\n", (*iter)->name.c_str());
       LOGD(TAG, "filePath:%s\r\n", (*iter)->filePath.c_str());
       LOGD(TAG, "libPath:%s\r\n", (*iter)->libPath.c_str());
       LOGD(TAG, "isAudio:%d\r\n", (*iter)->isAudio);
       LOGD(TAG, "isVideo:%d\r\n", (*iter)->isVideo);
   }
*/
    LOGD(TAG, "CObjFactory::init leave\n");
   return true;
}

CAPPControllerObj *CObjFactory::createObj(unsigned char appID)
{
    LOGD(TAG, "createObj:%d\n", appID);
    CAPPControllerObj *obj = NULL;
    LibObj *lib = NULL;
    CREATOBJ createAPPControllerObj = NULL;
    ObjProperty *property = findProperty(appID);

    if (property != NULL && (CAPPBaseObj::APPID_BTPHONE == appID
            || QFile::exists(QString::fromStdString(property->filePath)))) {
        lib = loadLib(property->libPath);
    }

    if (lib != NULL) {
        createAPPControllerObj = lib->getCreateInterface();
    }

    if (createAPPControllerObj != NULL) {
        obj = createAPPControllerObj(appID);
    }

    if (obj != NULL) {
        obj->setAppID(appID);
        obj->setAPPName(property->name.c_str());
        obj->setExeFile(property->filePath.c_str());
        obj->setPriority(property->priority);
        obj->setAudioApp(property->isAudio);
        obj->setVideoApp(property->isVideo);
        obj->setNeedResume(property->needResume);
        obj->setShareProcess(property->shareProcess);
        obj->setAudioState(property->audioState);

        m_objList.push_back(obj);
    }

    return obj;
}

bool CObjFactory::destroyObj(CAPPControllerObj *obj)
{
    LibObj *lib = NULL;
    RELEASEOBJ releaseAPPControllerObj = NULL;
    ObjProperty* property = NULL;
    CAPPControllerObj* findObj = NULL;
    bool ret =false;

    for(std::list<CAPPControllerObj*>::iterator iter = m_objList.begin();
            iter != m_objList.end();
            iter++) {
        if((*iter) == obj) {
            findObj = obj;
            m_objList.erase(iter);
            break;
        }
    }

    if (findObj != NULL) {
        property = findProperty(findObj->getAppID());
    }

    if (property != NULL) {
        lib = loadLib(property->libPath);
    }

    if (lib != NULL) {
        releaseAPPControllerObj = lib->getReleaseInterface();
    }

    if (releaseAPPControllerObj != NULL) {
        ret = releaseAPPControllerObj(findObj);
    }

    return ret;
}

CObjFactory::ObjProperty *CObjFactory::findProperty(unsigned char appID) const
{
    LOGD(TAG, "CObjFactory::findProperty appID:%d\n", appID);
    ObjProperty* property = NULL;

    for(std::list<ObjProperty*>::const_iterator iter = m_propertyList.begin();
            iter != m_propertyList.end();
            iter++) {
        if((*iter)->appID == appID) {
            property = (*iter);
            break;
        }
    }

    if (property != NULL) {
        LOGD(TAG, "appId:%d, priority:%d, isAudio:%d, isVideo:%d\n", property->appID, property->priority, property->isAudio,
            property->isVideo);
        LOGD(TAG ,"needResume:%d, audioState:%d, filePath:%s, libPath:%s\n", property->needResume, property->audioState,
            property->filePath.c_str(), property->libPath.c_str());
    }
    return property;
}

LibObj *CObjFactory::loadLib(std::string &libPath)
{
    LibObj *lib = NULL;
    bool ret = false;

    LOGD(TAG, "loadLib libPath:%s\n", libPath.c_str());

    for(std::list<LibObj *>::const_iterator iter = m_libList.begin();
            iter != m_libList.end();
            iter++) {
        if((*iter)->isLoaded(libPath)) {
            lib = (*iter);
            break;
        }
    }

    if (lib == NULL) {
        lib = new LibObj();
        if (NULL == lib) {
            LOGE(TAG, "new LibObj fail\n");
        } else {
            ret = lib->load(libPath);
            if (ret) {
                m_libList.push_back(lib);
            } else {
                SAFE_DELETE(lib);
                lib = NULL;
            }
        }
    }

    return lib;
}

const APPConfig *CObjFactory::getConf()
{
    return m_config;
}

static const char *TAG_ATTR_APP = "APP";
static const char *TAG_APPOBJ = "APPObj";
static const char *TAG_ATTR_APPID = "APPID";
static const char *TAG_ATTR_NAME = "name";
static const char *TAG_ATTR_FILEPATH = "filePath";
static const char *TAG_ATTR_APPOBJLIB = "APPObjlib";
static const char *TAG_ATTR_PRIORITY = "Priority";
static const char *TAG_ATTR_ISAUDIO = "isAudio";
static const char *TAG_ATTR_ISVIDEO = "isVideo";
static const char *TAG_ATTR_NEEDRESUMEAUDIO = "NeedResumeAudio";
static const char *TAG_ATTR_SHAREPORCESS = "ShareProcess";
static const char *TAG_ATTR_CURRENTAUDIO = "CurrentAudio";

static const char TAG_CONF[] = "APPCONFIG";
static const char TAG_CONF_MINMEMKB[] = "MinMemKB";
static const char TAG_CONF_MINMEMPCT[] = "MinMemPCT";
static const char TAG_CONF_MAXAPP[] = "MaxAPPCount";

bool CObjFactory::parserXml(const std::string &file,
                                std::list<ObjProperty *> &proList)
{
    LOGD(TAG, "CObjFactory::parserXml enter\n");
    tinyxml2::XMLDocument doc;
    if (doc.LoadFile(file.c_str()) != tinyxml2::XML_SUCCESS) {
        LOGE(TAG, "open file fail! (%s)\r\n", file.c_str());
        return false;
    }

    tinyxml2::XMLNode* cur = doc.FirstChildElement(TAG_ATTR_APP);
    if (cur == NULL) {
        LOGE(TAG, "empty document!\r\n");
        return false;
    }

    for (tinyxml2::XMLNode* child = cur->FirstChild(); child; child = child->NextSibling()) {
        tinyxml2::XMLElement* element = child->ToElement();
        if (element && strcmp(element->Name(), TAG_APPOBJ) == 0) {
            const char *appIDStr = element->Attribute(TAG_ATTR_APPID);
            const char *appNameStr = element->Attribute(TAG_ATTR_NAME);
            const char *appFilePathStr = element->Attribute(TAG_ATTR_FILEPATH);
            const char *appAppObjLibStr = element->Attribute(TAG_ATTR_APPOBJLIB);
            const char *appPriorityStr = element->Attribute(TAG_ATTR_PRIORITY);
            const char *appIsAudioStr = element->Attribute(TAG_ATTR_ISAUDIO);
            const char *appIsVideoStr = element->Attribute(TAG_ATTR_ISVIDEO);
            const char *appNeedResume = element->Attribute(TAG_ATTR_NEEDRESUMEAUDIO);
            const char *appShareProcess = element->Attribute(TAG_ATTR_SHAREPORCESS);
            const char *appCurrentAudio = element->Attribute(TAG_ATTR_CURRENTAUDIO);

            ObjProperty *obj = new ObjProperty;
            if (obj != NULL) {
                obj->appID = StringUtils::stringToInt(appIDStr);
                obj->priority = StringUtils::stringToInt(appPriorityStr);

                if (appNameStr != NULL) {
                    obj->name = appNameStr;
                }

                if (appFilePathStr != NULL) {
                    obj->filePath = appFilePathStr;
                }

                if (appAppObjLibStr != NULL) {
                    obj->libPath = appAppObjLibStr;
                }

                if (appIsAudioStr != NULL) {
                    obj->isAudio = ((StringUtils::stringToInt(appIsAudioStr) >= 1) ? true : false);
                }

                if (appIsVideoStr != NULL) {
                    obj->isVideo = ((StringUtils::stringToInt(appIsVideoStr) >= 1) ? true : false);
                }

                if (appNeedResume != NULL) {
                    obj->needResume = ((StringUtils::stringToInt(appNeedResume) >= 1) ? true : false);
                }

                if (appShareProcess != NULL) {
                    obj->shareProcess = appShareProcess;
                }

                if (appCurrentAudio != NULL) {
                    if (0 == strcmp(appCurrentAudio, "LEVEL_TRANSIENT"))
                        obj->audioState = CAPPBaseObj::LEVEL_TRANSIENT;
                    else if (0 == strcmp(appCurrentAudio, "LEVEL_TRANSIENT_CAN_DUCK"))
                        obj->audioState = CAPPBaseObj::LEVEL_TRANSIENT_CAN_DUCK;
                }

                proList.push_back(obj);
            } else {
                LOGE(TAG, "out of mem!\r\n");
                return false;
            }
        } else if (NULL == m_config && element && strcmp(element->Name(), TAG_CONF)) {
            m_config = new APPConfig;
            if (NULL == m_config) {
                LOGE(TAG, "new APPConfig fail\n");
                return false;
            }

            const char *minMemKB = element->Attribute(TAG_CONF_MINMEMKB);
            if (NULL != minMemKB) {
                m_config->m_minFreeRamKB = StringUtils::stringToInt(minMemKB);
            }

            const char *minMemPCT = element->Attribute(TAG_CONF_MINMEMPCT);
            if (NULL != minMemPCT) {
                m_config->m_minFreeRamPCT = StringUtils::stringToInt(minMemPCT);
                if (m_config->m_minFreeRamPCT > 50) {
                    LOGI(TAG, "%s? maybe wrong?\n", minMemPCT);
                }
            }

            const char *maxAPP = element->Attribute(TAG_CONF_MAXAPP);
            if (NULL != maxAPP) {
                m_config->m_maxAPPCount= StringUtils::stringToInt(maxAPP);
            }
        }
    }

    LOGD(TAG, "CObjFactory::parserXml leave\n");
    return true;
}

