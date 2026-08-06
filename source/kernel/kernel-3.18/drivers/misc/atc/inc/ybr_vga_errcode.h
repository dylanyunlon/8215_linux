#ifndef YPBPR_VGA_ERRCODE_H__
#define YPBPR_VGA_ERRCODE_H__
#include "is_errcode.h"

#define RET_VGA_OK                            0x0
#define RET_VGA_INV_ARG                       MAKE_ERR_CODE(MOD_ERRCODE_VGA, 1)
#define RET_VGA_OUT_OF_MEM                    MAKE_ERR_CODE(MOD_ERRCODE_VGA, 2)
#define RET_VGA_BITRATE_NOT_SUPPORT           MAKE_ERR_CODE(MOD_ERRCODE_VGA, 3)
#define RET_VGA_SAMPLERATE_NOT_SUPPORT        MAKE_ERR_CODE(MOD_ERRCODE_VGA, 4) 
#define RET_VGA_RESOLUTION_NOT_SUPPORT        MAKE_ERR_CODE(MOD_ERRCODE_VGA, 5)

#endif
