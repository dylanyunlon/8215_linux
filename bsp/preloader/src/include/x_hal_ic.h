#ifndef X_HAL_ic_H
#define X_HAL_ic_H

#include "targetConfig.h"
#include "x_typedef.h"
#include "x_hal_io.h"


#if (CONFIG_CHIP_VER_CURR == CONFIG_CHIP_VER_MT3360) 
#include "x_hal_3360.h"
#elif(CONFIG_CHIP_VER_CURR == CONFIG_CHIP_VER_MT8560) 
#include "x_hal_8560.h"
#elif(CONFIG_CHIP_VER_CURR == CONFIG_CHIP_VER_MT8580) 
#include "x_hal_8580.h"
#elif(CONFIG_CHIP_VER_CURR == CONFIG_CHIP_VER_MT3356)
#include "x_hal_3356.h"
#elif(CONFIG_CHIP_VER_CURR == CONFIG_CHIP_VER_MT3363)
#include "x_hal_3363.h"

#endif



#endif


