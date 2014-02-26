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
