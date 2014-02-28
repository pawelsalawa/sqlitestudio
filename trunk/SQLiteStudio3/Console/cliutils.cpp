#include "cliutils.h"
#include <QtGlobal>

#if defined(Q_OS_WIN32)
#include <windows.h>
#elif defined(Q_OS_UNIX)
#include <sys/ioctl.h>
#include <unistd.h>
#endif

#if defined(Q_OS_WIN32)

int getCliColumns()
{
    CONSOLE_SCREEN_BUFFER_INFO data;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &data);
    return data.dwSize.X;
}

int getCliRows()
{
    CONSOLE_SCREEN_BUFFER_INFO data;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &data);
    return data.dwSize.Y;
}

#elif defined(Q_OS_UNIX)

int getCliColumns()
{
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    return w.ws_col;
}

int getCliRows()
{
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    return w.ws_row;
}

#endif
