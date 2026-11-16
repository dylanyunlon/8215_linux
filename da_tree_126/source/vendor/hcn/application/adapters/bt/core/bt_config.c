#include "bt_core_internal.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct BtPersistedConfig {
    /* Preserve the existing binary config layout. */
    bool reserved;
    bool auto_connect;
    long long latest_address;
} BtPersistedConfig;

static void format_address_from_long(long long address, char *dst, size_t dst_size)
{
    const unsigned char *bytes;

    if (dst == NULL || dst_size == 0U) {
        return;
    }

    if (address == 0) {
        dst[0] = '\0';
        return;
    }

    bytes = (const unsigned char *)&address;
    (void)snprintf(dst, dst_size, "%02X:%02X:%02X:%02X:%02X:%02X",
                   bytes[5], bytes[4], bytes[3], bytes[2], bytes[1], bytes[0]);
}

static bool parse_address_to_long(const char *address, long long *out)
{
    unsigned char bytes[6];
    char *end = NULL;
    int i;

    if (out == NULL) {
        return false;
    }
    *out = 0;

    if (address == NULL || strlen(address) != 17U) {
        return false;
    }

    for (i = 0; i < 6; ++i) {
        char part[3];
        if (i < 5 && address[i * 3 + 2] != ':') {
            return false;
        }
        part[0] = address[i * 3];
        part[1] = address[i * 3 + 1];
        part[2] = '\0';
        end = NULL;
        bytes[5 - i] = (unsigned char)strtoul(part, &end, 16);
        if (end == NULL || *end != '\0') {
            *out = 0;
            return false;
        }
    }

    memcpy(out, bytes, sizeof(bytes));
    return true;
}

void save_config_locked(void)
{
    BtPersistedConfig config;
    FILE *file;

    memset(&config, 0, sizeof(config));
    config.auto_connect = s_state.auto_connect;
    (void)parse_address_to_long(s_state.latest_address, &config.latest_address);

    file = fopen(s_state.config_path, "wb");
    if (file == NULL) {
        return;
    }

    (void)fwrite(&config, sizeof(config), 1U, file);
    fclose(file);
}

void load_config_locked(void)
{
    BtPersistedConfig config;
    FILE *file;

    file = fopen(s_state.config_path, "rb");
    if (file == NULL) {
        return;
    }

    memset(&config, 0, sizeof(config));
    if (fread(&config, sizeof(config), 1U, file) == 1U) {
        s_state.auto_connect = config.auto_connect;
        format_address_from_long(config.latest_address, s_state.latest_address, sizeof(s_state.latest_address));
    }
    fclose(file);
}
