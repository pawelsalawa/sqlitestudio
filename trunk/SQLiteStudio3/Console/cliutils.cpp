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

QStringList toAsciiTree(const AsciiTree& hash, QList<bool> indents)
{
    if (hash.size() == 0)
        return QStringList();

    static const QString indentStr = "|   ";
    static const QString indentStrEmpty = "    ";
    static const QString branchStr = "+-";
    static const QString branchStrLast = "`-";
    QStringList lines;
    QString line;
    bool isLast = false;
    QString lastKey = hash.lastKey();
    QList<bool> subIndents;
    AsciiTree subHash;
    foreach (const QString& key, hash.keys())
    {
        isLast = (key == lastKey);

        foreach (bool indent, indents.mid(0, indents.size() - 1))
            line += (indent ? indentStr : indentStrEmpty);

        line += (isLast ? branchStrLast : branchStr);
        line += key;

        lines << line;

        if (!hash[key].isNull())
        {
            subHash = hash[key].value<AsciiTree>();
            subIndents = indents;
            subIndents << !isLast;
            lines += toAsciiTree(subHash, subIndents);
        }
    }
    return lines;
}

QString toAsciiTree(const AsciiTree& tree)
{
    QStringList lines;
    QString lastKey = tree.lastKey();
    AsciiTree subHash;
    QList<bool> subIndents;
    bool isLast;
    foreach (const QString& key, tree.keys())
    {
        isLast = (key == lastKey);
        lines << key;

        if (!tree[key].isNull())
        {
            subHash = tree[key].value<AsciiTree>();
            subIndents = {!isLast};
            lines += toAsciiTree(subHash, subIndents);
        }
    }
    return lines.join("\n");
}

void initCliUtils()
{
    qRegisterMetaType<AsciiTree>();
}
