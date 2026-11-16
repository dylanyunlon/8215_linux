#include "dashboard_bridge.h"

#ifdef HCN_APP_FULL

#include <stdbool.h>
#include <stdint.h>

#include "vehicle_services.h"
#include "MVVM/model/dashboard_model.h"

enum {
    PENDING_SPEED   = 1u << 0,
    PENDING_RPM     = 1u << 1,
    PENDING_GEAR    = 1u << 2,
    PENDING_POWER   = 1u << 3,
    PENDING_BATTERY = 1u << 4,
};

static bool     s_inited = false;
static volatile uint32_t s_pending = 0;
static volatile int32_t  s_speed = 0;
static volatile int32_t  s_rpm = 0;
static volatile int32_t  s_gear_raw = 0;
static volatile int32_t  s_power = 0;
static volatile int32_t  s_battery = 0;

static dashboard_gear_e map_gear(int32_t raw) {
    switch (raw) {
    case 0:  return DASHBOARD_GEAR_NEUTRAL;
    case 1:  return DASHBOARD_GEAR_DRIVE;
    case 2:  return DASHBOARD_GEAR_REVERSE;
    default: return DASHBOARD_GEAR_INVALID;
    }
}

/* Middleware thread. Store first, then publish the pending bit. */
static void on_vehicle_signal(vs_signal_e id, int32_t value) {
    uint32_t bit = 0;

    switch (id) {
    case VS_SIGNAL_SPEED:
        __atomic_store_n(&s_speed, value, __ATOMIC_RELAXED);
        bit = PENDING_SPEED;
        break;
    case VS_SIGNAL_RPM:
        __atomic_store_n(&s_rpm, value, __ATOMIC_RELAXED);
        bit = PENDING_RPM;
        break;
    case VS_SIGNAL_GEAR:
        __atomic_store_n(&s_gear_raw, value, __ATOMIC_RELAXED);
        bit = PENDING_GEAR;
        break;
    case VS_SIGNAL_POWER:
        __atomic_store_n(&s_power, value, __ATOMIC_RELAXED);
        bit = PENDING_POWER;
        break;
    case VS_SIGNAL_BATTERY:
        __atomic_store_n(&s_battery, value, __ATOMIC_RELAXED);
        bit = PENDING_BATTERY;
        break;
    default:
        break;
    }

    if (bit != 0) {
        __atomic_fetch_or(&s_pending, bit, __ATOMIC_RELEASE);
    }
}

static void publish_snapshot(void) {
    int32_t speed = __atomic_load_n(&s_speed, __ATOMIC_RELAXED);
    int32_t rpm = __atomic_load_n(&s_rpm, __ATOMIC_RELAXED);
    int32_t power = __atomic_load_n(&s_power, __ATOMIC_RELAXED);
    int32_t battery = __atomic_load_n(&s_battery, __ATOMIC_RELAXED);
    int32_t gear_raw = __atomic_load_n(&s_gear_raw, __ATOMIC_RELAXED);
    dashboard_gear_e gear = map_gear(gear_raw);

    dashboard_model_set(DASHBOARD_FIELD_SPEED, &speed);
    dashboard_model_set(DASHBOARD_FIELD_RPM, &rpm);
    dashboard_model_set(DASHBOARD_FIELD_GEAR, &gear);
    dashboard_model_set(DASHBOARD_FIELD_POWER, &power);
    dashboard_model_set(DASHBOARD_FIELD_BATTERY, &battery);
}

int dashboard_bridge_init(void) {
    if (s_inited) return 0;

    dashboard_model_init();

    /* Seed values already present before the subscription was installed. */
    __atomic_store_n(&s_speed, vs_signal_get(VS_SIGNAL_SPEED), __ATOMIC_RELAXED);
    __atomic_store_n(&s_rpm, vs_signal_get(VS_SIGNAL_RPM), __ATOMIC_RELAXED);
    __atomic_store_n(&s_gear_raw, vs_signal_get(VS_SIGNAL_GEAR), __ATOMIC_RELAXED);
    __atomic_store_n(&s_power, vs_signal_get(VS_SIGNAL_POWER), __ATOMIC_RELAXED);
    __atomic_store_n(&s_battery, vs_signal_get(VS_SIGNAL_BATTERY), __ATOMIC_RELAXED);
    __atomic_store_n(&s_pending, 0u, __ATOMIC_RELAXED);

    if (vs_subscribe(on_vehicle_signal) != 0) {
        return -1;
    }

    s_inited = true;
    publish_snapshot();
    return 0;
}

void dashboard_bridge_poll(void) {
    if (!s_inited) return;

    uint32_t pending = __atomic_exchange_n(&s_pending, 0u, __ATOMIC_ACQUIRE);
    if (pending == 0) return;

    if (pending & PENDING_SPEED) {
        int32_t v = __atomic_load_n(&s_speed, __ATOMIC_RELAXED);
        dashboard_model_set(DASHBOARD_FIELD_SPEED, &v);
    }
    if (pending & PENDING_RPM) {
        int32_t v = __atomic_load_n(&s_rpm, __ATOMIC_RELAXED);
        dashboard_model_set(DASHBOARD_FIELD_RPM, &v);
    }
    if (pending & PENDING_GEAR) {
        int32_t raw = __atomic_load_n(&s_gear_raw, __ATOMIC_RELAXED);
        dashboard_gear_e gear = map_gear(raw);
        dashboard_model_set(DASHBOARD_FIELD_GEAR, &gear);
    }
    if (pending & PENDING_POWER) {
        int32_t v = __atomic_load_n(&s_power, __ATOMIC_RELAXED);
        dashboard_model_set(DASHBOARD_FIELD_POWER, &v);
    }
    if (pending & PENDING_BATTERY) {
        int32_t v = __atomic_load_n(&s_battery, __ATOMIC_RELAXED);
        dashboard_model_set(DASHBOARD_FIELD_BATTERY, &v);
    }
}

void dashboard_bridge_deinit(void) {
    if (!s_inited) return;

    vs_unsubscribe();
    __atomic_store_n(&s_pending, 0u, __ATOMIC_RELAXED);
    s_inited = false;
}

#endif /* HCN_APP_FULL */

