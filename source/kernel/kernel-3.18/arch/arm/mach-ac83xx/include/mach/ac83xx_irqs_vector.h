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

#if !defined(AC83XX_IRQS_VECTOR_H)
#define AC83XX_IRQS_VECTOR_H

#if !defined(X_BIM_H)
#include "chip_ver.h"
#endif

#if (CONFIG_CHIP_VER_CURR == CONFIG_CHIP_VER_AC83XX)
#include "83xx_irqs_vector.h"
#else
#include "error_irqs_vector.h"  // need to add irq vector
#endif // end of #if (CONFIG_CHIP_VER_CURR == CONFIG_CHIP_VER_AC83XX)

#endif /* AC83XX_IRQS_VECTOR_H*/

