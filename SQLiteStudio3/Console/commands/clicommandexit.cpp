#include "clicommandexit.h"
#include "cli.h"
#include "unused.h"

CliCommandExit *CliCommandExit::create()
{
    return new CliCommandExit();
}

void CliCommandExit::execute(QStringList args)
{
    UNUSED(args);
    cli->exit();
}

bool CliCommandExit::validate(QStringList args)
{
    UNUSED(args);
    return true;
}
