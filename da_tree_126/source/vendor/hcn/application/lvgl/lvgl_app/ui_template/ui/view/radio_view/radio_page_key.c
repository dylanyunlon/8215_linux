#include "radio_page_key.h"
#include "window_manager.h"
#include "radio_logic.h"

void radio_manager_click(const click_e click)
{
    switch (click) {
        case CLICK_SHORT_BACK:
            window_manager_set_target(WINDOW_MAIN);
            break;
        default:
            break;
    }
}
