#ifdef _WIN32

#include "windowscrashhandler.h"
#include "../defines.h"

#include <tchar.h>
#include <stdio.h>
#include "StackWalker.h"

// minidump
#include <windows.h>
#include <TlHelp32.h>
//#include <tchar.h>
#include <dbghelp.h>
//#include <stdio.h>
#include <crtdbg.h>
#include <signal.h>

#include <locale>
#include <codecvt>
#include <ctime>
#include <sstream>
#include "../common/common.h"

#ifdef _MSC_VER
#if _MSC_VER < 1400
#define strcpy_s(dst, len, src) strcpy(dst, src)
#define strncpy_s(dst, len, src, maxLen) strncpy(dst, len, src)
#define strcat_s(dst, len, src) strcat(dst, src)
#define _snprintf_s _snprintf
#define _tcscat_s _tcscat
#endif
#endif

#ifdef __MINGW32__
#define strcpy_s(dst, len, src) strcpy(dst, src)
#define strncpy_s(dst, len, src, maxLen) strncpy(dst, src, len)
#define strcat_s(dst, len, src) strcat(dst, src)
#define _snprintf_s _snprintf
#endif

#define CR_SEH_EXCEPTION                0    //!< SEH exception.
#define CR_CPP_TERMINATE_CALL           1    //!< C++ terminate() call.
#define CR_CPP_UNEXPECTED_CALL          2    //!< C++ unexpected() call.
#define CR_CPP_PURE_CALL                3    //!< C++ pure virtual function call (VS .NET and later).
#define CR_CPP_NEW_OPERATOR_ERROR       4    //!< C++ new operator fault (VS .NET and later).
#define CR_CPP_SECURITY_ERROR           5    //!< Buffer overrun error (VS .NET only).
#define CR_CPP_INVALID_PARAMETER        6    //!< Invalid parameter exception (VS 2005 and later).
#define CR_CPP_SIGABRT                  7    //!< C++ SIGABRT signal (abort).
#define CR_CPP_SIGFPE                   8    //!< C++ SIGFPE signal (flotating point exception).
#define CR_CPP_SIGILL                   9    //!< C++ SIGILL signal (illegal instruction).
#define CR_CPP_SIGINT                   10   //!< C++ SIGINT signal (CTRL+C).
#define CR_CPP_SIGSEGV                  11   //!< C++ SIGSEGV signal (invalid storage access).
#define CR_CPP_SIGTERM                  12   //!< C++ SIGTERM signal (termination request).

#define STRINGIZE_(x) #x
#define STRINGIZE(x) STRINGIZE_(x)

#ifndef _AddressOfReturnAddress
    // Taken from: http://msdn.microsoft.com/en-us/library/s975zw7k(VS.71).aspx
    #ifdef __cplusplus
        #define EXTERNC extern "C"
    #else
        #define EXTERNC
    #endif

    // _ReturnAddress and _AddressOfReturnAddress should be prototyped before use
    EXTERNC void * _AddressOfReturnAddress(void);
    EXTERNC void * _ReturnAddress(void);
#endif

#pragma GCC diagnostic ignored "-Wcast-function-type"

namespace Debug {
    class StackWalkerWithCallback : public StackWalker
    {
    public:
        StackWalkerWithCallback(const std::function<void(const char * const)> &callback):
            StackWalker(RetrieveVerbose | SymBuildPath),
            m_callback(callback)
        { }

    protected:
        virtual void OnOutput(LPCSTR szText) override {
            m_callback(szText);
        }

    private:
        std::function<void(const char * const)> m_callback;
    };

    BOOL PreventSetUnhandledExceptionFilter()
    {
        HMODULE hKernel32 = LoadLibrary(_T("kernel32.dll"));
        if (hKernel32 == NULL) return FALSE;
        void* pOrgEntry = reinterpret_cast<void*>(GetProcAddress(hKernel32, "SetUnhandledExceptionFilter"));
        if (pOrgEntry == NULL) return FALSE;

#ifdef _M_IX86
        // Code for x86:
        // 33 C0                xor         eax,eax
        // C2 04 00             ret         4
        unsigned char szExecute[] = { 0x33, 0xC0, 0xC2, 0x04, 0x00 };
#elif _M_X64
        // 33 C0                xor         eax,eax
        // C3                   ret
        unsigned char szExecute[] = { 0x33, 0xC0, 0xC3 };
#elif _M_ARM64
        // 20 00 80 D2          mov         x0,#1
        // C0 03 5F D6          ret
        unsigned char szExecute[] = { 0x00, 0x00, 0x80, 0xD2, 0xC0, 0x03, 0x5F, 0xD6 };        
#else
#error "The following code only works for x86 and x64!"
#endif

        SIZE_T bytesWritten = 0;
        BOOL bRet = WriteProcessMemory(GetCurrentProcess(),
                                       pOrgEntry, szExecute, sizeof(szExecute), &bytesWritten);
        return bRet;
    }

    BOOL CALLBACK MyMiniDumpCallback(
            PVOID                            pParam,
            const PMINIDUMP_CALLBACK_INPUT   pInput,
            PMINIDUMP_CALLBACK_OUTPUT        pOutput
            )
    {
        BOOL bRet = FALSE;

        // Check parameters

        if( pInput == 0 )
            return FALSE;

        if( pOutput == 0 )
            return FALSE;

        // Process the callbacks
        WindowsCrashHandler *handler = (WindowsCrashHandler*)pParam;

        switch( pInput->CallbackType )
        {
        case IncludeModuleCallback:
        {
            // Include the module into the dump
            bRet = TRUE;
        }
            break;

        case IncludeThreadCallback:
        {
            // Include the thread into the dump
            bRet = TRUE;
        }
            break;

        case ModuleCallback:
        {
            // Are data sections available for this module ?
            if( pOutput->ModuleWriteFlags & ModuleWriteDataSeg )
            {
                // Yes, they are, but do we need them?

                if( !handler->isDataSectionNeeded( pInput->Module.FullPath ) )
                {
                    pOutput->ModuleWriteFlags &= (~ModuleWriteDataSeg);
                }
            }

            if( !(pOutput->ModuleWriteFlags & ModuleReferencedByMemory) )
            {
                // No, it does not - exclude it
                pOutput->ModuleWriteFlags &= (~ModuleWriteModule);
            }

            bRet = TRUE;
        }
            break;

        case ThreadCallback:
        {
            // Include all thread information into the minidump
            bRet = TRUE;
        }
            break;

        case ThreadExCallback:
        {
            // Include this information
            bRet = TRUE;
        }
            break;

        case MemoryCallback:
        {
            // We do not include any information here -> return FALSE
            bRet = FALSE;
        }
            break;

        case CancelCallback:
            break;
        }

        return bRet;

    }

    void DoHandleCrash() {
        WindowsCrashHandler &handler = WindowsCrashHandler::getInstance();
        handler.handleCrash();
    }

    // http://groups.google.com/group/crashrpt/browse_thread/thread/a1dbcc56acb58b27/fbd0151dd8e26daf?lnk=gst&q=stack+overflow#fbd0151dd8e26daf
    // Thread procedure doing the dump for stack overflow.
    DWORD WINAPI StackOverflowThreadFunction(LPVOID) {
        DoHandleCrash();
        TerminateProcess(GetCurrentProcess(), CHILLOUT_EXIT_CODE);
        return 0;
    }

    static LONG WINAPI SehHandler(EXCEPTION_POINTERS*) {
#ifdef _DEBUG
        fprintf(stderr, "Chillout SehHandler");
#endif

        DoHandleCrash();

        TerminateProcess(GetCurrentProcess(), CHILLOUT_EXIT_CODE);

        // unreachable
        return EXCEPTION_EXECUTE_HANDLER;
    }

    // The following code is intended to fix the issue with 32-bit applications in 64-bit environment.
    // http://support.microsoft.com/kb/976038/en-us
    // http://code.google.com/p/crashrpt/issues/detail?id=104
    void EnableCrashingOnCrashes() {
        typedef BOOL (WINAPI *tGetPolicy)(LPDWORD lpFlags);
        typedef BOOL (WINAPI *tSetPolicy)(DWORD dwFlags);
        static const DWORD EXCEPTION_SWALLOWING = 0x1;

        const HMODULE kernel32 = LoadLibraryA("kernel32.dll");
        const tGetPolicy pGetPolicy = reinterpret_cast<tGetPolicy>(GetProcAddress(kernel32, "GetProcessUserModeExceptionPolicy"));
        const tSetPolicy pSetPolicy = reinterpret_cast<tSetPolicy>(GetProcAddress(kernel32, "SetProcessUserModeExceptionPolicy"));
        if(pGetPolicy && pSetPolicy)
        {
            DWORD dwFlags;
            if(pGetPolicy(&dwFlags))
            {
                // Turn off the filter
                pSetPolicy(dwFlags & ~EXCEPTION_SWALLOWING);
            }
        }
    }

    WindowsCrashHandler::WindowsCrashHandler() {
        m_oldSehHandler = NULL;

#if _MSC_VER>=1300
        m_prevPurec = NULL;
        m_prevNewHandler = NULL;
#endif

#if _MSC_VER>=1300 && _MSC_VER<1400    
        m_prevSec = NULL;
#endif

#if _MSC_VER>=1400
        m_prevInvpar = NULL;
#endif  

        m_prevSigABRT = NULL;
        m_prevSigINT = NULL;
        m_prevSigTERM = NULL;
    }

    void WindowsCrashHandler::setup(const std::wstring &appName, const std::wstring &dumpsDir) {
        m_appName = appName;

        if (!appName.empty() && !dumpsDir.empty()) {
            std::wstring path = dumpsDir;
            while ((path.size() > 1) &&
                   (path[path.size() - 1] == L'\\')) {
                path.erase(path.size() - 1);
            }
        }

        EnableCrashingOnCrashes();
        setProcessExceptionHandlers();
        setThreadExceptionHandlers();
    }

    void WindowsCrashHandler::teardown() {
        unsetProcessExceptionHandlers();
        unsetThreadExceptionHandlers();
    }

    void WindowsCrashHandler::setCrashCallback(const std::function<void()> &callback) {
        m_crashCallback = callback;
    }

    void WindowsCrashHandler::setBacktraceCallback(const std::function<void(const char * const)> &callback) {
        m_backtraceCallback = callback;
    }

    void WindowsCrashHandler::handleCrash() {
        std::lock_guard<std::mutex> guard(m_crashMutex);
        (void)guard;

        if (m_crashCallback) {
            m_crashCallback();
        }

        // Terminate process
        TerminateProcess(GetCurrentProcess(), CHILLOUT_EXIT_CODE);
    }

    bool WindowsCrashHandler::isDataSectionNeeded(const WCHAR* pModuleName) {
        if( pModuleName == 0 ) {
            _ASSERTE( _T("Parameter is null.") );
            return false;
        }

        // Extract the module name

        WCHAR szFileName[_MAX_FNAME] = L"";
        _wsplitpath( pModuleName, NULL, NULL, szFileName, NULL );

        // Compare the name with the list of known names and decide
        // if contains app name in its path
        if( wcsstr( pModuleName, m_appName.c_str() ) != 0 ) {
            return true;
        } else if( _wcsicmp( szFileName, L"ntdll" ) == 0 ) {
            return true;
        } else if( wcsstr( szFileName, L"Qt5" ) != 0 ) {
            return true;
        }

        // Complete
        return false;
    }

    void WindowsCrashHandler::setProcessExceptionHandlers() {
        //SetErrorMode(SEM_NOGPFAULTERRORBOX | SEM_FAILCRITICALERRORS);
        m_oldSehHandler = SetUnhandledExceptionFilter(SehHandler);
#if defined _M_X64 || defined _M_IX86
        if (m_oldSehHandler)
            PreventSetUnhandledExceptionFilter();
#endif

#if _MSC_VER>=1300
        // Catch pure virtual function calls.
        // Because there is one _purecall_handler for the whole process,
        // calling this function immediately impacts all threads. The last
        // caller on any thread sets the handler.
        // http://msdn.microsoft.com/en-us/library/t296ys27.aspx
        m_prevPurec = _set_purecall_handler(PureCallHandler);

        // Catch new operator memory allocation exceptions
        //_set_new_mode(1); // Force malloc() to call new handler too
        m_prevNewHandler = _set_new_handler(NewHandler);
#endif

#if _MSC_VER>=1400
        // Catch invalid parameter exceptions.
        m_prevInvpar = _set_invalid_parameter_handler(InvalidParameterHandler);
#endif

#if _MSC_VER>=1300 && _MSC_VER<1400    
        // Catch buffer overrun exceptions
        // The _set_security_error_handler is deprecated in VC8 C++ run time library
        m_prevSec = _set_security_error_handler(SecurityHandler);
#endif

#if _MSC_VER>=1400
        _set_abort_behavior(0, _WRITE_ABORT_MSG);
        _set_abort_behavior(_CALL_REPORTFAULT, _CALL_REPORTFAULT);
#endif

#if defined(_MSC_VER)
        // Disable all of the possible ways Windows conspires to make automated
        // testing impossible.

        // ::SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX);
        // ::_set_error_mode(_OUT_TO_STDERR);

        // _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE | _CRTDBG_MODE_DEBUG);
        // _CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDERR);
        // _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_FILE | _CRTDBG_MODE_DEBUG);
        // _CrtSetReportFile(_CRT_ERROR, _CRTDBG_FILE_STDERR);
        // _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE | _CRTDBG_MODE_DEBUG);
        // _CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDERR);

        m_crtReportHook = _CrtSetReportHook(CrtReportHook);
#endif

        // Set up C++ signal handlers
        SetConsoleCtrlHandler(ConsoleCtrlHandler, true);

        // Catch an abnormal program termination
        m_prevSigABRT = signal(SIGABRT, SigabrtHandler);

        // Catch illegal instruction handler
        m_prevSigINT = signal(SIGINT, SigintHandler);

        // Catch a termination request
        m_prevSigINT = signal(SIGTERM, SigtermHandler);
    }

    void WindowsCrashHandler::unsetProcessExceptionHandlers() {
#if _MSC_VER>=1300
        if(m_prevPurec!=NULL) {
            _set_purecall_handler(m_prevPurec);
        }

        if(m_prevNewHandler!=NULL) {
            _set_new_handler(m_prevNewHandler);
        }
#endif

#if _MSC_VER>=1400
        if(m_prevInvpar!=NULL) {
            _set_invalid_parameter_handler(m_prevInvpar);
        }
#endif //_MSC_VER>=1400  

#if _MSC_VER>=1300 && _MSC_VER<1400    
        if(m_prevSec!=NULL)
            _set_security_error_handler(m_prevSec);
#endif //_MSC_VER<1400

        if(m_prevSigABRT!=NULL) {
            signal(SIGABRT, m_prevSigABRT);
        }

        if(m_prevSigINT!=NULL) {
            signal(SIGINT, m_prevSigINT);
        }

        if(m_prevSigTERM!=NULL) {
            signal(SIGTERM, m_prevSigTERM);
        }

        // Reset SEH exception filter
        if (m_oldSehHandler) {
            SetUnhandledExceptionFilter(m_oldSehHandler);
        }

        m_oldSehHandler = NULL;
    }

    int WindowsCrashHandler::setThreadExceptionHandlers()
    {
        DWORD dwThreadId = GetCurrentThreadId();

        std::lock_guard<std::mutex> guard(m_threadHandlersMutex);

        auto it = m_threadExceptionHandlers.find(dwThreadId);
        if (it != m_threadExceptionHandlers.end()) {
            // handlers are already set for the thread
            return 1; // failed
        }

        ThreadExceptionHandlers handlers;

        // Catch terminate() calls.
        // In a multithreaded environment, terminate functions are maintained
        // separately for each thread. Each new thread needs to install its own
        // terminate function. Thus, each thread is in charge of its own termination handling.
        // http://msdn.microsoft.com/en-us/library/t6fk7h29.aspx
        handlers.m_prevTerm = std::set_terminate(TerminateHandler);

        // Catch unexpected() calls.
        // In a multithreaded environment, unexpected functions are maintained
        // separately for each thread. Each new thread needs to install its own
        // unexpected function. Thus, each thread is in charge of its own unexpected handling.
        // http://msdn.microsoft.com/en-us/library/h46t5b69.aspx
        handlers.m_prevUnexp = std::set_unexpected(UnexpectedHandler);

        // Catch a floating point error
        typedef void (*sigh)(int);
        handlers.m_prevSigFPE = signal(SIGFPE, (sigh)SigfpeHandler);

        // Catch an illegal instruction
        handlers.m_prevSigILL = signal(SIGILL, SigillHandler);

        // Catch illegal storage access errors
        handlers.m_prevSigSEGV = signal(SIGSEGV, SigsegvHandler);

        m_threadExceptionHandlers[dwThreadId] = handlers;

        return 0;
    }

    int WindowsCrashHandler::unsetThreadExceptionHandlers() {
        DWORD dwThreadId = GetCurrentThreadId();
        std::lock_guard<std::mutex> guard(m_threadHandlersMutex);
        (void)guard;

        auto it = m_threadExceptionHandlers.find(dwThreadId);
        if (it == m_threadExceptionHandlers.end()) {
            return 1;
        }

        ThreadExceptionHandlers* handlers = &(it->second);

        if(handlers->m_prevTerm!=NULL) {
            std::set_terminate(handlers->m_prevTerm);
        }

        if(handlers->m_prevUnexp!=NULL) {
            std::set_unexpected(handlers->m_prevUnexp);
        }

        if(handlers->m_prevSigFPE!=NULL) {
            signal(SIGFPE, handlers->m_prevSigFPE);
        }

        if(handlers->m_prevSigILL!=NULL) {
            signal(SIGINT, handlers->m_prevSigILL);
        }

        if(handlers->m_prevSigSEGV!=NULL) {
            signal(SIGSEGV, handlers->m_prevSigSEGV);
        }

        // Remove from the list
        m_threadExceptionHandlers.erase(it);

        return 0;
    }

    int __cdecl WindowsCrashHandler::CrtReportHook(int nReportType, char* szMsg, int* pnRet) {
        (void)szMsg;

        switch (nReportType) {
        case _CRT_WARN:
        case _CRT_ERROR:
        case _CRT_ASSERT:
            // Put some debug code here
            break;
        }

        if (pnRet) {
            *pnRet = 0;
        }

        return TRUE;
    }

    // CRT terminate() call handler
    void __cdecl WindowsCrashHandler::TerminateHandler() {
        // Abnormal program termination (terminate() function was called)
        DoHandleCrash();
    }

    // CRT unexpected() call handler
    void __cdecl WindowsCrashHandler::UnexpectedHandler() {
        // Unexpected error (unexpected() function was called)
        DoHandleCrash();
    }

    // CRT Pure virtual method call handler
    void __cdecl WindowsCrashHandler::PureCallHandler() {
        // Pure virtual function call
        DoHandleCrash();
    }

    // CRT buffer overrun handler. Available in CRT 7.1 only
#if _MSC_VER>=1300 && _MSC_VER<1400
    void __cdecl WindowsCrashHandler::SecurityHandler(int code, void *x)
    {
        // Security error (buffer overrun).

        code;
        x;

        EXCEPTION_POINTERS* pExceptionPtrs = (PEXCEPTION_POINTERS)_pxcptinfoptrs;
        if (pExceptionPtrs == nullptr) {
            GetExceptionPointers(CR_CPP_SECURITY_ERROR, &pExceptionPtrs);
        }

        DoHandleCrash(pExceptionPtrs);
    }
#endif

#if _MSC_VER>=1400
    // CRT invalid parameter handler
    void __cdecl WindowsCrashHandler::InvalidParameterHandler(
            const wchar_t* expression,
            const wchar_t* function,
            const wchar_t* file,
            unsigned int line,
            uintptr_t pReserved) {
        pReserved;
        expression;
        function;
        file;
        line;

        // fwprintf(stderr, L"Invalid parameter detected in function %s."
        //        L" File: %s Line: %d\n", function, file, line);

        // Retrieve exception information
        EXCEPTION_POINTERS* pExceptionPtrs = NULL;
        GetExceptionPointers(CR_CPP_INVALID_PARAMETER, &pExceptionPtrs);

        DoHandleCrash(pExceptionPtrs);
    }
#endif

    // CRT new operator fault handler
    int __cdecl WindowsCrashHandler::NewHandler(size_t) {
        // 'new' operator memory allocation exception
        DoHandleCrash();

        // Unreacheable code
        return 0;
    }

    // CRT SIGABRT signal handler
    void WindowsCrashHandler::SigabrtHandler(int) {
        // Caught SIGABRT C++ signal
        DoHandleCrash();
    }

    // CRT SIGFPE signal handler
    void WindowsCrashHandler::SigfpeHandler(int /*code*/, int subcode) {
        // Floating point exception (SIGFPE)
        (void)subcode;
        DoHandleCrash();
    }

    // CRT sigill signal handler
    void WindowsCrashHandler::SigillHandler(int) {
        // Illegal instruction (SIGILL)
        DoHandleCrash();
    }

    // CRT sigint signal handler
    void WindowsCrashHandler::SigintHandler(int) {
        // Interruption (SIGINT)
        DoHandleCrash();
    }

    // CRT SIGSEGV signal handler
    void WindowsCrashHandler::SigsegvHandler(int) {
        // Invalid storage access (SIGSEGV)
        DoHandleCrash();
    }

    // CRT SIGTERM signal handler
    void WindowsCrashHandler::SigtermHandler(int) {
        // Termination request (SIGTERM)
        DoHandleCrash();
    }

    // CRT SIGTERM signal handler
    BOOL WINAPI WindowsCrashHandler::ConsoleCtrlHandler(DWORD type) {
        // Termination request (CTRL_C_EVENT)
        switch (type)
        {
            case CTRL_C_EVENT:
            case CTRL_LOGOFF_EVENT:
            case CTRL_SHUTDOWN_EVENT:
            case CTRL_CLOSE_EVENT:
                DoHandleCrash();
                break;
        }
        return false;
    }
}

#endif
