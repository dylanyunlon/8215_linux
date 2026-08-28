/* IDebug.h — shim, not used in embedded build */
#pragma once
namespace musik { namespace core { namespace sdk {
    class IDebug {
    public:
        virtual ~IDebug() {}
        virtual void Verbose(const char* tag, const char* msg) = 0;
        virtual void Info(const char* tag, const char* msg) = 0;
        virtual void Warning(const char* tag, const char* msg) = 0;
        virtual void Error(const char* tag, const char* msg) = 0;
    };
} } }
