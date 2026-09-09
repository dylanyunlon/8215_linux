/* DataStreamFactory.cpp — always LocalFileStream for embedded */
#include "../pch.hpp"
#include "DataStreamFactory.h"
#include "LocalFileStream.h"

using namespace musik::core::io;
using namespace musik::core::sdk;

using DataStreamPtr = DataStreamFactory::DataStreamPtr;

/* Custom deleter that calls IDataStream::Release() */
struct StreamDeleter {
    void operator()(IDataStream* s) { if (s) s->Release(); }
};

DataStreamPtr DataStreamFactory::OpenSharedDataStream(const char* uri, OpenFlags flags) {
    if (!uri) return DataStreamPtr();

    IDataStream* stream = new LocalFileStream();
    if (stream->Open(uri, flags)) {
        return DataStreamPtr(stream, StreamDeleter());
    }

    stream->Release();
    return DataStreamPtr();
}