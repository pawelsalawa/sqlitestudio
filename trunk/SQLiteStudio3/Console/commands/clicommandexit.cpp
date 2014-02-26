#include "clicommandexit.h"
#include "cli.h"
#include "unused.h"

CliCommandExit *CliCommandExit::create()
{
    return new CliCommandExit();
}

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
