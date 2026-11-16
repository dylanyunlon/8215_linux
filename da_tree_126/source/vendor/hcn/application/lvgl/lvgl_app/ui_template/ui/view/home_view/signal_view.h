#ifndef HOME_SIGNAL_VIEW_H
#define HOME_SIGNAL_VIEW_H
#include "ui_objects.h"
#include "vehicle_signal.h"   /* signal_e / gms_level_e：跨层契约类型，见 ui/common/ */
#include <stdbool.h>

void home_signal_view_init();

void home_refresh_signal(signal_e icon , bool visible) ;

void home_refresh_GMS_level(gms_level_e level );

void home_refresh_signal_visible(bool visible );

#endif