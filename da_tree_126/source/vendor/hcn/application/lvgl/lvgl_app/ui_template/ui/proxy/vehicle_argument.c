
#include "vehicle_argument.h"
#include "vehicle_data.h"
#include "set_param.h"
#include "usr_backlight.h"
#include "ui_feature_config.h"
#include "vehicle_services.h"      /* ui 与中间件之间唯一的边界 */

///< false:参数未准备好 true:已准备好
bool vehicle_get_param_recovery()
{
    return vs_usr_param_ready() ;
}

///< 语言 0:中文 1:英文,预留位
uint8_t vehicle_get_param_language()
{
    uint8_t value ;
    if (vs_usr_param_get(USR_PARAM_LANGUAGE, &value) ) 
        return value ;
    return 0 ;

}

void vehicle_set_param_language(uint8_t value)
{
    vs_usr_param_set(USR_PARAM_LANGUAGE, (void*)&value) ;
    return ;
}

///< 里程格式 0:公制 1：英制
uint8_t vehicle_get_param_unit()
{
    uint8_t value ;
    if (vs_usr_param_get(USR_PARAM_MILE_FORMAT, &value) )
        return value ;
    return 0 ;

}

void vehicle_set_param_unit(uint8_t value)
{
    vs_usr_param_set(USR_PARAM_MILE_FORMAT, (void*)&value) ;
    return ;
}

//亮度等级 0:自动  1-5级
uint8_t vehicle_get_param_brightness()
{
    uint8_t value ;
    if (vs_usr_param_get(USR_PARAM_BRIGHTNESS_LEVEL, &value) )
        return value ;
    return 0 ;

}

void vehicle_set_param_brightness(uint8_t value)
{
    vs_usr_param_set(USR_PARAM_BRIGHTNESS_LEVEL, (void*)&value) ;
    #if UI_BACKLIGHT_PWM_ENABLE
    vs_backlight_set(value);
    #endif
    
    return ;
}

//主题
uint8_t vehicle_get_param_display()
{
    uint8_t value ;
    if (vs_usr_param_get(USR_PARAM_THEME, &value) )
        return value ;
    return 0 ;

}

void vehicle_set_param_display(uint8_t value)
{
    vs_usr_param_set(USR_PARAM_THEME, (void*)&value) ;
    return ;
}

//蓝牙开关
uint8_t vehicle_get_param_bluetooth()
{
    uint8_t value ;
    if (vs_usr_param_get(USR_PARAM_BT_SWITCH, &value) )
        return value ;
    return 0 ;

}

void vehicle_set_param_bluetooth(uint8_t value)
{
    vs_usr_param_set(USR_PARAM_BT_SWITCH, (void*)&value) ;
    return ;
}
