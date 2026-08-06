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


#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define IOCTL_MTK_ADEC_CTRL			0xFF000001UL
#define IOCTL_MTK_ADEC_SET_FORMAT	0xFF000002UL
#define IOCTL_MTK_ADEC_CONNECT_ESM	0xFF000003UL
#define IOCTL_MTK_ADEC_GET_CACHE	0xFF000004UL

typedef struct {
	uint32_t u4Buf;
	uint32_t u4Size;
} ESM_BUF_T;

s32 main(s32 argc, s8 *argv[])
{
	if (argc < 2) {
		printf("argument err\n");
		return -1;
	}

	FILE *fp = fopen(argv[1], "r");
	if (!fp) {
		printf("fopen failed\n");
		return -2;
	}

	s32 fd = open("/dev/adec", O_RDWR);
	if (fd < 0) {
		printf("open device failed\n");
		return -3;
	}

	ioctl(fd, IOCTL_MTK_ADEC_SET_FORMAT, 4);
	ioctl(fd, IOCTL_MTK_ADEC_CONNECT_ESM, 0);
	ioctl(fd, IOCTL_MTK_ADEC_CTRL, 2);

	uint8_t buf[4096];
	uint32_t total = 0;
	while (1) {
		u32 nread = fread(buf, 1, 4096, fp);
		if (nread < 4096 && feof(fp))
			break;

		if (total >= 500 * 1024)
			break;
	//	if (total >= 0x100000)
//			break;

		if (nread > 0) {
			s32 nwrite;
			while (1) {
				uint32_t cache_size;
#if 0
				ioctl(fd, IOCTL_MTK_ADEC_GET_CACHE, &cache_size);
				if (cache_size >= 50 * 1024) {
					sleep(1);
					continue;
				}
#endif
				nwrite = write(fd, (void *)buf, nread);
				if (nwrite < 0) {
					printf("write failed, ret=%d", nwrite);
					continue;
				} else
					break;
			}
			if (nwrite < 0) {
				printf("nwrite=%d\n", nwrite);
				break;
			}

			total += nread;
		}
	}

	printf("write %d bytes\n", total);

//	ioctl(fd, 4, 0); // esm disconnect
//	ioctl(fd, 0, 1); // Stop

	close(fd);
	fclose(fp);
	
	printf("the end\n");

	return 0;
}
