#ifndef DASHBOARD_BRIDGE_H
#define DASHBOARD_BRIDGE_H

#ifdef __cplusplus
extern "C" {
#endif

/* Connect the vehicle service subscription to the dashboard MVVM model.
 * The middleware callback only records pending values; poll() applies them
 * on the GUI thread. */
int  dashboard_bridge_init(void);
void dashboard_bridge_poll(void);
void dashboard_bridge_deinit(void);

#ifdef __cplusplus
}
#endif
#endif

