#ifndef NFC_VIEW_H__
#define NFC_VIEW_H__

#include "ui_init.h"
#include "aic_ui.h"
#include "button_manager.h"

void nfc_view_init(void);

void on_nfc_view_deal_short_key(click_e key);

void nfc_view_clean_state(void);

void nfc_view_set_focused_item(int focusedIndex);

#endif