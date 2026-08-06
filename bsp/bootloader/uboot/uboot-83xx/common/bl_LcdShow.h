// Written by ding@ 2008.3.28 : 包含往LCD上显示英文的代码

#ifndef __LCDSHOW_H__
#define __LCDSHOW_H__


#include "x_typedef.h" 

#define FOREGROUNDCOLOR		0xFFFFFFFF		//字符的颜色(白色)
//#define BACKGROUNDCOLOR		0xFF00001F		//背景的颜色(蓝色)
#define BACKGROUNDCOLOR		0xFFFFFFFF		//


#define U8	    unsigned char
#define U16	    unsigned short
#define U32	    unsigned int

void LCD_CleanScreen(void);

void LCD_CleanScreen_ex(void);

void __Delay(UINT32 sec);

void LCD_WriteString(char* str);
void LCD_WriteString_FixedCharNum80(char* str); //display at most 80 chars


void LCD_Enable(BOOL bEnable);

void LCD_PrintNum(int num);

void Processing(void);

void ProgressBar(UINT32 progress);


#endif
