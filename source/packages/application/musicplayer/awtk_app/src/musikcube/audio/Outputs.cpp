/* Outputs.cpp — hardcoded ALSA output for embedded (replaces PluginFactory) */
#include "../pch.hpp"
#include "Outputs.h"
#include "../plugins/alsaout/AlsaOut.h"

namespace musik { namespace core { namespace audio { namespace outputs {

    /* AlsaOut doesn't have Release() that deletes,
       so we use a custom deleter */
    struct OutputDeleter {
        void operator()(IOutput* o) { if (o) o->Release(); }
    };

    std::shared_ptr<IOutput> SelectedOutput() {
        static std::shared_ptr<IOutput> instance(new AlsaOut(), OutputDeleter());
        return instance;
    }

} } } }