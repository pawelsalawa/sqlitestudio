#include "chillout.h"

#ifdef _WIN32
#include "windows/windowscrashhandler.h"
#else
#include "posix/posixcrashhandler.h"
#endif

namespace Debug {
    void Chillout::init(const string_t &appName, const string_t &pathToDumpsDir) {
        if (0 == m_InitCounter.fetch_add(1)) {
#ifdef _WIN32
            WindowsCrashHandler &handler = WindowsCrashHandler::getInstance();
#else
            PosixCrashHandler &handler = PosixCrashHandler::getInstance();
#endif
            handler.setup(appName, pathToDumpsDir);
        }
    }

    void Chillout::deinit() {
#ifdef _WIN32
        WindowsCrashHandler &handler = WindowsCrashHandler::getInstance();
#else
        PosixCrashHandler &handler = PosixCrashHandler::getInstance();
#endif
        handler.teardown();
    }

    void Chillout::setBacktraceCallback(const std::function<void(const char * const)> &callback) {
#ifdef _WIN32
        WindowsCrashHandler &handler = WindowsCrashHandler::getInstance();
#else
        PosixCrashHandler &handler = PosixCrashHandler::getInstance();
#endif
        handler.setBacktraceCallback(callback);
    }

    void Chillout::setCrashCallback(const std::function<void()> &callback) {
#ifdef _WIN32
        WindowsCrashHandler &handler = WindowsCrashHandler::getInstance();
#else
        PosixCrashHandler &handler = PosixCrashHandler::getInstance();
#endif
        handler.setCrashCallback(callback);
    }
}
