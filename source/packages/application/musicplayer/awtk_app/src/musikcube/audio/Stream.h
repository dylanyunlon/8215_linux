/* Stream.h — from musikcube (BSD-3), adapted for embedded build */
#pragma once

#include "../config.h"
#include "../io/DataStreamFactory.h"
#include "Buffer.h"
#include "IStream.h"
#include "../sdk/IDecoder.h"
#include "../sdk/IOutput.h"
#include "../sdk/IDSP.h"
#include "../sdk/constants.h"

#include <deque>
#include <list>
#include <vector>
#include <memory>

namespace musik { namespace core { namespace audio {

    class Stream : public IStream {
        using IDSP = musik::core::sdk::IDSP;
        using IDecoder = musik::core::sdk::IDecoder;
        using IBuffer = musik::core::sdk::IBuffer;
        using StreamFlags = musik::core::sdk::StreamFlags;

        public:
            static IStreamPtr Create(
                int samplesPerChannel = 2048,
                double bufferLengthSeconds = 5,
                StreamFlags options = StreamFlags::None);

        private:
            Stream(
                int samplesPerChannel,
                double bufferLengthSeconds,
                StreamFlags options);

        public:
            virtual ~Stream();

            IBuffer* GetNextProcessedOutputBuffer() override;
            void OnBufferProcessedByPlayer(IBuffer* buffer) override;
            double SetPosition(double seconds) override;
            double GetDuration() override;
            bool OpenStream(std::string uri, musik::core::sdk::IOutput* output) override;
            void Interrupt() override;
            int GetCapabilities() override;
            bool Eof() override { return this->done; }
            void Release() override { delete this; }

        private:
            bool GetNextBufferFromDecoder();
            Buffer* GetEmptyBuffer();
            void RefillInternalBuffers();

            typedef std::deque<Buffer*> BufferList;
            typedef std::shared_ptr<IDecoder> DecoderPtr;
            typedef std::shared_ptr<IDSP> DspPtr;
            typedef std::vector<DspPtr> Dsps;

            long decoderSampleRate;
            long decoderChannels;
            std::string uri;
            musik::core::io::DataStreamFactory::DataStreamPtr dataStream;

            BufferList recycledBuffers;
            BufferList filledBuffers;

            Buffer* decoderBuffer;
            long decoderSampleOffset;
            long decoderSamplesRemain;
            uint64_t decoderPosition;

            musik::core::sdk::StreamFlags options;
            int samplesPerChannel;
            long samplesPerBuffer;
            int bufferCount;
            bool done;
            double bufferLengthSeconds;
            int capabilities;

            float* rawBuffer;

            DecoderPtr decoder;
            Dsps dsps;
    };

} } }