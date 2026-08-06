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
#define IOCTL_MTK_ADEC_GET_DECREADY	0xFF000006UL

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

	s32 stc = open("/dev/stc", O_RDWR);
	if (stc < 0) {
		printf("open stc failed\n");
		return -4;
	}

	ioctl(fd, IOCTL_MTK_ADEC_SET_FORMAT, 4);
	ioctl(fd, IOCTL_MTK_ADEC_CONNECT_ESM, 0);
	ioctl(fd, IOCTL_MTK_ADEC_CTRL, 2);

#define BUF_SIZE	(0x80000UL)

	uint8_t *buf = (uint8_t *)malloc(BUF_SIZE);
	uint32_t total = 0;
	if (1) {
		u32 nread = fread(buf, 1, BUF_SIZE, fp);
		printf("fread nread=%d\n", nread);
		if (nread < BUF_SIZE && feof(fp))
			return -1;

		if (nread > 0) {
			s32 nwrite;
			nwrite = write(fd, (void *)buf, nread);
			if (nwrite < 0) {
				printf("write failed, ret=%d", nwrite);
				return -2;
			}
		}
	}

	if (1) {
		u32 nread = fread(buf, 1, BUF_SIZE, fp);
		printf("fread nread=%d\n", nread);
		if (nread < BUF_SIZE && feof(fp))
			return -1;

		if (nread > 0) {
			s32 nwrite;
			nwrite = write(fd, (void *)buf, nread);
			if (nwrite < 0) {
				printf("write failed, ret=%d", nwrite);
				return -2;
			}
		}
	}

	printf("wait for dec ready\n");
	while (1) {
		uint32_t u4DecReady;
		printf("enter ioctl\n");
		ioctl(fd, IOCTL_MTK_ADEC_GET_DECREADY, &u4DecReady);
		printf("out of ioctl\n");

		if (u4DecReady == 0) {
			sleep(1);
			continue;
		} else
			break;
	}

	while (1) {
		int64_t u8stc;
		s32 nstc = read(stc, &u8stc, sizeof(u8stc));
		printf("nstc=%d stc=%lld, mediatime=%lldms\n", nstc, u8stc, u8stc * 1000 / 90000);
		sleep(1);
	}

//	ioctl(fd, 4, 0); // esm disconnect
//	ioctl(fd, 0, 1); // Stop

	close(stc);

	close(fd);
	fclose(fp);
	
	printf("the end\n");

	return 0;
}
