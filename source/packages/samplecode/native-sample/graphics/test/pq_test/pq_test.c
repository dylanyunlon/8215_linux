#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <AtcDisplaySettings.h>

int main(int argc, char **argv)
{
	int ret = 0, index = 0;

	if (argv[2] > 0) {
		index = atoi(argv[2]);
	}

	if (0 == strcmp(argv[1], "set_brightness")) {
		ret = SetBrightnessLevel(index);
		if (ret) {
			printf("SetBrightnessLevel failed, ret: %d\n", ret);
			return -1;
		}
		printf("SetBrightnessLevel %d successed\n", index);
	} else if (0 == strcmp(argv[1], "set_contrast")) {
		ret = SetContrastLevel(index);
		if (ret) {
			printf("SetContrastLevel failed, ret: %d\n", ret);
			return -1;
		}
		printf("SetContrastLevel %d successed\n", index);
	} else if (0 == strcmp(argv[1], "set_backlight")) {
		index = 100 - index;
		ret = SetBackLightLevel(index);
		if (ret) {
			printf("SetBackLightLevel failed, ret: %d\n", ret);
			return -1;
		}
		printf("SetBackLightLevel %d successed\n", index);
	} else if (0 == strcmp(argv[1], "bkl_shutdown")) {
		ret = SetBklShutDown(1);
		if (ret) {
			printf("SetBklShutDown failed\n");
			return -1;
		}
		printf("SetBklShutDown successed\n");
	} else if (0 == strcmp(argv[1], "set_hue")) {
		ret = SetHueLevel(index);
		if (ret) {
			printf("SetHueLevel failed, ret: %d\n", ret);
			return -1;
		}
		printf("SetHueLevel %d successed\n", index);
	} else if (0 == strcmp(argv[1], "set_saturation")) {
		ret = SetSaturationLevel(index);
		if (ret) {
			printf("SetSaturationLevel failed, ret: %d\n", ret);
			return -1;
		}
		printf("SetSaturationLevel %d successed\n", index);
	} else if (0 == strcmp(argv[1], "set_y_gain")) {
		ret = SetYGainLevel(index);
		if (ret) {
			printf("SetYGainLevel failed, ret: %d\n", ret);
			return -1;
		}
		printf("SetYGainLevel %d successed\n", index);
	} else if (0 == strcmp(argv[1], "set_u_gain")) {
		ret = SetUGainLevel(index);
		if (ret) {
			printf("SetUGainLevel failed, ret: %d\n", ret);
			return -1;
		}
		printf("SetUGainLevel %d successed\n", index);
	} else if (0 == strcmp(argv[1], "set_v_gain")) {
		ret = SetVGainLevel(index);
		if (ret) {
			printf("SetVGainLevel failed, ret: %d\n", ret);
			return -1;
		}
		printf("SetVGainLevel %d successed\n", index);
	} else if (0 == strcmp(argv[1], "set_dither")) {
		ret = SetDitherLevel(index);
		if (ret) {
			printf("SetDitherLevel failed, ret: %d\n", ret);
			return -1;
		}
		printf("SetDitherLevel %d successed\n", index);
	} else if (0 == strcmp(argv[1], "set_dither_disable")) {
		ret = SetDitherDisable();
		if (ret) {
			printf("SetDitherDisable failed, ret: %d\n", ret);
			return -1;
		}
		printf("SetDitherDisable successed\n");
	}  else if (0 == strcmp(argv[1], "get_brightness")) {
		ret = GetBrightnessLevel();
		if (ret < 0) {
			printf("GetBrightnessLevel failed, ret: %d\n", ret);
			return -1;
		}
		printf("GetBrightnessLevel is %d\n", ret);
	} else if (0 == strcmp(argv[1], "get_contrast")) {
		ret = GetContrastLevel();
		if (ret < 0) {
			printf("GetContrastLevel failed, ret: %d\n", ret);
			return -1;
		}
		printf("GetContrastLevel is %d\n", ret);
	} else if (0 == strcmp(argv[1], "get_backlight")) {
		ret = GetBackLightLevel();
		if (ret < 0) {
			printf("GetBackLightLevel failed, ret: %d\n", ret);
			return -1;
		}
		printf("GetBackLightLevel %d\n", ret);
	} else if (0 == strcmp(argv[1], "get_hue")) {
		ret = GetHueLevel();
		if (ret < 0) {
			printf("GetHueLevel failed, ret: %d\n", ret);
			return -1;
		}
		printf("GetHueLevel %d\n", ret);
	} else if (0 == strcmp(argv[1], "get_saturation")) {
		ret = GetSaturationLevel();
		if (ret < 0) {
			printf("GetSaturationLevel failed, ret: %d\n", ret);
			return -1;
		}
		printf("GetSaturationLevel %d\n", ret);
	} else if (0 == strcmp(argv[1], "get_y_gain")) {
		ret = GetYGainLevel(index);
		if (ret < 0) {
			printf("GetYGainLevel failed, ret: %d\n", ret);
			return -1;
		}
		printf("GetYGainLevel %d\n", ret);
	} else if (0 == strcmp(argv[1], "get_u_gain")) {
		ret = GetUGainLevel(index);
		if (ret < 0) {
			printf("GetUGainLevel failed, ret: %d\n", ret);
			return -1;
		}
		printf("GetUGainLevel %d\n", ret);
	} else if (0 == strcmp(argv[1], "get_v_gain")) {
		ret = GetVGainLevel(index);
		if (ret < 0) {
			printf("GetVGainLevel failed, ret: %d\n", ret);
			return -1;
		}
		printf("GetVGainLevel %d\n", ret);
	} else if (0 == strcmp(argv[1], "get_dither")) {
		ret = GetDitherLevel();
		if (ret < 0) {
			printf("GetDitherLevel failed, ret: %d\n", ret);
			return -1;
		}
		printf("GetDitherLevel %d\n", ret);
	} else {
		printf("Parameter error, %s\n", argv[1]);
		return -1;
	}

	return 0;
}
