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

#include "Streams.h"
#include "../plugins/ffmpegdecoder/FfmpegDecoder.h"

#include <cstdio>
#include <mutex>

/* EMBEDDED ADAPTATION:
 * Original musikcube uses PluginFactory to discover IDecoderFactory plugins
 * at runtime via dlopen. On AC83xx embedded Linux we have exactly one
 * decoder (FFmpeg), so we hardcode it here.
 *
 * GetDspPlugins() returns empty — no DSP chain on embedded.
 * GetEncoderForType() removed entirely — no transcoding needed. */

using namespace musik::core::audio;
using namespace musik::core::sdk;

using DataStreamPtr = musik::core::io::DataStreamFactory::DataStreamPtr;

/* Simple release-deleter for shared_ptr<IDecoder> */
struct DecoderDeleter {
    void operator()(IDecoder* d) { if (d) d->Release(); }
};

using DecoderPtr = std::shared_ptr<IDecoder>;

namespace musik { namespace core { namespace audio {

    namespace streams {

        IDecoder* GetDecoderForDataStream(IDataStream* dataStream) {
            if (!dataStream) {
                return nullptr;
            }

            /* FFmpeg handles virtually all audio formats.
             * Original code would iterate IDecoderFactory plugins and
             * call CanHandle(dataStream->Type()); FFmpeg can handle
             * everything we care about on embedded, so skip the check. */
            IDecoder* decoder = new FfmpegDecoder();

            if (!decoder->Open(dataStream)) {
                fprintf(stderr, "[Streams] FfmpegDecoder failed to open: %s\n",
                        dataStream->Uri());
                decoder->Release();
                return nullptr;
            }

            fprintf(stderr, "[Streams] decoder opened: %s\n", dataStream->Uri());
            return decoder;
        }

        DecoderPtr GetDecoderForDataStream(DataStreamPtr dataStream) {
            auto decoder = GetDecoderForDataStream(dataStream.get());
            return decoder ? DecoderPtr(decoder, DecoderDeleter()) : DecoderPtr();
        }

        std::vector<std::shared_ptr<IDSP>> GetDspPlugins() {
            /* No DSP plugins on embedded */
            return std::vector<std::shared_ptr<IDSP>>();
        }

    };

} } }