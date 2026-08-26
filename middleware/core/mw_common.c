#include <stdio.h>
#include <stdint.h>
#include "mw_common.h"
#include "mw_pthread.h"
#include "mw_log.h"
#include "usr_utils.h"
#include "set_param.h"

static void* mw_common_thread(void* arg) {
    while (1) {
        mw_delay_ms(5000);
    }
    pthread_exit(NULL);
}

int mw_common_init(void) {
    int ret = -1;

#if 0
    /* 初始化用户设置参数(从 /data/set_param 投递点迁移到 /config 持久存储) */
    if (usr_param_init() != 0) {
        mw_log_error("usr_param_init failed, use default params\n");
    } else {
        mw_log_info("[mw] usr_param_init OK\n");
    }
#endif

    ret = mw_pthread_create("mw_common", NULL, mw_common_thread);
    if (ret != 0) {
        mw_log_error("mw_common pthread_create failed!\n");
        return -1;
    }
    mw_log_info("[mw] mw_common_init OK\n");
    return 0;
}
