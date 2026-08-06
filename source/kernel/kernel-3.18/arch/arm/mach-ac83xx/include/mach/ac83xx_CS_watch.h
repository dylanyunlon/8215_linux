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

#ifndef _AC83XX_CS_WATCH_H_
#define _AC83XX_CS_WATCH_H_


/* define & config */
#define CONFIG_AC83XX_CS_WATCH         0


/* global functions */
extern void ac83xx_CS_watch_init(void);
extern void ac83xx_CS_watch_start(unsigned int u4Flag);
extern void ac83xx_CS_watch_end(unsigned int u4Flag);
extern void ac83xx_CS_watch_dump(void);
#endif // _AC83XX_CS_WATCH_H_
