#include "bt_pbap.h"

#include "bt_core.h"
#include "bt_platform.h"

int bt_pbap_init(void)
{
    return 0;
}

void bt_pbap_deinit(void)
{
}

void bt_pbap_download_phonebook(void)
{
    if (bt_platform_pbap_download_phonebook() != 0) {
        bt_core_notify_status("PBAP phonebook request failed");
    }
}

void bt_pbap_download_call_records(void)
{
    if (bt_platform_pbap_download_call_records() != 0) {
        bt_core_notify_status("PBAP call record request failed");
    }
}

void bt_pbap_abort_download(void)
{
    if (bt_platform_pbap_abort() != 0) {
        bt_core_notify_status("PBAP abort request failed");
    }
}
