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

#ifndef CCMDTASK_H
#define CCMDTASK_H

#include "ctask.h"
#include "stddef.h"
#include "crawstring.h"

#define APPMANAGER_LISTENER_SOCKET_ADDR  "/tmp/appManagerListenerSocketAddr"

#pragma pack(1)
class CMDPacket
{
public:
    CMDPacket()
        : m_cmdSync(0)
        , m_mainFunc(0)
        , m_subFunc(0)
        , m_appID(0)
        , m_data(0)
        , m_retType(0)
    {}
    CMDPacket(unsigned long cmdSync,
                unsigned char mainFunc,
                unsigned char subFunc,
                unsigned char appID,
                unsigned int data,
                unsigned char retType)
        : m_cmdSync(cmdSync)
        , m_mainFunc(mainFunc)
        , m_subFunc(subFunc)
        , m_appID(appID)
        , m_data(data)
        , m_retType(retType)
    {}

    unsigned long m_cmdSync;
    unsigned char m_mainFunc;
    unsigned char m_subFunc;
    unsigned char m_appID;
    unsigned int m_data;
    // union {
    //     unsigned short m_data;
    //     struct {
    //         unsigned char m_param2 : 8;
    //         unsigned char m_param1 : 8;
    //     };
    // };
    unsigned char m_retType;
};
#pragma pack()

class CCmdTask : public CTask
{
public:
    typedef enum
    {
        MAIN_FUNC_ACK = 0,
        MAIN_FUNC_CDDVD = 1,
        MAIN_FUNC_USB,
        MAIN_FUNC_SD,
        MAIN_FUNC_FM,
        MAIN_FUNC_AM,
        MAIN_FUNC_CMMB,
        MAIN_FUNC_AVIN,
        MAIN_FUNC_CDC,
        MAIN_FUNC_IPOD,
        MAIN_FUNC_BT,            //10
        MAIN_FUNC_NAVI_FUNC,    //地图功能，需地图支持
        MAIN_FUNC_STEERING_WHEEL,
        MAIN_FUNC_BACKCAR,
        MAIN_FUNC_TIRE_PRESSURE,
        MAIN_FUNC_MAIN_VOLUME,
        MAIN_FUNC_SOUND_EFFECT,
        MAIN_FUNC_DISPLAY,
        MAIN_FUNC_FACTORY,
        MAIN_FUNC_PUSH_TO_TALK, //一键通
        MAIN_FUNC_SPEECH_RECOGNITION,    //20
        MAIN_FUNC_RESET,
        MAIN_FUNC_CALIBRATE,
        MAIN_FUNC_GPS,          //进入导航
        MAIN_FUNC_STEERING_WHEEL_STUDY,  //进入方向盘学习
        MAIN_FUNC_BACKCAR_LOCUS,
        MAIN_FUNC_KEY,
        MAIN_FUNC_SYNC_ALL_INFO,
        MAIN_FUNC_APP_WATCHDOG,
        MAIN_FUNC_SYNC_VERSION,
        MAIN_FUNC_SYNC_OPEN_STATE,  //30
        MAIN_FUNC_SYNC_FIRST_HANDSHAKE,
        MAIN_FUNC_APP_ACTION = 100,
        MAIN_FUNC_APP_JUMP,
    } E_MAINFUNC;

    typedef enum
    {
        RET_IGNORE = 0,
        RET_ACK,
        RET_SUCCESS,
        RET_DATA_DEC,
        RET_DATA_HEC,
        RET_DATA_CHAR,
        RET_DATA_WCHAR,
        RET_TEST_ERROR,
        RET_OTHER_ERROR,
    } E_RET_DATA_TYPE;

    typedef enum
    {
        SUBFUNC_DVP_VIDEO = 1,
        SUBFUNC_DVP_AUDIO,
        SUBFUNC_DVP_IMAGE,
    } E_SUBFUNC_DVP;

    typedef enum
    {
        SUBFUNC_USB_VIDEO = 1,
        SUBFUNC_USB_AUDIO,
        SUBFUNC_USB_IMAGE,
    } E_SUBFUNC_USB;

    typedef enum
    {
        SUBFUNC_SD_VIDEO = 1,
        SUBFUNC_SD_AUDIO,
        SUBFUNC_SD_IMAGE,
    } E_SUBFUNC_SD;

    typedef enum
    {
        SUBFUNC_CMMB_SEARCH = 1,
        SUBFUNC_CMMB_PLAY,
    } E_SUBFUNC_CMMB;

    //subfunction AVIN
    typedef enum
    {
    //  SUBFUNC_AVIN_CHANNEL0,
        SUBFUNC_AVIN_INVALID = 0,
        SUBFUNC_AVIN_CHANNEL1,
        SUBFUNC_AVIN_CHANNEL2,
        SUBFUNC_AVIN_CHANNEL3,
        SUBFUNC_AVIN_CHANNEL4,
        SUBFUNC_AVIN_CHANNEL5,
        SUBFUNC_AVIN_DGI,
    //  SUBFUNC_AVIN_MHL,
    //  SUBFUNC_AVIN_YPBPR,
    //  SUBFUNC_AVIN_VGA,
        SUBFUNC_AVIN_VGA_YPBPR,
    } E_SUBFUNC_AVIN;

    typedef enum
    {
        SUBFUNC_CDC_AUDIO = 2,
    } E_SUBFUNC_CDC;

    typedef enum
    {
        SUBFUNC_IPOD_VIDEO = 1,
        SUBFUNC_IPOD_AUDIO,
    } E_SUBFUNC_IPOD;

    typedef enum
    {
        SUBFUNC_BT_OPEN = 1,
        SUBFUNC_BT_CLOSE,
        SUBFUNC_BT_STATUS,
        SUBFUNC_BT_VERSION,
        SUBFUNC_BT_MAC,
        SUBFUNC_BT_NAME,
        SUBFUNC_BT_PIN,
        SUBFUNC_BT_PARING,
        SUBFUNC_BT_CONNECTING,
        SUBFUNC_BT_DISCONNECT,
        SUBFUNC_BT_DIALING,
        SUBFUNC_BT_ANSWER,
        SUBFUNC_BT_REJECT,
        SUBFUNC_BT_IGNORE,
        SUBFUNC_BT_SWITCH,
        SUBFUNC_BT_RECORDS,
        SUBFUNC_BT_PHONEBOOK,
        SUBFUNC_BT_STEREO,
    } E_SUBFUNC_BT;

    typedef enum
    {
        SUBFUNC_BACKCAR_START = 1,
        SUBFUNC_BACKCAR_STOP,
        SUBFUNC_BACKCAR_DISTANCE,
        SUBFUNC_BAKCCAR_TURNLEFT,
        SUBFUNC_BAKCCAR_TURNRIGHT,
    } E_SUBFUNC_BACKCAR;

    typedef enum
    {
        SUBFUNC_VOLUME_FRONT = 1,
        SUBFUNC_VOLUME_REAR,
    } E_SUBFUN_VOLUME;

    typedef enum
    {
        SUBFUNC_EFFECT_BASS = 1,
        SUBFUNC_EFFECT_TREBLE,
        SUBFUNC_EFFECT_BALANCE,
        SUBFUNC_EFFECT_LOUDNESS,
        SUBFUNC_EFFECT_EQ,
    } E_SUBFUNC_EFFECT;

    typedef enum
    {
        SUBFUNC_LCD_TONE = 1,
        SUBFUNC_LCD_BRIGHTNESS,
        SUBFUNC_LCD_CONTRAST,
        SUBFUNC_LCD_SATURATION,
        SUBFUNC_LCD_BACKLIGHT,
        SUBFUNC_LCD_PARAM,
    } E_SUBFUNC_LCD;

    typedef enum
    {
        SUBFUNC_FACTORY_LANGUAGE = 1,
        SUBFUNC_FACTORY_TIMEFORMAT,
        SUBFUNC_FACTORY_TIMESET,
        SUBFUNC_FACTORY_VIDEOFORMAT,
        SUBFUNC_FACTORY_VERSION,
        SUBFUNC_FACTORY_PARAM,
        SUBFUNC_FACTORY_WUUID,
        SUBFUNC_FACTORY_RUUID,
        SUBFUNC_FACRORY_MONTH_DATA_SET,
        SUBFUNC_FACTORY_YEAR_SET,
    } E_SUBFUNC_FACTORY;

    typedef enum
    {
        FACTORY_VERSION_BSP = 1,
        FACTORY_VERSION_APP,
        FACTORY_VERSION_DVP,
        FACTORY_VERSION_SERVO,
    } E_FACTORY_VERSION;

    //subfunction reset
    typedef enum
    {
        SUBFUNC_RESET_RESET,
    } E_SUBFUNC_RESET;

    //subfunction Key
    typedef enum
    {
        SUBFUNC_KEY_MENU = 1,
        SUBFUNC_KEY_PREVIOUS,
        SUBFUNC_KEY_NEXT,
        SUBFUNC_KEY_FF,
        SUBFUNC_KEY_FW,
        SUBFUNC_KEY_PLAYPAUSE,
        SUBFUNC_KEY_STOP,
        SUBFUNC_KEY_MODE,
        SUBFUNC_KEY_GPS,
        SUBFUNC_KEY_DVD,    // 10
        SUBFUNC_KEY_FMBAND,
        SUBFUNC_KEY_EJECT,
        SUBFUNC_KEY_SD,
        SUBFUNC_KEY_USB,
        SUBFUNC_KEY_BT,
        SUBFUNC_KEY_AVIN,
        SUBFUNC_KEY_SETTING,
        SUBFUNC_KEY_CMMB,
        SUBFUNC_KEY_CALC,
        SUBFUNC_KEY_BACK,   // 20
        SUBFUNC_KEY_CLOSE,
        SUBFUNC_KEY_ACCOFF,
        SUBFUNC_KEY_ACCON,
        SUBFUNC_KEY_CDC,
        SUBFUNC_KEY_IPOD,
        SUBFUNC_KEY_MHL,
        SUBFUNC_KEY_VIDEO,
        SUBFUNC_KEY_AUDIO,
        SUBFUNC_KEY_PICTURE,
        SUBFUNC_KEY_LAMPON, // 30
        SUBFUNC_KEY_LAMPOFF,
        SUBFUNC_KEY_MUTE_UNMUTE,
        SUBFUNC_KEY_EQ,
        SUBFUNC_KEY_BRAKE_ON,
        SUBFUNC_KEY_BRAKE_OFF,
        SUBFUNC_KEY_TURN_ON_SCREEN,
        SUBFUNC_KEY_TURN_OFF_SCREEN,
        SUBFUNC_KEY_POWER_OFF,
        SUBFUNC_KEY_HOME,
        SUBFUNC_KEY_FRONT_VOLUME_INC,   // 40
        SUBFUNC_KEY_FRONT_VOLUME_DEC,

        SUBFUNC_KEY_WIPER_FAST = 60,
        SUBFUNC_KEY_WIPER_MIDDLE,
        SUBFUNC_KEY_WIPER_CLOSE,
        SUBFUNC_KEY_WINDOW_LEFT_OPEN,
        SUBFUNC_KEY_WINDOW_LEFT_CLOSE,
        SUBFUNC_KEY_WINDOW_RIGHT_OPEN,
        SUBFUNC_KEY_WINDOW_RIGHT_CLOSE,
        SUBFUNC_KEY_HAZARD_LIGHT_OPEN,
        SUBFUNC_KEY_HAZARD_LIGHT_CLOSE,
        SUBFUNC_KEY_TURN_LEFT_SIGNAL_OPEN,
        SUBFUNC_KEY_TURN_RIGHT_SIGNAL_OPEN, // 70
        SUBFUNC_KEY_HAZARD_LIGHT_FLICKER = 72,
        SUBFUNC_KEY_BTEM,
    } E_SUBFUNC_KEY;
        //MCU version sync
    typedef enum
    {
        SUBFUNC_VERSION_HOUR_SEC_SET =1,
        SUBFUNC_VERSION_MONTH_DAY_SET,
        SUBFUNC_VERSION_YEAR_SET,
    } E_SYNC_VERSION;
    //sync open state
    typedef enum
    {
        MCU_CPU_OFF ,
        ONLY_CPU_OFF,
        CPU_NO_SYSTICK,
    } E_OPEN_STATE;

public:
    CCmdTask();
    CCmdTask(unsigned char mainFunc,
                    unsigned char subFunc,
                    unsigned char appID,
                    unsigned int data,
                    unsigned char retType,
                    const unsigned char *retData = NULL,
                    unsigned long retDataLen = 0);
    CCmdTask(const unsigned char *data, unsigned long dataLen);
    CCmdTask(const CMDPacket &pack);
    virtual ~CCmdTask();

    bool operator==(const CCmdTask &rhs) const;

    bool makeNewCmd(unsigned char mainFunc
                    , unsigned char subFunc
                    , unsigned char appID
                    , unsigned int data
                    , unsigned char retType
                    , const unsigned char *retData = NULL
                    , unsigned long retDataLen = 0);

    bool setCmdTask(const unsigned char *data, unsigned int dataLen);
    unsigned long getCmdBufLen() const;
    const unsigned char *getCmdBuf() const;

    bool setRetData(unsigned char retType,
                        const unsigned char *retData = NULL,
                        unsigned long retDataLen = 0);

    unsigned char getMainFunc() const;
    unsigned char getSubFunc() const;
    unsigned int getData() const;
    unsigned char getAppID() const;
    unsigned char getRetType() const;
    const unsigned char *getRetData() const;
    unsigned long getRetDataLen() const;
    unsigned long getSyncCount() const;
    static void resetSyncCount();
    static const char *decode(E_MAINFUNC code);

protected:
    bool setCmdData(const universal_utils::CRawString &data);
    const universal_utils::CRawString &getSetCmdData() const;

private:
    static unsigned long s_cmdSyncCount;
    universal_utils::CRawString m_strCmdBuf;
};

#endif // CCMDTASK_H
