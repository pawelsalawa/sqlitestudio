#ifndef CRASHHANDLER_H
#define CRASHHANDLER_H

#ifdef _WIN32

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include <new.h>
#include <signal.h>
#include <exception>
#include <sys/stat.h>
#include <psapi.h>
#include <rtcapi.h>
#include <Shellapi.h>
#include <dbghelp.h>

#include <functional>
#include <mutex>
#include <string>
#include <map>

#include "../common/common.h"

#ifdef __MINGW32__
    typedef int (__cdecl *_CRT_REPORT_HOOK)(int,char *,int *);
#endif

namespace Debug {
    struct ThreadExceptionHandlers
    {
        ThreadExceptionHandlers()
        {
            m_prevTerm = NULL;
            m_prevUnexp = NULL;
            m_prevSigFPE = NULL;
            m_prevSigILL = NULL;
            m_prevSigSEGV = NULL;
        }

        std::terminate_handler m_prevTerm;        // Previous terminate handler
        std::unexpected_handler m_prevUnexp;      // Previous unexpected handler
        void (__cdecl *m_prevSigFPE)(int);   // Previous FPE handler
        void (__cdecl *m_prevSigILL)(int);   // Previous SIGILL handler
        void (__cdecl *m_prevSigSEGV)(int);  // Previous illegal storage access handler
    };

    // code mostly from https://www.codeproject.com/articles/207464/WebControls/
    class WindowsCrashHandler
    {
    public:
        static WindowsCrashHandler& getInstance()
        {
            static WindowsCrashHandler instance; // Guaranteed to be destroyed.
            // Instantiated on first use.
            return instance;
        }

    private:
        WindowsCrashHandler();

    public:
        void setup(const std::wstring &appName, const std::wstring &dumpsDir);
        void teardown();
        void setCrashCallback(const std::function<void()> &callback);
        void setBacktraceCallback(const std::function<void(const char * const)> &callback);
        void handleCrash();

    public:
        bool isDataSectionNeeded(const WCHAR* pModuleName);

    public:
        // Sets exception handlers that work on per-process basis
        void setProcessExceptionHandlers();
        void unsetProcessExceptionHandlers();

        // Installs C++ exception handlers that function on per-thread basis
        int setThreadExceptionHandlers();
        int unsetThreadExceptionHandlers();

        /* Exception handler functions. */

        static int __cdecl CrtReportHook(int nReportType, char* szMsg, int* pnRet);

        static void __cdecl TerminateHandler();
        static void __cdecl UnexpectedHandler();

        static void __cdecl PureCallHandler();
#if _MSC_VER>=1300 && _MSC_VER<1400
        // Buffer overrun handler (deprecated in newest versions of Visual C++).
        // Since CRT 8.0, you can't intercept the buffer overrun errors in your code. When a buffer overrun is detected, CRT invokes Dr. Watson directly
        static void __cdecl SecurityHandler(int code, void *x);
#endif

#if _MSC_VER>=1400
        static void __cdecl InvalidParameterHandler(const wchar_t* expression,
                                                    const wchar_t* function, const wchar_t* file, unsigned int line, uintptr_t pReserved);
#endif

        static int __cdecl NewHandler(size_t);

        static void SigabrtHandler(int);
        static void SigfpeHandler(int /*code*/, int subcode);
        static void SigintHandler(int);
        static void SigillHandler(int);
        static void SigsegvHandler(int);
        static void SigtermHandler(int);
        static int ConsoleCtrlHandler(DWORD type);

        private:
        std::function<void()> m_crashCallback;
        std::function<void(const char * const)> m_backtraceCallback;
        std::mutex m_crashMutex;
        std::wstring m_appName;

        std::map<DWORD, ThreadExceptionHandlers> m_threadExceptionHandlers;
        std::mutex m_threadHandlersMutex;

        // Previous SEH exception filter.
        LPTOP_LEVEL_EXCEPTION_FILTER  m_oldSehHandler;

#if _MSC_VER>=1300
        _purecall_handler m_prevPurec;   // Previous pure virtual call exception filter.
        _PNH m_prevNewHandler; // Previous new operator exception filter.
#endif

#if _MSC_VER>=1400
        _invalid_parameter_handler m_prevInvpar; // Previous invalid parameter exception filter.
#endif

#if _MSC_VER>=1300 && _MSC_VER<1400
        _secerr_handler_func m_prevSec; // Previous security exception filter.
#endif

        void (__cdecl *m_prevSigABRT)(int); // Previous SIGABRT handler.
        void (__cdecl *m_prevSigINT)(int);  // Previous SIGINT handler.
        void (__cdecl *m_prevSigTERM)(int); // Previous SIGTERM handler.
    };
}

#endif

#endif // CRASHHANDLER_H

