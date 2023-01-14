#include "clicommandpwd.h"
#include "common/unused.h"
#include <QDir>

void CliCommandPwd::execute()
{
    QDir dir;
    println(dir.absolutePath());
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
                ).arg(cmdName("cd"), cmdName("dir"));
}

void CliCommandPwd::defineSyntax()
{
    syntax.setName("pwd");
}
