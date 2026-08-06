#include "ddr.h"
#include "ddr_includes.h"


/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Function:        SetDramRegionProtect

Description:     Set Dram Region Protector

Arguments:      u4RegionID      - [in] Region ID
                        ptRegionInfo    - [in] Region Information		

Return Value:   

-------------------------------------------------------------------*/
void SetDramRegionProtect(U32 u4RegionID, P_T_REGIONINFO ptRegionInfo)
{
   U32 u4Tmp = 0;

   /*0.Enable Region Protest or not*/
   if(ptRegionInfo->bEnableRegion)
   {
        /*1. Set Region high bound*/
        u4Tmp = REGION_BOUND_MASK&ptRegionInfo->dwHighAddr;
	    DRAM_DMARB_WRITE32(DRAMB_REG_PROT0_0+(u4RegionID << 4), u4Tmp);  
		
       
        /*2. Set Region Low bound*/
        u4Tmp = 0;
        u4Tmp = REGION_BOUND_MASK&ptRegionInfo->dwLowAddr;
        DRAM_DMARB_WRITE32(DRAMB_REG_PROT0_0+(u4RegionID << 4) + 4, u4Tmp);

		/*3.Set Protect permit Agent ID*/
		if(ptRegionInfo->dwProtectAgentID != 0)
		{
		    DRAM_DMARB_WRITE32(DRAMB_REG_PROT0_0+(u4RegionID << 4) + 8, ptRegionInfo->dwProtectAgentID);
		}
		
        /*4. Set Region Attrubite*/
        u4Tmp = 0;
        u4Tmp = BIT_PROTECT_EN;

        if(ptRegionInfo->bIncludeRegion ==  FALSE)
        {
            u4Tmp |= BIT_OUT_PROTECT_MODE;
        }
        else
        {
            u4Tmp |= BIT_IN_PROTECT_MODE; 
        }
      
        if(ptRegionInfo->bWriteProctect == TRUE)
        {
            u4Tmp |= BIT_WEN_PROTECT;
        }
      
        if(ptRegionInfo->bReadProctect == TRUE)
        { 
            u4Tmp |= BIT_REN_PROTECT;
        }
		DRAM_DMARB_WRITE32(DRAMB_REG_PROT0_0+(u4RegionID << 4)+ 12, u4Tmp);
    }
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Function:       GetDramRegionIntrInfo

Description:    Get Dram Region Intruder Information

Arguments:      u4RegionID      - [in] Region ID
                       ptRegionInfo    - [in/out] Region Informatio	

Return Value:   TRUE on success

-------------------------------------------------------------------*/
BOOL GetDramRegionIntrInfo(U32 u4RegionID,P_T_INTRUDEINFO ptIntrudeInfo)
{
    BOOL bIntruderFlag = FALSE;
    U32  u4IntruderReg = 0;

    /*1.Read Intruder information from register*/
	u4IntruderReg = DRAM_DMARB_READ32(DRAMB_REG_INTR0 + (u4RegionID<< 2));
    
    /*2.detect Inturder or not*/
    if(u4IntruderReg&BIT_INTR_INTRUDEN)
    {
        /*3.Get Inturde address*/
        ptIntrudeInfo->dwAddr  = u4IntruderReg&INTRADR_MASK;
	
	    /*4. Read Intrude information */ 
	    u4IntruderReg = DRAM_DMARB_READ32(DRAMB_REG_INTRUID0+(u4RegionID & (~0x3))); 
    	u4IntruderReg =(u4IntruderReg>>((u4RegionID & 0x3)<<3))&0xFF;
        ptIntrudeInfo->dwAgtID = u4IntruderReg&INTR_AGTID_MASK;
	    ptIntrudeInfo->dwSubID = (u4IntruderReg&INTR_SUBID_MASK)>>INTR_SUBID_SHIFT;
	    bIntruderFlag = TRUE;
    }
    return bIntruderFlag; 
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Function:         ResetDramRegionProtect

Description:     Reset Dram Region Protector

Arguments:      u4RegionID      - [in] Region ID		

Return Value:   

-------------------------------------------------------------------*/
void ResetDramRegionProtect(U32 u4RegionID)
{
    U32 u4Tmp = 0;
	u4Tmp = DRAM_DMARB_READ32(DRAMB_REG_PROT0_0+(u4RegionID << 4)); 
	DRAM_DMARB_WRITE32(DRAMB_REG_PROT0_0+(u4RegionID << 4), 0);

	u4Tmp = DRAM_DMARB_READ32(DRAMB_REG_PROT0_0+(u4RegionID << 4 + 4)); 
	DRAM_DMARB_WRITE32(DRAMB_REG_PROT0_0+(u4RegionID << 4) + 4,  0);
	
	u4Tmp = DRAM_DMARB_READ32(DRAMB_REG_PROT0_0+(u4RegionID << 4 + 12)); 
	DRAM_DMARB_WRITE32(DRAMB_REG_PROT0_0+(u4RegionID << 4) + 12, 0);

	u4Tmp = DRAM_DMARB_READ32(DRAMB_REG_INTR0 + (u4RegionID<< 2)); 
	DRAM_DMARB_WRITE32(DRAMB_REG_INTR0 + (u4RegionID<< 2), 0);

	u4Tmp = DRAM_DMARB_READ32(DRAMB_REG_INTRUID0+(u4RegionID & (~0x3))); 
	DRAM_DMARB_WRITE32(DRAMB_REG_INTRUID0+(u4RegionID & (~0x3)),0); 
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Function:       CleanDramInterrupt

Description:    Clean Dram Interrupt

Arguments:      		

Return Value:   

-------------------------------------------------------------------*/
void CleanDramInterrupt()
{
    U32 u4Tmp = 0;
	u4Tmp = DRAM_DMARB_READ32(DRAMB_REG_INTCLR); 
	u4Tmp = u4Tmp|1;
	DRAM_DMARB_WRITE32(DRAMB_REG_INTCLR,u4Tmp);
	
	u4Tmp = u4Tmp&INTCLR_MASK;
	DRAM_DMARB_WRITE32(DRAMB_REG_INTCLR,u4Tmp);
}


void CleanIntrLog()
{
    U32 u4Tmp = 0;
    u4Tmp = HAL_READ32(0xf0006000);
	u4Tmp = u4Tmp|1;
	HAL_WRITE32(0xf0006000,u4Tmp);
	
    u4Tmp = HAL_READ32(0xf0006000);
	u4Tmp = u4Tmp&INTCLR_MASK;
	HAL_WRITE32(0xf0006000,u4Tmp);

    u4Tmp = HAL_READ32(0xf000850C);
	u4Tmp = u4Tmp|1;;
	HAL_WRITE32(0xf000850C,u4Tmp);

    u4Tmp = HAL_READ32(0xf000850C);
	u4Tmp = u4Tmp&INTCLR_MASK;
	HAL_WRITE32(0xf000850C,u4Tmp);	
}


void GetAllDramProtectInfo()
{
    U32 u4Tmp = 0;
	T_REGIONINFO tRegionInfo = {0};
	T_INTRUDEINFO tIntrudeInfo = {0};
	tRegionInfo.bEnableRegion = TRUE;
	ResetDramRegionProtect(0);
	tRegionInfo.dwLowAddr  = 0;
	tRegionInfo.dwHighAddr = TCMGET_CHANNELA_SIZE()*1024*1024;
	tRegionInfo.bIncludeRegion = TRUE;
	tRegionInfo.bWriteProctect = FALSE;
	tRegionInfo.bReadProctect = TRUE;
	SetDramRegionProtect(0, &tRegionInfo);
	if(GetDramRegionIntrInfo(0, &tIntrudeInfo) == TRUE)
	{
	    mcSHOW_DBG_MSG("Intrude Address 0x%x AgentID %d, SubID %d\n", tIntrudeInfo.dwAddr,tIntrudeInfo.dwAgtID,tIntrudeInfo.dwSubID);
	}
	else
	{
		mcSHOW_DBG_MSG("No Intrude \n");
	}
	CleanIntrLog();
	CleanDramInterrupt();
	ResetDramRegionProtect(0);
	
}


