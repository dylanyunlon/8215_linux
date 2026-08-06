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




#include "x_lint.h"
#include <linux/types.h>
#include "x_assert.h"
#include "aud_oal.h"
#include "aud_debug.h"
#include "DspDrvInc.h"
#include <media/atc/drv_aud.h>
#include "aud_se.h"
#include "aud_comm_os.h"

AUD_SE_NOTIFY_SET_PP_TAB_T   g_aNotifyInfo = {0};

#define SE_REINIT_FLAG_SZ   2
u32 g_u4PostReinitFlag[SE_REINIT_FLAG_SZ] = {0, 0};


u32 AudSeEqCliCmd(u32 u4CLICmd)
{
    u8 u1Ind;
    AUD_SE_EQ_INST_T* prInst = AudSeGetEqInst();
    LOG(LOG_FEATURE, TEXT("[SE] EQ Status:\n"));
	
    LOG(LOG_FEATURE, TEXT("EQ On/Off   = %d.\n"), prInst->rUISetting.u4_enable);
	LOG(LOG_FEATURE, TEXT("EQ Band num = %d.\n"), prInst->rUISetting.u4_bandnum);
    // EQ Gain
    LOG(LOG_FEATURE, TEXT(">> EQ Gain List:\n"));
    for (u1Ind=0; u1Ind<7; u1Ind++)
    {
        LOG(LOG_FEATURE, TEXT("%d: [%08X] %08X, %08X, %08X, %08X, %08X\n"), 
			        u1Ind+1,
					prInst->rUISetting.au4_gain[u1Ind][0],
					prInst->rUISetting.au4_gain[u1Ind][1],
					prInst->rUISetting.au4_gain[u1Ind][2],
					prInst->rUISetting.au4_gain[u1Ind][3],
					prInst->rUISetting.au4_gain[u1Ind][4],
					prInst->rUISetting.au4_gain[u1Ind][5]);
        LOG(LOG_FEATURE, TEXT(" 			 %08X, %08X, %08X, %08X, %08X\n"),	  
					prInst->rUISetting.au4_gain[u1Ind][6],
					prInst->rUISetting.au4_gain[u1Ind][7],
					prInst->rUISetting.au4_gain[u1Ind][8],
					prInst->rUISetting.au4_gain[u1Ind][9],
					prInst->rUISetting.au4_gain[u1Ind][10]);
    }
	
    // EQ IIR Filter
    LOG(LOG_FEATURE, TEXT(">> EQ IIR Filter Coef (a,b,c): \n"));
    for (u1Ind=0; u1Ind<10; u1Ind++)
    {
        LOG(LOG_FEATURE, TEXT("Band%02d: %08X, %08X, %08X\n"), 
					u1Ind+1,
                    prInst->EQ_TABLE[0][u1Ind*3+0],
                    prInst->EQ_TABLE[0][u1Ind*3+1],
				    prInst->EQ_TABLE[0][u1Ind*3+2]);
    }
    return AUD_SE_ST_OK;
}


u32 AudSeUpMixCliCmd(u32 u4CLICmd) 
{
    AUD_SE_UPMIX_INST_T* prInst = AudSeGetUpmixInst();
    LOG(LOG_FEATURE, TEXT("[SE_UPMIX] UI Status %d\n"), prInst->fgUIEnableFlag);
    LOG(LOG_FEATURE, TEXT("[SE_UPMIX] Coef Log :\n"));
    LOG(LOG_FEATURE, TEXT(">>> Upmix Mode = 0x%x\n"), prInst->rCoef.e_upmix_mode);
    LOG(LOG_FEATURE, TEXT(">>> UPMix COEFF L2LS   = 0x%x \n"),prInst->rCoef.UPMIX_GAIN[0]);
    LOG(LOG_FEATURE, TEXT(">>> UPMix COEFF R2LS   = 0x%x \n"),prInst->rCoef.UPMIX_GAIN[1]);
    LOG(LOG_FEATURE, TEXT(">>> UPMix COEFF L2RS   = 0x%x \n"),prInst->rCoef.UPMIX_GAIN[2]);
    LOG(LOG_FEATURE, TEXT(">>> UPMix COEFF R2RS   = 0x%x \n"),prInst->rCoef.UPMIX_GAIN[3]);
    LOG(LOG_FEATURE, TEXT(">>> UPMix COEFF L2C    = 0x%x \n"),prInst->rCoef.UPMIX_GAIN[4]);
    LOG(LOG_FEATURE, TEXT(">>> UPMix COEFF R2C    = 0x%x \n"),prInst->rCoef.UPMIX_GAIN[5]);
    LOG(LOG_FEATURE, TEXT(">>> UPMix COEFF L2L    = 0x%x \n"),prInst->rCoef.UPMIX_GAIN[6]);
    LOG(LOG_FEATURE, TEXT(">>> UPMix COEFF R2R    = 0x%x \n"),prInst->rCoef.UPMIX_GAIN[7]);
    return AUD_SE_ST_OK;
}


u32 AudSeReverbCliCmd(u32 u4CLICmd)
{
    AUD_SE_REVERB_INST_T* prInst = AudSeGetReverbInst();
	u8 u1Ind, u1IndBank;

    LOG(LOG_FEATURE, TEXT("[SE] REVERB CLI Log:\n"));
    LOG(LOG_FEATURE, TEXT(">> Reverb on/off   = %d. \n"), prInst->fgUIEnableFlag);
    LOG(LOG_FEATURE, TEXT(">> Reverb Gain	  = 0x%X. \n"), prInst->rCoef.u4Gain);
    LOG(LOG_FEATURE, TEXT(">> Reverb FBGain   = 0x%X. \n"), prInst->rCoef.u4FeedBackGain);
	
    for (u1Ind=0; u1Ind<3; u1Ind++)
    {
        for(u1IndBank=0; u1IndBank<4; u1IndBank++)
        {
            LOG(LOG_FEATURE, TEXT("Coef %d.\n"), prInst->rCoef.u4BankSize[u1Ind][u1IndBank]);
        }
    }
    return AUD_SE_ST_OK;
}


u32 AudSeLoudNessCLICmd(u32 u4CLICmd)
{
    AUD_SE_LOUDNESS_INST_T* prInst = AudSeGetLoudnessInst();
	u8 u1Ind;
    LOG(LOG_FEATURE, TEXT("[SE] LoudNess CLI Log:\n"));
    LOG(LOG_FEATURE, TEXT("[SE] LoudNess %d.\n"),prInst->fgUIEnableFlag);

    LOG(LOG_FEATURE, TEXT(">>> DRAM_LOUDNESS_MODE   = 0x%x \n"),prInst->rCoef.e_loudness_mode);
    for(u1Ind = 0; u1Ind<6; u1Ind++)
    {
        LOG(LOG_FEATURE, TEXT("index %d, Gain %d.\r\n"),
			u1Ind, prInst->rCoef.Loud_GAIN[u1Ind]);
    }
    return AUD_SE_ST_OK;
}


u32 AudSeAtsCLICmd(u32 u4CLICmd)
{
    AUD_SE_ATS_INST_T* prInst = AudSeGetAtsInst();

    LOG(LOG_FEATURE, TEXT("[SE_ATS] CLI Log:\n"));
    LOG(LOG_FEATURE, TEXT("[SE_ATS] UI Status %d.\n"), prInst->fgUIEnableFlag);
    LOG(LOG_FEATURE, TEXT("ATS CTRL Mode = 0x%x\n"), prInst->rCoef.u4CtrlMode);
    LOG(LOG_FEATURE, TEXT("ATS INPUT_GAIN   = 0x%x \n"),prInst->rCoef.u4InputGain);
    LOG(LOG_FEATURE, TEXT("ATS CENTER_GAIN   = 0x%x \n"),prInst->rCoef.u4CenterGain);
    LOG(LOG_FEATURE, TEXT("ATS LR_GAIN   = 0x%x \n"),prInst->rCoef.u4LRGain);
    LOG(LOG_FEATURE, TEXT("ATS LSRS_GAIN   = 0x%x \n"),prInst->rCoef.u4LsRsGain);
    LOG(LOG_FEATURE, TEXT("ATS LFE_GAIN    = 0x%x \n"),prInst->rCoef.u4LfeGain);
    LOG(LOG_FEATURE, TEXT("ATS C_TO_LR_GAIN    = 0x%x \n"),prInst->rCoef.u4C2LRGain);
    LOG(LOG_FEATURE, TEXT("ATS C_TO_LSRS_GAIN    = 0x%x \n"),prInst->rCoef.u4C2LsRsGain);
    LOG(LOG_FEATURE, TEXT("ATS LR_TO_LSRS_GAIN    = 0x%x \n"),prInst->rCoef.u4Lr2LsRsGain);
    LOG(LOG_FEATURE, TEXT("ATS OVERALL_DELAY   = 0x%x \n"),prInst->rCoef.u4OverallDelay);
    LOG(LOG_FEATURE, TEXT("ATS LSRS_BAND_NUM_1   = 0x%x \n"),prInst->rIntCoef.u4LsRsBandNum);
    LOG(LOG_FEATURE, TEXT("ATS IIR_ORDER    = 0x%x \n"),prInst->rIntCoef.u4IIROrder);
    LOG(LOG_FEATURE, TEXT("ATS C_IN_GAIN    = 0x%x \n"),prInst->rCoef.u4CenterInGain);
    LOG(LOG_FEATURE, TEXT("ATS LFE_IN_GAIN    = 0x%x \n"),prInst->rCoef.u4LfeInGain);
    LOG(LOG_FEATURE, TEXT("ATS LSRS_IN_GAIN    = 0x%x \n"),prInst->rCoef.u4LsRsInGain);   

    return AUD_SE_ST_OK;
}

u32 AudSeMvsCLICmd(u32 u4CLICmd) 
{
    AUD_SE_MVS_INST_T* prInst = AudSeGetMvsInst();
    // Ctrl & Gain Control Value
    LOG(5, TEXT("[SE] MVS status        : %08X\n"), prInst->fgUIEnableFlag);
    LOG(5, TEXT("[SE] MVS Mode          : %08X\n"), prInst->u4Mod);
    LOG(0, TEXT("[SE] MVS GAIN_SCALE    : %08X\n"), prInst->rCoef.u4GainScale);
    LOG(0, TEXT("[SE] MVS WIDTH_GAIN    : %08X\n"), prInst->rCoef.u4WidthGain);
    LOG(0, TEXT("[SE] MVS LR_GAIN       : %08X\n"), prInst->rCoef.u4LRGain);
    LOG(0, TEXT("[SE] MVS CENTER_GAIN   : %08X\n"), prInst->rCoef.u4CenterGain);
    LOG(0, TEXT("[SE] MVS CROSSTALK_GAIN: %08X\n"), prInst->rCoef.u4CrosstalkGain);
    LOG(0, TEXT("[SE] MVS BASS_GAIN     : %08X\n"), prInst->rCoef.u4BassGain);
    LOG(0, TEXT("[SE] MVS OUTPUT_GAIN   : %08X\n"), prInst->rCoef.u4OutputGain);
    LOG(0, TEXT("[SE] MVS INPUT_GAIN    : %08X\n"), prInst->rCoef.u4InputGain);
    LOG(0, TEXT("[SE] MVS VR_PHASE      : %08X\n"), prInst->rCoef.u4VRPhase);

    return AUD_SE_ST_OK;
}


static void vSetPostReinitFlag(AUD_SE_TYPE_T u1Type)
{
    u32 u4Group = u1Type >> 5;  //one DRWD just have 32 bit
    if(u4Group < SE_REINIT_FLAG_SZ)
    {
        g_u4PostReinitFlag[u4Group] |= (1 << (u1Type - 32 * u4Group));
        LOG(LOG_FEATURE, TEXT("[SetPostReinitFlag] Set bit %d !\n"), u1Type);
    }
    else
    {
        LOG(LOG_FEATURE, TEXT("[SetPostReinitFlag] Not support this argument now!\n"));
    }
}

static bool fgClearPostReinitFlag(AUD_SE_TYPE_T u1Type)
{
    u32 u4Group = u1Type >> 5;  //one DRWD just have 32 bit
    if(u4Group < SE_REINIT_FLAG_SZ)
    {
        u32 u4Flag = 1 << (u1Type - 32 * u4Group);
        if(g_u4PostReinitFlag[u4Group] & u4Flag)
        {
            g_u4PostReinitFlag[u4Group] &= ~u4Flag;
            LOG(LOG_FEATURE, TEXT("[ClearPostReinitFlag] Clear bit %d!\n"), u1Type);
            return TRUE;
        }
    }
    else
    {
        LOG(LOG_FEATURE, TEXT("[ClearPostReinitFlag] Not support this argument now!\n"));    
    }
    return FALSE;
}

void vResetPostReinitFlag(void)
{
    s32 i = 0;
    const AUD_SE_OBJ_T * prObj = NULL;

    while (NULL != (prObj = AudSeGetObjPtrBaseInd(i++)))
    {
        // Init post reinit flag
        if(NULL != prObj->u4ProcessNotify)
        {
            vSetPostReinitFlag(prObj->u1Type);
        }
    }
}

/***************************************************************************
     Function : vAudSeInit
       Author : sszhou
  Description : Init audio post process share memory and common dram info
    Parameter : null
    Return    : null
***************************************************************************/
void vAudSeInit(void)
{
    s32 i = 0;
    const AUD_SE_OBJ_T* prObj = NULL;

    while (NULL != (prObj = AudSeGetObjPtrBaseInd(i++)))
    {
        if (NULL != prObj->vInit)
        {
            prObj->vInit();         // Do Init Process
        }
        
        // Init post reinit flag
        if(NULL != prObj->u4ProcessNotify)
        {
            vSetPostReinitFlag(prObj->u1Type);
        }
    }
    // DSP Support Post process notify after Load Common Code Table & SeInit
    {
        AUD_SE_NOTIFY_LOAD_POSTCODE_T rPostType;
        const u8 au1DspSupportType[] = {
            AUD_SE_REVERB,
            AUD_SE_EQUALIZER,
            AUD_SE_UPMIX,
            AUD_SE_LOUDNESS,
            AUD_SE_PROLOGICII,
            AUD_SE_CSII,
            AUD_SE_MVS,
            AUD_SE_ATS,
           };
        rPostType.pu1Type = (const u8*)au1DspSupportType;
        rPostType.u4TypeCnt = sizeof(au1DspSupportType)/sizeof(*au1DspSupportType);
        vAudSeProcessNotify(AUD_SE_NOTIFY_LOAD_POSTCODE, &rPostType);
    }
}

/***************************************************************************
     Function : fgAudSeProcessOpCmd
       Author : sszhou
  Description : Process SCC Operation Command
    Parameter : pvInfo:     [in/out] information base on command type
    Return    : bool:       TRUE: Process Command OK
                            FALSE:Process Command Failed
***************************************************************************/
bool fgAudSeProcessOpCmd(void *pvInfo)
{
    u32 u4Ret = 0;
    u32 u4DataSize = 0;
    void   *pvData = NULL;
    const AUD_SE_OBJ_T * prObj = NULL;
    AUD_SE_OPCMD_T rOpCmd;
    
    if (NULL != pvInfo)
    {
        //Because the rm already do user -> kernel convert
        //Only do mem copy is OK
        x_memcpy(&rOpCmd, pvInfo, sizeof(AUD_SE_OPCMD_T));

        if ((NULL != rOpCmd.pvData) && (rOpCmd.u4DataSize > 0))
        {
            u4DataSize = rOpCmd.u4DataSize;
            pvData = kzalloc(u4DataSize, GFP_KERNEL);

            if (NULL == pvData)
            {
                return FALSE;
            }
            // Load user data to kernel space
            if (copy_from_user(pvData, rOpCmd.pvData, u4DataSize))
            {
                kfree(pvData);
                pvData = NULL;
                return FALSE;
            }
        }

        // get real post process obj
        prObj = prAudSeGetObjPtr(rOpCmd.u1Type);
        if (NULL != prObj)
        {
            if (NULL != prObj->u4ProcessOpCmd)
            {
                // Process real OpCmd
                u4Ret = prObj->u4ProcessOpCmd(rOpCmd.u4OpCode, pvData, u4DataSize);
                
                // Check result
                if (AUD_SE_ST_OK == u4Ret || AUD_SE_ST_OK_NO_REINIT == u4Ret)
                {         
                    vSetPostReinitFlag(prObj->u1Type);
                    if((AUD_SE_ST_OK_NO_REINIT == u4Ret) && 
                       (g_aNotifyInfo.prDspSrcParam != NULL) && 
                       (g_aNotifyInfo.prDspOutputParam != NULL))
                    {
                        // for change SE setting very fast when playing
                        prObj->u4ProcessNotify(AUD_SE_NOTIFY_SET_PP_TAB, &g_aNotifyInfo); 
                        LOG(LOG_FEATURE, TEXT("[SE] Se type: 0x%x, don't need DSP reinit! \n"), prObj->u1Type);         
                    }               
                    else 
                    {
                        AudSeSetReinit();
                    }
                    
                    // Free Memory
                    if (NULL != pvData)
                    {
                        kfree(pvData);
                        pvData = NULL;
                    }
                    return TRUE;
                }
            }
        }
        else
        {
            LOG(LOG_FEATURE, TEXT("[SE] type %d is close. \r\n"), rOpCmd.u1Type);  
        }
    }

    // Free Memory
    if (NULL != pvData)
    {
        kfree(pvData);
        pvData = NULL;
    }

    return FALSE;
}


/***************************************************************************
     Function : fgAudSeProcessCLIOpCmd
       Author : sszhou
  Description : Process CLI Operation Command
    Parameter : pvInfo:     [in/out] information base on command type
    Return    : bool:       TRUE: Process Command OK
                            FALSE:Process Command Failed
***************************************************************************/
bool fgAudSeProcessCLIOpCmd(void *pvInfo)
{
    AUD_SE_CLICMD_T rCliCmd;

    if (NULL == pvInfo)
    {
        return FALSE;
    }
    x_memcpy(&rCliCmd, pvInfo, sizeof(AUD_SE_CLICMD_T));

    switch(rCliCmd.u1Type)
    {
    case AUD_SE_EQUALIZER:
		AudSeEqCliCmd(0);
		break;
    case AUD_SE_UPMIX:
        AudSeUpMixCliCmd(0);
		break;
    case AUD_SE_REVERB:
		AudSeReverbCliCmd(0);
		break;
    case AUD_SE_LOUDNESS:
        AudSeLoudNessCLICmd(0);
		break;
    case AUD_SE_ATS:
        AudSeAtsCLICmd(0);
		break;
    case AUD_SE_MVS:
        AudSeMvsCLICmd(0);
		break;

        default:
        break;
    }
    
    return TRUE;
}

/***************************************************************************
     Function : vAudSeProcessNotify
       Author : sszhou
  Description : Process audio driver notify information
    Parameter : u4NotifyType:   [in] current notify type
                pInfo:          [in/out] information base on notify type
    Return    : null
***************************************************************************/
void vAudSeProcessNotify(u32 u4NotifyType, void *pInfo)
{
    s32 i=0;
    const AUD_SE_OBJ_T * prObj = NULL;

    // dispatch notify information to all post process objects
    while(NULL != (prObj = AudSeGetObjPtrBaseInd(i++)))
    {
        if(NULL != prObj->u4ProcessNotify)
        {
            if(u4NotifyType == AUD_SE_NOTIFY_SET_PP_TAB)
            {
                if(fgClearPostReinitFlag(prObj->u1Type))
                {
                    prObj->u4ProcessNotify(AUD_SE_NOTIFY_SET_PP_TAB, pInfo);
                }
            }
            else 
            {
                prObj->u4ProcessNotify(u4NotifyType, pInfo);
            }
        }
    }
}


/***************************************************************************
     Function : vAudSeProcessUOP
       Author : sszhou
  Description : Process post process UOP operation command
    Parameter : u4Uop:          [in] UOP ID
                [31:16 UopOpCode][15:8 u4Type][7:0 = 0x1B]
    Return    : null
***************************************************************************/
void vAudSeProcessUOP(u32 u4Uop)
{
    u8 u1UopID = (u8)((u4Uop) & 0x00FF);
    AUD_SE_TYPE_T u1Type =  (AUD_SE_TYPE_T)((u4Uop>>8) & 0x00FF);
    u32 u2UopOpCode = (u16)((u4Uop>>16) & 0x0FFFF);
    const AUD_SE_OBJ_T * prObj;

    if (DSP_UOPID1B == u1UopID)
    {
        // Get real post process obj
        prObj = prAudSeGetObjPtr(u1Type);
        if (NULL != prObj)
        {
            // Process UOP
            if (NULL != prObj->u4ProcessUOP)
            {
                prObj->u4ProcessUOP(u2UopOpCode);
            }
        }
    }
}

/***************************************************************************
     Function : vAudSeSendUop
       Author : sszhou
  Description : Pack and send UOP to DSP Ctrl
    Parameter : u1Type:         [in] Post process type
                u2UopOpCode:    [in] UOP Operation command code
    Return    : null
***************************************************************************/
void vAudSeSendUop(u8 u1Type, u16 u2UopOpCode)
{
    u32 u4UopCmd;

    // post process type and uop opcode range protect
    if ((AUD_SE_TYPE_MAX > u1Type) &&
        (AUD_SE_UOP_MAX_OPCODE > u2UopOpCode))
    {
        // UOP Pack
        u4UopCmd = DSP_UOPID1B;
        u4UopCmd |= ((u32)u1Type & 0x00FF) << 8;
        u4UopCmd |= ((u32)u2UopOpCode & 0x0FFFF) << 16;

        // Send UOP to DSP Ctrl
        vDspCmd(u4UopCmd);
    }
}









