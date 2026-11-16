/*
 * OTA Logic - periodic refresh of OTA page data
 * Pulls UUID/BT name/SN/version/OTA state from BSP and pushes to view.
 */

#include "ota_logic.h"
#include "ota_view.h"
#include "vehicle_data.h"
#include "vehicle_param.h"
#include "aic_ui.h"
#include <string.h>
#include <stdio.h>
#include "ui_feature_config.h"
#include "ota_manage.h"
#include "mw_easy_navi.h"

#define REFRESH_INTERVAL_MS   (100)

static lv_timer_t *s_ota_timer = NULL;

static void timer_refresh(lv_timer_t *info);

static const char* update_status[] = { " " , "In progressing..." , "Update Success" ,"Update Failed" , "Update Timeout"} ;
static const char* update_error[] = { " " , "Error CRC" , "Error FLASH" ,"Error File Type" } ;
static const key_id_t ota_status_array[] = {ota_state_closed , ota_state_starting , ota_state_open} ;

update_info_t update_info = {0};

static bool initInfo = false;
static int32_t ota_status = 0;

void ota_page_init(void)
{
    if (s_ota_timer == NULL) {
        s_ota_timer = lv_timer_create(timer_refresh, REFRESH_INTERVAL_MS, NULL);
    }

    timer_refresh(NULL);
}

void on_ota_page_destory(void)
{
    if (s_ota_timer) {
        lv_timer_del(s_ota_timer);
        s_ota_timer = NULL;
    }
    initInfo = false ;
    ota_status = 0 ;
    memset(&update_info , 0 ,sizeof(update_info)) ;

}

static void timer_refresh(lv_timer_t *info)
{
    (void)info;

    //版本信息
    if (!initInfo)
    {
        ota_view_refresh_uuid(UI_CARLINK_EC_UUID) ;

        #if UI_EASY_NAVI_ENABLE
        ota_view_refresh_bluetooth(get_easy_navi_bt_name());
        #endif

        char buff[256] = {0};
        lv_snprintf(buff , sizeof(buff) - 1 , "%s / %s" ,UI_OTA_DEFAULT_SSID , UI_OTA_DEFAULT_PASSWD );
        ota_view_refresh_ota_passworld(buff);

        ota_view_refresh_sn(DEVICE_SN);

        ota_view_refresh_version(veicle_get_data_version());

        lv_log("device page info refresh successed\n") ;

        initInfo = true ;
    }

    //OTA信息
    update_info_t *_update_info =  get_current_update_info() ;
    if (_update_info)
    {
        if (_update_info->status != update_info.status && _update_info->status <= UPDATE_STATUS_TIMEOUT )
        {
            ota_view_update_status(update_status[_update_info->status]) ;
            update_info.status = _update_info->status ;
        }
        
        if (_update_info->progress != update_info.progress)
        {
            ota_view_set_progress(_update_info->progress) ;
            update_info.progress =  _update_info->progress ;
        }
        
        if (_update_info->error != update_info.error && _update_info->error <= UPDATE_ERROR_FILE_TYPE) 
        {
            ota_view_update_error(update_error[_update_info->error]) ;
            update_info.error = _update_info->error ;
        }
    }
    
    int _state = vehicle_get_ota_state() ;
    if (_state != ota_status && _state <= 2)
    {
        ota_view_refresh_ota_state(aic_get_translation(ota_status_array[_state])) ;
        ota_status = _state ;
    }
    
}


