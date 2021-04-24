#ifndef CHILLOUT_H
#define CHILLOUT_H

#include <functional>
#include <string>
#include <atomic>
#include "common/common.h"

namespace Debug {
    class Chillout {
    public:
        static Chillout& getInstance()
        {
            static Chillout instance; // Guaranteed to be destroyed.
            // Instantiated on first use.
            return instance;
        }

    public:
#ifdef _WIN32
        typedef std::wstring string_t;
#else
        typedef std::string string_t;
#endif

    public:
        void init(const string_t &appName, const string_t &pathToDumpsDir);
        void deinit();
        void setBacktraceCallback(const std::function<void(const char * const)> &callback);
        void setCrashCallback(const std::function<void()> &callback);

    private:
        Chillout(): m_InitCounter(0) {}
        Chillout(Chillout const&);
        void operator=(Chillout const&);

    private:
        std::atomic_int m_InitCounter;
    };
}

#endif // CHILLOUT_H
