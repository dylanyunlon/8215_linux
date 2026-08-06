#include <stdio.h>

#include <fcntl.h>
#include <unistd.h>

#include <backcar.h>
#include <atcsurface.h>
#include <sys/ioctl.h>
#include <string.h>



int main(int argc, char **argv)
{
    int fd;
    IAtcSurface *overlay = NULL;
    //struct v4l2_capability  cap;    
    fd = open("/dev/ttyMT0", O_RDWR);
    dup2(fd, 0);
    dup2(fd, 1);
    dup2(fd, 2);
    close(fd);

    fprintf(stderr, "Oh, My God backcar\r\n");
    printf("backcar_test - NO.1\r\n");
    IBackcarVideo *backcar = IBackcarVideo::getInstance();
     printf("backcar_test - before init\n");
		    backcar->init(); 
		    printf("backcar_test - after init\n");
    /*
    overlay = atc_createsurface(ATCSURF_TYPE_DEFAULT, 720, 480, ATC_PIX_FMT_NV12_PRIVATE1);
    if (!overlay) {
        printf("main(): atc_createsurface failed \n");
    } else {
        IAtcSurface_setBufferCount(overlay, 4); 
    }
    
   
    printf("backcar_test - before init\n");
    backcar->init(); 
    printf("backcar_test - after init\n");
    
    IAtcSurface_setWindow(overlay, 0, 0,1024, 600);
    backcar->setVideoSurface(overlay);
    */
    while (true) {
        int event = backcar->getEvent();

        if (event == EVENT_BACKCAR_START) {
	    printf("backcar_test - start overlay test 2%p\n", overlay);
	    if (!overlay) {
	    	overlay = atc_createsurface(ATCSURF_TYPE_DEFAULT, 720, 480, ATC_PIX_FMT_NV12_PRIVATE1);
		    if (!overlay) {
		        printf("main(): atc_createsurface failed \n");
		    } else {
		    	printf("main(): atc_createsurface s \n");
		        IAtcSurface_setBufferCount(overlay, 4); 
				IAtcSurface_setLayerZOrder(overlay, 3);
		    }
		   
		    
		    IAtcSurface_setWindow(overlay, 0, 0,1024, 600);
		    backcar->setVideoSurface(overlay);
	    }
            backcar->start();
        } else if (event == EVENT_BACKCAR_STOP) {
            printf("backcar_test - stop overlay %p\n", overlay);
	    backcar->stop();
	    backcar->setVideoSurface(NULL);
	    if (overlay) {
	    	IAtcSurface_release(overlay);
		overlay = NULL;
	    }
        } 
    }

    return (0);
}
