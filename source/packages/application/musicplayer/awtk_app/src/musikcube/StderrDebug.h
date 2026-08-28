/*
 * StderrDebug.h — Minimal IDebug implementation that writes to stderr.
 * Replaces musikcube's PluginFactory-provided debug backend.
 */
#pragma once

#include "sdk/IDebug.h"
#include <cstdio>

class StderrDebug : public musik::core::sdk::IDebug {
public:
    void Verbose(const char* tag, const char* msg) override {
        /* too noisy for embedded, skip */
    }
    void Info(const char* tag, const char* msg) override {
        fprintf(stderr, "[%s] %s\n", tag, msg);
    }
    void Warning(const char* tag, const char* msg) override {
        fprintf(stderr, "[%s] WARN: %s\n", tag, msg);
    }
    void Error(const char* tag, const char* msg) override {
        fprintf(stderr, "[%s] ERROR: %s\n", tag, msg);
    }
};
