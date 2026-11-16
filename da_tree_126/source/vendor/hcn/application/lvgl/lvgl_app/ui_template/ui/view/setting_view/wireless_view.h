#ifndef WIRELESS_VIEW_H__
#define WIRELESS_VIEW_H__

#include "ui_init.h"
#include "aic_ui.h"
#include "button_manager.h"

void wireless_view_init(void);

void on_wireless_view_deal_short_key(click_e key);

void wireless_view_clean_state(void);

void wireless_view_set_focused_item(int focusedIndex);

#endif
