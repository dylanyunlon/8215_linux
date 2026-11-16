#include "dvr_page_key.h"
#include "window_manager.h"
#include "dvr_logic.h"

/* DVR 页面的实体键路由：
 *   BACK 短按：在子视图（文件列表/设置）里 = 回主视图；已在主视图 = 退出本页
 *   UP/DOWN 短按：子视图里的焦点上下移动
 *   SET 短按：激活焦点项（列表=播放，设置=切换该项） */
void dvr_manager_click(const click_e click)
{
    switch (click) {
        case CLICK_SHORT_BACK:
            if (!dvr_logic_back()) {
                window_manager_set_target(WINDOW_MAIN);
            }
            break;
        case CLICK_SHORT_UP:
            dvr_logic_action(DVR_ACTION_UP);
            break;
        case CLICK_SHORT_DOWN:
            dvr_logic_action(DVR_ACTION_DOWN);
            break;
        case CLICK_SHORT_SET:
            dvr_logic_action(DVR_ACTION_SELECT);
            break;
        default:
            break;
    }
}
