#include "bt_a2dp.h"

#include "bt_core.h"
#include "bt_platform.h"

int bt_a2dp_init(void)
{
    if (bt_platform_a2dp_init() != 0) {
        bt_core_notify_status("A2DP init failed");
        return -1;
    }
    return 0;
}

void bt_a2dp_deinit(void)
{
}

void bt_a2dp_set_audio_enabled(bool enabled)
{
    if (bt_platform_a2dp_set_audio_enabled(enabled) != 0) {
        bt_core_notify_status(enabled ? "A2DP enable audio failed" : "A2DP disable audio failed");
    }
}
