/**
 * @file videofocusmanager.h
 * @brief Compatibility facade for displayfocusmanager.
 *
 * Existing adapters use the historical videofocusmanager names. New page
 * navigation code uses displayfocusmanager directly.
 */

#ifndef VIDEOFOCUSMANAGER_H
#define VIDEOFOCUSMANAGER_H

#include "displayfocusmanager.h"

#define videofocusmanager_init                     displayfocusmanager_init
#define videofocusmanager_deinit                   displayfocusmanager_deinit
#define videofocusmanager_register_app             displayfocusmanager_register_app
#define videofocusmanager_set_app_priority         displayfocusmanager_set_app_priority
#define videofocusmanager_set_screen_policy        displayfocusmanager_set_screen_policy
#define videofocusmanager_open_app                 displayfocusmanager_open_app
#define videofocusmanager_close_current_app        displayfocusmanager_close_current_app
#define videofocusmanager_close_app                displayfocusmanager_close_app
#define videofocusmanager_go_home                  displayfocusmanager_go_home
#define videofocusmanager_get_current_app          displayfocusmanager_get_current_app
#define videofocusmanager_get_stack_size           displayfocusmanager_get_stack_size
#define videofocusmanager_is_in_stack              displayfocusmanager_is_in_stack
#define videofocusmanager_register_focus_callback  displayfocusmanager_register_focus_callback
#define videofocusmanager_unregister_focus_callback displayfocusmanager_unregister_focus_callback
#define videofocusmanager_dump                     displayfocusmanager_dump

#endif /* VIDEOFOCUSMANAGER_H */
