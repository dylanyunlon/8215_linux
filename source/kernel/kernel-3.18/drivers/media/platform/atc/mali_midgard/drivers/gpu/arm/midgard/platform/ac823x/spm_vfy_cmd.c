/********************************************************************************************
 *     LEGAL DISCLAIMER
 *
 *     (Header of MediaTek Software/Firmware Release or Documentation)
 *
 *     BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *     THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE") RECEIVED
 *     FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS
 *     ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED,
 *     INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
 *     A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY
 *     WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK
 *     ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
 *     NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION
 *     OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 *     BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE LIABILITY WITH
 *     RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION,
 *     TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/
#if 0
#include "x_dramc.h"
#include "x_hal_ic.h"
#include "x_printf.h"
#include "x_rtos.h"
#include "x_debug.h"
#include "x_assert.h"
#include "x_timer.h"
#include "x_bim.h"
#include "x_lint.h"
#include "x_drv_cli.h"
#include "x_stl_lib.h"
#include "x_gpio.h"



#include "spm_vfy_cmd.h"



#include <asm/memory.h>
#include <asm/io.h>
#include <asm/cacheflush.h>
#include <mach/hardware.h>

#include <linux/dma-mapping.h>
#include <linux/slab.h>
#endif
#include <asm/io.h>
#include <linux/delay.h>
#include <linux/timer.h>

static void __iomem *spm_base_va;
static void __iomem *io_base_va;
//static void __iomem *cci_reg_vbase;
//static void __iomem *mcusys_reg_vbase;

#define SPM_BASE_VA				spm_base_va
#define IO_VIRT_BASE			io_base_va

#define SPM_READ32(REG)           	__raw_readl((unsigned long)(SPM_BASE_VA + REG))
#define SPM_WRITE32(VAL, REG)	__raw_writel(VAL, (unsigned long)(SPM_BASE_VA + REG))
#define REG_READ32(REG)			__raw_readl((unsigned long)(IO_VIRT_BASE + REG))
#define REG_WRITE32(VAL, REG)  	__raw_writel(VAL, (unsigned long)(IO_VIRT_BASE + REG))

#define SET_BIT(a, b)			((a) |= ((u32)1L<<(b)))
#define CLR_BIT(a, b)			((a) &= (~((u32)1L<<(b))))

#define IO_REG_PBASE		(0x10000000)
#define SPM_REG_PBASE		(0x10048000)
#define CCI_REG_PBASE			(0x13090000)
#define MCUSYS_REG_PBASE		(0x10055000)

/*
* cluster1 is power up already if cluster1_powerup_done is non-zero.
*/
//static unsigned int cluster1_powerup_bitmap = 0;

static inline void delay_us(u32 us)
{
	udelay(us);
}

static inline u32 spm_readl(u32 offset)
{
	//return IO_READ32(__io(spm_reg_vbase + offset), 0);
	return SPM_READ32(offset);
}

static inline void spm_writel(u32 regval32, u32 offset)
{
	//IO_WRITE32(__io(spm_reg_vbase + offset), 0 , regval32);
	SPM_WRITE32(regval32, offset);
}

static u32 spm_reg_read(u32 reg_offset)
{
	u32 val;

	/* enable spm clock */
	spm_writel(0x02860001, 0);

	val = spm_readl(reg_offset);
	spm_writel(0x02860000, 0);

	return val;
}

static void spm_reg_write(u32 reg_offset, u32 val)
{
	/* enable spm clock */
	spm_writel(0x02860001, 0);

	val = val & 0x0000FFFF;
	val = val | (0x0286 << 16);
	spm_writel(val, reg_offset);

	spm_writel(0x02860000, 0);
}

#if 0
/*
* l2c_from
*	0, Cluster<n> CPUSYS Power On withL2 Cache from Power off.
*	1, Cluster<n> CPUSYS Power On withL2 Cache from Dormant(Sleep).
*/
static void clusterx_poweron(u32 cluster_id, u32 l2c_from)
{
	u32 val;
	u32 mpx_cpusys_reg_offset;
	u32 l2c_afifo_bit_offset;
	void __iomem * mpx_cfg0_addr;

	if (cluster_id == 0) {
		mpx_cpusys_reg_offset = 0xE4;
		l2c_afifo_bit_offset = 0;
		mpx_cfg0_addr = mcusys_reg_vbase + 0x02C;
	}
	else if (cluster_id == 1) {
		mpx_cpusys_reg_offset = 0xE8;
		l2c_afifo_bit_offset = 4;
		mpx_cfg0_addr = mcusys_reg_vbase + 0x22C;
	} else {
		pr_err("cluster id is error.\n");
		return;
	}

	/*
	* Follow Cluster<n> Power on with L2 Cache from Power Off /Dormant sequence in
	* SPM Application Note
	*/

	/* Set mp<n>_cpusys_top_pwr_rst_en to high */
	val = spm_reg_read(mpx_cpusys_reg_offset);
	SET_BIT(val, 5);
	spm_reg_write(mpx_cpusys_reg_offset, val);

	/* Set mp<n>_cpusys_top_pwr_clk_dis to high */
	val = spm_reg_read(mpx_cpusys_reg_offset);
	SET_BIT(val, 6);
	spm_reg_write(mpx_cpusys_reg_offset, val);

	/* Set mp<n>_cpusys_top_pwr_on to high */
	val = spm_reg_read(mpx_cpusys_reg_offset);
	SET_BIT(val, 7);
	spm_reg_write(mpx_cpusys_reg_offset, val);

	/* Set mp<n>_cpusys_top_pwr_on_2nd to high */
	val = spm_reg_read(mpx_cpusys_reg_offset);
	SET_BIT(val, 8);
	spm_reg_write(mpx_cpusys_reg_offset, val);

	/* Wait until mp<n>_cpusys_top_pwr_ack and mp<n>_cpusys_top_pwr_ack_2nd are high */
	while ((spm_reg_read(mpx_cpusys_reg_offset) & (1 << 9)) == 0);
	while ((spm_reg_read(mpx_cpusys_reg_offset) & (1 << 10)) == 0);

	/* Set mp<n>_cpusys_top_clamp to low */
	val = spm_reg_read(mpx_cpusys_reg_offset);
	CLR_BIT(val, 11);
	spm_reg_write(mpx_cpusys_reg_offset, val);

	if (l2c_from == 0) {
		/* Set mp<n>_cpusys_top_mem_pd to low */
		val = spm_reg_read(mpx_cpusys_reg_offset);
		CLR_BIT(val, 0);
		spm_reg_write(mpx_cpusys_reg_offset, val);

		/* Wait until mp<n>_cpusys_top_mem_pd_ack is low */
		while (spm_reg_read(mpx_cpusys_reg_offset) & (1 << 3));

	} else {
		/* Set mp<n>_cpusys_top_mem_slpb to high */
		val = spm_reg_read(mpx_cpusys_reg_offset);
		SET_BIT(val, 1);
		spm_reg_write(mpx_cpusys_reg_offset, val);

		/* Wait until mp<n>_cpusys_top_mem_slpb_ack is high */
		while ((spm_reg_read(mpx_cpusys_reg_offset) & (1 << 4)) == 0);
	}
	/* Wait 1000ns */
	delay_us(10);

	/* Set mp<n>_cpusys_top_mem_ckiso to low */
	val = spm_reg_read(mpx_cpusys_reg_offset);
	CLR_BIT(val, 2);
	spm_reg_write(mpx_cpusys_reg_offset, val);

	/* Set mp<n>_cpusys_top_pwr_clk_dis to low */
	val = spm_reg_read(mpx_cpusys_reg_offset);
	CLR_BIT(val, 6);
	spm_reg_write(mpx_cpusys_reg_offset, val);

	/* Set mp<n>_cpusys_top_pwr_rst_en to low */
	val = spm_reg_read(mpx_cpusys_reg_offset);
	CLR_BIT(val, 5);
	spm_reg_write(mpx_cpusys_reg_offset, val);

	/* Set pwrdnreqn_mp<n>_l2c_afifo to high and wait pwrdnackn_mp<n>_l2c_afifo to high */
	val = spm_reg_read(0xF8);
	SET_BIT(val, l2c_afifo_bit_offset);
	spm_reg_write(0xF8, val);
	while ((spm_reg_read(0xF8) & (1 << (l2c_afifo_bit_offset + 2))) == 0);

	//Set pwrdnreqn_mp<n>_adb to high and wait pwrdnackn_mp<n>_adb to high
	val = spm_reg_read(0xF8);
	SET_BIT(val, l2c_afifo_bit_offset + 1);
	spm_reg_write(0xF8, val);
	while ((spm_reg_read(0xF8) & (1 << (l2c_afifo_bit_offset + 3))) == 0);

	/*
	* Program CCI400 to disable S4 (for MP0) or S3 (for MP1) interface
	* Snoop Control Registers bit0 to 1 if need snoop feature (0x13095000 for mp0, 0x13094000 for mp1)
	*
	* Bit1 is needed to set also b/c it enables DVM support.
	* SW needs to poll the regster write to make sure updates are already effective.
	*/
	if (cluster_id == 0) {
		val =IO_READ32(__io(cci_reg_vbase + 0x5000), 0);
		val = val | 0x3;
		IO_WRITE32(__io(cci_reg_vbase + 0x5000), 0, val);
	}
	else {
		val =IO_READ32(__io(cci_reg_vbase + 0x4000), 0);
		val = val | 0x3;
		IO_WRITE32(__io(cci_reg_vbase + 0x4000), 0, val);
	}

	while (IO_READ32(__io(cci_reg_vbase + 0xC), 0) & 0x1) {
		delay_us(1);
	}

	/* Program MP<n>_AXI_CONFIG acinactm to 0
	* if need snoop feature (0x1005502C for MP0, 0x1005522c for MP1, bit 4)
	*/
	val =IO_READ32(__io(mpx_cfg0_addr), 0);
	CLR_BIT(val, 4);
	IO_WRITE32(__io(mpx_cfg0_addr), 0, val);

	/* Finish power on and reset sequences */
}

static void corex_poweron(u32 core_id)
{
	u32 val;
	u32 mpx_corex_reg_offset;

	/*
	* core_id and cluster_id mappinng
	*
	*	 core_id		cluster_id	core
	*	0			0			0
	*	1			0			1
	*	2			0			2
	*	3			0			3
	*	4			1			0
	*	5			1			1
	*	6			1			2
	*	7			1			3
	*/

	if (core_id > 7) {
		pr_err("core_id(%d) is error.\n", core_id);
		return;
	}
	else
		mpx_corex_reg_offset = 0xC4 + (core_id << 2);

	/* Set mpx_cpux_pwr_rst_en to high */
	val = spm_reg_read(mpx_corex_reg_offset);
	SET_BIT(val, 3);
	spm_reg_write(mpx_corex_reg_offset, val);

	/* Set mpx_cpux_pwr_clk_dis to high */
	val = spm_reg_read(mpx_corex_reg_offset);
	SET_BIT(val, 4);
	spm_reg_write(mpx_corex_reg_offset, val);

	/* Set mpx_cpux_pwr_on to high */
	val = spm_reg_read(mpx_corex_reg_offset);
	SET_BIT(val, 5);
	spm_reg_write(mpx_corex_reg_offset, val);

	/* Set mpx_cpux_pwr_on_2nd to high */
	val = spm_reg_read(mpx_corex_reg_offset);
	SET_BIT(val, 6);
	spm_reg_write(mpx_corex_reg_offset, val);

	/* Wait until mpx_cpux_pwr_ack and mpx_cpux_pwr_ack_2nd are high */
	while ((spm_reg_read(mpx_corex_reg_offset) & (1 << 7)) == 0);
	while ((spm_reg_read(mpx_corex_reg_offset) & (1 << 8)) == 0);

	/* Set mpx_cpux_clamp/mpx_cpux_pd_slpb_clamp to low */
	val = spm_reg_read(mpx_corex_reg_offset);
	CLR_BIT(val, 9);
	CLR_BIT(val, 10);
	spm_reg_write(mpx_corex_reg_offset, val);

	/* Set mpx_corex_mem_pd to low */
	val = spm_reg_read(mpx_corex_reg_offset);
	CLR_BIT(val, 0);
	spm_reg_write(mpx_corex_reg_offset, val);

	/* Wait until mpx_corex_mem_pd_ack are low */
	while ((spm_reg_read(mpx_corex_reg_offset) & (1 << 2)));

	/* Wait 1000ns for memory power ready (defined in memory model) */
	delay_us(1);

	/* Set mpx_corex_mem_ckiso to low */
	val = spm_reg_read(mpx_corex_reg_offset);
	CLR_BIT(val, 1);
	spm_reg_write(mpx_corex_reg_offset, val);

	/* Set mpx_cpux_pwr_clk_dis to low */
	val = spm_reg_read(mpx_corex_reg_offset);
	CLR_BIT(val, 4);
	spm_reg_write(mpx_corex_reg_offset, val);

	/* Set mpx_cpux_pwr_rst_en to low to finish power on and reset sequences */
	val = spm_reg_read(mpx_corex_reg_offset);
	CLR_BIT(val, 3);
	spm_reg_write(mpx_corex_reg_offset, val);

}

/*
* l2c_into
*	0, Cluster<n> CPUSYS Power On withL2 Cache into Power off.
*	1, Cluster<n> CPUSYS Power On withL2 Cache into Dormant(Sleep).
*/
static void clusterx_poweroff(u32 cluster_id, u32 l2c_into)
{
	u32 val;
	u32 mpx_cpusys_reg_offset;
	u32 l2c_afifo_bit_offset;
	void __iomem * mpx_cfg0_addr;
	void __iomem * mpx_misc_cfg_addr;

	if (cluster_id == 0) {
		mpx_cpusys_reg_offset = 0xE4;
		l2c_afifo_bit_offset = 0;
		mpx_cfg0_addr = mcusys_reg_vbase + 0x02C;
		mpx_misc_cfg_addr = mcusys_reg_vbase + 0x064;
	}
	else if (cluster_id == 1) {
		mpx_cpusys_reg_offset = 0xE8;
		l2c_afifo_bit_offset = 4;
		mpx_cfg0_addr = mcusys_reg_vbase + 0x22C;
		mpx_misc_cfg_addr = mcusys_reg_vbase + 0x264;
	} else {
		pr_err("clusterx_poweroff, cluster id is error.\n");
		return;
	}

	/*
	* Follow Cluster<n> Power off with L2 Cache into Power Off /Dormant sequence in
	* SPM Application Note
	*/

	/* program all cores of ClusterX into standby wait for interrupt or standby wait for event
	* mode. Then follow core power off sequence to power down each core in cluster.
	*/

	/*
	* Program CCI400 to disable S4 (for MP0) or S3 (for MP1) interface
	* Snoop Control Registers bit0 to 0 if need snoop feature (0x13095000 for mp0, 0x13094000 for mp1)
	*
	* Bit1 is needed to clear also b/c it disables DVM support.
	* SW needs to poll the regster write to make sure updates are already effective.
	*/
	if (cluster_id == 0) {
		val =IO_READ32(__io(cci_reg_vbase + 0x5000), 0);
		val = val & (~0x3);
		IO_WRITE32(__io(cci_reg_vbase + 0x5000), 0, val);
	}
	else {
		val =IO_READ32(__io(cci_reg_vbase + 0x4000), 0);
		val = val & (~0x3);
		IO_WRITE32(__io(cci_reg_vbase + 0x4000), 0, val);
	}

	while (IO_READ32(__io(cci_reg_vbase + 0xC), 0) & 0x1) {
		delay_us(1);
	}

	/* Program MP<n>_AXI_CONFIG acinactm to 1
	* if need snoop feature (0x1005502C for MP0, 0x1005522c for MP1, bit 4)
	*/
	val =IO_READ32(__io(mpx_cfg0_addr), 0);
	SET_BIT(val, 4);
	IO_WRITE32(__io(mpx_cfg0_addr), 0, val);

	/* wait mpx_standbywfil2 to high */
	while ((IO_READ32(__io(mpx_misc_cfg_addr), 0) & (1 << 28)) == 0);

	/* Set pwrdnreqn_mp<n>_l2c_afifo to low and wait pwrdnackn_mp<n>_l2c_afifo to low */
	val = spm_reg_read(0xF8);
	CLR_BIT(val, l2c_afifo_bit_offset);
	spm_reg_write(0xF8, val);
	while ((spm_reg_read(0xF8) & (1 << (l2c_afifo_bit_offset + 2))));

	/* Set pwrdnreqn_mp<n>_adb to low and wait pwrdnackn_mp<n>_adb to low */
	val = spm_reg_read(0xF8);
	CLR_BIT(val, l2c_afifo_bit_offset + 1);
	spm_reg_write(0xF8, val);
	while ((spm_reg_read(0xF8) & (1 << (l2c_afifo_bit_offset + 3))));

	/* Set mp<n>_cpusys_top_clamp to high */
	val = spm_reg_read(mpx_cpusys_reg_offset);
	SET_BIT(val, 11);
	spm_reg_write(mpx_cpusys_reg_offset, val);

	/* Set mp<n>_cpusys_top_mem_ckiso to high */
	val = spm_reg_read(mpx_cpusys_reg_offset);
	SET_BIT(val, 2);
	spm_reg_write(mpx_cpusys_reg_offset, val);

	if (l2c_into == 0) {
		/* Set mp<n>_cpusys_top_mem_pd to high */
		val = spm_reg_read(mpx_cpusys_reg_offset);
		SET_BIT(val, 0);
		spm_reg_write(mpx_cpusys_reg_offset, val);

		/* Wait until mp<n>_cpusys_top_mem_pd_ack is high */
		while ((spm_reg_read(mpx_cpusys_reg_offset) & (1 << 3)) == 0);

	} else {
		/* wait 100ns */
		delay_us(1);

		/* Set mp<n>_cpusys_top_mem_slpb to low */
		val = spm_reg_read(mpx_cpusys_reg_offset);
		CLR_BIT(val, 1);
		spm_reg_write(mpx_cpusys_reg_offset, val);

		/* Wait until mp<n>_cpusys_top_mem_slpb_ack is low */
		while ((spm_reg_read(mpx_cpusys_reg_offset) & (1 << 4)));
	}

	/* Set mp<n>_cpusys_top_pwr_rst_en to high */
	val = spm_reg_read(mpx_cpusys_reg_offset);
	SET_BIT(val, 5);
	spm_reg_write(mpx_cpusys_reg_offset, val);

	/* Set mp<n>_cpusys_top_pwr_clk_dis to high */
	val = spm_reg_read(mpx_cpusys_reg_offset);
	SET_BIT(val, 6);
	spm_reg_write(mpx_cpusys_reg_offset, val);

	/* Set mp<n>_cpusys_top_pwr_on to low */
	val = spm_reg_read(mpx_cpusys_reg_offset);
	CLR_BIT(val, 7);
	spm_reg_write(mpx_cpusys_reg_offset, val);

	/* Set mp<n>_cpusys_top_pwr_on_2nd to low */
	val = spm_reg_read(mpx_cpusys_reg_offset);
	CLR_BIT(val, 8);
	spm_reg_write(mpx_cpusys_reg_offset, val);

	/* Wait until mp<n>_cpusys_top_pwr_ack and mp<n>_cpusys_top_pwr_ack_2nd are low */
	while ((spm_reg_read(mpx_cpusys_reg_offset) & (1 << 9)));
	while ((spm_reg_read(mpx_cpusys_reg_offset) & (1 << 10)));

	/* Finish power off sequences */
}

static void corex_poweroff(u32 core_id)
{
	u32 val;
	u32 mpx_corex_reg_offset;

	/*
	* core_id and cluster_id mappinng
	*
	*	 core_id		cluster_id	core
	*	0			0			0
	*	1			0			1
	*	2			0			2
	*	3			0			3
	*	4			1			0
	*	5			1			1
	*	6			1			2
	*	7			1			3
	*/

	if (core_id > 7) {
		pr_err(" corex_poweroff,core_id(%d) is error.\n", core_id);
		return;
	}
	else
		mpx_corex_reg_offset = 0xC4 + (core_id << 2);

	/* program corex into standby wait for interrupt or standby wait for event mode. */

	/* Set mpx_cpux_clamp/mpx_cpux_pd_slpb_clamp to high */
	val = spm_reg_read(mpx_corex_reg_offset);
	SET_BIT(val, 9);
	SET_BIT(val, 10);
	spm_reg_write(mpx_corex_reg_offset, val);

	/* Set mpx_corex_mem_ckiso to high */
	val = spm_reg_read(mpx_corex_reg_offset);
	SET_BIT(val, 1);
	spm_reg_write(mpx_corex_reg_offset, val);

	/* Set mpx_corex_mem_pd to high */
	val = spm_reg_read(mpx_corex_reg_offset);
	SET_BIT(val, 0);
	spm_reg_write(mpx_corex_reg_offset, val);

	/* Set mpx_cpux_pwr_rst_en to high */
	val = spm_reg_read(mpx_corex_reg_offset);
	SET_BIT(val, 3);
	spm_reg_write(mpx_corex_reg_offset, val);

	/* Set mpx_cpux_pwr_clk_dis to high */
	val = spm_reg_read(mpx_corex_reg_offset);
	SET_BIT(val, 4);
	spm_reg_write(mpx_corex_reg_offset, val);

	/* Set mpx_cpux_pwr_on to low */
	val = spm_reg_read(mpx_corex_reg_offset);
	CLR_BIT(val, 5);
	spm_reg_write(mpx_corex_reg_offset, val);

	/* Set mpx_cpux_pwr_on_2nd to low */
	val = spm_reg_read(mpx_corex_reg_offset);
	CLR_BIT(val, 6);
	spm_reg_write(mpx_corex_reg_offset, val);

	/* Wait until mpx_cpux_pwr_ack and mpx_cpux_pwr_ack_2nd are low */
	while ((spm_reg_read(mpx_corex_reg_offset) & (1 << 7)));
	while ((spm_reg_read(mpx_corex_reg_offset) & (1 << 8)));

	/* the power off sequence is finished. */

}

static void __spm_power_core(u32 cpu, u32 on_off)
{
	if (on_off) { //power on cpuX
		if (cpu > 3) {
			/* cluster1 is power up first time. */
			if (!cluster1_powerup_bitmap)
				clusterx_poweron(1, 0);
			cluster1_powerup_bitmap |= (1 << cpu);
		}

		delay_us(10);
		corex_poweron(cpu);
	} else { //power off cpuX
		corex_poweroff(cpu);
		delay_us(10);

		if (cpu > 3) {
			cluster1_powerup_bitmap &= (~(1 << cpu));

			/* cluster1 is power down. */
			if (!cluster1_powerup_bitmap)
				clusterx_poweroff(1, 0);
		}
	}
}

static INT32 spm_power_core(INT32 i4Argc, const CHAR ** szArgv)
{
	u32 cpu = 0;
	u32 on_off = 0; // 0: power off, 1: power on

	if (i4Argc == 3) {
		cpu = StrToInt(szArgv[1]);
		on_off = StrToInt(szArgv[2]);

		if (cpu == 0) {
			pr_err("spm_power_core: Can't power on/off core0.\n");
			return -1;
		} else if (cpu > 7) {
			pr_err("spm_power_core: core(%d) is invalied.\n", cpu);
			return -1;
		}

		if (on_off > 1) {
			pr_err("spm_power_core: on_off(%d) is invalid.\n", on_off);
			return -1;
		}

		pr_err("spm_power_core: %s Core%d begin...\n", (on_off ? "PowerOn" : "PowerOff"), cpu);
		__spm_power_core(cpu, on_off);
		pr_err("spm_power_core: %s Core%d end\n", (on_off ? "PowerOn" : "PowerOff"), cpu);

	} else {
		pr_err("Usage: spm_power_core [cpuX:1/2/3/4/5/6/7] [off/on:0/1]\n");
		return -1;
	}

	return 0;
}
#endif

static void spm_clear_onebit(u32 reg_offset, u32 bit_idx)
{
	u32 val;

	val = spm_reg_read(reg_offset);
	CLR_BIT(val, bit_idx);
	spm_reg_write(reg_offset, val);
}

static void spm_set_onebit(u32 reg_offset, u32 bit_idx)
{
	u32 val;

	val = spm_reg_read(reg_offset);
	SET_BIT(val, bit_idx);
	spm_reg_write(reg_offset, val);
}

static void spm_clear(u32 reg_offset, u32 width)
{
	u32 i;

	for (i = 0; i < width; i++) {
		spm_clear_onebit(reg_offset, i);
	}
}

static void spm_set(u32 reg_offset, u32 width)
{
	u32 i;

	for (i = 0; i < width; i++) {
		spm_set_onebit(reg_offset, i);
	}
}

static void __spm_power_g3d(u32 sw_hw_sel, u32 on_off)
{
	u32 val;

	if (on_off) { //power on
		/* write mfg_hier_pwr_on(bit2) = 1 */
		val = spm_reg_read(0x08);
		SET_BIT(val, 2);
		spm_reg_write(0x08, val);

		/* wait mfg_pwr_on_ack(bit22) == 1 */
		while (!(spm_reg_read(0x7C) & (1 << 22)));

		msleep(1);

		/* write mfg_hier_pwr_on_s(bit3) = 1 */
		val = spm_reg_read(0x08);
		SET_BIT(val, 3);
		spm_reg_write(0x08, val);

		/* wait mfg_pwr_on_ack_s(bit21) == 1 */
		while (!(spm_reg_read(0x7C) & (1 << 21)));

		/* write mfg_hier_clock_dis(bit4) = 0 */
		val = spm_reg_read(0x08);
		CLR_BIT(val, 4);
		spm_reg_write(0x08, val);

		if (sw_hw_sel) { //power on with hardware control
			/* write mfg_hier_mem_pd_sel(bit6) = 1
			 * write mfg_hier_mem_pd_hw(bit5) = 0
			 */
			val = spm_reg_read(0x08);
			CLR_BIT(val, 5);
			SET_BIT(val, 6);
			spm_reg_write(0x08, val);

			/* wait mfg_mem_pd_ack = 0 */
			while ((spm_reg_read(0x7C) & (1 << 20)));

			/* write mfg1_hier_mem_pd_sel(bit14) = 1
			 * write mfg1_hier_mem_pd_hw(bit13) = 0
			 */
			val = spm_reg_read(0x08);
			CLR_BIT(val, 13);
			SET_BIT(val, 14);
			spm_reg_write(0x08, val);

			/* wait mfg1_mem_pd_ack = 0 */
			while ((spm_reg_read(0x7C) & (1 << 16)));

		} else { //power on with software control
			/* write mfg_hier_mem_pd_sel(bit6) = 0 */
			val = spm_reg_read(0x08);
			CLR_BIT(val, 6);
			spm_reg_write(0x08, val);

			/* write mfg_hier_sft_sram_pd_l = 0x0000
			 * write mfg_hier_sft_sram_pd_h = 0x000
			 */
			spm_clear(0x24, 16);
			spm_clear(0x28, 7);

			/* wait mfg_hier_sram_pd == 0x00000000 */
			while (spm_reg_read(0x98) != 0x0);

			/* write mfg1_hier_mem_pd_sel(bit14) = 0 */
			val = spm_reg_read(0x08);
			CLR_BIT(val, 14);
			spm_reg_write(0x08, val);

			/* write mfg1_hier_sft_sram_pd_l = 0x0000
			 */
			spm_clear(0x2C, 9);

			/* wait mfg_hier_pp1_sram_pd == 0x0 */
			while (spm_reg_read(0x9C) != 0x0);

		}

		msleep(1);

		/* write mfg_hier_pwr_iso(bit1) = 0 */
		val = spm_reg_read(0x08);
		CLR_BIT(val, 1);
		spm_reg_write(0x08, val);

		/* write mfg_hier_pwr_rst_(bit0) = 1 */
		val = spm_reg_read(0x08);
		SET_BIT(val, 0);
		spm_reg_write(0x08, val);

		/* wait rg_mfg_mem_pd_ack == 0 */
		while (spm_reg_read(0x24) & (1 << 16));

		/* wait rg_mfg_mem_pd_ack_2nd == 0 */
		while (spm_reg_read(0x28) & (1 << 16));
	} else { //power off

		if (sw_hw_sel) { //power off with hardware control

			/* write mfg_hier_mem_pd_sel(bit6) = 1
			 * write mfg_hier_mem_pd_hw(bit5) = 1
			 */
			val = spm_reg_read(0x08);
			SET_BIT(val, 5);
			SET_BIT(val, 6);
			spm_reg_write(0x08, val);

			/* wait mfg_mem_pd_ack = 1 */
			while (!(spm_reg_read(0x7C) & (1 << 20)));

			/* write mfg1_hier_mem_pd_sel(bit14) = 1
			 * write mfg1_hier_mem_pd_hw(bit13) = 1
			 */
			val = spm_reg_read(0x08);
			SET_BIT(val, 13);
			SET_BIT(val, 14);
			spm_reg_write(0x08, val);

			/* wait mfg1_mem_pd_ack = 1 */
			while (!(spm_reg_read(0x7C) & (1 << 16)));

		} else { //power off with software control
			/* write mfg_hier_mem_pd_sel(bit6) = 0
			 */
			val = spm_reg_read(0x08);
			CLR_BIT(val, 6);
			spm_reg_write(0x08, val);

			/* write mfg_hier_sft_sram_pd_l = 0xFFFF
			 * write mfg_hier_sft_sram_pd_h(7bits) = 0x007F
			 */
			spm_set(0x24, 16);
			spm_set(0x28, 7);

			/* wait mfg_hier_sram_pd = 0x007FFFFF */
			while(spm_reg_read(0x98) != 0x007FFFFF);

			/* write mfg1_hier_mem_pd_sel(bit14) = 0 */
			val = spm_reg_read(0x08);
			CLR_BIT(val, 14);
			spm_reg_write(0x08, val);

			/* write mfg1_hier_sft_sram_pd_l(9bits) = 0x01FF
			 */
			spm_set(0x2C, 9);

			/* wait mfg_hier_pp1_sram_pd == 1FF(9bits) */
			while (spm_reg_read(0x9C) != 0x01FF);
		}

		/* wait rg_mfg_mem_pd_ack == 1 */
		while (!(spm_reg_read(0x24) & (1 << 16)));

		/* wait rg_mfg_mem_pd_ack_2nd == 1 */
		while (!(spm_reg_read(0x28) & (1 << 16)));

		msleep(1);

		/* write mfg_hier_pwr_iso(bit1/9) = 1 */
		val = spm_reg_read(0x08);
		SET_BIT(val, 1);
		spm_reg_write(0x08, val);

		/* write mfg_hier_clock_dis(bit4/12) = 1 */
		/* write mfg_hier_pwr_rst_(bit0/8) = 0 */
		val = spm_reg_read(0x08);
		SET_BIT(val, 4);
		CLR_BIT(val, 0);
		spm_reg_write(0x08, val);

		/* write mfg_hier_pwr_on(bit2/10) = 0 */
		/* write mfg_hier_pwr_on_s(bit3/11) = 0 */
		val = spm_reg_read(0x08);
		CLR_BIT(val, 2);
		CLR_BIT(val, 3);
		spm_reg_write(0x08, val);

		/* wait mfg_pwr_on_ack(bit22/18) == 0 */
		while ((spm_reg_read(0x7C) & (1 << 22)));

		/* wait mfg_pwr_on_ack_s(bit21/17) == 0 */
		while ((spm_reg_read(0x7C) & (1 << 21)));

	}

}

static int spm_power_g3d(u32 sw_hw_sel, u32 on_off)
{
	// 0: sw sel, 1: hw sel
	// 0: power off, 1: power on
	u32 regval32;
	spm_base_va = ioremap(SPM_REG_PBASE, 0x1000);
	if (!spm_base_va) {
		pr_err("MALI: ioremap spm base failed.\n");
		return -1;
	}
	io_base_va = ioremap(IO_REG_PBASE, 0x1000);
	if (!io_base_va) {
		pr_err("MALI: ioremap io base failed.\n");
		iounmap(spm_base_va);
		return -1;
	}

 	{

		if (sw_hw_sel > 1) {
			pr_err("spm_power_g3d: sw_hw_sel(%d) is invalid.\n", sw_hw_sel);
			return -1;
		}

		if (on_off > 1) {
			pr_err("spm_power_g3d: on_off(%d) is invalid.\n", on_off);
			return -1;
		}

		/*
		* mfg reset release.
		*/
		regval32 = REG_READ32(0xC8);
		regval32 |= (1 << 31);
		REG_WRITE32(regval32, 0xC8);

		/*
		* mfg clock enable
		*/
		regval32 = REG_READ32(0xAC);
		regval32 |= (1 << 31);
		REG_WRITE32(regval32, 0xAC);

		pr_err("spm_power_g3d: %s g3d with %s control begin...\n", (on_off ? "PowerOn" : "PowerOff"), (sw_hw_sel ? "Hardware" : "Software"));

		__spm_power_g3d(sw_hw_sel, on_off);

		pr_err("spm_power_g3d: %s g3d with %s control end\n", (on_off ? "PowerOn" : "PowerOff"), (sw_hw_sel ? "Hardware" : "Software"));
 	}
	iounmap(io_base_va);
	iounmap(spm_base_va);
	return 0;
}

#if 0
static void __spm_power_vdec(u32 corex, u32 sw_hw_sel, u32 on_off)
{
	u32 val;
	u32 ctrl_bit_offset;
	u32 ack_bit_offset;
	u32 mem_ctrl_offset;

	if (corex) {
		ctrl_bit_offset = 0;
		ack_bit_offset = 29;
		mem_ctrl_offset = 0x14;
	} else {
		ctrl_bit_offset = 8;
		ack_bit_offset = 25;
		mem_ctrl_offset = 0x1C;
	}

	if (on_off) { //power on
		/* write xx_pwr_on(bit10/2) = 1 */
		val = spm_reg_read(0x04);
		SET_BIT(val, ctrl_bit_offset + 2);
		spm_reg_write(0x04, val);

		/* wait xx_pwr_on_ack(bit26/30) == 1 */
		while (!(spm_reg_read(0x7C) & (1 << (ack_bit_offset + 1))));

		x_thread_delay(1);

		/* write xx_hier_pwr_on_s(bit11/3) = 1 */
		val = spm_reg_read(0x04);
		SET_BIT(val, ctrl_bit_offset + 3);
		spm_reg_write(0x04, val);

		/* wait xx_pwr_on_ack_s(bit25/29) == 1 */
		while (!(spm_reg_read(0x7C) & (1 << (ack_bit_offset))));

		/* write xx_hier_clock_dis(bit12/4) = 0 */
		val = spm_reg_read(0x04);
		CLR_BIT(val, ctrl_bit_offset + 4);
		spm_reg_write(0x04, val);

		if (sw_hw_sel) { //power on with hardware control
			/* write xx_hier_mem_pd_sel(bit14/6) = 1
			 * write xx_hier_mem_pd_hw(bit13/5) = 0
			 */
			val = spm_reg_read(0x04);
			CLR_BIT(val, ctrl_bit_offset + 5);
			SET_BIT(val, ctrl_bit_offset + 6);
			spm_reg_write(0x04, val);
		} else { //power on with software control
			/* write xx_hier_mem_pd_sel(bit14/6) = 0 */
			val = spm_reg_read(0x04);
			CLR_BIT(val, ctrl_bit_offset + 6);
			spm_reg_write(0x04, val);

			/* write xx_hier_sft_sram_pd_l = 0x0000
			 * write xx_hier_sft_sram_pd_h = 0x0000
			 */
			spm_clear(mem_ctrl_offset, 16);
			spm_clear(mem_ctrl_offset + 4, 16);
		}

		x_thread_delay(1);

		/* write xx_hier_pwr_iso(bit9/1) = 0 */
		val = spm_reg_read(0x04);
		CLR_BIT(val, ctrl_bit_offset + 1);
		spm_reg_write(0x04, val);

		/* write mfg_hier_pwr_rst_(bit8/0) = 1 */
		val = spm_reg_read(0x04);
		SET_BIT(val, ctrl_bit_offset);
		spm_reg_write(0x04, val);
	} else { //power off

		if (sw_hw_sel) { //power off with hardware control

			/* write xx_hier_mem_pd_sel(bit14/6) = 1
			 * write xx_hier_mem_pd_hw(bit13/5) = 1
			 */
			val = spm_reg_read(0x04);
			SET_BIT(val, ctrl_bit_offset + 5);
			SET_BIT(val, ctrl_bit_offset + 6);
			spm_reg_write(0x04, val);

		} else { //power off with software control
			/* write xx_hier_mem_pd_sel(bit14/6) = 0
			 */
			val = spm_reg_read(0x04);
			CLR_BIT(val, ctrl_bit_offset + 6);
			spm_reg_write(0x04, val);

			/* write xx_hier_sft_sram_pd_l = 0xFFFF
			 * write xx_hier_sft_sram_pd_h = 0xFFFF
			 */
			spm_set(mem_ctrl_offset, 16);
			spm_set(mem_ctrl_offset + 4, 16);
		}

		x_thread_delay(1);

		/* write xx_hier_pwr_iso(bit9/1) = 1 */
		val = spm_reg_read(0x04);
		SET_BIT(val, ctrl_bit_offset + 1);
		spm_reg_write(0x04, val);

		/* write xx_hier_clock_dis(bit12/4) = 1 */
		/* write xx_hier_pwr_rst_(bit8/0) = 0 */
		val = spm_reg_read(0x04);
		SET_BIT(val, ctrl_bit_offset + 4);
		CLR_BIT(val, ctrl_bit_offset);
		spm_reg_write(0x04, val);

		/* write xx_hier_pwr_on(bit10/2) = 0 */
		/* write xx_hier_pwr_on_s(bit11/3) = 0 */
		val = spm_reg_read(0x04);
		CLR_BIT(val, ctrl_bit_offset + 2);
		CLR_BIT(val, ctrl_bit_offset + 3);
		spm_reg_write(0x04, val);

		/* wait xx_pwr_on_ack(bit26/30) == 0 */
		while ((spm_reg_read(0x7C) & (1 << (ack_bit_offset + 1))));

		/* wait xx_pwr_on_ack_s(bit25/29) == 0 */
		while ((spm_reg_read(0x7C) & (1 << (ack_bit_offset))));

	}
}

static INT32 spm_power_vdec(INT32 i4Argc, const CHAR ** szArgv)
{
	u32 sw_hw_sel = 0; // 0: sw sel, 1: hw sel
	u32 on_off = 0; // 0: power off, 1: power on
	u32 corex = 0; // 0:vdec-core0, 1:vdec-core1

	if (i4Argc == 4) {
		corex = StrToInt(szArgv[1]);
		sw_hw_sel = StrToInt(szArgv[2]);
		on_off = StrToInt(szArgv[3]);

		if (corex > 1) {
			pr_err("spm_power_vdec: corex(%d) is invalid.\n", corex);
			return -1;
		}

		if (sw_hw_sel > 1) {
			pr_err("spm_power_vdec: sw_hw_sel(%d) is invalid.\n", sw_hw_sel);
			return -1;
		}

		if (on_off > 1) {
			pr_err("spm_power_vdec: on_off(%d) is invalid.\n", on_off);
			return -1;
		}

		pr_err("spm_power_vdec: %s vdec%d with %s control begin...\n", (on_off ? "PowerOn" : "PowerOff"), corex, (sw_hw_sel ? "Hardware" : "Software"));

		__spm_power_vdec(corex, sw_hw_sel, on_off);

		pr_err("spm_power_vdec: %s vdec%d with %s control end\n", (on_off ? "PowerOn" : "PowerOff"), corex, (sw_hw_sel ? "Hardware" : "Software"));
 	} else {
		pr_err("Usage: spm_power_vdec [corex:0/1] [sw_hw_sel:0/1] [off/on:0/1]\n");
		return -1;
	}

	return 0;
}

/*
* idx = 0, for arm9
* idx = 1, for msdc
* idx = 2, for usb20
* idx = 3, for ssusb
*/
static void __spm_power_sram(u32 idx, u32 sw_hw_sel, u32 on_off)
{
	u32 val;
	u32 ctrl_bit_offset;
	u32 mem_ctrl_offset;
	u32 pwr_ctrl_offset;

	if (idx == 0) { // for arm9
		ctrl_bit_offset = 13;
		mem_ctrl_offset = 0x34;
		pwr_ctrl_offset = 0x10;
	} else if (idx == 1) { // for msdc
		ctrl_bit_offset = 5;
		mem_ctrl_offset = 0x38;
		pwr_ctrl_offset = 0x10;
	} else if (idx == 2) { // for usb20
		ctrl_bit_offset = 13;
		mem_ctrl_offset = 0x3C;
		pwr_ctrl_offset = 0x0C;
	} else if (idx == 3) { // for ssusb
		ctrl_bit_offset = 5;
		mem_ctrl_offset = 0x40;
		pwr_ctrl_offset = 0x0C;
	}

	if (on_off) { //power on sram

		if (sw_hw_sel) { //power on with hardware control
			/*
			 * write xx_mem_pd_sel(bit14/6) = 1
			 * write xx_mem_pd_hw(bit13/5) = 0
			 */
			val = spm_reg_read(pwr_ctrl_offset);
			CLR_BIT(val, ctrl_bit_offset);
			SET_BIT(val, ctrl_bit_offset + 1);
			spm_reg_write(pwr_ctrl_offset, val);
		} else { // power on with software control
			/*
			 * write xx_mem_pd_sel(bit14/6) = 0
			 */
			val = spm_reg_read(pwr_ctrl_offset);
			CLR_BIT(val, ctrl_bit_offset + 1);
			spm_reg_write(pwr_ctrl_offset, val);

			/*
			 * write xx_hier_sft_sram_pd_l = 0x0000
			 */
			spm_clear(mem_ctrl_offset, 16);
		}

		x_thread_delay(1);

	} else { //power off sram

		if (sw_hw_sel) { //power off with hardware control
			/*
			 * write xx_mem_pd_sel(bit14/6) = 1
			 * write xx_mem_pd_hw(bit13/5) = 1
			 */
			val = spm_reg_read(pwr_ctrl_offset);
			SET_BIT(val, ctrl_bit_offset);
			SET_BIT(val, ctrl_bit_offset + 1);
			spm_reg_write(pwr_ctrl_offset, val);
		} else { // power off with software control
			/*
			 * write xx_mem_pd_sel(bit14/6) = 0
			 */
			val = spm_reg_read(pwr_ctrl_offset);
			SET_BIT(val, ctrl_bit_offset + 1);
			spm_reg_write(pwr_ctrl_offset, val);

			/*
			 * write xx_hier_sft_sram_pd_l = 0xFFFF
			 */
			spm_set(mem_ctrl_offset, 16);

			///* wait arm9_sram_pd == 0xFFFF(16bits) */
			//while (SPM_READ32(0xAC) != 0xFFFF);
		}

		x_thread_delay(1);
	}

}


static INT32 spm_power_arm9(INT32 i4Argc, const CHAR ** szArgv)
{
	u32 sw_hw_sel = 0; // 0: sw sel, 1: hw sel
	u32 on_off = 0; // 0: power off, 1: power on

 	if (i4Argc == 3) {
		sw_hw_sel = StrToInt(szArgv[1]);
		on_off = StrToInt(szArgv[2]);

		if (sw_hw_sel > 1) {
			pr_err("spm_power_arm9: sw_hw_sel(%d) is invalid.\n", sw_hw_sel);
			return -1;
		}

		if (on_off > 1) {
			pr_err("spm_power_arm9: on_off(%d) is invalid.\n", on_off);
			return -1;
		}

		pr_err("spm_power_arm9: %s arm9 with %s control begin...\n", (on_off ? "PowerOn" : "PowerOff"), (sw_hw_sel ? "Hardware" : "Software"));
		__spm_power_sram(0, sw_hw_sel, on_off);
		pr_err("spm_power_arm9: %s arm9 with %s control end\n", (on_off ? "PowerOn" : "PowerOff"), (sw_hw_sel ? "Hardware" : "Software"));

	} else {
		pr_err("Usage: spm_power_arm9 [sw_hw_sel:0/1] [off/on:0/1]\n");
		return -1;
	}

	return 0;
}

static INT32 spm_power_msdc(INT32 i4Argc, const CHAR ** szArgv)
{
	u32 sw_hw_sel = 0; // 0: sw sel, 1: hw sel
	u32 on_off = 0; // 0: power off, 1: power on

 	if (i4Argc == 3) {
		sw_hw_sel = StrToInt(szArgv[1]);
		on_off = StrToInt(szArgv[2]);

		if (sw_hw_sel > 1) {
			pr_err("spm_power_msdc: sw_hw_sel(%d) is invalid.\n", sw_hw_sel);
			return -1;
		}

		if (on_off > 1) {
			pr_err("spm_power_msdc: on_off(%d) is invalid.\n", on_off);
			return -1;
		}

		pr_err("spm_power_msdc: %s msdc with %s control begin...\n", (on_off ? "PowerOn" : "PowerOff"), (sw_hw_sel ? "Hardware" : "Software"));
		__spm_power_sram(1, sw_hw_sel, on_off);
		pr_err("spm_power_msdc: %s msdc with %s control end\n", (on_off ? "PowerOn" : "PowerOff"), (sw_hw_sel ? "Hardware" : "Software"));

	} else {
		pr_err("Usage: spm_power_msdc [sw_hw_sel:0/1] [off/on:0/1]\n");
		return -1;
	}

	return 0;
}

static INT32 spm_power_usb20(INT32 i4Argc, const CHAR ** szArgv)
{
	u32 sw_hw_sel = 0; // 0: sw sel, 1: hw sel
	u32 on_off = 0; // 0: power off, 1: power on

 	if (i4Argc == 3) {
		sw_hw_sel = StrToInt(szArgv[1]);
		on_off = StrToInt(szArgv[2]);

		if (sw_hw_sel > 1) {
			pr_err("spm_power_usb20: sw_hw_sel(%d) is invalid.\n", sw_hw_sel);
			return -1;
		}

		if (on_off > 1) {
			pr_err("spm_power_usb20: on_off(%d) is invalid.\n", on_off);
			return -1;
		}

		pr_err("spm_power_usb20: %s usb20 with %s control begin...\n", (on_off ? "PowerOn" : "PowerOff"), (sw_hw_sel ? "Hardware" : "Software"));
		__spm_power_sram(2, sw_hw_sel, on_off);
		pr_err("spm_power_usb20: %s usb20 with %s control end\n", (on_off ? "PowerOn" : "PowerOff"), (sw_hw_sel ? "Hardware" : "Software"));

	} else {
		pr_err("Usage: spm_power_usb20 [sw_hw_sel:0/1] [off/on:0/1]\n");
		return -1;
	}

	return 0;
}

static INT32 spm_power_ssusb(INT32 i4Argc, const CHAR ** szArgv)
{
	u32 sw_hw_sel = 0; // 0: sw sel, 1: hw sel
	u32 on_off = 0; // 0: power off, 1: power on

 	if (i4Argc == 3) {
		sw_hw_sel = StrToInt(szArgv[1]);
		on_off = StrToInt(szArgv[2]);

		if (sw_hw_sel > 1) {
			pr_err("spm_power_ssusb: sw_hw_sel(%d) is invalid.\n", sw_hw_sel);
			return -1;
		}

		if (on_off > 1) {
			pr_err("spm_power_ssusb: on_off(%d) is invalid.\n", on_off);
			return -1;
		}

		pr_err("spm_power_ssusb: %s ssusb with %s control begin...\n", (on_off ? "PowerOn" : "PowerOff"), (sw_hw_sel ? "Hardware" : "Software"));
		__spm_power_sram(3, sw_hw_sel, on_off);
		pr_err("spm_power_ssusb: %s ssusb with %s control end\n", (on_off ? "PowerOn" : "PowerOff"), (sw_hw_sel ? "Hardware" : "Software"));

	} else {
		pr_err("Usage: spm_power_ssusb [sw_hw_sel:0/1] [off/on:0/1]\n");
		return -1;
	}

	return 0;
}



static void __spm_power_all(u32 sw_hw_sel, u32 on_off)
{
	__spm_power_core(1, on_off);
	__spm_power_core(2, on_off);
	__spm_power_core(3, on_off);
	__spm_power_core(4, on_off);
	__spm_power_core(5, on_off);
	__spm_power_core(6, on_off);
	__spm_power_core(7, on_off);

	__spm_power_g3d(sw_hw_sel, on_off);
	__spm_power_vdec(0, sw_hw_sel, on_off);
	__spm_power_vdec(1, sw_hw_sel, on_off);

	__spm_power_sram(0, sw_hw_sel, on_off); //arm9
	__spm_power_sram(1, sw_hw_sel, on_off); //msdc
	__spm_power_sram(2, sw_hw_sel, on_off); //usb20
	__spm_power_sram(3, sw_hw_sel, on_off); //ssusb
}


static INT32 spm_power_all(INT32 i4Argc, const CHAR ** szArgv)
{
	u32 sw_hw_sel = 0; // 0: sw sel, 1: hw sel
	u32 on_off = 0; // 0: power off, 1: power on

	if (i4Argc == 3) {
		sw_hw_sel = StrToInt(szArgv[1]);
		on_off = StrToInt(szArgv[2]);

		if (sw_hw_sel > 1) {
			pr_err("spm_power_all: sw_hw_sel(%d) is invalid.\n", sw_hw_sel);
			return -1;
		}

		if (on_off > 1) {
			pr_err("spm_power_all: on_off(%d) is invalid.\n", on_off);
			return -1;
		}

		pr_err("spm_power_all: %s all with %s control begin...\n", (on_off ? "PowerOn" : "PowerOff"), (sw_hw_sel ? "Hardware" : "Software"));
		__spm_power_all(sw_hw_sel, on_off);
		pr_err("spm_power_all: %s all with %s control end\n", (on_off ? "PowerOn" : "PowerOff"), (sw_hw_sel ? "Hardware" : "Software"));
	} else {
		pr_err("Usage: spm_power_all [sw_hw_sel:0/1] [off/on:0/1]\n");
		return -1;
	}

	return 0;
}

static INT32 spm_wkup_arm9(INT32 i4Argc, const CHAR ** szArgv)
{
	u32 val;

	pr_err("spm_wkup_arm9 begin ...\n");

	val = REG_READ32(0x00008170); //BIM
	val |= 1;
	REG_WRITE32(val, 0x00008170); //BIM
	pr_err("spm_wkup_arm9 end\n");

	return 0;
}

/*
*
* the following code is just only for memory noncache alloc test.
*
*/
static INT32 spm_dma_mem_alloc(INT32 i4Argc, const CHAR ** szArgv)
{
	int *cpu_addr;
	dma_addr_t dma_addr;

	pr_err("spm_dma_mem_alloc ...\n");

	cpu_addr = (int *)dma_alloc_coherent(NULL, 4*1024, &dma_addr, GFP_KERNEL);

	printk("cpu_addr = 0x%lx, dma_addr = 0x%lx\n", (unsigned long)cpu_addr, (unsigned long)dma_addr);

	dma_free_coherent(NULL, 4*1024, cpu_addr, dma_addr);
	pr_err("spm_dma_mem_alloc end\n");

	return 0;
}

static INT32 spm_ch2_mem_alloc(INT32 i4Argc, const CHAR ** szArgv)
{
	int *addr;

	pr_err("spm_ch2_mem_alloc ...\n");

	addr = _x_mem_ch2_alloc(4*1024);

	printk("addr = %lx\n", (unsigned long)addr);

	_x_mem_free (addr);
	pr_err("spm_ch2_mem_alloc end\n");

	return 0;
}


#define FREQ_METER_CTRL	(0xFD0001D0)
#define FREQ_METER_CNT	(0xFD0001D4)
#define RING_OSC_CFG		(0xFD0000C0)
#define ARM_OSC_OUT_CFG	(0xFD008780)

#define AU_PERI_IDX		(0)
#define DVD_MALI_IDX		(1)
#define IMGIP_IDX			(2)
#define VDEC_IDX			(3)
#define CKGEN_IDX			(4)
#define ARM11_IDX		(5)
#define ARM_JIT_IDX		(6)

static void ring_osc_abist_setting(void)
{

	/* clear and set value  */
	(*((volatile unsigned int *)(FREQ_METER_CTRL))) = 0xA9D18171;

	/* toggle */
	(*((volatile unsigned int *)(FREQ_METER_CTRL))) = 0x99D18171;

	/* finish toggle  */
	(*((volatile unsigned int *)(FREQ_METER_CTRL))) = 0x89D18171;

}

static int freq_meter_get(void)
{
	u32 val;

	do {
		val = (*((volatile unsigned int *)(FREQ_METER_CNT)));
	} while (!(val & (1 << 29)));

	return val & (0x00000FFF);
}

static void _ring_osc_setting(u32 idx)
{
	u32 val;

	switch (idx) {
	case AU_PERI_IDX:
		val = (*((volatile unsigned int *)(RING_OSC_CFG)));
		val &= (~(0x1F));
		val |= (0x0F);
		val &= (~(7 << 29));
		val |= (2 << 29);
		(*((volatile unsigned int *)(RING_OSC_CFG))) = val;
		break;
	case DVD_MALI_IDX:
		val = (*((volatile unsigned int *)(RING_OSC_CFG)));
		val &= (~(0x1F << 5));
		val |= (0x0F << 5);
		val &= (~(7 << 29));
		val |= (3 << 29);
		(*((volatile unsigned int *)(RING_OSC_CFG))) = val;
		break;
	case IMGIP_IDX:
		val = (*((volatile unsigned int *)(RING_OSC_CFG)));
		val &= (~(0x1F << 10));
		val |= (0x0F << 10);
		val &= (~(7 << 29));
		(*((volatile unsigned int *)(RING_OSC_CFG))) = val;
		break;
	case VDEC_IDX:
		val = (*((volatile unsigned int *)(RING_OSC_CFG)));
		val &= (~(0x1F << 15));
		val |= (0x0F << 15);
		val &= (~(7 << 29));
		val |= (1 << 29);
		(*((volatile unsigned int *)(RING_OSC_CFG))) = val;
		break;
	case CKGEN_IDX:
		val = (*((volatile unsigned int *)(RING_OSC_CFG)));
		val &= (~(0x1F << 24));
		val |= (0x0F << 24);
		val &= (~(7 << 29));
		val |= (4 << 29);
		(*((volatile unsigned int *)(RING_OSC_CFG))) = val;
		break;
	case ARM11_IDX:
		val = (*((volatile unsigned int *)(ARM_OSC_OUT_CFG)));
		val &= (~(0x1F << 7));
		val |= (0x0F << 7);
		(*((volatile unsigned int *)(ARM_OSC_OUT_CFG))) = val;
		val = (*((volatile unsigned int *)(RING_OSC_CFG)));
		val &= (~(7 << 29));
		val |= (5 << 29);
		(*((volatile unsigned int *)(RING_OSC_CFG))) = val;
		break;
	case ARM_JIT_IDX:
		val = (*((volatile unsigned int *)(RING_OSC_CFG)));
		val &= (~(7 << 29));
		val |= (6 << 29);
		(*((volatile unsigned int *)(RING_OSC_CFG))) = val;
		break;
	default:
		break;
	}

}


static INT32 ring_osc_setting(INT32 i4Argc, const CHAR ** szArgv)
{
	u32 i = 0;
	u32 fm_cnt = 0;

	pr_err("\n");
	for (i = 0; i < 6; i++) {
		_ring_osc_setting(i);
		ring_osc_abist_setting();
		fm_cnt = freq_meter_get();
		pr_err("i = %d, FM_CNT=%d\n", i, fm_cnt);
	}

	return 0;
}

static INT32 mem_test(INT32 i4Argc, const CHAR ** szArgv)
{
	unsigned int *ch1_addr;
	//unsigned int *ch2_addr;
	unsigned int size =  1 * 1024 * 1024; //1M
	unsigned int i;
	unsigned long bus_addr;

	u64 a;
	u64 b;
	u64 c;

	pr_err("ch1_mem_alloc ...\n");
	ch1_addr = _x_mem_alloc(size);

	x_memset(ch1_addr, 0, 0x100);

	for (i = 0;  i < size/4;  i++) {
		*(ch1_addr + i) = i;
	}

	b = 0x654321789LL;
	a = (u64)ch1_addr + 0x999999999999LL;
	c = a / b;
	printk("a64/b64=%lld\n", c);

	bus_addr = VIRT_TO_BUS((unsigned long)ch1_addr);
	printk("ch1_addr(virtual) = 0x%lx, ch1_addr(physical) = 0x%lx\n, bus_addr = 0x%lx",
		    (unsigned long)ch1_addr, (unsigned long)(__virt_to_phys((unsigned long)ch1_addr)), bus_addr);


	ch1_addr = x_alloc_aligned_dma_mem(1024*1024, 128);
	for (i = 0;  i < size/4;  i++) {
		*(ch1_addr + i) = i;
	}

	x_free_aligned_dma_mem(ch1_addr);

#if 0
	pr_err("ch2_mem_alloc ...\n");
	ch2_addr = _x_mem_ch2_alloc(size);
	printk("ch2_addr(virtual) = 0x%x, ch2_addr(physical) = 0x%x\n", (unsigned int)ch2_addr, (unsigned int)ch2_addr - 0xF2000000 + 580*1024*1024);

	pr_err("copy ch2_mem to ch1_mem start ...\n");
	for (i = 0;  i < size/4;  i++) {
		*(ch2_addr + i) = i;
	}

	for (i = 0;  i < size/4;  i++) {
		*(ch1_addr + i) = i;
	}

	for (i = 0;  i < size/4;  i++) {
		if (*(ch2_addr + i) != i) {
			pr_err("data check error. ch2_addr(%p)\n", ch2_addr + i);
			while(1);
		}
	}
	pr_err("ch2 memory check ok.\n");

	for (i = 0;  i < size/4;  i++) {
		if (*(ch1_addr + i) != i) {
			pr_err("data check error. ch1_addr(%p)\n", ch1_addr + i);
			while(1);
		}
	}
	pr_err("ch1 memory check ok.\n");

	for (i = 0;  i < size/4;  i++) {
		if (*(ch1_addr + i) != i) {
			pr_err("data check errorx. ch1_addr(%p)\n", ch1_addr + i);
			while(1);
		}

		if (*(ch2_addr + i) != i) {
			pr_err("data check errorx. ch2_addr(%p)\n", ch2_addr + i);
			while(1);
		}
	}
	pr_err("ch1 and ch2 memory check ok.\n");

	for (i = 0;  i < size/4;  i++) {
		if (*(ch1_addr + i) != *(ch2_addr + i)) {
			pr_err("data check error. ch1_addr(%p), ch2_addr(%p)\n", ch1_addr + i, ch2_addr + i);
			while(1);
		}
	}
	pr_err("ch1 and ch2 memory checkx ok.\n");

	for (i = 0;  i < size/4;  i++) {
		*(ch2_addr + i) = i + 7;
	}

	for (i = 0;  i < size/4;  i++) {
		*(ch1_addr + i) = *(ch2_addr + i);
	}

	for (i = 0;  i < size/4;  i++) {
		if (*(ch1_addr + i) != *(ch2_addr + i)) {
			pr_err("data check errorx. ch1_addr(%p), ch2_addr(%p)\n", ch1_addr + i, ch2_addr + i);
			while(1);
		}
	}
	pr_err("ch2 to ch1 memory check ok.\n");

	for (i = 0;  i < size/4;  i++) {
		*(ch1_addr + i) = i + 2;
	}

	for (i = 0;  i < size/4;  i++) {
		*(ch2_addr + i) = *(ch1_addr + i);
	}

	for (i = 0;  i < size/4;  i++) {
		if (*(ch1_addr + i) != *(ch2_addr + i)) {
			pr_err("data check errorxx. ch1_addr(%p), ch2_addr(%p)\n", ch1_addr + i, ch2_addr + i);
			while(1);
		}
	}
	pr_err("ch1 to ch2 memory check ok.\n");

	_x_mem_free (ch1_addr);
	_x_mem_free (ch2_addr);

#endif

	return 0;
}

#define GPIOEN0		0x74
#define GPIOOUT0		0xE0
#define GPIOIN0		0x100

static atomic_t time_expire;

static void __gpio_test1(void)
{
	u32 rval;

	while (1) {
		/* configure GPIO3/5 as output function */
		/* configure GPIO4/6 as input function */
		rval = REG_READ32(GPIOEN0);
		rval |=((0x1 << 3) | (0x1 << 5));
		rval &= (~((0x1 << 4) | (0x1 << 6)));
		REG_WRITE32(rval, GPIOEN0);

		/* make GPIO3/5 ouput 1 */
		rval = REG_READ32(GPIOOUT0);
		rval |=( (0x1 << 3) | (0x1 << 5));
		REG_WRITE32(rval, GPIOOUT0);

		/* read GPIO4/6 */
		rval = REG_READ32(GPIOIN0);
		if ((rval & (0x1 << 4))  != (0x1 << 4)) {
			pr_err("ERROR: GPIO4 is 0, but expect 1.\n");
			break;
		}

		if ((rval & (0x1 << 6))  != (0x1 << 6)) {
			pr_err("ERROR: GPIO6 is 0, but expect 1.\n");
			break;
		}

		/* make GPIO3/5 ouput 0 */
		rval = REG_READ32(GPIOOUT0);
		rval &=(~( (0x1 << 3) | (0x1 << 5)));
		REG_WRITE32(rval, GPIOOUT0);

		/* read GPIO4/6 */
		rval = REG_READ32(GPIOIN0);
		if ((rval & (0x1 << 4))  != 0) {
			pr_err("ERROR: GPIO4 is 1, but expect 0.\n");
			break;
		}

		if ((rval & (0x1 << 6))  != 0) {
			pr_err("ERROR: GPIO6 is 1, but expect 0.\n");
			break;
		}

		/* configure GPIO4/6 as output function */
		/* configure GPIO3/5 as input function */
		rval = REG_READ32(GPIOEN0);
		rval |=((0x1 << 4) | (0x1 << 6));
		rval &= (~((0x1 << 3) | (0x1 << 5)));
		REG_WRITE32(rval, GPIOEN0);

		/* make GPIO4/6 ouput 1 */
		rval = REG_READ32(GPIOOUT0);
		rval |=( (0x1 << 4) | (0x1 << 6));
		REG_WRITE32(rval, GPIOOUT0);

		/* read GPIO3/5 */
		rval = REG_READ32(GPIOIN0);
		if ((rval & (0x1 << 3))  != (0x1 << 3)) {
			pr_err("ERROR: GPIO3 is 0, but expect 1.\n");
			break;
		}

		if ((rval & (0x1 << 5))  != (0x1 << 5)) {
			pr_err("ERROR: GPIO5 is 0, but expect 1.\n");
			break;
		}

		/* make GPIO4/6 ouput 0 */
		rval = REG_READ32(GPIOOUT0);
		rval &=(~( (0x1 << 4) | (0x1 << 6)));
		REG_WRITE32(rval, GPIOOUT0);

		/* read GPIO3/5 */
		rval = REG_READ32(GPIOIN0);
		if ((rval & (0x1 << 3))  != 0) {
			pr_err("ERROR: GPIO3 is 1, but expect 0.\n");
			break;
		}

		if ((rval & (0x1 << 5))  != 0) {
			pr_err("ERROR: GPIO5 is 1, but expect 0.\n");
			break;
		}

		if (atomic_read(&time_expire)) {
			atomic_set(&time_expire, 0);
			pr_err("-----> test is ongoing ...<-----\n");
		}

	}
}

static void __gpio_test2(void)
{
	u32 rval;

	while (1) {
		/* configure GPIO4/6 as output function */
		/* configure GPIO5/7 as input function */
		rval = REG_READ32(GPIOEN0);
		rval |=((0x1 << 4) | (0x1 << 6));
		rval &= (~((0x1 << 5) | (0x1 << 7)));
		REG_WRITE32(rval, GPIOEN0);

		/* make GPIO4/6 ouput 1 */
		rval = REG_READ32(GPIOOUT0);
		rval |=( (0x1 << 4) | (0x1 << 6));
		REG_WRITE32(rval, GPIOOUT0);

		/* read GPIO5/7 */
		rval = REG_READ32(GPIOIN0);
		if ((rval & (0x1 << 5))  != (0x1 << 5)) {
			pr_err("ERROR: GPIO5 is 0, but expect 1.\n");
			break;
		}

		if ((rval & (0x1 << 7))  != (0x1 << 7)) {
			pr_err("ERROR: GPIO7 is 0, but expect 1.\n");
			break;
		}

		/* make GPIO4/6 ouput 0 */
		rval = REG_READ32(GPIOOUT0);
		rval &=(~( (0x1 << 4) | (0x1 << 6)));
		REG_WRITE32(rval, GPIOOUT0);

		/* read GPIO5/7 */
		rval = REG_READ32(GPIOIN0);
		if ((rval & (0x1 << 5)) != 0) {
			pr_err("ERROR: GPIO5 is 1, but expect 0.\n");
			break;
		}

		if ((rval & (0x1 << 7)) != 0) {
			pr_err("ERROR: GPIO7 is 1, but expect 0.\n");
			break;
		}

		/* configure GPIO5/7 as output function */
		/* configure GPIO4/6 as input function */
		rval = REG_READ32(GPIOEN0);
		rval |=((0x1 << 5) | (0x1 << 7));
		rval &= (~((0x1 << 4) | (0x1 << 6)));
		REG_WRITE32(rval, GPIOEN0);

		/* make GPIO5/7 ouput 1 */
		rval = REG_READ32(GPIOOUT0);
		rval |=( (0x1 << 5) | (0x1 << 7));
		REG_WRITE32(rval, GPIOOUT0);

		/* read GPIO4/6 */
		rval = REG_READ32(GPIOIN0);
		if ((rval & (0x1 << 4))  != (0x1 << 4)) {
			pr_err("ERROR: GPIO4 is 0, but expect 1.\n");
			break;
		}

		if ((rval & (0x1 << 6))  != (0x1 << 6)) {
			pr_err("ERROR: GPIO6 is 0, but expect 1.\n");
			break;
		}

		/* make GPIO5/7 ouput 0 */
		rval = REG_READ32(GPIOOUT0);
		rval &=(~( (0x1 << 5) | (0x1 << 7)));
		REG_WRITE32(rval, GPIOOUT0);

		/* read GPIO4/6 */
		rval = REG_READ32(GPIOIN0);
		if ((rval & (0x1 << 4))  != 0) {
			pr_err("ERROR: GPIO4 is 1, but expect 0.\n");
			break;
		}

		if ((rval & (0x1 << 6))  != 0) {
			pr_err("ERROR: GPIO6 is 1, but expect 0.\n");
			break;
		}

		if (atomic_read(&time_expire)) {
			atomic_set(&time_expire, 0);
			pr_err("-----> test is ongoing ...<-----\n");
		}

	}
}

#define TIME_XSECOND	(5 * HZ)

static struct timer_list timer;

static void timer_handle(unsigned long arg)
{
	mod_timer(&timer, jiffies + TIME_XSECOND);
	atomic_set(&time_expire, 1);
}

static INT32 gpio_test(INT32 i4Argc, const CHAR ** szArgv)
{
	int idx;

	if (i4Argc == 2) {

		idx = StrToInt(szArgv[1]);

		if (idx == 1 || idx == 2) {
			init_timer(&timer);
			timer.function = &timer_handle;
			timer.expires = jiffies + TIME_XSECOND;
			add_timer(&timer);
			atomic_set(&time_expire, 0);
		}

		if (idx == 1) {
			pr_err("Start testing with GPIO3<-->GPIO4 and GPIO5<-->GPIO6\n");
			__gpio_test1();
		} else if (idx == 2) {
			pr_err("Start testing with GPIO4<-->GPIO5 and GPIO6<-->GPIO7\n");
			__gpio_test2();
		} else {
			pr_err("Usage: gpio_test [1/2]\n");
			return -1;
		}

	} else {
		pr_err("Usage: gpio_test [1/2]\n");
		return -1;
	}

	return 0;
}

static void __gpio_testx(void)
{
	u32 rval;

	/* configure GPIO3/4/5/6/7 as output function */
	rval = REG_READ32(GPIOEN0);
	rval |= (0x1F << 3);
	REG_WRITE32(rval, GPIOEN0);

	while (1) {
		/* make GPIO3/4/5/6/7 ouput 1 */
		rval = REG_READ32(GPIOOUT0);
		rval |= (0x1F << 3);
		REG_WRITE32(rval, GPIOOUT0);

		mdelay(50);

		/* make GPIO3/4/5/6/7 ouput 0 */
		rval = REG_READ32(GPIOOUT0);
		rval &= (~(0x1F << 3));
		REG_WRITE32(rval, GPIOOUT0);

		mdelay(50);
	}
}

static INT32 gpio_testx(INT32 i4Argc, const CHAR ** szArgv)
{
	__gpio_testx();

	return 0;
}

#define BUF_LEN	256
static char buf[BUF_LEN];
static INT32 semihosting_test(INT32 i4Argc, const CHAR ** szArgv)
{
	FILE *fp;
	int i;

	if ((fp = fopen("D:\\Test_Data\\Test.bin","rb")) != NULL) {
		printk("Open Test.bin Succeed!\n");
		fread((void *)buf, sizeof(char), BUF_LEN, fp);

		fclose(fp);
	} else {
		printk("Open Test.bin Failed!\n");
	}

	for(i = 0; i < BUF_LEN; i++) {
		if (i % 16 == 0)
			pr_err("\n");

		pr_err("%4x", buf[i]);
	}
	pr_err("\n");

	if ((fp = fopen("D:\\Test_Data\\WriteTest.bin","wb")) != NULL) {
		pr_err("Open WriteTest.bin Succeed!\n");
		fwrite((void *)buf, sizeof(char), BUF_LEN, fp);

		fclose(fp);
	} else {
		pr_err("Open WriteTest.bin Failed!\n");
	}

	return 0;
}


/*
* for udvt test.
*/
INT32 udvt_access_file_test(INT32 i4Argc, const CHAR ** szArgv)
{
	UINT8 *pData;
	u32 fHandle;
	u32 ReadLength;
	u32 fileLength;

	u32 n;
	u32 checksum = 0;

	//pData = x_alloc_aligned_dma_mem(0x100000,4);

	pData = x_alloc_aligned_dma_mem(0x1000000,4);

	if (pData == NULL) {
		Printf("[UDVT]allocate pData fail.\n ");
		return -1;
	} else {
		Printf("[UDVT] pData = %p.\n ", pData);
	}

	if (i4Argc < 3) {
		Printf("[UDVT]fa [filename] [mode]\n");
		return -1;
	}

	fHandle = UDVT_IF_OpenFile(szArgv[1],szArgv[2]);
	if (fHandle == 0) {
		Printf("[UDVT] Open pc file %s fail!\n",szArgv[1]);
	} else {
		Printf("[UDVT] Open pc file %s success!\n",szArgv[1]);
	}

	fileLength = UDVT_IF_GetFileLength(fHandle);
	Printf("file length is %d\n",fileLength);

	while(fileLength > 0) {
		if (fileLength >= 0x100000) {
			ReadLength = UDVT_IF_ReadFile(pData,1,0x100000,fHandle);
			if (ReadLength != 0x100000) {
				Printf("error occur at %d\n",fileLength);
				break;
			} else {
				fileLength -= ReadLength;
			}
			pData += 0x100000;
		} else {
			ReadLength = UDVT_IF_ReadFile(pData,1,fileLength,fHandle);
			if (ReadLength != fileLength) {
				Printf("error occur at %d\n",fileLength);
				break;
			} else {
				fileLength -= ReadLength;
			}
		}
	}

	Printf("\n[UDVT] Read %d btyes,checksum is %d\n",fileLength,checksum);
	UDVT_IF_CloseFile(fHandle);

	//x_free_aligned_dma_mem(pData);
	UDVT_IF_SendResult(UDVT_TEST_PASS);

	return 0;
}
#endif
#include <linux/clk.h>
#include <linux/clk-private.h>

struct clk *g3d_clk;
struct clk *g3d_parent_clk;

void g3d_power_on(void)
{
	int ret;
	unsigned long g3d_top_va;

	ret = spm_power_g3d(1, 1);
	if (ret == -1) {
		pr_err("MALI: power on failed.\n");
		return;
	}

	g3d_top_va = ioremap(IO_REG_PBASE + 0x6e294, 4);
	if (g3d_top_va == NULL) {
		pr_err("MALI error: g3d_top register io map failed.\n");
		return;
	}

	__raw_writel(__raw_readl(g3d_top_va) & 0xffff3fff, g3d_top_va);
	iounmap(g3d_top_va);

	g3d_clk = clk_get(NULL, "g3d_sel");
	if (g3d_clk == NULL) {
		pr_err("MALI error: get g3d_sel clk failed.\n");
		return;
	}

	g3d_parent_clk = clk_get(NULL, "g3dpll_ck");
	if (g3d_parent_clk == NULL) {
		pr_err("MALI error: get g3d_parent_clk clk failed.\n");
		return;
	}

	ret = clk_set_parent(g3d_clk, g3d_parent_clk);
	if (ret != 0) {
		pr_err("MALI error: clk set parent failed.\n");
		return;
	}

	ret = clk_prepare_enable(g3d_clk);
	if (ret != 0) {
		pr_err("MALI error: clk enable failed.\n");
		return;
	}
}

void g3d_power_off(void)
{
	int ret;

	ret = spm_power_g3d(1, 0);
	if (ret == -1)
		pr_err("MALI: power off failed.\n");
}
