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

QStringList toAsciiTree(const AsciiTree& tree, const QList<bool>& indents, bool topLevel, bool lastNode)
{
    static const QString indentStr = "  | ";
    static const QString indentStrEmpty = "    ";
    static const QString branchStr = "  +-";
    static const QString branchStrLast = "  `-";

    QStringList lines;
    QString line;

    if (!topLevel)
    {
        // Draw indent before this node
        foreach (bool indent, indents)
            line += (indent ? indentStr : indentStrEmpty);

        // Draw node prefix
        line += (lastNode ? branchStrLast : branchStr);
    }

    // Draw label
    line += tree.label;
    lines << line;

    if (tree.childs.size() == 0)
        return lines;

    // Draw childs
    int i = 0;
    int lastIdx = tree.childs.size() - 1;
    QList<bool> subIndents = indents;

    if (!topLevel)
        subIndents << (lastNode ? false : true);

    foreach (const AsciiTree& subTree, tree.childs)
    {
        lines += toAsciiTree(subTree, subIndents, false, i == lastIdx);
        i++;
    }

    return lines;
}

QString toAsciiTree(const AsciiTree& tree)
{
    QList<bool> subIndents;
    QStringList lines = toAsciiTree(tree, subIndents, true, true);
    return lines.join("\n");
}

void initCliUtils()
{
    qRegisterMetaType<AsciiTree>();
}
