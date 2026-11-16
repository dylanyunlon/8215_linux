/*
 * OTA page key handler
 * Dispatches button events while the OTA window is active.
 */

#include "ota_page_key.h"
#include "window_manager.h"
#include "ui_objects.h"
#include "ota_logic.h"
#include "vehicle_param.h"
#include "vehicle_data.h"

void ota_manager_click(const click_e click)
{
    switch (click) {
        case CLICK_SHORT_BACK:
            vehicle_set_ota_page_state( 2 );
            window_manager_set_target(WINDOW_MAIN);
            break;
        default:
            break;
    }
}
