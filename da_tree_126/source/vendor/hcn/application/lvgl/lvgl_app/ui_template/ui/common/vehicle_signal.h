#ifndef UI_COMMON_VEHICLE_SIGNAL_H
#define UI_COMMON_VEHICLE_SIGNAL_H

/*
 * 车身指示灯（顶栏图标）与手机信号等级的跨层契约类型。
 *
 * 原定义在 ui/view/home_view/signal_view.h。proxy 层的 vehicle_get_data_signal_lamp()
 * 需要 signal_e，若直接从 view 头取用会形成 proxy -> view 的反向依赖；
 * 故上移到 ui/common/，由 view 与 proxy 共同引用。
 *
 * ⚠ 枚举顺序即图标下标，signal_view.c 用 offsetof 建表，禁止调整顺序。
 */

typedef enum home_signal_com{
    ICON_GMS            ,   
    ICON_GPS            ,    
    ICON_BT             ,
    ICON_HIGH_BEAM      ,     
    ICON_LEFT           , 
    ICON_READY          , 
    ICON_RIGHT          ,
    ICON_AUTO_BEAM      ,   
    ICON_ABS            ,
    ICON_ECU            ,
    ICON_TCS            , 
    ICON_BRAKE          ,     

    ICON_SIGNAL_NUM_MAX ,
}signal_e;


typedef enum {
    GMS_LEVEL_0 ,
    GMS_LEVEL_1 ,
    GMS_LEVEL_2 ,
    GMS_LEVEL_3 ,
}gms_level_e ;

#endif