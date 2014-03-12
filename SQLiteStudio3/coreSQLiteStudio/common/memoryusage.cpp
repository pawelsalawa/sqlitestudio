#include "memoryusage.h"
#include <QtGlobal>

#ifdef Q_OS_LINUX
#include <QFile>
#include <QRegularExpression>
#else

#ifdef Q_OS_WIN32
#include "windows.h"
#include "psapi.h"
#else

#ifdef Q_OS_MAC
#include <mach/mach.h>
#endif // Q_OS_MAC

#endif // Q_OS_WIN32
#endif // Q_OS_LINUX

#ifdef Q_OS_LINUX

int getMemoryUsage()
{
    static const QRegularExpression re("VmSize\\:\\s+(\\d+)\\s+(\\w+)");

    QFile file("/proc/self/status");
    if (!file.open(QIODevice::ReadOnly))
        return -1;

    QString contents = file.readAll();
    QRegularExpressionMatch match = re.match(contents);
    if (!match.hasMatch())
        return -1;

    bool ok;
    int result = match.captured(1).toInt(&ok);
    if (!ok)
        return -1;

    QString unit = match.captured(2).toLower();
    if (unit == "mb")
        return result * 1024 * 1024;

    if (unit == "kb")
        return result * 1024;

    return result;
}

#else
#ifdef Q_OS_WIN32

int getMemoryUsage()
{
    PROCESS_MEMORY_COUNTERS_EX pmc;
    GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc));
    return pmc.PrivateUsage;
}

#else
#ifdef Q_OS_MAC

int getMemoryUsage()
{
    struct task_basic_info t_info;
    mach_msg_type_number_t t_info_count = TASK_BASIC_INFO_COUNT;

    if (KERN_SUCCESS != task_info(mach_task_self(), TASK_BASIC_INFO, (task_info_t)&t_info, &t_info_count))
        return -1;

    return t_info.virtual_size;
}

#else
int getMemoryUsage()
{
    return -1;
}

#endif // Q_OS_MAC
#endif // Q_OS_WIN32
#endif // Q_OS_LINUX
