#ifndef _BOOTANIMATIONDRV_H_
#define _BOOTANIMATIONDRV_H_

#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <stdbool.h>


#include "bootanidrv.h"


#ifdef __cplusplus
extern "C" {
#endif

int OpenAnimationDrv();

bool SendAnimationMsg(int fd);

bool GetAnimationMsg(int fd,DUALARM_PARAM * message);

bool GetVBARsvmemInfo(int fd,LOGO_BUF_INFO_T *rLogoBufInfo);

bool GetOsdInfo(int fd,LOGO_BUF_INFO_T *info);

bool ReleaseAnimationMem(int fd);

bool LightenScreen(int fd);

bool ShutDownScreen(int fd);

bool ShowBackCarUI(int fd);

bool HideBackCarUI(int fd);

void CloseAnimationDriver(int fd);

#ifdef __cplusplus
}
#endif

#endif
