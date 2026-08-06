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

#ifndef CSUBWINDOWHOME_H
#define CSUBWINDOWHOME_H

#include "cappitem.h"
#include "cappitemparser.h"
//#include "cfilesyswatcher.h"

#include <QTime>
#include <QDate>
#include <QQmlContext>

#include "apptype.h"
#include "appobj.h"
#include "ctllistener.h"

#include "globalbus.h"
#include "appobj.h"

#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <QQmlApplicationEngine>
#include <QQuickWindow>
#include "qobjlistener.h"
#include "funclistener.h"
#include "csync.h"
#include "cautothread.h"
#include "cconditionlock.h"
//#include "isettingadapter.h"
//#include "isetting.h"

class CvolumeOverlay;
class COtPowerOff;

class CSubWindowHome
    : public CQObjListener
    , public universal_utils::CListenerObj
    , public universal_utils::CAutoThread
{
    Q_OBJECT
public:
    explicit CSubWindowHome();
    ~CSubWindowHome();
    bool initContext(QQmlContext *rootContext, QString &appDirPath);
    bool initObjects(QObject *rootObject);
    bool doKeyEvent (int key, int param1, int param2);
    bool initVolumeWindow(QQuickWindow *volumeWindow);

private:
    bool initAppItemFromXml(QString filename);
    bool setBtState(int btState);
    bool setIpodState(int ipodState);
    bool setWifiState(int wifiState);
    bool setTimeFormat(int timeFormat);
    bool setLanguage(int language);
    bool updateWeather();
    int stringToInt (const char *str);
    void createFile(const QString filePath, const QString fileName);
signals:
    void appItemClicked(int);
    void hideWindow();
    bool stateChange(int str, int state);
    void timeformatChange(); //test
    void languageChange(QVariant); //test
    void timeout();
    void setDevState(QVariant, QVariant);
    void setUITime(QVariant, QVariant, QVariant, QVariant, QVariant);
    void sendMsgToQml(QVariant msg, QVariant wParam, QVariant lParam);

public slots:
    Q_INVOKABLE void onAppItemClicked(int);
    Q_INVOKABLE void timezoneClicked();
    Q_INVOKABLE void weatherzoneClicked(int);
    Q_INVOKABLE void topPageLoaded();
    Q_INVOKABLE void scrollPageLoaded();
    Q_INVOKABLE void getCurTime();
    Q_INVOKABLE QString getCurDay(int);
    Q_INVOKABLE QString getCurCity();
    Q_INVOKABLE QString getCurWeather();
    Q_INVOKABLE QString getCurTemperature();
    Q_INVOKABLE bool onStateChange(int devId, int state);
    Q_INVOKABLE bool onTimeformatChange();//test

protected:
    unsigned long threadRun();

private:
    bool otPowerOffListenerFunc(unsigned int msg, unsigned int wParam, unsigned int lParam);

private:
    QList<CAppItem*> m_appItemList;
    CAppItem *m_appItem;
    QQmlContext *m_rootContext;
    QObject *m_rootObject;
    QTime m_time;
    QDate m_date;
    GlobalBus::E_LANGUAGE_STATE m_language;
    GlobalBus::E_CLOCK_STATE m_timeFormat; //gClockState
    QQuickWindow *m_mainWindow;
    CvolumeOverlay* m_volumeOverlay;
    COtPowerOff* m_otPowerOff;
    universal_utils::CFuncListener *m_otPowerOffListner;
    bool m_arm2InBackcar;
    universal_utils::CConditionLock m_conditionLock;
    //ISetting *m_settingclient = NULL;
    //ISettingAdapter *m_extra = NULL;
    int m_trigerCond;
};

#endif // CSUBWINDOWHOME_H
