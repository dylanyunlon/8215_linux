#include "../ui.h"
#include "../fonts/ui_font_chinese_16.h"
#include "../frameworks/bt_focus_adapter.h"
#include "bt_app.h"
#include <stdint.h>
#include <string.h>
#include <limits.h>

typedef enum {
    BT_PAGE_DIAL = 0,
    BT_PAGE_PHONEBOOK,
    BT_PAGE_CALLRECORD,
    BT_PAGE_MUSIC,
    BT_PAGE_PAIR,
    BT_PAGE_SETTING,
    BT_PAGE_COUNT
} BtPageIndex;

typedef struct {
    const char *title;
    const lv_image_dsc_t *icon;
} BtMenuItem;

lv_obj_t * ui_Bluetooth = NULL;

static lv_obj_t * s_status_label = NULL;
static lv_obj_t * s_menu_buttons[BT_PAGE_COUNT];
static lv_obj_t * s_content = NULL;
static lv_obj_t * s_pages[BT_PAGE_COUNT];
static lv_obj_t * s_dial_label = NULL;
static lv_obj_t * s_call_panel = NULL;
static lv_obj_t * s_call_number_label = NULL;
static lv_obj_t * s_pair_state_label = NULL;
static lv_obj_t * s_pair_list_label = NULL;
static lv_obj_t * s_available_list_label = NULL;
static lv_obj_t * s_paired_device_list = NULL;
static lv_obj_t * s_available_device_list = NULL;
static lv_obj_t * s_music_state_label = NULL;
static lv_obj_t * s_music_title_label = NULL;
static lv_obj_t * s_music_artist_label = NULL;
static lv_obj_t * s_music_album_label = NULL;
static lv_obj_t * s_music_progress_label = NULL;
static lv_obj_t * s_phonebook_status_label = NULL;
static lv_obj_t * s_call_records_status_label = NULL;
static lv_obj_t * s_phonebook_search = NULL;
static lv_obj_t * s_local_name_input = NULL;
static lv_obj_t * s_pin_input = NULL;
static lv_obj_t * s_setting_keyboard = NULL;
static lv_obj_t * s_power_toggle = NULL;
static lv_obj_t * s_auto_connect_toggle = NULL;
static lv_obj_t * s_music_play_button = NULL;
static lv_obj_t * s_music_play_button_label = NULL;
static lv_obj_t * s_phonebook_download_button = NULL;
static lv_obj_t * s_phonebook_download_button_label = NULL;
static lv_obj_t * s_call_records_download_button = NULL;
static lv_obj_t * s_call_records_download_button_label = NULL;
static char s_dial_number[32];
static BtPageIndex s_current_page = BT_PAGE_PAIR;
static size_t s_selected_paired = 0U;
static size_t s_selected_available = 0U;
static int s_selected_device_list = 0;
static size_t s_selected_contact = 0U;
static size_t s_selected_call_record = 0U;
static BtCallRecordType s_call_record_filter = BT_CALL_RECORD_ALL;

// Additional variables for phonebook and call records
static lv_obj_t * s_phonebook_list = NULL;
static lv_obj_t * s_call_records_list = NULL;
static unsigned int s_rendered_device_version = UINT_MAX;
static unsigned int s_rendered_phonebook_version = UINT_MAX;
static unsigned int s_rendered_call_record_version = UINT_MAX;
static BtPbapState s_rendered_pbap_state = (BtPbapState)-1;
static BtPbapDownloadType s_rendered_pbap_download_type = (BtPbapDownloadType)-1;
static size_t s_rendered_selected_paired = (size_t)-1;
static size_t s_rendered_selected_available = (size_t)-1;
static size_t s_rendered_selected_contact = (size_t)-1;
static size_t s_rendered_selected_call_record = (size_t)-1;
static BtCallRecordType s_rendered_call_record_filter = (BtCallRecordType)-1;

static lv_obj_t * create_text_button_with_data(lv_obj_t * parent, const char * text, int32_t x, int32_t y,
                                               int32_t w, int32_t h, lv_event_cb_t cb, void * user_data);
static const char * device_display_name(const char * device, const char * address);

static const BtMenuItem k_menu_items[BT_PAGE_COUNT] = {
    { "Dial", NULL },
    { "PhoneBook", NULL },
    { "CallRecord", NULL },
    { "Music", NULL },
    { "PairRecord", NULL },
    { "Setting", NULL },
};

static bool is_pbap_downloading(const BtAppSnapshot * snapshot, BtPbapDownloadType type)
{
    return snapshot != NULL &&
           snapshot->pbap_state == BT_PBAP_STATE_DOWNLOADING &&
           snapshot->pbap_download_type == type;
}

static const lv_font_t * get_dynamic_text_font(void)
{
    return ui_font_chinese_16_get();
}

static const lv_font_t * get_keyboard_text_font(void)
{
    return &lv_font_montserrat_20;
}

static bool text_has_non_ascii(const char * text)
{
    if(text == NULL) {
        return false;
    }

    while(*text != '\0') {
        if(((unsigned char)*text) >= 0x80U) {
            return true;
        }
        text++;
    }

    return false;
}

static lv_obj_t * create_label(lv_obj_t * parent, const char * text, int32_t x, int32_t y, int32_t w, int32_t font_size)
{
    lv_obj_t * label = lv_label_create(parent);
    lv_obj_set_pos(label, x, y);
    lv_obj_set_width(label, w);
    lv_label_set_text(label, text);
    lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP);
    lv_obj_set_style_text_color(label, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(label, get_dynamic_text_font(), LV_PART_MAIN | LV_STATE_DEFAULT);
    if(font_size >= 28 && !text_has_non_ascii(text)) {
        lv_obj_set_style_text_font(label, &lv_font_montserrat_30, LV_PART_MAIN | LV_STATE_DEFAULT);
    }
    return label;
}

static lv_obj_t * create_panel(lv_obj_t * parent, int32_t x, int32_t y, int32_t w, int32_t h)
{
    lv_obj_t * panel = lv_obj_create(parent);
    lv_obj_set_pos(panel, x, y);
    lv_obj_set_size(panel, w, h);
    lv_obj_set_style_radius(panel, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(panel, lv_color_hex(0x17202A), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(panel, 220, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(panel, lv_color_hex(0x3E5267), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(panel, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_remove_flag(panel, LV_OBJ_FLAG_SCROLLABLE);
    return panel;
}

static void style_list(lv_obj_t * list)
{
    lv_obj_set_style_bg_opa(list, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(list, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(list, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
}

static void style_list_item(lv_obj_t * item)
{
    lv_obj_set_style_bg_opa(item, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(item, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(item, get_dynamic_text_font(), LV_PART_MAIN | LV_STATE_DEFAULT);
}

static lv_obj_t * create_text_button(lv_obj_t * parent, const char * text, int32_t x, int32_t y, int32_t w, int32_t h,
                                     lv_event_cb_t cb)
{
    return create_text_button_with_data(parent, text, x, y, w, h, cb, NULL);
}

static lv_obj_t * create_text_button_with_data(lv_obj_t * parent, const char * text, int32_t x, int32_t y, int32_t w, int32_t h,
                                               lv_event_cb_t cb, void * user_data)
{
    lv_obj_t * button = lv_button_create(parent);
    lv_obj_set_pos(button, x, y);
    lv_obj_set_size(button, w, h);
    lv_obj_set_style_radius(button, 6, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_event_cb(button, cb, LV_EVENT_CLICKED, user_data);

    lv_obj_t * label = lv_label_create(button);
    lv_label_set_text(label, text);
    lv_obj_center(label);
    return button;
}

static void set_toggle_image(lv_obj_t * image, bool enabled)
{
    if(image) {
        lv_obj_t * label = lv_obj_get_child(image, 0);
        lv_obj_set_style_bg_color(image, lv_color_hex(enabled ? 0x1E8E5A : 0x3A4652), LV_PART_MAIN | LV_STATE_DEFAULT);
        if(label) {
            lv_label_set_text(label, enabled ? "On" : "Off");
        }
    }
}

static const char * device_display_name(const char * device, const char * address)
{
    if(device != NULL && device[0] != '\0') {
        return device;
    }
    if(address != NULL && address[0] != '\0') {
        return address;
    }
    return "Unknown";
}

static bool copy_display_prefix(char * dst, size_t dst_size, const char * src)
{
    return bt_copy_utf8_display_prefix(dst, dst_size, src);
}

static void copy_display_text(char * dst, size_t dst_size, const char * src, const char * fallback)
{
    if(dst == NULL || dst_size == 0U) {
        return;
    }

    if(copy_display_prefix(dst, dst_size, src)) {
        return;
    }
    if(copy_display_prefix(dst, dst_size, fallback)) {
        return;
    }
    (void)copy_display_prefix(dst, dst_size, "Unknown");
}

static bool display_text_is_missing(const char * text)
{
    return text == NULL || text[0] == '\0' || strcmp(text, "Unknown") == 0;
}

static void hide_setting_keyboard(void)
{
    if(s_setting_keyboard) {
        lv_keyboard_set_textarea(s_setting_keyboard, NULL);
        lv_obj_add_flag(s_setting_keyboard, LV_OBJ_FLAG_HIDDEN);
    }
}

static void clamp_selected_indices(const BtAppSnapshot * snapshot)
{
    if(snapshot == NULL) {
        return;
    }
    if(snapshot->paired_count == 0U) {
        s_selected_paired = 0U;
    } else if(s_selected_paired >= snapshot->paired_count) {
        s_selected_paired = snapshot->paired_count - 1U;
    }
    if(snapshot->available_count == 0U) {
        s_selected_available = 0U;
    } else if(s_selected_available >= snapshot->available_count) {
        s_selected_available = snapshot->available_count - 1U;
    }
    if(snapshot->phonebook_filtered_count == 0U) {
        s_selected_contact = 0U;
    } else if(s_selected_contact >= snapshot->phonebook_filtered_count) {
        s_selected_contact = snapshot->phonebook_filtered_count - 1U;
    }
    if(snapshot->call_record_count == 0U) {
        s_selected_call_record = 0U;
    } else if(s_selected_call_record >= snapshot->call_record_count) {
        s_selected_call_record = snapshot->call_record_count - 1U;
    }
}

static void set_page(BtPageIndex page)
{
    if(page != s_current_page &&
       (s_current_page == BT_PAGE_SETTING || s_current_page == BT_PAGE_PHONEBOOK)) {
        hide_setting_keyboard();
    }

    s_current_page = page;
    bt_focus_adapter_set_bluetooth_music_page_selected(page == BT_PAGE_MUSIC);
    for(int i = 0; i < BT_PAGE_COUNT; ++i) {
        if(s_pages[i]) {
            if(i == page) {
                lv_obj_remove_flag(s_pages[i], LV_OBJ_FLAG_HIDDEN);
            } else {
                lv_obj_add_flag(s_pages[i], LV_OBJ_FLAG_HIDDEN);
            }
        }

        if(s_menu_buttons[i]) {
            if(i == page) {
                lv_obj_set_style_bg_color(s_menu_buttons[i], lv_color_hex(0xBFBFBF), LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_bg_opa(s_menu_buttons[i], 150, LV_PART_MAIN | LV_STATE_DEFAULT);
            } else {
                lv_obj_set_style_bg_opa(s_menu_buttons[i], 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            }
        }
    }

    if(ui_Bluetooth) {
        lv_obj_invalidate(ui_Bluetooth);
    }
}

static void update_dial_label(void)
{
    if(s_dial_label) {
        lv_label_set_text(s_dial_label, s_dial_number[0] ? s_dial_number : "");
    }
}

static void on_menu_clicked(lv_event_t * e)
{
    if(lv_event_get_code(e) == LV_EVENT_CLICKED) {
        uintptr_t page = (uintptr_t)lv_event_get_user_data(e);
        if(page < BT_PAGE_COUNT) {
            set_page((BtPageIndex)page);
        }
    }
}

static void on_setting_textarea_event(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * target = lv_event_get_target(e);

    if(s_setting_keyboard == NULL) {
        return;
    }

    if(code == LV_EVENT_FOCUSED || code == LV_EVENT_CLICKED || code == LV_EVENT_PRESSED) {
        lv_obj_t * textarea = target;
        lv_keyboard_mode_t mode = (lv_keyboard_mode_t)(uintptr_t)lv_event_get_user_data(e);
        lv_keyboard_set_textarea(s_setting_keyboard, textarea);
        lv_keyboard_set_mode(s_setting_keyboard, mode);
        lv_obj_set_size(s_setting_keyboard, 732, 246);
        lv_obj_set_align(s_setting_keyboard, LV_ALIGN_TOP_LEFT);
        lv_obj_remove_flag(s_setting_keyboard, LV_OBJ_FLAG_HIDDEN);
        lv_obj_move_foreground(s_setting_keyboard);
        lv_obj_update_layout(s_setting_keyboard);
        lv_obj_set_pos(s_setting_keyboard, 252, 330);
    }
}

static void on_setting_keyboard_event(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if(code == LV_EVENT_READY || code == LV_EVENT_CANCEL) {
        hide_setting_keyboard();
    }
}

static void on_home_clicked(lv_event_t * e)
{
    if(lv_event_get_code(e) == LV_EVENT_CLICKED) {
        hide_setting_keyboard();
        bt_app_go_home();
    }
}

static void on_power_clicked(lv_event_t * e)
{
    if(lv_event_get_code(e) == LV_EVENT_CLICKED) {
        BtAppSnapshot snapshot;
        bt_app_get_snapshot(&snapshot);
        bt_app_set_power(!snapshot.power_on);
    }
}

static void on_scan_clicked(lv_event_t * e)
{
    if(lv_event_get_code(e) == LV_EVENT_CLICKED) {
        BtAppSnapshot snapshot;
        bt_app_get_snapshot(&snapshot);
        if(snapshot.scanning) {
            bt_app_stop_scan();
        } else {
            s_selected_available = 0U;
            s_selected_device_list = 1;
            bt_app_start_scan();
        }
    }
}

static void on_pair_list_clicked(lv_event_t * e)
{
    if(lv_event_get_code(e) == LV_EVENT_CLICKED) {
        s_selected_device_list = 0;
        s_selected_paired = (size_t)(uintptr_t)lv_event_get_user_data(e);
        ui_Bluetooth_refresh();
    }
}

static void on_available_list_clicked(lv_event_t * e)
{
    if(lv_event_get_code(e) == LV_EVENT_CLICKED) {
        s_selected_device_list = 1;
        s_selected_available = (size_t)(uintptr_t)lv_event_get_user_data(e);
        ui_Bluetooth_refresh();
    }
}

static void on_pair_device_clicked(lv_event_t * e)
{
    if(lv_event_get_code(e) == LV_EVENT_CLICKED) {
        BtAppSnapshot snapshot;
        bt_app_get_snapshot(&snapshot);
        clamp_selected_indices(&snapshot);
        if(snapshot.available_count > 0U && snapshot.available_addresses[s_selected_available][0] != '\0') {
            bt_app_pair_device(snapshot.available_addresses[s_selected_available]);
        }
    }
}

static void on_connect_clicked(lv_event_t * e)
{
    if(lv_event_get_code(e) == LV_EVENT_CLICKED) {
        BtAppSnapshot snapshot;
        const char * address = NULL;

        bt_app_get_snapshot(&snapshot);
        clamp_selected_indices(&snapshot);
        if(s_selected_device_list == 1 &&
           snapshot.available_count > 0 && snapshot.available_addresses[s_selected_available][0] != '\0') {
            address = snapshot.available_addresses[s_selected_available];
        } else if(snapshot.paired_count > 0 && snapshot.paired_addresses[s_selected_paired][0] != '\0') {
            address = snapshot.paired_addresses[s_selected_paired];
        } else if(snapshot.available_count > 0 && snapshot.available_addresses[s_selected_available][0] != '\0') {
            address = snapshot.available_addresses[s_selected_available];
        } else if(snapshot.latest_address[0] != '\0') {
            address = snapshot.latest_address;
        }

        if(address != NULL) {
            bt_app_connect_phone(address);
        }
    }
}

static void on_disconnect_clicked(lv_event_t * e)
{
    if(lv_event_get_code(e) == LV_EVENT_CLICKED) {
        BtAppSnapshot snapshot;
        const char * address = NULL;

        bt_app_get_snapshot(&snapshot);
        clamp_selected_indices(&snapshot);
        if(snapshot.connected_address[0] != '\0') {
            address = snapshot.connected_address;
        } else if(snapshot.latest_address[0] != '\0') {
            address = snapshot.latest_address;
        } else if(snapshot.paired_count > 0 && snapshot.paired_addresses[s_selected_paired][0] != '\0') {
            address = snapshot.paired_addresses[s_selected_paired];
        }

        if(address != NULL) {
            bt_app_disconnect_phone(address);
        }
    }
}

static void on_remove_device_clicked(lv_event_t * e)
{
    if(lv_event_get_code(e) == LV_EVENT_CLICKED) {
        BtAppSnapshot snapshot;
        bt_app_get_snapshot(&snapshot);

        clamp_selected_indices(&snapshot);
        if(snapshot.paired_count > 0) {
            bt_app_remove_device(snapshot.paired_addresses[s_selected_paired]);
        }
    }
}

static void on_digit_clicked(lv_event_t * e)
{
    if(lv_event_get_code(e) != LV_EVENT_CLICKED) {
        return;
    }

    const char * digit = (const char *)lv_event_get_user_data(e);
    size_t len = strlen(s_dial_number);
    if(digit && len + strlen(digit) < sizeof(s_dial_number)) {
        strcat(s_dial_number, digit);
        update_dial_label();
    }
}

static void on_delete_clicked(lv_event_t * e)
{
    if(lv_event_get_code(e) == LV_EVENT_CLICKED) {
        size_t len = strlen(s_dial_number);
        if(len > 0) {
            s_dial_number[len - 1] = '\0';
            update_dial_label();
        }
    }
}

static void on_dial_clicked(lv_event_t * e)
{
    if(lv_event_get_code(e) == LV_EVENT_CLICKED) {
        bt_app_dial(s_dial_number);
    }
}

static void on_redial_clicked(lv_event_t * e)
{
    if(lv_event_get_code(e) == LV_EVENT_CLICKED) {
        bt_app_redial();
    }
}

static void on_hangup_clicked(lv_event_t * e)
{
    if(lv_event_get_code(e) == LV_EVENT_CLICKED) {
        bt_app_hangup_call();
    }
}

static void on_accept_call_clicked(lv_event_t * e)
{
    if(lv_event_get_code(e) == LV_EVENT_CLICKED) {
        bt_app_accept_call();
    }
}

static void on_audio_source_clicked(lv_event_t * e)
{
    if(lv_event_get_code(e) == LV_EVENT_CLICKED) {
        BtAppSnapshot snapshot;
        bt_app_get_snapshot(&snapshot);
        bt_app_switch_call_audio(!snapshot.call_audio_in_phone);
    }
}

static void on_dtmf_clicked(lv_event_t * e)
{
    if(lv_event_get_code(e) == LV_EVENT_CLICKED) {
        const char * dtmf_digit = (const char *)lv_event_get_user_data(e);
        if(dtmf_digit) {
            bt_app_send_dtmf(dtmf_digit);
        }
    }
}

static void on_music_prev_clicked(lv_event_t * e)
{
    if(lv_event_get_code(e) == LV_EVENT_CLICKED) {
        bt_app_music_prev();
    }
}

static void on_music_play_clicked(lv_event_t * e)
{
    if(lv_event_get_code(e) == LV_EVENT_CLICKED) {
        bt_app_music_play_pause();
    }
}

static void on_music_next_clicked(lv_event_t * e)
{
    if(lv_event_get_code(e) == LV_EVENT_CLICKED) {
        bt_app_music_next();
    }
}

static void on_phonebook_download_clicked(lv_event_t * e)
{
    if(lv_event_get_code(e) == LV_EVENT_CLICKED) {
        BtAppSnapshot snapshot;
        bt_app_get_snapshot(&snapshot);
        if(is_pbap_downloading(&snapshot, BT_PBAP_DOWNLOAD_PHONEBOOK)) {
            bt_app_abort_pbap_download();
        } else if(snapshot.pbap_state != BT_PBAP_STATE_DOWNLOADING) {
            bt_app_download_phonebook();
        }
    }
}

static void on_call_records_download_clicked(lv_event_t * e)
{
    if(lv_event_get_code(e) == LV_EVENT_CLICKED) {
        BtAppSnapshot snapshot;
        bt_app_get_snapshot(&snapshot);
        if(is_pbap_downloading(&snapshot, BT_PBAP_DOWNLOAD_CALL_RECORDS)) {
            bt_app_abort_pbap_download();
        } else if(snapshot.pbap_state != BT_PBAP_STATE_DOWNLOADING) {
            bt_app_download_call_records();
        }
    }
}

static void on_contact_call_clicked(lv_event_t * e)
{
    if(lv_event_get_code(e) == LV_EVENT_CLICKED) {
        if(s_current_page == BT_PAGE_CALLRECORD) {
            BtCallRecord record;
            if(bt_app_get_call_record(s_selected_call_record, s_call_record_filter, &record) &&
               record.number[0] != '\0') {
                bt_app_dial(record.number);
            }
        } else {
            BtContact contact;
            if(bt_app_get_contact(s_selected_contact, &contact) && contact.number[0] != '\0') {
                bt_app_dial(contact.number);
            }
        }
    }
}

static void on_contact_selected(lv_event_t * e)
{
    if(lv_event_get_code(e) == LV_EVENT_CLICKED) {
        s_selected_contact = (size_t)(uintptr_t)lv_event_get_user_data(e);
        ui_Bluetooth_refresh();
    }
}

static void on_call_record_selected(lv_event_t * e)
{
    if(lv_event_get_code(e) == LV_EVENT_CLICKED) {
        s_selected_call_record = (size_t)(uintptr_t)lv_event_get_user_data(e);
        ui_Bluetooth_refresh();
    }
}

static void on_phonebook_search_clicked(lv_event_t * e)
{
    if(lv_event_get_code(e) == LV_EVENT_CLICKED && s_phonebook_search) {
        bt_app_search_phonebook(lv_textarea_get_text(s_phonebook_search));
        s_selected_contact = 0U;
        hide_setting_keyboard();
        ui_Bluetooth_refresh();
    }
}

static void on_call_record_filter_clicked(lv_event_t * e)
{
    if(lv_event_get_code(e) == LV_EVENT_CLICKED) {
        s_call_record_filter = (BtCallRecordType)(uintptr_t)lv_event_get_user_data(e);
        s_selected_call_record = 0U;
        bt_app_filter_call_records(s_call_record_filter);
    }
}

static void on_set_local_name_clicked(lv_event_t * e)
{
    if(lv_event_get_code(e) == LV_EVENT_CLICKED && s_local_name_input) {
        bt_app_set_local_name(lv_textarea_get_text(s_local_name_input));
        hide_setting_keyboard();
    }
}

static void on_set_pin_clicked(lv_event_t * e)
{
    if(lv_event_get_code(e) == LV_EVENT_CLICKED && s_pin_input) {
        bt_app_set_pairing_pin(lv_textarea_get_text(s_pin_input));
        hide_setting_keyboard();
    }
}

static void on_auto_connect_changed(lv_event_t * e)
{
    if(lv_event_get_code(e) == LV_EVENT_CLICKED) {
        BtAppSnapshot snapshot;
        bt_app_get_snapshot(&snapshot);
        bt_app_set_auto_connect(!snapshot.auto_connect);
    }
}

static void create_menu(lv_obj_t * parent)
{
    create_panel(parent, 0, 0, 202, 504);

    for(int i = 0; i < BT_PAGE_COUNT; ++i) {
        lv_obj_t * item = lv_obj_create(parent);
        lv_obj_remove_style_all(item);
        lv_obj_set_pos(item, 0, i * 84);
        lv_obj_set_size(item, 202, 84);
        lv_obj_set_style_radius(item, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_add_flag(item, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_add_event_cb(item, on_menu_clicked, LV_EVENT_CLICKED, (void *)(uintptr_t)i);

        if(k_menu_items[i].icon) {
            lv_obj_t * icon = lv_image_create(item);
            lv_image_set_src(icon, k_menu_items[i].icon);
            lv_obj_align(icon, LV_ALIGN_LEFT_MID, 15, 0);
            lv_obj_add_flag(icon, LV_OBJ_FLAG_EVENT_BUBBLE);
        }

        lv_obj_t * text = lv_label_create(item);
        lv_label_set_text(text, k_menu_items[i].title);
        lv_obj_set_style_text_color(text, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_align(text, LV_ALIGN_LEFT_MID, k_menu_items[i].icon ? 72 : 24, 0);
        lv_obj_add_flag(text, LV_OBJ_FLAG_EVENT_BUBBLE);

        s_menu_buttons[i] = item;
    }
}

static void create_dial_page(lv_obj_t * parent)
{
    lv_obj_t * page = lv_obj_create(parent);
    lv_obj_remove_style_all(page);
    lv_obj_set_size(page, 732, 503);
    s_pages[BT_PAGE_DIAL] = page;

    create_panel(page, 0, 0, 732, 503);
    create_panel(page, 20, 16, 520, 56);
    s_dial_label = create_label(page, "", 48, 37, 420, 24);
    lv_obj_set_style_text_align(s_dial_label, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN | LV_STATE_DEFAULT);
    create_text_button(page, "Del", 478, 25, 62, 40, on_delete_clicked);

    const char * digits[] = {"1", "2", "3", "4", "5", "6", "7", "8", "9", "*", "0", "#"};

    for(int i = 0; i < 12; ++i) {
        int col = i % 3;
        int row = i / 3;
        create_text_button_with_data(page, digits[i], 20 + col * 147, 105 + row * 69, 112, 52,
                                     on_digit_clicked, (void *)digits[i]);
    }

    create_text_button(page, "Dial", 468, 105, 112, 52, on_dial_clicked);
    create_text_button(page, "Redial", 468, 246, 112, 52, on_redial_clicked);
}

static void create_pair_page(lv_obj_t * parent)
{
    lv_obj_t * page = lv_obj_create(parent);
    lv_obj_remove_style_all(page);
    lv_obj_set_size(page, 732, 503);
    s_pages[BT_PAGE_PAIR] = page;

    create_panel(page, 0, 0, 732, 503);
    create_panel(page, 24, 82, 320, 270);
    create_panel(page, 380, 82, 320, 270);
    create_label(page, "Paired devices", 42, 42, 260, 24);
    create_label(page, "Available devices", 398, 42, 280, 24);
    s_pair_state_label = create_label(page, "", 42, 338, 620, 20);

    s_paired_device_list = lv_list_create(page);
    lv_obj_set_pos(s_paired_device_list, 42, 140);
    lv_obj_set_size(s_paired_device_list, 280, 190);
    style_list(s_paired_device_list);

    s_available_device_list = lv_list_create(page);
    lv_obj_set_pos(s_available_device_list, 398, 140);
    lv_obj_set_size(s_available_device_list, 280, 190);
    style_list(s_available_device_list);

    create_text_button(page, "Scan/Stop", 34, 388, 110, 52, on_scan_clicked);
    create_text_button(page, "Pair", 162, 388, 88, 52, on_pair_device_clicked);
    create_text_button(page, "Connect", 268, 388, 110, 52, on_connect_clicked);
    create_text_button(page, "Disconnect", 396, 388, 128, 52, on_disconnect_clicked);
    create_text_button(page, "Delete", 542, 388, 110, 52, on_remove_device_clicked);
}

static void create_music_page(lv_obj_t * parent)
{
    lv_obj_t * page = lv_obj_create(parent);
    lv_obj_remove_style_all(page);
    lv_obj_set_size(page, 732, 503);
    s_pages[BT_PAGE_MUSIC] = page;

    create_panel(page, 0, 0, 732, 503);
    create_label(page, "Title", 250, 60, 80, 20);
    create_label(page, "Artist", 250, 112, 80, 20);
    create_label(page, "Album", 250, 164, 80, 20);
    s_music_title_label = create_label(page, "Unknown title", 300, 64, 260, 20);
    s_music_artist_label = create_label(page, "Unknown artist", 300, 116, 260, 20);
    s_music_album_label = create_label(page, "Unknown album", 300, 168, 260, 20);
    lv_label_set_long_mode(s_music_title_label, LV_LABEL_LONG_SCROLL_CIRCULAR);
    lv_label_set_long_mode(s_music_artist_label, LV_LABEL_LONG_SCROLL_CIRCULAR);
    lv_label_set_long_mode(s_music_album_label, LV_LABEL_LONG_SCROLL_CIRCULAR);
    lv_obj_set_style_text_font(s_music_title_label, get_dynamic_text_font(), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(s_music_artist_label, get_dynamic_text_font(), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(s_music_album_label, get_dynamic_text_font(), LV_PART_MAIN | LV_STATE_DEFAULT);

    s_music_progress_label = create_label(page, "00:00/00:00", 250, 230, 180, 22);
    s_music_state_label = create_label(page, "", 250, 256, 360, 22);

    create_text_button(page, "Prev", 220, 382, 90, 42, on_music_prev_clicked);
    s_music_play_button = create_text_button(page, "Play", 330, 382, 90, 42, on_music_play_clicked);
    s_music_play_button_label = lv_obj_get_child(s_music_play_button, 0);
    create_text_button(page, "Next", 440, 382, 90, 42, on_music_next_clicked);
}

static void create_phonebook_page(lv_obj_t * parent)
{
    lv_obj_t * page = lv_obj_create(parent);
    lv_obj_remove_style_all(page);
    lv_obj_set_size(page, 732, 503);
    s_pages[BT_PAGE_PHONEBOOK] = page;

    create_panel(page, 0, 0, 732, 503);
    create_panel(page, 24, 72, 480, 305);
    create_panel(page, 420, 38, 150, 42);
    create_label(page, "PhoneBook", 42, 30, 260, 28);
    s_phonebook_search = lv_textarea_create(page);
    lv_obj_set_pos(s_phonebook_search, 420, 38);
    lv_obj_set_size(s_phonebook_search, 150, 42);
    lv_textarea_set_one_line(s_phonebook_search, true);
    lv_textarea_set_placeholder_text(s_phonebook_search, "Search");
    lv_obj_set_style_text_font(s_phonebook_search, get_dynamic_text_font(), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_event_cb(s_phonebook_search, on_setting_textarea_event, LV_EVENT_ALL,
                        (void *)(uintptr_t)LV_KEYBOARD_MODE_TEXT_LOWER);
    create_text_button(page, "Search", 580, 38, 92, 42, on_phonebook_search_clicked);

    // Create a scrollable list for contacts
    s_phonebook_list = lv_list_create(page);
    lv_obj_set_pos(s_phonebook_list, 52, 114);
    lv_obj_set_size(s_phonebook_list, 450, 250);
    style_list(s_phonebook_list);
    lv_obj_add_flag(s_phonebook_list, LV_OBJ_FLAG_HIDDEN);

    s_phonebook_status_label = create_label(page, "No contacts loaded", 96, 120, 500, 22);

    s_phonebook_download_button = create_text_button(page, "Download", 520, 384, 92, 42, on_phonebook_download_clicked);
    s_phonebook_download_button_label = lv_obj_get_child(s_phonebook_download_button, 0);
    create_text_button(page, "Call", 624, 384, 72, 42, on_contact_call_clicked);
}

static void create_callrecord_page(lv_obj_t * parent)
{
    lv_obj_t * page = lv_obj_create(parent);
    lv_obj_remove_style_all(page);
    lv_obj_set_size(page, 732, 503);
    s_pages[BT_PAGE_CALLRECORD] = page;

    create_panel(page, 0, 0, 732, 503);
    create_panel(page, 24, 72, 480, 305);
    create_text_button(page, "All", 42, 34, 60, 42, on_call_record_filter_clicked);
    create_text_button_with_data(page, "In", 116, 34, 60, 42, on_call_record_filter_clicked, (void *)(uintptr_t)BT_CALL_RECORD_INCOMING);
    create_text_button_with_data(page, "Out", 190, 34, 60, 42, on_call_record_filter_clicked, (void *)(uintptr_t)BT_CALL_RECORD_OUTGOING);
    create_text_button_with_data(page, "Miss", 264, 34, 70, 42, on_call_record_filter_clicked, (void *)(uintptr_t)BT_CALL_RECORD_MISSED);

    // Create a scrollable list for call records
    s_call_records_list = lv_list_create(page);
    lv_obj_set_pos(s_call_records_list, 52, 114);
    lv_obj_set_size(s_call_records_list, 450, 250);
    style_list(s_call_records_list);
    lv_obj_add_flag(s_call_records_list, LV_OBJ_FLAG_HIDDEN);

    s_call_records_status_label = create_label(page, "No call records loaded", 64, 128, 500, 22);

    s_call_records_download_button = create_text_button(page, "Download", 520, 384, 92, 42, on_call_records_download_clicked);
    s_call_records_download_button_label = lv_obj_get_child(s_call_records_download_button, 0);
    create_text_button(page, "Call", 624, 384, 72, 42, on_contact_call_clicked);
}

static void create_setting_page(lv_obj_t * parent)
{
    lv_obj_t * page = lv_obj_create(parent);
    lv_obj_remove_style_all(page);
    lv_obj_set_size(page, 732, 503);
    s_pages[BT_PAGE_SETTING] = page;

    create_panel(page, 0, 0, 732, 503);
    create_label(page, "Bluetooth Settings", 42, 42, 360, 28);
    create_label(page, "BT Name", 42, 112, 170, 20);

    s_local_name_input = lv_textarea_create(page);
    lv_obj_set_pos(s_local_name_input, 236, 102);
    lv_obj_set_size(s_local_name_input, 240, 44);
    lv_textarea_set_one_line(s_local_name_input, true);
    lv_textarea_set_max_length(s_local_name_input, BT_LOCAL_NAME_MAX_BYTES);
    lv_textarea_set_text(s_local_name_input, "");
    lv_obj_add_flag(s_local_name_input, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_text_font(s_local_name_input, get_dynamic_text_font(), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_event_cb(s_local_name_input, on_setting_textarea_event, LV_EVENT_ALL,
                        (void *)(uintptr_t)LV_KEYBOARD_MODE_TEXT_LOWER);
    create_text_button(page, "Set", 552, 104, 90, 40, on_set_local_name_clicked);

    create_label(page, "PIN Code", 42, 192, 170, 20);

    s_pin_input = lv_textarea_create(page);
    lv_obj_set_pos(s_pin_input, 236, 182);
    lv_obj_set_size(s_pin_input, 240, 44);
    lv_textarea_set_one_line(s_pin_input, true);
    lv_textarea_set_text(s_pin_input, "");
    lv_obj_add_flag(s_pin_input, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_text_font(s_pin_input, get_dynamic_text_font(), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_event_cb(s_pin_input, on_setting_textarea_event, LV_EVENT_ALL,
                        (void *)(uintptr_t)LV_KEYBOARD_MODE_NUMBER);
    create_text_button(page, "Set", 552, 184, 90, 40, on_set_pin_clicked);

    create_label(page, "Auto Connect", 42, 272, 220, 20);
    s_auto_connect_toggle = create_text_button(page, "Off", 552, 268, 90, 40, on_auto_connect_changed);

    create_label(page, "BT Power", 42, 352, 220, 20);
    s_power_toggle = create_text_button(page, "Off", 552, 348, 90, 40, on_power_clicked);
}

static void create_setting_keyboard_overlay(lv_obj_t * parent)
{
    lv_obj_t * keyboard_parent = parent != NULL ? parent : lv_layer_top();

    s_setting_keyboard = lv_keyboard_create(keyboard_parent);
    lv_obj_set_size(s_setting_keyboard, 732, 246);
    lv_obj_set_align(s_setting_keyboard, LV_ALIGN_TOP_LEFT);
    lv_obj_set_style_bg_color(s_setting_keyboard, lv_color_hex(0x101820), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(s_setting_keyboard, 245, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(s_setting_keyboard, get_keyboard_text_font(), LV_PART_ITEMS | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(s_setting_keyboard, lv_color_hex(0x1F2933), LV_PART_ITEMS | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(s_setting_keyboard, lv_color_hex(0x1F2933), LV_PART_ITEMS | LV_STATE_CHECKED);
    lv_obj_set_style_text_color(s_setting_keyboard, lv_color_hex(0x1F2933), LV_PART_ITEMS | LV_STATE_FOCUSED);
    lv_obj_set_style_text_color(s_setting_keyboard, lv_color_hex(0x1F2933), LV_PART_ITEMS | LV_STATE_PRESSED);
    lv_obj_add_flag(s_setting_keyboard, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_event_cb(s_setting_keyboard, on_setting_keyboard_event, LV_EVENT_ALL, NULL);
    lv_obj_update_layout(s_setting_keyboard);
    lv_obj_set_pos(s_setting_keyboard, 252, 330);
}

static void create_call_overlay(lv_obj_t * parent)
{
    lv_obj_t * overlay_parent = parent != NULL ? parent : lv_layer_top();

    s_call_panel = lv_obj_create(overlay_parent);
    lv_obj_set_size(s_call_panel, 430, 500);
    lv_obj_center(s_call_panel);
    lv_obj_set_style_radius(s_call_panel, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(s_call_panel, lv_color_hex(0x101418), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(s_call_panel, 245, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(s_call_panel, 22, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_flag(s_call_panel, LV_OBJ_FLAG_HIDDEN);

    create_label(s_call_panel, "Call", 24, 20, 300, 28);
    s_call_number_label = create_label(s_call_panel, "", 24, 76, 360, 24);

    create_text_button(s_call_panel, "Answer", 24, 160, 96, 42, on_accept_call_clicked);
    create_text_button(s_call_panel, "Hangup", 160, 160, 96, 42, on_hangup_clicked);

    create_text_button(s_call_panel, "Audio", 24, 218, 82, 42, on_audio_source_clicked);

    const char * digits[] = {"1", "2", "3", "4", "5", "6", "7", "8", "9", "*", "0", "#"};
    for(int i = 0; i < 12; ++i) {
        int col = i % 3;
        int row = i / 3;
        create_text_button_with_data(s_call_panel, digits[i], 24 + col * 136, 270 + row * 52, 96, 42,
                                     on_dtmf_clicked, (void *)digits[i]);
    }
}

static void refresh_phonebook_list(const BtAppSnapshot * snapshot)
{
    size_t i;
    size_t render_count;

    if(s_phonebook_list == NULL || snapshot == NULL) {
        return;
    }

    lv_obj_clean(s_phonebook_list);
    if(snapshot->phonebook_filtered_count == 0U) {
        lv_obj_add_flag(s_phonebook_list, LV_OBJ_FLAG_HIDDEN);
        if(s_phonebook_status_label) {
            lv_obj_remove_flag(s_phonebook_status_label, LV_OBJ_FLAG_HIDDEN);
            lv_label_set_text(s_phonebook_status_label,
                              is_pbap_downloading(snapshot, BT_PBAP_DOWNLOAD_PHONEBOOK) ? "Downloading contacts..." :
                              snapshot->phonebook_count > 0U ? "No matching contacts" : "No contacts loaded");
        }
        return;
    }

    if(s_phonebook_status_label) {
        lv_obj_add_flag(s_phonebook_status_label, LV_OBJ_FLAG_HIDDEN);
    }
    lv_obj_remove_flag(s_phonebook_list, LV_OBJ_FLAG_HIDDEN);
    render_count = snapshot->phonebook_filtered_count;
    if(s_selected_contact >= render_count) {
        s_selected_contact = render_count - 1U;
    }
    for(i = 0U; i < render_count; ++i) {
        BtContact contact;
        char text[180];
        char name[96];
        char number[64];
        lv_obj_t * item;

        if(!bt_app_get_contact(i, &contact)) {
            continue;
        }
        copy_display_text(number, sizeof(number), contact.number, "");
        copy_display_text(name, sizeof(name), contact.name, number);
        lv_snprintf(text, sizeof(text), "%s%s  %s",
                    i == s_selected_contact ? "> " : "",
                    name,
                    number);
        item = lv_list_add_button(s_phonebook_list, NULL, text);
        style_list_item(item);
        lv_obj_add_event_cb(item, on_contact_selected, LV_EVENT_CLICKED, (void *)(uintptr_t)i);
    }
}

static const char * call_record_type_text(BtCallRecordType type)
{
    switch(type) {
    case BT_CALL_RECORD_INCOMING:
        return "Incoming";
    case BT_CALL_RECORD_OUTGOING:
        return "Outgoing";
    case BT_CALL_RECORD_MISSED:
        return "Missed";
    case BT_CALL_RECORD_ALL:
    default:
        return "Call";
    }
}

static const char * call_record_empty_text(const BtAppSnapshot * snapshot)
{
    if(snapshot != NULL &&
       is_pbap_downloading(snapshot, BT_PBAP_DOWNLOAD_CALL_RECORDS) &&
       s_call_record_filter == BT_CALL_RECORD_ALL) {
        return "Downloading call records...";
    }

    switch(s_call_record_filter) {
    case BT_CALL_RECORD_INCOMING:
        return "No incoming call records";
    case BT_CALL_RECORD_OUTGOING:
        return "No outgoing call records";
    case BT_CALL_RECORD_MISSED:
        return "No missed call records";
    case BT_CALL_RECORD_ALL:
    default:
        return "No call records loaded";
    }
}

static void refresh_call_records_list(const BtAppSnapshot * snapshot)
{
    size_t i;
    size_t render_count;

    if(s_call_records_list == NULL || snapshot == NULL) {
        return;
    }

    lv_obj_clean(s_call_records_list);
    if(snapshot->call_record_count == 0U) {
        lv_obj_add_flag(s_call_records_list, LV_OBJ_FLAG_HIDDEN);
        if(s_call_records_status_label) {
            lv_obj_remove_flag(s_call_records_status_label, LV_OBJ_FLAG_HIDDEN);
            lv_label_set_text(s_call_records_status_label, call_record_empty_text(snapshot));
        }
        return;
    }

    if(s_call_records_status_label) {
        lv_obj_add_flag(s_call_records_status_label, LV_OBJ_FLAG_HIDDEN);
    }
    lv_obj_remove_flag(s_call_records_list, LV_OBJ_FLAG_HIDDEN);
    render_count = snapshot->call_record_count;
    if(s_selected_call_record >= render_count) {
        s_selected_call_record = render_count - 1U;
    }
    for(i = 0U; i < render_count; ++i) {
        BtCallRecord record;
        char text[220];
        char name[96];
        char number[64];
        lv_obj_t * item;

        if(!bt_app_get_call_record(i, s_call_record_filter, &record)) {
            continue;
        }
        copy_display_text(number, sizeof(number), record.number, "");
        if(display_text_is_missing(record.name) && record.number[0] != '\0') {
            copy_display_text(name, sizeof(name), record.number, "Unknown");
        } else {
            copy_display_text(name, sizeof(name), record.name, number);
        }
        lv_snprintf(text, sizeof(text), "%s%s  %s",
                    i == s_selected_call_record ? "> " : "",
                    call_record_type_text(record.type),
                    name);
        item = lv_list_add_button(s_call_records_list, NULL, text);
        style_list_item(item);
        lv_obj_add_event_cb(item, on_call_record_selected, LV_EVENT_CLICKED, (void *)(uintptr_t)i);
    }
}

void ui_Bluetooth_refresh(void)
{
    if(!ui_Bluetooth || !s_status_label) {
        return;
    }

    BtAppSnapshot snapshot;
    bt_app_get_snapshot(&snapshot);
    clamp_selected_indices(&snapshot);
    if(snapshot.status[0] != '\0') {
        lv_label_set_text(s_status_label, snapshot.status);
    } else if(!bt_app_is_ready()) {
        lv_label_set_text(s_status_label, "Bluetooth starting...");
    } else {
    lv_label_set_text(s_status_label, "");
    }
    if(s_pair_state_label) {
        lv_label_set_text(s_pair_state_label, "");
    }

    if(s_paired_device_list &&
       (s_rendered_device_version != snapshot.device_version ||
        s_rendered_selected_paired != s_selected_paired ||
        s_rendered_selected_available != s_selected_available)) {
        lv_obj_clean(s_paired_device_list);
        if(snapshot.paired_count == 0U) {
            /* Keep the list area empty when there is no paired device. */
        } else {
            for(size_t i = 0; i < snapshot.paired_count; ++i) {
                char text[160];
                char name_text[96];
                lv_obj_t * item;
                const char * name = device_display_name(snapshot.paired_devices[i], snapshot.paired_addresses[i]);
                bool connected = snapshot.phone_connected &&
                                 snapshot.connected_address[0] != '\0' &&
                                 snapshot.paired_addresses[i][0] != '\0' &&
                                 strcmp(snapshot.connected_address, snapshot.paired_addresses[i]) == 0;

                copy_display_text(name_text, sizeof(name_text), name, snapshot.paired_addresses[i]);
                lv_snprintf(text, sizeof(text), "%s%s%s", i == s_selected_paired ? "> " : "",
                            name_text, connected ? "  Connected" : "");
                item = lv_list_add_button(s_paired_device_list, NULL, text);
                style_list_item(item);
                lv_obj_set_style_bg_opa(item, s_selected_device_list == 0 && i == s_selected_paired ? 80 : 0,
                                        LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_add_event_cb(item, on_pair_list_clicked, LV_EVENT_CLICKED, (void *)(uintptr_t)i);
            }
        }

        if(s_available_device_list) {
            lv_obj_clean(s_available_device_list);
            if(snapshot.available_count == 0U) {
                /* Keep the list area empty before current scan results arrive. */
            } else {
                for(size_t i = 0; i < snapshot.available_count; ++i) {
                    char text[160];
                    char name_text[96];
                    lv_obj_t * item;
                    const char * name = device_display_name(snapshot.available_devices[i], snapshot.available_addresses[i]);

                    copy_display_text(name_text, sizeof(name_text), name, snapshot.available_addresses[i]);
                    lv_snprintf(text, sizeof(text), "%s%s", i == s_selected_available ? "> " : "",
                                name_text);
                    item = lv_list_add_button(s_available_device_list, NULL, text);
                    style_list_item(item);
                    lv_obj_set_style_bg_opa(item, s_selected_device_list == 1 && i == s_selected_available ? 80 : 0,
                                            LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_add_event_cb(item, on_available_list_clicked, LV_EVENT_CLICKED, (void *)(uintptr_t)i);
                }
            }
        }
        s_rendered_device_version = snapshot.device_version;
        s_rendered_selected_paired = s_selected_paired;
        s_rendered_selected_available = s_selected_available;
    }

    if(s_music_state_label) {
        char state_buffer[128];
        lv_snprintf(state_buffer, sizeof(state_buffer),
                    "A2DP: %s\nAVRCP: %s\nState: %s",
                    snapshot.a2dp_connected ? "Connected" : "Disconnected",
                    snapshot.avrcp_connected ? "Connected" : "Disconnected",
                    snapshot.music_playing_state ? "Playing" : "Paused");
        lv_label_set_text(s_music_state_label, state_buffer);
    }
    if(s_music_progress_label) {
        char progress_buffer[40];
        lv_snprintf(progress_buffer, sizeof(progress_buffer), "%s/%s",
                    snapshot.media_position[0] ? snapshot.media_position : "00:00",
                    snapshot.media_duration[0] ? snapshot.media_duration : "00:00");
        lv_label_set_text(s_music_progress_label, progress_buffer);
    }
    if(s_music_play_button_label) {
        lv_label_set_text(s_music_play_button_label, snapshot.music_playing_state ? "Pause" : "Play");
        lv_obj_center(s_music_play_button_label);
    }
    if(s_phonebook_download_button_label) {
        lv_label_set_text(s_phonebook_download_button_label,
                          is_pbap_downloading(&snapshot, BT_PBAP_DOWNLOAD_PHONEBOOK) ? "Stop" : "Download");
        lv_obj_center(s_phonebook_download_button_label);
    }
    if(s_call_records_download_button_label) {
        lv_label_set_text(s_call_records_download_button_label,
                          is_pbap_downloading(&snapshot, BT_PBAP_DOWNLOAD_CALL_RECORDS) ? "Stop" : "Download");
        lv_obj_center(s_call_records_download_button_label);
    }

    if(s_music_title_label) {
        char title[96];
        copy_display_text(title, sizeof(title), snapshot.media_title, "Unknown");
        lv_label_set_text(s_music_title_label, title);
    }
    if(s_music_artist_label) {
        char artist[96];
        copy_display_text(artist, sizeof(artist), snapshot.media_artist, "Unknown");
        lv_label_set_text(s_music_artist_label, artist);
    }
    if(s_music_album_label) {
        char album[96];
        copy_display_text(album, sizeof(album), snapshot.media_album, "Unknown");
        lv_label_set_text(s_music_album_label, album);
    }

    if(s_local_name_input && snapshot.local_name[0] && !lv_obj_has_state(s_local_name_input, LV_STATE_FOCUSED)) {
        char local_name[BT_LOCAL_NAME_TEXT_LEN];
        if(!copy_display_prefix(local_name, sizeof(local_name), snapshot.local_name)) {
            local_name[0] = '\0';
        }
        lv_textarea_set_text(s_local_name_input, local_name);
    }
    if(s_pin_input && snapshot.pairing_pin[0] && !lv_obj_has_state(s_pin_input, LV_STATE_FOCUSED)) {
        char pairing_pin[20];
        if(!copy_display_prefix(pairing_pin, sizeof(pairing_pin), snapshot.pairing_pin)) {
            pairing_pin[0] = '\0';
        }
        lv_textarea_set_text(s_pin_input, pairing_pin);
    }
    set_toggle_image(s_power_toggle, snapshot.power_on);
    set_toggle_image(s_auto_connect_toggle, snapshot.auto_connect);

    bool pbap_state_changed = s_rendered_pbap_state != snapshot.pbap_state;
    bool pbap_download_type_changed = s_rendered_pbap_download_type != snapshot.pbap_download_type;
    if(s_rendered_phonebook_version != snapshot.phonebook_version ||
       pbap_state_changed ||
       pbap_download_type_changed ||
       s_rendered_selected_contact != s_selected_contact) {
        refresh_phonebook_list(&snapshot);
        s_rendered_phonebook_version = snapshot.phonebook_version;
        s_rendered_selected_contact = s_selected_contact;
    }
    if(s_rendered_call_record_version != snapshot.call_record_version ||
       pbap_state_changed ||
       pbap_download_type_changed ||
       s_rendered_selected_call_record != s_selected_call_record ||
       s_rendered_call_record_filter != s_call_record_filter) {
        refresh_call_records_list(&snapshot);
        s_rendered_call_record_version = snapshot.call_record_version;
        s_rendered_selected_call_record = s_selected_call_record;
        s_rendered_call_record_filter = s_call_record_filter;
    }
    s_rendered_pbap_state = snapshot.pbap_state;
    s_rendered_pbap_download_type = snapshot.pbap_download_type;

    if(s_call_panel != NULL && s_call_number_label != NULL && snapshot.in_call) {
        char call_buffer[180];
        const char * contact_name = snapshot.active_contact_name[0] ? snapshot.active_contact_name : "Unknown";
        lv_obj_remove_flag(s_call_panel, LV_OBJ_FLAG_HIDDEN);
        if(snapshot.active_number[0]) {
            lv_snprintf(call_buffer, sizeof(call_buffer), "%s\n%s\nAudio: %s",
                        contact_name,
                        snapshot.active_number,
                        snapshot.call_audio_in_phone ? "Phone" : "Car");
        } else {
            lv_snprintf(call_buffer, sizeof(call_buffer), "%s\nAudio: %s",
                        contact_name,
                        snapshot.call_audio_in_phone ? "Phone" : "Car");
        }
        lv_label_set_text(s_call_number_label, call_buffer);
    } else if(s_call_panel != NULL) {
        lv_obj_add_flag(s_call_panel, LV_OBJ_FLAG_HIDDEN);
    }
}

void ui_Bluetooth_screen_load(void)
{
    /*
    if(ui_Bluetooth == NULL) {
        ui_Bluetooth_screen_init();
    }
    */
    lv_screen_load_anim(ui_Bluetooth, LV_SCR_LOAD_ANIM_FADE_ON, 250, 0, false);
    ui_Bluetooth_refresh();
}

void ui_Bluetooth_screen_init(lv_obj_t **screen)
{
    if(ui_Bluetooth != NULL) {
        return;
    }

    ui_Bluetooth = lv_obj_create(NULL);
    *screen = ui_Bluetooth;

    lv_obj_set_size(ui_Bluetooth, 1024, 600);
    lv_obj_remove_flag(ui_Bluetooth, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(ui_Bluetooth, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_Bluetooth, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_Bluetooth, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(ui_Bluetooth, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t * top_bar = create_panel(ui_Bluetooth, 0, 0, 1024, 64);
    lv_obj_set_style_bg_opa(top_bar, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(top_bar, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(top_bar, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    create_text_button(ui_Bluetooth, "Home", 18, 15, 68, 31, on_home_clicked);
    create_text_button(ui_Bluetooth, "Back", 944, 14, 68, 31, on_home_clicked);
    create_label(ui_Bluetooth, "Bluetooth", 420, 14, 240, 28);
    lv_obj_t * bottom_bar = create_panel(ui_Bluetooth, 0, 576, 1024, 24);
    lv_obj_set_style_bg_opa(bottom_bar, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(bottom_bar, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(bottom_bar, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    s_status_label = create_label(ui_Bluetooth, "", 260, 566, 700, 20);

    lv_obj_t * menu = lv_obj_create(ui_Bluetooth);
    lv_obj_remove_style_all(menu);
    lv_obj_set_pos(menu, 40, 72);
    lv_obj_set_size(menu, 202, 504);
    create_menu(menu);

    s_content = lv_obj_create(ui_Bluetooth);
    lv_obj_remove_style_all(s_content);
    lv_obj_set_pos(s_content, 252, 72);
    lv_obj_set_size(s_content, 732, 503);

    create_dial_page(s_content);
    create_phonebook_page(s_content);
    create_callrecord_page(s_content);
    create_music_page(s_content);
    create_pair_page(s_content);
    create_setting_page(s_content);
    create_call_overlay(ui_Bluetooth);
    create_setting_keyboard_overlay(ui_Bluetooth);

    set_page(BT_PAGE_PAIR);
    ui_Bluetooth_refresh();
}

void ui_Bluetooth_screen_destroy(void)
{
    bt_focus_adapter_set_bluetooth_music_page_selected(false);

    if(s_setting_keyboard) {
        lv_keyboard_set_textarea(s_setting_keyboard, NULL);
        s_setting_keyboard = NULL;
    }
    if(s_call_panel) {
        s_call_panel = NULL;
        s_call_number_label = NULL;
    }

    /* displayfocusmanager owns deleting the root screen and its children. */
    ui_Bluetooth = NULL;
    s_status_label = NULL;
    s_content = NULL;
    s_dial_label = NULL;
    s_call_panel = NULL;
    s_call_number_label = NULL;
    s_pair_state_label = NULL;
    s_pair_list_label = NULL;
    s_available_list_label = NULL;
    s_music_state_label = NULL;
    s_music_title_label = NULL;
    s_music_artist_label = NULL;
    s_music_album_label = NULL;
    s_phonebook_status_label = NULL;
    s_call_records_status_label = NULL;
    s_phonebook_search = NULL;
    s_local_name_input = NULL;
    s_pin_input = NULL;
    s_paired_device_list = NULL;
    s_available_device_list = NULL;
    s_power_toggle = NULL;
    s_auto_connect_toggle = NULL;
    s_music_progress_label = NULL;
    s_music_play_button = NULL;
    s_music_play_button_label = NULL;
    s_phonebook_download_button = NULL;
    s_phonebook_download_button_label = NULL;
    s_call_records_download_button = NULL;
    s_call_records_download_button_label = NULL;
    s_setting_keyboard = NULL;
    s_phonebook_list = NULL;
    s_call_records_list = NULL;
    s_rendered_device_version = UINT_MAX;
    s_rendered_phonebook_version = UINT_MAX;
    s_rendered_call_record_version = UINT_MAX;
    s_rendered_pbap_state = (BtPbapState)-1;
    s_rendered_pbap_download_type = (BtPbapDownloadType)-1;
    s_rendered_selected_paired = (size_t)-1;
    s_rendered_selected_available = (size_t)-1;
    s_rendered_selected_contact = (size_t)-1;
    s_rendered_selected_call_record = (size_t)-1;
    s_rendered_call_record_filter = (BtCallRecordType)-1;
    s_dial_number[0] = '\0';
    s_selected_paired = 0U;
    s_selected_available = 0U;
    s_selected_device_list = 0;
    s_selected_contact = 0U;
    s_selected_call_record = 0U;
    s_call_record_filter = BT_CALL_RECORD_ALL;
    for(int i = 0; i < BT_PAGE_COUNT; ++i) {
        s_menu_buttons[i] = NULL;
        s_pages[i] = NULL;
    }
}
