/* IStream.h — from musikcube (BSD-3), adapted for embedded build */
#pragma once

#include "../config.h"
#include "../sdk/IBuffer.h"
#include "../sdk/IDecoder.h"
#include "../sdk/IDSP.h"
#include "../sdk/IDecoderFactory.h"
#include "../sdk/IOutput.h"

#include <list>
#include <memory>

namespace musik { namespace core { namespace audio {

    class IStream {
        public:
            virtual musik::core::sdk::IBuffer* GetNextProcessedOutputBuffer() = 0;
            virtual void OnBufferProcessedByPlayer(musik::core::sdk::IBuffer* buffer) = 0;
            virtual double SetPosition(double seconds) = 0;
            virtual double GetDuration() = 0;
            virtual bool OpenStream(std::string uri, musik::core::sdk::IOutput* output) = 0;
            virtual void Interrupt() = 0;
            virtual int GetCapabilities() = 0;
            virtual bool Eof() = 0;
            virtual void Release() = 0;
    };

    typedef std::shared_ptr<IStream> IStreamPtr;

} } }