/* IPreferences.h — shim for embedded build */
#pragma once
namespace musik { namespace core { namespace sdk {
    class IPreferences {
    public:
        virtual ~IPreferences() {}
        virtual bool GetBool(const char* key, bool defaultValue = false) = 0;
        virtual int GetInt(const char* key, int defaultValue = 0) = 0;
        virtual double GetDouble(const char* key, double defaultValue = 0.0) = 0;
        virtual int GetString(const char* key, char* dst, size_t size, const char* defaultValue = "") = 0;
        virtual void SetBool(const char* key, bool value) = 0;
        virtual void SetInt(const char* key, int value) = 0;
        virtual void SetDouble(const char* key, double value) = 0;
        virtual void SetString(const char* key, const char* value) = 0;
        virtual void Save() = 0;
        virtual void Release() = 0;
    };
} } }
