/*
 * LocalFileStream.cpp — Simple IDataStream backed by fopen/fread.
 * Stripped-down version for embedded Linux (no HTTP, no archive support).
 */

#include "LocalFileStream.h"
#include <cstring>

LocalFileStream::LocalFileStream()
: file(nullptr)
, filesize(-1) {
}

LocalFileStream::~LocalFileStream() {
    Close();
}

bool LocalFileStream::Open(const char* uri, musik::core::sdk::OpenFlags flags) {
    Close();
    if (!uri || !uri[0]) return false;

    this->uri = uri;

    /* extract extension for Type() */
    const char* dot = strrchr(uri, '.');
    this->extension = dot ? (dot + 1) : "";

    this->file = fopen(uri, "rb");
    if (!this->file) return false;

    /* get file size */
    fseek(this->file, 0, SEEK_END);
    this->filesize = ftell(this->file);
    fseek(this->file, 0, SEEK_SET);

    return true;
}

bool LocalFileStream::Close() {
    if (this->file) {
        fclose(this->file);
        this->file = nullptr;
    }
    return true;
}

musik::core::sdk::PositionType LocalFileStream::Read(
    void* buffer, musik::core::sdk::PositionType readBytes)
{
    if (!this->file) return 0;
    return (musik::core::sdk::PositionType)fread(buffer, 1, (size_t)readBytes, this->file);
}

bool LocalFileStream::SetPosition(musik::core::sdk::PositionType position) {
    if (!this->file) return false;
    return fseek(this->file, (long)position, SEEK_SET) == 0;
}

musik::core::sdk::PositionType LocalFileStream::Position() {
    if (!this->file) return 0;
    return (musik::core::sdk::PositionType)ftell(this->file);
}

bool LocalFileStream::Eof() {
    if (!this->file) return true;
    return feof(this->file) != 0;
}

long LocalFileStream::Length() {
    return this->filesize;
}

const char* LocalFileStream::Type() {
    return this->extension.c_str();
}

const char* LocalFileStream::Uri() {
    return this->uri.c_str();
}
