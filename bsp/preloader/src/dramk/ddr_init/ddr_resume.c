#include "ddr.h"
#include "ddr_includes.h"
extern 	void vDDR_BLConfig(void);
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Function:       DDR_EnterResume

Description:    Enter Resume mode

Arguments:      pu4SaveDramRegAddr  - [in] SDram Save Address
                        u4CalAddStart              - [in] Dram Calibartion address start 
		
Return Value:  

-------------------------------------------------------------------*/
void DDR_EnterResume(U32 *pu4SaveDramRegAddr, U32 u4CalAddStart)
{
    U32 u4Value = 0;
    /*1. Re-Init Dram*/
    ReInitDram(pu4SaveDramRegAddr);

    /*u4Value = ucDram_Register_Read(0xF0052004); 
	mcSET_BIT(u4Value, 24);
	mcSET_BIT(u4Value, 26);
	mcSET_BIT(u4Value, 27);
	mcSET_BIT(u4Value, 28);
	mcSET_BIT(u4Value, 29);
	mcSET_BIT(u4Value, 30);
	mcSET_BIT(u4Value, 31);
	ucDram_Register_Write(0xF0052004, u4Value);
	if (DEBUG_LOG == 1)
	    mcSHOW_DBG_MSG2("\nStep 6: Set 0x52004 bit 24, 26-31\n");

	u4Value = ucDram_Register_Read(0xF0052104);
	mcSET_BIT(u4Value, 24);
	mcSET_BIT(u4Value, 25);
	mcSET_BIT(u4Value, 26);
	mcSET_BIT(u4Value, 27);
	mcSET_BIT(u4Value, 28);
	mcSET_BIT(u4Value, 29);
	mcSET_BIT(u4Value, 30);
	ucDram_Register_Write(0xF0052104, u4Value);
	if (DEBUG_LOG == 1)
	    mcSHOW_DBG_MSG2("\nStep 7: Set 0x52104 bit 24-30\n");*/
    
	/*2. Set DMSU33  dram ic power on*/
    mcDELAY_us(100);
    u4Value = ucDram_Register_Read(REG_RW_DMSUS);
    mcCLR_BIT(u4Value, BIT_DMSUS);
    ucDram_Register_Write(REG_RW_DMSUS, u4Value);
    mcDELAY_us(10000);           //add here, or datalatch calibration failed
    
    /*3. Exit Self refresh*/
    DRAMC_Exit_Suspend(u4CalAddStart);
    
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Function:       ReInitDram

Description:    Re-Initiliaze Dram

Arguments:      pu4SaveDramRegAddr  - [in] Dram Save Address
		
Return Value:  

-------------------------------------------------------------------*/
void ReInitDram(U32 *pu4SaveDramRegAddr)
{
    /*1. Set Suspend mode*/
    TCMSET_DRAM_SUSPEND();
    // re-Init...(dramc_init, dramc_config) 
    vDDR_BLConfig();
    mcSHOW_DBG_MSG2("DdrPhyInit\n");
    TCMSET_DRAM_SUSPEND();
    TCMSET_CHANNELA_ACTIVE();
    DdrPhyInit();
    //ResetDemuxLocalArbiter();
    DramcInit();     
    DramcConfig();
    DramcWriteBackReg(pu4SaveDramRegAddr);
    DramcWriteBackCommonReg(pu4SaveDramRegAddr+(PHY_BACKUP_REG_NUM+DRAMC_BACKUP_REG_NUM+DRAMRB_BACKUP_REG_NUM));
    
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Function:       DRAMC_Exit_Suspend

Description:    Exit Suspend mode

Arguments:     u4CalAddStart              - [in] Calibartion address start  
		
Return Value:  

-------------------------------------------------------------------*/
void DRAMC_Exit_Suspend(U32 u4CalAddStart)
{

    // exit self refresh
    mcSHOW_DBG_MSG2("DRAMC_Exit_Suspend\n");
    TCMSET_DRAM_SUSPEND();
    TCMSET_CHANNELA_ACTIVE();
    DramcExitSR(u4CalAddStart);
    TCMSET_DRAM_NORMAL();
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Function:       DramcExitSR

Description:    Exit Self-refresh mode

Arguments:     u4CalAddStart              - [in] Calibartion address start
		
Return Value:  

-------------------------------------------------------------------*/
void  DramcExitSR(U32 u4CalAddStart)
{
    U32 u4value = 0;
 
    //Exit self-refresh command to dram
    //exit self refresh    4h [26]
    u4value = ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_CONF1));
    mcCLR_BIT(u4value, POS_CONF1_SELFREF);
    ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_CONF1), u4value);

    //Wait at least 9*tREFI+tCKSRE+20*tMIOCK (or 1ms)
    mcDELAY_us(1);

    //reset phy R_DMPHYRST: 0xf0[28]
    // 0x0f0[28] = 1 -> 0
    u4value = ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_PHYCTL1));
    mcSET_BIT(u4value, POS_PHYCTL1_PHYRST);
    ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_PHYCTL1), u4value);
    //delay 10ns, 1ms here
    mcDELAY_us(1);
    mcCLR_BIT(u4value, POS_PHYCTL1_PHYRST);
    ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_PHYCTL1), u4value);

    //Async FIFO RESET, R_DMSYNCRST 0x1b44[6]
    u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_WRAP_ASYNC_FIFO));
    mcSET_BIT(u4value, 6);
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_WRAP_ASYNC_FIFO), u4value);
    //delay 100ns, 1ms here
    mcDELAY_us(1);
    mcCLR_BIT(u4value, 6);
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_WRAP_ASYNC_FIFO), u4value);

    // read data counter reset
    // 0x0f4[25] = 1 -> 0
    u4value = ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_GDDR3CTL1));
    mcSET_BIT(u4value, POS_GDDR3CTL1_RDATRST);
    ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_GDDR3CTL1), u4value);
    //delay 10ns, 1ms here
    mcDELAY_us(1);
    mcCLR_BIT(u4value, POS_GDDR3CTL1_RDATRST);
    ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_GDDR3CTL1), u4value);
	
    mcSHOW_DBG_MSG2("fcSET_AXI_CLK_USE_CLK_DRAM\n");
#if fcSET_AXI_CLK_USE_CLK_DRAM
	/*[important]!!! change axi_clk to clk_dram after mempll enable, otherwise can't BIM access dram */
	mcDELAY_us(1);
	u4value = ucDram_Register_Read(0x700080CC);
	mcSET_BIT(u4value, 0);
	ucDram_Register_Write(0x700080CC, u4value);
	mcDELAY_us(1);
#endif

    mcSHOW_DBG_MSG2("DRAM_RX_DATLAT_CAL\n");
    
#if 1
    
#ifdef DRAM_RX_DATLAT_CAL
    // For MT5399, add DATLAT calibration
    // Issue from MT6589, async fifo, core power off and on, DATLAT window may have 1T shift, 2012/11/28
    DramcRxdatlatCal(u4CalAddStart);
    mcDELAY_us(100);
  #endif
#endif

}


void DramcWriteBackReg(U32 *u4DRAMCTemp)
{
    U8 ii;
    U32 u4value, u4addr, u4PHY_BASE_ADDR, u4PHY_BASE_ADDR1;
    // DRAMC backup registers
    U16 gu2dramc_backup_regaddr[DRAMC_BACKUP_REG_NUM] = { DRAMC_REG_CONF2, DRAMC_REG_DELDLY1, DRAMC_REG_TEST2_1, DRAMC_REG_TEST2_2, DRAMC_REG_TEST2_3, DRAMC_REG_DDR2CTL, DRAMC_REG_PADCTL4, \
                                                              DRAMC_REG_DQIDLY1, DRAMC_REG_DQIDLY2, DRAMC_REG_DQIDLY3, DRAMC_REG_DQIDLY4, DRAMC_REG_DQIDLY5, \
                                                              DRAMC_REG_DQIDLY6, DRAMC_REG_DQIDLY7, DRAMC_REG_DQIDLY8};
	// DRAMRB backup registers
	UINT16 gu2dramrb_backup_regaddr[DRAMRB_BACKUP_REG_NUM] = {REG_RW_CONF, REG_RW_PRI0, REG_RW_PRI1, REG_RW_AGTIM0, REG_RW_AGTIM1, REG_RW_STARVE1, REG_RW_STARVE2, REG_RW_STARVE6};

    //restore all the VCCK domain registers
    if (!IS_DRAM_CHANNELB_ACTIVE())
    {
        u4PHY_BASE_ADDR = mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG0);
        u4PHY_BASE_ADDR1 = mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG0);
    }
    else
    {
        u4PHY_BASE_ADDR = mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG0);
        u4PHY_BASE_ADDR1 = mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG20);
    }

    //DRAMC registers
    for (ii=0; ii<DRAMC_BACKUP_REG_NUM; ii++)
    {
        u4value = *(u4DRAMCTemp+ii);
        ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(gu2dramc_backup_regaddr[ii]), u4value);
    }
	
    //DRAMRB registers
    for (ii=0; ii<DRAMRB_BACKUP_REG_NUM; ii++)
    {
        u4value = *(u4DRAMCTemp+ii+DRAMC_BACKUP_REG_NUM);
        ucDram_Register_Write(mcSET_ARBITER_ADDR(gu2dramrb_backup_regaddr[ii]), u4value);
    }

    //DDRPHY registers
    //---A1, B2 registers
    u4addr = u4PHY_BASE_ADDR + 0x00;
    for (ii = 0; ii <= 3; ii++)
    {
        u4value = *(u4DRAMCTemp+ii+DRAMC_BACKUP_REG_NUM+DRAMRB_BACKUP_REG_NUM);
        ucDram_Register_Write(u4addr, u4value);
        u4addr = u4addr + 4;
    }

    u4addr = u4PHY_BASE_ADDR + 0x18;
    for (ii = 4; ii <= 14; ii++)
    {
        u4value = *(u4DRAMCTemp+ii+DRAMC_BACKUP_REG_NUM+DRAMRB_BACKUP_REG_NUM);
        ucDram_Register_Write(u4addr, u4value);
        u4addr = u4addr + 4;
    }

    u4addr = u4PHY_BASE_ADDR + 0x4c;
    for (ii = 15; ii <= 43; ii++)
    {
        u4value = *(u4DRAMCTemp+ii+DRAMC_BACKUP_REG_NUM+DRAMRB_BACKUP_REG_NUM);
        ucDram_Register_Write(u4addr, u4value);
        u4addr = u4addr + 4;
    }

    u4addr = u4PHY_BASE_ADDR + 0xcc;
    for (ii = 44; ii <= 46; ii++)
    {
        u4value = *(u4DRAMCTemp+ii+DRAMC_BACKUP_REG_NUM+DRAMRB_BACKUP_REG_NUM);
        ucDram_Register_Write(u4addr, u4value);
        u4addr = u4addr + 4;
    }

    u4addr = u4PHY_BASE_ADDR + 0xe0;
    for (ii = 47; ii <= 48; ii++)
    {
        u4value = *(u4DRAMCTemp+ii+DRAMC_BACKUP_REG_NUM+DRAMRB_BACKUP_REG_NUM);
        ucDram_Register_Write(u4addr, u4value);
        u4addr = u4addr + 4;
    }

    //ii=49
    u4addr = u4PHY_BASE_ADDR + 0xf0;
    u4value = *(u4DRAMCTemp+ii+DRAMC_BACKUP_REG_NUM+DRAMRB_BACKUP_REG_NUM);
    ucDram_Register_Write(u4addr, u4value);

    //---A2, B1 and PLL registers
    //A2, B1 (A2+0x50) registers
    //0x00~0x0c
    u4addr = u4PHY_BASE_ADDR1 + 0x00;
    for (ii = 50; ii <= 53; ii++)
    {
        u4value = *(u4DRAMCTemp+ii+DRAMC_BACKUP_REG_NUM+DRAMRB_BACKUP_REG_NUM);
        ucDram_Register_Write(u4addr, u4value);
        u4addr = u4addr + 4;
    }

    //0x18~0x40
    u4addr = u4PHY_BASE_ADDR1 + 0x18;
    for (ii = 54; ii <= 64; ii++)
    {
        u4value = *(u4DRAMCTemp+ii+DRAMC_BACKUP_REG_NUM+DRAMRB_BACKUP_REG_NUM);
        ucDram_Register_Write(u4addr, u4value);
        u4addr = u4addr + 4;
    }

    //0x4c~0x50
    u4addr = u4PHY_BASE_ADDR1 + 0x4c;
    for (ii = 65; ii <= 66; ii++)
    {
        u4value = *(u4DRAMCTemp+ii+DRAMC_BACKUP_REG_NUM+DRAMRB_BACKUP_REG_NUM);
        ucDram_Register_Write(u4addr, u4value);
        u4addr = u4addr + 4;
    }

    //0xc8~0xcc (0xd4~0xd8)
    if (!IS_DRAM_CHANNELB_ACTIVE())
    {
        u4addr = mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG50);
    }
    else
    {
        u4addr = mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG53);
    }
    for (ii = 67; ii <= 68; ii++)
    {
        u4value = *(u4DRAMCTemp+ii+DRAMC_BACKUP_REG_NUM+DRAMRB_BACKUP_REG_NUM);
        ucDram_Register_Write(u4addr, u4value);
        u4addr = u4addr + 4;
    }

    //0xd4 (0xe0)
    //ii=69
    if (!IS_DRAM_CHANNELB_ACTIVE())
    {
        u4addr = mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG53);
    }
    else
    {
        u4addr = mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG56);
    }
    u4value = *(u4DRAMCTemp+ii+DRAMC_BACKUP_REG_NUM+DRAMRB_BACKUP_REG_NUM);
    ucDram_Register_Write(u4addr, u4value);
}
void DramcWriteBackCommonReg(U32 *u4DRAMCTemp)
{
    U8 ii;
    U32 u4value, u4addr;
    //PLL registers, regardless of channel
    //May execute twice for chA & chB
    u4addr = mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG46);
    for (ii = 0; ii <= 1; ii++)
    {
        u4value = *(u4DRAMCTemp+ii);
        ucDram_Register_Write(u4addr, u4value);
        u4addr = u4addr + 4;
    }

    ii=2;
    u4addr = mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG50);
    u4value = *(u4DRAMCTemp+ii);
    ucDram_Register_Write(u4addr, u4value);

    //---DDRPHY wrapper registers
    ii=3;
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_WRAP_RESET), *(u4DRAMCTemp+ii));

    //Arbiter chsel
    ii=4;
    TCM_DRAM_SIZE = *(u4DRAMCTemp+ii);
#if defined(CC_CHA_CHB_NO_GAP)
    // Set CHSEL channel B according to channel A Mbytes.
    u4value = ucDram_Register_Read(mcSET_ARBITER_ADDR(0x0));
    u4value &= ~0x700000;
    if (TCMGET_CHANNELA_SIZE()==0x40)
        ucDram_Register_Write(mcSET_ARBITER_ADDR(0x0), u4value |0x100000);//channel A 64Mbytes
    else if (TCMGET_CHANNELA_SIZE()==0x80)
        ucDram_Register_Write(mcSET_ARBITER_ADDR(0x0), u4value |0x200000);//channel A 128Mbytes
    else if (TCMGET_CHANNELA_SIZE()==0x100)
        ucDram_Register_Write(mcSET_ARBITER_ADDR(0x0), u4value |0x300000);//channel A 256Mbytes
    else if (TCMGET_CHANNELA_SIZE()==0x200)
        ucDram_Register_Write(mcSET_ARBITER_ADDR(0x0), u4value |0x400000);//channel A 512Mbytes
    else
        ucDram_Register_Write(mcSET_ARBITER_ADDR(0x0), u4value |0x500000);//channel A 1Gbytes
#endif
}

