#ifndef POSIXCRASHHANDLER_H
#define POSIXCRASHHANDLER_H

#ifndef _WIN32

#include <functional>
#include <string>

namespace Debug {
    class PosixCrashHandler {
    public:
        static PosixCrashHandler& getInstance()
        {
            static PosixCrashHandler instance; // Guaranteed to be destroyed.
            // Instantiated on first use.
            return instance;
        }

    private:
        PosixCrashHandler();
        ~PosixCrashHandler();

    public:
        void setup(const std::string &appName, const std::string &crashDirPath);
        void teardown();
        void handleCrash();
        void setCrashCallback(const std::function<void()> &callback);
        void setBacktraceCallback(const std::function<void(const char * const)> &callback);

    private:
        void walkStackTrace(char *memory, size_t memorySize, int maxFrames=128);
        char *dlDemangle(void *addr, char *symbol, int frameIndex, char *stackMemory);

    private:
        std::function<void()> m_crashCallback;
        std::function<void(const char * const)> m_backtraceCallback;
        char *m_stackMemory;
        char *m_demangleMemory;
        std::string m_backtraceFilePath;
    };
}

#endif // _WIN32

#endif // POSIXCRASHHANDLER_H
