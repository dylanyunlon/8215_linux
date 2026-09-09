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

/* From musikcore/support/DeleteDefaults.h */
#define DELETE_COPY_AND_ASSIGNMENT_DEFAULTS(ClassName) \
    ClassName(const ClassName&) = delete; \
    ClassName(const ClassName&&) = delete; \
    ClassName& operator=(const ClassName&) = delete; \
    ClassName& operator=(const ClassName&&) = delete;

#include <cstdint>
#include <cstddef>
#include <cstring>
#include <string>
#include <memory>
#include <cstdio>

/* Replace musikcore/debug.h — just fprintf to stderr */
namespace musik { namespace debug {
    static inline void info(const std::string& tag, const std::string& msg) {
        fprintf(stderr, "[%s] %s\n", tag.c_str(), msg.c_str());
    }
    static inline void error(const std::string& tag, const std::string& msg) {
        fprintf(stderr, "[%s] ERROR: %s\n", tag.c_str(), msg.c_str());
    }
} }