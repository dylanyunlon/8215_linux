#include "ota_view.h"
#include "definition.h"
#include "aic_lang.h"

void ota_view_refresh_uuid(const char *str)
{
    ota_page_t *page = ota_page_get(&ui_manager);
    VIEW_MEMBER_VALID(page, uuid);
    lv_label_set_text(uuid, str );
}

void ota_view_refresh_bluetooth(const char *str)
{
    ota_page_t *page = ota_page_get(&ui_manager);
    VIEW_MEMBER_VALID(page, bluetooth);
    lv_label_set_text(bluetooth, str );
}

void ota_view_refresh_ota_passworld(const char *str)
{
    ota_page_t *page = ota_page_get(&ui_manager);
    VIEW_MEMBER_VALID(page, ota);
    lv_label_set_text(ota, str );
}

void ota_view_refresh_sn(const char *str)
{
    ota_page_t *page = ota_page_get(&ui_manager);
    VIEW_MEMBER_VALID(page, sn);
    lv_label_set_text(sn, str );
}

void ota_view_refresh_version(const char *str)
{
    ota_page_t *page = ota_page_get(&ui_manager);
    VIEW_MEMBER_VALID(page, version);
    lv_label_set_text(version, str );
}


void ota_view_refresh_ota_state(const char *str )
{
    ota_page_t *page = ota_page_get(&ui_manager);
    VIEW_MEMBER_VALID(page, ota_state);
    lv_label_set_text(ota_state, str);
}

void ota_view_set_progress(uint8_t percent)
{
    ota_page_t *page = ota_page_get(&ui_manager);
    VIEW_MEMBER_VALID(page, progress_arc);
    lv_arc_set_value(progress_arc, percent > 100 ? 100 : percent);
    lv_label_set_text_fmt (page->progress_label, "%d%%" , percent > 100 ? 100 : percent);
}

void ota_view_update_status(const char *str)
{
    ota_page_t *page = ota_page_get(&ui_manager);
    VIEW_MEMBER_VALID(page, update_state);
    lv_label_set_text(update_state, str);
}

void ota_view_update_error(const char *str)
{
    ota_page_t *page = ota_page_get(&ui_manager);
    VIEW_MEMBER_VALID(page, update_error);
    lv_label_set_text(update_error, str);
}