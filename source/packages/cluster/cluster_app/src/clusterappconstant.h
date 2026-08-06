#ifndef CLUSTERAPPCONSTANT_H
#define CLUSTERAPPCONSTANT_H
#include <QDebug>
#include <QStringList>

#define ICON_LIGHT "light"
#define ICON_KEY "key"
#define ICON_AIR "air"
#define ICON_SEATBELT "seatBelt"
#define ICON_REVERING_RADAR "reveringRadar"
#define ICON_SNOWFLAKE "snowFlake"
#define ICON_REFUEL "refuel"
#define ICON_PARKING_BRAKE "parking"

#define ICON_GEAR "gear"
#define ICON_ENGINE_FAIL "engineFail"
#define ICON_ABS "abs"
#define ICON_ENGINE_OIL "engineOil"
#define ICON_HIGH_light "highbeam"
#define ICON_LOW_light "lowbeam"
#define ICON_TEMPERATURE "temperature"
#define ICON_TRUN_RIGHT "turnright"
#define ICON_TRUN_LEFT "turnleft"

#define SMALL_ICON_GEAR "smallgear"
#define SMALL_ICON_ENGINE_FAIL "smallengineFail"
#define SMALL_ICON_ABS "smallabs"
#define SMALL_ICON_ENGINE_OIL "smallengineOil"
#define SMALL_ICON_HIGH_light "smallhighbeam"
#define SMALL_ICON_LOW_light "smalllowbeam"
#define SMALL_ICON_TEMPERATURE "smalltemperature"
#define SMALL_ICON_TRUN_RIGHT "smallturnright"
#define SMALL_ICON_TRUN_LEFT "smallturnleft"


#define TIME_UPDATE 20
#define TIME_FLASH 400

const QStringList ICONNAME_List = {ICON_TRUN_LEFT, ICON_GEAR, ICON_ENGINE_OIL, ICON_ABS,
                                 ICON_ENGINE_FAIL, ICON_TEMPERATURE, ICON_TRUN_RIGHT};

const QStringList SMALLICONNAME_List = {SMALL_ICON_TRUN_LEFT, SMALL_ICON_GEAR, SMALL_ICON_ENGINE_OIL, SMALL_ICON_ABS,
                                 SMALL_ICON_ENGINE_FAIL, SMALL_ICON_TEMPERATURE, SMALL_ICON_TRUN_RIGHT};


const QStringList LIGHT_List = {ICON_HIGH_light, ICON_LOW_light};

const QStringList SMALLLIGHT_List = {SMALL_ICON_HIGH_light, SMALL_ICON_LOW_light};

enum ClusterTheme {
    ComfortTheme = 0,
    EconomicTheme = 1,
};

enum GearPos
{
  GEAR_P = 0,
  GEAR_R,
  GEAR_N,
  GEAR_D
};

typedef enum {
  SPEED_NEEDLE,
  RPM_NEEDLE,
  GAUGE_NEEDLE,
  TEMPERATURE_NEEDLE
} ENUM_NEEDLE;

#define SAFE_DELETE(a) {if(a) delete a; a = nullptr;}

#endif // CLUSTERAPPCONSTANT_H
