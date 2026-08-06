/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of AutoChips Inc. (C) 2008 AutoChips Inc.
*
*  BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
*  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("AUTOCHIPS SOFTWARE")
*  RECEIVED FROM AUTOCHIPS AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON
*  AN "AS-IS" BASIS ONLY. AUTOCHIPS EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
*  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
*  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
*  NEITHER DOES AUTOCHIPS PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
*  SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
*  SUPPLIED WITH THE AUTOCHIPS SOFTWARE, AND BUYER AGREES TO LOOK ONLY TO SUCH
*  THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. AUTOCHIPS SHALL ALSO
*  NOT BE RESPONSIBLE FOR ANY AUTOCHIPS SOFTWARE RELEASES MADE TO BUYER'S
*  SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*
*  BUYER'S SOLE AND EXCLUSIVE REMEDY AND AUTOCHIPS'S ENTIRE AND CUMULATIVE
*  LIABILITY WITH RESPECT TO THE AUTOCHIPS SOFTWARE RELEASED HEREUNDER WILL BE,
*  AT AUTOCHIPS'S OPTION, TO REVISE OR REPLACE THE AUTOCHIPS SOFTWARE AT ISSUE,
*  OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY BUYER TO
*  AUTOCHIPS FOR SUCH AUTOCHIPS SOFTWARE AT ISSUE.
*
*  THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE
*  WITH THE LAWS OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF
*  LAWS PRINCIPLES.  ANY DISPUTES, CONTROVERSIES OR CLAIMS ARISING THEREOF AND
*  RELATED THERETO SHALL BE SETTLED BY ARBITRATION IN SAN FRANCISCO, CA, UNDER
*  THE RULES OF THE INTERNATIONAL CHAMBER OF COMMERCE (ICC).
*
*****************************************************************************/

/*******************************************************************************
*
* Filename:
* ---------
* file dvp_fsio.cpp
*
* Project:
* --------
*   CNB
*
* Description:
* ------------
*
*
*
*
*------------------------------------------------------------------------------
*
*******************************************************************************/

#include "dvp_fsIo.h"
#include "dvp_fs.h"
#include "mm_debug.h"
#include <linux/types.h>


void DVPPFs_Init(void)
{
    pr_debug("[dvp][drv] DVPPFs_Init!\r\n");
    DFileSysIOInit();
}

void DVPPFs_DeInit(void)
{
    pr_debug("[dvp][drv] DVPPFs_DeInit!\r\n");
    DFileSysIODeInit();
}

bool DVPFs_ReadFileInfo(struct DVPFsFileItem *pFileItem, u32 u4ReadIndex)
{
    pr_debug("[dvp][drv] DVPFs_ReadFileInfo! u4ReadIndex: %d \r\n",
        u4ReadIndex);
    return ReadFileInfo(pFileItem, u4ReadIndex);
}

bool DVPFs_SetFilter(enum E_FILE_FILTER eFilter)
{
    return SetFilter(eFilter);
}

bool DVPFs_SetWorkDir(u16 u2DirIndex)
{
    return SetWorkDir(u2DirIndex);
}

bool DVPFs_SetFsValid(u32 dwFsAddr, u32 dwFilter)
{
    return SetFsValid(dwFsAddr, dwFilter);
}

bool DVPFs_SetFsInvalid(void)
{
    return SetFsInvalid();
}

bool DVPFs_SetFLCodec(UINT8 u2Codec)
{
    return SetFLCodec(u2Codec);
}


