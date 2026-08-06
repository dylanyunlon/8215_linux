/*************************************************************************/
/*****************           ATC CONFIDENTIAL            *****************/
/*****************                                       *****************/
/*****************   Description : AC83xx trustzone      *****************/
/*****************                                       *****************/
/*****************                                       *****************/
/*****************       Company : Aucochips Inc.        *****************/
/*****************       Programmer : Emily Zhang        *****************/
/*************************************************************************/

    // set gic register
    /* main cpu regs */
#define GICC_PHY_OFFSET   0xF1002000
#define GICD_PHY_OFFSET   0xF1001000
#define GICC_CTLR               (GICC_PHY_OFFSET + 0x0000)
#define GICC_PMR                (GICC_PHY_OFFSET + 0x0004)
#define GICC_BPR                (GICC_PHY_OFFSET + 0x0008)
#define GICC_IAR                (GICC_PHY_OFFSET + 0x000c)
#define GICC_EOIR               (GICC_PHY_OFFSET + 0x0010)
#define GICC_RPR                (GICC_PHY_OFFSET + 0x0014)
#define GICC_HPPIR              (GICC_PHY_OFFSET + 0x0018)
#define GICC_APBR               (GICC_PHY_OFFSET + 0x001c)
#define GICC_AIAR               (GICC_PHY_OFFSET + 0x0020)
#define GICC_AEOIR              (GICC_PHY_OFFSET + 0x0024)
#define GICC_AHPPIR             (GICC_PHY_OFFSET + 0x0028)
#define GICC_APR               (GICC_PHY_OFFSET + 0x00d0)
#define GICC_NSAPR             (GICC_PHY_OFFSET + 0x00e0)
#define GICC_IIDR               (GICC_PHY_OFFSET + 0x00fc)
#define GICC_DIR                (GICC_PHY_OFFSET + 0x1000)

#define GICC_ENABLE			0x1
#define GICC_INT_PRI_THRESHOLD		0xf0
#define GICC_DIS_BYPASS_MASK		0x1e0


/* distribution regs */
#define GICD_CTLR               (GICD_PHY_OFFSET + 0x000)
#define GICD_TYPER              (GICD_PHY_OFFSET + 0x004)
#define GICD_IIDR               (GICD_PHY_OFFSET + 0x008)
#define GICD_IGROUPR             (GICD_PHY_OFFSET + 0x080)
#define GICD_ISENABLER          (GICD_PHY_OFFSET + 0x100)
#define GICD_ICENABLER          (GICD_PHY_OFFSET + 0x180)
#define GICD_ISPENDR            (GICD_PHY_OFFSET + 0x200)
#define GICD_ICPENDR            (GICD_PHY_OFFSET + 0x280)
#define GICD_ISACTIVER          (GICD_PHY_OFFSET + 0x300)
#define GICD_ICACTIVER          (GICD_PHY_OFFSET + 0x380)
#define GICD_IPRIORITYR         (GICD_PHY_OFFSET + 0x400)
#define GICD_ITARGETSR         (GICD_PHY_OFFSET + 0x800)
#define GICD_ICFGR             (GICD_PHY_OFFSET + 0xc00)
#define GICD_NSACR             (GICD_PHY_OFFSET + 0xe00)
#define GICD_SGIR               (GICD_PHY_OFFSET + 0xf00)
#define GICD_CPENDSGIR         (GICD_PHY_OFFSET + 0xf10)
#define GICD_SPENDSGIR         (GICD_PHY_OFFSET + 0xf20)

#define GICD_ENABLE			0x3
#define GICD_DISABLE		    0x0
#define GICD_GROUP_NUM    0x1f
#define GICD_INT_ACTLOW_LVLTRIG		0x0
#define GICD_INT_EN_CLR_X32		0xffffffff
#define GICD_INT_EN_SET_SGI		0x0000ffff
#define GICD_INT_EN_CLR_PPI		0xffff0000
#define GICD_INT_DEF_PRI		0xa0
#define GICD_INT_DEF_PRI_X4		((GICD_INT_DEF_PRI << 24) |\
					(GICD_INT_DEF_PRI << 16) |\
					(GICD_INT_DEF_PRI << 8) |\
					GICD_INT_DEF_PRI)


#define NR_IRQS 256

static void _writel(unsigned long long value, unsigned long long ptr)
{
	*((volatile unsigned long long *)ptr) = value;
}

static unsigned long long _readl(unsigned long long ptr)
{
	return *((volatile unsigned long long *)ptr);
}



void gic_secure_config(void)
{
    unsigned int i;
	unsigned long long group_num;
    
    _writel(GICD_DISABLE, GICD_CTLR); // disable GIC0 ns interrupts before config
	/*
	 * Iterate through all IRQs and set them to non-secure
	 * mode. This will allow the non-secure side to handle
	 * all the interrupts we don't explicitly claim.
	 */

	/*get group register number*/
	group_num = _readl(GICD_TYPER);
	group_num &= GICD_GROUP_NUM;
	group_num += 1;    /*count begin with 0*/
	
	_writel(0x00000000, GICD_IGROUPR); //0xFE00FFFF
	
	for (i = 1; i < group_num; i += 1) {
		_writel(0xFFFFFFFF, GICD_IGROUPR + (i*4));
    }   

	_writel(GICD_ENABLE, GICD_CTLR); // enable GIC0 ns interrupts

    _writel(0x01, GICC_CTLR);
    _writel(0x80, GICC_PMR);
}

/*use for protect trustzone reserve memory*/
#include "reserved_memory.h"

/*DRAM protect register*/
/*addr must be 64Byte aligned*/

#define DMPROT_BGN  0xF0038138
#define DMPROT_END  0xF003813C
#define SECURE_CFG0 0xF0038140
#define DM_SEC_EN   0x2        /*set it to enable protected ram only accessable in SEC mode*/

void set_dram_protection_for_tz(void)
{
	unsigned long long g_tz_reserve_mem_start;
	unsigned long long g_tz_reserve_mem_end;
	unsigned long long value = 0;
	RSV_MEM_T *trustzone = get_rsv_mem_by_name("trustzone");
	if (!trustzone)
		return;
#if 0   
	UART_Printf("\nset_dram_protection for tz :%s\n", trustzone->name);
    UART_Printf("\nset_dram_protection for tz :%x\n", (int)trustzone->start_addr);
    UART_Printf("\nset_dram_protection for tz :%x\n", (int)trustzone->size);
#endif
	g_tz_reserve_mem_start = trustzone->start_addr;
	g_tz_reserve_mem_end   = g_tz_reserve_mem_start + trustzone->size; 
	_writel(g_tz_reserve_mem_start, DMPROT_BGN);
	_writel(g_tz_reserve_mem_end, DMPROT_END);
	value = _readl(SECURE_CFG0);
	_writel(value | DM_SEC_EN, SECURE_CFG0);
}


