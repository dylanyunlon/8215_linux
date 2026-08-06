#ifndef METAZONE_DEFAULT_TABLE_H
#define METAZONE_DEFAULT_TABLE_H

/*
 * Generated from the values that are clearly readable in the provided sheet image.
 * Dword format requested by user: index + pointer to values.
 * For single-value dword entries, values points to a one-element array.
 * Binary entries use index + size + data.
 */

typedef struct _MetaZoneDworddata {
    uint32_t index;
    const uint32_t *values;
} MetaZoneDworddata;

typedef struct _MetaZoneBinarydata {
    uint32_t index;
    uint32_t size;
    const uint8_t *data;
} MetaZoneBinarydata;

typedef struct _MetaZoneReserveddata {
    uint32_t offset;
    uint32_t size;
    const uint8_t *data;
} MetaZoneReserveddata;

static const uint8_t metazone_backup_header_defaults[40] = {
    0x00, 0x00, 0x01, 0x00, 0x01, 0xEF, 0xCD, 0xAB, 0xE0, 0xE0, 0x00, 0x00,
    0xDC, 0x7F, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0xE8, 0x03, 0x00, 0x00,
    0xA0, 0x8F, 0x00, 0x00, 0xC8, 0x00, 0x00, 0x00, 0x64, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00
};

static const uint32_t g_mtz_dword_100D1[] = { 0x110u };
static const uint32_t g_mtz_dword_10035[] = { 0x20u };
static const uint32_t g_mtz_dword_10036[] = { 0x20u };
static const uint32_t g_mtz_dword_10037[] = { 0x20u };
static const uint32_t g_mtz_dword_10038[] = { 0x20u };
static const uint32_t g_mtz_dword_10039[] = { 0x20u };
static const uint32_t g_mtz_dword_10040[] = { 0x0u };
static const uint32_t g_mtz_dword_10041[] = { 0x0u };
static const uint32_t g_mtz_dword_10042[] = { 0x8u };

static const MetaZoneDworddata g_metazone_dword_defaults[] = {
    { 0x100D1u, g_mtz_dword_100D1 },
    { 0x10035u, g_mtz_dword_10035 },
    { 0x10036u, g_mtz_dword_10036 },
    { 0x10037u, g_mtz_dword_10037 },
    { 0x10038u, g_mtz_dword_10038 },
    { 0x10039u, g_mtz_dword_10039 },
    { 0x10040u, g_mtz_dword_10040 },
    { 0x10041u, g_mtz_dword_10041 },
    { 0x10042u, g_mtz_dword_10042 },
};

#define METAZONE_DWORD_DEFAULT_COUNT \
    ((uint32_t)(sizeof(g_metazone_dword_defaults) / sizeof(g_metazone_dword_defaults[0])))

static const uint8_t g_mtz_binary_1000C[] = {
    0x00, 0x04, 0x08, 0x0C, 0x10, 0x14, 0x18, 0x1C,
    0x20, 0x24, 0x28, 0x2D, 0x31, 0x35, 0x39, 0x3D,
    0x41, 0x45, 0x49, 0x4D, 0x51, 0x55, 0x59, 0x5D,
    0x61, 0x65, 0x69, 0x6D, 0x71, 0x75, 0x79, 0x7D,
    0x82, 0x86, 0x8A, 0x8E, 0x92, 0x96, 0x9A, 0x9E,
    0xA2, 0xA6, 0xAA, 0xAE, 0xB2, 0xB6, 0xBA, 0xBE,
    0xC2, 0xC6, 0xCA, 0xCE, 0xD2, 0xD7, 0xDB, 0xDF,
    0xE3, 0xE7, 0xEB, 0xEF, 0xF3, 0xF7, 0xFB, 0xFF
};

static const MetaZoneBinarydata g_metazone_binary_defaults[] = {
    { 0x1000Cu, 64, g_mtz_binary_1000C },
};
    

#define METAZONE_BINARY_DEFAULT_COUNT \
    ((uint32_t)(sizeof(g_metazone_binary_defaults) / sizeof(g_metazone_binary_defaults[0])))

static const MetaZoneReserveddata g_metazone_reserved_defaults[] = {
};

#define METAZONE_RESERVED_DEFAULT_COUNT \
    ((uint32_t)(sizeof(g_metazone_reserved_defaults) / sizeof(g_metazone_reserved_defaults[0])))

#endif /* METAZONE_DEFAULT_TABLE_H */
