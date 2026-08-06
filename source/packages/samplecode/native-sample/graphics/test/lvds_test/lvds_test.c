#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <AtcDisplaySettings.h>

int main(int argc, char **argv)
{
	int ret = 0, index = 0, freq = 0, range = 0;

	if (argv[2] > 0) {
		index = atoi(argv[2]);
	}

	if (argv[3] > 0) {
		freq = atoi(argv[3]);
	}

	if (argv[4] > 0) {
		range = atoi(argv[4]);
	}


	if (0 == strcmp(argv[1], "set_ssc")) {
		ret = SetLvdsSsc(index, freq, range);
		if (ret < 0) {
			printf("SetLvdsSsc failed, ret: %d\n", ret);
			return -1;
		}
		printf("SetLvdsSsc done\n");
	} else {
		printf("Parameter error, %s\n", argv[1]);
		return -1;
	}

	return 0;
}
