#include "cliutils.h"
#include <QtGlobal>
#ifdef Q_OS_WIN32
#include <windows.h>
#endif

int getCliColumns()
{
#ifdef Q_OS_WIN32
    CONSOLE_SCREEN_BUFFER_INFO data;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &data);
    return data.dwSize.X;
#endif
}

int getCliRows()
{
#ifdef Q_OS_WIN32
    CONSOLE_SCREEN_BUFFER_INFO data;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &data);
    return data.dwSize.Y;
#endif
}
