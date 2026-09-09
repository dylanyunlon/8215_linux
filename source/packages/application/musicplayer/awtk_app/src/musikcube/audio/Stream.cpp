/* Stream.cpp — from musikcube (BSD-3), adapted for embedded build.
 * Changes: replaced musikcore include paths, removed PluginFactory,
 * replaced musik::debug with fprintf. */

#include "../pch.hpp"
#include "Stream.h"
#include "Streams.h"

using namespace musik::core::audio;
using namespace musik::core::sdk;
using namespace musik::core::io;

static std::string TAG = "Stream";

#define MIN_BUFFER_COUNT 30

Stream::Stream(int samplesPerChannel, double bufferLengthSeconds, StreamFlags options)
: options(options)
, samplesPerChannel(samplesPerChannel)
, bufferLengthSeconds(bufferLengthSeconds)
, bufferCount(0)
, decoderSampleRate(0)
, decoderChannels(0)
, decoderPosition(0)
, decoderSampleOffset(0)
, decoderSamplesRemain(0)
, done(false)
, capabilities(0)
, rawBuffer(nullptr) {
    if (((int) this->options & (int) StreamFlags::NoDSP) == 0) {
        dsps = streams::GetDspPlugins();
    }

    this->decoderBuffer = new Buffer();
    this->decoderBuffer->SetSamples(0);
}

Stream::~Stream() {
    delete[] rawBuffer;
    delete this->decoderBuffer;

    for (Buffer* buffer : this->recycledBuffers) {
        delete buffer;
    }

    for (Buffer* buffer : this->filledBuffers) {
        delete buffer;
    }
}

IStreamPtr Stream::Create(int samplesPerChannel, double bufferLengthSeconds, StreamFlags options) {
    return IStreamPtr(new Stream(samplesPerChannel, bufferLengthSeconds, options));
}

double Stream::SetPosition(double requestedSeconds) {
    double actualSeconds = this->decoder->SetPosition(requestedSeconds);

    if (actualSeconds != -1) {
        double rate = (double) this->decoderSampleRate;

        this->decoderPosition =
            (uint64_t)(actualSeconds * rate) * this->decoderChannels;

        auto it = this->filledBuffers.begin();
        while (it != this->filledBuffers.end()) {
            this->recycledBuffers.push_back(*it);
            ++it;
        }

        this->filledBuffers.clear();
    }

    return actualSeconds;
}

double Stream::GetDuration() {
    return this->decoder ? this->decoder->GetDuration() : -1.0f;
}

int Stream::GetCapabilities() {
    return this->capabilities;
}

bool Stream::OpenStream(std::string uri, IOutput* output) {
    musik::debug::info(TAG, "opening " + uri);

    this->dataStream = DataStreamFactory::OpenSharedDataStream(uri.c_str(), OpenFlags::Read);

    if (!this->dataStream) {
        musik::debug::error(TAG, "failed to open " + uri);
        return false;
    }

    this->decoder = streams::GetDecoderForDataStream(this->dataStream);

    if (this->decoder) {
        if (output) {
            int defaultOutputSampleRate = output->GetDefaultSampleRate();
            if (defaultOutputSampleRate > 0) {
                this->decoder->SetPreferredSampleRate(defaultOutputSampleRate);
            }
        }
        if (this->dataStream->CanPrefetch()) {
            this->capabilities |= (int) musik::core::sdk::Capability::Prebuffer;
            this->RefillInternalBuffers();
        }
        return true;
    }

    return false;
}

void Stream::Interrupt() {
    if (this->dataStream) {
        this->dataStream->Interrupt();
    }
}

void Stream::OnBufferProcessedByPlayer(IBuffer* buffer) {
    this->recycledBuffers.push_back((Buffer*) buffer);
}

bool Stream::GetNextBufferFromDecoder() {
    if (!this->decoder->GetBuffer(this->decoderBuffer)) {
        return false;
    }

    if (!this->rawBuffer) {
        this->decoderSampleRate = this->decoderBuffer->SampleRate();
        this->decoderChannels = this->decoderBuffer->Channels();
        this->samplesPerBuffer = samplesPerChannel * decoderChannels;

        this->bufferCount = std::max(MIN_BUFFER_COUNT, (int)(this->bufferLengthSeconds *
            (double)(this->decoderSampleRate / this->samplesPerBuffer)));

        this->rawBuffer = new float[bufferCount * this->samplesPerBuffer];
        int offset = 0;
        for (int i = 0; i < bufferCount; i++) {
            auto buffer = new Buffer(this->rawBuffer + offset, this->samplesPerBuffer);
            buffer->SetSampleRate(this->decoderSampleRate);
            buffer->SetChannels(this->decoderChannels);
            this->recycledBuffers.push_back(buffer);
            offset += this->samplesPerBuffer;
        }
    }

    return true;
}

inline Buffer* Stream::GetEmptyBuffer() {
    if (recycledBuffers.size()) {
        Buffer* target = recycledBuffers.front();
        recycledBuffers.pop_front();
        return target;
    }
    return nullptr;
}

IBuffer* Stream::GetNextProcessedOutputBuffer() {
    this->RefillInternalBuffers();

    if (this->filledBuffers.size()) {
        Buffer* buffer = this->filledBuffers.front();
        this->filledBuffers.pop_front();

        for (std::shared_ptr<IDSP> dsp : this->dsps) {
            dsp->Process(buffer);
        }

        return buffer;
    }

    return nullptr;
}

void Stream::RefillInternalBuffers() {
    int recycled = (int) this->recycledBuffers.size();
    int count = 0;

    if (!this->rawBuffer) {
        count = -1;
    }
    else {
        count = std::min(recycled - 1, std::max(1, this->bufferCount / 4));
    }

    Buffer* target = nullptr;
    long targetSampleOffset = 0;
    long targetSamplesRemain = 0;

    while (!this->done && (count > 0 || count == -1)) {
        if (this->decoderSamplesRemain <= 0) {
            if (!GetNextBufferFromDecoder()) {
                if (target) {
                    target->SetSamples(targetSampleOffset);
                }
                this->done = true;
                break;
            }

            if (this->decoderBuffer->Samples() == 0) {
                continue;
            }

            this->decoderSamplesRemain = this->decoderBuffer->Samples();
            this->decoderSampleOffset = 0;
        }

        if (count < 0) {
            count = bufferCount / 4;
        }

        if (!target) {
            target = this->GetEmptyBuffer();

            if (!target) {
                break;
            }

            target->SetSamples(0);

            target->SetPosition(
                ((double) this->decoderPosition) /
                ((double) this->decoderChannels) /
                ((double) this->decoderSampleRate));

            filledBuffers.push_back(target);
        }

        targetSamplesRemain = this->samplesPerBuffer - targetSampleOffset;
        if (targetSamplesRemain > 0) {
            long samplesToCopy = std::min(this->decoderSamplesRemain, targetSamplesRemain);
            if (samplesToCopy > 0) {
                float* src = this->decoderBuffer->BufferPointer() + this->decoderSampleOffset;
                target->Copy(src, samplesToCopy, targetSampleOffset);

                this->decoderPosition += samplesToCopy;
                this->decoderSampleOffset += samplesToCopy;
                this->decoderSamplesRemain -= samplesToCopy;

                targetSampleOffset += samplesToCopy;

                if (targetSampleOffset == this->samplesPerBuffer) {
                    targetSampleOffset = 0;
                    target = nullptr;
                    --count;
                }
            }
        }
    }
}