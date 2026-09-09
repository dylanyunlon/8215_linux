/* Outputs.h — output device selection (hardcoded ALSA for embedded) */
#pragma once

#include "../config.h"
#include "../sdk/IOutput.h"
#include <memory>

namespace musik { namespace core { namespace audio { namespace outputs {

    using IOutput = musik::core::sdk::IOutput;

    std::shared_ptr<IOutput> SelectedOutput();

} } } }