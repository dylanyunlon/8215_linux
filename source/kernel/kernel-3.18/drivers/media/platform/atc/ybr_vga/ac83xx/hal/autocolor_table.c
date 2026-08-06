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

#include "autocolor_table.h"


s16  bEFuseCompensation[4][3] = {  /*notice if negative, how to express*/
	{0,  0,  0},    /*YPbPr     ch1, ch2, ch3 3, 5, 11*/
	{0,  0,  0},    /*VGA       ch1, ch2, ch3 2, 2, 4*/
	{0,  0, 0},    /*SCART     ch1, ch2, ch3*/
	{0,  0, 0}     /*VGA_YPBPR ch1, ch2, ch3*/
};

u8  bVGADefaultGainTABLE_100[4][3] = {
	/*MT5395 2010.5.12*/
	{0x87, 0x8F, 0x94},  /*YPBPR    //2009.7.6 MT5388 ECO*/
	{0x4F, 0x4E, 0x51},  /*RGB      //{0x4F, 0x52, 0x53},  //RGB*/
	{0x24, 0x23, 0x26},  /*SCART RGB//{0x26, 0x28, 0x29},  //SCART RGB*/
	{0x2D, 0x26, 0x30}    /*VGA_YPBPR*/

};

u16  bVGADefaultGainTABLE_100_DIGITAL[4][3] = {
	/*  {0xC544, 0xCBCB, 0xCBCB},  //YPBPR    //2010.02.25 MT8223H/L*/
	{0xA840, 0xB57C, 0xB204},/* 2014.08.28 AC8317*/
	{0xCA0D, 0xC7E3, 0xCA0F}, /* 2014.08.28 AC8317//RGB*/
	{0x9514, 0x9514, 0x9514},  /*SCART RGB*/
	{0xC645, 0xC03F, 0xC746}   /*VGA_YPBPR*/

};

u16  bVGADefaultOffsetTABLE_100_DIGITAL[4][3] = {
	{0xFF, 0xFF, 0xFF},  /*YPBPR*/
	{0xFF, 0xFF, 0xFF},  /*RGB*/
	{0xFF, 0xFF, 0xFF},  /*SCART RGB*/
	{0xFF, 0xFF, 0xFF}  /*VGA_YPBPR*/
};



u8  bVGADefaultOffsetTABLE_100[4][3] = {
	/*MT5395*/
	{0x30, 0x6F, 0x73},  /*YPBPR*/
	{0x56, 0x51, 0x58},  /*RGB*/
	{0x31, 0x30, 0x31},  /*SCART RGB*/
	{0x6F, 0x30, 0x73}  /*VGA_YPBPR //{0x78, 0x49, 0x7A}  //VGA_YPBPR*/
};


u8  bVGADefaultGainTABLE_120[4][3] = {
	/*{0x5D, 0x77, 0x79},    //YPBPR*/
	/*{0x49, 0x4B, 0x4C},  //RGB*/
	/*{0x21, 0x22, 0x23},  //SCART RGB*/
	{0x63, 0x7B, 0x7E},  /*YPBPR   //2009.7.6 MT5388 ECO*/
	{0x4F, 0x4E, 0x51},  /*RGB      //{0x4F, 0x52, 0x53},  //RGB*/
	{0x24, 0x23, 0x26},  /*SCART RGB//{0x26, 0x28, 0x29},  //SCART RGB*/
	{0x2D, 0x26, 0x30}    /*VGA_YPBPR*/
};
u8  bVGADefaultOffsetTABLE_120[4][3] = {
	{0x3D, 0x7E, 0x82},  /*YPBPR*/
	{0x56, 0x51, 0x58},  /*RGB*/
	{0x31, 0x30, 0x31},  /*SCART RGB*/
	{0x6F, 0x30, 0x73}  /*VGA_YPBPR //{0x78, 0x49, 0x7A}  //VGA_YPBPR*/
};

u8  bVGADefaultGainTABLE_75_100[4][3] = {
	/*{0x21, 0x2A, 0x2B},    //YPBPR*/
	/*{0x49, 0x4B, 0x4C},  //RGB*/
	/*{0x21, 0x22, 0x23},  //SCART RGB*/
	{0x28, 0x38, 0x38},  /*YPBPR   //2010.5.12 MT5395*/
	{0x4F, 0x4E, 0x51},  /*RGB      //{0x4F, 0x52, 0x53},  //RGB*/
	{0x24, 0x23, 0x26},  /*SCART RGB//{0x26, 0x28, 0x29},  //SCART RGB*/
	{0x2D, 0x26, 0x30}    /*VGA_YPBPR*/
};
u8  bVGADefaultOffsetTABLE_75_100[4][3] = {
	{0x3D, 0x7E, 0x82},  /*YPBPR*/
	{0x56, 0x51, 0x58},  /*RGB*/
	{0x31, 0x30, 0x31},  /*SCART RGB*/
	{0x6F, 0x30, 0x73}  /*VGA_YPBPR //{0x78, 0x49, 0x7A}  //VGA_YPBPR*/
};

/*
u8  bVGADefaultGainTABLE_SONY[4][3] =
{
    {0x88, 0x90, 0x93},  //YPBPR   //2009.7.6 MT5388 ECO
    {0x3F, 0x42, 0x44},  //RGB
    {0x0D, 0x10, 0x11},  //SCART RGB
    {0x2A, 0x21, 0x2B}    //VGA_YPBPR
};
*/

/*
u8  bVGADefaultOffsetTABLE_SONY[4][3] =
{
    {0x3D, 0x7E, 0x82},  //YPBPR
    {0x78, 0x70, 0x80},  //RGB
    {0x36, 0x3B, 0x3E},  //SCART RGB
    {0x7E, 0x3D, 0x82}  //VGA_YPBPR
};
*/

u16  wColorBlankValueNew[4][3] = {
	{((u32)0x10 << 5), ((u32)0x80 << 5), ((u32)0x80 << 5)}, /*YPBPR(5381)*/
	{((u32)0x02 << 5), ((u32)0x02 << 5), ((u32)0x02 << 5)}, /*RGB*/
	{((u32)0x10 << 5), ((u32)0x10 << 5), ((u32)0x10 << 5)}, /*SCART RGB*/
	{((u32)0x80 << 5), ((u32)0x10 << 5), ((u32)0x80 << 5)} /*VGA_YPBPR*/
};

/*Table for 100% input signal*/
u8  bColorMaxType1[4][3] = {  /*Type 1 is for YPbPr 100% signal*/
	{236, 241, 241},     /*YPBPR*/
	{253, 253, 253},  /*RGB*/
	{236, 236, 236},  /*SCART RGB*/
	{241, 236, 241}   /*VGA_YPBPR       //add VGACOMP by Hua 20061116*/
};

/*Table for 120% input signal*/
u8  bColorMaxType2[4][3] = { /*Type 2 is for YPbPr 120% signal*/
	{216, 230, 230},     /* (236-16)*0.9+16=214, (241-128)*0.9+128=229.7*/
	{253, 253, 253},  /*RGB*/
	{236, 236, 236},  /*SCART RGB*/
	{241, 236, 241}   /*VGA_YPBPR       //add VGACOMP by Hua 20061116*/
};

/*Table for SONY input signal*/
u8  bColorMaxType3[4][3] = { /*Type 3 is for SONY*/
	{235, 240, 240},  /*YPbPr*/
	{240, 240, 240},  /*RGB*/
	{216, 216, 216},  /*SCART RGB*/
	{241, 236, 241}   /*VGA_YPBPR*/
};

u16  wOnChipColorMaxType[4][3] = {
	{1809, 1851, 1851}, /*YPBPR(5381) //(236-16)=220, (241-16)=225, (241-16)=225*/
	{2065, 2065, 2065}, /*RGB         //(253-2)=251 , (253-2)=251,  (253-2)=251*/
	{1925, 1925, 1925}, /*SCART RGB   //(236-2)=234 , (236-2)=234,  (236-2)=234*/
	{1851, 1809, 1851}   /*VGA_YPBPR         //*/

	/*
	  {1735, 1774, 1774}, //YPBPR(5381) //(236-16)=220, (241-16)=225, (241-16)=225
	  {1987, 1987, 1987}, //VGA         //(254-2)=252 , (254-2)=252,  (254-2)=252
	  {1845, 1845, 1845}, //SCART RGB   //(236-2)=234 , (236-2)=234,  (236-2)=234
	  {1774, 1735, 1774}   //VGA_YPBPR         //
	*/
};

u16  wOnChipColorGainTable[4][3] = {
	{1100, 1130, 1130}, /*YPBPR(5381) (236-16)/2*74.5/56, (241-128)*74.5/56, (241-128)*74.5/56, all elementa*10*/
	{1255, 1255, 1255}, /*VGA (253-2)/2*/
	{1100, 1100, 1100}, /*SCART RGB (236-16)/2*/
	{1130, 1100, 1130}  /*VGA_YPBPR (236-16)/2*/
};
u16  wOnChipColorGainTable_75[4][3] = {
	{1100, 1130, 1130}, /*YPBPR(5381) (236-16)/2*74/56, (241-128)*74/56, (241-128)*74/56, all elementa*10*/
	{1255, 1255, 1255}, /*VGA (253-2)/2*/
	{1100, 1100, 1100}, /*SCART RGB (236-16)/2*/
	{1130, 1100, 1130}  /*VGA_YPBPR (236-16)/2*/
};

u16  wYPbPrMappingVgaGainTable[4][3] = {
	/*{110, 112, 112}, //YPBPR(5381) //ÂÂTable*/
	/*{126, 126, 126}, //VGA*/
	/*{118, 118, 118}, //SCART RGB*/
	/*{112, 110, 112} //VGA_YPBPR*/
	{1470, 1490, 1490}, /*YPBPR(5381) (236-16)/2*74/56, (241-128)*74/56, (241-128)*74/56, all elementa*10*/
	{1255, 1255, 1255}, /*VGA (253-2)/2*/
	{1100, 1100, 1100}, /*SCART RGB (236-16)/2*/
	{1255, 1255, 1255}  /*VGA_YPBPR (253-2)/2*/
};

/*
u16  wOnChipColorGainTable_SONY[4][3]=
{
    {1460, 1480, 1480}, //YPBPR (235-16)/2*74/56+2, (240-128)*74/56, (240-128)*74/56, all elements*10
    {1190, 1190, 1190}, //VGA (240-2)/2
    {1100, 1100, 1100}, //SCART RGB (236-16)/2
    {1255, 1255, 1255}  //VGA_YPBPR (240-2)/2 //kalcheng
};
*/
u8  bOnChipCalibrateTolerance[5][4] = {
	{0x04, 0x04, 0x30, 0x30},   /*mode=0,  gain calibration by using on chip voltage*/
	{0x0c, 0x0c, 0x40, 0x40},      /*mode=1,  offset calibration by using on chip voltage*/
	{0x0c, 0x0c, 0x40, 0x40},      /*mode=2,   offset calibration  0x20=1  (8bit)*/
	{0x00, 0x00, 0x04, 0x04},      /*mode=3,  gain calibration for external signal*/
	{0x00, 0x00, 0x04, 0x04},      /*mode=4,  digital offset calibration for external signal*/
};

u8  bOnChipCheckTolerance[5][2] = {
	{0x04, 0x04},   /*mode=0,    gain calibration by using on chip voltage*/
	{0x10, 0x10},   /*mode=1,    offset calibration by using on chip voltage*/
	{0x10, 0x10},   /*mode=2,    offset calibration  0x20=1  (8bit)*/
	{0x1F, 0x1F},   /*mode=3,    gain calibration for external signal*/
	{0x00, 0x00},   /*mode=4,    digital offset calibration for external signal*/
};

/* support 100% with 18/56 ohm*/
u8  GAIN_HIGH_LIMIT_100[4][3] = {
	{0xC6, 0xD7, 0xD6},  /*YPbPr*/
	{0x81, 0x85, 0x84},  /*VGA         //{0x70, 0x73, 0x74},  //VGA*/
	{0x52, 0x55, 0x54},  /*SCART R,G,B //{0x43, 0x45, 0x46},  //SCART R,G,B*/
	{0x45, 0x3E, 0x49}   /* VGA_YPBPR*/
};
u8  GAIN_LOW_LIMIT_100[4][3] = {
	{0x55, 0x62, 0x61}, /*YPbPr CH1 : 0x85 > X > 0X48,  CH2: 0xA1 > X > 0X60 , CH3: 0xA1 > X > 0X60*/
	{0x28, 0x2B, 0x2A}, /*VGA {0x31, 0x24, 0x35}, VGA CH1 : 0x73 > X > 0x36, CH2: 0x74>X>0x36,CH3: 0x74 > X > 0x36*/
	{0x04, 0x07, 0x06},
	/*SCART R,G,B{0x0C, 0x0E, 0x0F}, CH1: 0x44>X>0x0F, CH2: 0x44>X > 0x0F, CH3: 0x44 > X > 0x0F*/
	{0x16, 0x10, 0x19}  /* VGA_YPBPR*/
};
/*DIGITAL_NEW_GAIN*/
u16  GAIN_HIGH_LIMIT_100_DIGITAL[4][3] = {
	{0xD958, 0xE0E0, 0xE1E1},  /*YPbPr*/
	{0xD837, 0xD9B9, 0xDA39},  /*VGA*/
	{0xE1A1, 0xE2A2, 0xE322},  /*SCART R,G,B*/
	{0xDBDB, 0xD5D5, 0xDBDB}   /* VGA_YPBPR*/
};
u16  GAIN_LOW_LIMIT_100_DIGITAL[4][3] = {
	/*tempary setting*/
	{0x8000, 0x8000, 0x8000}, /*YPbPr CH1 : 0x85 > X > 0X48,  CH2: 0xA1 > X > 0X60 , CH3: 0xA1 > X > 0X60*/
	{0x7000, 0x7000, 0x7000}, /*VGA    CH1 : 0x73 > X > 0x36,  CH2: 0x74 > X > 0x36   ,CH3: 0x74 > X > 0x36*/
	{0x6000, 0x6000, 0x6000}, /*SCART R,G,B CH1: 0x44> X > 0x0F,  CH2: 0x44 > X > 0x0F,  CH3: 0x44 > X > 0x0F*/
	{0x8000, 0x8000, 0x8000}  /* VGA_YPBPR*/

	/*   {0xB02F, 0xB635, 0xB6B6}, */
	/*   {0x9898, 0x9A19, 0x9A9A}, */
	/*   {0x8605, 0x8706, 0x8787}, */
	/*   {0xB1B1, 0xABAB, 0xB231}  // VGA_YPBPR*/
};

/* support 120% with 18/56 ohm*/
u8  GAIN_HIGH_LIMIT_120[4][3] = {
	{0x9D, 0xBE, 0xBD},  /*YPbPr*/
	{0x81, 0x85, 0x84},  /*VGA*/
	{0x52, 0x55, 0x54},  /*SCART R,G,B*/
	{0x45, 0x3E, 0x49}   /* VGA_YPBPR*/
};

u8  GAIN_LOW_LIMIT_120[4][3] = {
	{0x36, 0x50, 0x45}, /*YPbPr              CH1 : 0x85 > X > 0X48,  CH2: 0xA1 > X > 0X60 , CH3: 0xA1 > X > 0X60*/
	{0x28, 0x2B, 0x2A}, /*VGA               CH1 : 0x73 > X > 0x36,  CH2: 0x74 > X > 0x36   ,CH3: 0x74 > X > 0x36*/
	{0x04, 0x07, 0x06}, /*SCART R,G,B    CH1: 0x44> X > 0x0F,  CH2: 0x44 > X > 0x0F,  CH3: 0x44 > X > 0x0F*/
	{0x16, 0x10, 0x19}  /* VGA_YPBPR*/
};

u8  GAIN_HIGH_LIMIT_75_100[4][3] = {
	{0x52, 0x5F, 0x5E},  /*YPbPr*/
	{0x81, 0x85, 0x84},  /*VGA*/
	{0x52, 0x55, 0x54},  /*SCART R,G,B*/
	{0x45, 0x3E, 0x49}   /* VGA_YPBPR*/
};

u8  GAIN_LOW_LIMIT_75_100[4][3] = {
	{0x04, 0x0E, 0x0D}, /*YPbPr*/
	{0x28, 0x2B, 0x2A}, /*VGA               CH1 : 0x73 > X > 0x36,  CH2: 0x74 > X > 0x36   ,CH3: 0x74 > X > 0x36*/
	{0x04, 0x07, 0x06}, /*SCART R,G,B    CH1: 0x44> X > 0x0F,  CH2: 0x44 > X > 0x0F,  CH3: 0x44 > X > 0x0F*/
	{0x16, 0x10, 0x19}  /* VGA_YPBPR*/
};
/*
u8  GAIN_HIGH_LIMIT_SONY[4] [3]=
{
  {0xC0, 0xC0, 0xC0},  //YPbPr
  {0x60, 0x64, 0x65},  //VGA
  {0x33, 0x35, 0x36},  //SCART R,G,B
  {0xB7, 0xAB, 0xB7}   // VGA_YPBPR
};
*/
/*
u8  GAIN_LOW_LIMIT_SONY[4][3] =
{
  {0x60, 0x6C, 0x6D}, //YPbPr
  {0x20, 0x20, 0x20}, //VGA
  {0x02, 0x02, 0x02}, //SCART R,G,B
  {0x63, 0x57, 0x64}  // VGA_YPBPR
};
*/



