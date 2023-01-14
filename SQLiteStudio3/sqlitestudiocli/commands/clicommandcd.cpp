#include "clicommandcd.h"
#include <QCoreApplication>
#include <QDir>

void CliCommandCd::execute()
{
    QDir dir;
    dir.cd(syntax.getArgument(DIR_PATH));
    if (QDir::setCurrent(dir.absolutePath()))
        println(tr("Changed directory to: %1").arg(QDir::currentPath()));
    else
        println(tr("Could not change directory to: %1").arg(QDir::currentPath()));
}

QString CliCommandCd::shortHelp() const
{
    return tr("changes current working directory");
}

QString CliCommandCd::fullHelp() const
{
    return tr(
                "Very similar command to 'cd' known from Unix systems and Windows. "
                "It requires a <path> argument to be passed, therefore calling %1 will always cause a change of the directory. "
                "To learn what's the current working directory use %2 command and to list contents of the current working directory "
                "use %3 command."
               ).arg(cmdName("cd"), cmdName("pwd"), cmdName("ls"));
}

void CliCommandCd::defineSyntax()
{
    syntax.setName("cd");
    syntax.addArgument(DIR_PATH, tr("path", "CLI command syntax"));
}
