#ifndef CARPLAYAPP_IF_H
#define CARPLAYAPP_IF_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct CarplayApp CarplayApp;

CarplayApp* carplayapp_get_instance();
void carplayapp_destroy(CarplayApp* app);
bool carplayapp_is_show_limit(CarplayApp* app);
void carplayapp_start_video(CarplayApp* app);
void carplayapp_release_surface(CarplayApp* app);
void carplayapp_request_ui(CarplayApp* app);
void carplayapp_on_ui_touched(CarplayApp* app, int press, int point_x, int point_y);
bool carplayapp_is_connected(CarplayApp* app);
bool carplayapp_id_is_connected(CarplayApp* app, const char *id);
bool carplayapp_id_is_connecting(CarplayApp* app, const char *id);
bool carplayapp_has_connected_call(CarplayApp* app);

void carplayapp_enable(CarplayApp* app);
void carplayapp_disable(CarplayApp* app);

void carplayapp_enable_discoverable(CarplayApp* app);
void carplayapp_disable_discoverable(CarplayApp* app);

void carplayapp_fresh(CarplayApp* app);

void carplayapp_connect(CarplayApp* app, const char* device_id);
void carplayapp_disconnect(CarplayApp* app, const char* device_id);
void carplayapp_forget(CarplayApp* app, const char* device_id);
void carplayapp_confirmFirstConnect(CarplayApp* app, const char* device_id);

void carplayapp_stop_video(CarplayApp* app);

bool carplayapp_is_enabled(CarplayApp* app);
bool carplayapp_is_discoverable(CarplayApp* app);

void carplayapp_update_list(CarplayApp* app);

void carplayapp_poll(void);

void carplayapp_wait_for_video_stopped(CarplayApp* app);

void carplayapp_signal_video_stopped(void);
void carplayapp_reset_video_stopped(void);

#ifdef __cplusplus
}
#endif

#endif /* CARPLAYAPP_H */

