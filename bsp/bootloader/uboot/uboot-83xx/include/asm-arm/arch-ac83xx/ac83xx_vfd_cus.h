/********************************************************************************************
 *     LEGAL DISCLAIMER
 *
 *     (Header of MediaTek Software/Firmware Release or Documentation)
 *
 *     BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *     THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE") RECEIVED
 *     FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS
 *     ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED,
 *     INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
 *     A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY
 *     WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK
 *     ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
 *     NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION
 *     OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 *     BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE LIABILITY WITH
 *     RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION,
 *     TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/

#ifndef _VFD_CUS_H_
#define _VFD_CUS_H_

/**Invalide char*/
#define VFD_CHAR_INVALID               0xff
#define VFD_DIGIT_0                              0
#define VFD_DIGIT_1                              1
#define VFD_DIGIT_2                              2
#define VFD_DIGIT_3                              3
#define VFD_DIGIT_4                              4
#define VFD_DIGIT_5                              5
#define VFD_DIGIT_6                              6
#define VFD_DIGIT_7                              7
#define VFD_DIGIT_8                              8
#define VFD_DIGIT_9                              9
#define VFD_CHAR_                                10
#define VFD_CHAR_A                               11
#define VFD_CHAR_a                               12
#define VFD_CHAR_B                               13
#define VFD_CHAR_b                               14
#define VFD_CHAR_C                               15
#define VFD_CHAR_c                               16
#define VFD_CHAR_D                               17
#define VFD_CHAR_d                               18
#define VFD_CHAR_E                               19
#define VFD_CHAR_e                               20
#define VFD_CHAR_F                               21
#define VFD_CHAR_f                               22
#define VFD_CHAR_G                               23
#define VFD_CHAR_g                               24
#define VFD_CHAR_H                               25
#define VFD_CHAR_h                               26
#define VFD_CHAR_I                               27
#define VFD_CHAR_i                               28
#define VFD_CHAR_J                               29
#define VFD_CHAR_j                               30
#define VFD_CHAR_K                               31
#define VFD_CHAR_k                               32
#define VFD_CHAR_L                               33
#define VFD_CHAR_l                               34
#define VFD_CHAR_M                               35
#define VFD_CHAR_m                               36
#define VFD_CHAR_N                               37
#define VFD_CHAR_n                               38
#define VFD_CHAR_O                               39
#define VFD_CHAR_o                               40
#define VFD_CHAR_P                               41
#define VFD_CHAR_p                               42
#define VFD_CHAR_Q                               43
#define VFD_CHAR_q                               44
#define VFD_CHAR_R                               45
#define VFD_CHAR_r                               46
#define VFD_CHAR_S                               47
#define VFD_CHAR_s                               48
#define VFD_CHAR_T                               49
#define VFD_CHAR_t                               50
#define VFD_CHAR_U                               51
#define VFD_CHAR_u                               52
#define VFD_CHAR_V                               53
#define VFD_CHAR_v                               54
#define VFD_CHAR_W                               55
#define VFD_CHAR_w                               56
#define VFD_CHAR_X                               57
#define VFD_CHAR_x                               58
#define VFD_CHAR_Y                               59
#define VFD_CHAR_y                               60
#define VFD_CHAR_Z                               61
#define VFD_CHAR_z                               62
#define VFD_SPACE                                 63
/** & */
#define CLR                                 		 64
/** & */
#define VFD_CHAR__                               65
/** & Horizontal Line: - */
#define VFD_CHAR_HL                             66
/** & Vertical Line: | */
#define VFD_CHAR_VL                              67
/** & Forward Slash: / */
#define VFD_CHAR_FS                              68
/** & Back Slash: \ */
#define VFD_CHAR_BS                              69
/** & Left Square Bracket: [ */
#define VFD_CHAR_LSB                             70
/** & Right Square Bracket: ] */
#define VFD_CHAR_RSB                             71
/** & ALL:  # */
#define VFD_CHAR_ALL                             72
/** & ?:  # */
#define VFD_CHAR_QUES                            73





#define DATA_SET_INC		  0x40 /* increamental address(to display memory) */
#define DATA_SET_FIX   		  0x44       // fixed address(to display memory
#define DATA_SET_LED   		  0x41       // write data to led port
#define ADDR_SET       		  0xC0
#define KEY_SCAN			  0x42

// never change
#define VFD_ADDR_LIMIT        50
#define VFD_UNKNOWN_ADDR      0xff

#define SUPPORT_CUSTOMER_VFD 0
#if SUPPORT_CUSTOMER_VFD
 
#define MAX_SEG_SEQ_NUM       8
#define MAX_DISC_NUM          8
#define SEG_SIZE              14
#define MODESET               4
#define KEY_SCAN_SIZE         0
#define TEST_MAX_VFD_ADDR     48
#define MAX_VFD_ADDR          16
#define DISPMODE_ON 		  0x8f /* set Display ON and Set Pulse width */

enum{
    SEG_MSG_PHILIPS = 0
    ,SEG_MSG_WAIT 
//    ,SEG_MSG_NULL
};

enum{
	IR_NONE = 0xFF
};


#else

#define MAX_SEG_SEQ_NUM       11
#define MAX_DISC_NUM          6
#define SEG_SIZE              14
#define MODESET               13
#define KEY_SCAN_SIZE         16
#define TEST_MAX_VFD_ADDR     48
#define MAX_VFD_ADDR          42
#define DISPMODE_ON 		  0x8c /* set Display ON and Set Pulse width */

#define SEG_MSG_HELLO     	  0
#define SEG_MSG_STOP              1
#define SEG_MSG_OPEN      	  2
#define SEG_MSG_CLOSE      	  3
#define SEG_MSG_OFF       	  4
#define SEG_MSG_ERR       	  5
#define SEG_MSG_HELLOWORLD    6
#define SEG_MSG_UBOOT         7
#define SEG_MSG_CLEAN     	  8

enum{
	IR_RECORD,
	IR_FF,
	IR_CH_UP,
	IR_SW13,
	IR_PLAY,
	IR_FR,
	IR_CH_DOWN,
	IR_SW14,
	IR_STOP,
	IR_NEXT,
	IR_POWER,
	IR_SW15,
	IR_PAUSE,
	IR_PREV,
	IR_EJECT,
	IR_SW16,
	IR_NONE
};
#endif

#endif
