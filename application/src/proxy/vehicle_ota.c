#include "vehicle_ota.h"
#include "ota_manage/hcn_ota.h"
#include "carlink_cb/hcn_carlink_cb.h"
#include "proxy/vehicle_data.h"

bool vehicle_get_uptate_state()
{
    #if !ON_PC_CACLE
    hcn_update_info_t * update_info = get_hcn_current_update_info();
    if (update_info)
    {
        if (update_info->type <= UPDATE_OTA && update_info->type != UPDATE_OTA)
            return update_info->type == UPDATE_NONE ? false : true ;
    }
    #endif
    return false ;
    
}

hcn_update_info_t* vehicle_get_uptate_info()
{
    return get_hcn_current_update_info();
}


const char* vehicle_get_ota_ssid()
{
    return hcn_get_ota_ssid();
}

const char* vehicle_get_ota_password()
{
    return hcn_get_ota_ap_pwd();
}