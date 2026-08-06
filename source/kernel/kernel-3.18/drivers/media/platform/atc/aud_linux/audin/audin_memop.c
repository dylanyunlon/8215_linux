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




/***************************************************************************/
/**************************  Header File Include     ******************/
/***************************************************************************/

#if 0


#include "x_assert.h"
#include "x_bsp.h"
#include "audin_reg.h"
#include "audin_ret_define.h"
#include "aud_debug.h"
#include "audin_if.h"

/***************************************************************************/
/**************************  Globe Variable Define     ******************/
/***************************************************************************/


/***************************************************************************/
/**************************  Function declare    ******************/
/***************************************************************************/


/***************************************************************************/
/*                     Function                                            */
/***************************************************************************/

/************************************************************************
Function    : void Aud_ParsePhyAddr()
Description : divider u4PhyAddr into bankaddr,blockaddr,channeladdr
Input Parameter   : u4PhyAddr
Output Parameter :*tPaddrTrans, *tPaddrSec
Return      : AUDIN_RET_OK
************************************************************************/
s32 Aud_ParsePhyAddr(u32 u4PhyAddr,
                              PHY_ADDR_TRANS_RELATION_T *tPaddrTrans,
                              ADDR_SEC_INFO_T *tPaddrSec)
{
    u32 u4AddrInBlkStartPos;
    u32 u4AddrInBlkLen;
    u32 u4BlockAddrStartPos;
    u32 u4BlockAddrLen;
    u32 u4BankAddrStartPos;
    u32 u4BankAddrLen;

    u32 u4TmpAddr;
    u32 u4ChannelAddr;
    u32 u4BlockAddr;
    u32 u4BankAddr;

    u4TmpAddr = u4PhyAddr;

    u4AddrInBlkStartPos = tPaddrTrans->u4OffsetInBlkSbit;
    u4AddrInBlkLen      = tPaddrTrans->u4OffsetLen;
    u4ChannelAddr = AUD_GET_BITS_VAL(u4TmpAddr, u4AddrInBlkStartPos,  u4AddrInBlkLen);

    u4TmpAddr -= u4ChannelAddr << u4AddrInBlkStartPos;

    u4BlockAddrStartPos = tPaddrTrans->u4BlkAddrSbit;
    u4BlockAddrLen      = tPaddrTrans->u4BlkAddrLen;
    u4BlockAddr         = AUD_GET_BITS_VAL(u4TmpAddr, u4BlockAddrStartPos,  u4BlockAddrLen);

    u4TmpAddr -= u4BlockAddr << u4BlockAddrStartPos;

    u4BankAddrStartPos = tPaddrTrans->u4BankAddrSbit;
    u4BankAddrLen      = tPaddrTrans->u4BankAddrLen;
    u4BankAddr         = AUD_GET_BITS_VAL(u4TmpAddr, u4BankAddrStartPos,  u4BankAddrLen);

    tPaddrSec->u4BankAddr = u4BankAddr;
    tPaddrSec->u4BlkAddr  = u4BlockAddr;
    tPaddrSec->u4OffsetInBlk = u4ChannelAddr;

    u4TmpAddr = (u4ChannelAddr<<u4AddrInBlkStartPos) + (u4BlockAddr<<u4BlockAddrStartPos) + (u4BankAddr<<u4BankAddrStartPos);

    return AUDIN_RET_OK;
}


/************************************************************************
Function    : void Aud_SetLineInBufArea()
Description : Set line in data to line in buffer
Input Parameter   : u4PhyLinSadr,u4PhyLinEadr
Return      : AUDIN_RET_OK
************************************************************************/
s32 Aud_SetLineInBufArea(u32 u4PhyLinSadr, u32 u4PhyLinEadr)
{
    PHY_ADDR_TRANS_RELATION_T tPhyAddrTransRelation = {0};
    ADDR_SEC_INFO_T           tSAddrSec = {0};
    ADDR_SEC_INFO_T           tEAddrSec = {0};

    u32 u4RegVal;
    s32 i4RetVal;

    tPhyAddrTransRelation.u4OffsetInBlkSbit = AUD_LINE_IN_ADDR_START_POS;
    tPhyAddrTransRelation.u4OffsetLen       = AUD_LINE_IN_ADDR_LEN;
    tPhyAddrTransRelation.u4BlkAddrSbit     = AUD_LINE_IN_BLK_START_POS;/*no blk addr in line_in addr*/
    tPhyAddrTransRelation.u4BlkAddrLen      = AUD_LINE_IN_BLK_ADDR_LEN;/*no blk addr in line_in addr, so len is zero*/
    tPhyAddrTransRelation.u4BankAddrSbit    = AUD_LINE_IN_BANK_ADDR_START_POS;
    tPhyAddrTransRelation.u4BankAddrLen     = AUD_LINE_IN_BANK_ADDR_LEN;

    i4RetVal = Aud_ParsePhyAddr(u4PhyLinSadr, &tPhyAddrTransRelation, &tSAddrSec);
    AUDIN_CHECK_RESULT(i4RetVal,"Aud_ParsePhyAddr");


    i4RetVal = Aud_ParsePhyAddr(u4PhyLinEadr, &tPhyAddrTransRelation, &tEAddrSec);
    AUDIN_CHECK_RESULT(i4RetVal,"Aud_ParsePhyAddr");

    if( (tSAddrSec.u4BankAddr != tEAddrSec.u4BankAddr) ||
        (tSAddrSec.u4BlkAddr != tEAddrSec.u4BlkAddr))
    {
        return AUDIN_NOT_IN_SAME_BANK;
    }

    u4RegVal = tEAddrSec.u4OffsetInBlk | (tSAddrSec.u4OffsetInBlk << AUD_SPLIN_START_BIT_START);

    AUD_REG_WRITE(AUD_SPLIN_BLK_ADDR, u4RegVal);/*lin_sadr, lin_eadr*/

    AUD_REG_BITS_WRITE(AUD_LINE_IN_BANK_ADDR,
                       AUD_LINE_IN_BANK_BIT_START,
                       AUD_LINE_IN_BANK_BIT_NUM,
                       tSAddrSec.u4BankAddr);

    return AUDIN_RET_OK;

}

/************************************************************************
Function    : s32 Aud_MemCpy(u8 *pucDesAddr, u8 *pucSrcAddr, u32 u4Len)
Description : memcpy: set linin buff data to dram
Parameter   : pucDesAddr,pucSrcAddr,u4Len
Return      : AUDIN_RET_OK
************************************************************************/
s32 Aud_MemCpy(u8 *pucDesAddr, u8 *pucSrcAddr, u32 u4Len)
{
    u32 i;
    if ( u4Len > AUD_LINE_IN_BUF_SIZE)
    {
        LOG(1, TEXT("[AudIn][Aud_MemCpy]Error.\n"));
        return AUDIN_RET_FAIL;
    }
    BSP_InvDCacheRange((u32)pucSrcAddr, u4Len);

    for (i = 0; i < u4Len; i++)
    {
        *pucDesAddr++ = *pucSrcAddr++;
    }
    BSP_CleanDCacheRange((u32)pucDesAddr, u4Len);
    return AUDIN_RET_OK;
}


#endif
