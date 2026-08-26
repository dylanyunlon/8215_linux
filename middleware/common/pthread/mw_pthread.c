#include <string.h>
#include "mw_log.h"
#include "mw_pthread.h"
#include "osal.h"

int mw_pthread_create(const char* pthread_name, osal_thread_t* tid,
                      mw_thread_fun fun) {
    MW_ASSERT_WITH_VALUE(pthread_name != NULL, -1);
    if (strlen(pthread_name) > 16) {
        mw_log_error("Pthread_name too long!\n");
        return -1;
    }
    /* 委托 OSAL：detach/命名/创建由后端统一处理 */
    return (osal_thread_create(tid, pthread_name, fun, NULL, 0, 0) == OSAL_OK)
               ? 0
               : -1;
}
