#ifndef COMMON_H
#define COMMON_H

#include <ctime>
#include <ostream>

#define CHILLOUT_DATETIME "%Y%m%d_%H%M%S"

namespace Debug {
    tm now();
#ifdef _WIN32
    std::wostream& formatDateTime(std::wostream& out, const tm& t, const wchar_t *fmt);
#else
    std::ostream& formatDateTime(std::ostream& out, const tm& t, const char* fmt);
#endif

    enum CrashDumpSize {
        CrashDumpSmall,
        CrashDumpNormal,
        CrashDumpFull
    };
}

#endif // COMMON_H
