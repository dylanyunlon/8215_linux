#ifndef __SCREEN_HVDETECT_H__
#define __SCREEN_HVDETECT_H__

//Identify wr_channel buffer rotate status algorithm
#define HALF_DIVISION (2)
#define VIDEO_RATIO_4_3  (3.0/4)
#define SCAN_INTERVAL_LINE_DFT_FOR_480P (4)
#define SCAN_INTERVAL_LINE_FOR_720P     (6) 
#define SCAN_INTERVAL_LINE_FOR_1080P    (9) 

typedef struct WFF_YC_VALUE_T{
    unsigned long u4YValue;
    unsigned long u4UValue;
    unsigned long u4VValue;
}WFF_YC_VALUE_T;

typedef enum{
    RECT_NULL,
    BUFFER_LRERR,       //Could not find left and right position!
    BUFFER_TBERR,        //Could not find top and bottom position!
    BUFFER_INACTIVE,
    BUFFER_ROTATE,
    BUFFER_UNROTATE
}RETNO;

typedef enum {
    MODE_NULL = 0x0,
    MODE_BLOCK = 0x1,
    MODE_LINE = 0x2
} VIDEO_MODE_INFO_E;

typedef struct VIDEO_INFO{
    unsigned long u4YVaAddr;
    unsigned long u4CVaAddr;
    unsigned long u4Height;
    unsigned long u4Width;
    VIDEO_MODE_INFO_E u4Mode;
}VIDEO_INFO_T;


typedef struct tagRECT_HV
{
	long left;
	long top;
	long right;
	long bottom;
}RECT_HV, *PRECT_HV;

RETNO GR_GetActiveRect(VIDEO_INFO_T InVdoInfo, PRECT_HV pRect);
    
#endif
