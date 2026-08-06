#include "ddr.h"
#include "ddr_includes.h"
#include "x_dram.h"

void SetPstWBufFlush()
{
    U32 u4Tmp = 0;

   	u4Tmp = DRAM_DMARB_READ32(DRAMB_REG_DYNPRI);
	u4Tmp = u4Tmp|BIT_PSTWRFLS_EN;
	DRAM_DMARB_WRITE32(DRAMB_REG_DYNPRI, u4Tmp);

    u4Tmp = DRAM_DMARB_READ32(DRAMB_REG_DYNPRI);
	u4Tmp = u4Tmp &(~BIT_PSTWRFLS_EN);
	DRAM_DMARB_WRITE32(DRAMB_REG_DYNPRI, u4Tmp); 
}



void SetMPstBufFulsh()
{
    U32 u4Tmp = 0;
	u4Tmp = DRAM_DMARB_READ32(DRAMB_REG_DYNPRI);
	u4Tmp = u4Tmp|BIT_MERFLS_EN;
	DRAM_DMARB_WRITE32(DRAMB_REG_DYNPRI, u4Tmp);

	u4Tmp = DRAM_DMARB_READ32(DRAMB_REG_DYNPRI);
	u4Tmp = u4Tmp &(~BIT_MERFLS_EN);
	DRAM_DMARB_WRITE32(DRAMB_REG_DYNPRI, u4Tmp); 
}


void SetFlush()
{
    SetPstWBufFlush();
	SetMPstBufFulsh();
}


/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Function:       DDR_EnterSuspend

Description:    Enter Suspend mode

Arguments:      pu4SaveAddr  - [in/out] SDRAM Save Address
		
Return Value:  

-------------------------------------------------------------------*/
void DDR_EnterSuspend(U32 *pu4SaveAddr)
{
   U32 u4Value = 0;
   /*1. Set Flush */
   //SetFlush();
   
   /*2. Backup Register*/
   SaveDramBackupReg(pu4SaveAddr);
   
   /*3. Enter Self-refresh*/
   DramcEnterSR();
   
   /*4. Set DMSU33  dram ic power down*/
    mcDELAY_us(1);
    u4Value = ucDram_Register_Read(REG_RW_DMSUS);
    mcSET_BIT(u4Value, BIT_DMSUS);
    ucDram_Register_Write(REG_RW_DMSUS, u4Value);
   
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Function:       SaveDramBackupReg

Description:    Save Dramc Register to Address

Arguments:      pu4SaveAddr  - [in/out] SDRAM Save Address
		
Return Value:  

-------------------------------------------------------------------*/
void SaveDramBackupReg(U32 *pu4SaveAddr)
{
    /*1. Save Backup Register*/
    DramcBackupReg(pu4SaveAddr);
    
    /*2. Save Dram Common Register*/
    DramcBackupCommonReg(pu4SaveAddr+(PHY_BACKUP_REG_NUM+DRAMC_BACKUP_REG_NUM+DRAMRB_BACKUP_REG_NUM));
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Function:       DramcBackupReg

Description:    Save Dramc Register to Address

Arguments:      u4DRAMCTemp  - [in/out] SDRAM Save Address
		
Return Value:  

-------------------------------------------------------------------*/
void DramcBackupReg(U32 *u4DRAMCTemp)
{
    UINT8 ii;
    UINT32 u4value, u4addr, u4PHY_BASE_ADDR, u4PHY_BASE_ADDR1;
    // DRAMC backup registers
    UINT16 gu2dramc_backup_regaddr[DRAMC_BACKUP_REG_NUM] = { DRAMC_REG_CONF2, DRAMC_REG_DELDLY1, DRAMC_REG_TEST2_1, DRAMC_REG_TEST2_2, DRAMC_REG_TEST2_3, DRAMC_REG_DDR2CTL, DRAMC_REG_PADCTL4, \
                                                              DRAMC_REG_DQIDLY1, DRAMC_REG_DQIDLY2, DRAMC_REG_DQIDLY3, DRAMC_REG_DQIDLY4, DRAMC_REG_DQIDLY5, \
                                                              DRAMC_REG_DQIDLY6, DRAMC_REG_DQIDLY7, DRAMC_REG_DQIDLY8};
    // DRAMRB backup registers
    UINT16 gu2dramrb_backup_regaddr[DRAMRB_BACKUP_REG_NUM] = {REG_RW_CONF, REG_RW_PRI0, REG_RW_PRI1, REG_RW_AGTIM0, REG_RW_AGTIM1, REG_RW_STARVE1, REG_RW_STARVE2, REG_RW_STARVE6};
    u4PHY_BASE_ADDR  = mcSET_PHY_REG_ADDR(0x000);
    u4PHY_BASE_ADDR1 = mcSET_PHY_REG_ADDR(0x200);
    //DRAMC registers
    for (ii=0; ii<DRAMC_BACKUP_REG_NUM; ii++)
    {
        u4value = DRAM_DRAMC_READ32(gu2dramc_backup_regaddr[ii]);
        *(u4DRAMCTemp+ii) = u4value;
    }
	
    //DRAMRB registers
    for (ii=0; ii<DRAMRB_BACKUP_REG_NUM; ii++)
    {
        u4value = DRAM_DMARB_READ32(gu2dramrb_backup_regaddr[ii]);
        *(u4DRAMCTemp+DRAMC_BACKUP_REG_NUM+ii) = u4value;
    }

    //DDRPHY registers
    //---A1, B2 registers
    u4addr = u4PHY_BASE_ADDR + 0x00;
    for (ii = 0; ii <= 3; ii++)
    {
        u4value = ucDram_Register_Read(u4addr);
        *(u4DRAMCTemp+ii+DRAMC_BACKUP_REG_NUM+DRAMRB_BACKUP_REG_NUM) = u4value;
        u4addr = u4addr + 4;
    }

    u4addr = u4PHY_BASE_ADDR + 0x18;
    for (ii = 4; ii <= 14; ii++)
    {
        u4value = ucDram_Register_Read(u4addr);
        *(u4DRAMCTemp+ii+DRAMC_BACKUP_REG_NUM+DRAMRB_BACKUP_REG_NUM) = u4value;
        u4addr = u4addr + 4;
    }

    u4addr = u4PHY_BASE_ADDR + 0x4c;
    for (ii = 15; ii <= 43; ii++)
    {
        u4value = ucDram_Register_Read(u4addr);
        *(u4DRAMCTemp+ii+DRAMC_BACKUP_REG_NUM+DRAMRB_BACKUP_REG_NUM) = u4value;
        u4addr = u4addr + 4;
    }

    u4addr = u4PHY_BASE_ADDR + 0xcc;
    for (ii = 44; ii <= 46; ii++)
    {
        u4value = ucDram_Register_Read(u4addr);
        *(u4DRAMCTemp+ii+DRAMC_BACKUP_REG_NUM+DRAMRB_BACKUP_REG_NUM) = u4value;
        u4addr = u4addr + 4;
    }

    u4addr = u4PHY_BASE_ADDR + 0xe0;
    for (ii = 47; ii <= 48; ii++)
    {
        u4value = ucDram_Register_Read(u4addr);
        *(u4DRAMCTemp+ii+DRAMC_BACKUP_REG_NUM+DRAMRB_BACKUP_REG_NUM) = u4value;
        u4addr = u4addr + 4;
    }

    //ii=49
    u4addr = u4PHY_BASE_ADDR + 0xf0;
    u4value = ucDram_Register_Read(u4addr);
    *(u4DRAMCTemp+ii+DRAMC_BACKUP_REG_NUM+DRAMRB_BACKUP_REG_NUM) = u4value;

    //---A2, B1 and PLL registers
    //A2, B1 (A2+0x50) registers
    //0x00~0x0c
    u4addr = u4PHY_BASE_ADDR1 + 0x00;
    for (ii = 50; ii <= 53; ii++)
    {
        u4value = ucDram_Register_Read(u4addr);
        *(u4DRAMCTemp+ii+DRAMC_BACKUP_REG_NUM+DRAMRB_BACKUP_REG_NUM) = u4value;
        u4addr = u4addr + 4;
    }

    //0x18~0x40
    u4addr = u4PHY_BASE_ADDR1 + 0x18;
    for (ii = 54; ii <= 64; ii++)
    {
        u4value = ucDram_Register_Read(u4addr);
        *(u4DRAMCTemp+ii+DRAMC_BACKUP_REG_NUM+DRAMRB_BACKUP_REG_NUM) = u4value;
        u4addr = u4addr + 4;
    }

    //0x4c~0x50
    u4addr = u4PHY_BASE_ADDR1 + 0x4c;
    for (ii = 65; ii <= 66; ii++)
    {
        u4value = ucDram_Register_Read(u4addr);
        *(u4DRAMCTemp+ii+DRAMC_BACKUP_REG_NUM+DRAMRB_BACKUP_REG_NUM) = u4value;
        u4addr = u4addr + 4;
    }

    //0xc8~0xcc (0xd4~0xd8)
    u4addr = mcSET_PHY_REG_ADDR(0x2c8);
    for (ii = 67; ii <= 68; ii++)
    {
        u4value = ucDram_Register_Read(u4addr);
        *(u4DRAMCTemp+ii+DRAMC_BACKUP_REG_NUM+DRAMRB_BACKUP_REG_NUM) = u4value;
        u4addr = u4addr + 4;
    }

    //0xd4 (0xe0)
    u4addr = mcSET_PHY_REG_ADDR(0x2d4);
    u4value = ucDram_Register_Read(u4addr);
    *(u4DRAMCTemp+ii+DRAMC_BACKUP_REG_NUM+DRAMRB_BACKUP_REG_NUM) = u4value;    
}    

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Function:       DramcBackupCommonReg

Description:    Save Dramc Register to Address

Arguments:      u4DRAMCTemp  - [in/out] SDRAM Save Address
		
Return Value:  

-------------------------------------------------------------------*/
void DramcBackupCommonReg(U32 *u4DRAMCTemp)
{
    UINT8 ii;
    UINT32 u4value, u4addr;
	
    //PLL registers, regardless of channel
    //May execute twice for chA & chB
    u4addr = mcSET_PHY_REG_ADDR(0x2b8);
    for (ii = 0; ii <= 1; ii++)
    {
        u4value = ucDram_Register_Read(u4addr);
        *(u4DRAMCTemp+ii) = u4value;
        u4addr = u4addr + 4;
     }

    ii=2;
    u4addr = mcSET_PHY_REG_ADDR(0x2c8);
    u4value = ucDram_Register_Read(u4addr);
    *(u4DRAMCTemp+ii) = u4value;

    //---DDRPHY wrapper registers
    ii=3;
    *(u4DRAMCTemp+ii) = ucDram_Register_Read(mcSET_PHY_REG_ADDR(0xa4c));

    //Arbiter chsel 
    ii=4;
    *(u4DRAMCTemp+ii) = 0;  // Only for compile to remove TCM_DRAM_SIZE
    //*(u4DRAMCTemp+ii) = TCM_DRAM_SIZE;
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Function:       DramcEnterSR

Description:    Save Dramc Register to Address

Arguments:      
		
Return Value:  

-------------------------------------------------------------------*/
void DramcEnterSR(void)
{
    UINT32 u4value = 0;
    //Issue self-refresh command to dram
    //entry self refresh    4h [26]	
    #if 1
    u4value = ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_CONF1));
    mcSET_BIT(u4value, POS_CONF1_SELFREF);
    ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_CONF1), u4value);
    #endif

    SetFlush();

    //Wait at least 9*tREFI+tCKSRE+20*tMIOCK (or 1ms)
    //mcDELAY_us(1000);
    //HAL_Delay_us(1);
    while ((ucDram_Register_Read(0xF00073B8) & 0x10000) != 0x10000);
	mcDELAY_us(10);

    mcSHOW_DBG_MSG2("%d (2: cha, 3: chb) DRAMC enter self refresh...err_value=%8x\n", IS_DRAM_CHANNELB_ACTIVE(), u4value);

   
}
