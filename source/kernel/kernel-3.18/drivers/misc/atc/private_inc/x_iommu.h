/*
* Copyright (c) 2016 AutoChips Inc.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License version 2 as
* published by the Free Software Foundation.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
*/

#ifndef X_IOMMU_H
#define X_IOMMU_H

//============================================================================
// Include files
//============================================================================
#include "x_hal_ic.h"
#include "x_typedef.h"
#include "drv_config.h"
#include "chip_ver.h"

#if 0
#if (CONFIG_CHIP_VER_CURR == CONFIG_CHIP_VER_MT8560)
#include "x_iommu_8560.h"
#elif (CONFIG_CHIP_VER_CURR == CONFIG_CHIP_VER_MT8561)
#include "x_iommu_8561.h"
#elif (CONFIG_CHIP_VER_CURR == CONFIG_CHIP_VER_MT8563)
#include "x_iommu_8563.h"
#elif (CONFIG_CHIP_VER_CURR == CONFIG_CHIP_VER_AC83XX)
#include "x_iommu_3363.h"
#elif (CONFIG_CHIP_VER_CURR == CONFIG_CHIP_VER_MT8580)
#include "x_iommu_8580.h"
#endif
#else
#include "x_iommu_3363.h"
#endif
//============================================================================
// Constant definitions
//============================================================================



#endif  // X_IOMMU_H

