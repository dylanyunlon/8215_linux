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

#ifndef CGLOBALDATA
#define CGLOBALDATA

#include <QString>
#include "applog.h"

#define WEEK_DAYS 7
#define MAX_PAGE 5
#define TAG_HOME "Home_APP"

#ifdef WAYLAND_WM
#define APPITEM_FILE_NAME "./waylandAppListItem.xml"
#else
#define APPITEM_FILE_NAME "appListItem.xml"
#endif

#define ID_SDCARD_STATE         1
#define ID_UDISK_STATE          2
#define ID_BT_STATE             3
#define ID_WIFI_STATE           4
#define ID_IPOD_STATE           5

#define ID_LANGUAGE             11
#define ID_TIMEFORMAT           12
#define ID_WEATHER              13


enum {
    STATE_BT_OFF,
    STATE_BT_ON,
};

enum {
    TIME_FORMAT_12,
    TIME_FORMAT_24,
};

enum {
    WEATHER_CITY,
    WEATHER_WEATHER,
    WEATHER_TEMP,
};

enum {
    LANGUAGE_ENGLISH,
    LANGUAGE_CHINESS,
    LANGUAGE_CHINESS_TW,
};

enum {
    STATE_SDCARD_NONE,
    STATE_SDCARD_EXIST,
};

enum {
    STATE_UDISK_NONE,
    STATE_UDISK_EXIST,
};


enum {
    STATE_IPOD_NONE,
    STATE_IPOD_EXIST,
};

enum {
    STATE_WIFI_LEVEL_0,
    STATE_WIFI_LEVEL_1,
    STATE_WIFI_LEVEL_2,
    STATE_WIFI_LEVEL_3,
    STATE_WIFI_LEVEL_4,
};


#endif // CGLOBALDATA

