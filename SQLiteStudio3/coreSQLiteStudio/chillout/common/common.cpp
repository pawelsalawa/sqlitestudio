#include "common.h"
#include <ctime>
#include <locale>
#include <cstring>

namespace Debug {
    tm now() {
        time_t now = time(0);
        return *localtime(&now);
    }

#ifdef _WIN32
    std::wostream& formatDateTime(std::wostream& out, const tm& t, const wchar_t* fmt) {
        const std::time_put<wchar_t>& dateWriter = std::use_facet<std::time_put<wchar_t> >(out.getloc());
        const size_t n = wcslen(fmt);
#else
    std::ostream& formatDateTime(std::ostream& out, const tm& t, const char* fmt) {
        const std::time_put<char>& dateWriter = std::use_facet<std::time_put<char> >(out.getloc());
        const size_t n = strlen(fmt);
#endif
        dateWriter.put(out, out, ' ', &t, fmt, fmt + n);
        return out;
    }
}
