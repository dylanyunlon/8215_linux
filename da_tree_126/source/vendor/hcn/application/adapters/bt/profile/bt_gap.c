#include "bt_gap.h"

#include "bt_core.h"
#include "bt_platform.h"

int bt_gap_init(void)
{
    return 0;
}

void bt_gap_deinit(void)
{
}

void bt_gap_set_power(bool enabled)
{
    if (bt_platform_gap_set_power(enabled) != 0) {
        bt_core_notify_status("Bluetooth power request failed");
    }
}

void bt_gap_start_scan(void)
{
    if (bt_platform_gap_start_scan() != 0) {
        bt_core_notify_status("Bluetooth scan request failed");
    }
}

void bt_gap_stop_scan(void)
{
    if (bt_platform_gap_stop_scan() != 0) {
        bt_core_notify_status("Bluetooth stop scan request failed");
    }
}

void bt_gap_pair_device(const char *address)
{
    if (bt_platform_gap_pair(address) != 0) {
        bt_core_notify_status("Bluetooth pair request failed");
    }
}

void bt_gap_remove_device(const char *address)
{
    if (bt_platform_gap_remove(address) != 0) {
        bt_core_notify_status("Bluetooth remove pair request failed");
    }
}
