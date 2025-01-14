#include "posixcrashhandler.h"

#ifndef _WIN32

#include <errno.h>
#include <dlfcn.h>
#include <cxxabi.h>
#include <cstdio>
#include <cstdlib>
#include <signal.h>
#include <cstring>
#include <cstdint>
#include <sstream>
#include <memory>

#include "../defines.h"
#include "../common/common.h"

#define KILOBYTE 1024
#define DEMANGLE_MEMORY_SIZE (10*(KILOBYTE))
#define STACK_MEMORY_SIZE (90*(KILOBYTE))

namespace Debug {
    struct FreeDeleter {
        void operator()(void* ptr) const {
            free(ptr);
        }
    };

    char *fake_alloc(char **memory, size_t size) {
        char *allocated = *memory;
        char *last = allocated + size;
        *last = '\0';
        *memory += size + 1;
        return allocated;
    }

    void chilltrace(const char * const stackEntry) {
        if (stackEntry) {
            fputs(stackEntry,stderr);
        }
    }

    void posixSignalHandler( int signum, siginfo_t* si, void* ucontext ) {
        (void)si;
        (void)signum;
        (void)ucontext;

        auto &handler = PosixCrashHandler::getInstance();
        handler.handleCrash();

        // If you caught one of the above signals, it is likely you just
        // want to quit your program right now.
        //exit( signum );
        std::_Exit(CHILLOUT_EXIT_CODE);
    }

    PosixCrashHandler::PosixCrashHandler():
        m_stackMemory(nullptr),
        m_demangleMemory(nullptr)
    {
        m_stackMemory = (char*)malloc(STACK_MEMORY_SIZE);
        memset(&m_stackMemory[0], 0, STACK_MEMORY_SIZE);

        m_demangleMemory = (char*)malloc(DEMANGLE_MEMORY_SIZE);
        memset(&m_demangleMemory[0], 0, DEMANGLE_MEMORY_SIZE);

        m_backtraceCallback = chilltrace;
    }

    PosixCrashHandler::~PosixCrashHandler() {
        free(m_stackMemory);
        free(m_demangleMemory);
    }

    void PosixCrashHandler::setup(const std::string &appName, const std::string &crashDirPath) {
        struct sigaction sa;
        sa.sa_sigaction = posixSignalHandler;
        sigemptyset( &sa.sa_mask );

#ifdef __APPLE__
        /* for some reason we backtrace() doesn't work on osx
       when we use an alternate stack */
        sa.sa_flags = SA_SIGINFO;
#else
        sa.sa_flags = SA_SIGINFO | SA_ONSTACK;
#endif

        sigaction( SIGABRT, &sa, NULL );
        sigaction( SIGSEGV, &sa, NULL );
        sigaction( SIGBUS,  &sa, NULL );
        sigaction( SIGILL,  &sa, NULL );
        sigaction( SIGFPE,  &sa, NULL );
        sigaction( SIGPIPE, &sa, NULL );
        sigaction( SIGTERM, &sa, NULL );

        if (!crashDirPath.empty()) {
            std::string path = crashDirPath;
            while ((path.size() > 1) &&
                   (path[path.size() - 1] == '/')) {
                path.erase(path.size() - 1);
            }

            std::stringstream s;
            s << path << "/" << appName << "_";
            formatDateTime(s, now(), CHILLOUT_DATETIME);
            s << ".bktr";
            m_backtraceFilePath = s.str();
        }
    }

    void PosixCrashHandler::teardown() {
        struct sigaction sa;
        sigset_t mysigset;
        
        sigemptyset(&mysigset);
        
        sa.sa_handler = SIG_DFL;
        sa.sa_mask = mysigset;
        sa.sa_flags = 0;

        sigaction( SIGABRT, &sa, NULL );
        sigaction( SIGSEGV, &sa, NULL );
        sigaction( SIGBUS,  &sa, NULL );
        sigaction( SIGILL,  &sa, NULL );
        sigaction( SIGFPE,  &sa, NULL );
        sigaction( SIGPIPE, &sa, NULL );
        sigaction( SIGTERM, &sa, NULL );
    }

    void PosixCrashHandler::handleCrash() {
        if (m_crashCallback) {
            m_crashCallback();
        }
    }
    
    void PosixCrashHandler::setCrashCallback(const std::function<void()> &callback) {
        m_crashCallback = callback;
    }
    
    void PosixCrashHandler::setBacktraceCallback(const std::function<void(const char * const)> &callback) {
        m_backtraceCallback = callback;
    }
}

#endif
