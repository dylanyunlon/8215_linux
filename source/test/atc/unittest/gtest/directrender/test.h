#ifndef GTEST_TEST_H_
#define GTEST_TEST_H_

#include <linux/types.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>
#include <syslog.h>
#include <unistd.h>
#include <locale.h>
#include <stdlib.h>
#include <stdio.h>
//#include <atcsurface.h>
#include <linux/fb.h>
#include "async_queue.h"
#include "avcodec.h"
#include "load_data.h"
#include "display.h"
#include "memdbg_c.h"
#include <dirent.h>

#define PRINT_INFO(format, ...)   fprintf(stderr, "[I][MM][DTDemo][%s:%d] " format"\n", __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define PRINT_ERROR(format, ...)  fprintf(stderr, "[E][MM][DTDemo][%s:%d] " format"\n", __FUNCTION__, __LINE__, ##__VA_ARGS__)

#define DEC_ALIGN_MASK(value, mask)			((((value) + ((mask) - 1)) / (mask)) * (mask))

int test( const char *filename, const char *goldenname);


#endif  // GTEST_TEST_H_
