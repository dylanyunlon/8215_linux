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

#include "csubwindowhome.h"
#include "cglobaldata.h"
#include "utils.h"
#include <QDir>
#include <QChar>
#include <QQmlProperty>
#include <QQmlComponent>
#include <QQmlEngine>
#include <sstream>
#include <QTimer>
#include <QDateTime>
#include <QFile>
#include <QIODevice>

#include <csync.h>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <dlfcn.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <sys/resource.h>

#include "volumeOverlay.h"
#include "cotpoweroff.h"
//#include "../libfsinfo/configinfo.h"
//#include "bootproflog.h"

#define CONFIGTIME24HFMT    "24HFormat"
#define CONFIGLANGUAGE      "languageType"

#define DECIMAL_CODE                 10
#define NOON_TIME_BASE               12
#define MAX_WEEK_DAYS                7
#define STRING_TEMP_LOW_TEMPERATURE  17
#define STRING_TEMP_HIGH_TEMPERATURE 25

#define STRING_CITY_NAME_EN     "Shenzhen"
#define STRING_CITY_NAME_CHN    "深圳"
#define STRING_WEATHER_EN       "Cloudy"
#define STRING_WEATHER_CHN      "多云"
#define STRING_TEMPERATURE_EN   "Temp"
#define STRING_TEMPERATURE_CHN  "温度"
#define STRING_RANGE_TO         "~"
#define STRING_COLON_EN         ":"
#define STRING_COLON_CHN        "："
#define STRING_CELSIUS          "℃"

#define STRING_AM               "AM"
#define STRING_PM               "PM"
#define STRING_NULL             ""

#define STRING_ROOT             "/"
#define STRING_PAGE             "Page"
#define STRING_START            "Start"
#define STRING_COUNT            "Count"
#define STRING_APP_ITEM         "AppItem"
#define STRING_SUBWNDHOME       "csubwndhome"
#define STRING_SCROLL_PAGES     "ScrollPages"
#define STRING_VOLUME_OVERLAY   "volumeOverlay"

#define STRING_TOP_PAGE_NAME    "topPageName"
#define STRING_FIRST_PAGE_NAME  "firstPageName"
#define STRING_FIXED_PAGE_NAME  "fixedPageName"
#define STRING_FOLLOWING_PAGE_NAME  "followingPageName"
#define STRING_APITEM_PAGE_NAME "appItemPageName"

#define MAX_LAN_NUM 3

#define STRING_IMAGE_BT_STATE   "imageBtState"
#define STRING_IMAGE_IPOD_STATE "imageIpodState"
#define STRING_IMAGE_WIFI_STATE "imageWifiState"

#include <apputils.h>

QString static weekDays[MAX_LAN_NUM][MAX_WEEK_DAYS] = {
    {"MON", "TUE", "WED", "THU", "FRI", "SAT", "SUN"},
    {"星期一", "星期二", "星期三", "星期四", "星期五", "星期六", "星期天"},
    {"星期一", "星期二", "星期三", "星期四", "星期五", "星期六", "星期天"},
};

CSubWindowHome::CSubWindowHome()
: CQObjListener(CAPPBaseObj::APPID_HOME)
    ,m_appItem(NULL)
    ,m_rootContext(NULL)
    ,m_rootObject(NULL)
    ,m_time(QTime::currentTime())
    ,m_date(QDate::currentDate())
    ,m_language(GlobalBus::ENGLISH)
    ,m_timeFormat(GlobalBus::CLOCK_12)
    ,m_mainWindow(NULL)
    ,m_volumeOverlay(NULL)
    , m_otPowerOff(NULL)
    , m_otPowerOffListner(NULL)
    , m_arm2InBackcar(true)
    , m_trigerCond(0)
{
#ifdef WAYLAND_WM
    mNeedHideFrontWindow = false;
#endif
    m_otPowerOff = new COtPowerOff("home");
    m_otPowerOffListner = new universal_utils::CFuncListener(this, static_cast<universal_utils::LISTENER_FUNC_WITH_PARA>(&CSubWindowHome::otPowerOffListenerFunc));
    if (m_otPowerOff != NULL)
    {
        LOGD(TAG_HOME,  "New m_otPowerOff sccess..\n");
        m_otPowerOff->setListner(m_otPowerOffListner);
    } else {
        LOGE(TAG_HOME,  "New m_otPowerOff fail..\n");
    }
    m_trigerCond = m_conditionLock.newCondition();
}

CSubWindowHome::~CSubWindowHome()
{
    // QObject::disconnect(&CFileSysWatcher::getInstance(), SIGNAL(sigdevChange(int, int)),
    //         this, SLOT(onStateChange(int, int)));
    SAFE_DELETE(m_otPowerOff);
    SAFE_DELETE(m_otPowerOffListner);
    m_conditionLock.releaseCondition(m_trigerCond);
}

//--------------------------------------------------------------------------------
//  @Function Name : CSubWindowHome::onTimeformatChange
//
//  @Param :
//        - none
//
//  @Return : BOOL
//
//  @Description : process when time format change
//
//---------------------------------------------------------------------------------
bool CSubWindowHome::onTimeformatChange()
{
    return true;
}

//--------------------------------------------------------------------------------
//  @Function Name : CSubWindowHome::onKeyEvent
//
//  @Param :
//        - int key
//        - int param1
//        - int param2
//
//  @Return : BOOL
//
//  @Description : receive key event from appmanager
//
//---------------------------------------------------------------------------------
bool CSubWindowHome::doKeyEvent (int key, int param1, int param2)
{
    bool ret = false;
    unsigned char mainfunc = getMainFunc(key);
    unsigned char subfunc = getSubFunc(key);

    LOGD(TAG_HOME, "%s: key = %d, param1 =%d, param2 = %d",
        __func__, key, param1, param2);

    switch (mainfunc) {
    case CCmdTask::MAIN_FUNC_APP_ACTION:
        switch (subfunc) {
        case GlobalBus::ACTION_STATUS_CHANGED: {
            GlobalBus::E_STATE_TYPE stateType = (GlobalBus::E_STATE_TYPE) param1;
            int state = GlobalBus::getState(stateType);

            switch (stateType) {
            case GlobalBus::STATE_BT: {
                setBtState(state);
                ret = true;
                break;
            }
            case GlobalBus::STATE_CLOCK: {
                setTimeFormat(state);
                ret = true;
                break;
            }
            case GlobalBus::STATE_LANGUAGE: {
                m_language = (GlobalBus::E_LANGUAGE_STATE)(state);
                setLanguage(state);
                updateWeather();
                ret = true;
                break;
            }
            case GlobalBus::STATE_WIFI: {
                LOGD(TAG_HOME, "GlobalBus::E_WIFI_STATE state = %d", state);
                setWifiState(state);
                ret = true;
                break;
            }

            case GlobalBus::STATE_IPOD : {
                LOGD(TAG_HOME, "GlobalBus::STATE_IPOD state = %d", state);
                setIpodState(state);
                ret = true;
                break;
            }

            default:
                break;
            }
            break;
        }

        case GlobalBus::ACTION_LAST_APP_DONE: {
            LOGI(TAG_HOME, "GlobalBus::ACTION_LAST_APP_DONE\n");
            if (m_otPowerOff != NULL) {
                m_otPowerOff->powerOff();
            }
            ret = true;
            break;
        }

        case GlobalBus::ACTION_ARM2_BACKCAR_OUT: {
            LOGI(TAG_HOME, "GlobalBus::ACTION_ARM2_BACKCAR_OUT\n");
            m_arm2InBackcar = false;
            ret = true;
            break;
        }

        default:
            break;
        }
        break;

    case CCmdTask::MAIN_FUNC_KEY: {
        m_conditionLock.lock();
        if (m_volumeOverlay != NULL) {
            ret = m_volumeOverlay->doKeyEvent(subfunc, param1, param2);
        }
        m_conditionLock.unlock();
    }

    default:
        break;
    }

    return ret;
}

bool CSubWindowHome::setWifiState(int wifiState)
{
    switch (wifiState) {
    case GlobalBus::WIFI_SIGLV_0:
        emit sendMsgToQml(ID_WIFI_STATE, STATE_WIFI_LEVEL_0, 0);
        break;
    case GlobalBus::WIFI_SIGLV_1:
        emit sendMsgToQml(ID_WIFI_STATE, STATE_WIFI_LEVEL_1, 0);
        break;
    case GlobalBus::WIFI_SIGLV_2:
        emit sendMsgToQml(ID_WIFI_STATE, STATE_WIFI_LEVEL_2, 0);
        break;
    case GlobalBus::WIFI_SIGLV_3:
        emit sendMsgToQml(ID_WIFI_STATE, STATE_WIFI_LEVEL_3, 0);
        break;
    case GlobalBus::WIFI_SIGLV_4:
        emit sendMsgToQml(ID_WIFI_STATE, STATE_WIFI_LEVEL_4, 0);
        break;
    case GlobalBus::WIFI_DISABLE:
    case GlobalBus::WIFI_ENABLE:
        emit sendMsgToQml(ID_WIFI_STATE, STATE_WIFI_LEVEL_0, 0);
    default:
        break;
    }
    return true;
}

bool CSubWindowHome::setBtState(int btState)
{
    switch (btState) {
    case GlobalBus::BT_CONNECT:
        emit sendMsgToQml(ID_BT_STATE, STATE_BT_ON, 0);
        break;
    case GlobalBus::BT_DISCONNECT:
        emit sendMsgToQml(ID_BT_STATE, STATE_BT_OFF, 0);
        break;
    default:
        break;
    }

    return true;
}

bool CSubWindowHome::setIpodState(int ipodState)
{
    switch (ipodState) {
    case GlobalBus::IPOD_CONNECT:
        emit sendMsgToQml(ID_IPOD_STATE, STATE_IPOD_EXIST, 0);
        break;
    case GlobalBus::IPOD_DISCONNECT:
        emit sendMsgToQml(ID_IPOD_STATE, STATE_IPOD_NONE, 0);
        break;
    default:
        break;
    }

    return true;
}

bool CSubWindowHome::setTimeFormat(int timeFormat)
{
    switch (timeFormat) {
    case GlobalBus::CLOCK_12:
        emit sendMsgToQml(ID_TIMEFORMAT, TIME_FORMAT_12, 0);
        break;
    case GlobalBus::CLOCK_24:
        emit sendMsgToQml(ID_TIMEFORMAT, TIME_FORMAT_24, 0);
        break;
    default:
        break;
    }
    return true;
}

bool CSubWindowHome::updateWeather()
{
    emit sendMsgToQml(ID_WEATHER, WEATHER_CITY, getCurCity());
    LOGI(TAG_HOME, "CSubWindowHome::emit sendMsgToQml(ID_WEATHER, WEATHER_CITY, getCurCity()\r\n");

    emit sendMsgToQml(ID_WEATHER, WEATHER_WEATHER, getCurWeather());
    LOGI(TAG_HOME, "CSubWindowHome::emit sendMsgToQml(ID_WEATHER, WEATHER_WEATHER, getCurWeather()\r\n");

    emit sendMsgToQml(ID_WEATHER, WEATHER_TEMP, getCurTemperature());
    LOGI(TAG_HOME, "CSubWindowHome::emit sendMsgToQml(ID_WEATHER, WEATHER_TEMP, getCurTemperature())\r\n");

    return true;
}

bool CSubWindowHome::setLanguage(int language)
{
    switch (language) {
        case GlobalBus::ENGLISH:
            emit sendMsgToQml(ID_LANGUAGE, LANGUAGE_ENGLISH, 0);
            break;
        case GlobalBus::CHINESE:
            emit sendMsgToQml(ID_LANGUAGE, LANGUAGE_CHINESS, 0);
            break;
        case GlobalBus::CHINESE_TW:
            emit sendMsgToQml(ID_LANGUAGE, LANGUAGE_CHINESS_TW, 0);
            break;
        default:
            emit sendMsgToQml(ID_LANGUAGE, LANGUAGE_ENGLISH, 0);
            break;
        }

   return true;
}



//--------------------------------------------------------------------------------
//  @Function Name : CSubWindowHome::onAppItemClicked
//
//  @Param :
//        - int appItemId
//
//  @Return : void
//
//  @Description : process when appItem clicked
//
//---------------------------------------------------------------------------------
void CSubWindowHome::onAppItemClicked(int appItemId)
{
    LOGD(TAG_HOME,"CSubWindowHome::onAppItemClicked -> appItemId = %d\n", appItemId);

    /*if (m_arm2InBackcar) {
        LOGD(TAG_HOME,"CSubWindowHome::onAppItemClicked, arm1 backcar not ready...\n ");
        return;
    }
    else if (isMonkeySingleAppTesting()) {
        LOGD(TAG_HOME,"CSubWindowHome::onAppItemClicked Monkey is run single test mode.\n");
    }
    else if (isMonkeyRunning()) {
        LOGD(TAG_HOME,"CSubWindowHome::onAppItemClicked Monkey is running.\n");
        if ((15 == appItemId)
           || (14 == appItemId)) {
            LOGD(TAG_HOME,"CSubWindowHome::onAppItemClicked Forbidden to run this application\n");
            return;
        }
    } else {
        LOGD(TAG_HOME,"CSubWindowHome::onAppItemClicked Monkey is not run.\n");
    }*/

    GlobalBus::applyFor(GlobalBus::ACTION_RUN, (unsigned char)(appItemId), 0);
}

void CSubWindowHome::createFile(const QString filePath, const QString fileName) {
    QDir tempDir;
    QString currentDir = tempDir.currentPath();
    if(!tempDir.exists(filePath)) {
        tempDir.mkpath(filePath);
    }
    QFile *tempFile = new QFile;
    tempDir.setCurrent(filePath);
    if(tempFile->exists(fileName)) {
        //qDebug()<<"file "<<filePath<<fileName<<" exists"<<endl;
    } else {
        tempFile->setFileName(fileName);
        if(!tempFile->open(QIODevice::WriteOnly|QIODevice::Text)) {
            //qDebug()<<"create "<<filePath<<fileName<<" fail"<<endl;
            return;
        }
        tempFile->close();
        //qDebug()<<"create "<<filePath<<fileName<<" ok"<<endl;
    }
}

void CSubWindowHome::topPageLoaded()
{
    LOGI(TAG_HOME, "topPageLoaded from CSubWindowHome::topPageLoaded()\n");
    // CFileSysWatcher::getInstance().initDevState();
    // QObject::connect(&CFileSysWatcher::getInstance(), SIGNAL(sigdevChange(int, int)),
    //         this, SLOT(onStateChange(int, int)));

    // CFileSysWatcher::getInstance().manualQueryMount();

    /*
    QString filePath("/tmp/");
    QString fileName("homeready");
    createFile(filePath, fileName);
    */
}

void CSubWindowHome::scrollPageLoaded()
{
    GlobalBus::E_LANGUAGE_STATE state = (GlobalBus::E_LANGUAGE_STATE)GlobalBus::getState(GlobalBus::STATE_LANGUAGE);
    m_language = (GlobalBus::E_LANGUAGE_STATE)(state);
    setLanguage(state);
    LOGI(TAG_HOME, "CSubWindowHome::emit sendMsgToQml(ID_LANGUAGE, %d, 0)\r\n", state);

    GlobalBus::E_CLOCK_STATE timefmt = (GlobalBus::E_CLOCK_STATE)GlobalBus::getState(GlobalBus::STATE_CLOCK);
    LOGI(TAG_HOME, "CSubWindowHome::SYH timefmt = %d, 0)\r\n", timefmt);
    setTimeFormat(timefmt);
    LOGI(TAG_HOME, "CSubWindowHome::emit sendMsgToQml(ID_TIMEFORMAT, %d, 0)\r\n", timefmt);

    updateWeather();

    LOGI(TAG_HOME, "scrollPageLoaded from CSubWindowHome::scrollPageLoaded()\n");
}

//--------------------------------------------------------------------------------
//  @Function Name : CSubWindowHome::timezoneClicked
//
//  @Param : None
//
//  @Return : void
//
//  @Description : process when time zone clicked
//
//---------------------------------------------------------------------------------
void CSubWindowHome::timezoneClicked()
{
    LOGD(TAG_HOME, "timezoneClicked \n");
    //if (!m_arm2InBackcar) {
        GlobalBus::jumpTo(CAPPBaseObj::APPID_HOME, CAPPBaseObj::APPID_SETTING, CAPPBaseObj::PAGE4);
    //} else {
        //LOGD(TAG_HOME, "timezoneClicked arm1 backcar not ready\n");
    //}
}

//--------------------------------------------------------------------------------
//  @Function Name : CSubWindowHome::weatherzoneClicked
//
//  @Param : None
//
//  @Return : void
//
//  @Description : process when time zone clicked
//
//---------------------------------------------------------------------------------
void CSubWindowHome::weatherzoneClicked(int languageIndex)
{
    LOGD(TAG_HOME, "this func is for test only, languageIndex = %d", languageIndex);
}


//--------------------------------------------------------------------------------
//  @Function Name : CSubWindowHome::getCurTime
//
//  @Param : - None
//
//  @Return : void
//
//  @Description :get current time
//
//---------------------------------------------------------------------------------
void CSubWindowHome::getCurTime()
{
    int hour1=0;
    int hour2=0;
    int min1=0;
    int min2=0;
    QString ampm = STRING_NULL;
    m_time = QTime::currentTime();
    int tempHour = m_time.hour();
    int tempMin = m_time.minute();

    if (m_timeFormat == GlobalBus::CLOCK_12) {
        if(tempHour > NOON_TIME_BASE)
        {
            tempHour -= NOON_TIME_BASE;
            ampm = STRING_PM;
        } else {
            ampm = STRING_AM;
        }
    } else {
        ampm = STRING_NULL;
    }
    if (DECIMAL_CODE == 0) {
        LOGI(TAG_HOME, "Time format error!\r\n");
    } else {
        hour1 = tempHour / DECIMAL_CODE;
        hour2 = tempHour % DECIMAL_CODE;
        min1 = tempMin / DECIMAL_CODE;
        min2 = tempMin % DECIMAL_CODE;
        emit setUITime(hour1,hour2,min1,min2,ampm);
    }
}

//--------------------------------------------------------------------------------
//  @Function Name : CSubWindowHome::getCurDay
//
//  @Param : - None
//
//  @Return : QString
//
//  @Description : Get current day
//
//---------------------------------------------------------------------------------
QString CSubWindowHome::getCurDay(int curWeekDay)
{
    if(curWeekDay <= 0 || curWeekDay > MAX_WEEK_DAYS) {
        return "";
    } else {
        return weekDays[m_language - 1][curWeekDay - 1];
    }
}


//--------------------------------------------------------------------------------
//  @Function Name : CSubWindowHome::getCurCity
//
//  @Param : - None
//
//  @Return : QString
//
//  @Description : Get current city
//
//---------------------------------------------------------------------------------
QString CSubWindowHome::getCurCity()
{
    QString text ;
    switch(m_language){
    case GlobalBus::ENGLISH:
        {
        text.append(STRING_CITY_NAME_EN);
        }
        break;
    case GlobalBus::CHINESE:
        {
        text.append(STRING_CITY_NAME_CHN);
        }
        break;
    case GlobalBus::CHINESE_TW:
        {
        text.append(STRING_CITY_NAME_CHN);
        }
        break;
    default:
        break;
    }

    return text;
}

//--------------------------------------------------------------------------------
//  @Function Name : CSubWindowHome::getCurWeather
//
//  @Param : - None
//
//  @Return : QString
//
//  @Description : Get current weather
//
//---------------------------------------------------------------------------------
QString CSubWindowHome::getCurWeather()
{
    QString text ;
    switch(m_language){
    case GlobalBus::ENGLISH:
        {
        text.append(STRING_WEATHER_EN);
        }
        break;
    case GlobalBus::CHINESE:
        {
        text.append(STRING_WEATHER_CHN);
        }
        break;
    case GlobalBus::CHINESE_TW:
        {
        text.append(STRING_WEATHER_CHN);
        }
        break;
    default:
        break;
    }

    return text;
}


//--------------------------------------------------------------------------------
//  @Function Name : CSubWindowHome::getCurTemperature
//
//  @Param : - None
//
//  @Return : QString
//
//  @Description : Get current temperature
//
//---------------------------------------------------------------------------------
QString CSubWindowHome::getCurTemperature()
{
    QString text ;
    int tempLow = STRING_TEMP_LOW_TEMPERATURE;
    int tempHigh = STRING_TEMP_HIGH_TEMPERATURE;
    switch(m_language){
    case GlobalBus::ENGLISH:
        {
        text.append(STRING_TEMPERATURE_EN);
        text.append(STRING_COLON_EN);
        text.append(QString::number(tempLow));
        text.append(STRING_RANGE_TO);
        text.append(QString::number(tempHigh));
        text.append(STRING_CELSIUS);
        }
        break;
    case GlobalBus::CHINESE:
    case GlobalBus::CHINESE_TW:
        {
        text.append(QString::number(tempLow));
        text.append(STRING_CELSIUS);
        text.append(STRING_RANGE_TO);
        text.append(QString::number(tempHigh));
        text.append(STRING_CELSIUS);
        }
        break;
    default:
        break;
    }

    return text;
}

//--------------------------------------------------------------------------------
//  @Function Name : CSubWindowHome::onStateChange
//
//  @Param :
//        - QString str
//        - int state
//
//  @Return : BOOL
//
//  @Description : process when state change
//                        param1 is the object that changed;
//                        param2 is the state that param1 changed to
//
//---------------------------------------------------------------------------------
bool CSubWindowHome::onStateChange(int devId, int state)
{
    emit sendMsgToQml(devId, state, 0);

    return true;
}

bool CSubWindowHome::initAppItemFromXml(QString filename)
{
    QTime  t1, t2;
    t1 = QTime::currentTime();
    LOGI(TAG_HOME, "CSubWindowHome::initAppItemFromXml Start\r\n");

    CAppItemParser *parser = new CAppItemParser;
    QString fileName = filename;
    if (parser == NULL || fileName.length() == 0) {
        LOGI(TAG_HOME, "parser == NULL\r\n");
        return false;
    }
    bool bRet = parser->parseXML(fileName);
    if(bRet)
    {
        QString pageName = STRING_PAGE;
        QString itemIndexName = STRING_APP_ITEM;
        int pageCount[MAX_PAGE]={0};
        int pageIndex[MAX_PAGE]={0};
        int pageMax = 1;
        int count = parser->membersCount();

        for(int i=0;i<count;i++)
        {
            QString itemName;
            itemName.clear();
            itemName = pageName;
            m_appItem = parser->member(i);

            int page = m_appItem->page();
            if(page >= pageMax)
            {
                pageMax = page+1;
            }
            pageIndex[page] +=1;
            itemName.append(QString::number(page));
            itemName.append(itemIndexName);
            itemName.append(QString::number(pageIndex[page]));
            pageCount[page]++;

            m_appItem->setText(itemName);

            m_appItemList.append(m_appItem);
            m_rootContext->setContextProperty(m_appItem->getText(), m_appItem);
        }

        m_rootContext->setContextProperty(STRING_SCROLL_PAGES, pageMax-2);

       for(int n=0;n<pageMax;n++){
           QString strPageCount = pageName;
           strPageCount.append(QString::number(n));
           strPageCount.append(STRING_COUNT);
           m_rootContext->setContextProperty(strPageCount, pageCount[n]);

           QString strPageStart = pageName;
           strPageStart.append(QString::number(n));
           strPageStart.append(STRING_START);
           int startNum = 0;
           for(int j=0;j<n;j++)
           {
               startNum += pageCount[j];
           }
           m_rootContext->setContextProperty(strPageStart, startNum);
       }
    }
    LOGI(TAG_HOME, "CSubWindowHome::initAppItemFromXml End\r\n");
    t2 = QTime::currentTime();
    LOGI(TAG_HOME, "CSubWindowHome::initAppItemFromXml waste time = %d\n", t1.msecsTo(t2));

    return true;
}

//--------------------------------------------------------------------------------
//  @Function Name : CSubWindowHome::initContext
//
//  @Param :
//        - QQmlContext *rootContext
//        - QString &appDirPath
//
//  @Return : BOOL
//
//  @Description :Init context
//
//---------------------------------------------------------------------------------
bool CSubWindowHome::initContext(QQmlContext *rootContext, QString &appDirPath)
{
    LOGI(TAG_HOME, "CSubWindowHome::initContext\r\n");
    QTime  t1, t2;
    t1 = QTime::currentTime();
    LOGI(TAG_HOME, "CSubWindowHome::initContext Start\r\n");

    if (rootContext == NULL) {
        LOGI(TAG_HOME, "rootContext == NULL\r\n");
        return false;
    }
    m_rootContext = rootContext;

    QString fileName = APPITEM_FILE_NAME;
    QString dirName = appDirPath;

    dirName.append(STRING_ROOT);
    fileName.prepend(dirName);
    initAppItemFromXml(fileName);

    m_rootContext->setContextProperty(STRING_SUBWNDHOME, this);

    LOGI(TAG_HOME, "CSubWindowHome::initContext End\r\n");
    t2 = QTime::currentTime();
    LOGI(TAG_HOME, "CSubWindowHome::initContext waste time = %d\n", t1.msecsTo(t2));

    return true;
}
int CSubWindowHome::stringToInt (const char *str)
{
    int ret = 0;
    if (str == NULL) {
        LOGE(TAG_HOME, "str is NULL.\n");
        return -1;
    }
    std::stringstream ss;
    ss<<str;
    ss>>ret;
    return ret;
}

//--------------------------------------------------------------------------------
//  @Function Name : CSubWindowHome::initObjects
//
//  @Param :
//        - QObject *rootObject
//
//  @Return : BOOL
//
//  @Description : Connect signals and slots
//
//---------------------------------------------------------------------------------
bool CSubWindowHome::initObjects(QObject *rootObject)
{
    LOGI(TAG_HOME, "CSubWindowHome::initObjects\r\n");

    QTime  t1, t2;
    std::string _time,_language;
    int time,language;
    t1 = QTime::currentTime();
    LOGI(TAG_HOME, "CSubWindowHome::initObjects Start\r\n");

    if (!rootObject) {
         LOGI(TAG_HOME, "CSubWindowHome::initObjects rootObject is null\r\n");
        return false;
    }

    //qlnTemp debug m_rootObject = rootObject;
    //qlnTemp debug m_mainWindow = qobject_cast<QQuickWindow *>(m_rootObject);

    QObject::connect(this, SIGNAL(appItemClicked(int)), this, SLOT(onAppItemClicked(int)));
    QObject::connect(this, SIGNAL(stateChange(int,int)), this, SLOT(onStateChange(int, int)));

    //qlnTemp debug
    /*
    m_settingclient =ISetting::getSettingInstance();
    if (m_settingclient != NULL) {
        m_extra = m_settingclient->getSettingAdapter("extra");
    } else {
        LOGE(TAG_HOME, "cann't get extra server. \n");
    }

    m_extra->getValue("24HFormat",_time);
    m_extra->getValue("languageType",_language);

    time = stringToInt(_time.c_str());
    language = stringToInt(_language.c_str());
    */

    // AtcConfignfo configInfo;
    // configInfo.parserCfgXml();
    // language = configInfo.getCfgValue(CONFIGLANGUAGE);
    // time = configInfo.getCfgValue(CONFIGTIME24HFMT);

    // if (0 == time) {
    //     GlobalBus::setState(GlobalBus::STATE_CLOCK, GlobalBus::CLOCK_12);
    // } else if (1 == time) {
    //     GlobalBus::setState(GlobalBus::STATE_CLOCK, GlobalBus::CLOCK_24);
    // }

    // if (0 == language) {
    //     GlobalBus::setState(GlobalBus::STATE_LANGUAGE, GlobalBus::CHINESE);
    // } else if (1 == language) {
    //     GlobalBus::setState(GlobalBus::STATE_LANGUAGE, GlobalBus::CHINESE_TW);
    // } else if ( 2== language) {
    //     GlobalBus::setState(GlobalBus::STATE_LANGUAGE, GlobalBus::ENGLISH);
    // }
    // LOGI(TAG_HOME, "CSubWindowHome::initObjects End\r\n");
    // t2 = QTime::currentTime();
    // LOGI(TAG_HOME, "CSubWindowHome::initObjects waste time = %d\n", t1.msecsTo(t2));

    return true;
}

bool CSubWindowHome::initVolumeWindow(QQuickWindow *volumeWindow)
{
    LOGI(TAG_HOME, "CSubWindowHome::initVolumeWindow start\r\n");
    if (volumeWindow != NULL) {
        volumeWindow->hide();
    }

    if (m_volumeOverlay == NULL) {
        m_volumeOverlay = new CvolumeOverlay(volumeWindow);
    }

    if ((m_volumeOverlay != NULL) && (m_rootContext != NULL)) {
        m_rootContext->setContextProperty(STRING_VOLUME_OVERLAY, m_volumeOverlay);
    }
    threadStart();
    LOGI(TAG_HOME, "CSubWindowHome::initVolumeWindow end\r\n");
    return true;
}

bool CSubWindowHome::otPowerOffListenerFunc(unsigned int msg, unsigned int wParam, unsigned int lParam)
{
    if (msg == E_POWER_OFF_SAVE_STATE) {
    }
    return true;
}

unsigned long CSubWindowHome::threadRun()
{
    CvolumeOverlay* volumeOverlay = NULL;
    LOGD(TAG_HOME, "threadRun enter, thread id, 0x%x\n" , (unsigned int)pthread_self());
    m_conditionLock.lock();
    volumeOverlay = m_volumeOverlay;
    m_conditionLock.unlock();

    if (volumeOverlay != NULL) {
        volumeOverlay->initObject();
    }
    LOGD(TAG_HOME, "threadRun leave\n");
    return 0;
}
