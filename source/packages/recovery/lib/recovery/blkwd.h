#ifndef BLKWD_H_
#define BLKWD_H_

struct wd_name {
    int fd;
    int wd;
    char *name;
};

#define SDCARDPLUGIN        1 //sdcarde has been plug-in
#define SDCARDPLUGOUT       2 //sdcard has been plug-out
#define UDISKPLUGIN         3 //udisk has been plug-in
#define UDISKPLUGOUT        4 //udisk has been plug-out

int init_blkwd(struct wd_name *pwd);
int destory_blkwd(struct wd_name *pwd);
int blkwd_event(struct wd_name *pwd);

#endif /* _BLKWD_H_ */

