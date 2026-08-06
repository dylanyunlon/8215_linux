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
/** @file osd_hw.h
 *  This header file declares hardware register interface of OSD.
 */

#ifndef OSD_HW_H
#define OSD_HW_H


/*-----------------------------------------------------------------------------*/
/* Include files*/
/*-----------------------------------------------------------------------------*/

#include "x_lint.h"

#include "x_hal_io.h"
/*#include "x_hal_8520.h"*/
/*#include "x_hal_926.h"*/


#ifndef __ARM2__
#include <media/atc/drv_osd_if.h>
#else
#include "drv_osd_if.h"
#endif
#include "chip_ver.h"


/*-----------------------------------------------------------------------------*/
/* Inter-file functions*/
/*-----------------------------------------------------------------------------*/

EXTERN void _OSD_AlwaysUpdateReg(bool fgEnable);
EXTERN void _OSD_UpdateReg(void);


/*-----------------------------------------------------------------------------*/
/* base register relative functions*/
/*-----------------------------------------------------------------------------*/
EXTERN __s32 _OSD_BASE_Update(__u32 u4Base);
EXTERN __s32 _OSD_BASE_GetReg(__u32 * pOsdBaseReg);
EXTERN __s32 _OSD_BASE_SetReg(const __u32 * pOsdBaseReg, bool fgHwReset);
EXTERN __s32 _OSD_BASE_UpdateHwReg(void);

EXTERN __s32 _OSD_BASE_GetUpdate(__u32 * pu4Value);
EXTERN __s32 _OSD_BASE_GetAlwaysUpdate(__u32 * pu4Value);
EXTERN __s32 _OSD_BASE_GetResetMainPath(__u32 * pu4Value);
EXTERN __s32 _OSD_BASE_GetResetAuxPath(__u32 * pu4Value);
EXTERN __s32 _OSD_BASE_GetResetMegPath(__u32 * pu4Value);
EXTERN __s32 _OSD_BASE_GetResetOsd1(__u32 * pu4Value);
EXTERN __s32 _OSD_BASE_GetResetOsd2(__u32 * pu4Value);
EXTERN __s32 _OSD_BASE_GetResetOsd3(__u32 * pu4Value);
EXTERN __s32 _OSD_BASE_GetResetOsd4(__u32 * pu4Value);
EXTERN __s32 _OSD_BASE_GetResetOsd5(__u32 * pu4Value);
EXTERN __s32 _OSD_BASE_GetResetCursor(__u32 * pu4Value);
/* 08h OSD mode configuration register */
EXTERN __s32 _OSD_BASE_GetHsEdge(__u32 * pu4Value);
EXTERN __s32 _OSD_BASE_GetVsEdge(__u32 * pu4Value);
EXTERN __s32 _OSD_BASE_GetFldPol(__u32 * pu4Value);
EXTERN __s32 _OSD_BASE_GetOsd1Prgs(__u32 * pu4Value);
EXTERN __s32 _OSD_BASE_GetOsd2Prgs(__u32 * pu4Value);
EXTERN __s32 _OSD_BASE_GetOsd3Prgs(__u32 * pu4Value);
EXTERN __s32 _OSD_BASE_GetOsd4Prgs(__u32 * pu4Value);
EXTERN __s32 _OSD_BASE_GetOsd5Prgs(__u32 * pu4Value);
EXTERN __s32 _OSD_BASE_GetOsd1Path(__u32 * pu4Value);
EXTERN __s32 _OSD_BASE_GetOsd2Path(__u32 * pu4Value);
EXTERN __s32 _OSD_BASE_GetOsd3Path(__u32 * pu4Value);
EXTERN __s32 _OSD_BASE_GetOsd4Path(__u32 * pu4Value);
EXTERN __s32 _OSD_BASE_GetOsd5Path(__u32 * pu4Value);
EXTERN __s32 _OSD_BASE_GetCsrPath(__u32 * pu4Value);
EXTERN __s32 _OSD_BASE_GetHsEdgeMeg(__u32 * pu4Value);
EXTERN __s32 _OSD_BASE_GetVsEdgeMeg(__u32 * pu4Value);
EXTERN __s32 _OSD_BASE_GetFldPolMeg(__u32 * pu4Value);
EXTERN __s32 _OSD_BASE_GetOsd1Dotctl(__u32 * pu4Value);
EXTERN __s32 _OSD_BASE_GetOsd2Dotctl(__u32 * pu4Value);
EXTERN __s32 _OSD_BASE_GetOsd3Dotctl(__u32 * pu4Value);
EXTERN __s32 _OSD_BASE_GetOsd4Dotctl(__u32 * pu4Value);
EXTERN __s32 _OSD_BASE_GetOsd5Dotctl(__u32 * pu4Value);
EXTERN __s32 _OSD_BASE_GetCsrDotctl(__u32 * pu4Value);

/* 0Ch Main FMT Vsync Timing Configuration Register */
EXTERN __s32 _OSD_BASE_GetOvtMain(__u32 * pu4Value);
EXTERN __s32 _OSD_BASE_GetVsWidthMain(__u32 * pu4Value);
EXTERN __s32 _OSD_BASE_GetHsWidthMain(__u32 * pu4Value);
/* 10h FMT H-Timing Configuration Register#1 */
EXTERN __s32 _OSD_BASE_GetScrnHStartOsd2(__u32 * pu4Value);
EXTERN __s32 _OSD_BASE_GetScrnHStartOsd1(__u32 * pu4Value);
/* 14h FMT H-Timing Configuration Register#2 */
EXTERN __s32 _OSD_BASE_GetScrnHStartCsr(__u32 * pu4Value);
EXTERN __s32 _OSD_BASE_GetScrnHStartOsd3(__u32 * pu4Value);
/* 18h Main FMT V-Timing Configuration Register #1 */
EXTERN __s32 _OSD_BASE_GetScrnVStartBotMain(__u32 * pu4Value);
EXTERN __s32 _OSD_BASE_GetScrnVStartTopMain(__u32 * pu4Value);
/* 1Ch Main FMT Timing Configuration Register #2 */
EXTERN __s32 _OSD_BASE_GetScrnVSize(__u32 * pu4Value);
EXTERN __s32 _OSD_BASE_GetScrnHSize(__u32 * pu4Value);
/* 20h OSD1 Window Position Configuration Register */
EXTERN __s32 _OSD_BASE_GetOsd1VStart(__u32 * pu4Value);
EXTERN __s32 _OSD_BASE_GetOsd1HStart(__u32 * pu4Value);
/* 24h OSD2 Window Position Configuration Register  */
EXTERN __s32 _OSD_BASE_GetOsd2VStart(__u32 * pu4Value);
EXTERN __s32 _OSD_BASE_GetOsd2HStart(__u32 * pu4Value);
/* 28h OSD3 Window Position Configuration Register */
EXTERN __s32 _OSD_BASE_GetOsd3VStart(__u32 * pu4Value);
EXTERN __s32 _OSD_BASE_GetOsd3HStart(__u32 * pu4Value);
/* 2Ch OSD Misc. Control Register */

EXTERN __s32 _OSD_BASE_GetOsd12Ex(__u32 * pu4Value);
EXTERN __s32 _OSD_BASE_GetOsd34Ex(__u32 * pu4Value);

/* 40h FMT H-Timing Configuration Register #3 */
EXTERN __s32 _OSD_BASE_GetScrnHStartOsd4(__u32 * pu4Value);
EXTERN __s32 _OSD_BASE_GetScrnHStartOsd5(__u32 * pu4Value);
/* 44h OSD4 Window Position Configuration Register */
EXTERN __s32 _OSD_BASE_GetOsd4VStart(__u32 * pu4Value);
EXTERN __s32 _OSD_BASE_GetOsd4HStart(__u32 * pu4Value);
/* 48h OSD5 Window Position Configuration Register */
EXTERN __s32 _OSD_BASE_GetOsd5VStart(__u32 * pu4Value);
EXTERN __s32 _OSD_BASE_GetOsd5HStart(__u32 * pu4Value);
/* 4Ch Aux FMT Vsync Timing Configuration Register */
EXTERN __s32 _OSD_BASE_GetOvtAux(__u32 * pu4Value);
EXTERN __s32 _OSD_BASE_GetVsWidthAux(__u32 * pu4Value);
EXTERN __s32 _OSD_BASE_GetHsWidthAux(__u32 * pu4Value);
/* 50h Aux FMT V-Timing Configuration Register #1  */
EXTERN __s32 _OSD_BASE_GetScrnVStartBotAux(__u32 * pu4Value);
EXTERN __s32 _OSD_BASE_GetScrnVStartTopAux(__u32 * pu4Value);
/* 54h Aux FMT Timing Configuration Register #2  */
EXTERN __s32 _OSD_BASE_GetScrnVSizeAux(__u32 * pu4Value);
EXTERN __s32 _OSD_BASE_GetScrnHSizeAux(__u32 * pu4Value);
/* 58h Message FMT Vsync Timing Configuration Register */
EXTERN __s32 _OSD_BASE_GetOvtMeg(__u32 * pu4Value);
EXTERN __s32 _OSD_BASE_GetVsWidthMeg(__u32 * pu4Value);
EXTERN __s32 _OSD_BASE_GetHsWidthMeg(__u32 * pu4Value);
/* 5Ch Message FMT V-Timing Configuration Register #1 */
EXTERN __s32 _OSD_BASE_GetScrnVStartBotMeg(__u32 * pu4Value);
EXTERN __s32 _OSD_BASE_GetScrnVStartTopMeg(__u32 * pu4Value);
/* 60h Message FMT Timing Configuration Register #2 */
EXTERN __s32 _OSD_BASE_GetScrnVSizeMeg(__u32 * pu4Value);
EXTERN __s32 _OSD_BASE_GetScrnHSizeMeg(__u32 * pu4Value);
/*  64 Main FMT Timing Configuration Register #2*/
EXTERN __s32 _OSD_BASE_GetOhtMain(__u32 * pu4Value);
/*  68h  */
EXTERN __s32 _OSD_BASE_GetOhtAux(__u32 * pu4Value);
/* 6Ch Message FMT Timing Configuration Register #2 */
EXTERN __s32 _OSD_BASE_GetOhtMeg(__u32 * pu4Value);
/*  70h */
EXTERN __s32 _OSD_BASE_GetIntTGen(__u32 * pu4Value);
EXTERN __s32 _OSD_BASE_GetCheckSumEn(__u32 * pu4Value);
EXTERN __s32 _OSD_BASE_GetSc1CheckSumSel(__u32 * pu4Value);
EXTERN __s32 _OSD_BASE_GetSc2CheckSumSel(__u32 * pu4Value);
EXTERN __s32 _OSD_BASE_GetSc3CheckSumSel(__u32 * pu4Value);
EXTERN __s32 _OSD_BASE_GetSc4CheckSumSel(__u32 * pu4Value);
/*  78h */
EXTERN __s32 _OSD_BASE_GetOsd1CheckSum(__u32 * pu4Value);
/* 7ch  */
EXTERN __s32 _OSD_BASE_GetOsd1ScCheckSum(__u32 * pu4Value);
/* 80h */
EXTERN __s32 _OSD_BASE_GetOsd2CheckSum(__u32 * pu4Value);
/* 84f */
EXTERN __s32 _OSD_BASE_GetOsd2ScCheckSum(__u32 * pu4Value);
/* 88h */
EXTERN __s32 _OSD_BASE_GetOsd3CheckSum(__u32 * pu4Value);
/* 8ch */
/* 90h */
EXTERN __s32 _OSD_BASE_GetOsd4CheckSum(__u32 * pu4Value);
/* 94h */
/* 98h */
EXTERN __s32 _OSD_BASE_GetOsd5CheckSum(__u32 * pu4Value);
/* 9ch */



/*---------------- set function ---------------*/
EXTERN __s32 _OSD_BASE_SetUpdate(__u32 u4Value);
EXTERN __s32 _OSD_BASE_SetAlwaysUpdate(__u32 u4Value);
EXTERN __s32 _OSD_BASE_SetResetMainPath(__u32 u4Value);
EXTERN __s32 _OSD_BASE_SetResetAuxPath(__u32 u4Value);
EXTERN __s32 _OSD_BASE_SetResetMegPath(__u32 u4Value);
EXTERN __s32 _OSD_BASE_SetResetOsd1(__u32 u4Value);
EXTERN __s32 _OSD_BASE_SetResetOsd2(__u32 u4Value);
EXTERN __s32 _OSD_BASE_SetResetOsd3(__u32 u4Value);
EXTERN __s32 _OSD_BASE_SetResetOsd4(__u32 u4Value);
EXTERN __s32 _OSD_BASE_SetResetOsd5(__u32 u4Value);
EXTERN __s32 _OSD_BASE_SetResetCursor(__u32 u4Value);
/* 08h OSD mode configuration register */
EXTERN __s32 _OSD_BASE_SetHsEdge(__u32 u4Value);
EXTERN __s32 _OSD_BASE_SetVsEdge(__u32 u4Value);
EXTERN __s32 _OSD_BASE_SetFldPol(__u32 u4Value);
EXTERN __s32 _OSD_BASE_SetOsd1Prgs(__u32 u4Value);
EXTERN __s32 _OSD_BASE_SetOsd2Prgs(__u32 u4Value);
EXTERN __s32 _OSD_BASE_SetOsd3Prgs(__u32 u4Value);
EXTERN __s32 _OSD_BASE_SetOsd4Prgs(__u32 u4Value);
EXTERN __s32 _OSD_BASE_SetOsd5Prgs(__u32 u4Value);
EXTERN __s32  _OSD_BASE_SetCsrPrgs(__u32 u4Value);
EXTERN __s32 _OSD_BASE_SetOsd1Path(__u32 u4Value);
EXTERN __s32 _OSD_BASE_SetOsd2Path(__u32 u4Value);
EXTERN __s32 _OSD_BASE_SetOsd3Path(__u32 u4Value);
EXTERN __s32 _OSD_BASE_SetOsd4Path(__u32 u4Value);
EXTERN __s32 _OSD_BASE_SetOsd5Path(__u32 u4Value);
EXTERN __s32 _OSD_BASE_SetCsrPath(__u32 u4Value);
EXTERN __s32 _OSD_BASE_SetHsEdgeMeg(__u32 u4Value);
EXTERN __s32 _OSD_BASE_SetVsEdgeMeg(__u32 u4Value);
EXTERN __s32 _OSD_BASE_SetFldPolMeg(__u32 u4Value);
EXTERN __s32 _OSD_BASE_SetOsd1Dotctl(__u32 u4Value);
EXTERN __s32 _OSD_BASE_SetOsd2Dotctl(__u32 u4Value);
EXTERN __s32 _OSD_BASE_SetOsd3Dotctl(__u32 u4Value);
EXTERN __s32 _OSD_BASE_SetOsd4Dotctl(__u32 u4Value);
EXTERN __s32 _OSD_BASE_SetOsd5Dotctl(__u32 u4Value);
EXTERN __s32 _OSD_BASE_SetCsrDotctl(__u32 u4Value);
/* 0Ch Main FMT Vsync Timing Configuration Register */
EXTERN __s32 _OSD_BASE_SetOvtMain(__u32 u4Value);
EXTERN __s32 _OSD_BASE_SetVsWidthMain(__u32 u4Value);
EXTERN __s32 _OSD_BASE_SetHsWidthMain(__u32 u4Value);
/* 10h FMT H-Timing Configuration Register#1 */
EXTERN __s32 _OSD_BASE_SetScrnHStartOsd2(__u32 u4Value);
EXTERN __s32 _OSD_BASE_SetScrnHStartOsd1(__u32 u4Value);
/* 14h FMT H-Timing Configuration Register#2 */
EXTERN __s32 _OSD_BASE_SetScrnHStartCsr(__u32 u4Value);
EXTERN __s32 _OSD_BASE_SetScrnHStartOsd3(__u32 u4Value);
/* 18h Main FMT V-Timing Configuration Register #1 */
EXTERN __s32 _OSD_BASE_SetScrnVStartBotMain(__u32 u4Value);
EXTERN __s32 _OSD_BASE_SetScrnVStartTopMain(__u32 u4Value);
/* 1Ch Main FMT Timing Configuration Register #2 */
EXTERN __s32 _OSD_BASE_SetScrnVSizeMain(__u32 u4Value);
EXTERN __s32 _OSD_BASE_SetScrnHSizeMain(__u32 u4Value);
/* 20h OSD1 Window Position Configuration Register */
EXTERN __s32 _OSD_BASE_SetOsd1VStart(__u32 u4Value);
EXTERN __s32 _OSD_BASE_SetOsd1HStart(__u32 u4Value);
/* 24h OSD2 Window Position Configuration Register  */
EXTERN __s32 _OSD_BASE_SetOsd2VStart(__u32 u4Value);
EXTERN __s32 _OSD_BASE_SetOsd2HStart(__u32 u4Value);
/* 28h OSD3 Window Position Configuration Register */
EXTERN __s32 _OSD_BASE_SetOsd3VStart(__u32 u4Value);
EXTERN __s32 _OSD_BASE_SetOsd3HStart(__u32 u4Value);
/* 2Ch OSD Misc. Control Register */
EXTERN __s32 _OSD_BASE_SetOsd12Ex(__u32 u4Value);
EXTERN __s32 _OSD_BASE_SetOsd34Ex(__u32 u4Value);

/* 38h FMT Main Active Display Configuration Register #1*/
/* 40h FMT H-Timing Configuration Register #3 */
EXTERN __s32 _OSD_BASE_SetScrnHStartOsd4(__u32 u4Value);
EXTERN __s32 _OSD_BASE_SetScrnHStartOsd5(__u32 u4Value);
/* 44h OSD4 Window Position Configuration Register */
EXTERN __s32 _OSD_BASE_SetOsd4VStart(__u32 u4Value);
EXTERN __s32 _OSD_BASE_SetOsd4HStart(__u32 u4Value);
/* 48h OSD5 Window Position Configuration Register */
EXTERN __s32 _OSD_BASE_SetOsd5VStart(__u32 u4Value);
EXTERN __s32 _OSD_BASE_SetOsd5HStart(__u32 u4Value);
/* 4Ch Aux FMT Vsync Timing Configuration Register */
EXTERN __s32 _OSD_BASE_SetOvtAux(__u32 u4Value);
EXTERN __s32 _OSD_BASE_SetVsWidthAux(__u32 u4Value);
EXTERN __s32 _OSD_BASE_SetHsWidthAux(__u32 u4Value);
/* 50h Aux FMT V-Timing Configuration Register #1  */
EXTERN __s32 _OSD_BASE_SetScrnVStartBotAux(__u32 u4Value);
EXTERN __s32 _OSD_BASE_SetScrnVStartTopAux(__u32 u4Value);
/* 54h Aux FMT Timing Configuration Register #2  */
EXTERN __s32 _OSD_BASE_SetScrnVSizeAux(__u32 u4Value);
EXTERN __s32 _OSD_BASE_SetScrnHSizeAux(__u32 u4Value);
/* 58h Message FMT Vsync Timing Configuration Register */
EXTERN __s32 _OSD_BASE_SetOvtMeg(__u32 u4Value);
EXTERN __s32 _OSD_BASE_SetVsWidthMeg(__u32 u4Value);
EXTERN __s32 _OSD_BASE_SetHsWidthMeg(__u32 u4Value);
/* 5Ch Message FMT V-Timing Configuration Register #1 */
EXTERN __s32 _OSD_BASE_SetScrnVStartBotMeg(__u32 u4Value);
EXTERN __s32 _OSD_BASE_SetScrnVStartTopMeg(__u32 u4Value);
/* 60h Message FMT Timing Configuration Register #2 */
EXTERN __s32 _OSD_BASE_SetScrnVSizeMeg(__u32 u4Value);
EXTERN __s32 _OSD_BASE_SetScrnHSizeMeg(__u32 u4Value);
/*  64 Main FMT Timing Configuration Register #2*/
EXTERN __s32 _OSD_BASE_SetOhtMain(__u32 u4Value);
/*  68h  */
EXTERN __s32 _OSD_BASE_SetOhtAux(__u32 u4Value);
/* 6Ch Message FMT Timing Configuration Register #2 */
EXTERN __s32 _OSD_BASE_SetOhtMeg(__u32 u4Value);
/*  70h */
EXTERN __s32 _OSD_BASE_SetIntTGen(__u32 u4Value);
EXTERN __s32 _OSD_BASE_SetCheckSumEn(__u32 u4Value);
EXTERN __s32 _OSD_BASE_SetSc1CheckSumSel(__u32 u4Value);
EXTERN __s32 _OSD_BASE_SetSc2CheckSumSel(__u32 u4Value);
EXTERN __s32 _OSD_BASE_SetSc3CheckSumSel(__u32 u4Value);
EXTERN __s32 _OSD_BASE_SetSc4CheckSumSel(__u32 u4Value);
/*  78h */
EXTERN __s32 _OSD_BASE_SetOsd1CheckSum(__u32 u4Value);
/* 7ch  */
EXTERN __s32 _OSD_BASE_SetOsd1ScCheckSum(__u32 u4Value);
/* 80h */
EXTERN __s32 _OSD_BASE_SetOsd2CheckSum(__u32 u4Value);
/* 84f */
EXTERN __s32 _OSD_BASE_SetOsd2ScCheckSum(__u32 u4Value);
/* 88h */
EXTERN __s32 _OSD_BASE_SetOsd3CheckSum(__u32 u4Value);
/* 8ch */
EXTERN __s32 _OSD_BASE_SetOsd3ScCheckSum(__u32 u4Value);
/* 90h */
EXTERN __s32 _OSD_BASE_SetOsd4CheckSum(__u32 u4Value);
/* 94h */
EXTERN __s32 _OSD_BASE_SetOsd4ScCheckSum(__u32 u4Value);
/* 98h */
EXTERN __s32 _OSD_BASE_SetOsd5CheckSum(__u32 u4Value);
/* 9ch */
EXTERN __s32 _OSD_BASE_SetOsd5ScCheckSum(__u32 u4Value);

#if 1
/*---------------- set function ---------------*/

/*-----------------------------------------------------------------------------*/
/* base register relative functions*/
/*-----------------------------------------------------------------------------*/
EXTERN void _OSD_R_AlwaysUpdateReg(bool fgEnable);
EXTERN void _OSD_R_UpdateReg(void);


/*-----------------------------------------------------------------------------*/
/* base register relative functions*/
/*-----------------------------------------------------------------------------*/
EXTERN __s32 _OSD_R_BASE_GetReg(__u32 * pOsdBaseReg);
EXTERN __s32 _OSD_R_BASE_SetReg(const __u32 * pOsdBaseReg);
EXTERN __s32 _OSD_R_BASE_UpdateHwReg(void);

EXTERN __s32 _OSD_R_BASE_GetUpdate(__u32 * pu4Value);
EXTERN __s32 _OSD_R_BASE_GetAlwaysUpdate(__u32 * pu4Value);
EXTERN __s32 _OSD_R_BASE_GetResetMainPath(__u32 * pu4Value);
EXTERN __s32 _OSD_R_BASE_GetResetAuxPath(__u32 * pu4Value);
EXTERN __s32 _OSD_R_BASE_GetResetMegPath(__u32 * pu4Value);
EXTERN __s32 _OSD_R_BASE_GetResetOsd6(__u32 * pu4Value);
EXTERN __s32 _OSD_R_BASE_GetResetOsd7(__u32 * pu4Value);
EXTERN __s32 _OSD_R_BASE_GetResetOsd8(__u32 * pu4Value);
/* 08h OSD mode configuration register */
EXTERN __s32 _OSD_R_BASE_GetHsEdge(__u32 * pu4Value);
EXTERN __s32 _OSD_R_BASE_GetVsEdge(__u32 * pu4Value);
EXTERN __s32 _OSD_R_BASE_GetFldPol(__u32 * pu4Value);
EXTERN __s32 _OSD_R_BASE_GetOsd6Prgs(__u32 * pu4Value);
EXTERN __s32 _OSD_R_BASE_GetOsd7Prgs(__u32 * pu4Value);
EXTERN __s32 _OSD_R_BASE_GetOsd8Prgs(__u32 * pu4Value);
EXTERN __s32 _OSD_R_BASE_GetOsd6Path(__u32 * pu4Value);
EXTERN __s32 _OSD_R_BASE_GetOsd7Path(__u32 * pu4Value);
EXTERN __s32 _OSD_R_BASE_GetOsd8Path(__u32 * pu4Value);
EXTERN __s32 _OSD_R_BASE_GetHsEdgeMeg(__u32 * pu4Value);
EXTERN __s32 _OSD_R_BASE_GetVsEdgeMeg(__u32 * pu4Value);
EXTERN __s32 _OSD_R_BASE_GetFldPolMeg(__u32 * pu4Value);
EXTERN __s32 _OSD_R_BASE_GetOsd6Dotctl(__u32 * pu4Value);
EXTERN __s32 _OSD_R_BASE_GetOsd7Dotctl(__u32 * pu4Value);
EXTERN __s32 _OSD_R_BASE_GetOsd8Dotctl(__u32 * pu4Value);

/* 0Ch Main FMT Vsync Timing Configuration Register */
EXTERN __s32 _OSD_R_BASE_GetOvtMain(__u32 * pu4Value);
EXTERN __s32 _OSD_R_BASE_GetVsWidthMain(__u32 * pu4Value);
EXTERN __s32 _OSD_R_BASE_GetHsWidthMain(__u32 * pu4Value);
/* 10h FMT H-Timing Configuration Register#1 */
EXTERN __s32 _OSD_R_BASE_GetScrnHStartOsd7(__u32 * pu4Value);
EXTERN __s32 _OSD_R_BASE_GetScrnHStartOsd6(__u32 * pu4Value);
/* 14h FMT H-Timing Configuration Register#2 */
EXTERN __s32 _OSD_R_BASE_GetScrnHStartOsd8(__u32 * pu4Value);
/* 18h Main FMT V-Timing Configuration Register #1 */
EXTERN __s32 _OSD_R_BASE_GetScrnVStartBotMain(__u32 * pu4Value);
EXTERN __s32 _OSD_R_BASE_GetScrnVStartTopMain(__u32 * pu4Value);
/* 1Ch Main FMT Timing Configuration Register #2 */
EXTERN __s32 _OSD_R_BASE_GetScrnVSize(__u32 * pu4Value);
EXTERN __s32 _OSD_R_BASE_GetScrnHSize(__u32 * pu4Value);
/* 20h OSD1 Window Position Configuration Register */
EXTERN __s32 _OSD_R_BASE_GetOsd6VStart(__u32 * pu4Value);
EXTERN __s32 _OSD_R_BASE_GetOsd6HStart(__u32 * pu4Value);
/* 24h OSD2 Window Position Configuration Register  */
EXTERN __s32 _OSD_R_BASE_GetOsd7VStart(__u32 * pu4Value);
EXTERN __s32 _OSD_R_BASE_GetOsd7HStart(__u32 * pu4Value);
/* 28h OSD3 Window Position Configuration Register */
EXTERN __s32 _OSD_R_BASE_GetOsd8VStart(__u32 * pu4Value);
EXTERN __s32 _OSD_R_BASE_GetOsd8HStart(__u32 * pu4Value);
/* 2Ch OSD Misc. Control Register */


/* 40h FMT H-Timing Configuration Register #3 */
/* 44h OSD4 Window Position Configuration Register */
/* 48h OSD5 Window Position Configuration Register */
/* 4Ch Aux FMT Vsync Timing Configuration Register */
EXTERN __s32 _OSD_R_BASE_GetOvtAux(__u32 * pu4Value);
EXTERN __s32 _OSD_R_BASE_GetVsWidthAux(__u32 * pu4Value);
EXTERN __s32 _OSD_R_BASE_GetHsWidthAux(__u32 * pu4Value);
/* 50h Aux FMT V-Timing Configuration Register #1  */
EXTERN __s32 _OSD_R_BASE_GetScrnVStartBotAux(__u32 * pu4Value);
EXTERN __s32 _OSD_R_BASE_GetScrnVStartTopAux(__u32 * pu4Value);
/* 54h Aux FMT Timing Configuration Register #2  */
EXTERN __s32 _OSD_R_BASE_GetScrnVSizeAux(__u32 * pu4Value);
EXTERN __s32 _OSD_R_BASE_GetScrnHSizeAux(__u32 * pu4Value);
/* 58h Message FMT Vsync Timing Configuration Register */
EXTERN __s32 _OSD_R_BASE_GetOvtMeg(__u32 * pu4Value);
EXTERN __s32 _OSD_R_BASE_GetVsWidthMeg(__u32 * pu4Value);
EXTERN __s32 _OSD_R_BASE_GetHsWidthMeg(__u32 * pu4Value);
/* 5Ch Message FMT V-Timing Configuration Register #1 */
EXTERN __s32 _OSD_R_BASE_GetScrnVStartBotMeg(__u32 * pu4Value);
EXTERN __s32 _OSD_R_BASE_GetScrnVStartTopMeg(__u32 * pu4Value);
/* 60h Message FMT Timing Configuration Register #2 */
EXTERN __s32 _OSD_R_BASE_GetScrnVSizeMeg(__u32 * pu4Value);
EXTERN __s32 _OSD_R_BASE_GetScrnHSizeMeg(__u32 * pu4Value);
/*  64 Main FMT Timing Configuration Register #2*/
EXTERN __s32 _OSD_R_BASE_GetOhtMain(__u32 * pu4Value);
/*  68h  */
EXTERN __s32 _OSD_R_BASE_GetOhtAux(__u32 * pu4Value);
/* 6Ch Message FMT Timing Configuration Register #2 */
EXTERN __s32 _OSD_R_BASE_GetOhtMeg(__u32 * pu4Value);
/*  70h */
EXTERN __s32 _OSD_R_BASE_GetIntTGen(__u32 * pu4Value);
EXTERN __s32 _OSD_R_BASE_GetCheckSumEn(__u32 * pu4Value);
EXTERN __s32 _OSD_R_BASE_GetSc1CheckSumSel(__u32 * pu4Value);
EXTERN __s32 _OSD_R_BASE_GetSc2CheckSumSel(__u32 * pu4Value);
EXTERN __s32 _OSD_R_BASE_GetSc3CheckSumSel(__u32 * pu4Value);
EXTERN __s32 _OSD_R_BASE_GetSc4CheckSumSel(__u32 * pu4Value);
/*  78h */
EXTERN __s32 _OSD_R_BASE_GetOsd6CheckSum(__u32 * pu4Value);
/* 7ch  */
/* 80h */
EXTERN __s32 _OSD_R_BASE_GetOsd7CheckSum(__u32 * pu4Value);
/* 84f */
/* 88h */
EXTERN __s32 _OSD_R_BASE_GetOsd8CheckSum(__u32 * pu4Value);
/* 8ch */
/* 90h */
/* 94h */
/* 98h */
/* 9ch */

EXTERN __s32 _OSD_R_BASE_SetUpdate(__u32 u4Value);
EXTERN __s32 _OSD_R_BASE_SetAlwaysUpdate(__u32 u4Value);
EXTERN __s32 _OSD_R_BASE_SetResetMainPath(__u32 u4Value);
EXTERN __s32 _OSD_R_BASE_SetResetAuxPath(__u32 u4Value);
EXTERN __s32 _OSD_R_BASE_SetResetMegPath(__u32 u4Value);
EXTERN __s32 _OSD_R_BASE_SetResetOsd6(__u32 u4Value);
EXTERN __s32 _OSD_R_BASE_SetResetOsd7(__u32 u4Value);
EXTERN __s32 _OSD_R_BASE_SetResetOsd8(__u32 u4Value);
/* 08h OSD mode configuration register */
EXTERN __s32 _OSD_R_BASE_SetHsEdge(__u32 u4Value);
EXTERN __s32 _OSD_R_BASE_SetVsEdge(__u32 u4Value);
EXTERN __s32 _OSD_R_BASE_SetFldPol(__u32 u4Value);
EXTERN __s32 _OSD_R_BASE_SetOsd6Prgs(__u32 u4Value);
EXTERN __s32 _OSD_R_BASE_SetOsd7Prgs(__u32 u4Value);
EXTERN __s32 _OSD_R_BASE_SetOsd8Prgs(__u32 u4Value);
EXTERN __s32 _OSD_R_BASE_SetOsd4Prgs(__u32 u4Value);
EXTERN __s32 _OSD_R_BASE_SetOsd5Prgs(__u32 u4Value);
EXTERN __s32  _OSD_R_BASE_SetCsrPrgs(__u32 u4Value);
EXTERN __s32 _OSD_R_BASE_SetOsd6Path(__u32 u4Value);
EXTERN __s32 _OSD_R_BASE_SetOsd7Path(__u32 u4Value);
EXTERN __s32 _OSD_R_BASE_SetOsd8Path(__u32 u4Value);
EXTERN __s32 _OSD_R_BASE_SetHsEdgeMeg(__u32 u4Value);
EXTERN __s32 _OSD_R_BASE_SetVsEdgeMeg(__u32 u4Value);
EXTERN __s32 _OSD_R_BASE_SetFldPolMeg(__u32 u4Value);
EXTERN __s32 _OSD_R_BASE_SetOsd6Dotctl(__u32 u4Value);
EXTERN __s32 _OSD_R_BASE_SetOsd7Dotctl(__u32 u4Value);
EXTERN __s32 _OSD_R_BASE_SetOsd8Dotctl(__u32 u4Value);
/* 0Ch Main FMT Vsync Timing Configuration Register */
EXTERN __s32 _OSD_R_BASE_SetOvtMain(__u32 u4Value);
EXTERN __s32 _OSD_R_BASE_SetVsWidthMain(__u32 u4Value);
EXTERN __s32 _OSD_R_BASE_SetHsWidthMain(__u32 u4Value);
/* 10h FMT H-Timing Configuration Register#1 */
EXTERN __s32 _OSD_R_BASE_SetScrnHStartOsd7(__u32 u4Value);
EXTERN __s32 _OSD_R_BASE_SetScrnHStartOsd6(__u32 u4Value);
/* 14h FMT H-Timing Configuration Register#2 */
EXTERN __s32 _OSD_R_BASE_SetScrnHStartOsd8(__u32 u4Value);
/* 18h Main FMT V-Timing Configuration Register #1 */
EXTERN __s32 _OSD_R_BASE_SetScrnVStartBotMain(__u32 u4Value);
EXTERN __s32 _OSD_R_BASE_SetScrnVStartTopMain(__u32 u4Value);
/* 1Ch Main FMT Timing Configuration Register #2 */
EXTERN __s32 _OSD_R_BASE_SetScrnVSizeMain(__u32 u4Value);
EXTERN __s32 _OSD_R_BASE_SetScrnHSizeMain(__u32 u4Value);
/* 20h OSD1 Window Position Configuration Register */
EXTERN __s32 _OSD_R_BASE_SetOsd6VStart(__u32 u4Value);
EXTERN __s32 _OSD_R_BASE_SetOsd6HStart(__u32 u4Value);
/* 24h OSD2 Window Position Configuration Register  */
EXTERN __s32 _OSD_R_BASE_SetOsd7VStart(__u32 u4Value);
EXTERN __s32 _OSD_R_BASE_SetOsd7HStart(__u32 u4Value);
/* 28h OSD3 Window Position Configuration Register */
EXTERN __s32 _OSD_R_BASE_SetOsd8VStart(__u32 u4Value);
EXTERN __s32 _OSD_R_BASE_SetOsd8HStart(__u32 u4Value);
/* 2Ch OSD Misc. Control Register */

/* 38h FMT Main Active Display Configuration Register #1*/
/* 40h FMT H-Timing Configuration Register #3 */
/* 4Ch Aux FMT Vsync Timing Configuration Register */
EXTERN __s32 _OSD_R_BASE_SetOvtAux(__u32 u4Value);
EXTERN __s32 _OSD_R_BASE_SetVsWidthAux(__u32 u4Value);
EXTERN __s32 _OSD_R_BASE_SetHsWidthAux(__u32 u4Value);
/* 50h Aux FMT V-Timing Configuration Register #1  */
EXTERN __s32 _OSD_R_BASE_SetScrnVStartBotAux(__u32 u4Value);
EXTERN __s32 _OSD_R_BASE_SetScrnVStartTopAux(__u32 u4Value);
/* 54h Aux FMT Timing Configuration Register #2  */
EXTERN __s32 _OSD_R_BASE_SetScrnVSizeAux(__u32 u4Value);
EXTERN __s32 _OSD_R_BASE_SetScrnHSizeAux(__u32 u4Value);
/* 58h Message FMT Vsync Timing Configuration Register */
EXTERN __s32 _OSD_R_BASE_SetOvtMeg(__u32 u4Value);
EXTERN __s32 _OSD_R_BASE_SetVsWidthMeg(__u32 u4Value);
EXTERN __s32 _OSD_R_BASE_SetHsWidthMeg(__u32 u4Value);
/* 5Ch Message FMT V-Timing Configuration Register #1 */
EXTERN __s32 _OSD_R_BASE_SetScrnVStartBotMeg(__u32 u4Value);
EXTERN __s32 _OSD_R_BASE_SetScrnVStartTopMeg(__u32 u4Value);
/* 60h Message FMT Timing Configuration Register #2 */
EXTERN __s32 _OSD_R_BASE_SetScrnVSizeMeg(__u32 u4Value);
EXTERN __s32 _OSD_R_BASE_SetScrnHSizeMeg(__u32 u4Value);
/*  64 Main FMT Timing Configuration Register #2*/
EXTERN __s32 _OSD_R_BASE_SetOhtMain(__u32 u4Value);
/*  68h  */
EXTERN __s32 _OSD_R_BASE_SetOhtAux(__u32 u4Value);
/* 6Ch Message FMT Timing Configuration Register #2 */
EXTERN __s32 _OSD_R_BASE_SetOhtMeg(__u32 u4Value);
/*  70h */
EXTERN __s32 _OSD_R_BASE_SetIntTGen(__u32 u4Value);
EXTERN __s32 _OSD_R_BASE_SetCheckSumEn(__u32 u4Value);
/*  78h */
EXTERN __s32 _OSD_R_BASE_SetOsd6CheckSum(__u32 u4Value);
/* 7ch  */
EXTERN __s32 _OSD_R_BASE_SetOsd6ScCheckSum(__u32 u4Value);
/* 80h */
EXTERN __s32 _OSD_R_BASE_SetOsd7CheckSum(__u32 u4Value);
/* 84f */
EXTERN __s32 _OSD_R_BASE_SetOsd7ScCheckSum(__u32 u4Value);
/* 88h */
EXTERN __s32 _OSD_R_BASE_SetOsd8CheckSum(__u32 u4Value);
/* 8ch */
EXTERN __s32 _OSD_R_BASE_SetOsd8ScCheckSum(__u32 u4Value);
#endif

/* ----------------------------------------------------------------------*/
/* plane register relative functions*/
/* ----------------------------------------------------------------------*/
/* plane regitser get and set function*/
EXTERN __s32 _OSD_PLA_GetReg(__u32 u4Plane, __u32 * pOsdPlaneReg);
EXTERN __s32 _OSD_PLA_SetReg(__u32 u4Plane, const __u32 * pOsdPlaneReg);
EXTERN __s32 _OSD_PLA_UpdateHwReg(__u32 u4Plane);

/* OSD Plane registers */
EXTERN __s32 _OSD_PLA_Enable(__u32 u4Plane);
EXTERN __s32 _OSD_PLA_SetEnable(__u32 u4Plane, __u32 u4Value);
EXTERN __s32 _OSD_PLA_SetFakeHdr(__u32 u4Plane, __u32 u4Value);
EXTERN __s32 _OSD_PLA_SetPrngEn(__u32 u4Plane, __u32 u4Value);
EXTERN __s32 _OSD_PLA_SetAlphaZeroBlack(__u32 u4Plane, __u32 u4Value);
EXTERN __s32 _OSD_PLA_SetOutRngColorMode(__u32 u4Plane, __u32 u4Value);
EXTERN __s32 _OSD_PLA_SetHeaderAddr(__u32 u4Plane, __u32 u4Value);
EXTERN __s32 _OSD_PLA_SetBlending(__u32 u4Plane, __u32 u4Value);
EXTERN __s32 _OSD_PLA_SetFading(__u32 u4Plane, __u32 u4Value);
EXTERN __s32 _OSD_PLA_SetHFilter(__u32 u4Plane, __u32 u4Value);
EXTERN __s32 _OSD_PLA_SetColorExpSel(__u32 u4Plane, __u32 u4Value);
EXTERN __s32 _OSD_PLA_SetAlphaRatioEn(__u32 u4Plane, __u32 u4Value);
EXTERN __s32 _OSD_PLA_SetContReqLmt(__u32 u4Plane, __u32 u4Value);
EXTERN __s32 _OSD_PLA_SetFifoSize(__u32 u4Plane, __u32 u4Value);
EXTERN __s32 _OSD_PLA_SetPauseCnt(__u32 u4Plane, __u32 u4Value);
EXTERN __s32 _OSD_PLA_SetContReqLmt0(__u32 u4Plane, __u32 u4Value);
EXTERN __s32 _OSD_PLA_SetBurstDis(__u32 u4Plane, __u32 u4Value);
EXTERN __s32 _OSD_PLA_SetRgbMode(__u32 u4Plane, __u32 u4Value);
EXTERN __s32 _OSD_PLA_SetVacancyThr(__u32 u4Plane, __u32 u4Value);

EXTERN __s32 _OSD_PLA_SetHMirrorEn(__u32 u4Plane, __u32 u4Value);
EXTERN __s32 _OSD_PLA_SetVFlipEn(__u32 u4Plane, __u32 u4Value);
EXTERN __s32 _OSD_PLA_SetRgb2YcbrbEn(__u32 u4Plane, __u32 u4Value);
EXTERN __s32 _OSD_PLA_SetXVYCCEn(__u32 u4Plane, __u32 u4Value);
EXTERN __s32 _OSD_PLA_SetYCbCr709En(__u32 u4Plane, __u32 u4Value);
EXTERN __s32 _OSD_PLA_SetOsd1ArbRgnEn(__u32 u4Plane, __u32 u4Value);

/* plane register get function */
EXTERN __s32 _OSD_PLA_GetEnable(__u32 u4Plane, __u32 * pu4Value);
EXTERN __s32 _OSD_PLA_GetFakeHdr(__u32 u4Plane, __u32 * pu4Value);
EXTERN __s32 _OSD_PLA_GetPrngEn(__u32 u4Plane, __u32 * pu4Value);
EXTERN __s32 _OSD_PLA_GetAlphaZeroBlack(__u32 u4Plane, __u32 * pu4Value);
EXTERN __s32 _OSD_PLA_GetOutRngColorMode(__u32 u4Plane, __u32 * pu4Value);
EXTERN __s32 _OSD_PLA_GetHeaderAddr(__u32 u4Plane, __u32 * pu4Value);
EXTERN __s32 _OSD_PLA_GetBlending(__u32 u4Plane, __u32 * pu4Value);
EXTERN __s32 _OSD_PLA_GetFading(__u32 u4Plane, __u32 * pu4Value);
EXTERN __s32 _OSD_PLA_GetHFilter(__u32 u4Plane, __u32 * pu4Value);
EXTERN __s32 _OSD_PLA_GetColorExpSel(__u32 u4Plane, __u32 * pu4Value);
EXTERN __s32 _OSD_PLA_GetAlphaRatioEn(__u32 u4Plane, __u32 * pu4Value);
EXTERN __s32 _OSD_PLA_GetContReqLmt(__u32 u4Plane, __u32 * u4Value);
EXTERN __s32 _OSD_PLA_GetFifoSize(__u32 u4Plane, __u32 * pu4Value);
EXTERN __s32 _OSD_PLA_GetPauseCnt(__u32 u4Plane, __u32 * pu4Value);
EXTERN __s32 _OSD_PLA_GetContReqLmt0(__u32 u4Plane, __u32 * pu4Value);
EXTERN __s32 _OSD_PLA_GetBurstDis(__u32 u4Plane, __u32 * pu4Value);
EXTERN __s32 _OSD_PLA_GetRgbMode(__u32 u4Plane, __u32 * pu4Value);
EXTERN __s32 _OSD_PLA_GetVacancyThr(__u32 u4Plane, __u32 * pu4Value);

EXTERN __s32 _OSD_PLA_GetHMirrorEn(__u32 u4Plane, __u32 * pu4Value);
EXTERN __s32 _OSD_PLA_GetVFlipEn(__u32 u4Plane, __u32 * pu4Value);
EXTERN __s32 _OSD_PLA_GetRgb2YcbrbEn(__u32 u4Plane, __u32 * pu4Value);
EXTERN __s32 _OSD_PLA_GetXVYCCEn(__u32 u4Plane, __u32 * pu4Value);
EXTERN __s32 _OSD_PLA_GetYCbCr709En(__u32 u4Plane, __u32 * pu4Value);
EXTERN __s32 _OSD_PLA_GetOsd5FifoWrapEn(__u32 u4Plane, __u32 * pu4Value);
EXTERN __s32 _OSD_PLA_GetOsd1ArbRgnEn(__u32 u4Plane, __u32 * pu4Value);

EXTERN void _OSD_PLA_SetDestColorKey(bool fgEnable, __u32 u4ColorKey);

/*-----------------------------------------------------------------------------*/
/* scaler register relative functions*/
/*-----------------------------------------------------------------------------*/
/* scaler regitser get and set function*/
EXTERN __s32 _OSD_SC_GetReg(__u32 u4Scaler, __u32 *pOsdScalerReg);
EXTERN __s32 _OSD_SC_SetReg(__u32 u4Scaler, const __u32 *pOsdScalerReg);
EXTERN __s32 _OSD_SC_UpdateHwReg(__u32 u4Scaler);

/* scaler register set function*/
EXTERN __s32 _OSD_SC_SetVuscAlphaEdgeRcvr(__u32 u4Scaler, __u32 u4Value);
EXTERN __s32 _OSD_SC_SetVuscAlphaEdgeElt(__u32 u4Scaler, __u32 u4Value);
EXTERN __s32 _OSD_SC_SetVdscAlphaEdgeRcvr(__u32 u4Scaler, __u32 u4Value);
EXTERN __s32 _OSD_SC_SetVdscAlphaEdgeElt(__u32 u4Scaler, __u32 u4Value);
EXTERN __s32 _OSD_SC_SetHuscAlphaEdgeRcvr(__u32 u4Scaler, __u32 u4Value);
EXTERN __s32 _OSD_SC_SetHuscAlphaEdgeElt(__u32 u4Scaler, __u32 u4Value);
EXTERN __s32 _OSD_SC_SetHdscAlphaEdgeRcvr(__u32 u4Scaler, __u32 u4Value);
EXTERN __s32 _OSD_SC_SetHdscAlphaEdgeElt(__u32 u4Scaler, __u32 u4Value);

EXTERN __s32 _OSD_SC_SetVuscEn(__u32 u4Scaler, __u32 u4Value);
EXTERN __s32 _OSD_SC_SetVdscEn(__u32 u4Scaler, __u32 u4Value);
EXTERN __s32 _OSD_SC_SetHuscEn(__u32 u4Scaler, __u32 u4Value);
EXTERN __s32 _OSD_SC_SetHdscEn(__u32 u4Scaler, __u32 u4Value);
EXTERN __s32 _OSD_SC_SetScLpfEn(__u32 u4Scaler, __u32 u4Value);
EXTERN __s32 _OSD_SC_SetScEn(__u32 u4Scaler, __u32 u4Value);
EXTERN __s32 _OSD_SC_SetSrcVSize(__u32 u4Scaler, __u32 u4Value);
EXTERN __s32 _OSD_SC_SetSrcHSize(__u32 u4Scaler, __u32 u4Value);
EXTERN __s32 _OSD_SC_SetDstVSize(__u32 u4Scaler, __u32 u4Value);
EXTERN __s32 _OSD_SC_SetDstHSize(__u32 u4Scaler, __u32 u4Value);
EXTERN __s32 _OSD_SC_SetVscHSize(__u32 u4Scaler, __u32 u4Value);
EXTERN __s32 _OSD_SC_SetHdscStep(__u32 u4Scaler, __u32 u4Value);
EXTERN __s32 _OSD_SC_SetHdscOfst(__u32 u4Scaler, __u32 u4Value);
EXTERN __s32 _OSD_SC_SetHuscStep(__u32 u4Scaler, __u32 u4Value);
EXTERN __s32 _OSD_SC_SetHuscOfst(__u32 u4Scaler, __u32 u4Value);
EXTERN __s32 _OSD_SC_SetVscOfstTop(__u32 u4Scaler, __u32 u4Value);
EXTERN __s32 _OSD_SC_SetVscOfstBot(__u32 u4Scaler, __u32 u4Value);
EXTERN __s32 _OSD_SC_SetVscStep(__u32 u4Scaler, __u32 u4Value);
EXTERN __s32 _OSD_SC_SetScLpfC3(__u32 u4Scaler, __u32 u4Value);
EXTERN __s32 _OSD_SC_SetScLpfC4(__u32 u4Scaler, __u32 u4Value);
EXTERN __s32 _OSD_SC_SetScLpfC5(__u32 u4Scaler, __u32 u4Value);

EXTERN __s32 _OSD_SC_SetSlackEn(__u32 u4Scaler, __u32 u4SlackEn);

/* scaler register get function*/
EXTERN __s32 _OSD_SC_GetVuscColorEdgeOnly(__u32 u4Scaler, __u32 *pu4Value);
EXTERN __s32 _OSD_SC_GetVuscAlphaEdgeEn(__u32 u4Scaler, __u32 *pu4Value);
EXTERN __s32 _OSD_SC_GetVdscColorEdgeOnly(__u32 u4Scaler, __u32 *pu4Value);
EXTERN __s32 _OSD_SC_GetVdscAlphaEdgeEn(__u32 u4Scaler, __u32 *pu4Value);
EXTERN __s32 _OSD_SC_GetHuscColorEdgeOnly(__u32 u4Scaler, __u32 *pu4Value);
EXTERN __s32 _OSD_SC_GetHuscAlphaEdgeEn(__u32 u4Scaler, __u32 *pu4Value);
EXTERN __s32 _OSD_SC_GetHdscColorEdgeOnly(__u32 u4Scaler, __u32 *pu4Value);
EXTERN __s32 _OSD_SC_GetHdscAlphaEdgeEn(__u32 u4Scaler, __u32 *pu4Value);
EXTERN __s32 _OSD_SC_GetVuscEn(__u32 u4Scaler, __u32 *pu4Value);
EXTERN __s32 _OSD_SC_GetVdscEn(__u32 u4Scaler, __u32 *pu4Value);
EXTERN __s32 _OSD_SC_GetHuscEn(__u32 u4Scaler, __u32 *pu4Value);
EXTERN __s32 _OSD_SC_GetHdscEn(__u32 u4Scaler, __u32 *pu4Value);
EXTERN __s32 _OSD_SC_GetScLpfEn(__u32 u4Scaler, __u32 *pu4Value);
EXTERN __s32 _OSD_SC_GetScEn(__u32 u4Scaler, __u32 *pu4Value);
EXTERN __s32 _OSD_SC_GetSrcVSize(__u32 u4Scaler, __u32 *pu4Value);
EXTERN __s32 _OSD_SC_GetSrcHSize(__u32 u4Scaler, __u32 *pu4Value);
EXTERN __s32 _OSD_SC_GetDstVSize(__u32 u4Scaler, __u32 *pu4Value);
EXTERN __s32 _OSD_SC_GetDstHSize(__u32 u4Scaler, __u32 *pu4Value);
EXTERN __s32 _OSD_SC_GetVscHSize(__u32 u4Scaler, __u32 *pu4Value);
EXTERN __s32 _OSD_SC_GetHdscStep(__u32 u4Scaler, __u32 *pu4Value);
EXTERN __s32 _OSD_SC_GetHdscOfst(__u32 u4Scaler, __u32 *pu4Value);
EXTERN __s32 _OSD_SC_GetHuscStep(__u32 u4Scaler, __u32 *pu4Value);
EXTERN __s32 _OSD_SC_GetHuscOfst(__u32 u4Scaler, __u32 *pu4Value);
EXTERN __s32 _OSD_SC_GetVscOfstTop(__u32 u4Scaler, __u32 *pu4Value);
EXTERN __s32 _OSD_SC_GetVscOfstBot(__u32 u4Scaler, __u32 *pu4Value);
EXTERN __s32 _OSD_SC_GetVscStep(__u32 u4Scaler, __u32 *pu4Value);
EXTERN __s32 _OSD_SC_GetScLpfC1(__u32 u4Scaler, __u32 *pu4Value);
EXTERN __s32 _OSD_SC_GetScLpfC2(__u32 u4Scaler, __u32 *pu4Value);
EXTERN __s32 _OSD_SC_GetScLpfC3(__u32 u4Scaler, __u32 *pu4Value);

EXTERN __s32 _OSD_SC_GetScLpfC4(__u32 u4Scaler, __u32 *pu4Value);
EXTERN __s32 _OSD_SC_GetScLpfC5(__u32 u4Scaler, __u32 *pu4Value);

/*-----------------------------------------------------------------------------*/
/* region register relative functions*/
/*-----------------------------------------------------------------------------*/
EXTERN void _OSD_RGN_InitApi(void);
EXTERN void _OSD_RGN_UninitApi(void);
EXTERN __s32 _OSD_PLA_Create_Semaphores(void);

EXTERN __s32 _OSD_PLA_Delete_Semaphores(void);

/* region register set function*/
EXTERN __s32 _OSD_RGN_SetNextRegion(__u32 u4Region, __u32 u4Value);
EXTERN __s32 _OSD_RGN_SetFifoEx(__u32 u4Region, __u32 u4Value);
EXTERN __s32 _OSD_RGN_SetNextEnable(__u32 u4Region, __u32 u4Value);
EXTERN __s32 _OSD_RGN_SetColorMode(__u32 u4Region, __u32 u4Value);
EXTERN __s32 _OSD_RGN_SetDataAddr(__u32 u4Region, __u32 u4Value);
EXTERN __s32 _OSD_RGN_SetAlpha(__u32 u4Region, __u32 u4Value);
EXTERN __s32 _OSD_RGN_SetHClip(__u32 u4Region, __u32 u4Value);
EXTERN __s32 _OSD_RGN_SetVClip(__u32 u4Region, __u32 u4Value);
EXTERN __s32 _OSD_RGN_SetLineSize8(__u32 u4Region, __u32 u4Value);
EXTERN __s32 _OSD_RGN_SetVbSel(__u32 u4Region, __u32 u4Value);
EXTERN __s32 _OSD_RGN_SetUgSel(__u32 u4Region, __u32 u4Value);
EXTERN __s32 _OSD_RGN_SetYrSel(__u32 u4Region, __u32 u4Value);
EXTERN __s32 _OSD_RGN_SetASel(__u32 u4Region, __u32 u4Value);
EXTERN __s32 _OSD_RGN_SetPaletteAddr(__u32 u4Region, __u32 u4Value);
EXTERN __s32 _OSD_RGN_SetPalettePA(__u32 u4Region, __u32 u4Value);
EXTERN __s32 _OSD_RGN_SetPaletteLen(__u32 u4Region, __u32 u4Value);
EXTERN __s32 _OSD_RGN_SetLoadPalette(__u32 u4Region, __u32 u4Value);
EXTERN __s32 _OSD_RGN_SetInputWidth(__u32 u4Region, __u32 u4Value);
EXTERN __s32 _OSD_RGN_SetInputHeight(__u32 u4Region, __u32 u4Value);
EXTERN __s32 _OSD_RGN_SetLineSize(__u32 u4Region, __u32 u4Value);
EXTERN __s32 _OSD_RGN_SetHStep(__u32 u4Region, __u32 u4Value);
EXTERN __s32 _OSD_RGN_SetVStep(__u32 u4Region, __u32 u4Value);
EXTERN __s32 _OSD_RGN_SetOutputHeight(__u32 u4Region, __u32 u4Value);
EXTERN __s32 _OSD_RGN_SetOutputPosY(__u32 u4Region, __u32 u4Value);
EXTERN __s32 _OSD_RGN_SetOutputWidth(__u32 u4Region, __u32 u4Value);
EXTERN __s32 _OSD_RGN_SetOutputPosX(__u32 u4Region, __u32 u4Value);
EXTERN __s32 _OSD_RGN_SetDataAddrHI(__u32 u4Region, __u32 u4Value);
EXTERN __s32 _OSD_RGN_SetColorKeyEnable(__u32 u4Region, __u32 u4Value);
EXTERN __s32 _OSD_RGN_SetColorKey(__u32 u4Region, __u32 u4Value);
EXTERN __s32 _OSD_RGN_SetFrameMode(__u32 u4Region, __u32 u4Value);
EXTERN __s32 _OSD_RGN_SetAutoMode(__u32 u4Region, __u32 u4Value);
EXTERN __s32 _OSD_RGN_SetTopField(__u32 u4Region, __u32 u4Value);
EXTERN __s32 _OSD_RGN_SetBlendMode(__u32 u4Region, __u32 u4Value);
EXTERN __s32 _OSD_RGN_SetSelectByteEn(__u32 u4Region, __u32 u4Value);
EXTERN __s32 _OSD_RGN_SetMixSel(__u32 u4Region, __u32 u4Value);
EXTERN __s32 _OSD_RGN_SetDeCompMode(__u32 u4Region, __u32 u4Value);
EXTERN __s32 _OSD_RGN_SetWTEn(__u32 u4Region, __u32 u4Value);

/* region register get function*/
EXTERN __s32 _OSD_RGN_GetNextRegion(__u32 u4Region, __u32 *pu4Value);
EXTERN __s32 _OSD_RGN_GetFifoEx(__u32 u4Region, __u32 *pu4Value);
EXTERN __s32 _OSD_RGN_GetNextEnable(__u32 u4Region, __u32 *pu4Value);
EXTERN __s32 _OSD_RGN_GetColorMode(__u32 u4Region, __u32 *pu4Value);
EXTERN __s32 _OSD_RGN_GetDataAddr(__u32 u4Region, __u32 *pu4Value);
EXTERN __s32 _OSD_RGN_GetAlpha(__u32 u4Region, __u32 *pu4Value);
EXTERN __s32 _OSD_RGN_GetHClip(__u32 u4Region, __u32 *pu4Value);
EXTERN __s32 _OSD_RGN_GetVClip(__u32 u4Region, __u32 *pu4Value);
EXTERN __s32 _OSD_RGN_GetLineSize8(__u32 u4Region, __u32 *pu4Value);
EXTERN __s32 _OSD_RGN_GetVbSel(__u32 u4Region, __u32 *pu4Value);
EXTERN __s32 _OSD_RGN_GetUgSel(__u32 u4Region, __u32 *pu4Value);
EXTERN __s32 _OSD_RGN_GetYrSel(__u32 u4Region, __u32 *pu4Value);
EXTERN __s32 _OSD_RGN_GetASel(__u32 u4Region, __u32 *pu4Value);
EXTERN __s32 _OSD_RGN_GetPaletteAddr(__u32 u4Region, __u32 *pu4Value);
EXTERN __s32 _OSD_RGN_GetPalettePA(__u32 u4Region, __u32 *pu4Value);
EXTERN __s32 _OSD_RGN_GetPaletteLen(__u32 u4Region, __u32 *pu4Value);
EXTERN __s32 _OSD_RGN_GetLoadPalette(__u32 u4Region, __u32 *pu4Value);
EXTERN __s32 _OSD_RGN_GetInputWidth(__u32 u4Region, __u32 *pu4Value);
EXTERN __s32 _OSD_RGN_GetInputHeight(__u32 u4Region, __u32 *pu4Value);
EXTERN __s32 _OSD_RGN_GetLineSize(__u32 u4Region, __u32 *pu4Value);
EXTERN __s32 _OSD_RGN_GetHStep(__u32 u4Region, __u32 *pu4Value);
EXTERN __s32 _OSD_RGN_GetVStep(__u32 u4Region, __u32 *pu4Value);
EXTERN __s32 _OSD_RGN_GetOutputHeight(__u32 u4Region, __u32 *pu4Value);
EXTERN __s32 _OSD_RGN_GetOutputPosY(__u32 u4Region, __u32 *pu4Value);
EXTERN __s32 _OSD_RGN_GetOutputWidth(__u32 u4Region, __u32 *pu4Value);
EXTERN __s32 _OSD_RGN_GetOutputPosX(__u32 u4Region, __u32 *pu4Value);
EXTERN __s32 _OSD_RGN_GetColorKeyEnable(__u32 u4Region, __u32 *pu4Value);
EXTERN __s32 _OSD_RGN_GetColorKey(__u32 u4Region, __u32 *pu4Value);
EXTERN __s32 _OSD_RGN_GetFrameMode(__u32 u4Region, __u32 *pu4Value);
EXTERN __s32 _OSD_RGN_GetAutoMode(__u32 u4Region, __u32 *pu4Value);
EXTERN __s32 _OSD_RGN_GetTopField(__u32 u4Region, __u32 *pu4Value);
EXTERN __s32 _OSD_RGN_GetBlendMode(__u32 u4Region, __u32 *pu4Value);
EXTERN __s32 _OSD_RGN_GetSelectByteEn(__u32 u4Region, __u32 *pu4Value);

/* utility function for region list maintain*/
EXTERN __s32 _OSD_RGN_GetHandle(__u32 u4Addr, __u32 *pu4OsdRegion);
EXTERN __s32 _OSD_RGN_GetAddress(__u32 u4Region , __u32 *pu4Addr);
EXTERN __s32 _OSD_RGN_AttachTail(__u32 u4List, __s32 i4Attachment);
EXTERN __s32 _OSD_RGN_SetAlloc(__u32 u4Region, __u32 fgStatus);
EXTERN __s32 _OSD_RGN_GetAlloc(__u32 u4Region, __u32 *pfgStatus);
EXTERN __s32 _OSD_RGN_Alloc(__u32 *pu4Region, __s32 i4MemChannel);
EXTERN __s32 _OSD_RGN_Free(__u32 u4Region);
EXTERN __s32 _OSD_RGN_FreeList(__u32 u4List);

#endif /*OSD_HW_H*/



