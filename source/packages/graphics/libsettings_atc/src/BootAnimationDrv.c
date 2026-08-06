#include "BootAnimationDrv.h"


int OpenAnimationDrv() {
	int fd = -1;

	fd = open ("/dev/bootanidrv",O_RDWR,0);
	if(fd < 0) {
		printf("[Er][Misc][BootAniDrv]open failed %s\n",__func__);
	}
	
	return fd;
}

bool SendAnimationMsg(int fd) {

	int ret =-1;

	if (fd < -1) {
		printf("[Er][Misc][BootAniDrv]invalid fd %s",__func__);
		return false;
	}

	DUALARM_PARAM param = {0x2 << 24,1,2,3};

	ret = ioctl(fd,BOOTANI_IOC_SENDMESSAGE,&param);

	if(ret<0) {
		printf("[Er][Misc][BootAniDrv] failed to send message %d @ %s",ret,__func__);
		return false;
	}
	printf("[I][Misc][BootAniDrv] bootanimation send\n");
	
	return true;

}

bool GetAnimationMsg(int fd,DUALARM_PARAM * message) {
	int ret;

	if (fd < -1) {
		printf("[Er][Misc][BootAniDrv]invalid fd %s",__func__);
		return false;
	}

	ret = ioctl(fd,BOOTANI_IOC_GETMESSAGE,message);
	
	if(ret<0) {
		printf("[Er][Misc][BootAniDrv] %s failed to get message \n ",__func__);
		return false;
	}
	printf("[I][Misc][BootAniDrv] bootanimation get\n");

	return true;
	


}

bool GetVBARsvmemInfo(int fd,LOGO_BUF_INFO_T *rLogoBufInfo)
{
    int ret;

    if (fd < -1) {
        printf("[Er][Misc][BootAniDrv]invalid fd %s",__func__);
        return false;
    }

    ret = ioctl(fd,BOOTANI_IOC_GETVBAPHY,rLogoBufInfo);

    if(ret<0) {
        printf("[Er][Misc][BootAniDrv] %s failed to get vba rsvmem info",__func__);
        return false;
    }
    printf("[I][Misc][BootAniDrv] %s Width %d,Height %d,PhyAddr %x,Size %d\n",
                                                     __func__,rLogoBufInfo->u4Width,rLogoBufInfo->u4Height,
                                                     rLogoBufInfo->u4BufPhyAdr,rLogoBufInfo->u4BufSz);
    printf("[I][Misc][BootAniDrv] get get vba rsvmem info\n");

    return true;
}

bool GetOsdInfo(int fd,LOGO_BUF_INFO_T *rLogoBufInfo)
{
	int ret;
	
	if (fd < -1) {
		printf("[Er][Misc][BootAniDrv]invalid fd %s",__func__);
		return false;
	}
	
	ret = ioctl(fd,BOOTANI_IOC_GETOSDPHY,rLogoBufInfo);
		
	if(ret<0) {
		printf("[Er][Misc][BootAniDrv] %s failed to get osd info",__func__);
		return false;
	}
	printf("[I][Misc][BootAniDrv] %s Width %d,Height %d,PhyAddr %x,Size %d\n",
													 __func__,rLogoBufInfo->u4Width,rLogoBufInfo->u4Height,
													 rLogoBufInfo->u4BufPhyAdr,rLogoBufInfo->u4BufSz);
	printf("[I][Misc][BootAniDrv] get osd info\n");
	
	return true;
}

bool ReleaseAnimationMem(int fd)
{
	int ret;
		
	if (fd < -1) {
		printf("[Er][Misc][BootAniDrv]invalid fd %s",__func__);
		return false;
	}
		
	ret = ioctl(fd,BOOTANI_IOC_RLSREVMEM,NULL);
			
	if(ret<0) {
		printf("[Er][Misc][BootAniDrv] %s failed release",__func__);
		return false;
	}
	printf("[I][Misc][BootAniDrv]%s release success \n",__func__);
		
	return true;
}
bool LightenScreen(int fd)
{
	int ret;
			
	if (fd < -1) {
		printf("[Er][Misc][BootAniDrv]invalid fd %s",__func__);
		return false;
	}
		
	ret = ioctl(fd,BOOTANI_IOC_WAKEUP,NULL);
			
	if(ret<0) {
		printf("[Er][Misc][BootAniDrv] %s failed light",__func__);
		return false;
	}
	printf("[I][Misc][BootAniDrv]%s light success \n",__func__);
		
	return true;

}

bool ShutDownScreen(int fd)
{
	int ret;
			
	if (fd < -1) {
		printf("[Er][Misc][BootAniDrv]invalid fd %s",__func__);
		return false;
	}
		
	ret = ioctl(fd,BOOTANI_IOC_SLEEP,NULL);
			
	if(ret<0) {
		printf("[Er][Misc][BootAniDrv] %s failed light",__func__);
		return false;
	}
	printf("[I][Misc][BootAniDrv]%s ShutDown Screen success \n",__func__);
		
	return true;

}

bool ShowBackCarUI(int fd) 
{
	int ret;
	if(fd < -1) {
		printf("[Er][Misc][BootAniDrv]invalid fd %s",__func__);
		return false;
	}
	ret = ioctl(fd,BOOTANI_IOC_ENABLE_BACKCARUI,NULL);

	if(ret < 0) {		
		printf("[Er][Misc][BootAniDrv] %s failed Show BackCar UI",__func__);
		return false;
	}

	return true;
}

bool HideBackCarUI(int fd)
{
	int ret;
	if(fd < -1) {
		printf("[Er][Misc][BootAniDrv]invalid fd %s",__func__);
		return false;
	}
	ret = ioctl(fd,BOOTANI_IOC_DISABLE_BACKCARUI,NULL);

	if(ret < 0) {		
		printf("[Er][Misc][BootAniDrv] %s failed Hide BackCar UI",__func__);
		return false;
	}

	return true;
}

void CloseAnimationDriver(int fd) 
{
	int ret;
	
	if (fd < -1) {
		printf("[Er][Misc][BootAniDrv]invalid fd %s",__func__);
		return;
	}
	
	printf("[I][Misc][BootAniDrv]%s close success \n",__func__);
	close(fd);
	return ;
}

