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




/**************************************************************************

* Header Files

**************************************************************************/
//#include "x_hal_ic.h"
#include "tvd_core.h"
#include "tvd_hw_reg.h"
#include "tvd_log.h"

#ifdef __ARM2__
#include "x_ckgen_8317.h"
#else
#include <linux/clk.h>
#include <linux/clk-provider.h>
#endif

static bool gfgAnaIOPwrOn;
static bool gfgCHAPwrOn;
static bool gfgCHBPwrOn;
bool gfgNeedTurnClamp;



/**************************************************************************

* Global Variable

**************************************************************************/


/**************************************************************************

* Local/Static Functions

**************************************************************************/

/**************************************************************************

* @brief  TVD Comb Setting.

*     The setting is experience value.

* @param

* @return None

**************************************************************************/
enum TVD_SIGNAL_MODE_E {
	TVD_PAL_N,
	TVD_PAL,
	TVD_PAL_M,
	TVD_NTSC,
	TVD_SECAM,
	TVD_PAL_60,
	TVD_UNSTABLE,  /* Represent video signal is not stable yet! In Hardware, it's reserved */
	TVD_NTSC443,
	TVD_NONE
};

void vTvd_Comb_Setting(void)
{

	TVD_WRITE32(0x640u, 0x22110A10u);

	TVD_WRITE32(0x644u, 0xF0000006u);

	TVD_WRITE32(0x648u, 0x10002000u);

	TVD_WRITE32(0x64Cu, 0x3E00408Au);

	TVD_WRITE32(0x650u, 0x40300888u);

	TVD_WRITE32(0x654u, 0x6C000000u);

	TVD_WRITE32(0x658u, 0x00000067u);

	TVD_WRITE32(0x65Cu, 0x03440030u);

	TVD_WRITE32(0x660u, 0x01234444u);

	TVD_WRITE32(0x664u, 0x45678888u);

	TVD_WRITE32(0x668u, 0xF0100A8Du);

	TVD_WRITE32(0x66Cu, 0x00000003u);

	TVD_WRITE32(0x670u, 0x100D2808u);

	TVD_WRITE32(0x674u, 0x00801010u);

	TVD_WRITE32(0x678u, 0x84100A14u);

	TVD_WRITE32(0x67Cu, 0x01234567u);

	TVD_WRITE32(0x680u, 0x100AF850u);

	TVD_WRITE32(0x684u, 0x00000000u);

	TVD_WRITE32(0x688u, 0x00061410u);

	TVD_WRITE32(0x68Cu, 0x64101114u);

	TVD_WRITE32(0x690u, 0x0A074A36u);

	TVD_WRITE32(0x694u, 0x0A074A36u);

	TVD_WRITE32(0x698u, 0x007F9C00u);

	TVD_WRITE32(0x69Cu, 0x06000000u);

	TVD_WRITE32(0x6A0u, 0x0C0490E8u);

	TVD_WRITE32(0x6A4u, 0x00110333u);

	TVD_WRITE32(0x6A8u, 0x1488C084u);

	TVD_WRITE32(0x6ACu, 0xE8013434u);

	TVD_WRITE32(0x6B0u, 0x11202823u);

	TVD_WRITE32(0x6B4u, 0x04202411u);

	TVD_WRITE32(0x6B8u, 0x11111111u);

	TVD_WRITE32(0x6BCu, 0x000D8284u);

	TVD_WRITE32(0x6C0u, 0x00400833u);

	TVD_WRITE32(0x6C4u, 0x38081808u);

	TVD_WRITE32(0x6C8u, 0x60967050u);

	TVD_WRITE32(0x6CCu, 0x78801010u);

	TVD_WRITE32(0x6D0u, 0x0A0B4145u);

	TVD_WRITE32(0x6D4u, 0x0120FF05u);

	TVD_WRITE32(0x6D8u, 0x1C00640Au);

	TVD_WRITE32(0x6DCu, 0x0C006000u);

	TVD_WRITE32(0x6E0u, 0x0010101Fu);

	TVD_WRITE32(0x6E4u, 0x0069A900u);

	TVD_WRITE32(0x6E8u, 0x8A045AF4u);

	TVD_WRITE32(0x6ECu, 0x27030203u);

	TVD_WRITE32(0x6F0u, 0xC800000Fu);

	TVD_WRITE32(0x6F4u, 0x2F038408u);

	TVD_WRITE32(0x6F8u, 0x70061E05u);

	TVD_WRITE32(0x6FCu, 0x12D50000u);

	TVD_WRITE32(0x700u, 0x00DFA1B8u);

	TVD_WRITE32(0x704u, 0x200A80AAu);

	TVD_WRITE32(0x708u, 0x80000820u);

	TVD_WRITE32(0x70Cu, 0x11110435u);

	TVD_WRITE32(0x710u, 0x12345678u);

	TVD_WRITE32(0x714u, 0x02345678u);

	TVD_WRITE32(0x718u, 0x02345678u);

	TVD_WRITE32(0x71Cu, 0x01234567u);

	TVD_WRITE32(0x720u, 0x00081014u);

	TVD_WRITE32(0x724u, 0x00020206u);

	TVD_WRITE32(0x728u, 0x00000000u);

	TVD_WRITE32(0x72Cu, 0x00002480u);

	TVD_WRITE32(0x730u, 0x82040102u);

	TVD_WRITE32(0x734u, 0x00000041u);

	TVD_WRITE32(0x738u, 0x02040127u);

	TVD_WRITE32(0x73Cu, 0x00080000u);

	TVD_WRITE32(0x740u, 0x00015C1Eu);

	TVD_WRITE32(0x744u, 0x00800A10u);

	TVD_WRITE32(0x748u, 0x00000000u);

	TVD_WRITE32(0x74Cu, 0x3E40C028u);

	TVD_WRITE32(0x750u, 0x00800000u);

	TVD_WRITE32(0x754u, 0x45198064u);

	TVD_WRITE32(0x758u, 0x00000003u);

	TVD_WRITE32(0x75Cu, 0x1E0F1904u);

	TVD_WRITE32(0x760u, 0x83001405u);

	TVD_WRITE32(0x764u, 0x60145030u);

	TVD_WRITE32(0x768u, 0x08050510u);

	TVD_WRITE32(0x76Cu, 0x02345678u);

	TVD_WRITE32(0x770u, 0x00006110u);

	TVD_WRITE32(0x774u, 0x80007010u);

	TVD_WRITE32(0x778u, 0x00103111u);

	TVD_WRITE32(0x77Cu, 0x10009111u);
}



/**************************************************************************

* @brief  TVD NTSC Setting.

*     The setting is experience value, for MT5365 TVD NTSC Dump registers setting.

* @param

* @return None

**************************************************************************/

void vTvd_NTSC_Setting(bool ntsc443)
{
	TVD_LOG(TVD_LOG_LVL_INFO, "vTvd_NTSC_Setting enter\n");

	/* MT5365 TVD NTSC Dump registers setting */
	TVD_WRITE32(0x4C0u, 0x51515C55u); /* SYNC_START,MOVAVC_WIN_START,CLAMP_START,AGC_START */
	TVD_WRITE32(0x4C4u, 0x04555D45u);
	TVD_WRITE32(0x4C8u, 0x27F7595Eu);
	TVD_WRITE32(0x4CCu, 0x8F340376u); /* [31:28] - AGC2_MODE(Used for AGain and DGain Control) */
	TVD_WRITE32(0x4D0u, 0x158C0F80u);
	TVD_WRITE32(0x4D4u, 0x26144040u);
	TVD_WRITE32(0x4D8u, 0x0A00F600u);
	TVD_WRITE32(0x4DCu, 0xF41F0800u);
	TVD_WRITE32(0x4E0u, 0x13178080u);
	TVD_WRITE32(0x4E4u, 0x8AA7075Eu);
	TVD_WRITE32(0x4E8u, 0xF08400A0u);
	TVD_WRITE32(0x4ECu, 0x08180C40u);
	TVD_WRITE32(0x4F0u, 0x2008C2C9u);
	TVD_WRITE32(0x4F4u, 0x7180001Fu);
	TVD_WRITE32_MASK(0x4F8u, 0x8000306Cu, 0xFFFFFFeFu);     /* TVD_WRITE32(0x4F8, 0x8000306C); */
	TVD_WRITE32(0x4FCu, 0x420C35A0u);
	TVD_WRITE32(0x500u, 0x6838FA27u);
	TVD_WRITE32(0x504u, 0x0DFA2090u);
	TVD_WRITE32(0x508u, 0x08E06B40u);
	TVD_WRITE32(0x50Cu, 0x83608016u);
	TVD_WRITE32(0x510u, 0x40204080u);
	TVD_WRITE32(0x514u, 0x80326485u);
	TVD_WRITE32(0x518u, 0x11A0A04Cu);
	TVD_WRITE32(0x51Cu, 0x8040A0C0u);
	TVD_WRITE32(0x520u, 0x7E35AF80u);
	TVD_WRITE32(0x524u, 0x0408F840u);
	TVD_WRITE32(0x528u, 0x5404E030u);
	TVD_WRITE32(0x52Cu, 0x30900AA0u);
	TVD_WRITE32(0x530u, 0x0B410B09u);
	TVD_WRITE32(0x534u, 0xF43E40F0u);
	TVD_WRITE32(0x538u, 0x452414F0u);
	TVD_WRITE32(0x53Cu, 0x55C29325u);
	TVD_WRITE32(0xAB0u, 0x22F08C03u);
	TVD_WRITE32(0xAB4u, 0x8F0070D4u);
	TVD_WRITE32(0xAB8u, 0x42400800u);
	TVD_WRITE32(0xABCu, 0x82000820u);
	/*TVD_WRITE32(0x628, 0x08880001);*/
	/* Read Only
	TVD_WRITE32(0x040, 0x83001205);
	TVD_WRITE32(0x044, 0x002400FF);
	TVD_WRITE32(0x048, 0x00640101);
	TVD_WRITE32(0x04C, 0x03C4022E);
	TVD_WRITE32(0x050, 0x08022105);
	TVD_WRITE32(0x054, 0x80040FE0);
	TVD_WRITE32(0x058, 0xFFF00143);
	TVD_WRITE32(0x05C, 0x01001FF4);
	TVD_WRITE32(0x060, 0x01C00002);
	TVD_WRITE32(0x064, 0xFFEF2008);
	TVD_WRITE32(0x09C, 0x00000303);
	*/
	TVD_WRITE32(0x580u, 0x208181D4u);
	TVD_WRITE32(0x584u, 0x28AF88CAu);
	TVD_WRITE32(0x588u, 0xFC844040u);
	TVD_WRITE32(0x58Cu, 0x2854C531u);
	TVD_WRITE32(0x590u, 0x0206E612u);
	TVD_WRITE32(0x594u, 0x4840310Eu);
	TVD_WRITE32(0x598u, 0x21594088u);
	if (true == ntsc443) {
		TVD_WRITE32(0x59Cu, 0x29E2C8A6u);
	} else {
		TVD_WRITE32(0x59Cu, 0x29E2C8B2u);
	}
	TVD_WRITE32_MASK(0x5A0u, 0x44800C80u, 0xF7FFFFFFu);
	TVD_WRITE32(0x5A4u, 0x20010000u);
	TVD_WRITE32(0x5A8u, 0x0A4BC680u);
	TVD_WRITE32_MASK(0x5ACu, 0x400010C0u, 0xFFFBFFFFu);
	TVD_WRITE32(0x5B0u, 0x20000400u);     /*    TVD_WRITE32(0x5B0, 0x20180400);*/
	TVD_WRITE32(0x5B4u, 0x00F9BC00u);
	TVD_WRITE32(0x5B8u, 0x20002004u);
	TVD_WRITE32(0x5BCu, 0x17E0114Fu);
	TVD_WRITE32(0x5C0u, 0x1F908420u);
	TVD_WRITE32(0x5C4u, 0x19181718u);    /* TVD_WRITE32(0x5C4, 0x1A191515); */
	TVD_WRITE32(0x5C8u, 0x06658819u);
	TVD_WRITE32(0x5CCu, 0x72B91AB9u);
	TVD_WRITE32(0x5D0u, 0x5A1640DBu);
	TVD_WRITE32(0x5D4u, 0x55F401F4u);
	TVD_WRITE32(0x5D8u, 0x100181FAu);
	/* TVD_WRITE32(0x5DC, 0x700F6E60); */
	TVD_WRITE32(0x5E0u, 0xA0A797A6u);
	TVD_WRITE32(0x5E4u, 0xFAC9D1A0u);
	TVD_WRITE32(0x5E8u, 0x7CB83BD4u);
	TVD_WRITE32(0x5ECu, 0x20DD43A9u);
	TVD_WRITE32(0x5F0u, 0x80554329u);
	TVD_WRITE32(0x5F4u, 0x1708A103u);
	TVD_WRITE32(0x5F8u, 0x5001C67Fu);
	TVD_WRITE32(0x5FCu, 0x71000480u);    /* TVD_WRITE32(0x5FC, 0x70D70480); */
	/*
	TVD_WRITE32(0x600, 0x3D34035F);
	TVD_WRITE32(0x604, 0xF9228320);
	TVD_WRITE32(0x608, 0x32201501);
	TVD_WRITE32(0x60C, 0x547EA833);
	TVD_WRITE32(0x610, 0x000028AD);
	TVD_WRITE32(0x614, 0x55E86270);
	TVD_WRITE32(0x618, 0xE6CB2224);
	TVD_WRITE32(0x61C, 0x84460080);
	*/
	TVD_WRITE32(0x620u, 0x00400000u);
	TVD_WRITE32(0x624u, 0x7F084330u);
	TVD_WRITE32(0x628u, 0x08880001u);
	TVD_WRITE32(0x62Cu, 0xBF488488u);
	/* Read Only
	TVD_WRITE32(0x088, 0xA000032F);
	TVD_WRITE32(0x08C, 0x20C8320C);
	TVD_WRITE32(0x090, 0x50000000);
	TVD_WRITE32(0x094, 0x000D0000);
	TVD_WRITE32(0x098, 0xE2C0B000);
	TVD_WRITE32(0x09C, 0x00000303);
	TVD_WRITE32(0x0A0, 0x5444438D);
	TVD_WRITE32(0x0A4, 0xFB03FC03);
	TVD_WRITE32(0x0A8, 0x89922212);
	TVD_WRITE32(0x0AC, 0x00000000);
	*/
	TVD_WRITE32_MASK(0x540u, 0x30173E63u, 0xFFFFFFF0u);    /* TVD_WRITE32(0x540, 0x30173E63); */
	TVD_WRITE32(0x544u, 0x20F40725u);
	TVD_WRITE32(0x548u, 0x30B080FEu);
	TVD_WRITE32(0x54Cu, 0x1245482Bu);
	TVD_WRITE32(0x550u, 0xB4000023u);
	TVD_WRITE32(0x554u, 0x04AE0318u);
	TVD_WRITE32(0x558u, 0x106441D0u);
	TVD_WRITE32(0x55Cu, 0x53876575u);
	TVD_WRITE32(0x560u, 0x29E5E0EFu);
	TVD_WRITE32(0x564u, 0x2010563Du);
	TVD_WRITE32(0x568u, 0x204CDEC9u);
	TVD_WRITE32(0x56Cu, 0x204A8181u);
	TVD_WRITE32_MASK(0x570u, 0xFF29DEC5u, 0xFFFFBFFFu);         /* TVD_WRITE32(0x570, 0xFF29DEC5); */
	TVD_WRITE32(0x574u, 0xCA8B0F00u);
	TVD_WRITE32(0x578u, 0x797A8E61u);
	TVD_WRITE32(0x57Cu, 0x420FA8A1u);
	/* Read Only
	TVD_WRITE32(0x080, 0x3C181100);
	TVD_WRITE32(0x084, 0x04B01603);
	TVD_WRITE32(0x0A4, 0xFB03FC03);
	*/
	TVD_WRITE32(0x630u, 0x0119812Cu);
	TVD_WRITE32(0x634u, 0x01000844u);
	TVD_WRITE32(0x638u, 0x88020818u);
	TVD_WRITE32(0x63Cu, 0x0FF0AA1Fu);
	/* TVD_WRITE32(0x434, 0xC4D00301); */
	TVD_SET_BIT(0x424, 0x1 << 31);
	TVD_WRITE32_MASK(0x424, (0x000000 << 0), 0xFFFFFF);
	TVD_LOG(TVD_LOG_LVL_INFO, "vTvd_NTSC_Setting leave\n");
}


/**************************************************************************
* @brief  TVD PAL Setting.
*     The setting is experience value, for MT5365 TVD PAL Dump registers setting.
* @param
* @return None
**************************************************************************/
void vTvd_PAL_Setting(void)
{
	TVD_LOG(TVD_LOG_LVL_DBG, "vTvd_PAL_Setting\n");
	/* MT5365 TVD PAL Dump registers setting.*/
	TVD_WRITE32(0x4C0u, 0x51515C55u);
	TVD_WRITE32(0x4C4u, 0x04555D45u);
	TVD_WRITE32(0x4C8u, 0x27F7595Eu);
	TVD_WRITE32(0x4CCu, 0x8F34039Cu);
	TVD_WRITE32(0x4D0u, 0x15890F80u);
	TVD_WRITE32(0x4D4u, 0x26144040u);
	TVD_WRITE32(0x4D8u, 0x0A00F600u);
	TVD_WRITE32(0x4DCu, 0xF41F0800u);
	TVD_WRITE32(0x4E0u, 0x13178080u);
	TVD_WRITE32(0x4E4u, 0x8AA7075Eu);
	TVD_WRITE32(0x4E8u, 0xF08400A0u);
	TVD_WRITE32(0x4ECu, 0x08180C40u);
	TVD_WRITE32(0x4F0u, 0x2008C2C9u);
	TVD_WRITE32(0x4F4u, 0x7180001Fu);
	TVD_WRITE32_MASK(0x4F8u, 0x8000306Cu, 0xFFFFFFeFu);     /* TVD_WRITE32(0x4F8, 0x8000306C); */
	TVD_WRITE32(0x4FCu, 0x420C35A0u);
	TVD_WRITE32(0x500u, 0x6838FA27u);
	TVD_WRITE32(0x504u, 0x0DFA2090u);
	TVD_WRITE32(0x508u, 0x08E06B40u);
	TVD_WRITE32(0x50Cu, 0x83608016u);
	TVD_WRITE32(0x510u, 0x40204080u);
	TVD_WRITE32(0x514u, 0x80326485u);
	TVD_WRITE32(0x518u, 0x11A0A04Cu);
	TVD_WRITE32(0x51Cu, 0x8040A0C0u);
	TVD_WRITE32(0x520u, 0x7E35AF80u);
	TVD_WRITE32(0x524u, 0x0408F840u);
	TVD_WRITE32(0x528u, 0x5404E030u);
	TVD_WRITE32(0x52Cu, 0x30900AA0u);
	TVD_WRITE32(0x530u, 0x0B410B09u);
	TVD_WRITE32(0x534u, 0xF43E40F0u);
	TVD_WRITE32(0x538u, 0x452414F0u);
	TVD_WRITE32(0x53Cu, 0x55C29325u);
	TVD_WRITE32(0xAB0u, 0x22F08C03u);
	TVD_WRITE32(0xAB4u, 0x8F0070D4u);
	TVD_WRITE32(0xAB8u, 0x42400800u);
	TVD_WRITE32(0xABCu, 0x82000820u);
	/* TVD_WRITE32(0x628, 0x08880001); */
	/* Read Only
	TVD_WRITE32(0x040, 0x83001309);
	TVD_WRITE32(0x044, 0x00190101);
	TVD_WRITE32(0x048, 0x009F0101);
	TVD_WRITE32(0x04C, 0x0322021E);
	TVD_WRITE32(0x050, 0x084D9F15);
	TVD_WRITE32(0x054, 0x80011006);
	TVD_WRITE32(0x058, 0xFFF00143);
	TVD_WRITE32(0x05C, 0x01001FF4);
	TVD_WRITE32(0x060, 0x02C00000);
	TVD_WRITE32(0x064, 0xFFFF2008);
	TVD_WRITE32(0x09C, 0x00000303);
	*/
	TVD_WRITE32(0x580u, 0x208181D4u);
	TVD_WRITE32(0x584u, 0x28AF88C8u);
	TVD_WRITE32(0x588u, 0xEC844040u);
	TVD_WRITE32(0x58Cu, 0x2854C531u);
	TVD_WRITE32(0x590u, 0x0206E612u);
	TVD_WRITE32(0x594u, 0x4840310Eu);
	TVD_WRITE32(0x598u, 0x21594088u);
	TVD_WRITE32(0x59Cu, 0x29E2C8A6u);
	TVD_WRITE32_MASK(0x5A0u, 0x44800C80u, 0xF7FFFFFFu);
	TVD_WRITE32(0x5A4u, 0x20010000u);
	TVD_WRITE32(0x5A8u, 0x0A4BC680u);
	TVD_WRITE32_MASK(0x5ACu, 0x400110C0u, 0xFFFBFFFFu);
	TVD_WRITE32(0x5B0u, 0x20000400u);        /*    TVD_WRITE32(0x5B0, 0x20180400); */
	TVD_WRITE32(0x5B4u, 0x00F9BC00u);
	TVD_WRITE32(0x5B8u, 0x20002004u);
	TVD_WRITE32(0x5BCu, 0x17E0114Fu);
	TVD_WRITE32(0x5C0u, 0x1F908420u);
	TVD_WRITE32(0x5C4u, 0x19181718u);        /* TVD_WRITE32(0x5C4, 0x1A191515); */
	TVD_WRITE32(0x5C8u, 0x06658819u);
	TVD_WRITE32(0x5CCu, 0x72B91AB9u);
	TVD_WRITE32(0x5D0u, 0x5A1640DBu);
	TVD_WRITE32(0x5D4u, 0x55F401F4u);
	TVD_WRITE32(0x5D8u, 0x1001825Eu);
	/* TVD_WRITE32(0x5DC, 0x700F6E60); */
	TVD_WRITE32(0x5E0u, 0xA0A793A6u);
	TVD_WRITE32(0x5E4u, 0xFACAD1A0u);
	TVD_WRITE32(0x5E8u, 0x7CB83BD4u);
	TVD_WRITE32(0x5ECu, 0x20DD43A9u);
	TVD_WRITE32(0x5F0u, 0x80554329u);
	TVD_WRITE32(0x5F4u, 0x1708A103u);
	TVD_WRITE32(0x5F8u, 0x5001C67Fu);
	TVD_WRITE32(0x5FCu, 0x71000480u);
	/*
	TVD_WRITE32(0x600, 0x3D34035F);
	TVD_WRITE32(0x604, 0xF9228320);
	TVD_WRITE32(0x608, 0x32201501);
	TVD_WRITE32(0x60C, 0x5387A833);
	TVD_WRITE32(0x610, 0x000028AD);
	TVD_WRITE32(0x614, 0x55E86575);
	TVD_WRITE32(0x618, 0xE6CB2224);
	TVD_WRITE32(0x61C, 0x84460080);
	*/
	TVD_WRITE32(0x620u, 0x00400000u);
	TVD_WRITE32(0x624u, 0x7F084330u);
	TVD_WRITE32(0x628u, 0x08880001u);
	TVD_WRITE32(0x62Cu, 0xBF488488u);
	/* Read Only
	TVD_WRITE32(0x088, 0x8700030F);
	TVD_WRITE32(0x08C, 0x2709C270);
	TVD_WRITE32(0x090, 0x50000000);
	TVD_WRITE32(0x094, 0x000D0000);
	TVD_WRITE32(0x098, 0x85C1E001);
	TVD_WRITE32(0x09C, 0x00000303);
	TVD_WRITE32(0x0A0, 0x55D5D46E);
	TVD_WRITE32(0x0A4, 0xFB03FC03);
	TVD_WRITE32(0x0A8, 0x4163E147);
	TVD_WRITE32(0x0AC, 0x00000000);
	*/
	TVD_WRITE32_MASK(0x540u, 0x30173F93u, 0xFFFFFFF0u);    /* TVD_WRITE32(0x540, 0x30173F93); */
	TVD_WRITE32(0x544u, 0x20F41025u);
	TVD_WRITE32(0x548u, 0x408080FEu);
	TVD_WRITE32(0x54Cu, 0x1245482Bu);
	TVD_WRITE32(0x550u, 0xB4000023u);
	TVD_WRITE32(0x554u, 0x04AE0318u);
	TVD_WRITE32(0x558u, 0x106443D0u);
	TVD_WRITE32(0x55Cu, 0x53876575u);
	TVD_WRITE32(0x560u, 0x29E5E0EFu);     /* TVD_WRITE32(0x560, 0x29E5E0EF); */
	TVD_WRITE32(0x564u, 0x2010543Bu);
	TVD_WRITE32(0x568u, 0x204CDEC9u);
	TVD_WRITE32(0x56Cu, 0x204A8181u);
	TVD_WRITE32_MASK(0x570u, 0xFF29DEC5u, 0xFFFFBFFFu);     /*    TVD_WRITE32(0x570, 0xFF29DEC5); */
	TVD_WRITE32(0x574u, 0xCA8B0F00u);
	TVD_WRITE32(0x578u, 0x797A8E61u);
	TVD_WRITE32(0x57Cu, 0x420FA8A1u);
	/* Read Only
	TVD_WRITE32(0x080, 0x1B241500);
	TVD_WRITE32(0x084, 0x04E00651);
	TVD_WRITE32(0x0A4, 0xFB03FC03);
	*/
	TVD_WRITE32(0x630u, 0x0119812Cu);
	TVD_WRITE32(0x634u, 0x01000844u);
	TVD_WRITE32(0x638u, 0x88020818u);
	TVD_WRITE32(0x63Cu, 0x0FF0AA1Fu);
	/* TVD_WRITE32(0x434, 0xC4D00301); */
	TVD_SET_BIT(0x424, 0x1 << 31);
	TVD_WRITE32_MASK(0x424, (0x0F0000 << 0), 0xFFFFFF);
}


/**************************************************************************
* @brief  TVD SECAM Setting.
*     The setting is experience value, for ac83xx TVD SECAM Dump default registers setting.
* @param
* @return None
**************************************************************************/
void vTvd_SECAM_Setting(void)
{
	TVD_LOG(TVD_LOG_LVL_DBG, "vTvd_SECAM_Setting\n");
	/*  ac83xx TVD Dump default registers setting, for SECAM mode. */
	TVD_WRITE32(0x4C0u, 0x51515555u);
	TVD_WRITE32(0x4C4u, 0x04555D45u);
	TVD_WRITE32(0x4C8u, 0x07E7195Eu);
	TVD_WRITE32(0x4CCu, 0x8F350380u);
	TVD_WRITE32(0x4D0u, 0x138C0F00u);
	TVD_WRITE32(0x4D4u, 0x22044040u);
	TVD_WRITE32(0x4D8u, 0x03D7FC29u);
	TVD_WRITE32(0x4DCu, 0xF4400800u);
	TVD_WRITE32(0x4E0u, 0x32048080u);
	TVD_WRITE32(0x4E4u, 0x98A7071Eu);
	TVD_WRITE32(0x4E8u, 0xF08400A0u);
	TVD_WRITE32(0x4ECu, 0x08180C40u);
	TVD_WRITE32(0x4F0u, 0x2008C2C9u);
	TVD_WRITE32(0x4F4u, 0x4180001Fu);
	TVD_WRITE32_MASK(0x4F8u, 0x8000307Cu, 0xFFFFFFeFu);    /*    TVD_WRITE32(0x4F8, 0x800040FD); */
	TVD_WRITE32(0x4FCu, 0x430C12A0u);
	TVD_WRITE32(0x500u, 0x6838FA1Fu);
	TVD_WRITE32(0x504u, 0x01FE2090u);
	TVD_WRITE32(0x508u, 0x08E06B40u);
	TVD_WRITE32(0x50Cu, 0x83028016u);
	TVD_WRITE32(0x510u, 0x40204080u);
	TVD_WRITE32(0x514u, 0x80326485u);
	TVD_WRITE32(0x518u, 0x11A0A060u);
	TVD_WRITE32(0x51Cu, 0x804040C0u);
	TVD_WRITE32(0x520u, 0x0021EF80u);
	TVD_WRITE32(0x524u, 0x0408E040u);
	TVD_WRITE32(0x528u, 0x5404E030u);
	TVD_WRITE32(0x52Cu, 0x30900AA0u);
	TVD_WRITE32(0x530u, 0x0B410B09u);
	TVD_WRITE32(0x534u, 0xF43E40F0u);
	TVD_WRITE32(0x538u, 0x452414F0u);
	TVD_WRITE32(0x53Cu, 0x17215313u);
	TVD_WRITE32(0xAB0u, 0x00000000u);
	TVD_WRITE32(0xAB4u, 0x00000000u);
	TVD_WRITE32(0xAB8u, 0x00000000u);
	TVD_WRITE32(0xABCu, 0x00000000u);
	/* TVD_WRITE32(0x628, 0x08880001); */
	TVD_WRITE32(0x580u, 0x20818314u);
	TVD_WRITE32(0x584u, 0x28AF8808u);
	TVD_WRITE32(0x588u, 0x1C844040u);
	TVD_WRITE32(0x58Cu, 0x2854C531u);
	TVD_WRITE32(0x590u, 0x0206F612u);
	TVD_WRITE32(0x594u, 0x4440310Eu);
	TVD_WRITE32(0x598u, 0x21894088u);
	TVD_WRITE32(0x59Cu, 0x21E29096u);
	TVD_WRITE32_MASK(0x5A0u, 0x04802C80u, 0xF7FFFFFFu);
	TVD_WRITE32(0x5A4u, 0x20010000u);
	TVD_WRITE32(0x5A8u, 0x0A4BC600u);
	TVD_WRITE32_MASK(0x5ACu, 0x000010C0u, 0xFFFBFFFFu);
	TVD_WRITE32(0x5B0u, 0x20008400u);
	TVD_WRITE32(0x5B4u, 0x00F4FC00u);
	TVD_WRITE32(0x5B8u, 0x20002004u);
	TVD_WRITE32(0x5BCu, 0x17E0110Fu);
	TVD_WRITE32(0x5C0u, 0x1F908420u);
	/* TVD_WRITE32(0x5C4, 0x1A1A1617); */
	TVD_WRITE32(0x5C4u, 0x19181718u);
	TVD_WRITE32(0x5C8u, 0x06658019u);
	TVD_WRITE32(0x5CCu, 0x72CE1ACEu);
	TVD_WRITE32(0x5D0u, 0x5A1646DBu);
	TVD_WRITE32(0x5D4u, 0x55F401F4u);
	TVD_WRITE32(0x5D8u, 0x100181FAu);
	TVD_WRITE32(0x5DCu, 0x700C4A60u);
	TVD_WRITE32(0x5E0u, 0xC4C013A6u);
	TVD_WRITE32(0x5E4u, 0xFAC3F8FFu);
	TVD_WRITE32(0x5E8u, 0x7CB83BD4u);
	TVD_WRITE32(0x5ECu, 0x80D54329u);
	TVD_WRITE32(0x5F0u, 0x80554329u);
	TVD_WRITE32(0x5F4u, 0x7648A100u);
	TVD_WRITE32(0x5F8u, 0x5001C67Fu);
	TVD_WRITE32(0x5FCu, 0x31000480u);
	TVD_WRITE32(0x620u, 0x00400000u);
	TVD_WRITE32(0x624u, 0x7F084330u);
	TVD_WRITE32(0x628u, 0x08880001u);
	TVD_WRITE32(0x62Cu, 0xCF888888u);
	TVD_WRITE32_MASK(0x540u, 0x30173F93u, 0xFFFFFFF0u);    /* TVD_WRITE32(0x540, 0x001FBE73); */
	TVD_WRITE32(0x544u, 0x20F41025u);
	TVD_WRITE32(0x548u, 0x30B080FEu);
	TVD_WRITE32(0x54Cu, 0x1245481Bu);
	TVD_WRITE32(0x550u, 0xB4000023u);
	TVD_WRITE32(0x554u, 0x04AE0318u);             /*  TVD_WRITE32(0x554, 0x04AA0050); */
	TVD_WRITE32(0x558u, 0x106543D0u);
	TVD_WRITE32(0x55Cu, 0x53876575u);
	TVD_WRITE32(0x560u, 0x29E5E0EFu);    /* TVD_WRITE32(0x560, 0xA9E0A36F); */
	TVD_WRITE32(0x564u, 0x2010543Bu);
	TVD_WRITE32(0x568u, 0x204CDEC9u);
	TVD_WRITE32(0x56Cu, 0x204A8181u);
	TVD_WRITE32_MASK(0x570u, 0xFF29DEC5u, 0xFFFFBFFFu);
	TVD_WRITE32(0x574u, 0xCA8B0F00u);
	TVD_WRITE32(0x578u, 0x797A8E61u);
	TVD_WRITE32(0x57Cu, 0x420FA8A1u);
	TVD_WRITE32(0x630u, 0x0119812Cu);
	TVD_WRITE32(0x634u, 0x01000844u);
	TVD_WRITE32(0x638u, 0xC8020818u);
	TVD_WRITE32(0x63Cu, 0x0FF0AA1Fu);
	/* TVD_WRITE32(0x434, 0xC4D00301); */

	/* SECAM setting */
	TVD_WRITE32(0x600u, 0x3D34035Fu);
	TVD_WRITE32(0x604u, 0xF9228320u);
	TVD_WRITE32(0x608u, 0x32201501u);
	TVD_WRITE32(0x60Cu, 0x3040A833u);
	TVD_WRITE32(0x610u, 0x000028ADu);
	TVD_WRITE32(0x614u, 0x55E8383Au);
	TVD_WRITE32(0x618u, 0xE6CB2224u);
	TVD_WRITE32(0x61Cu, 0x84460080u);
	TVD_SET_BIT(0x424, 0x1 << 31);
	TVD_WRITE32_MASK(0x424, (0x000000 << 0), 0xFFFFFF);
}

void Tvd_Core_SetUVSwap(bool fgUVSwap)
{
	if (fgUVSwap) {
		TVD_LOG(TVD_LOG_LVL_INFO, "set tvd UVSwap to 1\n");
		TVD_SET_BIT(0x424, 0x1 << 30);
	}
}

static void bTvd_Mode_Detect(void)
{
	u8 u1TvdMode;

	u1TvdMode = bHwTvdMode();

	switch (u1TvdMode) {
	case AV_PAL_N:
		TVD_LOG(TVD_LOG_LVL_WARN, "TVD mode auto detect: PAL_N.\n");
		vTvd_PAL_Setting();
		break;

	case AV_PAL:
		TVD_LOG(TVD_LOG_LVL_WARN, "TVD mode auto detect: PAL.\n");
		vTvd_PAL_Setting();
		/* PmxVerifySetMode(0, 576, 576, 0, 3, 0);  */
		break;

	case AV_PAL_M:
		TVD_LOG(TVD_LOG_LVL_WARN, "TVD mode auto detect: PAL_M.\n");
		vTvd_NTSC_Setting(false);
		break;

	case AV_NTSC:
		TVD_LOG(TVD_LOG_LVL_WARN, "TVD mode auto detect: NTSC.\n");
		vTvd_NTSC_Setting(false);
		break;

	case AV_SECAM:
		TVD_LOG(TVD_LOG_LVL_WARN, "TVD mode auto detect: SECAM.\n");
		vTvd_SECAM_Setting();
		break;

	case AV_PAL_60:
		TVD_LOG(TVD_LOG_LVL_WARN, "TVD mode auto detect: PAL_60.\n");
		vTvd_NTSC_Setting(false);
		break;

	case AV_UNSTABLE:
		TVD_LOG(TVD_LOG_LVL_WARN, "TVD mode auto detect: unstable.\n");
		break;

	case AV_NTSC443:
		TVD_LOG(TVD_LOG_LVL_WARN, "TVD mode auto detect: NTSC443.\n");
		vTvd_NTSC_Setting(true);
		break;

	default:
		TVD_LOG(TVD_LOG_LVL_WARN, "TVD mode auto detect: NONE.\n");
		break;
	}

	/*  vdo scan line front */
	/*   HAL_WRITE32((IO_BASE_VA+0x421E0), 0x008000B4); */
}

void Tvd_Core_SetMode(u32 u4TvdMode)
{
	switch (u4TvdMode) {
	case AV_MODE_PAL:
		TVD_LOG(TVD_LOG_LVL_DBG, "TVD mode set to PAL.\n");
		vTvd_PAL_Setting();
		break;

	case AV_MODE_NTSC443:
		TVD_LOG(TVD_LOG_LVL_DBG, "TVD mode set to NTSC443.\n");
		vTvd_NTSC_Setting(true);
		break;

	case AV_MODE_NTSC:
		TVD_LOG(TVD_LOG_LVL_DBG, "TVD mode set to NTSC.\n");
		vTvd_NTSC_Setting(false);
		break;

	case AV_MODE_SECAM:
		TVD_LOG(TVD_LOG_LVL_DBG, "TVD mode set to SECAM.\n");
		vTvd_SECAM_Setting();
		break;

	default:
		TVD_LOG(TVD_LOG_LVL_ERR, "TVD mode set to NONE.\n");
		break;
	}
}


u32 Tvd_Core_GetMode(void)
{
	u32 u4TvdMode = bHwTvdMode();

	switch (u4TvdMode) {
	case AV_PAL_N:
		TVD_LOG(TVD_LOG_LVL_WARN, "TVD Mode: PAL_N.\n");
		break;

	case AV_PAL:
		TVD_LOG(TVD_LOG_LVL_WARN, "TVD Mode: PAL.\n");
		break;

	case AV_PAL_M:
		TVD_LOG(TVD_LOG_LVL_WARN, "TVD Mode: PAL_M.\n");
		break;

	case AV_NTSC:
		TVD_LOG(TVD_LOG_LVL_WARN, "TVD Mode: NTSC.\n");
		break;

	case AV_SECAM:
		TVD_LOG(TVD_LOG_LVL_WARN, "TVD Mode: SECAM.\n");
		break;

	case AV_PAL_60:
		TVD_LOG(TVD_LOG_LVL_WARN, "TVD Mode: PAL_60.\n");
		break;

	case AV_UNSTABLE:
		TVD_LOG(TVD_LOG_LVL_WARN, "TVD Mode: unstable.\n");
		break;

	case AV_NTSC443:
		TVD_LOG(TVD_LOG_LVL_WARN, "TVD Mode: NTSC443.\n");
		break;

	default:
		TVD_LOG(TVD_LOG_LVL_WARN, "TVD Mode: NONE.\n");
		break;
	}

	return u4TvdMode;
}

/**************************************************************************

* @brief TVD VPRES interrupt status

*     Get the status of VPRES: VPRES  ON ( TVD SIGNAL COME)  /  VPRES OFF (TVD SIGNAL GO)

* @param .

* @return status of VPRES

**************************************************************************/

TVD_SIG_STATE_T tvd_core_get_signal_state(void)
{
	u32 vpres_on_off = TVD_READ32(STA_REG10);
	TVD_SIG_STATE_T signal_status = TVD_SIG_NONE;

	if ((vpres_on_off & VPRES_ON) && !(vpres_on_off & VPRES_OFF)) {
		signal_status = TVD_SIG_READY;
	} else if (!(vpres_on_off & VPRES_ON) && (vpres_on_off & VPRES_OFF)) {
		signal_status = TVD_SIG_LOST;
	} else {
		TVD_LOG(TVD_LOG_LVL_ERR, "Default Signal Status\n");
	}

	return signal_status;
}

/**************************************************************************

* IRQ Functions

**************************************************************************/



/**************************************************************************

* @brief TVD interrupt status clear

*     Clear TVD interrupt status

* @param TVD IRQ status, Wirte 1 to clear status.

* @return None

**************************************************************************/

void Tvd_Core_IrqClear(u32 u4IrqStatus)
{
	TVD_WRITE32(TVD_INTR_STA, u4IrqStatus);
}


/**************************************************************************

* @brief TVD interrupt enable

*     Only setting Vsync / TimerA / Mode Switching / VPRES for ac83xx.

* @param False is Disable, True is Enable

* @return None

**************************************************************************/

void Tvd_Core_IrqEnable(bool fgEnable)
{
	if (fgEnable) {
		TVD_CLR_BIT(TVD_INTR_EN, (INTR_WFF_VSYNC_TVD | INTR_TIMERA_TVD | INTR_MODE_TVD | INTR_VPRES_TVD));
	}

	else {
		TVD_SET_BIT(TVD_INTR_EN, (INTR_WFF_VSYNC_TVD | INTR_TIMERA_TVD | INTR_MODE_TVD | INTR_VPRES_TVD));
	}
}

u32 Tvd_Core_IrqStatus(void)
{
	u32 u4IrqEnable, u4IrqStatus = 0;

	u4IrqEnable = (~(TVD_READ32(TVD_INTR_EN))) &
		      (INTR_WFF_VSYNC_TVD | INTR_TIMERA_TVD | INTR_MODE_TVD | INTR_VPRES_TVD);

	u4IrqStatus = TVD_READ32(TVD_INTR_STA);
	TVD_LOG(TVD_LOG_LVL_HAL, "IrqEnable = %u, IrqStatus = %u\n", u4IrqEnable, u4IrqStatus);

	Tvd_Core_IrqClear(u4IrqStatus);

	/* VERIFY(BIM_ClearIrq(VECTOR_VDOIN)); */

	u4IrqStatus &= u4IrqEnable;
	return u4IrqStatus;
}

#if defined(__ARM2__)
void CKGEN_AgtOnClk(e_CLK_T eAgt)
{
	u32 u4Tmp, u4Reset;

	if (eAgt >= e_CLK_PWM0 && eAgt <= e_CLK_PWM3) {
		u4Tmp = (*((volatile u32 *)(IO_UCV_BASE + REG_RW_CLKGATE_CFG3)));
		u4Reset = (*((volatile u32 *)(IO_UCV_BASE + REG_RW_SYNC_RESET_CFG3)));
	} else {
		u4Tmp = (*((volatile u32 *)(IO_UCV_BASE + REG_RW_CLKGATE_CFG6)));
		u4Reset = (*((volatile u32 *)(IO_UCV_BASE + REG_RW_SYNC_RESET_CFG6)));
	}

	if (eAgt == e_CLK_TVD1) {
		u4Tmp = u4Tmp | (CLK_PDN_TVD1);
		u4Reset = u4Reset | (CLK_RESET_TVD1);
	} else if (eAgt == e_CLK_TVD2) {
		u4Tmp = u4Tmp | (CLK_PDN_TVD2);
		u4Reset = u4Reset | (CLK_RESET_TVD1);
	} else if (eAgt == e_CLK_WRITE_CHANEL) {
		u4Tmp = u4Tmp | (CLK_PDN_WRITE_CHANEL);
		u4Reset = u4Reset | (CLK_RESET_WRITE_CHANEL);
	} else if (eAgt == e_CLK_WRITE_CHANEL_2) {
		u4Tmp = u4Tmp | (CLK_PDN_WRITE_CHANEL2);
		u4Reset = u4Reset | (CLK_RESET_WRITE_CHANEL2);
	} else if (eAgt == e_CLK_PWM0) {
		u4Tmp = u4Tmp | (CLK_PDN_PWM0);
		u4Reset = u4Reset | (CLK_RESET_PWM0);
	} else if (eAgt == e_CLK_PWM1) {
		u4Tmp = u4Tmp | (CLK_PDN_PWM1);
		u4Reset = u4Reset | (CLK_RESET_PWM1);
	} else if (eAgt == e_CLK_PWM2) {
		u4Tmp = u4Tmp | (CLK_PDN_PWM2);
		u4Reset = u4Reset | (CLK_RESET_PWM2);
	} else if (eAgt == e_CLK_PWM3) {
		u4Tmp = u4Tmp | (CLK_PDN_PWM3);
		u4Reset = u4Reset | (CLK_RESET_PWM3);
	}


	if (eAgt >= e_CLK_PWM0 && eAgt <= e_CLK_PWM3) {
		(*((volatile u32 *)(IO_UCV_BASE + REG_RW_CLKGATE_CFG3))) = u4Tmp;
		(*((volatile u32 *)(IO_UCV_BASE + REG_RW_SYNC_RESET_CFG3))) = u4Reset;
	} else {
		(*((volatile u32 *)(IO_UCV_BASE + REG_RW_CLKGATE_CFG6))) = u4Tmp;
		(*((volatile u32 *)(IO_UCV_BASE + REG_RW_SYNC_RESET_CFG6))) = u4Reset;
	}
}


void CKGEN_AgtOffClk(e_CLK_T eAgt)
{
	u32 u4Tmp, u4Reset;

	if (eAgt >= e_CLK_PWM0 && eAgt <= e_CLK_PWM3) {
		u4Tmp = (*((volatile u32 *)(IO_UCV_BASE + REG_RW_CLKGATE_CFG3)));
		u4Reset = (*((volatile u32 *)(IO_UCV_BASE + REG_RW_SYNC_RESET_CFG3)));
	} else {
		u4Tmp = (*((volatile u32 *)(IO_UCV_BASE + REG_RW_CLKGATE_CFG6)));
		u4Reset = (*((volatile u32 *)(IO_UCV_BASE + REG_RW_SYNC_RESET_CFG6)));
	}

	if (eAgt == e_CLK_TVD1) {
		u4Tmp = u4Tmp & (~CLK_PDN_TVD1);
		u4Reset = u4Reset & (~CLK_RESET_TVD1);
	} else if (eAgt == e_CLK_TVD2) {
		u4Tmp = u4Tmp & (~CLK_PDN_TVD2);
		u4Reset = u4Reset & (~CLK_RESET_TVD1);
	} else if (eAgt == e_CLK_WRITE_CHANEL) {
		u4Tmp = u4Tmp & (~CLK_PDN_WRITE_CHANEL);
		u4Reset = u4Reset & (~CLK_RESET_WRITE_CHANEL);
	} else if (eAgt == e_CLK_WRITE_CHANEL_2) {
		u4Tmp = u4Tmp & (~CLK_PDN_WRITE_CHANEL2);
		u4Reset = u4Reset & (~CLK_RESET_WRITE_CHANEL2);
	} else if (eAgt == e_CLK_PWM0) {
		u4Tmp = u4Tmp & (~CLK_PDN_PWM0);
		u4Reset = u4Reset & (~CLK_RESET_PWM0);
	} else if (eAgt == e_CLK_PWM1) {
		u4Tmp = u4Tmp & (~CLK_PDN_PWM1);
		u4Reset = u4Reset & (~CLK_RESET_PWM1);
	} else if (eAgt == e_CLK_PWM2) {
		u4Tmp = u4Tmp & (~CLK_PDN_PWM2);
		u4Reset = u4Reset & (~CLK_RESET_PWM2);
	} else if (eAgt == e_CLK_PWM3) {
		u4Tmp = u4Tmp & (~CLK_PDN_PWM3);
		u4Reset = u4Reset & (~CLK_RESET_PWM3);
	}

	if (eAgt >= e_CLK_PWM0 && eAgt <= e_CLK_PWM3) {
		(*((volatile u32 *)(IO_UCV_BASE + REG_RW_SYNC_RESET_CFG3))) = u4Reset;
		(*((volatile u32 *)(IO_UCV_BASE + REG_RW_CLKGATE_CFG3))) = u4Tmp;
	} else {
		(*((volatile u32 *)(IO_UCV_BASE + REG_RW_SYNC_RESET_CFG6))) = u4Reset;
		(*((volatile u32 *)(IO_UCV_BASE + REG_RW_CLKGATE_CFG6))) = u4Tmp;
	}
}

void CKGEN_AgtSelClk(e_CLK_T eAgt, u32 u4Sel)
{
	u32 u4Tmp;

	if (eAgt < e_CLK_SEL_MAX) {     /* CONFIG 10:*/
		u4Tmp = (*((volatile u32 *)(IO_UCV_BASE + REG_RW_AP_REG10)));

		switch (eAgt) {
		case e_CLK_SEL_PWM3:
			u4Sel = (u4Sel << CLK_REG10_PWM3_SEL_OFFSET) & CLK_REG10_PWM3_SEL_MASK;
			u4Tmp = u4Tmp & (~CLK_REG10_PWM3_SEL_MASK);
			u4Tmp = u4Tmp | u4Sel;
			break;

		case e_CLK_SEL_PWM2:
			u4Sel = (u4Sel << CLK_REG10_PWM2_SEL_OFFSET) & CLK_REG10_PWM2_SEL_MASK;
			u4Tmp = u4Tmp & (~CLK_REG10_PWM2_SEL_MASK);
			u4Tmp = u4Tmp | u4Sel;
			break;

		case e_CLK_SEL_PWM1:
			u4Sel = (u4Sel << CLK_REG10_PWM1_SEL_OFFSET) & CLK_REG10_PWM1_SEL_MASK;
			u4Tmp = u4Tmp & (~CLK_REG10_PWM1_SEL_MASK);
			u4Tmp = u4Tmp | u4Sel;
			break;

		case e_CLK_SEL_PWM0:
			u4Sel = (u4Sel << CLK_REG10_PWM0_SEL_OFFSET) & CLK_REG10_PWM0_SEL_MASK;
			u4Tmp = u4Tmp & (~CLK_REG10_PWM0_SEL_MASK);
			u4Tmp = u4Tmp | u4Sel;
			break;

		default:
			return;
		}

		(*((volatile u32 *)(IO_UCV_BASE + REG_RW_AP_REG10))) = u4Tmp;

	}

}


#endif

void Tvd_ClkOnOff(bool fgOn)
{
	if (fgOn) {
#ifndef __ARM2__
		clk_prepare_enable(clk_ac8317_tvd1);
		//clk_prepare_enable(clk_ac8317_tvd2);
#else
		CKGEN_AgtOnClk(e_CLK_TVD1);
		//CKGEN_AgtOnClk(e_CLK_TVD2);
#endif
	} else {
#ifndef __ARM2__
		if (__clk_is_enabled(clk_ac8317_tvd1)) {
			clk_disable_unprepare(clk_ac8317_tvd1);
		}
		//clk_disable_unprepare(clk_ac8317_tvd2);
#else
		CKGEN_AgtOffClk(e_CLK_TVD1);
		//CKGEN_AgtOffClk(e_CLK_TVD2);

#endif
	}
}

void Tvd_Register_Rst(void)
{
	TVD_SET_BIT(0x400, 0x1 << 0); /* reset register  */
	TVD_SET_BIT(0x400, 0x1 << 2); /* reset TVD3D_core  */

	/*because it is level trigger,so we need to recovery it*/
	TVD_CLR_BIT(0x400, 0x1 << 0); /* reset register  */
	TVD_CLR_BIT(0x400, 0x1 << 2); /* reset TVD3D_core  */
}

/**************************************************************************

* @brief TVD Initialization.

* @param

* @return None

**************************************************************************/
u32 Tvd_Core_Init(void)
{
	/* u32 uErrorCode = ERROR_NONE; */
	TVD_LOG(TVD_LOG_LVL_TRACE, "enter\n");

	TVD_SET_BIT(0x56c, 0x1 << 8); /* fix mode detect error  */
	TVD_CLR_BIT(SNOW_MODE, 0x1 << 8); /* clear force output snow  */
	/*  auto output snow */
	/* TVD_SET_BIT(SNOW_MODE, 0x1<<9);  */
	TVD_CLR_BIT(SNOW_MODE, 0x1 << 9); /* do not auto output snow */

	/* set default value---0x424  */
	/* enable line average */
	/* TVD_CLR_BIT(0x424, (0x1<<31)); */

	/* the vCVBS_init is need in verification, and not need in emulation */
#if (!TVD_DRV_FPGA_BOARD)
	vCVBS_Init();
	TVD_LOG(TVD_LOG_LVL_WARN, "ASIC.\n");
#else
	/* only FPGA to make clock inverse to stable sync level and blank level */
	TVD_WRITE32_MASK(REG_VFE_00, 0x0F, 0x02);
	TVD_LOG(TVD_LOG_LVL_WARN, "FPGA.\n");
#endif

	TVD_SET_BIT(REG_VSRC_07, RG_VSRC_INV_AIDX);  /* select CHA to TVD */
	/* TVD_SET_BIT(REG_DFE_0E, VPRES4PIC_MODE); */
	TVD_CLR_BIT(REG_DFE_0E, VPRES4PIC_MODE);
	TVD_WRITE32(0x5DC, 0x700C4A60);
	/* TVD_WRITE32_MASK(0x654, 0x08, 0x08); */
	/* Burst frequency detection threshold, Disable for no burst det443 detection control */
	TVD_WRITE32_MASK(0x570, (0 << 14), (1 << 14));
	TVD_SET_BIT(REG_CDET_00, MODE000);    /* init tvd mode is : PAL              A7F540[4]= 1 */
	/* TVD_CLR_BIT(REG_CDET_00,MODE000);    */
	/* init tvd mode is :NTSC50   A7F540[4]= 0    */
	TVD_CLR_BIT(REG_CDET_00, DET443_SEL);
	/*   Tvd_Core_IrqClear(0xFFFFFFFF); */
	/*   Tvd_Core_IrqEnable(true); */

	TVD_WRITE32(0x418, 0xFF001996);
	TVD_WRITE32(0x41C, 0x1EBD0010);
	TVD_WRITE32(0x420, 0x15710000);
	TVD_CLR_BIT(0x424, 0x1<<31);
	TVD_WRITE32_MASK(0x424, (0x0F0000 << 0), 0xFFFFFF);
	bTvd_Mode_Detect();
	vTvd_Comb_Setting();
	/*  set default brightness to 2000; */
	/* TVD_SET_BIT(COLOR_PROCESS_S0, 0x1<<8); */
	/* TVD_SET_BIT(COLOR_PROCESS_S0, 0x1<<10); */
	TVD_LOG(TVD_LOG_LVL_DBG, "leave\n");
	return ERROR_NONE;
}

/**************************************************************************

* @brief TVD Initialization.

* @param

* @return None

**************************************************************************/

void Tvd_Core_DeInit(void)
{
	TVD_LOG(TVD_LOG_LVL_OFF, "enter and disable Tvd Interrupt!\n");
	Tvd_Core_IrqEnable(false);
	Tvd_Core_IrqClear(0xFFFFFFFF);
	TVD_LOG(TVD_LOG_LVL_DBG, "leave\n");
}

void Tvd_Color_Process(TVD_CORE_PREVIEW_CFG_T *prPreviewCfg)
{
#if SUPPORT_CALIBRATE_BRIGHTNESS

	u32 u4ColorProcessValS0;
	u32 u4ColorProcessValS1;
	u32 u4ColorProcessValS2;
	u32 u4ColorProcessValS3;

	u8 u1Mask = prPreviewCfg->u1Mask;
	u8 u1Ygain = prPreviewCfg->u1YGain;
	u8 u1Yoffset = prPreviewCfg->u1YOffset;
	u8 u1Uoffset = prPreviewCfg->u1UOffset;
	u8 u1Voffset = prPreviewCfg->u1VOffset;
	u8 u1Ucosgain = prPreviewCfg->u1UCosGain;
	u8 u1Usingain = prPreviewCfg->u1USinGain;
	u8 u1Vcosgain = prPreviewCfg->u1VCosGain;
	u8 u1Vsingain = prPreviewCfg->u1VSinGain;

	/* if you set YGAIN 0~7 bit */
	if (u1Mask & YGAIN_VALID_MASK) {
		u4ColorProcessValS0 = TVD_READ32(COLOR_PROCESS_S0);
		u4ColorProcessValS0 = u4ColorProcessValS0 & 0xFFFF0000;
		u1Ygain = u1Ygain & (u8)0xFFFF;
		u4ColorProcessValS0 = u4ColorProcessValS0 | u1Ygain;
		TVD_WRITE32(COLOR_PROCESS_S0, u4ColorProcessValS0);
		TVD_LOG(TVD_LOG_LVL_OFF, "u4ColorProcessValS0 %u\n", u4ColorProcessValS0);
	}

	/* if you set UCosgain 16~31 and USingain 0~15 */
	if ((u1Mask & UCOSGAIN_VALID_MASK) && (u1Mask & USINGAIN_VALID_MASK)) {
		u4ColorProcessValS1 = TVD_READ32(COLOR_PROCESS_S1);
		u4ColorProcessValS1 = u4ColorProcessValS1 & 0x0U;
		u4ColorProcessValS1 = (u32)((u32)u1Ucosgain << 16) | u1Usingain;
		TVD_WRITE32(COLOR_PROCESS_S1, u4ColorProcessValS1);
		TVD_LOG(TVD_LOG_LVL_OFF, "u4ColorProcessValS1 %u\n", u4ColorProcessValS1);
	}

	/* if you set VCosgain 16~31 and VSingain 0~15 */
	if ((u1Mask & VCOSGAIN_VALID_MASK) && (u1Mask & VSINGAIN_VALID_MASK)) {
		u4ColorProcessValS2 = TVD_READ32(COLOR_PROCESS_S2);
		u4ColorProcessValS2 = u4ColorProcessValS2 & 0x0U;
		u4ColorProcessValS2 = (u32)((u32)u1Vcosgain << 16) | u1Vsingain;
		TVD_WRITE32(COLOR_PROCESS_S2, u4ColorProcessValS2);
		TVD_LOG(TVD_LOG_LVL_OFF, "u4ColorProcessValS2 %u\n", u4ColorProcessValS2);
	}

	/* if you set u4Yoffset 16~23 and u4Uoffset 8~15 u4Voffset 0~7bit */
	if (u1Mask & (YOFFSET_VALID_MASK | UOFFSET_VALID_MASK | VOFFSET_VALID_MASK)) {
		u4ColorProcessValS3 = TVD_READ32(COLOR_PROCESS_S3);
		u4ColorProcessValS3 = u4ColorProcessValS3 & 0xFF000000U;
		u4ColorProcessValS3 |= (((u32)u1Yoffset << 16) &  0x00FF0000U);
		u4ColorProcessValS3 |= (((u32)u1Uoffset << 8) & 0x0000FF00U);
		u4ColorProcessValS3 |= (u1Voffset & 0x000000FFU);
		TVD_WRITE32(COLOR_PROCESS_S3, u4ColorProcessValS3);
		TVD_LOG(TVD_LOG_LVL_OFF, "u4ColorProcessValS3 %u\n", u4ColorProcessValS3);
	}

#endif
}



/**************************************************************************

* @brief TVD config.

* @param

* @return None

**************************************************************************/

void Tvd_Core_Config(TVD_CORE_PREVIEW_CFG_T *prPreviewCfg)
{
#if SUPPORT_CALIBRATE_BRIGHTNESS

	TVD_LOG(TVD_LOG_LVL_INFO, "u1Mask %x\n", prPreviewCfg->u1Mask);
	TVD_LOG(TVD_LOG_LVL_INFO, "u1YGain %x\n", prPreviewCfg->u1YGain);
	TVD_LOG(TVD_LOG_LVL_INFO, "u1YOffset %x\n", prPreviewCfg->u1YOffset);
#endif
#if (TVD_DRV_FPGA_BOARD)
	/*   vdo scan line */
	/*   HAL_WRITE32((IO_BASE_VA + 0x421E0), 0x008000B4); */
#endif

	if (NULL == prPreviewCfg) {
		TVD_LOG(TVD_LOG_LVL_WARN, "Tvd_Core_Config hasn't changed\n");
		return;
	}

#if SUPPORT_CALIBRATE_BRIGHTNESS
	Tvd_Color_Process(prPreviewCfg);
#endif
}

void TVD_Enable_ManualMode(void)
{
	TVD_SET_BIT(REG_CDET_00, TVD_MMODE);

	TVD_CLR_BIT(REG_CDET_00, SECAM_EN);
	TVD_CLR_BIT(REG_CDET_00, PAL60_EN);
	TVD_CLR_BIT(REG_CDET_00, PALM_EN);
	TVD_CLR_BIT(REG_CDET_00, PALN_EN);
	TVD_CLR_BIT(REG_CDET_00, NTSC443_EN);
}

void TVD_Disable_ManualMode(void)
{
	TVD_LOG(TVD_LOG_LVL_WARN, "Disable ManualMode!\n");
	TVD_CLR_BIT(REG_CDET_00, TVD_MMODE);
	/* TVD_CLR_BIT(REG_CDET_00, TVD_MODE()) */

	TVD_SET_BIT(REG_CDET_00, SECAM_EN);
	TVD_SET_BIT(REG_CDET_00, PAL60_EN);
	TVD_SET_BIT(REG_CDET_00, PALM_EN);
	TVD_SET_BIT(REG_CDET_00, PALN_EN);
	TVD_SET_BIT(REG_CDET_00, NTSC443_EN);
}

void _TVD_Set_ManualSigMode(u32 u4SigMode)
{
	TVD_LOG(TVD_LOG_LVL_WARN, "_TVD Set Manual signal mode as %u\n", u4SigMode);
	TVD_WRITE32_MASK(REG_CDET_00, u4SigMode, 0X7);
}


void TVD_ATV_Mode_Set(u32 u4AtvMode)
{
	switch (u4AtvMode) {
	case TVD_PAL_M:
	case TVD_NTSC:
	case TVD_PAL_60:
		TVD_LOG(TVD_LOG_LVL_WARN, "ATV Set mode as NTSC\n");
		vTvd_NTSC_Setting(false);
		break;

	case TVD_NTSC443:
		TVD_LOG(TVD_LOG_LVL_WARN, "ATV Set mode as NTSC443\n");
		vTvd_NTSC_Setting(true);
		break;
	case TVD_PAL_N:
	case TVD_PAL:
		TVD_LOG(TVD_LOG_LVL_WARN, "ATV Set mode as PAL\n");
		vTvd_PAL_Setting();
		break;

	case TVD_SECAM:
		TVD_LOG(TVD_LOG_LVL_WARN, "ATV Set mode as SECAM\n");
		vTvd_SECAM_Setting();
		break;

	default:
		TVD_LOG(TVD_LOG_LVL_WARN, "ATV Mode not support ! %u\n", u4AtvMode);
		break;
	}

	_TVD_Set_ManualSigMode(u4AtvMode);
}

void Tvd_Snow_Mode(void)
{
	u32 u4SnowModeVal;

	u4SnowModeVal = TVD_READ32(SNOW_MODE);
	u4SnowModeVal = u4SnowModeVal & 0xFFFFFCFFU;
	u4SnowModeVal = u4SnowModeVal | (SNOW_MODE_ON);
	TVD_WRITE32(SNOW_MODE, u4SnowModeVal);
}

void Tvd_Snow_Mode_Alaways(void)
{
	u32 u4SnowModeVal;

	u4SnowModeVal = TVD_READ32(SNOW_MODE);
	u4SnowModeVal = u4SnowModeVal & 0xFFFFFCFFU;
	u4SnowModeVal = u4SnowModeVal | (SNOW_MODE_ALAWAYS_ON);
	TVD_WRITE32(SNOW_MODE, u4SnowModeVal);
}

void Tvd_Line_Average(bool fgOn)
{
	u32 u4LineAverageVal;

	if (fgOn) {
		u4LineAverageVal = TVD_READ32(LINE_AVERAGE);
		u4LineAverageVal = u4LineAverageVal & (0xEFFFFFFFU);
		u4LineAverageVal = u4LineAverageVal | (LINE_AVERAGE_ON);
		TVD_WRITE32(LINE_AVERAGE, u4LineAverageVal);
	} else {
		u4LineAverageVal = TVD_READ32(LINE_AVERAGE);
		u4LineAverageVal = u4LineAverageVal & (0xEFFFFFFFU);
		u4LineAverageVal = u4LineAverageVal | (LINE_AVERAGE_OFF);
		TVD_WRITE32(LINE_AVERAGE, u4LineAverageVal);
	}
}



void Tvd_Ana_IO_PowerOn(bool fgOnOff)
{
	u32 u4Tmp;

	TVD_LOG(TVD_LOG_LVL_DBG, "Tvd_Ana_IO_PowerOn is %d\n", fgOnOff);

	if (fgOnOff) {
		if (gfgAnaIOPwrOn) {
			return;
		}

		u4Tmp = (u32)(RG_CVBS_PWD | RG_PROT_PWD | RG_INMUX_PWD | RG_CLAMP_PWD);
		TVD_WRITE32_MASK(REG_VFE_00, (~u4Tmp), u4Tmp);
		TVD_CLR_BIT(REG_VFE_01, RG_CVBSADC_PWD);
		/*  AUADC GLB bias PWD */
		HAL_WRITE32(IO_BASE_VA + 0x340, (HAL_READ32(IO_BASE_VA + 0x340) & (~(1 << 14))));

		gfgAnaIOPwrOn = true;
	} else {
		if (!gfgAnaIOPwrOn) {
			return;
		}

		u4Tmp = (RG_CVBS_PWD | RG_PROT_PWD | RG_INMUX_PWD | RG_CLAMP_PWD);
		TVD_WRITE32_MASK(REG_VFE_00, (u4Tmp), u4Tmp);
		TVD_SET_BIT(REG_VFE_01, RG_CVBSADC_PWD);
		/*  AUADC GLB bias PWD */
		HAL_WRITE32(IO_BASE_VA + 0x340, (HAL_READ32(IO_BASE_VA + 0x340) | ((1 << 14))));

		gfgAnaIOPwrOn = false;
	}
}

/**************************************************************************
* @brief  CVBS (TVD Analog) initialize Setting.
*     The function for CVBS initialize, only realchip valid.
* @param
* @return None
**************************************************************************/
void vChannelA_Init(void)
{
	/* A channel enable */
	TVD_SET_BIT(REG_VFE_00, RG_UPDN); /*  cvbd enable clamp on  blank for CHA */
	TVD_SET_BIT(REG_VFE_00, RG_VAGSELA); /* cvbs channel A    PGA CM buffer input  0.5v */
	TVD_CLR_BIT(REG_VFE_00, RG_PGABUFNA_PWD); /* cvbs  channel A  input  BUFFER  power on */
	TVD_CLR_BIT(REG_VFE_00, RG_SHIFTA_PWD); /*  cvbs channel A shift power on */
	TVD_CLR_BIT(REG_VFE_00, RG_OFFCURA_PWD); /*  cvbs channel A offset current power on */
}

void vChannelB_Init(void)
{
	/*  B Channel enable */
	TVD_SET_BIT(REG_VFE_03, RG_BTM_EN); /*  cvbs enable clamp on bottom */
	TVD_SET_BIT(REG_VFE_03, RG_VAGSELB); /* cvbs channel B PGA CM buffer input 0.5v */
	TVD_CLR_BIT(REG_VFE_03, RG_PGABUFNB_PWD); /* cvbs channel input  BUFFER  power on */
	TVD_CLR_BIT(REG_VFE_03, RG_SHIFTB_PWD); /* cvbs channel B shift power on */
	TVD_CLR_BIT(REG_VFE_03, RG_OFFCURB_PWD); /*cvbs channel B offset current power on */

	TVD_WRITE32_MASK(REG_VFE_03, (0x04 << 16), 0x3F << 16); /* the setting is same with mt3360 */
}

/**************************************************************************
* @brief  CVBS (TVD Analog) initialize Setting.
*     The function for CVBS initialize, only realchip valid.
* @param
* @return None
**************************************************************************/
void vCVBS_Init(void)
{
	/* top layer control register */
	TVD_CLR_BIT(REG_VFE_00, RG_CVBS_PWD);   /*cvbs power on */
	TVD_CLR_BIT(REG_VFE_01, RG_CVBSADC_PWD); /* cvbs ADC power on */
	TVD_CLR_BIT(REG_VFE_00, RG_CLAMP_PWD);   /*  input clamp power on */
	TVD_CLR_BIT(REG_VFE_02, RG_CVBSADC_SEL_CKPLL);    /*  ADC ref_clk select(0: Pll , 1 XTAL) */
	TVD_CLR_BIT(REG_VFE_00, RG_INMUX_PWD);
	/* RG_GLB_PWD */
	/* HAL_WRITE32((IO_BASE_VA+0x6A0), 0x00240000); */
	/* RG_GLB_PWD only bit 17 we need to care */
	HAL_WRITE32((IO_BASE_VA + 0x6A0), HAL_READ32(IO_BASE_VA + 0x6A0) & (0xFFFDFFFF));
	vChannelA_Init();
}

void Tvd_CHA_PowerOn(bool fgOnOff)
{
	u32 u4Tmp;

	TVD_LOG(TVD_LOG_LVL_DBG, "Tvd_CHA_PowerOn is %d\n", fgOnOff);

	if (fgOnOff) {
		if (gfgCHAPwrOn) {
			return;
		}

		u4Tmp = (RG_PGABUFNA_PWD | RG_OFFCURA_PWD | RG_SHIFTA_PWD);

		TVD_WRITE32_MASK(REG_VFE_00, (~u4Tmp), u4Tmp);

		gfgCHAPwrOn = true;
	} else {
		if (!gfgCHAPwrOn) {
			return;
		}

		u4Tmp = (RG_PGABUFNA_PWD | RG_OFFCURA_PWD | RG_SHIFTA_PWD);
		TVD_WRITE32_MASK(REG_VFE_00, (u4Tmp), u4Tmp);

		gfgCHAPwrOn = false;
	}
}

void Tvd_CHB_PowerOn(bool fgOnOff)
{
	TVD_LOG(TVD_LOG_LVL_DBG, "Tvd_CHB_PowerOn is %d\n", fgOnOff);

	if (fgOnOff) {
		if (gfgCHBPwrOn) {
			return;
		}

		HAL_WRITE32(IO_BASE_VA + 0x280, (HAL_READ32(IO_BASE_VA + 0x280) & (~(0x7 << 21)))); /* CHB power up */
		/*  CHB VCM select 0.5V */
		HAL_WRITE32(IO_BASE_VA + 0x280, (HAL_READ32(IO_BASE_VA + 0x280) | (1 << 30)));
		/*  CHB PGA gain control */
		HAL_WRITE32(IO_BASE_VA + 0x260, (HAL_READ32(IO_BASE_VA + 0x260) | (((~0x3F << 24)) & (0x04 << 24))));

		gfgCHBPwrOn = true;
	} else {
		if (!gfgCHBPwrOn) {
			return;
		}

		HAL_WRITE32(IO_BASE_VA + 0x280, (HAL_READ32(IO_BASE_VA + 0x280) | ((0x7 << 21)))); /* CHB power down */

		gfgCHBPwrOn = false;
	}
}

void CVBS_ByPass_Sel(u32 u4Channel)
{
	if (u4Channel == TVD_CHA_BYPASS) {
		TVD_SET_BIT(REG_SYS_00_RST_CTRL, (u32)((u32)1 << 17)); /*  select CHA by pass to rear */

		TVD_LOG(TVD_LOG_LVL_HAL, "CHA by pass to rear.\r\n");
	} else if (u4Channel == TVD_CHB_BYPASS) {
		TVD_CLR_BIT(REG_SYS_00_RST_CTRL, (u32)((u32)1 << 17)); /* select CHB by pass to rear */

		TVD_LOG(TVD_LOG_LVL_HAL, "CHB by pass to rear.\r\n");
	} else {
		TVD_LOG(TVD_LOG_LVL_HAL, "CH NONE by pass to rear.\r\n");
	}
}

bool CVBS_By_Pass(u32 u4CH, u32 u4CVBSInP, u32 u4BypassChannel, u32 u4CfgType)
{
	u32 u4CHA_CVBSxP, u4CHB_CVBSxP;

	TVD_LOG(TVD_LOG_LVL_DBG, "CVBS_By_Pass!\n");
	/* Tvd_Ana_IO_PowerOn(true); */

	if (TVD_ANALOG_CFG_CLAMP == u4CfgType) {
		return true;
	}

	if (u4CH == TVD_CHA) {
		vChannelA_Init();

		switch (u4CVBSInP) {
		case 1:
			TVD_LOG(TVD_LOG_LVL_HAL, "select cvbs1p to CHA !\r\n");
			TVD_SET_BIT(REG_VFE_02, RG_CVBS_REV_2); /*  select CVBS0P */
			TVD_WRITE32_MASK(REG_VFE_00, (0 << 8), RG_AISEL);
			break;

		case 2:
			TVD_LOG(TVD_LOG_LVL_HAL, "select cvbs2p to CHA !\r\n");
			TVD_CLR_BIT(REG_VFE_02, RG_CVBS_REV_2); /*  no select CVBS0P to CHA */
			TVD_WRITE32_MASK(REG_VFE_00, (1 << 8), RG_AISEL);     /* select CVBS1P to CHA */
			break;

		case 3:
			TVD_LOG(TVD_LOG_LVL_HAL, "select cvbs3p to CHA !\r\n");
			TVD_CLR_BIT(REG_VFE_02, RG_CVBS_REV_2); /* no select CVBS0P to CHA */
			TVD_WRITE32_MASK(REG_VFE_00, (2 << 8), RG_AISEL);     /* select CVBS2P to CHA */
			break;

		case 4:
			TVD_LOG(TVD_LOG_LVL_HAL, "select cvbs4p to CHA !\r\n");
			TVD_CLR_BIT(REG_VFE_02, RG_CVBS_REV_2); /* no select CVBS0P to CHA */
			TVD_WRITE32_MASK(REG_VFE_00, (4 << 8), RG_AISEL);     /* select CVBS3P to CHA */
			break;

		case 5:
			TVD_LOG(TVD_LOG_LVL_HAL, "select cvbs5p to CHA !\r\n");
			TVD_CLR_BIT(REG_VFE_02, RG_CVBS_REV_2); /*  no select CVBS0P to CHA */
			TVD_WRITE32_MASK(REG_VFE_00, (8 << 8), RG_AISEL);     /*  select CVBS4P to CHA */
			break;

		default:
			TVD_LOG(TVD_LOG_LVL_WARN, "CHA CVBS IN xP is NONE\r\n");
			TVD_LOG(TVD_LOG_LVL_WARN, "CHB CVBS IN xP is NONE\r\n");
			TVD_CLR_BIT(REG_VFE_02, RG_CVBS_REV_2);  /* no select CVBS0P to CHA */
			TVD_WRITE32_MASK(REG_VFE_00, 0, RG_AISEL);      /*  select CVBSNONE to CHA */

			/*TVD_SET_BIT(REG_SYS_00_RST_CTRL, 0x4);*/
			/*TVD_CLR_BIT(REG_SYS_00_RST_CTRL, 0x4);*/

			break;
		}

		TVD_LOG(TVD_LOG_LVL_HAL, "Select CHA and port num is %u\n", u4CVBSInP);
	} else {
		switch (u4CVBSInP) {
			vChannelB_Init();

		case 0:
			TVD_WRITE32_MASK(REG_VFE_00, (1 << 0), RG_VIDEOBYPASS);     /*  select CVBS0P to CHB */
			break;

		case 1:
			TVD_WRITE32_MASK(REG_VFE_00, (2 << 0), RG_VIDEOBYPASS);     /*  select CVBS1P to CHB */
			break;

		case 2:
			TVD_WRITE32_MASK(REG_VFE_00, (4 << 0), RG_VIDEOBYPASS);     /*  select CVBS2P to CHB */
			break;

		case 3:
			TVD_WRITE32_MASK(REG_VFE_00, (0x8 << 0), RG_VIDEOBYPASS);     /*  select CVBS3P to CHB */
			break;

		case 4:
			TVD_WRITE32_MASK(REG_VFE_00, (0x10 << 0), RG_VIDEOBYPASS);     /*  select CVBS4P to CHB */
			break;

		default:
			TVD_LOG(TVD_LOG_LVL_WARN, "CHB CVBS IN xP is NONE\n");
			TVD_WRITE32_MASK(REG_VFE_00, 0, RG_VIDEOBYPASS);              /*  select CVBS0P to CHB */
			break;
		}

		TVD_LOG(TVD_LOG_LVL_HAL, "Select CHB and port num is %u\n", u4CVBSInP);
	}

	if ((TVD_READ32(REG_VFE_02) & RG_CVBS_REV_2) == RG_CVBS_REV_2) {
		u4CHA_CVBSxP = 0;
	} else {
		u4CHA_CVBSxP = (TVD_READ32(REG_VFE_00) & RG_AISEL) >> 8;

		if (0 == u4CHA_CVBSxP) {
			u4CHA_CVBSxP = CVBSIN_NONE;
		} else if (u4CHA_CVBSxP == 4) {
			u4CHA_CVBSxP = 3;
		} else if (u4CHA_CVBSxP == 8) {
			u4CHA_CVBSxP = 4;
		} else {
			/*nothing*/
		}
	}

	u4CHB_CVBSxP = (TVD_READ32(REG_VFE_00) & RG_VIDEOBYPASS);

	if (u4CHB_CVBSxP == 0) {
		u4CHB_CVBSxP = CVBSIN_NONE;
	} else if (u4CHB_CVBSxP == 1) {
		u4CHB_CVBSxP = 0;
	} else if (u4CHB_CVBSxP == 2) {
		u4CHB_CVBSxP = 1;
	} else if (u4CHB_CVBSxP == 4) {
		u4CHB_CVBSxP = 2;
	} else if (u4CHB_CVBSxP == 0x8) {
		u4CHB_CVBSxP = 3;
	} else if (u4CHB_CVBSxP == 0x10) {
		u4CHB_CVBSxP = 4;
	} else {
		/*nothing*/
	}

	if ((u4CHA_CVBSxP == u4CHB_CVBSxP) && (u4CHA_CVBSxP != CVBSIN_NONE)) {
		/*
		when there is no signal & the same input source,
		CHB BYPASS on->CHA on, CHB will be interfered
		because CLAMP on Bottom -> Blank.
		Add by MTK40136 for BUG: CNB00003119.
		 if (TVD_SIG_GET == gu4CurSignalStatus)
		*/
		TVD_CLR_BIT(REG_VFE_03,    RG_BTM_EN);            /* cvbs disable clamp on bottom */
	} else {
		TVD_SET_BIT(REG_VFE_03,    RG_BTM_EN);            /*  cvbs enable clamp on bottom */
		/* gfgNeedTurnClamp = false; */
	}

	CVBS_ByPass_Sel(u4BypassChannel);

	return true;
}





