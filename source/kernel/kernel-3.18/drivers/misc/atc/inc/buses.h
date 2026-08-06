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
//
//  File: buses.h
//
#ifndef __BUSES_H
#define __BUSES_H

//------------------------------------------------------------------------------

#ifdef DEBUG

#define ZONE_ERROR          DEBUGZONE(0)
#define ZONE_WARN           DEBUGZONE(1)
#define ZONE_FUNCTION       DEBUGZONE(2)
#define ZONE_INIT           DEBUGZONE(3)
#define ZONE_INFO           DEBUGZONE(4)
#define ZONE_IST            DEBUGZONE(5)
#define ZONE_POWER          DEBUGZONE(6)
#define ZONE_BKL            DEBUGZONE(10)
#define ZONE_SPI            DEBUGZONE(11)
#define ZONE_I2C            DEBUGZONE(12)
#define ZONE_PWM            DEBUGZONE(13)
#define ZONE_GPIO           DEBUGZONE(14)
#define ZONE_CLK            DEBUGZONE(15)

extern DBGPARAM dpCurSettings;

#endif

//------------------------------------------------------------------------------

#endif
