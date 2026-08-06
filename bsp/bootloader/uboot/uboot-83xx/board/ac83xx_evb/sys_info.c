/*
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 *  Copyright(C) 2006, NXP BV, All rights reserved.
 *  by Jean-Paul Saman
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/ac83xx_basic.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/sys_info.h>

/**************************************************************************
 * get_cpu_type() - low level get cpu type
 * - no C globals yet.
 * - just looking to say if this is a 2422 or 2420 or ...
 * - to start with we will look at switch settings..
 * - 2422 id's same as 2420 for ES1 will rely on H4 board characteristics
 *   (mux for 2420, non-mux for 2422).
 ***************************************************************************/
u32 get_cpu_type(void)
{
	return(CPU_ARMCA7);
}

/******************************************
 * get_cpu_rev(void) - extract version info
 ******************************************/
u32 get_cpu_rev(void)
{
	return(1);
}

/***********************************************************************
 * get_board_type() - get board type based on current production stats.
 ************************************************************************/
u32 get_board_type(void)
{
	return(BOARD_AC83XX);
}

/*********************************************************************
 * wait_on_value() - common routine to allow waiting for changes in
 *   volatile regs.
 *********************************************************************/
u32 wait_on_value(u32 read_bit_mask, u32 match_value, u32 read_addr, u32 bound)
{
	u32 i = 0, val;
	do {
		++i;
		val = __raw_readl(read_addr) & read_bit_mask;
		if (val == match_value)
			return(1);
		if (i==bound)
			return(0);
	} while (1);
}

/*********************************************************************
 *  display_board_info() - print banner with board info.
 *********************************************************************/
void display_board_info(u32 btype)
{
	char *cpu_arm1176sjz = "1176JZF-S";
	char *cpu_armca7 = " Cortex A7 dual core";
	char *db_men = "AC83XX";
	char *cpu_s, *db_s;
	u32 cpu = get_cpu_type();

	if((cpu == CPU_ARMCA7) && (btype == BOARD_AC83XX))
	{
		cpu_s = cpu_armca7;
		db_s = db_men;
		printf("AutoChips. - %s SoC with ARM%s\n", db_s, cpu_s);
	}
	else
		printf( "unrecognized board [type=%u,cpu=%u]: no board info\n", btype, cpu );
}

/*************************************************************************
 * get_board_rev() - setup to pass kernel board revision information
 *          1 = Energizer I
 *************************************************************************/
u32 get_board_rev(void)
{
	u32 rev = 0;
	u32 btype = get_board_type();

	if (btype == BOARD_AC83XX)
	{
		rev = 1;
	}
	return(rev);
}
