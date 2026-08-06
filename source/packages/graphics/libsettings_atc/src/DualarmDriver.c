#include "DualarmDriver.h"


int OpenDualarmDriver() {
	int fd = -1;

	fd = open ("/dev/dualarm-dev",O_RDWR,0);

	return fd;
	

}

bool SendDualarmMessage(int fd) {

	int ret;
	
	if (fd < -1) {
		printf("invalid fd");
		return false;
	}

	DUALARM_PARAM param = {0x2 << 24,1,2,3};

	
	

	ret = ioctl(fd,DUALARM_IOC_SENDMESSAGE,&param);

	if(ret<0) {
		printf("failed to send message");
		return false;
	}

	return true;

}

bool GetDualarmMessage(int fd,DUALARM_PARAM * message) {
	int ret;
	
	if (fd < -1) {
		printf("invalid fd");
		return false;
	}

	ret = ioctl(fd,DUALARM_IOC_GETMESSAGE,message);
	
	if(ret<0) {
		printf("failed to send message");
		return false;
	}

	return true;
	


}

void CloseDualarmDriver(int fd) {
	if (fd < -1) {
		return ;
	}
	close(fd);

	return ;
}

