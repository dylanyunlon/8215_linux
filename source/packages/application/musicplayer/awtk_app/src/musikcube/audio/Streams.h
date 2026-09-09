/* Streams.h — decoder/DSP lookup (hardcoded for embedded, no PluginFactory) */
#pragma once

#include "../config.h"
#include "../sdk/IDecoder.h"
#include "../sdk/IDataStream.h"
#include "../sdk/IDSP.h"

#include <memory>
#include <vector>

namespace musik { namespace core { namespace audio {

    namespace streams {
        /* Returns a new FfmpegDecoder opened on the given data stream, or nullptr */
        std::shared_ptr<musik::core::sdk::IDecoder>
            GetDecoderForDataStream(std::shared_ptr<musik::core::sdk::IDataStream> dataStream);

        musik::core::sdk::IDecoder*
            GetDecoderForDataStream(musik::core::sdk::IDataStream* stream);

        /* No DSP plugins on embedded — returns empty vector */
        std::vector<std::shared_ptr<musik::core::sdk::IDSP>> GetDspPlugins();
    };

} } }