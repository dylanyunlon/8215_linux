/* Streams.cpp — hardcoded decoder lookup for embedded (replaces PluginFactory) */
#include "../pch.hpp"
#include "Streams.h"
#include "../plugins/ffmpegdecoder/FfmpegDecoder.h"

static std::string TAG = "Streams";

using namespace musik::core::audio;
using namespace musik::core::sdk;

using DecoderPtr = std::shared_ptr<IDecoder>;

/* Release helper — calls IDecoder::Release() when shared_ptr drops */
struct DecoderDeleter {
    void operator()(IDecoder* d) { if (d) d->Release(); }
};

namespace musik { namespace core { namespace audio {

    namespace streams {

        IDecoder* GetDecoderForDataStream(IDataStream* dataStream) {
            if (!dataStream) return nullptr;

            /* We only have one decoder: FFmpeg. It handles everything. */
            IDecoder* decoder = new FfmpegDecoder();

            if (!decoder->Open(dataStream)) {
                musik::debug::error(TAG, "FFmpeg could not decode " + std::string(dataStream->Uri()));
                decoder->Release();
                return nullptr;
            }

            musik::debug::info(TAG, "opened " + std::string(dataStream->Uri()));
            return decoder;
        }

        DecoderPtr GetDecoderForDataStream(std::shared_ptr<IDataStream> dataStream) {
            auto decoder = GetDecoderForDataStream(dataStream.get());
            return decoder ? DecoderPtr(decoder, DecoderDeleter()) : DecoderPtr();
        }

        std::vector<std::shared_ptr<IDSP>> GetDspPlugins() {
            /* No DSP plugins on embedded */
            return {};
        }

    };

} } }