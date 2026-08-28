/*
 * LocalFileStream.h — Simplified IDataStream for local file access.
 * Derived from musikcube's LocalFileStream (BSD-3-Clause).
 */
#pragma once

#include "../sdk/IDataStream.h"
#include <cstdio>
#include <string>

class LocalFileStream : public musik::core::sdk::IDataStream {
public:
    LocalFileStream();
    ~LocalFileStream() override;

    bool Open(const char* uri, musik::core::sdk::OpenFlags flags) override;
    bool Close() override;
    void Interrupt() override {}
    void Release() override { delete this; }
    bool Readable() override { return true; }
    bool Writable() override { return false; }
    musik::core::sdk::PositionType Read(void* buffer, musik::core::sdk::PositionType readBytes) override;
    musik::core::sdk::PositionType Write(void* buffer, musik::core::sdk::PositionType writeBytes) override { return 0; }
    bool SetPosition(musik::core::sdk::PositionType position) override;
    musik::core::sdk::PositionType Position() override;
    bool Seekable() override { return true; }
    bool Eof() override;
    long Length() override;
    const char* Type() override;
    const char* Uri() override;
    bool CanPrefetch() override { return true; }

private:
    FILE* file;
    std::string uri;
    std::string extension;
    long filesize;
};
