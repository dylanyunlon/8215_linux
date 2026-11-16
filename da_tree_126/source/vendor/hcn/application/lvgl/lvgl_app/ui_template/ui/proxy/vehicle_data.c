
#include <stddef.h>
#include <stdbool.h>
#include "vehicle_data.h"
#include "vehicle_param.h"
#include "usr_utils.h"
#include "mw_sys_datetime.h"
#include "ui_feature_config.h"
#include "mw_easy_navi.h"
#include "vehicle_services.h"      /* ui 与中间件之间唯一的边界 */
// #include "common/version/hcn_version.h"
// #include "uart_communicate/hcn_uart_parse_cmd.h"

typedef struct {
  signal_e signal_lamp;
  veh_data_e veh_data;
} Signal_Lamp_Mapping_t;

#define VEH_INVALID_VALUE INT32_MIN

static const Signal_Lamp_Mapping_t signalMaps[] = {
    {ICON_GMS         , VEH_BT_PHONE_SIGNAL     } ,
    {ICON_BT         , VEH_BT_CONNECTED_STATUS } ,
    {ICON_GPS          , VEH_LIGHT_GPS           } ,
    {ICON_HIGH_BEAM   , VEH_LIGHT_HIGH_BEAM     } ,
    {ICON_LEFT        , VEH_INDICATOR_TURN_LEFT } ,
    {ICON_READY       , VEH_LIGHT_READY         } ,
    {ICON_RIGHT       , VEH_INDICATOR_TURN_RIGHT} ,
    {ICON_AUTO_BEAM   , VEH_AUTO_HEADLIGH       } ,
    {ICON_ECU     , VEH_LIGHT_ENGINE_FAULT  } ,
    {ICON_TCS    , VEH_TCS_WARNING         } ,               
    {ICON_ABS    , VEH_LIGHT_ABS           } ,
    {ICON_BRAKE       , VEH_LIGHT_BRAKE         } ,
};

static const int drvModeMaps[] = {
    [0] = DRV_MODE_N ,
    [1] = DRV_MODE_E ,
    [2] = DRV_MODE_S ,
};



int32_t vehicle_get_data_signal_lamp(signal_e lamp)
{
    int count = sizeof(signalMaps) / sizeof(Signal_Lamp_Mapping_t) ;
    for (size_t i = 0; i < count; i++){
        if (signalMaps[i].signal_lamp == lamp){
            return vs_get(signalMaps[i].veh_data);
        }
    }
    return VEH_INVALID_VALUE;
}


int32_t vehicle_get_data_speed()
{
    int32_t speed = vs_get(VEH_SPEED_CURRENT) ;
    return speed > SPEED_MAX ? SPEED_MAX : speed;
}


int32_t vehicle_get_data_rpm()
{
    int32_t rpm = vs_get(VEH_SPEED_ENGINE);
    return rpm > RPM_MAX ? RPM_MAX : rpm;
}


int32_t vehicle_get_data_gear() 
{
    int32_t gear_id = vs_get(VEH_GEAR_POSITION);
    return (gear_id > GEAR_MAX) ?  GEAR_N : gear_id ;
}


int32_t vehicle_get_data_power() 
{
    int32_t power = vs_get(VEH_TRAM_POWR);
    return power > POWER_MAX ? POWER_MAX : power;
}

int32_t vehicle_get_data_drv_mode() 
{
    int32_t drv_mode = vs_get(VEH_DRIVE_MODE);
    return drv_mode > DRV_MODE_S ? VEH_INVALID_VALUE : drvModeMaps[drv_mode] ;
}

int32_t vehicle_get_data_remain_battary() 
{
    int32_t remain_battary = vs_get(VEH_TRAM_REMAIN_BATTARY);
    return remain_battary;
}

int32_t vehicle_get_data_easy_navi()
{
    int32_t easy_navi = vs_get(VEH_EASY_NAV_STATUS);
    return easy_navi;
}

const char* veicle_get_data_version()
{
    return vs_soc_version();
}

int32_t vehicle_get_time_min()
{
    SystemTime_t time = get_os_datetime() ;
    return time.tm_min ;
}

int32_t vehicle_get_time_hour()
{
    SystemTime_t time = get_os_datetime() ;
    return time.tm_hour;
}

void vehicle_set_time(int32_t hour, int32_t min)
{
    SystemTime_t time = get_os_datetime() ;
    time.tm_hour = hour ;
    time.tm_min = min ;
    set_os_datetime(time);
}

const char* vehicle_get_bluetooth_data() 
{
    #if UI_EASY_NAVI_ENABLE
        return get_easy_navi_bt_name();   
    #else
        return " ";
    #endif
}



////////////////////////////////////////////简易导航数据

bool vehicle_get_mirror_state()
{
    int32_t state = vs_get(VEH_CARLINK_CONNECTED) ;
    return state ? true : false ;
}

bool vehicle_get_mirror_url()
{
    int32_t state = vs_get(VEH_CARLINK_URL_STATUS) ;
    return state ? true : false ;
}

bool vehicle_get_mirror_navigation()
{
    int32_t state = vs_get(VEH_EASY_NAV_STATUS) ;
    return state ? true : false ;
}


/////< 0:not enter  1:enter ota page  2:exit ota page
void vehicle_set_ota_page_state(int32_t state)
{
    vs_set(VEH_ENTER_OTA_PAGE_STATE , state);
}

///< 0:not start  1:starting  2:Startup successful
int32_t vehicle_get_ota_state()
{
    int32_t ota_state = vs_get(VEH_OTA_START_STATUS);
    return ota_state ;
}

////////////////////////////////////////////简易导航数据

#if 0 //暂未开启

//mode  0：day 1:night
int32_t vehicle_get_data_current_display() 
{
    int32_t value = vs_get(VEH_CUR_DISPALY_MODE);
    return ( value == 0 ) ? DIAPLAY_DAY_OPTION : DIAPLAY_NIGHT_OPTION ;
}

const char* veicle_get_data_mcu_ver()
{
    return get_mcu_version();
}



#endif
