#include "vehicle_mile.h"
#include "vehicle_data.h"
#include "vehicle_param.h"
#include "mile_param.h"
#include "vehicle_services.h"      /* ui 与中间件之间唯一的边界 */


///< false:参数未准备好 true:已准备好
bool vehicle_get_mile_recovery()
{
    return get_recovery_mile_param() ;
}

//odo_value
uint32_t vehicle_get_mile_odo()
{
    uint32_t value ;
    if (get_mile_param(MILE_PARAM_ODO, &value) )
        return value ;
    return 0 ;

}

void vehicle_set_mile_odo(double value) 
{
    uint32_t __value = (uint32_t)value ;
    if(!set_mile_param(MILE_PARAM_ODO, &__value))
        LV_LOG_ERROR("Save odo data error!\r\n");

    return ;
}

//tripA
uint32_t vehicle_get_mile_tripA()
{
    uint32_t value ;
    if (get_mile_param(MILE_PARAM_TRIP, &value) )
        return value ;
    return 0 ;

}

void vehicle_set_mile_tripA(double value) 
{
    uint32_t __value = (uint32_t)value ;
    if(!set_mile_param(MILE_PARAM_TRIP, &__value) )
        LV_LOG_ERROR("Save tripA data error!\r\n");

    return ;
}


//tripB
uint32_t vehicle_get_mile_tripB()
{
    uint32_t value ;
    if (get_mile_param(MILE_PARAM_TRIP, &value) )
        return value ;
    return 0 ;

}

void vehicle_set_mile_tripB(double value) 
{
    uint32_t __value = (uint32_t)value ;
    set_mile_param(MILE_PARAM_TRIP, &__value) ;

    return ;
}



int32_t vehicle_get_mile_changed()
{
    uint32_t state = 0 ;
    state = vs_get(VEH_MILEAGE_CHANGE_MSG);
    if (state)
        vs_set(VEH_MILEAGE_CHANGE_MSG , 0);
    
    return state;
}
