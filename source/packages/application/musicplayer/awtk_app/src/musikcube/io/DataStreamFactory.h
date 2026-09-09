/* DataStreamFactory.h — simplified for embedded (no plugin discovery) */
#pragma once

#include "../config.h"
#include "../sdk/IDataStream.h"
#include <memory>

namespace musik { namespace core { namespace io {

    class DataStreamFactory {
        public:
            using DataStreamPtr = std::shared_ptr<musik::core::sdk::IDataStream>;
            using OpenFlags = musik::core::sdk::OpenFlags;

            static DataStreamPtr OpenSharedDataStream(const char* uri, OpenFlags flags);

        private:
            DataStreamFactory() = default;
    };

} } }