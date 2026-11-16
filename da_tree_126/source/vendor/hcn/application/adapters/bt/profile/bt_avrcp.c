#include "bt_avrcp.h"

#include "bt_core.h"
#include "bt_platform.h"

int bt_avrcp_init(void)
{
    return 0;
}

void bt_avrcp_deinit(void)
{
}

void bt_avrcp_play_pause(bool currently_playing)
{
    if (bt_platform_avrcp_play_pause(currently_playing) != 0) {
        bt_core_notify_status("AVRCP play pause request failed");
    }
}

void bt_avrcp_pause(void)
{
    if (bt_platform_avrcp_pause() != 0) {
        bt_core_notify_status("AVRCP pause request failed");
    }
}

void bt_avrcp_prev(void)
{
    if (bt_platform_avrcp_prev() != 0) {
        bt_core_notify_status("AVRCP previous request failed");
    }
}

void bt_avrcp_next(void)
{
    if (bt_platform_avrcp_next() != 0) {
        bt_core_notify_status("AVRCP next request failed");
    }
}
