/** 极简 GPIO HAL -- Linux sysfs 后端 */
#include "hal_gpio.h"
#include "hal_bsp.h" /* hal_bsp_gpio_map_t */
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define GPIO_SYSFS_ROOT "/sys/class/gpio"

/** 板级映射表：逻辑名 -> Linux 全局 GPIO 号。由 BSP 在启动时注册
 *  （通常经 hal_bsp_register 聚合，也可直接调 gpio_linux_register_map）。 */
static const hal_bsp_gpio_map_t* s_map = NULL;
static int s_map_n = 0;

/** BSP 注册板级映射（在 osal_init / mw_init 之后、应用使用 gpio_get
 * 之前调用）*/
void gpio_linux_register_map(const hal_bsp_gpio_map_t* map, int n) {
    s_map = map;
    s_map_n = n;
}

static int map_lookup(const char* name) {
    int i;
    if (!s_map) return -1;
    for (i = 0; i < s_map_n; i++)
        if (s_map[i].name && strcmp(s_map[i].name, name) == 0)
            return s_map[i].gpio;
    return -1;
}

static int wint(const char* path, int v) {
    FILE* f = fopen(path, "w");
    if (!f) return -1;
    fprintf(f, "%d", v);
    fclose(f);
    return 0;
}

static int wstr(const char* path, const char* s) {
    FILE* f = fopen(path, "w");
    if (!f) return -1;
    fprintf(f, "%s", s);
    fclose(f);
    return 0;
}

static int gpio_rint(const char* path, int* v) {
    FILE* f = fopen(path, "r");
    if (!f) return -1;
    int r = (fscanf(f, "%d", v) == 1) ? 0 : -1;
    fclose(f);
    return r;
}

gpio_t gpio_get(const char* name) {
    if (!name) return GPIO_INVALID;
    int g = map_lookup(name);
    if (g < 0) return GPIO_INVALID;

    /* export（已 export 时内核返回 EBUSY，忽略）*/
    char path[64];
    snprintf(path, sizeof(path), "%s/export", GPIO_SYSFS_ROOT);
    wint(path, g);
    return g;
}

int gpio_set_dir(gpio_t pin, gpio_dir_t dir) {
    if (pin < 0) return -1;
    char path[64];
    snprintf(path, sizeof(path), "%s/gpio%d/direction", GPIO_SYSFS_ROOT, pin);
    return wstr(path, dir == GPIO_DIR_OUTPUT ? "out" : "in");
}

int gpio_read(gpio_t pin) {
    if (pin < 0) return -1;
    char path[64];
    int v;
    snprintf(path, sizeof(path), "%s/gpio%d/value", GPIO_SYSFS_ROOT, pin);
    return gpio_rint(path, &v) ? -1 : v;
}

int gpio_write(gpio_t pin, int value) {
    if (pin < 0) return -1;
    char path[64];
    snprintf(path, sizeof(path), "%s/gpio%d/value", GPIO_SYSFS_ROOT, pin);
    return wint(path, value ? 1 : 0);
}

int gpio_toggle(gpio_t pin) {
    int v = gpio_read(pin);
    if (v < 0) return -1;
    return gpio_write(pin, !v);
}

int gpio_set_pull(gpio_t pin, gpio_pull_t pull) {
    /* sysfs 不支持上下拉配置；板级需通过 pinctrl/dts 预设 */
    (void)pin;
    (void)pull;
    return 0;
}
