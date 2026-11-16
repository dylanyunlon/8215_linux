#ifndef VEHICLE_SIGNAL_H
#define VEHICLE_SIGNAL_H

#include <stdint.h>
#include <stdbool.h>
#include "definition.h"       /* SPEED_MAX / POWER_MAX / DRV_MODE_* ... */
#include "vehicle_signal.h"   /* signal_e */

/////////////////车身信号
int32_t vehicle_get_data_signal_lamp(signal_e lamp);

int32_t vehicle_get_data_speed();

int32_t vehicle_get_data_rpm();

int32_t vehicle_get_data_gear();

int32_t vehicle_get_data_power();

int32_t vehicle_get_data_drv_mode() ;

int32_t vehicle_get_data_remain_battary();

int32_t vehicle_get_data_easy_navi();

const char* veicle_get_data_version();

///////////////时间
int32_t vehicle_get_time_min();

int32_t vehicle_get_time_hour();

void vehicle_set_time(int32_t hour, int32_t min);



///////////////简易导航
bool vehicle_get_mirror_state();

bool vehicle_get_mirror_url();

bool vehicle_get_mirror_navigation();


///////////////蓝牙数据

const char* vehicle_get_bluetooth_data() ;

////////////////OTA 页面
/////< 0:not enter  1:enter ota page  2:exit ota page
void vehicle_set_ota_page_state(int32_t state) ;

///< 0:not start  1:starting  2:Startup successful
int32_t vehicle_get_ota_state() ;

#if  0
int32_t vehicle_get_data_current_display() ;

int32_t vehicle_get_ota_state() ;

const char* veicle_get_data_mcu_ver();

void vehicle_set_ota_page_state(int32_t state);

#endif

#endif