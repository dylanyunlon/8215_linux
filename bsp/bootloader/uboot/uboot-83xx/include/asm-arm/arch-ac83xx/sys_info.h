/*
 * (C) Copyright 2006 NXP BV, All Rights Reserved
 * By Jean-Paul Saman
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef _AC83XX_SYS_INFO_H_
#define _AC83XX_SYS_INFO_H_

#define CPU_ARM1176	0x1176
#define CPU_ARMCA7        0xC070

#if defined(CONFIG_ARCH_AC83XX)
# define BOARD_AC83XX   1
#else
# error "Board type not defined."
#endif

#endif
