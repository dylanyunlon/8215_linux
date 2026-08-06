/*************************************************************************************
 *LEGAL DISCLAIMER
 *
 * (Header of AutoChips Software/Firmware Release or Documentation)
 *
 * BY OPENING OR USING THIS FILE, USER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND
 * AGREES THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("AUTOCHIPS SOFTWARE")
 * ARE PROVIDED TO USER ON AN "AS-IS" BASIS ONLY. AUTOCHIPS EXPRESSLY DISCLAIMS
 * ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES AUTOCHIPS PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED
 * IN, OR SUPPLIED WITH THE AUTOCHIPS SOFTWARE, AND USER AGREES TO LOOK ONLY TO
 * SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. AUTOCHIPS SHALL
 * ALSO NOT BE RESPONSIBLE FOR ANY AUTOCHIPS SOFTWARE RELEASES MADE TO USER'S
 * SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 * USER HEREBY ACKNOWLEDGES THE CONFIDENTIALITY OF AUTOCHIPS SOFTWARE AND AGREES
 * NOT TO DISCLOSE OR PERMIT DISCLOSURE OF ANY AUTOCHIPS SOFTWARE TO ANY THIRD
 * PARTY OR TO ANY OTHER PERSON, EXCEPT TO DIRECTORS, OFFICERS, EMPLOYEES OF
 * USER WHO ARE REQUIRED TO HAVE THE INFORMATION TO CARRY OUT THE PURPOSE OF
 * OPENING OR USING THIS FILE.
*************************************************************************************/


#ifndef TVP_HAL_H
#define TVP_HAL_H


#include <linux/videodev2.h>
#include <linux/types.h>
#include <linux/mutex.h>


#define TVP_CHIP_ID 						0x5158
#define TVP_SLAVE_ADDR						0x58
#define TVP_I2C_RETRY_TIME					10
#define TVP_START_CODE_SIZE 				4
#define TVP_SUPER_FRAME_HEIGHT_4CH_HALF_D1	2100
#define TVP_MAX_VIDEO_DEVS					4


/*************************TVP5158 Internal Control Registers*************************/
#define TVP_REG_CHIP_ID_MSB 								(0x08)
#define TVP_REG_CHIP_ID_LSB 								(0x09)
#define TVP_REG_VIDEO_STATUS_1								(0x00)
#define TVP_REG_VIDEO_STATUS_2								(0x01)
#define TVP_REG_VIDEO_STANDARD_STATUS						(0x0C)
#define TVP_REG_VIDEO_STANDARD_SELECT						(0x0D)
	#define VIDEO_STD_AUTO_SWITCH				(0x00)
	#define VIDEO_STD_NTSC_MJ					(0x01)
	#define VIDEO_STD_PAL_BDGHIN				(0x02)
	#define VIDEO_STD_PAL_M 					(0x03)
	#define VIDEO_STD_PAL_COMBINATION_N 		(0x04)
	#define VIDEO_STD_NTSC_443					(0x05)
	#define VIDEO_STD_PAL_60					(0x07)
#define TVP_REG_LUMINANCE_BRIGHTNESS						(0x10)
#define TVP_REG_LUMINANCE_CONTRAST							(0x11)
#define TVP_REG_CHROMINANCE_SATURATION						(0x13)
#define TVP_REG_CHROMINANCE_HUE 							(0x14)
#define TVP_REG_AVD_OUTPUT_CONTROL_1						(0xB0)
#define TVP_REG_AVD_OUTPUT_CONTROL_2						(0xB1)
#define TVP_REG_OFM_MODE_CONTROL							(0xB2)
#define TVP_REG_OFM_CHANNEL_SELECT_1						(0xB3)
#define TVP_REG_OFM_CHANNEL_SELECT_2						(0xB4)
#define TVP_REG_OFM_CHANNEL_SELECT_3						(0xB5)
#define TVP_REG_OFM_SUPER_FRAME_SIZE_1						(0xB6)
#define TVP_REG_OFM_SUPER_FRAME_SIZE_2						(0xB7)
#define TVP_REG_OFM_EAV2SAV_DURATION_1						(0xB8)
#define TVP_REG_OFM_EAV2SAV_DURATION_2						(0xB9)
#define TVP_REG_MISC_OFM_CONTROL							(0xBA)
#define TVP_REG_VBUS_DATA_ACCESS_NO_ADDR_INCREMENT			(0xE0)
#define TVP_REG_VBUS_DATA_ACCESS_ADDR_INCREMENT 			(0xE1)
#define TVP_REG_VBUS_ADDRESS_ACCESS_1						(0xE8)
#define TVP_REG_VBUS_ADDRESS_ACCESS_2						(0xE9)
#define TVP_REG_VBUS_ADDRESS_ACCESS_3						(0xEA)
#define TVP_REG_DECODER_WRITE_ENABLE						(0xFE)
#define TVP_REG_DECODER_READ_ENABLE 						(0xFF)
/*******************************************************************************/


/*************************TVP5158 Video Decoder Mode******************************/
#define TVP_VIDEO_DECODER_MODE_1CH_D1_PORT_A			 0	 ///< Video decoder mode: 1CH D1 via video decoder port A
#define TVP_VIDEO_DECODER_MODE_1CH_HALF_D1_PORT_A		 1	 ///< Video decoder mode: 1CH Half-D1 via video decoder port A
#define TVP_VIDEO_DECODER_MODE_2CH_D1_PORT_A			 2	 ///< Video decoder mode: 2CH D1 via video decoder port A
#define TVP_VIDEO_DECODER_MODE_2CH_D1_PORT_B			 3	 ///< Video decoder mode: 2CH D1 via video decoder port B
#define TVP_VIDEO_DECODER_MODE_4CH_D1					 4	 ///< Video decoder mode: 4CH D1
#define TVP_VIDEO_DECODER_MODE_4CH_HALF_D1				 5	 ///< Video decoder mode: 4CH Half-D1
#define TVP_VIDEO_DECODER_MODE_4CH_CIF					 6	 ///< Video decoder mode: 4CH CIF
#define TVP_VIDEO_DECODER_MODE_4CH_D1_16				 7	 ///< Video decoder mode: 4CH D1 16-bit
#define TVP_VIDEO_DECODER_MODE_4CH_HALF_D1_16			 8	 ///< Video decoder mode: 4CH Half-D1 16-bit
#define TVP_VIDEO_DECODER_MODE_8CH_HALF_D1				 9	 ///< Video decoder mode: 8CH Half-D1
#define TVP_VIDEO_DECODER_MODE_8CH_CIF					10	 ///< Video decoder mode: 8CH CIF
#define TVP_VIDEO_DECODER_MODE_4CH_HALF_D1_PLUS_D1		11	 ///< Video decoder mode: 4CH Half-D1 + D1
#define TVP_VIDEO_DECODER_MODE_4CH_CIF_PLUS_D1			12	 ///< Video decoder mode: 4CH CIF	  + D1
#define TVP_VIDEO_DECODER_MODE_8CH_CIF_PLUS_D1			13	 ///< Video decoder mode: 8CH CIF	  + D1

#define TVP_VIDEO_DECODER_MODE_1CH_HALF_D1_PORT_A_CROP	20	 ///< Video decoder mode: 1CH Half-D1 via video decoder port A
#define TVP_VIDEO_DECODER_MODE_2CH_D1_PORT_A_CROP		21	 ///< Video decoder mode: 2CH D1 via video decoder port A
#define TVP_VIDEO_DECODER_MODE_2CH_D1_PORT_B_CROP		22	 ///< Video decoder mode: 2CH D1 via video decoder port B
#define TVP_VIDEO_DECODER_MODE_4CH_D1_CROP				23	 ///< Video decoder mode: 4CH D1
#define TVP_VIDEO_DECODER_MODE_4CH_HALF_D1_CROP 		24	 ///< Video decoder mode: 4CH Half-D1
#define TVP_VIDEO_DECODER_MODE_4CH_CIF_CROP 			25	 ///< Video decoder mode: 4CH CIF
#define TVP_VIDEO_DECODER_MODE_4CH_D1_16_CROP			26	 ///< Video decoder mode: 4CH D1 16-bit
#define TVP_VIDEO_DECODER_MODE_4CH_HALF_D1_16_CROP		27	 ///< Video decoder mode: 4CH Half-D1 16-bit
#define TVP_VIDEO_DECODER_MODE_8CH_HALF_D1_CROP 		28	 ///< Video decoder mode: 8CH Half-D1
#define TVP_VIDEO_DECODER_MODE_8CH_CIF_CROP 			29	 ///< Video decoder mode: 8CH CIF
#define TVP_VIDEO_DECODER_MODE_4CH_HALF_D1_PLUS_D1_CROP 30	 ///< Video decoder mode: 4CH Half-D1 + D1
#define TVP_VIDEO_DECODER_MODE_4CH_CIF_PLUS_D1_CROP 	31	 ///< Video decoder mode: 4CH CIF	  + D1
#define TVP_VIDEO_DECODER_MODE_8CH_CIF_PLUS_D1_CROP 	32	 ///< Video decoder mode: 8CH CIF	  + D1
#define TVP_VIDEO_DECODER_MODE_4CH_HALF_D1_PROG 		33	 ///< Video decoder mode: 4CH Half-D1 Progressive
#define TVP_VIDEO_DECODER_MODE_4CH_HALF_D1_16_PROG		34	 ///< Video decoder mode: 4CH Half-D1 16-bit Progressive
#define TVP_VIDEO_DECODER_MODE_8CH_HALF_D1_PROG 		35	 ///< Video decoder mode: 8CH Half-D1 Progressive
#define TVP_VIDEO_DECODER_MODE_4CH_HALF_D1_PROG_CROP	36	 ///< Video decoder mode: 4CH Half-D1 Progressive
#define TVP_VIDEO_DECODER_MODE_4CH_HALF_D1_16_PROG_CROP 37	 ///< Video decoder mode: 4CH Half-D1 16-bit Progressive
#define TVP_VIDEO_DECODER_MODE_8CH_HALF_D1_PROG_CROP	38	 ///< Video decoder mode: 8CH Half-D1 Progressive
/*******************************************************************************/


typedef struct _TVP_INPUT_STANDARD_ {
	uint8_t dec1_std;
	uint8_t dec2_std;
	uint8_t dec3_std;
	uint8_t dec4_std;
} tvp_input_standard;

typedef enum {
	TVP_TYPE_656_I_480,
	TVP_TYPE_656_P_480,
	TVP_TYPE_656_I_576,
	TVP_TYPE_656_P_576,
	TVP_TYPE_656_I_352_480,
	TVP_TYPE_656_P_352_2100,
	TVP_TYPE_MAX,
} E_TVP_TYPE_T;

enum frame_status {
	FRAME_EMPTY,
	FRAME_READING,
	FRAME_READY,
	FRAME_ERROR
};

struct framebuf {
	int idx;
	int length;
	volatile enum frame_status status;
	u8 *data;
	unsigned long userptr;
};

struct tvp_data {
	int streaming;
	struct v4l2_pix_format pix;
	int memset_cnt;
	int num_frames;
	int cur_frames;
	struct framebuf buffers[VIDEO_MAX_FRAME];
};


extern struct mutex g_WchLock;
extern struct tvp_data g_tvp_data[TVP_MAX_VIDEO_DEVS];

int tvp_set_attribute(uint8_t chid, uint32_t control, int32_t value);
int tvp_get_attribute(uint8_t chid, uint32_t control, int32_t *pvalue);
int tvp_set_std(uint8_t chid, uint8_t std_type);
int tvp_get_std(uint8_t chid, v4l2_std_id *pstd_type);
int tvp_set_reg_value(uint8_t chid, uint8_t reg, uint8_t value);
int tvp_get_reg_value(uint8_t chid, uint8_t reg, uint8_t *pvalue);


int tvp_init(unsigned short addr);
int tvp_deinit(void);
int tvp_i2c_start(int ofmMode);
int tvp_i2c_stop(void);
int tvp_wch_start(E_TVP_TYPE_T eTVPFmt);
int tvp_wch_stop(void);

#endif

