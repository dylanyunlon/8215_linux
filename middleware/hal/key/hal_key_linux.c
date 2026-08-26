/** 极简按键采集 HAL -- Linux iio sysfs 后端 */
#include "hal_key.h"

#include <stdio.h>

#define IIO_DEV_ROOT "/sys/bus/iio/devices"

static const char* s_dev = "iio:device0";
static hal_key_adc_read_fn s_board_fn = NULL;

/** 板级指定 iio 设备名（默认 "iio:device0"），如 "iio:device1"。
 * 仅本后端有效，BSP 可 extern 声明后调用。 */
void hal_key_adc_linux_set_device(const char* name) {
    if (name && name[0]) s_dev = name;
}

void hal_key_adc_register_board(hal_key_adc_read_fn fn) { s_board_fn = fn; }

int hal_key_adc_read(uint8_t channel) {
    if (s_board_fn) return s_board_fn(channel);

    char path[96];
    snprintf(path, sizeof(path), "%s/%s/in_voltage%u_raw", IIO_DEV_ROOT, s_dev,
             (unsigned)channel);
    FILE* f = fopen(path, "r");
    if (!f) return -1;
    int v = -1;
    int ok = (fscanf(f, "%d", &v) == 1);
    fclose(f);
    return ok ? v : -1;
}
