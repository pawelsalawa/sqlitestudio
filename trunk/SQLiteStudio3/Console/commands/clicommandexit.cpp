#include "clicommandexit.h"
#include "cli.h"
#include "unused.h"

bool CliCommandExit::execute(QStringList args)
{
    UNUSED(args);
    cli->exit();

    return false;
}

bool CliCommandExit::validate(QStringList args)
{
    UNUSED(args);
    return true;
}

QString CliCommandExit::shortHelp() const
{
    return tr("quits the application");
}

QString CliCommandExit::fullHelp() const
{
    return tr(
                "Quits the application. Settings are stored in configuration file and will be restored on next startup."
             );
}

QString CliCommandExit::usage() const
{
    return "exit";
}

QStringList CliCommandExit::aliases() const
{
    return {"quit"};
}
