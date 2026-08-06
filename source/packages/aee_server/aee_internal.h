#ifndef __AEE_INTERNAL_H_
#define __AEE_INTERNAL_H_

#include <stdarg.h>
#include <string.h>
#include <sys/ioctl.h>

#include "utils.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DAL_MSG_LEN         1024

typedef struct {
	char msg[1024];
} AE_DAL_DATA;

typedef struct {
    unsigned int fgcolor;
    unsigned int bgcolor;
    unsigned int screencolor;
} AE_DAL_SETCOLOR;


#define AEEIOCTL_DAL_SHOW                   _IOW('p', 0x01, AE_DAL_DATA)
#define AEEIOCTL_DAL_CLEAN                  _IO('p', 0x02)
#define AEEIOCTL_DAL_SETCOLOR    	        _IOW('p', 0x03, AE_DAL_SETCOLOR)
#define AEEIOCTL_GET_PROCESS_BT_OLD 		_IOW('p', 0x04, struct aee_process_bt_OLD)
#define AEEIOCTL_GET_PROCESS_BT             _IOW('p', 0x04, struct aee_ioctl)
#define AEEIOCTL_GET_SMP_INFO               _IOR('p', 0x05, int)
#define AEEIOCTL_SET_AEE_MODE               _IOR('p', 0x06, int)
#define AEEIOCTL_GET_THREAD_REG 		    _IOW('p', 0x07, struct aee_thread_reg)
#define AEEIOCTL_CHECK_SUID_DUMPABLE   		_IOR('p', 0x08, int)
#define AEEIOCTL_SET_FORECE_RED_SCREEN 	    _IOR('p', 0x0B, int)
#define AEEIOCTL_USER_IOCTL_TO_KERNEL_WANING    _IOR('p', 0x0E, int)

#define AE_ROOT_DIR             "/data/aee_exp"
#define AE_ROOT_STORAGE         "/ext_sdcard2"
#define AE_CUID_ROOT            AE_ROOT_STORAGE "/mtklog/aee_exp"

#define AE_EXTERNAL_STORAGE        "/storage/ext_sd"
#define AE_EXTERNAL_STORAGE_ROOT   AE_EXTERNAL_STORAGE "/mtklog/aee_exp"

#define AE_CUID_ROOT_BACKUP     "/data/aee_exp"
#define AE_DATA_STORAGE         "/data"
#define AE_SDCARD_TEST_FILE     "/ext_sdcard2/.aee_test"
#define AE_DUMPSYS_DATA_PATH    "/data/dumpsys"
#define AE_DATA_TMPFS_LOG       "/data/data_tmpfs_log/aee_exp"

#define AE_EE_DEVICE_PATH       "/dev/aed0"
#define AE_KE_DEVICE_PATH       "/dev/aed1"

#define AEE_VERSION_INFO        "W1140.00"

//ftrace related
#define FTRACE_ENABLE_CNF_PATH  ("/sys/kernel/debug/tracing/tracing_on")
#define FTRACE_ENABLE_CNF_ON    ("1")
#define FTRACE_ENABLE_CNF_OFF   ("0")

//kptr_restrict related
#define RR_PROC_KPTR_RESTRICT   "/proc/sys/kernel/kptr_restrict"
#define BOOT_MODE_INFO_FILE     "/sys/class/BOOT/BOOT/boot/boot_mode"

typedef enum {
    AEE_MODE_MTK_ENG = 1,
    AEE_MODE_MTK_USER,
    AEE_MODE_CUSTOMER_ENG,
    AEE_MODE_CUSTOMER_USER
} AEE_MODE;

typedef enum {
    NORMAL_BOOT = 0,
    META_BOOT,
    RECOVERY_BOOT,
    SW_REBOOT,
    FACTORY_BOOT,
    ADVMETA_BOOT,
    ATE_FACTORY_BOOT,
    ALARM_BOOT,
    UNKNOWN_BOOT
} BOOTMODE;

typedef enum {
    AE_SUCC,
    AE_FAIL
} AE_ERR;

typedef enum {
    AE_PASS_BY_MEM,
    AE_PASS_BY_FILE,
    AE_PASS_METHOD_END
} AE_PASS_METHOD;

typedef enum {
    AE_REQ,
    AE_RSP,
    AE_IND,
    AE_CMD_TYPE_END
} AE_CMD_TYPE;

typedef enum  {
    AE_REQ_OBSOLETE_COMMAND0,

    AE_REQ_CLASS,
    AE_REQ_TYPE,
    AE_REQ_PROCESS,
    AE_REQ_MODULE,
    AE_REQ_BACKTRACE,
    AE_REQ_DETAIL,    /* Content of response message rule:
                       *   if msg.arg1==AE_PASS_BY_FILE => msg->data=file path
                       */

    AE_REQ_OBSOLETE_COMMAND1,
    AE_REQ_OBSOLETE_COMMAND2,
    AE_REQ_OBSOLETE_COMMAND3,
    AE_REQ_OBSOLETE_COMMAND4,

    AE_IND_FATAL_RAISED, // fatal event raised, indicate AED to notify users
    AE_IND_EXP_RAISED =12,// exception event raised, indicate AED to notify users
    AE_IND_WRN_RAISED,   // warning event raised, indicate AED to notify users
    AE_IND_REM_RAISED,   // reminding event raised, indicate AED to notify users

    AE_IND_LOG_STATUS, // arg = AE_ERR
    AE_IND_LOG_CLOSE,  // arg = AE_ERR

    AE_REQ_OBSOLETE_COMMAND5,
    AE_REQ_OBSOLETE_COMMAND6,
    AE_REQ_OBSOLETE_COMMAND7,
    AE_REQ_OBSOLETE_COMMAND8,
    AE_REQ_OBSOLETE_COMMAND9,
    AE_REQ_COREDUMP,        // msg->data=file path
    AE_REQ_OBSOLETE_COMMAND10,
    //AE_REQ_E2S_INIT,  //add only for dismatch define with kernel aed.h ???????
    AE_REQ_USERSPACEBACKTRACE=40,
    AE_REQ_USER_REG,
    AE_REQ_USER_MAPS,
    AE_CMD_ID_END
} AE_CMD_ID;

typedef struct {
    AE_CMD_TYPE cmdType;  /* command type			*/
    AE_CMD_ID   cmdId;    /* command Id				*/
    int dal_fd;			  /* dev/aed0 devices		*/
	int sig_no;			  /* Signal Nunber.			*/
} AE_Msg;

typedef struct {
    int     dal_and_vibrate_on;
    int     db_cnt;
    int     db_fatal_cnt;
    int     db_onlysd;  // 1:force dump to sd. 0:default,2:force to data
    int     mode;       // 1:Internal Eng, 2:Internal user, 3:custom Eng, 4:custom user
    char    exp_level;
    int     bSMP;
    int     dal_fd;
    int     vibrate_fd;
} AED_SYSTEM;

#define AID_SYSTEM        1000  /* system server */
#define AID_LOG           1007  /* log devices */
#define AID_SDCARD_RW     1015  /* external storage write access */
#define AID_SHELL         2000  /* adb and debug shell user */

extern int fd;
extern int ts_fd;
#define WRITE_STRING(s)                     \
    if (fd >= 0)                            \
        write(fd, s, strlen (s));           \
    if (ts_fd >= 0)                    \
        write(ts_fd, s, strlen (s));

#define WRITE_STRING_FD(fd, s)              \
    if (fd >= 0)                            \
        write(fd, s, strlen (s));           \


#define TAG               "AEE"
#define AEE_LOGI(format, ...) syslog(LOG_INFO, format, ##__VA_ARGS__)
#define AEE_LOGW(format, ...) syslog(LOG_WARNING, format, ##__VA_ARGS__)
#define AEE_LOGD(format, ...) syslog(LOG_DEBUG, format, ##__VA_ARGS__)

inline void AEE_LOGE(const char *msg, ...) {
    char buf[MAX_PATH] = {'\0'};

    if (msg != nullptr)
    {
        va_list ap;
        va_start(ap, msg);
        vsnprintf(buf, MAX_PATH, msg, ap);
        va_end(ap);
    }
    write(1, buf, strlen (buf));
}

void *dump_ke(void *p);
void dump_ne(int signo, int pid);

int aee_dal_clean(void);


#ifdef __cplusplus
};
#endif

#endif /* __aee_internal.h */
