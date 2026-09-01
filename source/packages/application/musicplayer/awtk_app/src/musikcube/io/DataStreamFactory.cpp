//////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2004-2023 musikcube team
//
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//    * Redistributions of source code must retain the above copyright notice,
//      this list of conditions and the following disclaimer.
//
//    * Redistributions in binary form must reproduce the above copyright
//      notice, this list of conditions and the following disclaimer in the
//      documentation and/or other materials provided with the distribution.
//
//    * Neither the name of the author nor the names of other contributors may
//      be used to endorse or promote products derived from this software
//      without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
//////////////////////////////////////////////////////////////////////////////

#include "../pch.hpp"

#include "DataStreamFactory.h"
#include "../config.h"
#include "LocalFileStream.h"

/* EMBEDDED ADAPTATION:
 * Original musikcube uses PluginFactory to dynamically discover
 * IDataStreamFactory plugins (HTTP streams, etc.) via dlopen.
 *
 * On AC83xx embedded Linux we only play local files from USB/SD,
 * so we hardcode LocalFileStream and skip the plugin machinery.
 * The public API (OpenDataStream / OpenSharedDataStream) stays
 * identical so Stream.cpp doesn't need to change. */

using namespace musik::core::io;
using namespace musik::core::sdk;

using DataStreamPtr = DataStreamFactory::DataStreamPtr;

/* Release-deleter: calls IDataStream::Release() when shared_ptr expires */
struct StreamDeleter {
    void operator()(IDataStream* s) { if (s) s->Release(); }
};

DataStreamFactory::DataStreamFactory() {
    /* No plugin discovery needed — we only have LocalFileStream */
}

DataStreamFactory* DataStreamFactory::Instance() {
    static DataStreamFactory* instance = nullptr;
    if (!instance) {
        instance = new DataStreamFactory();
    }
    return instance;
}

IDataStream* DataStreamFactory::OpenDataStream(const char* uri, OpenFlags flags) {
    if (!uri) {
        return nullptr;
    }

    /* On embedded, every URI is a local file path.
     * Original code would iterate plugin factories first;
     * we go straight to LocalFileStream. */
    IDataStream* file = new LocalFileStream();
    if (file->Open(uri, flags)) {
        return file;
    }

    file->Release();
    return nullptr;
}

DataStreamPtr DataStreamFactory::OpenSharedDataStream(const char *uri, OpenFlags flags) {
    auto stream = OpenDataStream(uri, flags);
    return stream ? DataStreamPtr(stream, StreamDeleter()) : DataStreamPtr();
}