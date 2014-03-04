#include "clicommandpwd.h"
#include "unused.h"
#include <QDir>

bool CliCommandPwd::execute(QStringList args)
{
    UNUSED(args);
    QDir dir;
    println(dir.absolutePath());
    return false;
}

bool CliCommandPwd::validate(QStringList args)
{
    if (args.size() != 0)
    {
        printUsage();
        return false;
    }
    return true;
}

QString CliCommandPwd::shortHelp() const
{
    return tr("prints the current working directory");
}

QString CliCommandPwd::fullHelp() const
{
    return tr(
                "This is the same as 'pwd' command on Unix systems and 'cd' command without arguments on Windows. "
                "It prints current working directory. You can change the current working directory with %1 command "
                "and you can also list contents of the current working directory with %2 command."
             ).arg(cmdName("cd")).arg(cmdName("dir"));
}

QString CliCommandPwd::usage() const
{
    return "pwd";
}
