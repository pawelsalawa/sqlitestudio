#include "clicommandcd.h"
#include <QCoreApplication>
#include <QDir>

bool CliCommandCd::execute(QStringList args)
{
    QDir dir;
    dir.cd(args[0]);
    if (QDir::setCurrent(dir.absolutePath()))
        println(tr("Changed directory to: %1").arg(QDir::currentPath()));
    else
        println(tr("Could not change directory to: %1").arg(QDir::currentPath()));

    return false;
}

bool CliCommandCd::validate(QStringList args)
{
    if (args.size() != 1)
    {
        printUsage();
        return false;
    }
    return true;
}

QString CliCommandCd::shortHelp() const
{
    return tr("changes current workind directory");
}

QString CliCommandCd::fullHelp() const
{
    return tr(
                "Very similar command to 'cd' known from Unix systems and Windows. "
                "It requires a <path> argument to be passed, therefore calling %1 will always cause a change of the directory. "
                "To learn what's the current working directory use %2 command and to list contents of the current working directory "
                "use %3 command."
             );
}

QString CliCommandCd::usage() const
{
    return "cd "+tr("<path>");
}
