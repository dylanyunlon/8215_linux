
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
#include "logorw.h"
#include <errno.h>
#include <mrf.h>

#define LOGO_DEV_NAME 	("/dev/logo")

static int fd = -1;
#define DEV_OFFSET 				(0x5C)	// Offset for MRF file, info of file list..

/***************************************************************************************************/
/* Get offset of mrf file ,info of file list															*/
/***************************************************************************************************/
static int get_mrf_offset()
{
	int offset;
	offset=sizeof(MRFHEADER)+sizeof(BITMAPOBJINFO);
	return offset;
}

/***************************************************************************************************/
/* Read logo deivce specified by LOGO_DEV_NAME															*/
/***************************************************************************************************/
static int _open_logo_device(void)
{
	fd = open(LOGO_DEV_NAME, O_RDWR);
	if(fd == (int)-1){
		ALOGI("[LOGOS] Open %s error, %s\n", LOGO_DEV_NAME, strerror(errno));
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
		ALOGI("[LOGOS] Close device error, the device does not be opened.\n");
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
static int _read_logo_data(byte *buf, int offset, long size)
{
	off_t roffset = (off_t)get_mrf_offset();
	int read_count = 0;

	if(fd == (int)-1 || buf == NULL){
		ALOGI("[LOGOS] The device %s does not be opened, can not read it.\n", LOGO_DEV_NAME);
		return -1;
	}
	roffset += offset;
	roffset = lseek(fd, roffset, SEEK_SET);
	if(roffset == (off_t)-1){
		ALOGI("[LOGOS] Set offset error, %s\n", strerror(errno));
		return -1;
	}

	read_count = read(fd, buf, size);
	if(read_count == (long)-1){
		ALOGI("[LOGOS] Read data from device %s error, %s\n", LOGO_DEV_NAME, strerror(errno));
		return -1;
	}
	if(read_count < size){
		ALOGI("[LOGOS] Read data not consistent with expections\n");
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
	off_t roffset = (off_t)get_mrf_offset();
	long write_count = 0;

	if(fd == (int)-1 || buf == NULL){
		ALOGI("[LOGOS] The device %s does not be opened, can not write it.\n", LOGO_DEV_NAME);
		return -1;
	}
	roffset += offset;
	ALOGI("[LOGOS]offset1 %08x \n",roffset);
	roffset = lseek(fd, roffset, SEEK_SET);
	if(roffset == (off_t)-1){
		ALOGI("[LOGOS] Set offset error, %s\n", strerror(errno));
		return -1;
	}
	ALOGI("[LOGOS]offset1 %08x \n",roffset);

	write_count = write(fd, buf, size);

	if(write_count < size){
		ALOGI("[LOGOS] Write data not consistent with expections\n");
	}

	/* Sync data from cache to storage media */
	fsync(fd);

	return write_count;
}

/***************************************************************************************************/
/* Read logo from device																				*/
/* @param buf		the buffer used to store logo data													*/
/* @param buf_size	the buffer size																	*/
/* note: if buffer is NULL, return value is the buffer size needed to store 									*/
/* the logo data. otherwise read the logo data and store it to buffer.										*/
/***************************************************************************************************/
int atc_read_logo(byte *buf, int offset, int size)
{
	int long read_count = 0;

	if (size <= 0) {
		ALOGI("[LOGOS] read logo size is invalid, (%d)\n", size);
		return -1;
	}

	if(buf == NULL){
		ALOGI("[LOGOS] Read buf is null.\n");
		return 0;
	}

	/* Open logo device */
	if(_open_logo_device()){
		ALOGI("[LOGOS] Can not open logo device\n");
		return 0;
	}

	/* Read data */
	if((read_count = _read_logo_data(buf, offset, size)) != size) {
		ALOGI("[LOGOS] Can not read enough data from logo device, have read %d bytes\n",size);
	}

	if(_close_logo_device()){
		ALOGI("[LOGOS] Can not close logo device\n");
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
	ALOGI("[LOGOS] write logo offset, (%d)\n", offset);

	if (size <= 0) {
		ALOGI("[LOGOS] write logo size is invalid, (%d)\n", size);
		return -1;
	}

	/* open logo device */
	if(_open_logo_device()){
		ALOGI("[LOGOS] Can not open logo device\n");
		return (wlen);
	}

	if((wlen = _write_logo_data(buf, offset, size))!= size){
		ALOGI("[LOGOS] Can not write data into logo device, write %d bytes\n", size);
	}

	if(_close_logo_device()){
		ALOGI("[LOGOS] Can not close device %s\n", LOGO_DEV_NAME);
	}

	return wlen;
}
