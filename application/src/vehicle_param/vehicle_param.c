/**
*
* @file vehicle_param.c
*
* @brief This message displayed in Doxygen Files index
*
* @ingroup PackageName
* (note: this needs exactly one @defgroup somewhere)
*
* @date	2025/09/05 10:14
* @author och
*
*/

#include <stddef.h>
#include <stdbool.h>
#include "vehicle_param.h"

#define VEH_INVALID_DATA INT32_MIN

static int32_t veh_data[VEH_DATA_END] = {0};

static bool is_valid_id(veh_data_e id) {
    return (id < VEH_DATA_END);
}

int32_t vehicle_hcn_get_data(veh_data_e id) {
   
        return veh_data[id];
}

void vehicle_hcn_set_data(veh_data_e id, int32_t value) {
    if (!is_valid_id(id)) {
        return;
    }

    if (veh_data[id] != value) {
        veh_data[id] = value;
    }
}

void mileage_calc_init()
{
    return ;
}