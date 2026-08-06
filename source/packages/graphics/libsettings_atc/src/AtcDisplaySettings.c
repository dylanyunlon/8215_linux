/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of AutoChips Inc. (C) 2013 AutoChips Inc.
*
*  BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
*  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
*  RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON
*  AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
*  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
*  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
*  NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
*  SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
*  SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK ONLY TO SUCH
*  THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
*  NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S
*  SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*
*  BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE
*  LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
*  AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
*  OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY BUYER TO
*  MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
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
* file AtcDisplaySettings.c
*
* Project:
* --------
*   CNB
*
* Description:
* ------------
*
*
* Author:
* -------
*
*
*------------------------------------------------------------------------------
* $Revision: #14 $
* $Modtime:$ 2015-08-24
* $Log:$
*
*******************************************************************************/
#include <fcntl.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include "../include/AtcDisplaySettings.h"

#define DISP_DEV_NAME	"/dev/fb0"
#define VCP_DEV_NAME "/dev/vcp"
#define BKL_DEV_NAME "/dev/bkl"
#define BKL_NODE "/sys/class/backlight/soc:bkl/brightness"

struct LVDS_SSC {
	unsigned int dir;
	unsigned int freq;
	unsigned int range;
};

int SetLvdsSsc(unsigned int dir, unsigned int freq, unsigned int range)
{
	int fd = 0, ret = 0;
	struct LVDS_SSC cfg;

	cfg.dir = dir;
	cfg.freq = freq;
	cfg.range = range;

	fd = open(DISP_DEV_NAME, O_RDWR, 0);
	if (fd < 0 ) {
		printf("GetDitherLevel: open %s failed \r\n", DISP_DEV_NAME);
		return -1;
	}

	ret = ioctl(fd, DISPLAY_SET_LVDS_SSC, &cfg);
	if (ret < 0) {
		printf("SetLvdsSsc: ioctl failed! \r\n");
		ret = close(fd);
		if (ret < 0) {
			printf("SetLvdsSsc: close %s failed \r\n", DISP_DEV_NAME);
		}
		return -1;
	}

	ret = close(fd);
	if (ret < 0) {
		printf("SetLvdsSsc: close %s failed \r\n", DISP_DEV_NAME);
	}
	return 0;
}


int GetDitherLevel(void)
{
	int fd = 0, ret = 0, value = 0, value_temp = 0;

	fd = open(DISP_DEV_NAME, O_RDWR, 0);
	if (fd < 0 ) {
		printf("GetDitherLevel: open %s failed \r\n", DISP_DEV_NAME);
		return -1;
	}

	ret = ioctl(fd, DISPLAY_GET_DITHER, &value);
	if (ret < 0) {
		printf("GetDitherLevel: ioctl failed! \r\n");
		ret = close(fd);
		if (ret < 0) {
			printf("GetDitherLevel: close %s failed \r\n", DISP_DEV_NAME);
		}
		return -1;
	}

	ret = close(fd);
	if (ret < 0) {
		printf("GetDitherLevel: close %s failed \r\n", DISP_DEV_NAME);
	}

	switch (value) {
	case 0:
		value_temp = 4;
		break;
	case 1:
		value_temp = 6;
		break;
	case 2:
		value_temp = 8;
		break;
	case 3:
		value_temp = 10;
		break;
	default:
		printf("GetDitherLevel: get dither value err = %d \r\n", value);
		return -1;
	}

	printf("GetDitherLevel: get dither ouput %dbit\r\n", value_temp);

	return value_temp;
}

int SetDitherDisable(void)
{
	int fd = 0, ret = 0;

	fd = open(DISP_DEV_NAME, O_RDWR, 0);
	if (fd < 0 ) {
		printf("SetDitherDisable: open %s failed \r\n", DISP_DEV_NAME);
		return -1;
	}

	ret = ioctl(fd, DISPLAY_SET_DITHER_DISABLE, 0);
	if (ret < 0) {
		printf("SetDitherDisable: ioctl failed! \r\n");
		ret = close(fd);
		if (ret < 0) {
			printf("SetDitherDisable: close %s failed \r\n", DISP_DEV_NAME);
		}
		return -1;
	}

	ret = close(fd);
	if (ret < 0) {
		printf("SetDitherDisable: close %s failed \r\n", DISP_DEV_NAME);
	}

	return 0;
}

int SetDitherLevel(int level)
{
	int fd = 0, ret = 0, level_temp = 0;

	switch (level) {
	case 4:
		level_temp = 0;
		break;
	case 6:
		level_temp = 1;
		break;
	case 8:
		level_temp = 2;
		break;
	case 10:
		level_temp = 3;
		break;
	default:
		printf("SetDitherLevel: set level value err = %d \r\n", level);
		return -1;
	}

	fd = open(DISP_DEV_NAME, O_RDWR, 0);
	if (fd < 0 ) {
		printf("SetDitherLevel: open %s failed \r\n", DISP_DEV_NAME);
		return -1;
	}

	ret = ioctl(fd, DISPLAY_SET_DITHER, level_temp);
	if (ret < 0) {
		printf("SetDitherLevel: ioctl failed! \r\n");
		ret = close(fd);
		if (ret < 0) {
			printf("SetDitherLevel: close %s failed \r\n", DISP_DEV_NAME);
		}
		return -1;
	}

	ret = close(fd);
	if (ret < 0) {
		printf("SetDitherLevel: close %s failed \r\n", DISP_DEV_NAME);
	}

	return 0;
}

/**
**  description-->set brightness level.
**  input-->'level' brightness level to be set.
**  return-->0 for success, -1 for failed.
**  note-->'level' set to between 0 and 100.
**/
int SetBrightnessLevel(int level)
{
	int fd = 0, ret = 0 , level_tmp = 0;

	if (level > 100 || level < 0) {
			printf("SetBrightnessLevel: set level value err = %d \r\n", level);
			return -1;
	} else{
			level_tmp = level * (80 - 15) / 100 + 15;
			printf("SetBrightnessLevel: set level value = %d \r\n", level);
	}

	fd = open(DISP_DEV_NAME, O_RDWR, 0);
	if (fd < 0 ) {
		printf("SetBrightnessLevel: open %s failed \r\n", DISP_DEV_NAME);
		return -1;
	}

	ret = ioctl(fd, DISPLAY_SET_BRIGNTNESS, level_tmp);
	if (ret < 0) {
		printf("SetBrightnessLevel: ioctl failed! \r\n");
		ret = close(fd);
		if (ret < 0) {
			printf("SetBrightnessLevel: close %s failed \r\n", DISP_DEV_NAME);
		}
		return -1;
	}

	ret = close(fd);
	if (ret < 0) {
		printf("SetBrightnessLevel: close %s failed \r\n", DISP_DEV_NAME);
	}

	return 0;
}

/**
**  description-->set contrast level.
**  input-->'level' contrast level to be set.
**  return-->0 for success, -1 for failed.
**  note-->'level' set to between 0 and 100.
**/
int SetContrastLevel(int level)
{
	int fd = 0, ret = 0, level_tmp = 0;

	if (level > 100 || level < 0) {
			printf("SetContrastLevel: set level value err = %d \r\n", level);
			return -1;
	} else{
			level_tmp = level * (80 - 15) / 100 + 15;
			printf("SetContrastLevel: set level value = %d \r\n", level);
	}

	fd = open(DISP_DEV_NAME, O_RDWR, 0);
	if (fd < 0 ) {
		printf("SetContrastLevel: open %s failed \r\n", DISP_DEV_NAME);
		return -1;
	}

	ret = ioctl(fd, DISPLAY_SET_CONTRAST, level_tmp);
	if (ret < 0) {
		printf("SetContrastLevel: ioctl failed! \r\n");
		ret = close(fd);
		if (ret < 0) {
			printf("SetContrastLevel: close %s failed \r\n", DISP_DEV_NAME);
		}
		return -1;
	}

	ret = close(fd);
	if (ret < 0) {
		printf("SetContrastLevel: close %s failed \r\n", DISP_DEV_NAME);
	}

	return 0;
}

/**
**  description-->set backlight level.
**  input-->'level' backlight level to be set.
**  return-->0 for success, -1 for failed.
**  note-->'level' set to between 0 and 100.
**/
int SetBackLightLevel(int level)
{
	int fd = 0, ret = 0, level_tmp = 0;

	if (level > 100 || level < 0) {
			printf("SetBackLightLevel: set level value err = %d \r\n", level);
			return -1;
	} else{
			level_tmp = level * (100 - 20) / 100 + 20;
			printf("SetBackLightLevel: set level value = %d \r\n", level);
	}

	fd = open(BKL_DEV_NAME, O_RDWR, 0);
	if (fd < 0 ) {
		printf("SetBackLightLevel: open %s failed \r\n", BKL_DEV_NAME);
		return -1;
	}

	ret = ioctl(fd, DISPLAY_SET_BKL_INTENSITY, level_tmp);
	if (ret < 0) {
		printf("SetBackLightLevel: ioctl failed! \r\n");
		ret = close(fd);
		if (ret < 0) {
			printf("SetBackLightLevel: close %s failed \r\n", BKL_DEV_NAME);
		}
		return -1;
	}

	ret = close(fd);
	if (ret < 0) {
		printf("SetBackLightLevel: close %s failed \r\n", BKL_DEV_NAME);
	}

	return 0;
}

/**
**  description-->set bkl module to be shut down.
**  input-->'fgShutdown' 1:shut down, other value:nothing to be done.
**  return-->0 for success, -1 for failed.
**/
int SetBklShutDown(int fgShutdown)
{
	int fd = 0, ret = 0;

	if (fgShutdown != 1) {
		printf("SetBklShutDown:fgShutdown's value should be '1'\n");
		return -1;
	}

	fd = open(BKL_DEV_NAME, O_RDWR, 0);
	if (fd < 0 ) {
		printf("SetBklShutDown: open %s failed \r\n", BKL_DEV_NAME);
		return -1;
	}

	ret = ioctl(fd, DISPLAY_SET_BKL_SHUTDOWN, fgShutdown);
	if (ret < 0) {
		printf("SetBklShutDown: ioctl failed! \r\n");
		ret = close(fd);
		if (ret < 0) {
			printf("SetBklShutDown: close %s failed \r\n", BKL_DEV_NAME);
		}
		return -1;
	}

	ret = close(fd);
	if (ret < 0) {
		printf("SetBklShutDown: close %s failed \r\n", BKL_DEV_NAME);
	}

	return 0;
}

/**
**  description-->set backlight level.
**  input-->'level' backlight level to be set.
**  return-->0 for success, -1 for failed.
**  note-->'level' set to between 0 and 100.
**/
int SetBackLightLevelBS(int level)
{
	int fd = 0, ret = 0;
	char level_tmp[10];

	if (level > 100 || level < 0) {
			printf("SetBackLightLevel: set level value err = %d \r\n", level);
			return -1;
	} else{
			level = level * (100 - 20) / 100 + 20;
			sprintf(level_tmp, "%d", level);
			printf("[YZQ]SetBackLightLevel: set level value = %d \r\n", level);
	}

	fd = open(BKL_NODE, O_RDWR, 0);
	if (fd < 0 ) {
		printf("SetBackLightLevel: open %s failed \r\n", BKL_DEV_NAME);
		return -1;
	}

	ret = write(fd, level_tmp, 10);
	if (ret < 0) {
		printf("SetBackLightLevel: ioctl failed! \r\n");
		ret = close(fd);
		if (ret < 0) {
			printf("SetBackLightLevel: close %s failed \r\n", BKL_DEV_NAME);
		}
		return -1;
	}

	ret = close(fd);
	if (ret < 0) {
		printf("SetBackLightLevel: close %s failed \r\n", BKL_DEV_NAME);
	}

	return 0;
}
/**
**  description-->set HUE level.
**  input-->'level' HUE level to be set.
**  return-->0 for success, -1 for failed.
**  note-->'level' set to between 0 and 100.
**/
int SetHueLevel(int level)
{
	int fd = 0, ret = 0;

	if (level > 100 || level < 0) {
			printf("SetHueLevel: set level value err = %d \r\n", level);
			return -1;	
	} else{
			printf("SetHueLevel: set level value = %d \r\n", level);
	}

	fd = open(DISP_DEV_NAME, O_RDWR, 0);
	if (fd < 0 ) {
		printf("SetHueLevel: open %s failed \r\n", DISP_DEV_NAME);
		return -1;
	}

	ret = ioctl(fd, DISPLAY_SET_HUE, level);
	if (ret < 0) {
		printf("SetHueLevel: ioctl failed! \r\n");
		ret = close(fd);
		if (ret < 0) {
			printf("SetHueLevel: close %s failed \r\n", DISP_DEV_NAME);
		}
		return -1;
	}

	ret = close(fd);
	if (ret < 0) {
		printf("SetHueLevel: close %s failed \r\n", DISP_DEV_NAME);
	}

	return 0;
}

/**
**  description-->set saturation level.
**  input-->'level' saturation level to be set.
**  return-->0 for success, -1 for failed.
**  note-->'level' set to between 0 and 100.
**/
int SetSaturationLevel(int level)
{
	int fd = 0, ret = 0;

	if (level > 100 || level < 0) {
			printf("SetSaturationLevel: set level value err = %d \r\n", level);
			return -1;	
	} else{
			printf("SetSaturationLevel: set level value = %d \r\n", level);
	}

	fd = open(DISP_DEV_NAME, O_RDWR, 0);
	if (fd < 0 ) {
		printf("SetSaturationLevel: open %s failed \r\n", DISP_DEV_NAME);
		return -1;
	}

	ret = ioctl(fd, DISPLAY_SET_SATURATION, level);
	if (ret < 0) {
		printf("SetSaturationLevel: ioctl failed! \r\n");
		ret = close(fd);
		if (ret < 0) {
			printf("SetSaturationLevel: close %s failed \r\n", DISP_DEV_NAME);
		}
		return -1;
	}

	ret = close(fd);
	if (ret < 0) {
		printf("SetSaturationLevel: close %s failed \r\n", DISP_DEV_NAME);
	}

	return 0;
}

/**
**  description-->set ygain level.
**  input-->'level' ygain level to be set.
**  return-->0 for success, -1 for failed.
**  note-->'level' set to between 0 and 0x1ff.
**/
int SetYGainLevel(int level)
{
	int fd = 0, ret = 0;

	if (level > 0x1ff || level < 0) {
			printf("SetYGainLevel: set level value err = %d \r\n", level);
			return -1;	
	} else{
			printf("SetYGainLevel: set level value = %d \r\n", level);
	}

	fd = open(DISP_DEV_NAME, O_RDWR, 0);
	if (fd < 0 ) {
		printf("SetYGainLevel: open %s failed \r\n", DISP_DEV_NAME);
		return -1;
	}

	ret = ioctl(fd, DISPLAY_SET_YGAIN, level);
	if (ret < 0) {
		printf("SetYGainLevel: ioctl failed! \r\n");
		ret = close(fd);
		if (ret < 0) {
			printf("SetYGainLevel: close %s failed \r\n", DISP_DEV_NAME);
		}
		return -1;
	}

	ret = close(fd);
	if (ret < 0) {
		printf("SetYGainLevel: close %s failed \r\n", DISP_DEV_NAME);
	}

	return 0;
}

/**
**  description-->set ugain level.
**  input-->'level' ugain level to be set.
**  return-->0 for success, -1 for failed.
**  note-->'level' set to between 0 and 0x1ff.
**/
int SetUGainLevel(int level)
{
	int fd = 0, ret = 0;

	if (level > 0x1ff || level < 0) {
			printf("SetUGainLevel: set level value err = %d \r\n", level);
			return -1;	
	} else{
			printf("SetUGainLevel: set level value = %d \r\n", level);
	}

	fd = open(DISP_DEV_NAME, O_RDWR, 0);
	if (fd < 0 ) {
		printf("SetUGainLevel: open %s failed \r\n", DISP_DEV_NAME);
		return -1;
	}

	ret = ioctl(fd, DISPLAY_SET_UGAIN, level);
	if (ret < 0) {
		printf("SetUGainLevel: ioctl failed! \r\n");
		ret = close(fd);
		if (ret < 0) {
			printf("SetUGainLevel: close %s failed \r\n", DISP_DEV_NAME);
		}
		return -1;
	}

	ret = close(fd);
	if (ret < 0) {
		printf("SetUGainLevel: close %s failed \r\n", DISP_DEV_NAME);
	}

	return 0;
}

/**
**  description-->set vgain level.
**  input-->'level' vgain level to be set.
**  return-->0 for success, -1 for failed.
**  note-->'level' set to between 0 and 0x1ff.
**/
int SetVGainLevel(int level)
{
	int fd = 0, ret = 0;

	if (level > 0x1ff || level < 0) {
			printf("SetVGainLevel: set level value err = %d \r\n", level);
			return -1;	
	} else{
			printf("SetVGainLevel: set level value = %d \r\n", level);
	}

	fd = open(DISP_DEV_NAME, O_RDWR, 0);
	if (fd < 0 ) {
		printf("SetVGainLevel: open %s failed \r\n", DISP_DEV_NAME);
		return -1;
	}

	ret = ioctl(fd, DISPLAY_SET_VGAIN, level);
	if (ret < 0) {
		printf("SetVGainLevel: ioctl failed! \r\n");
		ret = close(fd);
		if (ret < 0) {
			printf("SetVGainLevel: close %s failed \r\n", DISP_DEV_NAME);
		}
		return -1;
	}

	ret = close(fd);
	if (ret < 0) {
		printf("SetVGainLevel: close %s failed \r\n", DISP_DEV_NAME);
	}

	return 0;
}

/**
**  description-->set gamma table.
**  input-->'GamTbl' gamma table to be set.
**  return-->0 for success, -1 for failed.
**  note-->'GamTbl' array address,the size of this array is 64.
**/
int SetGammaTblLevel(unsigned char* GamTbl)
{
	int fd = 0, ret = 0;

	if (GamTbl == NULL) {
		printf("SetGammaTblLevel:gama table is null! \r\n");
		return -1;
	} else{
		printf("SetGammaTblLevel: sgama table is  %p \r\n", GamTbl);
	}

	fd = open(DISP_DEV_NAME, O_RDWR, 0);
	if (fd < 0 ) {
		printf("SetGammaTblLevel: open %s failed \r\n", DISP_DEV_NAME);
		return -1;
	}

	ret = ioctl(fd, DISPLAY_SET_GAMMA, GamTbl);
	if (ret < 0) {
		printf("SetGammaTblLevel: ioctl failed! \r\n");
		ret = close(fd);
		if (ret < 0) {
			printf("SetGammaTblLevel: close %s failed \r\n", DISP_DEV_NAME);
		}
		return -1;
	}

	ret = close(fd);
	if (ret < 0) {
		printf("SetGammaTblLevel: close %s failed \r\n", DISP_DEV_NAME);
	}

	return 0;
}

/**
**  description-->get brightness level.
**  input-->none.
**  return-->brightness level.
**  note-->none.
**/
int GetBrightnessLevel()
{
	int fd = 0, ret = 0, value = 0;

	fd = open(DISP_DEV_NAME, O_RDWR, 0);
	if (fd < 0 ) {
		printf("GetBrightnessLevel: open %s failed \r\n", DISP_DEV_NAME);
		return -1;
	}

	ret = ioctl(fd, DISPLAY_GET_BRIGNTNESS, &value);
	value = (((((value - 15)*1000))/65)+5)/10;
	if (ret < 0) {
		printf("GetBrightnessLevel: ioctl failed! \r\n");
		ret = close(fd);
		if (ret < 0) {
			printf("GetBrightnessLevel: close %s failed \r\n", DISP_DEV_NAME);
		}
		return -1;
	}

	ret = close(fd);
	if (ret < 0) {
		printf("GetBrightnessLevel: close %s failed \r\n", DISP_DEV_NAME);
	}

	printf("GetBrightnessLevel: get value = %d \r\n", value);

	return value;
}

/**
**  description-->get contrast level.
**  input-->none.
**  return-->contrast level.
**  note-->none.
**/
int GetContrastLevel()
{
	int fd = 0, ret = 0, value = 0;

	fd = open(DISP_DEV_NAME, O_RDWR, 0);
	if (fd < 0 ) {
		printf("GetContrastLevel: open %s failed \r\n", DISP_DEV_NAME);
		return -1;
	}

	ret = ioctl(fd, DISPLAY_GET_CONTRAST, &value);
	value = (((((value - 15)*1000))/65)+5)/10;
	if (ret < 0) {
		printf("GetContrastLevel: ioctl failed! \r\n");
		ret = close(fd);
		if (ret < 0) {
			printf("GetContrastLevel: close %s failed \r\n", DISP_DEV_NAME);
		}
		return -1;
	}

	ret = close(fd);
	if (ret < 0) {
		printf("GetContrastLevel: close %s failed \r\n", DISP_DEV_NAME);
	}

	printf("GetContrastLevel: get value = %d \r\n", value);

	return value;
}

/**
**  description-->get backlight level.
**  input-->none.
**  return-->backlight level.
**  note-->none.
**/
int GetBackLightLevel()
{
	int fd = 0, ret = 0, value = 0;

	fd = open(BKL_DEV_NAME, O_RDWR, 0);
	if (fd < 0 ) {
		printf("GetBackLightLevel: open %s failed \r\n", BKL_DEV_NAME);
		return -1;
	}

	ret = ioctl(fd, DISPLAY_GET_BKL_INTENSITY, &value);
	if (ret < 0) {
		printf("GetBackLightLevel: ioctl failed! \r\n");
		ret = close(fd);
		if (ret < 0) {
			printf("GetBackLightLevel: close %s failed \r\n", BKL_DEV_NAME);
		}
		return -1;
	}

	ret = close(fd);
	if (ret < 0) {
		printf("GetBackLightLevel: close %s failed \r\n", BKL_DEV_NAME);
	}

	printf("GetBackLightLevel: get value = %d \r\n", value);

	return value;
}

/**
**  description-->get HUE level.
**  input-->none.
**  return-->HUE level.
**  note-->none.
**/
int GetHueLevel()
{
	int fd = 0, ret = 0, value = 0;

	fd = open(DISP_DEV_NAME, O_RDWR, 0);
	if (fd < 0 ) {
		printf("GetHueLevel: open %s failed \r\n", DISP_DEV_NAME);
		return -1;
	}

	ret = ioctl(fd, DISPLAY_GET_HUE, &value);
	if (ret < 0) {
		printf("GetHueLevel: ioctl failed! \r\n");
		ret = close(fd);
		if (ret < 0) {
			printf("GetHueLevel: close %s failed \r\n", DISP_DEV_NAME);
		}
		return -1;
	}

	ret = close(fd);
	if (ret < 0) {
		printf("GetHueLevel: close %s failed \r\n", DISP_DEV_NAME);
	}

	printf("GetHueLevel: get value = %d \r\n", value);

	return value;
}

/**
**  description-->get saturation level.
**  input-->none.
**  return-->saturation level.
**  note-->none.
**/
int GetSaturationLevel()
{
	int fd = 0, ret = 0, value = 0;

	fd = open(DISP_DEV_NAME, O_RDWR, 0);
	if (fd < 0 ) {
		printf("GetSaturationLevel: open %s failed \r\n", DISP_DEV_NAME);
		return -1;
	}

	ret = ioctl(fd, DISPLAY_GET_SATURATION, &value);
	if (ret < 0) {
		printf("GetSaturationLevel: ioctl failed! \r\n");
		ret = close(fd);
		if (ret < 0) {
			printf("GetSaturationLevel: close %s failed \r\n", DISP_DEV_NAME);
		}
		return -1;
	}

	ret = close(fd);
	if (ret < 0) {
		printf("GetSaturationLevel: close %s failed \r\n", DISP_DEV_NAME);
	}

	printf("GetSaturationLevel: get value = %d \r\n", value);

	return value;
}

/**
**  description-->get ygain level.
**  input-->none.
**  return-->ygain level.
**  note-->none.
**/
int GetYGainLevel()
{
	int fd = 0, ret = 0, value = 0;

	fd = open(DISP_DEV_NAME, O_RDWR, 0);
	if (fd < 0 ) {
		printf("GetYGainLevel: open %s failed \r\n", DISP_DEV_NAME);
		return -1;
	}

	ret = ioctl(fd, DISPLAY_GET_YGAIN, &value);
	if (ret < 0) {
		printf("GetYGainLevel: ioctl failed! \r\n");
		ret = close(fd);
		if (ret < 0) {
			printf("GetYGainLevel: close %s failed \r\n", DISP_DEV_NAME);
		}
		return -1;
	}

	ret = close(fd);
	if (ret < 0) {
		printf("GetYGainLevel: close %s failed \r\n", DISP_DEV_NAME);
	}

	printf("GetYGainLevel: get value = %d \r\n", value);

	return value;
}

/**
**  description-->get ugain level.
**  input-->none.
**  return-->ugain level.
**  note-->none.
**/
int GetUGainLevel()
{
	int fd = 0, ret = 0, value = 0;

	fd = open(DISP_DEV_NAME, O_RDWR, 0);
	if (fd < 0 ) {
		printf("GetUGainLevel: open %s failed \r\n", DISP_DEV_NAME);
		return -1;
	}

	ret = ioctl(fd, DISPLAY_GET_UGAIN, &value);
	if (ret < 0) {
		printf("GetUGainLevel: ioctl failed! \r\n");
		ret = close(fd);
		if (ret < 0) {
			printf("GetUGainLevel: close %s failed \r\n", DISP_DEV_NAME);
		}
		return -1;
	}

	ret = close(fd);
	if (ret < 0) {
		printf("GetUGainLevel: close %s failed \r\n", DISP_DEV_NAME);
	}

	printf("GetUGainLevel: get value = %d \r\n", value);

	return value;
}
/**
**  description-->get vgain level.
**  input-->none.
**  return-->vgain level.
**  note-->none.
**/
int GetVGainLevel()
{
	int fd = 0, ret = 0, value = 0;

	fd = open(DISP_DEV_NAME, O_RDWR, 0);
	if (fd < 0 ) {
		printf("GetVGainLevel: open %s failed \r\n", DISP_DEV_NAME);
		return -1;
	}

	ret = ioctl(fd, DISPLAY_GET_VGAIN, &value);
	if (ret < 0) {
		printf("GetVGainLevel: ioctl failed! \r\n");
		ret = close(fd);
		if (ret < 0) {
			printf("GetVGainLevel: close %s failed \r\n", DISP_DEV_NAME);
		}
		return -1;
	}

	ret = close(fd);
	if (ret < 0) {
		printf("GetVGainLevel: close %s failed \r\n", DISP_DEV_NAME);
	}

	printf("GetVGainLevel: get value = %d \r\n", value);

	return value;
}
/**
**  description-->get gamma table.
**  input-->'GamTbl' gamma table to be get.
**  return-->0 for success, -1 for failed.
**  note-->'GamTbl' array address,the size of this array is 64.
**/
int GetGammaTblLevel(unsigned char* GamTbl)
{
	int fd = 0, ret = 0;

	fd = open(DISP_DEV_NAME, O_RDWR, 0);
	if (fd < 0 ) {
		printf("GetVGainLevel: open %s failed \r\n", DISP_DEV_NAME);
		return -1;
	}

	ret = ioctl(fd, DISPLAY_GET_GAMMA, GamTbl);
	if (ret < 0) {
		printf("GetVGainLevel: ioctl failed! \r\n");
		ret = close(fd);
		if (ret < 0) {
			printf("GetVGainLevel: close %s failed \r\n", DISP_DEV_NAME);
		}
		return -1;
	}

	ret = close(fd);
	if (ret < 0) {
		printf("GetVGainLevel: close %s failed \r\n", DISP_DEV_NAME);
	}

	printf("GetVGainLevel: gama address = %p \r\n", GamTbl);

	return 0;
}

/**
**  description-->get rotate value.
**  input-->'value' rotate value to be get.
**  return-->0 for success, -1 for failed.
**  note-->'value': 0:0, 1:90, 2:180, 3:270.
**/
int GetRotateValue(unsigned char *value)
{
	int fd = 0, ret = 0;

	fd = open(DISP_DEV_NAME, O_RDWR, 0);
	if (fd < 0 ) {
		printf("GetRotateValue: open %s failed \r\n", DISP_DEV_NAME);
		return -1;
	}

	ret = ioctl(fd, DISPLAY_GET_ROTATE, value);
	if (ret < 0) {
		printf("GetRotateValue: ioctl failed! \r\n");
		ret = close(fd);
		if (ret < 0) {
			printf("GetRotateValue: close %s failed \r\n", DISP_DEV_NAME);
		}
		return -1;
	}

	ret = close(fd);
	if (ret < 0) {
		printf("GetRotateValue: close %s failed \r\n", DISP_DEV_NAME);
	}

	printf("GetRotateValue: rotate = %d \r\n", *value);

	return 0;
}

/**
**  description-->set cp on.
**  input-->none.
**  return-->0 for success, -1 for failed.
**  note-->none.
**/
/*
int SetVcpOn()
{
	int fd = 0, ret = 0, value = 0;

	fd = open(VCP_DEV_NAME, O_RDWR, 0);
	if (fd < 0 ) {
		printf("SetVcpOn: open %s failed \r\n", VCP_DEV_NAME);
		return -1;
	}

	ret = ioctl(fd, VCP_IOC_ON, &value);
	if (ret < 0) {
		printf("SetVcpOn: ioctl failed! \r\n");
		ret = close(fd);
		if (ret < 0) {
			printf("SetVcpOn: close %s failed \r\n", VCP_DEV_NAME);
		}
		return -1;
	}

	ret = close(fd);
	if (ret < 0) {
		printf("SetVcpOn: close %s failed \r\n", VCP_DEV_NAME);
	}

	printf("SetVcpOn \r\n");

	return 0;
}
*/
/**
**  description-->set cp off.
**  input-->none.
**  return-->0 for success, -1 for failed.
**  note-->none.
**/
/*
int SetVcpOff()
{
	int fd = 0, ret = 0, value = 0;

	fd = open(VCP_DEV_NAME, O_RDWR, 0);
	if (fd < 0 ) {
		printf("SetVcpOff: open %s failed \r\n", VCP_DEV_NAME);
		return -1;
	}

	ret = ioctl(fd, VCP_IOC_OFF, &value);
	if (ret < 0) {
		printf("SetVcpOff: ioctl failed! \r\n");
		ret = close(fd);
		if (ret < 0) {
			printf("SetVcpOff: close %s failed \r\n", VCP_DEV_NAME);
		}
		return -1;
	}

	ret = close(fd);
	if (ret < 0) {
		printf("SetVcpOff: close %s failed \r\n", VCP_DEV_NAME);
	}

	printf("SetVcpOff \r\n");

	return 0;
}
*/
/**
**  description-->set vcp hue level.
**  input-->'hue_t' hue struct to be set.
**  return-->0 for success, -1 for failed.
**  note-->'hue_t.i4hue' set to between 0 and 0x3f.
**/
/*
int SetVcpHUELevel(vcp_hue_paras hue_t)
{
	int fd = 0, ret = 0;

	if (hue_t.i4hue > 0x3f || hue_t.i4hue < 0) {
		printf("SetVcpHUELevel param error, hue = %d \r\n", hue_t.i4hue);
		return -1;
	}
	
	fd = open(VCP_DEV_NAME, O_RDWR, 0);
	if (fd < 0 ) {
		printf("SetVcpHUELevel: open %s failed \r\n", VCP_DEV_NAME);
		return -1;
	}

	ret = ioctl(fd, VCP_IOC_SET_HUE, &hue_t);
	if (ret < 0) {
		printf("SetVcpHUELevel: ioctl failed! \r\n");
		ret = close(fd);
		if (ret < 0) {
			printf("SetVcpHUELevel: close %s failed \r\n", VCP_DEV_NAME);
		}
		return -1;
	}

	ret = close(fd);
	if (ret < 0) {
		printf("SetVcpHUELevel: close %s failed \r\n", VCP_DEV_NAME);
	}

	printf("SetVcpHUELevel hue = %d \r\n", hue_t.i4hue);

	return 0;
}
*/
/**
**  description-->set vcp yuv level.
**  input-->'yuvgain_t' yuv struct to be set.
**  return-->0 for success, -1 for failed.
**  note-->'yuvgain_t.i4YGain(yuvgain_t.i4UGain,yuvgain_t.i4VGain)' set to between 0 and 0x1ff.
**/
/*
int SetVcpYUVGainLevel(vcp_yuv_paras yuvgain_t)
{
	int fd = 0, ret = 0;

	if (yuvgain_t.i4YGain > 0x1ff || yuvgain_t.i4YGain < 0 || 
			yuvgain_t.i4UGain > 0x1ff || yuvgain_t.i4UGain < 0 || 
			yuvgain_t.i4VGain > 0x1ff || yuvgain_t.i4VGain < 0) {
		printf("SetVcpYUVGainLevel param error, ygain = %d ugain = %d vgain = %d\r\n", 
					yuvgain_t.i4YGain,
					yuvgain_t.i4UGain,
					yuvgain_t.i4VGain);
		return -1;
	}
	
	fd = open(VCP_DEV_NAME, O_RDWR, 0);
	if (fd < 0 ) {
		printf("SetVcpYUVGainLevel: open %s failed \r\n", VCP_DEV_NAME);
		return -1;
	}

	ret = ioctl(fd, VCP_IOC_SET_YUV, &yuvgain_t);
	if (ret < 0) {
		printf("SetVcpYUVGainLevel: ioctl failed! \r\n");
		ret = close(fd);
		if (ret < 0) {
			printf("SetVcpYUVGainLevel: close %s failed \r\n", VCP_DEV_NAME);
		}
		return -1;
	}

	ret = close(fd);
	if (ret < 0) {
		printf("SetVcpYUVGainLevel: close %s failed \r\n", VCP_DEV_NAME);
	}

	printf("SetVcpYUVGainLevel ygain = %d ugain = %d vgain = %d\r\n", 
					yuvgain_t.i4YGain,
					yuvgain_t.i4UGain,
					yuvgain_t.i4VGain);

	return 0;
}
*/
/**
**  description-->set vcp contrast/brightness/saturation level.
**  input-->'cbs_t' cbs struct to be set.
**  return-->0 for success, -1 for failed.
**  note-->'cbs_t.i4Contr(cbs_t.i4Brit,cbs_t.i4Satr)' set to between 0 and 0xff.
**/
/*
int SetVcpContrBritSatrLevel(vcp_cbs_paras cbs_t)
{
	int fd = 0, ret = 0;

	if (cbs_t.i4Contr > 0xff || cbs_t.i4Contr < 0 || 
			cbs_t.i4Brit > 0xff || cbs_t.i4Brit < 0 || 
			cbs_t.i4Satr > 0xff || cbs_t.i4Satr < 0) {
		printf("SetVcpContrBritSatrLevel param error, contrast = %d brightness = %d saturation = %d\r\n", 
					cbs_t.i4Contr,
					cbs_t.i4Brit,
					cbs_t.i4Satr);
		return -1;
	}
	
	fd = open(VCP_DEV_NAME, O_RDWR, 0);
	if (fd < 0 ) {
		printf("SetVcpContrBritSatrLevel: open %s failed \r\n", VCP_DEV_NAME);
		return -1;
	}

	ret = ioctl(fd, VCP_IOC_SET_CBS, &cbs_t);
	if (ret < 0) {
		printf("SetVcpContrBritSatrLevel: ioctl failed! \r\n");
		ret = close(fd);
		if (ret < 0) {
			printf("SetVcpContrBritSatrLevel: close %s failed \r\n", VCP_DEV_NAME);
		}
		return -1;
	}

	ret = close(fd);
	if (ret < 0) {
		printf("SetVcpContrBritSatrLevel: close %s failed \r\n", VCP_DEV_NAME);
	}

	printf("SetVcpContrBritSatrLevel param error, contrast = %d brightness = %d saturation = %d\r\n", 
					cbs_t.i4Contr,
					cbs_t.i4Brit,
					cbs_t.i4Satr);

	return 0;
}
*/
/**
**  description-->get vcp hue level.
**  input-->none.
**  return-->return value is hue level.
**  note-->none.
**/
/*
int GetVcpHUELevel()
{
	int fd = 0, ret = 0, value = 0;

	fd = open(VCP_DEV_NAME, O_RDWR, 0);
	if (fd < 0 ) {
		printf("GetVcpHUELevel: open %s failed \r\n", VCP_DEV_NAME);
		return -1;
	}

	ret = ioctl(fd, VCP_IOC_GET_HUE, &value);
	if (ret < 0) {
		printf("GetVcpHUELevel: ioctl failed! \r\n");
		ret = close(fd);
		if (ret < 0) {
			printf("GetVcpHUELevel: close %s failed \r\n", VCP_DEV_NAME);
		}
		return -1;
	}

	ret = close(fd);
	if (ret < 0) {
		printf("GetVcpHUELevel: close %s failed \r\n", VCP_DEV_NAME);
	}

	printf("GetVcpHUELevel hue = %d \r\n", value);

	return value;
}
*/
/**
**  description-->get vcp hue level.
**  input-->'hue_t' will get the register value.
**  return-->0 for success, -1 for failed.
**  note-->none.
**/
/*
int GetVcpYUVGainLevel(vcp_yuv_paras *yuvgain_t)
{
	int fd = 0, ret = 0;
	
	fd = open(VCP_DEV_NAME, O_RDWR, 0);
	if (fd < 0 ) {
		printf("GetVcpYUVGainLevel: open %s failed \r\n", VCP_DEV_NAME);
		return -1;
	}

	ret = ioctl(fd, VCP_IOC_GET_YUV, yuvgain_t);
	if (ret < 0) {
		printf("GetVcpYUVGainLevel: ioctl failed! \r\n");
		ret = close(fd);
		if (ret < 0) {
			printf("GetVcpYUVGainLevel: close %s failed \r\n", VCP_DEV_NAME);
		}
		return -1;
	}

	ret = close(fd);
	if (ret < 0) {
		printf("GetVcpYUVGainLevel: close %s failed \r\n", VCP_DEV_NAME);
	}

	printf("GetVcpYUVGainLevel ygain = %d ugain = %d vgain = %d\r\n", 
					yuvgain_t->i4YGain,
					yuvgain_t->i4UGain,
					yuvgain_t->i4VGain);

	return 0;
}
*/
/**
**  description-->get vcp hue level.
**  input-->'cbs_t' will get the register value.
**  return-->0 for success, -1 for failed.
**  note-->.none
**/
/*
int GetVcpContrBritSatrLevel(vcp_cbs_paras *cbs_t)
{
	int fd = 0, ret = 0;
	
	fd = open(VCP_DEV_NAME, O_RDWR, 0);
	if (fd < 0 ) {
		printf("GetVcpContrBritSatrLevel: open %s failed \r\n", VCP_DEV_NAME);
		return -1;
	}

	ret = ioctl(fd, VCP_IOC_GET_CBS, cbs_t);
	if (ret < 0) {
		printf("GetVcpContrBritSatrLevel: ioctl failed! \r\n");
		ret = close(fd);
		if (ret < 0) {
			printf("GetVcpContrBritSatrLevel: close %s failed \r\n", VCP_DEV_NAME);
		}
		return -1;
	}

	ret = close(fd);
	if (ret < 0) {
		printf("GetVcpContrBritSatrLevel: close %s failed \r\n", VCP_DEV_NAME);
	}

	printf("GetVcpContrBritSatrLevel ygain = %d ugain = %d vgain = %d\r\n", 
					cbs_t->i4Contr,
					cbs_t->i4Brit,
					cbs_t->i4Satr);

	return 0;
}
*/
