/*
* Copyright (c) 2016 AutoChips Inc.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*    http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#include <sys/time.h>
#include <sys/resource.h>

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <fcntl.h>
#include <string.h>
#include <sys/mman.h>
#include <malloc.h>
#include "AtcLogoSettings.h"

#include <errno.h>

#ifdef BOOT_NAND
#define LOGO_DEV_NAME "/dev/mtdblock11" 
#else
#define LOGO_DEV_NAME "/dev/logo"
#endif

static int fd = -1;
#define DEV_OFFSET 				(0x58ll)	// Offset for MRF file, info of file list..
#define LOGO_PARTITION_SIZE		(0x400000)	// 4MB


/***************************************************************************************************/
/* Read logo deivce specified by LOGO_DEV_NAME															*/
/***************************************************************************************************/
static int _open_logo_device(void)
{
	fd = open(LOGO_DEV_NAME, O_RDWR);
	if(fd == (int)-1){
		printf("[libLOGO] Open %s error, %s\n", LOGO_DEV_NAME, strerror(errno));
		return -1;
	}
	return 0;
}

/***************************************************************************************************/
/* Close logo device specified by LOGO_DEV_NAME															*/
/***************************************************************************************************/
static int _close_logo_device(void)
{
	if(fd == (int)-1){
		printf("[libLOGO] Close device error, the device does not be opened.\n");
		return -1;
	}
	close(fd);
	fd = -1;
	return 0;
}

/***************************************************************************************************/
/* Read data from the logo device. 																			*/
/* @param buf		the buffer has data																	*/
/* @param offset		the offset from the start of logo partition												*/
/* @param size		the size should be read in bytes														*/
/* return value is the bytes read from logo device, and the data is copyed into buf. -1 means read error 			*/
/***************************************************************************************************/
static long _read_logo_data(byte *buf, int offset, long size)
{
	off_t roffset = DEV_OFFSET;
	long read_count = 0;

	if(fd == (int)-1 || buf == NULL){
		printf("[libLOGO] The device %s does not be opened, can not read it.\n", LOGO_DEV_NAME);
		return -1;
	}
	roffset += offset;
	roffset = lseek(fd, roffset, SEEK_SET);
	if(offset == (off_t)-1){
		printf("[libLOGO] Set offset error, %s\n", strerror(errno));
		return -1;
	}

	if ((size + roffset) > LOGO_PARTITION_SIZE) {
		printf("[libLOGO] Total size large than logo partition size, total size is %d\n", (size + roffset));
		return -1;
	}

	read_count = read(fd, buf, size);
	if(read_count == (long)-1){
		printf("[libLOGO] Read data from device %s error, %s\n", LOGO_DEV_NAME, strerror(errno));
		return -1;
	}
	if(read_count < size){
		printf("[libLOGO] Read data not consistent with expections\n");
	}
	return read_count;
}

/***************************************************************************************************/
/* Read data from the logo device.																			*/
/* @param buf		the buffer has data																	*/
/* @param buf_size	the buffer size																		*/
/* @param start		the offset from the start of LOGO_DEV_NAME											*/
/* @param size		the size should be read in bytes														*/
/* return value is the bytes read from logo device, and the data is copyed into buf. -1 means write error.			*/
/***************************************************************************************************/
static long _write_logo_data(byte *buf, int offset , long size)
{
	off_t roffset = DEV_OFFSET;
	long write_count = 0;

	if(fd == (int)-1 || buf == NULL){
		printf("[libLOGO] The device %s does not be opened, can not write it.\n", LOGO_DEV_NAME);
		return -1;
	}
	roffset += offset;
	roffset = lseek(fd, roffset, SEEK_SET);
	if(offset == (off_t)-1){
		printf("[libLOGO] Set offset error, %s\n", strerror(errno));
		return -1;
	}

	if ((size + roffset) > LOGO_PARTITION_SIZE) {
		printf("[libLOGO] Total size large than logo partition size, total size is %d\n", (size + roffset));
		return -1;
	}
		
	write_count = write(fd, buf, size);

	if(write_count < size){
		printf("[libLOGO] Write data not consistent with expections\n");
	}

	/* Sync data from cache to storage media */
	fsync(fd);

	return write_count;
}

/***************************************************************************************************/
/* read logo from device																				*/
/* @param buf		the buffer used to store logo data													*/
/* @param buf_size	the buffer size																	*/
/* note: if buffer is NULL, return value is the buffer size needed to store 									*/
/* the logo data. otherwise read the logo data and store it to buffer.										*/
/***************************************************************************************************/
int atc_read_logo(byte *buf, int offset, int size)
{
	unsigned long read_count = 0;
	
	if (size <= 0) {
		printf("[libLOGO] read logo size is invalid, (%d)\n", size);
		return -1;
	}

	if(buf == NULL){
		printf("[libLOGO] Read buf is null.\n");
		return 0;
	}

	/* Open logo device */
	if(_open_logo_device()){
		printf("[libLOGO] Can not open logo device\n");
		return 0;
	}

	/* Read data */
	if((read_count = _read_logo_data(buf, offset, size))!= size){
		printf("[libLOGO] Can not read enough data from logo device, have read %d bytes\n",size);
	}
	if(_close_logo_device()){
		printf("[libLOGO] Can not close logo device\n");
	}

	return read_count;
}

/***************************************************************************************************/
/* Write the file data as logo 																				*/
/* @param file_name 	the file name used as logo																*/
/***************************************************************************************************/
int atc_write_logo(byte *buf, int offset, int size)
{
	int wlen = 0;

	if (size <= 0) {
		printf("[libLOGO] write logo size is invalid, (%d)\n", size);
		return -1;
	}

	/* open logo device */
	if(_open_logo_device()){
		printf("[libLOGO] Can not open logo device\n");
		return (wlen);
	}	

	if((wlen = _write_logo_data(buf, offset, size))!= size){
		printf("[libLOGO] Can not write data into logo device, write %d bytes\n", size);
	}

	if(_close_logo_device()){
		printf("[libLOGO] Can not close device %s\n", LOGO_DEV_NAME);
	}

	return wlen;
}
