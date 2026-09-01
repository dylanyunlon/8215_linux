/*
 * config.h — Shim replacing musikcore/config.h for embedded Linux.
 * Provides platform detection macros and base types.
 */
#pragma once

/* We're always on Linux ARM in this project */
#define HAVE_ALSA 1

/* musikcube uses these for export/import on Windows; no-op on Linux */
#define EXPORT
#define IMPORT

#include <cstdint>
#include <cstddef>
#include <cstring>
#include <string>
#include <memory>

/* From musikcube support/DeleteDefaults.h — delete copy/assign shortcuts */
#define DELETE_COPY_AND_ASSIGNMENT_DEFAULTS(ClassName) \
    ClassName(const ClassName&) = delete; \
    ClassName(const ClassName&&) = delete; \
    ClassName& operator=(const ClassName&) = delete; \
    ClassName& operator=(const ClassName&&) = delete;